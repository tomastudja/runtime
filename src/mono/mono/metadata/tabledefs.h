/*
 * tabledefs.h: This file contains the various definitions for constants
 * found on the metadata tables
 *
 * Author:
 *   Miguel de Icaza (miguel@ximian.com)
 *
 * (C) 2001 Ximian, Inc.
 *
 * From the ECMA documentation
 */
 
#ifndef _MONO_METADATA_TABLEDEFS_H_
#define _MONO_METADATA_TABLEDEFS_H_

/*
 * 22.1.1  Values for AssemblyHashAlgorithm 
 */

enum {
	ASSEMBLY_HASH_NONE,
	ASSEMBLY_HASH_MD5  = 0x8003,
	ASSEMBLY_HASH_SHA1 = 0x8004
};

/*
 * 22.1.4 Flags for Event.EventAttributes
 */

enum {
	EVENT_SPECIALNAME   = 0x0200,
	EVENT_RTSPECIALNAME = 0x0400
};

/*
 * 22.1.6 Flags for FileAttributes
 */

enum {
	FILE_CONTAINS_METADATA      = 0,
	FILE_CONTAINS_NO_METADATA   = 1
};

/* keep in synch with System.Security.Permissions.SecurityAction enum 
   (except for the special non-CAS cases) */
enum {
	SECURITY_ACTION_DEMAND        = 2,
	SECURITY_ACTION_ASSERT        = 3,
	SECURITY_ACTION_DENY          = 4,
	SECURITY_ACTION_PERMITONLY    = 5,
	SECURITY_ACTION_LINKDEMAND    = 6,
	SECURITY_ACTION_INHERITDEMAND = 7,
	SECURITY_ACTION_REQMIN        = 8,
	SECURITY_ACTION_REQOPT        = 9,
	SECURITY_ACTION_REQREFUSE     = 10,
	/* Special cases (non CAS permissions) */
	SECURITY_ACTION_NONCASDEMAND        = 13,
	SECURITY_ACTION_NONCASLINKDEMAND    = 14,
	SECURITY_ACTION_NONCASINHERITANCE   = 15,
	/* Fx 2.0 actions (for both CAS and non-CAS permissions) */
	SECURITY_ACTION_LINKDEMANDCHOICE    = 16,
	SECURITY_ACTION_INHERITDEMANDCHOICE = 17,
	SECURITY_ACTION_DEMANDCHOICE        = 18
};

/*
 * Field Attributes (21.1.5).
 */

#define FIELD_ATTRIBUTE_FIELD_ACCESS_MASK     0x0007
#define FIELD_ATTRIBUTE_COMPILER_CONTROLLED   0x0000
#define FIELD_ATTRIBUTE_PRIVATE               0x0001
#define FIELD_ATTRIBUTE_FAM_AND_ASSEM         0x0002
#define FIELD_ATTRIBUTE_ASSEMBLY              0x0003
#define FIELD_ATTRIBUTE_FAMILY                0x0004
#define FIELD_ATTRIBUTE_FAM_OR_ASSEM          0x0005
#define FIELD_ATTRIBUTE_PUBLIC                0x0006

#define FIELD_ATTRIBUTE_STATIC                0x0010
#define FIELD_ATTRIBUTE_INIT_ONLY             0x0020
#define FIELD_ATTRIBUTE_LITERAL               0x0040
#define FIELD_ATTRIBUTE_NOT_SERIALIZED        0x0080
#define FIELD_ATTRIBUTE_SPECIAL_NAME          0x0200
#define FIELD_ATTRIBUTE_PINVOKE_IMPL          0x2000

/* For runtime use only */
#define FIELD_ATTRIBUTE_RESERVED_MASK         0x9500
#define FIELD_ATTRIBUTE_RT_SPECIAL_NAME       0x0400
#define FIELD_ATTRIBUTE_HAS_FIELD_MARSHAL     0x1000
#define FIELD_ATTRIBUTE_HAS_DEFAULT           0x8000
#define FIELD_ATTRIBUTE_HAS_FIELD_RVA         0x0100

/*
 * Type Attributes (21.1.13).
 */
#define TYPE_ATTRIBUTE_VISIBILITY_MASK       0x00000007
#define TYPE_ATTRIBUTE_NOT_PUBLIC            0x00000000
#define TYPE_ATTRIBUTE_PUBLIC                0x00000001
#define TYPE_ATTRIBUTE_NESTED_PUBLIC         0x00000002
#define TYPE_ATTRIBUTE_NESTED_PRIVATE        0x00000003
#define TYPE_ATTRIBUTE_NESTED_FAMILY         0x00000004
#define TYPE_ATTRIBUTE_NESTED_ASSEMBLY       0x00000005
#define TYPE_ATTRIBUTE_NESTED_FAM_AND_ASSEM  0x00000006
#define TYPE_ATTRIBUTE_NESTED_FAM_OR_ASSEM   0x00000007

#define TYPE_ATTRIBUTE_LAYOUT_MASK           0x00000018
#define TYPE_ATTRIBUTE_AUTO_LAYOUT           0x00000000
#define TYPE_ATTRIBUTE_SEQUENTIAL_LAYOUT     0x00000008
#define TYPE_ATTRIBUTE_EXPLICIT_LAYOUT       0x00000010

#define TYPE_ATTRIBUTE_CLASS_SEMANTIC_MASK   0x00000020
#define TYPE_ATTRIBUTE_CLASS                 0x00000000
#define TYPE_ATTRIBUTE_INTERFACE             0x00000020

#define TYPE_ATTRIBUTE_ABSTRACT              0x00000080
#define TYPE_ATTRIBUTE_SEALED                0x00000100
#define TYPE_ATTRIBUTE_SPECIAL_NAME          0x00000400

#define TYPE_ATTRIBUTE_IMPORT                0x00001000
#define TYPE_ATTRIBUTE_SERIALIZABLE          0x00002000

#define TYPE_ATTRIBUTE_STRING_FORMAT_MASK    0x00030000
#define TYPE_ATTRIBUTE_ANSI_CLASS            0x00000000
#define TYPE_ATTRIBUTE_UNICODE_CLASS         0x00010000
#define TYPE_ATTRIBUTE_AUTO_CLASS            0x00020000

#define TYPE_ATTRIBUTE_BEFORE_FIELD_INIT     0x00100000

#define TYPE_ATTRIBUTE_RESERVED_MASK         0x00040800
#define TYPE_ATTRIBUTE_RT_SPECIAL_NAME       0x00000800
#define TYPE_ATTRIBUTE_HAS_SECURITY          0x00040000

/*
 * Method Attributes (22.1.9)
 */

#define METHOD_IMPL_ATTRIBUTE_CODE_TYPE_MASK       0x0003
#define METHOD_IMPL_ATTRIBUTE_IL                   0x0000
#define METHOD_IMPL_ATTRIBUTE_NATIVE               0x0001
#define METHOD_IMPL_ATTRIBUTE_OPTIL                0x0002
#define METHOD_IMPL_ATTRIBUTE_RUNTIME              0x0003

#define METHOD_IMPL_ATTRIBUTE_MANAGED_MASK         0x0004
#define METHOD_IMPL_ATTRIBUTE_UNMANAGED            0x0004
#define METHOD_IMPL_ATTRIBUTE_MANAGED              0x0000

#define METHOD_IMPL_ATTRIBUTE_FORWARD_REF          0x0010
#define METHOD_IMPL_ATTRIBUTE_PRESERVE_SIG         0x0080
#define METHOD_IMPL_ATTRIBUTE_INTERNAL_CALL        0x1000
#define METHOD_IMPL_ATTRIBUTE_SYNCHRONIZED         0x0020
#define METHOD_IMPL_ATTRIBUTE_NOINLINING           0x0008
#define METHOD_IMPL_ATTRIBUTE_MAX_METHOD_IMPL_VAL  0xffff

#define METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK        0x0007
#define METHOD_ATTRIBUTE_COMPILER_CONTROLLED       0x0000
#define METHOD_ATTRIBUTE_PRIVATE                   0x0001
#define METHOD_ATTRIBUTE_FAM_AND_ASSEM             0x0002
#define METHOD_ATTRIBUTE_ASSEM                     0x0003
#define METHOD_ATTRIBUTE_FAMILY                    0x0004
#define METHOD_ATTRIBUTE_FAM_OR_ASSEM              0x0005
#define METHOD_ATTRIBUTE_PUBLIC                    0x0006

#define METHOD_ATTRIBUTE_STATIC                    0x0010
#define METHOD_ATTRIBUTE_FINAL                     0x0020
#define METHOD_ATTRIBUTE_VIRTUAL                   0x0040
#define METHOD_ATTRIBUTE_HIDE_BY_SIG               0x0080

#define METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK        0x0100
#define METHOD_ATTRIBUTE_REUSE_SLOT                0x0000
#define METHOD_ATTRIBUTE_NEW_SLOT                  0x0100

#define METHOD_ATTRIBUTE_ABSTRACT                  0x0400
#define METHOD_ATTRIBUTE_SPECIAL_NAME              0x0800

#define METHOD_ATTRIBUTE_PINVOKE_IMPL              0x2000
#define METHOD_ATTRIBUTE_UNMANAGED_EXPORT          0x0008

/*
 * For runtime use only
 */
#define METHOD_ATTRIBUTE_RESERVED_MASK             0xd000
#define METHOD_ATTRIBUTE_RT_SPECIAL_NAME           0x1000
#define METHOD_ATTRIBUTE_HAS_SECURITY              0x4000
#define METHOD_ATTRIBUTE_REQUIRE_SEC_OBJECT        0x8000


/*
 * Method Semantics ([MethodSemanticAttributes]) 22.1.10
 */

#define METHOD_SEMANTIC_SETTER    0x0001
#define METHOD_SEMANTIC_GETTER    0x0002
#define METHOD_SEMANTIC_OTHER     0x0004
#define METHOD_SEMANTIC_ADD_ON    0x0008
#define METHOD_SEMANTIC_REMOVE_ON 0x0010
#define METHOD_SEMANTIC_FIRE      0x0020

/*
 * Flags for Params (22.1.12)
 */
#define PARAM_ATTRIBUTE_IN                 0x0001
#define PARAM_ATTRIBUTE_OUT                0x0002
#define PARAM_ATTRIBUTE_OPTIONAL           0x0010
#define PARAM_ATTRIBUTE_RESERVED_MASK      0xf000
#define PARAM_ATTRIBUTE_HAS_DEFAULT        0x1000
#define PARAM_ATTRIBUTE_HAS_FIELD_MARSHAL  0x2000
#define PARAM_ATTRIBUTE_UNUSED             0xcfe0

/*
 * 22.1.12 PropertyAttributes
 */
#define PROPERTY_ATTRIBUTE_SPECIAL_NAME    0x0200
#define PROPERTY_ATTRIBUTE_RESERVED_MASK   0xf400
#define PROPERTY_ATTRIBUTE_RT_SPECIAL_NAME 0x0400
#define PROPERTY_ATTRIBUTE_HAS_DEFAULT     0x1000
#define PROPERTY_ATTRIBUTE_UNUSED          0xe9ff

/*
 * 22.1.7 Flags for ImplMap [PInvokeAttributes]
 */
#define PINVOKE_ATTRIBUTE_NO_MANGLE           0x0001
#define PINVOKE_ATTRIBUTE_CHAR_SET_MASK       0x0006
#define PINVOKE_ATTRIBUTE_CHAR_SET_NOT_SPEC   0x0000
#define PINVOKE_ATTRIBUTE_CHAR_SET_ANSI       0x0002
#define PINVOKE_ATTRIBUTE_CHAR_SET_UNICODE    0x0004
#define PINVOKE_ATTRIBUTE_CHAR_SET_AUTO       0x0006
#define PINVOKE_ATTRIBUTE_BEST_FIT_ENABLED    0x0010
#define PINVOKE_ATTRIBUTE_BEST_FIT_DISABLED   0x0020
#define PINVOKE_ATTRIBUTE_BEST_FIT_MASK       0x0030
#define PINVOKE_ATTRIBUTE_SUPPORTS_LAST_ERROR 0x0040
#define PINVOKE_ATTRIBUTE_CALL_CONV_MASK      0x0700
#define PINVOKE_ATTRIBUTE_CALL_CONV_WINAPI    0x0100
#define PINVOKE_ATTRIBUTE_CALL_CONV_CDECL     0x0200
#define PINVOKE_ATTRIBUTE_CALL_CONV_STDCALL   0x0300
#define PINVOKE_ATTRIBUTE_CALL_CONV_THISCALL  0x0400
#define PINVOKE_ATTRIBUTE_CALL_CONV_FASTCALL  0x0500
#define PINVOKE_ATTRIBUTE_THROW_ON_UNMAPPABLE_ENABLED    0x1000
#define PINVOKE_ATTRIBUTE_THROW_ON_UNMAPPABLE_DISABLED   0x2000
#define PINVOKE_ATTRIBUTE_THROW_ON_UNMAPPABLE_MASK       0x3000
#define PINVOKE_ATTRIBUTE_BEST_FIT_MASK       0x0030
#define PINVOKE_ATTRIBUTE_CALL_CONV_GENERIC   0x0010
#define PINVOKE_ATTRIBUTE_CALL_CONV_GENERICINST 0x000a

/**
 * 21.5 AssemblyRefs
 */
#define ASSEMBLYREF_FULL_PUBLIC_KEY_FLAG      0x00000001
#endif
