/*
 * opcodes.c: CIL instruction information
 *
 * Author:
 *   Paolo Molaro (lupus@ximian.com)
 *
 * (C) 2002 Ximian, Inc.
 */
#include <mono/metadata/opcodes.h>
#include <stddef.h> /* for NULL */

#define MONO_PREFIX1_OFFSET MONO_CEE_ARGLIST
#define MONO_CUSTOM_PREFIX_OFFSET MONO_CEE_MONO_ICALL

#define OPDEF(a,b,c,d,e,f,g,h,i,j) \
	{ Mono ## e, MONO_FLOW_ ## j, MONO_ ## a },

const MonoOpcode
mono_opcodes [MONO_CEE_LAST + 1] = {
#include "mono/cil/opcode.def"
	{0}
};

#undef OPDEF

#define MSGSTRFIELD(line) MSGSTRFIELD1(line)
#define MSGSTRFIELD1(line) str##line
static const struct msgstr_t {
#define OPDEF(a,b,c,d,e,f,g,h,i,j) char MSGSTRFIELD(__LINE__) [sizeof (b)];
#include "mono/cil/opcode.def"
#undef OPDEF
} opstr = {
#define OPDEF(a,b,c,d,e,f,g,h,i,j) b,
#include "mono/cil/opcode.def"
#undef OPDEF
};
static const int opidx [] = {
#define OPDEF(a,b,c,d,e,f,g,h,i,j) [MONO_ ## a] = offsetof (struct msgstr_t, MSGSTRFIELD(__LINE__)),
#include "mono/cil/opcode.def"
#undef OPDEF
};

const char*
mono_opcode_name (int opcode)
{
	return (const char*)&opstr + opidx [opcode];
}

MonoOpcodeEnum
mono_opcode_value (const guint8 **ip, const guint8 *end)
{
	MonoOpcodeEnum res;
	const guint8 *p = *ip;

	if (p >= end)
		return -1;
	if (*p == 0xfe) {
		++p;
		if (p >= end)
			return -1;
		res = *p + MONO_PREFIX1_OFFSET;
	} else if (*p == MONO_CUSTOM_PREFIX) {
		++p;
		if (p >= end)
			return -1;
		res = *p + MONO_CUSTOM_PREFIX_OFFSET;
	} else {
		res = *p;
	}
	*ip = p;
	return res;
}

