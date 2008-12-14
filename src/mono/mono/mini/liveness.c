/*
 * liveness.c: liveness analysis
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * (C) 2002 Ximian, Inc.
 */

#include "mini.h"
#include "inssel.h"
#include "aliasing.h"

#define SPILL_COST_INCREMENT (1 << (bb->nesting << 1))

//#define DEBUG_LIVENESS

#if SIZEOF_VOID_P == 8
#define BITS_PER_CHUNK 64
#else
#define BITS_PER_CHUNK 32
#endif

/* 
 * The liveness2 pass can't handle long vars on 32 bit platforms because the component
 * vars have the same 'idx'.
 */
#if SIZEOF_VOID_P == 8
//#define ENABLE_LIVENESS2
#endif

#ifdef ENABLE_LIVENESS2
static void mono_analyze_liveness2 (MonoCompile *cfg);
#endif

static void
optimize_initlocals (MonoCompile *cfg);

static void
optimize_initlocals2 (MonoCompile *cfg);

/* mono_bitset_mp_new:
 * 
 * allocates a MonoBitSet inside a memory pool
 */
static inline MonoBitSet* 
mono_bitset_mp_new (MonoMemPool *mp, guint32 size, guint32 max_size)
{
	guint8 *mem = mono_mempool_alloc0 (mp, size);
	return mono_bitset_mem_new (mem, max_size, MONO_BITSET_DONT_FREE);
}

static inline MonoBitSet* 
mono_bitset_mp_new_noinit (MonoMemPool *mp, guint32 size, guint32 max_size)
{
	guint8 *mem = mono_mempool_alloc (mp, size);
	return mono_bitset_mem_new (mem, max_size, MONO_BITSET_DONT_FREE);
}

G_GNUC_UNUSED static void
mono_bitset_print (MonoBitSet *set)
{
	int i;

	printf ("{");
	for (i = 0; i < mono_bitset_size (set); i++) {

		if (mono_bitset_test (set, i))
			printf ("%d, ", i);

	}
	printf ("}\n");
}

static inline void
update_live_range (MonoCompile *cfg, int idx, int block_dfn, int tree_pos)
{
	MonoLiveRange *range = &MONO_VARINFO (cfg, idx)->range;
	guint32 abs_pos = (block_dfn << 16) | tree_pos;

	if (range->first_use.abs_pos > abs_pos)
		range->first_use.abs_pos = abs_pos;

	if (range->last_use.abs_pos < abs_pos)
		range->last_use.abs_pos = abs_pos;
}

static void
update_gen_kill_set (MonoCompile *cfg, MonoBasicBlock *bb, MonoInst *inst, int inst_num)
{
	int arity;
	int max_vars = cfg->num_varinfo;

	arity = mono_burg_arity [inst->opcode];
	if (arity)
		update_gen_kill_set (cfg, bb, inst->inst_i0, inst_num);

	if (arity > 1)
		update_gen_kill_set (cfg, bb, inst->inst_i1, inst_num);

	if ((inst->ssa_op & MONO_SSA_LOAD_STORE) || (inst->opcode == OP_DUMMY_STORE)) {
		MonoLocalVariableList* affected_variables;
		MonoLocalVariableList local_affected_variable;
		
		if (cfg->aliasing_info == NULL) {
			if ((inst->ssa_op == MONO_SSA_LOAD) || (inst->ssa_op == MONO_SSA_STORE) || (inst->opcode == OP_DUMMY_STORE)) {
				local_affected_variable.variable_index = inst->inst_i0->inst_c0;
				local_affected_variable.next = NULL;
				affected_variables = &local_affected_variable;
			} else {
				affected_variables = NULL;
			}
		} else {
			affected_variables = mono_aliasing_get_affected_variables_for_inst_traversing_code (cfg->aliasing_info, inst);
		}
		
		if (inst->ssa_op & MONO_SSA_LOAD) {
			MonoLocalVariableList* affected_variable = affected_variables;
			while (affected_variable != NULL) {
				int idx = affected_variable->variable_index;
				MonoMethodVar *vi = MONO_VARINFO (cfg, idx);
				g_assert (idx < max_vars);
				update_live_range (cfg, idx, bb->dfn, inst_num); 
				if (!mono_bitset_test_fast (bb->kill_set, idx))
					mono_bitset_set_fast (bb->gen_set, idx);
				if (inst->ssa_op == MONO_SSA_LOAD)
					vi->spill_costs += SPILL_COST_INCREMENT;
				
				affected_variable = affected_variable->next;
			}
		} else if ((inst->ssa_op == MONO_SSA_STORE) || (inst->opcode == OP_DUMMY_STORE)) {
			MonoLocalVariableList* affected_variable = affected_variables;
			while (affected_variable != NULL) {
				int idx = affected_variable->variable_index;
				MonoMethodVar *vi = MONO_VARINFO (cfg, idx);
				g_assert (idx < max_vars);
				//if (arity > 0)
					//g_assert (inst->inst_i1->opcode != OP_PHI);
				update_live_range (cfg, idx, bb->dfn, inst_num); 
				mono_bitset_set_fast (bb->kill_set, idx);
				if (inst->ssa_op == MONO_SSA_STORE)
					vi->spill_costs += SPILL_COST_INCREMENT;
				
				affected_variable = affected_variable->next;
			}
		}
	} else if (inst->opcode == OP_JMP) {
		/* Keep arguments live! */
		int i;
		for (i = 0; i < cfg->num_varinfo; i++) {
			if (cfg->varinfo [i]->opcode == OP_ARG) {
				if (!mono_bitset_test_fast (bb->kill_set, i))
					mono_bitset_set_fast (bb->gen_set, i);
			}
		}
	}
} 

static void
update_volatile (MonoCompile *cfg, MonoBasicBlock *bb, MonoInst *inst)
{
	int arity = mono_burg_arity [inst->opcode];
	int max_vars = cfg->num_varinfo;

	if (arity)
		update_volatile (cfg, bb, inst->inst_i0);

	if (arity > 1)
		update_volatile (cfg, bb, inst->inst_i1);

	if (inst->ssa_op & MONO_SSA_LOAD_STORE) {
		MonoLocalVariableList* affected_variables;
		MonoLocalVariableList local_affected_variable;
		
		if (cfg->aliasing_info == NULL) {
			if ((inst->ssa_op == MONO_SSA_LOAD) || (inst->ssa_op == MONO_SSA_STORE)) {
				local_affected_variable.variable_index = inst->inst_i0->inst_c0;
				local_affected_variable.next = NULL;
				affected_variables = &local_affected_variable;
			} else {
				affected_variables = NULL;
			}
		} else {
			affected_variables = mono_aliasing_get_affected_variables_for_inst_traversing_code (cfg->aliasing_info, inst);
		}
		
		while (affected_variables != NULL) {
			int idx = affected_variables->variable_index;
			MonoMethodVar *vi = MONO_VARINFO (cfg, idx);
			g_assert (idx < max_vars);
			cfg->varinfo [vi->idx]->flags |= MONO_INST_VOLATILE;
			
			affected_variables = affected_variables->next;
		}
	}
} 

static void
visit_bb (MonoCompile *cfg, MonoBasicBlock *bb, GSList **visited)
{
	int i;
	MonoInst *ins;

	if (g_slist_find (*visited, bb))
		return;

	if (cfg->new_ir) {
		for (ins = bb->code; ins; ins = ins->next) {
			const char *spec = INS_INFO (ins->opcode);
			int regtype, srcindex, sreg;

			if (ins->opcode == OP_NOP)
				continue;

			/* DREG */
			regtype = spec [MONO_INST_DEST];
			g_assert (((ins->dreg == -1) && (regtype == ' ')) || ((ins->dreg != -1) && (regtype != ' ')));
				
			if ((ins->dreg != -1) && get_vreg_to_inst (cfg, ins->dreg)) {
				MonoInst *var = get_vreg_to_inst (cfg, ins->dreg);
				int idx = var->inst_c0;
				MonoMethodVar *vi = MONO_VARINFO (cfg, idx);

				cfg->varinfo [vi->idx]->flags |= MONO_INST_VOLATILE;
			}
			
			/* SREGS */
			for (srcindex = 0; srcindex < 2; ++srcindex) {
				regtype = spec [(srcindex == 0) ? MONO_INST_SRC1 : MONO_INST_SRC2];
				sreg = srcindex == 0 ? ins->sreg1 : ins->sreg2;

				g_assert (((sreg == -1) && (regtype == ' ')) || ((sreg != -1) && (regtype != ' ')));
				if ((sreg != -1) && get_vreg_to_inst (cfg, sreg)) {
					MonoInst *var = get_vreg_to_inst (cfg, sreg);
					int idx = var->inst_c0;
					MonoMethodVar *vi = MONO_VARINFO (cfg, idx);

					cfg->varinfo [vi->idx]->flags |= MONO_INST_VOLATILE;
				}
			}
		}
	} else {
		if (cfg->aliasing_info != NULL)
			mono_aliasing_initialize_code_traversal (cfg->aliasing_info, bb);

		for (ins = bb->code; ins; ins = ins->next) {
			update_volatile (cfg, bb, ins);
		}
	}

	*visited = g_slist_append (*visited, bb);

	/* 
	 * Need to visit all bblocks reachable from this one since they can be
	 * reached during exception handling.
	 */
	for (i = 0; i < bb->out_count; ++i) {
		visit_bb (cfg, bb->out_bb [i], visited);
	}
}

void
mono_liveness_handle_exception_clauses (MonoCompile *cfg)
{
	MonoBasicBlock *bb;
	GSList *visited = NULL;

	/*
	 * Variables in exception handler register cannot be allocated to registers
	 * so make them volatile. See bug #42136. This will not be neccessary when
	 * the back ends could guarantee that the variables will be in the
	 * correct registers when a handler is called.
	 * This includes try blocks too, since a variable in a try block might be
	 * accessed after an exception handler has been run.
	 */
	for (bb = cfg->bb_entry; bb; bb = bb->next_bb) {

		if (bb->region == -1 || MONO_BBLOCK_IS_IN_REGION (bb, MONO_REGION_TRY))
			continue;

		visit_bb (cfg, bb, &visited);
	}
	g_slist_free (visited);
}

static inline void
update_live_range2 (MonoMethodVar *var, int abs_pos)
{
	if (var->range.first_use.abs_pos > abs_pos)
		var->range.first_use.abs_pos = abs_pos;

	if (var->range.last_use.abs_pos < abs_pos)
		var->range.last_use.abs_pos = abs_pos;
}

static void
analyze_liveness_bb (MonoCompile *cfg, MonoBasicBlock *bb)
{
	MonoInst *ins;
	int sreg, inst_num;
	MonoMethodVar *vars = cfg->vars;
	guint32 abs_pos = (bb->dfn << 16);
	
	for (inst_num = 0, ins = bb->code; ins; ins = ins->next, inst_num += 2) {
		const char *spec = INS_INFO (ins->opcode);

#ifdef DEBUG_LIVENESS
			printf ("\t"); mono_print_ins (ins);
#endif

		if (ins->opcode == OP_NOP)
			continue;

		if (ins->opcode == OP_LDADDR) {
			MonoInst *var = ins->inst_p0;
			int idx = var->inst_c0;
			MonoMethodVar *vi = MONO_VARINFO (cfg, idx);

#ifdef DEBUG_LIVENESS
			printf ("\tGEN: R%d(%d)\n", var->dreg, idx);
#endif
			update_live_range2 (&vars [idx], abs_pos + inst_num); 
			if (!mono_bitset_test_fast (bb->kill_set, idx))
				mono_bitset_set_fast (bb->gen_set, idx);
			vi->spill_costs += SPILL_COST_INCREMENT;
		}				

		/* SREGs must come first, so MOVE r <- r is handled correctly */

		/* SREG1 */
		sreg = ins->sreg1;
		if ((spec [MONO_INST_SRC1] != ' ') && get_vreg_to_inst (cfg, sreg)) {
			MonoInst *var = get_vreg_to_inst (cfg, sreg);
			int idx = var->inst_c0;
			MonoMethodVar *vi = MONO_VARINFO (cfg, idx);

#ifdef DEBUG_LIVENESS
			printf ("\tGEN: R%d(%d)\n", sreg, idx);
#endif
			update_live_range2 (&vars [idx], abs_pos + inst_num); 
			if (!mono_bitset_test_fast (bb->kill_set, idx))
				mono_bitset_set_fast (bb->gen_set, idx);
			vi->spill_costs += SPILL_COST_INCREMENT;
		}

		/* SREG2 */
		sreg = ins->sreg2;
		if ((spec [MONO_INST_SRC2] != ' ') && get_vreg_to_inst (cfg, sreg)) {
			MonoInst *var = get_vreg_to_inst (cfg, sreg);
			int idx = var->inst_c0;
			MonoMethodVar *vi = MONO_VARINFO (cfg, idx);

#ifdef DEBUG_LIVENESS
			printf ("\tGEN: R%d(%d)\n", sreg, idx);
#endif
			update_live_range2 (&vars [idx], abs_pos + inst_num); 
			if (!mono_bitset_test_fast (bb->kill_set, idx))
				mono_bitset_set_fast (bb->gen_set, idx);
			vi->spill_costs += SPILL_COST_INCREMENT;
		}

		/* DREG */
		if ((spec [MONO_INST_DEST] != ' ') && get_vreg_to_inst (cfg, ins->dreg)) {
			MonoInst *var = get_vreg_to_inst (cfg, ins->dreg);
			int idx = var->inst_c0;
			MonoMethodVar *vi = MONO_VARINFO (cfg, idx);

			if (MONO_IS_STORE_MEMBASE (ins)) {
				update_live_range2 (&vars [idx], abs_pos + inst_num); 
				if (!mono_bitset_test_fast (bb->kill_set, idx))
					mono_bitset_set_fast (bb->gen_set, idx);
				vi->spill_costs += SPILL_COST_INCREMENT;
			} else {
#ifdef DEBUG_LIVENESS
				printf ("\tKILL: R%d(%d)\n", ins->dreg, idx);
#endif
				update_live_range2 (&vars [idx], abs_pos + inst_num + 1); 
				mono_bitset_set_fast (bb->kill_set, idx);
				vi->spill_costs += SPILL_COST_INCREMENT;
			}
		}
	}
}

/* generic liveness analysis code. CFG specific parts are 
 * in update_gen_kill_set()
 */
void
mono_analyze_liveness (MonoCompile *cfg)
{
	MonoBitSet *old_live_out_set;
	int i, j, max_vars = cfg->num_varinfo;
	int out_iter;
	gboolean *in_worklist;
	MonoBasicBlock **worklist;
	guint32 l_end;
	int bitsize;

#ifdef DEBUG_LIVENESS
	printf ("LIVENESS %s\n", mono_method_full_name (cfg->method, TRUE));
#endif

	g_assert (!(cfg->comp_done & MONO_COMP_LIVENESS));

	cfg->comp_done |= MONO_COMP_LIVENESS;
	
	if (max_vars == 0)
		return;

	bitsize = mono_bitset_alloc_size (max_vars, 0);

	for (i = 0; i < max_vars; i ++) {
		MONO_VARINFO (cfg, i)->range.first_use.abs_pos = ~ 0;
		MONO_VARINFO (cfg, i)->range.last_use .abs_pos =   0;
		MONO_VARINFO (cfg, i)->spill_costs = 0;
	}

	for (i = 0; i < cfg->num_bblocks; ++i) {
		MonoBasicBlock *bb = cfg->bblocks [i];
		MonoInst *inst;
		int tree_num;

		bb->gen_set = mono_bitset_mp_new (cfg->mempool, bitsize, max_vars);
		bb->kill_set = mono_bitset_mp_new (cfg->mempool, bitsize, max_vars);

		if (cfg->new_ir) {
			analyze_liveness_bb (cfg, bb);
		} else {
			if (cfg->aliasing_info != NULL)
				mono_aliasing_initialize_code_traversal (cfg->aliasing_info, bb);

			tree_num = 0;
			MONO_BB_FOR_EACH_INS (bb, inst) {
#ifdef DEBUG_LIVENESS
				mono_print_tree (inst); printf ("\n");
#endif
				update_gen_kill_set (cfg, bb, inst, tree_num);
				tree_num ++;
			}
		}

#ifdef DEBUG_LIVENESS
		printf ("BLOCK BB%d (", bb->block_num);
		for (j = 0; j < bb->out_count; j++) 
			printf ("BB%d, ", bb->out_bb [j]->block_num);
		
		printf (")\n");
		printf ("GEN  BB%d: ", bb->block_num); mono_bitset_print (bb->gen_set);
		printf ("KILL BB%d: ", bb->block_num); mono_bitset_print (bb->kill_set);
#endif
	}

	old_live_out_set = mono_bitset_new (max_vars, 0);
	in_worklist = g_new0 (gboolean, cfg->num_bblocks + 1);

	worklist = g_new (MonoBasicBlock *, cfg->num_bblocks + 1);
	l_end = 0;

	/*
	 * This is a backward dataflow analysis problem, so we process blocks in
	 * decreasing dfn order, this speeds up the iteration.
	 */
	for (i = 0; i < cfg->num_bblocks; i ++) {
		MonoBasicBlock *bb = cfg->bblocks [i];

		worklist [l_end ++] = bb;
		in_worklist [bb->dfn] = TRUE;

		/* Initialized later */
		bb->live_in_set = NULL;
		bb->live_out_set = mono_bitset_mp_new (cfg->mempool, bitsize, max_vars);
	}

	out_iter = 0;

	while (l_end != 0) {
		MonoBasicBlock *bb = worklist [--l_end];
		MonoBasicBlock *out_bb;
		gboolean changed;

		in_worklist [bb->dfn] = FALSE;

#ifdef DEBUG_LIVENESS
		printf ("P: %d(%d): IN: ", bb->block_num, bb->dfn);
		for (j = 0; j < bb->in_count; ++j) 
			printf ("BB%d ", bb->in_bb [j]->block_num);
		printf ("OUT:");
		for (j = 0; j < bb->out_count; ++j) 
			printf ("BB%d ", bb->out_bb [j]->block_num);
		printf ("\n");
#endif


		if (bb->out_count == 0)
			continue;

		out_iter ++;

		if (!bb->live_in_set) {
			/* First pass over this bblock */
			changed = TRUE;
		}
		else {
			changed = FALSE;
			mono_bitset_copyto_fast (bb->live_out_set, old_live_out_set);
		}
 
		for (j = 0; j < bb->out_count; j++) {
			out_bb = bb->out_bb [j];

			if (!out_bb->live_in_set) {
				out_bb->live_in_set = mono_bitset_mp_new_noinit (cfg->mempool, bitsize, max_vars);

				mono_bitset_copyto_fast (out_bb->live_out_set, out_bb->live_in_set);
				mono_bitset_sub_fast (out_bb->live_in_set, out_bb->kill_set);
				mono_bitset_union_fast (out_bb->live_in_set, out_bb->gen_set);
			}

			mono_bitset_union_fast (bb->live_out_set, out_bb->live_in_set);
		}
				
		if (changed || !mono_bitset_equal (old_live_out_set, bb->live_out_set)) {
			if (!bb->live_in_set)
				bb->live_in_set = mono_bitset_mp_new_noinit (cfg->mempool, bitsize, max_vars);
			mono_bitset_copyto_fast (bb->live_out_set, bb->live_in_set);
			mono_bitset_sub_fast (bb->live_in_set, bb->kill_set);
			mono_bitset_union_fast (bb->live_in_set, bb->gen_set);

			for (j = 0; j < bb->in_count; j++) {
				MonoBasicBlock *in_bb = bb->in_bb [j];
				/* 
				 * Some basic blocks do not seem to be in the 
				 * cfg->bblocks array...
				 */
				if (in_bb->gen_set && !in_worklist [in_bb->dfn]) {
#ifdef DEBUG_LIVENESS
					printf ("\tADD: %d\n", in_bb->block_num);
#endif
					/*
					 * Put the block at the top of the stack, so it
					 * will be processed right away.
					 */
					worklist [l_end ++] = in_bb;
					in_worklist [in_bb->dfn] = TRUE;
				}
			}
		}
	}

#ifdef DEBUG_LIVENESS
		printf ("IT: %d %d.\n", cfg->num_bblocks, out_iter);
#endif

	mono_bitset_free (old_live_out_set);

	g_free (worklist);
	g_free (in_worklist);

	/* Compute live_in_set for bblocks skipped earlier */
	for (i = 0; i < cfg->num_bblocks; ++i) {
		MonoBasicBlock *bb = cfg->bblocks [i];

		if (!bb->live_in_set) {
			bb->live_in_set = mono_bitset_mp_new (cfg->mempool, bitsize, max_vars);

			mono_bitset_copyto_fast (bb->live_out_set, bb->live_in_set);
			mono_bitset_sub_fast (bb->live_in_set, bb->kill_set);
			mono_bitset_union_fast (bb->live_in_set, bb->gen_set);
		}
	}

	for (i = 0; i < cfg->num_bblocks; ++i) {
		MonoBasicBlock *bb = cfg->bblocks [i];
		guint32 rem, max;
		guint32 abs_pos = (bb->dfn << 16);
		MonoMethodVar *vars = cfg->vars;

		if (!bb->live_out_set)
			continue;

		rem = max_vars % BITS_PER_CHUNK;
		max = ((max_vars + (BITS_PER_CHUNK -1)) / BITS_PER_CHUNK);
		for (j = 0; j < max; ++j) {
			gsize bits_in;
			gsize bits_out;
			int k;

			bits_in = mono_bitset_get_fast (bb->live_in_set, j);
			bits_out = mono_bitset_get_fast (bb->live_out_set, j);

			k = (j * BITS_PER_CHUNK);
			while ((bits_in || bits_out)) {
				if (bits_in & 1)
					update_live_range2 (&vars [k], abs_pos + 0);
				if (bits_out & 1)
					update_live_range2 (&vars [k], abs_pos + 0xffff);
				bits_in >>= 1;
				bits_out >>= 1;
				k ++;
			}
		}
	}

	/*
	 * Arguments need to have their live ranges extended to the beginning of
	 * the method to account for the arg reg/memory -> global register copies
	 * in the prolog (bug #74992).
	 */

	for (i = 0; i < max_vars; i ++) {
		MonoMethodVar *vi = MONO_VARINFO (cfg, i);
		if (cfg->varinfo [vi->idx]->opcode == OP_ARG) {
			if (vi->range.last_use.abs_pos == 0 && !(cfg->varinfo [vi->idx]->flags & (MONO_INST_VOLATILE|MONO_INST_INDIRECT))) {
				/* 
				 * Can't eliminate the this variable in gshared code, since
				 * it is needed during exception handling to identify the
				 * method.
				 * It is better to check for this here instead of marking the variable
				 * VOLATILE, since that would prevent it from being allocated to
				 * registers.
				 */
				 if (!(cfg->generic_sharing_context && mono_method_signature (cfg->method)->hasthis && cfg->varinfo [vi->idx] == cfg->args [0]))
					 cfg->varinfo [vi->idx]->flags |= MONO_INST_IS_DEAD;
			}
			vi->range.first_use.abs_pos = 0;
		}
	}

#ifdef DEBUG_LIVENESS
	for (i = cfg->num_bblocks - 1; i >= 0; i--) {
		MonoBasicBlock *bb = cfg->bblocks [i];
		
		printf ("LIVE IN  BB%d: ", bb->block_num); 
		mono_bitset_print (bb->live_in_set); 
		printf ("LIVE OUT BB%d: ", bb->block_num); 
		mono_bitset_print (bb->live_out_set); 
	}
#endif

	if (cfg->new_ir) {
		if (!cfg->disable_initlocals_opt)
			optimize_initlocals2 (cfg);

#ifdef ENABLE_LIVENESS2
		/* This improves code size by about 5% but slows down compilation too much */
		/* FIXME: This crashes when compiling 2.0 mscorlib */
		if (FALSE && cfg->compile_aot)
			mono_analyze_liveness2 (cfg);
#endif
	}
	else {
		if (!cfg->disable_initlocals_opt)
			optimize_initlocals (cfg);
	}
}

/**
 * optimize_initlocals:
 *
 * Try to optimize away some of the redundant initialization code inserted because of
 * 'locals init' using the liveness information.
 */
static void
optimize_initlocals2 (MonoCompile *cfg)
{
	MonoBitSet *used;
	MonoInst *ins;
	MonoBasicBlock *initlocals_bb;

	used = mono_bitset_new (cfg->next_vreg + 1, 0);

	mono_bitset_clear_all (used);
	initlocals_bb = cfg->bb_entry->next_bb;
	for (ins = initlocals_bb->code; ins; ins = ins->next) {
		const char *spec = INS_INFO (ins->opcode);

		if (spec [MONO_INST_SRC1] != ' ')
			mono_bitset_set_fast (used, ins->sreg1);
		if (spec [MONO_INST_SRC2] != ' ')
			mono_bitset_set_fast (used, ins->sreg2);
		if (MONO_IS_STORE_MEMBASE (ins))
			mono_bitset_set_fast (used, ins->dreg);
	}

	for (ins = initlocals_bb->code; ins; ins = ins->next) {
		const char *spec = INS_INFO (ins->opcode);

		/* Look for statements whose dest is not used in this bblock and not live on exit. */
		if ((spec [MONO_INST_DEST] != ' ') && !MONO_IS_STORE_MEMBASE (ins)) {
			MonoInst *var = get_vreg_to_inst (cfg, ins->dreg);

			if (var && !mono_bitset_test_fast (used, ins->dreg) && !mono_bitset_test_fast (initlocals_bb->live_out_set, var->inst_c0) && (var != cfg->ret) && !(var->flags & (MONO_INST_VOLATILE|MONO_INST_INDIRECT))) {
				//printf ("DEAD: "); mono_print_ins (ins);
				if ((ins->opcode == OP_ICONST) || (ins->opcode == OP_I8CONST) || (ins->opcode == OP_R8CONST)) {
					NULLIFY_INS (ins);
					MONO_VARINFO (cfg, var->inst_c0)->spill_costs -= 1;
					/* 
					 * We should shorten the liveness interval of these vars as well, but
					 * don't have enough info to do that.
					 */
				}
			}
		}
	}

	g_free (used);
}

void
mono_linterval_add_range (MonoCompile *cfg, MonoLiveInterval *interval, int from, int to)
{
	MonoLiveRange2 *prev_range, *next_range, *new_range;

	g_assert (to >= from);

	/* Optimize for extending the first interval backwards */
	if (G_LIKELY (interval->range && (interval->range->from > from) && (interval->range->from == to))) {
		interval->range->from = from;
		return;
	}

	/* Find a place in the list for the new range */
	prev_range = NULL;
	next_range = interval->range;
	while ((next_range != NULL) && (next_range->from <= from)) {
		prev_range = next_range;
		next_range = next_range->next;
	}

	if (prev_range && prev_range->to == from) {
		/* Merge with previous */
		prev_range->to = to;
	} else if (next_range && next_range->from == to) {
		/* Merge with previous */
		next_range->from = from;
	} else {
		/* Insert it */
		new_range = mono_mempool_alloc (cfg->mempool, sizeof (MonoLiveRange2));
		new_range->from = from;
		new_range->to = to;
		new_range->next = NULL;

		if (prev_range)
			prev_range->next = new_range;
		else
			interval->range = new_range;
		if (next_range)
			new_range->next = next_range;
		else
			interval->last_range = new_range;
	}

	/* FIXME: Merge intersecting ranges */
}

void
mono_linterval_print (MonoLiveInterval *interval)
{
	MonoLiveRange2 *range;

	for (range = interval->range; range != NULL; range = range->next)
		printf ("[%x-%x] ", range->from, range->to);
}

void
mono_linterval_print_nl (MonoLiveInterval *interval)
{
	mono_linterval_print (interval);
	printf ("\n");
}

/**
 * mono_linterval_convers:
 *
 *   Return whenever INTERVAL covers the position POS.
 */
gboolean
mono_linterval_covers (MonoLiveInterval *interval, int pos)
{
	MonoLiveRange2 *range;

	for (range = interval->range; range != NULL; range = range->next) {
		if (pos >= range->from && pos <= range->to)
			return TRUE;
		if (range->from > pos)
			return FALSE;
	}

	return FALSE;
}

/**
 * mono_linterval_get_intersect_pos:
 *
 *   Determine whenever I1 and I2 intersect, and if they do, return the first
 * point of intersection. Otherwise, return -1.
 */
gint32
mono_linterval_get_intersect_pos (MonoLiveInterval *i1, MonoLiveInterval *i2)
{
	MonoLiveRange2 *r1, *r2;

	/* FIXME: Optimize this */
	for (r1 = i1->range; r1 != NULL; r1 = r1->next) {
		for (r2 = i2->range; r2 != NULL; r2 = r2->next) {
			if (r2->to > r1->from && r2->from < r1->to) {
				if (r2->from <= r1->from)
					return r1->from;
				else
					return r2->from;
			}
		}
	}

	return -1;
}
 
/**
 * mono_linterval_split
 *
 *   Split L at POS and store the newly created intervals into L1 and L2. POS becomes
 * part of L2.
 */
void
mono_linterval_split (MonoCompile *cfg, MonoLiveInterval *interval, MonoLiveInterval **i1, MonoLiveInterval **i2, int pos)
{
	MonoLiveRange2 *r;

	g_assert (pos > interval->range->from && pos <= interval->last_range->to);

	*i1 = mono_mempool_alloc0 (cfg->mempool, sizeof (MonoLiveInterval));
	*i2 = mono_mempool_alloc0 (cfg->mempool, sizeof (MonoLiveInterval));

	for (r = interval->range; r; r = r->next) {
		if (pos > r->to) {
			/* Add it to the first child */
			mono_linterval_add_range (cfg, *i1, r->from, r->to);
		} else if (pos > r->from && pos <= r->to) {
			/* Split at pos */
			mono_linterval_add_range (cfg, *i1, r->from, pos - 1);
			mono_linterval_add_range (cfg, *i2, pos, r->to);
		} else {
			/* Add it to the second child */
			mono_linterval_add_range (cfg, *i2, r->from, r->to);
		}
	}
}

#ifdef ENABLE_LIVENESS2

#if 0
#define LIVENESS_DEBUG(a) do { a; } while (0)
#else
#define LIVENESS_DEBUG(a)
#endif

static inline void
update_liveness2 (MonoCompile *cfg, MonoInst *ins, gboolean set_volatile, int inst_num, gint32 *last_use)
{
	const char *spec = INS_INFO (ins->opcode);
	int sreg;

	LIVENESS_DEBUG (printf ("\t%x: ", inst_num); mono_print_ins (ins));

	if (ins->opcode == OP_NOP)
		return;

	/* DREG */
	if ((spec [MONO_INST_DEST] != ' ') && get_vreg_to_inst (cfg, ins->dreg)) {
		MonoInst *var = get_vreg_to_inst (cfg, ins->dreg);
		int idx = var->inst_c0;
		MonoMethodVar *vi = MONO_VARINFO (cfg, idx);

		if (MONO_IS_STORE_MEMBASE (ins)) {
			if (last_use [idx] == 0) {
				LIVENESS_DEBUG (printf ("\tlast use of R%d set to %x\n", ins->dreg, inst_num));
				last_use [idx] = inst_num;
			}
		} else {
			if (last_use [idx] > 0) {
				LIVENESS_DEBUG (printf ("\tadd range to R%d: [%x, %x)\n", ins->dreg, inst_num, last_use [idx]));
				mono_linterval_add_range (cfg, vi->interval, inst_num, last_use [idx]);
				last_use [idx] = 0;
			}
			else {
				/* Try dead code elimination */
				if ((var != cfg->ret) && !(var->flags & (MONO_INST_VOLATILE|MONO_INST_INDIRECT)) && ((ins->opcode == OP_ICONST) || (ins->opcode == OP_I8CONST) || (ins->opcode == OP_R8CONST)) && !(var->flags & MONO_INST_VOLATILE)) {
					LIVENESS_DEBUG (printf ("\tdead def of R%d, eliminated\n", ins->dreg));
					ins->opcode = OP_NOP;
					ins->dreg = ins->sreg1 = ins->sreg2 = -1;
					return;
				}

				LIVENESS_DEBUG (printf ("\tdead def of R%d, add range to R%d: [%x, %x]\n", ins->dreg, ins->dreg, inst_num, inst_num + 1));
				mono_linterval_add_range (cfg, vi->interval, inst_num, inst_num + 1);
			}
		}
	}

	/* SREG1 */
	sreg = ins->sreg1;
	if ((spec [MONO_INST_SRC1] != ' ') && get_vreg_to_inst (cfg, sreg)) {
		MonoInst *var = get_vreg_to_inst (cfg, sreg);
		int idx = var->inst_c0;

		if (last_use [idx] == 0) {
			LIVENESS_DEBUG (printf ("\tlast use of R%d set to %x\n", sreg, inst_num));
			last_use [idx] = inst_num;
		}
	}

	/* SREG2 */
	sreg = ins->sreg2;
	if ((spec [MONO_INST_SRC2] != ' ') && get_vreg_to_inst (cfg, sreg)) {
		MonoInst *var = get_vreg_to_inst (cfg, sreg);
		int idx = var->inst_c0;

		if (last_use [idx] == 0) {
			LIVENESS_DEBUG (printf ("\tlast use of R%d set to %x\n", sreg, inst_num));
			last_use [idx] = inst_num;
		}
	}
}

static void
mono_analyze_liveness2 (MonoCompile *cfg)
{
	int bnum, idx, i, j, nins, rem, max, max_vars, block_from, block_to, pos, reverse_len;
	gint32 *last_use;
	static guint32 disabled = -1;
	MonoInst **reverse;

	if (disabled == -1)
		disabled = getenv ("DISABLED") != NULL;

	if (disabled)
		return;

	LIVENESS_DEBUG (printf ("LIVENESS 2 %s\n", mono_method_full_name (cfg->method, TRUE)));

	/*
	if (strstr (cfg->method->name, "test_") != cfg->method->name)
		return;
	*/

	max_vars = cfg->num_varinfo;
	last_use = g_new0 (gint32, max_vars);

	reverse_len = 1024;
	reverse = mono_mempool_alloc (cfg->mempool, sizeof (MonoInst*) * reverse_len);

	for (idx = 0; idx < max_vars; ++idx) {
		MonoMethodVar *vi = MONO_VARINFO (cfg, idx);

		vi->interval = mono_mempool_alloc0 (cfg->mempool, sizeof (MonoLiveInterval));
	}

	/*
	 * Process bblocks in reverse order, so the addition of new live ranges
	 * to the intervals is faster.
	 */
	for (bnum = cfg->num_bblocks - 1; bnum >= 0; --bnum) {
		MonoBasicBlock *bb = cfg->bblocks [bnum];
		MonoInst *ins;

		block_from = (bb->dfn << 16) + 1; /* so pos > 0 */
		if (bnum < cfg->num_bblocks - 1)
			/* Beginning of the next bblock */
			block_to = (cfg->bblocks [bnum + 1]->dfn << 16) + 1;
		else
			block_to = (bb->dfn << 16) + 0xffff;

		LIVENESS_DEBUG (printf ("LIVENESS BLOCK BB%d:\n", bb->block_num));

		memset (last_use, 0, max_vars * sizeof (gint32));
		
		/* For variables in bb->live_out, set last_use to block_to */

		rem = max_vars % BITS_PER_CHUNK;
		max = ((max_vars + (BITS_PER_CHUNK -1)) / BITS_PER_CHUNK);
		for (j = 0; j < max; ++j) {
			gsize bits_out;
			int k;

			bits_out = mono_bitset_get_fast (bb->live_out_set, j);
			k = (j * BITS_PER_CHUNK);	
			while (bits_out) {
				if (bits_out & 1) {
					LIVENESS_DEBUG (printf ("Var R%d live at exit, set last_use to %x\n", cfg->varinfo [k]->dreg, block_to));
					last_use [k] = block_to;
				}
				bits_out >>= 1;
				k ++;
			}
		}

		if (cfg->ret)
			last_use [cfg->ret->inst_c0] = block_to;

		for (nins = 0, pos = block_from, ins = bb->code; ins; ins = ins->next, ++nins, ++pos) {
			if (nins >= reverse_len) {
				int new_reverse_len = reverse_len * 2;
				MonoInst **new_reverse = mono_mempool_alloc (cfg->mempool, sizeof (MonoInst*) * new_reverse_len);
				memcpy (new_reverse, reverse, sizeof (MonoInst*) * reverse_len);
				reverse = new_reverse;
				reverse_len = new_reverse_len;
			}

			reverse [nins] = ins;
		}

		/* Process instructions backwards */
		for (i = nins - 1; i >= 0; --i) {
			MonoInst *ins = (MonoInst*)reverse [i];

 			update_liveness2 (cfg, ins, FALSE, pos, last_use);

			pos --;
		}

		for (idx = 0; idx < max_vars; ++idx) {
			MonoMethodVar *vi = MONO_VARINFO (cfg, idx);

			if (last_use [idx] != 0) {
				/* Live at exit, not written -> live on enter */
				LIVENESS_DEBUG (printf ("Var R%d live at enter, add range to R%d: [%x, %x)\n", cfg->varinfo [idx]->dreg, cfg->varinfo [idx]->dreg, block_from, last_use [idx]));
				mono_linterval_add_range (cfg, vi->interval, block_from, last_use [idx]);
			}
		}
	}

	/*
	 * Arguments need to have their live ranges extended to the beginning of
	 * the method to account for the arg reg/memory -> global register copies
	 * in the prolog (bug #74992).
	 */
	for (i = 0; i < max_vars; i ++) {
		MonoMethodVar *vi = MONO_VARINFO (cfg, i);
		if (cfg->varinfo [vi->idx]->opcode == OP_ARG)
			mono_linterval_add_range (cfg, vi->interval, 0, 1);
	}

#if 0
	for (idx = 0; idx < max_vars; ++idx) {
		MonoMethodVar *vi = MONO_VARINFO (cfg, idx);
		
		LIVENESS_DEBUG (printf ("LIVENESS R%d: ", cfg->varinfo [idx]->dreg));
		LIVENESS_DEBUG (mono_linterval_print (vi->interval));
		LIVENESS_DEBUG (printf ("\n"));
	}
#endif

	g_free (last_use);
}

#endif

static void
update_used (MonoCompile *cfg, MonoInst *inst, MonoBitSet *used)
{
	int arity = mono_burg_arity [inst->opcode];

	if (arity)
		update_used (cfg, inst->inst_i0, used);

	if (arity > 1)
		update_used (cfg, inst->inst_i1, used);

	if (inst->ssa_op & MONO_SSA_LOAD_STORE) {
		if (inst->ssa_op == MONO_SSA_LOAD) {
			int idx = inst->inst_i0->inst_c0;

			mono_bitset_set_fast (used, idx);
		}
	}
} 

/**
 * optimize_initlocals:
 *
 * Try to optimize away some of the redundant initialization code inserted because of
 * 'locals init' using the liveness information.
 */
static void
optimize_initlocals (MonoCompile *cfg)
{
	MonoBitSet *used;
	MonoInst *ins;
	MonoBasicBlock *initlocals_bb;

	used = mono_bitset_new (cfg->num_varinfo, 0);

	mono_bitset_clear_all (used);
	initlocals_bb = cfg->bb_entry->next_bb;
	MONO_BB_FOR_EACH_INS (initlocals_bb, ins)
		update_used (cfg, ins, used);

	MONO_BB_FOR_EACH_INS (initlocals_bb, ins) {
		if (ins->ssa_op == MONO_SSA_STORE) {
			int idx = ins->inst_i0->inst_c0;
			MonoInst *var = cfg->varinfo [idx];

			if (var && !mono_bitset_test_fast (used, idx) && !mono_bitset_test_fast (initlocals_bb->live_out_set, var->inst_c0) && (var != cfg->ret) && !(var->flags & (MONO_INST_VOLATILE|MONO_INST_INDIRECT))) {
				if (ins->inst_i1 && ((ins->inst_i1->opcode == OP_ICONST) || (ins->inst_i1->opcode == OP_I8CONST))) {
					NULLIFY_INS (ins);
					ins->ssa_op = MONO_SSA_NOP;
					MONO_VARINFO (cfg, var->inst_c0)->spill_costs -= 1;					
				}
			}
		}
	}

	g_free (used);
}

