/*
 * threads.h: Thread support internal calls
 *
 * Author:
 *	Dick Porter (dick@ximian.com)
 *	Patrik Torstensson (patrik.torstensson@labs2.com)
 *
 * (C) 2001 Ximian, Inc
 */

#ifndef _MONO_METADATA_THREADS_H_
#define _MONO_METADATA_THREADS_H_

#include <mono/metadata/object.h>
#include <mono/metadata/appdomain.h>

G_BEGIN_DECLS

typedef void (*MonoThreadCleanupFunc) (MonoThread* thread);

extern int  mono_thread_get_abort_signal (void);

extern void mono_thread_init (MonoThreadStartCB start_cb,
			      MonoThreadAttachCB attach_cb);
extern void mono_thread_manage(void);
extern void mono_thread_abort_all_other_threads (void);
extern void mono_thread_suspend_all_other_threads (void);

extern void mono_thread_push_appdomain_ref (MonoDomain *domain);
extern void mono_thread_pop_appdomain_ref (void);
extern gboolean mono_thread_has_appdomain_ref (MonoThread *thread, MonoDomain *domain);

extern MonoException * mono_thread_get_pending_exception (void);

extern gboolean mono_threads_abort_appdomain_threads (MonoDomain *domain, int timeout);
extern void mono_threads_clear_cached_culture (MonoDomain *domain);

extern MonoThread *mono_thread_current (void);

extern void        mono_thread_set_main (MonoThread *thread);
extern MonoThread *mono_thread_get_main (void);

extern void mono_thread_stop (MonoThread *thread);

typedef struct {
	gpointer (* thread_start_compile_func) (MonoMethod *delegate);
	void (* thread_created) (gsize tid, gpointer stack_start, gpointer func);
	void (* start_resume) (gsize tid);
	void (* end_resume) (gsize tid);
} MonoThreadCallbacks;

extern void mono_install_thread_callbacks (MonoThreadCallbacks *callbacks);

extern void mono_thread_new_init (gsize tid, gpointer stack_start,
				  gpointer func);
extern void mono_thread_create (MonoDomain *domain, gpointer func, gpointer arg);
extern MonoThread *mono_thread_attach (MonoDomain *domain);
extern void mono_thread_detach (MonoThread *thread);
extern void mono_thread_exit (void);

void     mono_threads_install_cleanup   (MonoThreadCleanupFunc func);

extern void mono_threads_set_default_stacksize (guint32 stacksize);
extern guint32 mono_threads_get_default_stacksize (void);
guint32  mono_alloc_special_static_data (guint32 static_type, guint32 size, guint32 align);
gpointer mono_get_special_static_data   (guint32 offset);

void mono_gc_stop_world (void);

void mono_gc_start_world (void);

void mono_threads_request_thread_dump (void);

extern MonoException* mono_thread_request_interruption (gboolean running_managed);
extern gboolean mono_thread_interruption_requested (void);
extern void mono_thread_interruption_checkpoint (void);
extern void mono_thread_force_interruption_checkpoint (void);
extern gint32* mono_thread_interruption_request_flag (void);
extern void mono_debugger_create_all_threads (void);

G_END_DECLS

#endif /* _MONO_METADATA_THREADS_H_ */
