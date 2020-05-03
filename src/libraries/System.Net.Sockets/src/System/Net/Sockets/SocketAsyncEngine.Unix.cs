// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;

namespace System.Net.Sockets
{
    internal sealed unsafe class SocketAsyncEngine : IThreadPoolWorkItem
    {
        //
        // Encapsulates a particular SocketAsyncContext object's access to a SocketAsyncEngine.
        //
        public readonly struct Token
        {
            private readonly SocketAsyncEngine? _engine;
            private readonly IntPtr _handle;

            public Token(SocketAsyncContext context)
            {
                AllocateToken(context, out _engine, out _handle);
            }

            public bool WasAllocated
            {
                get { return _engine != null; }
            }

            public void Free()
            {
                if (WasAllocated)
                {
                    _engine!.FreeHandle(_handle);
                }
            }

            public bool TryRegister(SafeSocketHandle socket, out Interop.Error error)
            {
                Debug.Assert(WasAllocated, "Expected WasAllocated to be true");
                return _engine!.TryRegister(socket, _handle, out error);
            }
        }

        private const int EventBufferCount =
#if DEBUG
            32;
#else
            1024;
#endif

        private static readonly object s_lock = new object();

        // In debug builds, force there to be 2 engines. In release builds, use half the number of processors when
        // there are at least 6. The lower bound is to avoid using multiple engines on systems which aren't servers.
#pragma warning disable CA1802 // const works for debug, but needs to be static readonly for release
        private static readonly int s_engineCount =
#if DEBUG
            2;
#else
            Environment.ProcessorCount >= 6 ? Environment.ProcessorCount / 2 : 1;
#endif
#pragma warning restore CA1802

        //
        // The current engines. We replace an engine when it runs out of "handle" values.
        // Must be accessed under s_lock.
        //
        private static readonly SocketAsyncEngine?[] s_currentEngines = new SocketAsyncEngine?[s_engineCount];
        private static int s_allocateFromEngine = 0;

        private readonly IntPtr _port;
        private readonly Interop.Sys.SocketEvent* _buffer;

        //
        // The read and write ends of a native pipe, used to signal that this instance's event loop should stop
        // processing events.
        //
        private readonly int _shutdownReadPipe;
        private readonly int _shutdownWritePipe;

        //
        // Each SocketAsyncContext is associated with a particular "handle" value, used to identify that
        // SocketAsyncContext when events are raised.  These handle values are never reused, because we do not have
        // a way to ensure that we will never see an event for a socket/handle that has been freed.  Instead, we
        // allocate monotonically increasing handle values up to some limit; when we would exceed that limit,
        // we allocate a new SocketAsyncEngine (and thus a new event port) and start the handle values over at zero.
        // Thus we can uniquely identify a given SocketAsyncContext by the *pair* {SocketAsyncEngine, handle},
        // and avoid any issues with misidentifying the target of an event we read from the port.
        //
#if DEBUG
        //
        // In debug builds, force rollover to new SocketAsyncEngine instances so that code doesn't go untested, since
        // it's very unlikely that the "real" limits will ever be reached in test code.
        //
        private static readonly IntPtr MaxHandles = (IntPtr)(EventBufferCount * 2);
#else
        //
        // In release builds, we use *very* high limits.  No 64-bit process running on release builds should ever
        // reach the handle limit for a single event port, and even 32-bit processes should see this only very rarely.
        //
        private static readonly IntPtr MaxHandles = IntPtr.Size == 4 ? (IntPtr)int.MaxValue : (IntPtr)long.MaxValue;
#endif
        private static readonly IntPtr MinHandlesForAdditionalEngine = s_engineCount == 1 ? MaxHandles : (IntPtr)32;

        //
        // Sentinel handle value to identify events from the "shutdown pipe," used to signal an event loop to stop
        // processing events.
        //
        private static readonly IntPtr ShutdownHandle = (IntPtr)(-1);

        //
        // The next handle value to be allocated for this event port.
        // Must be accessed under s_lock.
        //
        private IntPtr _nextHandle;

        //
        // Count of handles that have been allocated for this event port, but not yet freed.
        // Must be accessed under s_lock.
        //
        private IntPtr _outstandingHandles;

        //
        // Maps handle values to SocketAsyncContext instances.
        //
        private readonly ConcurrentDictionary<IntPtr, SocketAsyncContext> _handleToContextMap = new ConcurrentDictionary<IntPtr, SocketAsyncContext>();

        //
        // Queue of events generated by EventLoop() that would be processed by the thread pool
        //
        private readonly ConcurrentQueue<SocketIOEvent> _eventQueue = new ConcurrentQueue<SocketIOEvent>();

        //
        // This field is set to 1 to indicate that a thread pool work item is scheduled to process events in _eventQueue. It is
        // set to 0 when the scheduled work item starts running, to indicate that a thread pool work item to process events is
        // not scheduled. Changes are protected by atomic operations as appropriate.
        //
        private int _eventQueueProcessingRequested;

        //
        // True if we've reached the handle value limit for this event port, and thus must allocate a new event port
        // on the next handle allocation.
        //
        private bool IsFull { get { return _nextHandle == MaxHandles; } }

        // True if we've don't have sufficient active sockets to allow allocating a new engine.
        private bool HasLowNumberOfSockets
        {
            get
            {
                return IntPtr.Size == 4 ? _outstandingHandles.ToInt32() < MinHandlesForAdditionalEngine.ToInt32() :
                                          _outstandingHandles.ToInt64() < MinHandlesForAdditionalEngine.ToInt64();
            }
        }

        //
        // Allocates a new {SocketAsyncEngine, handle} pair.
        //
        private static void AllocateToken(SocketAsyncContext context, out SocketAsyncEngine? engine, out IntPtr handle)
        {
            lock (s_lock)
            {
                engine = s_currentEngines[s_allocateFromEngine];
                if (engine == null)
                {
                    // We minimize the number of engines on applications that have a low number of concurrent sockets.
                    for (int i = 0; i < s_allocateFromEngine; i++)
                    {
                        var previousEngine = s_currentEngines[i];
                        if (previousEngine == null || previousEngine.HasLowNumberOfSockets)
                        {
                            s_allocateFromEngine = i;
                            engine = previousEngine;
                            break;
                        }
                    }
                    if (engine == null)
                    {
                        s_currentEngines[s_allocateFromEngine] = engine = new SocketAsyncEngine();
                    }
                }

                handle = engine.AllocateHandle(context);

                if (engine.IsFull)
                {
                    // We'll need to create a new event port for the next handle.
                    s_currentEngines[s_allocateFromEngine] = null;
                }

                // Round-robin to the next engine once we have sufficient sockets on this one.
                if (!engine.HasLowNumberOfSockets)
                {
                    s_allocateFromEngine = (s_allocateFromEngine + 1) % s_engineCount;
                }
            }
        }

        private IntPtr AllocateHandle(SocketAsyncContext context)
        {
            Debug.Assert(Monitor.IsEntered(s_lock), "Expected s_lock to be held");
            Debug.Assert(!IsFull, "Expected !IsFull");

            IntPtr handle = _nextHandle;
            _handleToContextMap.TryAdd(handle, context);

            _nextHandle = IntPtr.Add(_nextHandle, 1);
            _outstandingHandles = IntPtr.Add(_outstandingHandles, 1);

            Debug.Assert(handle != ShutdownHandle, $"Expected handle != ShutdownHandle: {handle}");
            return handle;
        }

        private void FreeHandle(IntPtr handle)
        {
            Debug.Assert(handle != ShutdownHandle, $"Expected handle != ShutdownHandle: {handle}");

            bool shutdownNeeded = false;

            lock (s_lock)
            {
                if (_handleToContextMap.TryRemove(handle, out _))
                {
                    _outstandingHandles = IntPtr.Subtract(_outstandingHandles, 1);
                    Debug.Assert(_outstandingHandles.ToInt64() >= 0, $"Unexpected _outstandingHandles: {_outstandingHandles}");

                    //
                    // If we've allocated all possible handles for this instance, and freed them all, then
                    // we don't need the event loop any more, and can reclaim resources.
                    //
                    if (IsFull && _outstandingHandles == IntPtr.Zero)
                    {
                        shutdownNeeded = true;
                    }
                }
            }

            //
            // Signal shutdown outside of the lock to reduce contention.
            //
            if (shutdownNeeded)
            {
                RequestEventLoopShutdown();
            }
        }

        private SocketAsyncEngine()
        {
            _port = (IntPtr)(-1);
            _shutdownReadPipe = -1;
            _shutdownWritePipe = -1;
            try
            {
                //
                // Create the event port and buffer
                //
                Interop.Error err = Interop.Sys.CreateSocketEventPort(out _port);
                if (err != Interop.Error.SUCCESS)
                {
                    throw new InternalException(err);
                }
                err = Interop.Sys.CreateSocketEventBuffer(EventBufferCount, out _buffer);
                if (err != Interop.Error.SUCCESS)
                {
                    throw new InternalException(err);
                }

                //
                // Create the pipe for signaling shutdown, and register for "read" events for the pipe.  Now writing
                // to the pipe will send an event to the event loop.
                //
                int* pipeFds = stackalloc int[2];
                int pipeResult = Interop.Sys.Pipe(pipeFds, Interop.Sys.PipeFlags.O_CLOEXEC);
                if (pipeResult != 0)
                {
                    throw new InternalException(pipeResult);
                }
                _shutdownReadPipe = pipeFds[Interop.Sys.ReadEndOfPipe];
                _shutdownWritePipe = pipeFds[Interop.Sys.WriteEndOfPipe];

                err = Interop.Sys.TryChangeSocketEventRegistration(_port, (IntPtr)_shutdownReadPipe, Interop.Sys.SocketEvents.None, Interop.Sys.SocketEvents.Read, ShutdownHandle);
                if (err != Interop.Error.SUCCESS)
                {
                    throw new InternalException(err);
                }

                bool suppressFlow = !ExecutionContext.IsFlowSuppressed();
                try
                {
                    if (suppressFlow) ExecutionContext.SuppressFlow();

                    Thread thread = new Thread(s => ((SocketAsyncEngine)s!).EventLoop());
                    thread.IsBackground = true;
                    thread.Name = ".NET Sockets";
                    thread.Start(this);
                }
                finally
                {
                    if (suppressFlow) ExecutionContext.RestoreFlow();
                }
            }
            catch
            {
                FreeNativeResources();
                throw;
            }
        }

        private void EventLoop()
        {
            try
            {
                bool shutdown = false;
                Interop.Sys.SocketEvent* buffer = _buffer;
                ConcurrentDictionary<IntPtr, SocketAsyncContext> handleToContextMap = _handleToContextMap;
                ConcurrentQueue<SocketIOEvent> eventQueue = _eventQueue;
                while (!shutdown)
                {
                    int numEvents = EventBufferCount;
                    Interop.Error err = Interop.Sys.WaitForSocketEvents(_port, buffer, &numEvents);
                    if (err != Interop.Error.SUCCESS)
                    {
                        throw new InternalException(err);
                    }

                    // The native shim is responsible for ensuring this condition.
                    Debug.Assert(numEvents > 0, $"Unexpected numEvents: {numEvents}");

                    bool enqueuedEvent = false;
                    for (int i = 0; i < numEvents; i++)
                    {
                        IntPtr handle = buffer[i].Data;
                        if (handle == ShutdownHandle)
                        {
                            shutdown = true;
                        }
                        else
                        {
                            Debug.Assert(handle.ToInt64() < MaxHandles.ToInt64(), $"Unexpected values: handle={handle}, MaxHandles={MaxHandles}");
                            handleToContextMap.TryGetValue(handle, out SocketAsyncContext? context);
                            if (context != null)
                            {
                                Interop.Sys.SocketEvents events = buffer[i].Events;
                                events = context.HandleSyncEventsSpeculatively(events);
                                if (events != Interop.Sys.SocketEvents.None)
                                {
                                    var ev = new SocketIOEvent(context, events);
                                    eventQueue.Enqueue(ev);
                                    enqueuedEvent = true;

                                    // This is necessary when the JIT generates unoptimized code (debug builds, live debugging,
                                    // quick JIT, etc.) to ensure that the context does not remain referenced by this method, as
                                    // such code may keep the stack location live for longer than necessary
                                    ev = default;
                                }

                                // This is necessary when the JIT generates unoptimized code (debug builds, live debugging,
                                // quick JIT, etc.) to ensure that the context does not remain referenced by this method, as
                                // such code may keep the stack location live for longer than necessary
                                context = null;
                            }
                        }
                    }

                    if (enqueuedEvent)
                    {
                        ScheduleToProcessEvents();
                    }
                }

                FreeNativeResources();
            }
            catch (Exception e)
            {
                Environment.FailFast("Exception thrown from SocketAsyncEngine event loop: " + e.ToString(), e);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private void ScheduleToProcessEvents()
        {
            // Schedule a thread pool work item to process events. Only one work item is scheduled at any given time to avoid
            // over-parallelization. When the work item begins running, this field is reset to 0, allowing for another work item
            // to be scheduled for parallelizing processing of events.
            if (Interlocked.CompareExchange(ref _eventQueueProcessingRequested, 1, 0) == 0)
            {
                ThreadPool.UnsafeQueueUserWorkItem(this, preferLocal: false);
            }
        }

        void IThreadPoolWorkItem.Execute()
        {
            // Indicate that a work item is no longer scheduled to process events. The change needs to be visible to enqueuer
            // threads (only for EventLoop() currently) before an event is attempted to be dequeued. In particular, if an
            // enqueuer queues an event and does not schedule a work item because it is already scheduled, and this thread is
            // the last thread processing events, it must see the event queued by the enqueuer.
            Interlocked.Exchange(ref _eventQueueProcessingRequested, 0);

            ConcurrentQueue<SocketIOEvent> eventQueue = _eventQueue;
            if (!eventQueue.TryDequeue(out SocketIOEvent ev))
            {
                return;
            }

            int startTimeMs = Environment.TickCount;

            // An event was successfully dequeued, and there may be more events to process. Schedule a work item to parallelize
            // processing of events, before processing more events. Following this, it is the responsibility of the new work
            // item and the epoll thread to schedule more work items as necessary. The parallelization may be necessary here if
            // the user callback as part of handling the event blocks for some reason that may have a dependency on other queued
            // socket events.
            ScheduleToProcessEvents();

            while (true)
            {
                ev.Context.HandleEvents(ev.Events);

                // If there is a constant stream of new events, and/or if user callbacks take long to process an event, this
                // work item may run for a long time. If work items of this type are using up all of the thread pool threads,
                // collectively they may starve other types of work items from running. Before dequeuing and processing another
                // event, check the elapsed time since the start of the work item and yield the thread after some time has
                // elapsed to allow the thread pool to run other work items.
                //
                // The threshold chosen below was based on trying various thresholds and in trying to keep the latency of
                // running another work item low when these work items are using up all of the thread pool worker threads. In
                // such cases, the latency would be something like threshold / proc count. Smaller thresholds were tried and
                // using Stopwatch instead (like 1 ms, 5 ms, etc.), from quick tests they appeared to have a slightly greater
                // impact on throughput compared to the threshold chosen below, though it is slight enough that it may not
                // matter much. Higher thresholds didn't seem to have any noticeable effect.
                if (Environment.TickCount - startTimeMs >= 15)
                {
                    break;
                }

                if (!eventQueue.TryDequeue(out ev))
                {
                    return;
                }
            }

            // The queue was not observed to be empty, schedule another work item before yielding the thread
            ScheduleToProcessEvents();
        }

        private void RequestEventLoopShutdown()
        {
            //
            // Write to the pipe, which will wake up the event loop and cause it to exit.
            //
            byte b = 1;
            int bytesWritten = Interop.Sys.Write((IntPtr)_shutdownWritePipe, &b, 1);
            if (bytesWritten != 1)
            {
                throw new InternalException(bytesWritten);
            }
        }

        private void FreeNativeResources()
        {
            if (_shutdownReadPipe != -1)
            {
                Interop.Sys.Close((IntPtr)_shutdownReadPipe);
            }
            if (_shutdownWritePipe != -1)
            {
                Interop.Sys.Close((IntPtr)_shutdownWritePipe);
            }
            if (_buffer != null)
            {
                Interop.Sys.FreeSocketEventBuffer(_buffer);
            }
            if (_port != (IntPtr)(-1))
            {
                Interop.Sys.CloseSocketEventPort(_port);
            }
        }

        private bool TryRegister(SafeSocketHandle socket, IntPtr handle, out Interop.Error error)
        {
            error = Interop.Sys.TryChangeSocketEventRegistration(_port, socket, Interop.Sys.SocketEvents.None,
                Interop.Sys.SocketEvents.Read | Interop.Sys.SocketEvents.Write, handle);
            return error == Interop.Error.SUCCESS;
        }

        private readonly struct SocketIOEvent
        {
            public SocketAsyncContext Context { get; }
            public Interop.Sys.SocketEvents Events { get; }

            public SocketIOEvent(SocketAsyncContext context, Interop.Sys.SocketEvents events)
            {
                Context = context;
                Events = events;
            }
        }
    }
}
