#ifndef __DIAGNOSTICS_PROCESS_PROTOCOL_H__
#define __DIAGNOSTICS_PROCESS_PROTOCOL_H__

#include <config.h>

#ifdef ENABLE_PERFTRACING
#include "ds-rt-config.h"
#include "ds-types.h"
#include "ds-ipc.h"

#undef DS_IMPL_GETTER_SETTER
#ifdef DS_IMPL_PROCESS_PROTOCOL_GETTER_SETTER
#define DS_IMPL_GETTER_SETTER
#endif
#include "ds-getter-setter.h"

/*
* DiagnosticsProcessInfoPayload
*/

// command = 0x0400
#if defined(DS_INLINE_GETTER_SETTER) || defined(DS_IMPL_PROCESS_PROTOCOL_GETTER_SETTER)
struct _DiagnosticsProcessInfoPayload {
#else
struct _DiagnosticsProcessInfoPayload_Internal {
#endif
	// The protocol buffer is defined as:
	// X, Y, Z means encode bytes for X followed by bytes for Y followed by bytes for Z
	// uint = 4 little endian bytes
	// long = 8 little endian bytes
	// GUID = 16 little endian bytes
	// wchar = 2 little endian bytes, UTF16 encoding
	// array<T> = uint length, length # of Ts
	// string = (array<char> where the last char must = 0) or (length = 0)

	// ProcessInfo = long pid, string cmdline, string OS, string arch, GUID runtimeCookie
	uint64_t process_id;
	const ep_char16_t *command_line;
	const ep_char16_t *os;
	const ep_char16_t *arch;
	uint8_t runtime_cookie [EP_ACTIVITY_ID_SIZE];
};

#if !defined(DS_INLINE_GETTER_SETTER) && !defined(DS_IMPL_PROCESS_PROTOCOL_GETTER_SETTER)
struct _DiagnosticsProcessInfoPayload {
	uint8_t _internal [sizeof (struct _DiagnosticsProcessInfoPayload_Internal)];
};
#endif

DiagnosticsProcessInfoPayload *
ds_process_info_payload_init (
	DiagnosticsProcessInfoPayload *payload,
	const ep_char16_t *command_line,
	const ep_char16_t *os,
	const ep_char16_t *arch,
	uint32_t process_id,
	const uint8_t *runtime_cookie);

void
ds_process_info_payload_fini (DiagnosticsProcessInfoPayload *payload);

/*
 * DiagnosticsProcessProtocolHelper.
 */

void
ds_process_protocol_helper_handle_ipc_message (
	DiagnosticsIpcMessage *message,
	DiagnosticsIpcStream *stream);

#endif /* ENABLE_PERFTRACING */
#endif /* __DIAGNOSTICS_PROCESS_PROTOCOL_H__ */
