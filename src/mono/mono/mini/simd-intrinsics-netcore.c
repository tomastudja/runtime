/**
 * SIMD Intrinsics support for netcore
 */

#include <config.h>
#include <mono/utils/mono-compiler.h>

/*
 * Only LLVM is supported as a backend.
 */

#if !defined(DISABLE_JIT) && defined(ENABLE_NETCORE)

#include "mini.h"
#include "ir-emit.h"
#include "llvm-jit.h"
#include "mono/utils/bsearch.h"
#include <mono/metadata/abi-details.h>
#include <mono/metadata/reflection-internals.h>

#define MSGSTRFIELD(line) MSGSTRFIELD1(line)
#define MSGSTRFIELD1(line) str##line
static const struct msgstr_t {
#define METHOD(name) char MSGSTRFIELD(__LINE__) [sizeof (#name)];
#define METHOD2(str,name) char MSGSTRFIELD(__LINE__) [sizeof (str)];
#include "simd-methods-netcore.h"
#undef METHOD
#undef METHOD2
} method_names = {
#define METHOD(name) #name,
#define METHOD2(str,name) str,
#include "simd-methods-netcore.h"
#undef METHOD
#undef METHOD2
};

enum {
#define METHOD(name) SN_ ## name = offsetof (struct msgstr_t, MSGSTRFIELD(__LINE__)),
#define METHOD2(str,name) SN_ ## name = offsetof (struct msgstr_t, MSGSTRFIELD(__LINE__)),
#include "simd-methods-netcore.h"
};
#define method_name(idx) ((const char*)&method_names + (idx))

static int register_size;

void
mono_simd_intrinsics_init (void)
{
	register_size = 16;
#if FALSE
	if ((mono_llvm_get_cpu_features () & MONO_CPU_X86_AVX) != 0)
		register_size = 32;
#endif
	/* Tell the class init code the size of the System.Numerics.Register type */
	mono_simd_register_size = register_size;
}

MonoInst*
mono_emit_simd_field_load (MonoCompile *cfg, MonoClassField *field, MonoInst *addr)
{
	return NULL;
}

static int
simd_intrinsic_compare_by_name (const void *key, const void *value)
{
	return strcmp ((const char*)key, method_name (*(guint16*)value));
}

static int
lookup_intrins (guint16 *intrinsics, int size, MonoMethod *cmethod)
{
	const guint16 *result = (const guint16 *)mono_binary_search (cmethod->name, intrinsics, size / sizeof (guint16), sizeof (guint16), &simd_intrinsic_compare_by_name);

	for (int i = 0; i < (size / sizeof (guint16)) - 1; ++i) {
		if (method_name (intrinsics [i])[0] > method_name (intrinsics [i + 1])[0]) {
			printf ("%s %s\n",method_name (intrinsics [i]), method_name (intrinsics [i + 1]));
			g_assert_not_reached ();
		}
	}

	if (result == NULL)
		return -1;
	else
		return (int)*result;
}

static guint16 vector_methods [] = {
	SN_get_IsHardwareAccelerated
};

static MonoInst*
emit_sys_numerics_vector (MonoCompile *cfg, MonoMethod *cmethod, MonoMethodSignature *fsig, MonoInst **args)
{
	MonoInst *ins;
	gboolean supported = FALSE;
	int id;

	id = lookup_intrins (vector_methods, sizeof (vector_methods), cmethod);
	if (id == -1)
		return NULL;

	//printf ("%s\n", mono_method_full_name (cmethod, 1));

#ifdef MONO_ARCH_SIMD_INTRINSICS
	supported = TRUE;
#endif

	switch (id) {
	case SN_get_IsHardwareAccelerated:
		EMIT_NEW_ICONST (cfg, ins, supported ? 1 : 0);
		ins->type = STACK_I4;
		return ins;
	default:
		break;
	}

	return NULL;
}

static int
type_to_expand_op (MonoType *type)
{
	switch (type->type) {
	case MONO_TYPE_I1:
	case MONO_TYPE_U1:
		return OP_EXPAND_I1;
	case MONO_TYPE_I2:
	case MONO_TYPE_U2:
		return OP_EXPAND_I2;
	case MONO_TYPE_I4:
	case MONO_TYPE_U4:
		return OP_EXPAND_I4;
	case MONO_TYPE_I8:
	case MONO_TYPE_U8:
		return OP_EXPAND_I8;
	case MONO_TYPE_R4:
		return OP_EXPAND_R4;
	case MONO_TYPE_R8:
		return OP_EXPAND_R8;
	default:
		g_assert_not_reached ();
	}
}

/*
 * Return a simd vreg for the simd value represented by SRC.
 * SRC is the 'this' argument to methods.
 * Set INDIRECT to TRUE if the value was loaded from memory.
 */
static int
load_simd_vreg_class (MonoCompile *cfg, MonoClass *klass, MonoInst *src, gboolean *indirect)
{
	const char *spec = INS_INFO (src->opcode);

	if (indirect)
		*indirect = FALSE;
	if (src->opcode == OP_XMOVE) {
		return src->sreg1;
	} else if (src->opcode == OP_LDADDR) {
		int res = ((MonoInst*)src->inst_p0)->dreg;
		return res;
	} else if (spec [MONO_INST_DEST] == 'x') {
		return src->dreg;
	} else if (src->type == STACK_PTR || src->type == STACK_MP) {
		MonoInst *ins;
		if (indirect)
			*indirect = TRUE;

		MONO_INST_NEW (cfg, ins, OP_LOADX_MEMBASE);
		ins->klass = klass;
		ins->sreg1 = src->dreg;
		ins->type = STACK_VTYPE;
		ins->dreg = alloc_ireg (cfg);
		MONO_ADD_INS (cfg->cbb, ins);
		return ins->dreg;
	}
	g_warning ("load_simd_vreg:: could not infer source simd (%d) vreg for op", src->type);
	mono_print_ins (src);
	g_assert_not_reached ();
}

static int
load_simd_vreg (MonoCompile *cfg, MonoMethod *cmethod, MonoInst *src, gboolean *indirect)
{
	return load_simd_vreg_class (cfg, cmethod->klass, src, indirect);
}

/* Create and emit a SIMD instruction, dreg is auto-allocated */
static MonoInst*
emit_simd_ins (MonoCompile *cfg, MonoClass *klass, int opcode, int sreg1, int sreg2)
{
	const char *spec = INS_INFO (opcode);
	MonoInst *ins;

	MONO_INST_NEW (cfg, ins, opcode);
	if (spec [MONO_INST_DEST] == 'x') {
		ins->dreg = alloc_xreg (cfg);
		ins->type = STACK_VTYPE;
	} else if (spec [MONO_INST_DEST] == 'i') {
		ins->dreg = alloc_ireg (cfg);
		ins->type = STACK_I4;
	} else {
		g_assert_not_reached ();
	}
	ins->sreg1 = sreg1;
	ins->sreg2 = sreg2;
	ins->klass = klass;
	MONO_ADD_INS (cfg->cbb, ins);
	return ins;
}

static MonoInst*
emit_xcompare (MonoCompile *cfg, MonoClass *klass, MonoType *etype, MonoInst *arg1, MonoInst *arg2)
{
	MonoInst *ins;
	gboolean is_fp = etype->type == MONO_TYPE_R4 || etype->type == MONO_TYPE_R8;

	ins = emit_simd_ins (cfg, klass, is_fp ? OP_XCOMPARE_FP : OP_XCOMPARE, arg1->dreg, arg2->dreg);
	ins->inst_c0 = CMP_EQ;
	return ins;
}

static guint16 vector_t_methods [] = {
	SN_ctor,
	SN_Equals,
	SN_GreaterThan,
	SN_LessThan,
	SN_get_AllOnes,
	SN_get_Count,
	SN_get_Item,
	SN_get_Zero,
	SN_op_Addition,
	SN_op_BitwiseAnd,
	SN_op_BitwiseOr,
	SN_op_Division,
	SN_op_Equality,
	SN_op_ExclusiveOr,
	SN_op_Explicit,
	SN_op_Inequality,
	SN_op_Multiply,
	SN_op_Subtraction
};

static MonoInst*
emit_sys_numerics_vector_t (MonoCompile *cfg, MonoMethod *cmethod, MonoMethodSignature *fsig, MonoInst **args)
{
	MonoInst *ins;
	MonoType *type, *etype;
	MonoClass *klass;
	int size, len, id, index;
	gboolean is_unsigned;

	id = lookup_intrins (vector_t_methods, sizeof (vector_t_methods), cmethod);
	if (id == -1)
		return NULL;

	klass = cmethod->klass;
	type = m_class_get_byval_arg (klass);
	etype = mono_class_get_context (klass)->class_inst->type_argv [0];
	size = mono_class_value_size (mono_class_from_mono_type_internal (etype), NULL);
	g_assert (size);
	len = register_size / size;

	if (!MONO_TYPE_IS_PRIMITIVE (etype) || etype->type == MONO_TYPE_CHAR || etype->type == MONO_TYPE_BOOLEAN)
		return NULL;

	if (cfg->verbose_level > 1) {
		char *name = mono_method_full_name (cmethod, TRUE);
		printf ("  SIMD intrinsic %s\n", name);
		g_free (name);
	}

	switch (id) {
	case SN_get_Count:
		if (!(fsig->param_count == 0 && fsig->ret->type == MONO_TYPE_I4))
			break;
		EMIT_NEW_ICONST (cfg, ins, len);
		return ins;
	case SN_get_Zero:
		g_assert (fsig->param_count == 0 && mono_metadata_type_equal (fsig->ret, type));
		return emit_simd_ins (cfg, klass, OP_XZERO, -1, -1);
	case SN_get_AllOnes: {
		/* Compare a zero vector with itself */
		ins = emit_simd_ins (cfg, klass, OP_XZERO, -1, -1);
		return emit_xcompare (cfg, klass, etype, ins, ins);
	}
	case SN_get_Item:
		if (args [1]->opcode != OP_ICONST)
			return NULL;
		index = args [1]->inst_c0;
		if (index < 0 || index >= len)
			return NULL;
		return NULL;
	case SN_ctor:
		if (fsig->param_count == 1 && mono_metadata_type_equal (fsig->params [0], etype)) {
			int dreg = load_simd_vreg (cfg, cmethod, args [0], NULL);

			int opcode = type_to_expand_op (etype);
			ins = emit_simd_ins (cfg, klass, opcode, args [1]->dreg, -1);
			ins->dreg = dreg;
			return ins;
		}
		break;
	case SN_Equals:
		if (fsig->param_count == 1 && fsig->ret->type == MONO_TYPE_BOOLEAN && mono_metadata_type_equal (fsig->params [0], type)) {
			int sreg1 = load_simd_vreg (cfg, cmethod, args [0], NULL);

			return emit_simd_ins (cfg, klass, OP_XEQUAL, sreg1, args [1]->dreg);
		} else if (fsig->param_count == 2 && mono_metadata_type_equal (fsig->ret, type) && mono_metadata_type_equal (fsig->params [0], type) && mono_metadata_type_equal (fsig->params [1], type)) {
			/* Per element equality */
			return emit_xcompare (cfg, klass, etype, args [0], args [1]);
		}
		break;
	case SN_op_Equality:
	case SN_op_Inequality:
		g_assert (fsig->param_count == 2 && fsig->ret->type == MONO_TYPE_BOOLEAN &&
				  mono_metadata_type_equal (fsig->params [0], type) &&
				  mono_metadata_type_equal (fsig->params [1], type));
		ins = emit_simd_ins (cfg, klass, OP_XEQUAL, args [0]->dreg, args [1]->dreg);
		if (id == SN_op_Inequality) {
			int sreg = ins->dreg;
			int dreg = alloc_ireg (cfg);
			EMIT_NEW_UNALU (cfg, ins, OP_INOT, dreg, sreg);
		}
		return ins;
	case SN_GreaterThan:
	case SN_LessThan:
		g_assert (fsig->param_count == 2 && mono_metadata_type_equal (fsig->ret, type) && mono_metadata_type_equal (fsig->params [0], type) && mono_metadata_type_equal (fsig->params [1], type));
		is_unsigned = etype->type == MONO_TYPE_U1 || etype->type == MONO_TYPE_U2 || etype->type == MONO_TYPE_U4 || etype->type == MONO_TYPE_U8;
		ins = emit_xcompare (cfg, klass, etype, args [0], args [1]);
		switch (id) {
		case SN_GreaterThan:
			ins->inst_c0 = is_unsigned ? CMP_GT_UN : CMP_GT;
			break;
		case SN_LessThan:
			ins->inst_c0 = is_unsigned ? CMP_LT_UN : CMP_LT;
			break;
		default:
			g_assert_not_reached ();
		}
		return ins;
	case SN_op_Explicit:
		return emit_simd_ins (cfg, klass, OP_XMOVE, args [0]->dreg, -1);
	case SN_op_Addition:
	case SN_op_Subtraction:
	case SN_op_Division:
	case SN_op_Multiply:
	case SN_op_BitwiseAnd:
	case SN_op_BitwiseOr:
	case SN_op_ExclusiveOr:
		if (!(fsig->param_count == 2 && mono_metadata_type_equal (fsig->ret, type) && mono_metadata_type_equal (fsig->params [0], type) && mono_metadata_type_equal (fsig->params [1], type)))
			return NULL;
		ins = emit_simd_ins (cfg, klass, OP_XBINOP, args [0]->dreg, args [1]->dreg);
		if (etype->type == MONO_TYPE_R4 || etype->type == MONO_TYPE_R8) {
			switch (id) {
			case SN_op_Addition:
				ins->inst_c0 = OP_FADD;
				break;
			case SN_op_Subtraction:
				ins->inst_c0 = OP_FSUB;
				break;
			case SN_op_Multiply:
				ins->inst_c0 = OP_FMUL;
				break;
			case SN_op_Division:
				ins->inst_c0 = OP_FDIV;
				break;
			default:
				NULLIFY_INS (ins);
				return NULL;
			}
		} else {
			switch (id) {
			case SN_op_Addition:
				ins->inst_c0 = OP_IADD;
				break;
			case SN_op_Subtraction:
				ins->inst_c0 = OP_ISUB;
				break;
				/*
			case SN_op_Division:
				ins->inst_c0 = OP_IDIV;
				break;
			case SN_op_Multiply:
				ins->inst_c0 = OP_IMUL;
				break;
				*/
			case SN_op_BitwiseAnd:
				ins->inst_c0 = OP_IAND;
				break;
			case SN_op_BitwiseOr:
				ins->inst_c0 = OP_IOR;
				break;
			case SN_op_ExclusiveOr:
				ins->inst_c0 = OP_IXOR;
				break;
			default:
				NULLIFY_INS (ins);
				return NULL;
			}
		}
		return ins;
	default:
		break;
	}

	return NULL;
}

#ifdef TARGET_AMD64

static guint16 popcnt_methods [] = {
	SN_PopCount,
	SN_get_IsSupported
};

static guint16 bmi1_methods [] = {
	SN_TrailingZeroCount,
	SN_get_IsSupported,
};

static guint16 bmi2_methods [] = {
	SN_ParallelBitDeposit,
	SN_ParallelBitExtract,
	SN_get_IsSupported,
};

static MonoInst*
emit_x86_intrinsics (MonoCompile *cfg, MonoMethod *cmethod, MonoMethodSignature *fsig, MonoInst **args)
{
	const char *class_name;
	const char *class_ns;
	MonoInst *ins;
	int id;
	gboolean supported, is_64bit;
	MonoClass *klass = cmethod->klass;

	class_ns = m_class_get_name_space (klass);
	class_name = m_class_get_name (klass);
	if (!strcmp (class_name, "Popcnt") || (!strcmp (class_name, "X64") && cmethod->klass->nested_in && !strcmp (m_class_get_name (cmethod->klass->nested_in), "Popcnt"))) {
		id = lookup_intrins (popcnt_methods, sizeof (popcnt_methods), cmethod);
		if (id == -1)
			return NULL;

		supported = (mono_llvm_get_cpu_features () & MONO_CPU_X86_POPCNT) != 0;
		is_64bit = !strcmp (class_name, "X64");

		switch (id) {
		case SN_get_IsSupported:
			EMIT_NEW_ICONST (cfg, ins, supported ? 1 : 0);
			ins->type = STACK_I4;
			return ins;
		case SN_PopCount:
			if (!supported)
				return NULL;
			MONO_INST_NEW (cfg, ins, is_64bit ? OP_POPCNT64 : OP_POPCNT32);
			ins->dreg = alloc_ireg (cfg);
			ins->sreg1 = args [0]->dreg;
			MONO_ADD_INS (cfg->cbb, ins);
			return ins;
		default:
			return NULL;
		}
	}
	if (!strcmp (class_name, "Bmi1") || (!strcmp (class_name, "X64") && cmethod->klass->nested_in && !strcmp (m_class_get_name (cmethod->klass->nested_in), "Bmi1"))) {
		// We only support the subset used by corelib
		if (m_class_get_image (cfg->method->klass) != mono_get_corlib ())
			return NULL;
		id = lookup_intrins (bmi1_methods, sizeof (bmi1_methods), cmethod);
		g_assert (id != -1);
		supported = (mono_llvm_get_cpu_features () & MONO_CPU_X86_BMI1) != 0;
		is_64bit = !strcmp (class_name, "X64");

		switch (id) {
		case SN_get_IsSupported:
			EMIT_NEW_ICONST (cfg, ins, supported ? 1 : 0);
			ins->type = STACK_I4;
			return ins;
		case SN_TrailingZeroCount:
			MONO_INST_NEW (cfg, ins, is_64bit ? OP_CTTZ64 : OP_CTTZ32);
			ins->dreg = alloc_ireg (cfg);
			ins->sreg1 = args [0]->dreg;
			ins->type = STACK_I4;
			MONO_ADD_INS (cfg->cbb, ins);
			return ins;
		default:
			g_assert_not_reached ();
		}
	}
	if (!strcmp (class_name, "Bmi2") || (!strcmp (class_name, "X64") && cmethod->klass->nested_in && !strcmp (m_class_get_name (cmethod->klass->nested_in), "Bmi2"))) {
		// We only support the subset used by corelib
		if (m_class_get_image (cfg->method->klass) != mono_get_corlib ())
			return NULL;
		id = lookup_intrins (bmi2_methods, sizeof (bmi2_methods), cmethod);
		g_assert (id != -1);
		supported = (mono_llvm_get_cpu_features () & MONO_CPU_X86_BMI2) != 0;
		is_64bit = !strcmp (class_name, "X64");

		switch (id) {
		case SN_get_IsSupported:
			EMIT_NEW_ICONST (cfg, ins, supported ? 1 : 0);
			ins->type = STACK_I4;
			return ins;
		case SN_ParallelBitExtract:
			MONO_INST_NEW (cfg, ins, is_64bit ? OP_PEXT64 : OP_PEXT32);
			ins->dreg = alloc_ireg (cfg);
			ins->sreg1 = args [0]->dreg;
			ins->sreg2 = args [1]->dreg;
			ins->type = STACK_I4;
			MONO_ADD_INS (cfg->cbb, ins);
			return ins;
		case SN_ParallelBitDeposit:
			MONO_INST_NEW (cfg, ins, is_64bit ? OP_PDEP64 : OP_PDEP32);
			ins->dreg = alloc_ireg (cfg);
			ins->sreg1 = args [0]->dreg;
			ins->sreg2 = args [1]->dreg;
			ins->type = STACK_I4;
			MONO_ADD_INS (cfg->cbb, ins);
			return ins;
		default:
			g_assert_not_reached ();
		}
		//printf ("%s %s\n", mono_method_get_full_name (cfg->method), mono_method_get_full_name (cmethod));
	}

	return NULL;
}
#endif

MonoInst*
mono_emit_simd_intrinsics (MonoCompile *cfg, MonoMethod *cmethod, MonoMethodSignature *fsig, MonoInst **args)
{
	const char *class_name;
	const char *class_ns;
	MonoImage *image = m_class_get_image (cmethod->klass);

	if (image != mono_get_corlib ())
		return NULL;
	if (!COMPILE_LLVM (cfg))
		return NULL;
	// FIXME:
	if (cfg->compile_aot)
		return NULL;

	class_ns = m_class_get_name_space (cmethod->klass);
	class_name = m_class_get_name (cmethod->klass);
	if (!strcmp (class_ns, "System.Numerics") && !strcmp (class_name, "Vector"))
		return emit_sys_numerics_vector (cfg, cmethod, fsig, args);
	if (!strcmp (class_ns, "System.Numerics") && !strcmp (class_name, "Vector`1")) {
		MonoInst *ins = emit_sys_numerics_vector_t (cfg, cmethod, fsig, args);
		if (!ins) {
			//printf ("M: %s %s\n", mono_method_get_full_name (cfg->method), mono_method_get_full_name (cmethod));
		}
		return ins;
	}
#ifdef TARGET_AMD64
	if (cmethod->klass->nested_in)
		class_ns = m_class_get_name_space (cmethod->klass->nested_in), class_name, cmethod->klass->nested_in;
	if (!strcmp (class_ns, "System.Runtime.Intrinsics.X86"))
		return emit_x86_intrinsics (cfg ,cmethod, fsig, args);
#endif

	return NULL;
}

void
mono_simd_decompose_intrinsic (MonoCompile *cfg, MonoBasicBlock *bb, MonoInst *ins)
{
}

void
mono_simd_simplify_indirection (MonoCompile *cfg)
{
}

#else

MONO_EMPTY_SOURCE_FILE (simd_intrinsics_netcore);

#endif
