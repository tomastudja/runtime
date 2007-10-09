/*
 * tramp-arm.c: JIT trampoline code for ARM
 *
 * Authors:
 *   Paolo Molaro (lupus@ximian.com)
 *
 * (C) 2001 Ximian, Inc.
 */

#include <config.h>
#include <glib.h>

#include <mono/metadata/appdomain.h>
#include <mono/metadata/marshal.h>
#include <mono/metadata/tabledefs.h>
#include <mono/arch/arm/arm-codegen.h>

#include "mini.h"
#include "mini-arm.h"

/*
 * mono_arch_get_unbox_trampoline:
 * @m: method pointer
 * @addr: pointer to native code for @m
 *
 * when value type methods are called through the vtable we need to unbox the
 * this argument. This method returns a pointer to a trampoline which does
 * unboxing before calling the method
 */
gpointer
mono_arch_get_unbox_trampoline (MonoMethod *m, gpointer addr)
{
	guint8 *code, *start;
	int this_pos = 0;
	MonoDomain *domain = mono_domain_get ();

	if (!mono_method_signature (m)->ret->byref && MONO_TYPE_ISSTRUCT (mono_method_signature (m)->ret))
		this_pos = 1;

	mono_domain_lock (domain);
	start = code = mono_code_manager_reserve (domain->code_mp, 16);
	mono_domain_unlock (domain);

	ARM_LDR_IMM (code, ARMREG_IP, ARMREG_PC, 4);
	ARM_ADD_REG_IMM8 (code, this_pos, this_pos, sizeof (MonoObject));
	ARM_MOV_REG_REG (code, ARMREG_PC, ARMREG_IP);
	*(guint32*)code = (guint32)addr;
	code += 4;
	mono_arch_flush_icache (start, code - start);
	g_assert ((code - start) <= 16);
	/*g_print ("unbox trampoline at %d for %s:%s\n", this_pos, m->klass->name, m->name);
	g_print ("unbox code is at %p for method at %p\n", start, addr);*/

	return start;
}

void
mono_arch_patch_callsite (guint8 *code_ptr, guint8 *addr)
{
	guint32 *code = (guint32*)code_ptr;

	/* This is the 'bl' or the 'mov pc' instruction */
	--code;
	
	/*
	 * Note that methods are called also with the bl opcode.
	 */
	if ((((*code) >> 25)  & 7) == 5) {
		/*g_print ("direct patching\n");*/
		arm_patch ((char*)code, addr);
		mono_arch_flush_icache ((char*)code, 4);
		return;
	}

	g_assert_not_reached ();
}

void
mono_arch_patch_plt_entry (guint8 *code, guint8 *addr)
{
	g_assert_not_reached ();
}

void
mono_arch_nullify_class_init_trampoline (guint8 *code, gssize *regs)
{
	return;
}

void
mono_arch_nullify_plt_entry (guint8 *code)
{
	g_assert_not_reached ();
}


/* Stack size for trampoline function 
 */
#define STACK (sizeof (MonoLMF))

/* Method-specific trampoline code fragment size */
#define METHOD_TRAMPOLINE_SIZE 64

/* Jump-specific trampoline code fragment size */
#define JUMP_TRAMPOLINE_SIZE   64

#define GEN_TRAMP_SIZE 148

/*
 * Stack frame description when the generic trampoline is called.
 * caller frame
 * ------------------- old sp
 *  MonoLMF
 * ------------------- sp
 */
guchar*
mono_arch_create_trampoline_code (MonoTrampolineType tramp_type)
{
	guint8 *buf, *code = NULL;
	int i, offset;
	guint8 *load_get_lmf_addr, *load_trampoline;
	gpointer *constants;

	/* Now we'll create in 'buf' the ARM trampoline code. This
	 is the trampoline code common to all methods  */
	
	code = buf = mono_global_codeman_reserve (GEN_TRAMP_SIZE);

	/*
	 * At this point r0 has the specific arg and sp points to the saved
	 * regs on the stack (all but PC and SP).
	 */
	ARM_MOV_REG_REG (buf, ARMREG_V1, ARMREG_SP);
	ARM_MOV_REG_REG (buf, ARMREG_V2, ARMREG_R0);
	ARM_MOV_REG_REG (buf, ARMREG_V3, ARMREG_LR);

	/* ok, now we can continue with the MonoLMF setup, mostly untouched 
	 * from emit_prolog in mini-arm.c
	 * This is a sinthetized call to mono_get_lmf_addr ()
	 */
	load_get_lmf_addr = buf;
	buf += 4;
	ARM_MOV_REG_REG (buf, ARMREG_LR, ARMREG_PC);
	ARM_MOV_REG_REG (buf, ARMREG_PC, ARMREG_R0);

	/* we build the MonoLMF structure on the stack - see mini-arm.h
	 * The pointer to the struct is put in r1.
	 * the iregs array is already allocated on the stack by push.
	 */
	ARM_SUB_REG_IMM8 (buf, ARMREG_SP, ARMREG_SP, sizeof (MonoLMF) - sizeof (guint) * 14);
	ARM_ADD_REG_IMM8 (buf, ARMREG_R1, ARMREG_SP, STACK - sizeof (MonoLMF));
	/* r0 is the result from mono_get_lmf_addr () */
	ARM_STR_IMM (buf, ARMREG_R0, ARMREG_R1, G_STRUCT_OFFSET (MonoLMF, lmf_addr));
	/* new_lmf->previous_lmf = *lmf_addr */
	ARM_LDR_IMM (buf, ARMREG_R2, ARMREG_R0, G_STRUCT_OFFSET (MonoLMF, previous_lmf));
	ARM_STR_IMM (buf, ARMREG_R2, ARMREG_R1, G_STRUCT_OFFSET (MonoLMF, previous_lmf));
	/* *(lmf_addr) = r1 */
	ARM_STR_IMM (buf, ARMREG_R1, ARMREG_R0, G_STRUCT_OFFSET (MonoLMF, previous_lmf));
	/* save method info (it's in v2) */
	if ((tramp_type == MONO_TRAMPOLINE_GENERIC) || (tramp_type == MONO_TRAMPOLINE_JUMP))
		ARM_STR_IMM (buf, ARMREG_V2, ARMREG_R1, G_STRUCT_OFFSET (MonoLMF, method));
	ARM_STR_IMM (buf, ARMREG_SP, ARMREG_R1, G_STRUCT_OFFSET (MonoLMF, ebp));
	/* save the IP (caller ip) */
	if (tramp_type == MONO_TRAMPOLINE_JUMP) {
		ARM_MOV_REG_IMM8 (buf, ARMREG_R2, 0);
	} else {
		/* assumes STACK == sizeof (MonoLMF) */
		ARM_LDR_IMM (buf, ARMREG_R2, ARMREG_SP, (G_STRUCT_OFFSET (MonoLMF, iregs) + 13*4));
	}
	ARM_STR_IMM (buf, ARMREG_R2, ARMREG_R1, G_STRUCT_OFFSET (MonoLMF, eip));

	/*
	 * Now we're ready to call xxx_trampoline ().
	 */
	/* Arg 1: the saved registers. It was put in v1 */
	ARM_MOV_REG_REG (buf, ARMREG_R0, ARMREG_V1);

	/* Arg 2: code (next address to the instruction that called us) */
	if (tramp_type == MONO_TRAMPOLINE_JUMP) {
		ARM_MOV_REG_IMM8 (buf, ARMREG_R1, 0);
	} else {
		ARM_MOV_REG_REG (buf, ARMREG_R1, ARMREG_V3);
	}
	
	/* Arg 3: the specific argument, stored in v2
	 */
	ARM_MOV_REG_REG (buf, ARMREG_R2, ARMREG_V2);

	load_trampoline = buf;
	buf += 4;

	ARM_MOV_REG_REG (buf, ARMREG_LR, ARMREG_PC);
	ARM_MOV_REG_REG (buf, ARMREG_PC, ARMREG_IP);
	
	/* OK, code address is now on r0. Move it to the place on the stack
	 * where IP was saved (it is now no more useful to us and it can be
	 * clobbered). This way we can just restore all the regs in one inst
	 * and branch to IP.
	 */
	ARM_STR_IMM (buf, ARMREG_R0, ARMREG_V1, (ARMREG_R12 * 4));

	/*
	 * Now we restore the MonoLMF (see emit_epilogue in mini-arm.c)
	 * and the rest of the registers, so the method called will see
	 * the same state as before we executed.
	 * The pointer to MonoLMF is in r2.
	 */
	ARM_MOV_REG_REG (buf, ARMREG_R2, ARMREG_SP);
	/* ip = previous_lmf */
	ARM_LDR_IMM (buf, ARMREG_IP, ARMREG_R2, G_STRUCT_OFFSET (MonoLMF, previous_lmf));
	/* lr = lmf_addr */
	ARM_LDR_IMM (buf, ARMREG_LR, ARMREG_R2, G_STRUCT_OFFSET (MonoLMF, lmf_addr));
	/* *(lmf_addr) = previous_lmf */
	ARM_STR_IMM (buf, ARMREG_IP, ARMREG_LR, G_STRUCT_OFFSET (MonoLMF, previous_lmf));

	/* Non-standard function epilogue. Instead of doing a proper
	 * return, we just jump to the compiled code.
	 */
	/* Restore the registers and jump to the code:
	 * Note that IP has been conveniently set to the method addr.
	 */
	ARM_ADD_REG_IMM8 (buf, ARMREG_SP, ARMREG_SP, sizeof (MonoLMF) - sizeof (guint) * 14);
	ARM_POP_NWB (buf, 0x5fff);
	/* do we need to set sp? */
	ARM_ADD_REG_IMM8 (buf, ARMREG_SP, ARMREG_SP, (14 * 4));
	if (tramp_type == MONO_TRAMPOLINE_CLASS_INIT)
		ARM_MOV_REG_REG (buf, ARMREG_PC, ARMREG_LR);
	else
		ARM_MOV_REG_REG (buf, ARMREG_PC, ARMREG_IP);

	constants = (gpointer*)buf;
	constants [0] = mono_get_lmf_addr;
	if (tramp_type == MONO_TRAMPOLINE_CLASS_INIT)
		constants [1] = mono_class_init_trampoline;
	else if (tramp_type == MONO_TRAMPOLINE_AOT)
		constants [1] = mono_aot_trampoline;
	else if (tramp_type == MONO_TRAMPOLINE_AOT_PLT)
		constants [1] = mono_aot_plt_trampoline;
	else if (tramp_type == MONO_TRAMPOLINE_DELEGATE)
		constants [1] = mono_delegate_trampoline;
	else
		constants [1] = mono_magic_trampoline;

	/* backpatch by emitting the missing instructions skipped above */
	ARM_LDR_IMM (load_get_lmf_addr, ARMREG_R0, ARMREG_PC, (buf - load_get_lmf_addr - 8));
	ARM_LDR_IMM (load_trampoline, ARMREG_IP, ARMREG_PC, (buf + 4 - load_trampoline - 8));

	buf += 8;

	/* Flush instruction cache, since we've generated code */
	mono_arch_flush_icache (code, buf - code);

	/* Sanity check */
	g_assert ((buf - code) <= GEN_TRAMP_SIZE);

	return code;
}

gpointer
mono_arch_create_specific_trampoline (gpointer arg1, MonoTrampolineType tramp_type, MonoDomain *domain, guint32 *code_len)
{
	guint8 *code, *buf, *tramp;
	gpointer *constants;

	tramp = mono_get_trampoline_code (tramp_type);

	mono_domain_lock (domain);
	code = buf = mono_code_manager_reserve (domain->code_mp, 24);
	mono_domain_unlock (domain);

	/* we could reduce this to 12 bytes if tramp is within reach:
	 * ARM_PUSH ()
	 * ARM_BL ()
	 * method-literal
	 * The called code can access method using the lr register
	 * A 20 byte sequence could be:
	 * ARM_PUSH ()
	 * ARM_MOV_REG_REG (lr, pc)
	 * ARM_LDR_IMM (pc, pc, 0)
	 * method-literal
	 * tramp-literal
	 */
	/* We save all the registers, except PC and SP */
	ARM_PUSH (buf, 0x5fff);
	ARM_LDR_IMM (buf, ARMREG_R0, ARMREG_PC, 4); /* arg1 is the only arg */
	ARM_LDR_IMM (buf, ARMREG_R1, ARMREG_PC, 4); /* temp reg */
	ARM_MOV_REG_REG (buf, ARMREG_PC, ARMREG_R1);

	constants = (gpointer*)buf;
	constants [0] = arg1;
	constants [1] = tramp;
	buf += 8;

	/* Flush instruction cache, since we've generated code */
	mono_arch_flush_icache (code, buf - code);

	g_assert ((buf - code) <= 24);

	if (code_len)
		*code_len = buf - code;

	return code;
}

/*
 * This method is only called when running in the Mono Debugger.
 */
gpointer
mono_debugger_create_notification_function (void)
{
	guint8 *ptr, *buf;

	ptr = buf = mono_global_codeman_reserve (8);
	//FIXME: ARM_SWI (buf, 0x9F0001);
	ARM_MOV_REG_REG (buf, ARMREG_PC, ARMREG_LR);
	mono_arch_flush_icache (ptr, buf - ptr);

	return ptr;
}

