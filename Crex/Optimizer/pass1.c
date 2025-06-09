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

/* pass 1 (Simple local optimizations)
 * - persistent constant substitution (true, false, null, etc)
 * - constant casting (ADD expects numbers, CONCAT strings, etc)
 * - constant expression evaluation
 * - optimize constant conditional JMPs
 * - pre-evaluate constant function calls
 */

#include "Optimizer/crex_optimizer.h"
#include "Optimizer/crex_optimizer_internal.h"
#include "crex_API.h"
#include "crex_constants.h"
#include "crex_execute.h"
#include "crex_vm.h"

static void replace_by_const_or_qm_assign(crex_op_array *op_array, crex_op *opline, zval *result) {
	if (opline->op1_type == IS_CONST) {
		literal_dtor(&CREX_OP1_LITERAL(opline));
	}
	if (opline->op2_type == IS_CONST) {
		literal_dtor(&CREX_OP2_LITERAL(opline));
	}
	if (crex_optimizer_replace_by_const(op_array, opline + 1, opline->result_type, opline->result.var, result)) {
		MAKE_NOP(opline);
	} else {
		opline->opcode = CREX_QM_ASSIGN;
		opline->extended_value = 0;
		SET_UNUSED(opline->op2);
		crex_optimizer_update_op1_const(op_array, opline, result);
	}
}

void crex_optimizer_pass1(crex_op_array *op_array, crex_optimizer_ctx *ctx)
{
	crex_op *opline = op_array->opcodes;
	crex_op *end = opline + op_array->last;
	bool collect_constants = (CREX_OPTIMIZER_PASS_15 & ctx->optimization_level)?
		(op_array == &ctx->script->main_op_array) : 0;
	zval result;

	while (opline < end) {
		switch (opline->opcode) {
		case CREX_CONCAT:
		case CREX_FAST_CONCAT:
			if (opline->op1_type == IS_CONST && C_TYPE(CREX_OP1_LITERAL(opline)) != IS_STRING) {
				convert_to_string(&CREX_OP1_LITERAL(opline));
			}
			if (opline->op2_type == IS_CONST && C_TYPE(CREX_OP2_LITERAL(opline)) != IS_STRING) {
				convert_to_string(&CREX_OP2_LITERAL(opline));
			}
			CREX_FALLTHROUGH;
		case CREX_ADD:
		case CREX_SUB:
		case CREX_MUL:
		case CREX_DIV:
		case CREX_POW:
		case CREX_MOD:
		case CREX_SL:
		case CREX_SR:
		case CREX_BW_OR:
		case CREX_BW_AND:
		case CREX_BW_XOR:
		case CREX_IS_EQUAL:
		case CREX_IS_NOT_EQUAL:
		case CREX_IS_SMALLER:
		case CREX_IS_SMALLER_OR_EQUAL:
		case CREX_IS_IDENTICAL:
		case CREX_IS_NOT_IDENTICAL:
		case CREX_BOOL_XOR:
		case CREX_SPACESHIP:
		case CREX_CASE:
		case CREX_CASE_STRICT:
			if (opline->op1_type == IS_CONST && opline->op2_type == IS_CONST &&
					crex_optimizer_eval_binary_op(&result, opline->opcode, &CREX_OP1_LITERAL(opline), &CREX_OP2_LITERAL(opline)) == SUCCESS) {
				replace_by_const_or_qm_assign(op_array, opline, &result);
			}
			break;

		case CREX_ASSIGN_OP:
			if (opline->extended_value == CREX_CONCAT && opline->op2_type == IS_CONST
					&& C_TYPE(CREX_OP2_LITERAL(opline)) != IS_STRING) {
				convert_to_string(&CREX_OP2_LITERAL(opline));
			}
			break;

		case CREX_CAST:
			if (opline->op1_type == IS_CONST &&
					crex_optimizer_eval_cast(&result, opline->extended_value, &CREX_OP1_LITERAL(opline)) == SUCCESS) {
				replace_by_const_or_qm_assign(op_array, opline, &result);
			}
			break;

		case CREX_BW_NOT:
		case CREX_BOOL_NOT:
			if (opline->op1_type == IS_CONST &&
					crex_optimizer_eval_unary_op(&result, opline->opcode, &CREX_OP1_LITERAL(opline)) == SUCCESS) {
				replace_by_const_or_qm_assign(op_array, opline, &result);
			}
			break;

		case CREX_FETCH_CONSTANT:
			if (opline->op2_type == IS_CONST &&
				C_TYPE(CREX_OP2_LITERAL(opline)) == IS_STRING &&
				crex_string_equals_literal(C_STR(CREX_OP2_LITERAL(opline)), "__COMPILER_HALT_OFFSET__")) {
				/* substitute __COMPILER_HALT_OFFSET__ constant */
				crex_execute_data *orig_execute_data = EG(current_execute_data);
				crex_execute_data fake_execute_data;
				zval *offset;

				memset(&fake_execute_data, 0, sizeof(crex_execute_data));
				fake_execute_data.func = (crex_function*)op_array;
				EG(current_execute_data) = &fake_execute_data;
				if ((offset = crex_get_constant_str("__COMPILER_HALT_OFFSET__", sizeof("__COMPILER_HALT_OFFSET__") - 1)) != NULL) {

					literal_dtor(&CREX_OP2_LITERAL(opline));
					replace_by_const_or_qm_assign(op_array, opline, offset);
				}
				EG(current_execute_data) = orig_execute_data;
				break;
			}

			if (opline->op2_type == IS_CONST &&
				C_TYPE(CREX_OP2_LITERAL(opline)) == IS_STRING) {
				/* substitute persistent constants */
				if (!crex_optimizer_get_persistent_constant(C_STR(CREX_OP2_LITERAL(opline)), &result, 1)) {
					if (!ctx->constants || !crex_optimizer_get_collected_constant(ctx->constants, &CREX_OP2_LITERAL(opline), &result)) {
						break;
					}
				}
				if (C_TYPE(result) == IS_CONSTANT_AST) {
					break;
				}
				replace_by_const_or_qm_assign(op_array, opline, &result);
			}
			break;

		case CREX_FETCH_CLASS_CONSTANT:
			if (opline->op2_type == IS_CONST &&
				C_TYPE(CREX_OP2_LITERAL(opline)) == IS_STRING) {

				crex_class_entry *ce = crex_optimizer_get_class_entry_from_op1(
					ctx->script, op_array, opline);
				if (ce) {
					crex_class_constant *cc = crex_hash_find_ptr(
						&ce->constants_table, C_STR(CREX_OP2_LITERAL(opline)));
					if (cc && !(CREX_CLASS_CONST_FLAGS(cc) & CREX_ACC_DEPRECATED) && (CREX_CLASS_CONST_FLAGS(cc) & CREX_ACC_PPP_MASK) == CREX_ACC_PUBLIC && !(ce->ce_flags & CREX_ACC_TRAIT)) {
						zval *c = &cc->value;
						if (C_TYPE_P(c) == IS_CONSTANT_AST) {
							crex_ast *ast = C_ASTVAL_P(c);
							if (ast->kind != CREX_AST_CONSTANT
							 || !crex_optimizer_get_persistent_constant(crex_ast_get_constant_name(ast), &result, 1)
							 || C_TYPE(result) == IS_CONSTANT_AST) {
								break;
							}
						} else {
							ZVAL_COPY_OR_DUP(&result, c);
						}

						replace_by_const_or_qm_assign(op_array, opline, &result);
					}
				}
			}
			break;

		case CREX_DO_ICALL: {
			crex_op *send1_opline = opline - 1;
			crex_op *send2_opline = NULL;
			crex_op *init_opline = NULL;

			while (send1_opline->opcode == CREX_NOP) {
				send1_opline--;
			}
			if (send1_opline->opcode != CREX_SEND_VAL ||
			    send1_opline->op1_type != IS_CONST) {
				/* don't collect constants after unknown function call */
				collect_constants = 0;
				break;
			}
			if (send1_opline->op2.num == 2) {
				send2_opline = send1_opline;
				send1_opline--;
				while (send1_opline->opcode == CREX_NOP) {
					send1_opline--;
				}
				if (send1_opline->opcode != CREX_SEND_VAL ||
				    send1_opline->op1_type != IS_CONST) {
					/* don't collect constants after unknown function call */
					collect_constants = 0;
					break;
				}
			}
			init_opline = send1_opline - 1;
			while (init_opline->opcode == CREX_NOP) {
				init_opline--;
			}
			if (init_opline->opcode != CREX_INIT_FCALL ||
			    init_opline->op2_type != IS_CONST ||
			    C_TYPE(CREX_OP2_LITERAL(init_opline)) != IS_STRING) {
				/* don't collect constants after unknown function call */
				collect_constants = 0;
				break;
			}

			/* define("name", scalar); */
			if (crex_string_equals_literal_ci(C_STR(CREX_OP2_LITERAL(init_opline)), "define")) {

				if (C_TYPE(CREX_OP1_LITERAL(send1_opline)) == IS_STRING && send2_opline) {

					if (collect_constants) {
						crex_optimizer_collect_constant(ctx, &CREX_OP1_LITERAL(send1_opline), &CREX_OP1_LITERAL(send2_opline));
					}

					if (RESULT_UNUSED(opline) &&
					    !crex_memnstr(C_STRVAL(CREX_OP1_LITERAL(send1_opline)), "::", sizeof("::") - 1, C_STRVAL(CREX_OP1_LITERAL(send1_opline)) + C_STRLEN(CREX_OP1_LITERAL(send1_opline)))) {

						opline->opcode = CREX_DECLARE_CONST;
						opline->op1_type = IS_CONST;
						opline->op2_type = IS_CONST;
						opline->result_type = IS_UNUSED;
						opline->op1.constant = send1_opline->op1.constant;
						opline->op2.constant = send2_opline->op1.constant;
						opline->result.num = 0;

						literal_dtor(&CREX_OP2_LITERAL(init_opline));
						MAKE_NOP(init_opline);
						MAKE_NOP(send1_opline);
						MAKE_NOP(send2_opline);
					}
					break;
				}
			}

			if (!send2_opline && C_TYPE(CREX_OP1_LITERAL(send1_opline)) == IS_STRING &&
					crex_optimizer_eval_special_func_call(&result, C_STR(CREX_OP2_LITERAL(init_opline)), C_STR(CREX_OP1_LITERAL(send1_opline))) == SUCCESS) {
				literal_dtor(&CREX_OP2_LITERAL(init_opline));
				MAKE_NOP(init_opline);
				literal_dtor(&CREX_OP1_LITERAL(send1_opline));
				MAKE_NOP(send1_opline);
				replace_by_const_or_qm_assign(op_array, opline, &result);
				break;
			}

			/* don't collect constants after any other function call */
			collect_constants = 0;
			break;
		}
		case CREX_STRLEN:
			if (opline->op1_type == IS_CONST &&
					crex_optimizer_eval_strlen(&result, &CREX_OP1_LITERAL(opline)) == SUCCESS) {
				replace_by_const_or_qm_assign(op_array, opline, &result);
			}
			break;
		case CREX_DEFINED:
			if (!crex_optimizer_get_persistent_constant(C_STR(CREX_OP1_LITERAL(opline)), &result, 0)) {
				break;
			}
			ZVAL_TRUE(&result);
			literal_dtor(&CREX_OP1_LITERAL(opline));
			replace_by_const_or_qm_assign(op_array, opline, &result);
			break;
		case CREX_DECLARE_CONST:
			if (collect_constants &&
			    C_TYPE(CREX_OP1_LITERAL(opline)) == IS_STRING &&
			    C_TYPE(CREX_OP2_LITERAL(opline)) != IS_CONSTANT_AST) {
				crex_optimizer_collect_constant(ctx, &CREX_OP1_LITERAL(opline), &CREX_OP2_LITERAL(opline));
			}
			break;

		case CREX_JMPC_EX:
		case CREX_JMPNC_EX:
			/* convert Ti = JMPC_EX(C, L) => Ti = QM_ASSIGN(C)
			   in case we know it wouldn't jump */
			if (opline->op1_type == IS_CONST) {
				if (crex_is_true(&CREX_OP1_LITERAL(opline))) {
					if (opline->opcode == CREX_JMPC_EX) {
						opline->opcode = CREX_QM_ASSIGN;
						zval_ptr_dtor_nogc(&CREX_OP1_LITERAL(opline));
						ZVAL_TRUE(&CREX_OP1_LITERAL(opline));
						opline->op2.num = 0;
						break;
					}
				} else {
					if (opline->opcode == CREX_JMPNC_EX) {
						opline->opcode = CREX_QM_ASSIGN;
						zval_ptr_dtor_nogc(&CREX_OP1_LITERAL(opline));
						ZVAL_FALSE(&CREX_OP1_LITERAL(opline));
						opline->op2.num = 0;
						break;
					}
				}
			}
			collect_constants = 0;
			break;

		case CREX_JMPZ:
		case CREX_JMPNZ:
			if (opline->op1_type == IS_CONST) {
				int should_jmp = crex_is_true(&CREX_OP1_LITERAL(opline));

				if (opline->opcode == CREX_JMPZ) {
					should_jmp = !should_jmp;
				}
				literal_dtor(&CREX_OP1_LITERAL(opline));
				opline->op1_type = IS_UNUSED;
				if (should_jmp) {
					opline->opcode = CREX_JMP;
					COPY_NODE(opline->op1, opline->op2);
					opline->op2.num = 0;
				} else {
					MAKE_NOP(opline);
					break;
				}
			}
			collect_constants = 0;
			break;

		case CREX_RETURN:
		case CREX_RETURN_BY_REF:
		case CREX_GENERATOR_RETURN:
		case CREX_EXIT:
		case CREX_THROW:
		case CREX_MATCH_ERROR:
		case CREX_CATCH:
		case CREX_FAST_CALL:
		case CREX_FAST_RET:
		case CREX_JMP:
		case CREX_FE_RESET_R:
		case CREX_FE_RESET_RW:
		case CREX_FE_FETCH_R:
		case CREX_FE_FETCH_RW:
		case CREX_JMP_SET:
		case CREX_COALESCE:
		case CREX_ASSERT_CHECK:
		case CREX_JMP_NULL:
		case CREX_VERIFY_NEVER_TYPE:
		case CREX_BIND_INIT_STATIC_OR_JMP:
			collect_constants = 0;
			break;
		}
		opline++;
	}
}
