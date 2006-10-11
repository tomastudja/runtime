/*
 * process.h: System.Diagnostics.Process support
 *
 * Author:
 *	Dick Porter (dick@ximian.com)
 *
 * (C) 2002 Ximian, Inc.
 */

#ifndef _MONO_METADATA_PROCESS_H_
#define _MONO_METADATA_PROCESS_H_

#include <config.h>
#include <glib.h>

#include <mono/metadata/object.h>
#include <mono/io-layer/io-layer.h>
#include "mono/utils/mono-compiler.h"

typedef struct 
{
	HANDLE process_handle;
	HANDLE thread_handle;
	guint32 pid; /* Contains GetLastError () on failure */
	guint32 tid;
	MonoArray *env_keys;
	MonoArray *env_values;
} MonoProcInfo;

typedef struct
{
	MonoObject object;
	MonoString *arguments;
	gpointer error_dialog_parent_handle;
	MonoString *filename;
	MonoString *verb;
	MonoString *working_directory;
	MonoObject *envVars;
	MonoBoolean create_no_window;
	MonoBoolean error_dialog;
	MonoBoolean redirect_standard_error;
	MonoBoolean redirect_standard_input;
	MonoBoolean redirect_standard_output;
	MonoBoolean use_shell_execute;
	guint32 window_style;
} MonoProcessStartInfo;

G_BEGIN_DECLS

HANDLE ves_icall_System_Diagnostics_Process_GetProcess_internal (guint32 pid) MONO_INTERNAL;
MonoArray *ves_icall_System_Diagnostics_Process_GetProcesses_internal (void) MONO_INTERNAL;
guint32 ves_icall_System_Diagnostics_Process_GetPid_internal (void) MONO_INTERNAL;
void ves_icall_System_Diagnostics_Process_Process_free_internal (MonoObject *this, HANDLE process) MONO_INTERNAL;
MonoArray *ves_icall_System_Diagnostics_Process_GetModules_internal (MonoObject *this) MONO_INTERNAL;
void ves_icall_System_Diagnostics_FileVersionInfo_GetVersionInfo_internal (MonoObject *this, MonoString *filename) MONO_INTERNAL;
MonoBoolean ves_icall_System_Diagnostics_Process_ShellExecuteEx_internal (MonoProcessStartInfo *proc_start_info, MonoProcInfo *process_handle) MONO_INTERNAL;
MonoBoolean ves_icall_System_Diagnostics_Process_CreateProcess_internal (MonoProcessStartInfo *proc_start_info, HANDLE stdin_handle, HANDLE stdout_handle, HANDLE stderr_handle, MonoProcInfo *process_handle) MONO_INTERNAL;
MonoBoolean ves_icall_System_Diagnostics_Process_WaitForExit_internal (MonoObject *this, HANDLE process, gint32 ms) MONO_INTERNAL;
gint64 ves_icall_System_Diagnostics_Process_ExitTime_internal (HANDLE process) MONO_INTERNAL;
gint64 ves_icall_System_Diagnostics_Process_StartTime_internal (HANDLE process) MONO_INTERNAL;
gint32 ves_icall_System_Diagnostics_Process_ExitCode_internal (HANDLE process) MONO_INTERNAL;
MonoString *ves_icall_System_Diagnostics_Process_ProcessName_internal (HANDLE process) MONO_INTERNAL;
MonoBoolean ves_icall_System_Diagnostics_Process_GetWorkingSet_internal (HANDLE process, guint32 *min, guint32 *max) MONO_INTERNAL;
MonoBoolean ves_icall_System_Diagnostics_Process_SetWorkingSet_internal (HANDLE process, guint32 min, guint32 max, MonoBoolean use_min) MONO_INTERNAL;
MonoBoolean ves_icall_System_Diagnostics_Process_Kill_internal (HANDLE process, gint32 sig) MONO_INTERNAL;

G_END_DECLS

#endif /* _MONO_METADATA_PROCESS_H_ */

