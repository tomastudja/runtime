/**
 * \file
 * Support for verbose unmanaged crash dumps
 *
 * Author:
 *   Alexander Kyte (alkyte@microsoft.com)
 *
 * (C) 2018 Microsoft, Inc.
 *
 */
#include <config.h>
#include <glib.h>

#include <mono/utils/json.h>
#include <mono/mini/debugger-state-machine.h>
#include <mono/mini/debugger-state-machine.h>
#include <mono/metadata/object-internals.h>
#include <mono/mini/mini-runtime.h>
#include <mono/mini/debugger-engine.h>

typedef enum {
	DEBUG_LOG_ILLEGAL = 0x0,
	DEBUG_LOG_STATE_CHANGE = 0x1,
	DEBUG_LOG_BREAKPOINT = 0x2,
	DEBUG_LOG_COMMAND = 0x3,
	DEBUG_LOG_EVENT = 0x4,
	DEBUG_LOG_EXIT = 0x5
} MonoDebugLogKind;

typedef struct {
	MonoDebugLogKind kind;

	intptr_t tid;
	const char *message;

	long counter; // The number of logs allocated
} MonoDebugLogItem;

typedef struct {
	intptr_t cursor;
	intptr_t max_size;
	MonoDebugLogItem *items;
} MonoDebuggerLog;

typedef struct {
	intptr_t lowest_index;
	intptr_t highest_index;
} MonoDebuggerLogIter;

static MonoDebuggerLog *debugger_log;

#define MAX_DEBUGGER_LOG_LEN 4
#define MONO_DEBUGGER_LOG_UNINIT -1

static const char *
mono_debug_log_kind_to_string (MonoDebugLogKind kind)
{
	switch (kind) {
	case DEBUG_LOG_STATE_CHANGE: return "transition";
	case DEBUG_LOG_BREAKPOINT: return "breakpoint";
	case DEBUG_LOG_COMMAND: return "command";
	case DEBUG_LOG_EVENT: return "event";
	case DEBUG_LOG_EXIT: return "exit";
	default:
		g_assert_not_reached ();
	}
}

static const char *
mono_debug_log_thread_state_to_string (MonoDebuggerThreadState state)
{
	switch (state) {
	case MONO_DEBUGGER_SUSPENDED: return "suspended";
	case MONO_DEBUGGER_RESUMED: return "resumed";
	case MONO_DEBUGGER_TERMINATED: return "terminated";
	case MONO_DEBUGGER_STARTED: return "started";
	default:
		g_assert_not_reached ();
	}
}

void
mono_debugger_log_init (void)
{
	mono_loader_lock ();

	if (debugger_log == GINT_TO_POINTER (MONO_DEBUGGER_LOG_UNINIT))
		g_error ("Attempted to initialize debugger log after cleanup");

	debugger_log = g_malloc0 (sizeof (MonoDebuggerLog));
	debugger_log->max_size = MAX_DEBUGGER_LOG_LEN;
	debugger_log->cursor = MONO_DEBUGGER_LOG_UNINIT;
	debugger_log->items = g_malloc0 (sizeof (MonoDebugLogItem) * debugger_log->max_size);
	mono_loader_unlock ();
}

void
mono_debugger_log_free (void)
{
	mono_loader_lock ();
	g_free (debugger_log->items);
	g_free (debugger_log);
	debugger_log = GINT_TO_POINTER (MONO_DEBUGGER_LOG_UNINIT);
	mono_loader_unlock ();
}

static void
debugger_log_append (MonoDebugLogKind kind, intptr_t tid, const char *message)
{
	MonoDebugLogItem *item, *old_item;

	if (!debugger_log || (debugger_log == GINT_TO_POINTER (MONO_DEBUGGER_LOG_UNINIT)))
		mono_debugger_log_init ();

	mono_loader_lock ();

	if (debugger_log->cursor == MONO_DEBUGGER_LOG_UNINIT) {
		item = &debugger_log->items [0];
		item->counter = 0;
	} else {
		// We have a ring buffer
		old_item = &debugger_log->items [debugger_log->cursor % debugger_log->max_size];
		item = &debugger_log->items [(debugger_log->cursor + 1) % debugger_log->max_size];
		item->counter = old_item->counter + 1;
	}

	debugger_log->cursor++;

	item->tid = tid;
	item->kind = kind;
	item->message = message;

#if 0
	MOSTLY_ASYNC_SAFE_PRINTF ("DEBUGGER LOG (%zu): (%s:0x%x:%s) \n", item->counter, mono_debug_log_kind_to_string (item->kind), item->tid, item->message);
#endif

	mono_loader_unlock ();
}

static void
debugger_log_iter_init (MonoDebuggerLogIter *iter) 
{
	// Make sure we are initialized
	g_assert (debugger_log->max_size > 0);

	if (debugger_log->cursor == MONO_DEBUGGER_LOG_UNINIT) {
		iter->lowest_index = MONO_DEBUGGER_LOG_UNINIT; 
		iter->highest_index = MONO_DEBUGGER_LOG_UNINIT;
	} else if (debugger_log->cursor >= debugger_log->max_size) {
		// Ring buffer has wrapped around
		// So the item *after* the highest index is the lowest index
		iter->highest_index = (debugger_log->cursor + 1) % debugger_log->max_size;
		iter->lowest_index = (iter->highest_index + 1) % debugger_log->max_size;
	} else {
		iter->lowest_index = 0;
		iter->highest_index = debugger_log->cursor;
	}
}

static void
debugger_log_iter_destroy (MonoDebuggerLogIter *iter) 
{
	return;
}

static gboolean
debugger_log_iter_next (MonoDebuggerLogIter *iter, MonoDebugLogItem **item)
{
	if (iter->lowest_index == MONO_DEBUGGER_LOG_UNINIT)
		return FALSE;

	if (iter->lowest_index == iter->highest_index)
		return FALSE;

	if (iter->lowest_index == MONO_DEBUGGER_LOG_UNINIT)
		return FALSE;

	g_assert (iter->lowest_index >= 0);
	g_assert (iter->lowest_index < debugger_log->max_size);

	*item = &debugger_log->items [iter->lowest_index];
	iter->lowest_index++;

	if (iter->lowest_index >= debugger_log->max_size)
		iter->lowest_index = iter->lowest_index % debugger_log->max_size;

	return TRUE;
}

void
mono_debugger_log_command (const char *command_set, const char *command, guint8 *buf, int len)
{
	// FIXME: print the array in a format that can be decoded / printed?
	char *msg = g_strdup_printf ("Command Logged: %s %s Response: %d", command_set, command, len);
	debugger_log_append (DEBUG_LOG_COMMAND, 0x0, msg);
}

void
mono_debugger_log_event (DebuggerTlsData *tls, const char *event, guint8 *buf, int len)
{
	// FIXME: print the array in a format that can be decoded / printed?
	intptr_t tid = mono_debugger_tls_thread_id (tls);
	char *msg = g_strdup_printf ("Event logged of type %s Response: %d", event, len);
	debugger_log_append (DEBUG_LOG_EVENT, tid, msg);
}

void
mono_debugger_log_exit (int exit_code)
{
	char *msg = g_strdup_printf ("Exited with code %d", exit_code);
	debugger_log_append (DEBUG_LOG_EXIT, 0x0, msg);
}

void
mono_debugger_log_add_bp (MonoMethod *method, long il_offset)
{
	char *msg = g_strdup_printf ("Add breakpoint %s %lu", method ? mono_method_full_name (method, TRUE) : "No method", il_offset);
	debugger_log_append (DEBUG_LOG_BREAKPOINT, 0x0, msg);
}

void
mono_debugger_log_remove_bp (MonoMethod *method, long il_offset)
{
	char *msg = g_strdup_printf ("Remove breakpoint %s %lu", method ? mono_method_full_name (method, TRUE) : "No method", il_offset);
	debugger_log_append (DEBUG_LOG_BREAKPOINT, 0x0, msg);
}

void
mono_debugger_log_bp_hit (DebuggerTlsData *tls, MonoMethod *method, long il_offset)
{
	intptr_t tid = mono_debugger_tls_thread_id (tls);
	char *msg = g_strdup_printf ("Hit breakpoint %s %lu", method ? mono_method_full_name (method, TRUE) : "No method", il_offset);
	debugger_log_append (DEBUG_LOG_BREAKPOINT, tid, msg);
}

void
mono_debugger_log_resume (DebuggerTlsData *tls)
{
	intptr_t tid = mono_debugger_tls_thread_id (tls);
	MonoDebuggerThreadState prev_state = mono_debugger_get_thread_state (tls);
	g_assert (prev_state == MONO_DEBUGGER_SUSPENDED || prev_state == MONO_DEBUGGER_STARTED);
	mono_debugger_set_thread_state (tls, prev_state, MONO_DEBUGGER_RESUMED);

	char *msg = g_strdup_printf ("Resuming 0x%x from state %s", tid, mono_debug_log_thread_state_to_string (prev_state));
	debugger_log_append (DEBUG_LOG_STATE_CHANGE, tid, msg);
}

void
mono_debugger_log_suspend (DebuggerTlsData *tls)
{
	intptr_t tid = mono_debugger_tls_thread_id (tls);
	MonoDebuggerThreadState prev_state = mono_debugger_get_thread_state (tls);
	g_assert (prev_state == MONO_DEBUGGER_RESUMED || prev_state == MONO_DEBUGGER_STARTED);
	mono_debugger_set_thread_state (tls, prev_state, MONO_DEBUGGER_SUSPENDED);

	char *msg = g_strdup_printf ("Suspending 0x%x from state %s", tid, tid, mono_debug_log_thread_state_to_string (prev_state));
	debugger_log_append (DEBUG_LOG_STATE_CHANGE, tid, msg);
}

typedef struct {
	JsonWriter *writer;
	gboolean not_first;
} DebuggerThreadIterState;

// FIXME: log TCP handshake / connection state
static void
dump_thread_state (gpointer key, gpointer value, gpointer user_data)
{
	DebuggerTlsData *debugger_tls = (DebuggerTlsData *) value;
	DebuggerThreadIterState *data = (DebuggerThreadIterState *) user_data;

	if (data->not_first)
		mono_json_writer_printf (data->writer, ",\n");
	else
		data->not_first = TRUE;

	mono_json_writer_indent (data->writer);
	mono_json_writer_object_begin (data->writer);

	mono_json_writer_indent (data->writer);
	mono_json_writer_object_key(data->writer, "thread_id");
	mono_json_writer_printf (data->writer, "\"0x%x\",\n", mono_debugger_tls_thread_id (debugger_tls));

	mono_json_writer_indent (data->writer);
	mono_json_writer_object_key (data->writer, "thread_state");
	const char *state = mono_debug_log_thread_state_to_string (mono_debugger_get_thread_state (debugger_tls));
	mono_json_writer_printf (data->writer, "\"%s\"\n", state);

	mono_json_writer_indent_pop (data->writer);
	mono_json_writer_indent (data->writer);
	mono_json_writer_object_end (data->writer);
}

void
mono_debugger_state (JsonWriter *writer)
{
	if (debugger_log->max_size == 0) {
		return;
	}

	mono_loader_lock ();
	mono_json_writer_object_begin(writer);

	mono_json_writer_indent (writer);
	mono_json_writer_object_key(writer, "debugger_state");
	mono_json_writer_object_begin(writer);

	mono_json_writer_indent (writer);
	mono_json_writer_object_key(writer, "thread_states");
	mono_json_writer_array_begin (writer);
	mono_json_writer_indent_push (writer);

	DebuggerThreadIterState iterState;
	iterState.writer = writer;
	iterState.not_first = FALSE;
	MonoGHashTable *thread_to_tls = mono_debugger_get_thread_states ();
	mono_g_hash_table_foreach (thread_to_tls, dump_thread_state, &iterState);
	mono_json_writer_printf (writer, "\n");

	mono_json_writer_indent_pop (writer);
	mono_json_writer_indent (writer);
	mono_json_writer_array_end (writer);

	mono_json_writer_printf (writer, ",\n");

	// FIXME: Log breakpoint state
	MonoBreakpoint *bps;
	int num_bps = mono_de_current_breakpoints (&bps);
	if (num_bps) {
		mono_json_writer_indent (writer);
		mono_json_writer_object_key(writer, "breakpoints");
		mono_json_writer_array_begin (writer);

		for (int i=0; i < num_bps; i++) {
			mono_json_writer_indent (writer);
			mono_json_writer_object_begin(writer);

			mono_json_writer_indent (writer);
			mono_json_writer_object_key(writer, "method");
			mono_json_writer_printf (writer, "\"%s\",\n", bps [i].method ? mono_method_full_name (bps [i].method, TRUE) : "No method");

			mono_json_writer_indent (writer);
			mono_json_writer_object_key(writer, "il_offset");
			mono_json_writer_printf (writer, "\"0x%x\",\n", bps [i].il_offset);

			mono_json_writer_indent_pop (writer);
			mono_json_writer_indent (writer);
			mono_json_writer_object_end (writer);
			mono_json_writer_printf (writer, ",\n");
		}

		mono_json_writer_printf (writer, ",\n");

		mono_json_writer_indent_pop (writer);
		mono_json_writer_indent (writer);
		mono_json_writer_array_end (writer);
		mono_json_writer_printf (writer, ",\n");
	}

	g_free (bps);

	// Log history
	MonoDebuggerLogIter diter;
	MonoDebugLogItem *item;
	debugger_log_iter_init (&diter);

	mono_json_writer_indent (writer);
	mono_json_writer_object_key(writer, "debugger history");
	mono_json_writer_array_begin (writer);

	while (debugger_log_iter_next (&diter, &item)) {
		mono_json_writer_indent (writer);
		mono_json_writer_object_begin(writer);

		mono_json_writer_indent (writer);
		mono_json_writer_object_key(writer, "kind");
		mono_json_writer_printf (writer, "\"%s\",\n", mono_debug_log_kind_to_string (item->kind));

		mono_json_writer_indent (writer);
		mono_json_writer_object_key(writer, "tid");
		mono_json_writer_printf (writer, "\"0x%x\",\n", item->tid);

		mono_json_writer_indent (writer);
		mono_json_writer_object_key(writer, "message");
		mono_json_writer_printf (writer, "\"%s\",\n", item->message);

		mono_json_writer_indent (writer);
		mono_json_writer_object_key(writer, "counter");
		mono_json_writer_printf (writer, "\"%d\",\n", item->counter);

		mono_json_writer_indent_pop (writer);
		mono_json_writer_indent (writer);
		mono_json_writer_object_end (writer);
		mono_json_writer_printf (writer, ",\n");
	}
	mono_json_writer_printf (writer, "\n");

	mono_json_writer_indent_pop (writer);
	mono_json_writer_indent (writer);
	mono_json_writer_array_end (writer);

	debugger_log_iter_destroy (&diter);

	// Log client/connection state
	gboolean disconnected = mono_debugger_is_disconnected ();
	mono_json_writer_indent (writer);
	mono_json_writer_object_key(writer, "client_state");
	mono_json_writer_printf (writer, "\"%s\",\n", disconnected ? "disconnected" : "connected");

	mono_json_writer_indent_pop (writer);
	mono_json_writer_indent (writer);
	mono_json_writer_object_end (writer);

	mono_loader_unlock ();
}

