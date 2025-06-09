/*
   +----------------------------------------------------------------------+
   | Crex Engine, DFG - Data Flow Graph                                   |
   +----------------------------------------------------------------------+
   | Copyright (c) The CRX Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the CRX license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.crx.net/license/3_01.txt                                 |
   | If you did not receive a copy of the CRX license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crx.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crex_compile.h"
#include "crex_dfg.h"

static crex_always_inline void _crex_dfg_add_use_def_op(const crex_op_array *op_array, const crex_op *opline, uint32_t build_flags, crex_bitset use, crex_bitset def) /* {{{ */
{
	uint32_t var_num;
	const crex_op *next;

	if (opline->op1_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
		var_num = EX_VAR_TO_NUM(opline->op1.var);
		if (!crex_bitset_in(def, var_num)) {
			crex_bitset_incl(use, var_num);
		}
	}
	if (((opline->op2_type & (IS_VAR|IS_TMP_VAR)) != 0
	  && opline->opcode != CREX_FE_FETCH_R
	  && opline->opcode != CREX_FE_FETCH_RW)
	 || (opline->op2_type == IS_CV)) {
		var_num = EX_VAR_TO_NUM(opline->op2.var);
		if (!crex_bitset_in(def, var_num)) {
			crex_bitset_incl(use, var_num);
		}
	}
	if ((build_flags & CREX_SSA_USE_CV_RESULTS)
	 && opline->result_type == IS_CV
	 && opline->opcode != CREX_RECV) {
		var_num = EX_VAR_TO_NUM(opline->result.var);
		if (!crex_bitset_in(def, var_num)) {
			crex_bitset_incl(use, var_num);
		}
	}

	switch (opline->opcode) {
		case CREX_ASSIGN:
			if ((build_flags & CREX_SSA_RC_INFERENCE) && opline->op2_type == IS_CV) {
				crex_bitset_incl(def, EX_VAR_TO_NUM(opline->op2.var));
			}
			if (opline->op1_type == IS_CV) {
add_op1_def:
				crex_bitset_incl(def, EX_VAR_TO_NUM(opline->op1.var));
			}
			break;
		case CREX_ASSIGN_REF:
			if (opline->op2_type == IS_CV) {
				crex_bitset_incl(def, EX_VAR_TO_NUM(opline->op2.var));
			}
			if (opline->op1_type == IS_CV) {
				goto add_op1_def;
			}
			break;
		case CREX_ASSIGN_DIM:
		case CREX_ASSIGN_OBJ:
			next = opline + 1;
			if (next->op1_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
				var_num = EX_VAR_TO_NUM(next->op1.var);
				if (!crex_bitset_in(def, var_num)) {
					crex_bitset_incl(use, var_num);
				}
				if (build_flags & CREX_SSA_RC_INFERENCE && next->op1_type == IS_CV) {
					crex_bitset_incl(def, var_num);
				}
			}
			if (opline->op1_type == IS_CV) {
				goto add_op1_def;
			}
			break;
		case CREX_ASSIGN_OBJ_REF:
			next = opline + 1;
			if (next->op1_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
				var_num = EX_VAR_TO_NUM(next->op1.var);
				if (!crex_bitset_in(def, var_num)) {
					crex_bitset_incl(use, var_num);
				}
				if (next->op1_type == IS_CV) {
					crex_bitset_incl(def, var_num);
				}
			}
			if (opline->op1_type == IS_CV) {
				goto add_op1_def;
			}
			break;
		case CREX_ASSIGN_STATIC_PROP:
			next = opline + 1;
			if (next->op1_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
				var_num = EX_VAR_TO_NUM(next->op1.var);
				if (!crex_bitset_in(def, var_num)) {
					crex_bitset_incl(use, var_num);
				}
				if ((build_flags & CREX_SSA_RC_INFERENCE) && next->op1_type == IS_CV) {
					crex_bitset_incl(def, var_num);
				}
			}
			break;
		case CREX_ASSIGN_STATIC_PROP_REF:
			next = opline + 1;
			if (next->op1_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
				var_num = EX_VAR_TO_NUM(next->op1.var);
				if (!crex_bitset_in(def, var_num)) {
					crex_bitset_incl(use, var_num);
				}
				if (next->op1_type == IS_CV) {
					crex_bitset_incl(def, var_num);
				}
			}
			break;
		case CREX_ASSIGN_STATIC_PROP_OP:
			next = opline + 1;
			if (next->op1_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
				var_num = EX_VAR_TO_NUM(next->op1.var);
				if (!crex_bitset_in(def, var_num)) {
					crex_bitset_incl(use, var_num);
				}
			}
			break;
		case CREX_ASSIGN_DIM_OP:
		case CREX_ASSIGN_OBJ_OP:
			next = opline + 1;
			if (next->op1_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
				var_num = EX_VAR_TO_NUM(next->op1.var);
				if (!crex_bitset_in(def, var_num)) {
					crex_bitset_incl(use, var_num);
				}
			}
			if (opline->op1_type == IS_CV) {
				goto add_op1_def;
			}
			break;
		case CREX_ASSIGN_OP:
		case CREX_PRE_INC:
		case CREX_PRE_DEC:
		case CREX_POST_INC:
		case CREX_POST_DEC:
		case CREX_BIND_GLOBAL:
		case CREX_BIND_STATIC:
		case CREX_BIND_INIT_STATIC_OR_JMP:
		case CREX_SEND_VAR_NO_REF:
		case CREX_SEND_VAR_NO_REF_EX:
		case CREX_SEND_VAR_EX:
		case CREX_SEND_FUNC_ARG:
		case CREX_SEND_REF:
		case CREX_SEND_UNPACK:
		case CREX_FE_RESET_RW:
		case CREX_MAKE_REF:
		case CREX_PRE_INC_OBJ:
		case CREX_PRE_DEC_OBJ:
		case CREX_POST_INC_OBJ:
		case CREX_POST_DEC_OBJ:
		case CREX_UNSET_DIM:
		case CREX_UNSET_OBJ:
		case CREX_FETCH_DIM_W:
		case CREX_FETCH_DIM_RW:
		case CREX_FETCH_DIM_FUNC_ARG:
		case CREX_FETCH_DIM_UNSET:
		case CREX_FETCH_LIST_W:
			if (opline->op1_type == IS_CV) {
				goto add_op1_def;
			}
			break;
		case CREX_SEND_VAR:
		case CREX_CAST:
		case CREX_QM_ASSIGN:
		case CREX_JMP_SET:
		case CREX_COALESCE:
		case CREX_FE_RESET_R:
			if ((build_flags & CREX_SSA_RC_INFERENCE) && opline->op1_type == IS_CV) {
				goto add_op1_def;
			}
			break;
		case CREX_ADD_ARRAY_UNPACK:
			var_num = EX_VAR_TO_NUM(opline->result.var);
			if (!crex_bitset_in(def, var_num)) {
				crex_bitset_incl(use, var_num);
			}
			break;
		case CREX_ADD_ARRAY_ELEMENT:
			var_num = EX_VAR_TO_NUM(opline->result.var);
			if (!crex_bitset_in(def, var_num)) {
				crex_bitset_incl(use, var_num);
			}
			CREX_FALLTHROUGH;
		case CREX_INIT_ARRAY:
			if (((build_flags & CREX_SSA_RC_INFERENCE)
						|| (opline->extended_value & CREX_ARRAY_ELEMENT_REF))
					&& opline->op1_type == IS_CV) {
				goto add_op1_def;
			}
			break;
		case CREX_YIELD:
			if (opline->op1_type == IS_CV
					&& ((op_array->fn_flags & CREX_ACC_RETURN_REFERENCE)
						|| (build_flags & CREX_SSA_RC_INFERENCE))) {
				goto add_op1_def;
			}
			break;
		case CREX_UNSET_CV:
			goto add_op1_def;
		case CREX_VERIFY_RETURN_TYPE:
			if (opline->op1_type & (IS_TMP_VAR|IS_VAR|IS_CV)) {
				goto add_op1_def;
			}
			break;
		case CREX_FE_FETCH_R:
		case CREX_FE_FETCH_RW:
#if 0
			/* This special case was handled above the switch */
			if (opline->op2_type != IS_CV) {
				op2_use = -1; /* not used */
			}
#endif
			crex_bitset_incl(def, EX_VAR_TO_NUM(opline->op2.var));
			break;
		case CREX_BIND_LEXICAL:
			if ((opline->extended_value & CREX_BIND_REF) || (build_flags & CREX_SSA_RC_INFERENCE)) {
				crex_bitset_incl(def, EX_VAR_TO_NUM(opline->op2.var));
			}
			break;
		default:
			break;
	}

	if (opline->result_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
		crex_bitset_incl(def, EX_VAR_TO_NUM(opline->result.var));
	}
}
/* }}} */

CREX_API void crex_dfg_add_use_def_op(const crex_op_array *op_array, const crex_op *opline, uint32_t build_flags, crex_bitset use, crex_bitset def) /* {{{ */
{
	_crex_dfg_add_use_def_op(op_array, opline, build_flags, use, def);
}
/* }}} */

void crex_build_dfg(const crex_op_array *op_array, const crex_cfg *cfg, crex_dfg *dfg, uint32_t build_flags) /* {{{ */
{
	int set_size;
	crex_basic_block *blocks = cfg->blocks;
	int blocks_count = cfg->blocks_count;
	crex_bitset tmp, def, use, in, out;
	int k;
	int j;

	set_size = dfg->size;
	tmp = dfg->tmp;
	def = dfg->def;
	use = dfg->use;
	in  = dfg->in;
	out = dfg->out;

	/* Collect "def" and "use" sets */
	for (j = 0; j < blocks_count; j++) {
		crex_op *opline, *end;
		crex_bitset b_use, b_def;

		if ((blocks[j].flags & CREX_BB_REACHABLE) == 0) {
			continue;
		}

		opline = op_array->opcodes + blocks[j].start;
		end = opline + blocks[j].len;
		b_use = DFG_BITSET(use, set_size, j);
		b_def = DFG_BITSET(def, set_size, j);
		for (; opline < end; opline++) {
			if (opline->opcode != CREX_OP_DATA) {
				_crex_dfg_add_use_def_op(op_array, opline, build_flags, b_use, b_def);
			}
		}
	}

	/* Calculate "in" and "out" sets */
	{
		uint32_t worklist_len = crex_bitset_len(blocks_count);
		crex_bitset worklist;
		ALLOCA_FLAG(use_heap);
		worklist = CREX_BITSET_ALLOCA(worklist_len, use_heap);
		memset(worklist, 0, worklist_len * CREX_BITSET_ELM_SIZE);
		for (j = 0; j < blocks_count; j++) {
			crex_bitset_incl(worklist, j);
		}
		while (!crex_bitset_empty(worklist, worklist_len)) {
			/* We use the last block on the worklist, because predecessors tend to be located
			 * before the succeeding block, so this converges faster. */
			j = crex_bitset_last(worklist, worklist_len);
			crex_bitset_excl(worklist, j);

			if ((blocks[j].flags & CREX_BB_REACHABLE) == 0) {
				continue;
			}
			if (blocks[j].successors_count != 0) {
				crex_bitset_copy(DFG_BITSET(out, set_size, j), DFG_BITSET(in, set_size, blocks[j].successors[0]), set_size);
				for (k = 1; k < blocks[j].successors_count; k++) {
					crex_bitset_union(DFG_BITSET(out, set_size, j), DFG_BITSET(in, set_size, blocks[j].successors[k]), set_size);
				}
			} else {
				crex_bitset_clear(DFG_BITSET(out, set_size, j), set_size);
			}
			crex_bitset_union_with_difference(tmp, DFG_BITSET(use, set_size, j), DFG_BITSET(out, set_size, j), DFG_BITSET(def, set_size, j), set_size);
			if (!crex_bitset_equal(DFG_BITSET(in, set_size, j), tmp, set_size)) {
				crex_bitset_copy(DFG_BITSET(in, set_size, j), tmp, set_size);

				/* Add predecessors of changed block to worklist */
				{
					int *predecessors = &cfg->predecessors[blocks[j].predecessor_offset];
					for (k = 0; k < blocks[j].predecessors_count; k++) {
						crex_bitset_incl(worklist, predecessors[k]);
					}
				}
			}
		}

		free_alloca(worklist, use_heap);
	}
}
/* }}} */
