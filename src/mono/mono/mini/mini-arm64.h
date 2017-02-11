/**
 * \file
 *
 * Copyright 2013 Xamarin Inc
 *
 * Based on mini-arm.h:
 *
 * Copyright 2011 Xamarin Inc
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */

#ifndef __MONO_MINI_ARM64_H__
#define __MONO_MINI_ARM64_H__

#include <mono/arch/arm64/arm64-codegen.h>
#include <mono/mini/mini-arm64-gsharedvt.h>

#define MONO_ARCH_CPU_SPEC mono_arm64_cpu_desc

#define MONO_MAX_IREGS 32
#define MONO_MAX_FREGS 32

#define MONO_CONTEXT_SET_LLVM_EXC_REG(ctx, exc) do { (ctx)->regs [0] = (gsize)exc; } while (0)

#define MONO_INIT_CONTEXT_FROM_FUNC(ctx,func) do {	\
		MONO_CONTEXT_SET_BP ((ctx), __builtin_frame_address (0));	\
		MONO_CONTEXT_SET_SP ((ctx), __builtin_frame_address (0));	\
		MONO_CONTEXT_SET_IP ((ctx), (func));	\
	} while (0)

#define MONO_ARCH_INIT_TOP_LMF_ENTRY(lmf)

/* Parameters used by the register allocator */
/* r0..r7, r9..r14 (r15 is the imt/rgctx reg) */
#define MONO_ARCH_CALLEE_REGS 0xfeff
/* r19..r28 */
#define MONO_ARCH_CALLEE_SAVED_REGS (0x3ff << 19)

/* v16/v17 is reserved for a scratch reg */
#define MONO_ARCH_CALLEE_FREGS 0xfffc00ff
/* v8..v15 */
#define MONO_ARCH_CALLEE_SAVED_FREGS 0xff00

#define MONO_ARCH_USE_FPSTACK FALSE
#define MONO_ARCH_FPSTACK_SIZE 0

#define MONO_ARCH_INST_SREG2_MASK(ins) (0)

#define MONO_ARCH_INST_FIXED_REG(desc) ((desc) == 'a' ? ARMREG_R0 : -1)

#define MONO_ARCH_INST_IS_REGPAIR(desc) (0)

#define MONO_ARCH_INST_IS_FLOAT(desc) ((desc) == 'f')

#define MONO_ARCH_INST_REGPAIR_REG2(desc,hreg1) (-1)

#define MONO_ARCH_USE_FPSTACK FALSE

#define MONO_ARCH_FRAME_ALIGNMENT 16

#define MONO_ARCH_CODE_ALIGNMENT 32

/* callee saved regs + fp + sp */
#define MONO_ARCH_LMF_REGS ((0x3ff << 19) | (1 << ARMREG_FP) | (1 << ARMREG_SP))
#define MONO_ARCH_NUM_LMF_REGS (10 + 2)
#define MONO_ARCH_FIRST_LMF_REG ARMREG_R19
#define MONO_ARCH_LMF_REG_FP 10
#define MONO_ARCH_LMF_REG_SP 11

struct MonoLMF {
	/* 
	 * If the second lowest bit is set to 1, then this is a MonoLMFExt structure, and
	 * the other fields are not valid.
	 */
	gpointer    previous_lmf;
	gpointer    lmf_addr;
	mgreg_t    pc;
	mgreg_t    gregs [MONO_ARCH_NUM_LMF_REGS];
};

/* Structure used by the sequence points in AOTed code */
typedef struct {
	gpointer ss_trigger_page;
	gpointer bp_trigger_page;
	gpointer ss_tramp_addr;
	guint8* bp_addrs [MONO_ZERO_LEN_ARRAY];
} SeqPointInfo;

#define PARAM_REGS 8
#define FP_PARAM_REGS 8

#define DYN_CALL_STACK_ARGS 6

typedef struct {
	/* The +1 is for r8 */
	mgreg_t regs [PARAM_REGS + 1 + DYN_CALL_STACK_ARGS];
	mgreg_t res, res2;
	guint8 *ret;
	double fpregs [FP_PARAM_REGS];
	int n_fpargs, n_fpret;
	guint8 buffer [256];
} DynCallArgs;

typedef struct {
	gpointer cinfo;
	int saved_gregs_offset;
	/* Points to arguments received on the stack */
	int args_reg;
	gboolean cond_branch_islands;
	gpointer vret_addr_loc;
	gpointer seq_point_info_var;
	gpointer ss_tramp_var;
	gpointer bp_tramp_var;
	guint8 *thunks;
	int thunks_size;
} MonoCompileArch;

#define MONO_ARCH_EMULATE_FREM 1
#define MONO_ARCH_NO_EMULATE_LONG_MUL_OPTS 1
#define MONO_ARCH_EMULATE_LONG_MUL_OVF_OPTS 1
#define MONO_ARCH_NO_EMULATE_LONG_SHIFT_OPS 1
#define MONO_ARCH_NEED_DIV_CHECK 1
#define MONO_ARCH_EMULATE_MUL_OVF 1
#define MONO_ARCH_HAVE_IMT 1
#define MONO_ARCH_HAVE_OP_TAIL_CALL 1
#define MONO_ARCH_THIS_AS_FIRST_ARG 1
#define MONO_ARCH_RGCTX_REG ARMREG_R15
#define MONO_ARCH_IMT_REG MONO_ARCH_RGCTX_REG
#define MONO_ARCH_VTABLE_REG ARMREG_R0
#define MONO_ARCH_EXC_REG ARMREG_R0
#define MONO_ARCH_HAVE_XP_UNWIND 1
#define MONO_ARCH_HAVE_CREATE_DELEGATE_TRAMPOLINE 1
#define MONO_ARCH_HAVE_GENERALIZED_IMT_TRAMPOLINE 1
#define MONO_ARCH_USE_SIGACTION 1
#define MONO_ARCH_HAVE_SIGCTX_TO_MONOCTX 1
#define MONO_ARCH_HAVE_CONTEXT_SET_INT_REG 1
#define MONO_ARCH_GSHARED_SUPPORTED 1
#define MONO_ARCH_AOT_SUPPORTED 1
#define MONO_ARCH_LLVM_SUPPORTED 1
#define MONO_ARCH_HAVE_FULL_AOT_TRAMPOLINES 1
#define MONO_ARCH_HAVE_EXCEPTIONS_INIT 1
#define MONO_ARCH_HAVE_GET_TRAMPOLINES 1
#define MONO_ARCH_DYN_CALL_SUPPORTED 1
#define MONO_ARCH_DYN_CALL_PARAM_AREA (DYN_CALL_STACK_ARGS * 8)
#define MONO_ARCH_SOFT_DEBUG_SUPPORTED 1
#define MONO_ARCH_GSHAREDVT_SUPPORTED 1
#define MONO_ARCH_HAVE_SETUP_RESUME_FROM_SIGNAL_HANDLER_CTX 1
#define MONO_ARCH_HAVE_SETUP_ASYNC_CALLBACK 1
#define MONO_ARCH_HAVE_GENERAL_RGCTX_LAZY_FETCH_TRAMPOLINE 1
#define MONO_ARCH_HAVE_OP_GET_EX_OBJ 1
#define MONO_ARCH_HAVE_OBJC_GET_SELECTOR 1
#define MONO_ARCH_HAVE_SDB_TRAMPOLINES 1
#define MONO_ARCH_HAVE_PATCH_CODE_NEW 1
#define MONO_ARCH_HAVE_OP_GENERIC_CLASS_INIT 1
#define MONO_ARCH_HAVE_OPCODE_NEEDS_EMULATION 1
#define MONO_ARCH_HAVE_DECOMPOSE_LONG_OPTS 1
#define MONO_ARCH_HAVE_HANDLER_BLOCK_GUARD 1
#define MONO_ARCH_HAVE_HANDLER_BLOCK_GUARD_AOT 1

#ifdef TARGET_IOS

#define MONO_ARCH_REDZONE_SIZE 128

#else

#define MONO_ARCH_REDZONE_SIZE 0

#endif

#if defined(TARGET_APPLETVOS) || defined(TARGET_IOS)
#define MONO_ARCH_HAVE_UNWIND_BACKTRACE 1
#endif

/* Relocations */
#define MONO_R_ARM64_B 1
#define MONO_R_ARM64_BCC 2
#define MONO_R_ARM64_IMM 3
#define MONO_R_ARM64_BL 4
#define MONO_R_ARM64_BL_SHORT 5
#define MONO_R_ARM64_CBZ 6


typedef enum {
	ArgInIReg,
	ArgInFReg,
	ArgInFRegR4,
	ArgOnStack,
	ArgOnStackR8,
	ArgOnStackR4,
	/*
	 * Vtype passed in consecutive int registers.
	 * ainfo->reg is the firs register,
	 * ainfo->nregs is the number of registers,
	 * ainfo->size is the size of the structure.
	 */
	ArgVtypeInIRegs,
	ArgVtypeByRef,
	ArgVtypeByRefOnStack,
	ArgVtypeOnStack,
	ArgHFA,
	ArgNone
} ArgStorage;

typedef struct {
	ArgStorage storage;
	int reg;
	/* ArgOnStack */
	int offset;
	/* ArgVtypeInIRegs/ArgHFA */
	int nregs, size;
	/* ArgHFA */
	int esize;
	/* ArgHFA */
	/* The offsets of the float values inside the arg */
	guint16 foffsets [4];
	/* ArgOnStack */
	int slot_size;
	/* hfa */
	int nfregs_to_skip;
	gboolean sign;
	gboolean gsharedvt;
	gboolean hfa;
} ArgInfo;

typedef struct {
	int nargs;
	int gr, fr, stack_usage;
	gboolean pinvoke;
	ArgInfo ret;
	ArgInfo sig_cookie;
	ArgInfo args [1];
} CallInfo;


guint8* mono_arm_emit_imm64 (guint8 *code, int dreg, gint64 imm);

guint8* mono_arm_emit_ldrx (guint8 *code, int rt, int rn, int imm);

guint8* mono_arm_emit_destroy_frame (guint8 *code, int stack_offset, guint64 temp_regs);

guint8* mono_arm_emit_store_regset (guint8 *code, guint64 regs, int basereg, int offset);

guint8* mono_arm_emit_store_regarray (guint8 *code, guint64 regs, int basereg, int offset);

guint8* mono_arm_emit_load_regarray (guint8 *code, guint64 regs, int basereg, int offset);

/* MonoJumpInfo **ji */
guint8* mono_arm_emit_aotconst (gpointer ji, guint8 *code, guint8 *code_start, int dreg, guint32 patch_type, gconstpointer data);

void mono_arm_patch (guint8 *code, guint8 *target, int relocation);

void mono_arm_throw_exception (gpointer arg, mgreg_t pc, mgreg_t *int_regs, gdouble *fp_regs, gboolean corlib, gboolean rethrow);

void mono_arm_gsharedvt_init (void);

GSList* mono_arm_get_exception_trampolines (gboolean aot);

void mono_arm_resume_unwind (gpointer arg, mgreg_t pc, mgreg_t *int_regs, gdouble *fp_regs, gboolean corlib, gboolean rethrow);

gpointer mono_arm_handler_block_trampoline_helper (gpointer *ptr);

CallInfo* mono_arch_get_call_info (MonoMemPool *mp, MonoMethodSignature *sig);

#endif /* __MONO_MINI_ARM64_H__ */
