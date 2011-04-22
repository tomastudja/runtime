/*
 * mono-threads.c: Low-level threading
 *
 * Author:
 *	Rodrigo Kumpera (kumpera@gmail.com)
 *
 * (C) 2011 Novell, Inc
 */

#include <mono/utils/mono-compiler.h>
#include <mono/utils/mono-semaphore.h>
#include <mono/utils/mono-threads.h>
#include <mono/utils/mono-tls.h>
#include <mono/utils/hazard-pointer.h>
#include <mono/metadata/gc-internal.h>
#include <mono/metadata/appdomain.h>

#include <errno.h>

#define THREADS_DEBUG(...)
//#define THREADS_DEBUG(...) g_message(__VA_ARGS__)

/*
Mutex that makes sure only a single thread can be suspending others.
Suspend is a very racy operation since it requires restarting until
the target thread is not on an unsafe region.

We could implement this using critical regions, but would be much much
harder for an operation that is hardly performance critical.

The GC has to acquire this lock before starting a STW to make sure
a runtime suspend won't make it wronly see a thread in a safepoint
+when it is in fact not.
*/
static CRITICAL_SECTION global_suspend_lock;


static int thread_info_size;
static MonoThreadInfoCallbacks threads_callbacks;
static MonoThreadInfoRuntimeCallbacks runtime_callbacks;
static MonoNativeTlsKey thread_info_key;
static MonoLinkedListSet thread_list;

static inline void
mono_hazard_pointer_clear_all (MonoThreadHazardPointers *hp, int retain)
{
	if (retain != 0)
		mono_hazard_pointer_clear (hp, 0);
	if (retain != 1)
		mono_hazard_pointer_clear (hp, 1);
	if (retain != 2)
		mono_hazard_pointer_clear (hp, 2);
}

/*
If return non null Hazard Pointer 1 holds the return value.
*/
MonoThreadInfo*
mono_thread_info_lookup (MonoNativeThreadId id)
{
		MonoThreadHazardPointers *hp = mono_hazard_pointer_get ();

	if (!mono_lls_find (&thread_list, hp, (uintptr_t)id)) {
		mono_hazard_pointer_clear_all (hp, -1);
		return NULL;
	} 

	mono_hazard_pointer_clear_all (hp, 1);
	return mono_hazard_pointer_get_val (hp, 1);
}

static gboolean
mono_thread_info_insert (MonoThreadInfo *info)
{
	MonoThreadHazardPointers *hp = mono_hazard_pointer_get ();

	if (!mono_lls_insert (&thread_list, hp, (MonoLinkedListSetNode*)info)) {
		mono_hazard_pointer_clear_all (hp, -1);
		return FALSE;
	} 

	mono_hazard_pointer_clear_all (hp, -1);
	return TRUE;
}

static gboolean
mono_thread_info_remove (MonoThreadInfo *info)
{
	/*TLS is gone by now, so we can't rely on it to retrieve hp*/
	MonoThreadHazardPointers *hp = mono_hazard_pointer_get_by_id (info->small_id);
	gboolean res;

	THREADS_DEBUG ("removing info %p\n", info);
	res = mono_lls_remove (&thread_list, hp, (MonoLinkedListSetNode*)info);
	mono_hazard_pointer_clear_all (hp, -1);
	return res;
}

static void
free_thread_info (gpointer mem)
{
	MonoThreadInfo *info = mem;

	DeleteCriticalSection (&info->suspend_lock);
	mono_threads_platform_free (info);

	g_free (info);
}

static void*
register_thread (MonoThreadInfo *info, gpointer baseptr)
{
	gboolean result;
	mono_thread_info_set_tid (info, mono_native_thread_id_get ());
	info->small_id = mono_thread_small_id_alloc ();

	InitializeCriticalSection (&info->suspend_lock);

	THREADS_DEBUG ("registering info %p tid %p small id %x\n", info, mono_thread_info_get_tid (info), info->small_id);

	if (threads_callbacks.thread_register) {
		if (threads_callbacks.thread_register (info, baseptr) == NULL) {
			g_warning ("thread registation failed\n");
			g_free (info);
			return NULL;
		}
	}

	mono_threads_platform_register (info);
	mono_native_tls_set_value (thread_info_key, info);

	/*If this fail it means a given thread has been registered twice, which doesn't make sense. */
	result = mono_thread_info_insert (info);
	g_assert (result);
	return info;
}

static void
unregister_thread (void *arg)
{
	gboolean result;
	MonoThreadInfo *info = arg;
	int small_id = info->small_id;
	g_assert (info);

	THREADS_DEBUG ("unregistering info %p\n", info);

	if (threads_callbacks.thread_unregister)
		threads_callbacks.thread_unregister (info);

	result = mono_thread_info_remove (info);
	g_assert (result);

	mono_thread_small_id_free (small_id);
}

MonoThreadInfo*
mono_thread_info_current (void)
{
	return mono_native_tls_get_value (thread_info_key);
}

MonoLinkedListSet*
mono_thread_info_list_head (void)
{
	return &thread_list;
}

MonoThreadInfo*
mono_thread_info_attach (void *baseptr)
{
	MonoThreadInfo *info = mono_native_tls_get_value (thread_info_key);
	if (!info) {
		info = g_malloc0 (thread_info_size);
		THREADS_DEBUG ("attaching %p\n", info);
		if (!register_thread (info, baseptr))
			return NULL;
	} else if (threads_callbacks.thread_attach) {
		threads_callbacks.thread_attach (info);
	}
	return info;
}

void
mono_thread_info_dettach (void)
{
	MonoThreadInfo *info = mono_native_tls_get_value (thread_info_key);
	if (info) {
		THREADS_DEBUG ("detaching %p\n", info);
		unregister_thread (info);
	}
}

void
mono_threads_init (MonoThreadInfoCallbacks *callbacks, size_t info_size)
{
	gboolean res;
	threads_callbacks = *callbacks;
	thread_info_size = info_size;
#ifdef HOST_WIN32
	res = mono_native_tls_alloc (thread_info_key, NULL);
#else
	res = mono_native_tls_alloc (thread_info_key, unregister_thread);
#endif
	InitializeCriticalSection (&global_suspend_lock);

	mono_lls_init (&thread_list, free_thread_info);
	mono_thread_smr_init ();
	mono_threads_init_platform ();

	g_assert (res);
	g_assert (sizeof (MonoNativeThreadId) <= sizeof (uintptr_t));
}

void
mono_threads_runtime_init (MonoThreadInfoRuntimeCallbacks *callbacks)
{
	runtime_callbacks = *callbacks;
}

MonoThreadInfoRuntimeCallbacks *
mono_threads_get_runtime_callbacks (void)
{
	return &runtime_callbacks;
}

/*
The return value is only valid until a matching mono_thread_info_resume is called
*/
static MonoThreadInfo*
mono_thread_info_suspend_sync (MonoNativeThreadId tid, gboolean interrupt_kernel)
{
	MonoThreadHazardPointers *hp = mono_hazard_pointer_get ();	
	MonoThreadInfo *info = mono_thread_info_lookup (tid); /*info on HP1*/
	if (!info)
		return NULL;

	EnterCriticalSection (&info->suspend_lock);

	/*thread is on the process of detaching*/
	if (info->thread_state > STATE_RUNNING) {
		mono_hazard_pointer_clear (hp, 1);
		return NULL;
	}

	THREADS_DEBUG ("suspend %x IN COUNT %d\n", tid, info->suspend_count);

	if (info->suspend_count) {
		++info->suspend_count;
		mono_hazard_pointer_clear (hp, 1);
		LeaveCriticalSection (&info->suspend_lock);
		return info;
	}

	if (!mono_threads_core_suspend (info)) {
		LeaveCriticalSection (&info->suspend_lock);
		mono_hazard_pointer_clear (hp, 1);
		return NULL;
	}

	if (interrupt_kernel) 
		mono_threads_core_interrupt (info);

	++info->suspend_count;
	LeaveCriticalSection (&info->suspend_lock);
	mono_hazard_pointer_clear (hp, 1);

	return info;
}

void
mono_thread_info_self_suspend ()
{
	MonoThreadInfo *info = mono_thread_info_current ();
	if (!info)
		return;

	EnterCriticalSection (&info->suspend_lock);

	THREADS_DEBUG ("self suspend IN COUNT %d\n", info->suspend_count);

	g_assert (info->suspend_count == 0);
	++info->suspend_count;

	/*
	The internal API contract with this function is a bit out of the ordinary.
	mono_threads_core_self_suspend executes with suspend_lock taken and must
	release it after capturing the current context.
	*/
	mono_threads_core_self_suspend (info);
}

gboolean
mono_thread_info_resume (MonoNativeThreadId tid)
{
	gboolean result = TRUE;
	MonoThreadHazardPointers *hp = mono_hazard_pointer_get ();	
	MonoThreadInfo *info = mono_thread_info_lookup (tid); /*info on HP1*/
	if (!info)
		return FALSE;

	EnterCriticalSection (&info->suspend_lock);

	THREADS_DEBUG ("resume %x IN COUNT %d\n",tid, info->suspend_count);

	if (info->suspend_count <= 0) {
		LeaveCriticalSection (&info->suspend_lock);
		mono_hazard_pointer_clear (hp, 1);
		return FALSE;
	}

	/*
	 * The theory here is that if we manage to suspend the thread it means it did not
	 * start cleanup since it take the same lock. 
	*/
	g_assert (mono_thread_info_get_tid (info));

	if (--info->suspend_count == 0)
		result = mono_threads_core_resume (info);

	LeaveCriticalSection (&info->suspend_lock);
	mono_hazard_pointer_clear (hp, 1);

	return result;
}

/*
FIXME fix cardtable WB to be out of line and check with the runtime if the target is not the
WB trampoline. Another option is to encode wb ranges in MonoJitInfo, but that is somewhat hard.
*/
static gboolean
is_thread_in_critical_region (MonoThreadInfo *info)
{
	MonoMethod *method;
	MonoJitInfo *ji = mono_jit_info_table_find (
		info->suspend_state.unwind_data [MONO_UNWIND_DATA_DOMAIN],
		MONO_CONTEXT_GET_IP (&info->suspend_state.ctx));

	if (!ji)
		return FALSE;

	method = ji->method;

	return mono_gc_is_critical_method (method);
}

/*
WARNING:
If we are trying to suspend a target that is on a critical region
and running a syscall we risk looping forever if @interrupt_kernel is FALSE.
So, be VERY carefull in calling this with @interrupt_kernel == FALSE.
*/
MonoThreadInfo*
mono_thread_info_safe_suspend_sync (MonoNativeThreadId id, gboolean interrupt_kernel)
{
	MonoThreadInfo *info = NULL;
	int sleep_duration = 0;

	/*FIXME: unify this with self-suspend*/
	g_assert (id != mono_native_thread_id_get ());

	mono_thread_info_suspend_lock ();

	for (;;) {
		if (!(info = mono_thread_info_suspend_sync (id, interrupt_kernel))) {
			g_warning ("failed to suspend thread %p, hopefully it is dead", (gpointer)id);
			mono_thread_info_suspend_unlock ();
			return NULL;
		}
		/*WARNING: We now are in interrupt context until we resume the thread. */
		if (!is_thread_in_critical_region (info))
			break;

		if (!mono_thread_info_resume (id)) {
			g_warning ("failed to result thread %p, hopefully it is dead", (gpointer)id);
			mono_thread_info_suspend_unlock ();
			return NULL;
		}
		THREADS_DEBUG ("restarted thread %p\n", (gpointer)id);

		if (!sleep_duration) {
#ifdef HOST_WIN32
			SwitchToThread ();
#else
			sched_yield ();
#endif
		}
		else {
			g_usleep (sleep_duration);
		}
		sleep_duration += 10;
	}

	mono_thread_info_suspend_unlock ();
	return info;
}

/*
The suspend lock is held during any suspend in progress.
A GC that has safepoints must take this lock as part of its
STW to make sure no unsafe pending suspend is in progress.   
*/
void
mono_thread_info_suspend_lock (void)
{
	EnterCriticalSection (&global_suspend_lock);
}

void
mono_thread_info_suspend_unlock (void)
{
	LeaveCriticalSection (&global_suspend_lock);
}

/*
Disabled by default for now.
To enable this we need mini to implement the callbacks by MonoThreadInfoRuntimeCallbacks
which means mono-context and setup_async_callback, and we need a mono-threads backend.
*/
gboolean
mono_thread_info_new_interrupt_enabled (void)
{
	return FALSE;
}
