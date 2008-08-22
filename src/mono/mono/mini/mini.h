#ifndef __MONO_MINI_H__
#define __MONO_MINI_H__

#include "config.h"
#include <glib.h>
#include <signal.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/mempool.h>
#include <mono/utils/monobitset.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/opcodes.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/domain-internals.h>
#include "mono/metadata/class-internals.h"
#include "mono/metadata/object-internals.h"
#include <mono/metadata/profiler-private.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/utils/mono-compiler.h>

#define MONO_BREAKPOINT_ARRAY_SIZE 64

#include "mini-arch.h"
#include "regalloc.h"
#include "declsec.h"

#ifndef G_LIKELY
#define G_LIKELY(a) (a)
#define G_UNLIKELY(a) (a)
#endif

#ifndef G_MAXINT32
#define G_MAXINT32 2147483647
#endif

#ifndef G_MININT32
#define G_MININT32 (-G_MAXINT32 - 1)
#endif

#if DISABLE_LOGGING
#define MINI_DEBUG(level,limit,code)
#else
#define MINI_DEBUG(level,limit,code) do {if (G_UNLIKELY ((level) >= (limit))) code} while (0)
#endif

#define NOT_IMPLEMENTED do { g_assert_not_reached (); } while (0)

#ifndef DISABLE_AOT
#define MONO_USE_AOT_COMPILER
#endif

/* for 32 bit systems */
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
#define MINI_LS_WORD_IDX 0
#define MINI_MS_WORD_IDX 1
#else
#define MINI_LS_WORD_IDX 1
#define MINI_MS_WORD_IDX 0
#endif
#define MINI_LS_WORD_OFFSET (MINI_LS_WORD_IDX * 4)
#define MINI_MS_WORD_OFFSET (MINI_MS_WORD_IDX * 4)
#define inst_ls_word data.op[MINI_LS_WORD_IDX].const_val
#define inst_ms_word data.op[MINI_MS_WORD_IDX].const_val

#define MONO_FAKE_IMT_METHOD ((MonoMethod*)GINT_TO_POINTER(-1))
#define MONO_FAKE_VTABLE_METHOD ((MonoMethod*)GINT_TO_POINTER(-2))

/* Version number of the AOT file format */
#define MONO_AOT_FILE_VERSION "38"
 
/* Per-domain information maintained by the JIT */
typedef struct
{
	/* Maps MonoMethod's to a GSList of GOT slot addresses pointing to its code */
	GHashTable *jump_target_got_slot_hash;
} MonoJitDomainInfo;

#define jit_domain_info(domain) ((MonoJitDomainInfo*)((domain)->runtime_info))

#if 0
#define mono_bitset_foreach_bit(set,b,n) \
	for (b = 0; b < n; b++)\
		if (mono_bitset_test_fast(set,b))
#define mono_bitset_foreach_bit_rev(set,b,n) \
	for (b = n - 1; b >= 0; b--)\
		if (mono_bitset_test_fast(set,b))
#else
#define mono_bitset_foreach_bit(set,b,n) \
	for (b = mono_bitset_find_start (set); b < n && b >= 0; b = mono_bitset_find_first (set, b))
#define mono_bitset_foreach_bit_rev(set,b,n) \
	for (b = mono_bitset_find_last (set, n - 1); b >= 0; b = b ? mono_bitset_find_last (set, b) : -1)
 
#endif

/*
 * Pull the list of opcodes
 */
#define OPDEF(a,b,c,d,e,f,g,h,i,j) \
	a = i,

enum {
#include "mono/cil/opcode.def"
	CEE_LASTOP
};
#undef OPDEF

#define MONO_VARINFO(cfg,varnum) (&(cfg)->vars [varnum])

#define MONO_INST_NEW(cfg,dest,op) do {	\
		(dest) = mono_mempool_alloc0 ((cfg)->mempool, sizeof (MonoInst));	\
		(dest)->opcode = (op);	\
        (dest)->dreg = (dest)->sreg1 = (dest)->sreg2 = -1;  \
        (dest)->cil_code = (cfg)->ip;  \
        (dest)->cil_code = (cfg)->ip; \
	} while (0)

#define MONO_INST_NEW_CALL(cfg,dest,op) do {	\
		(dest) = mono_mempool_alloc0 ((cfg)->mempool, sizeof (MonoCallInst));	\
		(dest)->inst.opcode = (op);	\
        (dest)->inst.dreg = (dest)->inst.sreg1 = (dest)->inst.sreg2 = -1;  \
        (dest)->inst.cil_code = (cfg)->ip;  \
	} while (0)

#define MONO_INST_NEW_CALL_ARG(cfg,dest,op) do {	\
		(dest) = mono_mempool_alloc0 ((cfg)->mempool, sizeof (MonoCallArgParm));	\
		(dest)->ins.opcode = (op);	\
	} while (0)

#define MONO_ADD_INS(b,inst) do {	\
		if ((b)->last_ins) {	\
			(b)->last_ins->next = (inst);	\
            (inst)->prev = (b)->last_ins;   \
			(b)->last_ins = (inst);	\
		} else {	\
			(b)->code = (b)->last_ins = (inst);	\
		}	\
	} while (0)

#define NULLIFY_INS(ins) do { \
        (ins)->opcode = OP_NOP; \
        (ins)->dreg = (ins)->sreg1 = (ins)->sreg2 = -1; \
		(ins)->ssa_op = MONO_SSA_NOP; \
    } while (0)

/* Remove INS from BB */
#define MONO_REMOVE_INS(bb,ins) do { \
        if ((ins)->prev) \
            (ins)->prev->next = (ins)->next; \
        if ((ins)->next) \
            (ins)->next->prev = (ins)->prev; \
        if ((bb)->code == (ins)) \
            (bb)->code = (ins)->next; \
        if ((bb)->last_ins == (ins)) \
            (bb)->last_ins = (ins)->prev; \
    } while (0)

/* Remove INS from BB and nullify it */
#define MONO_DELETE_INS(bb,ins) do { \
        MONO_REMOVE_INS ((bb), (ins)); \
        NULLIFY_INS ((ins)); \
    } while (0)

/* 
 * this is used to determine when some branch optimizations are possible: we exclude FP compares
 * because they have weird semantics with NaNs.
 */
#define MONO_IS_COND_BRANCH_OP(ins) (((ins)->opcode >= CEE_BEQ && (ins)->opcode <= CEE_BLT_UN) || ((ins)->opcode >= OP_LBEQ && (ins)->opcode <= OP_LBLT_UN) || ((ins)->opcode >= OP_FBEQ && (ins)->opcode <= OP_FBLT_UN) || ((ins)->opcode >= OP_IBEQ && (ins)->opcode <= OP_IBLT_UN))
#define MONO_IS_COND_BRANCH_NOFP(ins) (MONO_IS_COND_BRANCH_OP(ins) && !(((ins)->opcode >= OP_FBEQ) && ((ins)->opcode <= OP_FBLT_UN)) && (!(ins)->inst_left || (ins)->inst_left->inst_left->type != STACK_R8))

#define MONO_IS_BRANCH_OP(ins) (MONO_IS_COND_BRANCH_OP(ins) || ((ins)->opcode == OP_BR) || ((ins)->opcode == OP_BR_REG) || ((ins)->opcode == OP_SWITCH))

#define MONO_IS_COND_EXC(ins) ((((ins)->opcode >= OP_COND_EXC_EQ) && ((ins)->opcode <= OP_COND_EXC_LT_UN)) || (((ins)->opcode >= OP_COND_EXC_IEQ) && ((ins)->opcode <= OP_COND_EXC_ILT_UN)))

#define MONO_IS_SETCC(ins) ((((ins)->opcode >= OP_CEQ) && ((ins)->opcode <= OP_CLT_UN)) || (((ins)->opcode >= OP_ICEQ) && ((ins)->opcode <= OP_ICLT_UN)) || (((ins)->opcode >= OP_LCEQ) && ((ins)->opcode <= OP_LCLT_UN)) || (((ins)->opcode >= OP_FCEQ) && ((ins)->opcode <= OP_FCLT_UN)))

#define MONO_IS_PHI(ins) (((ins)->opcode == OP_PHI) || ((ins)->opcode == OP_FPHI) || ((ins)->opcode == OP_VPHI))
#define MONO_IS_MOVE(ins) (((ins)->opcode == OP_MOVE) || ((ins)->opcode == OP_FMOVE) || ((ins)->opcode == OP_VMOVE))

#define MONO_IS_LOAD_MEMBASE(ins) (((ins)->opcode >= OP_LOAD_MEMBASE) && ((ins)->opcode <= OP_LOADV_MEMBASE))
#define MONO_IS_STORE_MEMBASE(ins) (((ins)->opcode >= OP_STORE_MEMBASE_REG) && ((ins)->opcode <= OP_STOREV_MEMBASE))
#define MONO_IS_STORE_MEMINDEX(ins) (((ins)->opcode >= OP_STORE_MEMINDEX) && ((ins)->opcode <= OP_STORER8_MEMINDEX))

#define MONO_IS_CALL(ins) (((ins->opcode >= OP_VOIDCALL) && (ins->opcode <= OP_VOIDCALL_MEMBASE)) || ((ins->opcode >= OP_FCALL) && (ins->opcode <= OP_FCALL_MEMBASE)) || ((ins->opcode >= OP_LCALL) && (ins->opcode <= OP_LCALL_MEMBASE)) || ((ins->opcode >= OP_VCALL) && (ins->opcode <= OP_VCALL_MEMBASE)) || ((ins->opcode >= OP_CALL) && (ins->opcode <= OP_CALL_MEMBASE)) || ((ins->opcode >= OP_VCALL2) && (ins->opcode <= OP_VCALL2_MEMBASE)) || (ins->opcode == OP_TAILCALL))

/* FIXME: Handle OP_GOT_ENTRY too */
#define MONO_IS_JUMP_TABLE(ins) (((ins)->opcode == OP_JUMP_TABLE) ? TRUE : ((((ins)->opcode == OP_AOTCONST) && (ins->inst_i1 == (gpointer)MONO_PATCH_INFO_SWITCH)) ? TRUE : ((ins)->opcode == OP_SWITCH) ? TRUE : FALSE))

#define MONO_JUMP_TABLE_FROM_INS(ins) (((ins)->opcode == OP_JUMP_TABLE) ? (ins)->inst_p0 : (((ins)->opcode == OP_AOTCONST) && (ins->inst_i1 == (gpointer)MONO_PATCH_INFO_SWITCH) ? (ins)->inst_p0 : (((ins)->opcode == OP_SWITCH) ? (ins)->inst_p0 : NULL)))

/* FIXME: Add more instructions */
#define MONO_INS_HAS_NO_SIDE_EFFECT(ins) (MONO_IS_MOVE (ins) || (ins->opcode == OP_ICONST) || (ins->opcode == OP_I8CONST) || (ins->opcode == OP_VZERO) || (ins->opcode == OP_ADD_IMM) || (ins->opcode == OP_R8CONST) || (ins->opcode == OP_LADD_IMM) || (ins->opcode == OP_ISUB_IMM) || (ins->opcode == OP_IADD_IMM) || (ins->opcode == OP_INEG) || (ins->opcode == OP_LNEG) || (ins->opcode == OP_ISUB) || (ins->opcode == OP_CMOV_IGE) || (ins->opcode == OP_ISHL_IMM) || (ins->opcode == OP_ISHR_IMM) || (ins->opcode == OP_ISHR_UN_IMM) || (ins->opcode == OP_IAND_IMM) || (ins->opcode == OP_ICONV_TO_U1) || (ins->opcode == OP_ICONV_TO_I1) || (ins->opcode == OP_SEXT_I4) || (ins->opcode == OP_LCONV_TO_U1) || (ins->opcode == OP_ICONV_TO_U2) || (ins->opcode == OP_ICONV_TO_I2) || (ins->opcode == OP_LCONV_TO_I2))

typedef struct MonoInstList MonoInstList;
typedef struct MonoInst MonoInst;
typedef struct MonoCallInst MonoCallInst;
typedef struct MonoCallArgParm MonoCallArgParm;
typedef struct MonoEdge MonoEdge;
typedef struct MonoMethodVar MonoMethodVar;
typedef struct MonoBasicBlock MonoBasicBlock;
typedef struct MonoLMF MonoLMF;
typedef struct MonoSpillInfo MonoSpillInfo;
typedef struct MonoTraceSpec MonoTraceSpec;

extern guint32 mono_jit_tls_id;
extern MonoTraceSpec *mono_jit_trace_calls;
extern gboolean mono_break_on_exc;
extern int mono_exc_esp_offset;
#ifdef DISABLE_AOT
#define mono_compile_aot 0
#else
extern gboolean mono_compile_aot;
#endif
extern gboolean mono_aot_only;
extern gboolean mono_use_imt;
extern MonoMethodDesc *mono_inject_async_exc_method;
extern int mono_inject_async_exc_pos;
extern MonoMethodDesc *mono_break_at_bb_method;
extern int mono_break_at_bb_bb_num;
extern gboolean check_for_pending_exc;
extern gboolean disable_vtypes_in_regs;
extern gboolean mono_verify_all;

#define INS_INFO(opcode) (&ins_info [((opcode) - OP_START - 1) * 3])

extern const char ins_info[];

#define MONO_BB_FOR_EACH_INS(bb, ins) for ((ins) = (bb)->code; (ins); (ins) = (ins)->next)

#define MONO_BB_FOR_EACH_INS_SAFE(bb, n, ins) for ((ins) = (bb)->code, n = (ins) ? (ins)->next : NULL; (ins); (ins) = (n), (n) = (ins) ? (ins)->next : NULL)

#define MONO_BB_FOR_EACH_INS_REVERSE_SAFE(bb, p, ins) for ((ins) = (bb)->last_ins, p = (ins) ? (ins)->prev : NULL; (ins); (ins) = (p), (p) = (ins) ? (ins)->prev : NULL)

#define mono_bb_first_ins(bb) (bb)->code

#if 0

static inline void
MONO_INST_LIST_ADD_TAIL (MonoInstList *new, MonoInstList *head)
{
	__MONO_INST_LIST_ADD (new, head->prev, head);
}

static inline void
__MONO_INST_LIST_DEL (MonoInstList *prev, MonoInstList *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void
__MONO_INST_LIST_SPLICE (MonoInstList *list, MonoInstList *head)
{
	MonoInstList *first = list->next;
	MonoInstList *last = list->prev;
	MonoInstList *at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

static inline void
MONO_INST_LIST_SPLICE (MonoInstList *list, MonoInstList *head) 
{
	if (!MONO_INST_LIST_EMPTY (list))
		__MONO_INST_LIST_SPLICE (list, head);
}

static inline void
MONO_INST_LIST_SPLICE_TAIL (MonoInstList *list, MonoInstList *head) 
{
	if (!MONO_INST_LIST_EMPTY (list))
		__MONO_INST_LIST_SPLICE (list, head->prev);
}

static inline void
MONO_INST_LIST_SPLICE_INIT (MonoInstList *list, MonoInstList *head)
{
	if (!MONO_INST_LIST_EMPTY (list)) {
		__MONO_INST_LIST_SPLICE (list, head);
		MONO_INST_LIST_INIT (list);
	}
}

static inline void
MONO_INST_LIST_SPLICE_TAIL_INIT (MonoInstList *list, MonoInstList *head)
{
	if (!MONO_INST_LIST_EMPTY (list)) {
		__MONO_INST_LIST_SPLICE (list, head->prev);
		MONO_INST_LIST_INIT (list);
	}
}

/*#define mono_container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define MONO_INST_LIST_ENTRY(ptr, type, member) \
	mono_container_of(ptr, type, member)*/

#define MONO_INST_LIST_ENTRY(ptr, type, member)	\
	((type *)(gpointer)(ptr))

#define MONO_INST_LIST_FIRST_ENTRY(ptr, type, member) \
	MONO_INST_LIST_ENTRY((ptr)->next, type, member)

#define MONO_INST_LIST_LAST_ENTRY(ptr, type, member) \
	MONO_INST_LIST_ENTRY((ptr)->prev, type, member)

#define MONO_INST_LIST_FOR_EACH(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define MONO_INST_LIST_FOR_EACH_PREV(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define MONO_INST_LIST_FOR_EACH_SAFE(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

#define MONO_INST_LIST_FOR_EACH_PREV_SAFE(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; pos != (head); \
		pos = n, n = pos->prev)

#define MONO_INST_LIST_FOR_EACH_ENTRY(pos, head, member) \
	for (pos = MONO_INST_LIST_ENTRY ((head)->next, MonoInst, member);\
	     &pos->member != (head);\
	     pos = MONO_INST_LIST_ENTRY (pos->member.next, MonoInst, member))

#define MONO_INST_LIST_FOR_EACH_ENTRY_REVERSE(pos, head, member) \
	for (pos = MONO_INST_LIST_ENTRY ((head)->prev, MonoInst, member);\
	     &pos->member != (head);\
	     pos = MONO_INST_LIST_ENTRY (pos->member.prev, MonoInst, member))

#define MONO_INST_LIST_FOR_EACH_ENTRY_SAFE(pos, n, head, member) \
	for (pos = MONO_INST_LIST_ENTRY ((head)->next, MonoInst, member),\
		n = MONO_INST_LIST_ENTRY (pos->member.next, MonoInst, member);\
	     &pos->member != (head); 					\
	     pos = n, n = MONO_INST_LIST_ENTRY (n->member.next, MonoInst, member))

#define MONO_BB_FOR_EACH_INS(bb, ins) MONO_INST_LIST_FOR_EACH_ENTRY ((ins), &((bb)->ins_list), node)

#define MONO_BB_FOR_EACH_INS_SAFE(bb, next, ins) MONO_INST_LIST_FOR_EACH_ENTRY_SAFE ((ins), (next), &((bb)->ins_list), node)

#define MONO_BB_FOR_EACH_INS_REVERSE(bb, ins) MONO_INST_LIST_FOR_EACH_ENTRY_REVERSE ((ins), &((bb)->ins_list), node)

#endif

struct MonoEdge {
	MonoEdge *next;
	MonoBasicBlock *bb;
	/* add edge type? */
};

struct MonoSpillInfo {
	int offset;
};

/*
 * The IR-level extended basic block.  
 *
 * A basic block can have multiple exits just fine, as long as the point of
 * 'departure' is the last instruction in the basic block. Extended basic
 * blocks, on the other hand, may have instructions that leave the block
 * midstream. The important thing is that they cannot be _entered_
 * midstream, ie, execution of a basic block (or extened bb) always start
 * at the beginning of the block, never in the middle.
 */
struct MonoBasicBlock {
	MonoInst *last_ins;

	/* the next basic block in the order it appears in IL */
	MonoBasicBlock *next_bb;

	/*
	 * Before instruction selection it is the first tree in the
	 * forest and the first item in the list of trees. After
	 * instruction selection it is the first instruction and the
	 * first item in the list of instructions.
	 */
	MonoInst *code;

	/* unique block number identification */
	gint32 block_num;
	
	gint32 dfn;

	/* Basic blocks: incoming and outgoing counts and pointers */
	/* Each bb should only appear once in each array */
	gint16 out_count, in_count;
	MonoBasicBlock **in_bb;
	MonoBasicBlock **out_bb;

	/* Points to the start of the CIL code that initiated this BB */
	unsigned char* cil_code;

	/* Length of the CIL block */
	gint32 cil_length;

	/* The address of the generated code, used for fixups */
	int native_offset;
	int max_offset;

	/* Visited and reachable flags */
	guint32 flags;

	/*
	 * SSA and loop based flags
	 */
	MonoBitSet *dominators;
	MonoBitSet *dfrontier;
	MonoBasicBlock *idom;
	GSList *dominated;
	/* fast dominator algorithm */
	MonoBasicBlock *df_parent, *ancestor, *child, *label;
	MonoEdge *bucket;
	int size, sdom, idomn;
	
	/* loop nesting and recognition */
	GList *loop_blocks;
	gint8  nesting;
	gint8  loop_body_start;

	/* 
	 * Whenever the bblock is rarely executed so it should be emitted after
	 * the function epilog.
	 */
	guint out_of_line : 1;
	/* Caches the result of uselessness calculation during optimize_branches */
	guint not_useless : 1;
	/* Whenever the decompose_array_access_opts () pass needs to process this bblock */
	guint has_array_access : 1;
	/* Whenever this bblock is extended, ie. it has branches inside it */
	guint extended : 1;
	
	/* use for liveness analysis */
	MonoBitSet *gen_set;
	MonoBitSet *kill_set;
	MonoBitSet *live_in_set;
	MonoBitSet *live_out_set;

	/* fields to deal with non-empty stack slots at bb boundary */
	guint16 out_scount, in_scount;
	MonoInst **out_stack;
	MonoInst **in_stack;

	/* we use that to prevent merging of bblocks covered by different clauses*/
	guint real_offset;

	/*
	 * The region encodes whether the basic block is inside
	 * a finally, catch, filter or none of these.
	 *
	 * If the value is -1, then it is neither finally, catch nor filter
	 *
	 * Otherwise the format is:
	 *
	 *  Bits: |     0-3      |       4-7      |     8-31
	 * 	  |		 |                |
	 *        | clause-flags |   MONO_REGION  | clause-index 
	 *
	 */
        guint region;

	/* The current symbolic register number, used in local register allocation. */
	guint32 max_vreg;
};

/* BBlock flags */
enum {
	BB_VISITED            = 1 << 0,
	BB_REACHABLE          = 1 << 1,
	BB_EXCEPTION_DEAD_OBJ = 1 << 2,
	BB_EXCEPTION_UNSAFE   = 1 << 3,
	BB_EXCEPTION_HANDLER  = 1 << 4
};

typedef struct MonoMemcpyArgs {
	int size, align;
} MonoMemcpyArgs;

struct MonoInst {
 	guint16 opcode;
	guint8  type; /* stack type */
	guint   ssa_op : 3;
	guint8  flags  : 5;
	
	/* used by the register allocator */
	gint32 dreg, sreg1, sreg2;

	MonoInst *next, *prev;

	union {
		union {
			MonoInst *src;
			MonoMethodVar *var;
			gssize const_val;
			gpointer p;
			MonoMethod *method;
			MonoMethodSignature *signature;
			MonoBasicBlock **many_blocks;
			MonoBasicBlock *target_block;
			MonoInst **args;
			MonoType *vtype;
			MonoClass *klass;
			int *phi_args;
			MonoCallInst *call_inst;
		} op [2];
		gint64 i8const;
		double r8const;
	} data;

	const unsigned char* cil_code; /* for debugging and bblock splitting */

	/* used mostly by the backend to store additional info it may need */
	union {
		gint32 reg3;
		gint32 arg_info;
		gint32 size;
		MonoMemcpyArgs *memcpy_args; /* in OP_MEMSET and OP_MEMCPY */
		gpointer data;
		gint shift_amount;
		gboolean is_pinvoke; /* for variables in the unmanaged marshal format */
		gboolean record_cast_details; /* For CEE_CASTCLASS */
	} backend;
	
	MonoClass *klass;
};
	
struct MonoCallInst {
	MonoInst inst;
	MonoMethodSignature *signature;
	MonoMethod *method;
	MonoInst **args;
	MonoInst *out_args;
	MonoInst *vret_var;
	gconstpointer fptr;
	guint stack_usage;
	guint virtual : 1;
	guint tail_call : 1;
	/* If this is TRUE, 'fptr' points to a MonoJumpInfo instead of an address. */
	guint fptr_is_patch : 1;
	/*
	 * If this is true, then the call returns a vtype in a register using the same 
	 * calling convention as OP_CALL.
	 */
	guint vret_in_reg : 1;
	regmask_t used_iregs;
	regmask_t used_fregs;
	GSList *out_ireg_args;
	GSList *out_freg_args;
};

struct MonoCallArgParm {
	MonoInst ins;
	gint32 size;
	gint32 offset;
	gint32 offPrm;
};

/* 
 * flags for MonoInst
 * Note: some of the values overlap, because they can't appear
 * in the same MonoInst.
 */
enum {
	MONO_INST_HAS_METHOD = 1,
	/* temp local created by a DUP: used only within a BB */
	MONO_INST_IS_TEMP    = 1,
	MONO_INST_INIT       = 1, /* in localloc */
	MONO_INST_IS_DEAD    = 2,
	MONO_INST_TAILCALL   = 4,
	MONO_INST_VOLATILE   = 4,
	MONO_INST_BRLABEL    = 4,
	MONO_INST_NOTYPECHECK    = 4,
	MONO_INST_UNALIGNED  = 8,
    MONO_INST_CFOLD_TAKEN = 8, /* On branches */
    MONO_INST_CFOLD_NOT_TAKEN = 16, /* On branches */
	MONO_INST_DEFINITION_HAS_SIDE_EFFECTS = 8,
	/* the address of the variable has been taken */
	MONO_INST_INDIRECT   = 16,
	MONO_INST_NORANGECHECK   = 16
};

#define inst_c0 data.op[0].const_val
#define inst_c1 data.op[1].const_val
#define inst_i0 data.op[0].src
#define inst_i1 data.op[1].src
#define inst_p0 data.op[0].p
#define inst_p1 data.op[1].p
#define inst_l  data.i8const
#define inst_r  data.r8const
#define inst_left  data.op[0].src
#define inst_right data.op[1].src

#define inst_newa_len   data.op[0].src
#define inst_newa_class data.op[1].klass

#define inst_var    data.op[0].var
#define inst_vtype  data.op[1].vtype
/* in branch instructions */
#define inst_many_bb   data.op[1].many_blocks
#define inst_target_bb data.op[0].target_block
#define inst_true_bb   data.op[1].many_blocks[0]
#define inst_false_bb  data.op[1].many_blocks[1]

#define inst_basereg sreg1
#define inst_indexreg sreg2
#define inst_destbasereg dreg
#define inst_offset data.op[0].const_val
#define inst_imm    data.op[1].const_val
#define inst_call   data.op[1].call_inst

#define inst_phi_args   data.op[1].phi_args

/* instruction description for use in regalloc/scheduling */
enum {
	MONO_INST_DEST,
	MONO_INST_SRC1,
	MONO_INST_SRC2,
	MONO_INST_LEN,
	MONO_INST_CLOB,
	/* Unused, commented out to reduce the size of the mdesc tables
	MONO_INST_FLAGS,
	MONO_INST_COST,
	MONO_INST_DELAY,
	MONO_INST_RES,
	*/
	MONO_INST_MAX
};

typedef union {
	struct {
		guint16 tid; /* tree number */
		guint16 bid; /* block number */
	} pos ;
	guint32 abs_pos; 
} MonoPosition;

typedef struct {
	MonoPosition first_use, last_use;
} MonoLiveRange;

typedef struct MonoLiveRange2 MonoLiveRange2;

struct MonoLiveRange2 {
	int from, to;
	MonoLiveRange2 *next;
};

typedef struct {
	/* List of live ranges sorted by 'from' */
	MonoLiveRange2 *range;
	MonoLiveRange2 *last_range;
} MonoLiveInterval;

/*
 * Additional information about a variable
 */
struct MonoMethodVar {
	guint           idx; /* inside cfg->varinfo, cfg->vars */
	MonoLiveRange   range; /* generated by liveness analysis */
	MonoLiveInterval *interval; /* generated by liveness analysis */
	int             reg; /* != -1 if allocated into a register */
	int             spill_costs;
	MonoBitSet     *def_in; /* used by SSA */
	MonoInst       *def;    /* used by SSA */
	MonoBasicBlock *def_bb; /* used by SSA */
	GList          *uses;   /* used by SSA */
	char            cpstate;  /* used by SSA conditional  constant propagation */
};

typedef struct {
	gpointer          end_of_stack;
	guint32           stack_size;
#if !defined(HAVE_KW_THREAD) || !defined(MONO_ARCH_ENABLE_MONO_LMF_VAR)
	MonoLMF          *lmf;
#endif
	MonoLMF          *first_lmf;
	gpointer         signal_stack;
	guint32          signal_stack_size;
	gpointer         stack_ovf_guard_base;
	guint32          stack_ovf_guard_size;
	void            (*abort_func) (MonoObject *object);
	/* Used to implement --debug=casts */
	MonoClass       *class_cast_from, *class_cast_to;
} MonoJitTlsData;

typedef enum {
#define PATCH_INFO(a,b) MONO_PATCH_INFO_ ## a,
#include "patch-info.h"
#undef PATCH_INFO
	MONO_PATCH_INFO_NUM
} MonoJumpInfoType;

/*
 * We need to store the image which the token refers to along with the token,
 * since the image might not be the same as the image of the method which
 * contains the relocation, because of inlining.
 */
typedef struct MonoJumpInfoToken {
	MonoImage *image;
	guint32 token;
} MonoJumpInfoToken;

typedef struct MonoJumpInfoBBTable {
	MonoBasicBlock **table;
	int table_size;
} MonoJumpInfoBBTable;

typedef struct MonoJumpInfoRgctxEntry MonoJumpInfoRgctxEntry;

typedef struct MonoJumpInfo MonoJumpInfo;
struct MonoJumpInfo {
	MonoJumpInfo *next;
	union {
		int i;
		guint8 *p;
		MonoInst *label;
	} ip;

	MonoJumpInfoType type;
	union {
		gconstpointer   target;
#if SIZEOF_VOID_P == 8
		gint64          offset;
#else
		int             offset;
#endif
		MonoBasicBlock *bb;
		MonoInst       *inst;
		MonoMethod     *method;
		MonoClass      *klass;
		MonoClassField *field;
		MonoImage      *image;
		MonoVTable     *vtable;
		const char     *name;
		MonoJumpInfoToken  *token;
		MonoJumpInfoBBTable *table;
		MonoJumpInfoRgctxEntry *rgctx_entry;
	} data;
};
 
/* Contains information describing an rgctx entry */
struct MonoJumpInfoRgctxEntry {
	MonoMethod *method;
	gboolean in_mrgctx;
	MonoJumpInfo *data; /* describes the data to be loaded */
	int info_type;
};

typedef enum {
	MONO_TRAMPOLINE_JIT,
	MONO_TRAMPOLINE_JUMP,
	MONO_TRAMPOLINE_CLASS_INIT,
	MONO_TRAMPOLINE_GENERIC_CLASS_INIT,
	MONO_TRAMPOLINE_RGCTX_LAZY_FETCH,
	MONO_TRAMPOLINE_AOT,
	MONO_TRAMPOLINE_AOT_PLT,
	MONO_TRAMPOLINE_DELEGATE,
	MONO_TRAMPOLINE_NUM
} MonoTrampolineType;

/* optimization flags */
#define OPTFLAG(id,shift,name,descr) MONO_OPT_ ## id = 1 << shift,
enum {
#include "optflags-def.h"
	MONO_OPT_LAST
};

/* Bit-fields in the MonoBasicBlock.region */
#define MONO_REGION_TRY       0
#define MONO_REGION_FINALLY  16
#define MONO_REGION_CATCH    32
#define MONO_REGION_FAULT    64         /* Currently unused */
#define MONO_REGION_FILTER  128

#define MONO_BBLOCK_IS_IN_REGION(bblock, regtype) (((bblock)->region & (0xf << 4)) == (regtype))

#define get_vreg_to_inst(cfg, vreg) ((vreg) < (cfg)->vreg_to_inst_len ? (cfg)->vreg_to_inst [(vreg)] : NULL)

#define vreg_is_volatile(cfg, vreg) (G_UNLIKELY (get_vreg_to_inst ((cfg), (vreg)) && (get_vreg_to_inst ((cfg), (vreg))->flags & (MONO_INST_VOLATILE|MONO_INST_INDIRECT))))

/*
 * Control Flow Graph and compilation unit information
 */
typedef struct {
	MonoMethod      *method;
	MonoMemPool     *mempool;
	MonoInst       **varinfo;
	MonoMethodVar   *vars;
	MonoInst        *ret;
	MonoBasicBlock  *bb_entry;
	MonoBasicBlock  *bb_exit;
	MonoBasicBlock  *bb_init;
	MonoBasicBlock **bblocks;
	MonoBasicBlock **cil_offset_to_bb;
	MonoMemPool     *state_pool; /* used by instruction selection */
	MonoBasicBlock  *cbb;        /* used by instruction selection */
	MonoInst        *prev_ins;   /* in decompose */
	MonoJumpInfo    *patch_info;
	MonoJitInfo     *jit_info;
	MonoJitDynamicMethodInfo *dynamic_info;
	guint            num_bblocks;
	guint            locals_start;
	guint            num_varinfo; /* used items in varinfo */
	guint            varinfo_count; /* total storage in varinfo */
	gint             stack_offset;
	gint             max_ireg;
	gint             cil_offset_to_bb_len;
	MonoRegState    *rs;
	MonoSpillInfo   *spill_info; /* machine register spills */
	MonoSpillInfo   *spill_info_float; /* fp register spills */
	gint             spill_count;
	gint             spill_info_len, spill_info_float_len;
	/* unsigned char   *cil_code; */
	MonoMethod      *inlined_method; /* the method which is currently inlined */
	MonoInst        *domainvar; /* a cache for the current domain */
	MonoInst        *got_var; /* Global Offset Table variable */
	MonoInst        **locals;
	MonoInst	*rgctx_var; /* Runtime generic context variable (for static generic methods) */
	MonoInst        **args;
	MonoType        **arg_types;
	MonoMethod      *current_method; /* The method currently processed by method_to_ir () */
	MonoGenericContext *generic_context;

	/* 
	 * This variable represents the hidden argument holding the vtype
	 * return address. If the method returns something other than a vtype, or
	 * the vtype is returned in registers this is NULL.
	 */
	MonoInst        *vret_addr;

	/*
	 * This is used to initialize the cil_code field of MonoInst's.
	 */
	const unsigned char *ip;
	
	struct MonoAliasingInformation *aliasing_info;

	/* A hashtable of region ID-> SP var mappings */
	/* An SP var is a place to store the stack pointer (used by handlers)*/
	GHashTable      *spvars;

	/* A hashtable of region ID -> EX var mappings */
	/* An EX var stores the exception object passed to catch/filter blocks */
	GHashTable      *exvars;

	GList           *ldstr_list; /* used by AOT */
	
	MonoDomain      *domain;

	guint            real_offset;
	GHashTable      *cbb_hash;

	/* The current virtual register number */
	guint32 next_vreg;

	MonoGenericSharingContext *generic_sharing_context;

	unsigned char   *cil_start;
	unsigned char   *native_code;
	guint            code_size;
	guint            code_len;
	guint            prolog_end;
	guint            epilog_begin;
	regmask_t        used_int_regs;
	guint32          opt;
	guint32          prof_options;
	guint32          flags;
	guint32          comp_done;
	guint32          verbose_level;
	guint32          stack_usage;
	guint32          param_area;
	guint32          frame_reg;
	gint32           sig_cookie;
	guint            disable_aot : 1;
	guint            disable_ssa : 1;
	guint            enable_extended_bblocks : 1;
	guint            run_cctors : 1;
	guint            need_lmf_area : 1;
	guint            compile_aot : 1;
	guint            got_var_allocated : 1;
	guint            ret_var_is_local : 1;
	guint            ret_var_set : 1;
	guint            new_ir : 1;
	guint            globalra : 1;
	guint            unverifiable : 1;
	guint            skip_visibility : 1;
	guint            disable_reuse_registers : 1;
	guint            disable_reuse_stack_slots : 1;
	guint            disable_initlocals_opt : 1;
	guint            disable_omit_fp : 1;
	guint            has_got_slots : 1;
	guint            uses_rgctx_reg : 1;
	guint            uses_vtable_reg : 1;
	gpointer         debug_info;
	guint32          lmf_offset;
    guint16          *intvars;
	MonoProfileCoverageInfo *coverage_info;
	GHashTable       *token_info_hash;
	MonoCompileArch  arch;
	guint32          exception_type;	/* MONO_EXCEPTION_* */
	guint32          exception_data;
	char*            exception_message;
	gpointer         exception_ptr;

	/* Fields used by the local reg allocator */
	void*            reginfo;
	int              reginfo_len;

	/* Maps vregs to their associated MonoInst's */
	/* vregs with an associated MonoInst are 'global' while others are 'local' */
	MonoInst **vreg_to_inst;

	/* Size of above array */
	guint32 vreg_to_inst_len;

	/* 
	 * The original method to compile, differs from 'method' when doing generic
	 * sharing.
	 */
	MonoMethod *orig_method;

	/* Patches which describe absolute addresses embedded into the native code */
	GHashTable *abs_patches;
} MonoCompile;

typedef enum {
	MONO_CFG_HAS_ALLOCA = 1 << 0,
	MONO_CFG_HAS_CALLS  = 1 << 1,
	MONO_CFG_HAS_LDELEMA  = 1 << 2,
	MONO_CFG_HAS_VARARGS  = 1 << 3,
	MONO_CFG_HAS_TAIL     = 1 << 4,
	MONO_CFG_HAS_FPOUT    = 1 << 5, /* there are fp values passed in int registers */
	MONO_CFG_HAS_SPILLUP  = 1 << 6, /* spill var slots are allocated from bottom to top */
	MONO_CFG_HAS_CHECK_THIS  = 1 << 7,
	MONO_CFG_HAS_ARRAY_ACCESS = 1 << 8
} MonoCompileFlags;

typedef struct {
	gulong methods_compiled;
	gulong methods_aot;
	gulong methods_lookups;
	gulong method_trampolines;
	gulong allocate_var;
	gulong analyze_stack_repeat;
	gulong cil_code_size;
	gulong native_code_size;
	gulong code_reallocs;
	gulong max_code_size_ratio;
	gulong biggest_method_size;
	gulong allocated_code_size;
	gulong inlineable_methods;
	gulong inlined_methods;
	gulong basic_blocks;
	gulong max_basic_blocks;
	gulong locals_stack_size;
	gulong regvars;
	gulong cas_declsec_check;
	gulong cas_linkdemand_icall;
	gulong cas_linkdemand_pinvoke;
	gulong cas_linkdemand_aptc;
	gulong cas_linkdemand;
	gulong cas_demand_generation;
	gulong generic_virtual_invocations;
	char *max_ratio_method;
	char *biggest_method;
	gboolean enabled;
} MonoJitStats;

extern MonoJitStats mono_jit_stats;

/* values for MonoInst.ssa_op */
enum {
	MONO_SSA_NOP = 0,
	MONO_SSA_ADDRESS_TAKEN = 1,
	MONO_SSA_LOAD = 2,
	MONO_SSA_STORE = 4,
	MONO_SSA_LOAD_STORE = MONO_SSA_LOAD|MONO_SSA_STORE,
	MONO_SSA_INDIRECT_LOAD = MONO_SSA_LOAD|MONO_SSA_ADDRESS_TAKEN,
	MONO_SSA_INDIRECT_STORE = MONO_SSA_STORE|MONO_SSA_ADDRESS_TAKEN,
	MONO_SSA_INDIRECT_LOAD_STORE =
	MONO_SSA_LOAD|MONO_SSA_STORE|MONO_SSA_ADDRESS_TAKEN
};

/* opcodes: value assigned after all the CIL opcodes */
#ifdef MINI_OP
#undef MINI_OP
#endif
#define MINI_OP(a,b,dest,src1,src2) a,
enum {
	OP_START = MONO_CEE_LAST - 1,
#include "mini-ops.h"
	OP_LAST
};
#undef MINI_OP

/* Can't use the same with both JITs since that would break the burg rules */
#if defined(NEW_IR)

#if SIZEOF_VOID_P == 8
#define OP_PCONST OP_I8CONST
#define OP_PADD OP_LADD
#define OP_PADD_IMM OP_LADD_IMM
#define OP_PSUB OP_LSUB
#define OP_PMUL OP_LMUL
#define OP_PMUL_IMM OP_LMUL_IMM
#define OP_PNEG OP_LNEG
#define OP_PCONV_TO_I1 OP_LCONV_TO_I1
#define OP_PCONV_TO_U1 OP_LCONV_TO_U1
#define OP_PCONV_TO_I2 OP_LCONV_TO_I2
#define OP_PCONV_TO_U2 OP_LCONV_TO_U2
#define OP_PCONV_TO_OVF_I1_UN OP_LCONV_TO_OVF_I1_UN
#define OP_PCONV_TO_OVF_I1 OP_LCONV_TO_OVF_I1
#define OP_PBEQ OP_LBEQ
#define OP_PCEQ OP_LCEQ
#define OP_PBNE_UN OP_LBNE_UN
#define OP_PBGE_UN OP_LBGE_UN
#define OP_PBLT_UN OP_LBLT_UN
#define OP_PBGE OP_LBGE
#define OP_STOREP_MEMBASE_REG OP_STOREI8_MEMBASE_REG
#define OP_STOREP_MEMBASE_IMM OP_STOREI8_MEMBASE_IMM
#else
#define OP_PCONST OP_ICONST
#define OP_PADD OP_IADD
#define OP_PADD_IMM OP_IADD_IMM
#define OP_PSUB OP_ISUB
#define OP_PMUL OP_IMUL
#define OP_PMUL_IMM OP_IMUL_IMM
#define OP_PNEG OP_INEG
#define OP_PCONV_TO_U2 OP_ICONV_TO_U2
#define OP_PCONV_TO_OVF_I1_UN OP_ICONV_TO_OVF_I1_UN
#define OP_PCONV_TO_OVF_I1 OP_ICONV_TO_OVF_I1
#define OP_PBEQ OP_IBEQ
#define OP_PCEQ OP_ICEQ
#define OP_PBNE_UN OP_IBNE_UN
#define OP_PBGE_UN OP_IBGE_UN
#define OP_PBLT_UN OP_IBLT_UN
#define OP_PBGE OP_IBGE
#define OP_STOREP_MEMBASE_REG OP_STOREI4_MEMBASE_REG
#define OP_STOREP_MEMBASE_IMM OP_STOREI4_MEMBASE_IMM
#endif

#else

#if SIZEOF_VOID_P == 8
#define OP_PCONST OP_I8CONST
#define OP_PADD OP_LADD
#define OP_PADD_IMM OP_LADD_IMM
#define OP_PNEG OP_LNEG
#define OP_PCONV_TO_I1 OP_LCONV_TO_I1
#define OP_PCONV_TO_U1 OP_LCONV_TO_U1
#define OP_PCONV_TO_I2 OP_LCONV_TO_I2
#define OP_PCONV_TO_U2 OP_LCONV_TO_U2
#define OP_PCONV_TO_OVF_I1_UN OP_LCONV_TO_OVF_I1_UN
#define OP_PCONV_TO_OVF_I1 OP_LCONV_TO_OVF_I1
#define OP_PBEQ OP_LBEQ
#define OP_PCEQ CEE_CEQ
#define OP_STOREP_MEMBASE_REG OP_STOREI8_MEMBASE_REG
#define OP_STOREP_MEMBASE_IMM OP_STOREI8_MEMBASE_IMM
#else
#define OP_PCONST OP_ICONST
#define OP_PADD CEE_ADD
#define OP_PADD2 OP_IADD
#define OP_PNEG CEE_NEG
#define OP_PCONV_TO_I1 OP_ICONV_TO_I1
#define OP_PCONV_TO_U1 OP_ICONV_TO_U1
#define OP_PCONV_TO_I2 OP_ICONV_TO_I2
#define OP_PCONV_TO_U2 CEE_CONV_U2
#define OP_PCONV_TO_OVF_I1_UN CEE_CONV_OVF_I1_UN
#define OP_PCONV_TO_OVF_I1 CEE_CONV_OVF_I1
#define OP_PBEQ OP_IBEQ
#define OP_PCEQ CEE_CEQ
#define OP_STOREP_MEMBASE_REG OP_STOREI4_MEMBASE_REG
#define OP_STOREP_MEMBASE_IMM OP_STOREI4_MEMBASE_IMM
#endif

#endif

typedef enum {
	STACK_INV,
	STACK_I4,
	STACK_I8,
	STACK_PTR,
	STACK_R8,
	STACK_MP,
	STACK_OBJ,
	STACK_VTYPE,
	STACK_MAX
} MonoStackType;

typedef struct {
	union {
		double   r8;
		gint32   i4;
		gint64   i8;
		gpointer p;
		MonoClass *klass;
	} data;
	int type;
} StackSlot;

#if HAVE_ARRAY_ELEM_INIT
extern const guint8 mono_burg_arity [];
#else
extern guint8 mono_burg_arity [];
#endif

extern const char MONO_ARCH_CPU_SPEC [] MONO_INTERNAL;
#define MONO_ARCH_CPU_SPEC_IDX_COMBINE(a) a ## _idx
#define MONO_ARCH_CPU_SPEC_IDX(a) MONO_ARCH_CPU_SPEC_IDX_COMBINE(a)
extern const guint16 MONO_ARCH_CPU_SPEC_IDX(MONO_ARCH_CPU_SPEC) [] MONO_INTERNAL;
#define ins_get_spec(op) ((const char*)&MONO_ARCH_CPU_SPEC + MONO_ARCH_CPU_SPEC_IDX(MONO_ARCH_CPU_SPEC)[(op)])

enum {
	MONO_COMP_DOM = 1,
	MONO_COMP_IDOM = 2,
	MONO_COMP_DFRONTIER = 4,
	MONO_COMP_DOM_REV = 8,
	MONO_COMP_LIVENESS = 16,
	MONO_COMP_SSA = 32,
	MONO_COMP_SSA_DEF_USE = 64,
	MONO_COMP_REACHABILITY = 128,
	MONO_COMP_LOOPS = 256
};

typedef enum {
	MONO_GRAPH_CFG = 1,
	MONO_GRAPH_DTREE = 2,
	MONO_GRAPH_CFG_CODE = 4,
	MONO_GRAPH_CFG_SSA = 8,
	MONO_GRAPH_CFG_OPTCODE = 16
} MonoGraphOptions;

typedef struct {
	guint16 size;
	guint16 offset;
	guint8  pad;
} MonoJitArgumentInfo;

typedef struct {
	gboolean handle_sigint;
	gboolean keep_delegates;
	gboolean collect_pagefault_stats;
	gboolean break_on_unverified;
	gboolean better_cast_details;
	gboolean mdb_optimizations;
	gboolean no_gdb_backtrace;
} MonoDebugOptions;

enum {
	BRANCH_NOT_TAKEN,
	BRANCH_TAKEN,
	BRANCH_UNDEF
};

typedef enum {
	CMP_EQ,
	CMP_NE,
	CMP_LE,
	CMP_GE,
	CMP_LT,
	CMP_GT,
	CMP_LE_UN,
	CMP_GE_UN,
	CMP_LT_UN,
	CMP_GT_UN
} CompRelation;

typedef enum {
	CMP_TYPE_L,
	CMP_TYPE_I,
	CMP_TYPE_F
} CompType;

/* Implicit exceptions */
enum {
	MONO_EXC_INDEX_OUT_OF_RANGE,
	MONO_EXC_OVERFLOW,
	MONO_EXC_ARITHMETIC,
	MONO_EXC_DIVIDE_BY_ZERO,
	MONO_EXC_INVALID_CAST,
	MONO_EXC_NULL_REF,
	MONO_EXC_ARRAY_TYPE_MISMATCH,
	MONO_EXC_INTRINS_NUM
};

enum {
	MINI_TOKEN_SOURCE_CLASS,
	MINI_TOKEN_SOURCE_METHOD,
	MINI_TOKEN_SOURCE_FIELD
};

typedef void (*MonoInstFunc) (MonoInst *tree, gpointer data);

/* main function */
int         mono_main                      (int argc, char* argv[]);
void        mono_set_defaults              (int verbose_level, guint32 opts);
MonoDomain* mini_init                      (const char *filename, const char *runtime_version) MONO_INTERNAL;
void        mini_cleanup                   (MonoDomain *domain) MONO_INTERNAL;
MonoDebugOptions *mini_get_debug_options   (void) MONO_INTERNAL;

/* helper methods */
MonoJumpInfoToken * mono_jump_info_token_new (MonoMemPool *mp, MonoImage *image, guint32 token) MONO_INTERNAL;
MonoInst* mono_find_spvar_for_region        (MonoCompile *cfg, int region) MONO_INTERNAL;
void      mono_precompile_assemblies        (void) MONO_INTERNAL;
int       mono_parse_default_optimizations  (const char* p);
void      mono_bblock_add_inst              (MonoBasicBlock *bb, MonoInst *inst) MONO_INTERNAL;
void      mono_bblock_insert_after_ins      (MonoBasicBlock *bb, MonoInst *ins, MonoInst *ins_to_insert) MONO_INTERNAL;
void      mono_bblock_insert_before_ins     (MonoBasicBlock *bb, MonoInst *ins, MonoInst *ins_to_insert) MONO_INTERNAL;
void      mono_verify_bblock                (MonoBasicBlock *bb) MONO_INTERNAL;
void      mono_verify_cfg                   (MonoCompile *cfg) MONO_INTERNAL;
void      mono_constant_fold                (MonoCompile *cfg) MONO_INTERNAL;
void      mono_constant_fold_inst           (MonoInst *inst, gpointer data) MONO_INTERNAL;
MonoInst* mono_constant_fold_ins2           (MonoCompile *cfg, MonoInst *ins, MonoInst *arg1, MonoInst *arg2, gboolean overwrite) MONO_INTERNAL;
int       mono_eval_cond_branch             (MonoInst *branch) MONO_INTERNAL;
int       mono_is_power_of_two              (guint32 val) MONO_INTERNAL;
void      mono_cprop_local                  (MonoCompile *cfg, MonoBasicBlock *bb, MonoInst **acp, int acp_size) MONO_INTERNAL;
MonoInst* mono_compile_create_var           (MonoCompile *cfg, MonoType *type, int opcode) MONO_INTERNAL;
MonoInst* mono_compile_create_var_for_vreg  (MonoCompile *cfg, MonoType *type, int opcode, int vreg) MONO_INTERNAL;
void      mono_compile_make_var_load        (MonoCompile *cfg, MonoInst *dest, gssize var_index) MONO_INTERNAL;
MonoInst* mono_compile_create_var_load      (MonoCompile *cfg, gssize var_index) MONO_INTERNAL;
MonoInst* mono_compile_create_var_store     (MonoCompile *cfg, gssize var_index, MonoInst *value) MONO_INTERNAL;
MonoType* mono_type_from_stack_type         (MonoInst *ins) MONO_INTERNAL;
guint32   mono_alloc_ireg                   (MonoCompile *cfg) MONO_INTERNAL;
guint32   mono_alloc_freg                   (MonoCompile *cfg) MONO_INTERNAL;
guint32   mono_alloc_preg                   (MonoCompile *cfg) MONO_INTERNAL;
guint32   mono_alloc_dreg                   (MonoCompile *cfg, MonoStackType stack_type) MONO_INTERNAL;

void      mono_link_bblock                  (MonoCompile *cfg, MonoBasicBlock *from, MonoBasicBlock* to) MONO_INTERNAL;
void      mono_unlink_bblock                (MonoCompile *cfg, MonoBasicBlock *from, MonoBasicBlock* to) MONO_INTERNAL;
gboolean  mono_bblocks_linked               (MonoBasicBlock *bb1, MonoBasicBlock *bb2) MONO_INTERNAL;
void      mono_remove_bblock                (MonoCompile *cfg, MonoBasicBlock *bb) MONO_INTERNAL;
void      mono_nullify_basic_block          (MonoBasicBlock *bb) MONO_INTERNAL;
void      mono_merge_basic_blocks           (MonoCompile *cfg, MonoBasicBlock *bb, MonoBasicBlock *bbn) MONO_INTERNAL;
void      mono_optimize_branches            (MonoCompile *cfg) MONO_INTERNAL;

void      mono_blockset_print               (MonoCompile *cfg, MonoBitSet *set, const char *name, guint idom) MONO_INTERNAL;
void      mono_print_tree                   (MonoInst *tree) MONO_INTERNAL;
void      mono_print_tree_nl                (MonoInst *tree) MONO_INTERNAL;
void      mono_print_ins_index              (int i, MonoInst *ins) MONO_INTERNAL;
void      mono_print_ins                    (MonoInst *ins) MONO_INTERNAL;
void      mono_print_bb                     (MonoBasicBlock *bb, const char *msg) MONO_INTERNAL;
void      mono_print_code                   (MonoCompile *cfg, const char *msg) MONO_INTERNAL;
void      mono_print_method_from_ip         (void *ip);
char     *mono_pmip                         (void *ip);
void      mono_select_instructions          (MonoCompile *cfg) MONO_INTERNAL;
const char* mono_inst_name                  (int op);
int       mono_op_to_op_imm                 (int opcode) MONO_INTERNAL;
int       mono_op_imm_to_op                 (int opcode) MONO_INTERNAL;
int       mono_load_membase_to_load_mem     (int opcode) MONO_INTERNAL;
guint     mono_type_to_load_membase         (MonoCompile *cfg, MonoType *type) MONO_INTERNAL;
guint     mono_type_to_store_membase        (MonoCompile *cfg, MonoType *type) MONO_INTERNAL;
guint     mini_type_to_stind                (MonoCompile* cfg, MonoType *type) MONO_INTERNAL;
guint32   mono_reverse_branch_op            (guint32 opcode) MONO_INTERNAL;
void      mono_inst_foreach                 (MonoInst *tree, MonoInstFunc func, gpointer data) MONO_INTERNAL;
void      mono_disassemble_code             (MonoCompile *cfg, guint8 *code, int size, char *id) MONO_INTERNAL;
void      mono_add_patch_info               (MonoCompile *cfg, int ip, MonoJumpInfoType type, gconstpointer target) MONO_INTERNAL;
void      mono_remove_patch_info            (MonoCompile *cfg, int ip) MONO_INTERNAL;
MonoJumpInfo* mono_patch_info_dup_mp        (MonoMemPool *mp, MonoJumpInfo *patch_info) MONO_INTERNAL;
guint     mono_patch_info_hash (gconstpointer data) MONO_INTERNAL;
gint      mono_patch_info_equal (gconstpointer ka, gconstpointer kb) MONO_INTERNAL;
MonoJumpInfo *mono_patch_info_list_prepend  (MonoJumpInfo *list, int ip, MonoJumpInfoType type, gconstpointer target) MONO_INTERNAL;
gpointer  mono_resolve_patch_target         (MonoMethod *method, MonoDomain *domain, guint8 *code, MonoJumpInfo *patch_info, gboolean run_cctors) MONO_INTERNAL;
gpointer  mono_jit_find_compiled_method     (MonoDomain *domain, MonoMethod *method) MONO_INTERNAL;
MonoLMF * mono_get_lmf                      (void) MONO_INTERNAL;
MonoLMF** mono_get_lmf_addr                 (void) MONO_INTERNAL;
void      mono_jit_thread_attach            (MonoDomain *domain);
guint32   mono_get_jit_tls_key              (void) MONO_INTERNAL;
gint32    mono_get_jit_tls_offset           (void) MONO_INTERNAL;
gint32    mono_get_lmf_tls_offset           (void) MONO_INTERNAL;
gint32    mono_get_lmf_addr_tls_offset      (void) MONO_INTERNAL;
MonoInst* mono_get_jit_tls_intrinsic        (MonoCompile *cfg) MONO_INTERNAL;
GList    *mono_varlist_insert_sorted        (MonoCompile *cfg, GList *list, MonoMethodVar *mv, gboolean sort_end) MONO_INTERNAL;
GList    *mono_varlist_sort                 (MonoCompile *cfg, GList *list, int sort_type) MONO_INTERNAL;
void      mono_analyze_liveness             (MonoCompile *cfg) MONO_INTERNAL;
void      mono_linear_scan                  (MonoCompile *cfg, GList *vars, GList *regs, regmask_t *used_mask) MONO_INTERNAL;
void      mono_global_regalloc              (MonoCompile *cfg) MONO_INTERNAL;
void      mono_create_jump_table            (MonoCompile *cfg, MonoInst *label, MonoBasicBlock **bbs, int num_blocks) MONO_INTERNAL;
int       mono_compile_assembly             (MonoAssembly *ass, guint32 opts, const char *aot_options) MONO_INTERNAL;
MonoCompile *mini_method_compile            (MonoMethod *method, guint32 opts, MonoDomain *domain, gboolean run_cctors, gboolean compile_aot, int parts) MONO_INTERNAL;
void      mono_destroy_compile              (MonoCompile *cfg) MONO_INTERNAL;
MonoJitICallInfo *mono_find_jit_opcode_emulation (int opcode) MONO_INTERNAL;
void	  mono_print_ins_index (int i, MonoInst *ins) MONO_INTERNAL;
void	  mono_print_ins (MonoInst *ins) MONO_INTERNAL;
gboolean  mini_assembly_can_skip_verification (MonoDomain *domain, MonoMethod *method) MONO_INTERNAL;
gboolean  mini_method_verify (MonoCompile *cfg, MonoMethod *method) MONO_INTERNAL;

gboolean  mini_class_is_system_array (MonoClass *klass) MONO_INTERNAL;
MonoMethodSignature *mono_get_element_address_signature (int arity) MONO_INTERNAL;
MonoJitICallInfo    *mono_get_element_address_icall (int rank) MONO_INTERNAL;
MonoJitICallInfo    *mono_get_array_new_va_icall (int rank) MONO_INTERNAL;

void      mono_linterval_add_range          (MonoCompile *cfg, MonoLiveInterval *interval, int from, int to) MONO_INTERNAL;
void      mono_linterval_print              (MonoLiveInterval *interval) MONO_INTERNAL;
void      mono_linterval_print_nl (MonoLiveInterval *interval) MONO_INTERNAL;
gboolean  mono_linterval_covers             (MonoLiveInterval *interval, int pos) MONO_INTERNAL;
gint32    mono_linterval_get_intersect_pos  (MonoLiveInterval *i1, MonoLiveInterval *i2) MONO_INTERNAL;
void      mono_linterval_split              (MonoCompile *cfg, MonoLiveInterval *interval, MonoLiveInterval **i1, MonoLiveInterval **i2, int pos) MONO_INTERNAL;
void      mono_liveness_handle_exception_clauses (MonoCompile *cfg) MONO_INTERNAL;

/* AOT */
void      mono_aot_init                     (void) MONO_INTERNAL;
gpointer  mono_aot_get_method               (MonoDomain *domain,
											 MonoMethod *method) MONO_INTERNAL;
gpointer  mono_aot_get_method_from_token    (MonoDomain *domain, MonoImage *image, guint32 token) MONO_INTERNAL;
gboolean  mono_aot_is_got_entry             (guint8 *code, guint8 *addr) MONO_INTERNAL;
guint8*   mono_aot_get_plt_entry            (guint8 *code) MONO_INTERNAL;
guint32   mono_aot_get_plt_info_offset      (gssize *regs, guint8 *code) MONO_INTERNAL;
gboolean  mono_aot_get_cached_class_info    (MonoClass *klass, MonoCachedClassInfo *res) MONO_INTERNAL;
gboolean  mono_aot_get_class_from_name      (MonoImage *image, const char *name_space, const char *name, MonoClass **klass) MONO_INTERNAL;
MonoJitInfo* mono_aot_find_jit_info         (MonoDomain *domain, MonoImage *image, gpointer addr) MONO_INTERNAL;
void mono_aot_set_make_unreadable           (gboolean unreadable) MONO_INTERNAL;
gboolean mono_aot_is_pagefault              (void *ptr) MONO_INTERNAL;
void mono_aot_handle_pagefault              (void *ptr) MONO_INTERNAL;
guint32 mono_aot_get_n_pagefaults           (void) MONO_INTERNAL;
gpointer mono_aot_plt_resolve               (gpointer aot_module, guint32 plt_info_offset, guint8 *code) MONO_INTERNAL;
gpointer mono_aot_get_method_from_vt_slot   (MonoDomain *domain, MonoVTable *vtable, int slot) MONO_INTERNAL;
gpointer mono_aot_create_specific_trampoline   (MonoImage *image, gpointer arg1, MonoTrampolineType tramp_type, MonoDomain *domain, guint32 *code_len) MONO_INTERNAL;
gpointer mono_aot_get_named_code            (const char *name) MONO_INTERNAL;
gpointer mono_aot_get_unbox_trampoline      (MonoMethod *method) MONO_INTERNAL;
/* This is an exported function */
void     mono_aot_register_globals          (gpointer *globals);
/* This too */
void     mono_aot_register_module           (gpointer *aot_info);

gboolean  mono_method_blittable             (MonoMethod *method) MONO_INTERNAL;
gboolean  mono_method_same_domain           (MonoJitInfo *caller, MonoJitInfo *callee) MONO_INTERNAL;

void      mono_register_opcode_emulation    (int opcode, const char* name, const char *sigstr, gpointer func, gboolean no_throw) MONO_INTERNAL;
void      mono_draw_graph                   (MonoCompile *cfg, MonoGraphOptions draw_options) MONO_INTERNAL;
void      mono_add_varcopy_to_end           (MonoCompile *cfg, MonoBasicBlock *bb, int src, int dest) MONO_INTERNAL;
void      mono_add_ins_to_end               (MonoBasicBlock *bb, MonoInst *inst) MONO_INTERNAL;
gpointer  mono_create_ftnptr                (MonoDomain *domain, gpointer addr) MONO_INTERNAL;

void      mono_replace_ins                  (MonoCompile *cfg, MonoBasicBlock *bb, MonoInst *ins, MonoInst **prev, MonoBasicBlock *first_bb, MonoBasicBlock *last_bb);

int               mono_find_method_opcode      (MonoMethod *method) MONO_INTERNAL;
MonoJitICallInfo *mono_find_jit_icall_by_name  (const char *name) MONO_INTERNAL;
MonoJitICallInfo *mono_find_jit_icall_by_addr  (gconstpointer addr) MONO_INTERNAL;
MonoJitICallInfo *mono_register_jit_icall      (gconstpointer func, const char *name, MonoMethodSignature *sig, gboolean is_save) MONO_INTERNAL;
gconstpointer     mono_icall_get_wrapper       (MonoJitICallInfo* callinfo) MONO_INTERNAL;

void              mono_trampolines_init (void) MONO_INTERNAL;
void              mono_trampolines_cleanup (void) MONO_INTERNAL;
guint8 *          mono_get_trampoline_code (MonoTrampolineType tramp_type) MONO_INTERNAL;
gpointer          mono_create_specific_trampoline (gpointer arg1, MonoTrampolineType tramp_type, MonoDomain *domain, guint32 *code_len) MONO_INTERNAL;
gpointer          mono_create_jump_trampoline (MonoDomain *domain, 
											   MonoMethod *method, 
											   gboolean add_sync_wrapper) MONO_INTERNAL;
gpointer          mono_create_class_init_trampoline (MonoVTable *vtable) MONO_INTERNAL;
gpointer          mono_create_generic_class_init_trampoline (void) MONO_INTERNAL;
gpointer          mono_create_jit_trampoline (MonoMethod *method) MONO_INTERNAL;
gpointer          mono_create_jit_trampoline_from_token (MonoImage *image, guint32 token) MONO_INTERNAL;
gpointer          mono_create_jit_trampoline_in_domain (MonoDomain *domain, MonoMethod *method) MONO_INTERNAL;
gpointer          mono_create_delegate_trampoline (MonoClass *klass) MONO_INTERNAL;
gpointer          mono_create_rgctx_lazy_fetch_trampoline (guint32 offset) MONO_INTERNAL;
MonoVTable*       mono_find_class_init_trampoline_by_addr (gconstpointer addr) MONO_INTERNAL;
MonoClass*        mono_find_delegate_trampoline_by_addr (gconstpointer addr) MONO_INTERNAL;
guint32           mono_find_rgctx_lazy_fetch_trampoline_by_addr (gconstpointer addr) MONO_INTERNAL;
gpointer          mono_magic_trampoline (gssize *regs, guint8 *code, MonoMethod *m, guint8* tramp) MONO_INTERNAL;
gpointer          mono_delegate_trampoline (gssize *regs, guint8 *code, gpointer *tramp_data, guint8* tramp) MONO_INTERNAL;
gpointer          mono_aot_trampoline (gssize *regs, guint8 *code, guint8 *token_info, 
									   guint8* tramp) MONO_INTERNAL;
gpointer          mono_aot_plt_trampoline (gssize *regs, guint8 *code, guint8 *token_info, 
										   guint8* tramp) MONO_INTERNAL;
void              mono_class_init_trampoline (gssize *regs, guint8 *code, MonoVTable *vtable, guint8 *tramp) MONO_INTERNAL;
void              mono_generic_class_init_trampoline (gssize *regs, guint8 *code, MonoVTable *vtable, guint8 *tramp) MONO_INTERNAL;
gconstpointer     mono_get_trampoline_func (MonoTrampolineType tramp_type);
gpointer          mini_get_vtable_trampoline (void) MONO_INTERNAL;

gboolean          mono_running_on_valgrind (void) MONO_INTERNAL;
void*             mono_global_codeman_reserve (int size) MONO_INTERNAL;
const char       *mono_regname_full (int reg, gboolean fp) MONO_INTERNAL;
gint32*           mono_allocate_stack_slots_full (MonoCompile *cfg, gboolean backward, guint32 *stack_size, guint32 *stack_align) MONO_INTERNAL;
gint32*           mono_allocate_stack_slots (MonoCompile *cfg, guint32 *stack_size, guint32 *stack_align) MONO_INTERNAL;
void              mono_local_regalloc (MonoCompile *cfg, MonoBasicBlock *bb) MONO_INTERNAL;
MonoInst         *mono_branch_optimize_exception_target (MonoCompile *cfg, MonoBasicBlock *bb, const char * exname) MONO_INTERNAL;
void              mono_remove_critical_edges (MonoCompile *cfg) MONO_INTERNAL;
gboolean          mono_is_regsize_var (MonoType *t) MONO_INTERNAL;
void              mini_emit_memcpy2 (MonoCompile *cfg, int destreg, int doffset, int srcreg, int soffset, int size, int align) MONO_INTERNAL;
CompRelation      mono_opcode_to_cond (int opcode) MONO_INTERNAL;
CompType          mono_opcode_to_type (int opcode, int cmp_opcode) MONO_INTERNAL;
CompRelation      mono_negate_cond (CompRelation cond) MONO_INTERNAL;
int               mono_op_imm_to_op (int opcode) MONO_INTERNAL;
void              mono_decompose_op_imm (MonoCompile *cfg, MonoBasicBlock *bb, MonoInst *ins) MONO_INTERNAL;
void              mono_peephole_ins (MonoBasicBlock *bb, MonoInst *ins) MONO_INTERNAL;

void              mono_decompose_opcode (MonoCompile *cfg, MonoInst *ins) MONO_INTERNAL;
void              mono_decompose_long_opts (MonoCompile *cfg) MONO_INTERNAL;
void              mono_decompose_vtype_opts (MonoCompile *cfg) MONO_INTERNAL;
void              mono_decompose_array_access_opts (MonoCompile *cfg) MONO_INTERNAL;
void              mono_handle_soft_float (MonoCompile *cfg) MONO_INTERNAL;
void              mono_handle_global_vregs (MonoCompile *cfg) MONO_INTERNAL;
void              mono_spill_global_vars (MonoCompile *cfg, gboolean *need_local_opts) MONO_INTERNAL;
void              mono_if_conversion (MonoCompile *cfg) MONO_INTERNAL;

/* methods that must be provided by the arch-specific port */
void      mono_arch_init                        (void) MONO_INTERNAL;
void      mono_arch_cleanup                     (void) MONO_INTERNAL;
void      mono_arch_cpu_init                    (void) MONO_INTERNAL;
guint32   mono_arch_cpu_optimizazions           (guint32 *exclude_mask) MONO_INTERNAL;
void      mono_arch_instrument_mem_needs        (MonoMethod *method, int *stack, int *code) MONO_INTERNAL;
void     *mono_arch_instrument_prolog           (MonoCompile *cfg, void *func, void *p, gboolean enable_arguments) MONO_INTERNAL;
void     *mono_arch_instrument_epilog           (MonoCompile *cfg, void *func, void *p, gboolean enable_arguments) MONO_INTERNAL;
MonoCallInst *mono_arch_call_opcode             (MonoCompile *cfg, MonoBasicBlock* bb, MonoCallInst *call, int is_virtual) MONO_INTERNAL;
MonoInst *mono_arch_get_inst_for_method         (MonoCompile *cfg, MonoMethod *cmethod, MonoMethodSignature *fsig, MonoInst **args) MONO_INTERNAL;
void      mono_codegen                          (MonoCompile *cfg) MONO_INTERNAL;
void      mono_call_inst_add_outarg_reg         (MonoCompile *cfg, MonoCallInst *call, int vreg, int hreg, gboolean fp) MONO_INTERNAL;
const char *mono_arch_regname                   (int reg) MONO_INTERNAL;
const char *mono_arch_fregname                  (int reg) MONO_INTERNAL;
gpointer  mono_arch_get_throw_exception         (void) MONO_INTERNAL;
gpointer  mono_arch_get_rethrow_exception       (void) MONO_INTERNAL;
gpointer  mono_arch_get_throw_exception_by_name (void) MONO_INTERNAL;
gpointer  mono_arch_get_throw_corlib_exception  (void) MONO_INTERNAL;
guchar*   mono_arch_create_trampoline_code      (MonoTrampolineType tramp_type) MONO_INTERNAL;
guchar*   mono_arch_create_trampoline_code_full (MonoTrampolineType tramp_type, guint32 *code_size, MonoJumpInfo **ji, gboolean aot) MONO_INTERNAL;
gpointer  mono_arch_create_rgctx_lazy_fetch_trampoline (guint32 slot) MONO_INTERNAL;
gpointer  mono_arch_create_generic_class_init_trampoline (void) MONO_INTERNAL;
gpointer  mono_arch_get_nullified_class_init_trampoline (guint32 *code_len) MONO_INTERNAL;
GList    *mono_arch_get_allocatable_int_vars    (MonoCompile *cfg) MONO_INTERNAL;
GList    *mono_arch_get_global_int_regs         (MonoCompile *cfg) MONO_INTERNAL;
GList    *mono_arch_get_global_fp_regs          (MonoCompile *cfg) MONO_INTERNAL;
GList    *mono_arch_get_iregs_clobbered_by_call (MonoCallInst *call) MONO_INTERNAL;
GList    *mono_arch_get_fregs_clobbered_by_call (MonoCallInst *call) MONO_INTERNAL;
guint32   mono_arch_regalloc_cost               (MonoCompile *cfg, MonoMethodVar *vmv) MONO_INTERNAL;
void      mono_arch_patch_code                  (MonoMethod *method, MonoDomain *domain, guint8 *code, MonoJumpInfo *ji, gboolean run_cctors) MONO_INTERNAL;
void      mono_arch_flush_icache                (guint8 *code, gint size) MONO_INTERNAL;
int       mono_arch_max_epilog_size             (MonoCompile *cfg) MONO_INTERNAL;
guint8   *mono_arch_emit_prolog                 (MonoCompile *cfg) MONO_INTERNAL;
void      mono_arch_emit_epilog                 (MonoCompile *cfg) MONO_INTERNAL;
void      mono_arch_emit_exceptions             (MonoCompile *cfg) MONO_INTERNAL;
void      mono_arch_lowering_pass               (MonoCompile *cfg, MonoBasicBlock *bb) MONO_INTERNAL;
void      mono_arch_peephole_pass_1             (MonoCompile *cfg, MonoBasicBlock *bb) MONO_INTERNAL;
void      mono_arch_peephole_pass_2             (MonoCompile *cfg, MonoBasicBlock *bb) MONO_INTERNAL;
void      mono_arch_output_basic_block          (MonoCompile *cfg, MonoBasicBlock *bb) MONO_INTERNAL;
gboolean  mono_arch_has_unwind_info             (gconstpointer addr) MONO_INTERNAL;
void      mono_arch_setup_jit_tls_data          (MonoJitTlsData *tls) MONO_INTERNAL;
void      mono_arch_free_jit_tls_data           (MonoJitTlsData *tls) MONO_INTERNAL;
void      mono_arch_emit_this_vret_args         (MonoCompile *cfg, MonoCallInst *inst, int this_reg, int this_type, int vt_reg) MONO_INTERNAL;
void      mono_arch_fill_argument_info          (MonoCompile *cfg) MONO_INTERNAL;
void      mono_arch_allocate_vars               (MonoCompile *m) MONO_INTERNAL;
int       mono_arch_get_argument_info           (MonoMethodSignature *csig, int param_count, MonoJitArgumentInfo *arg_info) MONO_INTERNAL;
gboolean  mono_arch_print_tree			(MonoInst *tree, int arity) MONO_INTERNAL;
void      mono_arch_emit_call                   (MonoCompile *cfg, MonoCallInst *call) MONO_INTERNAL;
void      mono_arch_emit_outarg_vt              (MonoCompile *cfg, MonoInst *ins, MonoInst *src) MONO_INTERNAL;
void      mono_arch_emit_setret                 (MonoCompile *cfg, MonoMethod *method, MonoInst *val) MONO_INTERNAL;
MonoInst *mono_arch_emit_inst_for_method        (MonoCompile *cfg, MonoMethod *cmethod, MonoMethodSignature *fsig, MonoInst **args) MONO_INTERNAL;
void      mono_arch_decompose_opts              (MonoCompile *cfg, MonoInst *ins) MONO_INTERNAL;
void      mono_arch_decompose_long_opts         (MonoCompile *cfg, MonoInst *ins) MONO_INTERNAL;

MonoJitInfo *mono_arch_find_jit_info            (MonoDomain *domain, 
						 MonoJitTlsData *jit_tls, 
						 MonoJitInfo *res, 
						 MonoJitInfo *prev_ji, 
						 MonoContext *ctx, 
						 MonoContext *new_ctx, 
						 MonoLMF **lmf, 
						 gboolean *managed) MONO_INTERNAL;
gpointer mono_arch_get_call_filter              (void) MONO_INTERNAL;
gpointer mono_arch_get_restore_context          (void) MONO_INTERNAL;
gpointer mono_arch_get_call_filter_full         (guint32 *code_size, MonoJumpInfo **ji, gboolean aot) MONO_INTERNAL;
gpointer mono_arch_get_restore_context_full     (guint32 *code_size, MonoJumpInfo **ji, gboolean aot) MONO_INTERNAL;
gpointer  mono_arch_get_throw_exception_full    (guint32 *code_size, MonoJumpInfo **ji, gboolean aot) MONO_INTERNAL;
gpointer  mono_arch_get_rethrow_exception_full  (guint32 *code_size, MonoJumpInfo **ji, gboolean aot) MONO_INTERNAL;
gpointer  mono_arch_get_throw_exception_by_name_full (guint32 *code_size, MonoJumpInfo **ji, gboolean aot) MONO_INTERNAL;
gpointer  mono_arch_get_throw_corlib_exception_full (guint32 *code_size, MonoJumpInfo **ji, gboolean aot) MONO_INTERNAL;
gboolean mono_arch_handle_exception             (void *sigctx, gpointer obj, gboolean test_only) MONO_INTERNAL;
void     mono_arch_handle_altstack_exception    (void *sigctx, gpointer fault_addr, gboolean stack_ovf) MONO_INTERNAL;
gpointer mono_arch_ip_from_context              (void *sigctx) MONO_INTERNAL;
void     mono_arch_sigctx_to_monoctx            (void *sigctx, MonoContext *ctx) MONO_INTERNAL;
void     mono_arch_monoctx_to_sigctx            (MonoContext *mctx, void *ctx) MONO_INTERNAL;
gpointer mono_arch_context_get_int_reg		(MonoContext *ctx, int reg) MONO_INTERNAL;
void     mono_arch_flush_register_windows       (void) MONO_INTERNAL;
gboolean mono_arch_is_inst_imm                  (gint64 imm) MONO_INTERNAL;
MonoInst* mono_arch_get_domain_intrinsic        (MonoCompile* cfg) MONO_INTERNAL;
MonoInst* mono_arch_get_thread_intrinsic        (MonoCompile* cfg) MONO_INTERNAL;
gboolean mono_arch_is_int_overflow              (void *sigctx, void *info) MONO_INTERNAL;
void     mono_arch_invalidate_method            (MonoJitInfo *ji, void *func, gpointer func_arg) MONO_INTERNAL;
guint32  mono_arch_get_patch_offset             (guint8 *code) MONO_INTERNAL;
gpointer*mono_arch_get_vcall_slot_addr          (guint8* code, gpointer *regs) MONO_INTERNAL;
gpointer mono_arch_get_vcall_slot               (guint8 *code, gpointer *regs, int *displacement) MONO_INTERNAL;
gpointer*mono_arch_get_delegate_method_ptr_addr (guint8* code, gpointer *regs) MONO_INTERNAL;
void     mono_arch_create_vars                  (MonoCompile *cfg) MONO_INTERNAL;
void     mono_arch_save_unwind_info             (MonoCompile *cfg) MONO_INTERNAL;
void     mono_arch_register_lowlevel_calls      (void) MONO_INTERNAL;
gpointer mono_arch_get_unbox_trampoline         (MonoGenericSharingContext *gsctx, MonoMethod *m, gpointer addr) MONO_INTERNAL;
void     mono_arch_patch_callsite               (guint8 *method_start, guint8 *code, guint8 *addr) MONO_INTERNAL;
void     mono_arch_patch_plt_entry              (guint8 *code, guint8 *addr) MONO_INTERNAL;
void     mono_arch_nullify_class_init_trampoline(guint8 *code, gssize *regs) MONO_INTERNAL;
void     mono_arch_nullify_plt_entry            (guint8 *code) MONO_INTERNAL;
int      mono_arch_get_this_arg_reg             (MonoMethodSignature *sig, MonoGenericSharingContext *gsctx, guint8 *code) MONO_INTERNAL;
gpointer mono_arch_get_this_arg_from_call       (MonoGenericSharingContext *gsctx, MonoMethodSignature *sig, gssize *regs, guint8 *code) MONO_INTERNAL;
MonoObject* mono_arch_find_this_argument        (gpointer *regs, MonoMethod *method, MonoGenericSharingContext *gsctx) MONO_INTERNAL;
gpointer mono_arch_get_delegate_invoke_impl     (MonoMethodSignature *sig, gboolean has_target) MONO_INTERNAL;
gpointer mono_arch_create_specific_trampoline   (gpointer arg1, MonoTrampolineType tramp_type, MonoDomain *domain, guint32 *code_len) MONO_INTERNAL;
void        mono_arch_emit_imt_argument         (MonoCompile *cfg, MonoCallInst *call) MONO_INTERNAL;
MonoMethod* mono_arch_find_imt_method           (gpointer *regs, guint8 *code) MONO_INTERNAL;
MonoVTable* mono_arch_find_static_call_vtable   (gpointer *regs, guint8 *code) MONO_INTERNAL;
gpointer    mono_arch_build_imt_thunk           (MonoVTable *vtable, MonoDomain *domain, MonoIMTCheckItem **imt_entries, int count) MONO_INTERNAL;
void    mono_arch_notify_pending_exc            (void) MONO_INTERNAL;
void    mono_arch_fixup_jinfo                   (MonoCompile *cfg) MONO_INTERNAL;

/* Exception handling */
void     mono_exceptions_init                   (void) MONO_INTERNAL;
gboolean mono_handle_exception                  (MonoContext *ctx, gpointer obj,
						 gpointer original_ip, gboolean test_only) MONO_INTERNAL;
void     mono_handle_native_sigsegv             (int signal, void *sigctx) MONO_INTERNAL;
void     mono_print_thread_dump                 (void *sigctx);
void     mono_jit_walk_stack                    (MonoStackWalk func, gboolean do_il_offset, gpointer user_data) MONO_INTERNAL;
void     mono_jit_walk_stack_from_ctx           (MonoStackWalk func, MonoContext *ctx, gboolean do_il_offset, gpointer user_data) MONO_INTERNAL;
void     mono_setup_altstack                    (MonoJitTlsData *tls) MONO_INTERNAL;
void     mono_free_altstack                     (MonoJitTlsData *tls) MONO_INTERNAL;
MonoJitInfo * mono_find_jit_info                (MonoDomain *domain, MonoJitTlsData *jit_tls, MonoJitInfo *res, MonoJitInfo *prev_ji, MonoContext *ctx, MonoContext *new_ctx, char **trace, MonoLMF **lmf, int *native_offset, gboolean *managed) MONO_INTERNAL;

gpointer mono_get_throw_exception               (void) MONO_INTERNAL;
gpointer mono_get_rethrow_exception             (void) MONO_INTERNAL;
gpointer mono_get_call_filter                   (void) MONO_INTERNAL;
gpointer mono_get_restore_context               (void) MONO_INTERNAL;
gpointer mono_get_throw_exception_by_name       (void) MONO_INTERNAL;
gpointer mono_get_throw_corlib_exception        (void) MONO_INTERNAL;

/* the new function to do stack walks */
typedef gboolean (*MonoStackFrameWalk)          (MonoDomain *domain, MonoContext *ctx, MonoJitInfo *ji, gpointer data);
void      mono_walk_stack                       (MonoDomain *domain, MonoJitTlsData *jit_tls, MonoContext *start_ctx, MonoStackFrameWalk func, gpointer user_data);

MonoArray *ves_icall_get_trace                  (MonoException *exc, gint32 skip, MonoBoolean need_file_info) MONO_INTERNAL;
MonoBoolean ves_icall_get_frame_info            (gint32 skip, MonoBoolean need_file_info, 
						 MonoReflectionMethod **method, 
						 gint32 *iloffset, gint32 *native_offset,
						 MonoString **file, gint32 *line, gint32 *column) MONO_INTERNAL;
MonoString *ves_icall_System_Exception_get_trace (MonoException *exc) MONO_INTERNAL;

/* Dominator/SSA methods */
void        mono_compile_dominator_info         (MonoCompile *cfg, int dom_flags) MONO_INTERNAL;
void        mono_compute_natural_loops          (MonoCompile *cfg) MONO_INTERNAL;
MonoBitSet* mono_compile_iterated_dfrontier     (MonoCompile *cfg, MonoBitSet *set) MONO_INTERNAL;
void        mono_ssa_compute                    (MonoCompile *cfg) MONO_INTERNAL;
void        mono_ssa_remove                     (MonoCompile *cfg) MONO_INTERNAL;
void        mono_ssa_cprop                      (MonoCompile *cfg) MONO_INTERNAL;
void        mono_ssa_deadce                     (MonoCompile *cfg) MONO_INTERNAL;
void        mono_ssa_strength_reduction         (MonoCompile *cfg) MONO_INTERNAL;
void        mono_free_loop_info                 (MonoCompile *cfg) MONO_INTERNAL;

void        mono_ssa_compute2                   (MonoCompile *cfg);
void        mono_ssa_remove2                    (MonoCompile *cfg);
void        mono_ssa_cprop2                     (MonoCompile *cfg);
void        mono_ssa_deadce2                    (MonoCompile *cfg);

/* debugging support */
void      mono_debug_init_method                (MonoCompile *cfg, MonoBasicBlock *start_block,
						 guint32 breakpoint_id) MONO_INTERNAL;
void      mono_debug_open_method                (MonoCompile *cfg) MONO_INTERNAL;
void      mono_debug_close_method               (MonoCompile *cfg) MONO_INTERNAL;
void      mono_debug_open_block                 (MonoCompile *cfg, MonoBasicBlock *bb, guint32 address) MONO_INTERNAL;
void      mono_debug_record_line_number         (MonoCompile *cfg, MonoInst *ins, guint32 address) MONO_INTERNAL;
void      mono_debug_serialize_debug_info       (MonoCompile *cfg, guint8 **out_buf, guint32 *buf_len) MONO_INTERNAL;
void      mono_debug_add_aot_method             (MonoDomain *domain,
						 MonoMethod *method, guint8 *code_start, 
						 guint8 *debug_info, guint32 debug_info_len) MONO_INTERNAL;
void      mono_debug_add_icall_wrapper          (MonoMethod *method, MonoJitICallInfo* info) MONO_INTERNAL;
void      mono_debug_print_vars                 (gpointer ip, gboolean only_arguments);
void      mono_debugger_run_finally             (MonoContext *start_ctx);

extern gssize mono_breakpoint_info_index [MONO_BREAKPOINT_ARRAY_SIZE];

gboolean mono_breakpoint_clean_code (guint8 *method_start, guint8 *code, int offset, guint8 *buf, int size);

/* Mono Debugger support */
void      mono_debugger_init                    (void);
int       mono_debugger_main                    (MonoDomain *domain, MonoAssembly *assembly, int argc, char **argv);


/* Tracing */
MonoTraceSpec *mono_trace_parse_options         (const char *options) MONO_INTERNAL;
void           mono_trace_set_assembly          (MonoAssembly *assembly) MONO_INTERNAL;
gboolean       mono_trace_eval                  (MonoMethod *method) MONO_INTERNAL;

extern void
mono_perform_abc_removal (MonoCompile *cfg) MONO_INTERNAL;
extern void
mono_perform_abc_removal2 (MonoCompile *cfg) MONO_INTERNAL;
extern void
mono_perform_ssapre (MonoCompile *cfg) MONO_INTERNAL;
extern void
mono_local_cprop (MonoCompile *cfg) MONO_INTERNAL;
extern void
mono_local_cprop2 (MonoCompile *cfg);
extern void
mono_local_deadce (MonoCompile *cfg);

/* CAS - stack walk */
MonoSecurityFrame* ves_icall_System_Security_SecurityFrame_GetSecurityFrame (gint32 skip) MONO_INTERNAL;
MonoArray* ves_icall_System_Security_SecurityFrame_GetSecurityStack (gint32 skip) MONO_INTERNAL;

/* Generic sharing */

MonoGenericSharingContext* mono_get_generic_context_from_code (guint8 *code) MONO_INTERNAL;

MonoGenericContext* mini_method_get_context (MonoMethod *method) MONO_INTERNAL;

int mono_method_check_context_used (MonoMethod *method) MONO_INTERNAL;

gboolean mono_generic_context_equal_deep (MonoGenericContext *context1, MonoGenericContext *context2) MONO_INTERNAL;

gboolean mono_generic_context_is_sharable (MonoGenericContext *context, gboolean allow_type_vars) MONO_INTERNAL;

gboolean mono_method_is_generic_impl (MonoMethod *method) MONO_INTERNAL;
gboolean mono_method_is_generic_sharable_impl (MonoMethod *method, gboolean allow_type_vars) MONO_INTERNAL;

gpointer mono_helper_get_rgctx_other_ptr (MonoClass *caller_class, MonoVTable *vtable,
					  guint32 token, guint32 token_source, guint32 rgctx_type,
					  gint32 rgctx_index) MONO_INTERNAL;

void mono_generic_sharing_init (void) MONO_INTERNAL;

MonoClass* mini_class_get_container_class (MonoClass *class) MONO_INTERNAL;
MonoGenericContext* mini_class_get_context (MonoClass *class) MONO_INTERNAL;

MonoType* mini_get_basic_type_from_generic (MonoGenericSharingContext *gsctx, MonoType *type) MONO_INTERNAL;

int mini_type_stack_size (MonoGenericSharingContext *gsctx, MonoType *t, int *align) MONO_INTERNAL;

/* wapihandles.c */
int mini_wapi_hps (int argc, char **argv) MONO_INTERNAL;

int mini_wapi_semdel (int argc, char **argv) MONO_INTERNAL;

int mini_wapi_seminfo (int argc, char **argv) MONO_INTERNAL;

#endif /* __MONO_MINI_H__ */
