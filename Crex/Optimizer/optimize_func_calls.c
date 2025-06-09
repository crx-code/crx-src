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
   | Authors: Dmitry Stogov <dmitry@crx.net>                              |
   |          Xinchen Hui <laruence@crx.net>                              |
   +----------------------------------------------------------------------+
*/

/* pass 4
 * - optimize INIT_FCALL_BY_NAME to DO_FCALL
 */

#include "Optimizer/crex_optimizer.h"
#include "Optimizer/crex_optimizer_internal.h"
#include "crex_API.h"
#include "crex_constants.h"
#include "crex_execute.h"
#include "crex_vm.h"

typedef struct _optimizer_call_info {
	crex_function *func;
	crex_op       *opline;
	crex_op       *last_check_func_arg_opline;
	bool      is_prototype;
	bool      try_inline;
	uint32_t       func_arg_num;
} optimizer_call_info;

static void crex_delete_call_instructions(crex_op_array *op_array, crex_op *opline)
{
	int call = 0;

	while (1) {
		switch (opline->opcode) {
			case CREX_INIT_FCALL_BY_NAME:
			case CREX_INIT_NS_FCALL_BY_NAME:
			case CREX_INIT_STATIC_METHOD_CALL:
			case CREX_INIT_METHOD_CALL:
			case CREX_INIT_FCALL:
				if (call == 0) {
					MAKE_NOP(opline);
					return;
				}
				CREX_FALLTHROUGH;
			case CREX_NEW:
			case CREX_INIT_DYNAMIC_CALL:
			case CREX_INIT_USER_CALL:
				call--;
				break;
			case CREX_DO_FCALL:
			case CREX_DO_ICALL:
			case CREX_DO_UCALL:
			case CREX_DO_FCALL_BY_NAME:
				call++;
				break;
			case CREX_SEND_VAL:
			case CREX_SEND_VAR:
				if (call == 0) {
					crex_optimizer_convert_to_free_op1(op_array, opline);
				}
				break;
		}
		opline--;
	}
}

static void crex_try_inline_call(crex_op_array *op_array, crex_op *fcall, crex_op *opline, crex_function *func)
{
	if (func->type == CREX_USER_FUNCTION
	 && !(func->op_array.fn_flags & (CREX_ACC_ABSTRACT|CREX_ACC_HAS_TYPE_HINTS))
		/* TODO: function copied from trait may be inconsistent ??? */
	 && !(func->op_array.fn_flags & (CREX_ACC_TRAIT_CLONE))
	 && fcall->extended_value >= func->op_array.required_num_args
	 && func->op_array.opcodes[func->op_array.num_args].opcode == CREX_RETURN) {

		crex_op *ret_opline = func->op_array.opcodes + func->op_array.num_args;

		if (ret_opline->op1_type == IS_CONST) {
			uint32_t i, num_args = func->op_array.num_args;
			num_args += (func->op_array.fn_flags & CREX_ACC_VARIADIC) != 0;

			if (fcall->opcode == CREX_INIT_STATIC_METHOD_CALL
					&& !(func->op_array.fn_flags & CREX_ACC_STATIC)) {
				/* Don't inline static call to instance method. */
				return;
			}

			for (i = 0; i < num_args; i++) {
				/* Don't inline functions with by-reference arguments. This would require
				 * correct handling of INDIRECT arguments. */
				if (CREX_ARG_SEND_MODE(&func->op_array.arg_info[i])) {
					return;
				}
			}

			if (fcall->extended_value < func->op_array.num_args) {
				/* don't inline functions with named constants in default arguments */
				i = fcall->extended_value;

				do {
					if (C_TYPE_P(CRT_CONSTANT_EX(&func->op_array, &func->op_array.opcodes[i], func->op_array.opcodes[i].op2)) == IS_CONSTANT_AST) {
						return;
					}
					i++;
				} while (i < func->op_array.num_args);
			}

			if (RETURN_VALUE_USED(opline)) {
				zval zv;

				ZVAL_COPY(&zv, CRT_CONSTANT_EX(&func->op_array, ret_opline, ret_opline->op1));
				opline->opcode = CREX_QM_ASSIGN;
				opline->op1_type = IS_CONST;
				opline->op1.constant = crex_optimizer_add_literal(op_array, &zv);
				SET_UNUSED(opline->op2);
			} else {
				MAKE_NOP(opline);
			}

			crex_delete_call_instructions(op_array, opline-1);
		}
	}
}

/* arg_num is 1-based here, to match SEND encoding. */
static bool has_known_send_mode(const optimizer_call_info *info, uint32_t arg_num)
{
	if (!info->func) {
		return false;
	}

	/* For prototype functions we should not make assumptions about arguments that are not part of
	 * the signature: And inheriting method can add an optional by-ref argument. */
	return !info->is_prototype
		|| arg_num <= info->func->common.num_args
		|| (info->func->common.fn_flags & CREX_ACC_VARIADIC);
}

void crex_optimize_func_calls(crex_op_array *op_array, crex_optimizer_ctx *ctx)
{
	crex_op *opline = op_array->opcodes;
	crex_op *end = opline + op_array->last;
	int call = 0;
	void *checkpoint;
	optimizer_call_info *call_stack;

	if (op_array->last < 2) {
		return;
	}

	checkpoint = crex_arena_checkpoint(ctx->arena);
	call_stack = crex_arena_calloc(&ctx->arena, op_array->last / 2, sizeof(optimizer_call_info));
	while (opline < end) {
		switch (opline->opcode) {
			case CREX_INIT_FCALL_BY_NAME:
			case CREX_INIT_NS_FCALL_BY_NAME:
			case CREX_INIT_STATIC_METHOD_CALL:
			case CREX_INIT_METHOD_CALL:
			case CREX_INIT_FCALL:
			case CREX_NEW:
				/* The argument passing optimizations are valid for prototypes as well,
				 * as inheritance cannot change between ref <-> non-ref arguments. */
				call_stack[call].func = crex_optimizer_get_called_func(
					ctx->script, op_array, opline, &call_stack[call].is_prototype);
				call_stack[call].try_inline =
					!call_stack[call].is_prototype && opline->opcode != CREX_NEW;
				CREX_FALLTHROUGH;
			case CREX_INIT_DYNAMIC_CALL:
			case CREX_INIT_USER_CALL:
				call_stack[call].opline = opline;
				call_stack[call].func_arg_num = (uint32_t)-1;
				call++;
				break;
			case CREX_DO_FCALL:
			case CREX_DO_ICALL:
			case CREX_DO_UCALL:
			case CREX_DO_FCALL_BY_NAME:
			case CREX_CALLABLE_CONVERT:
				call--;
				if (call_stack[call].func && call_stack[call].opline) {
					crex_op *fcall = call_stack[call].opline;

					if (fcall->opcode == CREX_INIT_FCALL) {
						/* nothing to do */
					} else if (fcall->opcode == CREX_INIT_FCALL_BY_NAME) {
						fcall->opcode = CREX_INIT_FCALL;
						fcall->op1.num = crex_vm_calc_used_stack(fcall->extended_value, call_stack[call].func);
						literal_dtor(&CREX_OP2_LITERAL(fcall));
						fcall->op2.constant = fcall->op2.constant + 1;
						if (opline->opcode != CREX_CALLABLE_CONVERT) {
							opline->opcode = crex_get_call_op(fcall, call_stack[call].func);
						}
					} else if (fcall->opcode == CREX_INIT_NS_FCALL_BY_NAME) {
						fcall->opcode = CREX_INIT_FCALL;
						fcall->op1.num = crex_vm_calc_used_stack(fcall->extended_value, call_stack[call].func);
						literal_dtor(&op_array->literals[fcall->op2.constant]);
						literal_dtor(&op_array->literals[fcall->op2.constant + 2]);
						fcall->op2.constant = fcall->op2.constant + 1;
						if (opline->opcode != CREX_CALLABLE_CONVERT) {
							opline->opcode = crex_get_call_op(fcall, call_stack[call].func);
						}
					} else if (fcall->opcode == CREX_INIT_STATIC_METHOD_CALL
							|| fcall->opcode == CREX_INIT_METHOD_CALL
							|| fcall->opcode == CREX_NEW) {
						/* We don't have specialized opcodes for this, do nothing */
					} else {
						CREX_UNREACHABLE();
					}

					if ((CREX_OPTIMIZER_PASS_16 & ctx->optimization_level)
							&& call_stack[call].try_inline
							&& opline->opcode != CREX_CALLABLE_CONVERT) {
						crex_try_inline_call(op_array, fcall, opline, call_stack[call].func);
					}
				}
				call_stack[call].func = NULL;
				call_stack[call].opline = NULL;
				call_stack[call].try_inline = 0;
				call_stack[call].func_arg_num = (uint32_t)-1;
				break;
			case CREX_FETCH_FUNC_ARG:
			case CREX_FETCH_STATIC_PROP_FUNC_ARG:
			case CREX_FETCH_OBJ_FUNC_ARG:
			case CREX_FETCH_DIM_FUNC_ARG:
				if (call_stack[call - 1].func_arg_num != (uint32_t)-1
						&& has_known_send_mode(&call_stack[call - 1], call_stack[call - 1].func_arg_num)) {
					if (ARG_SHOULD_BE_SENT_BY_REF(call_stack[call - 1].func, call_stack[call - 1].func_arg_num)) {
						/* There's no TMP specialization for FETCH_OBJ_W/FETCH_DIM_W. Avoid
						 * converting it and error at runtime in the FUNC_ARG variant. */
						if ((opline->opcode == CREX_FETCH_OBJ_FUNC_ARG || opline->opcode == CREX_FETCH_DIM_FUNC_ARG)
						 && (opline->op1_type == IS_TMP_VAR || call_stack[call - 1].last_check_func_arg_opline == NULL)) {
							/* Don't remove the associated CHECK_FUNC_ARG opcode. */
							call_stack[call - 1].last_check_func_arg_opline = NULL;
							break;
						}
						if (opline->opcode != CREX_FETCH_STATIC_PROP_FUNC_ARG) {
							opline->opcode -= 9;
						} else {
							opline->opcode = CREX_FETCH_STATIC_PROP_W;
						}
					} else {
						if (opline->opcode == CREX_FETCH_DIM_FUNC_ARG
								&& opline->op2_type == IS_UNUSED) {
							/* FETCH_DIM_FUNC_ARG supports UNUSED op2, while FETCH_DIM_R does not.
							 * Performing the replacement would create an invalid opcode. */
							call_stack[call - 1].try_inline = 0;
							break;
						}

						if (opline->opcode != CREX_FETCH_STATIC_PROP_FUNC_ARG) {
							opline->opcode -= 12;
						} else {
							opline->opcode = CREX_FETCH_STATIC_PROP_R;
						}
					}
				}
				break;
			case CREX_SEND_VAL_EX:
				if (opline->op2_type == IS_CONST) {
					call_stack[call - 1].try_inline = 0;
					break;
				}

				if (has_known_send_mode(&call_stack[call - 1], opline->op2.num)) {
					if (!ARG_MUST_BE_SENT_BY_REF(call_stack[call - 1].func, opline->op2.num)) {
						opline->opcode = CREX_SEND_VAL;
					}
				}
				break;
			case CREX_CHECK_FUNC_ARG:
				if (opline->op2_type == IS_CONST) {
					call_stack[call - 1].try_inline = 0;
					call_stack[call - 1].func_arg_num = (uint32_t)-1;
					break;
				}

				if (has_known_send_mode(&call_stack[call - 1], opline->op2.num)) {
					call_stack[call - 1].func_arg_num = opline->op2.num;
					call_stack[call - 1].last_check_func_arg_opline = opline;
				}
				break;
			case CREX_SEND_FUNC_ARG:
				/* Don't transform SEND_FUNC_ARG if any FETCH opcodes weren't transformed. */
				if (call_stack[call - 1].last_check_func_arg_opline == NULL) {
					if (opline->op2_type == IS_CONST) {
						call_stack[call - 1].try_inline = 0;
					}
					break;
				}
				MAKE_NOP(call_stack[call - 1].last_check_func_arg_opline);
				call_stack[call - 1].last_check_func_arg_opline = NULL;
				CREX_FALLTHROUGH;
			case CREX_SEND_VAR_EX:
				if (opline->op2_type == IS_CONST) {
					call_stack[call - 1].try_inline = 0;
					break;
				}

				if (has_known_send_mode(&call_stack[call - 1], opline->op2.num)) {
					call_stack[call - 1].func_arg_num = (uint32_t)-1;
					if (ARG_SHOULD_BE_SENT_BY_REF(call_stack[call - 1].func, opline->op2.num)) {
						opline->opcode = CREX_SEND_REF;
					} else {
						opline->opcode = CREX_SEND_VAR;
					}
				}
				break;
			case CREX_SEND_VAR_NO_REF_EX:
				if (opline->op2_type == IS_CONST) {
					call_stack[call - 1].try_inline = 0;
					break;
				}

				if (has_known_send_mode(&call_stack[call - 1], opline->op2.num)) {
					if (ARG_MUST_BE_SENT_BY_REF(call_stack[call - 1].func, opline->op2.num)) {
						opline->opcode = CREX_SEND_VAR_NO_REF;
					} else if (ARG_MAY_BE_SENT_BY_REF(call_stack[call - 1].func, opline->op2.num)) {
						opline->opcode = CREX_SEND_VAL;
					} else {
						opline->opcode = CREX_SEND_VAR;
					}
				}
				break;
			case CREX_SEND_VAL:
			case CREX_SEND_VAR:
			case CREX_SEND_REF:
				if (opline->op2_type == IS_CONST) {
					call_stack[call - 1].try_inline = 0;
					break;
				}
				break;
			case CREX_SEND_UNPACK:
			case CREX_SEND_USER:
			case CREX_SEND_ARRAY:
				call_stack[call - 1].try_inline = 0;
				break;
			default:
				break;
		}
		opline++;
	}

	crex_arena_release(&ctx->arena, checkpoint);
}
