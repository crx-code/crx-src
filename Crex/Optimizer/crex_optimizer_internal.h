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

#ifndef CREX_OPTIMIZER_INTERNAL_H
#define CREX_OPTIMIZER_INTERNAL_H

#include "crex_ssa.h"
#include "crex_func_info.h"

#include <stdint.h>

#define CREX_OP1_LITERAL(opline)		(op_array)->literals[(opline)->op1.constant]
#define CREX_OP1_JMP_ADDR(opline)		OP_JMP_ADDR(opline, (opline)->op1)
#define CREX_OP2_LITERAL(opline)		(op_array)->literals[(opline)->op2.constant]
#define CREX_OP2_JMP_ADDR(opline)		OP_JMP_ADDR(opline, (opline)->op2)

#define VAR_NUM(v) EX_VAR_TO_NUM(v)
#define NUM_VAR(v) EX_NUM_TO_VAR(v)

#define INV_COND(op)       ((op) == CREX_JMPZ    ? CREX_JMPNZ    : CREX_JMPZ)
#define INV_EX_COND(op)    ((op) == CREX_JMPC_EX ? CREX_JMPNZ    : CREX_JMPZ)
#define INV_COND_EX(op)    ((op) == CREX_JMPZ    ? CREX_JMPNC_EX : CREX_JMPC_EX)
#define INV_EX_COND_EX(op) ((op) == CREX_JMPC_EX ? CREX_JMPNC_EX : CREX_JMPC_EX)

#define RESULT_UNUSED(op)	(op->result_type == IS_UNUSED)
#define SAME_VAR(op1, op2)  (op1 ## _type == op2 ## _type && op1.var == op2.var)

typedef struct _crex_optimizer_ctx {
	crex_arena             *arena;
	crex_script            *script;
	HashTable              *constants;
	crex_long               optimization_level;
	crex_long               debug_level;
} crex_optimizer_ctx;

#define LITERAL_LONG(op, val) do { \
		zval _c; \
		ZVAL_LONG(&_c, val); \
		op.constant = crex_optimizer_add_literal(op_array, &_c); \
	} while (0)

#define LITERAL_BOOL(op, val) do { \
		zval _c; \
		ZVAL_BOOL(&_c, val); \
		op.constant = crex_optimizer_add_literal(op_array, &_c); \
	} while (0)

#define literal_dtor(zv) do { \
		zval_ptr_dtor_nogc(zv); \
		ZVAL_NULL(zv); \
	} while (0)

#define COPY_NODE(target, src) do { \
		target ## _type = src ## _type; \
		target = src; \
	} while (0)

static inline bool crex_optimizer_is_loop_var_free(const crex_op *opline) {
	return (opline->opcode == CREX_FE_FREE && opline->extended_value != CREX_FREE_ON_RETURN)
		|| (opline->opcode == CREX_FREE && opline->extended_value == CREX_FREE_SWITCH);
}

void crex_optimizer_convert_to_free_op1(crex_op_array *op_array, crex_op *opline);
int  crex_optimizer_add_literal(crex_op_array *op_array, const zval *zv);
bool crex_optimizer_get_persistent_constant(crex_string *name, zval *result, int copy);
void crex_optimizer_collect_constant(crex_optimizer_ctx *ctx, zval *name, zval* value);
bool crex_optimizer_get_collected_constant(HashTable *constants, zval *name, zval* value);
crex_result crex_optimizer_eval_binary_op(zval *result, uint8_t opcode, zval *op1, zval *op2);
crex_result crex_optimizer_eval_unary_op(zval *result, uint8_t opcode, zval *op1);
crex_result crex_optimizer_eval_cast(zval *result, uint32_t type, zval *op1);
crex_result crex_optimizer_eval_strlen(zval *result, const zval *op1);
crex_result crex_optimizer_eval_special_func_call(
		zval *result, crex_string *name, crex_string *arg);
bool crex_optimizer_update_op1_const(crex_op_array *op_array,
                                    crex_op       *opline,
                                    zval          *val);
bool crex_optimizer_update_op2_const(crex_op_array *op_array,
                                    crex_op       *opline,
                                    zval          *val);
bool crex_optimizer_replace_by_const(crex_op_array *op_array,
                                     crex_op       *opline,
                                     uint8_t     type,
                                     uint32_t       var,
                                     zval          *val);
crex_op *crex_optimizer_get_loop_var_def(const crex_op_array *op_array, crex_op *free_opline);
crex_class_entry *crex_optimizer_get_class_entry(
		const crex_script *script, const crex_op_array *op_array, crex_string *lcname);
crex_class_entry *crex_optimizer_get_class_entry_from_op1(
		const crex_script *script, const crex_op_array *op_array, const crex_op *opline);

void crex_optimizer_pass1(crex_op_array *op_array, crex_optimizer_ctx *ctx);
void crex_optimizer_pass3(crex_op_array *op_array, crex_optimizer_ctx *ctx);
void crex_optimize_func_calls(crex_op_array *op_array, crex_optimizer_ctx *ctx);
void crex_optimize_cfg(crex_op_array *op_array, crex_optimizer_ctx *ctx);
void crex_optimize_dfa(crex_op_array *op_array, crex_optimizer_ctx *ctx);
crex_result crex_dfa_analyze_op_array(crex_op_array *op_array, crex_optimizer_ctx *ctx, crex_ssa *ssa);
void crex_dfa_optimize_op_array(crex_op_array *op_array, crex_optimizer_ctx *ctx, crex_ssa *ssa, crex_call_info **call_map);
void crex_optimize_temporary_variables(crex_op_array *op_array, crex_optimizer_ctx *ctx);
void crex_optimizer_nop_removal(crex_op_array *op_array, crex_optimizer_ctx *ctx);
void crex_optimizer_compact_literals(crex_op_array *op_array, crex_optimizer_ctx *ctx);
void crex_optimizer_compact_vars(crex_op_array *op_array);
crex_function *crex_optimizer_get_called_func(
		crex_script *script, crex_op_array *op_array, crex_op *opline, bool *is_prototype);
uint32_t crex_optimizer_classify_function(crex_string *name, uint32_t num_args);
void crex_optimizer_migrate_jump(crex_op_array *op_array, crex_op *new_opline, crex_op *opline);
void crex_optimizer_shift_jump(crex_op_array *op_array, crex_op *opline, uint32_t *shiftlist);
int sccp_optimize_op_array(crex_optimizer_ctx *ctx, crex_op_array *op_array, crex_ssa *ssa, crex_call_info **call_map);
int dce_optimize_op_array(crex_op_array *op_array, crex_optimizer_ctx *optimizer_ctx, crex_ssa *ssa, bool reorder_dtor_effects);
crex_result crex_ssa_escape_analysis(const crex_script *script, crex_op_array *op_array, crex_ssa *ssa);

typedef void (*crex_op_array_func_t)(crex_op_array *, void *context);
void crex_foreach_op_array(crex_script *script, crex_op_array_func_t func, void *context);

#endif
