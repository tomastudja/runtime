/*
 * threadpool.c: global thread pool
 *
 * Authors:
 *   Dietmar Maurer (dietmar@ximian.com)
 *   Gonzalo Paniagua Javier (gonzalo@ximian.com)
 *
 * (C) 2001-2003 Ximian, Inc.
 * (c) 2004 Novell, Inc. (http://www.novell.com)
 */

#include <config.h>
#include <glib.h>

#ifdef PLATFORM_WIN32
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#endif

#include <mono/metadata/appdomain.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/file-io.h>
#include <mono/metadata/monitor.h>
#include <mono/metadata/marshal.h>
#include <mono/io-layer/io-layer.h>
#include <mono/os/gc_wrapper.h>

#include "threadpool.h"

/* maximum number of worker threads */
int mono_max_worker_threads = 25; /* fixme: should be 25 per available CPU */
/* current number of worker threads */
static int mono_worker_threads = 0;

/* current number of busy threads */
int busy_worker_threads = 0;

/* we use this to store a reference to the AsyncResult to avoid GC */
static MonoGHashTable *ares_htable = NULL;

/* we append a job */
static HANDLE job_added;

typedef struct {
	MonoMethodMessage *msg;
	HANDLE             wait_event;
	MonoMethod        *cb_method;
	MonoDelegate      *cb_target;
	MonoObject        *state;
	MonoObject        *res;
	MonoArray         *out_args;
} ASyncCall;

static void async_invoke_thread (gpointer data);
static void append_job (MonoAsyncResult *ar);

static GList *async_call_queue = NULL;

static void
mono_async_invoke (MonoAsyncResult *ares)
{
	ASyncCall *ac = (ASyncCall *)ares->data;

	ac->msg->exc = NULL;
	ac->res = mono_message_invoke (ares->async_delegate, ac->msg, 
				       &ac->msg->exc, &ac->out_args);

	ares->completed = 1;

	/* call async callback if cb_method != null*/
	if (ac->cb_method) {
		MonoObject *exc = NULL;
		void *pa = &ares;
		mono_runtime_invoke (ac->cb_method, ac->cb_target, pa, &exc);
		if (!ac->msg->exc)
			ac->msg->exc = exc;
	}

	/* notify listeners */
	mono_monitor_try_enter ((MonoObject *) ares, INFINITE);
	if (ares->handle != NULL) {
		ac->wait_event = ((MonoWaitHandle *) ares->handle)->handle;
		SetEvent (ac->wait_event);
	}
	mono_monitor_exit ((MonoObject *) ares);

	mono_g_hash_table_remove (ares_htable, ares);
}

MonoAsyncResult *
mono_thread_pool_add (MonoObject *target, MonoMethodMessage *msg, MonoDelegate *async_callback,
		      MonoObject *state)
{
	MonoDomain *domain = mono_domain_get ();
	MonoAsyncResult *ares;
	ASyncCall *ac;
	int busy, worker;

#ifdef HAVE_BOEHM_GC
	ac = GC_MALLOC (sizeof (ASyncCall));
#else
	/* We'll leak the event if creaated... */
	ac = g_new0 (ASyncCall, 1);
#endif
	ac->wait_event = NULL;
	ac->msg = msg;
	ac->state = state;

	if (async_callback) {
		ac->cb_method = mono_get_delegate_invoke (((MonoObject *)async_callback)->vtable->klass);
		ac->cb_target = async_callback;
	}

	ares = mono_async_result_new (domain, NULL, ac->state, ac);
	ares->async_delegate = target;

	if (!ares_htable) {
		ares_htable = mono_g_hash_table_new (NULL, NULL);
		job_added = CreateSemaphore (NULL, 0, 0x7fffffff, NULL);
	}

	mono_g_hash_table_insert (ares_htable, ares, ares);

	busy = (int) InterlockedCompareExchange (&busy_worker_threads, 0, -1);
	worker = (int) InterlockedCompareExchange (&mono_worker_threads, 0, -1); 
	if (worker <= ++busy &&
	    worker < mono_max_worker_threads) {
		InterlockedIncrement (&mono_worker_threads);
		InterlockedIncrement (&busy_worker_threads);
		mono_thread_create (domain, async_invoke_thread, ares);
	} else {
		append_job (ares);
		ReleaseSemaphore (job_added, 1, NULL);
	}

	return ares;
}

MonoObject *
mono_thread_pool_finish (MonoAsyncResult *ares, MonoArray **out_args, MonoObject **exc)
{
	ASyncCall *ac;

	*exc = NULL;
	*out_args = NULL;

	/* check if already finished */
	mono_monitor_try_enter ((MonoObject *) ares, INFINITE);
	if (ares->endinvoke_called) {
		*exc = (MonoObject *)mono_exception_from_name (mono_defaults.corlib, "System", 
					      "InvalidOperationException");
		mono_monitor_exit ((MonoObject *) ares);
		return NULL;
	}

	ares->endinvoke_called = 1;
	ac = (ASyncCall *)ares->data;

	g_assert (ac != NULL);

	/* wait until we are really finished */
	if (!ares->completed) {
		if (ares->handle == NULL) {
			ac->wait_event = CreateEvent (NULL, TRUE, FALSE, NULL);
			ares->handle = (MonoObject *) mono_wait_handle_new (mono_object_domain (ares), ac->wait_event);
		}
		mono_monitor_exit ((MonoObject *) ares);
		WaitForSingleObject (ac->wait_event, INFINITE);
	} else {
		mono_monitor_exit ((MonoObject *) ares);
	}

	*exc = ac->msg->exc;
	*out_args = ac->out_args;

	return ac->res;
}

static void
append_job (MonoAsyncResult *ar)
{
	EnterCriticalSection (&mono_delegate_section);
	async_call_queue = g_list_append (async_call_queue, ar); 
	LeaveCriticalSection (&mono_delegate_section);
}

static MonoAsyncResult *
dequeue_job (void)
{
	MonoAsyncResult *ar = NULL;
	GList *tmp = NULL;

	EnterCriticalSection (&mono_delegate_section);
	if (async_call_queue) {
		ar = (MonoAsyncResult *)async_call_queue->data;
		tmp = async_call_queue;
		async_call_queue = g_list_remove_link (tmp, tmp); 
	}
	LeaveCriticalSection (&mono_delegate_section);
	if (tmp)
		g_list_free_1 (tmp);

	return ar;
}

static void
async_invoke_thread (gpointer data)
{
	MonoDomain *domain;
	MonoThread *thread;
 
	thread = mono_thread_current ();
	thread->threadpool_thread = TRUE;
	thread->state |= ThreadState_Background;
	for (;;) {
		MonoAsyncResult *ar;

		ar = (MonoAsyncResult *) data;
		if (ar) {
			/* worker threads invokes methods in different domains,
			 * so we need to set the right domain here */
			domain = ((MonoObject *)ar)->vtable->domain;
			if (mono_domain_set (domain, FALSE))
				mono_async_invoke (ar);
			InterlockedDecrement (&busy_worker_threads);
		}

		data = dequeue_job ();
		if (!data && WaitForSingleObject (job_added, 500) != WAIT_TIMEOUT)
			data = dequeue_job ();

		if (!data) {
			InterlockedDecrement (&mono_worker_threads);
			return;
		}
		InterlockedIncrement (&busy_worker_threads);
	}

	g_assert_not_reached ();
}

void
ves_icall_System_Threading_ThreadPool_GetAvailableThreads (gint *workerThreads, gint *completionPortThreads)
{
	gint busy;

	MONO_ARCH_SAVE_REGS;

	busy = (gint) InterlockedCompareExchange (&busy_worker_threads, 0, -1);
	*workerThreads = mono_max_worker_threads - busy;
	*completionPortThreads = 0;
}

void
ves_icall_System_Threading_ThreadPool_GetMaxThreads (gint *workerThreads, gint *completionPortThreads)
{
	MONO_ARCH_SAVE_REGS;

	*workerThreads = mono_max_worker_threads;
	*completionPortThreads = 0;
}

static void
overlapped_callback (guint32 error, guint32 numbytes, WapiOverlapped *overlapped)
{
	MonoFSAsyncResult *ares;
	MonoThread *thread;
	gpointer ftn;
 
	MONO_ARCH_SAVE_REGS;

	ares = (MonoFSAsyncResult *) overlapped->handle1;
	ares->completed = TRUE;
	if (ares->bytes_read != -1)
		ares->bytes_read = numbytes;
	else
		ares->count = numbytes;

	thread = mono_thread_attach (mono_object_domain (ares));
	SetEvent (ares->wait_handle->handle);
	if (ares->async_callback != NULL) {
		gpointer p [1];

		*p = ares;
		mono_runtime_invoke (ares->async_callback->method_info->method, NULL, p, NULL);
	}

	mono_thread_detach (thread);
	CloseHandle (thread->handle);
	g_free (overlapped);
}

MonoBoolean
ves_icall_System_Threading_ThreadPool_BindHandle (gpointer handle)
{
	MONO_ARCH_SAVE_REGS;

#ifdef PLATFORM_WIN32
	return FALSE;
#else
	if (!BindIoCompletionCallback (handle, overlapped_callback, 0)) {
		gint error = GetLastError ();
		MonoException *exc;
		gchar *msg;

		if (error == ERROR_INVALID_PARAMETER) {
			exc = mono_get_exception_argument (NULL, "Invalid parameter.");
		} else {
			msg = g_strdup_printf ("Win32 error %d.", error);
			exc = mono_exception_from_name_msg (mono_defaults.corlib,
							    "System",
							    "ApplicationException", msg);
			g_free (msg);
		}

		mono_raise_exception (exc);
	}

	return TRUE;
#endif
}

