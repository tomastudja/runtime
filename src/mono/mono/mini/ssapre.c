/*
 * ssapre.h: SSA Partial Redundancy Elimination
 *
 * Author:
 *   Massimiliano Mantione (massi@ximian.com)
 *
 * (C) 2004 Novell, Inc.  http://www.novell.com
 */

#include <string.h>
#include <stdio.h>

#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/opcodes.h>

#include "inssel.h"

#include "ssapre.h"

extern guint8 mono_burg_arity [];

/* Logging conditions */
#define DUMP_LEVEL (4)
#define TRACE_LEVEL (3)
#define STATISTICS_LEVEL (2)
#define LOG_LEVEL (1)
#define DUMP_SSAPRE (area->cfg->verbose_level >= DUMP_LEVEL)
#define TRACE_SSAPRE (area->cfg->verbose_level >= TRACE_LEVEL)
#define STATISTICS_SSAPRE (area->cfg->verbose_level >= STATISTICS_LEVEL)
#define LOG_SSAPRE (area->cfg->verbose_level >= LOG_LEVEL)

/* "Bottom" symbol definition (see paper) */
#define BOTTOM_REDUNDANCY_CLASS (-1)

/* Various printing functions... */
static void
print_argument (MonoSsapreExpressionArgument *argument) {
	switch (argument->type) {
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_ANY:
			printf ("ANY");
			break;
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_NOT_PRESENT:
			printf ("NONE");
			break;
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_ORIGINAL_VARIABLE:
			printf ("ORIGINAL_VARIABLE %d", argument->argument.original_variable);
			break;
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_SSA_VARIABLE:
			printf ("SSA_VARIABLE %d", argument->argument.ssa_variable);
			break;
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_INTEGER_CONSTANT:
			printf ("INTEGER_CONSTANT %d", argument->argument.integer_constant);
			break;
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_LONG_COSTANT:
			printf ("LONG_COSTANT %lld", *(argument->argument.long_constant));
			break;
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_FLOAT_COSTANT:
			printf ("FLOAT_COSTANT %f", *(argument->argument.float_constant));
			break;
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_DOUBLE_COSTANT:
			printf ("DOUBLE_COSTANT %f", *(argument->argument.double_constant));
			break;
		default:
			printf ("UNKNOWN: %d", argument->type);
	}
}


static void
print_expression_description (MonoSsapreExpressionDescription *expression_description) {
	if (expression_description->opcode != 0) {
		printf ("%s ([", mono_inst_name (expression_description->opcode) );
		print_argument (&(expression_description->left_argument));
		printf ("],[");
		print_argument (&(expression_description->right_argument));
		printf ("])");
	} else {
		printf ("ANY");
	}
}

#define GBOOLEAN_TO_STRING(b) ((b)?"TRUE":"FALSE")

static void
print_expression_occurrence (MonoSsapreExpressionOccurrence *occurrence, int number) {
	printf (" ([%d][bb %d [ID %d]][class %d]: ", number, occurrence->bb_info->cfg_dfn, occurrence->bb_info->bb->block_num, occurrence->redundancy_class);
	print_expression_description (&(occurrence->description));
	if (occurrence->is_first_in_bb) {
		printf (" [FIRST in BB]");
	}
	if (occurrence->is_last_in_bb) {
		printf (" [LAST in BB]");
	}
	printf (" (save = %s)", GBOOLEAN_TO_STRING (occurrence->save));
	printf (" (reload = %s)", GBOOLEAN_TO_STRING (occurrence->reload));
	printf ("\n");
	occurrence = occurrence->next;
}

static void
print_expression_occurrences (MonoSsapreExpressionOccurrence *occurrences) {
	int i = 0;
	while (occurrences != NULL) {
		i++;
		print_expression_occurrence (occurrences, i);
		occurrences = occurrences->next;
	}
}

static void
print_worklist (MonoSsapreExpression *expression) {
	if (expression != NULL) {
		print_worklist (expression->previous);
		
		printf ("{%d}: ", expression->tree_size);
		print_expression_description (&(expression->description));
		printf ("\n");
		print_expression_occurrences (expression->occurrences);
		
		print_worklist (expression->next);
	}
}

static void
print_bb_info (MonoSsapreBBInfo *bb_info, gboolean print_occurrences) {
	int i;
	MonoSsapreExpressionOccurrence *current_occurrence = bb_info->first_expression_in_bb;
	
	printf ("bb %d [ID %d]: IN { ", bb_info->cfg_dfn, bb_info->bb->block_num);
	for (i = 0; i < bb_info->in_count; i++) {
		printf ("%d [ID %d] ", bb_info->in_bb [i]->cfg_dfn, bb_info->in_bb [i]->bb->block_num);
	}
	printf ("}, OUT {");
	for (i = 0; i < bb_info->out_count; i++) {
		printf ("%d [ID %d] ", bb_info->out_bb [i]->cfg_dfn, bb_info->out_bb [i]->bb->block_num);
	}
	printf ("}");
	if (bb_info->next_interesting_bb != NULL) {
		printf (", NEXT %d [ID %d]", bb_info->next_interesting_bb->cfg_dfn, bb_info->next_interesting_bb->bb->block_num);
	}
	printf ("\n");
	if (bb_info->has_phi) {
		printf (" PHI, class %d [ ", bb_info->phi_redundancy_class);
		for (i = 0; i < bb_info->in_count; i++) {
			int argument_class = bb_info->phi_arguments_classes [i];
			if (argument_class != BOTTOM_REDUNDANCY_CLASS) {
				printf ("%d ", argument_class);
			} else {
				printf ("BOTTOM ");
			}
		}
		printf ("]\n(phi_defines_a_real_occurrence:%s) (phi_is_down_safe:%s) (phi_can_be_available:%s) (phi_is_later:%s)\n",
				GBOOLEAN_TO_STRING (bb_info->phi_defines_a_real_occurrence), GBOOLEAN_TO_STRING (bb_info->phi_is_down_safe),
				GBOOLEAN_TO_STRING (bb_info->phi_can_be_available), GBOOLEAN_TO_STRING (bb_info->phi_is_later));
	}
	if (print_occurrences) {
		i = 0;
		while ((current_occurrence != NULL) && (current_occurrence->bb_info == bb_info)) {
			print_expression_occurrence (current_occurrence, i);
			current_occurrence = current_occurrence->next;
			i++;
		}
	}
	if (bb_info->has_phi_argument) {
		printf (" PHI ARGUMENT, class ");
		if (bb_info->phi_argument_class != BOTTOM_REDUNDANCY_CLASS) {
			printf ("%d ", bb_info->phi_argument_class);
		} else {
			printf ("BOTTOM ");
		}
		if (bb_info->phi_argument_defined_by_real_occurrence != NULL) {
			printf ("(Defined by real occurrence %d)", bb_info->phi_argument_defined_by_real_occurrence->redundancy_class);
		} else if (bb_info->phi_argument_defined_by_phi != NULL) {
			printf ("(Defined by phi %d)", bb_info->phi_argument_defined_by_phi->phi_redundancy_class);
		} else {
			printf ("(Undefined)");
		}
		printf (" (real occurrence arguments: left ");
		if (bb_info->phi_argument_left_argument_version != BOTTOM_REDUNDANCY_CLASS) {
			printf ("%d ", bb_info->phi_argument_left_argument_version);
		} else {
			printf ("NONE ");
		}
		printf (", right ");
		if (bb_info->phi_argument_right_argument_version != BOTTOM_REDUNDANCY_CLASS) {
			printf ("%d ", bb_info->phi_argument_right_argument_version);
		} else {
			printf ("NONE ");
		}
		printf (")\n(phi_argument_has_real_use:%s) (phi_argument_needs_insert:%s) (phi_argument_has_been_processed:%s)\n",
				GBOOLEAN_TO_STRING (bb_info->phi_argument_has_real_use), GBOOLEAN_TO_STRING (bb_info->phi_argument_needs_insert),
				GBOOLEAN_TO_STRING (bb_info->phi_argument_has_been_processed));
	}
}

static void
print_bb_infos (MonoSsapreWorkArea *area) {
	int i;
	for (i = 0; i < area->num_bblocks; i++) {
		MonoSsapreBBInfo *bb_info = &(area->bb_infos [i]);
		print_bb_info (bb_info, TRUE);
	}
}

static void
print_interesting_bb_infos (MonoSsapreWorkArea *area) {
	MonoSsapreBBInfo *current_bb;
	
	for (current_bb = area->first_interesting_bb; current_bb != NULL; current_bb = current_bb->next_interesting_bb) {
		print_bb_info (current_bb, FALSE);
	}
}

static void dump_code (MonoSsapreWorkArea *area) {
	int i;
	
	for (i = 0; i < area->num_bblocks; i++) {
		MonoSsapreBBInfo *current_bb = &(area->bb_infos [i]);
		MonoInst *current_inst;
		
		print_bb_info (current_bb, TRUE);
		for (current_inst = current_bb->bb->code; current_inst != NULL; current_inst = current_inst->next) {
			mono_print_tree_nl (current_inst);
		}
	}
}

/*
 * Given a MonoInst, it tries to describe it as an expression argument.
 * If this is not possible, the result will be of type
 * MONO_SSAPRE_EXPRESSION_ARGUMENT_ANY.
 */
static void
analyze_argument (MonoInst *argument, MonoSsapreExpressionArgument *result) {
	switch (argument->opcode) {
	case CEE_LDIND_I1:
	case CEE_LDIND_U1:
	case CEE_LDIND_I2:
	case CEE_LDIND_U2:
	case CEE_LDIND_I4:
	case CEE_LDIND_U4:
	case CEE_LDIND_I8:
	case CEE_LDIND_I:
	case CEE_LDIND_R4:
	case CEE_LDIND_R8:
	case CEE_LDIND_REF:
		analyze_argument (argument->inst_left, result);
		break;
	case OP_ICONST:
		result->type = MONO_SSAPRE_EXPRESSION_ARGUMENT_INTEGER_CONSTANT;
		result->argument.integer_constant = argument->inst_c0;
		break;
	case OP_I8CONST:
		result->type = MONO_SSAPRE_EXPRESSION_ARGUMENT_LONG_COSTANT;
		result->argument.long_constant = &(argument->inst_l);
		break;
	case OP_R4CONST:
		result->type = MONO_SSAPRE_EXPRESSION_ARGUMENT_FLOAT_COSTANT;
		result->argument.float_constant = (float*)argument->inst_p0;
		break;
	case OP_R8CONST:
		result->type = MONO_SSAPRE_EXPRESSION_ARGUMENT_DOUBLE_COSTANT;
		result->argument.double_constant = (double*)argument->inst_p0;
		break;
	case OP_ARG:
	case OP_LOCAL:
		result->type = MONO_SSAPRE_EXPRESSION_ARGUMENT_SSA_VARIABLE;
		result->argument.ssa_variable = argument->inst_c0;
		break;
	default:
		result->type = MONO_SSAPRE_EXPRESSION_ARGUMENT_ANY;
	}
}

/*
 * Given a MonoInst, it tries to describe it as an expression.
 * If this is not possible, the result will have opcode 0.
 */
static void
analyze_expression (MonoInst *expression, MonoSsapreExpressionDescription *result) {
	switch (expression->opcode) {
	case CEE_LDIND_I1:
	case CEE_LDIND_U1:
	case CEE_LDIND_I2:
	case CEE_LDIND_U2:
	case CEE_LDIND_I4:
	case CEE_LDIND_U4:
	case CEE_LDIND_I8:
	case CEE_LDIND_I:
	case CEE_LDIND_R4:
	case CEE_LDIND_R8:
	case CEE_LDIND_REF:
		analyze_expression (expression->inst_left, result);
		break;
	case CEE_SWITCH:
	case CEE_ISINST:
	case CEE_CASTCLASS:
	case OP_OUTARG:
	case OP_CALL_REG:
	case OP_FCALL_REG:
	case OP_LCALL_REG:
	case OP_VCALL_REG:
	case OP_VOIDCALL_REG:
	case CEE_CALL:
	case CEE_CALLVIRT:
	case OP_FCALL:
	case OP_FCALLVIRT:
	case OP_LCALL:
	case OP_LCALLVIRT:
	case OP_VCALL:
	case OP_VCALLVIRT:
	case OP_VOIDCALL:
	case OP_VOIDCALLVIRT:
	case OP_RENAME:
	case OP_RETARG:
//	case OP_OUTARG:
	case OP_OUTARG_REG:
	case OP_OUTARG_IMM:
	case OP_OUTARG_R4:
	case OP_OUTARG_R8:
	case OP_OUTARG_VT:
	case CEE_NOP:
	case CEE_JMP:
	case CEE_BREAK:
	case OP_COMPARE:
	case OP_COMPARE_IMM:
	case OP_FCOMPARE:
	case OP_LCOMPARE:
	case OP_ICOMPARE:
	case OP_ICOMPARE_IMM:
		result->opcode = 0;
		break;
	default:
		if ( (expression->type == STACK_I4) ||
				(expression->type == STACK_I8) ||
				(expression->type == STACK_R8) ) {
			if (mono_burg_arity [expression->opcode] > 0) {
				analyze_argument (expression->inst_left, &(result->left_argument));
				if (result->left_argument.type != MONO_SSAPRE_EXPRESSION_ARGUMENT_ANY) {
					if (mono_burg_arity [expression->opcode] > 1) {
						analyze_argument (expression->inst_right, &(result->right_argument));
						if (result->right_argument.type != MONO_SSAPRE_EXPRESSION_ARGUMENT_ANY) {
							result->opcode = expression->opcode;
						} else {
							result->opcode = 0;
						}
					} else {
						result->right_argument.type = MONO_SSAPRE_EXPRESSION_ARGUMENT_NOT_PRESENT;
						result->opcode = expression->opcode;
					}
				} else {
					result->opcode = 0;
				}
			} else {
				result->opcode = 0;
			}
		} else {
			result->opcode = 0;
		}
	}
}

/*
 * Given an expression description, if any of its arguments has type
 * MONO_SSAPRE_EXPRESSION_ARGUMENT_SSA_VARIABLE it transforms it to a
 * MONO_SSAPRE_EXPRESSION_ARGUMENT_ORIGINAL_VARIABLE, converting the
 * variable index to its "original" one.
 * The resulting description is OK for a "MonoSsapreExpression" (the
 * initial description came from a "MonoSsapreExpressionOccurrence").
 */
static void
convert_ssa_variables_to_original_names (MonoSsapreExpressionDescription *result, MonoSsapreExpressionDescription *expression, MonoCompile *cfg) {
	gssize original_variable;
	
	result->opcode = expression->opcode;
	if (expression->left_argument.type != MONO_SSAPRE_EXPRESSION_ARGUMENT_SSA_VARIABLE) {
		result->left_argument = expression->left_argument;
	} else {
		result->left_argument.type = MONO_SSAPRE_EXPRESSION_ARGUMENT_ORIGINAL_VARIABLE;
		original_variable = cfg->vars [expression->left_argument.argument.ssa_variable]->reg;
		if (original_variable >= 0) {
			result->left_argument.argument.original_variable = original_variable;
		} else {
			result->left_argument.argument.original_variable = expression->left_argument.argument.ssa_variable;
		}
	}
	if (expression->right_argument.type != MONO_SSAPRE_EXPRESSION_ARGUMENT_SSA_VARIABLE) {
		result->right_argument = expression->right_argument;
	} else {
		result->right_argument.type = MONO_SSAPRE_EXPRESSION_ARGUMENT_ORIGINAL_VARIABLE;
		original_variable = cfg->vars [expression->right_argument.argument.ssa_variable]->reg;
		if (original_variable >= 0) {
			result->right_argument.argument.original_variable = original_variable;
		} else {
			result->right_argument.argument.original_variable = expression->right_argument.argument.ssa_variable;
		}
	}
}

/*
 * Given a MonoInst, checks if it is a phi definition.
 */
static gboolean
is_phi_definition (MonoInst *definition) {
	if (definition != NULL) {
		switch (definition->opcode) {
		case CEE_STIND_REF:
		case CEE_STIND_I:
		case CEE_STIND_I4:
		case CEE_STIND_I1:
		case CEE_STIND_I2:
		case CEE_STIND_I8:
		case CEE_STIND_R4:
		case CEE_STIND_R8:
			if ((definition->inst_left->opcode == OP_LOCAL) &&
					(definition->inst_right->opcode == OP_PHI)) {
				return TRUE;
			} else {
				return FALSE;
			}
		default:
			return FALSE;
		}
	} else {
		return FALSE;
	}
}

/*
 * Given a variable index, checks if it is a phi definition
 * (it assumes the "area" is visible).
 */
#define IS_PHI_DEFINITION(v) is_phi_definition (area->cfg->vars [v]->def)

/*
 * Given a variable index, checks if it is a phi definition.
 * If it is so, it returns the array of integers pointed to by the phi MonoInst
 * (the one containing the length and the phi arguments), otherwise returns NULL.
 */
static int*
get_phi_definition (MonoCompile *cfg, gssize variable) {
	MonoInst *definition = cfg->vars [variable]->def;
	if (is_phi_definition (definition) && (definition->inst_left->inst_c0 == variable)) {
		return definition->inst_right->inst_phi_args;
	} else {
		return NULL;
	}
}

/*
 * Given a variable index, returns the BB where the variable is defined.
 * If the variable appears to have no definition, it returns the entry BB
 * (the logic is that if it has no definition it is an argument, or a sort
 * of constant, and in both cases it is defined in the entry BB).
 */
static MonoSsapreBBInfo*
get_bb_info_of_definition (MonoSsapreWorkArea *area, gssize variable) {
	MonoBasicBlock *def_bb = area->cfg->vars [variable]->def_bb;
	if (def_bb != NULL) {
		return area->bb_infos_in_cfg_dfn_order [def_bb->dfn];
	} else {
		return area->bb_infos;
	}
}

/*
 * Given:
 * - a variable index,
 * - a BB (the "current" BB), and
 * - a "phi argument index" (or index in the "in_bb" of the current BB),
 * it checks if the variable is a phi.
 * If it is so, it returns the variable index at the in_bb corresponding to
 * the given index, otherwise it return the variable index unchanged.
 * The logic is to return the variable version at the given predecessor BB.
 */
static gssize
get_variable_version_at_predecessor_bb (MonoSsapreWorkArea *area, gssize variable, MonoSsapreBBInfo *current_bb, int phi_operand) {
	MonoSsapreBBInfo *definition_bb = get_bb_info_of_definition (area, variable);
	int *phi_definition = get_phi_definition (area->cfg, variable);
	if ((definition_bb == current_bb) && (phi_definition != NULL)) {
		return phi_definition [phi_operand + 1];
	} else {
		return variable;
	}
}

/*
 * Adds an expression to an autobalancing binary tree of expressions.
 * Returns the new tree root (it can be different from "tree" because of the
 * autobalancing).
 */
static MonoSsapreExpression*
add_expression_to_tree (MonoSsapreExpression *tree, MonoSsapreExpression *expression) {
	MonoSsapreExpression *subtree;
	MonoSsapreExpression **subtree_reference;
	int comparison;
	gssize max_tree_depth, max_subtree_size;
	
	if (tree == NULL) {
		expression->previous = NULL;
		expression->next = NULL;
		expression->tree_size = 1;
		return expression;
	}
	
	tree->tree_size++;
	
	comparison = MONO_COMPARE_SSAPRE_EXPRESSION_DESCRIPTIONS (expression->description, tree->description);
	if (comparison > 0) {
		if (tree->next != NULL) {
			subtree = tree->next;
			subtree_reference = &(tree->next);
		} else {
			tree->next = expression;
			expression->father = tree;
			expression->previous = NULL;
			expression->next = NULL;
			expression->tree_size = 1;
			return tree;
		}
	} else if (comparison < 0) {
		if (tree->previous != NULL) {
			subtree = tree->previous;
			subtree_reference = &(tree->previous);
		} else {
			tree->previous = expression;
			expression->father = tree;
			expression->previous = NULL;
			expression->next = NULL;
			expression->tree_size = 1;
			return tree;
		}
	} else {
		g_assert_not_reached ();
		return NULL;
	}
	
	MONO_SSAPRE_MAX_TREE_DEPTH (tree->tree_size, max_tree_depth);
	max_subtree_size = (1 << max_tree_depth) -1;
	
	if (subtree->tree_size < max_subtree_size) {
		*subtree_reference = add_expression_to_tree (subtree, expression);
		(*subtree_reference)->father = tree;
		return tree;
	} else {
		MonoSsapreExpression *other_subtree;
		MonoSsapreExpression *border_expression;
		MonoSsapreExpression *border_expression_subtree;
		MonoSsapreExpression **border_expression_reference_from_father;
		int comparison_with_border;
		
		border_expression = subtree;
		if (comparison > 0) {
			other_subtree = tree->previous;
			MONO_SSAPRE_GOTO_FIRST_EXPRESSION (border_expression);
			border_expression_subtree = border_expression->next;
			border_expression_reference_from_father = &(border_expression->father->previous);
		} else if (comparison < 0) {
			other_subtree = tree->next;
			MONO_SSAPRE_GOTO_LAST_EXPRESSION (border_expression);
			border_expression_subtree = border_expression->previous;
			border_expression_reference_from_father = &(border_expression->father->next);
		} else {
			g_assert_not_reached ();
			return NULL;
		}
		comparison_with_border = MONO_COMPARE_SSAPRE_EXPRESSION_DESCRIPTIONS (expression->description, border_expression->description);
		
		if ((comparison + comparison_with_border) != 0) {
			MonoSsapreExpression *border_expression_iterator = border_expression->father;
			while (border_expression_iterator != tree) {
				border_expression_iterator->tree_size--;
				border_expression_iterator = border_expression_iterator->father;
			}
			
			if (border_expression != subtree) {
				*border_expression_reference_from_father = border_expression_subtree;
			} else {
				subtree = border_expression_subtree;
			}
			if (border_expression_subtree != NULL) {
				border_expression_subtree->father = border_expression->father;
			}
			
			if (comparison > 0) {
				border_expression->previous = add_expression_to_tree (other_subtree, tree);
				border_expression->next = add_expression_to_tree (subtree, expression);
			} else if (comparison < 0) {
				border_expression->previous = add_expression_to_tree (subtree, expression);
				border_expression->next = add_expression_to_tree (other_subtree, tree);
			} else {
				g_assert_not_reached ();
				return NULL;
			}
			border_expression->tree_size = 1 + border_expression->previous->tree_size + border_expression->next->tree_size;
			return border_expression;
		} else {
			if (comparison > 0) {
				expression->previous = add_expression_to_tree (other_subtree, tree);
				expression->next = subtree;
			} else if (comparison < 0) {
				expression->previous = subtree;
				expression->next = add_expression_to_tree (other_subtree, tree);
			} else {
				g_assert_not_reached ();
				return NULL;
			}
			expression->tree_size = 1 + expression->previous->tree_size + expression->next->tree_size;
			return expression;
		}
	}
}

/*
 * Adds an expression to the worklist (putting the given occurrence as first
 * occurrence of this expression).
 */
static void
add_expression_to_worklist (MonoSsapreWorkArea *area, MonoSsapreExpressionOccurrence *occurrence) {
	MonoSsapreExpression *expression;
	
	expression = (MonoSsapreExpression*) mono_mempool_alloc (area->mempool, sizeof (MonoSsapreExpression));
	
	convert_ssa_variables_to_original_names (&(expression->description), &(occurrence->description), area->cfg);
	expression->type = mono_type_from_stack_type (occurrence->occurrence);
	expression->occurrences = NULL;
	expression->last_occurrence = NULL;
	MONO_SSAPRE_ADD_EXPRESSION_OCCURRANCE (expression, occurrence);
	
	area->worklist = add_expression_to_tree (area->worklist, expression);
	area->worklist->father = NULL;
}

/*
 * Adds an expression occurrence to the worklist.
 * If its expression is not yet in the worklist, it is created.
 */
static void
add_occurrence_to_worklist (MonoSsapreWorkArea *area, MonoSsapreExpressionOccurrence *occurrence) {
	MonoSsapreExpressionDescription description;
	MonoSsapreExpression *current_expression;
	int comparison;
	
	convert_ssa_variables_to_original_names (&description, &(occurrence->description), area->cfg);
	current_expression = area->worklist;
	
	do {
		if (current_expression != NULL) {
			comparison = MONO_COMPARE_SSAPRE_EXPRESSION_DESCRIPTIONS (description, current_expression->description);

			if (comparison > 0) {
				current_expression = current_expression->next;
			} else if (comparison < 0) {
				current_expression = current_expression->previous;
			} else {
				MONO_SSAPRE_ADD_EXPRESSION_OCCURRANCE (current_expression, occurrence);
			}
		} else {
			add_expression_to_worklist (area, occurrence);
			comparison = 0;
		}
	} while (comparison != 0);
}

/*
 * Process a MonoInst, and of it is a valid expression add it to the worklist.
 */
static MonoSsapreExpressionOccurrence*
process_inst (MonoSsapreWorkArea *area, MonoInst* inst, MonoInst* previous_inst, MonoSsapreBBInfo *bb_info, MonoSsapreExpressionOccurrence *current_occurrence) {
	
	/* Ugly hack to fix missing variable definitions */
	/* (the SSA construction code should have done it already!) */
	switch (inst->opcode) {
	case CEE_STIND_REF:
	case CEE_STIND_I:
	case CEE_STIND_I4:
	case CEE_STIND_I1:
	case CEE_STIND_I2:
	case CEE_STIND_I8:
	case CEE_STIND_R4:
	case CEE_STIND_R8:
		if ((inst->inst_left->opcode == OP_LOCAL) || (inst->inst_left->opcode == OP_ARG)) {
			int variable_index = inst->inst_left->inst_c0;
			
			if (area->cfg->vars [variable_index]->def_bb == NULL) {
				if (area->cfg->verbose_level >= 4) {
					printf ("SSAPRE WARNING: variable %d has no definition, fixing.\n", variable_index);
				}
				area->cfg->vars [variable_index]->def_bb = bb_info->bb;
			}
		}
		break;
	default:
		break;
	}
	
	if (mono_burg_arity [inst->opcode] > 0) {
		current_occurrence = process_inst (area, inst->inst_left, previous_inst, bb_info, current_occurrence);
		if (mono_burg_arity [inst->opcode] > 1) {
			current_occurrence = process_inst (area, inst->inst_right, previous_inst, bb_info, current_occurrence);
		}
	}
	
	analyze_expression (inst, &(current_occurrence->description));
	if (current_occurrence->description.opcode != 0) {
		current_occurrence->occurrence = inst;
		current_occurrence->previous_tree = previous_inst;
		current_occurrence->bb_info = bb_info;
		current_occurrence->is_first_in_bb = FALSE;
		current_occurrence->is_last_in_bb = FALSE;
		add_occurrence_to_worklist (area, current_occurrence);
		current_occurrence = (MonoSsapreExpressionOccurrence*) mono_mempool_alloc (area->mempool, sizeof (MonoSsapreExpressionOccurrence));
	}
	
	return current_occurrence;
}

/*
 * Process a BB, and (recursively) all its children in the dominator tree.
 * The result is that all the expressions end up in the worklist, and the
 * auxiliary MonoSsapreBBInfo fields (dt_dfn, dt_descendants) are initialized
 * (with all the info that comes from the MonoBasicBlock).
 */
static MonoSsapreExpressionOccurrence*
process_bb (MonoSsapreWorkArea *area, MonoBasicBlock *bb, int *dt_dfn, int *upper_descendants, MonoSsapreExpressionOccurrence *current_occurrence) {
	MonoSsapreBBInfo *bb_info;
	int descendants;
	GList *dominated_bb;
	MonoInst* current_inst;
	MonoInst* previous_inst;
	
	bb_info = &(area->bb_infos [*dt_dfn]);
	bb_info->dt_dfn = *dt_dfn;
	(*dt_dfn)++;
	bb_info->cfg_dfn = bb->dfn;
	area->bb_infos_in_cfg_dfn_order [bb->dfn] = bb_info;
	bb_info->in_count = bb->in_count;
	bb_info->out_count = bb->out_count;
	bb_info->dfrontier = bb->dfrontier;
	bb_info->bb = bb;
	bb_info->in_bb = (MonoSsapreBBInfo**) mono_mempool_alloc (area->mempool, sizeof (MonoSsapreBBInfo*) * (bb->in_count));
	bb_info->out_bb = (MonoSsapreBBInfo**) mono_mempool_alloc (area->mempool, sizeof (MonoSsapreBBInfo*) * (bb->out_count));
	bb_info->phi_arguments_classes = (gssize*) mono_mempool_alloc (area->mempool, sizeof (gssize) * (bb->in_count));
	
	bb_info->phi_insertion_point = NULL;
	
	current_inst = bb->code;
	previous_inst = NULL;
	while (current_inst != NULL) {
		if (is_phi_definition (current_inst)) {
			bb_info->phi_insertion_point = current_inst;
		}
		current_occurrence = process_inst (area, current_inst, previous_inst, bb_info, current_occurrence);
		previous_inst = current_inst;
		current_inst = current_inst->next;
	}
	
	descendants = 0;
	for (dominated_bb = g_list_first (bb->dominated); dominated_bb != NULL; dominated_bb = g_list_next (dominated_bb)) {
		current_occurrence = process_bb (area, (MonoBasicBlock*) (dominated_bb->data), dt_dfn, &descendants, current_occurrence);
	}
	bb_info->dt_descendants = descendants;
	*upper_descendants += (descendants + 1);
	
	return current_occurrence;
}

/*
 * Reset the MonoSsapreBBInfo fields that must be recomputed for each expression.
 */
static void
clean_bb_infos (MonoSsapreWorkArea *area) {
	int i;
	for (i = 0; i < area->num_bblocks; i++) {
		MonoSsapreBBInfo *bb_info = &(area->bb_infos [i]);
		bb_info->has_phi = FALSE;
		bb_info->phi_defines_a_real_occurrence = FALSE;
		bb_info->phi_is_down_safe = FALSE;
		bb_info->phi_can_be_available = FALSE;
		bb_info->phi_is_later = FALSE;
		bb_info->phi_redundancy_class = BOTTOM_REDUNDANCY_CLASS;
		bb_info->phi_variable_index = BOTTOM_REDUNDANCY_CLASS;
		bb_info->has_phi_argument = FALSE;
		bb_info->phi_argument_has_real_use = FALSE;
		bb_info->phi_argument_needs_insert = FALSE;
		bb_info->phi_argument_has_been_processed = FALSE;
		bb_info->phi_argument_class = BOTTOM_REDUNDANCY_CLASS;
		bb_info->phi_argument_variable_index = BOTTOM_REDUNDANCY_CLASS;
		bb_info->phi_argument_defined_by_real_occurrence = NULL;
		bb_info->phi_argument_defined_by_phi = NULL;
		bb_info->phi_argument_left_argument_version = BOTTOM_REDUNDANCY_CLASS;
		bb_info->phi_argument_right_argument_version = BOTTOM_REDUNDANCY_CLASS;
		bb_info->first_expression_in_bb = NULL;
		bb_info->next_interesting_bb = NULL;
		bb_info->next_in_renaming_stack = NULL;
		bb_info->top_of_local_renaming_stack = NULL;
	}
}

/*
 * Reset the MonoSsapreWorkArea fields that must be recomputed for each expression.
 */
static void
clean_area_infos (MonoSsapreWorkArea *area) {
	mono_bitset_clear_all (area->left_argument_bb_bitset);
	mono_bitset_clear_all (area->right_argument_bb_bitset);
	area->num_vars = area->cfg->num_varinfo;
	area->top_of_renaming_stack = NULL;
	area->bb_on_top_of_renaming_stack = NULL;
	area->first_interesting_bb = NULL;
	area->saved_occurrences = 0;
	area->reloaded_occurrences = 0;
	area->inserted_occurrences = 0;
	area->unaltered_occurrences = 0;
	area->added_phis = 0;
	clean_bb_infos (area);
}

/*
 * Utility function to combine the dominance frontiers of a set of BBs.
 */
static void
compute_combined_dfrontier (MonoSsapreWorkArea *area, MonoBitSet* result, MonoBitSet *bblocks) 
{
	int i;
	mono_bitset_clear_all (result);
	mono_bitset_foreach_bit (bblocks, i, area->num_bblocks) {
		mono_bitset_union (result, area->bb_infos_in_cfg_dfn_order [i]->dfrontier);
	}
}

/*
 * Utility function to compute the combined dominance frontier of a set of BBs.
 */
static void
compute_combined_iterated_dfrontier (MonoSsapreWorkArea *area, MonoBitSet *result, MonoBitSet *bblocks, MonoBitSet *buffer) 
{
	gboolean change = TRUE;

	compute_combined_dfrontier (area, result, bblocks);
	do {
		change = FALSE;
		compute_combined_dfrontier (area, buffer, result);
		mono_bitset_union (buffer, result);

		if (!mono_bitset_equal (buffer, result)) {
			mono_bitset_copyto (buffer, result);
			change = TRUE;
		}
	} while (change);
}

/*
 * See paper, figure 18, function Set_var_phis
 */
static void process_phi_variable_in_phi_insertion (MonoSsapreWorkArea *area, gssize variable, MonoBitSet *phi_bbs) {
	int* phi_definition = get_phi_definition (area->cfg, variable);
	
	if (phi_definition != NULL) {
		int definition_bb = area->cfg->vars [variable]->def_bb->dfn;
		if (! mono_bitset_test (phi_bbs, definition_bb)) {
			int i;
			mono_bitset_set (phi_bbs, definition_bb);
			for (i = 0; i < *phi_definition; i++) {
				process_phi_variable_in_phi_insertion (area, phi_definition [i+1], phi_bbs);
			}
		}
	}
}

/*
 * See paper, figure 18, function phi_insertion
 */
static void
phi_insertion (MonoSsapreWorkArea *area, MonoSsapreExpression *expression) {
	MonoSsapreExpressionOccurrence *current_expression = NULL;
	MonoSsapreExpressionOccurrence *previous_expression = NULL;
	MonoSsapreBBInfo *current_bb = NULL;
	MonoSsapreBBInfo *previous_interesting_bb = NULL;
	int i, j, current_bb_dt_dfn;
	
	mono_bitset_clear_all (area->expression_occurrences_buffer);
 	for (current_expression = expression->occurrences; current_expression != NULL; current_expression = current_expression->next) {
		mono_bitset_set (area->expression_occurrences_buffer, current_expression->bb_info->cfg_dfn);
		if (current_expression->description.left_argument.type == MONO_SSAPRE_EXPRESSION_ARGUMENT_SSA_VARIABLE) {
			process_phi_variable_in_phi_insertion (area, current_expression->description.left_argument.argument.ssa_variable, area->left_argument_bb_bitset);
		}
		if (current_expression->description.right_argument.type == MONO_SSAPRE_EXPRESSION_ARGUMENT_SSA_VARIABLE) {
			process_phi_variable_in_phi_insertion (area, current_expression->description.right_argument.argument.ssa_variable, area->right_argument_bb_bitset);
		}
		
		if (previous_expression != NULL) {
			if (previous_expression->bb_info != current_expression->bb_info) {
				previous_expression->is_last_in_bb = TRUE;
				current_expression->is_first_in_bb = TRUE;
				current_expression->bb_info->first_expression_in_bb = current_expression;
			}
		} else {
			current_expression->is_first_in_bb = TRUE;
			current_expression->bb_info->first_expression_in_bb = current_expression;
		}
		
		previous_expression = current_expression;
	}
	previous_expression->is_last_in_bb = TRUE;
	
	compute_combined_iterated_dfrontier (area, area->iterated_dfrontier_buffer, area->expression_occurrences_buffer, area->bb_iteration_buffer);
	
	mono_bitset_union (area->iterated_dfrontier_buffer, area->left_argument_bb_bitset);
	mono_bitset_union (area->iterated_dfrontier_buffer, area->right_argument_bb_bitset);
	
	mono_bitset_foreach_bit (area->iterated_dfrontier_buffer, i, area->num_bblocks) {
		area->bb_infos_in_cfg_dfn_order [i]->has_phi = TRUE;
		area->bb_infos_in_cfg_dfn_order [i]->phi_can_be_available = TRUE;
		for (j = 0; j < area->bb_infos_in_cfg_dfn_order [i]->in_count; j++) {
			area->bb_infos_in_cfg_dfn_order [i]->in_bb [j]->has_phi_argument = TRUE;
		}
	}
	
	for (current_bb = area->bb_infos, current_bb_dt_dfn = 0; current_bb_dt_dfn < area->num_bblocks; current_bb++, current_bb_dt_dfn++) {
		if ((current_bb->first_expression_in_bb != NULL) || (current_bb->has_phi) || (current_bb->has_phi_argument)) {
			if (previous_interesting_bb != NULL) {
				previous_interesting_bb->next_interesting_bb = current_bb;
			} else {
				area->first_interesting_bb = current_bb;
			}
			previous_interesting_bb = current_bb;
		}
	}
}

/*
 * Macro that pushes a real occurrence on the stack
 */
#define PUSH_REAL_OCCURRENCE(o) do{\
			if (area->bb_on_top_of_renaming_stack != (o)->bb_info) { \
				(o)->bb_info->next_in_renaming_stack = area->bb_on_top_of_renaming_stack; \
				area->bb_on_top_of_renaming_stack =(o)->bb_info; \
				(o)->next_in_renaming_stack = NULL; \
			} else { \
				(o)->next_in_renaming_stack = area->top_of_renaming_stack; \
			} \
			(o)->bb_info->top_of_local_renaming_stack = (o); \
			area->top_of_renaming_stack = (o); \
			area->bb_on_top_of_renaming_stack->phi_argument_has_real_use = FALSE; \
		} while(0)

/*
 * Macro that pushes a PHI occurrence on the stack
 */
#define PUSH_PHI_OCCURRENCE(b) do{\
			if (area->bb_on_top_of_renaming_stack != (b)) { \
				(b)->next_in_renaming_stack = area->bb_on_top_of_renaming_stack; \
				area->bb_on_top_of_renaming_stack = (b); \
			} \
			(b)->top_of_local_renaming_stack = NULL; \
			area->top_of_renaming_stack = NULL; \
			area->bb_on_top_of_renaming_stack->phi_argument_has_real_use = FALSE; \
		} while(0)

/*
 * See paper, section 5.1, definition of "Dominate"
 */
static gboolean
dominates (MonoSsapreBBInfo *dominator, MonoSsapreBBInfo *dominated) {
	if ((dominator->dt_dfn <= dominated->dt_dfn) && (dominated->dt_dfn <= (dominator->dt_dfn + dominator->dt_descendants))) {
		return TRUE;
	} else {
		return FALSE;
	}
}

/*
 * See paper, section 5.4.
 * The two passes are coded sequentially (no separate "rename1" and "rename2" functions).
 */
static void
renaming_pass (MonoSsapreWorkArea *area) {
	MonoSsapreBBInfo *current_bb = NULL;
	MonoSsapreBBInfo *previous_bb = NULL;
	MonoSsapreExpressionOccurrence *current_expression = NULL;
	gssize current_class = 0;
	
	/* This loop is "rename1" */
	for (current_bb = area->first_interesting_bb, previous_bb = NULL; current_bb != NULL; previous_bb = current_bb, current_bb = current_bb->next_interesting_bb) {
		if ((previous_bb != NULL) && ! dominates (previous_bb, current_bb)) {
			if ((area->bb_on_top_of_renaming_stack != NULL) && (area->top_of_renaming_stack == NULL) && (previous_bb->phi_argument_has_real_use == FALSE)) {
				if (TRACE_SSAPRE) {
					printf ("Clearing down safe in PHI %d because of backtracking (previous block is [bb %d [ID %d]])\n",
							area->bb_on_top_of_renaming_stack->phi_redundancy_class, previous_bb->cfg_dfn, previous_bb->bb->block_num);
				}
				area->bb_on_top_of_renaming_stack->phi_is_down_safe = FALSE;
			}
			while ((area->bb_on_top_of_renaming_stack != NULL) && ! dominates (area->bb_on_top_of_renaming_stack, current_bb)) {
				MonoSsapreBBInfo *top_bb = area->bb_on_top_of_renaming_stack;
				if (top_bb->next_in_renaming_stack != NULL) {
					area->top_of_renaming_stack = top_bb->next_in_renaming_stack->top_of_local_renaming_stack;
				} else {
					area->top_of_renaming_stack = NULL;
				}
				area->bb_on_top_of_renaming_stack = top_bb->next_in_renaming_stack;
			}
		}
		if (current_bb->idominator != NULL) {
			current_bb->phi_argument_has_real_use = current_bb->idominator->phi_argument_has_real_use;
		} else {
			current_bb->phi_argument_has_real_use = FALSE;
		}
		
		if (current_bb->has_phi) {
			current_bb->phi_is_down_safe = TRUE;
			current_bb->phi_redundancy_class = current_class;
			current_class++;
			PUSH_PHI_OCCURRENCE (current_bb);
		}
		
	 	for (current_expression = current_bb->first_expression_in_bb; (current_expression != NULL) && (current_expression->bb_info == current_bb); current_expression = current_expression->next) {
			if (area->top_of_renaming_stack != NULL) {
				MonoSsapreExpressionOccurrence *top = area->top_of_renaming_stack;
				
				if (((current_expression->description.left_argument.type != MONO_SSAPRE_EXPRESSION_ARGUMENT_SSA_VARIABLE) ||
						(current_expression->description.left_argument.argument.ssa_variable == top->description.left_argument.argument.ssa_variable)) &&
						((current_expression->description.right_argument.type != MONO_SSAPRE_EXPRESSION_ARGUMENT_SSA_VARIABLE) ||
						(current_expression->description.right_argument.argument.ssa_variable == top->description.right_argument.argument.ssa_variable))) {
					current_expression->redundancy_class = top->redundancy_class;
					current_expression->defined_by_real_occurrence = top;
				} else {
					current_expression->redundancy_class = current_class;
					current_class++;
					current_expression->defined_by_real_occurrence = NULL;
					PUSH_REAL_OCCURRENCE (current_expression);
				}
				current_expression->defined_by_phi = NULL;
			} else if (area->bb_on_top_of_renaming_stack != NULL) {
				MonoSsapreBBInfo *phi_bb = area->bb_on_top_of_renaming_stack;
				gssize left_argument_version = ((current_expression->description.left_argument.type == MONO_SSAPRE_EXPRESSION_ARGUMENT_SSA_VARIABLE)?
						(current_expression->description.left_argument.argument.ssa_variable):BOTTOM_REDUNDANCY_CLASS);
				gssize right_argument_version = ((current_expression->description.right_argument.type == MONO_SSAPRE_EXPRESSION_ARGUMENT_SSA_VARIABLE)?
						(current_expression->description.right_argument.argument.ssa_variable):BOTTOM_REDUNDANCY_CLASS);
				
				/* See remark in section 5.4 about the dominance relation between PHI */
				/* occurrences and phi definitions */
				MonoSsapreBBInfo *left_argument_definition_bb = ((left_argument_version != BOTTOM_REDUNDANCY_CLASS)?
						(get_bb_info_of_definition (area, left_argument_version)):NULL);
				MonoSsapreBBInfo *right_argument_definition_bb = ((right_argument_version != BOTTOM_REDUNDANCY_CLASS)?
						(get_bb_info_of_definition (area, right_argument_version)):NULL);
				
				gboolean left_same_class_condition = ((left_argument_definition_bb == NULL) || ((left_argument_definition_bb != phi_bb) ? dominates (left_argument_definition_bb, phi_bb) :
						(IS_PHI_DEFINITION (left_argument_version) ? TRUE : FALSE)));
				gboolean right_same_class_condition = ((right_argument_definition_bb == NULL) || ((right_argument_definition_bb != phi_bb) ? dominates (right_argument_definition_bb, phi_bb) :
						(IS_PHI_DEFINITION (right_argument_version) ? TRUE : FALSE)));
					
				if (left_same_class_condition && right_same_class_condition) {
					int phi_argument;
					
					phi_bb->phi_defines_a_real_occurrence = TRUE;
					current_bb->phi_argument_has_real_use = TRUE;
					current_expression->redundancy_class = phi_bb->phi_redundancy_class;
					current_expression->defined_by_phi = phi_bb;
					
					/*
					 * Major difference from the paper here...
					 * Instead of maintaining "set_for_rename2" (see figure 20), we just
					 * compute "phi_argument_left_argument_version" (and right) here, and
					 * use that in rename2, saving us the hassle of maintaining a set
					 * data structure (and the related allocations).
					 */
					for (phi_argument = 0; phi_argument < phi_bb->in_count; phi_argument++) {
						MonoSsapreBBInfo *previous_bb = phi_bb->in_bb [phi_argument];
						if (left_argument_version != BOTTOM_REDUNDANCY_CLASS) {
							previous_bb->phi_argument_left_argument_version = get_variable_version_at_predecessor_bb (area,
									left_argument_version, phi_bb, phi_argument);
						}
						if (right_argument_version != BOTTOM_REDUNDANCY_CLASS) {
							previous_bb->phi_argument_right_argument_version = get_variable_version_at_predecessor_bb (area,
									right_argument_version, phi_bb, phi_argument);
						}
					}
				} else {
					current_expression->redundancy_class = current_class;
					current_class++;
					current_expression->defined_by_phi = NULL;
					PUSH_REAL_OCCURRENCE (current_expression);
					phi_bb->phi_is_down_safe = FALSE;
					if (TRACE_SSAPRE) {
						printf ("Clearing down safe in PHI %d because of real occurrence %d\n",
								phi_bb->phi_redundancy_class, current_expression->redundancy_class);
					}
				}
				current_expression->defined_by_real_occurrence = NULL;
			} else {
				current_expression->redundancy_class = current_class;
				current_class++;
				current_expression->defined_by_real_occurrence = NULL;
				current_expression->defined_by_phi = NULL;
				PUSH_REAL_OCCURRENCE (current_expression);
			}
		}
		
		if (current_bb->has_phi_argument) {
			if (area->top_of_renaming_stack != NULL) {
				current_bb->phi_argument_defined_by_real_occurrence = area->top_of_renaming_stack;
				current_bb->phi_argument_class = area->top_of_renaming_stack->redundancy_class;
			} else if (area->bb_on_top_of_renaming_stack != NULL) {
				current_bb->phi_argument_defined_by_phi = area->bb_on_top_of_renaming_stack;
				current_bb->phi_argument_class = area->bb_on_top_of_renaming_stack->phi_redundancy_class;
			} else {
				current_bb->phi_argument_class = BOTTOM_REDUNDANCY_CLASS;
			}
		}
	}
	if ((area->bb_on_top_of_renaming_stack != NULL) && (area->top_of_renaming_stack == NULL) && (previous_bb->phi_argument_has_real_use == FALSE)) {
		if (TRACE_SSAPRE) {
			printf ("Clearing down safe in PHI %d because of backtracking (previous block is [bb %d [ID %d]])\n",
					area->bb_on_top_of_renaming_stack->phi_redundancy_class, previous_bb->cfg_dfn, previous_bb->bb->block_num);
		}
		area->bb_on_top_of_renaming_stack->phi_is_down_safe = FALSE;
	}
	area->number_of_classes = current_class;
	
	/* This loop is "rename2" */
	for (current_bb = area->first_interesting_bb; current_bb != NULL; current_bb = current_bb->next_interesting_bb) {
		if (current_bb->has_phi_argument) {
			if ((current_bb->phi_argument_left_argument_version != BOTTOM_REDUNDANCY_CLASS) || (current_bb->phi_argument_right_argument_version != BOTTOM_REDUNDANCY_CLASS)) {
				if (current_bb->phi_argument_defined_by_real_occurrence != NULL) {
					if (((current_bb->phi_argument_left_argument_version != BOTTOM_REDUNDANCY_CLASS) &&
							(current_bb->phi_argument_left_argument_version != current_bb->phi_argument_defined_by_real_occurrence->description.left_argument.argument.ssa_variable)) ||
							((current_bb->phi_argument_right_argument_version != BOTTOM_REDUNDANCY_CLASS) &&
							(current_bb->phi_argument_right_argument_version != current_bb->phi_argument_defined_by_real_occurrence->description.right_argument.argument.ssa_variable))) {
						current_bb->phi_argument_class = BOTTOM_REDUNDANCY_CLASS;
					}
				} else if (current_bb->phi_argument_defined_by_phi != NULL) {
					MonoSsapreBBInfo *phi_bb = current_bb->phi_argument_defined_by_phi;
					MonoSsapreBBInfo *left_argument_definition_bb = (current_bb->phi_argument_left_argument_version != BOTTOM_REDUNDANCY_CLASS) ?
							get_bb_info_of_definition (area, current_bb->phi_argument_left_argument_version) : NULL;
					MonoSsapreBBInfo *right_argument_definition_bb = (current_bb->phi_argument_right_argument_version != BOTTOM_REDUNDANCY_CLASS) ?
							get_bb_info_of_definition (area, current_bb->phi_argument_right_argument_version) : NULL;
					/* See remark in section 5.4 about the dominance relation between PHI */
					/* occurrences and phi definitions */
					gboolean left_bottom_condition = ((left_argument_definition_bb != NULL) && ! ((left_argument_definition_bb != phi_bb) ? dominates (left_argument_definition_bb, phi_bb) :
							(IS_PHI_DEFINITION (current_bb->phi_argument_left_argument_version) ? TRUE : FALSE)));
					gboolean right_bottom_condition = ((right_argument_definition_bb != NULL) && ! ((right_argument_definition_bb != phi_bb) ? dominates (right_argument_definition_bb, phi_bb) :
							(IS_PHI_DEFINITION (current_bb->phi_argument_right_argument_version) ? TRUE : FALSE)));
					
					if (left_bottom_condition || right_bottom_condition) {
						current_bb->phi_argument_class = BOTTOM_REDUNDANCY_CLASS;
					}
				} else {
					current_bb->phi_argument_class = BOTTOM_REDUNDANCY_CLASS;
				}
			} else {
				current_bb->phi_argument_class = BOTTOM_REDUNDANCY_CLASS;
			}
			
			if (current_bb->phi_argument_class == BOTTOM_REDUNDANCY_CLASS) {
				if ((current_bb->phi_argument_defined_by_phi != NULL) && (! current_bb->phi_argument_has_real_use)) {
					if (TRACE_SSAPRE) {
						printf ("Clearing down safe in PHI %d because PHI argument in block [bb %d [ID %d]] is BOTTOM\n",
								current_bb->phi_argument_defined_by_phi->phi_redundancy_class, current_bb->cfg_dfn, current_bb->bb->block_num);
					}
					current_bb->phi_argument_defined_by_phi->phi_is_down_safe = FALSE;
				}
				current_bb->phi_argument_has_real_use = FALSE;
			}
		}
	}
	
	/* Final quick pass to copy the result of "rename2" into PHI nodes */
	for (current_bb = area->first_interesting_bb; current_bb != NULL; current_bb = current_bb->next_interesting_bb) {
		if (current_bb->has_phi) {
			int i;
			for (i = 0; i < current_bb->in_count; i++) {
				current_bb->phi_arguments_classes [i] = current_bb->in_bb [i]->phi_argument_class;
			}
		}
	}
}

#undef PUSH_REAL_OCCURRENCE
#undef PUSH_PHI_OCCURRENCE

/*
 * See paper, figure 8
 */
static void
reset_down_safe (MonoSsapreBBInfo *phi_argument) {
	if ((phi_argument->phi_argument_class != BOTTOM_REDUNDANCY_CLASS) && (! phi_argument->phi_argument_has_real_use) && (phi_argument->phi_argument_defined_by_phi != NULL) && (phi_argument->phi_argument_defined_by_phi->phi_is_down_safe)) {
		int i;
		MonoSsapreBBInfo *phi = phi_argument->phi_argument_defined_by_phi;
//		if (TRACE_SSAPRE) {
//			printf ("Clearing down safe in PHI %d inside reset_down_safe\n", phi->phi_redundancy_class);
//		}
		phi->phi_is_down_safe = FALSE;
		for (i = 0; i < phi->in_count; i++) {
			reset_down_safe (phi->in_bb [i]);
		}
	}
}

/*
 * See paper, figure 8
 */
static void
down_safety (MonoSsapreWorkArea *area) {
	MonoSsapreBBInfo *current_bb = NULL;
	
	for (current_bb = area->first_interesting_bb; current_bb != NULL; current_bb = current_bb->next_interesting_bb) {
		if (current_bb->has_phi) {
			if (! current_bb->phi_is_down_safe) {
				int i;
				for (i = 0; i < current_bb->in_count; i++) {
					reset_down_safe (current_bb->in_bb [i]);
				}
			}
		}
	}
}

/*
 * See paper, figure 10
 */
static void
reset_can_be_available (MonoSsapreWorkArea *area, MonoSsapreBBInfo *phi) {
	MonoSsapreBBInfo *current_bb = NULL;
	
	phi->phi_can_be_available = FALSE;
	for (current_bb = area->first_interesting_bb; current_bb != NULL; current_bb = current_bb->next_interesting_bb) {
		if (current_bb->has_phi) {
			gboolean phi_is_interesting = FALSE;
			int i;
			
			for (i = 0; i < current_bb->in_count; i++) {
				MonoSsapreBBInfo *phi_argument = current_bb->in_bb [i];
				if ((phi_argument->phi_argument_class == current_bb->phi_redundancy_class) && (! phi_argument->phi_argument_has_real_use)) {
					phi_is_interesting = TRUE;
					break;
				}
			}
			
			if (phi_is_interesting && (! current_bb->phi_is_down_safe) && (current_bb->phi_can_be_available)) {
				reset_can_be_available (area, current_bb);
			}
		}
	}
}

/*
 * See paper, figure 10
 */
static void
compute_can_be_available (MonoSsapreWorkArea *area) {
	MonoSsapreBBInfo *current_bb = NULL;
	
	for (current_bb = area->first_interesting_bb; current_bb != NULL; current_bb = current_bb->next_interesting_bb) {
		if (current_bb->has_phi) {
			if ((! current_bb->phi_is_down_safe) && (current_bb->phi_can_be_available)) {
				gboolean phi_is_interesting = FALSE;
				int i;
				
				for (i = 0; i < current_bb->in_count; i++) {
					if (current_bb->phi_arguments_classes [i] == BOTTOM_REDUNDANCY_CLASS) {
						phi_is_interesting = TRUE;
						break;
					}
				}
				
				if (phi_is_interesting) {
					reset_can_be_available (area, current_bb);
				}
			}
		}
	}
}

/*
 * See paper, figure 10
 */
static void
reset_later (MonoSsapreWorkArea *area, MonoSsapreBBInfo *phi) {
	MonoSsapreBBInfo *current_bb = NULL;
	
	if (phi->phi_is_later) {
		phi->phi_is_later = FALSE;
		for (current_bb = area->first_interesting_bb; current_bb != NULL; current_bb = current_bb->next_interesting_bb) {
			if (current_bb->has_phi) {
				gboolean phi_is_interesting = FALSE;
				int i;
				
				for (i = 0; i < current_bb->in_count; i++) {
					MonoSsapreBBInfo *phi_argument = current_bb->in_bb [i];
					if (phi_argument->phi_argument_class == current_bb->phi_redundancy_class) {
						phi_is_interesting = TRUE;
						break;
					}
				}
				
				if (phi_is_interesting) {
					reset_later (area, current_bb);
				}
			}
		}
	}
}

/*
 * See paper, figure 10
 */
static void
compute_later (MonoSsapreWorkArea *area) {
	MonoSsapreBBInfo *current_bb = NULL;
	
	for (current_bb = area->first_interesting_bb; current_bb != NULL; current_bb = current_bb->next_interesting_bb) {
		if (current_bb->has_phi) {
			current_bb->phi_is_later = current_bb->phi_can_be_available;
		}
	}
	for (current_bb = area->first_interesting_bb; current_bb != NULL; current_bb = current_bb->next_interesting_bb) {
		if (current_bb->has_phi) {
			if (current_bb->phi_is_later) {
				gboolean phi_is_interesting = FALSE;
				int i;
				
				for (i = 0; i < current_bb->in_count; i++) {
					MonoSsapreBBInfo *phi_argument = current_bb->in_bb [i];
					if ((phi_argument->phi_argument_class != BOTTOM_REDUNDANCY_CLASS) && (phi_argument->phi_argument_has_real_use)) {
						phi_is_interesting = TRUE;
						break;
					}
				}
				
				if (phi_is_interesting) {
					reset_later (area, current_bb);
				}
			}
		}
	}
}

/*
 * See paper, figure 12, function Finalize_1
 */
static void finalize_availability_and_reload (MonoSsapreWorkArea *area, MonoSsapreAvailabilityTableElement *availability_table) {
	MonoSsapreBBInfo *current_bb = NULL;
	MonoSsapreExpressionOccurrence *current_expression = NULL;
	
	for (current_bb = area->first_interesting_bb; current_bb != NULL; current_bb = current_bb->next_interesting_bb) {
		if (current_bb->has_phi) {
			if (current_bb->phi_can_be_available && ! current_bb->phi_is_later) {
				availability_table [current_bb->phi_redundancy_class].class_defined_by_phi = current_bb;
			}
		}
		
	 	for (current_expression = current_bb->first_expression_in_bb; (current_expression != NULL) && (current_expression->bb_info == current_bb); current_expression = current_expression->next) {
	 		MonoSsapreBBInfo *defining_bb = availability_table [current_expression->redundancy_class].class_defined_by_phi;
	 		if (defining_bb == NULL && availability_table [current_expression->redundancy_class].class_defined_by_real_occurrence != NULL) {
	 			defining_bb = availability_table [current_expression->redundancy_class].class_defined_by_real_occurrence->bb_info;
	 		}
	 		if (defining_bb != NULL) {
	 			if (! dominates (defining_bb, current_bb)) {
	 				defining_bb = NULL;
	 			}
	 		}
	 		
	 		if (defining_bb == NULL) {
	 			current_expression->reload = FALSE;
	 			availability_table [current_expression->redundancy_class].class_defined_by_real_occurrence = current_expression;
	 		} else {
	 			current_expression->reload = TRUE;
	 			current_expression->defined_by_phi = availability_table [current_expression->redundancy_class].class_defined_by_phi;
	 			current_expression->defined_by_real_occurrence = availability_table [current_expression->redundancy_class].class_defined_by_real_occurrence;
	 		}
	 		
	 		current_expression->save = FALSE;
	 	}
		
		if (current_bb->has_phi_argument) {
			MonoSsapreBBInfo *phi_bb = current_bb->out_bb [0];
			if (((phi_bb->phi_can_be_available) && (! phi_bb->phi_is_later)) &&
					((current_bb->phi_argument_class == BOTTOM_REDUNDANCY_CLASS) ||
					((current_bb->phi_argument_has_real_use == FALSE) && (current_bb->phi_argument_defined_by_phi != NULL) &&
						(! ((current_bb->phi_argument_defined_by_phi->phi_can_be_available) && (! current_bb->phi_argument_defined_by_phi->phi_is_later)))
					))) {
				current_bb->phi_argument_needs_insert = TRUE;
				current_bb->phi_argument_defined_by_real_occurrence = NULL;
				current_bb->phi_argument_defined_by_phi = NULL;
			} else {
				current_bb->phi_argument_needs_insert = FALSE;
				if (current_bb->phi_argument_class != BOTTOM_REDUNDANCY_CLASS) {
					current_bb->phi_argument_defined_by_real_occurrence = availability_table [current_bb->phi_argument_class].class_defined_by_real_occurrence;
					current_bb->phi_argument_defined_by_phi = availability_table [current_bb->phi_argument_class].class_defined_by_phi;
				}
			}
			
			current_bb->phi_argument_has_been_processed = FALSE;
		}
	}
}

/*
 * See paper, figure 13, function Set_save
 */
static void set_save (MonoSsapreBBInfo *phi_occurrance, MonoSsapreExpressionOccurrence *real_occurrance) {
	if (real_occurrance != NULL) {
		real_occurrance->save = TRUE;
	} else if (phi_occurrance != NULL) {
		int i;
		for (i = 0; i < phi_occurrance->in_count; i++) {
			MonoSsapreBBInfo *phi_argument = phi_occurrance->in_bb [i];
			if (! phi_argument->phi_argument_has_been_processed) {
				phi_argument->phi_argument_has_been_processed = TRUE;
				set_save (phi_argument->phi_argument_defined_by_phi, phi_argument->phi_argument_defined_by_real_occurrence);
			}
		}
	}
}

/*
 * See paper, figure 13, function Finalize_2 (note that we still don't do
 * extraneous PHI elimination, see function Set_replacement)
 */
static void finalize_save (MonoSsapreWorkArea *area) {
	MonoSsapreBBInfo *current_bb = NULL;
	MonoSsapreExpressionOccurrence *current_expression = NULL;
	
	for (current_bb = area->first_interesting_bb; current_bb != NULL; current_bb = current_bb->next_interesting_bb) {
	 	for (current_expression = current_bb->first_expression_in_bb; (current_expression != NULL) && (current_expression->bb_info == current_bb); current_expression = current_expression->next) {
	 		if (current_expression->reload) {
	 			set_save (current_expression->defined_by_phi, current_expression->defined_by_real_occurrence);
	 		}
	 	}
	 	
	 	if ((current_bb->has_phi_argument) &&
	 			(! current_bb->phi_argument_needs_insert) &&
	 			(current_bb->phi_argument_class != BOTTOM_REDUNDANCY_CLASS) &&
	 			(current_bb->phi_argument_defined_by_real_occurrence != NULL)) {
	 		current_bb->phi_argument_defined_by_real_occurrence->save = TRUE;
	 	}
	}
}

/*
 * Perform all "finalize" steps
 */
static void finalize (MonoSsapreWorkArea *area) {
	MonoSsapreAvailabilityTableElement *availability_table = alloca (sizeof (MonoSsapreAvailabilityTableElement) * (area->number_of_classes));
	int i;
	
	for (i = 0; i < area->number_of_classes; i++) {
		availability_table[i].class_defined_by_phi = NULL;
		availability_table[i].class_defined_by_real_occurrence = NULL;
	}
	
	finalize_availability_and_reload (area, availability_table);
	finalize_save (area);
}

/*
 * Macros that help in creating constants
 */
#define NEW_INST(dest,op) do {	\
		(dest) = mono_mempool_alloc0 (area->cfg->mempool, sizeof (MonoInst));	\
		(dest)->opcode = (op);	\
	} while (0)

#define NEW_I4(dest,val) do {	\
		NEW_INST ((dest), OP_ICONST); \
		(dest)->inst_c0 = (val);	\
		(dest)->type = STACK_I4;	\
	} while (0)

#define NEW_I8(dest,val) do {	\
		NEW_INST ((dest), OP_I8CONST); \
		(dest)->inst_l = (val);	\
		(dest)->type = STACK_I8;	\
	} while (0)

#define NEW_R4(dest,f) do {	\
		NEW_INST ((dest), OP_R4CONST); \
		(dest)->inst_p0 = f;	\
		(dest)->type = STACK_R8;	\
	} while (0)

#define NEW_R8(dest,d) do {	\
		NEW_INST ((dest), OP_R8CONST); \
		(dest)->inst_p0 = d;	\
		(dest)->type = STACK_R8;	\
	} while (0)

/*
 * Create a MonoInst that represents an expression argument
 */
static MonoInst*
create_expression_argument (MonoSsapreWorkArea *area, MonoSsapreExpressionArgument *argument) {
	MonoInst *result;
	
	switch (argument->type) {
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_NOT_PRESENT:
			return NULL;
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_SSA_VARIABLE:
			return mono_compile_create_var_load (area->cfg, argument->argument.ssa_variable);
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_INTEGER_CONSTANT:
			NEW_I4 (result, argument->argument.integer_constant);
			return result;
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_LONG_COSTANT:
			NEW_I8 (result, *(argument->argument.long_constant));
			return result;
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_FLOAT_COSTANT:
			NEW_R4 (result, argument->argument.float_constant);
			return result;
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_DOUBLE_COSTANT:
			NEW_R8 (result, argument->argument.double_constant);
			return result;
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_ORIGINAL_VARIABLE:
		case MONO_SSAPRE_EXPRESSION_ARGUMENT_ANY:
		default:
			g_assert_not_reached ();
			return NULL;
	}
}

/*
 * Create a MonoInst that represents an expression
 */
static MonoInst*
create_expression (MonoSsapreWorkArea *area, MonoSsapreExpressionDescription *expression) {
	MonoInst *result;
	NEW_INST (result, expression->opcode);
	result->inst_left = create_expression_argument (area, &(expression->left_argument));
	result->inst_right = create_expression_argument (area, &(expression->right_argument));
	return result;
}

/*
 * See paper, section 3.6
 */
static void code_motion (MonoSsapreWorkArea *area) {
	MonoSsapreBBInfo *current_bb = NULL;
	MonoSsapreExpressionOccurrence *current_expression = NULL;
	
	for (current_bb = area->first_interesting_bb; current_bb != NULL; current_bb = current_bb->next_interesting_bb) {	
		if ((current_bb->has_phi) && (current_bb->phi_can_be_available && ! current_bb->phi_is_later)) {
			MonoInst *new_var = mono_compile_create_var (area->cfg, area->current_expression->type, OP_LOCAL);
			current_bb->phi_variable_index = new_var->inst_c0;
		} else {
			current_bb->phi_variable_index = BOTTOM_REDUNDANCY_CLASS;
		}
		
	 	for (current_expression = current_bb->first_expression_in_bb; (current_expression != NULL) && (current_expression->bb_info == current_bb); current_expression = current_expression->next) {
	 		if (current_expression->save) {
				MonoInst *new_var = mono_compile_create_var (area->cfg, area->current_expression->type, OP_LOCAL);
				current_expression->variable_index = new_var->inst_c0;
	 		} else {
				current_expression->variable_index = BOTTOM_REDUNDANCY_CLASS;
	 		}
	 	}
	 	
		if ((current_bb->has_phi_argument) && (current_bb->phi_argument_needs_insert)) {
			MonoInst *new_var = mono_compile_create_var (area->cfg, area->current_expression->type, OP_LOCAL);
			current_bb->phi_argument_variable_index = new_var->inst_c0;
		} else {
			current_bb->phi_argument_variable_index = BOTTOM_REDUNDANCY_CLASS;
		}
	}
	
	for (current_bb = area->first_interesting_bb; current_bb != NULL; current_bb = current_bb->next_interesting_bb) {
		if (current_bb->phi_variable_index != BOTTOM_REDUNDANCY_CLASS) {
			MonoInst *phi;
			MonoInst *store;
			int in_bb;
			
			NEW_INST (phi, OP_PHI);
			phi->inst_phi_args = mono_mempool_alloc (area->cfg->mempool, (sizeof (int) * ((current_bb->in_count) + 1)));
			phi->inst_phi_args [0] = current_bb->in_count;
			for (in_bb = 0; in_bb < current_bb->in_count; in_bb++) {
				gssize phi_argument_variable_index = current_bb->in_bb [in_bb]->phi_argument_variable_index;
				if (phi_argument_variable_index == BOTTOM_REDUNDANCY_CLASS) {
					MonoSsapreBBInfo *phi_argument_bb = current_bb->in_bb [in_bb];
					if (phi_argument_bb->phi_argument_defined_by_phi !=NULL) {
						phi_argument_variable_index = phi_argument_bb->phi_argument_defined_by_phi->phi_variable_index;
					} else if (phi_argument_bb->phi_argument_defined_by_real_occurrence !=NULL) {
						phi_argument_variable_index = phi_argument_bb->phi_argument_defined_by_real_occurrence->variable_index;
					} else {
						g_assert_not_reached ();
					}
				}
				phi->inst_phi_args [in_bb + 1] = phi_argument_variable_index;
			}
			store = mono_compile_create_var_store (area->cfg, current_bb->phi_variable_index, phi);
			if (current_bb->phi_insertion_point != NULL) {
				store->next = current_bb->phi_insertion_point->next;
				current_bb->phi_insertion_point->next = store;
			} else {
				store->next = current_bb->bb->code;
				current_bb->bb->code = store;
			}
			current_bb->phi_insertion_point = store;
			
			area->added_phis ++;
		}
		
	 	for (current_expression = current_bb->first_expression_in_bb; (current_expression != NULL) && (current_expression->bb_info == current_bb); current_expression = current_expression->next) {
	 		gboolean altered = FALSE;
	 		if (current_expression->reload) {
	 			gssize variable_index;
	 			if (current_expression->defined_by_phi != NULL) {
	 				variable_index = current_expression->defined_by_phi->phi_variable_index;
	 			} else if (current_expression->defined_by_real_occurrence != NULL) {
	 				variable_index = current_expression->defined_by_real_occurrence->variable_index;
	 			} else {
	 				variable_index = BOTTOM_REDUNDANCY_CLASS;
	 				g_assert_not_reached ();
	 			}
	 			mono_compile_make_var_load (area->cfg, current_expression->occurrence, variable_index);
	 			
	 			area->reloaded_occurrences ++;
	 			altered = TRUE;
	 		}
	 		if (current_expression->save) {
	 			MonoInst *store;
	 			MonoInst *moved_expression = mono_mempool_alloc (area->cfg->mempool, sizeof (MonoInst));
	 			*moved_expression = *(current_expression->occurrence);
	 			store = mono_compile_create_var_store (area->cfg, current_expression->variable_index, moved_expression);
	 			if (current_expression->previous_tree != NULL) {
		 			store->next = current_expression->previous_tree->next;
		 			current_expression->previous_tree->next = store;
	 			} else {
		 			store->next = current_bb->bb->code;
		 			current_bb->bb->code = store;
	 			}
	 			mono_compile_make_var_load (area->cfg, current_expression->occurrence, current_expression->variable_index);
	 			
	 			area->saved_occurrences ++;
	 			altered = TRUE;
	 		}
	 		if (! altered) {
	 			area->unaltered_occurrences ++;
	 		}
	 	}
	 	
	 	if (current_bb->phi_argument_variable_index != BOTTOM_REDUNDANCY_CLASS) {
	 		MonoSsapreExpressionDescription expression_description;
 			MonoInst *inserted_expression;
 			MonoInst *store;
	 		
	 		expression_description = area->current_expression->description;
	 		if (expression_description.left_argument.type == MONO_SSAPRE_EXPRESSION_ARGUMENT_ORIGINAL_VARIABLE) {
	 			expression_description.left_argument.argument.ssa_variable = current_bb->phi_argument_left_argument_version;
	 			expression_description.left_argument.type = MONO_SSAPRE_EXPRESSION_ARGUMENT_SSA_VARIABLE;
	 		}
	 		if (expression_description.right_argument.type == MONO_SSAPRE_EXPRESSION_ARGUMENT_ORIGINAL_VARIABLE) {
	 			expression_description.right_argument.argument.ssa_variable = current_bb->phi_argument_right_argument_version;
	 			expression_description.right_argument.type = MONO_SSAPRE_EXPRESSION_ARGUMENT_SSA_VARIABLE;
	 		}
	 		
	 		inserted_expression = create_expression (area, &expression_description);
	 		store = mono_compile_create_var_store (area->cfg, current_bb->phi_argument_variable_index, inserted_expression);
	 		store->next = NULL;
	 		mono_add_ins_to_end (current_bb->bb, store);
	 		
	 		area->inserted_occurrences ++;
	 	}
	}
}

/*
 * Perform all SSAPRE steps for an expression
 */
static void
process_expression (MonoSsapreWorkArea *area, MonoSsapreExpression *expression) {
	if (area->cfg->verbose_level >= TRACE_LEVEL) {
		printf ("SSAPRE STARTS PROCESSING EXPRESSION ");
		print_expression_description (&(expression->description));
		printf ("\n");
	}
	
	clean_area_infos (area);
	
	area->current_expression = expression;
	phi_insertion (area, expression);
	renaming_pass (area);
	
	if (area->cfg->verbose_level >= TRACE_LEVEL) {
		printf ("START SUMMARY OF BB INFOS\n");
		print_bb_infos (area);
		printf ("END SUMMARY OF BB INFOS\n");
	}
	
	down_safety (area);
	compute_can_be_available (area);
	compute_later (area);
	finalize (area);
	code_motion (area);
	
	if (area->cfg->verbose_level >= DUMP_LEVEL) {
		printf ("START DUMP OF BB INFOS\n");
		dump_code (area);
		printf ("END DUMP OF BB INFOS\n");
	} else if (area->cfg->verbose_level >= TRACE_LEVEL) {
		printf ("START SUMMARY OF BB INFOS\n");
		print_interesting_bb_infos (area);
		printf ("END SUMMARY OF BB INFOS\n");
	}
	
	if (area->cfg->verbose_level >= STATISTICS_LEVEL) {
		printf ("STATISTICS: saved_occurrences = %d, reloaded_occurrences = %d, inserted_occurrences = %d, unaltered_occurrences = %d, added_phis = %d\n",
			area->saved_occurrences, area->reloaded_occurrences, area->inserted_occurrences, area->unaltered_occurrences, area->added_phis);
	}
	
	if (area->cfg->verbose_level >= TRACE_LEVEL) {
		printf ("SSAPRE ENDS PROCESSING EXPRESSION ");
		print_expression_description (&(expression->description));
		printf ("\n");
	}
}

/*
 * Perform all SSAPRE steps for all the expressions in the worklist
 */
static void
process_worklist (MonoSsapreWorkArea *area, MonoSsapreExpression *expression) {
	if (expression != NULL) {
		process_worklist (area, expression->previous);
		process_expression (area, expression);
		process_worklist (area, expression->next);
	}
}

/*
 * Hack to apply SSAPRE only to a given method (invaluable in debugging)
 */
#define APPLY_SSAPRE_TO_SINGLE_METHOD 0
#if (APPLY_SSAPRE_TO_SINGLE_METHOD)
static char *mono_ssapre_method_name = NULL;
static gboolean check_ssapre_method_name (MonoCompile *cfg) {
	if (mono_ssapre_method_name == NULL) {
		mono_ssapre_method_name = getenv ("MONO_SSAPRE_METHOD_NAME");
	}
	if (mono_ssapre_method_name != NULL) {
		char *method_name = mono_method_full_name (cfg->method, TRUE);
		if (strstr (mono_ssapre_method_name, method_name) != NULL) {
			return TRUE;
		} else {
			return FALSE;
		}
	} else {
		return TRUE;
	}
}
#endif

/*
 * Apply SSAPRE to a MonoCompile
 */
void
mono_perform_ssapre (MonoCompile *cfg) {
	MonoSsapreWorkArea area;
	MonoSsapreExpressionOccurrence *current_occurrence;
	int dt_dfn, descendants, block, i;
	
#if (APPLY_SSAPRE_TO_SINGLE_METHOD)
	if (! check_ssapre_method_name (cfg)) return;
#endif
	
	area.cfg = cfg;
	area.mempool = mono_mempool_new ();
	
	area.num_bblocks = cfg->num_bblocks;
	area.bb_infos = (MonoSsapreBBInfo*) mono_mempool_alloc (area.mempool, sizeof (MonoSsapreBBInfo) * (cfg->num_bblocks));
	area.bb_infos_in_cfg_dfn_order = (MonoSsapreBBInfo**) mono_mempool_alloc (area.mempool, sizeof (MonoSsapreBBInfo*) * (cfg->num_bblocks));
	
	area.sizeof_bb_bitset = mono_bitset_alloc_size (cfg->num_bblocks, 0);
	area.expression_occurrences_buffer = mono_bitset_mem_new (mono_mempool_alloc (area.mempool, area.sizeof_bb_bitset), area.num_bblocks, 0);
	area.bb_iteration_buffer = mono_bitset_mem_new (mono_mempool_alloc (area.mempool, area.sizeof_bb_bitset), area.num_bblocks, 0);
	area.iterated_dfrontier_buffer = mono_bitset_mem_new (mono_mempool_alloc (area.mempool, area.sizeof_bb_bitset), area.num_bblocks, 0);
	area.left_argument_bb_bitset = mono_bitset_mem_new (mono_mempool_alloc (area.mempool, area.sizeof_bb_bitset), area.num_bblocks, 0);
	area.right_argument_bb_bitset = mono_bitset_mem_new (mono_mempool_alloc (area.mempool, area.sizeof_bb_bitset), area.num_bblocks, 0);
	
	area.worklist = NULL;
	
	if (area.cfg->verbose_level >= LOG_LEVEL) {
		printf ("SSAPRE STARTS PROCESSING METHOD %s\n", mono_method_full_name (cfg->method, TRUE));
	}
	
	current_occurrence = (MonoSsapreExpressionOccurrence*) mono_mempool_alloc (area.mempool, sizeof (MonoSsapreExpressionOccurrence));
	dt_dfn = 0;
	descendants = 0;
	process_bb (&area, cfg->bblocks [0], &dt_dfn, &descendants, current_occurrence);
	for (block = 0; block < area.num_bblocks; block++) {
		MonoSsapreBBInfo *bb_info = &(area.bb_infos [block]);
		MonoBasicBlock *bb = bb_info->bb;
		if (bb->idom != NULL) {
			bb_info->idominator = area.bb_infos_in_cfg_dfn_order [bb->idom->dfn];
		} else {
			bb_info->idominator = NULL;
		}
		for (i = 0; i < bb->in_count; i++) {
			bb_info->in_bb [i] = area.bb_infos_in_cfg_dfn_order [bb->in_bb [i]->dfn];
		}
		for (i = 0; i < bb->out_count; i++) {
			bb_info->out_bb [i] = area.bb_infos_in_cfg_dfn_order [bb->out_bb [i]->dfn];
		}
	}
	
	if (area.cfg->verbose_level >= TRACE_LEVEL) {
		printf ("SSAPRE START WORKLIST\n");
		print_worklist (area.worklist);
		printf ("SSAPRE END WORKLIST\n");
	}
	
	process_worklist (&area, area.worklist);
	
	if (area.cfg->verbose_level >= TRACE_LEVEL) {
		printf ("SSAPRE ENDS PROCESSING METHOD %s\n", mono_method_full_name (cfg->method, TRUE));
	}
	
	mono_mempool_destroy (area.mempool);
}
