/*
   +----------------------------------------------------------------------+
   | Crex Engine, e-SSA based Type & Range Inference                      |
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
#include "crex_generators.h"
#include "crex_inference.h"
#include "crex_func_info.h"
#include "crex_call_graph.h"
#include "crex_closures.h"
#include "crex_worklist.h"
#include "crex_optimizer_internal.h"

/* The used range inference algorithm is described in:
 *     V. Campos, R. Rodrigues, I. de Assis Costa and F. Pereira.
 *     "Speed and Precision in Range Analysis", SBLP'12.
 *
 * There are a couple degrees of freedom, we use:
 *  * Propagation on SCCs.
 *  * e-SSA for live range splitting.
 *  * Only intra-procedural inference.
 *  * Widening with warmup passes, but without jump sets.
 */

/* Whether to handle symbolic range constraints */
#define SYM_RANGE

/* Whether to handle negative range constraints */
/* Negative range inference is buggy, so disabled for now */
#undef NEG_RANGE

/* Number of warmup passes to use prior to widening */
#define RANGE_WARMUP_PASSES 16

/* Logging for range inference in general */
#if 0
#define LOG_SSA_RANGE(...) fprintf(stderr, __VA_ARGS__)
#else
#define LOG_SSA_RANGE(...)
#endif

/* Logging for negative range constraints */
#if 0
#define LOG_NEG_RANGE(...) fprintf(stderr, __VA_ARGS__)
#else
#define LOG_NEG_RANGE(...)
#endif

/* Pop elements in unspecified order from worklist until it is empty */
#define WHILE_WORKLIST(worklist, len, i) do { \
	bool _done = 0; \
	while (!_done) { \
		_done = 1; \
		CREX_BITSET_FOREACH(worklist, len, i) { \
			crex_bitset_excl(worklist, i); \
			_done = 0;

#define WHILE_WORKLIST_END() \
		} CREX_BITSET_FOREACH_END(); \
	} \
} while (0)

#define CHECK_SCC_VAR(var2) \
	do { \
		if (!ssa->vars[var2].no_val) { \
			if (ssa->vars[var2].scc < 0) { \
				crex_ssa_check_scc_var(op_array, ssa, var2, index, stack); \
			} \
			if (ssa->vars[var2].scc < ssa->vars[var].scc) { \
				ssa->vars[var].scc = ssa->vars[var2].scc; \
				is_root = 0; \
			} \
		} \
	} while (0)

#define CHECK_SCC_ENTRY(var2) \
	do { \
		if (ssa->vars[var2].scc != ssa->vars[var].scc) { \
			ssa->vars[var2].scc_entry = 1; \
		} \
	} while (0)

#define ADD_SCC_VAR(_var) \
	do { \
		if (ssa->vars[_var].scc == scc && \
		    !(ssa->var_info[_var].type & MAY_BE_REF)) { \
			crex_bitset_incl(worklist, _var); \
		} \
	} while (0)

#define ADD_SCC_VAR_1(_var) \
	do { \
		if (ssa->vars[_var].scc == scc && \
		    !(ssa->var_info[_var].type & MAY_BE_REF) && \
		    !crex_bitset_in(visited, _var)) { \
			crex_bitset_incl(worklist, _var); \
		} \
	} while (0)

#define FOR_EACH_DEFINED_VAR(line, MACRO) \
	do { \
		if (ssa->ops[line].op1_def >= 0) { \
			MACRO(ssa->ops[line].op1_def); \
		} \
		if (ssa->ops[line].op2_def >= 0) { \
			MACRO(ssa->ops[line].op2_def); \
		} \
		if (ssa->ops[line].result_def >= 0) { \
			MACRO(ssa->ops[line].result_def); \
		} \
		if (op_array->opcodes[line].opcode == CREX_OP_DATA) { \
			if (ssa->ops[line-1].op1_def >= 0) { \
				MACRO(ssa->ops[line-1].op1_def); \
			} \
			if (ssa->ops[line-1].op2_def >= 0) { \
				MACRO(ssa->ops[line-1].op2_def); \
			} \
			if (ssa->ops[line-1].result_def >= 0) { \
				MACRO(ssa->ops[line-1].result_def); \
			} \
		} else if ((uint32_t)line+1 < op_array->last && \
		           op_array->opcodes[line+1].opcode == CREX_OP_DATA) { \
			if (ssa->ops[line+1].op1_def >= 0) { \
				MACRO(ssa->ops[line+1].op1_def); \
			} \
			if (ssa->ops[line+1].op2_def >= 0) { \
				MACRO(ssa->ops[line+1].op2_def); \
			} \
			if (ssa->ops[line+1].result_def >= 0) { \
				MACRO(ssa->ops[line+1].result_def); \
			} \
		} \
	} while (0)


#define FOR_EACH_VAR_USAGE(_var, MACRO) \
	do { \
		crex_ssa_phi *p = ssa->vars[_var].phi_use_chain; \
		int use = ssa->vars[_var].use_chain; \
		while (use >= 0) { \
			FOR_EACH_DEFINED_VAR(use, MACRO); \
			use = crex_ssa_next_use(ssa->ops, _var, use); \
		} \
		p = ssa->vars[_var].phi_use_chain; \
		while (p) { \
			MACRO(p->ssa_var); \
			p = crex_ssa_next_use_phi(ssa, _var, p); \
		} \
	} while (0)

static inline bool add_will_overflow(crex_long a, crex_long b) {
	return (b > 0 && a > CREX_LONG_MAX - b)
		|| (b < 0 && a < CREX_LONG_MIN - b);
}
#if 0
static inline bool sub_will_overflow(crex_long a, crex_long b) {
	return (b > 0 && a < CREX_LONG_MIN + b)
		|| (b < 0 && a > CREX_LONG_MAX + b);
}
#endif

#if 0
/* Recursive Pearce's SCC algorithm implementation */
static void crex_ssa_check_scc_var(const crex_op_array *op_array, crex_ssa *ssa, int var, int *index, crex_worklist_stack *stack) /* {{{ */
{
	int is_root = 1;
#ifdef SYM_RANGE
	crex_ssa_phi *p;
#endif

	ssa->vars[var].scc = *index;
	(*index)++;

	FOR_EACH_VAR_USAGE(var, CHECK_SCC_VAR);

#ifdef SYM_RANGE
	/* Process symbolic control-flow constraints */
	p = ssa->vars[var].sym_use_chain;
	while (p) {
		CHECK_SCC_VAR(p->ssa_var);
		p = p->sym_use_chain;
	}
#endif

	if (is_root) {
		ssa->sccs--;
		while (stack->len > 0) {
			int var2 = crex_worklist_stack_peek(stack);
			if (ssa->vars[var2].scc < ssa->vars[var].scc) {
				break;
			}
			crex_worklist_stack_pop(stack);
			ssa->vars[var2].scc = ssa->sccs;
			(*index)--;
		}
		ssa->vars[var].scc = ssa->sccs;
		ssa->vars[var].scc_entry = 1;
		(*index)--;
	} else {
		crex_worklist_stack_push(stack, var);
	}
}
/* }}} */

CREX_API void crex_ssa_find_sccs(const crex_op_array *op_array, crex_ssa *ssa) /* {{{ */
{
	int index = 0;
	crex_worklist_stack stack;
	int j;
	ALLOCA_FLAG(stack_use_heap)

	CREX_WORKLIST_STACK_ALLOCA(&stack, ssa->vars_count, stack_use_heap);

	/* Find SCCs using Pearce's algorithm. */
	ssa->sccs = ssa->vars_count;
	for (j = 0; j < ssa->vars_count; j++) {
		if (!ssa->vars[j].no_val && ssa->vars[j].scc < 0) {
			crex_ssa_check_scc_var(op_array, ssa, j, &index, &stack);
		}
	}

	if (ssa->sccs) {
		/* Shift SCC indexes. */
		for (j = 0; j < ssa->vars_count; j++) {
			if (ssa->vars[j].scc >= 0) {
				ssa->vars[j].scc -= ssa->sccs;
			}
		}
	}
	ssa->sccs = ssa->vars_count - ssa->sccs;

	for (j = 0; j < ssa->vars_count; j++) {
		if (ssa->vars[j].scc >= 0) {
			int var = j;
			FOR_EACH_VAR_USAGE(var, CHECK_SCC_ENTRY);
		}
	}

	CREX_WORKLIST_STACK_FREE_ALLOCA(&stack, stack_use_heap);
}
/* }}} */

#else
/* Iterative Pearce's SCC algorithm implementation */

typedef struct _crex_scc_iterator {
	int               state;
	int               last;
	union {
		int           use;
		crex_ssa_phi *phi;
	};
} crex_scc_iterator;

static int crex_scc_next(const crex_op_array *op_array, crex_ssa *ssa, int var, crex_scc_iterator *iterator) /* {{{ */
{
	crex_ssa_phi *phi;
	int use, var2;

	switch (iterator->state) {
		case 0:                       goto state_0;
		case 1:  use = iterator->use; goto state_1;
		case 2:  use = iterator->use; goto state_2;
		case 3:  use = iterator->use; goto state_3;
		case 4:  use = iterator->use; goto state_4;
		case 5:  use = iterator->use; goto state_5;
		case 6:  use = iterator->use; goto state_6;
		case 7:  use = iterator->use; goto state_7;
		case 8:  use = iterator->use; goto state_8;
		case 9:  phi = iterator->phi; goto state_9;
#ifdef SYM_RANGE
		case 10: phi = iterator->phi; goto state_10;
#endif
		case 11:                      goto state_11;
	}

state_0:
	use = ssa->vars[var].use_chain;
	while (use >= 0) {
		iterator->use = use;
		var2 = ssa->ops[use].op1_def;
		if (var2 >= 0 && !ssa->vars[var2].no_val) {
			iterator->state = 1;
			return var2;
		}
state_1:
		var2 = ssa->ops[use].op2_def;
		if (var2 >= 0 && !ssa->vars[var2].no_val) {
			iterator->state = 2;
			return var2;
		}
state_2:
		var2 = ssa->ops[use].result_def;
		if (var2 >= 0 && !ssa->vars[var2].no_val) {
			iterator->state = 3;
			return var2;
		}
state_3:
		if (op_array->opcodes[use].opcode == CREX_OP_DATA) {
			var2 = ssa->ops[use-1].op1_def;
			if (var2 >= 0 && !ssa->vars[var2].no_val) {
				iterator->state = 4;
				return var2;
			}
state_4:
			var2 = ssa->ops[use-1].op2_def;
			if (var2 >= 0 && !ssa->vars[var2].no_val) {
				iterator->state = 5;
				return var2;
			}
state_5:
			var2 = ssa->ops[use-1].result_def;
			if (var2 >= 0 && !ssa->vars[var2].no_val) {
				iterator->state = 8;
				return var2;
			}
		} else if ((uint32_t)use+1 < op_array->last &&
		           op_array->opcodes[use+1].opcode == CREX_OP_DATA) {
			var2 = ssa->ops[use+1].op1_def;
			if (var2 >= 0 && !ssa->vars[var2].no_val) {
				iterator->state = 6;
				return var2;
			}
state_6:
			var2 = ssa->ops[use+1].op2_def;
			if (var2 >= 0 && !ssa->vars[var2].no_val) {
				iterator->state = 7;
				return var2;
			}
state_7:
			var2 = ssa->ops[use+1].result_def;
			if (var2 >= 0 && !ssa->vars[var2].no_val) {
				iterator->state = 8;
				return var2;
			}
		}
state_8:
		use = crex_ssa_next_use(ssa->ops, var, use);
	}

	phi = ssa->vars[var].phi_use_chain;
	while (phi) {
		var2 = phi->ssa_var;
		if (!ssa->vars[var2].no_val) {
			iterator->state = 9;
			iterator->phi = phi;
			return var2;
		}
state_9:
		phi = crex_ssa_next_use_phi(ssa, var, phi);
	}

#ifdef SYM_RANGE
	/* Process symbolic control-flow constraints */
	phi = ssa->vars[var].sym_use_chain;
	while (phi) {
		var2 = phi->ssa_var;
		if (!ssa->vars[var2].no_val) {
			iterator->state = 10;
			iterator->phi = phi;
			return var2;
		}
state_10:
		phi = phi->sym_use_chain;
	}
#endif

	iterator->state = 11;
state_11:
	return -1;
}
/* }}} */

static void crex_ssa_check_scc_var(const crex_op_array *op_array, crex_ssa *ssa, int var, int *index, crex_worklist_stack *stack, crex_worklist_stack *vstack, crex_scc_iterator *iterators) /* {{{ */
{
restart:
	crex_worklist_stack_push(vstack, var);
	iterators[var].state = 0;
	iterators[var].last = -1;
	ssa->vars[var].scc_entry = 1;
	ssa->vars[var].scc = *index;
	(*index)++;

	while (vstack->len > 0) {
		var = crex_worklist_stack_peek(vstack);
		while (1) {
			int var2;

			if (iterators[var].last >= 0) {
				/* finish edge */
				var2 = iterators[var].last;
				if (ssa->vars[var2].scc < ssa->vars[var].scc) {
					ssa->vars[var].scc = ssa->vars[var2].scc;
					ssa->vars[var].scc_entry = 0;
				}
			}
			var2 = crex_scc_next(op_array, ssa, var, iterators + var);
			iterators[var].last = var2;
			if (var2 < 0) break;
			/* begin edge */
			if (ssa->vars[var2].scc < 0) {
				var = var2;
				goto restart;
			}
		}

		/* finish visiting */
		crex_worklist_stack_pop(vstack);
		if (ssa->vars[var].scc_entry) {
			ssa->sccs--;
			while (stack->len > 0) {
				int var2 = crex_worklist_stack_peek(stack);
				if (ssa->vars[var2].scc < ssa->vars[var].scc) {
					break;
				}
				crex_worklist_stack_pop(stack);
				ssa->vars[var2].scc = ssa->sccs;
				(*index)--;
			}
			ssa->vars[var].scc = ssa->sccs;
			(*index)--;
		} else {
			crex_worklist_stack_push(stack, var);
		}
	}
}
/* }}} */

CREX_API void crex_ssa_find_sccs(const crex_op_array *op_array, crex_ssa *ssa) /* {{{ */
{
	int index = 0;
	crex_worklist_stack stack, vstack;
	crex_scc_iterator *iterators;
	int j;
	ALLOCA_FLAG(stack_use_heap)
	ALLOCA_FLAG(vstack_use_heap)
	ALLOCA_FLAG(iterators_use_heap)

	iterators = do_alloca(sizeof(crex_scc_iterator) * ssa->vars_count, iterators_use_heap);
	CREX_WORKLIST_STACK_ALLOCA(&vstack, ssa->vars_count, vstack_use_heap);
	CREX_WORKLIST_STACK_ALLOCA(&stack, ssa->vars_count, stack_use_heap);

	/* Find SCCs using Pearce's algorithm. */
	ssa->sccs = ssa->vars_count;
	for (j = 0; j < ssa->vars_count; j++) {
		if (!ssa->vars[j].no_val && ssa->vars[j].scc < 0) {
			crex_ssa_check_scc_var(op_array, ssa, j, &index, &stack, &vstack, iterators);
		}
	}

	if (ssa->sccs) {
		/* Shift SCC indexes. */
		for (j = 0; j < ssa->vars_count; j++) {
			if (ssa->vars[j].scc >= 0) {
				ssa->vars[j].scc -= ssa->sccs;
			}
		}
	}
	ssa->sccs = ssa->vars_count - ssa->sccs;

	for (j = 0; j < ssa->vars_count; j++) {
		if (ssa->vars[j].scc >= 0) {
			int var = j;
			FOR_EACH_VAR_USAGE(var, CHECK_SCC_ENTRY);
		}
	}

	CREX_WORKLIST_STACK_FREE_ALLOCA(&stack, stack_use_heap);
	CREX_WORKLIST_STACK_FREE_ALLOCA(&vstack, vstack_use_heap);
	free_alloca(iterators, iterators_use_heap);
}
/* }}} */

#endif

CREX_API void crex_ssa_find_false_dependencies(const crex_op_array *op_array, crex_ssa *ssa) /* {{{ */
{
	crex_ssa_var *ssa_vars = ssa->vars;
	crex_ssa_op *ssa_ops = ssa->ops;
	int ssa_vars_count = ssa->vars_count;
	crex_bitset worklist;
	int i, j, use;
	crex_ssa_phi *p;
	ALLOCA_FLAG(use_heap);

	if (!op_array->function_name || !ssa->vars || !ssa->ops) {
		return;
	}

	worklist = do_alloca(sizeof(crex_ulong) * crex_bitset_len(ssa_vars_count), use_heap);
	memset(worklist, 0, sizeof(crex_ulong) * crex_bitset_len(ssa_vars_count));

	for (i = 0; i < ssa_vars_count; i++) {
		ssa_vars[i].no_val = 1; /* mark as unused */
		use = ssa->vars[i].use_chain;
		while (use >= 0) {
			if (!crex_ssa_is_no_val_use(&op_array->opcodes[use], &ssa->ops[use], i)) {
				ssa_vars[i].no_val = 0; /* used directly */
				crex_bitset_incl(worklist, i);
				break;
			}
			use = crex_ssa_next_use(ssa_ops, i, use);
		}
	}

	WHILE_WORKLIST(worklist, crex_bitset_len(ssa_vars_count), i) {
		if (ssa_vars[i].definition_phi) {
			/* mark all possible sources as used */
			p = ssa_vars[i].definition_phi;
			if (p->pi >= 0) {
				if (ssa_vars[p->sources[0]].no_val) {
					ssa_vars[p->sources[0]].no_val = 0; /* used indirectly */
					crex_bitset_incl(worklist, p->sources[0]);
				}
			} else {
				for (j = 0; j < ssa->cfg.blocks[p->block].predecessors_count; j++) {
					CREX_ASSERT(p->sources[j] >= 0);
					if (ssa->vars[p->sources[j]].no_val) {
						ssa_vars[p->sources[j]].no_val = 0; /* used indirectly */
						crex_bitset_incl(worklist, p->sources[j]);
					}
				}
			}
		}
	} WHILE_WORKLIST_END();

	free_alloca(worklist, use_heap);
}
/* }}} */

/* From "Hacker's Delight" */
crex_ulong minOR(crex_ulong a, crex_ulong b, crex_ulong c, crex_ulong d)
{
	crex_ulong m, temp;

	m = C_UL(1) << (sizeof(crex_ulong) * 8 - 1);
	while (m != 0) {
		if (~a & c & m) {
			temp = (a | m) & -m;
			if (temp <= b) {
				a = temp;
				break;
			}
		} else if (a & ~c & m) {
			temp = (c | m) & -m;
			if (temp <= d) {
				c = temp;
				break;
			}
		}
		m = m >> 1;
	}
	return a | c;
}

crex_ulong maxOR(crex_ulong a, crex_ulong b, crex_ulong c, crex_ulong d)
{
	crex_ulong m, temp;

	m = C_UL(1) << (sizeof(crex_ulong) * 8 - 1);
	while (m != 0) {
		if (b & d & m) {
			temp = (b - m) | (m - 1);
			if (temp >= a) {
				b = temp;
				break;
			}
			temp = (d - m) | (m - 1);
			if (temp >= c) {
				d = temp;
				break;
			}
		}
		m = m >> 1;
	}
	return b | d;
}

crex_ulong minAND(crex_ulong a, crex_ulong b, crex_ulong c, crex_ulong d)
{
	crex_ulong m, temp;

	m = C_UL(1) << (sizeof(crex_ulong) * 8 - 1);
	while (m != 0) {
		if (~a & ~c & m) {
			temp = (a | m) & -m;
			if (temp <= b) {
				a = temp;
				break;
			}
			temp = (c | m) & -m;
			if (temp <= d) {
				c = temp;
				break;
			}
		}
		m = m >> 1;
	}
	return a & c;
}

crex_ulong maxAND(crex_ulong a, crex_ulong b, crex_ulong c, crex_ulong d)
{
	crex_ulong m, temp;

	m = C_UL(1) << (sizeof(crex_ulong) * 8 - 1);
	while (m != 0) {
		if (b & ~d & m) {
			temp = (b | ~m) | (m - 1);
			if (temp >= a) {
				b = temp;
				break;
			}
		} else if (~b & d & m) {
			temp = (d | ~m) | (m - 1);
			if (temp >= c) {
				d = temp;
				break;
			}
		}
		m = m >> 1;
	}
	return b & d;
}

crex_ulong minXOR(crex_ulong a, crex_ulong b, crex_ulong c, crex_ulong d)
{
	return minAND(a, b, ~d, ~c) | minAND(~b, ~a, c, d);
}

crex_ulong maxXOR(crex_ulong a, crex_ulong b, crex_ulong c, crex_ulong d)
{
	return maxOR(0, maxAND(a, b, ~d, ~c), 0, maxAND(~b, ~a, c, d));
}

/* Based on "Hacker's Delight" */

/*
0: + + + + 0 0 0 0 => 0 0 + min/max
2: + + - + 0 0 1 0 => 1 0 ? min(a,b,c,-1)/max(a,b,0,d)
3: + + - - 0 0 1 1 => 1 1 - min/max
8: - + + + 1 0 0 0 => 1 0 ? min(a,-1,b,d)/max(0,b,c,d)
a: - + - + 1 0 1 0 => 1 0 ? MIN(a,c)/max(0,b,0,d)
b: - + - - 1 0 1 1 => 1 1 - c/-1
c: - - + + 1 1 0 0 => 1 1 - min/max
e: - - - + 1 1 1 0 => 1 1 - a/-1
f  - - - - 1 1 1 1 => 1 1 - min/max
*/
static void crex_ssa_range_or(crex_long a, crex_long b, crex_long c, crex_long d, crex_ssa_range *tmp)
{
	int x = ((a < 0) ? 8 : 0) |
	        ((b < 0) ? 4 : 0) |
	        ((c < 0) ? 2 : 0) |
	        ((d < 0) ? 1 : 0);
	switch (x) {
		case 0x0:
		case 0x3:
		case 0xc:
		case 0xf:
			tmp->min = minOR(a, b, c, d);
			tmp->max = maxOR(a, b, c, d);
			break;
		case 0x2:
			tmp->min = minOR(a, b, c, -1);
			tmp->max = maxOR(a, b, 0, d);
			break;
		case 0x8:
			tmp->min = minOR(a, -1, c, d);
			tmp->max = maxOR(0, b, c, d);
			break;
		case 0xa:
			tmp->min = MIN(a, c);
			tmp->max = maxOR(0, b, 0, d);
			break;
		case 0xb:
			tmp->min = c;
			tmp->max = -1;
			break;
		case 0xe:
			tmp->min = a;
			tmp->max = -1;
			break;
	}
}

/*
0: + + + + 0 0 0 0 => 0 0 + min/max
2: + + - + 0 0 1 0 => 0 0 + 0/b
3: + + - - 0 0 1 1 => 0 0 + min/max
8: - + + + 1 0 0 0 => 0 0 + 0/d
a: - + - + 1 0 1 0 => 1 0 ? min(a,-1,c,-1)/NAX(b,d)
b: - + - - 1 0 1 1 => 1 0 ? min(a,-1,c,d)/max(0,b,c,d)
c: - - + + 1 1 0 0 => 1 1 - min/max
e: - - - + 1 1 1 0 => 1 0 ? min(a,b,c,-1)/max(a,b,0,d)
f  - - - - 1 1 1 1 => 1 1 - min/max
*/
static void crex_ssa_range_and(crex_long a, crex_long b, crex_long c, crex_long d, crex_ssa_range *tmp)
{
	int x = ((a < 0) ? 8 : 0) |
	        ((b < 0) ? 4 : 0) |
	        ((c < 0) ? 2 : 0) |
	        ((d < 0) ? 1 : 0);
	switch (x) {
		case 0x0:
		case 0x3:
		case 0xc:
		case 0xf:
			tmp->min = minAND(a, b, c, d);
			tmp->max = maxAND(a, b, c, d);
			break;
		case 0x2:
			tmp->min = 0;
			tmp->max = b;
			break;
		case 0x8:
			tmp->min = 0;
			tmp->max = d;
			break;
		case 0xa:
			tmp->min = minAND(a, -1, c, -1);
			tmp->max = MAX(b, d);
			break;
		case 0xb:
			tmp->min = minAND(a, -1, c, d);
			tmp->max = maxAND(0, b, c, d);
			break;
		case 0xe:
			tmp->min = minAND(a, b, c, -1);
			tmp->max = maxAND(a, b, 0, d);
			break;
	}
}

static inline bool crex_abs_range(
		crex_long min, crex_long max, crex_long *abs_min, crex_long *abs_max) {
	if (min == CREX_LONG_MIN) {
		/* Cannot take absolute value of LONG_MIN  */
		return 0;
	}

	if (min >= 0) {
		*abs_min = min;
		*abs_max = max;
	} else if (max <= 0) {
		*abs_min = -max;
		*abs_max = -min;
	} else {
		/* Range crossing zero */
		*abs_min = 0;
		*abs_max = MAX(max, -min);
	}

	return 1;
}

static inline crex_long safe_shift_left(crex_long n, crex_long s) {
	return (crex_long) ((crex_ulong) n << (crex_ulong) s);
}

static inline bool shift_left_overflows(crex_long n, crex_long s) {
	/* This considers shifts that shift in the sign bit to be overflowing as well */
	if (n >= 0) {
		return s >= SIZEOF_CREX_LONG * 8 - 1 || safe_shift_left(n, s) < n;
	} else {
		return s >= SIZEOF_CREX_LONG * 8 || safe_shift_left(n, s) > n;
	}
}

/* If b does not divide a exactly, return the two adjacent values between which the real result
 * lies. */
static void float_div(crex_long a, crex_long b, crex_long *r1, crex_long *r2) {
	*r1 = *r2 = a / b;
	if (a % b != 0) {
		if (*r2 < 0) {
			(*r2)--;
		} else {
			(*r2)++;
		}
	}
}

static bool crex_inference_calc_binary_op_range(
		const crex_op_array *op_array, const crex_ssa *ssa,
		const crex_op *opline, const crex_ssa_op *ssa_op, uint8_t opcode, crex_ssa_range *tmp) {
	crex_long op1_min, op2_min, op1_max, op2_max, t1, t2, t3, t4;

	switch (opcode) {
		case CREX_ADD:
			if (OP1_HAS_RANGE() && OP2_HAS_RANGE()) {
				op1_min = OP1_MIN_RANGE();
				op2_min = OP2_MIN_RANGE();
				op1_max = OP1_MAX_RANGE();
				op2_max = OP2_MAX_RANGE();
				if (OP1_RANGE_UNDERFLOW() ||
					OP2_RANGE_UNDERFLOW() ||
					crex_add_will_overflow(op1_min, op2_min)) {
					tmp->underflow = 1;
					tmp->min = CREX_LONG_MIN;
				} else {
					tmp->min = op1_min + op2_min;
				}
				if (OP1_RANGE_OVERFLOW() ||
					OP2_RANGE_OVERFLOW() ||
					crex_add_will_overflow(op1_max, op2_max)) {
					tmp->overflow = 1;
					tmp->max = CREX_LONG_MAX;
				} else {
					tmp->max = op1_max + op2_max;
				}
				return 1;
			}
			break;
		case CREX_SUB:
			if (OP1_HAS_RANGE() && OP2_HAS_RANGE()) {
				op1_min = OP1_MIN_RANGE();
				op2_min = OP2_MIN_RANGE();
				op1_max = OP1_MAX_RANGE();
				op2_max = OP2_MAX_RANGE();
				if (OP1_RANGE_UNDERFLOW() ||
					OP2_RANGE_OVERFLOW() ||
					crex_sub_will_overflow(op1_min, op2_max)) {
					tmp->underflow = 1;
					tmp->min = CREX_LONG_MIN;
				} else {
					tmp->min = op1_min - op2_max;
				}
				if (OP1_RANGE_OVERFLOW() ||
					OP2_RANGE_UNDERFLOW() ||
					crex_sub_will_overflow(op1_max, op2_min)) {
					tmp->overflow = 1;
					tmp->max = CREX_LONG_MAX;
				} else {
					tmp->max = op1_max - op2_min;
				}
				return 1;
			}
			break;
		case CREX_MUL:
			if (OP1_HAS_RANGE() && OP2_HAS_RANGE()) {
				double dummy;
				crex_long t1_overflow, t2_overflow, t3_overflow, t4_overflow;
				op1_min = OP1_MIN_RANGE();
				op2_min = OP2_MIN_RANGE();
				op1_max = OP1_MAX_RANGE();
				op2_max = OP2_MAX_RANGE();
				/* Suppress uninit variable warnings, these will only be used if the overflow
				 * flags are all false. */
				t1 = t2 = t3 = t4 = 0;
				CREX_SIGNED_MULTIPLY_LONG(op1_min, op2_min, t1, dummy, t1_overflow);
				CREX_SIGNED_MULTIPLY_LONG(op1_min, op2_max, t2, dummy, t2_overflow);
				CREX_SIGNED_MULTIPLY_LONG(op1_max, op2_min, t3, dummy, t3_overflow);
				CREX_SIGNED_MULTIPLY_LONG(op1_max, op2_max, t4, dummy, t4_overflow);
				(void) dummy;

				// FIXME: more careful overflow checks?
				if (OP1_RANGE_UNDERFLOW() || OP2_RANGE_UNDERFLOW() ||
					OP1_RANGE_OVERFLOW() || OP2_RANGE_OVERFLOW()  ||
					t1_overflow || t2_overflow || t3_overflow || t4_overflow
				) {
					tmp->underflow = 1;
					tmp->overflow = 1;
					tmp->min = CREX_LONG_MIN;
					tmp->max = CREX_LONG_MAX;
				} else {
					tmp->min = MIN(MIN(t1, t2), MIN(t3, t4));
					tmp->max = MAX(MAX(t1, t2), MAX(t3, t4));
				}
				return 1;
			}
			break;
		case CREX_DIV:
			if (OP1_HAS_RANGE() && OP2_HAS_RANGE()) {
				op1_min = OP1_MIN_RANGE();
				op2_min = OP2_MIN_RANGE();
				op1_max = OP1_MAX_RANGE();
				op2_max = OP2_MAX_RANGE();

				/* If op2 crosses zero, then floating point values close to zero might be
				 * possible, which will result in arbitrarily large results (overflow). Also
				 * avoid dividing LONG_MIN by -1, which is UB. */
				if (OP1_RANGE_UNDERFLOW() || OP2_RANGE_UNDERFLOW() ||
					OP1_RANGE_OVERFLOW() || OP2_RANGE_OVERFLOW() ||
					(op2_min <= 0 && op2_max >= 0) ||
					(op1_min == CREX_LONG_MIN && op2_max == -1)
				) {
					tmp->underflow = 1;
					tmp->overflow = 1;
					tmp->min = CREX_LONG_MIN;
					tmp->max = CREX_LONG_MAX;
				} else {
					crex_long t1_, t2_, t3_, t4_;
					float_div(op1_min, op2_min, &t1, &t1_);
					float_div(op1_min, op2_max, &t2, &t2_);
					float_div(op1_max, op2_min, &t3, &t3_);
					float_div(op1_max, op2_max, &t4, &t4_);

					tmp->min = MIN(MIN(MIN(t1, t2), MIN(t3, t4)), MIN(MIN(t1_, t2_), MIN(t3_, t4_)));
					tmp->max = MAX(MAX(MAX(t1, t2), MAX(t3, t4)), MAX(MAX(t1_, t2_), MAX(t3_, t4_)));
				}
				return 1;
			}
			break;
		case CREX_MOD:
			if (OP1_HAS_RANGE() && OP2_HAS_RANGE()) {
				if (OP1_RANGE_UNDERFLOW() ||
					OP2_RANGE_UNDERFLOW() ||
					OP1_RANGE_OVERFLOW()  ||
					OP2_RANGE_OVERFLOW()) {
					tmp->min = CREX_LONG_MIN;
					tmp->max = CREX_LONG_MAX;
				} else {
					crex_long op2_abs_min, op2_abs_max;

					op1_min = OP1_MIN_RANGE();
					op2_min = OP2_MIN_RANGE();
					op1_max = OP1_MAX_RANGE();
					op2_max = OP2_MAX_RANGE();
					if (!crex_abs_range(op2_min, op2_max, &op2_abs_min, &op2_abs_max)) {
						break;
					}

					if (op2_abs_max == 0) {
						/* Always modulus by zero, nothing we can do */
						break;
					}
					if (op2_abs_min == 0) {
						/* Ignore the modulus by zero case, which will throw */
						op2_abs_min++;
					}

					if (op1_min >= 0) {
						tmp->min = op1_max < op2_abs_min ? op1_min : 0;
						tmp->max = MIN(op1_max, op2_abs_max - 1);
					} else if (op1_max <= 0) {
						tmp->min = MAX(op1_min, -op2_abs_max + 1);
						tmp->max = op1_min > -op2_abs_min ? op1_max : 0;
					} else {
						tmp->min = MAX(op1_min, -op2_abs_max + 1);
						tmp->max = MIN(op1_max, op2_abs_max - 1);
					}
				}
				return 1;
			}
			break;
		case CREX_SL:
			if (OP1_HAS_RANGE() && OP2_HAS_RANGE()) {
				if (OP1_RANGE_UNDERFLOW() ||
					OP2_RANGE_UNDERFLOW() ||
					OP1_RANGE_OVERFLOW() ||
					OP2_RANGE_OVERFLOW()) {
					tmp->min = CREX_LONG_MIN;
					tmp->max = CREX_LONG_MAX;
				} else {
					op1_min = OP1_MIN_RANGE();
					op2_min = OP2_MIN_RANGE();
					op1_max = OP1_MAX_RANGE();
					op2_max = OP2_MAX_RANGE();

					/* Shifts by negative numbers will throw, ignore them */
					if (op2_min < 0) {
						op2_min = 0;
					}
					if (op2_max < 0) {
						op2_max = 0;
					}

					if (shift_left_overflows(op1_min, op2_max)
							|| shift_left_overflows(op1_max, op2_max)) {
						tmp->min = CREX_LONG_MIN;
						tmp->max = CREX_LONG_MAX;
					} else {
						t1 = safe_shift_left(op1_min, op2_min);
						t2 = safe_shift_left(op1_min, op2_max);
						t3 = safe_shift_left(op1_max, op2_min);
						t4 = safe_shift_left(op1_max, op2_max);
						tmp->min = MIN(MIN(t1, t2), MIN(t3, t4));
						tmp->max = MAX(MAX(t1, t2), MAX(t3, t4));
					}
				}
				return 1;
			}
			break;
		case CREX_SR:
			if (OP1_HAS_RANGE() && OP2_HAS_RANGE()) {
				if (OP1_RANGE_UNDERFLOW() ||
					OP2_RANGE_UNDERFLOW() ||
					OP1_RANGE_OVERFLOW() ||
					OP2_RANGE_OVERFLOW()) {
					tmp->min = CREX_LONG_MIN;
					tmp->max = CREX_LONG_MAX;
				} else {
					op1_min = OP1_MIN_RANGE();
					op2_min = OP2_MIN_RANGE();
					op1_max = OP1_MAX_RANGE();
					op2_max = OP2_MAX_RANGE();

					/* Shifts by negative numbers will throw, ignore them */
					if (op2_min < 0) {
						op2_min = 0;
					}
					if (op2_max < 0) {
						op2_max = 0;
					}

					/* Shifts by more than the integer size will be 0 or -1 */
					if (op2_min >= SIZEOF_CREX_LONG * 8) {
						op2_min = SIZEOF_CREX_LONG * 8 - 1;
					}
					if (op2_max >= SIZEOF_CREX_LONG * 8) {
						op2_max = SIZEOF_CREX_LONG * 8 - 1;
					}

					t1 = op1_min >> op2_min;
					t2 = op1_min >> op2_max;
					t3 = op1_max >> op2_min;
					t4 = op1_max >> op2_max;
					tmp->min = MIN(MIN(t1, t2), MIN(t3, t4));
					tmp->max = MAX(MAX(t1, t2), MAX(t3, t4));
				}
				return 1;
			}
			break;
		case CREX_BW_OR:
			if (OP1_HAS_RANGE() && OP2_HAS_RANGE()) {
				if (OP1_RANGE_UNDERFLOW() ||
					OP2_RANGE_UNDERFLOW() ||
					OP1_RANGE_OVERFLOW() ||
					OP2_RANGE_OVERFLOW()) {
					tmp->min = CREX_LONG_MIN;
					tmp->max = CREX_LONG_MAX;
				} else {
					op1_min = OP1_MIN_RANGE();
					op2_min = OP2_MIN_RANGE();
					op1_max = OP1_MAX_RANGE();
					op2_max = OP2_MAX_RANGE();
					crex_ssa_range_or(op1_min, op1_max, op2_min, op2_max, tmp);
				}
				return 1;
			}
			break;
		case CREX_BW_AND:
			if (OP1_HAS_RANGE() && OP2_HAS_RANGE()) {
				if (OP1_RANGE_UNDERFLOW() ||
					OP2_RANGE_UNDERFLOW() ||
					OP1_RANGE_OVERFLOW() ||
					OP2_RANGE_OVERFLOW()) {
					tmp->min = CREX_LONG_MIN;
					tmp->max = CREX_LONG_MAX;
				} else {
					op1_min = OP1_MIN_RANGE();
					op2_min = OP2_MIN_RANGE();
					op1_max = OP1_MAX_RANGE();
					op2_max = OP2_MAX_RANGE();
					crex_ssa_range_and(op1_min, op1_max, op2_min, op2_max, tmp);
				}
				return 1;
			}
			break;
		case CREX_BW_XOR:
			// TODO
			break;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
	return 0;
}

static bool crex_inference_calc_range(const crex_op_array *op_array, const crex_ssa *ssa, int var, int widening, int narrowing, crex_ssa_range *tmp)
{
	uint32_t line;
	const crex_op *opline;
	const crex_ssa_op *ssa_op;

	if (ssa->vars[var].definition_phi) {
		const crex_ssa_phi *p = ssa->vars[var].definition_phi;
		int i;

		tmp->underflow = 0;
		tmp->min = CREX_LONG_MAX;
		tmp->max = CREX_LONG_MIN;
		tmp->overflow = 0;
		if (p->pi >= 0 && p->has_range_constraint) {
			const crex_ssa_range_constraint *constraint = &p->constraint.range;
			if (constraint->negative) {
				int src1 = p->sources[0];

				if (ssa->var_info[src1].has_range) {
					*tmp = ssa->var_info[src1].range;
					if (constraint->range.min == constraint->range.max
					 && !constraint->range.underflow
					 && !constraint->range.overflow
					 && p->constraint.range.min_ssa_var < 0
					 && p->constraint.range.max_ssa_var < 0
					 && ssa->vars[src1].definition >= 0) {
						/* Check for constrained induction variable */
						line = ssa->vars[src1].definition;
						opline = op_array->opcodes + line;
						switch (opline->opcode) {
							case CREX_PRE_DEC:
							case CREX_POST_DEC:
								if (!tmp->underflow) {
									const crex_ssa_phi *p = ssa->vars[ssa->ops[line].op1_use].definition_phi;

									if (p && p->pi < 0
									 && ssa->cfg.blocks[p->block].predecessors_count == 2
									 && p->sources[1] == var
									 && ssa->var_info[p->sources[0]].has_range
									 && ssa->var_info[p->sources[0]].range.min > constraint->range.max) {
										tmp->min = constraint->range.max + 1;
									}
								}
								break;
							case CREX_PRE_INC:
							case CREX_POST_INC:
								if (!tmp->overflow) {
									const crex_ssa_phi *p = ssa->vars[ssa->ops[line].op1_use].definition_phi;

									if (p && p->pi < 0
									 && ssa->cfg.blocks[p->block].predecessors_count == 2
									 && p->sources[1] == var
									 && ssa->var_info[p->sources[0]].has_range
									 && ssa->var_info[p->sources[0]].range.max < constraint->range.min) {
										tmp->max = constraint->range.min - 1;
									}
								}
								break;
						}
					}
				} else if (narrowing) {
					tmp->underflow = 1;
					tmp->min = CREX_LONG_MIN;
					tmp->max = CREX_LONG_MAX;
					tmp->overflow = 1;
				}

#ifdef NEG_RANGE
				if (constraint->min_ssa_var < 0 &&
				    constraint->max_ssa_var < 0 &&
				    ssa->var_info[p->ssa_var].has_range) {
					LOG_NEG_RANGE("%s() #%d [%ld..%ld] -> [%ld..%ld]?\n",
						ZSTR_VAL(op_array->function_name),
						p->ssa_var,
						ssa->var_info[p->ssa_var].range.min,
						ssa->var_info[p->ssa_var].range.max,
						tmp->min,
						tmp->max);
					if (constraint->negative == NEG_USE_LT &&
					    tmp->max >= constraint->range.min) {
						tmp->overflow = 0;
						tmp->max = constraint->range.min - 1;
						LOG_NEG_RANGE("  => [%ld..%ld]\n", tmp->min, tmp->max);
					} else if (constraint->negative == NEG_USE_GT &&
					           tmp->min <= constraint->range.max) {
						tmp->underflow = 0;
						tmp->min = constraint->range.max + 1;
						LOG_NEG_RANGE("  => [%ld..%ld]\n", tmp->min, tmp->max);
					}
				}
#endif
			} else if (ssa->var_info[p->sources[0]].has_range) {
				/* intersection */
				*tmp = ssa->var_info[p->sources[0]].range;
				if (constraint->min_ssa_var < 0) {
					tmp->underflow = constraint->range.underflow && tmp->underflow;
					tmp->min = MAX(constraint->range.min, tmp->min);
#ifdef SYM_RANGE
				} else if (narrowing && ssa->var_info[constraint->min_ssa_var].has_range) {
					tmp->underflow = ssa->var_info[constraint->min_ssa_var].range.underflow && tmp->underflow;
					if (!add_will_overflow(ssa->var_info[constraint->min_ssa_var].range.min, constraint->range.min)) {
						tmp->min = MAX(ssa->var_info[constraint->min_ssa_var].range.min + constraint->range.min, tmp->min);
					}
#endif
				}
				if (constraint->max_ssa_var < 0) {
					tmp->max = MIN(constraint->range.max, tmp->max);
					tmp->overflow = constraint->range.overflow && tmp->overflow;
#ifdef SYM_RANGE
				} else if (narrowing && ssa->var_info[constraint->max_ssa_var].has_range) {
					if (!add_will_overflow(ssa->var_info[constraint->max_ssa_var].range.max, constraint->range.max)) {
						tmp->max = MIN(ssa->var_info[constraint->max_ssa_var].range.max + constraint->range.max, tmp->max);
					}
					tmp->overflow = ssa->var_info[constraint->max_ssa_var].range.overflow && tmp->overflow;
#endif
				}
			} else if (narrowing) {
				if (constraint->min_ssa_var < 0) {
					tmp->underflow = constraint->range.underflow;
					tmp->min = constraint->range.min;
#ifdef SYM_RANGE
				} else if (narrowing && ssa->var_info[constraint->min_ssa_var].has_range) {
					if (add_will_overflow(ssa->var_info[constraint->min_ssa_var].range.min, constraint->range.min)) {
						tmp->underflow = 1;
						tmp->min = CREX_LONG_MIN;
					} else {
						tmp->underflow = ssa->var_info[constraint->min_ssa_var].range.underflow;
						tmp->min = ssa->var_info[constraint->min_ssa_var].range.min + constraint->range.min;
					}
#endif
				} else {
					tmp->underflow = 1;
					tmp->min = CREX_LONG_MIN;
				}
				if (constraint->max_ssa_var < 0) {
					tmp->max = constraint->range.max;
					tmp->overflow = constraint->range.overflow;
#ifdef SYM_RANGE
				} else if (narrowing && ssa->var_info[constraint->max_ssa_var].has_range) {
					if (add_will_overflow(ssa->var_info[constraint->max_ssa_var].range.max, constraint->range.max)) {
						tmp->overflow = 1;
						tmp->max = CREX_LONG_MAX;
					} else {
						tmp->max = ssa->var_info[constraint->max_ssa_var].range.max + constraint->range.max;
						tmp->overflow = ssa->var_info[constraint->max_ssa_var].range.overflow;
					}
#endif
				} else {
					tmp->max = CREX_LONG_MAX;
					tmp->overflow = 1;
				}
			}
		} else {
			for (i = 0; i < ssa->cfg.blocks[p->block].predecessors_count; i++) {
				CREX_ASSERT(p->sources[i] >= 0);
				if (ssa->var_info[p->sources[i]].has_range) {
					/* union */
					tmp->underflow |= ssa->var_info[p->sources[i]].range.underflow;
					tmp->min = MIN(tmp->min, ssa->var_info[p->sources[i]].range.min);
					tmp->max = MAX(tmp->max, ssa->var_info[p->sources[i]].range.max);
					tmp->overflow |= ssa->var_info[p->sources[i]].range.overflow;
				} else if (narrowing) {
					tmp->underflow = 1;
					tmp->min = CREX_LONG_MIN;
					tmp->max = CREX_LONG_MAX;
					tmp->overflow = 1;
				}
			}
		}
		return (tmp->min <= tmp->max);
	} else if (ssa->vars[var].definition < 0) {
		return 0;
	}
	line = ssa->vars[var].definition;
	opline = op_array->opcodes + line;
	ssa_op = &ssa->ops[line];

	return crex_inference_propagate_range(op_array, ssa, opline, ssa_op, var, tmp);
}

CREX_API bool crex_inference_propagate_range(const crex_op_array *op_array, const crex_ssa *ssa, const crex_op *opline, const crex_ssa_op* ssa_op, int var, crex_ssa_range *tmp)
{
	tmp->underflow = 0;
	tmp->overflow = 0;
	switch (opline->opcode) {
		case CREX_ADD:
		case CREX_SUB:
		case CREX_MUL:
		case CREX_DIV:
		case CREX_MOD:
		case CREX_SL:
		case CREX_SR:
		case CREX_BW_OR:
		case CREX_BW_AND:
		case CREX_BW_XOR:
			if (ssa_op->result_def == var) {
				return crex_inference_calc_binary_op_range(
					op_array, ssa, opline, ssa_op, opline->opcode, tmp);
			}
			break;

		case CREX_BW_NOT:
			if (ssa_op->result_def == var) {
				if (OP1_HAS_RANGE()) {
					if (OP1_RANGE_UNDERFLOW() ||
					    OP1_RANGE_OVERFLOW()) {
						tmp->min = CREX_LONG_MIN;
						tmp->max = CREX_LONG_MAX;
					} else {
						crex_long op1_min = OP1_MIN_RANGE();
						crex_long op1_max = OP1_MAX_RANGE();
						tmp->min = ~op1_max;
						tmp->max = ~op1_min;
					}
					return 1;
				}
			}
			break;
		case CREX_CAST:
			if (ssa_op->op1_def == var) {
				if (ssa_op->op1_def >= 0) {
					if (OP1_HAS_RANGE()) {
						tmp->underflow = OP1_RANGE_UNDERFLOW();
						tmp->min = OP1_MIN_RANGE();
						tmp->max = OP1_MAX_RANGE();
						tmp->overflow  = OP1_RANGE_OVERFLOW();
						return 1;
					}
				}
			} else if (ssa_op->result_def == var) {
				if (opline->extended_value == IS_LONG) {
					if (OP1_HAS_RANGE()) {
						tmp->min = OP1_MIN_RANGE();
						tmp->max = OP1_MAX_RANGE();
						return 1;
					} else {
						tmp->min = CREX_LONG_MIN;
						tmp->max = CREX_LONG_MAX;
						return 1;
					}
				}
			}
			break;
		case CREX_QM_ASSIGN:
		case CREX_JMP_SET:
		case CREX_COALESCE:
		case CREX_COPY_TMP:
			if (ssa_op->op1_def == var) {
				if (ssa_op->op1_def >= 0) {
					if (OP1_HAS_RANGE()) {
						tmp->underflow = OP1_RANGE_UNDERFLOW();
						tmp->min = OP1_MIN_RANGE();
						tmp->max = OP1_MAX_RANGE();
						tmp->overflow  = OP1_RANGE_OVERFLOW();
						return 1;
					}
				}
			}
			if (ssa_op->result_def == var) {
				if (OP1_HAS_RANGE()) {
					tmp->min = OP1_MIN_RANGE();
					tmp->max = OP1_MAX_RANGE();
					tmp->underflow = OP1_RANGE_UNDERFLOW();
					tmp->overflow  = OP1_RANGE_OVERFLOW();
					return 1;
				}
			}
			break;
		case CREX_SEND_VAR:
			if (ssa_op->op1_def == var) {
				if (ssa_op->op1_def >= 0) {
					if (OP1_HAS_RANGE()) {
						tmp->underflow = OP1_RANGE_UNDERFLOW();
						tmp->min = OP1_MIN_RANGE();
						tmp->max = OP1_MAX_RANGE();
						tmp->overflow  = OP1_RANGE_OVERFLOW();
						return 1;
					}
				}
			}
			break;
		case CREX_PRE_INC:
			if (ssa_op->op1_def == var || ssa_op->result_def == var) {
				if (OP1_HAS_RANGE()) {
					tmp->min = OP1_MIN_RANGE();
					tmp->max = OP1_MAX_RANGE();
					tmp->underflow = OP1_RANGE_UNDERFLOW();
					tmp->overflow = OP1_RANGE_OVERFLOW();
					if (tmp->max < CREX_LONG_MAX) {
						tmp->max++;
					} else {
						tmp->overflow = 1;
					}
					if (tmp->min < CREX_LONG_MAX && !tmp->underflow) {
						tmp->min++;
					}
					return 1;
				}
			}
			break;
		case CREX_PRE_DEC:
			if (ssa_op->op1_def == var || ssa_op->result_def == var) {
				if (OP1_HAS_RANGE()) {
					tmp->min = OP1_MIN_RANGE();
					tmp->max = OP1_MAX_RANGE();
					tmp->underflow = OP1_RANGE_UNDERFLOW();
					tmp->overflow = OP1_RANGE_OVERFLOW();
					if (tmp->min > CREX_LONG_MIN) {
						tmp->min--;
					} else {
						tmp->underflow = 1;
					}
					if (tmp->max > CREX_LONG_MIN && !tmp->overflow) {
						tmp->max--;
					}
					return 1;
				}
			}
			break;
		case CREX_POST_INC:
			if (ssa_op->op1_def == var || ssa_op->result_def == var) {
				if (OP1_HAS_RANGE()) {
					tmp->min = OP1_MIN_RANGE();
					tmp->max = OP1_MAX_RANGE();
					tmp->underflow = OP1_RANGE_UNDERFLOW();
					tmp->overflow = OP1_RANGE_OVERFLOW();
					if (ssa_op->result_def == var) {
						return 1;
					}
					if (tmp->max < CREX_LONG_MAX) {
						tmp->max++;
					} else {
						tmp->overflow = 1;
					}
					if (tmp->min < CREX_LONG_MAX && !tmp->underflow) {
						tmp->min++;
					}
					return 1;
				}
			}
			break;
		case CREX_POST_DEC:
			if (ssa_op->op1_def == var || ssa_op->result_def == var) {
				if (OP1_HAS_RANGE()) {
					tmp->min = OP1_MIN_RANGE();
					tmp->max = OP1_MAX_RANGE();
					tmp->underflow = OP1_RANGE_UNDERFLOW();
					tmp->overflow = OP1_RANGE_OVERFLOW();
					if (ssa_op->result_def == var) {
						return 1;
					}
					if (tmp->min > CREX_LONG_MIN) {
						tmp->min--;
					} else {
						tmp->underflow = 1;
					}
					if (tmp->max > CREX_LONG_MIN && !tmp->overflow) {
						tmp->max--;
					}
					return 1;
				}
			}
			break;
		case CREX_UNSET_DIM:
		case CREX_UNSET_OBJ:
			if (ssa_op->op1_def == var) {
				/* If op1 is scalar, UNSET_DIM and UNSET_OBJ have no effect, so we can keep
				 * the previous ranges. */
				if (OP1_HAS_RANGE()) {
					tmp->min = OP1_MIN_RANGE();
					tmp->max = OP1_MAX_RANGE();
					tmp->underflow = OP1_RANGE_UNDERFLOW();
					tmp->overflow  = OP1_RANGE_OVERFLOW();
					return 1;
				}
			}
			break;
		case CREX_ASSIGN:
			if (ssa_op->op1_def == var || ssa_op->op2_def == var || ssa_op->result_def == var) {
				if (OP2_HAS_RANGE()) {
					tmp->min = OP2_MIN_RANGE();
					tmp->max = OP2_MAX_RANGE();
					tmp->underflow = OP2_RANGE_UNDERFLOW();
					tmp->overflow  = OP2_RANGE_OVERFLOW();
					return 1;
				}
			}
			break;
		case CREX_ASSIGN_DIM:
		case CREX_ASSIGN_OBJ:
		case CREX_ASSIGN_STATIC_PROP:
		case CREX_ASSIGN_DIM_OP:
		case CREX_ASSIGN_OBJ_OP:
		case CREX_ASSIGN_STATIC_PROP_OP:
			if ((ssa_op+1)->op1_def == var) {
				opline++;
				ssa_op++;
				if (OP1_HAS_RANGE()) {
					tmp->min = OP1_MIN_RANGE();
					tmp->max = OP1_MAX_RANGE();
					tmp->underflow = OP1_RANGE_UNDERFLOW();
					tmp->overflow  = OP1_RANGE_OVERFLOW();
				}
				return 1;
			}
			break;
		case CREX_ASSIGN_OP:
			if (opline->extended_value != CREX_CONCAT
			 && opline->extended_value != CREX_POW) {
				if (ssa_op->op1_def == var || ssa_op->result_def == var) {
					return crex_inference_calc_binary_op_range(
						op_array, ssa, opline, ssa_op,
						opline->extended_value, tmp);
				}
			}
			break;
		case CREX_OP_DATA:
			if (ssa_op->op1_def == var) {
				if ((opline-1)->opcode == CREX_ASSIGN_DIM ||
				    (opline-1)->opcode == CREX_ASSIGN_OBJ ||
				    (opline-1)->opcode == CREX_ASSIGN_STATIC_PROP ||
				    (opline-1)->opcode == CREX_ASSIGN_DIM_OP ||
				    (opline-1)->opcode == CREX_ASSIGN_OBJ_OP ||
				    (opline-1)->opcode == CREX_ASSIGN_STATIC_PROP_OP) {
					if (OP1_HAS_RANGE()) {
						tmp->min = OP1_MIN_RANGE();
						tmp->max = OP1_MAX_RANGE();
						tmp->underflow = OP1_RANGE_UNDERFLOW();
						tmp->overflow  = OP1_RANGE_OVERFLOW();
						return 1;
					}
				}
				break;
			}
			break;
		case CREX_RECV:
		case CREX_RECV_INIT:
			if (ssa_op->result_def == var) {
				if (op_array->arg_info &&
				    opline->op1.num <= op_array->num_args) {
					crex_type type = op_array->arg_info[opline->op1.num-1].type;
					uint32_t mask = CREX_TYPE_PURE_MASK_WITHOUT_NULL(type);
					if (mask == MAY_BE_LONG) {
						tmp->underflow = 0;
						tmp->min = CREX_LONG_MIN;
						tmp->max = CREX_LONG_MAX;
						tmp->overflow = 0;
						return 1;
					}
				}
			}
			break;
		case CREX_STRLEN:
			if (ssa_op->result_def == var) {
#if SIZEOF_CREX_LONG == 4
				/* The length of a string is a non-negative integer. However, on 32-bit
				 * platforms overflows into negative lengths may occur, so it's better
				 * to not assume any particular range. */
				tmp->min = CREX_LONG_MIN;
#else
				tmp->min = 0;
#endif
				tmp->max = CREX_LONG_MAX;
				return 1;
			}
			break;
		case CREX_FUNC_NUM_ARGS:
			tmp->min = 0;
			tmp->max = CREX_LONG_MAX;
			return 1;
		case CREX_COUNT:
			/* count() on Countable objects may return negative numbers */
			tmp->min = CREX_LONG_MIN;
			tmp->max = CREX_LONG_MAX;
			return 1;
		case CREX_DO_FCALL:
		case CREX_DO_ICALL:
		case CREX_DO_UCALL:
		case CREX_DO_FCALL_BY_NAME:
			if (ssa_op->result_def == var) {
				const crex_func_info *func_info = CREX_FUNC_INFO(op_array);
				const crex_call_info *call_info;
				if (!func_info || !func_info->call_map) {
					break;
				}

				call_info = func_info->call_map[opline - op_array->opcodes];
				if (!call_info || call_info->is_prototype) {
					break;
				}
				if (call_info->callee_func->type == CREX_USER_FUNCTION) {
					func_info = CREX_FUNC_INFO(&call_info->callee_func->op_array);
					if (func_info && func_info->return_info.has_range) {
						*tmp = func_info->return_info.range;
						return 1;
					}
				}
//TODO: we can't use type inference for internal functions at this point ???
#if 0
					uint32_t type;

					type = crex_get_func_info(call_info, ssa);
					if (!(type & (MAY_BE_ANY - (MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_LONG)))) {
						tmp->underflow = 0;
						tmp->min = 0;
						tmp->max = 0;
						tmp->overflow = 0;
						if (type & MAY_BE_LONG) {
							tmp->min = CREX_LONG_MIN;
							tmp->max = CREX_LONG_MAX;
						} else if (type & MAY_BE_TRUE) {
							if (!(type & (MAY_BE_NULL|MAY_BE_FALSE))) {
								tmp->min = 1;
							}
							tmp->max = 1;
						}
						return 1;
					}
#endif
			}
			break;
		// FIXME: support for more opcodes
		default:
			break;
	}
	return 0;
}

static void crex_inference_init_range(const crex_op_array *op_array, crex_ssa *ssa, int var, bool underflow, crex_long min, crex_long max, bool overflow)
{
	if (underflow) {
		min = CREX_LONG_MIN;
	}
	if (overflow) {
		max = CREX_LONG_MAX;
	}
	ssa->var_info[var].has_range = 1;
	ssa->var_info[var].range.underflow = underflow;
	ssa->var_info[var].range.min = min;
	ssa->var_info[var].range.max = max;
	ssa->var_info[var].range.overflow = overflow;
	LOG_SSA_RANGE("  change range (init      SCC %2d) %2d [%s%ld..%ld%s]\n", ssa->vars[var].scc, var, (underflow?"-- ":""), min, max, (overflow?" ++":""));
}

static bool crex_inference_widening_meet(crex_ssa_var_info *var_info, crex_ssa_range *r)
{
	if (!var_info->has_range) {
		var_info->has_range = 1;
	} else {
		if (r->underflow ||
		    var_info->range.underflow ||
		    r->min < var_info->range.min) {
			r->underflow = 1;
			r->min = CREX_LONG_MIN;
		}
		if (r->overflow ||
		    var_info->range.overflow ||
		    r->max > var_info->range.max) {
			r->overflow = 1;
			r->max = CREX_LONG_MAX;
		}
		if (var_info->range.min == r->min &&
		    var_info->range.max == r->max &&
		    var_info->range.underflow == r->underflow &&
		    var_info->range.overflow == r->overflow) {
			return 0;
		}
	}
	var_info->range = *r;
	return 1;
}

static bool crex_ssa_range_widening(const crex_op_array *op_array, crex_ssa *ssa, int var, int scc)
{
	crex_ssa_range tmp;

	if (crex_inference_calc_range(op_array, ssa, var, 1, 0, &tmp)) {
		if (crex_inference_widening_meet(&ssa->var_info[var], &tmp)) {
			LOG_SSA_RANGE("  change range (widening  SCC %2d) %2d [%s%ld..%ld%s]\n", scc, var, (tmp.underflow?"-- ":""), tmp.min, tmp.max, (tmp.overflow?" ++":""));
			return 1;
		}
	}
	return 0;
}

static bool crex_inference_narrowing_meet(crex_ssa_var_info *var_info, crex_ssa_range *r)
{
	if (!var_info->has_range) {
		var_info->has_range = 1;
	} else {
		if (!r->underflow &&
		    !var_info->range.underflow &&
		    var_info->range.min < r->min) {
			r->min = var_info->range.min;
		}
		if (!r->overflow &&
		    !var_info->range.overflow &&
		    var_info->range.max > r->max) {
			r->max = var_info->range.max;
		}
		if (r->underflow) {
			r->min = CREX_LONG_MIN;
		}
		if (r->overflow) {
			r->max = CREX_LONG_MAX;
		}
		if (var_info->range.min == r->min &&
		    var_info->range.max == r->max &&
		    var_info->range.underflow == r->underflow &&
		    var_info->range.overflow == r->overflow) {
			return 0;
		}
	}
	var_info->range = *r;
	return 1;
}

static bool crex_ssa_range_narrowing(const crex_op_array *op_array, crex_ssa *ssa, int var, int scc)
{
	crex_ssa_range tmp;

	if (crex_inference_calc_range(op_array, ssa, var, 0, 1, &tmp)) {
		if (crex_inference_narrowing_meet(&ssa->var_info[var], &tmp)) {
			LOG_SSA_RANGE("  change range (narrowing SCC %2d) %2d [%s%ld..%ld%s]\n", scc, var, (tmp.underflow?"-- ":""), tmp.min, tmp.max, (tmp.overflow?" ++":""));
			return 1;
		}
	}
	return 0;
}

#ifdef NEG_RANGE
# define CHECK_INNER_CYCLE(var2) \
	do { \
		if (ssa->vars[var2].scc == ssa->vars[var].scc && \
		    !ssa->vars[var2].scc_entry && \
		    !crex_bitset_in(visited, var2) && \
			crex_check_inner_cycles(op_array, ssa, worklist, visited, var2)) { \
			return 1; \
		} \
	} while (0)

static bool crex_check_inner_cycles(const crex_op_array *op_array, crex_ssa *ssa, crex_bitset worklist, crex_bitset visited, int var)
{
	if (crex_bitset_in(worklist, var)) {
		return 1;
	}
	crex_bitset_incl(worklist, var);
	FOR_EACH_VAR_USAGE(var, CHECK_INNER_CYCLE);
	crex_bitset_incl(visited, var);
	return 0;
}
#endif

static void crex_infer_ranges_warmup(const crex_op_array *op_array, crex_ssa *ssa, const int *scc_var, const int *next_scc_var, int scc)
{
	int worklist_len = crex_bitset_len(ssa->vars_count);
	int j, n;
	crex_ssa_range tmp;
	ALLOCA_FLAG(use_heap)
	crex_bitset worklist = do_alloca(sizeof(crex_ulong) * worklist_len * 2, use_heap);
	crex_bitset visited = worklist + worklist_len;
#ifdef NEG_RANGE
	int has_inner_cycles = 0;

	memset(worklist, 0, sizeof(crex_ulong) * worklist_len);
	memset(visited, 0, sizeof(crex_ulong) * worklist_len);
	j = scc_var[scc];
	while (j >= 0) {
		if (!crex_bitset_in(visited, j) &&
		    crex_check_inner_cycles(op_array, ssa, worklist, visited, j)) {
			has_inner_cycles = 1;
			break;
		}
		j = next_scc_var[j];
	}
#endif

	memset(worklist, 0, sizeof(crex_ulong) * worklist_len);

	for (n = 0; n < RANGE_WARMUP_PASSES; n++) {
		j= scc_var[scc];
		while (j >= 0) {
			if (ssa->vars[j].scc_entry
			 && !(ssa->var_info[j].type & MAY_BE_REF)) {
				crex_bitset_incl(worklist, j);
			}
			j = next_scc_var[j];
		}

		memset(visited, 0, sizeof(crex_ulong) * worklist_len);

		WHILE_WORKLIST(worklist, worklist_len, j) {
			if (crex_inference_calc_range(op_array, ssa, j, 0, 0, &tmp)) {
#ifdef NEG_RANGE
				if (!has_inner_cycles &&
				    ssa->var_info[j].has_range &&
				    ssa->vars[j].definition_phi &&
				    ssa->vars[j].definition_phi->pi >= 0 &&
					ssa->vars[j].definition_phi->has_range_constraint &&
				    ssa->vars[j].definition_phi->constraint.range.negative &&
				    ssa->vars[j].definition_phi->constraint.range.min_ssa_var < 0 &&
				    ssa->vars[j].definition_phi->constraint.range.max_ssa_var < 0) {
					crex_ssa_range_constraint *constraint =
						&ssa->vars[j].definition_phi->constraint.range;
					if (tmp.min == ssa->var_info[j].range.min &&
					    tmp.max == ssa->var_info[j].range.max) {
						if (constraint->negative == NEG_INIT) {
							LOG_NEG_RANGE("#%d INVARIANT\n", j);
							constraint->negative = NEG_INVARIANT;
						}
					} else if (tmp.min == ssa->var_info[j].range.min &&
					           tmp.max == ssa->var_info[j].range.max + 1 &&
					           tmp.max < constraint->range.min) {
						if (constraint->negative == NEG_INIT ||
						    constraint->negative == NEG_INVARIANT) {
							LOG_NEG_RANGE("#%d LT\n", j);
							constraint->negative = NEG_USE_LT;
//???NEG
						} else if (constraint->negative == NEG_USE_GT) {
							LOG_NEG_RANGE("#%d UNKNOWN\n", j);
							constraint->negative = NEG_UNKNOWN;
						}
					} else if (tmp.max == ssa->var_info[j].range.max &&
				               tmp.min == ssa->var_info[j].range.min - 1 &&
					           tmp.min > constraint->range.max) {
						if (constraint->negative == NEG_INIT ||
						    constraint->negative == NEG_INVARIANT) {
							LOG_NEG_RANGE("#%d GT\n", j);
							constraint->negative = NEG_USE_GT;
//???NEG
						} else if (constraint->negative == NEG_USE_LT) {
							LOG_NEG_RANGE("#%d UNKNOWN\n", j);
							constraint->negative = NEG_UNKNOWN;
						}
					} else {
						LOG_NEG_RANGE("#%d UNKNOWN\n", j);
						constraint->negative = NEG_UNKNOWN;
					}
				}
#endif
				if (crex_inference_narrowing_meet(&ssa->var_info[j], &tmp)) {
					LOG_SSA_RANGE("  change range (warmup %2d SCC %2d) %2d [%s%ld..%ld%s]\n", n, scc, j, (tmp.underflow?"-- ":""), tmp.min, tmp.max, (tmp.overflow?" ++":""));
					crex_bitset_incl(visited, j);
					FOR_EACH_VAR_USAGE(j, ADD_SCC_VAR_1);
				}
			}
		} WHILE_WORKLIST_END();
	}
	free_alloca(worklist, use_heap);
}

static void crex_infer_ranges(const crex_op_array *op_array, crex_ssa *ssa) /* {{{ */
{
	int worklist_len = crex_bitset_len(ssa->vars_count);
	crex_bitset worklist;
	int *next_scc_var;
	int *scc_var;
	crex_ssa_phi *p;
	crex_ssa_range tmp;
	int scc, j;
	ALLOCA_FLAG(use_heap);

	worklist = do_alloca(
		CREX_MM_ALIGNED_SIZE(sizeof(crex_ulong) * worklist_len) +
		CREX_MM_ALIGNED_SIZE(sizeof(int) * ssa->vars_count) +
		sizeof(int) * ssa->sccs, use_heap);
	next_scc_var = (int*)((char*)worklist + CREX_MM_ALIGNED_SIZE(sizeof(crex_ulong) * worklist_len));
	scc_var = (int*)((char*)next_scc_var + CREX_MM_ALIGNED_SIZE(sizeof(int) * ssa->vars_count));

	LOG_SSA_RANGE("Range Inference\n");

	/* Create linked lists of SSA variables for each SCC */
	memset(scc_var, -1, sizeof(int) * ssa->sccs);
	for (j = 0; j < ssa->vars_count; j++) {
		if (ssa->vars[j].scc >= 0) {
			next_scc_var[j] = scc_var[ssa->vars[j].scc];
			scc_var[ssa->vars[j].scc] = j;
		}
	}

	for (scc = 0; scc < ssa->sccs; scc++) {
		j = scc_var[scc];
		if (next_scc_var[j] < 0) {
			/* SCC with a single element */
			if (ssa->var_info[j].type & MAY_BE_REF) {
				/* pass */
			} else if (crex_inference_calc_range(op_array, ssa, j, 0, 1, &tmp)) {
				crex_inference_init_range(op_array, ssa, j, tmp.underflow, tmp.min, tmp.max, tmp.overflow);
			} else {
				crex_inference_init_range(op_array, ssa, j, 1, CREX_LONG_MIN, CREX_LONG_MAX, 1);
			}
		} else {
			/* Find SCC entry points */
			memset(worklist, 0, sizeof(crex_ulong) * worklist_len);
			do {
				if (ssa->vars[j].scc_entry
				 && !(ssa->var_info[j].type & MAY_BE_REF)) {
					crex_bitset_incl(worklist, j);
				}
				j = next_scc_var[j];
			} while (j >= 0);

#if RANGE_WARMUP_PASSES > 0
			crex_infer_ranges_warmup(op_array, ssa, scc_var, next_scc_var, scc);
			j = scc_var[scc];
			do {
				if (!(ssa->var_info[j].type & MAY_BE_REF)) {
					crex_bitset_incl(worklist, j);
				}
				j = next_scc_var[j];
			} while (j >= 0);
#endif

			/* widening */
			WHILE_WORKLIST(worklist, worklist_len, j) {
				if (crex_ssa_range_widening(op_array, ssa, j, scc)) {
					FOR_EACH_VAR_USAGE(j, ADD_SCC_VAR);
				}
			} WHILE_WORKLIST_END();

			/* initialize missing ranges */
			for (j = scc_var[scc]; j >= 0; j = next_scc_var[j]) {
				if (!ssa->var_info[j].has_range
				 && !(ssa->var_info[j].type & MAY_BE_REF)) {
					crex_inference_init_range(op_array, ssa, j, 1, CREX_LONG_MIN, CREX_LONG_MAX, 1);
					FOR_EACH_VAR_USAGE(j, ADD_SCC_VAR);
				}
			}

			/* widening (second round) */
			WHILE_WORKLIST(worklist, worklist_len, j) {
				if (crex_ssa_range_widening(op_array, ssa, j, scc)) {
					FOR_EACH_VAR_USAGE(j, ADD_SCC_VAR);
				}
			} WHILE_WORKLIST_END();

			/* Add all SCC entry variables into worklist for narrowing */
			for (j = scc_var[scc]; j >= 0; j = next_scc_var[j]) {
				if (ssa->vars[j].definition_phi
				 && ssa->vars[j].definition_phi->pi < 0
				 && !(ssa->var_info[j].type & MAY_BE_REF)) {
					/* narrowing Phi functions first */
					crex_ssa_range_narrowing(op_array, ssa, j, scc);
				}
				crex_bitset_incl(worklist, j);
			}

			/* narrowing */
			WHILE_WORKLIST(worklist, worklist_len, j) {
				if (crex_ssa_range_narrowing(op_array, ssa, j, scc)) {
					FOR_EACH_VAR_USAGE(j, ADD_SCC_VAR);
#ifdef SYM_RANGE
					/* Process symbolic control-flow constraints */
					p = ssa->vars[j].sym_use_chain;
					while (p) {
						ADD_SCC_VAR(p->ssa_var);
						p = p->sym_use_chain;
					}
#endif
				}
			} WHILE_WORKLIST_END();
		}
	}

	free_alloca(worklist, use_heap);
}
/* }}} */

static uint32_t get_ssa_alias_types(crex_ssa_alias_kind alias) {
	if (alias == HTTP_RESPONSE_HEADER_ALIAS) {
		return MAY_BE_ARRAY | MAY_BE_ARRAY_KEY_LONG | MAY_BE_ARRAY_OF_STRING | MAY_BE_RC1 | MAY_BE_RCN;
	} else {
		return MAY_BE_UNDEF | MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
	}
}

#define UPDATE_SSA_TYPE(_type, _var)									\
	do {																\
		uint32_t __type = (_type) & ~MAY_BE_GUARD;						\
		int __var = (_var);												\
		if (__type & MAY_BE_REF) {										\
			__type |= MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF; \
		}																\
		if (__var >= 0) {												\
			crex_ssa_var *__ssa_var = &ssa_vars[__var];					\
			if (__ssa_var->var < op_array->num_args) {					\
				if (__type & MAY_BE_RC1) {                              \
					/* TODO: may be captured by exception backtreace */ \
					__type |= MAY_BE_RCN;                               \
				}                                                       \
			}                                                           \
			if (__ssa_var->var < op_array->last_var) {					\
				if (__type & (MAY_BE_REF|MAY_BE_RCN)) {					\
					__type |= MAY_BE_RC1 | MAY_BE_RCN;					\
				}														\
				if ((__type & MAY_BE_RC1) && (__type & MAY_BE_STRING)) {\
					/* TODO: support for array keys and ($str . "")*/   \
					__type |= MAY_BE_RCN;                               \
				}                                                       \
				if (__ssa_var->alias) {									\
					__type |= get_ssa_alias_types(__ssa_var->alias);	\
				}														\
			}															\
			if (ssa_var_info[__var].type != __type) { 					\
				CREX_ASSERT(ssa_opcodes != NULL ||						\
					__ssa_var->var >= op_array->last_var ||				\
					(ssa_var_info[__var].type & MAY_BE_REF)				\
						== (__type & MAY_BE_REF));						\
				if (ssa_var_info[__var].type & ~__type) {				\
					emit_type_narrowing_warning(op_array, ssa, __var);	\
					return FAILURE;										\
				}														\
				ssa_var_info[__var].type = __type;						\
				if (update_worklist) { 									\
					add_usages(op_array, ssa, worklist, __var);			\
				}														\
			}															\
			/*crex_bitset_excl(worklist, var);*/						\
		}																\
	} while (0)

#define UPDATE_SSA_OBJ_TYPE(_ce, _is_instanceof, var)				    \
	do {                                                                \
		if (var >= 0) {													\
			if (ssa_var_info[var].ce != (_ce) ||                        \
			    ssa_var_info[var].is_instanceof != (_is_instanceof)) {  \
				ssa_var_info[var].ce = (_ce);						    \
				ssa_var_info[var].is_instanceof = (_is_instanceof);     \
				if (update_worklist) { 									\
					add_usages(op_array, ssa, worklist, var);			\
				}														\
			}															\
			/*crex_bitset_excl(worklist, var);*/						\
		}																\
	} while (0)

#define COPY_SSA_OBJ_TYPE(from_var, to_var) do { \
	if ((from_var) >= 0 && (ssa_var_info[(from_var)].type & MAY_BE_OBJECT) \
			&& ssa_var_info[(from_var)].ce && !(ssa_var_info[(to_var)].type & MAY_BE_REF)) { \
		UPDATE_SSA_OBJ_TYPE(ssa_var_info[(from_var)].ce, \
			ssa_var_info[(from_var)].is_instanceof, (to_var)); \
	} else { \
		UPDATE_SSA_OBJ_TYPE(NULL, 0, (to_var)); \
	} \
} while (0)

static void add_usages(const crex_op_array *op_array, crex_ssa *ssa, crex_bitset worklist, int var)
{
	if (ssa->vars[var].phi_use_chain) {
		crex_ssa_phi *p = ssa->vars[var].phi_use_chain;
		do {
			crex_bitset_incl(worklist, p->ssa_var);
			p = crex_ssa_next_use_phi(ssa, var, p);
		} while (p);
	}
	if (ssa->vars[var].use_chain >= 0) {
		int use = ssa->vars[var].use_chain;
		crex_ssa_op *op;

		do {
			op = ssa->ops + use;
			if (op->result_def >= 0) {
				crex_bitset_incl(worklist, op->result_def);
			}
			if (op->op1_def >= 0) {
				crex_bitset_incl(worklist, op->op1_def);
			}
			if (op->op2_def >= 0) {
				crex_bitset_incl(worklist, op->op2_def);
			}
			if (op_array->opcodes[use].opcode == CREX_OP_DATA) {
				op--;
				if (op->result_def >= 0) {
					crex_bitset_incl(worklist, op->result_def);
				}
				if (op->op1_def >= 0) {
					crex_bitset_incl(worklist, op->op1_def);
				}
				if (op->op2_def >= 0) {
					crex_bitset_incl(worklist, op->op2_def);
				}
			} else if (use + 1 < op_array->last
			 && op_array->opcodes[use + 1].opcode == CREX_OP_DATA) {
				op++;
				if (op->result_def >= 0) {
					crex_bitset_incl(worklist, op->result_def);
				}
				if (op->op1_def >= 0) {
					crex_bitset_incl(worklist, op->op1_def);
				}
				if (op->op2_def >= 0) {
					crex_bitset_incl(worklist, op->op2_def);
				}
			}
			use = crex_ssa_next_use(ssa->ops, var, use);
		} while (use >= 0);
	}
}

static void emit_type_narrowing_warning(const crex_op_array *op_array, crex_ssa *ssa, int var)
{
	int def_op_num = ssa->vars[var].definition;
	const crex_op *def_opline = def_op_num >= 0 ? &op_array->opcodes[def_op_num] : NULL;
	const char *def_op_name = def_opline ? crex_get_opcode_name(def_opline->opcode) : "PHI";
	uint32_t lineno = def_opline ? def_opline->lineno : 0;
	crex_error_at(
		E_WARNING, op_array->filename, lineno,
		"Narrowing occurred during type inference of %s. Please file a bug report on https://github.com/crx/crx-src/issues", def_op_name);
#if CREX_DEBUG
	CREX_ASSERT(0 && "Narrowing during type inference");
#endif
}

CREX_API uint32_t CREX_FASTCALL crex_array_type_info(const zval *zv)
{
	HashTable *ht = C_ARRVAL_P(zv);
	uint32_t tmp = MAY_BE_ARRAY;
	crex_string *str;
	zval *val;

	if (C_REFCOUNTED_P(zv)) {
		tmp |= MAY_BE_RC1 | MAY_BE_RCN;
	} else {
		tmp |= MAY_BE_RCN;
	}

	if (crex_hash_num_elements(ht) == 0) {
		tmp |=  MAY_BE_ARRAY_EMPTY;
	} else if (HT_IS_PACKED(ht)) {
		tmp |= MAY_BE_ARRAY_PACKED;
		CREX_HASH_PACKED_FOREACH_VAL(ht, val) {
			tmp |= 1 << (C_TYPE_P(val) + MAY_BE_ARRAY_SHIFT);
		} CREX_HASH_FOREACH_END();
	} else {
		CREX_HASH_MAP_FOREACH_STR_KEY_VAL(ht, str, val) {
			if (str) {
				tmp |= MAY_BE_ARRAY_STRING_HASH;
			} else {
				tmp |= MAY_BE_ARRAY_NUMERIC_HASH;
			}
			tmp |= 1 << (C_TYPE_P(val) + MAY_BE_ARRAY_SHIFT);
		} CREX_HASH_FOREACH_END();
	}
	return tmp;
}


CREX_API uint32_t crex_array_element_type(uint32_t t1, uint8_t op_type, int write, int insert)
{
	uint32_t tmp = 0;

	if (t1 & MAY_BE_OBJECT) {
	    if (!write) {
			/* can't be REF  because of ZVAL_COPY_DEREF() usage */
			tmp |= MAY_BE_ANY | MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
		} else {
			tmp |= MAY_BE_ANY | MAY_BE_REF | MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
	    }
		if (write) {
			tmp |= MAY_BE_INDIRECT;
		}
	}
	if (t1 & MAY_BE_ARRAY) {
		if (insert) {
			tmp |= MAY_BE_NULL;
		} else {
			tmp |= MAY_BE_NULL | ((t1 & MAY_BE_ARRAY_OF_ANY) >> MAY_BE_ARRAY_SHIFT);
			if (tmp & MAY_BE_ARRAY) {
				tmp |= MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
			}
			if (tmp & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
				if (!write) {
					/* can't be REF  because of ZVAL_COPY_DEREF() usage */
					tmp |= MAY_BE_RCN;
					if ((op_type & (IS_VAR|IS_TMP_VAR)) && (t1 & MAY_BE_RC1)) {
						tmp |= MAY_BE_RC1;
					}
				} else if (t1 & MAY_BE_ARRAY_OF_REF) {
					tmp |= MAY_BE_REF | MAY_BE_RC1 | MAY_BE_RCN;
				} else {
					tmp |= MAY_BE_RC1 | MAY_BE_RCN;
				}
			}
		}
		if (write) {
			tmp |= MAY_BE_INDIRECT;
		}
	}
	if (t1 & MAY_BE_STRING) {
		tmp |= MAY_BE_STRING | MAY_BE_RC1;
		if (write) {
			tmp |= MAY_BE_NULL;
		}
	}
	if (t1 & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE)) {
		tmp |= MAY_BE_NULL;
		if (write) {
			tmp |= MAY_BE_INDIRECT;
		}
	}
	if (t1 & (MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_RESOURCE)) {
		if (!write) {
			tmp |= MAY_BE_NULL;
		}
	}
	return tmp;
}

static uint32_t assign_dim_array_result_type(
		uint32_t arr_type, uint32_t dim_type, uint32_t value_type, uint8_t dim_op_type) {
	uint32_t tmp = 0;
	/* Only add key type if we have a value type. We want to maintain the invariant that a
	 * key type exists iff a value type exists even in dead code that may use empty types. */
	if (value_type & (MAY_BE_ANY|MAY_BE_UNDEF)) {
		if (value_type & MAY_BE_UNDEF) {
			value_type |= MAY_BE_NULL;
		}
		if (dim_op_type == IS_UNUSED) {
			if (arr_type & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE)) {
				tmp |= MAY_BE_ARRAY_PACKED;
			}
			tmp |= MAY_BE_HASH_ONLY(arr_type) ? MAY_BE_ARRAY_NUMERIC_HASH : MAY_BE_ARRAY_KEY_LONG;
		} else {
			if (dim_type & (MAY_BE_LONG|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_RESOURCE|MAY_BE_DOUBLE)) {
				if (arr_type & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE)) {
					tmp |= MAY_BE_ARRAY_PACKED;
				}
				tmp |= MAY_BE_HASH_ONLY(arr_type) ? MAY_BE_ARRAY_NUMERIC_HASH : MAY_BE_ARRAY_KEY_LONG;
			}
			if (dim_type & MAY_BE_STRING) {
				tmp |= MAY_BE_ARRAY_KEY_STRING;
				if (dim_op_type != IS_CONST) {
					// FIXME: numeric string
					if (arr_type & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE)) {
						tmp |= MAY_BE_ARRAY_PACKED;
					}
					tmp |= MAY_BE_HASH_ONLY(arr_type) ? MAY_BE_ARRAY_NUMERIC_HASH : MAY_BE_ARRAY_KEY_LONG;
				}
			}
			if (dim_type & (MAY_BE_UNDEF|MAY_BE_NULL)) {
				tmp |= MAY_BE_ARRAY_KEY_STRING;
			}
		}
	}
	/* Only add value type if we have a key type. It might be that the key type is illegal
	 * for arrays. */
	if (tmp & MAY_BE_ARRAY_KEY_ANY) {
		tmp |= (value_type & MAY_BE_ANY) << MAY_BE_ARRAY_SHIFT;
	}
	tmp &= ~MAY_BE_ARRAY_EMPTY;
	return tmp;
}

static uint32_t assign_dim_result_type(
		uint32_t arr_type, uint32_t dim_type, uint32_t value_type, uint8_t dim_op_type) {
	uint32_t tmp = arr_type & ~(MAY_BE_RC1|MAY_BE_RCN);

	if (arr_type & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE)) {
		tmp &= ~(MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE);
		tmp |= MAY_BE_ARRAY|MAY_BE_RC1;
	}
	if (tmp & (MAY_BE_ARRAY|MAY_BE_STRING)) {
		tmp |= MAY_BE_RC1;
	}
	if (tmp & (MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
		tmp |= MAY_BE_RC1 | MAY_BE_RCN;
	}
	if (tmp & MAY_BE_ARRAY) {
		tmp |= assign_dim_array_result_type(arr_type, dim_type, value_type, dim_op_type);
	}
	return tmp;
}

/* For binary ops that have compound assignment operators */
static uint32_t binary_op_result_type(
		crex_ssa *ssa, uint8_t opcode, uint32_t t1, uint32_t t2, int result_var,
		crex_long optimization_level) {
	uint32_t tmp = 0;
	uint32_t t1_type = (t1 & MAY_BE_ANY) | (t1 & MAY_BE_UNDEF ? MAY_BE_NULL : 0);
	uint32_t t2_type = (t2 & MAY_BE_ANY) | (t2 & MAY_BE_UNDEF ? MAY_BE_NULL : 0);

	if (!(CREX_OPTIMIZER_IGNORE_OVERLOADING & optimization_level)) {
		/* Handle potentially overloaded operators.
		 * This could be made more precise by checking the class type, if known. */
		if ((t1_type & MAY_BE_OBJECT) || (t2_type & MAY_BE_OBJECT)) {
			/* This is somewhat GMP specific. */
			tmp |= MAY_BE_OBJECT | MAY_BE_FALSE | MAY_BE_RC1;
		}
	}

	switch (opcode) {
		case CREX_ADD:
			if (t1_type == MAY_BE_LONG && t2_type == MAY_BE_LONG) {
				if (result_var < 0 ||
					!ssa->var_info[result_var].has_range ||
				    ssa->var_info[result_var].range.underflow ||
				    ssa->var_info[result_var].range.overflow) {
					/* may overflow */
					tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
				} else {
					tmp |= MAY_BE_LONG;
				}
			} else if (t1_type == MAY_BE_DOUBLE || t2_type == MAY_BE_DOUBLE) {
				tmp |= MAY_BE_DOUBLE;
			} else if (t1_type == MAY_BE_ARRAY && t2_type == MAY_BE_ARRAY) {
				tmp |= MAY_BE_ARRAY | MAY_BE_RC1;
				tmp |= t1 & (MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF);
				tmp |= t2 & (MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF);
			} else {
				tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
				if ((t1_type & MAY_BE_ARRAY) && (t2_type & MAY_BE_ARRAY)) {
					tmp |= MAY_BE_ARRAY | MAY_BE_RC1;
					tmp |= t1 & (MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF);
					tmp |= t2 & (MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF);
				}
			}
			break;
		case CREX_SUB:
		case CREX_MUL:
			if (t1_type == MAY_BE_LONG && t2_type == MAY_BE_LONG) {
				if (result_var < 0 ||
					!ssa->var_info[result_var].has_range ||
				    ssa->var_info[result_var].range.underflow ||
				    ssa->var_info[result_var].range.overflow) {
					/* may overflow */
					tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
				} else {
					tmp |= MAY_BE_LONG;
				}
			} else if (t1_type == MAY_BE_DOUBLE || t2_type == MAY_BE_DOUBLE) {
				tmp |= MAY_BE_DOUBLE;
			} else {
				tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
			}
			break;
		case CREX_DIV:
		case CREX_POW:
			if (t1_type == MAY_BE_DOUBLE || t2_type == MAY_BE_DOUBLE) {
				tmp |= MAY_BE_DOUBLE;
			} else {
				tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
			}
			/* Division by zero results in Inf/-Inf/Nan (double), so it doesn't need any special
			 * handling */
			break;
		case CREX_MOD:
			tmp |= MAY_BE_LONG;
			/* Division by zero results in an exception, so it doesn't need any special handling */
			break;
		case CREX_BW_OR:
		case CREX_BW_AND:
		case CREX_BW_XOR:
			if ((t1_type & MAY_BE_STRING) && (t2_type & MAY_BE_STRING)) {
				tmp |= MAY_BE_STRING | MAY_BE_RC1 | MAY_BE_RCN;
			}
			if ((t1_type & ~MAY_BE_STRING) || (t2_type & ~MAY_BE_STRING)) {
				tmp |= MAY_BE_LONG;
			}
			break;
		case CREX_SL:
		case CREX_SR:
			tmp |= MAY_BE_LONG;
			break;
		case CREX_CONCAT:
		case CREX_FAST_CONCAT:
			/* TODO: +MAY_BE_OBJECT ??? */
			tmp = MAY_BE_STRING | MAY_BE_RC1 | MAY_BE_RCN;
			break;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
	return tmp;
}

static uint32_t crex_convert_type_declaration_mask(uint32_t type_mask) {
	uint32_t result_mask = type_mask & MAY_BE_ANY;
	if (type_mask & MAY_BE_VOID) {
		result_mask |= MAY_BE_NULL;
	}
	if (type_mask & MAY_BE_CALLABLE) {
		result_mask |= MAY_BE_STRING|MAY_BE_OBJECT|MAY_BE_ARRAY|MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF;
	}
	if (type_mask & MAY_BE_STATIC) {
		result_mask |= MAY_BE_OBJECT;
	}
	if (type_mask & MAY_BE_ARRAY) {
		result_mask |= MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF;
	}
	return result_mask;
}

static uint32_t crex_convert_type(const crex_script *script, crex_type type, crex_class_entry **pce)
{
	if (pce) {
		*pce = NULL;
	}

	if (!CREX_TYPE_IS_SET(type)) {
		return MAY_BE_ANY|MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF|MAY_BE_RC1|MAY_BE_RCN;
	}

	uint32_t tmp = crex_convert_type_declaration_mask(CREX_TYPE_PURE_MASK(type));
	if (CREX_TYPE_IS_COMPLEX(type)) {
		tmp |= MAY_BE_OBJECT;
		if (pce) {
			/* As we only have space to store one CE,
			 * we use a plain object type for class unions. */
			if (CREX_TYPE_HAS_NAME(type)) {
				crex_string *lcname = crex_string_tolower(CREX_TYPE_NAME(type));
				// TODO: Pass through op_array.
				*pce = crex_optimizer_get_class_entry(script, NULL, lcname);
				crex_string_release_ex(lcname, 0);
			}
		}
	}
	if (tmp & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
		tmp |= MAY_BE_RC1 | MAY_BE_RCN;
	}
	return tmp;
}

CREX_API uint32_t crex_fetch_arg_info_type(const crex_script *script, const crex_arg_info *arg_info, crex_class_entry **pce)
{
	return crex_convert_type(script, arg_info->type, pce);
}

static const crex_property_info *lookup_prop_info(const crex_class_entry *ce, crex_string *name, crex_class_entry *scope) {
	const crex_property_info *prop_info;

	/* If the class is linked, reuse the precise runtime logic. */
	if ((ce->ce_flags & CREX_ACC_LINKED)
	 && (!scope || (scope->ce_flags & CREX_ACC_LINKED))) {
		crex_class_entry *prev_scope = EG(fake_scope);
		EG(fake_scope) = scope;
		prop_info = crex_get_property_info(ce, name, 1);
		EG(fake_scope) = prev_scope;
		if (prop_info && prop_info != CREX_WRONG_PROPERTY_INFO) {
			return prop_info;
		}
		return NULL;
	}

	/* Otherwise, handle only some safe cases */
	prop_info = crex_hash_find_ptr(&ce->properties_info, name);
	if (prop_info &&
		((prop_info->ce == scope) ||
		 (!scope && (prop_info->flags & CREX_ACC_PUBLIC)))
	) {
		return prop_info;
	}
	return NULL;
}

static const crex_property_info *crex_fetch_prop_info(const crex_op_array *op_array, crex_ssa *ssa, const crex_op *opline, const crex_ssa_op *ssa_op)
{
	const crex_property_info *prop_info = NULL;
	if (opline->op2_type == IS_CONST) {
		const crex_class_entry *ce = NULL;

		if (opline->op1_type == IS_UNUSED && !(op_array->fn_flags & CREX_ACC_TRAIT_CLONE)) {
			ce = op_array->scope;
		} else if (ssa_op->op1_use >= 0) {
			ce = ssa->var_info[ssa_op->op1_use].ce;
		}
		if (ce) {
			prop_info = lookup_prop_info(ce,
				C_STR_P(CRT_CONSTANT(opline->op2)),
				op_array->scope);
			if (prop_info && (prop_info->flags & CREX_ACC_STATIC)) {
				prop_info = NULL;
			}
		}
	}
	return prop_info;
}

static const crex_property_info *crex_fetch_static_prop_info(const crex_script *script, const crex_op_array *op_array, const crex_ssa *ssa, const crex_op *opline)
{
	const crex_property_info *prop_info = NULL;
	if (opline->op1_type == IS_CONST) {
		crex_class_entry *ce = NULL;
		if (opline->op2_type == IS_UNUSED) {
			int fetch_type = opline->op2.num & CREX_FETCH_CLASS_MASK;
			switch (fetch_type) {
				case CREX_FETCH_CLASS_SELF:
				case CREX_FETCH_CLASS_STATIC:
					/* We enforce that static property types cannot change during inheritance, so
					 * handling static the same way as self here is legal. */
					ce = op_array->scope;
					break;
				case CREX_FETCH_CLASS_PARENT:
					if (op_array->scope && (op_array->scope->ce_flags & CREX_ACC_LINKED)) {
						ce = op_array->scope->parent;
					}
					break;
			}
		} else if (opline->op2_type == IS_CONST) {
			zval *zv = CRT_CONSTANT(opline->op2);
			ce = crex_optimizer_get_class_entry(script, op_array, C_STR_P(zv + 1));
		}

		if (ce) {
			zval *zv = CRT_CONSTANT(opline->op1);
			prop_info = lookup_prop_info(ce, C_STR_P(zv), op_array->scope);
			if (prop_info && !(prop_info->flags & CREX_ACC_STATIC)) {
				prop_info = NULL;
			}
		}
	}
	return prop_info;
}

static uint32_t crex_fetch_prop_type(const crex_script *script, const crex_property_info *prop_info, crex_class_entry **pce)
{
	if (!prop_info) {
		if (pce) {
			*pce = NULL;
		}
		return MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF | MAY_BE_RC1 | MAY_BE_RCN;
	}

	return crex_convert_type(script, prop_info->type, pce);
}

static bool result_may_be_separated(crex_ssa *ssa, crex_ssa_op *ssa_op)
{
	int tmp_var = ssa_op->result_def;

	if (ssa->vars[tmp_var].use_chain >= 0
	 && !ssa->vars[tmp_var].phi_use_chain) {
		crex_ssa_op *use_op = &ssa->ops[ssa->vars[tmp_var].use_chain];

		/* TODO: analyze instructions between ssa_op and use_op */
		if (use_op == ssa_op + 1) {
			if ((use_op->op1_use == tmp_var && use_op->op1_use_chain < 0)
			 || (use_op->op2_use == tmp_var && use_op->op2_use_chain < 0)) {
				return 0;
			}
		}
	}
	return 1;
}

static crex_always_inline crex_result _crex_update_type_info(
			const crex_op_array *op_array,
			crex_ssa            *ssa,
			const crex_script   *script,
			crex_bitset          worklist,
			const crex_op       *opline,
			crex_ssa_op         *ssa_op,
			const crex_op      **ssa_opcodes,
			crex_long            optimization_level,
			bool            update_worklist)
{
	uint32_t t1, t2;
	uint32_t tmp, orig;
	crex_ssa_var *ssa_vars = ssa->vars;
	crex_ssa_var_info *ssa_var_info = ssa->var_info;
	crex_class_entry *ce;
	int j;

	if (opline->opcode == CREX_OP_DATA) {
		opline--;
		ssa_op--;
	}

	t1 = OP1_INFO();
	t2 = OP2_INFO();

	/* If one of the operands cannot have any type, this means the operand derives from
	 * unreachable code. Propagate the empty result early, so that that the following
	 * code may assume that operands have at least one type. */
	if (!(t1 & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_CLASS))
	 || !(t2 & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_CLASS))
	 || (ssa_op->result_use >= 0 && !(RES_USE_INFO() & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_CLASS)))
	 || ((opline->opcode == CREX_ASSIGN_DIM_OP
	   || opline->opcode == CREX_ASSIGN_OBJ_OP
	   || opline->opcode == CREX_ASSIGN_STATIC_PROP_OP
	   || opline->opcode == CREX_ASSIGN_DIM
	   || opline->opcode == CREX_ASSIGN_OBJ)
	    && !(OP1_DATA_INFO() & (MAY_BE_ANY|MAY_BE_UNDEF|MAY_BE_CLASS)) /*&& 0*/)) {
		tmp = 0;
		if (ssa_op->result_def >= 0 && !(ssa_var_info[ssa_op->result_def].type & MAY_BE_REF)) {
			UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
		}
		if (ssa_op->op1_def >= 0 && !(ssa_var_info[ssa_op->op1_def].type & MAY_BE_REF)) {
			UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
		}
		if (ssa_op->op2_def >= 0 && !(ssa_var_info[ssa_op->op2_def].type & MAY_BE_REF)) {
			UPDATE_SSA_TYPE(tmp, ssa_op->op2_def);
		}
		if (opline->opcode == CREX_ASSIGN_DIM_OP
		 || opline->opcode == CREX_ASSIGN_OBJ_OP
		 || opline->opcode == CREX_ASSIGN_STATIC_PROP_OP
		 || opline->opcode == CREX_ASSIGN_DIM
		 || opline->opcode == CREX_ASSIGN_OBJ) {
			if ((ssa_op+1)->op1_def >= 0 && !(ssa_var_info[(ssa_op+1)->op1_def].type & MAY_BE_REF)) {
				UPDATE_SSA_TYPE(tmp, (ssa_op+1)->op1_def);
			}
		}
		return SUCCESS;
	}

	switch (opline->opcode) {
		case CREX_ADD:
		case CREX_SUB:
		case CREX_MUL:
		case CREX_DIV:
		case CREX_POW:
		case CREX_MOD:
		case CREX_BW_OR:
		case CREX_BW_AND:
		case CREX_BW_XOR:
		case CREX_SL:
		case CREX_SR:
		case CREX_CONCAT:
			tmp = binary_op_result_type(ssa, opline->opcode, t1, t2, ssa_op->result_def, optimization_level);
			UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			break;
		case CREX_BW_NOT:
			tmp = 0;
			if (t1 & MAY_BE_STRING) {
				tmp |= MAY_BE_STRING | MAY_BE_RC1 | MAY_BE_RCN;
			}
			if (t1 & (MAY_BE_ANY-MAY_BE_STRING)) {
				tmp |= MAY_BE_LONG;
			}
			if (!(CREX_OPTIMIZER_IGNORE_OVERLOADING & optimization_level)) {
				if (t1 & MAY_BE_OBJECT) {
					/* Potentially overloaded operator. */
					tmp |= MAY_BE_OBJECT | MAY_BE_RC1;
				}
			}
			UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			break;
		case CREX_BEGIN_SILENCE:
			UPDATE_SSA_TYPE(MAY_BE_LONG, ssa_op->result_def);
			break;
		case CREX_BOOL_NOT:
		case CREX_BOOL_XOR:
		case CREX_IS_IDENTICAL:
		case CREX_IS_NOT_IDENTICAL:
		case CREX_IS_EQUAL:
		case CREX_IS_NOT_EQUAL:
		case CREX_IS_SMALLER:
		case CREX_IS_SMALLER_OR_EQUAL:
		case CREX_INSTANCEOF:
		case CREX_JMPC_EX:
		case CREX_JMPNC_EX:
		case CREX_CASE:
		case CREX_CASE_STRICT:
		case CREX_BOOL:
		case CREX_ISSET_ISEMPTY_CV:
		case CREX_ISSET_ISEMPTY_VAR:
		case CREX_ISSET_ISEMPTY_DIM_OBJ:
		case CREX_ISSET_ISEMPTY_PROP_OBJ:
		case CREX_ISSET_ISEMPTY_STATIC_PROP:
		case CREX_ASSERT_CHECK:
		case CREX_IN_ARRAY:
		case CREX_ARRAY_KEY_EXISTS:
			UPDATE_SSA_TYPE(MAY_BE_FALSE|MAY_BE_TRUE, ssa_op->result_def);
			break;
		case CREX_CAST:
			if (ssa_op->op1_def >= 0) {
				tmp = t1;
				if ((t1 & (MAY_BE_ARRAY|MAY_BE_OBJECT)) &&
				    (opline->extended_value == IS_ARRAY ||
				     opline->extended_value == IS_OBJECT)) {
					tmp |= MAY_BE_RCN;
				} else if ((t1 & MAY_BE_STRING) &&
				    opline->extended_value == IS_STRING) {
					tmp |= MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
				COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->op1_def);
			}
			tmp = 1 << opline->extended_value;
			if (tmp & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
				if ((tmp & MAY_BE_ANY) == (t1 & MAY_BE_ANY)) {
					tmp |= (t1 & MAY_BE_RC1) | MAY_BE_RCN;
				} else if ((opline->extended_value == IS_ARRAY ||
							opline->extended_value == IS_OBJECT) &&
						   (t1 & (MAY_BE_ARRAY|MAY_BE_OBJECT))) {
						tmp |= MAY_BE_RC1 | MAY_BE_RCN;
				} else if (opline->extended_value == IS_STRING &&
						   (t1 & (MAY_BE_STRING|MAY_BE_OBJECT))) {
					tmp |= MAY_BE_RC1 | MAY_BE_RCN;
				} else {
					tmp |= MAY_BE_RC1;
					if (opline->extended_value == IS_ARRAY
					 && (t1 & (MAY_BE_UNDEF|MAY_BE_NULL))) {
						tmp |= MAY_BE_RCN;
					}
				}
			}
			if (opline->extended_value == IS_ARRAY) {
				if (t1 & (MAY_BE_UNDEF|MAY_BE_NULL)) {
					tmp |= MAY_BE_ARRAY_EMPTY;
				}
				if (t1 & MAY_BE_ARRAY) {
					tmp |= t1 & (MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF);
				}
				if (t1 & MAY_BE_OBJECT) {
					tmp |= MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				} else if (t1 & (MAY_BE_ANY - MAY_BE_NULL)) {
					tmp |= ((t1 & (MAY_BE_ANY - MAY_BE_NULL)) << MAY_BE_ARRAY_SHIFT) | ((t1 & MAY_BE_NULL) ? MAY_BE_ARRAY_KEY_LONG : MAY_BE_ARRAY_PACKED);
				}
			}
			UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			break;
		case CREX_QM_ASSIGN:
		case CREX_JMP_SET:
		case CREX_COALESCE:
		case CREX_COPY_TMP:
			if (ssa_op->op1_def >= 0) {
				tmp = t1;
				if (t1 & (MAY_BE_RC1|MAY_BE_REF)) {
					tmp |= MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
				COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->op1_def);
			}
			tmp = t1 & ~MAY_BE_UNDEF;
			if (opline->opcode != CREX_COPY_TMP || opline->op1_type != IS_VAR) {
				tmp &= ~MAY_BE_REF;
			}
			if (t1 & MAY_BE_UNDEF) {
				tmp |= MAY_BE_NULL;
			}
			if (t1 & (MAY_BE_RC1|MAY_BE_RCN)) {
				tmp |= (t1 & (MAY_BE_RC1|MAY_BE_RCN));
				if (opline->opcode == CREX_COPY_TMP || opline->op1_type == IS_CV) {
					tmp |= MAY_BE_RCN;
				}
			}
			if (opline->opcode == CREX_COALESCE || opline->opcode == CREX_JMP_SET) {
				/* COALESCE and JMP_SET result can't be null */
				tmp &= ~MAY_BE_NULL;
				if (opline->opcode == CREX_JMP_SET) {
					/* JMP_SET result can't be false either */
					tmp &= ~MAY_BE_FALSE;
				}
			}
			UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->result_def);
			break;
		case CREX_JMP_NULL:
		{
			uint32_t short_circuiting_type = opline->extended_value & CREX_SHORT_CIRCUITING_CHAIN_MASK;
			if (short_circuiting_type == CREX_SHORT_CIRCUITING_CHAIN_EXPR) {
				tmp = MAY_BE_NULL;
			} else if (short_circuiting_type == CREX_SHORT_CIRCUITING_CHAIN_ISSET) {
				tmp = MAY_BE_FALSE;
			} else {
				CREX_ASSERT(short_circuiting_type == CREX_SHORT_CIRCUITING_CHAIN_EMPTY);
				tmp = MAY_BE_TRUE;
			}
			UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			break;
		}
		case CREX_ASSIGN_OP:
		case CREX_ASSIGN_DIM_OP:
		case CREX_ASSIGN_OBJ_OP:
		case CREX_ASSIGN_STATIC_PROP_OP:
		{
			const crex_property_info *prop_info = NULL;
			orig = 0;
			tmp = 0;
			if (opline->opcode == CREX_ASSIGN_OBJ_OP) {
				prop_info = crex_fetch_prop_info(op_array, ssa, opline, ssa_op);
				orig = t1;
				t1 = crex_fetch_prop_type(script, prop_info, NULL);
				t2 = OP1_DATA_INFO();
			} else if (opline->opcode == CREX_ASSIGN_DIM_OP) {
				if (t1 & MAY_BE_ARRAY_OF_REF) {
			        tmp |= MAY_BE_REF;
				}
				orig = t1;
				t1 = crex_array_element_type(t1, opline->op1_type, 1, 0);
				t2 = OP1_DATA_INFO();
			} else if (opline->opcode == CREX_ASSIGN_STATIC_PROP_OP) {
				prop_info = crex_fetch_static_prop_info(script, op_array, ssa, opline);
				t1 = crex_fetch_prop_type(script, prop_info, NULL);
				t2 = OP1_DATA_INFO();
			} else {
				if (t1 & MAY_BE_REF) {
			        tmp |= MAY_BE_REF;
				}
			}

			tmp |= binary_op_result_type(
				ssa, opline->extended_value, t1, t2,
				opline->opcode == CREX_ASSIGN_OP ? ssa_op->op1_def : -1, optimization_level);
			if (tmp & (MAY_BE_STRING|MAY_BE_ARRAY)) {
				tmp |= MAY_BE_RC1 | MAY_BE_RCN;
			}
			if (tmp & (MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
				tmp |= MAY_BE_RC1 | MAY_BE_RCN;
			}

			if (opline->opcode == CREX_ASSIGN_DIM_OP) {
				if (opline->op1_type == IS_CV) {
					orig = assign_dim_result_type(orig, OP2_INFO(), tmp, opline->op2_type);
					UPDATE_SSA_TYPE(orig, ssa_op->op1_def);
					COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->op1_def);
				}
			} else if (opline->opcode == CREX_ASSIGN_OBJ_OP) {
				if (opline->op1_type == IS_CV) {
					orig = (orig & (MAY_BE_REF|MAY_BE_OBJECT))|MAY_BE_RC1|MAY_BE_RCN;
					UPDATE_SSA_TYPE(orig, ssa_op->op1_def);
					COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->op1_def);
				}
			} else if (opline->opcode == CREX_ASSIGN_STATIC_PROP_OP) {
				/* Nothing to do */
			} else {
				if (opline->opcode == CREX_ASSIGN_OP && ssa_op->result_def >= 0 && (tmp & MAY_BE_RC1)) {
					tmp |= MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			}
			if (ssa_op->result_def >= 0) {
				ce = NULL;
				if (opline->opcode == CREX_ASSIGN_DIM_OP) {
					if (opline->op2_type == IS_UNUSED) {
						/* When appending to an array and the LONG_MAX key is already used
						 * null will be returned. */
						tmp |= MAY_BE_NULL;
					}
					if (t2 & (MAY_BE_ARRAY | MAY_BE_OBJECT)) {
						/* Arrays and objects cannot be used as keys. */
						tmp |= MAY_BE_NULL;
					}
					if (t1 & (MAY_BE_ANY - (MAY_BE_NULL | MAY_BE_FALSE | MAY_BE_STRING | MAY_BE_ARRAY))) {
						/* null and false are implicitly converted to array, anything else
						 * results in a null return value. */
						tmp |= MAY_BE_NULL;
					}
					if (tmp & MAY_BE_REF) {
						/* Typed reference may cause auto conversion */
						tmp |= MAY_BE_ANY;
					}
				} else if (opline->opcode == CREX_ASSIGN_OBJ_OP) {
					/* The return value must also satisfy the property type */
					if (prop_info) {
						t1 = crex_fetch_prop_type(script, prop_info, &ce);
						if ((t1 & (MAY_BE_LONG|MAY_BE_DOUBLE)) == MAY_BE_LONG
						 && (tmp & (MAY_BE_LONG|MAY_BE_DOUBLE)) == MAY_BE_DOUBLE) {
							/* DOUBLE may be auto-converted to LONG */
							tmp |= MAY_BE_LONG;
							tmp &= ~MAY_BE_DOUBLE;
						}
						tmp &= t1;
					}
				} else if (opline->opcode == CREX_ASSIGN_STATIC_PROP_OP) {
					/* The return value must also satisfy the property type */
					if (prop_info) {
						t1 = crex_fetch_prop_type(script, prop_info, &ce);
						if ((t1 & (MAY_BE_LONG|MAY_BE_DOUBLE)) == MAY_BE_LONG
						 && (tmp & (MAY_BE_LONG|MAY_BE_DOUBLE)) == MAY_BE_DOUBLE) {
							/* DOUBLE may be auto-converted to LONG */
							tmp |= MAY_BE_LONG;
							tmp &= ~MAY_BE_DOUBLE;
						}
						tmp &= t1;
					}
				} else {
					if (tmp & MAY_BE_REF) {
						/* Typed reference may cause auto conversion */
						tmp |= MAY_BE_ANY;
					}
				}
				tmp &= ~MAY_BE_REF;
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
				if (ce) {
					UPDATE_SSA_OBJ_TYPE(ce, 1, ssa_op->result_def);
				}
			}
			break;
		}
		case CREX_PRE_INC:
		case CREX_PRE_DEC:
			tmp = 0;
			if (t1 & (MAY_BE_RC1|MAY_BE_RCN)) {
				tmp |= MAY_BE_RC1;
				if (ssa_op->result_def >= 0) {
					tmp |= MAY_BE_RCN;
				}
			}
			if ((t1 & (MAY_BE_ANY|MAY_BE_UNDEF)) == MAY_BE_LONG) {
				if (!ssa_var_info[ssa_op->op1_use].has_range ||
				    (opline->opcode == CREX_PRE_DEC &&
				     (ssa_var_info[ssa_op->op1_use].range.underflow ||
				      ssa_var_info[ssa_op->op1_use].range.min == CREX_LONG_MIN)) ||
				     (opline->opcode == CREX_PRE_INC &&
				      (ssa_var_info[ssa_op->op1_use].range.overflow ||
				       ssa_var_info[ssa_op->op1_use].range.max == CREX_LONG_MAX))) {
					/* may overflow */
					tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
				} else {
					tmp |= MAY_BE_LONG;
				}
			} else {
				if (t1 & (MAY_BE_UNDEF | MAY_BE_NULL)) {
					if (opline->opcode == CREX_PRE_INC) {
						tmp |= MAY_BE_LONG;
					} else {
						tmp |= MAY_BE_NULL;
					}
				}
				if (t1 & MAY_BE_LONG) {
					tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
				}
				if (t1 & MAY_BE_DOUBLE) {
					tmp |= MAY_BE_DOUBLE;
				}
				if (t1 & MAY_BE_STRING) {
					tmp |= MAY_BE_STRING | MAY_BE_LONG | MAY_BE_DOUBLE;
				}
				tmp |= t1 & (MAY_BE_FALSE | MAY_BE_TRUE | MAY_BE_OBJECT);
			}
			if (ssa_op->result_def >= 0) {
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			}
			if (ssa_op->op1_def >= 0) {
				if (t1 & MAY_BE_REF) {
					tmp |= MAY_BE_REF;
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			}
			break;
		case CREX_POST_INC:
		case CREX_POST_DEC:
			if (ssa_op->result_def >= 0) {
				tmp = 0;
				if (t1 & (MAY_BE_RC1|MAY_BE_RCN)) {
					tmp |= MAY_BE_RC1|MAY_BE_RCN;
				}
				tmp |= t1 & ~(MAY_BE_UNDEF|MAY_BE_REF|MAY_BE_RCN);
				if (t1 & MAY_BE_UNDEF) {
					tmp |= MAY_BE_NULL;
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			}
			tmp = 0;
			if (t1 & MAY_BE_REF) {
				tmp |= MAY_BE_REF;
			}
			if (t1 & (MAY_BE_RC1|MAY_BE_RCN)) {
				tmp |= MAY_BE_RC1;
			}
			if ((t1 & (MAY_BE_ANY|MAY_BE_UNDEF)) == MAY_BE_LONG) {
				if (!ssa_var_info[ssa_op->op1_use].has_range ||
				     (opline->opcode == CREX_POST_DEC &&
				      (ssa_var_info[ssa_op->op1_use].range.underflow ||
				       ssa_var_info[ssa_op->op1_use].range.min == CREX_LONG_MIN)) ||
				      (opline->opcode == CREX_POST_INC &&
				       (ssa_var_info[ssa_op->op1_use].range.overflow ||
				        ssa_var_info[ssa_op->op1_use].range.max == CREX_LONG_MAX))) {
					/* may overflow */
					tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
				} else {
					tmp |= MAY_BE_LONG;
				}
			} else {
				if (t1 & (MAY_BE_UNDEF | MAY_BE_NULL)) {
					if (opline->opcode == CREX_POST_INC) {
						tmp |= MAY_BE_LONG;
					} else {
						tmp |= MAY_BE_NULL;
					}
				}
				if (t1 & MAY_BE_LONG) {
					tmp |= MAY_BE_LONG | MAY_BE_DOUBLE;
				}
				if (t1 & MAY_BE_DOUBLE) {
					tmp |= MAY_BE_DOUBLE;
				}
				if (t1 & MAY_BE_STRING) {
					tmp |= MAY_BE_STRING | MAY_BE_LONG | MAY_BE_DOUBLE;
				}
				tmp |= t1 & (MAY_BE_FALSE | MAY_BE_TRUE | MAY_BE_RESOURCE | MAY_BE_ARRAY | MAY_BE_OBJECT | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF | MAY_BE_ARRAY_KEY_ANY);
			}
			if (ssa_op->op1_def >= 0) {
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			}
			break;
		case CREX_ASSIGN_DIM:
			if (opline->op1_type == IS_CV) {
				tmp = assign_dim_result_type(t1, t2, OP1_DATA_INFO(), opline->op2_type);
				tmp |= ssa->var_info[ssa_op->op1_def].type & (MAY_BE_ARRAY_PACKED|MAY_BE_ARRAY_NUMERIC_HASH|MAY_BE_ARRAY_STRING_HASH);
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
				COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->op1_def);
			}
			if (ssa_op->result_def >= 0) {
				tmp = 0;
				if (t1 & MAY_BE_STRING) {
					tmp |= MAY_BE_STRING | MAY_BE_NULL;
				}
				if (t1 & MAY_BE_OBJECT) {
					tmp |= (MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF);
				}
				if (t1 & (MAY_BE_ARRAY|MAY_BE_FALSE|MAY_BE_NULL|MAY_BE_UNDEF)) {
					tmp |= (OP1_DATA_INFO() & (MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF));

					if (OP1_DATA_INFO() & MAY_BE_UNDEF) {
						tmp |= MAY_BE_NULL;
					}
					if (t1 & MAY_BE_ARRAY_OF_REF) {
						/* A scalar type conversion may occur when assigning to a typed reference. */
						tmp |= MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_STRING;
					}
				}
				if (t1 & (MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_RESOURCE)) {
					tmp |= MAY_BE_NULL;
				}
				tmp |= MAY_BE_RC1 | MAY_BE_RCN;
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			}
			if ((ssa_op+1)->op1_def >= 0) {
				opline++;
				ssa_op++;
				tmp = OP1_INFO();
				if (tmp & (MAY_BE_ANY | MAY_BE_REF)) {
					if (tmp & MAY_BE_RC1) {
						tmp |= MAY_BE_RCN;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			}
			break;
		case CREX_ASSIGN_OBJ:
			if (opline->op1_type == IS_CV) {
				tmp = (t1 & (MAY_BE_REF|MAY_BE_OBJECT))|MAY_BE_RC1|MAY_BE_RCN;
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
				COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->op1_def);
			}
			if (ssa_op->result_def >= 0) {
				// TODO: If there is no __set we might do better
				tmp = crex_fetch_prop_type(script,
					crex_fetch_prop_info(op_array, ssa, opline, ssa_op), &ce);
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
				if (ce) {
					UPDATE_SSA_OBJ_TYPE(ce, 1, ssa_op->result_def);
				}
			}
			if ((ssa_op+1)->op1_def >= 0) {
				opline++;
				ssa_op++;
				tmp = OP1_INFO();
				if (tmp & MAY_BE_RC1) {
					tmp |= MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			}
			break;
		case CREX_ASSIGN_STATIC_PROP:
			if (ssa_op->result_def >= 0) {
				tmp = MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF | MAY_BE_RC1 | MAY_BE_RCN;
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			}
			if ((ssa_op+1)->op1_def >= 0) {
				opline++;
				ssa_op++;
				tmp = OP1_INFO();
				if (tmp & MAY_BE_RC1) {
					tmp |= MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			}
			break;
		case CREX_PRE_INC_OBJ:
		case CREX_PRE_DEC_OBJ:
		case CREX_POST_INC_OBJ:
		case CREX_POST_DEC_OBJ:
			if (opline->op1_type == IS_CV) {
				tmp = (t1 & (MAY_BE_REF|MAY_BE_OBJECT))|MAY_BE_RC1|MAY_BE_RCN;
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
				COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->op1_def);
			}
			if (ssa_op->result_def >= 0) {
				// TODO: ???
				tmp = MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			}
			break;
		case CREX_ASSIGN:
			if (ssa_op->op2_def >= 0) {
				tmp = t2;
				if (tmp & MAY_BE_RC1) {
					tmp |= MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op2_def);
			}
			tmp = t2 & ~(MAY_BE_UNDEF|MAY_BE_REF|MAY_BE_RC1|MAY_BE_RCN);
			if (t2 & MAY_BE_UNDEF) {
				tmp |= MAY_BE_NULL;
			}
			if (t1 & MAY_BE_REF) {
				tmp |= MAY_BE_REF;
			}
			if (t2 & MAY_BE_REF) {
				tmp |= MAY_BE_RC1 | MAY_BE_RCN;
			} else if (opline->op2_type & (IS_TMP_VAR|IS_VAR)) {
				tmp |= t2 & (MAY_BE_RC1|MAY_BE_RCN);
			} else if (t2 & (MAY_BE_RC1|MAY_BE_RCN)) {
				tmp |= MAY_BE_RCN;
			}
			if (RETURN_VALUE_USED(opline) && (tmp & MAY_BE_RC1)) {
				tmp |= MAY_BE_RCN;
			}
			if (ssa_op->op1_def >= 0) {
				if (ssa_var_info[ssa_op->op1_def].use_as_double) {
					tmp &= ~MAY_BE_LONG;
					tmp |= MAY_BE_DOUBLE;
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
				COPY_SSA_OBJ_TYPE(ssa_op->op2_use, ssa_op->op1_def);
			}
			if (ssa_op->result_def >= 0) {
				if (tmp & MAY_BE_REF) {
					/* A scalar type conversion may occur when assigning to a typed reference. */
					tmp &= ~MAY_BE_REF;
					tmp |= MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_STRING|MAY_BE_RC1|MAY_BE_RCN;
				}
				if ((tmp & (MAY_BE_RC1|MAY_BE_RCN)) == MAY_BE_RCN) {
					/* refcount may be indirectly decremented. Make an exception if the result is used in the next instruction */
					if (!ssa_opcodes) {
						if (ssa->vars[ssa_op->result_def].use_chain < 0
						 || opline + 1 != op_array->opcodes + ssa->vars[ssa_op->result_def].use_chain) {
							tmp |= MAY_BE_RC1;
					    }
					} else {
						if (ssa->vars[ssa_op->result_def].use_chain < 0
						 || opline + 1 != ssa_opcodes[ssa->vars[ssa_op->result_def].use_chain]) {
							tmp |= MAY_BE_RC1;
					    }
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
				COPY_SSA_OBJ_TYPE(ssa_op->op2_use, ssa_op->result_def);
			}
			break;
		case CREX_ASSIGN_REF:
// TODO: ???
			if (opline->op2_type == IS_CV) {
				tmp = (MAY_BE_REF | t2) & ~(MAY_BE_UNDEF|MAY_BE_RC1|MAY_BE_RCN);
				if (t2 & MAY_BE_UNDEF) {
					tmp |= MAY_BE_NULL;
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op2_def);
			}
			if (opline->op2_type == IS_VAR && opline->extended_value == CREX_RETURNS_FUNCTION) {
				tmp = (MAY_BE_REF | MAY_BE_RCN | MAY_BE_RC1 | t2) & ~MAY_BE_UNDEF;
			} else {
				tmp = (MAY_BE_REF | t2) & ~(MAY_BE_UNDEF|MAY_BE_RC1|MAY_BE_RCN);
			}
			if (t2 & MAY_BE_UNDEF) {
				tmp |= MAY_BE_NULL;
			}
			UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			if (ssa_op->result_def >= 0) {
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			}
			break;
		case CREX_ASSIGN_OBJ_REF:
			if (opline->op1_type == IS_CV) {
				tmp = t1;
				if (tmp & MAY_BE_OBJECT) {
					tmp |= MAY_BE_RC1 | MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
				COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->op1_def);
			}

			t2 = OP1_DATA_INFO();
			if ((opline+1)->op1_type == IS_VAR && (opline->extended_value & CREX_RETURNS_FUNCTION)) {
				tmp = (MAY_BE_REF | MAY_BE_RCN | MAY_BE_RC1 | t2) & ~MAY_BE_UNDEF;
			} else {
				tmp = (MAY_BE_REF | t2) & ~(MAY_BE_UNDEF|MAY_BE_RC1|MAY_BE_RCN);
			}
			if (t2 & MAY_BE_UNDEF) {
				tmp |= MAY_BE_NULL;
			}
			if (ssa_op->result_def >= 0) {
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			}
			if ((opline+1)->op1_type == IS_CV) {
				opline++;
				ssa_op++;
				tmp = (MAY_BE_REF | t2) & ~(MAY_BE_UNDEF|MAY_BE_RC1|MAY_BE_RCN);
				if (t2 & MAY_BE_UNDEF) {
					tmp |= MAY_BE_NULL;
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			}
			break;
		case CREX_ASSIGN_STATIC_PROP_REF:
			if (ssa_op->result_def >= 0) {
				UPDATE_SSA_TYPE(MAY_BE_REF, ssa_op->result_def);
			}
			if ((opline+1)->op1_type == IS_CV) {
				opline++;
				ssa_op++;
				UPDATE_SSA_TYPE(MAY_BE_REF, ssa_op->op1_def);
			}
			break;
		case CREX_BIND_GLOBAL:
			tmp = MAY_BE_REF | MAY_BE_ANY
				| MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
			UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			break;
		case CREX_BIND_STATIC:
			tmp = MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF
				| ((opline->extended_value & CREX_BIND_REF) ? MAY_BE_REF : (MAY_BE_RC1 | MAY_BE_RCN));
			if (opline->extended_value & CREX_BIND_IMPLICIT) {
				tmp |= MAY_BE_UNDEF;
			}
			UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			break;
		case CREX_BIND_INIT_STATIC_OR_JMP:
			tmp = MAY_BE_UNDEF | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF | MAY_BE_REF;
			UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			break;
		case CREX_SEND_VAR:
			if (ssa_op->op1_def >= 0) {
				tmp = t1;
				if (t1 & (MAY_BE_RC1|MAY_BE_REF)) {
					tmp |= MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
				COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->op1_def);
			}
			break;
		case CREX_BIND_LEXICAL:
			if (ssa_op->op2_def >= 0) {
				if (opline->extended_value & CREX_BIND_REF) {
					tmp = t2 | MAY_BE_REF;
				} else {
					tmp = t2 & ~(MAY_BE_RC1|MAY_BE_RCN);
					if (t2 & (MAY_BE_RC1|MAY_BE_RCN)) {
						tmp |= MAY_BE_RCN;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op2_def);
				COPY_SSA_OBJ_TYPE(ssa_op->op2_use, ssa_op->op2_def);
			}
			break;
		case CREX_YIELD:
			if (ssa_op->op1_def >= 0) {
				if (op_array->fn_flags & CREX_ACC_RETURN_REFERENCE) {
					tmp = t1 | MAY_BE_REF;
				} else {
					tmp = t1 & ~(MAY_BE_RC1|MAY_BE_RCN);
					if (t1 & (MAY_BE_RC1|MAY_BE_RCN)) {
						tmp |= MAY_BE_RCN;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
				COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->op1_def);
			}
			if (ssa_op->result_def >= 0) {
				tmp = MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF
					| MAY_BE_RC1 | MAY_BE_RCN;
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			}
			break;
		case CREX_SEND_VAR_EX:
		case CREX_SEND_FUNC_ARG:
			if (ssa_op->op1_def >= 0) {
				tmp = (t1 & MAY_BE_UNDEF)|MAY_BE_REF|MAY_BE_RC1|MAY_BE_RCN|MAY_BE_ANY|MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF;
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			}
			break;
		case CREX_SEND_REF:
			if (ssa_op->op1_def >= 0) {
				tmp = MAY_BE_REF|MAY_BE_RC1|MAY_BE_RCN|MAY_BE_ANY|MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF;
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			}
			break;
		case CREX_SEND_UNPACK:
			if (ssa_op->op1_def >= 0) {
				tmp = t1;
				if (t1 & MAY_BE_ARRAY) {
					tmp |= MAY_BE_RC1 | MAY_BE_RCN;
					if (t1 & MAY_BE_ARRAY_OF_ANY) {
						/* SEND_UNPACK may acquire references into the array */
						tmp |= MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
					}
				}
				if (t1 & MAY_BE_OBJECT) {
					tmp |= MAY_BE_RC1 | MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			}
			break;
		case CREX_FAST_CONCAT:
		case CREX_ROPE_INIT:
		case CREX_ROPE_ADD:
		case CREX_ROPE_END:
			UPDATE_SSA_TYPE(MAY_BE_STRING|MAY_BE_RC1|MAY_BE_RCN, ssa_op->result_def);
			break;
		case CREX_RECV:
		case CREX_RECV_INIT:
		case CREX_RECV_VARIADIC:
		{
			/* Typehinting */
			crex_arg_info *arg_info = &op_array->arg_info[opline->op1.num-1];

			ce = NULL;
			tmp = crex_fetch_arg_info_type(script, arg_info, &ce);
			if (CREX_ARG_SEND_MODE(arg_info)) {
				tmp |= MAY_BE_REF;
				ce = NULL;
			}

			if (opline->opcode == CREX_RECV_VARIADIC) {
				uint32_t elem_type = tmp & MAY_BE_REF
					? MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF
					: (tmp & MAY_BE_ANY) << MAY_BE_ARRAY_SHIFT;
				tmp = MAY_BE_RC1|MAY_BE_RCN|MAY_BE_ARRAY|MAY_BE_ARRAY_KEY_ANY|elem_type;
				ce = NULL;
			}

			UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			if (ce) {
				UPDATE_SSA_OBJ_TYPE(ce, 1, ssa_op->result_def);
			} else {
				UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_op->result_def);
			}
			break;
		}
		case CREX_DECLARE_ANON_CLASS:
			UPDATE_SSA_TYPE(MAY_BE_CLASS, ssa_op->result_def);
			if (script && (ce = crex_hash_find_ptr(&script->class_table, C_STR_P(CRT_CONSTANT(opline->op1)))) != NULL) {
				UPDATE_SSA_OBJ_TYPE(ce, 0, ssa_op->result_def);
			}
			break;
		case CREX_FETCH_CLASS:
			UPDATE_SSA_TYPE(MAY_BE_CLASS, ssa_op->result_def);
			if (opline->op2_type == IS_UNUSED) {
				switch (opline->op1.num & CREX_FETCH_CLASS_MASK) {
					case CREX_FETCH_CLASS_SELF:
						if (op_array->scope) {
							UPDATE_SSA_OBJ_TYPE(op_array->scope, 0, ssa_op->result_def);
						} else {
							UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_op->result_def);
						}
						break;
					case CREX_FETCH_CLASS_PARENT:
						if (op_array->scope && op_array->scope->parent && (op_array->scope->ce_flags & CREX_ACC_LINKED)) {
							UPDATE_SSA_OBJ_TYPE(op_array->scope->parent, 0, ssa_op->result_def);
						} else {
							UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_op->result_def);
						}
						break;
					case CREX_FETCH_CLASS_STATIC:
					default:
						UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_op->result_def);
						break;
				}
			} else if (opline->op2_type == IS_CONST) {
				zval *zv = CRT_CONSTANT(opline->op2);
				if (C_TYPE_P(zv) == IS_STRING) {
					ce = crex_optimizer_get_class_entry(script, op_array, C_STR_P(zv+1));
					UPDATE_SSA_OBJ_TYPE(ce, 0, ssa_op->result_def);
				} else {
					UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_op->result_def);
				}
			} else {
				COPY_SSA_OBJ_TYPE(ssa_op->op2_use, ssa_op->result_def);
			}
			break;
		case CREX_NEW:
			tmp = MAY_BE_RC1|MAY_BE_RCN|MAY_BE_OBJECT;
			ce = crex_optimizer_get_class_entry_from_op1(script, op_array, opline);
			if (ce) {
				UPDATE_SSA_OBJ_TYPE(ce, 0, ssa_op->result_def);
			} else if ((t1 & MAY_BE_CLASS) && ssa_op->op1_use >= 0 && ssa_var_info[ssa_op->op1_use].ce) {
				UPDATE_SSA_OBJ_TYPE(ssa_var_info[ssa_op->op1_use].ce, ssa_var_info[ssa_op->op1_use].is_instanceof, ssa_op->result_def);
			} else {
				UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_op->result_def);
			}
			UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			break;
		case CREX_CLONE:
			UPDATE_SSA_TYPE(MAY_BE_RC1|MAY_BE_RCN|MAY_BE_OBJECT, ssa_op->result_def);
			COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->result_def);
			break;
		case CREX_INIT_ARRAY:
		case CREX_ADD_ARRAY_ELEMENT:
			if (ssa_op->op1_def >= 0) {
				if (opline->extended_value & CREX_ARRAY_ELEMENT_REF) {
					tmp = (MAY_BE_REF | t1) & ~(MAY_BE_UNDEF|MAY_BE_RC1|MAY_BE_RCN);
					if (t1 & MAY_BE_UNDEF) {
						tmp |= MAY_BE_NULL;
					}
				} else if ((t1 & (MAY_BE_REF|MAY_BE_RC1|MAY_BE_RCN)) == MAY_BE_REF) {
					tmp = (MAY_BE_REF | t1) & ~(MAY_BE_UNDEF|MAY_BE_RC1|MAY_BE_RCN);
					if (t1 & MAY_BE_UNDEF) {
						tmp |= MAY_BE_NULL;
					}
				} else if (t1 & MAY_BE_REF) {
					tmp = (MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | t1);
				} else {
					tmp = t1;
					if (t1 & MAY_BE_RC1) {
						tmp |= MAY_BE_RCN;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			}
			if (ssa_op->result_def >= 0) {
				uint32_t arr_type;
				if (opline->opcode == CREX_INIT_ARRAY) {
					arr_type = 0;
				} else {
					arr_type = RES_USE_INFO();
				}
				tmp = MAY_BE_RC1|MAY_BE_ARRAY|arr_type;
				if (opline->opcode == CREX_INIT_ARRAY && opline->op1_type == IS_UNUSED) {
					tmp |= MAY_BE_ARRAY_EMPTY;
				}
				if (opline->op1_type != IS_UNUSED
				 && (opline->op2_type == IS_UNUSED
				  || (t2 & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_RESOURCE|MAY_BE_STRING)))) {
					tmp |= assign_dim_array_result_type(arr_type, t2, t1, opline->op2_type);
					if (opline->extended_value & CREX_ARRAY_ELEMENT_REF) {
						tmp |= MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			}
			break;
		case CREX_ADD_ARRAY_UNPACK:
			tmp = ssa_var_info[ssa_op->result_use].type;
			CREX_ASSERT(tmp & MAY_BE_ARRAY);
			tmp |= t1 & (MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF);
			if (t1 & MAY_BE_OBJECT) {
				tmp |= MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY;
			}
			UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			break;
		case CREX_UNSET_CV:
			tmp = MAY_BE_UNDEF;
			if (!op_array->function_name) {
				/* In global scope, we know nothing */
				tmp |= MAY_BE_REF;
			}
			UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			break;
		case CREX_UNSET_DIM:
		case CREX_UNSET_OBJ:
			if (ssa_op->op1_def >= 0) {
				UPDATE_SSA_TYPE(t1, ssa_op->op1_def);
				COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->op1_def);
			}
			break;
		case CREX_FE_RESET_R:
		case CREX_FE_RESET_RW:
			if (ssa_op->op1_def >= 0) {
				tmp = t1;
				if (opline->opcode == CREX_FE_RESET_RW) {
					tmp |= MAY_BE_REF;
				} else if (t1 & MAY_BE_RC1) {
					tmp |= MAY_BE_RCN;
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
				COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->op1_def);
			}
			if (opline->opcode == CREX_FE_RESET_RW) {
//???
				tmp = MAY_BE_REF | (t1 & (MAY_BE_ARRAY | MAY_BE_OBJECT));
			} else {
				tmp = MAY_BE_RC1 | MAY_BE_RCN | (t1 & (MAY_BE_ARRAY | MAY_BE_OBJECT | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF));
			}
			/* The result is set to UNDEF for invalid foreach inputs. */
			if ((t1 & (MAY_BE_ANY | MAY_BE_UNDEF)) & ~(MAY_BE_ARRAY | MAY_BE_OBJECT)) {
				tmp |= MAY_BE_UNDEF;
			}
			UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->result_def);
			break;
		case CREX_FE_FETCH_R:
		case CREX_FE_FETCH_RW:
			tmp = 0;
			if (opline->op2_type == IS_CV) {
				tmp = t2 & MAY_BE_REF;
			}
			if (t1 & MAY_BE_OBJECT) {
				if (opline->opcode == CREX_FE_FETCH_RW) {
					tmp |= MAY_BE_REF | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				} else {
					tmp |= MAY_BE_RCN | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
					if (opline->op2_type != IS_CV) {
						tmp |= MAY_BE_REF;
					}
				}
			}
			if (t1 & MAY_BE_ARRAY) {
				if (opline->opcode == CREX_FE_FETCH_RW) {
					tmp |= MAY_BE_REF | MAY_BE_RCN | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				} else {
					tmp |= ((t1 & MAY_BE_ARRAY_OF_ANY) >> MAY_BE_ARRAY_SHIFT);
					if (tmp & MAY_BE_ARRAY) {
						tmp |= MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
					}
					if (t1 & MAY_BE_ARRAY_OF_REF) {
						tmp |= MAY_BE_RC1 | MAY_BE_RCN;
						if (opline->op2_type != IS_CV) {
							tmp |= MAY_BE_REF;
						}
					} else if (tmp & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
						tmp |= MAY_BE_RC1 | MAY_BE_RCN;
					}
				}
			}
			UPDATE_SSA_TYPE(tmp, ssa_op->op2_def);
			if (ssa_op->result_def >= 0) {
				tmp = (ssa_op->result_use >= 0) ? RES_USE_INFO() : 0;
				if (t1 & MAY_BE_OBJECT) {
					tmp |= MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				}
				if (t1 & MAY_BE_ARRAY) {
					if (t1 & MAY_BE_ARRAY_KEY_LONG) {
						tmp |= MAY_BE_LONG;
					}
					if (t1 & MAY_BE_ARRAY_KEY_STRING) {
						tmp |= MAY_BE_STRING | MAY_BE_RCN;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			}
			break;
		case CREX_FETCH_DIM_R:
		case CREX_FETCH_DIM_IS:
		case CREX_FETCH_DIM_RW:
		case CREX_FETCH_DIM_W:
		case CREX_FETCH_DIM_UNSET:
		case CREX_FETCH_DIM_FUNC_ARG:
		case CREX_FETCH_LIST_R:
		case CREX_FETCH_LIST_W:
			if (ssa_op->op1_def >= 0) {
				uint32_t key_type = 0;
				tmp = t1 & ~(MAY_BE_RC1|MAY_BE_RCN);
				if (opline->opcode == CREX_FETCH_DIM_W ||
				    opline->opcode == CREX_FETCH_DIM_RW ||
				    opline->opcode == CREX_FETCH_DIM_FUNC_ARG ||
				    opline->opcode == CREX_FETCH_LIST_W) {
					if (t1 & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE)) {
						if (opline->opcode != CREX_FETCH_DIM_FUNC_ARG) {
							tmp &= ~(MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE);
						}
						tmp |= MAY_BE_ARRAY | MAY_BE_RC1;
					}
					if (t1 & (MAY_BE_STRING|MAY_BE_ARRAY)) {
						tmp |= MAY_BE_RC1;
						if (opline->opcode == CREX_FETCH_DIM_FUNC_ARG) {
							tmp |= t1 & MAY_BE_RCN;
						}
					}
					if (t1 & (MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
						tmp |= t1 & (MAY_BE_RC1|MAY_BE_RCN);
					}
					if (opline->op2_type == IS_UNUSED) {
						if (t1 & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE)) {
							key_type |= MAY_BE_ARRAY_PACKED;
						}
						if (t1 & MAY_BE_ARRAY) {
							key_type |= MAY_BE_HASH_ONLY(t1) ?
								MAY_BE_ARRAY_NUMERIC_HASH : MAY_BE_ARRAY_KEY_LONG;
						}
					} else {
						if (t2 & (MAY_BE_LONG|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_RESOURCE|MAY_BE_DOUBLE)) {
							if (t1 & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE)) {
								key_type |= MAY_BE_ARRAY_PACKED;
							}
							if (t1 & MAY_BE_ARRAY) {
								key_type |= MAY_BE_HASH_ONLY(t1) ?
									MAY_BE_ARRAY_NUMERIC_HASH : MAY_BE_ARRAY_KEY_LONG;
						    }
						}
						if (t2 & MAY_BE_STRING) {
							key_type |= MAY_BE_ARRAY_KEY_STRING;
							if (opline->op2_type != IS_CONST) {
								// FIXME: numeric string
								if (t1 & (MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE)) {
									key_type |= MAY_BE_ARRAY_PACKED;
								}
								if (t1 & MAY_BE_ARRAY) {
									key_type |= MAY_BE_HASH_ONLY(t1) ?
										MAY_BE_ARRAY_NUMERIC_HASH : MAY_BE_ARRAY_KEY_LONG;
							    }
							}
						}
						if (t2 & (MAY_BE_UNDEF | MAY_BE_NULL)) {
							key_type |= MAY_BE_ARRAY_KEY_STRING;
						}
					}
				} else if (opline->opcode == CREX_FETCH_DIM_UNSET) {
					if (t1 & MAY_BE_ARRAY) {
						tmp |= MAY_BE_RC1;
					}
					if (t1 & (MAY_BE_OBJECT|MAY_BE_RESOURCE)) {
						tmp |= t1 & (MAY_BE_RC1|MAY_BE_RCN);
					}
				}
				if ((key_type & (MAY_BE_ARRAY_KEY_LONG|MAY_BE_ARRAY_KEY_STRING))
						&& (opline->opcode == CREX_FETCH_DIM_RW
						|| opline->opcode == CREX_FETCH_DIM_W
						|| opline->opcode == CREX_FETCH_DIM_FUNC_ARG
						|| opline->opcode == CREX_FETCH_LIST_W)) {
					j = ssa_vars[ssa_op->result_def].use_chain;
					if (j < 0) {
						/* no uses */
						tmp |= key_type | MAY_BE_ARRAY | MAY_BE_ARRAY_OF_NULL;
					}
					while (j >= 0) {
						uint8_t opcode;

						if (!ssa_opcodes) {
							if (j != (opline - op_array->opcodes) + 1) {
								/* Use must be in next opline */
								tmp |= key_type | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
								break;
							}
							opcode = op_array->opcodes[j].opcode;
						} else {
							if (ssa_opcodes[j] != opline + 1) {
								/* Use must be in next opline */
								tmp |= key_type | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
								break;
							}
							opcode = ssa_opcodes[j]->opcode;
						}
						switch (opcode) {
							case CREX_FETCH_DIM_W:
							case CREX_FETCH_DIM_RW:
							case CREX_FETCH_DIM_FUNC_ARG:
							case CREX_FETCH_LIST_W:
							case CREX_ASSIGN_DIM:
							case CREX_ASSIGN_DIM_OP:
								tmp |= key_type | MAY_BE_ARRAY | MAY_BE_ARRAY_OF_ARRAY;
								break;
							case CREX_SEND_VAR_EX:
							case CREX_SEND_FUNC_ARG:
							case CREX_SEND_VAR_NO_REF:
							case CREX_SEND_VAR_NO_REF_EX:
							case CREX_SEND_REF:
							case CREX_ASSIGN_REF:
							case CREX_YIELD:
							case CREX_INIT_ARRAY:
							case CREX_ADD_ARRAY_ELEMENT:
							case CREX_RETURN_BY_REF:
							case CREX_VERIFY_RETURN_TYPE:
							case CREX_MAKE_REF:
							case CREX_FE_RESET_RW:
								tmp |= key_type | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
								break;
							case CREX_PRE_INC:
							case CREX_PRE_DEC:
							case CREX_POST_INC:
							case CREX_POST_DEC:
								if (tmp & MAY_BE_ARRAY_OF_LONG) {
									/* may overflow */
									tmp |= key_type | MAY_BE_ARRAY_OF_DOUBLE;
								} else if (!(tmp & (MAY_BE_ARRAY_OF_LONG|MAY_BE_ARRAY_OF_DOUBLE))) {
									tmp |= key_type | MAY_BE_ARRAY_OF_LONG | MAY_BE_ARRAY_OF_DOUBLE;
								}
								break;
							case CREX_FETCH_OBJ_W:
							case CREX_FETCH_OBJ_RW:
							case CREX_FETCH_OBJ_FUNC_ARG:
							case CREX_ASSIGN_OBJ:
							case CREX_ASSIGN_OBJ_OP:
							case CREX_ASSIGN_OBJ_REF:
							case CREX_PRE_INC_OBJ:
							case CREX_PRE_DEC_OBJ:
							case CREX_POST_INC_OBJ:
							case CREX_POST_DEC_OBJ:
								/* These will result in an error exception, unless the element
								 * is already an object. */
								break;
							case CREX_SEND_VAR:
							case CREX_FETCH_DIM_R:
								/* This can occur if a DIM_FETCH_FUNC_ARG with UNUSED op2 is left
								 * behind, because it can't be converted to DIM_FETCH_R. */
								break;
							case CREX_FREE:
								/* This may happen if the using opcode is DCEd.  */
								break;
							EMPTY_SWITCH_DEFAULT_CASE()
						}
						j = crex_ssa_next_use(ssa->ops, ssa_op->result_def, j);
						if (j >= 0) {
							tmp |= key_type | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
							break;
						}
					}
					if (opline->opcode != CREX_FETCH_DIM_FUNC_ARG) {
						tmp &= ~MAY_BE_ARRAY_EMPTY;
					}
				}
				if (((tmp & MAY_BE_ARRAY) && (tmp & MAY_BE_ARRAY_KEY_ANY))
				 || opline->opcode == CREX_FETCH_DIM_FUNC_ARG
				 || opline->opcode == CREX_FETCH_DIM_R
				 || opline->opcode == CREX_FETCH_DIM_IS
				 || opline->opcode == CREX_FETCH_DIM_UNSET
				 || opline->opcode == CREX_FETCH_LIST_R) {
					UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
				} else {
					/* invalid key type */
					tmp = (tmp & (MAY_BE_RC1|MAY_BE_RCN|MAY_BE_ARRAY)) |
						(t1 & ~(MAY_BE_RC1|MAY_BE_RCN|MAY_BE_UNDEF|MAY_BE_NULL|MAY_BE_FALSE));
					UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
				}
				COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->op1_def);
			}
			/* FETCH_LIST on a string behaves like FETCH_R on null */
			tmp = crex_array_element_type(
				opline->opcode != CREX_FETCH_LIST_R ? t1 : ((t1 & ~MAY_BE_STRING) | MAY_BE_NULL),
				opline->op1_type,
				opline->opcode != CREX_FETCH_DIM_R && opline->opcode != CREX_FETCH_DIM_IS
					&& opline->opcode != CREX_FETCH_LIST_R,
				opline->op2_type == IS_UNUSED);
			if (opline->opcode == CREX_FETCH_DIM_FUNC_ARG && (t1 & (MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_RESOURCE))) {
				tmp |= MAY_BE_NULL;
			}
			if (opline->opcode == CREX_FETCH_DIM_IS && (t1 & MAY_BE_STRING)) {
				tmp |= MAY_BE_NULL;
			}
			if ((tmp & (MAY_BE_RC1|MAY_BE_RCN)) == MAY_BE_RCN && opline->result_type == IS_TMP_VAR) {
				/* refcount may be indirectly decremented. Make an exception if the result is used in the next instruction */
				if (!ssa_opcodes) {
					if (ssa->vars[ssa_op->result_def].use_chain < 0
					 || opline + 1 != op_array->opcodes + ssa->vars[ssa_op->result_def].use_chain) {
						tmp |= MAY_BE_RC1;
				    }
				} else {
					if (ssa->vars[ssa_op->result_def].use_chain < 0
					 || opline + 1 != ssa_opcodes[ssa->vars[ssa_op->result_def].use_chain]) {
						tmp |= MAY_BE_RC1;
				    }
				}
			}
			UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			break;
		case CREX_FETCH_THIS:
			if (!(op_array->fn_flags & CREX_ACC_TRAIT_CLONE)) {
				UPDATE_SSA_OBJ_TYPE(op_array->scope, 1, ssa_op->result_def);
			}
			UPDATE_SSA_TYPE(MAY_BE_RCN|MAY_BE_OBJECT, ssa_op->result_def);
			break;
		case CREX_FETCH_OBJ_R:
		case CREX_FETCH_OBJ_IS:
		case CREX_FETCH_OBJ_RW:
		case CREX_FETCH_OBJ_W:
		case CREX_FETCH_OBJ_UNSET:
		case CREX_FETCH_OBJ_FUNC_ARG:
			if (ssa_op->result_def >= 0) {
				uint32_t tmp = 0;
				ce = NULL;
				if (opline->op1_type != IS_UNUSED
						&& (t1 & (MAY_BE_ANY | MAY_BE_UNDEF) & ~MAY_BE_OBJECT)) {
					tmp |= MAY_BE_NULL;
				}
				if (opline->op1_type == IS_UNUSED || (t1 & MAY_BE_OBJECT)) {
					const crex_property_info *prop_info = crex_fetch_prop_info(op_array, ssa, opline, ssa_op);
					tmp |= crex_fetch_prop_type(script, prop_info, &ce);
					if (opline->opcode != CREX_FETCH_OBJ_R && opline->opcode != CREX_FETCH_OBJ_IS) {
						tmp |= MAY_BE_REF | MAY_BE_INDIRECT;
						if ((opline->extended_value & CREX_FETCH_OBJ_FLAGS) == CREX_FETCH_DIM_WRITE) {
							tmp |= MAY_BE_UNDEF;
						}
						ce = NULL;
					} else if (!(opline->op1_type & (IS_VAR|IS_TMP_VAR)) || !(t1 & MAY_BE_RC1)) {
						const crex_class_entry *ce = NULL;

						if (opline->op1_type == IS_UNUSED) {
							ce = op_array->scope;
						} else if (ssa_op->op1_use >= 0 && !ssa->var_info[ssa_op->op1_use].is_instanceof) {
							ce = ssa->var_info[ssa_op->op1_use].ce;
						}
						/* Unset properties will resort back to __get/__set */
						if (ce
						 && !ce->create_object
						 && !ce->__get
						 && !result_may_be_separated(ssa, ssa_op)) {
							tmp &= ~MAY_BE_RC1;
						}
						if (opline->opcode == CREX_FETCH_OBJ_IS) {
							/* IS check may return null for uninitialized typed property. */
							tmp |= MAY_BE_NULL;
						}
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
				if (ce) {
					UPDATE_SSA_OBJ_TYPE(ce, 1, ssa_op->result_def);
				}
			}
			break;
		case CREX_FETCH_STATIC_PROP_R:
		case CREX_FETCH_STATIC_PROP_IS:
		case CREX_FETCH_STATIC_PROP_RW:
		case CREX_FETCH_STATIC_PROP_W:
		case CREX_FETCH_STATIC_PROP_UNSET:
		case CREX_FETCH_STATIC_PROP_FUNC_ARG:
			tmp = crex_fetch_prop_type(script,
				crex_fetch_static_prop_info(script, op_array, ssa, opline), &ce);
			if (opline->opcode != CREX_FETCH_STATIC_PROP_R
					&& opline->opcode != CREX_FETCH_STATIC_PROP_IS) {
				tmp |= MAY_BE_REF | MAY_BE_INDIRECT;
				if ((opline->extended_value & CREX_FETCH_OBJ_FLAGS) == CREX_FETCH_DIM_WRITE) {
					tmp |= MAY_BE_UNDEF;
				}
				ce = NULL;
			} else {
				if (!result_may_be_separated(ssa, ssa_op)) {
					tmp &= ~MAY_BE_RC1;
				}
				if (opline->opcode == CREX_FETCH_STATIC_PROP_IS) {
					tmp |= MAY_BE_UNDEF;
				}
			}
			UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			if (ce) {
				UPDATE_SSA_OBJ_TYPE(ce, 1, ssa_op->result_def);
			}
			break;
		case CREX_DO_FCALL:
		case CREX_DO_ICALL:
		case CREX_DO_UCALL:
		case CREX_DO_FCALL_BY_NAME:
			if (ssa_op->result_def >= 0) {
				crex_func_info *func_info = CREX_FUNC_INFO(op_array);
				crex_call_info *call_info;

				if (!func_info || !func_info->call_map) {
					goto unknown_opcode;
				}
				call_info = func_info->call_map[opline - op_array->opcodes];
				if (!call_info) {
					goto unknown_opcode;
				}

				crex_class_entry *ce;
				bool ce_is_instanceof;
				tmp = crex_get_func_info(call_info, ssa, &ce, &ce_is_instanceof);
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
				if (ce) {
					UPDATE_SSA_OBJ_TYPE(ce, ce_is_instanceof, ssa_op->result_def);
				}
			}
			break;
		case CREX_CALLABLE_CONVERT:
			UPDATE_SSA_TYPE(MAY_BE_OBJECT | MAY_BE_RC1 | MAY_BE_RCN, ssa_op->result_def);
			UPDATE_SSA_OBJ_TYPE(crex_ce_closure, /* is_instanceof */ false, ssa_op->result_def);
			break;
		case CREX_FETCH_CONSTANT:
		case CREX_FETCH_CLASS_CONSTANT:
			UPDATE_SSA_TYPE(MAY_BE_RC1|MAY_BE_RCN|MAY_BE_ANY|MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY, ssa_op->result_def);
			break;
		case CREX_STRLEN:
		case CREX_COUNT:
		case CREX_FUNC_NUM_ARGS:
			UPDATE_SSA_TYPE(MAY_BE_LONG, ssa_op->result_def);
			break;
		case CREX_FUNC_GET_ARGS:
			UPDATE_SSA_TYPE(MAY_BE_RC1|MAY_BE_RCN|MAY_BE_ARRAY|MAY_BE_ARRAY_EMPTY|MAY_BE_ARRAY_PACKED|MAY_BE_ARRAY_OF_ANY, ssa_op->result_def);
			break;
		case CREX_GET_CLASS:
		case CREX_GET_CALLED_CLASS:
			UPDATE_SSA_TYPE(MAY_BE_STRING|MAY_BE_RCN, ssa_op->result_def);
			break;
		case CREX_GET_TYPE:
			UPDATE_SSA_TYPE(MAY_BE_STRING|MAY_BE_RC1|MAY_BE_RCN, ssa_op->result_def);
			break;
		case CREX_TYPE_CHECK: {
			uint32_t expected_type_mask = opline->extended_value;
			if (t1 & MAY_BE_UNDEF) {
				t1 |= MAY_BE_NULL;
			}
			tmp = 0;
			if (t1 & expected_type_mask) {
				tmp |= MAY_BE_TRUE;
				if ((t1 & expected_type_mask) & MAY_BE_RESOURCE) {
					tmp |= MAY_BE_FALSE;
				}
			}
			if (t1 & (MAY_BE_ANY - expected_type_mask)) {
				tmp |= MAY_BE_FALSE;
			}
			UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			break;
		}
		case CREX_DEFINED:
			UPDATE_SSA_TYPE(MAY_BE_FALSE|MAY_BE_TRUE, ssa_op->result_def);
			break;
		case CREX_VERIFY_RETURN_TYPE:
			if (t1 & MAY_BE_REF) {
				tmp = t1;
				ce = NULL;
			} else {
				crex_arg_info *ret_info = op_array->arg_info - 1;
				tmp = crex_fetch_arg_info_type(script, ret_info, &ce);
				tmp |= (t1 & MAY_BE_INDIRECT);

				// TODO: We could model more precisely how illegal types are converted.
				uint32_t extra_types = t1 & ~tmp;
				if (!extra_types) {
					tmp &= t1;
				}
			}
			if (opline->op1_type & (IS_TMP_VAR|IS_VAR|IS_CV)) {
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
				if (ce) {
					UPDATE_SSA_OBJ_TYPE(ce, 1, ssa_op->op1_def);
				} else {
					UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_op->op1_def);
				}
			} else {
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
				if (ce) {
					UPDATE_SSA_OBJ_TYPE(ce, 1, ssa_op->result_def);
				} else {
					UPDATE_SSA_OBJ_TYPE(NULL, 0, ssa_op->result_def);
				}
			}
			break;
		case CREX_MAKE_REF:
			tmp = MAY_BE_REF|MAY_BE_RC1|MAY_BE_RCN|MAY_BE_ANY|MAY_BE_ARRAY_KEY_ANY|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_OF_REF;
			UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			if (ssa_op->op1_def >= 0) {
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			}
			break;
		case CREX_CATCH:
			/* Forbidden opcodes */
			CREX_UNREACHABLE();
			break;
		default:
unknown_opcode:
			if (ssa_op->op1_def >= 0) {
				tmp = MAY_BE_ANY | MAY_BE_REF | MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
			}
			if (ssa_op->result_def >= 0) {
				tmp = MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
				if (opline->result_type == IS_TMP_VAR) {
					if (opline->opcode == CREX_FETCH_R || opline->opcode == CREX_FETCH_IS) {
						/* Variable reference counter may be decremented before use */
						/* See: ext/opcache/tests/jit/fetch_r_001.crxt */
						tmp |= MAY_BE_RC1 | MAY_BE_RCN;
					} else {
						tmp |= MAY_BE_RC1 | MAY_BE_RCN;
					}
				} else if (opline->result_type == IS_CV) {
					tmp |= MAY_BE_RC1 | MAY_BE_RCN;
				} else {
					tmp |= MAY_BE_REF | MAY_BE_RC1 | MAY_BE_RCN;
					switch (opline->opcode) {
						case CREX_FETCH_W:
						case CREX_FETCH_RW:
						case CREX_FETCH_FUNC_ARG:
						case CREX_FETCH_UNSET:
						case CREX_FETCH_DIM_W:
						case CREX_FETCH_DIM_RW:
						case CREX_FETCH_DIM_FUNC_ARG:
						case CREX_FETCH_DIM_UNSET:
						case CREX_FETCH_OBJ_W:
						case CREX_FETCH_OBJ_RW:
						case CREX_FETCH_OBJ_FUNC_ARG:
						case CREX_FETCH_OBJ_UNSET:
						case CREX_FETCH_STATIC_PROP_W:
						case CREX_FETCH_STATIC_PROP_RW:
						case CREX_FETCH_STATIC_PROP_FUNC_ARG:
						case CREX_FETCH_STATIC_PROP_UNSET:
							tmp |= MAY_BE_INDIRECT;
							break;
					}
				}
				UPDATE_SSA_TYPE(tmp, ssa_op->result_def);
			}
			break;
	}

	return SUCCESS;
}

CREX_API crex_result crex_update_type_info(
			const crex_op_array *op_array,
			crex_ssa            *ssa,
			const crex_script   *script,
			crex_op             *opline,
			crex_ssa_op         *ssa_op,
			const crex_op      **ssa_opcodes,
			crex_long            optimization_level)
{
	return _crex_update_type_info(op_array, ssa, script, NULL, opline, ssa_op, ssa_opcodes, optimization_level, 0);
}

static uint32_t get_class_entry_rank(crex_class_entry *ce) {
	uint32_t rank = 0;
	if (ce->ce_flags & CREX_ACC_LINKED) {
		while (ce->parent) {
			rank++;
			ce = ce->parent;
		}
	}
	return rank;
}

/* Compute least common ancestor on class inheritance tree only */
static crex_class_entry *join_class_entries(
		crex_class_entry *ce1, crex_class_entry *ce2, int *is_instanceof) {
	uint32_t rank1, rank2;
	if (ce1 == ce2) {
		return ce1;
	}
	if (!ce1 || !ce2) {
		return NULL;
	}

	rank1 = get_class_entry_rank(ce1);
	rank2 = get_class_entry_rank(ce2);

	while (rank1 != rank2) {
		if (rank1 > rank2) {
			ce1 = !(ce1->ce_flags & CREX_ACC_LINKED) ? NULL : ce1->parent;
			rank1--;
		} else {
			ce2 = !(ce2->ce_flags & CREX_ACC_LINKED) ? NULL : ce2->parent;
			rank2--;
		}
	}

	while (ce1 != ce2) {
		ce1 = !(ce1->ce_flags & CREX_ACC_LINKED) ? NULL : ce1->parent;
		ce2 = !(ce2->ce_flags & CREX_ACC_LINKED) ? NULL : ce2->parent;
	}

	if (ce1) {
		*is_instanceof = 1;
	}
	return ce1;
}

static bool safe_instanceof(crex_class_entry *ce1, crex_class_entry *ce2) {
	if (ce1 == ce2) {
		return 1;
	}
	if (!(ce1->ce_flags & CREX_ACC_LINKED)) {
		/* This case could be generalized, similarly to unlinked_instanceof */
		return 0;
	}
	return instanceof_function(ce1, ce2);
}

static crex_result crex_infer_types_ex(const crex_op_array *op_array, const crex_script *script, crex_ssa *ssa, crex_bitset worklist, crex_long optimization_level)
{
	crex_basic_block *blocks = ssa->cfg.blocks;
	crex_ssa_var *ssa_vars = ssa->vars;
	crex_ssa_var_info *ssa_var_info = ssa->var_info;
	int ssa_vars_count = ssa->vars_count;
	int i, j;
	uint32_t tmp, worklist_len = crex_bitset_len(ssa_vars_count);
	bool update_worklist = 1;
	const crex_op **ssa_opcodes = NULL;

	while (!crex_bitset_empty(worklist, worklist_len)) {
		j = crex_bitset_first(worklist, worklist_len);
		crex_bitset_excl(worklist, j);
		if (ssa_vars[j].definition_phi) {
			crex_ssa_phi *p = ssa_vars[j].definition_phi;
			if (p->pi >= 0) {
				crex_class_entry *ce = ssa_var_info[p->sources[0]].ce;
				int is_instanceof = ssa_var_info[p->sources[0]].is_instanceof;
				tmp = get_ssa_var_info(ssa, p->sources[0]);

				if (!p->has_range_constraint) {
					crex_ssa_type_constraint *constraint = &p->constraint.type;
					tmp &= constraint->type_mask;
					if (!(tmp & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE))) {
						tmp &= ~(MAY_BE_RC1|MAY_BE_RCN);
					}
					if ((tmp & MAY_BE_OBJECT) && constraint->ce && ce != constraint->ce) {
						if (!ce) {
							ce = constraint->ce;
							is_instanceof = 1;
						} else if (is_instanceof && safe_instanceof(constraint->ce, ce)) {
							ce = constraint->ce;
						} else {
							/* Ignore the constraint (either ce instanceof constraint->ce or
							 * they are unrelated, as far as we can statically determine) */
						}
					}
				}

				UPDATE_SSA_TYPE(tmp, j);
				if (tmp & MAY_BE_REF) {
					UPDATE_SSA_OBJ_TYPE(NULL, 0, j);
				} else {
					UPDATE_SSA_OBJ_TYPE(ce, is_instanceof, j);
				}
			} else {
				int first = 1;
				int is_instanceof = 0;
				crex_class_entry *ce = NULL;

				tmp = 0;
				for (i = 0; i < blocks[p->block].predecessors_count; i++) {
					tmp |= get_ssa_var_info(ssa, p->sources[i]);
				}
				UPDATE_SSA_TYPE(tmp, j);
				for (i = 0; i < blocks[p->block].predecessors_count; i++) {
					crex_ssa_var_info *info;

					CREX_ASSERT(p->sources[i] >= 0);
					info = &ssa_var_info[p->sources[i]];
					if (info->type & MAY_BE_OBJECT) {
						if (first) {
							ce = info->ce;
							is_instanceof = info->is_instanceof;
							first = 0;
						} else {
							is_instanceof |= info->is_instanceof;
							ce = join_class_entries(ce, info->ce, &is_instanceof);
						}
					}
				}
				UPDATE_SSA_OBJ_TYPE(ce, ce ? is_instanceof : 0, j);
			}
		} else if (ssa_vars[j].definition >= 0) {
			i = ssa_vars[j].definition;
			if (_crex_update_type_info(op_array, ssa, script, worklist, op_array->opcodes + i, ssa->ops + i, NULL, optimization_level, 1) == FAILURE) {
				return FAILURE;
			}
		}
	}
	return SUCCESS;
}

static bool is_narrowable_instr(crex_op *opline)  {
	return opline->opcode == CREX_ADD || opline->opcode == CREX_SUB
		|| opline->opcode == CREX_MUL || opline->opcode == CREX_DIV;
}

static bool is_effective_op1_double_cast(crex_op *opline, zval *op2) {
	return (opline->opcode == CREX_ADD && C_LVAL_P(op2) == 0)
		|| (opline->opcode == CREX_SUB && C_LVAL_P(op2) == 0)
		|| (opline->opcode == CREX_MUL && C_LVAL_P(op2) == 1)
		|| (opline->opcode == CREX_DIV && C_LVAL_P(op2) == 1);
}
static bool is_effective_op2_double_cast(crex_op *opline, zval *op1) {
	/* In CRX it holds that (double)(0-$int) is bitwise identical to 0.0-(double)$int,
	 * so allowing SUB here is fine. */
	return (opline->opcode == CREX_ADD && C_LVAL_P(op1) == 0)
		|| (opline->opcode == CREX_SUB && C_LVAL_P(op1) == 0)
		|| (opline->opcode == CREX_MUL && C_LVAL_P(op1) == 1);
}

/* This function recursively checks whether it's possible to convert an integer variable
 * initialization to a double initialization. The basic idea is that if the value is used
 * only in add/sub/mul/div ("narrowable" instructions) with a double result value, then it
 * will be cast to double at that point anyway, so we may as well do it earlier already.
 *
 * The tricky case are chains of operations, where it's not necessarily a given that converting
 * an integer to double before the chain of operations is the same as converting it after the
 * chain. What this function does is detect two cases where it is safe:
 *  * If the operations only involve constants, then we can simply verify that performing the
 *    calculation on integers and doubles yields the same value.
 *  * Even if one operand is not known, we may be able to determine that the operations with the
 *    integer replaced by a double only acts as an effective double cast on the unknown operand.
 *    E.g. 0+$i and 0.0+$i only differ by that cast. If then the consuming instruction of this
 *    result will perform a double cast anyway, the conversion is safe.
 *
 * The checks happens recursively, while keeping track of which variables are already visited to
 * avoid infinite loops. An iterative, worklist driven approach would be possible, but the state
 * management more cumbersome to implement, so we don't bother for now.
 */
static bool can_convert_to_double(
		const crex_op_array *op_array, crex_ssa *ssa, int var_num,
		zval *value, crex_bitset visited) {
	crex_ssa_var *var = &ssa->vars[var_num];
	crex_ssa_phi *phi;
	int use;
	uint32_t type;

	if (crex_bitset_in(visited, var_num)) {
		return 1;
	}
	crex_bitset_incl(visited, var_num);

	for (use = var->use_chain; use >= 0; use = crex_ssa_next_use(ssa->ops, var_num, use)) {
		crex_op *opline = &op_array->opcodes[use];
		crex_ssa_op *ssa_op = &ssa->ops[use];

		if (crex_ssa_is_no_val_use(opline, ssa_op, var_num)) {
			continue;
		}

		if (!is_narrowable_instr(opline)) {
			return 0;
		}

		/* Instruction always returns double, the conversion is certainly fine */
		type = ssa->var_info[ssa_op->result_def].type;
		if ((type & MAY_BE_ANY) == MAY_BE_DOUBLE) {
			continue;
		}

		/* UNDEF signals that the previous result is an effective double cast, this is only allowed
		 * if this instruction would have done the cast anyway (previous check). */
		if (C_ISUNDEF_P(value)) {
			return 0;
		}

		/* Check that narrowing can actually be useful */
		if ((type & MAY_BE_ANY) & ~(MAY_BE_LONG|MAY_BE_DOUBLE)) {
			return 0;
		}

		{
			/* For calculation on original values */
			zval orig_op1, orig_op2, orig_result;
			/* For calculation with var_num cast to double */
			zval dval_op1, dval_op2, dval_result;

			ZVAL_UNDEF(&orig_op1);
			ZVAL_UNDEF(&dval_op1);
			if (ssa_op->op1_use == var_num) {
				ZVAL_COPY_VALUE(&orig_op1, value);
				ZVAL_DOUBLE(&dval_op1, (double) C_LVAL_P(value));
			} else if (opline->op1_type == IS_CONST) {
				zval *zv = CRT_CONSTANT(opline->op1);
				if (C_TYPE_P(zv) == IS_LONG || C_TYPE_P(zv) == IS_DOUBLE) {
					ZVAL_COPY_VALUE(&orig_op1, zv);
					ZVAL_COPY_VALUE(&dval_op1, zv);
				}
			}

			ZVAL_UNDEF(&orig_op2);
			ZVAL_UNDEF(&dval_op2);
			if (ssa_op->op2_use == var_num) {
				ZVAL_COPY_VALUE(&orig_op2, value);
				ZVAL_DOUBLE(&dval_op2, (double) C_LVAL_P(value));
			} else if (opline->op2_type == IS_CONST) {
				zval *zv = CRT_CONSTANT(opline->op2);
				if (C_TYPE_P(zv) == IS_LONG || C_TYPE_P(zv) == IS_DOUBLE) {
					ZVAL_COPY_VALUE(&orig_op2, zv);
					ZVAL_COPY_VALUE(&dval_op2, zv);
				}
			}

			CREX_ASSERT(!C_ISUNDEF(orig_op1) || !C_ISUNDEF(orig_op2));
			if (C_ISUNDEF(orig_op1)) {
				if (opline->opcode == CREX_MUL && C_LVAL(orig_op2) == 0) {
					ZVAL_LONG(&orig_result, 0);
				} else if (is_effective_op1_double_cast(opline, &orig_op2)) {
					ZVAL_UNDEF(&orig_result);
				} else {
					return 0;
				}
			} else if (C_ISUNDEF(orig_op2)) {
				if (opline->opcode == CREX_MUL && C_LVAL(orig_op1) == 0) {
					ZVAL_LONG(&orig_result, 0);
				} else if (is_effective_op2_double_cast(opline, &orig_op1)) {
					ZVAL_UNDEF(&orig_result);
				} else {
					return 0;
				}
			} else {
				uint8_t opcode = opline->opcode;

				if (opcode == CREX_ASSIGN_OP) {
					opcode = opline->extended_value;
				}

				/* Avoid division by zero */
				if (opcode == CREX_DIV && zval_get_double(&orig_op2) == 0.0) {
					return 0;
				}

				get_binary_op(opcode)(&orig_result, &orig_op1, &orig_op2);
				get_binary_op(opcode)(&dval_result, &dval_op1, &dval_op2);
				CREX_ASSERT(C_TYPE(dval_result) == IS_DOUBLE);
				if (zval_get_double(&orig_result) != C_DVAL(dval_result)) {
					return 0;
				}
			}

			if (!can_convert_to_double(op_array, ssa, ssa_op->result_def, &orig_result, visited)) {
				return 0;
			}
		}
	}

	for (phi = var->phi_use_chain; phi; phi = crex_ssa_next_use_phi(ssa, var_num, phi)) {
		/* Check that narrowing can actually be useful */
		type = ssa->var_info[phi->ssa_var].type;
		if ((type & MAY_BE_ANY) & ~(MAY_BE_LONG|MAY_BE_DOUBLE)) {
			return 0;
		}

		if (!can_convert_to_double(op_array, ssa, phi->ssa_var, value, visited)) {
			return 0;
		}
	}

	return 1;
}

static crex_result crex_type_narrowing(const crex_op_array *op_array, const crex_script *script, crex_ssa *ssa, crex_long optimization_level)
{
	uint32_t bitset_len = crex_bitset_len(ssa->vars_count);
	crex_bitset visited, worklist;
	int i, v;
	crex_op *opline;
	bool narrowed = 0;
	ALLOCA_FLAG(use_heap)

	visited = CREX_BITSET_ALLOCA(2 * bitset_len, use_heap);
	worklist = visited + bitset_len;

	crex_bitset_clear(worklist, bitset_len);

	for (v = op_array->last_var; v < ssa->vars_count; v++) {
		if ((ssa->var_info[v].type & (MAY_BE_REF | MAY_BE_ANY | MAY_BE_UNDEF)) != MAY_BE_LONG) continue;
		if (ssa->vars[v].definition < 0) continue;
		if (ssa->vars[v].no_val) continue;
		opline = op_array->opcodes + ssa->vars[v].definition;
		/* Go through assignments of literal integers and check if they can be converted to
		 * doubles instead, in the hope that we'll narrow long|double to double. */
		if (opline->opcode == CREX_ASSIGN && opline->result_type == IS_UNUSED &&
				opline->op1_type == IS_CV && opline->op2_type == IS_CONST) {
			zval *value = CRT_CONSTANT(opline->op2);

			crex_bitset_clear(visited, bitset_len);
			if (can_convert_to_double(op_array, ssa, v, value, visited)) {
				narrowed = 1;
				ssa->var_info[v].use_as_double = 1;
				/* The "visited" vars are exactly those which may change their type due to
				 * narrowing. Reset their types and add them to the type inference worklist */
				CREX_BITSET_FOREACH(visited, bitset_len, i) {
					ssa->var_info[i].type &= ~MAY_BE_ANY;
				} CREX_BITSET_FOREACH_END();
				crex_bitset_union(worklist, visited, bitset_len);
			}
		}
	}

	if (!narrowed) {
		free_alloca(visited, use_heap);
		return SUCCESS;
	}

	if (crex_infer_types_ex(op_array, script, ssa, worklist, optimization_level) == FAILURE) {
		free_alloca(visited, use_heap);
		return FAILURE;
	}

	free_alloca(visited, use_heap);
	return SUCCESS;
}

static bool is_recursive_tail_call(const crex_op_array *op_array,
                                  crex_op             *opline)
{
	crex_func_info *info = CREX_FUNC_INFO(op_array);

	if (info->ssa.ops && info->ssa.vars && info->call_map &&
	    info->ssa.ops[opline - op_array->opcodes].op1_use >= 0 &&
	    info->ssa.vars[info->ssa.ops[opline - op_array->opcodes].op1_use].definition >= 0) {

		crex_op *op = op_array->opcodes + info->ssa.vars[info->ssa.ops[opline - op_array->opcodes].op1_use].definition;

		if (op->opcode == CREX_DO_UCALL) {
			crex_call_info *call_info = info->call_map[op - op_array->opcodes];
			if (call_info && op_array == &call_info->callee_func->op_array) {
				return 1;
			}
		}
	}
	return 0;
}

uint32_t crex_get_return_info_from_signature_only(
		const crex_function *func, const crex_script *script,
		crex_class_entry **ce, bool *ce_is_instanceof, bool use_tentative_return_info) {
	uint32_t type;
	if (func->common.fn_flags & CREX_ACC_HAS_RETURN_TYPE &&
		(use_tentative_return_info || !CREX_ARG_TYPE_IS_TENTATIVE(func->common.arg_info - 1))
	) {
		crex_arg_info *ret_info = func->common.arg_info - 1;
		type = crex_fetch_arg_info_type(script, ret_info, ce);
		*ce_is_instanceof = ce != NULL;
	} else {
		type = MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF
			| MAY_BE_RC1 | MAY_BE_RCN;
		*ce = NULL;
		*ce_is_instanceof = false;
	}

	/* For generators RETURN_REFERENCE refers to the yielded values. */
	if ((func->common.fn_flags & CREX_ACC_RETURN_REFERENCE)
			&& !(func->common.fn_flags & CREX_ACC_GENERATOR)) {
		type |= MAY_BE_REF;
		*ce = NULL;
		*ce_is_instanceof = 0;
	}
	return type;
}

CREX_API void crex_init_func_return_info(
	const crex_op_array *op_array, const crex_script *script, crex_ssa_var_info *ret)
{
	CREX_ASSERT((op_array->fn_flags & CREX_ACC_HAS_RETURN_TYPE));

	crex_ssa_range tmp_range = {0, 0, 0, 0};
	bool is_instanceof = false;
	ret->type = crex_get_return_info_from_signature_only(
		(crex_function *) op_array, script, &ret->ce, &is_instanceof, /* use_tentative_return_info */ 1);
	ret->is_instanceof = is_instanceof;
	ret->range = tmp_range;
	ret->has_range = 0;
}

static void crex_func_return_info(const crex_op_array   *op_array,
                                  const crex_script     *script,
                                  int                    recursive,
                                  int                    widening,
                                  crex_ssa_var_info     *ret)
{
	crex_func_info *info = CREX_FUNC_INFO(op_array);
	crex_ssa *ssa = &info->ssa;
	int blocks_count = info->ssa.cfg.blocks_count;
	crex_basic_block *blocks = info->ssa.cfg.blocks;
	int j;
	uint32_t t1;
	uint32_t tmp = 0;
	crex_class_entry *tmp_ce = NULL;
	int tmp_is_instanceof = -1;
	crex_class_entry *arg_ce;
	int arg_is_instanceof;
	crex_ssa_range tmp_range = {0, 0, 0, 0};
	int tmp_has_range = -1;

	if (op_array->fn_flags & CREX_ACC_GENERATOR) {
		ret->type = MAY_BE_OBJECT | MAY_BE_RC1 | MAY_BE_RCN;
		ret->ce = crex_ce_generator;
		ret->is_instanceof = 0;
		ret->range = tmp_range;
		ret->has_range = 0;
		return;
	}

	if (!ret->type) {
		/* We will intersect the type later. */
		ret->type = MAY_BE_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF | MAY_BE_ARRAY_KEY_ANY
			| MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF;
	}

	for (j = 0; j < blocks_count; j++) {
		if ((blocks[j].flags & CREX_BB_REACHABLE) && blocks[j].len != 0) {
			crex_op *opline = op_array->opcodes + blocks[j].start + blocks[j].len - 1;

			if (opline->opcode == CREX_RETURN || opline->opcode == CREX_RETURN_BY_REF) {
				crex_ssa_op *ssa_op = ssa->ops ? &ssa->ops[opline - op_array->opcodes] : NULL;
				if (!recursive && ssa_op && info->ssa.var_info &&
				    ssa_op->op1_use >= 0 &&
				    info->ssa.var_info[ssa_op->op1_use].recursive) {
					continue;
				}
				if (is_recursive_tail_call(op_array, opline)) {
					continue;
				}
				t1 = OP1_INFO();
				if (t1 & MAY_BE_UNDEF) {
					t1 |= MAY_BE_NULL;
				}
				if (opline->opcode == CREX_RETURN) {
					if (t1 & MAY_BE_RC1) {
						t1 |= MAY_BE_RCN;
					}
					t1 &= ~(MAY_BE_UNDEF | MAY_BE_REF);
				} else {
					t1 |= MAY_BE_REF;
					t1 &= ~(MAY_BE_UNDEF | MAY_BE_RC1 | MAY_BE_RCN);
				}
				tmp |= t1;

				if (ssa_op && info->ssa.var_info &&
				    ssa_op->op1_use >= 0 && !(t1 & MAY_BE_REF) &&
				    info->ssa.var_info[ssa_op->op1_use].ce) {
					arg_ce = info->ssa.var_info[ssa_op->op1_use].ce;
					arg_is_instanceof = info->ssa.var_info[ssa_op->op1_use].is_instanceof;
				} else {
					arg_ce = NULL;
					arg_is_instanceof = 0;
				}

				if (tmp_is_instanceof < 0) {
					tmp_ce = arg_ce;
					tmp_is_instanceof = arg_is_instanceof;
				} else if (arg_ce && arg_ce == tmp_ce) {
					if (tmp_is_instanceof != arg_is_instanceof) {
						tmp_is_instanceof = 1;
					}
				} else {
					tmp_ce = NULL;
					tmp_is_instanceof = 0;
				}

				if (opline->op1_type == IS_CONST) {
					zval *zv = CRT_CONSTANT(opline->op1);

					if (C_TYPE_P(zv) == IS_LONG) {
						if (tmp_has_range < 0) {
							tmp_has_range = 1;
							tmp_range.underflow = 0;
							tmp_range.min = C_LVAL_P(zv);
							tmp_range.max = C_LVAL_P(zv);
							tmp_range.overflow = 0;
						} else if (tmp_has_range) {
							if (!tmp_range.underflow) {
								tmp_range.min = MIN(tmp_range.min, C_LVAL_P(zv));
							}
							if (!tmp_range.overflow) {
								tmp_range.max = MAX(tmp_range.max, C_LVAL_P(zv));
							}
						}
					} else {
						tmp_has_range = 0;
					}
				} else if (ssa_op && info->ssa.var_info && ssa_op->op1_use >= 0) {
					if (info->ssa.var_info[ssa_op->op1_use].has_range) {
						if (tmp_has_range < 0) {
							tmp_has_range = 1;
							tmp_range = info->ssa.var_info[ssa_op->op1_use].range;
						} else if (tmp_has_range) {
							/* union */
							if (info->ssa.var_info[ssa_op->op1_use].range.underflow) {
								tmp_range.underflow = 1;
								tmp_range.min = CREX_LONG_MIN;
							} else {
								tmp_range.min = MIN(tmp_range.min, info->ssa.var_info[ssa_op->op1_use].range.min);
							}
							if (info->ssa.var_info[ssa_op->op1_use].range.overflow) {
								tmp_range.overflow = 1;
								tmp_range.max = CREX_LONG_MAX;
							} else {
								tmp_range.max = MAX(tmp_range.max, info->ssa.var_info[ssa_op->op1_use].range.max);
							}
						}
					} else if (!widening) {
						tmp_has_range = 1;
						tmp_range.underflow = 1;
						tmp_range.min = CREX_LONG_MIN;
						tmp_range.max = CREX_LONG_MAX;
						tmp_range.overflow = 1;
					}
				} else {
					tmp_has_range = 0;
				}
			}
		}
	}

	if (!(op_array->fn_flags & CREX_ACC_HAS_RETURN_TYPE)) {
		if (tmp_is_instanceof < 0) {
			tmp_is_instanceof = 0;
			tmp_ce = NULL;
		}
		if (tmp_has_range < 0) {
			tmp_has_range = 0;
		}
		ret->ce = tmp_ce;
		ret->is_instanceof = tmp_is_instanceof;
	}
	ret->type &= tmp;
	ret->range = tmp_range;
	ret->has_range = tmp_has_range;
}

static crex_result crex_infer_types(const crex_op_array *op_array, const crex_script *script, crex_ssa *ssa, crex_long optimization_level)
{
	int ssa_vars_count = ssa->vars_count;
	int j;
	crex_bitset worklist;
	ALLOCA_FLAG(use_heap);

	worklist = do_alloca(sizeof(crex_ulong) * crex_bitset_len(ssa_vars_count), use_heap);
	memset(worklist, 0, sizeof(crex_ulong) * crex_bitset_len(ssa_vars_count));

	/* Type Inference */
	for (j = op_array->last_var; j < ssa_vars_count; j++) {
		crex_bitset_incl(worklist, j);
	}

	if (crex_infer_types_ex(op_array, script, ssa, worklist, optimization_level) == FAILURE) {
		free_alloca(worklist,  use_heap);
		return FAILURE;
	}

	if (optimization_level & CREX_OPTIMIZER_NARROW_TO_DOUBLE) {
		/* Narrowing integer initialization to doubles */
		crex_type_narrowing(op_array, script, ssa, optimization_level);
	}

	if (CREX_FUNC_INFO(op_array)) {
		crex_func_return_info(op_array, script, 1, 0, &CREX_FUNC_INFO(op_array)->return_info);
	}

	free_alloca(worklist,  use_heap);
	return SUCCESS;
}

static void crex_mark_cv_references(const crex_op_array *op_array, const crex_script *script, crex_ssa *ssa)
{
	int var, def;
	const crex_op *opline;
	crex_arg_info *arg_info;
	uint32_t worklist_len = crex_bitset_len(ssa->vars_count);
	crex_bitset worklist;
	ALLOCA_FLAG(use_heap);

	worklist = do_alloca(sizeof(crex_ulong) * worklist_len, use_heap);
	memset(worklist, 0, sizeof(crex_ulong) * worklist_len);

	/* Collect SSA variables which definitions creates CRX reference */
	for (var = 0; var < ssa->vars_count; var++) {
		def = ssa->vars[var].definition;
		if (def >= 0 && ssa->vars[var].var < op_array->last_var) {
			opline = op_array->opcodes + def;
			if (ssa->ops[def].result_def == var) {
				switch (opline->opcode) {
					case CREX_RECV:
					case CREX_RECV_INIT:
						arg_info = &op_array->arg_info[opline->op1.num-1];
						if (!CREX_ARG_SEND_MODE(arg_info)) {
							continue;
						}
						break;
					default:
						continue;
				}
			} else if (ssa->ops[def].op1_def == var) {
				switch (opline->opcode) {
					case CREX_ASSIGN_REF:
					case CREX_MAKE_REF:
					case CREX_FE_RESET_RW:
					case CREX_BIND_GLOBAL:
					case CREX_SEND_REF:
					case CREX_SEND_VAR_EX:
					case CREX_SEND_FUNC_ARG:
					case CREX_BIND_INIT_STATIC_OR_JMP:
						break;
					case CREX_INIT_ARRAY:
					case CREX_ADD_ARRAY_ELEMENT:
						if (!(opline->extended_value & CREX_ARRAY_ELEMENT_REF)) {
							continue;
						}
						break;
					case CREX_BIND_STATIC:
						if (!(opline->extended_value & CREX_BIND_REF)) {
							continue;
						}
						break;
					case CREX_YIELD:
						if (!(op_array->fn_flags & CREX_ACC_RETURN_REFERENCE)) {
							continue;
						}
						break;
					case CREX_OP_DATA:
						switch ((opline-1)->opcode) {
							case CREX_ASSIGN_OBJ_REF:
							case CREX_ASSIGN_STATIC_PROP_REF:
								break;
							default:
								continue;
						}
						break;
					default:
						continue;
				}
			} else if (ssa->ops[def].op2_def == var) {
				switch (opline->opcode) {
					case CREX_ASSIGN_REF:
					case CREX_FE_FETCH_RW:
						break;
					case CREX_BIND_LEXICAL:
						if (!(opline->extended_value & CREX_BIND_REF)) {
							continue;
						}
						break;
					default:
						continue;
				}
			} else {
				CREX_UNREACHABLE();
			}
			crex_bitset_incl(worklist, var);
		} else if (ssa->var_info[var].type & MAY_BE_REF) {
			crex_bitset_incl(worklist, var);
		} else if (ssa->vars[var].alias == SYMTABLE_ALIAS) {
			crex_bitset_incl(worklist, var);
		}
	}

	/* Set and propagate MAY_BE_REF */
	WHILE_WORKLIST(worklist, worklist_len, var) {

		ssa->var_info[var].type |= MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;

		if (ssa->vars[var].phi_use_chain) {
			crex_ssa_phi *p = ssa->vars[var].phi_use_chain;
			do {
				if (!(ssa->var_info[p->ssa_var].type & MAY_BE_REF)) {
					crex_bitset_incl(worklist, p->ssa_var);
				}
				p = crex_ssa_next_use_phi(ssa, var, p);
			} while (p);
		}

		if (ssa->vars[var].use_chain >= 0) {
			int use = ssa->vars[var].use_chain;
			FOREACH_USE(&ssa->vars[var], use) {
				crex_ssa_op *op = ssa->ops + use;
				if (op->op1_use == var && op->op1_def >= 0) {
					if (!(ssa->var_info[op->op1_def].type & MAY_BE_REF)) {
						/* Unset breaks references (outside global scope). */
						if (op_array->opcodes[use].opcode == CREX_UNSET_CV
								&& op_array->function_name) {
							continue;
						}
						crex_bitset_incl(worklist, op->op1_def);
					}
				}
				if (op->op2_use == var && op->op2_def >= 0) {
					if (!(ssa->var_info[op->op2_def].type & MAY_BE_REF)) {
						crex_bitset_incl(worklist, op->op2_def);
					}
				}
				if (op->result_use == var && op->result_def >= 0) {
					if (!(ssa->var_info[op->result_def].type & MAY_BE_REF)) {
						crex_bitset_incl(worklist, op->result_def);
					}
				}
			} FOREACH_USE_END();
		}
	} WHILE_WORKLIST_END();

	free_alloca(worklist,  use_heap);
}

CREX_API crex_result crex_ssa_inference(crex_arena **arena, const crex_op_array *op_array, const crex_script *script, crex_ssa *ssa, crex_long optimization_level) /* {{{ */
{
	crex_ssa_var_info *ssa_var_info;
	int i;

	if (!ssa->var_info) {
		ssa->var_info = crex_arena_calloc(arena, ssa->vars_count, sizeof(crex_ssa_var_info));
	}
	ssa_var_info = ssa->var_info;

	if (!op_array->function_name) {
		for (i = 0; i < op_array->last_var; i++) {
			ssa_var_info[i].type = MAY_BE_UNDEF | MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_ANY  | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
			ssa_var_info[i].has_range = 0;
		}
	} else {
		for (i = 0; i < op_array->last_var; i++) {
			ssa_var_info[i].type = MAY_BE_UNDEF;
			ssa_var_info[i].has_range = 0;
			if (ssa->vars[i].alias) {
				ssa_var_info[i].type |= get_ssa_alias_types(ssa->vars[i].alias);
			}
		}
	}
	for (i = op_array->last_var; i < ssa->vars_count; i++) {
		ssa_var_info[i].type = 0;
		ssa_var_info[i].has_range = 0;
	}

	crex_mark_cv_references(op_array, script, ssa);

	crex_infer_ranges(op_array, ssa);

	if (crex_infer_types(op_array, script, ssa, optimization_level) == FAILURE) {
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

CREX_API bool crex_may_throw_ex(const crex_op *opline, const crex_ssa_op *ssa_op, const crex_op_array *op_array, const crex_ssa *ssa, uint32_t t1, uint32_t t2)
{
	if (opline->op1_type == IS_CV) {
		if (t1 & MAY_BE_UNDEF) {
			switch (opline->opcode) {
				case CREX_UNSET_VAR:
				case CREX_ISSET_ISEMPTY_VAR:
					return 1;
				case CREX_ISSET_ISEMPTY_DIM_OBJ:
				case CREX_ISSET_ISEMPTY_PROP_OBJ:
				case CREX_ASSIGN:
				case CREX_ASSIGN_DIM:
				case CREX_ASSIGN_REF:
				case CREX_BIND_GLOBAL:
				case CREX_BIND_STATIC:
				case CREX_BIND_INIT_STATIC_OR_JMP:
				case CREX_FETCH_DIM_IS:
				case CREX_FETCH_OBJ_IS:
				case CREX_SEND_REF:
				case CREX_UNSET_CV:
				case CREX_ISSET_ISEMPTY_CV:
				case CREX_MAKE_REF:
				case CREX_FETCH_DIM_W:
					break;
				default:
					/* undefined variable warning */
					return 1;
			}
		}
	} else if (opline->op1_type & (IS_TMP_VAR|IS_VAR)) {
		if ((t1 & MAY_BE_RC1)
		 && (t1 & (MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_ARRAY_OF_OBJECT|MAY_BE_ARRAY_OF_RESOURCE|MAY_BE_ARRAY_OF_ARRAY))) {
			switch (opline->opcode) {
				case CREX_CASE:
				case CREX_CASE_STRICT:
				case CREX_FE_FETCH_R:
				case CREX_FE_FETCH_RW:
				case CREX_FETCH_LIST_R:
				case CREX_QM_ASSIGN:
				case CREX_SEND_VAL:
				case CREX_SEND_VAL_EX:
				case CREX_SEND_VAR:
				case CREX_SEND_VAR_EX:
				case CREX_SEND_FUNC_ARG:
				case CREX_SEND_VAR_NO_REF:
				case CREX_SEND_VAR_NO_REF_EX:
				case CREX_SEND_REF:
				case CREX_SEPARATE:
				case CREX_END_SILENCE:
				case CREX_MAKE_REF:
					break;
				default:
					/* destructor may be called */
					return 1;
			}
		}
	}

	if (opline->op2_type == IS_CV) {
		if (t2 & MAY_BE_UNDEF) {
			switch (opline->opcode) {
				case CREX_ASSIGN_REF:
				case CREX_FE_FETCH_R:
				case CREX_FE_FETCH_RW:
					break;
				default:
					/* undefined variable warning */
					return 1;
			}
		}
	} else if (opline->op2_type & (IS_TMP_VAR|IS_VAR)) {
		if ((t2 & MAY_BE_RC1)
		 && (t2 & (MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_ARRAY_OF_OBJECT|MAY_BE_ARRAY_OF_RESOURCE|MAY_BE_ARRAY_OF_ARRAY))) {
			switch (opline->opcode) {
				case CREX_ASSIGN:
				case CREX_FE_FETCH_R:
				case CREX_FE_FETCH_RW:
					break;
				default:
					/* destructor may be called */
					return 1;
			}
		}
	}

	switch (opline->opcode) {
		case CREX_NOP:
		case CREX_IS_IDENTICAL:
		case CREX_IS_NOT_IDENTICAL:
		case CREX_QM_ASSIGN:
		case CREX_JMP:
		case CREX_CHECK_VAR:
		case CREX_MAKE_REF:
		case CREX_BEGIN_SILENCE:
		case CREX_END_SILENCE:
		case CREX_FREE:
		case CREX_FE_FREE:
		case CREX_SEPARATE:
		case CREX_TYPE_CHECK:
		case CREX_DEFINED:
		case CREX_ISSET_ISEMPTY_THIS:
		case CREX_COALESCE:
		case CREX_SWITCH_LONG:
		case CREX_SWITCH_STRING:
		case CREX_MATCH:
		case CREX_ISSET_ISEMPTY_VAR:
		case CREX_ISSET_ISEMPTY_CV:
		case CREX_FUNC_NUM_ARGS:
		case CREX_FUNC_GET_ARGS:
		case CREX_COPY_TMP:
		case CREX_CASE_STRICT:
		case CREX_JMP_NULL:
			return 0;
		case CREX_SEND_VAR:
		case CREX_SEND_VAL:
		case CREX_SEND_REF:
		case CREX_SEND_VAR_EX:
		case CREX_SEND_FUNC_ARG:
		case CREX_CHECK_FUNC_ARG:
			/* May throw for named params. */
			return opline->op2_type == IS_CONST;
		case CREX_INIT_FCALL:
			/* can't throw, because call is resolved at compile time */
			return 0;
		case CREX_BIND_GLOBAL:
			if ((opline+1)->opcode == CREX_BIND_GLOBAL) {
				return crex_may_throw(opline + 1, ssa_op + 1, op_array, ssa);
			}
			return 0;
		case CREX_ADD:
			if ((t1 & MAY_BE_ANY) == MAY_BE_ARRAY
			 && (t2 & MAY_BE_ANY) == MAY_BE_ARRAY) {
				return 0;
			}
			return (t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) ||
				(t2 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
		case CREX_DIV:
			if (!OP2_HAS_RANGE() ||
				(OP2_MIN_RANGE() <= 0 && OP2_MAX_RANGE() >= 0)) {
				/* Division by zero */
				return 1;
			}
			CREX_FALLTHROUGH;
		case CREX_SUB:
		case CREX_MUL:
		case CREX_POW:
			return (t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) ||
				(t2 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
		/* Ops may throw if not an integer */
		case CREX_MOD:
			if (!OP2_HAS_RANGE() ||
				(OP2_MIN_RANGE() <= 0 && OP2_MAX_RANGE() >= 0)) {
				/* Division by zero */
				return 1;
			}
			CREX_FALLTHROUGH;
		case CREX_SL:
		case CREX_SR:
			return (t1 & (MAY_BE_STRING|MAY_BE_DOUBLE|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) ||
				(t2 & (MAY_BE_STRING|MAY_BE_DOUBLE|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) ||
				!OP2_HAS_RANGE() ||
				OP2_MIN_RANGE() < 0;
		case CREX_CONCAT:
		case CREX_FAST_CONCAT:
			return (t1 & (MAY_BE_ARRAY|MAY_BE_OBJECT)) ||
				(t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT));
		case CREX_BW_OR:
		case CREX_BW_AND:
		case CREX_BW_XOR:
			if ((t1 & MAY_BE_ANY) == MAY_BE_STRING
			 && (t2 & MAY_BE_ANY) == MAY_BE_STRING) {
				return 0;
			}
			return (t1 & (MAY_BE_STRING|MAY_BE_DOUBLE|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) ||
				(t2 & (MAY_BE_STRING|MAY_BE_DOUBLE|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
		case CREX_BW_NOT:
			return (t1 & (MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_DOUBLE|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
		case CREX_PRE_INC:
		case CREX_POST_INC:
			return (t1 & (MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
		/* null emits a warning as it has no effect compared to ++ which converts the value to 1 */
		case CREX_PRE_DEC:
		case CREX_POST_DEC:
			return (t1 & (MAY_BE_NULL|MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
		case CREX_BOOL_NOT:
		case CREX_JMPZ:
		case CREX_JMPNZ:
		case CREX_JMPC_EX:
		case CREX_JMPNC_EX:
		case CREX_BOOL:
		case CREX_JMP_SET:
			return (t1 & MAY_BE_OBJECT);
		case CREX_BOOL_XOR:
			return (t1 & MAY_BE_OBJECT) || (t2 & MAY_BE_OBJECT);
		case CREX_IS_EQUAL:
		case CREX_IS_NOT_EQUAL:
		case CREX_IS_SMALLER:
		case CREX_IS_SMALLER_OR_EQUAL:
		case CREX_CASE:
		case CREX_SPACESHIP:
			if ((t1 & MAY_BE_ANY) == MAY_BE_NULL
			 || (t2 & MAY_BE_ANY) == MAY_BE_NULL) {
				return 0;
			}
			return (t1 & (MAY_BE_OBJECT|MAY_BE_ARRAY_OF_ARRAY|MAY_BE_ARRAY_OF_OBJECT)) || (t2 & (MAY_BE_OBJECT|MAY_BE_ARRAY_OF_ARRAY|MAY_BE_ARRAY_OF_OBJECT));
		case CREX_ASSIGN_OP:
			if (opline->extended_value == CREX_ADD) {
				if ((t1 & MAY_BE_ANY) == MAY_BE_ARRAY
				 && (t2 & MAY_BE_ANY) == MAY_BE_ARRAY) {
					return 0;
				}
				return (t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) ||
					(t2 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
			} else if (opline->extended_value == CREX_DIV ||
				opline->extended_value == CREX_MOD) {
				if (!OP2_HAS_RANGE() ||
					(OP2_MIN_RANGE() <= 0 && OP2_MAX_RANGE() >= 0)) {
					/* Division by zero */
					return 1;
				}
				return (t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) ||
					(t2 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
			} else if (opline->extended_value == CREX_SUB ||
				opline->extended_value == CREX_MUL ||
				opline->extended_value == CREX_POW) {
				return (t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) ||
					(t2 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
			} else if (opline->extended_value == CREX_SL ||
				opline->extended_value == CREX_SR) {
				return (t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) ||
					(t2 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) ||
					!OP2_HAS_RANGE() ||
					OP2_MIN_RANGE() < 0;
			} else if (opline->extended_value == CREX_CONCAT) {
				return (t1 & (MAY_BE_ARRAY|MAY_BE_OBJECT)) ||
					(t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT));
			} else if (opline->extended_value == CREX_BW_OR ||
				opline->extended_value == CREX_BW_AND ||
				opline->extended_value == CREX_BW_XOR) {
				if ((t1 & MAY_BE_ANY) == MAY_BE_STRING
				 && (t2 & MAY_BE_ANY) == MAY_BE_STRING) {
					return 0;
				}
				return (t1 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE)) ||
					(t2 & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
			}
			return 1;
		case CREX_ASSIGN:
			if (t1 & MAY_BE_REF) {
				return 1;
			}
			CREX_FALLTHROUGH;
		case CREX_UNSET_VAR:
			return (t1 & (MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_ARRAY_OF_OBJECT|MAY_BE_ARRAY_OF_RESOURCE|MAY_BE_ARRAY_OF_ARRAY));
		case CREX_BIND_STATIC:
		case CREX_BIND_INIT_STATIC_OR_JMP:
			if (t1 & (MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_ARRAY_OF_OBJECT|MAY_BE_ARRAY_OF_RESOURCE|MAY_BE_ARRAY_OF_ARRAY)) {
				/* Destructor may throw. */
				return 1;
			}
			return 0;
		case CREX_ASSIGN_DIM:
			if ((opline+1)->op1_type == IS_CV) {
				if (_ssa_op1_info(op_array, ssa, opline+1, ssa_op+1) & MAY_BE_UNDEF) {
					return 1;
				}
			}
			return (t1 & (MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_TRUE|MAY_BE_FALSE|MAY_BE_STRING|MAY_BE_LONG|MAY_BE_DOUBLE)) || opline->op2_type == IS_UNUSED ||
				(t2 & (MAY_BE_UNDEF|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
		case CREX_ASSIGN_OBJ:
			if (t1 & (MAY_BE_ANY-MAY_BE_OBJECT)) {
				return 1;
			}
			if ((opline+1)->op1_type == IS_CV) {
				if (_ssa_op1_info(op_array, ssa, opline+1, ssa_op+1) & MAY_BE_UNDEF) {
					return 1;
				}
			}
			if (ssa_op->op1_use) {
				const crex_ssa_var_info *var_info = ssa->var_info + ssa_op->op1_use;
				const crex_class_entry *ce = var_info->ce;

				if (var_info->is_instanceof ||
				    !ce || ce->create_object || ce->__get || ce->__set || ce->parent) {
					return 1;
				}

				if (opline->op2_type != IS_CONST) {
					return 1;
				}

				crex_string *prop_name = C_STR_P(CRT_CONSTANT(opline->op2));
				if (ZSTR_LEN(prop_name) > 0 && ZSTR_VAL(prop_name)[0] == '\0') {
					return 1;
				}

				crex_property_info *prop_info =
					crex_hash_find_ptr(&ce->properties_info, prop_name);
				if (prop_info) {
					if (CREX_TYPE_IS_SET(prop_info->type)) {
						return 1;
					}
					return !(prop_info->flags & CREX_ACC_PUBLIC)
						&& prop_info->ce != op_array->scope;
				} else {
					return !(ce->ce_flags & CREX_ACC_ALLOW_DYNAMIC_PROPERTIES);
				}
			}
			return 1;
		case CREX_ROPE_INIT:
		case CREX_ROPE_ADD:
		case CREX_ROPE_END:
			return t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT);
		case CREX_INIT_ARRAY:
			return (opline->op2_type != IS_UNUSED) && (t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
		case CREX_ADD_ARRAY_ELEMENT:
			return (opline->op2_type == IS_UNUSED) || (t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
		case CREX_STRLEN:
			return (t1 & MAY_BE_ANY) != MAY_BE_STRING;
		case CREX_COUNT:
			return (t1 & MAY_BE_ANY) != MAY_BE_ARRAY;
		case CREX_RECV_INIT:
			if (C_TYPE_P(CRT_CONSTANT(opline->op2)) == IS_CONSTANT_AST) {
				return 1;
			}
			if (op_array->fn_flags & CREX_ACC_HAS_TYPE_HINTS) {
				uint32_t arg_num = opline->op1.num;
				const crex_arg_info *cur_arg_info;

				if (EXPECTED(arg_num <= op_array->num_args)) {
					cur_arg_info = &op_array->arg_info[arg_num-1];
				} else if (UNEXPECTED(op_array->fn_flags & CREX_ACC_VARIADIC)) {
					cur_arg_info = &op_array->arg_info[op_array->num_args];
				} else {
					return 0;
				}
				return CREX_TYPE_IS_SET(cur_arg_info->type);
			} else {
				return 0;
			}
		case CREX_FETCH_IS:
			return (t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT));
		case CREX_ISSET_ISEMPTY_DIM_OBJ:
			return (t1 & MAY_BE_OBJECT) || (t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT));
		case CREX_FETCH_DIM_IS:
			return (t1 & MAY_BE_OBJECT) || (t2 & (MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE));
		case CREX_CAST:
			switch (opline->extended_value) {
				case IS_LONG:
				case IS_DOUBLE:
					return (t1 & MAY_BE_OBJECT);
				case IS_STRING:
					return (t1 & (MAY_BE_ARRAY|MAY_BE_OBJECT));
				case IS_ARRAY:
					return (t1 & MAY_BE_OBJECT);
				case IS_OBJECT:
					return 0;
				EMPTY_SWITCH_DEFAULT_CASE()
			}
			/* GCC is getting confused here for the Wimplicit-fallthrough warning with
			 * EMPTY_SWITCH_DEFAULT_CASE() macro */
			return 0;
		case CREX_ARRAY_KEY_EXISTS:
			if ((t2 & MAY_BE_ANY) != MAY_BE_ARRAY) {
				return 1;
			}
			if ((t1 & (MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE))) {
				return 1;
			}
			return 0;
		case CREX_FE_RESET_R:
		case CREX_FE_RESET_RW:
			if ((t1 & (MAY_BE_ANY|MAY_BE_REF)) != MAY_BE_ARRAY) {
				return 1;
			}
			return 0;
		case CREX_FE_FETCH_R:
			if ((t1 & (MAY_BE_ANY|MAY_BE_REF)) != MAY_BE_ARRAY) {
				return 1;
			}
			if (opline->op2_type == IS_CV
			 && (t2 & MAY_BE_RC1)
			 && (t2 & (MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_ARRAY_OF_OBJECT|MAY_BE_ARRAY_OF_RESOURCE|MAY_BE_ARRAY_OF_ARRAY))) {
				return 1;
			}
			return 0;
		case CREX_FETCH_DIM_W:
		case CREX_FETCH_LIST_W:
			if (t1 & (MAY_BE_FALSE|MAY_BE_TRUE|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_STRING|MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_REF)) {
				return 1;
			}
			if (t2 & (MAY_BE_RESOURCE|MAY_BE_ARRAY|MAY_BE_OBJECT)) {
				return 1;
			}
			if (opline->op2_type == IS_UNUSED) {
				return 1;
			}
			return 0;
		default:
			return 1;
	}
}

CREX_API bool crex_may_throw(const crex_op *opline, const crex_ssa_op *ssa_op, const crex_op_array *op_array, const crex_ssa *ssa)
{
	return crex_may_throw_ex(opline, ssa_op, op_array, ssa, OP1_INFO(), OP2_INFO());
}
