/*
 * trampolinesparc.c: JIT trampoline code for Sparc 64
 *
 * Authors:
 *   Mark Crichton (crichton@gimp.org)
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * (C) 2003 Ximian, Inc.
 */

#include <config.h>
#include <glib.h>

#include <mono/arch/sparc/sparc-codegen.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/marshal.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/mono-debug-debugger.h>

#include "mini.h"
#include "mini-sparc.h"

#warning NotReady

/* adapt to mini later... */
#define mono_jit_share_code (1)

/*
 * Address of the Sparc trampoline code.  This is used by the debugger to check
 * whether a method is a trampoline.
 */
guint8 *mono_generic_trampoline_code = NULL;

/*
 * get_unbox_trampoline:
 * @m: method pointer
 * @addr: pointer to native code for @m
 *
 * when value type methods are called through the vtable we need to unbox the
 * this argument. This method returns a pointer to a trampoline which does
 * unboxing before calling the method
 */
static gpointer
get_unbox_trampoline (MonoMethod *m, gpointer addr)
{
	guint8 *code, *start;
	int this_pos = 4;

	if (!m->signature->ret->byref && MONO_TYPE_ISSTRUCT (m->signature->ret))
		this_pos = 8;
	    
	start = code = g_malloc (16);

	//x86_alu_membase_imm (code, X86_ADD, X86_ESP, this_pos, sizeof (MonoObject));
	//x86_jump_code (code, addr);
	g_assert ((code - start) < 16);

	return start;
}

/**
 * x86_magic_trampoline:
 * @eax: saved x86 register 
 * @ecx: saved x86 register 
 * @edx: saved x86 register 
 * @esi: saved x86 register 
 * @edi: saved x86 register 
 * @ebx: saved x86 register
 * @code: pointer into caller code
 * @method: the method to translate
 *
 * This method is called by the trampoline functions for virtual
 * methods. It inspects the caller code to find the address of the
 * vtable slot, then calls the JIT compiler and writes the address
 * of the compiled method back to the vtable. All virtual methods 
 * are called with: x86_call_membase (inst, basereg, disp). We always
 * use 32 bit displacement to ensure that the length of the call 
 * instruction is 6 bytes. We need to get the value of the basereg 
 * and the constant displacement.
 */
static gpointer
x86_magic_trampoline (int eax, int ecx, int edx, int esi, int edi, 
		      int ebx, guint8 *code, MonoMethod *m)
{
	guint8 reg;
	gint32 disp;
	char *o;
	gpointer addr;

	addr = mono_compile_method (m);
	g_assert (addr);

	/* the method was jumped to */
	if (!code)
		return addr;

	/* go to the start of the call instruction
	 *
	 * address_byte = (m << 6) | (o << 3) | reg
	 * call opcode: 0xff address_byte displacement
	 * 0xff m=1,o=2 imm8
	 * 0xff m=2,o=2 imm32
	 */
	code -= 6;
	if ((code [1] != 0xe8) && (code [3] == 0xff) && ((code [4] & 0x18) == 0x10) && ((code [4] >> 6) == 1)) {
		reg = code [4] & 0x07;
		disp = (signed char)code [5];
	} else {
		if ((code [0] == 0xff) && ((code [1] & 0x18) == 0x10) && ((code [1] >> 6) == 2)) {
			reg = code [1] & 0x07;
			disp = *((gint32*)(code + 2));
		} else if ((code [1] == 0xe8)) {
			*((guint32*)(code + 2)) = (guint)addr - ((guint)code + 1) - 5; 
			return addr;
		} else if ((code [4] == 0xff) && (((code [5] >> 6) & 0x3) == 0) && (((code [5] >> 3) & 0x7) == 2)) {
			/*
			 * This is a interface call: should check the above code can't catch it earlier 
			 * 8b 40 30   mov    0x30(%eax),%eax
			 * ff 10      call   *(%eax)
			 */
			disp = 0;
			reg = code [5] & 0x07;
		} else {
			printf ("Invalid trampoline sequence: %x %x %x %x %x %x %x\n", code [0], code [1], code [2], code [3],
				code [4], code [5], code [6]);
			g_assert_not_reached ();
		}
	}

	o += disp;

	if (m->klass->valuetype) {
		return *((gpointer *)o) = get_unbox_trampoline (m, addr);
	} else {
		return *((gpointer *)o) = addr;
	}
}

static guchar*
create_trampoline_code (int is_jump)
{
	guint8 *buf, *code;
	static guint8* generic_jump_trampoline = NULL;
	
	if (is_jump) {
		if (generic_jump_trampoline)
			return generic_jump_trampoline;
	} else {
		if (mono_generic_trampoline_code)
			return mono_generic_trampoline_code;
	}
	
	code = buf = g_malloc (256);
	/* save caller save regs because we need to do a call */ 
	//x86_push_reg (buf, X86_EDX);
	//x86_push_reg (buf, X86_EAX);
	//x86_push_reg (buf, X86_ECX);

#if 0
	/* save the IP (caller ip) */
	if (is_jump)
		x86_push_imm (buf, 0);
	else
		x86_push_membase (buf, X86_ESP, 16);

	x86_push_reg (buf, X86_EBX);
	x86_push_reg (buf, X86_EDI);
	x86_push_reg (buf, X86_ESI);
	x86_push_reg (buf, X86_EBP);

	/* save method info */
	x86_push_membase (buf, X86_ESP, 32);
	/* get the address of lmf for the current thread */
	x86_call_code (buf, mono_get_lmf_addr);
	/* push lmf */
	x86_push_reg (buf, X86_EAX); 
	/* push *lfm (previous_lmf) */
	x86_push_membase (buf, X86_EAX, 0);
	/* *(lmf) = ESP */
	x86_mov_membase_reg (buf, X86_EAX, 0, X86_ESP, 4);
	/* save LFM end */

	/* push the method info */
	x86_push_membase (buf, X86_ESP, 44);
	/* push the return address onto the stack */
	if (is_jump)
		x86_push_imm (buf, 0);
	else
		x86_push_membase (buf, X86_ESP, 52);

	/* save all register values */
	x86_push_reg (buf, X86_EBX);
	x86_push_reg (buf, X86_EDI);
	x86_push_reg (buf, X86_ESI);
	x86_push_membase (buf, X86_ESP, 64); /* EDX */
	x86_push_membase (buf, X86_ESP, 64); /* ECX */
	x86_push_membase (buf, X86_ESP, 64); /* EAX */

	x86_call_code (buf, x86_magic_trampoline);
	x86_alu_reg_imm (buf, X86_ADD, X86_ESP, 8*4);

	/* restore LMF start */
	/* ebx = previous_lmf */
	x86_pop_reg (buf, X86_EBX);
	/* edi = lmf */
	x86_pop_reg (buf, X86_EDI);
	/* *(lmf) = previous_lmf */
	x86_mov_membase_reg (buf, X86_EDI, 0, X86_EBX, 4);
	/* discard method info */
	x86_pop_reg (buf, X86_ESI);
	/* restore caller saved regs */
	x86_pop_reg (buf, X86_EBP);
	x86_pop_reg (buf, X86_ESI);
	x86_pop_reg (buf, X86_EDI);
	x86_pop_reg (buf, X86_EBX);
	/* discard save IP */
	x86_alu_reg_imm (buf, X86_ADD, X86_ESP, 4);		
	/* restore LMF end */

	x86_alu_reg_imm (buf, X86_ADD, X86_ESP, 16);

	/* call the compiled method */
	x86_jump_reg (buf, X86_EAX);
#endif

	g_assert ((buf - code) <= 256);

	if (is_jump) {
		return generic_jump_trampoline = code;
	} else {
		return mono_generic_trampoline_code = code;
	}
}

#define TRAMPOLINE_SIZE 10

gpointer
mono_arch_create_jump_trampoline (MonoMethod *method)
{
	guint8 *code, *buf, *tramp;

	if (method->iflags & METHOD_IMPL_ATTRIBUTE_SYNCHRONIZED)
		return mono_arch_create_jump_trampoline (mono_marshal_get_synchronized_wrapper (method));

	/* icalls use method->addr */
	if ((method->iflags & METHOD_IMPL_ATTRIBUTE_INTERNAL_CALL) ||
	    (method->flags & METHOD_ATTRIBUTE_PINVOKE_IMPL)) {
		MonoMethod *nm;

		if (!method->addr) {
			if (method->iflags & METHOD_IMPL_ATTRIBUTE_INTERNAL_CALL)
				method->addr = mono_lookup_internal_call (method);
			if (method->flags & METHOD_ATTRIBUTE_PINVOKE_IMPL)
				mono_lookup_pinvoke_call (method);
		}		
#ifdef MONO_USE_EXC_TABLES
		if (mono_method_blittable (method)) {
			return method->addr;
		} else {
#endif
			nm = mono_marshal_get_native_wrapper (method);
			return mono_compile_method (nm);
#ifdef MONO_USE_EXC_TABLES
		}
#endif
	}
	
	tramp = create_trampoline_code (TRUE);

	code = buf = g_malloc (TRAMPOLINE_SIZE);
	//x86_push_imm (buf, method);
	//x86_jump_code (buf, tramp);
	g_assert ((buf - code) <= TRAMPOLINE_SIZE);

	mono_jit_stats.method_trampolines++;

	return code;

}

/**
 * mono_arch_create_jit_trampoline:
 * @method: pointer to the method info
 *
 * Creates a trampoline function for virtual methods. If the created
 * code is called it first starts JIT compilation of method,
 * and then calls the newly created method. I also replaces the
 * corresponding vtable entry (see x86_magic_trampoline).
 * 
 * Returns: a pointer to the newly created code 
 */
gpointer
mono_arch_create_jit_trampoline (MonoMethod *method)
{
	guint8 *code, *buf;

	/* previously created trampoline code */
	if (method->info)
		return method->info;

	if (method->iflags & METHOD_IMPL_ATTRIBUTE_SYNCHRONIZED)
		return mono_arch_create_jit_trampoline (mono_marshal_get_synchronized_wrapper (method));

	create_trampoline_code (FALSE);

	code = buf = g_malloc (TRAMPOLINE_SIZE);
	//x86_push_imm (buf, method);
	//x86_jump_code (buf, mono_generic_trampoline_code);
	g_assert ((buf - code) <= TRAMPOLINE_SIZE);

	/* store trampoline address */
	method->info = code;

	mono_jit_stats.method_trampolines++;

	return code;
}

/*
 * This method is only called when running in the Mono Debugger.
 */
gpointer
mono_debugger_create_notification_function (gpointer *notification_address)
{
	guint8 *ptr, *buf;

	ptr = buf = g_malloc0 (16);
	//x86_breakpoint (buf);
	if (notification_address)
		*notification_address = buf;
	//x86_ret (buf);

	return ptr;
}

