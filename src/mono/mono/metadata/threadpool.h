#ifndef _MONO_THREADPOOL_H_
#define _MONO_THREADPOOL_H_

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

extern int mono_max_worker_threads;
extern int mono_worker_threads;

MonoAsyncResult *
mono_thread_pool_add     (MonoObject *target, MonoMethodMessage *msg, 
			  MonoDelegate *async_callback, MonoObject *state);

MonoObject *
mono_thread_pool_finish (MonoAsyncResult *ares, MonoArray **out_args, 
			 MonoObject **exc);

#endif
