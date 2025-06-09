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
#include "crex_cfg.h"
#include "crex_func_info.h"
#include "crex_call_graph.h"
#include "crex_inference.h"
#include "crex_dump.h"
#include "crx.h"
#include "crex_observer.h"

#ifndef CREX_OPTIMIZER_MAX_REGISTERED_PASSES
# define CREX_OPTIMIZER_MAX_REGISTERED_PASSES 32
#endif

struct {
	crex_optimizer_pass_t pass[CREX_OPTIMIZER_MAX_REGISTERED_PASSES];
	int last;
} crex_optimizer_registered_passes = {{NULL}, 0};

void crex_optimizer_collect_constant(crex_optimizer_ctx *ctx, zval *name, zval* value)
{
	if (!ctx->constants) {
		ctx->constants = crex_arena_alloc(&ctx->arena, sizeof(HashTable));
		crex_hash_init(ctx->constants, 16, NULL, zval_ptr_dtor_nogc, 0);
	}

	if (crex_hash_add(ctx->constants, C_STR_P(name), value)) {
		C_TRY_ADDREF_P(value);
	}
}

crex_result crex_optimizer_eval_binary_op(zval *result, uint8_t opcode, zval *op1, zval *op2) /* {{{ */
{
	if (crex_binary_op_produces_error(opcode, op1, op2)) {
		return FAILURE;
	}

	binary_op_type binary_op = get_binary_op(opcode);
	return binary_op(result, op1, op2);
}
/* }}} */

crex_result crex_optimizer_eval_unary_op(zval *result, uint8_t opcode, zval *op1) /* {{{ */
{
	unary_op_type unary_op = get_unary_op(opcode);

	if (unary_op) {
		if (crex_unary_op_produces_error(opcode, op1)) {
			return FAILURE;
		}
		return unary_op(result, op1);
	} else { /* CREX_BOOL */
		ZVAL_BOOL(result, crex_is_true(op1));
		return SUCCESS;
	}
}
/* }}} */

crex_result crex_optimizer_eval_cast(zval *result, uint32_t type, zval *op1) /* {{{ */
{
	switch (type) {
		case IS_NULL:
			ZVAL_NULL(result);
			return SUCCESS;
		case _IS_BOOL:
			ZVAL_BOOL(result, zval_is_true(op1));
			return SUCCESS;
		case IS_LONG:
			ZVAL_LONG(result, zval_get_long(op1));
			return SUCCESS;
		case IS_DOUBLE:
			ZVAL_DOUBLE(result, zval_get_double(op1));
			return SUCCESS;
		case IS_STRING:
			/* Conversion from double to string takes into account run-time
			   'precision' setting and cannot be evaluated at compile-time */
			if (C_TYPE_P(op1) != IS_ARRAY && C_TYPE_P(op1) != IS_DOUBLE) {
				ZVAL_STR(result, zval_get_string(op1));
				return SUCCESS;
			}
			break;
		case IS_ARRAY:
			ZVAL_COPY(result, op1);
			convert_to_array(result);
			return SUCCESS;
	}
	return FAILURE;
}
/* }}} */

crex_result crex_optimizer_eval_strlen(zval *result, const zval *op1) /* {{{ */
{
	if (C_TYPE_P(op1) != IS_STRING) {
		return FAILURE;
	}
	ZVAL_LONG(result, C_STRLEN_P(op1));
	return SUCCESS;
}
/* }}} */

crex_result crex_optimizer_eval_special_func_call(
		zval *result, crex_string *name, crex_string *arg) {
	if (crex_string_equals_literal(name, "function_exists") ||
			crex_string_equals_literal(name, "is_callable")) {
		crex_string *lc_name = crex_string_tolower(arg);
		crex_internal_function *func = crex_hash_find_ptr(EG(function_table), lc_name);
		crex_string_release_ex(lc_name, 0);

		if (func && func->type == CREX_INTERNAL_FUNCTION
				&& func->module->type == MODULE_PERSISTENT
#ifdef CREX_WIN32
				&& func->module->handle == NULL
#endif
		) {
			ZVAL_TRUE(result);
			return SUCCESS;
		}
		return FAILURE;
	}
	if (crex_string_equals_literal(name, "extension_loaded")) {
		crex_string *lc_name = crex_string_tolower(arg);
		crex_module_entry *m = crex_hash_find_ptr(&module_registry, lc_name);
		crex_string_release_ex(lc_name, 0);

		if (!m) {
			if (PG(enable_dl)) {
				return FAILURE;
			}
			ZVAL_FALSE(result);
			return SUCCESS;
		}

		if (m->type == MODULE_PERSISTENT
#ifdef CREX_WIN32
			&& m->handle == NULL
#endif
		) {
			ZVAL_TRUE(result);
			return SUCCESS;
		}
		return FAILURE;
	}
	if (crex_string_equals_literal(name, "constant")) {
		return crex_optimizer_get_persistent_constant(arg, result, 1) ? SUCCESS : FAILURE;
	}
	if (crex_string_equals_literal(name, "dirname")) {
		if (!IS_ABSOLUTE_PATH(ZSTR_VAL(arg), ZSTR_LEN(arg))) {
			return FAILURE;
		}

		crex_string *dirname = crex_string_init(ZSTR_VAL(arg), ZSTR_LEN(arg), 0);
		ZSTR_LEN(dirname) = crex_dirname(ZSTR_VAL(dirname), ZSTR_LEN(dirname));
		if (IS_ABSOLUTE_PATH(ZSTR_VAL(dirname), ZSTR_LEN(dirname))) {
			ZVAL_STR(result, dirname);
			return SUCCESS;
		}
		crex_string_release_ex(dirname, 0);
		return FAILURE;
	}
	if (crex_string_equals_literal(name, "ini_get")) {
		crex_ini_entry *ini_entry = crex_hash_find_ptr(EG(ini_directives), arg);
		if (!ini_entry) {
			if (PG(enable_dl)) {
				return FAILURE;
			}
			ZVAL_FALSE(result);
		} else if (ini_entry->modifiable != CREX_INI_SYSTEM) {
			return FAILURE;
		} else if (ini_entry->value) {
			ZVAL_STR_COPY(result, ini_entry->value);
		} else {
			ZVAL_EMPTY_STRING(result);
		}
		return SUCCESS;
	}
	return FAILURE;
}

bool crex_optimizer_get_collected_constant(HashTable *constants, zval *name, zval* value)
{
	zval *val;

	if ((val = crex_hash_find(constants, C_STR_P(name))) != NULL) {
		ZVAL_COPY(value, val);
		return 1;
	}
	return 0;
}

void crex_optimizer_convert_to_free_op1(crex_op_array *op_array, crex_op *opline)
{
	if (opline->op1_type == IS_CV) {
		opline->opcode = CREX_CHECK_VAR;
		SET_UNUSED(opline->op2);
		SET_UNUSED(opline->result);
		opline->extended_value = 0;
	} else if (opline->op1_type & (IS_TMP_VAR|IS_VAR)) {
		opline->opcode = CREX_FREE;
		SET_UNUSED(opline->op2);
		SET_UNUSED(opline->result);
		opline->extended_value = 0;
	} else {
		CREX_ASSERT(opline->op1_type == IS_CONST);
		literal_dtor(&CREX_OP1_LITERAL(opline));
		MAKE_NOP(opline);
	}
}

int crex_optimizer_add_literal(crex_op_array *op_array, const zval *zv)
{
	int i = op_array->last_literal;
	op_array->last_literal++;
	op_array->literals = (zval*)erealloc(op_array->literals, op_array->last_literal * sizeof(zval));
	ZVAL_COPY_VALUE(&op_array->literals[i], zv);
	C_EXTRA(op_array->literals[i]) = 0;
	return i;
}

static inline int crex_optimizer_add_literal_string(crex_op_array *op_array, crex_string *str) {
	zval zv;
	ZVAL_STR(&zv, str);
	crex_string_hash_val(str);
	return crex_optimizer_add_literal(op_array, &zv);
}

static inline void drop_leading_backslash(zval *val) {
	if (C_STRVAL_P(val)[0] == '\\') {
		crex_string *str = crex_string_init(C_STRVAL_P(val) + 1, C_STRLEN_P(val) - 1, 0);
		zval_ptr_dtor_nogc(val);
		ZVAL_STR(val, str);
	}
}

static inline uint32_t alloc_cache_slots(crex_op_array *op_array, uint32_t num) {
	uint32_t ret = op_array->cache_size;
	op_array->cache_size += num * sizeof(void *);
	return ret;
}

#define REQUIRES_STRING(val) do { \
	if (C_TYPE_P(val) != IS_STRING) { \
		return 0; \
	} \
} while (0)

#define TO_STRING_NOWARN(val) do { \
	if (C_TYPE_P(val) >= IS_ARRAY) { \
		return 0; \
	} \
	convert_to_string(val); \
} while (0)

bool crex_optimizer_update_op1_const(crex_op_array *op_array,
                                    crex_op       *opline,
                                    zval          *val)
{
	switch (opline->opcode) {
		case CREX_OP_DATA:
			switch ((opline-1)->opcode) {
				case CREX_ASSIGN_OBJ_REF:
				case CREX_ASSIGN_STATIC_PROP_REF:
					return 0;
			}
			opline->op1.constant = crex_optimizer_add_literal(op_array, val);
			break;
		case CREX_FREE:
		case CREX_CHECK_VAR:
			MAKE_NOP(opline);
			zval_ptr_dtor_nogc(val);
			return 1;
		case CREX_SEND_VAR_EX:
		case CREX_SEND_FUNC_ARG:
		case CREX_FETCH_DIM_W:
		case CREX_FETCH_DIM_RW:
		case CREX_FETCH_DIM_FUNC_ARG:
		case CREX_FETCH_DIM_UNSET:
		case CREX_FETCH_LIST_W:
		case CREX_ASSIGN_DIM:
		case CREX_RETURN_BY_REF:
		case CREX_INSTANCEOF:
		case CREX_MAKE_REF:
		case CREX_SEPARATE:
		case CREX_SEND_VAR_NO_REF:
		case CREX_SEND_VAR_NO_REF_EX:
			return 0;
		case CREX_CATCH:
			REQUIRES_STRING(val);
			drop_leading_backslash(val);
			opline->op1.constant = crex_optimizer_add_literal(op_array, val);
			opline->extended_value = alloc_cache_slots(op_array, 1) | (opline->extended_value & CREX_LAST_CATCH);
			crex_optimizer_add_literal_string(op_array, crex_string_tolower(C_STR_P(val)));
			break;
		case CREX_DEFINED:
			REQUIRES_STRING(val);
			drop_leading_backslash(val);
			opline->op1.constant = crex_optimizer_add_literal(op_array, val);
			opline->extended_value = alloc_cache_slots(op_array, 1);
			crex_optimizer_add_literal_string(op_array, crex_string_tolower(C_STR_P(val)));
			break;
		case CREX_NEW:
			REQUIRES_STRING(val);
			drop_leading_backslash(val);
			opline->op1.constant = crex_optimizer_add_literal(op_array, val);
			opline->op2.num = alloc_cache_slots(op_array, 1);
			crex_optimizer_add_literal_string(op_array, crex_string_tolower(C_STR_P(val)));
			break;
		case CREX_INIT_STATIC_METHOD_CALL:
			REQUIRES_STRING(val);
			drop_leading_backslash(val);
			opline->op1.constant = crex_optimizer_add_literal(op_array, val);
			if (opline->op2_type != IS_CONST) {
				opline->result.num = alloc_cache_slots(op_array, 1);
			}
			crex_optimizer_add_literal_string(op_array, crex_string_tolower(C_STR_P(val)));
			break;
		case CREX_FETCH_CLASS_CONSTANT:
			REQUIRES_STRING(val);
			drop_leading_backslash(val);
			opline->op1.constant = crex_optimizer_add_literal(op_array, val);
			if (opline->op2_type != IS_CONST) {
				opline->extended_value = alloc_cache_slots(op_array, 1);
			}
			crex_optimizer_add_literal_string(op_array, crex_string_tolower(C_STR_P(val)));
			break;
		case CREX_ASSIGN_OP:
		case CREX_ASSIGN_DIM_OP:
		case CREX_ASSIGN_OBJ_OP:
			break;
		case CREX_ASSIGN_STATIC_PROP_OP:
		case CREX_ASSIGN_STATIC_PROP:
		case CREX_ASSIGN_STATIC_PROP_REF:
		case CREX_FETCH_STATIC_PROP_R:
		case CREX_FETCH_STATIC_PROP_W:
		case CREX_FETCH_STATIC_PROP_RW:
		case CREX_FETCH_STATIC_PROP_IS:
		case CREX_FETCH_STATIC_PROP_UNSET:
		case CREX_FETCH_STATIC_PROP_FUNC_ARG:
		case CREX_UNSET_STATIC_PROP:
		case CREX_ISSET_ISEMPTY_STATIC_PROP:
		case CREX_PRE_INC_STATIC_PROP:
		case CREX_PRE_DEC_STATIC_PROP:
		case CREX_POST_INC_STATIC_PROP:
		case CREX_POST_DEC_STATIC_PROP:
			TO_STRING_NOWARN(val);
			opline->op1.constant = crex_optimizer_add_literal(op_array, val);
			if (opline->op2_type == IS_CONST && (opline->extended_value & ~CREX_FETCH_OBJ_FLAGS) + sizeof(void*) == op_array->cache_size) {
				op_array->cache_size += sizeof(void *);
			} else {
				opline->extended_value = alloc_cache_slots(op_array, 3) | (opline->extended_value & CREX_FETCH_OBJ_FLAGS);
			}
			break;
		case CREX_SEND_VAR:
			opline->opcode = CREX_SEND_VAL;
			opline->op1.constant = crex_optimizer_add_literal(op_array, val);
			break;
		case CREX_CASE:
			opline->opcode = CREX_IS_EQUAL;
			opline->op1.constant = crex_optimizer_add_literal(op_array, val);
			break;
		case CREX_CASE_STRICT:
			opline->opcode = CREX_IS_IDENTICAL;
			opline->op1.constant = crex_optimizer_add_literal(op_array, val);
			break;
		case CREX_VERIFY_RETURN_TYPE:
			/* This would require a non-local change.
			 * crex_optimizer_replace_by_const() supports this. */
			return 0;
		case CREX_COPY_TMP:
		case CREX_FETCH_CLASS_NAME:
			return 0;
		case CREX_ECHO:
		{
			zval zv;
			if (C_TYPE_P(val) != IS_STRING && crex_optimizer_eval_cast(&zv, IS_STRING, val) == SUCCESS) {
				zval_ptr_dtor_nogc(val);
				val = &zv;
			}
			opline->op1.constant = crex_optimizer_add_literal(op_array, val);
			if (C_TYPE_P(val) == IS_STRING && C_STRLEN_P(val) == 0) {
				MAKE_NOP(opline);
				return 1;
			}
			/* TODO: In a subsequent pass, *after* this step and compacting nops, combine consecutive CREX_ECHOs using the block information from ssa->cfg */
			/* (e.g. for ext/opcache/tests/opt/sccp_010.crxt) */
			break;
		}
		case CREX_CONCAT:
		case CREX_FAST_CONCAT:
		case CREX_FETCH_R:
		case CREX_FETCH_W:
		case CREX_FETCH_RW:
		case CREX_FETCH_IS:
		case CREX_FETCH_UNSET:
		case CREX_FETCH_FUNC_ARG:
		case CREX_ISSET_ISEMPTY_VAR:
		case CREX_UNSET_VAR:
			TO_STRING_NOWARN(val);
			if (opline->opcode == CREX_CONCAT && opline->op2_type == IS_CONST) {
				opline->opcode = CREX_FAST_CONCAT;
			}
			CREX_FALLTHROUGH;
		default:
			opline->op1.constant = crex_optimizer_add_literal(op_array, val);
			break;
	}

	opline->op1_type = IS_CONST;
	if (C_TYPE(CREX_OP1_LITERAL(opline)) == IS_STRING) {
		crex_string_hash_val(C_STR(CREX_OP1_LITERAL(opline)));
	}
	return 1;
}

bool crex_optimizer_update_op2_const(crex_op_array *op_array,
                                    crex_op       *opline,
                                    zval          *val)
{
	zval tmp;

	switch (opline->opcode) {
		case CREX_ASSIGN_REF:
		case CREX_FAST_CALL:
			return 0;
		case CREX_FETCH_CLASS:
		case CREX_INSTANCEOF:
			REQUIRES_STRING(val);
			drop_leading_backslash(val);
			opline->op2.constant = crex_optimizer_add_literal(op_array, val);
			crex_optimizer_add_literal_string(op_array, crex_string_tolower(C_STR_P(val)));
			opline->extended_value = alloc_cache_slots(op_array, 1);
			break;
		case CREX_INIT_FCALL_BY_NAME:
			REQUIRES_STRING(val);
			drop_leading_backslash(val);
			opline->op2.constant = crex_optimizer_add_literal(op_array, val);
			crex_optimizer_add_literal_string(op_array, crex_string_tolower(C_STR_P(val)));
			opline->result.num = alloc_cache_slots(op_array, 1);
			break;
		case CREX_ASSIGN_STATIC_PROP:
		case CREX_ASSIGN_STATIC_PROP_REF:
		case CREX_FETCH_STATIC_PROP_R:
		case CREX_FETCH_STATIC_PROP_W:
		case CREX_FETCH_STATIC_PROP_RW:
		case CREX_FETCH_STATIC_PROP_IS:
		case CREX_FETCH_STATIC_PROP_UNSET:
		case CREX_FETCH_STATIC_PROP_FUNC_ARG:
		case CREX_UNSET_STATIC_PROP:
		case CREX_ISSET_ISEMPTY_STATIC_PROP:
		case CREX_PRE_INC_STATIC_PROP:
		case CREX_PRE_DEC_STATIC_PROP:
		case CREX_POST_INC_STATIC_PROP:
		case CREX_POST_DEC_STATIC_PROP:
		case CREX_ASSIGN_STATIC_PROP_OP:
			REQUIRES_STRING(val);
			drop_leading_backslash(val);
			opline->op2.constant = crex_optimizer_add_literal(op_array, val);
			crex_optimizer_add_literal_string(op_array, crex_string_tolower(C_STR_P(val)));
			if (opline->op1_type != IS_CONST) {
				opline->extended_value = alloc_cache_slots(op_array, 1) | (opline->extended_value & (CREX_RETURNS_FUNCTION|CREX_ISEMPTY|CREX_FETCH_OBJ_FLAGS));
			}
			break;
		case CREX_INIT_FCALL:
			REQUIRES_STRING(val);
			if (C_REFCOUNT_P(val) == 1) {
				crex_str_tolower(C_STRVAL_P(val), C_STRLEN_P(val));
			} else {
				ZVAL_STR(&tmp, crex_string_tolower(C_STR_P(val)));
				zval_ptr_dtor_nogc(val);
				val = &tmp;
			}
			opline->op2.constant = crex_optimizer_add_literal(op_array, val);
			opline->result.num = alloc_cache_slots(op_array, 1);
			break;
		case CREX_INIT_DYNAMIC_CALL:
			if (C_TYPE_P(val) == IS_STRING) {
				if (crex_memrchr(C_STRVAL_P(val), ':', C_STRLEN_P(val))) {
					return 0;
				}

				if (crex_optimizer_classify_function(C_STR_P(val), opline->extended_value)) {
					/* Dynamic call to various special functions must stay dynamic,
					 * otherwise would drop a warning */
					return 0;
				}

				opline->opcode = CREX_INIT_FCALL_BY_NAME;
				drop_leading_backslash(val);
				opline->op2.constant = crex_optimizer_add_literal(op_array, val);
				crex_optimizer_add_literal_string(op_array, crex_string_tolower(C_STR_P(val)));
				opline->result.num = alloc_cache_slots(op_array, 1);
			} else {
				opline->op2.constant = crex_optimizer_add_literal(op_array, val);
			}
			break;
		case CREX_INIT_METHOD_CALL:
			REQUIRES_STRING(val);
			opline->op2.constant = crex_optimizer_add_literal(op_array, val);
			crex_optimizer_add_literal_string(op_array, crex_string_tolower(C_STR_P(val)));
			opline->result.num = alloc_cache_slots(op_array, 2);
			break;
		case CREX_INIT_STATIC_METHOD_CALL:
			REQUIRES_STRING(val);
			opline->op2.constant = crex_optimizer_add_literal(op_array, val);
			crex_optimizer_add_literal_string(op_array, crex_string_tolower(C_STR_P(val)));
			if (opline->op1_type != IS_CONST) {
				opline->result.num = alloc_cache_slots(op_array, 2);
			}
			break;
		case CREX_ASSIGN_OBJ:
		case CREX_ASSIGN_OBJ_REF:
		case CREX_FETCH_OBJ_R:
		case CREX_FETCH_OBJ_W:
		case CREX_FETCH_OBJ_RW:
		case CREX_FETCH_OBJ_IS:
		case CREX_FETCH_OBJ_UNSET:
		case CREX_FETCH_OBJ_FUNC_ARG:
		case CREX_UNSET_OBJ:
		case CREX_PRE_INC_OBJ:
		case CREX_PRE_DEC_OBJ:
		case CREX_POST_INC_OBJ:
		case CREX_POST_DEC_OBJ:
			TO_STRING_NOWARN(val);
			opline->op2.constant = crex_optimizer_add_literal(op_array, val);
			opline->extended_value = alloc_cache_slots(op_array, 3);
			break;
		case CREX_ASSIGN_OBJ_OP:
			TO_STRING_NOWARN(val);
			opline->op2.constant = crex_optimizer_add_literal(op_array, val);
			CREX_ASSERT((opline + 1)->opcode == CREX_OP_DATA);
			(opline + 1)->extended_value = alloc_cache_slots(op_array, 3);
			break;
		case CREX_ISSET_ISEMPTY_PROP_OBJ:
			TO_STRING_NOWARN(val);
			opline->op2.constant = crex_optimizer_add_literal(op_array, val);
			opline->extended_value = alloc_cache_slots(op_array, 3) | (opline->extended_value & CREX_ISEMPTY);
			break;
		case CREX_ASSIGN_DIM_OP:
		case CREX_ISSET_ISEMPTY_DIM_OBJ:
		case CREX_ASSIGN_DIM:
		case CREX_UNSET_DIM:
		case CREX_FETCH_DIM_R:
		case CREX_FETCH_DIM_W:
		case CREX_FETCH_DIM_RW:
		case CREX_FETCH_DIM_IS:
		case CREX_FETCH_DIM_FUNC_ARG:
		case CREX_FETCH_DIM_UNSET:
		case CREX_FETCH_LIST_R:
		case CREX_FETCH_LIST_W:
			if (C_TYPE_P(val) == IS_STRING) {
				crex_ulong index;

				if (CREX_HANDLE_NUMERIC(C_STR_P(val), index)) {
					ZVAL_LONG(&tmp, index);
					opline->op2.constant = crex_optimizer_add_literal(op_array, &tmp);
					crex_string_hash_val(C_STR_P(val));
					crex_optimizer_add_literal(op_array, val);
					C_EXTRA(op_array->literals[opline->op2.constant]) = CREX_EXTRA_VALUE;
					break;
				}
			}
			opline->op2.constant = crex_optimizer_add_literal(op_array, val);
			break;
		case CREX_ADD_ARRAY_ELEMENT:
		case CREX_INIT_ARRAY:
			if (C_TYPE_P(val) == IS_STRING) {
				crex_ulong index;
				if (CREX_HANDLE_NUMERIC(C_STR_P(val), index)) {
					zval_ptr_dtor_nogc(val);
					ZVAL_LONG(val, index);
				}
			}
			opline->op2.constant = crex_optimizer_add_literal(op_array, val);
			break;
		case CREX_ROPE_INIT:
		case CREX_ROPE_ADD:
		case CREX_ROPE_END:
		case CREX_CONCAT:
		case CREX_FAST_CONCAT:
			TO_STRING_NOWARN(val);
			if (opline->opcode == CREX_CONCAT && opline->op1_type == IS_CONST) {
				opline->opcode = CREX_FAST_CONCAT;
			}
			CREX_FALLTHROUGH;
		default:
			opline->op2.constant = crex_optimizer_add_literal(op_array, val);
			break;
	}

	opline->op2_type = IS_CONST;
	if (C_TYPE(CREX_OP2_LITERAL(opline)) == IS_STRING) {
		crex_string_hash_val(C_STR(CREX_OP2_LITERAL(opline)));
	}
	return 1;
}

bool crex_optimizer_replace_by_const(crex_op_array *op_array,
                                    crex_op       *opline,
                                    uint8_t        type,
                                    uint32_t       var,
                                    zval          *val)
{
	crex_op *end = op_array->opcodes + op_array->last;

	while (opline < end) {
		if (opline->op1_type == type &&
			opline->op1.var == var) {
			switch (opline->opcode) {
				/* In most cases IS_TMP_VAR operand may be used only once.
				 * The operands are usually destroyed by the opcode handler.
				 * However, there are some exception which keep the operand alive. In that case
				 * we want to try to replace all uses of the temporary.
				 */
				case CREX_FETCH_LIST_R:
				case CREX_CASE:
				case CREX_CASE_STRICT:
				case CREX_SWITCH_LONG:
				case CREX_SWITCH_STRING:
				case CREX_MATCH:
				case CREX_JMP_NULL: {
					crex_op *end = op_array->opcodes + op_array->last;
					while (opline < end) {
						if (opline->op1_type == type && opline->op1.var == var) {
							/* If this opcode doesn't keep the operand alive, we're done. Check
							 * this early, because op replacement may modify the opline. */
							bool is_last = opline->opcode != CREX_FETCH_LIST_R
								&& opline->opcode != CREX_CASE
								&& opline->opcode != CREX_CASE_STRICT
								&& opline->opcode != CREX_SWITCH_LONG
								&& opline->opcode != CREX_SWITCH_STRING
								&& opline->opcode != CREX_MATCH
								&& opline->opcode != CREX_JMP_NULL
								&& (opline->opcode != CREX_FREE
									|| opline->extended_value != CREX_FREE_ON_RETURN);

							C_TRY_ADDREF_P(val);
							if (!crex_optimizer_update_op1_const(op_array, opline, val)) {
								zval_ptr_dtor(val);
								return 0;
							}
							if (is_last) {
								break;
							}
						}
						opline++;
					}
					zval_ptr_dtor_nogc(val);
					return 1;
				}
				case CREX_VERIFY_RETURN_TYPE: {
					crex_arg_info *ret_info = op_array->arg_info - 1;
					if (!CREX_TYPE_CONTAINS_CODE(ret_info->type, C_TYPE_P(val))
						|| (op_array->fn_flags & CREX_ACC_RETURN_REFERENCE)) {
						return 0;
					}
					MAKE_NOP(opline);

					/* crex_handle_loops_and_finally may inserts other oplines */
					do {
						++opline;
					} while (opline->opcode != CREX_RETURN && opline->opcode != CREX_RETURN_BY_REF);
					CREX_ASSERT(opline->op1.var == var);

					break;
				}
				default:
					break;
			}
			return crex_optimizer_update_op1_const(op_array, opline, val);
		}

		if (opline->op2_type == type &&
			opline->op2.var == var) {
			return crex_optimizer_update_op2_const(op_array, opline, val);
		}
		opline++;
	}

	return 1;
}

/* Update jump offsets after a jump was migrated to another opline */
void crex_optimizer_migrate_jump(crex_op_array *op_array, crex_op *new_opline, crex_op *opline) {
	switch (new_opline->opcode) {
		case CREX_JMP:
		case CREX_FAST_CALL:
			CREX_SET_OP_JMP_ADDR(new_opline, new_opline->op1, CREX_OP1_JMP_ADDR(opline));
			break;
		case CREX_JMPZ:
		case CREX_JMPNZ:
		case CREX_JMPC_EX:
		case CREX_JMPNC_EX:
		case CREX_FE_RESET_R:
		case CREX_FE_RESET_RW:
		case CREX_JMP_SET:
		case CREX_COALESCE:
		case CREX_ASSERT_CHECK:
		case CREX_JMP_NULL:
		case CREX_BIND_INIT_STATIC_OR_JMP:
			CREX_SET_OP_JMP_ADDR(new_opline, new_opline->op2, CREX_OP2_JMP_ADDR(opline));
			break;
		case CREX_FE_FETCH_R:
		case CREX_FE_FETCH_RW:
			new_opline->extended_value = CREX_OPLINE_NUM_TO_OFFSET(op_array, new_opline, CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value));
			break;
		case CREX_CATCH:
			if (!(opline->extended_value & CREX_LAST_CATCH)) {
				CREX_SET_OP_JMP_ADDR(new_opline, new_opline->op2, CREX_OP2_JMP_ADDR(opline));
			}
			break;
		case CREX_SWITCH_LONG:
		case CREX_SWITCH_STRING:
		case CREX_MATCH:
		{
			HashTable *jumptable = C_ARRVAL(CREX_OP2_LITERAL(opline));
			zval *zv;
			CREX_HASH_FOREACH_VAL(jumptable, zv) {
				C_LVAL_P(zv) = CREX_OPLINE_NUM_TO_OFFSET(op_array, new_opline, CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, C_LVAL_P(zv)));
			} CREX_HASH_FOREACH_END();
			new_opline->extended_value = CREX_OPLINE_NUM_TO_OFFSET(op_array, new_opline, CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value));
			break;
		}
	}
}

/* Shift jump offsets based on shiftlist */
void crex_optimizer_shift_jump(crex_op_array *op_array, crex_op *opline, uint32_t *shiftlist) {
	switch (opline->opcode) {
		case CREX_JMP:
		case CREX_FAST_CALL:
			CREX_SET_OP_JMP_ADDR(opline, opline->op1, CREX_OP1_JMP_ADDR(opline) - shiftlist[CREX_OP1_JMP_ADDR(opline) - op_array->opcodes]);
			break;
		case CREX_JMPZ:
		case CREX_JMPNZ:
		case CREX_JMPC_EX:
		case CREX_JMPNC_EX:
		case CREX_FE_RESET_R:
		case CREX_FE_RESET_RW:
		case CREX_JMP_SET:
		case CREX_COALESCE:
		case CREX_ASSERT_CHECK:
		case CREX_JMP_NULL:
		case CREX_BIND_INIT_STATIC_OR_JMP:
			CREX_SET_OP_JMP_ADDR(opline, opline->op2, CREX_OP2_JMP_ADDR(opline) - shiftlist[CREX_OP2_JMP_ADDR(opline) - op_array->opcodes]);
			break;
		case CREX_CATCH:
			if (!(opline->extended_value & CREX_LAST_CATCH)) {
				CREX_SET_OP_JMP_ADDR(opline, opline->op2, CREX_OP2_JMP_ADDR(opline) - shiftlist[CREX_OP2_JMP_ADDR(opline) - op_array->opcodes]);
			}
			break;
		case CREX_FE_FETCH_R:
		case CREX_FE_FETCH_RW:
			opline->extended_value = CREX_OPLINE_NUM_TO_OFFSET(op_array, opline, CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value) - shiftlist[CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value)]);
			break;
		case CREX_SWITCH_LONG:
		case CREX_SWITCH_STRING:
		case CREX_MATCH:
		{
			HashTable *jumptable = C_ARRVAL(CREX_OP2_LITERAL(opline));
			zval *zv;
			CREX_HASH_FOREACH_VAL(jumptable, zv) {
				C_LVAL_P(zv) = CREX_OPLINE_NUM_TO_OFFSET(op_array, opline, CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, C_LVAL_P(zv)) - shiftlist[CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, C_LVAL_P(zv))]);
			} CREX_HASH_FOREACH_END();
			opline->extended_value = CREX_OPLINE_NUM_TO_OFFSET(op_array, opline, CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value) - shiftlist[CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value)]);
			break;
		}
	}
}

crex_class_entry *crex_optimizer_get_class_entry(
		const crex_script *script, const crex_op_array *op_array, crex_string *lcname) {
	crex_class_entry *ce = script ? crex_hash_find_ptr(&script->class_table, lcname) : NULL;
	if (ce) {
		return ce;
	}

	ce = crex_hash_find_ptr(CG(class_table), lcname);
	if (ce && ce->type == CREX_INTERNAL_CLASS) {
		return ce;
	}

	if (op_array && op_array->scope && crex_string_equals_ci(op_array->scope->name, lcname)) {
		return op_array->scope;
	}

	return NULL;
}

crex_class_entry *crex_optimizer_get_class_entry_from_op1(
		const crex_script *script, const crex_op_array *op_array, const crex_op *opline) {
	if (opline->op1_type == IS_CONST) {
		zval *op1 = CRT_CONSTANT(opline->op1);
		if (C_TYPE_P(op1) == IS_STRING) {
			return crex_optimizer_get_class_entry(script, op_array, C_STR_P(op1 + 1));
		}
	} else if (opline->op1_type == IS_UNUSED && op_array->scope
			&& !(op_array->scope->ce_flags & CREX_ACC_TRAIT)
			&& (opline->op1.num & CREX_FETCH_CLASS_MASK) == CREX_FETCH_CLASS_SELF) {
		return op_array->scope;
	}
	return NULL;
}

crex_function *crex_optimizer_get_called_func(
		crex_script *script, crex_op_array *op_array, crex_op *opline, bool *is_prototype)
{
	*is_prototype = 0;
	switch (opline->opcode) {
		case CREX_INIT_FCALL:
		{
			crex_string *function_name = C_STR_P(CRT_CONSTANT(opline->op2));
			crex_function *func;
			if (script && (func = crex_hash_find_ptr(&script->function_table, function_name)) != NULL) {
				return func;
			} else if ((func = crex_hash_find_ptr(EG(function_table), function_name)) != NULL) {
				if (func->type == CREX_INTERNAL_FUNCTION) {
					return func;
				} else if (func->type == CREX_USER_FUNCTION &&
				           func->op_array.filename &&
				           func->op_array.filename == op_array->filename) {
					return func;
				}
			}
			break;
		}
		case CREX_INIT_FCALL_BY_NAME:
		case CREX_INIT_NS_FCALL_BY_NAME:
			if (opline->op2_type == IS_CONST && C_TYPE_P(CRT_CONSTANT(opline->op2)) == IS_STRING) {
				zval *function_name = CRT_CONSTANT(opline->op2) + 1;
				crex_function *func;
				if (script && (func = crex_hash_find_ptr(&script->function_table, C_STR_P(function_name)))) {
					return func;
				} else if ((func = crex_hash_find_ptr(EG(function_table), C_STR_P(function_name))) != NULL) {
					if (func->type == CREX_INTERNAL_FUNCTION) {
						return func;
					} else if (func->type == CREX_USER_FUNCTION &&
					           func->op_array.filename &&
					           func->op_array.filename == op_array->filename) {
						return func;
					}
				}
			}
			break;
		case CREX_INIT_STATIC_METHOD_CALL:
			if (opline->op2_type == IS_CONST && C_TYPE_P(CRT_CONSTANT(opline->op2)) == IS_STRING) {
				crex_class_entry *ce = crex_optimizer_get_class_entry_from_op1(
					script, op_array, opline);
				if (ce) {
					crex_string *func_name = C_STR_P(CRT_CONSTANT(opline->op2) + 1);
					crex_function *fbc = crex_hash_find_ptr(&ce->function_table, func_name);
					if (fbc) {
						bool is_public = (fbc->common.fn_flags & CREX_ACC_PUBLIC) != 0;
						bool same_scope = fbc->common.scope == op_array->scope;
						if (is_public || same_scope) {
							return fbc;
						}
					}
				}
			}
			break;
		case CREX_INIT_METHOD_CALL:
			if (opline->op1_type == IS_UNUSED
					&& opline->op2_type == IS_CONST && C_TYPE_P(CRT_CONSTANT(opline->op2)) == IS_STRING
					&& op_array->scope
					&& !(op_array->fn_flags & CREX_ACC_TRAIT_CLONE)
					&& !(op_array->scope->ce_flags & CREX_ACC_TRAIT)) {
				crex_string *method_name = C_STR_P(CRT_CONSTANT(opline->op2) + 1);
				crex_function *fbc = crex_hash_find_ptr(
					&op_array->scope->function_table, method_name);
				if (fbc) {
					bool is_private = (fbc->common.fn_flags & CREX_ACC_PRIVATE) != 0;
					if (is_private) {
						/* Only use private method if in the same scope. We can't even use it
						 * as a prototype, as it may be overridden with changed signature. */
						bool same_scope = fbc->common.scope == op_array->scope;
						return same_scope ? fbc : NULL;
					}
					/* Prototype methods are potentially overridden. fbc still contains useful type information.
					 * Some optimizations may not be applied, like inlining or inferring the send-mode of superfluous args.
					 * A method cannot be overridden if the class or method is final. */
					if ((fbc->common.fn_flags & CREX_ACC_FINAL) == 0 &&
						(fbc->common.scope->ce_flags & CREX_ACC_FINAL) == 0) {
						*is_prototype = true;
					}
					return fbc;
				}
			}
			break;
		case CREX_NEW:
		{
			crex_class_entry *ce = crex_optimizer_get_class_entry_from_op1(
				script, op_array, opline);
			if (ce && ce->type == CREX_USER_CLASS) {
				return ce->constructor;
			}
			break;
		}
	}
	return NULL;
}

uint32_t crex_optimizer_classify_function(crex_string *name, uint32_t num_args) {
	if (crex_string_equals_literal(name, "extract")) {
		return CREX_FUNC_INDIRECT_VAR_ACCESS;
	} else if (crex_string_equals_literal(name, "compact")) {
		return CREX_FUNC_INDIRECT_VAR_ACCESS;
	} else if (crex_string_equals_literal(name, "get_defined_vars")) {
		return CREX_FUNC_INDIRECT_VAR_ACCESS;
	} else if (crex_string_equals_literal(name, "db2_execute")) {
		return CREX_FUNC_INDIRECT_VAR_ACCESS;
	} else if (crex_string_equals_literal(name, "func_num_args")) {
		return CREX_FUNC_VARARG;
	} else if (crex_string_equals_literal(name, "func_get_arg")) {
		return CREX_FUNC_VARARG;
	} else if (crex_string_equals_literal(name, "func_get_args")) {
		return CREX_FUNC_VARARG;
	} else {
		return 0;
	}
}

crex_op *crex_optimizer_get_loop_var_def(const crex_op_array *op_array, crex_op *free_opline) {
	uint32_t var = free_opline->op1.var;
	CREX_ASSERT(crex_optimizer_is_loop_var_free(free_opline));

	while (--free_opline >= op_array->opcodes) {
		if ((free_opline->result_type & (IS_TMP_VAR|IS_VAR)) && free_opline->result.var == var) {
			return free_opline;
		}
	}
	return NULL;
}

static void crex_optimize(crex_op_array      *op_array,
                          crex_optimizer_ctx *ctx)
{
	if (op_array->type == CREX_EVAL_CODE) {
		return;
	}

	if (ctx->debug_level & CREX_DUMP_BEFORE_OPTIMIZER) {
		crex_dump_op_array(op_array, CREX_DUMP_LIVE_RANGES, "before optimizer", NULL);
	}

	/* pass 1 (Simple local optimizations)
	 * - persistent constant substitution (true, false, null, etc)
	 * - constant casting (ADD expects numbers, CONCAT strings, etc)
	 * - constant expression evaluation
	 * - optimize constant conditional JMPs
	 * - pre-evaluate constant function calls
	 * - eliminate FETCH $GLOBALS followed by FETCH_DIM/UNSET_DIM/ISSET_ISEMPTY_DIM
	 */
	if (CREX_OPTIMIZER_PASS_1 & ctx->optimization_level) {
		crex_optimizer_pass1(op_array, ctx);
		if (ctx->debug_level & CREX_DUMP_AFTER_PASS_1) {
			crex_dump_op_array(op_array, 0, "after pass 1", NULL);
		}
	}

	/* pass 3: (Jump optimization)
	 * - optimize series of JMPs
	 */
	if (CREX_OPTIMIZER_PASS_3 & ctx->optimization_level) {
		crex_optimizer_pass3(op_array, ctx);
		if (ctx->debug_level & CREX_DUMP_AFTER_PASS_3) {
			crex_dump_op_array(op_array, 0, "after pass 3", NULL);
		}
	}

	/* pass 4:
	 * - INIT_FCALL_BY_NAME -> DO_FCALL
	 */
	if (CREX_OPTIMIZER_PASS_4 & ctx->optimization_level) {
		crex_optimize_func_calls(op_array, ctx);
		if (ctx->debug_level & CREX_DUMP_AFTER_PASS_4) {
			crex_dump_op_array(op_array, 0, "after pass 4", NULL);
		}
	}

	/* pass 5:
	 * - CFG optimization
	 */
	if (CREX_OPTIMIZER_PASS_5 & ctx->optimization_level) {
		crex_optimize_cfg(op_array, ctx);
		if (ctx->debug_level & CREX_DUMP_AFTER_PASS_5) {
			crex_dump_op_array(op_array, 0, "after pass 5", NULL);
		}
	}

	/* pass 6:
	 * - DFA optimization
	 */
	if ((CREX_OPTIMIZER_PASS_6 & ctx->optimization_level) &&
	    !(CREX_OPTIMIZER_PASS_7 & ctx->optimization_level)) {
		crex_optimize_dfa(op_array, ctx);
		if (ctx->debug_level & CREX_DUMP_AFTER_PASS_6) {
			crex_dump_op_array(op_array, 0, "after pass 6", NULL);
		}
	}

	/* pass 9:
	 * - Optimize temp variables usage
	 */
	if ((CREX_OPTIMIZER_PASS_9 & ctx->optimization_level) &&
	    !(CREX_OPTIMIZER_PASS_7 & ctx->optimization_level)) {
		crex_optimize_temporary_variables(op_array, ctx);
		if (ctx->debug_level & CREX_DUMP_AFTER_PASS_9) {
			crex_dump_op_array(op_array, 0, "after pass 9", NULL);
		}
	}

	/* pass 10:
	 * - remove NOPs
	 */
	if (((CREX_OPTIMIZER_PASS_10|CREX_OPTIMIZER_PASS_5) & ctx->optimization_level) == CREX_OPTIMIZER_PASS_10) {
		crex_optimizer_nop_removal(op_array, ctx);
		if (ctx->debug_level & CREX_DUMP_AFTER_PASS_10) {
			crex_dump_op_array(op_array, 0, "after pass 10", NULL);
		}
	}

	/* pass 11:
	 * - Compact literals table
	 */
	if ((CREX_OPTIMIZER_PASS_11 & ctx->optimization_level) &&
	    (!(CREX_OPTIMIZER_PASS_6 & ctx->optimization_level) ||
	     !(CREX_OPTIMIZER_PASS_7 & ctx->optimization_level))) {
		crex_optimizer_compact_literals(op_array, ctx);
		if (ctx->debug_level & CREX_DUMP_AFTER_PASS_11) {
			crex_dump_op_array(op_array, 0, "after pass 11", NULL);
		}
	}

	if ((CREX_OPTIMIZER_PASS_13 & ctx->optimization_level) &&
	    (!(CREX_OPTIMIZER_PASS_6 & ctx->optimization_level) ||
	     !(CREX_OPTIMIZER_PASS_7 & ctx->optimization_level))) {
		crex_optimizer_compact_vars(op_array);
		if (ctx->debug_level & CREX_DUMP_AFTER_PASS_13) {
			crex_dump_op_array(op_array, 0, "after pass 13", NULL);
		}
	}

	if (CREX_OPTIMIZER_PASS_7 & ctx->optimization_level) {
		return;
	}

	if (ctx->debug_level & CREX_DUMP_AFTER_OPTIMIZER) {
		crex_dump_op_array(op_array, 0, "after optimizer", NULL);
	}
}

static void crex_revert_pass_two(crex_op_array *op_array)
{
	crex_op *opline, *end;

	CREX_ASSERT((op_array->fn_flags & CREX_ACC_DONE_PASS_TWO) != 0);

	opline = op_array->opcodes;
	end = opline + op_array->last;
	while (opline < end) {
		if (opline->op1_type == IS_CONST) {
			CREX_PASS_TWO_UNDO_CONSTANT(op_array, opline, opline->op1);
		}
		if (opline->op2_type == IS_CONST) {
			CREX_PASS_TWO_UNDO_CONSTANT(op_array, opline, opline->op2);
		}
		/* reset smart branch flags IS_SMART_BRANCH_JMP[N]Z */
		opline->result_type &= (IS_TMP_VAR|IS_VAR|IS_CV|IS_CONST);
		opline++;
	}
#if !CREX_USE_ABS_CONST_ADDR
	if (op_array->literals) {
		zval *literals = emalloc(sizeof(zval) * op_array->last_literal);
		memcpy(literals, op_array->literals, sizeof(zval) * op_array->last_literal);
		op_array->literals = literals;
	}
#endif

	op_array->T -= CREX_OBSERVER_ENABLED;

	op_array->fn_flags &= ~CREX_ACC_DONE_PASS_TWO;
}

static void crex_redo_pass_two(crex_op_array *op_array)
{
	crex_op *opline, *end;
#if CREX_USE_ABS_JMP_ADDR && !CREX_USE_ABS_CONST_ADDR
	crex_op *old_opcodes = op_array->opcodes;
#endif

	CREX_ASSERT((op_array->fn_flags & CREX_ACC_DONE_PASS_TWO) == 0);

#if !CREX_USE_ABS_CONST_ADDR
	if (op_array->last_literal) {
		op_array->opcodes = (crex_op *) erealloc(op_array->opcodes,
			CREX_MM_ALIGNED_SIZE_EX(sizeof(crex_op) * op_array->last, 16) +
			sizeof(zval) * op_array->last_literal);
		memcpy(((char*)op_array->opcodes) + CREX_MM_ALIGNED_SIZE_EX(sizeof(crex_op) * op_array->last, 16),
			op_array->literals, sizeof(zval) * op_array->last_literal);
		efree(op_array->literals);
		op_array->literals = (zval*)(((char*)op_array->opcodes) + CREX_MM_ALIGNED_SIZE_EX(sizeof(crex_op) * op_array->last, 16));
	} else {
		if (op_array->literals) {
			efree(op_array->literals);
		}
		op_array->literals = NULL;
	}
#endif

	op_array->T += CREX_OBSERVER_ENABLED; // reserve last temporary for observers if enabled

	opline = op_array->opcodes;
	end = opline + op_array->last;
	while (opline < end) {
		if (opline->op1_type == IS_CONST) {
			CREX_PASS_TWO_UPDATE_CONSTANT(op_array, opline, opline->op1);
		}
		if (opline->op2_type == IS_CONST) {
			CREX_PASS_TWO_UPDATE_CONSTANT(op_array, opline, opline->op2);
		}
		/* fix jumps to point to new array */
		switch (opline->opcode) {
#if CREX_USE_ABS_JMP_ADDR && !CREX_USE_ABS_CONST_ADDR
			case CREX_JMP:
			case CREX_FAST_CALL:
				opline->op1.jmp_addr = &op_array->opcodes[opline->op1.jmp_addr - old_opcodes];
				break;
			case CREX_JMPZ:
			case CREX_JMPNZ:
			case CREX_JMPC_EX:
			case CREX_JMPNC_EX:
			case CREX_JMP_SET:
			case CREX_COALESCE:
			case CREX_FE_RESET_R:
			case CREX_FE_RESET_RW:
			case CREX_ASSERT_CHECK:
			case CREX_JMP_NULL:
			case CREX_BIND_INIT_STATIC_OR_JMP:
				opline->op2.jmp_addr = &op_array->opcodes[opline->op2.jmp_addr - old_opcodes];
				break;
			case CREX_CATCH:
				if (!(opline->extended_value & CREX_LAST_CATCH)) {
					opline->op2.jmp_addr = &op_array->opcodes[opline->op2.jmp_addr - old_opcodes];
				}
				break;
			case CREX_FE_FETCH_R:
			case CREX_FE_FETCH_RW:
			case CREX_SWITCH_LONG:
			case CREX_SWITCH_STRING:
			case CREX_MATCH:
				/* relative extended_value don't have to be changed */
				break;
#endif
			case CREX_IS_IDENTICAL:
			case CREX_IS_NOT_IDENTICAL:
			case CREX_IS_EQUAL:
			case CREX_IS_NOT_EQUAL:
			case CREX_IS_SMALLER:
			case CREX_IS_SMALLER_OR_EQUAL:
			case CREX_CASE:
			case CREX_CASE_STRICT:
			case CREX_ISSET_ISEMPTY_CV:
			case CREX_ISSET_ISEMPTY_VAR:
			case CREX_ISSET_ISEMPTY_DIM_OBJ:
			case CREX_ISSET_ISEMPTY_PROP_OBJ:
			case CREX_ISSET_ISEMPTY_STATIC_PROP:
			case CREX_INSTANCEOF:
			case CREX_TYPE_CHECK:
			case CREX_DEFINED:
			case CREX_IN_ARRAY:
			case CREX_ARRAY_KEY_EXISTS:
				if (opline->result_type & IS_TMP_VAR) {
					/* reinitialize result_type of smart branch instructions */
					if (opline + 1 < end) {
						if ((opline+1)->opcode == CREX_JMPZ
						 && (opline+1)->op1_type == IS_TMP_VAR
						 && (opline+1)->op1.var == opline->result.var) {
							opline->result_type = IS_SMART_BRANCH_JMPZ | IS_TMP_VAR;
						} else if ((opline+1)->opcode == CREX_JMPNZ
						 && (opline+1)->op1_type == IS_TMP_VAR
						 && (opline+1)->op1.var == opline->result.var) {
							opline->result_type = IS_SMART_BRANCH_JMPNZ | IS_TMP_VAR;
						}
					}
				}
				break;
		}
		CREX_VM_SET_OPCODE_HANDLER(opline);
		opline++;
	}

	op_array->fn_flags |= CREX_ACC_DONE_PASS_TWO;
}

static void crex_redo_pass_two_ex(crex_op_array *op_array, crex_ssa *ssa)
{
	crex_op *opline, *end;
#if CREX_USE_ABS_JMP_ADDR && !CREX_USE_ABS_CONST_ADDR
	crex_op *old_opcodes = op_array->opcodes;
#endif

	CREX_ASSERT((op_array->fn_flags & CREX_ACC_DONE_PASS_TWO) == 0);

#if !CREX_USE_ABS_CONST_ADDR
	if (op_array->last_literal) {
		op_array->opcodes = (crex_op *) erealloc(op_array->opcodes,
			CREX_MM_ALIGNED_SIZE_EX(sizeof(crex_op) * op_array->last, 16) +
			sizeof(zval) * op_array->last_literal);
		memcpy(((char*)op_array->opcodes) + CREX_MM_ALIGNED_SIZE_EX(sizeof(crex_op) * op_array->last, 16),
			op_array->literals, sizeof(zval) * op_array->last_literal);
		efree(op_array->literals);
		op_array->literals = (zval*)(((char*)op_array->opcodes) + CREX_MM_ALIGNED_SIZE_EX(sizeof(crex_op) * op_array->last, 16));
	} else {
		if (op_array->literals) {
			efree(op_array->literals);
		}
		op_array->literals = NULL;
	}
#endif

	opline = op_array->opcodes;
	end = opline + op_array->last;
	while (opline < end) {
		crex_ssa_op *ssa_op = &ssa->ops[opline - op_array->opcodes];
		uint32_t op1_info = opline->op1_type == IS_UNUSED ? 0 : (OP1_INFO() & (MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_KEY_ANY));
		uint32_t op2_info = opline->op1_type == IS_UNUSED ? 0 : (OP2_INFO() & (MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_KEY_ANY));
		uint32_t res_info =
			(opline->opcode == CREX_PRE_INC ||
			 opline->opcode == CREX_PRE_DEC ||
			 opline->opcode == CREX_POST_INC ||
			 opline->opcode == CREX_POST_DEC) ?
				((ssa->ops[opline - op_array->opcodes].op1_def >= 0) ? (OP1_DEF_INFO() & (MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_KEY_ANY)) : MAY_BE_ANY) :
				(opline->result_type == IS_UNUSED ? 0 : (RES_INFO() & (MAY_BE_UNDEF|MAY_BE_ANY|MAY_BE_REF|MAY_BE_ARRAY_OF_ANY|MAY_BE_ARRAY_KEY_ANY)));

		if (opline->op1_type == IS_CONST) {
			CREX_PASS_TWO_UPDATE_CONSTANT(op_array, opline, opline->op1);
		}
		if (opline->op2_type == IS_CONST) {
			CREX_PASS_TWO_UPDATE_CONSTANT(op_array, opline, opline->op2);
		}

		/* fix jumps to point to new array */
		switch (opline->opcode) {
#if CREX_USE_ABS_JMP_ADDR && !CREX_USE_ABS_CONST_ADDR
			case CREX_JMP:
			case CREX_FAST_CALL:
				opline->op1.jmp_addr = &op_array->opcodes[opline->op1.jmp_addr - old_opcodes];
				break;
			case CREX_JMPZ:
			case CREX_JMPNZ:
			case CREX_JMPC_EX:
			case CREX_JMPNC_EX:
			case CREX_JMP_SET:
			case CREX_COALESCE:
			case CREX_FE_RESET_R:
			case CREX_FE_RESET_RW:
			case CREX_ASSERT_CHECK:
			case CREX_JMP_NULL:
			case CREX_BIND_INIT_STATIC_OR_JMP:
				opline->op2.jmp_addr = &op_array->opcodes[opline->op2.jmp_addr - old_opcodes];
				break;
			case CREX_CATCH:
				if (!(opline->extended_value & CREX_LAST_CATCH)) {
					opline->op2.jmp_addr = &op_array->opcodes[opline->op2.jmp_addr - old_opcodes];
				}
				break;
			case CREX_FE_FETCH_R:
			case CREX_FE_FETCH_RW:
			case CREX_SWITCH_LONG:
			case CREX_SWITCH_STRING:
			case CREX_MATCH:
				/* relative extended_value don't have to be changed */
				break;
#endif
			case CREX_IS_IDENTICAL:
			case CREX_IS_NOT_IDENTICAL:
			case CREX_IS_EQUAL:
			case CREX_IS_NOT_EQUAL:
			case CREX_IS_SMALLER:
			case CREX_IS_SMALLER_OR_EQUAL:
			case CREX_CASE:
			case CREX_CASE_STRICT:
			case CREX_ISSET_ISEMPTY_CV:
			case CREX_ISSET_ISEMPTY_VAR:
			case CREX_ISSET_ISEMPTY_DIM_OBJ:
			case CREX_ISSET_ISEMPTY_PROP_OBJ:
			case CREX_ISSET_ISEMPTY_STATIC_PROP:
			case CREX_INSTANCEOF:
			case CREX_TYPE_CHECK:
			case CREX_DEFINED:
			case CREX_IN_ARRAY:
			case CREX_ARRAY_KEY_EXISTS:
				if (opline->result_type & IS_TMP_VAR) {
					/* reinitialize result_type of smart branch instructions */
					if (opline + 1 < end) {
						if ((opline+1)->opcode == CREX_JMPZ
						 && (opline+1)->op1_type == IS_TMP_VAR
						 && (opline+1)->op1.var == opline->result.var) {
							opline->result_type = IS_SMART_BRANCH_JMPZ | IS_TMP_VAR;
						} else if ((opline+1)->opcode == CREX_JMPNZ
						 && (opline+1)->op1_type == IS_TMP_VAR
						 && (opline+1)->op1.var == opline->result.var) {
							opline->result_type = IS_SMART_BRANCH_JMPNZ | IS_TMP_VAR;
						}
					}
				}
				break;
		}
		crex_vm_set_opcode_handler_ex(opline, op1_info, op2_info, res_info);
		opline++;
	}

	op_array->fn_flags |= CREX_ACC_DONE_PASS_TWO;
}

static void crex_optimize_op_array(crex_op_array      *op_array,
                                   crex_optimizer_ctx *ctx)
{
	/* Revert pass_two() */
	crex_revert_pass_two(op_array);

	/* Do actual optimizations */
	crex_optimize(op_array, ctx);

	/* Redo pass_two() */
	crex_redo_pass_two(op_array);

	if (op_array->live_range) {
		crex_recalc_live_ranges(op_array, NULL);
	}
}

static void crex_adjust_fcall_stack_size(crex_op_array *op_array, crex_optimizer_ctx *ctx)
{
	crex_function *func;
	crex_op *opline, *end;

	opline = op_array->opcodes;
	end = opline + op_array->last;
	while (opline < end) {
		if (opline->opcode == CREX_INIT_FCALL) {
			func = crex_hash_find_ptr(
				&ctx->script->function_table,
				C_STR_P(RT_CONSTANT(opline, opline->op2)));
			if (func) {
				opline->op1.num = crex_vm_calc_used_stack(opline->extended_value, func);
			}
		}
		opline++;
	}
}

static void crex_adjust_fcall_stack_size_graph(crex_op_array *op_array)
{
	crex_func_info *func_info = CREX_FUNC_INFO(op_array);

	if (func_info) {
		crex_call_info *call_info =func_info->callee_info;

		while (call_info) {
			crex_op *opline = call_info->caller_init_opline;

			if (opline && call_info->callee_func && opline->opcode == CREX_INIT_FCALL) {
				CREX_ASSERT(!call_info->is_prototype);
				opline->op1.num = crex_vm_calc_used_stack(opline->extended_value, call_info->callee_func);
			}
			call_info = call_info->next_callee;
		}
	}
}

static bool needs_live_range(crex_op_array *op_array, crex_op *def_opline) {
	crex_func_info *func_info = CREX_FUNC_INFO(op_array);
	crex_ssa_op *ssa_op = &func_info->ssa.ops[def_opline - op_array->opcodes];
	int ssa_var = ssa_op->result_def;
	if (ssa_var < 0) {
		/* Be conservative. */
		return 1;
	}

	/* If the variable is used by a PHI, this may be the assignment of the final branch of a
	 * ternary/etc structure. While this is where the live range starts, the value from the other
	 * branch may also be used. As such, use the type of the PHI node for the following check. */
	if (func_info->ssa.vars[ssa_var].phi_use_chain) {
		ssa_var = func_info->ssa.vars[ssa_var].phi_use_chain->ssa_var;
	}

	uint32_t type = func_info->ssa.var_info[ssa_var].type;
	return (type & (MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_OBJECT|MAY_BE_RESOURCE|MAY_BE_REF)) != 0;
}

static void crex_foreach_op_array_helper(
		crex_op_array *op_array, crex_op_array_func_t func, void *context) {
	func(op_array, context);
	for (uint32_t i = 0; i < op_array->num_dynamic_func_defs; i++) {
		crex_foreach_op_array_helper(op_array->dynamic_func_defs[i], func, context);
	}
}

void crex_foreach_op_array(crex_script *script, crex_op_array_func_t func, void *context)
{
	zval *zv;
	crex_op_array *op_array;

	crex_foreach_op_array_helper(&script->main_op_array, func, context);

	CREX_HASH_MAP_FOREACH_PTR(&script->function_table, op_array) {
		crex_foreach_op_array_helper(op_array, func, context);
	} CREX_HASH_FOREACH_END();

	CREX_HASH_MAP_FOREACH_VAL(&script->class_table, zv) {
		if (C_TYPE_P(zv) == IS_ALIAS_PTR) {
			continue;
		}
		crex_class_entry *ce = C_CE_P(zv);
		CREX_HASH_MAP_FOREACH_PTR(&ce->function_table, op_array) {
			if (op_array->scope == ce
					&& op_array->type == CREX_USER_FUNCTION
					&& !(op_array->fn_flags & CREX_ACC_ABSTRACT)
					&& !(op_array->fn_flags & CREX_ACC_TRAIT_CLONE)) {
				crex_foreach_op_array_helper(op_array, func, context);
			}
		} CREX_HASH_FOREACH_END();
	} CREX_HASH_FOREACH_END();
}

static void step_optimize_op_array(crex_op_array *op_array, void *context) {
	crex_optimize_op_array(op_array, (crex_optimizer_ctx *) context);
}

static void step_adjust_fcall_stack_size(crex_op_array *op_array, void *context) {
	crex_adjust_fcall_stack_size(op_array, (crex_optimizer_ctx *) context);
}

static void step_dump_after_optimizer(crex_op_array *op_array, void *context) {
	crex_dump_op_array(op_array, CREX_DUMP_LIVE_RANGES, "after optimizer", NULL);
}

static void crex_optimizer_call_registered_passes(crex_script *script, void *ctx) {
	for (int i = 0; i < crex_optimizer_registered_passes.last; i++) {
		if (!crex_optimizer_registered_passes.pass[i]) {
			continue;
		}

		crex_optimizer_registered_passes.pass[i](script, ctx);
	}
}

CREX_API void crex_optimize_script(crex_script *script, crex_long optimization_level, crex_long debug_level)
{
	crex_op_array *op_array;
	crex_string *name;
	crex_optimizer_ctx ctx;
	zval *zv;

	ctx.arena = crex_arena_create(64 * 1024);
	ctx.script = script;
	ctx.constants = NULL;
	ctx.optimization_level = optimization_level;
	ctx.debug_level = debug_level;

	if ((CREX_OPTIMIZER_PASS_6 & optimization_level) &&
	    (CREX_OPTIMIZER_PASS_7 & optimization_level)) {
		/* Optimize using call-graph */
		crex_call_graph call_graph;
		crex_build_call_graph(&ctx.arena, script, &call_graph);

		int i;
		crex_func_info *func_info;

		for (i = 0; i < call_graph.op_arrays_count; i++) {
			crex_revert_pass_two(call_graph.op_arrays[i]);
			crex_optimize(call_graph.op_arrays[i], &ctx);
		}

	    crex_analyze_call_graph(&ctx.arena, script, &call_graph);

		for (i = 0; i < call_graph.op_arrays_count; i++) {
			func_info = CREX_FUNC_INFO(call_graph.op_arrays[i]);
			if (func_info) {
				func_info->call_map = crex_build_call_map(&ctx.arena, func_info, call_graph.op_arrays[i]);
				if (call_graph.op_arrays[i]->fn_flags & CREX_ACC_HAS_RETURN_TYPE) {
					crex_init_func_return_info(call_graph.op_arrays[i], script, &func_info->return_info);
				}
			}
		}

		for (i = 0; i < call_graph.op_arrays_count; i++) {
			func_info = CREX_FUNC_INFO(call_graph.op_arrays[i]);
			if (func_info) {
				if (crex_dfa_analyze_op_array(call_graph.op_arrays[i], &ctx, &func_info->ssa) == SUCCESS) {
					func_info->flags = func_info->ssa.cfg.flags;
				} else {
					CREX_SET_FUNC_INFO(call_graph.op_arrays[i], NULL);
				}
			}
		}

		//TODO: perform inner-script inference???
		for (i = 0; i < call_graph.op_arrays_count; i++) {
			func_info = CREX_FUNC_INFO(call_graph.op_arrays[i]);
			if (func_info) {
				crex_dfa_optimize_op_array(call_graph.op_arrays[i], &ctx, &func_info->ssa, func_info->call_map);
			}
		}

		if (debug_level & CREX_DUMP_AFTER_PASS_7) {
			for (i = 0; i < call_graph.op_arrays_count; i++) {
				crex_dump_op_array(call_graph.op_arrays[i], 0, "after pass 7", NULL);
			}
		}

		if (CREX_OPTIMIZER_PASS_9 & optimization_level) {
			for (i = 0; i < call_graph.op_arrays_count; i++) {
				crex_optimize_temporary_variables(call_graph.op_arrays[i], &ctx);
				if (debug_level & CREX_DUMP_AFTER_PASS_9) {
					crex_dump_op_array(call_graph.op_arrays[i], 0, "after pass 9", NULL);
				}
			}
		}

		if (CREX_OPTIMIZER_PASS_11 & optimization_level) {
			for (i = 0; i < call_graph.op_arrays_count; i++) {
				crex_optimizer_compact_literals(call_graph.op_arrays[i], &ctx);
				if (debug_level & CREX_DUMP_AFTER_PASS_11) {
					crex_dump_op_array(call_graph.op_arrays[i], 0, "after pass 11", NULL);
				}
			}
		}

		if (CREX_OPTIMIZER_PASS_13 & optimization_level) {
			for (i = 0; i < call_graph.op_arrays_count; i++) {
				crex_optimizer_compact_vars(call_graph.op_arrays[i]);
				if (debug_level & CREX_DUMP_AFTER_PASS_13) {
					crex_dump_op_array(call_graph.op_arrays[i], 0, "after pass 13", NULL);
				}
			}
		}

		if (CREX_OBSERVER_ENABLED) {
			for (i = 0; i < call_graph.op_arrays_count; i++) {
				++call_graph.op_arrays[i]->T; // ensure accurate temporary count for stack size precalculation
			}
		}

		if (CREX_OPTIMIZER_PASS_12 & optimization_level) {
			for (i = 0; i < call_graph.op_arrays_count; i++) {
				crex_adjust_fcall_stack_size_graph(call_graph.op_arrays[i]);
			}
		}

		for (i = 0; i < call_graph.op_arrays_count; i++) {
			op_array = call_graph.op_arrays[i];
			func_info = CREX_FUNC_INFO(op_array);
			if (func_info && func_info->ssa.var_info) {
				crex_redo_pass_two_ex(op_array, &func_info->ssa);
				if (op_array->live_range) {
					crex_recalc_live_ranges(op_array, needs_live_range);
				}
			} else {
				op_array->T -= CREX_OBSERVER_ENABLED; // redo_pass_two will re-increment it

				crex_redo_pass_two(op_array);
				if (op_array->live_range) {
					crex_recalc_live_ranges(op_array, NULL);
				}
			}
		}

		for (i = 0; i < call_graph.op_arrays_count; i++) {
			CREX_SET_FUNC_INFO(call_graph.op_arrays[i], NULL);
		}
	} else {
		crex_foreach_op_array(script, step_optimize_op_array, &ctx);

		if (CREX_OPTIMIZER_PASS_12 & optimization_level) {
			crex_foreach_op_array(script, step_adjust_fcall_stack_size, &ctx);
		}
	}

	CREX_HASH_MAP_FOREACH_VAL(&script->class_table, zv) {
		if (C_TYPE_P(zv) == IS_ALIAS_PTR) {
			continue;
		}
		crex_class_entry *ce = C_CE_P(zv);
		CREX_HASH_MAP_FOREACH_STR_KEY_PTR(&ce->function_table, name, op_array) {
			if (op_array->scope != ce && op_array->type == CREX_USER_FUNCTION) {
				crex_op_array *orig_op_array =
					crex_hash_find_ptr(&op_array->scope->function_table, name);

				CREX_ASSERT(orig_op_array != NULL);
				if (orig_op_array != op_array) {
					uint32_t fn_flags = op_array->fn_flags;
					crex_function *prototype = op_array->prototype;
					HashTable *ht = op_array->static_variables;

					*op_array = *orig_op_array;
					op_array->fn_flags = fn_flags;
					op_array->prototype = prototype;
					op_array->static_variables = ht;
				}
			}
		} CREX_HASH_FOREACH_END();
	} CREX_HASH_FOREACH_END();

	crex_optimizer_call_registered_passes(script, &ctx);

	if ((debug_level & CREX_DUMP_AFTER_OPTIMIZER) &&
			(CREX_OPTIMIZER_PASS_7 & optimization_level)) {
		crex_foreach_op_array(script, step_dump_after_optimizer, NULL);
	}

	if (ctx.constants) {
		crex_hash_destroy(ctx.constants);
	}
	crex_arena_destroy(ctx.arena);
}

CREX_API int crex_optimizer_register_pass(crex_optimizer_pass_t pass)
{
	if (!pass) {
		return -1;
	}

	if (crex_optimizer_registered_passes.last == CREX_OPTIMIZER_MAX_REGISTERED_PASSES) {
		return -1;
	}

	crex_optimizer_registered_passes.pass[
		crex_optimizer_registered_passes.last++] = pass;

	return crex_optimizer_registered_passes.last;
}

CREX_API void crex_optimizer_unregister_pass(int idx)
{
	crex_optimizer_registered_passes.pass[idx-1] = NULL;
}

crex_result crex_optimizer_startup(void)
{
	return crex_func_info_startup();
}

crex_result crex_optimizer_shutdown(void)
{
	return crex_func_info_shutdown();
}
