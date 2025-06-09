/*
   +----------------------------------------------------------------------+
   | Crex OPcache, Escape Analysis                                        |
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

#include "Optimizer/crex_optimizer.h"
#include "Optimizer/crex_optimizer_internal.h"
#include "crex_bitset.h"
#include "crex_cfg.h"
#include "crex_ssa.h"
#include "crex_inference.h"
#include "crex_dump.h"

/*
 * T. Kotzmann and H. Mossenbock. Escape analysis  in the context of dynamic
 * compilation and deoptimization. In Proceedings of the International
 * Conference on Virtual Execution Environments, pages 111-120, Chicago,
 * June 2005
 */

static crex_always_inline void union_find_init(int *parent, int *size, int count) /* {{{ */
{
	int i;

	for (i = 0; i < count; i++) {
		parent[i] = i;
		size[i] = 1;
	}
}
/* }}} */

static crex_always_inline int union_find_root(int *parent, int i) /* {{{ */
{
	int p = parent[i];

	while (i != p) {
		p = parent[p];
		parent[i] = p;
		i = p;
		p = parent[i];
	}
	return i;
}
/* }}} */

static crex_always_inline void union_find_unite(int *parent, int *size, int i, int j) /* {{{ */
{
	int r1 = union_find_root(parent, i);
	int r2 = union_find_root(parent, j);

	if (r1 != r2) {
		if (size[r1] < size[r2]) {
			parent[r1] = r2;
			size[r2] += size[r1];
		} else {
			parent[r2] = r1;
			size[r1] += size[r2];
		}
	}
}
/* }}} */

static crex_result crex_build_equi_escape_sets(int *parent, crex_op_array *op_array, crex_ssa *ssa) /* {{{ */
{
	crex_ssa_var *ssa_vars = ssa->vars;
	int ssa_vars_count = ssa->vars_count;
	crex_ssa_phi *p;
	int i, j;
	int *size;
	ALLOCA_FLAG(use_heap)

	size = do_alloca(sizeof(int) * ssa_vars_count, use_heap);
	if (!size) {
		return FAILURE;
	}
	union_find_init(parent, size, ssa_vars_count);

	for (i = 0; i < ssa_vars_count; i++) {
		if (ssa_vars[i].definition_phi) {
			p = ssa_vars[i].definition_phi;
			if (p->pi >= 0) {
				union_find_unite(parent, size, i, p->sources[0]);
			} else {
				for (j = 0; j < ssa->cfg.blocks[p->block].predecessors_count; j++) {
					union_find_unite(parent, size, i, p->sources[j]);
				}
			}
		} else if (ssa_vars[i].definition >= 0) {
			int def = ssa_vars[i].definition;
			crex_ssa_op *op = ssa->ops + def;
			crex_op *opline =  op_array->opcodes + def;

			if (op->op1_def >= 0) {
				if (op->op1_use >= 0) {
					if (opline->opcode != CREX_ASSIGN) {
						union_find_unite(parent, size, op->op1_def, op->op1_use);
					}
				}
				if (opline->opcode == CREX_ASSIGN && op->op2_use >= 0) {
					union_find_unite(parent, size, op->op1_def, op->op2_use);
				}
			}
			if (op->op2_def >= 0) {
				if (op->op2_use >= 0) {
					union_find_unite(parent, size, op->op2_def, op->op2_use);
				}
			}
			if (op->result_def >= 0) {
				if (op->result_use >= 0) {
					if (opline->opcode != CREX_QM_ASSIGN) {
						union_find_unite(parent, size, op->result_def, op->result_use);
					}
				}
				if (opline->opcode == CREX_QM_ASSIGN && op->op1_use >= 0) {
					union_find_unite(parent, size, op->result_def, op->op1_use);
				}
				if (opline->opcode == CREX_ASSIGN && op->op2_use >= 0) {
					union_find_unite(parent, size, op->result_def, op->op2_use);
				}
				if (opline->opcode == CREX_ASSIGN && op->op1_def >= 0) {
					union_find_unite(parent, size, op->result_def, op->op1_def);
				}
			}
		}
	}

	for (i = 0; i < ssa_vars_count; i++) {
		parent[i] = union_find_root(parent, i);
	}

	free_alloca(size, use_heap);

	return SUCCESS;
}
/* }}} */

static bool is_allocation_def(crex_op_array *op_array, crex_ssa *ssa, int def, int var, const crex_script *script) /* {{{ */
{
	crex_ssa_op *ssa_op = ssa->ops + def;
	crex_op *opline = op_array->opcodes + def;

	if (ssa_op->result_def == var) {
		switch (opline->opcode) {
			case CREX_INIT_ARRAY:
				return 1;
			case CREX_NEW: {
			    /* objects with destructors should escape */
				crex_class_entry *ce = crex_optimizer_get_class_entry_from_op1(
					script, op_array, opline);
				uint32_t forbidden_flags =
					/* These flags will always cause an exception */
					CREX_ACC_IMPLICIT_ABSTRACT_CLASS | CREX_ACC_EXPLICIT_ABSTRACT_CLASS
					| CREX_ACC_INTERFACE | CREX_ACC_TRAIT;
				if (ce && !ce->parent && !ce->create_object && !ce->constructor &&
					!ce->destructor && !ce->__get && !ce->__set &&
					!(ce->ce_flags & forbidden_flags) &&
					(ce->ce_flags & CREX_ACC_CONSTANTS_UPDATED)) {
					return 1;
				}
				break;
			}
			case CREX_QM_ASSIGN:
				if (opline->op1_type == IS_CONST
				 && C_TYPE_P(CRT_CONSTANT(opline->op1)) == IS_ARRAY) {
					return 1;
				}
				if (opline->op1_type == IS_CV && (OP1_INFO() & MAY_BE_ARRAY)) {
					return 1;
				}
				break;
			case CREX_ASSIGN:
				if (opline->op1_type == IS_CV && (OP1_INFO() & MAY_BE_ARRAY)) {
					return 1;
				}
				break;
		}
	} else if (ssa_op->op1_def == var) {
		switch (opline->opcode) {
			case CREX_ASSIGN:
				if (opline->op2_type == IS_CONST
				 && C_TYPE_P(CRT_CONSTANT(opline->op2)) == IS_ARRAY) {
					return 1;
				}
				if (opline->op2_type == IS_CV && (OP2_INFO() & MAY_BE_ARRAY)) {
					return 1;
				}
				break;
			case CREX_ASSIGN_DIM:
				if (OP1_INFO() & (MAY_BE_UNDEF | MAY_BE_NULL | MAY_BE_FALSE)) {
					/* implicit object/array allocation */
					return 1;
				}
				break;
		}
	}

	return 0;
}
/* }}} */

static bool is_local_def(crex_op_array *op_array, crex_ssa *ssa, int def, int var, const crex_script *script) /* {{{ */
{
	crex_ssa_op *op = ssa->ops + def;
	crex_op *opline = op_array->opcodes + def;

	if (op->result_def == var) {
		switch (opline->opcode) {
			case CREX_INIT_ARRAY:
			case CREX_ADD_ARRAY_ELEMENT:
			case CREX_QM_ASSIGN:
			case CREX_ASSIGN:
				return 1;
			case CREX_NEW: {
				/* objects with destructors should escape */
				crex_class_entry *ce = crex_optimizer_get_class_entry_from_op1(
					script, op_array, opline);
				if (ce && !ce->create_object && !ce->constructor &&
					!ce->destructor && !ce->__get && !ce->__set && !ce->parent) {
					return 1;
				}
				break;
			}
		}
	} else if (op->op1_def == var) {
		switch (opline->opcode) {
			case CREX_ASSIGN:
			case CREX_ASSIGN_DIM:
			case CREX_ASSIGN_OBJ:
			case CREX_ASSIGN_OBJ_REF:
			case CREX_ASSIGN_DIM_OP:
			case CREX_ASSIGN_OBJ_OP:
			case CREX_PRE_INC_OBJ:
			case CREX_PRE_DEC_OBJ:
			case CREX_POST_INC_OBJ:
			case CREX_POST_DEC_OBJ:
				return 1;
		}
	}

	return 0;
}
/* }}} */

static bool is_escape_use(crex_op_array *op_array, crex_ssa *ssa, int use, int var) /* {{{ */
{
	crex_ssa_op *ssa_op = ssa->ops + use;
	crex_op *opline = op_array->opcodes + use;

	if (ssa_op->op1_use == var) {
		switch (opline->opcode) {
			case CREX_ASSIGN:
				/* no_val */
				break;
			case CREX_QM_ASSIGN:
				if (opline->op1_type == IS_CV) {
					if (OP1_INFO() & MAY_BE_OBJECT) {
						/* object aliasing */
						return 1;
					}
				}
				break;
			case CREX_ISSET_ISEMPTY_DIM_OBJ:
			case CREX_ISSET_ISEMPTY_PROP_OBJ:
			case CREX_FETCH_DIM_R:
			case CREX_FETCH_OBJ_R:
			case CREX_FETCH_DIM_IS:
			case CREX_FETCH_OBJ_IS:
				break;
			case CREX_ASSIGN_OP:
				return 1;
			case CREX_ASSIGN_DIM_OP:
			case CREX_ASSIGN_OBJ_OP:
			case CREX_ASSIGN_STATIC_PROP_OP:
			case CREX_ASSIGN_DIM:
			case CREX_ASSIGN_OBJ:
			case CREX_ASSIGN_OBJ_REF:
				break;
			case CREX_PRE_INC_OBJ:
			case CREX_PRE_DEC_OBJ:
			case CREX_POST_INC_OBJ:
			case CREX_POST_DEC_OBJ:
				break;
			case CREX_INIT_ARRAY:
			case CREX_ADD_ARRAY_ELEMENT:
				if (opline->extended_value & CREX_ARRAY_ELEMENT_REF) {
					return 1;
				}
				if (OP1_INFO() & MAY_BE_OBJECT) {
					/* object aliasing */
					return 1;
				}
				/* reference dependencies processed separately */
				break;
			case CREX_OP_DATA:
				if ((opline-1)->opcode != CREX_ASSIGN_DIM
				 && (opline-1)->opcode != CREX_ASSIGN_OBJ) {
					return 1;
				}
				if (OP1_INFO() & MAY_BE_OBJECT) {
					/* object aliasing */
					return 1;
				}
				opline--;
				ssa_op--;
				if (opline->op1_type != IS_CV
				 || (OP1_INFO() & MAY_BE_REF)
				 || (ssa_op->op1_def >= 0 && ssa->vars[ssa_op->op1_def].alias)) {
					/* assignment into escaping structure */
					return 1;
				}
				/* reference dependencies processed separately */
				break;
			default:
				return 1;
		}
	}

	if (ssa_op->op2_use == var) {
		switch (opline->opcode) {
			case CREX_ASSIGN:
				if (opline->op1_type != IS_CV
				 || (OP1_INFO() & MAY_BE_REF)
				 || (ssa_op->op1_def >= 0 && ssa->vars[ssa_op->op1_def].alias)) {
					/* assignment into escaping variable */
					return 1;
				}
				if (opline->op2_type == IS_CV || opline->result_type != IS_UNUSED) {
					if (OP2_INFO() & MAY_BE_OBJECT) {
						/* object aliasing */
						return 1;
					}
				}
				break;
			default:
				return 1;
		}
	}

	if (ssa_op->result_use == var) {
		switch (opline->opcode) {
			case CREX_ASSIGN:
			case CREX_QM_ASSIGN:
			case CREX_INIT_ARRAY:
			case CREX_ADD_ARRAY_ELEMENT:
				break;
			default:
				return 1;
		}
	}

	return 0;
}
/* }}} */

crex_result crex_ssa_escape_analysis(const crex_script *script, crex_op_array *op_array, crex_ssa *ssa) /* {{{ */
{
	crex_ssa_var *ssa_vars = ssa->vars;
	int ssa_vars_count = ssa->vars_count;
	int i, root, use;
	int *ees;
	bool has_allocations;
	int num_non_escaped;
	ALLOCA_FLAG(use_heap)

	if (!ssa_vars) {
		return SUCCESS;
	}

	has_allocations = 0;
	for (i = op_array->last_var; i < ssa_vars_count; i++) {
		if (ssa_vars[i].definition >= 0
		  && (ssa->var_info[i].type & (MAY_BE_ARRAY|MAY_BE_OBJECT))
		  && is_allocation_def(op_array, ssa, ssa_vars[i].definition, i, script)) {
			has_allocations = 1;
			break;
		}
	}
	if (!has_allocations) {
		return SUCCESS;
	}


	/* 1. Build EES (Equi-Escape Sets) */
	ees = do_alloca(sizeof(int) * ssa_vars_count, use_heap);
	if (!ees) {
		return FAILURE;
	}

	if (crex_build_equi_escape_sets(ees, op_array, ssa) == FAILURE) {
		return FAILURE;
	}

	/* 2. Identify Allocations */
	num_non_escaped = 0;
	for (i = op_array->last_var; i < ssa_vars_count; i++) {
		root = ees[i];
		if (ssa_vars[root].escape_state > ESCAPE_STATE_NO_ESCAPE) {
			/* already escape. skip */
		} else if (ssa_vars[i].alias && (ssa->var_info[i].type & MAY_BE_REF)) {
			if (ssa_vars[root].escape_state == ESCAPE_STATE_NO_ESCAPE) {
				num_non_escaped--;
			}
			ssa_vars[root].escape_state = ESCAPE_STATE_GLOBAL_ESCAPE;
		} else if (ssa_vars[i].definition >= 0
			 && (ssa->var_info[i].type & (MAY_BE_ARRAY|MAY_BE_OBJECT))) {
			if (!is_local_def(op_array, ssa, ssa_vars[i].definition, i, script)) {
				if (ssa_vars[root].escape_state == ESCAPE_STATE_NO_ESCAPE) {
					num_non_escaped--;
				}
				ssa_vars[root].escape_state = ESCAPE_STATE_GLOBAL_ESCAPE;
			} else if (ssa_vars[root].escape_state == ESCAPE_STATE_UNKNOWN
			 && is_allocation_def(op_array, ssa, ssa_vars[i].definition, i, script)) {
				ssa_vars[root].escape_state = ESCAPE_STATE_NO_ESCAPE;
				num_non_escaped++;
			}
		}
	}

	/* 3. Mark escaped EES */
	if (num_non_escaped) {
		for (i = 0; i < ssa_vars_count; i++) {
			if (ssa_vars[i].use_chain >= 0) {
				root = ees[i];
				if (ssa_vars[root].escape_state == ESCAPE_STATE_NO_ESCAPE) {
					FOREACH_USE(ssa_vars + i, use) {
						if (is_escape_use(op_array, ssa, use, i)) {
							ssa_vars[root].escape_state = ESCAPE_STATE_GLOBAL_ESCAPE;
							num_non_escaped--;
							if (num_non_escaped == 0) {
								i = ssa_vars_count;
							}
							break;
						}
					} FOREACH_USE_END();
				}
			}
		}
	}

	/* 4. Process referential dependencies */
	if (num_non_escaped) {
		bool changed;

		do {
			changed = 0;
			for (i = 0; i < ssa_vars_count; i++) {
				if (ssa_vars[i].use_chain >= 0) {
					root = ees[i];
					if (ssa_vars[root].escape_state == ESCAPE_STATE_NO_ESCAPE) {
						FOREACH_USE(ssa_vars + i, use) {
							crex_ssa_op *op = ssa->ops + use;
							crex_op *opline = op_array->opcodes + use;
							int enclosing_root;

							if (opline->opcode == CREX_OP_DATA &&
							    ((opline-1)->opcode == CREX_ASSIGN_DIM ||
							     (opline-1)->opcode == CREX_ASSIGN_OBJ ||
							     (opline-1)->opcode == CREX_ASSIGN_OBJ_REF) &&
							    op->op1_use == i &&
							    (op-1)->op1_use >= 0) {
								enclosing_root = ees[(op-1)->op1_use];
							} else if ((opline->opcode == CREX_INIT_ARRAY ||
							     opline->opcode == CREX_ADD_ARRAY_ELEMENT) &&
							    op->op1_use == i &&
							    op->result_def >= 0) {
								enclosing_root = ees[op->result_def];
							} else {
								continue;
							}

							if (ssa_vars[enclosing_root].escape_state == ESCAPE_STATE_UNKNOWN ||
							    ssa_vars[enclosing_root].escape_state > ssa_vars[root].escape_state) {
							    if (ssa_vars[enclosing_root].escape_state == ESCAPE_STATE_UNKNOWN) {
									ssa_vars[root].escape_state = ESCAPE_STATE_GLOBAL_ESCAPE;
							    } else {
									ssa_vars[root].escape_state = ssa_vars[enclosing_root].escape_state;
								}
								if (ssa_vars[root].escape_state == ESCAPE_STATE_GLOBAL_ESCAPE) {
									num_non_escaped--;
									if (num_non_escaped == 0) {
										changed = 0;
									} else {
										changed = 1;
									}
									break;
								} else {
									changed = 1;
								}
							}
						} FOREACH_USE_END();
					}
				}
			}
		} while (changed);
	}

	/* 5. Propagate values of escape sets to variables */
	for (i = 0; i < ssa_vars_count; i++) {
		root = ees[i];
		if (i != root) {
			ssa_vars[i].escape_state = ssa_vars[root].escape_state;
		}
	}

	free_alloca(ees, use_heap);

	return SUCCESS;
}
/* }}} */
