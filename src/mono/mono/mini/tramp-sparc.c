/*
 * tramp-sparc.c: JIT trampoline code for Sparc
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
	int this_pos = 4, reg;

	if (!mono_method_signature (m)->ret->byref && MONO_TYPE_ISSTRUCT (mono_method_signature (m)->ret))
		this_pos = 8;
	    
	start = code = mono_global_codeman_reserve (36);

	/* This executes in the context of the caller, hence o0 */
	sparc_add_imm (code, 0, sparc_o0, sizeof (MonoObject), sparc_o0);
#ifdef SPARCV9
	reg = sparc_g4;
#else
	reg = sparc_g1;
#endif
	sparc_set (code, addr, reg);
	sparc_jmpl (code, reg, sparc_g0, sparc_g0);
	sparc_nop (code);

	g_assert ((code - start) <= 36);

	mono_arch_flush_icache (start, code - start);

	return start;
}

void
mono_arch_patch_callsite (guint8 *code, guint8 *addr)
{
	if (sparc_inst_op (*code) == 0x1) {
		sparc_call_simple (code, (guint8*)addr - (guint8*)code);
	}
}

void
mono_arch_nullify_class_init_trampoline (guint8 *code, gssize *regs)
{
	/* Patch calling code */
	sparc_nop (code);
}

#define ALIGN_TO(val,align) (((val) + ((align) - 1)) & ~((align) - 1))

guchar*
mono_arch_create_trampoline_code (MonoTrampolineType tramp_type)
{
	guint8 *buf, *code, *tramp_addr;
	guint32 lmf_offset, regs_offset, method_reg, i;
	gboolean has_caller;

	if (tramp_type == MONO_TRAMPOLINE_JUMP)
		has_caller = FALSE;
	else
		has_caller = TRUE;

	code = buf = mono_global_codeman_reserve (1024);

	sparc_save_imm (code, sparc_sp, -1608, sparc_sp);

#ifdef SPARCV9
	method_reg = sparc_g4;
#else
	method_reg = sparc_g1;
#endif

#ifdef SPARCV9
	/* Save fp regs since they are not preserved by calls */
	for (i = 0; i < 16; i ++)
		sparc_stdf_imm (code, sparc_f0 + (i * 2), sparc_sp, MONO_SPARC_STACK_BIAS + 320 + (i * 8));
#endif	

	/* We receive the method address in %r1, so save it here */
	sparc_sti_imm (code, method_reg, sparc_sp, MONO_SPARC_STACK_BIAS + 200);

	/* Save lmf since compilation can raise exceptions */
	lmf_offset = MONO_SPARC_STACK_BIAS - sizeof (MonoLMF);

	/* Save the data for the parent (managed) frame */

	/* Save ip */
	sparc_sti_imm (code, sparc_i7, sparc_fp, lmf_offset + G_STRUCT_OFFSET (MonoLMF, ip));
	/* Save sp */
	sparc_sti_imm (code, sparc_fp, sparc_fp, lmf_offset + G_STRUCT_OFFSET (MonoLMF, sp));
	/* Save fp */
	/* Load previous fp from the saved register window */
	sparc_flushw (code);
	sparc_ldi_imm (code, sparc_fp, MONO_SPARC_STACK_BIAS + (sparc_i6 - 16) * sizeof (gpointer), sparc_o7);
	sparc_sti_imm (code, sparc_o7, sparc_fp, lmf_offset + G_STRUCT_OFFSET (MonoLMF, ebp));
	/* Save method */
	sparc_sti_imm (code, method_reg, sparc_fp, lmf_offset + G_STRUCT_OFFSET (MonoLMF, method));

	sparc_set (code, mono_get_lmf_addr, sparc_o7);
	sparc_jmpl (code, sparc_o7, sparc_g0, sparc_o7);
	sparc_nop (code);

	code = mono_sparc_emit_save_lmf (code, lmf_offset);

	regs_offset = MONO_SPARC_STACK_BIAS + 1000;

	if (has_caller) {
		/* Load all registers of the caller into a table inside this frame */
		/* first the out registers */
		for (i = 0; i < 8; ++i)
			sparc_sti_imm (code, sparc_i0 + i, sparc_sp, regs_offset + (i * sizeof (gpointer)));
		/* then the in+local registers */
		for (i = 0; i < 16; i ++) {
			sparc_ldi_imm (code, sparc_fp, MONO_SPARC_STACK_BIAS + (i * sizeof (gpointer)), sparc_o7);
			sparc_sti_imm (code, sparc_o7, sparc_sp, regs_offset + ((i + 8) * sizeof (gpointer)));
		}
	}

	if (tramp_type == MONO_TRAMPOLINE_CLASS_INIT)
		tramp_addr = &mono_class_init_trampoline;
	else
		tramp_addr = &mono_magic_trampoline;
	sparc_ldi_imm (code, sparc_sp, MONO_SPARC_STACK_BIAS + 200, sparc_o2);
	/* pass address of register table as third argument */
	sparc_add_imm (code, FALSE, sparc_sp, regs_offset, sparc_o0);
	sparc_set (code, tramp_addr, sparc_o7);
	/* set %o1 to caller address */
	if (has_caller)
		sparc_mov_reg_reg (code, sparc_i7, sparc_o1);
	else
		sparc_set (code, 0, sparc_o1);
	sparc_set (code, 0, sparc_o3);
	sparc_jmpl (code, sparc_o7, sparc_g0, sparc_o7);
	sparc_nop (code);

	/* Save result */
	sparc_sti_imm (code, sparc_o0, sparc_sp, MONO_SPARC_STACK_BIAS + 304);

	/* Restore lmf */
	code = mono_sparc_emit_restore_lmf (code, lmf_offset);

	/* Reload result */
	sparc_ldi_imm (code, sparc_sp, MONO_SPARC_STACK_BIAS + 304, sparc_o0);

#ifdef SPARCV9
	/* Reload fp regs */
	for (i = 0; i < 16; i ++)
		sparc_lddf_imm (code, sparc_sp, MONO_SPARC_STACK_BIAS + 320 + (i * 8), sparc_f0 + (i * 2));
#endif	

	if (tramp_type == MONO_TRAMPOLINE_CLASS_INIT)
		sparc_ret (code);
	else
		sparc_jmpl (code, sparc_o0, sparc_g0, sparc_g0);

	/* restore previous frame in delay slot */
	sparc_restore_simple (code);

/*
{
	gpointer addr;

	sparc_save_imm (code, sparc_sp, -608, sparc_sp);
	addr = code;
	sparc_call_simple (code, 16);
	sparc_nop (code);
	sparc_rett_simple (code);
	sparc_nop (code);

	sparc_save_imm (code, sparc_sp, -608, sparc_sp);
	sparc_ta (code, 1);
	tramp_addr = &sparc_magic_trampoline;
	sparc_call_simple (code, tramp_addr - code);
	sparc_nop (code);
	sparc_rett_simple (code);
	sparc_nop (code);
}
*/

	g_assert ((code - buf) <= 512);

	mono_arch_flush_icache (buf, code - buf);

	return buf;
}

#define TRAMPOLINE_SIZE (((SPARC_SET_MAX_SIZE >> 2) * 2) + 2)

static MonoJitInfo*
create_specific_trampoline (gpointer arg1, MonoTrampolineType tramp_type, MonoDomain *domain)
{
	MonoJitInfo *ji;
	guint32 *code, *buf, *tramp;

	tramp = mono_get_trampoline_code (tramp_type);

	mono_domain_lock (domain);
	code = buf = mono_code_manager_reserve (domain->code_mp, TRAMPOLINE_SIZE * 4);
	mono_domain_unlock (domain);

	/* We have to use g5 here because there is no other free register */
	sparc_set (code, tramp, sparc_g5);
#ifdef SPARCV9
	sparc_set (code, arg1, sparc_g4);
#else
	sparc_set (code, arg1, sparc_g1);
#endif
	sparc_jmpl (code, sparc_g5, sparc_g0, sparc_g0);
	sparc_nop (code);

	g_assert ((code - buf) <= TRAMPOLINE_SIZE);

	ji = g_new0 (MonoJitInfo, 1);
	ji->code_start = buf;
	ji->code_size = (code - buf) * 4;

	mono_jit_stats.method_trampolines++;

	mono_arch_flush_icache (ji->code_start, ji->code_size);

	return ji;
}	

MonoJitInfo*
mono_arch_create_jump_trampoline (MonoMethod *method)
{
	MonoJitInfo *ji = create_specific_trampoline (method, MONO_TRAMPOLINE_JUMP, mono_domain_get ());

	ji->method = method;
	return ji;
}

/**
 * mono_arch_create_jit_trampoline:
 * @method: pointer to the method info
 *
 * Creates a trampoline function for virtual methods. If the created
 * code is called it first starts JIT compilation of method,
 * and then calls the newly created method. I also replaces the
 * corresponding vtable entry (see sparc_magic_trampoline).
 * 
 * Returns: a pointer to the newly created code 
 */
gpointer
mono_arch_create_jit_trampoline (MonoMethod *method)
{
	MonoJitInfo *ji;
	gpointer code_start;

	ji = create_specific_trampoline (method, MONO_TRAMPOLINE_GENERIC, mono_domain_get ());
	code_start = ji->code_start;
	g_free (ji);

	return code_start;
}

/**
 * mono_arch_create_class_init_trampoline:
 *  @vtable: the type to initialize
 *
 * Creates a trampoline function to run a type initializer. 
 * If the trampoline is called, it calls mono_runtime_class_init with the
 * given vtable, then patches the caller code so it does not get called any
 * more.
 * 
 * Returns: a pointer to the newly created code 
 */
gpointer
mono_arch_create_class_init_trampoline (MonoVTable *vtable)
{
	MonoJitInfo *ji;
	gpointer code;

	ji = create_specific_trampoline (vtable, MONO_TRAMPOLINE_CLASS_INIT, vtable->domain);
	code = ji->code_start;
	g_free (ji);

	return code;
}

/*
 * This method is only called when running in the Mono Debugger.
 */
gpointer
mono_debugger_create_notification_function (gpointer *notification_address)
{
	guint8 *ptr, *buf;

	ptr = buf = mono_global_codeman_reserve (16);
	if (notification_address)
		*notification_address = buf;

	g_assert_not_reached ();

	return ptr;
}
