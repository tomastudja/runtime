/**
* \file
* Win32 OS wait wrappers and interrupt/abort APC handling.
*
* Author:
*   Johan Lorensson (lateralusx.github@gmail.com)
*
* Licensed under the MIT license. See LICENSE file in the project root for full license information.
*/

#include <mono/utils/mono-os-wait.h>
#include <mono/utils/mono-threads.h>
#include <mono/utils/mono-threads-debug.h>

#define THREAD_WAIT_INFO_CLEARED 0
#define THREAD_WAIT_INFO_ALERTABLE_WAIT_SLOT 1
#define THREAD_WAIT_INFO_PENDING_INTERRUPT_APC_SLOT 2
#define THREAD_WAIT_INFO_PENDING_ABORT_APC_SLOT 4

static inline void
reqeust_interrupt (gpointer thread_info, HANDLE native_thread_handle, gint32 pending_apc_slot, PAPCFUNC apc_callback, DWORD tid)
{
	/*
	* On Windows platforms, an async interrupt/abort request queues an APC
	* that needs to be processed by target thread before it can return from an
	* alertable OS wait call and complete the mono interrupt/abort request.
	* Uncontrolled queuing of APC's could flood the APC queue preventing the target thread
	* to return from its alertable OS wait call, blocking the interrupt/abort requests to complete
	* This check makes sure that only one APC per type gets queued, preventing potential flooding
	* of the APC queue. NOTE, this code will execute regardless if targeted thread is currently in
	* an alertable wait or not. This is done to prevent races between interrupt/abort requests and
	* alertable wait calls. Threads already in an alertable wait should handle WAIT_IO_COMPLETION
	* return scenarios and restart the alertable wait operation if needed or take other actions
	* (like service the interrupt/abort request).
	*/
	MonoThreadInfo *info = (MonoThreadInfo *)thread_info;
	gboolean queue_apc = FALSE;

	while (!queue_apc) {
		gint32 old_wait_info = InterlockedRead (&info->thread_wait_info);
		if (old_wait_info & pending_apc_slot)
			break;

		gint32 new_wait_info = old_wait_info | pending_apc_slot;
		if (InterlockedCompareExchange (&info->thread_wait_info, new_wait_info, old_wait_info) == old_wait_info) {
			queue_apc = TRUE;
		}
	}

	if (queue_apc == TRUE) {
		THREADS_INTERRUPT_DEBUG ("%06d - Interrupting/Aborting syscall in thread %06d", GetCurrentThreadId (), tid);
		QueueUserAPC (apc_callback, native_thread_handle, (ULONG_PTR)NULL);
	}
}

static void CALLBACK
interrupt_apc (ULONG_PTR param)
{
	THREADS_INTERRUPT_DEBUG ("%06d - interrupt_apc () called", GetCurrentThreadId ());
}

void
mono_win32_interrupt_wait (PVOID thread_info, HANDLE native_thread_handle, DWORD tid)
{
	reqeust_interrupt (thread_info, native_thread_handle, THREAD_WAIT_INFO_PENDING_INTERRUPT_APC_SLOT, interrupt_apc, tid);
}

static void CALLBACK
abort_apc (ULONG_PTR param)
{
	THREADS_INTERRUPT_DEBUG ("%06d - abort_apc () called", GetCurrentThreadId ());
}

void
mono_win32_abort_wait (PVOID thread_info, HANDLE native_thread_handle, DWORD tid)
{
	reqeust_interrupt (thread_info, native_thread_handle, THREAD_WAIT_INFO_PENDING_ABORT_APC_SLOT, abort_apc, tid);
}

static inline void
enter_alertable_wait (MonoThreadInfo *info)
{
	// Clear any previous flags. Set alertable wait flag.
	InterlockedExchange (&info->thread_wait_info, THREAD_WAIT_INFO_ALERTABLE_WAIT_SLOT);
}

static inline void
leave_alertable_wait (MonoThreadInfo *info)
{
	// Clear any previous flags. Thread is exiting alertable wait state, and info around pending interrupt/abort APC's
	// can now be discarded as well, thread is out of wait operation and can proceed it's execution.
	InterlockedExchange (&info->thread_wait_info, THREAD_WAIT_INFO_CLEARED);
}

DWORD
mono_win32_sleep_ex (DWORD timeout, BOOL alertable)
{
	DWORD result = WAIT_FAILED;
	MonoThreadInfo *info = mono_thread_info_current_unchecked ();

	if (alertable && info) {
		enter_alertable_wait (info);
	}

	result = SleepEx (timeout, alertable);

	// NOTE, leave_alertable_wait should not affect GetLastError but
	// if changed, GetLastError needs to be preserved and reset before returning.
	if (alertable && info) {
		leave_alertable_wait (info);
	}

	return result;
}

DWORD
mono_win32_wait_for_single_object_ex (HANDLE handle, DWORD timeout, BOOL alertable)
{
	DWORD result = WAIT_FAILED;
	MonoThreadInfo *info = mono_thread_info_current_unchecked ();

	if (alertable && info) {
		enter_alertable_wait (info);
	}

	result = WaitForSingleObjectEx (handle, timeout, alertable);

	// NOTE, leave_alertable_wait should not affect GetLastError but
	// if changed, GetLastError needs to be preserved and reset before returning.
	if (alertable && info) {
		leave_alertable_wait (info);
	}

	return result;
}

DWORD
mono_win32_wait_for_multiple_objects_ex (DWORD count, CONST HANDLE *handles, BOOL waitAll, DWORD timeout, BOOL alertable)
{
	DWORD result = WAIT_FAILED;
	MonoThreadInfo *info = mono_thread_info_current_unchecked ();

	if (alertable && info) {
		enter_alertable_wait (info);
	}

	result = WaitForMultipleObjectsEx (count, handles, waitAll, timeout, alertable);

	// NOTE, leave_alertable_wait should not affect GetLastError but
	// if changed, GetLastError needs to be preserved and reset before returning.
	if (alertable && info) {
		leave_alertable_wait (info);
	}

	return result;
}

DWORD
mono_win32_signal_object_and_wait (HANDLE toSignal, HANDLE toWait, DWORD timeout, BOOL alertable)
{
	DWORD result = WAIT_FAILED;
	MonoThreadInfo *info = mono_thread_info_current_unchecked ();

	if (alertable && info) {
		enter_alertable_wait (info);
	}

	result = SignalObjectAndWait (toSignal, toWait, timeout, alertable);

	// NOTE, leave_alertable_wait should not affect GetLastError but
	// if changed, GetLastError needs to be preserved and reset before returning.
	if (alertable && info) {
		leave_alertable_wait (info);
	}

	return result;
}

DWORD
mono_win32_msg_wait_for_multiple_objects_ex (DWORD count, CONST HANDLE *handles, DWORD timeout, DWORD wakeMask, DWORD flags)
{
	DWORD result = WAIT_FAILED;
	MonoThreadInfo *info = mono_thread_info_current_unchecked ();
	BOOL alertable = flags & MWMO_ALERTABLE;

	if (alertable && info) {
		enter_alertable_wait (info);
	}

	result = MsgWaitForMultipleObjectsEx (count, handles, timeout, wakeMask, flags);

	// NOTE, leave_alertable_wait should not affect GetLastError but
	// if changed, GetLastError needs to be preserved and reset before returning.
	if (alertable && info) {
		leave_alertable_wait (info);
	}

	return result;
}

DWORD
mono_win32_wsa_wait_for_multiple_events (DWORD count, const WSAEVENT FAR *handles, BOOL waitAll, DWORD timeout, BOOL alertable)
{
	DWORD result = WAIT_FAILED;
	MonoThreadInfo *info = mono_thread_info_current_unchecked ();

	if (alertable && info) {
		enter_alertable_wait (info);
	}

	result = WSAWaitForMultipleEvents (count, handles, waitAll, timeout, alertable);

	// NOTE, leave_alertable_wait should not affect GetLastError but
	// if changed, GetLastError needs to be preserved and reset before returning.
	if (alertable && info) {
		leave_alertable_wait (info);
	}

	return result;
}
