/*
 * tramp-ia64.c: JIT trampoline code for ia64
 *
 * Authors:
 *   Zoltan Varga (vargaz@gmail.com)
 *
 * (C) 2001 Ximian, Inc.
 */

#include <config.h>
#include <glib.h>

#include <mono/metadata/appdomain.h>
#include <mono/metadata/marshal.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/mono-debug-debugger.h>
#include <mono/arch/ia64/ia64-codegen.h>

#include "mini.h"
#include "mini-ia64.h"

#define NOT_IMPLEMENTED g_assert_not_reached ()

#define GP_SCRATCH_REG 31
#define GP_SCRATCH_REG2 30

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
	guint8 *buf;
	gpointer func_addr, func_gp;
	Ia64CodegenState code;
	int this_reg = 0;
	gpointer *desc;
	MonoDomain *domain = mono_domain_get ();

	/* FIXME: Optimize this */

	if (!mono_method_signature (m)->ret->byref && MONO_TYPE_ISSTRUCT (mono_method_signature (m)->ret))
		this_reg = 1;

	func_addr = ((gpointer*)addr) [0];
	func_gp = ((gpointer*)addr) [1];

	mono_domain_lock (domain);
	buf = mono_code_manager_reserve (domain->code_mp, 256);
	mono_domain_unlock (domain);

	/* Since the this reg is a stacked register, its a bit hard to access it */
	ia64_codegen_init (code, buf);
	ia64_alloc (code, 40, 8, 1, 0, 0);
	ia64_adds_imm (code, 32 + this_reg, sizeof (MonoObject), 32 + this_reg);
	ia64_mov_to_ar_i (code, IA64_PFS, 40);	
	ia64_movl (code, GP_SCRATCH_REG, func_addr);
	ia64_mov_to_br (code, IA64_B6, GP_SCRATCH_REG);
	ia64_br_cond_reg (code, IA64_B6);
	ia64_codegen_close (code);

	g_assert (code.buf - buf < 256);

	mono_arch_flush_icache (buf, code.buf - buf);

	/* FIXME: */
	desc = g_malloc0 (sizeof (gpointer) * 2);
	desc [0] = buf;
	desc [1] = func_gp;

	return desc;
}

void
mono_arch_patch_callsite (guint8 *code, guint8 *addr)
{
	guint8 *callsite_begin;
	guint64 *callsite = (guint64*)(gpointer)(code - 16);
	guint64 *next_bundle;
	guint64 ins, instructions [3];
	guint64 buf [16];
	Ia64CodegenState gen;
	gpointer func = ((gpointer*)(gpointer)addr)[0];

	while ((ia64_bundle_template (callsite) != IA64_TEMPLATE_MLX) &&
		   (ia64_bundle_template (callsite) != IA64_TEMPLATE_MLXS))
		callsite -= 2;
	callsite_begin = (guint8*)callsite;

	next_bundle = callsite + 2;
	ins = ia64_bundle_ins1 (next_bundle);
	if (ia64_ins_opcode (ins) == 5) {
		/* ld8_inc_imm -> indirect call through a function pointer */
		g_assert (ia64_ins_r1 (ins) == GP_SCRATCH_REG2);
		g_assert (ia64_ins_r3 (ins) == GP_SCRATCH_REG);
		return;
	}

	/* Patch the code generated by emit_call */

	instructions [0] = ia64_bundle_ins1 (callsite);
	instructions [1] = ia64_bundle_ins2 (callsite);
	instructions [2] = ia64_bundle_ins3 (callsite);

	ia64_codegen_init (gen, (guint8*)buf);
	ia64_movl (gen, GP_SCRATCH_REG, func);
	instructions [1] = gen.instructions [0];
	instructions [2] = gen.instructions [1];

	ia64_codegen_init (gen, (guint8*)buf);
	ia64_emit_bundle_template (&gen, ia64_bundle_template (callsite), instructions [0], instructions [1], instructions [2]);
	ia64_codegen_close (gen);

	/* This might not be safe, but not all itanium processors support st16 */
	callsite [0] = buf [0];
	callsite [1] = buf [1];

	mono_arch_flush_icache (callsite_begin, code - callsite_begin);
}

void
mono_arch_patch_plt_entry (guint8 *code, guint8 *addr)
{
	g_assert_not_reached ();
}

void
mono_arch_nullify_class_init_trampoline (guint8 *code, gssize *regs)
{
	guint8 *callsite_begin;
	guint64 *callsite = (guint64*)(gpointer)(code - 16);
	guint64 instructions [3];
	guint64 buf [16];
	Ia64CodegenState gen;

	while ((ia64_bundle_template (callsite) != IA64_TEMPLATE_MLX) &&
		   (ia64_bundle_template (callsite) != IA64_TEMPLATE_MLXS))
		callsite -= 2;
	callsite_begin = (guint8*)callsite;

	/* Replace the code generated by emit_call with a sets of nops */

	/* The first bundle might have other instructions in it */
	instructions [0] = ia64_bundle_ins1 (callsite);
	instructions [1] = IA64_NOP_X;
	instructions [2] = IA64_NOP_X;

	ia64_codegen_init (gen, (guint8*)buf);
	ia64_emit_bundle_template (&gen, ia64_bundle_template (callsite), instructions [0], instructions [1], instructions [2]);
	ia64_codegen_close (gen);

	/* This might not be safe, but not all itanium processors support st16 */
	callsite [0] = buf [0];
	callsite [1] = buf [1];

	callsite += 2;

	/* The other bundles can be full replaced with nops */

	ia64_codegen_init (gen, (guint8*)buf);
	ia64_emit_bundle_template (&gen, IA64_TEMPLATE_MII, IA64_NOP_M, IA64_NOP_I, IA64_NOP_I);
	ia64_codegen_close (gen);

	while ((guint8*)callsite < code) {
		callsite [0] = buf [0];
		callsite [1] = buf [1];
		callsite += 2;
	}

	mono_arch_flush_icache (callsite_begin, code - callsite_begin);
}

void
mono_arch_nullify_plt_entry (guint8 *code)
{
	g_assert_not_reached ();
}

void
mono_arch_patch_delegate_trampoline (guint8 *code, guint8 *tramp, gssize *regs, guint8 *addr)
{
	/* 
	 * This is called by the code generated by OP_CALL_REG:
	 * ld8 r30=[r8],8
	 * nop.i 0x0;;
	 * mov.sptk b6=r30
	 * ld8 r1=[r8]
	 * br.call.sptk.few b0=b6
	 */

	/* We patch the function descriptor instead of delegate->method_ptr */
	//g_assert (((gpointer*)(regs [8] - 8))[0] == tramp);
	((gpointer*)(regs [8] - 8))[0] = mono_get_addr_from_ftnptr (addr);
	((gpointer*)(regs [8] - 8))[1] = NULL;
}

guchar*
mono_arch_create_trampoline_code (MonoTrampolineType tramp_type)
{
	guint8 *buf, *tramp;
	int i, offset, saved_regs_offset, saved_fpregs_offset, last_offset, framesize;
	int in0, local0, out0, l0, l1, l2, l3, l4, l5, l6, l7, l8, o0, o1, o2, o3;
	gboolean has_caller;
	Ia64CodegenState code;
	unw_dyn_info_t *di;
	unw_dyn_region_info_t *r_pro;

	/* 
	 * Since jump trampolines are not patched, this trampoline is executed every
	 * time a call is made to a jump trampoline. So we try to keep things faster
	 * in that case.
	 */
	if (tramp_type == MONO_TRAMPOLINE_JUMP)
		has_caller = FALSE;
	else
		has_caller = TRUE;

	buf = mono_global_codeman_reserve (2048);

	ia64_codegen_init (code, buf);

	/* FIXME: Save/restore lmf */

	/* Stacked Registers */
	in0 = 32;
	local0 = in0 + 8;
	out0 = local0 + 16;
	l0 = 40;
	l1 = 41;
	l2 = 42;
	l3 = 43;
	l4 = 44;
	l5 = 45; /* saved ar.pfs */
	l6 = 46; /* arg */
	l7 = 47; /* code */
	l8 = 48; /* saved sp */
	o0 = out0 + 0; /* regs */
	o1 = out0 + 1; /* code */
	o2 = out0 + 2; /* arg */
	o3 = out0 + 3; /* tramp */

	framesize = (128 * 8) + 1024;
	framesize = (framesize + (MONO_ARCH_FRAME_ALIGNMENT - 1)) & ~ (MONO_ARCH_FRAME_ALIGNMENT - 1);

	/*
	 * Allocate a new register+memory stack frame.
	 * 8 input registers (the max used by the ABI)
	 * 16 locals
	 * 4 output (number of parameters passed to trampoline)
	 */
	ia64_unw_save_reg (code, UNW_IA64_AR_PFS, UNW_IA64_GR + l5);
	ia64_alloc (code, l5, local0 - in0, out0 - local0, 4, 0);
	ia64_unw_save_reg (code, UNW_IA64_SP, UNW_IA64_GR + l8);
	ia64_mov (code, l8, IA64_SP);
	ia64_adds_imm (code, IA64_SP, (-framesize), IA64_SP);

	offset = 16; /* scratch area */

	/* Save the argument received from the specific trampoline */
	ia64_mov (code, l6, GP_SCRATCH_REG);

	/* Save the calling address */
	ia64_unw_save_reg (code, UNW_IA64_RP, UNW_IA64_GR + local0 + 7);
	ia64_mov_from_br (code, l7, IA64_B0);

	/* Create unwind info for the prolog */
	ia64_begin_bundle (code);
	r_pro = mono_ia64_create_unwind_region (&code);

	/* Save registers */
	/* Not needed for jump trampolines */
	if (tramp_type != MONO_TRAMPOLINE_JUMP) {
		saved_regs_offset = offset;
		offset += 128 * 8;
		/* 
		 * Only the registers which are needed for computing vtable slots need
		 * to be saved.
		 */
		last_offset = -1;
		for (i = 0; i < 64; ++i)
			if ((1 << i) & MONO_ARCH_CALLEE_REGS) {
				if (last_offset != i * 8)
					ia64_adds_imm (code, l1, saved_regs_offset + (i * 8), IA64_SP);
				ia64_st8_spill_inc_imm_hint (code, l1, i, 8, 0);
				last_offset = (i + 1) * 8;
			}
	}

	/* Save fp registers */
	saved_fpregs_offset = offset;
	offset += 8 * 8;
	ia64_adds_imm (code, l1, saved_fpregs_offset, IA64_SP);
	for (i = 0; i < 8; ++i)
		ia64_stfd_inc_imm_hint (code, l1, i + 8, 8, 0);

	g_assert (offset < framesize);

	/* Arg1 is the pointer to the saved registers */
	ia64_adds_imm (code, o0, saved_regs_offset, IA64_SP);

	/* Arg2 is the address of the calling code */
	if (has_caller)
		ia64_mov (code, o1, l7);
	else
		ia64_mov (code, o1, 0);

	/* Arg3 is the method/vtable ptr */
	ia64_mov (code, o2, l6);

	/* Arg4 is the trampoline address */
	/* FIXME: */
	ia64_mov (code, o3, 0);

	if (tramp_type == MONO_TRAMPOLINE_CLASS_INIT)
		tramp = (guint8*)mono_class_init_trampoline;
	else if (tramp_type == MONO_TRAMPOLINE_AOT)
		tramp = (guint8*)mono_aot_trampoline;
	else if (tramp_type == MONO_TRAMPOLINE_DELEGATE)
		tramp = (guint8*)mono_delegate_trampoline;
	else
		tramp = (guint8*)mono_magic_trampoline;

	/* Call the trampoline using an indirect call */
	ia64_movl (code, l0, tramp);
	ia64_ld8_inc_imm (code, l1, l0, 8);
	ia64_mov_to_br (code, IA64_B6, l1);
	ia64_ld8 (code, IA64_GP, l0);
	ia64_br_call_reg (code, 0, IA64_B6);

	/* Restore fp regs */
	ia64_adds_imm (code, l1, saved_fpregs_offset, IA64_SP);
	for (i = 0; i < 8; ++i)
		ia64_ldfd_inc_imm (code, i + 8, l1, 8);

	/* FIXME: Handle NATs in fp regs / scratch regs */

	if (tramp_type != MONO_TRAMPOLINE_CLASS_INIT) {
		/* Load method address from function descriptor */
		ia64_ld8 (code, l0, IA64_R8);
		ia64_mov_to_br (code, IA64_B6, l0);
	}

	/* Clean up register/memory stack frame */
	ia64_adds_imm (code, IA64_SP, framesize, IA64_SP);
	ia64_mov_to_ar_i (code, IA64_PFS, l5);

	if (tramp_type == MONO_TRAMPOLINE_CLASS_INIT) {
		ia64_mov_ret_to_br (code, IA64_B0, l7);
		ia64_br_ret_reg (code, IA64_B0);
	}
	else {
		/* Call the compiled method */
		ia64_mov_to_br (code, IA64_B0, l7);
		ia64_br_cond_reg (code, IA64_B6);
	}

	ia64_codegen_close (code);

	g_assert ((code.buf - buf) <= 2048);

	/* FIXME: emit unwind info for epilog */
	di = g_malloc0 (sizeof (unw_dyn_info_t));
	di->start_ip = (unw_word_t) buf;
	di->end_ip = (unw_word_t) code.buf;
	di->gp = 0;
	di->format = UNW_INFO_FORMAT_DYNAMIC;
	di->u.pi.name_ptr = (unw_word_t)"ia64_generic_trampoline";
	di->u.pi.regions = r_pro;

	_U_dyn_register (di);

	mono_arch_flush_icache (buf, code.buf - buf);

	return buf;
}

#define TRAMPOLINE_SIZE 128

gpointer
mono_arch_create_specific_trampoline (gpointer arg1, MonoTrampolineType tramp_type, MonoDomain *domain, guint32 *code_len)
{
	guint8 *buf, *tramp;
	gint64 disp;
	Ia64CodegenState code;

	tramp = mono_get_trampoline_code (tramp_type);

	mono_domain_lock (domain);
	buf = mono_code_manager_reserve (domain->code_mp, TRAMPOLINE_SIZE);
	mono_domain_unlock (domain);

	/* FIXME: Optimize this */

	ia64_codegen_init (code, buf);

	ia64_movl (code, GP_SCRATCH_REG, arg1);

	ia64_begin_bundle (code);
	disp = (tramp - code.buf) >> 4;
	if (ia64_is_imm21 (disp)) {
		ia64_br_cond (code, disp);
	}
	else {
		ia64_movl (code, GP_SCRATCH_REG2, tramp);
		ia64_mov_to_br (code, IA64_B6, GP_SCRATCH_REG2);
		ia64_br_cond_reg (code, IA64_B6);
	}

	ia64_codegen_close (code);

	g_assert (code.buf - buf <= TRAMPOLINE_SIZE);

	mono_arch_flush_icache (buf, code.buf - buf);

	if (code_len)
		*code_len = code.buf - buf;

	return buf;
}

void
mono_arch_invalidate_method (MonoJitInfo *ji, void *func, gpointer func_arg)
{
	NOT_IMPLEMENTED;
}

guint8*
mono_debugger_create_notification_function (MonoCodeManager *codeman)
{
	NOT_IMPLEMENTED;

	return NULL;
}
