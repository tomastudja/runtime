/*
 * mono-threads-posix.c: Low-level threading, posix version
 *
 * Author:
 *	Rodrigo Kumpera (kumpera@gmail.com)
 *
 * (C) 2011 Novell, Inc
 */

#include <config.h>

#include <mono/utils/mono-compiler.h>
#include <mono/utils/mono-semaphore.h>
#include <mono/utils/mono-threads.h>
#include <mono/utils/mono-tls.h>
#include <mono/utils/gc_wrapper.h>
#include <mono/metadata/threads-types.h>

#include <errno.h>

#if defined(PLATFORM_ANDROID)
extern int tkill (pid_t tid, int signal);
#endif

#if defined(_POSIX_VERSION) || defined(__native_client__)
#include <signal.h>

#if defined(__native_client__)
void nacl_shutdown_gc_thread(void);
#endif

typedef struct {
	void *(*start_routine)(void*);
	void *arg;
	int flags;
	MonoSemType registered;
	HANDLE handle;
} StartInfo;

static void*
inner_start_thread (void *arg)
{
	StartInfo *start_info = arg;
	void *t_arg = start_info->arg;
	int res;
	void *(*start_func)(void*) = start_info->start_routine;
	guint32 flags = start_info->flags;
	void *result;
	HANDLE handle;
	MonoThreadInfo *info;

	/* Register the thread with the io-layer */
	handle = wapi_create_thread_handle ();
	if (!handle) {
		res = MONO_SEM_POST (&(start_info->registered));
		g_assert (!res);
		return NULL;
	}
	start_info->handle = handle;

	if (!(flags & CREATE_NO_DETACH)) {
		res = mono_gc_pthread_detach (pthread_self ());
		g_assert (!res);
	}

	info = mono_thread_info_attach (&result);
	info->runtime_thread = TRUE;

	/* start_info is not valid after this */
	res = MONO_SEM_POST (&(start_info->registered));
	g_assert (!res);
	start_info = NULL;

	if (flags & CREATE_SUSPENDED)
		wapi_thread_suspend (handle);

	/* Run the actual main function of the thread */
	result = start_func (t_arg);

	g_assert (!mono_domain_get ());
	mono_thread_info_dettach ();

#if defined(__native_client__)
	nacl_shutdown_gc_thread();
#endif

	wapi_thread_set_exit_code (GPOINTER_TO_UINT (result), handle);

	// FIXME: Why is this needed ?
	mono_gc_pthread_exit (NULL);

	g_assert_not_reached ();
	return result;
}

HANDLE
mono_threads_core_create_thread (LPTHREAD_START_ROUTINE start_routine, gpointer arg, guint32 stack_size, guint32 creation_flags, MonoNativeThreadId *out_tid)
{
	pthread_attr_t attr;
	int res;
	pthread_t thread;
	StartInfo start_info;

	res = pthread_attr_init (&attr);
	g_assert (!res);

	if (stack_size == 0) {
#if HAVE_VALGRIND_MEMCHECK_H
		if (RUNNING_ON_VALGRIND)
			stack_size = 1 << 20;
		else
			stack_size = (SIZEOF_VOID_P / 4) * 1024 * 1024;
#else
		stack_size = (SIZEOF_VOID_P / 4) * 1024 * 1024;
#endif
	}

#ifdef PTHREAD_STACK_MIN
	if (stack_size < PTHREAD_STACK_MIN)
		stack_size = PTHREAD_STACK_MIN;
#endif

#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
	res = pthread_attr_setstacksize (&attr, stack_size);
	g_assert (!res);
#endif

	memset (&start_info, 0, sizeof (StartInfo));
	start_info.start_routine = (gpointer)start_routine;
	start_info.arg = arg;
	start_info.flags = creation_flags;
	MONO_SEM_INIT (&(start_info.registered), 0);

	/* Actually start the thread */
	res = mono_threads_get_callbacks ()->mono_gc_pthread_create (&thread, &attr, inner_start_thread, &start_info);
	if (res) {
		// FIXME:
		g_assert_not_reached ();
	}

	/* Wait until the thread register itself in various places */
	while (MONO_SEM_WAIT (&(start_info.registered)) != 0) {
		/*if (EINTR != errno) ABORT("sem_wait failed"); */
	}
	MONO_SEM_DESTROY (&(start_info.registered));

	if (out_tid)
		*out_tid = thread;

	return start_info.handle;
}

#if !defined (__MACH__)

#if !defined(__native_client__)
static void
suspend_signal_handler (int _dummy, siginfo_t *info, void *context)
{
	MonoThreadInfo *current = mono_thread_info_current ();
	gboolean ret;
	
	if (current->syscall_break_signal) {
		current->syscall_break_signal = FALSE;
		return;
	}

	ret = mono_threads_get_runtime_callbacks ()->thread_state_init_from_sigctx (&current->suspend_state, context);

	/* thread_state_init_from_sigctx return FALSE if the current thread is detaching and suspend can't continue. */
	current->suspend_can_continue = ret;

	MONO_SEM_POST (&current->begin_suspend_semaphore);

	/* This thread is doomed, all we can do is give up and let the suspender recover. */
	if (!ret)
		return;

	while (MONO_SEM_WAIT (&current->resume_semaphore) != 0) {
		/*if (EINTR != errno) ABORT("sem_wait failed"); */
	}

	if (current->async_target) {
#if MONO_ARCH_HAS_MONO_CONTEXT
		MonoContext tmp = current->suspend_state.ctx;
		mono_threads_get_runtime_callbacks ()->setup_async_callback (&tmp, current->async_target, current->user_data);
		current->async_target = current->user_data = NULL;
		mono_monoctx_to_sigctx (&tmp, context);
#else
		g_error ("The new interruption machinery requires a working mono-context");
#endif
	}

	MONO_SEM_POST (&current->finish_resume_semaphore);
}
#endif

static void
mono_posix_add_signal_handler (int signo, gpointer handler)
{
#if !defined(__native_client__)
	/*FIXME, move the code from mini to utils and do the right thing!*/
	struct sigaction sa;
	struct sigaction previous_sa;
	int ret;

	sa.sa_sigaction = handler;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	ret = sigaction (signo, &sa, &previous_sa);

	g_assert (ret != -1);
#endif
}

void
mono_threads_init_platform (void)
{
#if !defined(__native_client__)
	/*
	FIXME we should use all macros from mini to make this more portable
	FIXME it would be very sweet if sgen could end up using this too.
	*/
	if (mono_thread_info_new_interrupt_enabled ())
		mono_posix_add_signal_handler (mono_thread_get_abort_signal (), suspend_signal_handler);
#endif
}

/*nothing to be done here since suspend always abort syscalls due using signals*/
void
mono_threads_core_interrupt (MonoThreadInfo *info)
{
}

int
mono_threads_pthread_kill (MonoThreadInfo *info, int signum)
{
#if defined (PLATFORM_ANDROID)
	int result, old_errno = errno;
	result = tkill (info->native_handle, signum);
	if (result < 0) {
		result = errno;
		errno = old_errno;
	}
	return result;
#elif defined(__native_client__)
	/* Workaround pthread_kill abort() in NaCl glibc. */
	return 0;
#else
	return pthread_kill (mono_thread_info_get_tid (info), signum);
#endif

}

void
mono_threads_core_abort_syscall (MonoThreadInfo *info)
{
	/*
	We signal a thread to break it from the urrent syscall.
	This signal should not be interpreted as a suspend request.
	*/
	info->syscall_break_signal = TRUE;
	mono_threads_pthread_kill (info, mono_thread_get_abort_signal ());
}

gboolean
mono_threads_core_needs_abort_syscall (void)
{
	return TRUE;
}

gboolean
mono_threads_core_suspend (MonoThreadInfo *info)
{
	/*FIXME, check return value*/
	mono_threads_pthread_kill (info, mono_thread_get_abort_signal ());
	while (MONO_SEM_WAIT (&info->begin_suspend_semaphore) != 0) {
		/* g_assert (errno == EINTR); */
	}
	return info->suspend_can_continue;
}

gboolean
mono_threads_core_resume (MonoThreadInfo *info)
{
	MONO_SEM_POST (&info->resume_semaphore);
	while (MONO_SEM_WAIT (&info->finish_resume_semaphore) != 0) {
		/* g_assert (errno == EINTR); */
	}

	return TRUE;
}

void
mono_threads_platform_register (MonoThreadInfo *info)
{
	MONO_SEM_INIT (&info->begin_suspend_semaphore, 0);

#if defined (PLATFORM_ANDROID)
	info->native_handle = (gpointer) gettid ();
#endif
}

void
mono_threads_platform_free (MonoThreadInfo *info)
{
	MONO_SEM_DESTROY (&info->begin_suspend_semaphore);
}

MonoNativeThreadId
mono_native_thread_id_get (void)
{
	return pthread_self ();
}

gboolean
mono_native_thread_id_equals (MonoNativeThreadId id1, MonoNativeThreadId id2)
{
	return pthread_equal (id1, id2);
}

/*
 * mono_native_thread_create:
 *
 *   Low level thread creation function without any GC wrappers.
 */
gboolean
mono_native_thread_create (MonoNativeThreadId *tid, gpointer func, gpointer arg)
{
	return pthread_create (tid, NULL, func, arg) == 0;
}

#endif /*!defined (__MACH__)*/

#endif
