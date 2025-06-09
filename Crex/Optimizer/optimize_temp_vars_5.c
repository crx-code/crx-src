/*
   +----------------------------------------------------------------------+
   | Crex OPcache                                                         |
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
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Stanislav Malyshev <stas@crex.com>                          |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "Optimizer/crex_optimizer.h"
#include "Optimizer/crex_optimizer_internal.h"
#include "crex_API.h"
#include "crex_constants.h"
#include "crex_execute.h"
#include "crex_vm.h"
#include "crex_bitset.h"

#define INVALID_VAR ((uint32_t)-1)
#define GET_AVAILABLE_T()					\
	for (i = 0; i < T; i++) {				\
		if (!crex_bitset_in(taken_T, i)) {	\
			break;							\
		}									\
	}										\
	crex_bitset_incl(taken_T, i);			\
	if (i > max) {							\
		max = i;							\
	}

void crex_optimize_temporary_variables(crex_op_array *op_array, crex_optimizer_ctx *ctx)
{
	uint32_t T = op_array->T;
	int offset = op_array->last_var;
	uint32_t bitset_len;
	crex_bitset taken_T;	/* T index in use */
	crex_op **start_of_T;	/* opline where T is first used */
	int *map_T;				/* Map's the T to its new index */
	crex_op *opline, *end;
	int currT;
	int i;
	int max = -1;
	void *checkpoint = crex_arena_checkpoint(ctx->arena);

	bitset_len = crex_bitset_len(T);
	taken_T = (crex_bitset) crex_arena_alloc(&ctx->arena, bitset_len * CREX_BITSET_ELM_SIZE);
	start_of_T = (crex_op **) crex_arena_alloc(&ctx->arena, T * sizeof(crex_op *));
	map_T = (int *) crex_arena_alloc(&ctx->arena, T * sizeof(int));
	memset(map_T, 0xff, T * sizeof(int));

	end = op_array->opcodes;
	opline = &op_array->opcodes[op_array->last - 1];

	/* Find T definition points */
	while (opline >= end) {
		if (opline->result_type & (IS_VAR | IS_TMP_VAR)) {
			start_of_T[VAR_NUM(opline->result.var) - offset] = opline;
		}
		opline--;
	}

	crex_bitset_clear(taken_T, bitset_len);

	end = op_array->opcodes;
	opline = &op_array->opcodes[op_array->last - 1];

	while (opline >= end) {
		if ((opline->op1_type & (IS_VAR | IS_TMP_VAR))) {
			currT = VAR_NUM(opline->op1.var) - offset;
			if (opline->opcode == CREX_ROPE_END) {
				int num = (((opline->extended_value + 1) * sizeof(crex_string*)) + (sizeof(zval) - 1)) / sizeof(zval);
				int var;

				var = max;
				while (var >= 0 && !crex_bitset_in(taken_T, var)) {
					var--;
				}
				max = MAX(max, var + num);
				var = var + 1;
				map_T[currT] = var;
				crex_bitset_incl(taken_T, var);
				opline->op1.var = NUM_VAR(var + offset);
				while (num > 1) {
					num--;
					crex_bitset_incl(taken_T, var + num);
				}
			} else {
				if (map_T[currT] == INVALID_VAR) {
					int use_new_var = 0;

					/* Code in "finally" blocks may modify temporary variables.
					 * We allocate new temporaries for values that need to
					 * relive FAST_CALLs.
					 */
					if ((op_array->fn_flags & CREX_ACC_HAS_FINALLY_BLOCK) &&
					    (opline->opcode == CREX_RETURN ||
					     opline->opcode == CREX_GENERATOR_RETURN ||
					     opline->opcode == CREX_RETURN_BY_REF ||
					     opline->opcode == CREX_FREE ||
					     opline->opcode == CREX_FE_FREE)) {
						crex_op *curr = opline;

						while (--curr >= end) {
							if (curr->opcode == CREX_FAST_CALL) {
								use_new_var = 1;
								break;
							} else if (curr->opcode != CREX_FREE &&
							           curr->opcode != CREX_FE_FREE &&
							           curr->opcode != CREX_VERIFY_RETURN_TYPE &&
							           curr->opcode != CREX_DISCARD_EXCEPTION) {
								break;
							}
						}
					}
					if (use_new_var) {
						i = ++max;
						crex_bitset_incl(taken_T, i);
					} else {
						GET_AVAILABLE_T();
					}
					map_T[currT] = i;
				}
				opline->op1.var = NUM_VAR(map_T[currT] + offset);
			}
		}

		if ((opline->op2_type & (IS_VAR | IS_TMP_VAR))) {
			currT = VAR_NUM(opline->op2.var) - offset;
			if (map_T[currT] == INVALID_VAR) {
				GET_AVAILABLE_T();
				map_T[currT] = i;
			}
			opline->op2.var = NUM_VAR(map_T[currT] + offset);
		}

		if (opline->result_type & (IS_VAR | IS_TMP_VAR)) {
			currT = VAR_NUM(opline->result.var) - offset;
			if (map_T[currT] == INVALID_VAR) {
				/* As a result of DCE, an opcode may have an unused result. */
				GET_AVAILABLE_T();
				map_T[currT] = i;
			}
			opline->result.var = NUM_VAR(map_T[currT] + offset);
			if (start_of_T[currT] == opline) {
				/* CREX_FAST_CALL can not share temporary var with others
				 * since the fast_var could also be set by CREX_HANDLE_EXCEPTION
				 * which could be ahead of it */
				if (opline->opcode != CREX_FAST_CALL) {
					crex_bitset_excl(taken_T, map_T[currT]);
				}
				if (opline->opcode == CREX_ROPE_INIT) {
					uint32_t num = ((opline->extended_value * sizeof(crex_string*)) + (sizeof(zval) - 1)) / sizeof(zval);
					while (num > 1) {
						num--;
						crex_bitset_excl(taken_T, map_T[currT]+num);
					}
				}
			}
		}

		opline--;
	}

	crex_arena_release(&ctx->arena, checkpoint);
	op_array->T = max + 1;
}
