#ifndef __MONO_DEBUGGER_PROTOCOL_H__
#define __MONO_DEBUGGER_PROTOCOL_H__

#include <stdint.h>

#define HEADER_LENGTH 11
#define REPLY_PACKET 0x80

/* 
 * Wire Protocol definitions
 */

#define MAJOR_VERSION 2
#define MINOR_VERSION 60

typedef enum {
	MDBGPROT_CMD_COMPOSITE = 100
} MdbgProtCmdComposite;

typedef enum {
	MDBGPROT_CMD_VM_VERSION = 1,
	MDBGPROT_CMD_VM_ALL_THREADS = 2,
	MDBGPROT_CMD_VM_SUSPEND = 3,
	MDBGPROT_CMD_VM_RESUME = 4,
	MDBGPROT_CMD_VM_EXIT = 5,
	MDBGPROT_CMD_VM_DISPOSE = 6,
	MDBGPROT_CMD_VM_INVOKE_METHOD = 7,
	MDBGPROT_CMD_VM_SET_PROTOCOL_VERSION = 8,
	MDBGPROT_CMD_VM_ABORT_INVOKE = 9,
	MDBGPROT_CMD_VM_SET_KEEPALIVE = 10,
	MDBGPROT_CMD_VM_GET_TYPES_FOR_SOURCE_FILE = 11,
	MDBGPROT_CMD_VM_GET_TYPES = 12,
	MDBGPROT_CMD_VM_INVOKE_METHODS = 13,
	MDBGPROT_CMD_VM_START_BUFFERING = 14,
	MDBGPROT_CMD_VM_STOP_BUFFERING = 15,
	MDBGPROT_CMD_VM_READ_MEMORY = 16,
	MDBGPROT_CMD_VM_WRITE_MEMORY = 17,
	MDBGPROT_CMD_GET_ASSEMBLY_BY_NAME = 18,
	MDBGPROT_CMD_GET_MODULE_BY_GUID = 19
} MdbgProtCmdVM;

typedef enum {
	MDBGPROT_CMD_SET_VM = 1,
	MDBGPROT_CMD_SET_OBJECT_REF = 9,
	MDBGPROT_CMD_SET_STRING_REF = 10,
	MDBGPROT_CMD_SET_THREAD = 11,
	MDBGPROT_CMD_SET_ARRAY_REF = 13,
	MDBGPROT_CMD_SET_EVENT_REQUEST = 15,
	MDBGPROT_CMD_SET_STACK_FRAME = 16,
	MDBGPROT_CMD_SET_APPDOMAIN = 20,
	MDBGPROT_CMD_SET_ASSEMBLY = 21,
	MDBGPROT_CMD_SET_METHOD = 22,
	MDBGPROT_CMD_SET_TYPE = 23,
	MDBGPROT_CMD_SET_MODULE = 24,
	MDBGPROT_CMD_SET_FIELD = 25,
	MDBGPROT_CMD_SET_EVENT = 64,
	MDBGPROT_CMD_SET_POINTER = 65
} MdbgProtCommandSet;

typedef enum {
	MDBGPROT_ERR_NONE = 0,
	MDBGPROT_ERR_INVALID_OBJECT = 20,
	MDBGPROT_ERR_INVALID_FIELDID = 25,
	MDBGPROT_ERR_INVALID_FRAMEID = 30,
	MDBGPROT_ERR_NOT_IMPLEMENTED = 100,
	MDBGPROT_ERR_NOT_SUSPENDED = 101,
	MDBGPROT_ERR_INVALID_ARGUMENT = 102,
	MDBGPROT_ERR_UNLOADED = 103,
	MDBGPROT_ERR_NO_INVOCATION = 104,
	MDBGPROT_ERR_ABSENT_INFORMATION = 105,
	MDBGPROT_ERR_NO_SEQ_POINT_AT_IL_OFFSET = 106,
	MDBGPROT_ERR_INVOKE_ABORTED = 107,
	MDBGPROT_ERR_LOADER_ERROR = 200, /*XXX extend the protocol to pass this information down the pipe */
} MdbgProtErrorCode;

typedef enum {
	MDBGPROT_TOKEN_TYPE_STRING = 0,
	MDBGPROT_TOKEN_TYPE_TYPE = 1,
	MDBGPROT_TOKEN_TYPE_FIELD = 2,
	MDBGPROT_TOKEN_TYPE_METHOD = 3,
	MDBGPROT_TOKEN_TYPE_UNKNOWN = 4
} MdbgProtDebuggerTokenType;

typedef enum {
	MDBGPROT_VALUE_TYPE_ID_NULL = 0xf0,
	MDBGPROT_VALUE_TYPE_ID_TYPE = 0xf1,
	MDBGPROT_VALUE_TYPE_ID_PARENT_VTYPE = 0xf2,
	MDBGPROT_VALUE_TYPE_ID_FIXED_ARRAY = 0xf3
} MdbgProtValueTypeId;

typedef enum {
	MDBGPROT_FRAME_FLAG_DEBUGGER_INVOKE = 1,
	MDBGPROT_FRAME_FLAG_NATIVE_TRANSITION = 2
} MdbgProtStackFrameFlags;

typedef enum {
	MDBGPROT_INVOKE_FLAG_DISABLE_BREAKPOINTS = 1,
	MDBGPROT_INVOKE_FLAG_SINGLE_THREADED = 2,
	MDBGPROT_INVOKE_FLAG_RETURN_OUT_THIS = 4,
	MDBGPROT_INVOKE_FLAG_RETURN_OUT_ARGS = 8,
	MDBGPROT_INVOKE_FLAG_VIRTUAL = 16
} MdbgProtInvokeFlags;

typedef enum {
	BINDING_FLAGS_IGNORE_CASE = 0x70000000,
} MdbgProtBindingFlagsExtensions;

typedef enum {
	MDBGPROT_CMD_THREAD_GET_FRAME_INFO = 1,
	MDBGPROT_CMD_THREAD_GET_NAME = 2,
	MDBGPROT_CMD_THREAD_GET_STATE = 3,
	MDBGPROT_CMD_THREAD_GET_INFO = 4,
	MDBGPROT_CMD_THREAD_GET_ID = 5,
	MDBGPROT_CMD_THREAD_GET_TID = 6,
	MDBGPROT_CMD_THREAD_SET_IP = 7,
	MDBGPROT_CMD_THREAD_ELAPSED_TIME = 8,
	MDBGPROT_CMD_THREAD_GET_APPDOMAIN = 9,
	MDBGPROT_CMD_THREAD_GET_CONTEXT = 10,
	MDBGPROT_CMD_THREAD_SET_CONTEXT = 11
} MdbgProtCmdThread;

typedef enum {
	MDBGPROT_CMD_APPDOMAIN_GET_ROOT_DOMAIN = 1,
	MDBGPROT_CMD_APPDOMAIN_GET_FRIENDLY_NAME = 2,
	MDBGPROT_CMD_APPDOMAIN_GET_ASSEMBLIES = 3,
	MDBGPROT_CMD_APPDOMAIN_GET_ENTRY_ASSEMBLY = 4,
	MDBGPROT_CMD_APPDOMAIN_CREATE_STRING = 5,
	MDBGPROT_CMD_APPDOMAIN_GET_CORLIB = 6,
	MDBGPROT_CMD_APPDOMAIN_CREATE_BOXED_VALUE = 7,
	MDBGPROT_CMD_APPDOMAIN_CREATE_BYTE_ARRAY = 8,
} MdbgProtCmdAppDomain;

typedef enum {
	MDBGPROT_CMD_ASSEMBLY_GET_LOCATION = 1,
	MDBGPROT_CMD_ASSEMBLY_GET_ENTRY_POINT = 2,
	MDBGPROT_CMD_ASSEMBLY_GET_MANIFEST_MODULE = 3,
	MDBGPROT_CMD_ASSEMBLY_GET_OBJECT = 4,
	MDBGPROT_CMD_ASSEMBLY_GET_TYPE = 5,
	MDBGPROT_CMD_ASSEMBLY_GET_NAME = 6,
	MDBGPROT_CMD_ASSEMBLY_GET_DOMAIN = 7,
	MDBGPROT_CMD_ASSEMBLY_GET_METADATA_BLOB = 8,
	MDBGPROT_CMD_ASSEMBLY_GET_IS_DYNAMIC = 9,
	MDBGPROT_CMD_ASSEMBLY_GET_PDB_BLOB = 10,
	MDBGPROT_CMD_ASSEMBLY_GET_TYPE_FROM_TOKEN = 11,
	MDBGPROT_CMD_ASSEMBLY_GET_METHOD_FROM_TOKEN = 12,
	MDBGPROT_CMD_ASSEMBLY_HAS_DEBUG_INFO = 13,
	MDBGPROT_CMD_ASSEMBLY_GET_CATTRS = 14,
	MDBGPROT_CMD_ASSEMBLY_GET_CUSTOM_ATTRIBUTES = 15,
	MDBGPROT_CMD_ASSEMBLY_GET_PEIMAGE_ADDRESS = 16,
} MdbgProtCmdAssembly;

typedef enum {
	MDBGPROT_CMD_MODULE_GET_INFO = 1,
	MDBGPROT_CMD_MODULE_APPLY_CHANGES = 2
} MdbgProtCmdModule;

typedef enum {
	MDBGPROT_CMD_FIELD_GET_INFO = 1,
} MdbgProtCmdField;

typedef enum {
	MDBGPROT_CMD_PROPERTY_GET_INFO = 1,
} MdbgProtCmdProperty;

typedef enum {
	MDBGPROT_CMD_METHOD_GET_NAME = 1,
	MDBGPROT_CMD_METHOD_GET_DECLARING_TYPE = 2,
	MDBGPROT_CMD_METHOD_GET_DEBUG_INFO = 3,
	MDBGPROT_CMD_METHOD_GET_PARAM_INFO = 4,
	MDBGPROT_CMD_METHOD_GET_LOCALS_INFO = 5,
	MDBGPROT_CMD_METHOD_GET_INFO = 6,
	MDBGPROT_CMD_METHOD_GET_BODY = 7,
	MDBGPROT_CMD_METHOD_RESOLVE_TOKEN = 8,
	MDBGPROT_CMD_METHOD_GET_CATTRS = 9,
	MDBGPROT_CMD_METHOD_MAKE_GENERIC_METHOD = 10,
	MDBGPROT_CMD_METHOD_TOKEN = 11,
	MDBGPROT_CMD_METHOD_ASSEMBLY = 12,
	MDBGPROT_CMD_METHOD_GET_CLASS_TOKEN = 13,
	MDBGPROT_CMD_METHOD_HAS_ASYNC_DEBUG_INFO = 14,
	MDBGPROT_CMD_METHOD_GET_NAME_FULL = 15
} MdbgProtCmdMethod;

typedef enum {
	MDBGPROT_CMD_TYPE_GET_INFO = 1,
	MDBGPROT_CMD_TYPE_GET_METHODS = 2,
	MDBGPROT_CMD_TYPE_GET_FIELDS = 3,
	MDBGPROT_CMD_TYPE_GET_VALUES = 4,
	MDBGPROT_CMD_TYPE_GET_OBJECT = 5,
	MDBGPROT_CMD_TYPE_GET_SOURCE_FILES = 6,
	MDBGPROT_CMD_TYPE_SET_VALUES = 7,
	MDBGPROT_CMD_TYPE_IS_ASSIGNABLE_FROM = 8,
	MDBGPROT_CMD_TYPE_GET_PROPERTIES = 9,
	MDBGPROT_CMD_TYPE_GET_CATTRS = 10,
	MDBGPROT_CMD_TYPE_GET_FIELD_CATTRS = 11,
	MDBGPROT_CMD_TYPE_GET_PROPERTY_CATTRS = 12,
	MDBGPROT_CMD_TYPE_GET_SOURCE_FILES_2 = 13,
	MDBGPROT_CMD_TYPE_GET_VALUES_2 = 14,
	MDBGPROT_CMD_TYPE_GET_METHODS_BY_NAME_FLAGS = 15,
	MDBGPROT_CMD_TYPE_GET_INTERFACES = 16,
	MDBGPROT_CMD_TYPE_GET_INTERFACE_MAP = 17,
	MDBGPROT_CMD_TYPE_IS_INITIALIZED = 18,
	MDBGPROT_CMD_TYPE_CREATE_INSTANCE = 19,
	MDBGPROT_CMD_TYPE_GET_VALUE_SIZE = 20,
	MDBGPROT_CMD_TYPE_GET_VALUES_ICORDBG = 21,
	MDBGPROT_CMD_TYPE_GET_PARENTS = 22,
	MDBGPROT_CMD_TYPE_INITIALIZE = 23
} MdbgProtCmdType;

typedef enum {
	MDBGPROT_CMD_STACK_FRAME_GET_VALUES = 1,
	MDBGPROT_CMD_STACK_FRAME_GET_THIS = 2,
	MDBGPROT_CMD_STACK_FRAME_SET_VALUES = 3,
	MDBGPROT_CMD_STACK_FRAME_GET_DOMAIN = 4,
	MDBGPROT_CMD_STACK_FRAME_SET_THIS = 5,
	MDBGPROT_CMD_STACK_FRAME_GET_ARGUMENT = 6,
	MDBGPROT_CMD_STACK_FRAME_GET_ARGUMENTS = 7
} MdbgProtCmdStackFrame;

typedef enum {
	MDBGPROT_CMD_ARRAY_REF_GET_LENGTH = 1,
	MDBGPROT_CMD_ARRAY_REF_GET_VALUES = 2,
	MDBGPROT_CMD_ARRAY_REF_SET_VALUES = 3,
	MDBGPROT_CMD_ARRAY_REF_GET_TYPE = 4
} MdbgProtCmdArray;

typedef enum {
	MDBGPROT_CMD_STRING_REF_GET_VALUE = 1,
	MDBGPROT_CMD_STRING_REF_GET_LENGTH = 2,
	MDBGPROT_CMD_STRING_REF_GET_CHARS = 3
} MdbgProtCmdString;

typedef enum {
	MDBGPROT_CMD_POINTER_GET_VALUE = 1
} MdbgProtCmdPointer;

typedef enum {
	MDBGPROT_CMD_OBJECT_REF_GET_TYPE = 1,
	MDBGPROT_CMD_OBJECT_REF_GET_VALUES = 2,
	MDBGPROT_CMD_OBJECT_REF_IS_COLLECTED = 3,
	MDBGPROT_CMD_OBJECT_REF_GET_ADDRESS = 4,
	MDBGPROT_CMD_OBJECT_REF_GET_DOMAIN = 5,
	MDBGPROT_CMD_OBJECT_REF_SET_VALUES = 6,
	MDBGPROT_CMD_OBJECT_REF_GET_INFO = 7,
	MDBGPROT_CMD_OBJECT_REF_GET_VALUES_ICORDBG = 8,
	MDBGPROT_CMD_OBJECT_REF_DELEGATE_GET_METHOD = 9,
	MDBGPROT_CMD_OBJECT_IS_DELEGATE = 10
} MdbgProtCmdObject;

typedef enum {
	MDBGPROT_SUSPEND_POLICY_NONE = 0,
	MDBGPROT_SUSPEND_POLICY_EVENT_THREAD = 1,
	MDBGPROT_SUSPEND_POLICY_ALL = 2
} MdbgProtSuspendPolicy;

typedef enum {
	MDBGPROT_CMD_EVENT_REQUEST_SET = 1,
	MDBGPROT_CMD_EVENT_REQUEST_CLEAR = 2,
	MDBGPROT_CMD_EVENT_REQUEST_CLEAR_ALL_BREAKPOINTS = 3
} MdbgProtCmdEvent;

typedef struct {
	uint8_t *buf, *p, *end;
} MdbgProtBuffer;

typedef struct {
	int len;
	int id;
	int flags;
	int command_set;
	int command;
	int error;
	int error_2;
} MdbgProtHeader;

typedef struct ReplyPacket {
	int id;
	int error;
	MdbgProtBuffer *data;
} MdbgProtReplyPacket;

typedef enum {
	MDBGPROT_EVENT_KIND_VM_START = 0,
	MDBGPROT_EVENT_KIND_VM_DEATH = 1,
	MDBGPROT_EVENT_KIND_THREAD_START = 2,
	MDBGPROT_EVENT_KIND_THREAD_DEATH = 3,
	MDBGPROT_EVENT_KIND_APPDOMAIN_CREATE = 4,
	MDBGPROT_EVENT_KIND_APPDOMAIN_UNLOAD = 5,
	MDBGPROT_EVENT_KIND_METHOD_ENTRY = 6,
	MDBGPROT_EVENT_KIND_METHOD_EXIT = 7,
	MDBGPROT_EVENT_KIND_ASSEMBLY_LOAD = 8,
	MDBGPROT_EVENT_KIND_ASSEMBLY_UNLOAD = 9,
	MDBGPROT_EVENT_KIND_BREAKPOINT = 10,
	MDBGPROT_EVENT_KIND_STEP = 11,
	MDBGPROT_EVENT_KIND_TYPE_LOAD = 12,
	MDBGPROT_EVENT_KIND_EXCEPTION = 13,
	MDBGPROT_EVENT_KIND_KEEPALIVE = 14,
	MDBGPROT_EVENT_KIND_USER_BREAK = 15,
	MDBGPROT_EVENT_KIND_USER_LOG = 16,
	MDBGPROT_EVENT_KIND_CRASH = 17,
	MDBGPROT_EVENT_KIND_ENC_UPDATE = 18,
	MDBGPROT_EVENT_KIND_METHOD_UPDATE = 19,
} MdbgProtEventKind;

typedef enum {
	MDBGPROT_MOD_KIND_COUNT = 1,
	MDBGPROT_MOD_KIND_THREAD_ONLY = 3,
	MDBGPROT_MOD_KIND_LOCATION_ONLY = 7,
	MDBGPROT_MOD_KIND_EXCEPTION_ONLY = 8,
	MDBGPROT_MOD_KIND_STEP = 10,
	MDBGPROT_MOD_KIND_ASSEMBLY_ONLY = 11,
	MDBGPROT_MOD_KIND_SOURCE_FILE_ONLY = 12,
	MDBGPROT_MOD_KIND_TYPE_NAME_ONLY = 13,
	MDBGPROT_MOD_KIND_NONE = 14
} MdbgProtModifierKind;

typedef enum {
	MDBGPROT_STEP_DEPTH_INTO = 0,
	MDBGPROT_STEP_DEPTH_OVER = 1,
	MDBGPROT_STEP_DEPTH_OUT = 2
} MdbgProtStepDepth;

typedef enum {
	MDBGPROT_STEP_SIZE_MIN = 0,
	MDBGPROT_STEP_SIZE_LINE = 1
} MdbgProtStepSize;

typedef enum {
	MDBGPROT_STEP_FILTER_NONE = 0,
	MDBGPROT_STEP_FILTER_STATIC_CTOR = 1,
	MDBGPROT_STEP_FILTER_DEBUGGER_HIDDEN = 2,
	MDBGPROT_STEP_FILTER_DEBUGGER_STEP_THROUGH = 4,
	MDBGPROT_STEP_FILTER_DEBUGGER_NON_USER_CODE = 8
} MdbgProtStepFilter;

/*
 * IDS
 */

typedef enum {
	ID_ASSEMBLY = 0,
	ID_MODULE = 1,
	ID_TYPE = 2,
	ID_METHOD = 3,
	ID_FIELD = 4,
	ID_DOMAIN = 5,
	ID_PROPERTY = 6,
	ID_PARAMETER = 7,
	ID_NUM
} IdType;

int m_dbgprot_buffer_add_command_header (MdbgProtBuffer *recvbuf, int cmd_set, int cmd, MdbgProtBuffer *out);
void m_dbgprot_decode_command_header (MdbgProtBuffer *recvbuf, MdbgProtHeader *header);

/*
 * Functions to decode protocol data
 */

int m_dbgprot_decode_byte (uint8_t *buf, uint8_t **endbuf, uint8_t *limit);
int m_dbgprot_decode_int (uint8_t *buf, uint8_t **endbuf, uint8_t *limit);
int64_t m_dbgprot_decode_long (uint8_t *buf, uint8_t **endbuf, uint8_t *limit);
int m_dbgprot_decode_id (uint8_t *buf, uint8_t **endbuf, uint8_t *limit);
char* m_dbgprot_decode_string (uint8_t *buf, uint8_t **endbuf, uint8_t *limit);
char* m_dbgprot_decode_string_with_len(uint8_t* buf, uint8_t** endbuf, uint8_t* limit, int *len);
uint8_t* m_dbgprot_decode_byte_array(uint8_t *buf, uint8_t **endbuf, uint8_t *limit, int32_t *len);

/*
 * Functions to encode protocol data
 */

void m_dbgprot_buffer_init (MdbgProtBuffer *buf, uint32_t size);
uint32_t m_dbgprot_buffer_len (MdbgProtBuffer *buf);
void m_dbgprot_buffer_make_room (MdbgProtBuffer *buf, uint32_t size);
void m_dbgprot_buffer_add_byte (MdbgProtBuffer *buf, uint8_t val);
void m_dbgprot_buffer_add_short (MdbgProtBuffer *buf, uint32_t val);
void m_dbgprot_buffer_add_int (MdbgProtBuffer *buf, uint32_t val);
void m_dbgprot_buffer_add_long (MdbgProtBuffer *buf, uint64_t l);
void m_dbgprot_buffer_add_id (MdbgProtBuffer *buf, uint32_t id);
void m_dbgprot_buffer_add_data (MdbgProtBuffer *buf, uint8_t *data, uint32_t len);
void m_dbgprot_buffer_add_utf16 (MdbgProtBuffer *buf, uint8_t *data, uint32_t len);
void m_dbgprot_buffer_add_string (MdbgProtBuffer *buf, const char *str);
void m_dbgprot_buffer_add_byte_array (MdbgProtBuffer *buf, uint8_t *bytes, uint32_t arr_len);
void m_dbgprot_buffer_add_buffer (MdbgProtBuffer *buf, MdbgProtBuffer *data);
void m_dbgprot_buffer_free (MdbgProtBuffer *buf);

const char* m_dbgprot_event_to_string (MdbgProtEventKind event);

#endif

