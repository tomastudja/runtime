#ifndef _MONO_THREADPOOL_H_
#define _MONO_THREADPOOL_H_

#include <mono/metadata/object-internals.h>
#include <mono/metadata/reflection.h>

/* No managed code here */
void mono_thread_pool_init (void) MONO_INTERNAL;

MonoAsyncResult *
mono_thread_pool_add     (MonoObject *target, MonoMethodMessage *msg, 
			  MonoDelegate *async_callback, MonoObject *state) MONO_INTERNAL;

MonoObject *
mono_thread_pool_finish (MonoAsyncResult *ares, MonoArray **out_args, 
			 MonoObject **exc) MONO_INTERNAL;

void mono_thread_pool_cleanup (void) MONO_INTERNAL;

void
ves_icall_System_Threading_ThreadPool_GetAvailableThreads (int *workerThreads,
							   int *completionPortThreads) MONO_INTERNAL;

void
ves_icall_System_Threading_ThreadPool_GetMaxThreads (int *workerThreads,
						     int *completionPortThreads) MONO_INTERNAL;

void
ves_icall_System_Threading_ThreadPool_GetMinThreads (gint *workerThreads, 
								gint *completionPortThreads) MONO_INTERNAL;

MonoBoolean
ves_icall_System_Threading_ThreadPool_SetMinThreads (gint workerThreads, 
								gint completionPortThreads) MONO_INTERNAL;

#endif
