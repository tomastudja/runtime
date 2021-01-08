// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;

namespace System.Threading
{
    //
    // Under netcore, there is only one thread object per thread
    //
    [StructLayout(LayoutKind.Sequential)]
    public partial class Thread
    {
#pragma warning disable 169, 414, 649
        #region Sync with metadata/object-internals.h and InternalThread in mcs/class/corlib/System.Threading/Thread.cs
        private int lock_thread_id;
        // stores a thread handle
        private IntPtr handle;
        private IntPtr native_handle; // used only on Win32
        /* accessed only from unmanaged code */
        private IntPtr name;
        private int name_free; // bool
        private int name_length;
        private ThreadState state;
        private object? abort_exc;
        private int abort_state_handle;
        /* thread_id is only accessed from unmanaged code */
        internal long thread_id;
        private IntPtr debugger_thread; // FIXME switch to bool as soon as CI testing with corlib version bump works
        private UIntPtr static_data; /* GC-tracked */
        private IntPtr runtime_thread_info;
        private object? root_domain_thread;
        internal byte[]? _serialized_principal;
        internal int _serialized_principal_version;
        private IntPtr appdomain_refs;
        private int interruption_requested;
        private IntPtr longlived;
        internal bool threadpool_thread;
        private bool thread_interrupt_requested;
        /* These are used from managed code */
        internal int stack_size;
        internal byte apartment_state;
        internal volatile int critical_region_level;
        internal int managed_id;
        private int small_id;
        private IntPtr manage_callback;
        private IntPtr flags;
        private IntPtr thread_pinning_ref;
        private IntPtr abort_protected_block_count;
        private int priority;
        private IntPtr owned_mutex;
        private IntPtr suspended_event;
        private int self_suspended;
        private IntPtr thread_state;

        private Thread self = null!;
        private object? pending_exception;
        private object? start_obj;

        /* This is used only to check that we are in sync between the representation
         * of MonoInternalThread in native and InternalThread in managed
         *
         * DO NOT RENAME! DO NOT ADD FIELDS AFTER! */
        private IntPtr last;
        #endregion
#pragma warning restore 169, 414, 649

        private string? _name;
        private Delegate? m_start;
        private object? m_start_arg;
        private CultureInfo? culture, ui_culture;
        internal ExecutionContext? _executionContext;
        internal SynchronizationContext? _synchronizationContext;

        // This is used for a quick check on thread pool threads after running a work item to determine if the name, background
        // state, or priority were changed by the work item, and if so to reset it. Other threads may also change some of those,
        // but those types of changes may race with the reset anyway, so this field doesn't need to be synchronized.
        private bool _mayNeedResetForThreadPool;

        private Thread()
        {
            InitInternal(this);
        }

        ~Thread()
        {
            FreeInternal();
        }

        internal static ulong CurrentOSThreadId
        {
            get
            {
                return GetCurrentOSThreadId();
            }
        }

        public bool IsAlive
        {
            get
            {
                ThreadState state = GetState(this);
                return (state & (ThreadState.Unstarted | ThreadState.Stopped | ThreadState.Aborted)) == 0;
            }
        }

        public bool IsBackground
        {
            get
            {
                ThreadState state = ValidateThreadState();
                return (state & ThreadState.Background) != 0;
            }
            set
            {
                ValidateThreadState();
                if (value)
                {
                    SetState(this, ThreadState.Background);
                }
                else
                {
                    ClrState(this, ThreadState.Background);
                    _mayNeedResetForThreadPool = true;
                }
            }
        }

        public bool IsThreadPoolThread
        {
            get
            {
                ValidateThreadState();
                return threadpool_thread;
            }
            internal set
            {
                threadpool_thread = value;
            }
        }

        public int ManagedThreadId => managed_id;

        internal static int OptimalMaxSpinWaitsPerSpinIteration
        {
            get
            {
                // Default from coreclr (src/utilcode/yieldprocessornormalized.cpp)
                return 7;
            }
        }

        public ThreadPriority Priority
        {
            get
            {
                ValidateThreadState();
                return (ThreadPriority)priority;
            }
            set
            {
                // TODO: arguments check
                SetPriority(this, (int)value);
                if (value != ThreadPriority.Normal)
                {
                    _mayNeedResetForThreadPool = true;
                }
            }
        }

        public ThreadState ThreadState => GetState(this);

        public Thread(ThreadStart start)
            : this()
        {
            if (start == null)
            {
                throw new ArgumentNullException(nameof(start));
            }

            Create(start);
        }

        public Thread(ThreadStart start, int maxStackSize)
            : this()
        {
            if (start == null)
            {
                throw new ArgumentNullException(nameof(start));
            }
            if (maxStackSize < 0)
            {
                throw new ArgumentOutOfRangeException(nameof(maxStackSize), SR.ArgumentOutOfRange_NeedNonNegNum);
            }

            Create(start, maxStackSize);
        }

        public Thread(ParameterizedThreadStart start)
            : this()
        {
            if (start == null)
            {
                throw new ArgumentNullException(nameof(start));
            }

            Create(start);
        }

        public Thread(ParameterizedThreadStart start, int maxStackSize)
            : this()
        {
            if (start == null)
            {
                throw new ArgumentNullException(nameof(start));
            }
            if (maxStackSize < 0)
            {
                throw new ArgumentOutOfRangeException(nameof(maxStackSize), SR.ArgumentOutOfRange_NeedNonNegNum);
            }

            Create(start, maxStackSize);
        }

        private void RequireCurrentThread()
        {
            if (this != CurrentThread)
            {
                throw new InvalidOperationException(SR.Thread_Operation_RequiresCurrentThread);
            }
        }

        private void SetCultureOnUnstartedThread(CultureInfo value, bool uiCulture)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value));
            }
            if ((ThreadState & ThreadState.Unstarted) == 0)
            {
                throw new InvalidOperationException(SR.Thread_Operation_RequiresCurrentThread);
            }
            if (uiCulture)
                ui_culture = value;
            else
                culture = value;
        }

        private void Create(ThreadStart start) => SetStartHelper((Delegate)start, 0); // 0 will setup Thread with default stackSize

        private void Create(ThreadStart start, int maxStackSize) => SetStartHelper((Delegate)start, maxStackSize);

        private void Create(ParameterizedThreadStart start) => SetStartHelper((Delegate)start, 0);

        private void Create(ParameterizedThreadStart start, int maxStackSize) => SetStartHelper((Delegate)start, maxStackSize);

        public ApartmentState GetApartmentState() => ApartmentState.Unknown;

        public void DisableComObjectEagerCleanup()
        {
            // no-op
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern int GetCurrentProcessorNumber();

        public static int GetCurrentProcessorId()
        {
            int id = GetCurrentProcessorNumber();

            if (id < 0)
                id = Environment.CurrentManagedThreadId;

            return id;
        }

        public void Interrupt()
        {
            InterruptInternal(this);
        }

        public bool Join(int millisecondsTimeout)
        {
            if (millisecondsTimeout < Timeout.Infinite)
                throw new ArgumentOutOfRangeException(nameof(millisecondsTimeout), millisecondsTimeout, SR.ArgumentOutOfRange_NeedNonNegOrNegative1);
            return JoinInternal(this, millisecondsTimeout);
        }

        private void SetStartHelper(Delegate start, int maxStackSize)
        {
            m_start = start;
            stack_size = maxStackSize;
        }

        public static void SpinWait(int iterations)
        {
            if (iterations < 0)
                return;

            while (iterations-- > 0)
                SpinWait_nop();
        }

        public static void Sleep(int millisecondsTimeout)
        {
            if (millisecondsTimeout < Timeout.Infinite)
                throw new ArgumentOutOfRangeException(nameof(millisecondsTimeout), millisecondsTimeout, SR.ArgumentOutOfRange_NeedNonNegOrNegative1);

            SleepInternal(millisecondsTimeout, true);
        }

        internal static void UninterruptibleSleep0() => SleepInternal(0, false);

#if !TARGET_BROWSER
        internal const bool IsThreadStartSupported = true;

        [UnsupportedOSPlatform("browser")]
        public void Start()
        {
            _executionContext = ExecutionContext.Capture();
            StartInternal(this);
        }

        [UnsupportedOSPlatform("browser")]
        public void Start(object parameter)
        {
            if (m_start is ThreadStart)
                throw new InvalidOperationException(SR.InvalidOperation_ThreadWrongThreadStart);

            m_start_arg = parameter;
            Start();
        }

        [UnsupportedOSPlatform("browser")]
        internal void UnsafeStart()
        {
            StartInternal(this);
        }

        [UnsupportedOSPlatform("browser")]
        internal void UnsafeStart(object parameter)
        {
            Debug.Assert(m_start is ThreadStart);

            m_start_arg = parameter;
            UnsafeStart();
        }

        // Called from the runtime
        internal void StartCallback()
        {
            ExecutionContext? context = _executionContext;
            _executionContext = null;
            if (context != null && !context.IsDefault)
            {
                ExecutionContext.RunInternal(context, s_threadStartContextCallback, this);
            }
            else
            {
                StartCallbackWorker();
            }
        }

        private static readonly ContextCallback s_threadStartContextCallback = new ContextCallback(StartCallback_Context);

        private static void StartCallback_Context(object? state)
        {
            Debug.Assert(state is Thread);
            ((Thread)state).StartCallbackWorker();
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)] // otherwise an unnecessary long-lived stack frame in many threads
        private void StartCallbackWorker()
        {
            if (culture != null)
            {
                CultureInfo.CurrentCulture = culture;
                culture = null;
            }

            if (ui_culture != null)
            {
                CultureInfo.CurrentUICulture = ui_culture;
                ui_culture = null;
            }

            if (m_start is ThreadStart del)
            {
                m_start = null;
                del();
            }
            else
            {
                Debug.Assert(m_start is ParameterizedThreadStart);
                var pdel = (ParameterizedThreadStart)m_start!;
                object? arg = m_start_arg;
                m_start = null;
                m_start_arg = null;
                pdel(arg);
            }
        }

        // Called from the runtime
        internal static void ThrowThreadStartException(Exception ex) => throw new ThreadStartException(ex);

        [DynamicDependency(nameof(StartCallback))]
        [DynamicDependency(nameof(ThrowThreadStartException))]
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void StartInternal(Thread runtime_thread);
#endif

        partial void ThreadNameChanged(string? value)
        {
            // TODO: Should only raise the events
            SetName(this, value);
        }

        public static bool Yield()
        {
            return YieldInternal();
        }

        private static bool SetApartmentStateUnchecked(ApartmentState state, bool throwOnError)
        {
             if (state != ApartmentState.Unknown)
             {
                if (throwOnError)
                {
                    throw new PlatformNotSupportedException(SR.PlatformNotSupported_ComInterop);
                }

                return false;
             }

             return true;
        }

        private ThreadState ValidateThreadState()
        {
            ThreadState state = GetState(this);
            if ((state & ThreadState.Stopped) != 0)
                throw new ThreadStateException("Thread is dead; state can not be accessed.");
            return state;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern ulong GetCurrentOSThreadId();

        [MemberNotNull("self")]
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void InitInternal(Thread thread);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void InitializeCurrentThread_icall([NotNull] ref Thread? thread);

        [MethodImpl(MethodImplOptions.NoInlining)]
        private static Thread InitializeCurrentThread()
        {
            InitializeCurrentThread_icall(ref t_currentThread);
            return t_currentThread;
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private extern void FreeInternal();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern ThreadState GetState(Thread thread);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void SetState(Thread thread, ThreadState set);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void ClrState(Thread thread, ThreadState clr);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern string GetName(Thread thread);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern unsafe void SetName_icall(Thread thread, char* name, int nameLength);

        private static unsafe void SetName(Thread thread, string? name)
        {
            fixed (char* fixed_name = name)
                SetName_icall(thread, fixed_name, name?.Length ?? 0);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern bool YieldInternal();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void SleepInternal(int millisecondsTimeout, bool allowInterruption);

        [Intrinsic]
        private static void SpinWait_nop()
        {
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern bool JoinInternal(Thread thread, int millisecondsTimeout);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void InterruptInternal(Thread thread);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void SetPriority(Thread thread, int priority);
    }
}
