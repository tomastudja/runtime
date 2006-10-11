/*
 * threads-types.h: Generic thread typedef support (includes
 * system-specific files)
 *
 * Author:
 *	Dick Porter (dick@ximian.com)
 *
 * (C) 2001 Ximian, Inc
 */

#ifndef _MONO_METADATA_THREADS_TYPES_H_
#define _MONO_METADATA_THREADS_TYPES_H_

#include <glib.h>

#include <mono/io-layer/io-layer.h>
#include "mono/utils/mono-compiler.h"

/* This is a copy of System.Threading.ThreadState */
typedef enum {
	ThreadState_Running = 0x00000000,
	ThreadState_StopRequested = 0x00000001,
	ThreadState_SuspendRequested = 0x00000002,
	ThreadState_Background = 0x00000004,
	ThreadState_Unstarted = 0x00000008,
	ThreadState_Stopped = 0x00000010,
	ThreadState_WaitSleepJoin = 0x00000020,
	ThreadState_Suspended = 0x00000040,
	ThreadState_AbortRequested = 0x00000080,
	ThreadState_Aborted = 0x00000100
} MonoThreadState;

#define SPECIAL_STATIC_NONE 0
#define SPECIAL_STATIC_THREAD 1
#define SPECIAL_STATIC_CONTEXT 2

extern HANDLE ves_icall_System_Threading_Thread_Thread_internal(MonoThread *this_obj, MonoObject *start) MONO_INTERNAL;
extern void ves_icall_System_Threading_Thread_Thread_free_internal(MonoThread *this_obj, HANDLE thread) MONO_INTERNAL;
extern void ves_icall_System_Threading_Thread_Sleep_internal(int ms) MONO_INTERNAL;
extern gboolean ves_icall_System_Threading_Thread_Join_internal(MonoThread *this_obj, int ms, HANDLE thread) MONO_INTERNAL;
extern gint32 ves_icall_System_Threading_Thread_GetDomainID (void) MONO_INTERNAL;
extern MonoString* ves_icall_System_Threading_Thread_GetName_internal (MonoThread *this_obj) MONO_INTERNAL;
extern void ves_icall_System_Threading_Thread_SetName_internal (MonoThread *this_obj, MonoString *name) MONO_INTERNAL;
extern MonoObject* ves_icall_System_Threading_Thread_GetCachedCurrentCulture (MonoThread *this_obj) MONO_INTERNAL;
extern MonoArray* ves_icall_System_Threading_Thread_GetSerializedCurrentCulture (MonoThread *this_obj) MONO_INTERNAL;
extern void ves_icall_System_Threading_Thread_SetCachedCurrentCulture (MonoThread *this_obj, MonoObject *culture) MONO_INTERNAL;
void ves_icall_System_Threading_Thread_SetSerializedCurrentCulture (MonoThread *this_obj, MonoArray *arr) MONO_INTERNAL;
extern MonoObject* ves_icall_System_Threading_Thread_GetCachedCurrentUICulture (MonoThread *this_obj) MONO_INTERNAL;
extern MonoArray* ves_icall_System_Threading_Thread_GetSerializedCurrentUICulture (MonoThread *this_obj) MONO_INTERNAL;
extern void ves_icall_System_Threading_Thread_SetCachedCurrentUICulture (MonoThread *this_obj, MonoObject *culture) MONO_INTERNAL;
void ves_icall_System_Threading_Thread_SetSerializedCurrentUICulture (MonoThread *this_obj, MonoArray *arr) MONO_INTERNAL;
extern HANDLE ves_icall_System_Threading_Mutex_CreateMutex_internal(MonoBoolean owned, MonoString *name, MonoBoolean *created) MONO_INTERNAL;
extern void ves_icall_System_Threading_Mutex_ReleaseMutex_internal (HANDLE handle ) MONO_INTERNAL;
extern HANDLE ves_icall_System_Threading_Mutex_OpenMutex_internal (MonoString *name, gint32 rights, gint32 *error) MONO_INTERNAL;
extern HANDLE ves_icall_System_Threading_Semaphore_CreateSemaphore_internal (gint32 initialCount, gint32 maximumCount, MonoString *name, MonoBoolean *created) MONO_INTERNAL;
extern gint32 ves_icall_System_Threading_Semaphore_ReleaseSemaphore_internal (HANDLE handle, gint32 releaseCount, MonoBoolean *fail) MONO_INTERNAL;
extern HANDLE ves_icall_System_Threading_Semaphore_OpenSemaphore_internal (MonoString *name, gint32 rights, gint32 *error) MONO_INTERNAL;
extern HANDLE ves_icall_System_Threading_Events_CreateEvent_internal (MonoBoolean manual, MonoBoolean initial, MonoString *name, MonoBoolean *created) MONO_INTERNAL;
extern gboolean ves_icall_System_Threading_Events_SetEvent_internal (HANDLE handle) MONO_INTERNAL;
extern gboolean ves_icall_System_Threading_Events_ResetEvent_internal (HANDLE handle) MONO_INTERNAL;
extern void ves_icall_System_Threading_Events_CloseEvent_internal (HANDLE handle) MONO_INTERNAL;
extern HANDLE ves_icall_System_Threading_Events_OpenEvent_internal (MonoString *name, gint32 rights, gint32 *error) MONO_INTERNAL;

extern gboolean ves_icall_System_Threading_WaitHandle_WaitAll_internal(MonoArray *mono_handles, gint32 ms, gboolean exitContext) MONO_INTERNAL;
extern gint32 ves_icall_System_Threading_WaitHandle_WaitAny_internal(MonoArray *mono_handles, gint32 ms, gboolean exitContext) MONO_INTERNAL;
extern gboolean ves_icall_System_Threading_WaitHandle_WaitOne_internal(MonoObject *this_obj, HANDLE handle, gint32 ms, gboolean exitContext) MONO_INTERNAL;

extern gint32 ves_icall_System_Threading_Interlocked_Increment_Int(gint32 *location) MONO_INTERNAL;
extern gint64 ves_icall_System_Threading_Interlocked_Increment_Long(gint64 *location) MONO_INTERNAL;
extern gint32 ves_icall_System_Threading_Interlocked_Decrement_Int(gint32 *location) MONO_INTERNAL;
extern gint64 ves_icall_System_Threading_Interlocked_Decrement_Long(gint64 * location) MONO_INTERNAL;

extern gint32 ves_icall_System_Threading_Interlocked_Exchange_Int(gint32 *location, gint32 value) MONO_INTERNAL;
extern gint64 ves_icall_System_Threading_Interlocked_Exchange_Long(gint64 *location, gint64 value) MONO_INTERNAL;
extern MonoObject *ves_icall_System_Threading_Interlocked_Exchange_Object(MonoObject **location, MonoObject *value) MONO_INTERNAL;
extern gfloat ves_icall_System_Threading_Interlocked_Exchange_Single(gfloat *location, gfloat value) MONO_INTERNAL;
extern gdouble ves_icall_System_Threading_Interlocked_Exchange_Double(gdouble *location, gdouble value) MONO_INTERNAL;

extern gint32 ves_icall_System_Threading_Interlocked_CompareExchange_Int(gint32 *location, gint32 value, gint32 comparand) MONO_INTERNAL;
extern gint64 ves_icall_System_Threading_Interlocked_CompareExchange_Long(gint64 *location, gint64 value, gint64 comparand) MONO_INTERNAL;
extern MonoObject *ves_icall_System_Threading_Interlocked_CompareExchange_Object(MonoObject **location, MonoObject *value, MonoObject *comparand) MONO_INTERNAL;
extern gfloat ves_icall_System_Threading_Interlocked_CompareExchange_Single(gfloat *location, gfloat value, gfloat comparand) MONO_INTERNAL;
extern gdouble ves_icall_System_Threading_Interlocked_CompareExchange_Double(gdouble *location, gdouble value, gdouble comparand) MONO_INTERNAL;
extern MonoObject* ves_icall_System_Threading_Interlocked_CompareExchange_T(MonoObject **location, MonoObject *value, MonoObject *comparand) MONO_INTERNAL;
extern MonoObject* ves_icall_System_Threading_Interlocked_Exchange_T(MonoObject **location, MonoObject *value) MONO_INTERNAL;

extern gint32 ves_icall_System_Threading_Interlocked_Add_Int(gint32 *location, gint32 value) MONO_INTERNAL;
extern gint64 ves_icall_System_Threading_Interlocked_Add_Long(gint64 *location, gint64 value) MONO_INTERNAL;
extern gint64 ves_icall_System_Threading_Interlocked_Read_Long(gint64 *location) MONO_INTERNAL;

extern gint32 ves_icall_System_Threading_Interlocked_Increment_Int(gint32 *location) MONO_INTERNAL;
extern gint64 ves_icall_System_Threading_Interlocked_Increment_Long(gint64 *location) MONO_INTERNAL;

extern gint32 ves_icall_System_Threading_Interlocked_Decrement_Int(gint32 *location) MONO_INTERNAL;
extern gint64 ves_icall_System_Threading_Interlocked_Decrement_Long(gint64 * location) MONO_INTERNAL;

extern void ves_icall_System_Threading_Thread_Abort (MonoThread *thread, MonoObject *state) MONO_INTERNAL;
extern void ves_icall_System_Threading_Thread_ResetAbort (void) MONO_INTERNAL;
extern void ves_icall_System_Threading_Thread_Suspend (MonoThread *thread) MONO_INTERNAL;
extern void ves_icall_System_Threading_Thread_Resume (MonoThread *thread) MONO_INTERNAL;
extern void ves_icall_System_Threading_Thread_ClrState (MonoThread *thread, guint32 state) MONO_INTERNAL;
extern void ves_icall_System_Threading_Thread_SetState (MonoThread *thread, guint32 state) MONO_INTERNAL;
extern guint32 ves_icall_System_Threading_Thread_GetState (MonoThread *thread) MONO_INTERNAL;

gint8 ves_icall_System_Threading_Thread_VolatileRead1 (void *ptr) MONO_INTERNAL;
gint16 ves_icall_System_Threading_Thread_VolatileRead2 (void *ptr) MONO_INTERNAL;
gint32 ves_icall_System_Threading_Thread_VolatileRead4 (void *ptr) MONO_INTERNAL;
gint64 ves_icall_System_Threading_Thread_VolatileRead8 (void *ptr) MONO_INTERNAL;
void * ves_icall_System_Threading_Thread_VolatileReadIntPtr (void *ptr) MONO_INTERNAL;

void ves_icall_System_Threading_Thread_VolatileWrite1 (void *ptr, gint8) MONO_INTERNAL;
void ves_icall_System_Threading_Thread_VolatileWrite2 (void *ptr, gint16) MONO_INTERNAL;
void ves_icall_System_Threading_Thread_VolatileWrite4 (void *ptr, gint32) MONO_INTERNAL;
void ves_icall_System_Threading_Thread_VolatileWrite8 (void *ptr, gint64) MONO_INTERNAL;
void ves_icall_System_Threading_Thread_VolatileWriteIntPtr (void *ptr, void *) MONO_INTERNAL;

void ves_icall_System_Threading_Thread_MemoryBarrier (void) MONO_INTERNAL;

void mono_thread_free_local_slot_values (int slot, MonoBoolean thread_local) MONO_INTERNAL;

#endif /* _MONO_METADATA_THREADS_TYPES_H_ */
