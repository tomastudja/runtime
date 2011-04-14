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
#include <mono/metadata/threads-types.h>

#include <errno.h>

#if defined(_POSIX_VERSION)

typedef struct {
	void *(*start_routine)(void*);
	void *arg;
	int flags;
	MonoSemType registered;
} ThreadStartInfo;


static void*
inner_start_thread (void *arg)
{
	ThreadStartInfo *start_info = arg;
	void *t_arg = start_info->arg;
	int post_result;
	void *(*start_func)(void*) = start_info->start_routine;
	void *result;

	mono_thread_info_attach (&result);

	post_result = MONO_SEM_POST (&(start_info->registered));
	g_assert (!post_result);

	result = start_func (t_arg);
	g_assert (!mono_domain_get ());


	return result;
}

int
mono_threads_pthread_create (pthread_t *new_thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
	ThreadStartInfo *start_info;
	int result;

	start_info = g_malloc0 (sizeof (ThreadStartInfo));
	if (!start_info)
		return ENOMEM;
	MONO_SEM_INIT (&(start_info->registered), 0);
	start_info->arg = arg;
	start_info->start_routine = start_routine;

	result = pthread_create (new_thread, attr, inner_start_thread, start_info);
	if (result == 0) {
		while (MONO_SEM_WAIT (&(start_info->registered)) != 0) {
			/*if (EINTR != errno) ABORT("sem_wait failed"); */
		}
	}
	MONO_SEM_DESTROY (&(start_info->registered));
	g_free (start_info);
	return result;
}

#if !defined (__MACH__)

static void
suspend_signal_handler (int _dummy, siginfo_t *info, void *context)
{
	MonoThreadInfo *current = mono_thread_info_current ();
	gboolean ret = mono_threads_get_runtime_callbacks ()->thread_state_init_from_sigctx (&current->suspend_state, context);

	g_assert (ret);

	if (current->self_suspend)
		LeaveCriticalSection (&current->suspend_lock);
	else
		MONO_SEM_POST (&current->suspend_semaphore);
		
	while (MONO_SEM_WAIT (&current->resume_semaphore) != 0) {
		/*if (EINTR != errno) ABORT("sem_wait failed"); */
	}

	MONO_SEM_POST (&current->finish_resume_semaphore);
}

static void
mono_posix_add_signal_handler (int signo, gpointer handler)
{
	/*FIXME, move the code from mini to utils and do the right thing!*/
	struct sigaction sa;
	struct sigaction previous_sa;
	int ret;

	sa.sa_sigaction = handler;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	ret = sigaction (signo, &sa, &previous_sa);

	g_assert (ret != -1);
}

void
mono_threads_init_platform (void)
{
	/*
	FIXME we should use all macros from mini to make this more portable
	FIXME it would be very sweet if sgen could end up using this too.
	*/
	if (mono_thread_info_new_interrupt_enabled ())
		mono_posix_add_signal_handler (mono_thread_get_abort_signal (), suspend_signal_handler);
}

/*nothing to be done here since suspend always abort syscalls due using signals*/
void
mono_threads_core_interrupt (MonoThreadInfo *info)
{
}

/*
We self suspend using signals since thread_state_init_from_sigctx only supports
a null context on a few targets.
*/
void
mono_threads_core_self_suspend (MonoThreadInfo *info)
{
	/*FIXME, check return value*/
	info->self_suspend = TRUE;
	pthread_kill (mono_thread_info_get_tid (info), mono_thread_get_abort_signal ());
}

gboolean
mono_threads_core_suspend (MonoThreadInfo *info)
{
	/*FIXME, check return value*/
	info->self_suspend = FALSE;
	pthread_kill (mono_thread_info_get_tid (info), mono_thread_get_abort_signal ());
	while (MONO_SEM_WAIT (&info->suspend_semaphore) != 0) {
		/* g_assert (errno == EINTR); */
	}
	return TRUE;
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
	MONO_SEM_INIT (&info->suspend_semaphore, 0);
	MONO_SEM_INIT (&info->resume_semaphore, 0);
	MONO_SEM_INIT (&info->finish_resume_semaphore, 0);
}

void
mono_threads_platform_free (MonoThreadInfo *info)
{
	MONO_SEM_DESTROY (&info->suspend_semaphore);
	MONO_SEM_DESTROY (&info->resume_semaphore);
	MONO_SEM_DESTROY (&info->finish_resume_semaphore);
}

#endif /*!defined (__MACH__)*/

#endif
