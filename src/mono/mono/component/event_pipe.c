// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
//

#include <config.h>
#include <mono/component/event_pipe.h>
#include <mono/component/event_pipe-wasm.h>
#include <mono/utils/mono-publib.h>
#include <mono/utils/mono-compiler.h>
#include <mono/utils/mono-threads-api.h>
#include <eventpipe/ep.h>
#include <eventpipe/ep-event.h>
#include <eventpipe/ep-event-instance.h>
#include <eventpipe/ep-session.h>

#if defined(HOST_WASM) && !defined(HOST_WASI)
#include <emscripten/emscripten.h>
#endif

extern void ep_rt_mono_component_init (void);
static bool _event_pipe_component_inited = false;

struct _EventPipeProviderConfigurationNative {
	gunichar2 *provider_name;
	uint64_t keywords;
	uint32_t logging_level;
	gunichar2 *filter_data;
};

struct _EventPipeSessionInfo {
	int64_t starttime_as_utc_filetime;
	int64_t start_timestamp;
	int64_t timestamp_frequency;
};

struct _EventPipeEventInstanceData {
	intptr_t provider_id;
	uint32_t event_id;
	uint32_t thread_id;
	int64_t timestamp;
	uint8_t activity_id [EP_ACTIVITY_ID_SIZE];
	uint8_t related_activity_id [EP_ACTIVITY_ID_SIZE];
	const uint8_t *payload;
	uint32_t payload_len;
};

/*
 * Forward declares of all static functions.
 */

static bool
event_pipe_available (void);

static EventPipeSessionID
event_pipe_enable (
	const ep_char8_t *output_path,
	uint32_t circular_buffer_size_in_mb,
	const EventPipeProviderConfigurationNative *providers,
	uint32_t providers_len,
	EventPipeSessionType session_type,
	EventPipeSerializationFormat format,
	bool rundown_requested,
	IpcStream *stream,
	EventPipeSessionSynchronousCallback sync_callback);

static bool
event_pipe_get_next_event (
	EventPipeSessionID session_id,
	EventPipeEventInstanceData *instance);

static bool
event_pipe_add_rundown_execution_checkpoint (const ep_char8_t *name);

static bool
event_pipe_add_rundown_execution_checkpoint_2 (
	const ep_char8_t *name,
	ep_timestamp_t timestamp);

static ep_timestamp_t
event_pipe_convert_100ns_ticks_to_timestamp_t (int64_t ticks_100ns);

static bool
event_pipe_get_session_info (
	EventPipeSessionID session_id,
	EventPipeSessionInfo *instance);

static bool
event_pipe_thread_ctrl_activity_id(
	EventPipeActivityControlCode activity_control_code,
	uint8_t *activity_id,
	uint32_t activity_id_len);

static bool
event_pipe_signal_session (EventPipeSessionID session_id);

static bool
event_pipe_wait_for_session_signal (
	EventPipeSessionID session_id,
	uint32_t timeout);

#if defined(HOST_WASM)  && !defined(HOST_WASI)
static void
mono_wasm_event_pipe_init (void);
#endif

static MonoComponentEventPipe fn_table = {
	{ MONO_COMPONENT_ITF_VERSION, &event_pipe_available },
#if !defined(HOST_WASM) || defined(HOST_WASI)
	&ep_init,
#else
	&mono_wasm_event_pipe_init,
#endif
	&ep_finish_init,
	&ep_shutdown,
	&event_pipe_enable,
	&ep_disable,
	&event_pipe_get_next_event,
	&ep_get_wait_handle,
	&ep_start_streaming,
	&ep_write_event_2,
	&event_pipe_add_rundown_execution_checkpoint,
	&event_pipe_add_rundown_execution_checkpoint_2,
	&event_pipe_convert_100ns_ticks_to_timestamp_t,
	&ep_create_provider,
	&ep_delete_provider,
	&ep_get_provider,
	&ep_provider_add_event,
	&event_pipe_get_session_info,
	&event_pipe_thread_ctrl_activity_id,
	&ep_rt_mono_write_event_ee_startup_start,
	&ep_rt_write_event_threadpool_worker_thread_start,
	&ep_rt_write_event_threadpool_worker_thread_stop,
	&ep_rt_write_event_threadpool_worker_thread_wait,
	&ep_rt_write_event_threadpool_min_max_threads,
	&ep_rt_write_event_threadpool_worker_thread_adjustment_sample,
	&ep_rt_write_event_threadpool_worker_thread_adjustment_adjustment,
	&ep_rt_write_event_threadpool_worker_thread_adjustment_stats,
	&ep_rt_write_event_threadpool_io_enqueue,
	&ep_rt_write_event_threadpool_io_dequeue,
	&ep_rt_write_event_threadpool_working_thread_count,
	&ep_rt_write_event_threadpool_io_pack,
	&event_pipe_signal_session,
	&event_pipe_wait_for_session_signal
};

static bool
event_pipe_available (void)
{
	return true;
}

static EventPipeSessionID
event_pipe_enable (
	const ep_char8_t *output_path,
	uint32_t circular_buffer_size_in_mb,
	const EventPipeProviderConfigurationNative *providers,
	uint32_t providers_len,
	EventPipeSessionType session_type,
	EventPipeSerializationFormat format,
	bool rundown_requested,
	IpcStream *stream,
	EventPipeSessionSynchronousCallback sync_callback)
{
	ERROR_DECL (error);
	EventPipeSessionID session_id = 0;

	EventPipeProviderConfiguration *config_providers = g_new0 (EventPipeProviderConfiguration, providers_len);

	if (config_providers) {
		for (guint32 i = 0; i < providers_len; ++i) {
			ep_provider_config_init (
				&config_providers[i],
				providers[i].provider_name ? mono_utf16_to_utf8 (providers[i].provider_name, g_utf16_len (providers[i].provider_name), error) : NULL,
				providers [i].keywords,
				(EventPipeEventLevel)providers [i].logging_level,
				providers[i].filter_data ? mono_utf16_to_utf8 (providers[i].filter_data, g_utf16_len (providers[i].filter_data), error) : NULL);
		}
	}

	session_id = ep_enable (
		output_path,
		circular_buffer_size_in_mb,
		config_providers,
		providers_len,
		session_type,
		format,
		rundown_requested,
		stream,
		sync_callback,
		NULL);

	if (config_providers) {
		for (guint32 i = 0; i < providers_len; ++i) {
			ep_provider_config_fini (&config_providers[i]);
			g_free ((ep_char8_t *)ep_provider_config_get_provider_name (&config_providers[i]));
			g_free ((ep_char8_t *)ep_provider_config_get_filter_data (&config_providers[i]));
		}
	}

	return session_id;
}

static bool
event_pipe_get_next_event (
	EventPipeSessionID session_id,
	EventPipeEventInstanceData *instance)
{
	EP_ASSERT (instance != NULL);

	EventPipeEventInstance *const next_instance = ep_get_next_event (session_id);
	EventPipeEventInstanceData *const data = (EventPipeEventInstanceData *)instance;
	if (next_instance && data) {
		const EventPipeEvent *const ep_event = ep_event_instance_get_ep_event (next_instance);
		if (ep_event) {
			data->provider_id = (intptr_t)ep_event_get_provider (ep_event);
			data->event_id = ep_event_get_event_id (ep_event);
		}
		data->thread_id = GUINT64_TO_UINT32 (ep_event_instance_get_thread_id (next_instance));
		data->timestamp = ep_event_instance_get_timestamp (next_instance);
		memcpy (&data->activity_id, ep_event_instance_get_activity_id_cref (next_instance), EP_ACTIVITY_ID_SIZE);
		memcpy (&data->related_activity_id, ep_event_instance_get_related_activity_id_cref (next_instance), EP_ACTIVITY_ID_SIZE);
		data->payload = ep_event_instance_get_data (next_instance);
		data->payload_len = ep_event_instance_get_data_len (next_instance);
	}

	return next_instance != NULL;
}

static bool
event_pipe_add_rundown_execution_checkpoint (const ep_char8_t *name)
{
	return ep_add_rundown_execution_checkpoint (name, ep_perf_timestamp_get ());
}

static bool
event_pipe_add_rundown_execution_checkpoint_2 (
	const ep_char8_t *name,
	ep_timestamp_t timestamp)
{
	return ep_add_rundown_execution_checkpoint (name, timestamp);
}

static ep_timestamp_t
event_pipe_convert_100ns_ticks_to_timestamp_t (int64_t ticks_100ns)
{
	// Convert into event pipe timestamp from a relative number of 100ns ticks (+/-).
	int64_t freq = ep_perf_frequency_query ();
	ep_timestamp_t ticks = (ep_timestamp_t)(((double)ticks_100ns / 10000000) * freq);
	ep_timestamp_t timestamp = ep_perf_timestamp_get () + ticks;

	return timestamp > 0 ? timestamp : 0;
}

static bool
event_pipe_get_session_info (
	EventPipeSessionID session_id,
	EventPipeSessionInfo *instance)
{
	bool result = false;
	if (instance) {
		EventPipeSession *session = ep_get_session ((EventPipeSessionID)session_id);
		if (session) {
			instance->starttime_as_utc_filetime = ep_session_get_session_start_time (session);
			instance->start_timestamp = ep_session_get_session_start_timestamp (session);
			instance->timestamp_frequency = ep_perf_frequency_query ();
			result = true;
		}
	}

	return result;
}

static bool
event_pipe_thread_ctrl_activity_id (
	EventPipeActivityControlCode activity_control_code,
	uint8_t *activity_id,
	uint32_t activity_id_len)
{
	bool result = true;
	ep_rt_thread_activity_id_handle_t activity_id_handle = ep_thread_get_activity_id_handle ();

	if (activity_id_handle == NULL)
		return false;

	uint8_t current_activity_id [EP_ACTIVITY_ID_SIZE];
	switch (activity_control_code) {
	case EP_ACTIVITY_CONTROL_GET_ID:
		ep_thread_get_activity_id (activity_id_handle, activity_id, EP_ACTIVITY_ID_SIZE);
		break;
	case EP_ACTIVITY_CONTROL_SET_ID:
		ep_thread_set_activity_id (activity_id_handle, activity_id, EP_ACTIVITY_ID_SIZE);
		break;
	case EP_ACTIVITY_CONTROL_CREATE_ID:
		ep_thread_create_activity_id (activity_id, EP_ACTIVITY_ID_SIZE);
		break;
	case EP_ACTIVITY_CONTROL_GET_SET_ID:
		ep_thread_get_activity_id (activity_id_handle, current_activity_id, EP_ACTIVITY_ID_SIZE);
		ep_thread_set_activity_id (activity_id_handle, activity_id, EP_ACTIVITY_ID_SIZE);
		memcpy (activity_id, current_activity_id, EP_ACTIVITY_ID_SIZE);
		break;
	case EP_ACTIVITY_CONTROL_CREATE_SET_ID:
		ep_thread_get_activity_id (activity_id_handle, activity_id, EP_ACTIVITY_ID_SIZE);
		ep_thread_create_activity_id (current_activity_id, EP_ACTIVITY_ID_SIZE);
		ep_thread_set_activity_id (activity_id_handle, current_activity_id, EP_ACTIVITY_ID_SIZE);
		break;
	default:
		result = false;
		break;
	}

	return result;
}

static bool
event_pipe_signal_session (EventPipeSessionID session_id)
{
	EventPipeSession *const session = ep_get_session (session_id);
	if (!session)
		return false;

	return ep_rt_wait_event_set (ep_session_get_wait_event (session));
}

static bool
event_pipe_wait_for_session_signal (
	EventPipeSessionID session_id,
	uint32_t timeout)
{
	EventPipeSession *const session = ep_get_session (session_id);
	if (!session)
		return false;

	return !ep_rt_wait_event_wait (ep_session_get_wait_event (session), timeout, false) ? true : false;
}

MonoComponentEventPipe *
mono_component_event_pipe_init (void)
{
	if (!_event_pipe_component_inited) {
		ep_rt_mono_component_init ();
		_event_pipe_component_inited = true;
	}

	return &fn_table;
}


#if defined(HOST_WASM) && !defined(HOST_WASI)


static MonoWasmEventPipeSessionID
ep_to_wasm_session_id (EventPipeSessionID session_id)
{
	g_assert (0 == (uint64_t)session_id >> 32);
	return (uint32_t)session_id;
}

static EventPipeSessionID
wasm_to_ep_session_id (MonoWasmEventPipeSessionID session_id)
{
	return session_id;
}

EMSCRIPTEN_KEEPALIVE gboolean
mono_wasm_event_pipe_enable (const ep_char8_t *output_path,
			     IpcStream *ipc_stream,
			     uint32_t circular_buffer_size_in_mb,
			     const ep_char8_t *providers,
			     /* EventPipeSessionType session_type = EP_SESSION_TYPE_FILE, */
			     /* EventPipieSerializationFormat format = EP_SERIALIZATION_FORMAT_NETTRACE_V4, */
			     /* bool */ gboolean rundown_requested,
			     /* EventPipeSessionSycnhronousCallback sync_callback = NULL, */
			     /* void *callback_additional_data, */
			     MonoWasmEventPipeSessionID *out_session_id)
{
	MONO_ENTER_GC_UNSAFE;
	EventPipeSerializationFormat format = EP_SERIALIZATION_FORMAT_NETTRACE_V4;
	EventPipeSessionType session_type = output_path != NULL ? EP_SESSION_TYPE_FILE : EP_SESSION_TYPE_IPCSTREAM;

	g_assert ((output_path == NULL && ipc_stream != NULL) ||
		  (output_path != NULL && ipc_stream == NULL));

	EventPipeSessionID session;
	session = ep_enable_2 (output_path,
			       circular_buffer_size_in_mb,
			       providers,
			       session_type,
			       format,
			       !!rundown_requested,
			       ipc_stream,
			       /* callback*/ NULL,
			       /* callback_data*/ NULL);
  
	if (out_session_id)
		*out_session_id = ep_to_wasm_session_id (session);
	MONO_EXIT_GC_UNSAFE;
	return TRUE;
}

EMSCRIPTEN_KEEPALIVE gboolean
mono_wasm_event_pipe_session_start_streaming (MonoWasmEventPipeSessionID session_id)
{
	MONO_ENTER_GC_UNSAFE;
	ep_start_streaming (wasm_to_ep_session_id (session_id));
	MONO_EXIT_GC_UNSAFE;
	return TRUE;
}

EMSCRIPTEN_KEEPALIVE gboolean
mono_wasm_event_pipe_session_disable (MonoWasmEventPipeSessionID session_id)
{
	MONO_ENTER_GC_UNSAFE;
	ep_disable (wasm_to_ep_session_id (session_id));
	MONO_EXIT_GC_UNSAFE;
	return TRUE;
}

// JS callback to invoke on the main thread early during runtime initialization once eventpipe is functional but before too much of the rest of the runtime is loaded.
extern void mono_wasm_event_pipe_early_startup_callback (void);


static void
mono_wasm_event_pipe_init (void)
{
	ep_init ();
	mono_wasm_event_pipe_early_startup_callback ();
}

#endif /* HOST_WASM && !HOST_WASI */
