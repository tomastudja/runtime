// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Collections.Concurrent;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Threading;

namespace System.Net.Sockets
{
    internal sealed unsafe class SocketAsyncEngine : IThreadPoolWorkItem
    {
        private const int EventBufferCount =
#if DEBUG
            32;
#else
            1024;
#endif

        // Socket continuations are dispatched to the ThreadPool from the event thread.
        // This avoids continuations blocking the event handling.
        // Setting PreferInlineCompletions allows continuations to run directly on the event thread.
        // PreferInlineCompletions defaults to false and can be set to true using the DOTNET_SYSTEM_NET_SOCKETS_INLINE_COMPLETIONS envvar.
        internal static readonly bool InlineSocketCompletionsEnabled = Environment.GetEnvironmentVariable("DOTNET_SYSTEM_NET_SOCKETS_INLINE_COMPLETIONS") == "1";

        private static int GetEngineCount()
        {
            // The responsibility of SocketAsyncEngine is to get notifications from epoll|kqueue
            // and schedule corresponding work items to ThreadPool (socket reads and writes).
            //
            // Using TechEmpower benchmarks that generate a LOT of SMALL socket reads and writes under a VERY HIGH load
            // we have observed that a single engine is capable of keeping busy up to thirty x64 and eight ARM64 CPU Cores.
            //
            // The vast majority of real-life scenarios is never going to generate such a huge load (hundreds of thousands of requests per second)
            // and having a single producer should be almost always enough.
            //
            // We want to be sure that we can handle extreme loads and that's why we have decided to use these values.
            //
            // It's impossible to predict all possible scenarios so we have added a possibility to configure this value using environment variables.
            if (uint.TryParse(Environment.GetEnvironmentVariable("DOTNET_SYSTEM_NET_SOCKETS_THREAD_COUNT"), out uint count))
            {
                return (int)count;
            }

            // When inlining continuations, we default to ProcessorCount to make sure event threads cannot be a bottleneck.
            if (InlineSocketCompletionsEnabled)
            {
                return Environment.ProcessorCount;
            }

            Architecture architecture = RuntimeInformation.ProcessArchitecture;
            int coresPerEngine = architecture == Architecture.Arm64 || architecture == Architecture.Arm
                ? 8
                : 30;

            return Math.Max(1, (int)Math.Round(Environment.ProcessorCount / (double)coresPerEngine));
        }

        private static readonly SocketAsyncEngine[] s_engines = CreateEngines();
        private static int s_allocateFromEngine = -1;

        private static SocketAsyncEngine[] CreateEngines()
        {
            int engineCount = GetEngineCount();

            var engines = new SocketAsyncEngine[engineCount];

            for (int i = 0; i < engineCount; i++)
            {
                engines[i] = new SocketAsyncEngine();
            }

            return engines;
        }

        private readonly IntPtr _port;
        private readonly Interop.Sys.SocketEvent* _buffer;

        //
        // Maps handle values to SocketAsyncContext instances.
        //
        private readonly ConcurrentDictionary<IntPtr, SocketAsyncContextWrapper> _handleToContextMap = new ConcurrentDictionary<IntPtr, SocketAsyncContextWrapper>();

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
        // Registers the Socket with a SocketAsyncEngine, and returns the associated engine.
        //
        public static SocketAsyncEngine RegisterSocket(IntPtr socketHandle, SocketAsyncContext context)
        {
            int engineIndex = Math.Abs(Interlocked.Increment(ref s_allocateFromEngine) % s_engines.Length);
            SocketAsyncEngine engine = s_engines[engineIndex];
            engine.RegisterCore(socketHandle, context);
            return engine;
        }

        private void RegisterCore(IntPtr socketHandle, SocketAsyncContext context)
        {
            bool added = _handleToContextMap.TryAdd(socketHandle, new SocketAsyncContextWrapper(context));
            if (!added)
            {
                // Using public SafeSocketHandle(IntPtr) a user can add the same handle
                // from a different Socket instance.
                throw new InvalidOperationException("Handle is already used by another Socket.");
            }

            Interop.Error error = Interop.Sys.TryChangeSocketEventRegistration(_port, socketHandle, Interop.Sys.SocketEvents.None,
                Interop.Sys.SocketEvents.Read | Interop.Sys.SocketEvents.Write, socketHandle);
            if (error == Interop.Error.SUCCESS)
            {
                return;
            }

            _handleToContextMap.TryRemove(socketHandle, out _);
            if (error == Interop.Error.ENOMEM || error == Interop.Error.ENOSPC)
            {
                throw new OutOfMemoryException();
            }
            else
            {
                throw new InternalException(error);
            }
        }

        public void UnregisterSocket(IntPtr socketHandle)
        {
            _handleToContextMap.TryRemove(socketHandle, out _);
        }

        private SocketAsyncEngine()
        {
            _port = (IntPtr)(-1);
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
                Interop.Sys.SocketEvent* buffer = _buffer;
                ConcurrentDictionary<IntPtr, SocketAsyncContextWrapper> handleToContextMap = _handleToContextMap;
                ConcurrentQueue<SocketIOEvent> eventQueue = _eventQueue;
                SocketAsyncContext? context = null;
                while (true)
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
                    foreach (var socketEvent in new ReadOnlySpan<Interop.Sys.SocketEvent>(buffer, numEvents))
                    {
                        IntPtr handle = socketEvent.Data;

                        if (handleToContextMap.TryGetValue(handle, out SocketAsyncContextWrapper contextWrapper) && (context = contextWrapper.Context) != null)
                        {
                            if (context.PreferInlineCompletions)
                            {
                                context.HandleEventsInline(socketEvent.Events);
                            }
                            else
                            {
                                Interop.Sys.SocketEvents events = context.HandleSyncEventsSpeculatively(socketEvent.Events);

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
                            }

                            // This is necessary when the JIT generates unoptimized code (debug builds, live debugging,
                            // quick JIT, etc.) to ensure that the context does not remain referenced by this method, as
                            // such code may keep the stack location live for longer than necessary
                            context = null;
                            contextWrapper = default;
                        }
                    }

                    if (enqueuedEvent)
                    {
                        ScheduleToProcessEvents();
                    }
                }
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

        private void FreeNativeResources()
        {
            if (_buffer != null)
            {
                Interop.Sys.FreeSocketEventBuffer(_buffer);
            }
            if (_port != (IntPtr)(-1))
            {
                Interop.Sys.CloseSocketEventPort(_port);
            }
        }

        // struct wrapper is used in order to improve the performance of the epoll thread hot path by up to 3% of some TechEmpower benchmarks
        // the goal is to have a dedicated generic instantiation and using:
        // System.Collections.Concurrent.ConcurrentDictionary`2[System.IntPtr,System.Net.Sockets.SocketAsyncContextWrapper]::TryGetValueInternal(!0,int32,!1&)
        // instead of:
        // System.Collections.Concurrent.ConcurrentDictionary`2[System.IntPtr,System.__Canon]::TryGetValueInternal(!0,int32,!1&)
        private readonly struct SocketAsyncContextWrapper
        {
            public SocketAsyncContextWrapper(SocketAsyncContext context) => Context = context;

            internal SocketAsyncContext Context { get; }
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
