/*
 * mini-arm.c: ARM backend for the Mono code generator
 *
 * Authors:
 *   Paolo Molaro (lupus@ximian.com)
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * (C) 2003 Ximian, Inc.
 */
#include "mini.h"
#include <string.h>

#include <mono/metadata/appdomain.h>
#include <mono/metadata/debug-helpers.h>

#include "mini-arm.h"
#include "inssel.h"
#include "cpu-arm.h"
#include "trace.h"
#include "mono/arch/arm/arm-fpa-codegen.h"

/*
 * TODO:
 * floating point support: on ARM it is a mess, there are at least 3
 * different setups, each of which binary incompat with the other.
 * 1) FPA: old and ugly, but unfortunately what current distros use
 *    the double binary format has the two words swapped. 8 double registers.
 *    Implemented usually by kernel emulation.
 * 2) softfloat: the compiler emulates all the fp ops. Usually uses the
 *    ugly swapped double format (I guess a softfloat-vfp exists, too, though).
 * 3) VFP: the new and actually sensible and useful FP support. Implemented
 *    in HW or kernel-emulated, requires new tools. I think this ios what symbian uses.
 *
 * The plan is to write the FPA support first. softfloat can be tested in a chroot.
 */
int mono_exc_esp_offset = 0;

#define arm_is_imm12(v) ((v) > -4096 && (v) < 4096)
#define arm_is_imm8(v) ((v) > -256 && (v) < 256)
#define arm_is_fpimm8(v) ((v) >= -1020 && (v) <= 1020)

const char*
mono_arch_regname (int reg) {
	static const char * rnames[] = {
		"arm_r0", "arm_r1", "arm_r2", "arm_r3", "arm_v1",
		"arm_v2", "arm_v3", "arm_v4", "arm_v5", "arm_v6",
		"arm_v7", "arm_fp", "arm_ip", "arm_sp", "arm_lr",
		"arm_pc"
	};
	if (reg >= 0 && reg < 16)
		return rnames [reg];
	return "unknown";
}

const char*
mono_arch_fregname (int reg) {
	static const char * rnames[] = {
		"arm_f0", "arm_f1", "arm_f2", "arm_f3", "arm_f4",
		"arm_f5", "arm_f6", "arm_f7", "arm_f8", "arm_f9",
		"arm_f10", "arm_f11", "arm_f12", "arm_f13", "arm_f14",
		"arm_f15", "arm_f16", "arm_f17", "arm_f18", "arm_f19",
		"arm_f20", "arm_f21", "arm_f22", "arm_f23", "arm_f24",
		"arm_f25", "arm_f26", "arm_f27", "arm_f28", "arm_f29",
		"arm_f30", "arm_f31"
	};
	if (reg >= 0 && reg < 32)
		return rnames [reg];
	return "unknown";
}

static guint8*
emit_memcpy (guint8 *code, int size, int dreg, int doffset, int sreg, int soffset)
{
	return code;
}

/*
 * mono_arch_get_argument_info:
 * @csig:  a method signature
 * @param_count: the number of parameters to consider
 * @arg_info: an array to store the result infos
 *
 * Gathers information on parameters such as size, alignment and
 * padding. arg_info should be large enought to hold param_count + 1 entries. 
 *
 * Returns the size of the activation frame.
 */
int
mono_arch_get_argument_info (MonoMethodSignature *csig, int param_count, MonoJitArgumentInfo *arg_info)
{
	int k, frame_size = 0;
	int size, align, pad;
	int offset = 8;

	if (MONO_TYPE_ISSTRUCT (csig->ret)) { 
		frame_size += sizeof (gpointer);
		offset += 4;
	}

	arg_info [0].offset = offset;

	if (csig->hasthis) {
		frame_size += sizeof (gpointer);
		offset += 4;
	}

	arg_info [0].size = frame_size;

	for (k = 0; k < param_count; k++) {
		
		if (csig->pinvoke)
			size = mono_type_native_stack_size (csig->params [k], &align);
		else
			size = mono_type_stack_size (csig->params [k], &align);

		/* ignore alignment for now */
		align = 1;

		frame_size += pad = (align - (frame_size & (align - 1))) & (align - 1);	
		arg_info [k].pad = pad;
		frame_size += size;
		arg_info [k + 1].pad = 0;
		arg_info [k + 1].size = size;
		offset += pad;
		arg_info [k + 1].offset = offset;
		offset += size;
	}

	align = MONO_ARCH_FRAME_ALIGNMENT;
	frame_size += pad = (align - (frame_size & (align - 1))) & (align - 1);
	arg_info [k].pad = pad;

	return frame_size;
}

/*
 * Initialize the cpu to execute managed code.
 */
void
mono_arch_cpu_init (void)
{
}

/*
 * This function returns the optimizations supported on this cpu.
 */
guint32
mono_arch_cpu_optimizazions (guint32 *exclude_mask)
{
	guint32 opts = 0;

	/* no arm-specific optimizations yet */
	*exclude_mask = 0;
	return opts;
}

static gboolean
is_regsize_var (MonoType *t) {
	if (t->byref)
		return TRUE;
	t = mono_type_get_underlying_type (t);
	switch (t->type) {
	case MONO_TYPE_I4:
	case MONO_TYPE_U4:
	case MONO_TYPE_I:
	case MONO_TYPE_U:
	case MONO_TYPE_PTR:
	case MONO_TYPE_FNPTR:
		return TRUE;
	case MONO_TYPE_OBJECT:
	case MONO_TYPE_STRING:
	case MONO_TYPE_CLASS:
	case MONO_TYPE_SZARRAY:
	case MONO_TYPE_ARRAY:
		return TRUE;
	case MONO_TYPE_VALUETYPE:
		return FALSE;
	}
	return FALSE;
}

GList *
mono_arch_get_allocatable_int_vars (MonoCompile *cfg)
{
	GList *vars = NULL;
	int i;

	for (i = 0; i < cfg->num_varinfo; i++) {
		MonoInst *ins = cfg->varinfo [i];
		MonoMethodVar *vmv = MONO_VARINFO (cfg, i);

		/* unused vars */
		if (vmv->range.first_use.abs_pos >= vmv->range.last_use.abs_pos)
			continue;

		if (ins->flags & (MONO_INST_VOLATILE|MONO_INST_INDIRECT) || (ins->opcode != OP_LOCAL && ins->opcode != OP_ARG))
			continue;

		/* we can only allocate 32 bit values */
		if (is_regsize_var (ins->inst_vtype)) {
			g_assert (MONO_VARINFO (cfg, i)->reg == -1);
			g_assert (i == vmv->idx);
			vars = mono_varlist_insert_sorted (cfg, vars, vmv, FALSE);
		}
	}

	return vars;
}

#define USE_EXTRA_TEMPS 0

GList *
mono_arch_get_global_int_regs (MonoCompile *cfg)
{
	GList *regs = NULL;
	regs = g_list_prepend (regs, GUINT_TO_POINTER (ARMREG_V1));
	regs = g_list_prepend (regs, GUINT_TO_POINTER (ARMREG_V2));
	regs = g_list_prepend (regs, GUINT_TO_POINTER (ARMREG_V3));
	regs = g_list_prepend (regs, GUINT_TO_POINTER (ARMREG_V4));
	regs = g_list_prepend (regs, GUINT_TO_POINTER (ARMREG_V5));
	regs = g_list_prepend (regs, GUINT_TO_POINTER (ARMREG_V6));
	regs = g_list_prepend (regs, GUINT_TO_POINTER (ARMREG_V7));

	return regs;
}

/*
 * mono_arch_regalloc_cost:
 *
 *  Return the cost, in number of memory references, of the action of 
 * allocating the variable VMV into a register during global register
 * allocation.
 */
guint32
mono_arch_regalloc_cost (MonoCompile *cfg, MonoMethodVar *vmv)
{
	/* FIXME: */
	return 2;
}

void
mono_arch_flush_icache (guint8 *code, gint size)
{
	__asm __volatile ("mov r0, %0\n"
			"mov r1, %1\n"
			"mov r2, %2\n"
			"swi 0x9f0002       @ sys_cacheflush"
			: /* no outputs */
			: "r" (code), "r" (code + size), "r" (0)
			: "r0", "r1", "r3" );

}

#define NOT_IMPLEMENTED(x) \
                g_error ("FIXME: %s is not yet implemented. (trampoline)", x);

enum {
	RegTypeGeneral,
	RegTypeBase,
	RegTypeFP,
	RegTypeStructByVal,
	RegTypeStructByAddr
};

typedef struct {
	gint32  offset;
	guint16 vtsize; /* in param area */
	guint8  reg;
	guint8  regtype : 4; /* 0 general, 1 basereg, 2 floating point register, see RegType* */
	guint8  size    : 4; /* 1, 2, 4, 8, or regs used by RegTypeStructByVal */
} ArgInfo;

typedef struct {
	int nargs;
	guint32 stack_usage;
	guint32 struct_ret;
	ArgInfo ret;
	ArgInfo sig_cookie;
	ArgInfo args [1];
} CallInfo;

#define DEBUG(a)

static void inline
add_general (guint *gr, guint *stack_size, ArgInfo *ainfo, gboolean simple)
{
	if (simple) {
		if (*gr > ARMREG_R3) {
			ainfo->offset = *stack_size;
			ainfo->reg = ARMREG_SP; /* in the caller */
			ainfo->regtype = RegTypeBase;
			*stack_size += 4;
		} else {
			ainfo->reg = *gr;
		}
	} else {
		if (*gr > ARMREG_R2) {
#ifdef ALIGN_DOUBLES
			//*stack_size += (*stack_size % 8);
#endif
			ainfo->offset = *stack_size;
			ainfo->reg = ARMREG_SP; /* in the caller */
			ainfo->regtype = RegTypeBase;
			*stack_size += 8;
		} else {
			if ((*gr) & 1)
				(*gr) ++;
			ainfo->reg = *gr;
		}
		(*gr) ++;
	}
	(*gr) ++;
}

static CallInfo*
calculate_sizes (MonoMethodSignature *sig, gboolean is_pinvoke)
{
	guint i, gr;
	int n = sig->hasthis + sig->param_count;
	guint32 simpletype;
	guint32 stack_size = 0;
	CallInfo *cinfo = g_malloc0 (sizeof (CallInfo) + sizeof (ArgInfo) * n);

	gr = ARMREG_R0;

	/* FIXME: handle returning a struct */
	if (MONO_TYPE_ISSTRUCT (sig->ret)) {
		add_general (&gr, &stack_size, &cinfo->ret, TRUE);
		cinfo->struct_ret = ARMREG_R0;
	}

	n = 0;
	if (sig->hasthis) {
		add_general (&gr, &stack_size, cinfo->args + n, TRUE);
		n++;
	}
        DEBUG(printf("params: %d\n", sig->param_count));
	for (i = 0; i < sig->param_count; ++i) {
		if ((sig->call_convention == MONO_CALL_VARARG) && (i == sig->sentinelpos)) {
                        /* Prevent implicit arguments and sig_cookie from
			   being passed in registers */
                        gr = ARMREG_R3 + 1;
                        /* Emit the signature cookie just before the implicit arguments */
                        add_general (&gr, &stack_size, &cinfo->sig_cookie, TRUE);
                }
                DEBUG(printf("param %d: ", i));
		if (sig->params [i]->byref) {
                        DEBUG(printf("byref\n"));
			add_general (&gr, &stack_size, cinfo->args + n, TRUE);
			n++;
			continue;
		}
		simpletype = mono_type_get_underlying_type (sig->params [i])->type;
	enum_calc_size:
		switch (simpletype) {
		case MONO_TYPE_BOOLEAN:
		case MONO_TYPE_I1:
		case MONO_TYPE_U1:
			cinfo->args [n].size = 1;
			add_general (&gr, &stack_size, cinfo->args + n, TRUE);
			n++;
			break;
		case MONO_TYPE_CHAR:
		case MONO_TYPE_I2:
		case MONO_TYPE_U2:
			cinfo->args [n].size = 2;
			add_general (&gr, &stack_size, cinfo->args + n, TRUE);
			n++;
			break;
		case MONO_TYPE_I4:
		case MONO_TYPE_U4:
			cinfo->args [n].size = 4;
			add_general (&gr, &stack_size, cinfo->args + n, TRUE);
			n++;
			break;
		case MONO_TYPE_I:
		case MONO_TYPE_U:
		case MONO_TYPE_PTR:
		case MONO_TYPE_FNPTR:
		case MONO_TYPE_CLASS:
		case MONO_TYPE_OBJECT:
		case MONO_TYPE_STRING:
		case MONO_TYPE_SZARRAY:
		case MONO_TYPE_ARRAY:
		case MONO_TYPE_R4:
			cinfo->args [n].size = sizeof (gpointer);
			add_general (&gr, &stack_size, cinfo->args + n, TRUE);
			n++;
			break;
		case MONO_TYPE_VALUETYPE: {
			gint size;
			MonoClass *klass;
			klass = mono_class_from_mono_type (sig->params [i]);
			if (is_pinvoke)
			    size = mono_class_native_size (klass, NULL);
			else
			    size = mono_class_value_size (klass, NULL);
			DEBUG(printf ("load %d bytes struct\n",
				      mono_class_native_size (sig->params [i]->data.klass, NULL)));
#if 0
#if PPC_PASS_STRUCTS_BY_VALUE
			{
				int align_size = size;
				int nwords = 0;
				align_size += (sizeof (gpointer) - 1);
				align_size &= ~(sizeof (gpointer) - 1);
				nwords = (align_size + sizeof (gpointer) -1 ) / sizeof (gpointer);
				cinfo->args [n].regtype = RegTypeStructByVal;
				if (gr > PPC_LAST_ARG_REG || (size >= 3 && size % 4 != 0)) {
					cinfo->args [n].size = 0;
					cinfo->args [n].vtsize = nwords;
				} else {
					int rest = PPC_LAST_ARG_REG - gr + 1;
					int n_in_regs = rest >= nwords? nwords: rest;
					cinfo->args [n].size = n_in_regs;
					cinfo->args [n].vtsize = nwords - n_in_regs;
					cinfo->args [n].reg = gr;
					gr += n_in_regs;
				}
				cinfo->args [n].offset = PPC_STACK_PARAM_OFFSET + stack_size;
				/*g_print ("offset for arg %d at %d\n", n, PPC_STACK_PARAM_OFFSET + stack_size);*/
				stack_size += nwords * sizeof (gpointer);
			}
#else
			add_general (&gr, &stack_size, cinfo->args + n, TRUE);
			cinfo->args [n].regtype = RegTypeStructByAddr;
#endif
#endif
			n++;
			break;
		}
		case MONO_TYPE_TYPEDBYREF: {
			int size = sizeof (MonoTypedRef);
			/* keep in sync or merge with the valuetype case */
#if 0
#if PPC_PASS_STRUCTS_BY_VALUE
			{
				int nwords = (size + sizeof (gpointer) -1 ) / sizeof (gpointer);
				cinfo->args [n].regtype = RegTypeStructByVal;
				if (gr <= PPC_LAST_ARG_REG) {
					int rest = PPC_LAST_ARG_REG - gr + 1;
					int n_in_regs = rest >= nwords? nwords: rest;
					cinfo->args [n].size = n_in_regs;
					cinfo->args [n].vtsize = nwords - n_in_regs;
					cinfo->args [n].reg = gr;
					gr += n_in_regs;
				} else {
					cinfo->args [n].size = 0;
					cinfo->args [n].vtsize = nwords;
				}
				cinfo->args [n].offset = PPC_STACK_PARAM_OFFSET + stack_size;
				/*g_print ("offset for arg %d at %d\n", n, PPC_STACK_PARAM_OFFSET + stack_size);*/
				stack_size += nwords * sizeof (gpointer);
			}
#else
			add_general (&gr, &stack_size, cinfo->args + n, TRUE);
			cinfo->args [n].regtype = RegTypeStructByAddr;
#endif
#endif
			n++;
			break;
		}
		case MONO_TYPE_U8:
		case MONO_TYPE_I8:
		case MONO_TYPE_R8:
			cinfo->args [n].size = 8;
			add_general (&gr, &stack_size, cinfo->args + n, FALSE);
			n++;
			break;
		default:
			g_error ("Can't trampoline 0x%x", sig->params [i]->type);
		}
	}

	{
		simpletype = mono_type_get_underlying_type (sig->ret)->type;
enum_retvalue:
		switch (simpletype) {
		case MONO_TYPE_BOOLEAN:
		case MONO_TYPE_I1:
		case MONO_TYPE_U1:
		case MONO_TYPE_I2:
		case MONO_TYPE_U2:
		case MONO_TYPE_CHAR:
		case MONO_TYPE_I4:
		case MONO_TYPE_U4:
		case MONO_TYPE_I:
		case MONO_TYPE_U:
		case MONO_TYPE_PTR:
		case MONO_TYPE_FNPTR:
		case MONO_TYPE_CLASS:
		case MONO_TYPE_OBJECT:
		case MONO_TYPE_SZARRAY:
		case MONO_TYPE_ARRAY:
		case MONO_TYPE_STRING:
			cinfo->ret.reg = ARMREG_R0;
			break;
		case MONO_TYPE_U8:
		case MONO_TYPE_I8:
			cinfo->ret.reg = ARMREG_R0;
			break;
		case MONO_TYPE_R4:
		case MONO_TYPE_R8:
			cinfo->ret.reg = ARMREG_R0;
			/* FIXME: cinfo->ret.reg = ???;
			cinfo->ret.regtype = RegTypeFP;*/
			break;
		case MONO_TYPE_VALUETYPE:
			break;
		case MONO_TYPE_TYPEDBYREF:
		case MONO_TYPE_VOID:
			break;
		default:
			g_error ("Can't handle as return value 0x%x", sig->ret->type);
		}
	}

	/* align stack size to 8 */
	DEBUG (printf ("      stack size: %d (%d)\n", (stack_size + 15) & ~15, stack_size));
	stack_size = (stack_size + 7) & ~7;

	cinfo->stack_usage = stack_size;
	return cinfo;
}


/*
 * Set var information according to the calling convention. arm version.
 * The locals var stuff should most likely be split in another method.
 */
void
mono_arch_allocate_vars (MonoCompile *m)
{
	MonoMethodSignature *sig;
	MonoMethodHeader *header;
	MonoInst *inst;
	int i, offset, size, align, curinst;
	int frame_reg = ARMREG_SP;


	/* allow room for the vararg method args: void* and long/double */
	if (mono_jit_trace_calls != NULL && mono_trace_eval (m->method))
		m->param_area = MAX (m->param_area, sizeof (gpointer)*8);
	/* this is bug #60332: remove when #59509 is fixed, so no weird vararg 
	 * call convs needs to be handled this way.
	 */
	if (m->flags & MONO_CFG_HAS_VARARGS)
		m->param_area = MAX (m->param_area, sizeof (gpointer)*8);
	/* gtk-sharp and other broken code will dllimport vararg functions even with
	 * non-varargs signatures. Since there is little hope people will get this right
	 * we assume they won't.
	 */
	if (m->method->wrapper_type == MONO_WRAPPER_MANAGED_TO_NATIVE)
		m->param_area = MAX (m->param_area, sizeof (gpointer)*8);

	header = mono_method_get_header (m->method);

	/* 
	 * We use the frame register also for any method that has
	 * exception clauses. This way, when the handlers are called,
	 * the code will reference local variables using the frame reg instead of
	 * the stack pointer: if we had to restore the stack pointer, we'd
	 * corrupt the method frames that are already on the stack (since
	 * filters get called before stack unwinding happens) when the filter
	 * code would call any method (this also applies to finally etc.).
	 */ 
	if ((m->flags & MONO_CFG_HAS_ALLOCA) || header->num_clauses)
		frame_reg = ARMREG_FP;
	m->frame_reg = frame_reg;
	if (frame_reg != ARMREG_SP) {
		m->used_int_regs |= 1 << frame_reg;
	}

	sig = mono_method_signature (m->method);
	
	offset = 0;
	curinst = 0;
	if (MONO_TYPE_ISSTRUCT (sig->ret)) {
		m->ret->opcode = OP_REGVAR;
		m->ret->inst_c0 = ARMREG_R0;
	} else {
		/* FIXME: handle long and FP values */
		switch (mono_type_get_underlying_type (sig->ret)->type) {
		case MONO_TYPE_VOID:
			break;
		default:
			m->ret->opcode = OP_REGVAR;
			m->ret->inst_c0 = ARMREG_R0;
			break;
		}
	}
	/* local vars are at a positive offset from the stack pointer */
	/* 
	 * also note that if the function uses alloca, we use FP
	 * to point at the local variables.
	 */
	offset = 0; /* linkage area */
	/* align the offset to 16 bytes: not sure this is needed here  */
	//offset += 8 - 1;
	//offset &= ~(8 - 1);

	/* add parameter area size for called functions */
	offset += m->param_area;
	offset += 8 - 1;
	offset &= ~(8 - 1);

	/* allow room to save the return value */
	if (mono_jit_trace_calls != NULL && mono_trace_eval (m->method))
		offset += 8;

	/* the MonoLMF structure is stored just below the stack pointer */

	if (sig->call_convention == MONO_CALL_VARARG) {
                m->sig_cookie = 0;
        }

	if (MONO_TYPE_ISSTRUCT (sig->ret)) {
		inst = m->ret;
		offset += sizeof(gpointer) - 1;
		offset &= ~(sizeof(gpointer) - 1);
		inst->inst_offset = offset;
		inst->opcode = OP_REGOFFSET;
		inst->inst_basereg = frame_reg;
		offset += sizeof(gpointer);
		if (sig->call_convention == MONO_CALL_VARARG)
			m->sig_cookie += sizeof (gpointer);
	}

	curinst = m->locals_start;
	for (i = curinst; i < m->num_varinfo; ++i) {
		inst = m->varinfo [i];
		if ((inst->flags & MONO_INST_IS_DEAD) || inst->opcode == OP_REGVAR)
			continue;

		/* inst->unused indicates native sized value types, this is used by the
		* pinvoke wrappers when they call functions returning structure */
		if (inst->unused && MONO_TYPE_ISSTRUCT (inst->inst_vtype) && inst->inst_vtype->type != MONO_TYPE_TYPEDBYREF)
			size = mono_class_native_size (mono_class_from_mono_type (inst->inst_vtype), &align);
		else
			size = mono_type_size (inst->inst_vtype, &align);

		offset += align - 1;
		offset &= ~(align - 1);
		inst->inst_offset = offset;
		inst->opcode = OP_REGOFFSET;
		inst->inst_basereg = frame_reg;
		offset += size;
		//g_print ("allocating local %d to %d\n", i, inst->inst_offset);
	}

	curinst = 0;
	if (sig->hasthis) {
		inst = m->varinfo [curinst];
		if (inst->opcode != OP_REGVAR) {
			inst->opcode = OP_REGOFFSET;
			inst->inst_basereg = frame_reg;
			offset += sizeof (gpointer) - 1;
			offset &= ~(sizeof (gpointer) - 1);
			inst->inst_offset = offset;
			offset += sizeof (gpointer);
			if (sig->call_convention == MONO_CALL_VARARG)
				m->sig_cookie += sizeof (gpointer);
		}
		curinst++;
	}

	for (i = 0; i < sig->param_count; ++i) {
		inst = m->varinfo [curinst];
		if (inst->opcode != OP_REGVAR) {
			inst->opcode = OP_REGOFFSET;
			inst->inst_basereg = frame_reg;
			size = mono_type_size (sig->params [i], &align);
			offset += align - 1;
			offset &= ~(align - 1);
			inst->inst_offset = offset;
			offset += size;
			if ((sig->call_convention == MONO_CALL_VARARG) && (i < sig->sentinelpos)) 
				m->sig_cookie += size;
		}
		curinst++;
	}

	/* align the offset to 8 bytes */
	offset += 8 - 1;
	offset &= ~(8 - 1);

	/* change sign? */
	m->stack_offset = offset;

}

/* Fixme: we need an alignment solution for enter_method and mono_arch_call_opcode,
 * currently alignment in mono_arch_call_opcode is computed without arch_get_argument_info 
 */

/* 
 * take the arguments and generate the arch-specific
 * instructions to properly call the function in call.
 * This includes pushing, moving arguments to the right register
 * etc.
 * Issue: who does the spilling if needed, and when?
 */
MonoCallInst*
mono_arch_call_opcode (MonoCompile *cfg, MonoBasicBlock* bb, MonoCallInst *call, int is_virtual) {
	MonoInst *arg, *in;
	MonoMethodSignature *sig;
	int i, n;
	CallInfo *cinfo;
	ArgInfo *ainfo;

	sig = call->signature;
	n = sig->param_count + sig->hasthis;
	
	cinfo = calculate_sizes (sig, sig->pinvoke);
	if (cinfo->struct_ret)
		call->used_iregs |= 1 << cinfo->struct_ret;

	for (i = 0; i < n; ++i) {
		ainfo = cinfo->args + i;
		if ((sig->call_convention == MONO_CALL_VARARG) && (i == sig->sentinelpos)) {
			MonoInst *sig_arg;
			cfg->disable_aot = TRUE;
				
			MONO_INST_NEW (cfg, sig_arg, OP_ICONST);
			sig_arg->inst_p0 = call->signature;
			
			MONO_INST_NEW (cfg, arg, OP_OUTARG);
			arg->inst_imm = cinfo->sig_cookie.offset;
			arg->inst_left = sig_arg;
			
			/* prepend, so they get reversed */
			arg->next = call->out_args;
			call->out_args = arg;
		}
		if (is_virtual && i == 0) {
			/* the argument will be attached to the call instrucion */
			in = call->args [i];
			call->used_iregs |= 1 << ainfo->reg;
		} else {
			MONO_INST_NEW (cfg, arg, OP_OUTARG);
			in = call->args [i];
			arg->cil_code = in->cil_code;
			arg->inst_left = in;
			arg->inst_right = (MonoInst*)call;
			arg->type = in->type;
			/* prepend, we'll need to reverse them later */
			arg->next = call->out_args;
			call->out_args = arg;
			if (ainfo->regtype == RegTypeGeneral) {
				arg->unused = ainfo->reg;
				call->used_iregs |= 1 << ainfo->reg;
				if (arg->type == STACK_I8)
					call->used_iregs |= 1 << (ainfo->reg + 1);
			} else if (ainfo->regtype == RegTypeStructByAddr) {
				/* FIXME: where si the data allocated? */
				arg->unused = ainfo->reg;
				call->used_iregs |= 1 << ainfo->reg;
			} else if (ainfo->regtype == RegTypeStructByVal) {
				int cur_reg;
				/* mark the used regs */
				for (cur_reg = 0; cur_reg < ainfo->size; ++cur_reg) {
					call->used_iregs |= 1 << (ainfo->reg + cur_reg);
				}
				arg->opcode = OP_OUTARG_VT;
				arg->unused = ainfo->reg | (ainfo->size << 8) | (ainfo->vtsize << 16);
				arg->inst_imm = ainfo->offset;
			} else if (ainfo->regtype == RegTypeBase) {
				arg->opcode = OP_OUTARG_MEMBASE;
				arg->unused = ainfo->reg | (ainfo->size << 8);
				arg->inst_imm = ainfo->offset;
			} else if (ainfo->regtype == RegTypeFP) {
				arg->opcode = OP_OUTARG_R8;
				arg->unused = ainfo->reg;
				call->used_fregs |= 1 << ainfo->reg;
				if (ainfo->size == 4) {
					arg->opcode = OP_OUTARG_R8;
					/* we reduce the precision */
					/*MonoInst *conv;
					MONO_INST_NEW (cfg, conv, OP_FCONV_TO_R4);
					conv->inst_left = arg->inst_left;
					arg->inst_left = conv;*/
				}
			} else {
				g_assert_not_reached ();
			}
		}
	}
	/*
	 * Reverse the call->out_args list.
	 */
	{
		MonoInst *prev = NULL, *list = call->out_args, *next;
		while (list) {
			next = list->next;
			list->next = prev;
			prev = list;
			list = next;
		}
		call->out_args = prev;
	}
	call->stack_usage = cinfo->stack_usage;
	cfg->param_area = MAX (cfg->param_area, cinfo->stack_usage);
	cfg->flags |= MONO_CFG_HAS_CALLS;
	/* 
	 * should set more info in call, such as the stack space
	 * used by the args that needs to be added back to esp
	 */

	g_free (cinfo);
	return call;
}

/*
 * Allow tracing to work with this interface (with an optional argument)
 */

void*
mono_arch_instrument_prolog (MonoCompile *cfg, void *func, void *p, gboolean enable_arguments)
{
	guchar *code = p;

	code = mono_arm_emit_load_imm (code, ARMREG_R0, (guint32)cfg->method);
	ARM_MOV_REG_IMM8 (code, ARMREG_R1, 0); /* NULL ebp for now */
	code = mono_arm_emit_load_imm (code, ARMREG_R2, (guint32)func);
	ARM_MOV_REG_REG (code, ARMREG_LR, ARMREG_PC);
	ARM_MOV_REG_REG (code, ARMREG_PC, ARMREG_R2);
	return code;
}

enum {
	SAVE_NONE,
	SAVE_STRUCT,
	SAVE_ONE,
	SAVE_TWO,
	SAVE_FP
};

void*
mono_arch_instrument_epilog (MonoCompile *cfg, void *func, void *p, gboolean enable_arguments)
{
	guchar *code = p;
	int save_mode = SAVE_NONE;
	int offset;
	MonoMethod *method = cfg->method;
	int rtype = mono_type_get_underlying_type (mono_method_signature (method)->ret)->type;
	int save_offset = cfg->param_area;
	save_offset += 7;
	save_offset &= ~7;
	
	offset = code - cfg->native_code;
	/* we need about 16 instructions */
	if (offset > (cfg->code_size - 16 * 4)) {
		cfg->code_size *= 2;
		cfg->native_code = g_realloc (cfg->native_code, cfg->code_size);
		code = cfg->native_code + offset;
	}
handle_enum:
	switch (rtype) {
	case MONO_TYPE_VOID:
		/* special case string .ctor icall */
		if (strcmp (".ctor", method->name) && method->klass == mono_defaults.string_class)
			save_mode = SAVE_ONE;
		else
			save_mode = SAVE_NONE;
		break;
	case MONO_TYPE_I8:
	case MONO_TYPE_U8:
		save_mode = SAVE_TWO;
		break;
	case MONO_TYPE_R4:
	case MONO_TYPE_R8:
		save_mode = SAVE_FP;
		break;
	case MONO_TYPE_VALUETYPE:
		save_mode = SAVE_STRUCT;
		break;
	default:
		save_mode = SAVE_ONE;
		break;
	}

	switch (save_mode) {
	case SAVE_TWO:
		ARM_STR_IMM (code, ARMREG_R0, cfg->frame_reg, save_offset);
		ARM_STR_IMM (code, ARMREG_R1, cfg->frame_reg, save_offset + 4);
		if (enable_arguments) {
			ARM_MOV_REG_REG (code, ARMREG_R2, ARMREG_R1);
			ARM_MOV_REG_REG (code, ARMREG_R1, ARMREG_R0);
		}
		break;
	case SAVE_ONE:
		ARM_STR_IMM (code, ARMREG_R0, cfg->frame_reg, save_offset);
		if (enable_arguments) {
			ARM_MOV_REG_REG (code, ARMREG_R1, ARMREG_R0);
		}
		break;
	case SAVE_FP:
		/* FIXME: what reg?  */
		if (enable_arguments) {
			/* FIXME: what reg?  */
		}
		break;
	case SAVE_STRUCT:
		if (enable_arguments) {
			/* FIXME: get the actual address  */
			ARM_MOV_REG_REG (code, ARMREG_R1, ARMREG_R0);
		}
		break;
	case SAVE_NONE:
	default:
		break;
	}

	code = mono_arm_emit_load_imm (code, ARMREG_R0, (guint32)cfg->method);
	code = mono_arm_emit_load_imm (code, ARMREG_IP, (guint32)func);
	ARM_MOV_REG_REG (code, ARMREG_LR, ARMREG_PC);
	ARM_MOV_REG_REG (code, ARMREG_PC, ARMREG_IP);

	switch (save_mode) {
	case SAVE_TWO:
		ARM_LDR_IMM (code, ARMREG_R0, cfg->frame_reg, save_offset);
		ARM_LDR_IMM (code, ARMREG_R1, cfg->frame_reg, save_offset + 4);
		break;
	case SAVE_ONE:
		ARM_LDR_IMM (code, ARMREG_R0, cfg->frame_reg, save_offset);
		break;
	case SAVE_FP:
		/* FIXME */
		break;
	case SAVE_NONE:
	default:
		break;
	}

	return code;
}

/*
 * The immediate field for cond branches is big enough for all reasonable methods
 */
#define EMIT_COND_BRANCH_FLAGS(ins,condcode) \
if (ins->flags & MONO_INST_BRLABEL) { \
        if (0 && ins->inst_i0->inst_c0) { \
		ARM_B_COND (code, (condcode), (code - cfg->native_code + ins->inst_i0->inst_c0) & 0xffffff);	\
        } else { \
	        mono_add_patch_info (cfg, code - cfg->native_code, MONO_PATCH_INFO_LABEL, ins->inst_i0); \
		ARM_B_COND (code, (condcode), 0);	\
        } \
} else { \
        if (0 && ins->inst_true_bb->native_offset) { \
		ARM_B_COND (code, (condcode), (code - cfg->native_code + ins->inst_true_bb->native_offset) & 0xffffff); \
        } else { \
		mono_add_patch_info (cfg, code - cfg->native_code, MONO_PATCH_INFO_BB, ins->inst_true_bb); \
		ARM_B_COND (code, (condcode), 0);	\
        } \
}

#define EMIT_COND_BRANCH(ins,cond) EMIT_COND_BRANCH_FLAGS(ins, branch_cc_table [(cond)])

/* emit an exception if condition is fail
 *
 * We assign the extra code used to throw the implicit exceptions
 * to cfg->bb_exit as far as the big branch handling is concerned
 */
#define EMIT_COND_SYSTEM_EXCEPTION_FLAGS(condcode,exc_name)            \
        do {                                                        \
		mono_add_patch_info (cfg, code - cfg->native_code,   \
				    MONO_PATCH_INFO_EXC, exc_name);  \
		ARM_BL_COND (code, (condcode), 0);	\
	} while (0); 

#define EMIT_COND_SYSTEM_EXCEPTION(cond,exc_name) EMIT_COND_SYSTEM_EXCEPTION_FLAGS(branch_cc_table [(cond)], (exc_name))

static void
peephole_pass (MonoCompile *cfg, MonoBasicBlock *bb)
{
	MonoInst *ins, *last_ins = NULL;
	ins = bb->code;

	while (ins) {

		switch (ins->opcode) {
		case OP_MUL_IMM: 
			/* remove unnecessary multiplication with 1 */
			if (ins->inst_imm == 1) {
				if (ins->dreg != ins->sreg1) {
					ins->opcode = OP_MOVE;
				} else {
					last_ins->next = ins->next;				
					ins = ins->next;				
					continue;
				}
			} else {
				int power2 = mono_is_power_of_two (ins->inst_imm);
				if (power2 > 0) {
					ins->opcode = OP_SHL_IMM;
					ins->inst_imm = power2;
				}
			}
			break;
		case OP_LOAD_MEMBASE:
		case OP_LOADI4_MEMBASE:
			/* 
			 * OP_STORE_MEMBASE_REG reg, offset(basereg) 
			 * OP_LOAD_MEMBASE offset(basereg), reg
			 */
			if (last_ins && (last_ins->opcode == OP_STOREI4_MEMBASE_REG 
					 || last_ins->opcode == OP_STORE_MEMBASE_REG) &&
			    ins->inst_basereg == last_ins->inst_destbasereg &&
			    ins->inst_offset == last_ins->inst_offset) {
				if (ins->dreg == last_ins->sreg1) {
					last_ins->next = ins->next;				
					ins = ins->next;				
					continue;
				} else {
					//static int c = 0; printf ("MATCHX %s %d\n", cfg->method->name,c++);
					ins->opcode = OP_MOVE;
					ins->sreg1 = last_ins->sreg1;
				}

			/* 
			 * Note: reg1 must be different from the basereg in the second load
			 * OP_LOAD_MEMBASE offset(basereg), reg1
			 * OP_LOAD_MEMBASE offset(basereg), reg2
			 * -->
			 * OP_LOAD_MEMBASE offset(basereg), reg1
			 * OP_MOVE reg1, reg2
			 */
			} if (last_ins && (last_ins->opcode == OP_LOADI4_MEMBASE
					   || last_ins->opcode == OP_LOAD_MEMBASE) &&
			      ins->inst_basereg != last_ins->dreg &&
			      ins->inst_basereg == last_ins->inst_basereg &&
			      ins->inst_offset == last_ins->inst_offset) {

				if (ins->dreg == last_ins->dreg) {
					last_ins->next = ins->next;				
					ins = ins->next;				
					continue;
				} else {
					ins->opcode = OP_MOVE;
					ins->sreg1 = last_ins->dreg;
				}

				//g_assert_not_reached ();

#if 0
			/* 
			 * OP_STORE_MEMBASE_IMM imm, offset(basereg) 
			 * OP_LOAD_MEMBASE offset(basereg), reg
			 * -->
			 * OP_STORE_MEMBASE_IMM imm, offset(basereg) 
			 * OP_ICONST reg, imm
			 */
			} else if (last_ins && (last_ins->opcode == OP_STOREI4_MEMBASE_IMM
						|| last_ins->opcode == OP_STORE_MEMBASE_IMM) &&
				   ins->inst_basereg == last_ins->inst_destbasereg &&
				   ins->inst_offset == last_ins->inst_offset) {
				//static int c = 0; printf ("MATCHX %s %d\n", cfg->method->name,c++);
				ins->opcode = OP_ICONST;
				ins->inst_c0 = last_ins->inst_imm;
				g_assert_not_reached (); // check this rule
#endif
			}
			break;
		case OP_LOADU1_MEMBASE:
		case OP_LOADI1_MEMBASE:
			if (last_ins && (last_ins->opcode == OP_STOREI1_MEMBASE_REG) &&
					ins->inst_basereg == last_ins->inst_destbasereg &&
					ins->inst_offset == last_ins->inst_offset) {
				if (ins->dreg == last_ins->sreg1) {
					last_ins->next = ins->next;				
					ins = ins->next;				
					continue;
				} else {
					//static int c = 0; printf ("MATCHX %s %d\n", cfg->method->name,c++);
					ins->opcode = OP_MOVE;
					ins->sreg1 = last_ins->sreg1;
				}
			}
			break;
		case OP_LOADU2_MEMBASE:
		case OP_LOADI2_MEMBASE:
			if (last_ins && (last_ins->opcode == OP_STOREI2_MEMBASE_REG) &&
					ins->inst_basereg == last_ins->inst_destbasereg &&
					ins->inst_offset == last_ins->inst_offset) {
				if (ins->dreg == last_ins->sreg1) {
					last_ins->next = ins->next;				
					ins = ins->next;				
					continue;
				} else {
					//static int c = 0; printf ("MATCHX %s %d\n", cfg->method->name,c++);
					ins->opcode = OP_MOVE;
					ins->sreg1 = last_ins->sreg1;
				}
			}
			break;
		case CEE_CONV_I4:
		case CEE_CONV_U4:
		case OP_MOVE:
		case OP_SETREG:
			ins->opcode = OP_MOVE;
			/* 
			 * OP_MOVE reg, reg 
			 */
			if (ins->dreg == ins->sreg1) {
				if (last_ins)
					last_ins->next = ins->next;				
				ins = ins->next;
				continue;
			}
			/* 
			 * OP_MOVE sreg, dreg 
			 * OP_MOVE dreg, sreg
			 */
			if (last_ins && last_ins->opcode == OP_MOVE &&
			    ins->sreg1 == last_ins->dreg &&
			    ins->dreg == last_ins->sreg1) {
				last_ins->next = ins->next;				
				ins = ins->next;				
				continue;
			}
			break;
		}
		last_ins = ins;
		ins = ins->next;
	}
	bb->last_ins = last_ins;
}

/* 
 * the branch_cc_table should maintain the order of these
 * opcodes.
case CEE_BEQ:
case CEE_BGE:
case CEE_BGT:
case CEE_BLE:
case CEE_BLT:
case CEE_BNE_UN:
case CEE_BGE_UN:
case CEE_BGT_UN:
case CEE_BLE_UN:
case CEE_BLT_UN:
 */
static const guchar 
branch_cc_table [] = {
	ARMCOND_EQ, 
	ARMCOND_GE, 
	ARMCOND_GT, 
	ARMCOND_LE,
	ARMCOND_LT, 
	
	ARMCOND_NE, 
	ARMCOND_HS, 
	ARMCOND_HI, 
	ARMCOND_LS,
	ARMCOND_LO
};


static void
insert_after_ins (MonoBasicBlock *bb, MonoInst *ins, MonoInst *to_insert)
{
	if (ins == NULL) {
		ins = bb->code;
		bb->code = to_insert;
		to_insert->next = ins;
	} else {
		to_insert->next = ins->next;
		ins->next = to_insert;
	}
}

#define NEW_INS(cfg,dest,op) do {       \
		(dest) = mono_mempool_alloc0 ((cfg)->mempool, sizeof (MonoInst));       \
		(dest)->opcode = (op);  \
		insert_after_ins (bb, last_ins, (dest)); \
	} while (0)

static int
map_to_reg_reg_op (int op)
{
	switch (op) {
	case OP_ADD_IMM:
		return CEE_ADD;
	case OP_SUB_IMM:
		return CEE_SUB;
	case OP_AND_IMM:
		return CEE_AND;
	case OP_COMPARE_IMM:
		return OP_COMPARE;
	case OP_ADDCC_IMM:
		return OP_ADDCC;
	case OP_ADC_IMM:
		return OP_ADC;
	case OP_SUBCC_IMM:
		return OP_SUBCC;
	case OP_SBB_IMM:
		return OP_SBB;
	case OP_OR_IMM:
		return CEE_OR;
	case OP_XOR_IMM:
		return CEE_XOR;
	}
	g_assert_not_reached ();
}

/*
 * Remove from the instruction list the instructions that can't be
 * represented with very simple instructions with no register
 * requirements.
 */
static void
mono_arch_lowering_pass (MonoCompile *cfg, MonoBasicBlock *bb)
{
	MonoInst *ins, *next, *temp, *last_ins = NULL;
	int rot_amount, imm8;

	/* setup the virtual reg allocator */
	if (bb->max_ireg > cfg->rs->next_vireg)
		cfg->rs->next_vireg = bb->max_ireg;

	ins = bb->code;
	while (ins) {
		switch (ins->opcode) {
		case OP_ADD_IMM:
		case OP_SUB_IMM:
		case OP_AND_IMM:
		case OP_COMPARE_IMM:
		case OP_ADDCC_IMM:
		case OP_ADC_IMM:
		case OP_SUBCC_IMM:
		case OP_SBB_IMM:
		case OP_OR_IMM:
		case OP_XOR_IMM:
			if ((imm8 = mono_arm_is_rotated_imm8 (ins->inst_imm, &rot_amount)) < 0) {
				NEW_INS (cfg, temp, OP_ICONST);
				temp->inst_c0 = ins->inst_imm;
				temp->dreg = mono_regstate_next_int (cfg->rs);
				ins->sreg2 = temp->dreg;
				ins->opcode = map_to_reg_reg_op (ins->opcode);
			}
			break;
		case OP_MUL_IMM:
			NEW_INS (cfg, temp, OP_ICONST);
			temp->inst_c0 = ins->inst_imm;
			temp->dreg = mono_regstate_next_int (cfg->rs);
			ins->sreg2 = temp->dreg;
			ins->opcode = CEE_MUL;
			break;
		}
		last_ins = ins;
		ins = ins->next;
	}
	bb->last_ins = last_ins;
	bb->max_ireg = cfg->rs->next_vireg;

}

void
mono_arch_local_regalloc (MonoCompile *cfg, MonoBasicBlock *bb)
{
	if (!bb->code)
		return;
	mono_arch_lowering_pass (cfg, bb);
	mono_local_regalloc (cfg, bb);
}

static guchar*
emit_float_to_int (MonoCompile *cfg, guchar *code, int dreg, int sreg, int size, gboolean is_signed)
{
	/* sreg is a float, dreg is an integer reg  */
	ARM_FIXZ (code, dreg, sreg);
	if (!is_signed) {
		if (size == 1)
			ARM_AND_REG_IMM8 (code, dreg, dreg, 0xff);
		else if (size == 2) {
			ARM_SHL_IMM (code, dreg, dreg, 16);
			ARM_SHR_IMM (code, dreg, dreg, 16);
		}
	} else {
		if (size == 1) {
			ARM_SHL_IMM (code, dreg, dreg, 24);
			ARM_SAR_IMM (code, dreg, dreg, 24);
		} else if (size == 2) {
			ARM_SHL_IMM (code, dreg, dreg, 16);
			ARM_SAR_IMM (code, dreg, dreg, 16);
		}
	}
	return code;
}

typedef struct {
	guchar *code;
	guchar *target;
	int absolute;
	int found;
} PatchData;

#define is_call_imm(diff) ((gint)(diff) >= -33554432 && (gint)(diff) <= 33554431)

static int
search_thunk_slot (void *data, int csize, int bsize, void *user_data) {
	PatchData *pdata = (PatchData*)user_data;
	guchar *code = data;
	guint32 *thunks = data;
	guint32 *endthunks = (guint32*)(code + bsize);
	int i, count = 0;
	int difflow, diffhigh;

	/* always ensure a call from pdata->code can reach to the thunks without further thunks */
	difflow = (char*)pdata->code - (char*)thunks;
	diffhigh = (char*)pdata->code - (char*)endthunks;
	if (!((is_call_imm (thunks) && is_call_imm (endthunks)) || (is_call_imm (difflow) && is_call_imm (diffhigh))))
		return 0;

	/*
	 * The thunk is composed of 3 words:
	 * load constant from thunks [2] into ARM_IP
	 * bx to ARM_IP
	 * address constant
	 * Note that the LR register is already setup
	 */
	//g_print ("thunk nentries: %d\n", ((char*)endthunks - (char*)thunks)/16);
	if ((pdata->found == 2) || (pdata->code >= code && pdata->code <= code + csize)) {
		while (thunks < endthunks) {
			//g_print ("looking for target: %p at %p (%08x-%08x)\n", pdata->target, thunks, thunks [0], thunks [1]);
			if (thunks [2] == (guint32)pdata->target) {
				arm_patch (pdata->code, (guchar*)thunks);
				mono_arch_flush_icache (pdata->code, 4);
				pdata->found = 1;
				return 1;
			} else if ((thunks [0] == 0) && (thunks [1] == 0) && (thunks [2] == 0)) {
				/* found a free slot instead: emit thunk */
				code = (guchar*)thunks;
				ARM_LDR_IMM (code, ARMREG_IP, ARMREG_PC, 0);
				ARM_MOV_REG_REG (code, ARMREG_PC, ARMREG_IP);
				thunks [2] = (guint32)pdata->target;
				mono_arch_flush_icache ((guchar*)thunks, 12);

				arm_patch (pdata->code, (guchar*)thunks);
				mono_arch_flush_icache (pdata->code, 4);
				pdata->found = 1;
				return 1;
			}
			/* skip 12 bytes, the size of the thunk */
			thunks += 3;
			count++;
		}
		//g_print ("failed thunk lookup for %p from %p at %p (%d entries)\n", pdata->target, pdata->code, data, count);
	}
	return 0;
}

static void
handle_thunk (int absolute, guchar *code, guchar *target) {
	MonoDomain *domain = mono_domain_get ();
	PatchData pdata;

	pdata.code = code;
	pdata.target = target;
	pdata.absolute = absolute;
	pdata.found = 0;

	mono_domain_lock (domain);
	mono_code_manager_foreach (domain->code_mp, search_thunk_slot, &pdata);

	if (!pdata.found) {
		/* this uses the first available slot */
		pdata.found = 2;
		mono_code_manager_foreach (domain->code_mp, search_thunk_slot, &pdata);
	}
	mono_domain_unlock (domain);

	if (pdata.found != 1)
		g_print ("thunk failed for %p from %p\n", target, code);
	g_assert (pdata.found == 1);
}

void
arm_patch (guchar *code, guchar *target)
{
	guint32 ins = *(guint32*)code;
	guint32 prim = (ins >> 25) & 7;
	guint32 ovf;

	//g_print ("patching 0x%08x (0x%08x) to point to 0x%08x\n", code, ins, target);
	if (prim == 5) { /* 101b */
		/* the diff starts 8 bytes from the branch opcode */
		gint diff = target - code - 8;
		if (diff >= 0) {
			if (diff <= 33554431) {
				diff >>= 2;
				ins = (ins & 0xff000000) | diff;
				*(guint32*)code = ins;
				return;
			}
		} else {
			/* diff between 0 and -33554432 */
			if (diff >= -33554432) {
				diff >>= 2;
				ins = (ins & 0xff000000) | (diff & ~0xff000000);
				*(guint32*)code = ins;
				return;
			}
		}
		
		handle_thunk (TRUE, code, target);
		return;
	}


	if ((ins & 0x0ffffff0) == 0x12fff10) {
		/* branch and exchange: the address is constructed in a reg */
		g_assert_not_reached ();
	} else {
		g_assert_not_reached ();
	}
//	g_print ("patched with 0x%08x\n", ins);
}

/* 
 * Return the >= 0 uimm8 value if val can be represented with a byte + rotation
 * (with the rotation amount in *rot_amount. rot_amount is already adjusted
 * to be used with the emit macros.
 * Return -1 otherwise.
 */
int
mono_arm_is_rotated_imm8 (guint32 val, gint *rot_amount)
{
	guint32 res, i;
	for (i = 0; i < 31; i+= 2) {
		res = (val << (32 - i)) | (val >> i);
		if (res & ~0xff)
			continue;
		*rot_amount = i? 32 - i: 0;
		return res;
	}
	return -1;
}

/*
 * Emits in code a sequence of instructions that load the value 'val'
 * into the dreg register. Uses at most 4 instructions.
 */
guint8*
mono_arm_emit_load_imm (guint8 *code, int dreg, guint32 val)
{
	int imm8, rot_amount;
	if ((imm8 = mono_arm_is_rotated_imm8 (val, &rot_amount)) >= 0) {
		ARM_MOV_REG_IMM (code, dreg, imm8, rot_amount);
	} else if ((imm8 = mono_arm_is_rotated_imm8 (~val, &rot_amount)) >= 0) {
		ARM_MVN_REG_IMM (code, dreg, imm8, rot_amount);
	} else {
		if (val & 0xFF) {
			ARM_MOV_REG_IMM8 (code, dreg, (val & 0xFF));
			if (val & 0xFF00) {
				ARM_ADD_REG_IMM (code, dreg, dreg, (val & 0xFF00) >> 8, 24);
			}
			if (val & 0xFF0000) {
				ARM_ADD_REG_IMM (code, dreg, dreg, (val & 0xFF0000) >> 16, 16);
			}
			if (val & 0xFF000000) {
				ARM_ADD_REG_IMM (code, dreg, dreg, (val & 0xFF000000) >> 24, 8);
			}
		} else if (val & 0xFF00) {
			ARM_MOV_REG_IMM (code, dreg, (val & 0xFF00) >> 8, 24);
			if (val & 0xFF0000) {
				ARM_ADD_REG_IMM (code, dreg, dreg, (val & 0xFF0000) >> 16, 16);
			}
			if (val & 0xFF000000) {
				ARM_ADD_REG_IMM (code, dreg, dreg, (val & 0xFF000000) >> 24, 8);
			}
		} else if (val & 0xFF0000) {
			ARM_MOV_REG_IMM (code, dreg, (val & 0xFF0000) >> 16, 16);
			if (val & 0xFF000000) {
				ARM_ADD_REG_IMM (code, dreg, dreg, (val & 0xFF000000) >> 24, 8);
			}
		}
		//g_assert_not_reached ();
	}
	return code;
}

void
mono_arch_output_basic_block (MonoCompile *cfg, MonoBasicBlock *bb)
{
	MonoInst *ins;
	MonoCallInst *call;
	guint offset;
	guint8 *code = cfg->native_code + cfg->code_len;
	MonoInst *last_ins = NULL;
	guint last_offset = 0;
	int max_len, cpos;
	int imm8, rot_amount;

	if (cfg->opt & MONO_OPT_PEEPHOLE)
		peephole_pass (cfg, bb);

	/* we don't align basic blocks of loops on arm */

	if (cfg->verbose_level > 2)
		g_print ("Basic block %d starting at offset 0x%x\n", bb->block_num, bb->native_offset);

	cpos = bb->max_offset;

	if (cfg->prof_options & MONO_PROFILE_COVERAGE) {
		//MonoCoverageInfo *cov = mono_get_coverage_info (cfg->method);
		//g_assert (!mono_compile_aot);
		//cpos += 6;
		//if (bb->cil_code)
		//	cov->data [bb->dfn].iloffset = bb->cil_code - cfg->cil_code;
		/* this is not thread save, but good enough */
		/* fixme: howto handle overflows? */
		//x86_inc_mem (code, &cov->data [bb->dfn].count); 
	}

	ins = bb->code;
	while (ins) {
		offset = code - cfg->native_code;

		max_len = ((guint8 *)arm_cpu_desc [ins->opcode])[MONO_INST_LEN];

		if (offset > (cfg->code_size - max_len - 16)) {
			cfg->code_size *= 2;
			cfg->native_code = g_realloc (cfg->native_code, cfg->code_size);
			code = cfg->native_code + offset;
		}
	//	if (ins->cil_code)
	//		g_print ("cil code\n");
		mono_debug_record_line_number (cfg, ins, offset);

		switch (ins->opcode) {
		case OP_TLS_GET:
			g_assert_not_reached ();
			break;
		/*case OP_BIGMUL:
			ppc_mullw (code, ppc_r4, ins->sreg1, ins->sreg2);
			ppc_mulhw (code, ppc_r3, ins->sreg1, ins->sreg2);
			break;
		case OP_BIGMUL_UN:
			ppc_mullw (code, ppc_r4, ins->sreg1, ins->sreg2);
			ppc_mulhwu (code, ppc_r3, ins->sreg1, ins->sreg2);
			break;*/
		case OP_STOREI1_MEMBASE_IMM:
			code = mono_arm_emit_load_imm (code, ARMREG_LR, ins->inst_imm & 0xFF);
			g_assert (arm_is_imm12 (ins->inst_offset));
			ARM_STRB_IMM (code, ARMREG_LR, ins->inst_destbasereg, ins->inst_offset);
			break;
		case OP_STOREI2_MEMBASE_IMM:
			code = mono_arm_emit_load_imm (code, ARMREG_LR, ins->inst_imm & 0xFFFF);
			g_assert (arm_is_imm8 (ins->inst_offset));
			ARM_STRH_IMM (code, ARMREG_LR, ins->inst_destbasereg, ins->inst_offset);
			break;
		case OP_STORE_MEMBASE_IMM:
		case OP_STOREI4_MEMBASE_IMM:
			code = mono_arm_emit_load_imm (code, ARMREG_LR, ins->inst_imm);
			g_assert (arm_is_imm12 (ins->inst_offset));
			ARM_STR_IMM (code, ARMREG_LR, ins->inst_destbasereg, ins->inst_offset);
			break;
		case OP_STOREI1_MEMBASE_REG:
			g_assert (arm_is_imm12 (ins->inst_offset));
			ARM_STRB_IMM (code, ins->sreg1, ins->inst_destbasereg, ins->inst_offset);
			break;
		case OP_STOREI2_MEMBASE_REG:
			g_assert (arm_is_imm8 (ins->inst_offset));
			ARM_STRH_IMM (code, ins->sreg1, ins->inst_destbasereg, ins->inst_offset);
			break;
		case OP_STORE_MEMBASE_REG:
		case OP_STOREI4_MEMBASE_REG:
			g_assert (arm_is_imm12 (ins->inst_offset));
			ARM_STR_IMM (code, ins->sreg1, ins->inst_destbasereg, ins->inst_offset);
			break;
		case CEE_LDIND_I:
		case CEE_LDIND_I4:
		case CEE_LDIND_U4:
			g_assert_not_reached ();
			break;
		case OP_LOADU4_MEM:
			g_assert_not_reached ();
			break;
		case OP_LOAD_MEMBASE:
		case OP_LOADI4_MEMBASE:
		case OP_LOADU4_MEMBASE:
			g_assert (arm_is_imm12 (ins->inst_offset));
			ARM_LDR_IMM (code, ins->dreg, ins->inst_basereg, ins->inst_offset);
			break;
		case OP_LOADI1_MEMBASE:
			g_assert (arm_is_imm8 (ins->inst_offset));
			ARM_LDRSB_IMM (code, ins->dreg, ins->inst_basereg, ins->inst_offset);
			break;
		case OP_LOADU1_MEMBASE:
			g_assert (arm_is_imm12 (ins->inst_offset));
			ARM_LDRB_IMM (code, ins->dreg, ins->inst_basereg, ins->inst_offset);
			break;
		case OP_LOADU2_MEMBASE:
			g_assert (arm_is_imm8 (ins->inst_offset));
			ARM_LDRH_IMM (code, ins->dreg, ins->inst_basereg, ins->inst_offset);
			break;
		case OP_LOADI2_MEMBASE:
			g_assert (arm_is_imm8 (ins->inst_offset));
			ARM_LDRSH_IMM (code, ins->dreg, ins->inst_basereg, ins->inst_offset);
			break;
		case CEE_CONV_I1:
			ARM_SHL_IMM (code, ins->dreg, ins->sreg1, 24);
			ARM_SAR_IMM (code, ins->dreg, ins->dreg, 24);
			break;
		case CEE_CONV_I2:
			ARM_SHL_IMM (code, ins->dreg, ins->sreg1, 16);
			ARM_SAR_IMM (code, ins->dreg, ins->dreg, 16);
			break;
		case CEE_CONV_U1:
			ARM_AND_REG_IMM8 (code, ins->dreg, ins->sreg1, 0xff);
			break;
		case CEE_CONV_U2:
			ARM_SHL_IMM (code, ins->dreg, ins->sreg1, 16);
			ARM_SHR_IMM (code, ins->dreg, ins->dreg, 16);
			break;
		case OP_COMPARE:
			ARM_CMP_REG_REG (code, ins->sreg1, ins->sreg2);
			break;
		case OP_COMPARE_IMM:
			imm8 = mono_arm_is_rotated_imm8 (ins->inst_imm, &rot_amount);
			g_assert (imm8 >= 0);
			ARM_CMP_REG_IMM (code, ins->sreg1, imm8, rot_amount);
			break;
		case OP_X86_TEST_NULL:
			g_assert_not_reached ();
			break;
		case CEE_BREAK:
			ARM_DBRK (code);
			break;
		case OP_ADDCC:
			ARM_ADDS_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;
		case CEE_ADD:
			ARM_ADD_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;
		case OP_ADC:
			ARM_ADCS_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;
		case OP_ADDCC_IMM:
			imm8 = mono_arm_is_rotated_imm8 (ins->inst_imm, &rot_amount);
			g_assert (imm8 >= 0);
			ARM_ADDS_REG_IMM (code, ins->dreg, ins->sreg1, imm8, rot_amount);
			break;
		case OP_ADD_IMM:
			imm8 = mono_arm_is_rotated_imm8 (ins->inst_imm, &rot_amount);
			g_assert (imm8 >= 0);
			ARM_ADD_REG_IMM (code, ins->dreg, ins->sreg1, imm8, rot_amount);
			break;
		case OP_ADC_IMM:
			imm8 = mono_arm_is_rotated_imm8 (ins->inst_imm, &rot_amount);
			g_assert (imm8 >= 0);
			ARM_ADCS_REG_IMM (code, ins->dreg, ins->sreg1, imm8, rot_amount);
			break;
		case CEE_ADD_OVF:
			ARM_ADD_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			//EMIT_COND_SYSTEM_EXCEPTION_FLAGS (PPC_BR_FALSE, PPC_BR_EQ, "OverflowException");
			break;
		case CEE_ADD_OVF_UN:
			ARM_ADD_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			//EMIT_COND_SYSTEM_EXCEPTION_FLAGS (PPC_BR_FALSE, PPC_BR_EQ, "OverflowException");
			break;
		case CEE_SUB_OVF:
			ARM_SUB_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			//EMIT_COND_SYSTEM_EXCEPTION_FLAGS (PPC_BR_FALSE, PPC_BR_EQ, "OverflowException");
			break;
		case CEE_SUB_OVF_UN:
			ARM_SUB_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			//EMIT_COND_SYSTEM_EXCEPTION_FLAGS (PPC_BR_TRUE, PPC_BR_EQ, "OverflowException");
			break;
		case OP_ADD_OVF_CARRY:
			ARM_ADCS_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			//EMIT_COND_SYSTEM_EXCEPTION_FLAGS (PPC_BR_FALSE, PPC_BR_EQ, "OverflowException");
			break;
		case OP_ADD_OVF_UN_CARRY:
			ARM_ADCS_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			//EMIT_COND_SYSTEM_EXCEPTION_FLAGS (PPC_BR_FALSE, PPC_BR_EQ, "OverflowException");
			break;
		case OP_SUB_OVF_CARRY:
			ARM_SBCS_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			//EMIT_COND_SYSTEM_EXCEPTION_FLAGS (PPC_BR_FALSE, PPC_BR_EQ, "OverflowException");
			break;
		case OP_SUB_OVF_UN_CARRY:
			ARM_SBCS_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			//EMIT_COND_SYSTEM_EXCEPTION_FLAGS (PPC_BR_TRUE, PPC_BR_EQ, "OverflowException");
			break;
		case OP_SUBCC:
			ARM_SUBS_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;
		case OP_SUBCC_IMM:
			imm8 = mono_arm_is_rotated_imm8 (ins->inst_imm, &rot_amount);
			g_assert (imm8 >= 0);
			ARM_SUBS_REG_IMM (code, ins->dreg, ins->sreg1, imm8, rot_amount);
			break;
		case CEE_SUB:
			ARM_SUB_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;
		case OP_SBB:
			ARM_SBCS_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;
		case OP_SUB_IMM:
			imm8 = mono_arm_is_rotated_imm8 (ins->inst_imm, &rot_amount);
			g_assert (imm8 >= 0);
			ARM_SUB_REG_IMM (code, ins->dreg, ins->sreg1, imm8, rot_amount);
			break;
		case OP_SBB_IMM:
			imm8 = mono_arm_is_rotated_imm8 (ins->inst_imm, &rot_amount);
			g_assert (imm8 >= 0);
			ARM_SBCS_REG_IMM (code, ins->dreg, ins->sreg1, imm8, rot_amount);
			break;
		case OP_ARM_RSBS_IMM:
			imm8 = mono_arm_is_rotated_imm8 (ins->inst_imm, &rot_amount);
			g_assert (imm8 >= 0);
			ARM_RSBS_REG_IMM (code, ins->dreg, ins->sreg1, imm8, rot_amount);
			break;
		case OP_ARM_RSC_IMM:
			imm8 = mono_arm_is_rotated_imm8 (ins->inst_imm, &rot_amount);
			g_assert (imm8 >= 0);
			ARM_RSC_REG_IMM (code, ins->dreg, ins->sreg1, imm8, rot_amount);
			break;
		case CEE_AND:
			ARM_AND_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;
		case OP_AND_IMM:
			imm8 = mono_arm_is_rotated_imm8 (ins->inst_imm, &rot_amount);
			g_assert (imm8 >= 0);
			ARM_AND_REG_IMM (code, ins->dreg, ins->sreg1, imm8, rot_amount);
			break;
		case CEE_DIV:
		case CEE_DIV_UN:
		case OP_DIV_IMM:
		case CEE_REM:
		case CEE_REM_UN:
		case OP_REM_IMM:
			/* crappy ARM arch doesn't have a DIV instruction */
			g_assert_not_reached ();
		case CEE_OR:
			ARM_ORR_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;
		case OP_OR_IMM:
			imm8 = mono_arm_is_rotated_imm8 (ins->inst_imm, &rot_amount);
			g_assert (imm8 >= 0);
			ARM_ORR_REG_IMM (code, ins->dreg, ins->sreg1, imm8, rot_amount);
			break;
		case CEE_XOR:
			ARM_EOR_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;
		case OP_XOR_IMM:
			imm8 = mono_arm_is_rotated_imm8 (ins->inst_imm, &rot_amount);
			g_assert (imm8 >= 0);
			ARM_EOR_REG_IMM (code, ins->dreg, ins->sreg1, imm8, rot_amount);
			break;
		case CEE_SHL:
			ARM_SHL_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;
		case OP_SHL_IMM:
			ARM_SHL_IMM (code, ins->dreg, ins->sreg1, (ins->inst_imm & 0x1f));
			break;
		case CEE_SHR:
			ARM_SAR_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;
		case OP_SHR_IMM:
			ARM_SAR_IMM (code, ins->dreg, ins->sreg1, (ins->inst_imm & 0x1f));
			break;
		case OP_SHR_UN_IMM:
			ARM_SHR_IMM (code, ins->dreg, ins->sreg1, (ins->inst_imm & 0x1f));
			break;
		case CEE_SHR_UN:
			ARM_SHR_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;
		case CEE_NOT:
			ARM_MVN_REG_REG (code, ins->dreg, ins->sreg1);
			break;
		case CEE_NEG:
			ARM_RSB_REG_IMM8 (code, ins->dreg, ins->sreg1, 0);
			break;
		case CEE_MUL:
			if (ins->dreg == ins->sreg2)
				ARM_MUL_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			else
				ARM_MUL_REG_REG (code, ins->dreg, ins->sreg2, ins->sreg1);
			break;
		case OP_MUL_IMM:
			g_assert_not_reached ();
			break;
		case CEE_MUL_OVF:
			/* FIXME: handle ovf/ sreg2 != dreg */
			ARM_MUL_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;
		case CEE_MUL_OVF_UN:
			/* FIXME: handle ovf/ sreg2 != dreg */
			ARM_MUL_REG_REG (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;
		case OP_ICONST:
		case OP_SETREGIMM:
			code = mono_arm_emit_load_imm (code, ins->dreg, ins->inst_c0);
			break;
		case OP_AOTCONST:
			g_assert_not_reached ();
			mono_add_patch_info (cfg, offset, (MonoJumpInfoType)ins->inst_i1, ins->inst_p0);
			break;
		case CEE_CONV_I4:
		case CEE_CONV_U4:
		case OP_MOVE:
		case OP_SETREG:
			ARM_MOV_REG_REG (code, ins->dreg, ins->sreg1);
			break;
		case OP_SETLRET: {
			int saved = ins->sreg2;
			if (ins->sreg2 == ARMREG_R0) {
				ARM_MOV_REG_REG (code, ARMREG_LR, ins->sreg2);
				saved = ARMREG_LR;
			}
			if (ins->sreg1 != ARMREG_R0)
				ARM_MOV_REG_REG (code, ARMREG_R0, ins->sreg1);
			if (saved != ARMREG_R1)
				ARM_MOV_REG_REG (code, ARMREG_R1, saved);
			break;
		}
		case OP_SETFREG:
		case OP_FMOVE:
			ARM_MVFD (code, ins->dreg, ins->sreg1);
			break;
		case OP_FCONV_TO_R4:
			ARM_MVFS (code, ins->dreg, ins->sreg1);
			break;
		case CEE_JMP: {
#if ARM_PORT
			int i, pos = 0;
			
			/*
			 * Keep in sync with mono_arch_emit_epilog
			 */
			g_assert (!cfg->method->save_lmf);
			if (1 || cfg->flags & MONO_CFG_HAS_CALLS) {
				if (ppc_is_imm16 (cfg->stack_usage + PPC_RET_ADDR_OFFSET)) {
					ppc_lwz (code, ppc_r0, cfg->stack_usage + PPC_RET_ADDR_OFFSET, cfg->frame_reg);
				} else {
					ppc_load (code, ppc_r11, cfg->stack_usage + PPC_RET_ADDR_OFFSET);
					ppc_lwzx (code, ppc_r0, cfg->frame_reg, ppc_r11);
				}
				ppc_mtlr (code, ppc_r0);
			}
			if (ppc_is_imm16 (cfg->stack_usage)) {
				ppc_addic (code, ppc_sp, cfg->frame_reg, cfg->stack_usage);
			} else {
				ppc_load (code, ppc_r11, cfg->stack_usage);
				ppc_add (code, ppc_sp, cfg->frame_reg, ppc_r11);
			}
			if (!cfg->method->save_lmf) {
				/*for (i = 31; i >= 14; --i) {
					if (cfg->used_float_regs & (1 << i)) {
						pos += sizeof (double);
						ppc_lfd (code, i, -pos, cfg->frame_reg);
					}
				}*/
				for (i = 31; i >= 13; --i) {
					if (cfg->used_int_regs & (1 << i)) {
						pos += sizeof (gulong);
						ppc_lwz (code, i, -pos, cfg->frame_reg);
					}
				}
			} else {
				/* FIXME restore from MonoLMF: though this can't happen yet */
			}
			mono_add_patch_info (cfg, (guint8*) code - cfg->native_code, MONO_PATCH_INFO_METHOD_JUMP, ins->inst_p0);
			ppc_b (code, 0);
#endif
			break;
		}
		case OP_CHECK_THIS:
			/* ensure ins->sreg1 is not NULL */
			ARM_LDR_IMM (code, ARMREG_LR, ins->sreg1, 0);
			break;
		case OP_ARGLIST: {
#if ARM_PORT
			if (ppc_is_imm16 (cfg->sig_cookie + cfg->stack_usage)) {
				ppc_addi (code, ppc_r11, cfg->frame_reg, cfg->sig_cookie + cfg->stack_usage);
			} else {
				ppc_load (code, ppc_r11, cfg->sig_cookie + cfg->stack_usage);
				ppc_add (code, ppc_r11, cfg->frame_reg, ppc_r11);
			}
			ppc_stw (code, ppc_r11, 0, ins->sreg1);
#endif
			break;
		}
		case OP_FCALL:
		case OP_LCALL:
		case OP_VCALL:
		case OP_VOIDCALL:
		case CEE_CALL:
			call = (MonoCallInst*)ins;
			if (ins->flags & MONO_INST_HAS_METHOD)
				mono_add_patch_info (cfg, offset, MONO_PATCH_INFO_METHOD, call->method);
			else
				mono_add_patch_info (cfg, offset, MONO_PATCH_INFO_ABS, call->fptr);
			if (cfg->method->dynamic) {
				ARM_LDR_IMM (code, ARMREG_IP, ARMREG_PC, 0);
				ARM_B (code, 0);
				*(gpointer*)code = NULL;
				code += 4;
				ARM_MOV_REG_REG (code, ARMREG_LR, ARMREG_PC);
				ARM_MOV_REG_REG (code, ARMREG_PC, ARMREG_IP);
			} else {
				ARM_BL (code, 0);
			}
			break;
		case OP_FCALL_REG:
		case OP_LCALL_REG:
		case OP_VCALL_REG:
		case OP_VOIDCALL_REG:
		case OP_CALL_REG:
			ARM_MOV_REG_REG (code, ARMREG_LR, ARMREG_PC);
			ARM_MOV_REG_REG (code, ARMREG_PC, ins->sreg1);
			break;
		case OP_FCALL_MEMBASE:
		case OP_LCALL_MEMBASE:
		case OP_VCALL_MEMBASE:
		case OP_VOIDCALL_MEMBASE:
		case OP_CALL_MEMBASE:
			g_assert (ins->inst_offset >= 0 && ins->inst_offset < 4096);
			ARM_LDR_IMM (code, ARMREG_IP, ins->sreg1, ins->inst_offset);
			ARM_MOV_REG_REG (code, ARMREG_LR, ARMREG_PC);
			ARM_MOV_REG_REG (code, ARMREG_PC, ARMREG_IP);
			break;
		case OP_OUTARG:
			g_assert_not_reached ();
			break;
		case OP_LOCALLOC: {
			/* keep alignment */
			int alloca_waste = cfg->param_area;
			alloca_waste += 7;
			alloca_waste &= ~7;
			/* round the size to 8 bytes */
			ARM_ADD_REG_IMM8 (code, ins->dreg, ins->sreg1, 7);
			ARM_SHL_IMM (code, ins->dreg, ins->dreg, 3);
			ARM_SHR_IMM (code, ins->dreg, ins->dreg, 3);
			ARM_ADD_REG_IMM8 (code, ins->dreg, ins->dreg, alloca_waste);
			ARM_SUB_REG_REG (code, ARMREG_SP, ARMREG_SP, ins->dreg);
			/* memzero the area */
			if (ins->flags & MONO_INST_INIT) {
				guint8 *start_loop, *branch_to_cond;
				ARM_MOV_REG_IMM8 (code, ARMREG_LR, 0);
				branch_to_cond = code;
				ARM_B (code, 0);
				start_loop = code;
				ARM_STR_REG_REG (code, ARMREG_LR, ARMREG_SP, ins->dreg);
				arm_patch (branch_to_cond, code);
				/* decrement by 4 and set flags */
				ARM_SUBS_REG_IMM8 (code, ins->dreg, ins->dreg, 4);
				ARM_B_COND (code, ARMCOND_LT, 0);
				arm_patch (code - 4, start_loop);
			}
			ARM_ADD_REG_IMM8 (code, ins->dreg, ARMREG_SP, alloca_waste);
			break;
		}
		case CEE_RET:
			ARM_MOV_REG_REG (code, ARMREG_PC, ARMREG_LR);
			break;
		case CEE_THROW: {
			if (ins->sreg1 != ARMREG_R0)
				ARM_MOV_REG_REG (code, ARMREG_R0, ins->sreg1);
			mono_add_patch_info (cfg, code - cfg->native_code, MONO_PATCH_INFO_INTERNAL_METHOD, 
					     (gpointer)"mono_arch_throw_exception");
			if (cfg->method->dynamic) {
				ARM_LDR_IMM (code, ARMREG_IP, ARMREG_PC, 0);
				ARM_B (code, 0);
				*(gpointer*)code = NULL;
				code += 4;
				ARM_MOV_REG_REG (code, ARMREG_LR, ARMREG_PC);
				ARM_MOV_REG_REG (code, ARMREG_PC, ARMREG_IP);
			} else {
				ARM_BL (code, 0);
			}
			break;
		}
		case OP_RETHROW: {
			if (ins->sreg1 != ARMREG_R0)
				ARM_MOV_REG_REG (code, ARMREG_R0, ins->sreg1);
			mono_add_patch_info (cfg, code - cfg->native_code, MONO_PATCH_INFO_INTERNAL_METHOD, 
					     (gpointer)"mono_arch_rethrow_exception");
			if (cfg->method->dynamic) {
				ARM_LDR_IMM (code, ARMREG_IP, ARMREG_PC, 0);
				ARM_B (code, 0);
				*(gpointer*)code = NULL;
				code += 4;
				ARM_MOV_REG_REG (code, ARMREG_LR, ARMREG_PC);
				ARM_MOV_REG_REG (code, ARMREG_PC, ARMREG_IP);
			} else {
				ARM_BL (code, 0);
			}
			break;
		}
		case OP_START_HANDLER:
#if ARM_PORT
			ppc_mflr (code, ppc_r0);
			if (ppc_is_imm16 (ins->inst_left->inst_offset)) {
				ppc_stw (code, ppc_r0, ins->inst_left->inst_offset, ins->inst_left->inst_basereg);
			} else {
				ppc_load (code, ppc_r11, ins->inst_left->inst_offset);
				ppc_stwx (code, ppc_r0, ppc_r11, ins->inst_left->inst_basereg);
			}
			break;
#endif
		case OP_ENDFILTER:
#if ARM_PORT
			if (ins->sreg1 != ARMREG_R0)
				ARM_MOV_REG_REG (code, ARMREG_R0, ins->sreg1);
			if (ppc_is_imm16 (ins->inst_left->inst_offset)) {
				ppc_lwz (code, ppc_r0, ins->inst_left->inst_offset, ins->inst_left->inst_basereg);
			} else {
				ppc_load (code, ppc_r11, ins->inst_left->inst_offset);
				ppc_lwzx (code, ppc_r0, ins->inst_left->inst_basereg, ppc_r11);
			}
			ppc_mtlr (code, ppc_r0);
			ppc_blr (code);
#endif
			break;
		case CEE_ENDFINALLY:
#if ARM_PORT
			ppc_lwz (code, ppc_r0, ins->inst_left->inst_offset, ins->inst_left->inst_basereg);
			ppc_mtlr (code, ppc_r0);
			ppc_blr (code);
#endif
			break;
		case OP_CALL_HANDLER: 
			mono_add_patch_info (cfg, code - cfg->native_code, MONO_PATCH_INFO_BB, ins->inst_target_bb);
			ARM_BL (code, 0);
			break;
		case OP_LABEL:
			ins->inst_c0 = code - cfg->native_code;
			break;
		case CEE_BR:
			if (ins->flags & MONO_INST_BRLABEL) {
				/*if (ins->inst_i0->inst_c0) {
					ARM_B (code, 0);
					//x86_jump_code (code, cfg->native_code + ins->inst_i0->inst_c0);
				} else*/ {
					mono_add_patch_info (cfg, offset, MONO_PATCH_INFO_LABEL, ins->inst_i0);
					ARM_B (code, 0);
				}
			} else {
				/*if (ins->inst_target_bb->native_offset) {
					ARM_B (code, 0);
					//x86_jump_code (code, cfg->native_code + ins->inst_target_bb->native_offset); 
				} else*/ {
					mono_add_patch_info (cfg, offset, MONO_PATCH_INFO_BB, ins->inst_target_bb);
					ARM_B (code, 0);
				} 
			}
			break;
		case OP_BR_REG:
			ARM_MOV_REG_REG (code, ARMREG_PC, ins->sreg1);
			break;
		case CEE_SWITCH:
			/* 
			 * In the normal case we have:
			 * 	ldr pc, [pc, ins->sreg1 << 2]
			 * 	nop
			 * If aot, we have:
			 * 	ldr lr, [pc, ins->sreg1 << 2]
			 * 	add pc, pc, lr
			 * After follows the data.
			 * FIXME: add aot support.
			 */
			max_len += 4 * GPOINTER_TO_INT (ins->klass);
			if (offset > (cfg->code_size - max_len - 16)) {
				cfg->code_size += max_len;
				cfg->code_size *= 2;
				cfg->native_code = g_realloc (cfg->native_code, cfg->code_size);
				code = cfg->native_code + offset;
			}
			ARM_LDR_REG_REG_SHIFT (code, ARMREG_PC, ARMREG_PC, ins->sreg1, ARMSHIFT_LSL, 2);
			ARM_NOP (code);
			code += 4 * GPOINTER_TO_INT (ins->klass);
			break;
		case OP_CEQ:
			ARM_MOV_REG_IMM8 (code, ins->dreg, 0);
			ARM_MOV_REG_IMM8_COND (code, ins->dreg, 1, ARMCOND_EQ);
			break;
		case OP_CLT:
			ARM_MOV_REG_IMM8 (code, ins->dreg, 0);
			ARM_MOV_REG_IMM8_COND (code, ins->dreg, 1, ARMCOND_LT);
			break;
		case OP_CLT_UN:
			ARM_MOV_REG_IMM8 (code, ins->dreg, 0);
			ARM_MOV_REG_IMM8_COND (code, ins->dreg, 1, ARMCOND_LO);
			break;
		case OP_CGT:
			ARM_MOV_REG_IMM8 (code, ins->dreg, 0);
			ARM_MOV_REG_IMM8_COND (code, ins->dreg, 1, ARMCOND_GT);
			break;
		case OP_CGT_UN:
			ARM_MOV_REG_IMM8 (code, ins->dreg, 0);
			ARM_MOV_REG_IMM8_COND (code, ins->dreg, 1, ARMCOND_HI);
			break;
		case OP_COND_EXC_EQ:
		case OP_COND_EXC_NE_UN:
		case OP_COND_EXC_LT:
		case OP_COND_EXC_LT_UN:
		case OP_COND_EXC_GT:
		case OP_COND_EXC_GT_UN:
		case OP_COND_EXC_GE:
		case OP_COND_EXC_GE_UN:
		case OP_COND_EXC_LE:
		case OP_COND_EXC_LE_UN:
			EMIT_COND_SYSTEM_EXCEPTION (ins->opcode - OP_COND_EXC_EQ, ins->inst_p1);
			break;
		case OP_COND_EXC_C:
		case OP_COND_EXC_OV:
		case OP_COND_EXC_NC:
		case OP_COND_EXC_NO:
			g_assert_not_reached ();
			break;
		case CEE_BEQ:
		case CEE_BNE_UN:
		case CEE_BLT:
		case CEE_BLT_UN:
		case CEE_BGT:
		case CEE_BGT_UN:
		case CEE_BGE:
		case CEE_BGE_UN:
		case CEE_BLE:
		case CEE_BLE_UN:
			EMIT_COND_BRANCH (ins, ins->opcode - CEE_BEQ);
			break;

		/* floating point opcodes */
		case OP_R8CONST:
			/* FIXME: we can optimize the imm load by dealing with part of 
			 * the displacement in LDFD (aligning to 512).
			 */
			code = mono_arm_emit_load_imm (code, ARMREG_LR, (guint32)ins->inst_p0);
			ARM_LDFD (code, ins->dreg, ARMREG_LR, 0);
			break;
		case OP_R4CONST:
			code = mono_arm_emit_load_imm (code, ARMREG_LR, (guint32)ins->inst_p0);
			ARM_LDFS (code, ins->dreg, ARMREG_LR, 0);
			break;
		case OP_STORER8_MEMBASE_REG:
			g_assert (arm_is_fpimm8 (ins->inst_offset));
			ARM_STFD (code, ins->sreg1, ins->inst_destbasereg, ins->inst_offset);
			break;
		case OP_LOADR8_MEMBASE:
			g_assert (arm_is_fpimm8 (ins->inst_offset));
			ARM_LDFD (code, ins->dreg, ins->inst_basereg, ins->inst_offset);
			break;
		case OP_STORER4_MEMBASE_REG:
			g_assert (arm_is_fpimm8 (ins->inst_offset));
			ARM_STFS (code, ins->sreg1, ins->inst_destbasereg, ins->inst_offset);
			break;
		case OP_LOADR4_MEMBASE:
			g_assert (arm_is_fpimm8 (ins->inst_offset));
			ARM_LDFS (code, ins->dreg, ins->inst_basereg, ins->inst_offset);
			break;
		case CEE_CONV_R_UN: {
			/*static const guint64 adjust_val = 0x4330000000000000ULL;
			ppc_addis (code, ppc_r0, ppc_r0, 0x4330);
			ppc_stw (code, ppc_r0, -8, ppc_sp);
			ppc_stw (code, ins->sreg1, -4, ppc_sp);
			ppc_load (code, ppc_r11, &adjust_val);
			ppc_lfd (code, ins->dreg, -8, ppc_sp);
			ppc_lfd (code, ppc_f0, 0, ppc_r11);
			ppc_fsub (code, ins->dreg, ins->dreg, ppc_f0);*/
			g_assert_not_reached ();
			break;
		}
		case CEE_CONV_R4:
			ARM_FLTS (code, ins->dreg, ins->sreg1);
			break;
		case CEE_CONV_R8:
			ARM_FLTD (code, ins->dreg, ins->sreg1);
			break;
		case OP_X86_FP_LOAD_I8:
			g_assert_not_reached ();
			/*x86_fild_membase (code, ins->inst_basereg, ins->inst_offset, TRUE);*/
			break;
		case OP_X86_FP_LOAD_I4:
			g_assert_not_reached ();
			/*x86_fild_membase (code, ins->inst_basereg, ins->inst_offset, FALSE);*/
			break;
		case OP_FCONV_TO_I1:
			code = emit_float_to_int (cfg, code, ins->dreg, ins->sreg1, 1, TRUE);
			break;
		case OP_FCONV_TO_U1:
			code = emit_float_to_int (cfg, code, ins->dreg, ins->sreg1, 1, FALSE);
			break;
		case OP_FCONV_TO_I2:
			code = emit_float_to_int (cfg, code, ins->dreg, ins->sreg1, 2, TRUE);
			break;
		case OP_FCONV_TO_U2:
			code = emit_float_to_int (cfg, code, ins->dreg, ins->sreg1, 2, FALSE);
			break;
		case OP_FCONV_TO_I4:
		case OP_FCONV_TO_I:
			code = emit_float_to_int (cfg, code, ins->dreg, ins->sreg1, 4, TRUE);
			break;
		case OP_FCONV_TO_U4:
		case OP_FCONV_TO_U:
			code = emit_float_to_int (cfg, code, ins->dreg, ins->sreg1, 4, FALSE);
			break;
		case OP_FCONV_TO_I8:
		case OP_FCONV_TO_U8:
			g_assert_not_reached ();
			/* Implemented as helper calls */
			break;
		case OP_LCONV_TO_R_UN:
			g_assert_not_reached ();
			/* Implemented as helper calls */
			break;
		case OP_LCONV_TO_OVF_I: {
#if ARM_PORT
			guint32 *negative_branch, *msword_positive_branch, *msword_negative_branch, *ovf_ex_target;
			// Check if its negative
			ppc_cmpi (code, 0, 0, ins->sreg1, 0);
			negative_branch = code;
			ppc_bc (code, PPC_BR_TRUE, PPC_BR_LT, 0);
			// Its positive msword == 0
			ppc_cmpi (code, 0, 0, ins->sreg2, 0);
			msword_positive_branch = code;
			ppc_bc (code, PPC_BR_TRUE, PPC_BR_EQ, 0);

			ovf_ex_target = code;
			//EMIT_COND_SYSTEM_EXCEPTION_FLAGS (PPC_BR_ALWAYS, 0, "OverflowException");
			// Negative
			ppc_patch (negative_branch, code);
			ppc_cmpi (code, 0, 0, ins->sreg2, -1);
			msword_negative_branch = code;
			ppc_bc (code, PPC_BR_FALSE, PPC_BR_EQ, 0);
			ppc_patch (msword_negative_branch, ovf_ex_target);
			
			ppc_patch (msword_positive_branch, code);
			if (ins->dreg != ins->sreg1)
				ppc_mr (code, ins->dreg, ins->sreg1);
#endif
			g_assert_not_reached ();
			break;
		}
		case OP_FADD:
			ARM_FPA_ADFD (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;
		case OP_FSUB:
			ARM_FPA_SUFD (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;		
		case OP_FMUL:
			ARM_FPA_MUFD (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;		
		case OP_FDIV:
			ARM_FPA_DVFD (code, ins->dreg, ins->sreg1, ins->sreg2);
			break;		
		case OP_FNEG:
			ARM_MNFD (code, ins->dreg, ins->sreg1);
			break;		
		case OP_FREM:
			/* emulated */
			g_assert_not_reached ();
			break;
		case OP_FCOMPARE:
			ARM_FCMP (code, ARM_FPA_CMF, ins->sreg1, ins->sreg2);
			break;
		case OP_FCEQ:
			g_assert_not_reached ();
			/*ppc_fcmpo (code, 0, ins->sreg1, ins->sreg2);
			ppc_li (code, ins->dreg, 0);
			ppc_bc (code, PPC_BR_FALSE, PPC_BR_EQ, 2);
			ppc_li (code, ins->dreg, 1);*/
			break;
		case OP_FCLT:
			g_assert_not_reached ();
			/*ppc_fcmpo (code, 0, ins->sreg1, ins->sreg2);
			ppc_li (code, ins->dreg, 1);
			ppc_bc (code, PPC_BR_TRUE, PPC_BR_LT, 2);
			ppc_li (code, ins->dreg, 0);*/
			break;
		case OP_FCLT_UN:
			g_assert_not_reached ();
			/*ppc_fcmpu (code, 0, ins->sreg1, ins->sreg2);
			ppc_li (code, ins->dreg, 1);
			ppc_bc (code, PPC_BR_TRUE, PPC_BR_SO, 3);
			ppc_bc (code, PPC_BR_TRUE, PPC_BR_LT, 2);
			ppc_li (code, ins->dreg, 0);*/
			break;
		case OP_FCGT:
			g_assert_not_reached ();
			/*ppc_fcmpo (code, 0, ins->sreg1, ins->sreg2);
			ppc_li (code, ins->dreg, 1);
			ppc_bc (code, PPC_BR_TRUE, PPC_BR_GT, 2);
			ppc_li (code, ins->dreg, 0);*/
			break;
		case OP_FCGT_UN:
			g_assert_not_reached ();
			/*ppc_fcmpu (code, 0, ins->sreg1, ins->sreg2);
			ppc_li (code, ins->dreg, 1);
			ppc_bc (code, PPC_BR_TRUE, PPC_BR_SO, 3);
			ppc_bc (code, PPC_BR_TRUE, PPC_BR_GT, 2);
			ppc_li (code, ins->dreg, 0);*/
			break;
		case OP_FBEQ:
			EMIT_COND_BRANCH (ins, CEE_BEQ - CEE_BEQ);
			break;
		case OP_FBNE_UN:
			EMIT_COND_BRANCH (ins, CEE_BNE_UN - CEE_BEQ);
			break;
		case OP_FBLT:
			EMIT_COND_BRANCH (ins, CEE_BLT - CEE_BEQ);
			break;
		case OP_FBLT_UN:
			//EMIT_COND_BRANCH_FLAGS (ins, PPC_BR_TRUE, PPC_BR_SO);
			EMIT_COND_BRANCH (ins, CEE_BLT_UN - CEE_BEQ);
			break;
		case OP_FBGT:
			EMIT_COND_BRANCH (ins, CEE_BGT - CEE_BEQ);
			break;
		case OP_FBGT_UN:
			//EMIT_COND_BRANCH_FLAGS (ins, PPC_BR_TRUE, PPC_BR_SO);
			EMIT_COND_BRANCH (ins, CEE_BGT_UN - CEE_BEQ);
			break;
		case OP_FBGE:
			EMIT_COND_BRANCH (ins, CEE_BGE - CEE_BEQ);
			break;
		case OP_FBGE_UN:
			EMIT_COND_BRANCH (ins, CEE_BGE_UN - CEE_BEQ);
			break;
		case OP_FBLE:
			EMIT_COND_BRANCH (ins, CEE_BLE - CEE_BEQ);
			break;
		case OP_FBLE_UN:
			EMIT_COND_BRANCH (ins, CEE_BLE_UN - CEE_BEQ);
			break;
		case CEE_CKFINITE: {
			/*ppc_stfd (code, ins->sreg1, -8, ppc_sp);
			ppc_lwz (code, ppc_r11, -8, ppc_sp);
			ppc_rlwinm (code, ppc_r11, ppc_r11, 0, 1, 31);
			ppc_addis (code, ppc_r11, ppc_r11, -32752);
			ppc_rlwinmd (code, ppc_r11, ppc_r11, 1, 31, 31);
			EMIT_COND_SYSTEM_EXCEPTION (CEE_BEQ - CEE_BEQ, "ArithmeticException");*/
			g_assert_not_reached ();
			break;
		}
		default:
			g_warning ("unknown opcode %s in %s()\n", mono_inst_name (ins->opcode), __FUNCTION__);
			g_assert_not_reached ();
		}

		if ((cfg->opt & MONO_OPT_BRANCH) && ((code - cfg->native_code - offset) > max_len)) {
			g_warning ("wrong maximal instruction length of instruction %s (expected %d, got %d)",
				   mono_inst_name (ins->opcode), max_len, code - cfg->native_code - offset);
			g_assert_not_reached ();
		}
	       
		cpos += max_len;

		last_ins = ins;
		last_offset = offset;
		
		ins = ins->next;
	}

	cfg->code_len = code - cfg->native_code;
}

void
mono_arch_register_lowlevel_calls (void)
{
}

#define patch_lis_ori(ip,val) do {\
		guint16 *__lis_ori = (guint16*)(ip);	\
		__lis_ori [1] = (((guint32)(val)) >> 16) & 0xffff;	\
		__lis_ori [3] = ((guint32)(val)) & 0xffff;	\
	} while (0)

void
mono_arch_patch_code (MonoMethod *method, MonoDomain *domain, guint8 *code, MonoJumpInfo *ji, gboolean run_cctors)
{
	MonoJumpInfo *patch_info;

	for (patch_info = ji; patch_info; patch_info = patch_info->next) {
		unsigned char *ip = patch_info->ip.i + code;
		const unsigned char *target;

		if (patch_info->type == MONO_PATCH_INFO_SWITCH) {
			gpointer *table = (gpointer *)patch_info->data.table->table;
			gpointer *jt = (gpointer*)(ip + 8);
			int i;
			/* jt is the inlined jump table, 2 instructions after ip
			 * In the normal case we store the absolute addresses,
			 * otherwise the displacements.
			 */
			for (i = 0; i < patch_info->data.table->table_size; i++) { 
				jt [i] = code + (int)patch_info->data.table->table [i];
			}
			continue;
		}
		target = mono_resolve_patch_target (method, domain, code, patch_info, run_cctors);

		switch (patch_info->type) {
		case MONO_PATCH_INFO_IP:
			patch_lis_ori (ip, ip);
			continue;
		case MONO_PATCH_INFO_METHOD_REL:
			g_assert_not_reached ();
			*((gpointer *)(ip)) = code + patch_info->data.offset;
			continue;
		case MONO_PATCH_INFO_METHODCONST:
		case MONO_PATCH_INFO_CLASS:
		case MONO_PATCH_INFO_IMAGE:
		case MONO_PATCH_INFO_FIELD:
		case MONO_PATCH_INFO_VTABLE:
		case MONO_PATCH_INFO_IID:
		case MONO_PATCH_INFO_SFLDA:
		case MONO_PATCH_INFO_LDSTR:
		case MONO_PATCH_INFO_TYPE_FROM_HANDLE:
		case MONO_PATCH_INFO_LDTOKEN:
			/* from OP_AOTCONST : lis + ori */
			patch_lis_ori (ip, target);
			continue;
		case MONO_PATCH_INFO_R4:
		case MONO_PATCH_INFO_R8:
			g_assert_not_reached ();
			*((gconstpointer *)(ip + 2)) = patch_info->data.target;
			continue;
		case MONO_PATCH_INFO_EXC_NAME:
			g_assert_not_reached ();
			*((gconstpointer *)(ip + 1)) = patch_info->data.name;
			continue;
		case MONO_PATCH_INFO_NONE:
		case MONO_PATCH_INFO_BB_OVF:
		case MONO_PATCH_INFO_EXC_OVF:
			/* everything is dealt with at epilog output time */
			continue;
		default:
			break;
		}
		arm_patch (ip, target);
	}
}

/*
 * Stack frame layout:
 * 
 *   ------------------- fp
 *   	MonoLMF structure or saved registers
 *   -------------------
 *   	locals
 *   -------------------
 *   	spilled regs
 *   -------------------
 *   	optional 8 bytes for tracing
 *   -------------------
 *   	param area             size is cfg->param_area
 *   ------------------- sp
 */
guint8 *
mono_arch_emit_prolog (MonoCompile *cfg)
{
	MonoMethod *method = cfg->method;
	MonoBasicBlock *bb;
	MonoMethodSignature *sig;
	MonoInst *inst;
	int alloc_size, pos, max_offset, i;
	guint8 *code;
	CallInfo *cinfo;
	int tracing = 0;
	int lmf_offset = 0;

	if (mono_jit_trace_calls != NULL && mono_trace_eval (method))
		tracing = 1;

	sig = mono_method_signature (method);
	cfg->code_size = 256 + sig->param_count * 20;
	code = cfg->native_code = g_malloc (cfg->code_size);

	if (cfg->max_ireg >= 29)
		cfg->used_int_regs |= USE_EXTRA_TEMPS;
	ARM_MOV_REG_REG (code, ARMREG_IP, ARMREG_SP);
	ARM_PUSH (code, (cfg->used_int_regs | (1 << ARMREG_IP) | (1 << ARMREG_LR)));

	alloc_size = cfg->stack_offset;
	pos = 0;

	if (!method->save_lmf) {
		/*for (i = 31; i >= 14; --i) {
			if (cfg->used_float_regs & (1 << i)) {
				pos += sizeof (gdouble);
				ppc_stfd (code, i, -pos, ppc_sp);
			}
		}*/
	} else {
		/*int ofs;
		pos += sizeof (MonoLMF);
		lmf_offset = pos;
		ofs = -pos + G_STRUCT_OFFSET(MonoLMF, iregs);
		ppc_stmw (code, ppc_r13, ppc_r1, ofs);
		for (i = 14; i < 32; i++) {
			ppc_stfd (code, i, (-pos + G_STRUCT_OFFSET(MonoLMF, fregs) + ((i-14) * sizeof (gdouble))), ppc_r1);
		}*/
	}
	alloc_size += pos;
	// align to MONO_ARCH_FRAME_ALIGNMENT bytes
	if (alloc_size & (MONO_ARCH_FRAME_ALIGNMENT - 1)) {
		alloc_size += MONO_ARCH_FRAME_ALIGNMENT - 1;
		alloc_size &= ~(MONO_ARCH_FRAME_ALIGNMENT - 1);
	}

	cfg->stack_usage = alloc_size;
	g_assert ((alloc_size & (MONO_ARCH_FRAME_ALIGNMENT-1)) == 0);
	if (alloc_size) {
		ARM_SUB_REG_IMM8 (code, ARMREG_SP, ARMREG_SP, alloc_size);
	}
	if (cfg->frame_reg != ARMREG_SP)
		ARM_MOV_REG_REG (code, cfg->frame_reg, ARMREG_SP);

        /* compute max_offset in order to use short forward jumps
	 * we always do it on ppc because the immediate displacement
	 * for jumps is too small 
	 */
	max_offset = 0;
	for (bb = cfg->bb_entry; bb; bb = bb->next_bb) {
		MonoInst *ins = bb->code;
		bb->max_offset = max_offset;

		if (cfg->prof_options & MONO_PROFILE_COVERAGE)
			max_offset += 6; 

		while (ins) {
			max_offset += ((guint8 *)arm_cpu_desc [ins->opcode])[MONO_INST_LEN];
			ins = ins->next;
		}
	}

	/* load arguments allocated to register from the stack */
	pos = 0;

	cinfo = calculate_sizes (sig, sig->pinvoke);

	if (MONO_TYPE_ISSTRUCT (sig->ret)) {
		ArgInfo *ainfo = &cinfo->ret;
		inst = cfg->ret;
		g_assert (arm_is_imm12 (inst->inst_offset));
		ARM_STR_IMM (code, ainfo->reg, inst->inst_basereg, inst->inst_offset);
	}
	for (i = 0; i < sig->param_count + sig->hasthis; ++i) {
		ArgInfo *ainfo = cinfo->args + i;
		inst = cfg->varinfo [pos];
		
		if (cfg->verbose_level > 2)
			g_print ("Saving argument %d (type: %d)\n", i, ainfo->regtype);
		if (inst->opcode == OP_REGVAR) {
			if (ainfo->regtype == RegTypeGeneral)
				ARM_MOV_REG_REG (code, inst->dreg, ainfo->reg);
			else if (ainfo->regtype == RegTypeFP) {
				g_assert_not_reached ();
				//ppc_fmr (code, inst->dreg, ainfo->reg);
			} else if (ainfo->regtype == RegTypeBase) {
				g_assert_not_reached ();
				//ppc_lwz (code, ppc_r11, 0, ppc_sp);
				//ppc_lwz (code, inst->dreg, ainfo->offset, ppc_r11);
			} else
				g_assert_not_reached ();

			if (cfg->verbose_level > 2)
				g_print ("Argument %d assigned to register %s\n", pos, mono_arch_regname (inst->dreg));
		} else {
#if ARM_PORT
			/* the argument should be put on the stack: FIXME handle size != word  */
			if (ainfo->regtype == RegTypeGeneral) {
				switch (ainfo->size) {
				case 1:
					if (ppc_is_imm16 (inst->inst_offset)) {
						ppc_stb (code, ainfo->reg, inst->inst_offset, inst->inst_basereg);
					} else {
						ppc_load (code, ppc_r11, inst->inst_offset);
						ppc_stbx (code, ainfo->reg, ppc_r11, inst->inst_basereg);
					}
					break;
				case 2:
					if (ppc_is_imm16 (inst->inst_offset)) {
						ppc_sth (code, ainfo->reg, inst->inst_offset, inst->inst_basereg);
					} else {
						ppc_load (code, ppc_r11, inst->inst_offset);
						ppc_sthx (code, ainfo->reg, ppc_r11, inst->inst_basereg);
					}
					break;
				case 8:
					if (ppc_is_imm16 (inst->inst_offset + 4)) {
						ppc_stw (code, ainfo->reg, inst->inst_offset, inst->inst_basereg);
						ppc_stw (code, ainfo->reg + 1, inst->inst_offset + 4, inst->inst_basereg);
					} else {
						ppc_load (code, ppc_r11, inst->inst_offset);
						ppc_add (code, ppc_r11, ppc_r11, inst->inst_basereg);
						ppc_stw (code, ainfo->reg, 0, ppc_r11);
						ppc_stw (code, ainfo->reg + 1, 4, ppc_r11);
					}
					break;
				default:
					if (ppc_is_imm16 (inst->inst_offset)) {
						ppc_stw (code, ainfo->reg, inst->inst_offset, inst->inst_basereg);
					} else {
						ppc_load (code, ppc_r11, inst->inst_offset);
						ppc_stwx (code, ainfo->reg, ppc_r11, inst->inst_basereg);
					}
					break;
				}
			} else if (ainfo->regtype == RegTypeBase) {
				/* load the previous stack pointer in r11 */
				ppc_lwz (code, ppc_r11, 0, ppc_sp);
				ppc_lwz (code, ppc_r0, ainfo->offset, ppc_r11);
				switch (ainfo->size) {
				case 1:
					if (ppc_is_imm16 (inst->inst_offset)) {
						ppc_stb (code, ppc_r0, inst->inst_offset, inst->inst_basereg);
					} else {
						ppc_load (code, ppc_r11, inst->inst_offset);
						ppc_stbx (code, ppc_r0, ppc_r11, inst->inst_basereg);
					}
					break;
				case 2:
					if (ppc_is_imm16 (inst->inst_offset)) {
						ppc_sth (code, ppc_r0, inst->inst_offset, inst->inst_basereg);
					} else {
						ppc_load (code, ppc_r11, inst->inst_offset);
						ppc_sthx (code, ppc_r0, ppc_r11, inst->inst_basereg);
					}
					break;
				case 8:
					if (ppc_is_imm16 (inst->inst_offset + 4)) {
						ppc_stw (code, ppc_r0, inst->inst_offset, inst->inst_basereg);
						ppc_lwz (code, ppc_r0, ainfo->offset + 4, ppc_r11);
						ppc_stw (code, ppc_r0, inst->inst_offset + 4, inst->inst_basereg);
					} else {
						/* FIXME */
						g_assert_not_reached ();
					}
					break;
				default:
					if (ppc_is_imm16 (inst->inst_offset)) {
						ppc_stw (code, ppc_r0, inst->inst_offset, inst->inst_basereg);
					} else {
						ppc_load (code, ppc_r11, inst->inst_offset);
						ppc_stwx (code, ppc_r0, ppc_r11, inst->inst_basereg);
					}
					break;
				}
			} else if (ainfo->regtype == RegTypeFP) {
				g_assert (ppc_is_imm16 (inst->inst_offset));
				if (ainfo->size == 8)
					ppc_stfd (code, ainfo->reg, inst->inst_offset, inst->inst_basereg);
				else if (ainfo->size == 4)
					ppc_stfs (code, ainfo->reg, inst->inst_offset, inst->inst_basereg);
				else
					g_assert_not_reached ();
			} else if (ainfo->regtype == RegTypeStructByVal) {
				int doffset = inst->inst_offset;
				int soffset = 0;
				int cur_reg;
				int size = 0;
				g_assert (ppc_is_imm16 (inst->inst_offset));
				g_assert (ppc_is_imm16 (inst->inst_offset + ainfo->size * sizeof (gpointer)));
				if (mono_class_from_mono_type (inst->inst_vtype))
					size = mono_class_native_size (mono_class_from_mono_type (inst->inst_vtype), NULL);
				for (cur_reg = 0; cur_reg < ainfo->size; ++cur_reg) {
/*
Darwin handles 1 and 2 byte structs specially by loading h/b into the arg
register.  Should this case include linux/ppc?
*/
#if __APPLE__
					if (size == 2)
						ppc_sth (code, ainfo->reg + cur_reg, doffset, inst->inst_basereg);
					else if (size == 1)
						ppc_stb (code, ainfo->reg + cur_reg, doffset, inst->inst_basereg);
					else 
#endif
						ppc_stw (code, ainfo->reg + cur_reg, doffset, inst->inst_basereg);
					soffset += sizeof (gpointer);
					doffset += sizeof (gpointer);
				}
				if (ainfo->vtsize) {
					/* load the previous stack pointer in r11 (r0 gets overwritten by the memcpy) */
					ppc_lwz (code, ppc_r11, 0, ppc_sp);
					/* FIXME: handle overrun! with struct sizes not multiple of 4 */
					code = emit_memcpy (code, ainfo->vtsize * sizeof (gpointer), inst->inst_basereg, doffset, ppc_r11, ainfo->offset + soffset);
				}
			} else if (ainfo->regtype == RegTypeStructByAddr) {
				g_assert (ppc_is_imm16 (inst->inst_offset));
				/* FIXME: handle overrun! with struct sizes not multiple of 4 */
				code = emit_memcpy (code, ainfo->vtsize * sizeof (gpointer), inst->inst_basereg, inst->inst_offset, ainfo->reg, 0);
			} else
				g_assert_not_reached ();
#endif
		}
		pos++;
	}

	if (method->save_lmf) {

		mono_add_patch_info (cfg, code - cfg->native_code, MONO_PATCH_INFO_INTERNAL_METHOD, 
			     (gpointer)"mono_get_lmf_addr");
		if (cfg->method->dynamic) {
			/*ppc_lis (code, ppc_r0, 0);
			ppc_ori (code, ppc_r0, ppc_r0, 0);
			ppc_mtlr (code, ppc_r0);
			ppc_blrl (code);*/
		} else {
			ARM_BL (code, 0);
		}
#if ARM_PORT
		/* we build the MonoLMF structure on the stack - see mini-ppc.h */
		/* lmf_offset is the offset from the previous stack pointer,
		 * alloc_size is the total stack space allocated, so the offset
		 * of MonoLMF from the current stack ptr is alloc_size - lmf_offset.
		 * The pointer to the struct is put in ppc_r11 (new_lmf).
		 * The callee-saved registers are already in the MonoLMF structure
		 */
		ppc_addi (code, ppc_r11, ppc_sp, alloc_size - lmf_offset);
		/* ppc_r3 is the result from mono_get_lmf_addr () */
		ppc_stw (code, ppc_r3, G_STRUCT_OFFSET(MonoLMF, lmf_addr), ppc_r11);
		/* new_lmf->previous_lmf = *lmf_addr */
		ppc_lwz (code, ppc_r0, G_STRUCT_OFFSET(MonoLMF, previous_lmf), ppc_r3);
		ppc_stw (code, ppc_r0, G_STRUCT_OFFSET(MonoLMF, previous_lmf), ppc_r11);
		/* *(lmf_addr) = r11 */
		ppc_stw (code, ppc_r11, G_STRUCT_OFFSET(MonoLMF, previous_lmf), ppc_r3);
		/* save method info */
		ppc_load (code, ppc_r0, method);
		ppc_stw (code, ppc_r0, G_STRUCT_OFFSET(MonoLMF, method), ppc_r11);
		ppc_stw (code, ppc_sp, G_STRUCT_OFFSET(MonoLMF, ebp), ppc_r11);
		/* save the current IP */
		mono_add_patch_info (cfg, code - cfg->native_code, MONO_PATCH_INFO_IP, NULL);
		ppc_load (code, ppc_r0, 0x01010101);
		ppc_stw (code, ppc_r0, G_STRUCT_OFFSET(MonoLMF, eip), ppc_r11);
#endif
	}

	if (tracing)
		code = mono_arch_instrument_prolog (cfg, mono_trace_enter_method, code, TRUE);

	cfg->code_len = code - cfg->native_code;
	g_assert (cfg->code_len < cfg->code_size);
	g_free (cinfo);

	return code;
}

void
mono_arch_emit_epilog (MonoCompile *cfg)
{
	MonoJumpInfo *patch_info;
	MonoMethod *method = cfg->method;
	int pos, i;
	int max_epilog_size = 16 + 20*4;
	guint8 *code;

	if (cfg->method->save_lmf)
		max_epilog_size += 128;
	
	if (mono_jit_trace_calls != NULL)
		max_epilog_size += 50;

	if (cfg->prof_options & MONO_PROFILE_ENTER_LEAVE)
		max_epilog_size += 50;

	while (cfg->code_len + max_epilog_size > (cfg->code_size - 16)) {
		cfg->code_size *= 2;
		cfg->native_code = g_realloc (cfg->native_code, cfg->code_size);
		mono_jit_stats.code_reallocs++;
	}

	/*
	 * Keep in sync with CEE_JMP
	 */
	code = cfg->native_code + cfg->code_len;

	if (mono_jit_trace_calls != NULL && mono_trace_eval (method)) {
		code = mono_arch_instrument_epilog (cfg, mono_trace_leave_method, code, TRUE);
	}
	pos = 0;

	if (method->save_lmf) {
#if ARM_PORT
		int lmf_offset;
		pos +=  sizeof (MonoLMF);
		lmf_offset = pos;
		/* save the frame reg in r8 */
		ppc_mr (code, ppc_r8, cfg->frame_reg);
		ppc_addi (code, ppc_r11, cfg->frame_reg, cfg->stack_usage - lmf_offset);
		/* r5 = previous_lmf */
		ppc_lwz (code, ppc_r5, G_STRUCT_OFFSET(MonoLMF, previous_lmf), ppc_r11);
		/* r6 = lmf_addr */
		ppc_lwz (code, ppc_r6, G_STRUCT_OFFSET(MonoLMF, lmf_addr), ppc_r11);
		/* *(lmf_addr) = previous_lmf */
		ppc_stw (code, ppc_r5, G_STRUCT_OFFSET(MonoLMF, previous_lmf), ppc_r6);
		/* FIXME: speedup: there is no actual need to restore the registers if
		 * we didn't actually change them (idea from Zoltan).
		 */
		/* restore iregs */
		ppc_lmw (code, ppc_r13, ppc_r11, G_STRUCT_OFFSET(MonoLMF, iregs));
		/* restore fregs */
		/*for (i = 14; i < 32; i++) {
			ppc_lfd (code, i, G_STRUCT_OFFSET(MonoLMF, fregs) + ((i-14) * sizeof (gdouble)), ppc_r11);
		}*/
		g_assert (ppc_is_imm16 (cfg->stack_usage + PPC_RET_ADDR_OFFSET));
		/* use the saved copy of the frame reg in r8 */
		if (1 || cfg->flags & MONO_CFG_HAS_CALLS) {
			ppc_lwz (code, ppc_r0, cfg->stack_usage + PPC_RET_ADDR_OFFSET, ppc_r8);
			ppc_mtlr (code, ppc_r0);
		}
		ppc_addic (code, ppc_sp, ppc_r8, cfg->stack_usage);
#endif
		ARM_ADD_REG_IMM8 (code, ARMREG_SP, ARMREG_SP, cfg->stack_usage);
		ARM_POP_NWB (code, cfg->used_int_regs | ((1 << ARMREG_SP) | (1 << ARMREG_PC)));
	} else {
		ARM_ADD_REG_IMM8 (code, ARMREG_SP, ARMREG_SP, cfg->stack_usage);
		ARM_POP_NWB (code, cfg->used_int_regs | ((1 << ARMREG_SP) | (1 << ARMREG_PC)));
	}

	cfg->code_len = code - cfg->native_code;

	g_assert (cfg->code_len < cfg->code_size);

}

/* remove once throw_exception_by_name is eliminated */
static int
exception_id_by_name (const char *name)
{
	if (strcmp (name, "IndexOutOfRangeException") == 0)
		return MONO_EXC_INDEX_OUT_OF_RANGE;
	if (strcmp (name, "OverflowException") == 0)
		return MONO_EXC_OVERFLOW;
	if (strcmp (name, "ArithmeticException") == 0)
		return MONO_EXC_ARITHMETIC;
	if (strcmp (name, "DivideByZeroException") == 0)
		return MONO_EXC_DIVIDE_BY_ZERO;
	if (strcmp (name, "InvalidCastException") == 0)
		return MONO_EXC_INVALID_CAST;
	if (strcmp (name, "NullReferenceException") == 0)
		return MONO_EXC_NULL_REF;
	if (strcmp (name, "ArrayTypeMismatchException") == 0)
		return MONO_EXC_ARRAY_TYPE_MISMATCH;
	g_error ("Unknown intrinsic exception %s\n", name);
}

void
mono_arch_emit_exceptions (MonoCompile *cfg)
{
	MonoJumpInfo *patch_info;
	int nthrows, i;
	guint8 *code;
	const guint8* exc_throw_pos [MONO_EXC_INTRINS_NUM] = {NULL};
	guint8 exc_throw_found [MONO_EXC_INTRINS_NUM] = {0};
	guint32 code_size;
	int exc_count = 0;
	int max_epilog_size = 50;

	/* count the number of exception infos */
     
	/* 
	 * make sure we have enough space for exceptions
	 * 12 is the simulated call to throw_exception_by_name
	 */
	for (patch_info = cfg->patch_info; patch_info; patch_info = patch_info->next) {
		if (patch_info->type == MONO_PATCH_INFO_EXC) {
			i = exception_id_by_name (patch_info->data.target);
			if (!exc_throw_found [i]) {
				max_epilog_size += 12;
				exc_throw_found [i] = TRUE;
			}
		}
	}

	while (cfg->code_len + max_epilog_size > (cfg->code_size - 16)) {
		cfg->code_size *= 2;
		cfg->native_code = g_realloc (cfg->native_code, cfg->code_size);
		mono_jit_stats.code_reallocs++;
	}

	code = cfg->native_code + cfg->code_len;

	/* add code to raise exceptions */
	for (patch_info = cfg->patch_info; patch_info; patch_info = patch_info->next) {
		switch (patch_info->type) {
		case MONO_PATCH_INFO_EXC: {
			unsigned char *ip = patch_info->ip.i + cfg->native_code;
			i = exception_id_by_name (patch_info->data.target);
			if (exc_throw_pos [i]) {
				arm_patch (ip, exc_throw_pos [i]);
				patch_info->type = MONO_PATCH_INFO_NONE;
				break;
			} else {
				exc_throw_pos [i] = code;
			}
			arm_patch (ip, code);
			/*mono_add_patch_info (cfg, code - cfg->native_code, MONO_PATCH_INFO_EXC_NAME, patch_info->data.target);*/
			ARM_LDR_IMM (code, ARMREG_R0, ARMREG_PC, 0);
			/* we got here from a conditional call, so the calling ip is set in lr already */
			patch_info->type = MONO_PATCH_INFO_INTERNAL_METHOD;
			patch_info->data.name = "mono_arch_throw_exception_by_name";
			patch_info->ip.i = code - cfg->native_code;
			ARM_B (code, 0);
			*(gpointer*)code = patch_info->data.target;
			code += 4;
			break;
		}
		default:
			/* do nothing */
			break;
		}
	}

	cfg->code_len = code - cfg->native_code;

	g_assert (cfg->code_len < cfg->code_size);

}

void
mono_arch_setup_jit_tls_data (MonoJitTlsData *tls)
{
}

void
mono_arch_free_jit_tls_data (MonoJitTlsData *tls)
{
}

void
mono_arch_emit_this_vret_args (MonoCompile *cfg, MonoCallInst *inst, int this_reg, int this_type, int vt_reg)
{
	
	int this_dreg = ARMREG_R0;
	
	if (vt_reg != -1)
		this_dreg = ARMREG_R1;

	/* add the this argument */
	if (this_reg != -1) {
		MonoInst *this;
		MONO_INST_NEW (cfg, this, OP_SETREG);
		this->type = this_type;
		this->sreg1 = this_reg;
		this->dreg = mono_regstate_next_int (cfg->rs);
		mono_bblock_add_inst (cfg->cbb, this);
		mono_call_inst_add_outarg_reg (inst, this->dreg, this_dreg, FALSE);
	}

	if (vt_reg != -1) {
		MonoInst *vtarg;
		MONO_INST_NEW (cfg, vtarg, OP_SETREG);
		vtarg->type = STACK_MP;
		vtarg->sreg1 = vt_reg;
		vtarg->dreg = mono_regstate_next_int (cfg->rs);
		mono_bblock_add_inst (cfg->cbb, vtarg);
		mono_call_inst_add_outarg_reg (inst, vtarg->dreg, ARMREG_R0, FALSE);
	}
}

MonoInst*
mono_arch_get_inst_for_method (MonoCompile *cfg, MonoMethod *cmethod, MonoMethodSignature *fsig, MonoInst **args)
{
	return NULL;
}

gboolean
mono_arch_print_tree (MonoInst *tree, int arity)
{
	return 0;
}

MonoInst* mono_arch_get_domain_intrinsic (MonoCompile* cfg)
{
	return NULL;
}

MonoInst* 
mono_arch_get_thread_intrinsic (MonoCompile* cfg)
{
	return NULL;
}

void
mono_arch_flush_register_windows (void)
{
}

