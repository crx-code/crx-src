/*
   +----------------------------------------------------------------------+
   | Crex Engine, SCCP - Sparse Conditional Constant Propagation          |
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
   | Authors: Nikita Popov <nikic@crx.net>                                |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crex_API.h"
#include "crex_exceptions.h"
#include "crex_ini.h"
#include "crex_type_info.h"
#include "Optimizer/crex_optimizer_internal.h"
#include "Optimizer/crex_call_graph.h"
#include "Optimizer/crex_inference.h"
#include "Optimizer/scdf.h"
#include "Optimizer/crex_dump.h"

/* This implements sparse conditional constant propagation (SCCP) based on the SCDF framework. The
 * used value lattice is defined as follows:
 *
 * BOT < {constant values} < TOP
 *
 * TOP indicates an underdefined value, i.e. that we do not yet know the value of variable.
 * BOT indicates an overdefined value, i.e. that we know the variable to be non-constant.
 *
 * All variables are optimistically initialized to TOP, apart from the implicit variables defined
 * at the start of the first block. Note that variables that MAY_BE_REF are *not* initialized to
 * BOT. We rely on the fact that any operation resulting in a reference will produce a BOT anyway.
 * This is better because such operations might never be reached due to the conditional nature of
 * the algorithm.
 *
 * The meet operation for phi functions is defined as follows:
 * BOT + any = BOT
 * TOP + any = any
 * C_i + C_i = C_i (i.e. two equal constants)
 * C_i + C_j = BOT (i.e. two different constants)
 *
 * When evaluating instructions TOP and BOT are handled as follows:
 * a) If any operand is BOT, the result is BOT. The main exception to this is op1 of ASSIGN, which
 *    is ignored. However, if the op1 MAY_BE_REF we do have to propagate the BOT.
 * b) Otherwise, if the instruction can never be evaluated (either in general, or with the
 *    specific modifiers) the result is BOT.
 * c) Otherwise, if any operand is TOP, the result is TOP.
 * d) Otherwise (at this point all operands are known and constant), if we can compute the result
 *    for these specific constants (without throwing notices or similar) then that is the result.
 * e) Otherwise the result is BOT.
 *
 * It is sometimes possible to determine a result even if one argument is TOP / BOT, e.g. for things
 * like BOT*0. Right now we don't bother with this.
 *
 * Feasible successors for conditional branches are determined as follows:
 * a) If we don't support the branch type or branch on BOT, all successors are feasible.
 * b) Otherwise, if we branch on TOP none of the successors are feasible.
 * c) Otherwise (we branch on a constant), the feasible successors are marked based on the constant
 *    (usually only one successor will be feasible).
 *
 * The original SCCP algorithm is extended with ability to propagate constant array
 * elements and object properties. The extension is based on a variation of Array
 * SSA form and its application to Spare Constant Propagation, described at
 * "Array SSA Form" by Vivek Sarkar, Kathleen Knobe and Stephen Fink in chapter
 * 16 of the SSA book.
 */

#define SCP_DEBUG 0

typedef struct _sccp_ctx {
	scdf_ctx scdf;
	crex_call_info **call_map;
	zval *values;
	zval top;
	zval bot;
} sccp_ctx;

#define TOP ((uint8_t)-1)
#define BOT ((uint8_t)-2)
#define PARTIAL_ARRAY ((uint8_t)-3)
#define PARTIAL_OBJECT ((uint8_t)-4)
#define IS_TOP(zv) (C_TYPE_P(zv) == TOP)
#define IS_BOT(zv) (C_TYPE_P(zv) == BOT)
#define IS_PARTIAL_ARRAY(zv) (C_TYPE_P(zv) == PARTIAL_ARRAY)
#define IS_PARTIAL_OBJECT(zv) (C_TYPE_P(zv) == PARTIAL_OBJECT)

#define MAKE_PARTIAL_ARRAY(zv) (C_TYPE_INFO_P(zv) = PARTIAL_ARRAY | (IS_TYPE_REFCOUNTED << C_TYPE_FLAGS_SHIFT))
#define MAKE_PARTIAL_OBJECT(zv) (C_TYPE_INFO_P(zv) = PARTIAL_OBJECT | (IS_TYPE_REFCOUNTED << C_TYPE_FLAGS_SHIFT))

#define MAKE_TOP(zv) (C_TYPE_INFO_P(zv) = TOP)
#define MAKE_BOT(zv) (C_TYPE_INFO_P(zv) = BOT)

static void scp_dump_value(zval *zv) {
	if (IS_TOP(zv)) {
		fprintf(stderr, " top");
	} else if (IS_BOT(zv)) {
		fprintf(stderr, " bot");
	} else if (C_TYPE_P(zv) == IS_ARRAY || IS_PARTIAL_ARRAY(zv)) {
		fprintf(stderr, " %s[", IS_PARTIAL_ARRAY(zv) ? "partial " : "");
		crex_dump_ht(C_ARRVAL_P(zv));
		fprintf(stderr, "]");
	} else if (IS_PARTIAL_OBJECT(zv)) {
		fprintf(stderr, " {");
		crex_dump_ht(C_ARRVAL_P(zv));
		fprintf(stderr, "}");
	} else {
		crex_dump_const(zv);
	}
}

static void empty_partial_array(zval *zv)
{
	MAKE_PARTIAL_ARRAY(zv);
	C_ARR_P(zv) = crex_new_array(8);
}

static void dup_partial_array(zval *dst, const zval *src)
{
	MAKE_PARTIAL_ARRAY(dst);
	C_ARR_P(dst) = crex_array_dup(C_ARR_P(src));
}

static void empty_partial_object(zval *zv)
{
	MAKE_PARTIAL_OBJECT(zv);
	C_ARR_P(zv) = crex_new_array(8);
}

static void dup_partial_object(zval *dst, const zval *src)
{
	MAKE_PARTIAL_OBJECT(dst);
	C_ARR_P(dst) = crex_array_dup(C_ARR_P(src));
}

static inline bool value_known(zval *zv) {
	return !IS_TOP(zv) && !IS_BOT(zv);
}

/* Sets new value for variable and ensures that it is lower or equal
 * the previous one in the constant propagation lattice. */
static void set_value(scdf_ctx *scdf, sccp_ctx *ctx, int var, const zval *new) {
	zval *value = &ctx->values[var];
	if (IS_BOT(value) || IS_TOP(new)) {
		return;
	}

#if SCP_DEBUG
	fprintf(stderr, "Lowering #%d.", var);
	crex_dump_var(scdf->op_array, IS_CV, scdf->ssa->vars[var].var);
	fprintf(stderr, " from");
	scp_dump_value(value);
	fprintf(stderr, " to");
	scp_dump_value(new);
	fprintf(stderr, "\n");
#endif

	if (IS_TOP(value) || IS_BOT(new)) {
		zval_ptr_dtor_nogc(value);
		ZVAL_COPY(value, new);
		scdf_add_to_worklist(scdf, var);
		return;
	}

	/* Always replace PARTIAL_(ARRAY|OBJECT), as new maybe changed by join_partial_(arrays|object) */
	if (IS_PARTIAL_ARRAY(new) || IS_PARTIAL_OBJECT(new)) {
		if (C_TYPE_P(value) != C_TYPE_P(new)
			|| crex_hash_num_elements(C_ARR_P(new)) != crex_hash_num_elements(C_ARR_P(value))) {
			zval_ptr_dtor_nogc(value);
			ZVAL_COPY(value, new);
			scdf_add_to_worklist(scdf, var);
		}
		return;
	}

#if CREX_DEBUG
	CREX_ASSERT(crex_is_identical(value, new) ||
		(C_TYPE_P(value) == IS_DOUBLE && C_TYPE_P(new) == IS_DOUBLE && isnan(C_DVAL_P(value)) && isnan(C_DVAL_P(new))));
#endif
}

static zval *get_op1_value(sccp_ctx *ctx, crex_op *opline, const crex_ssa_op *ssa_op) {
	if (opline->op1_type == IS_CONST) {
		return CT_CONSTANT_EX(ctx->scdf.op_array, opline->op1.constant);
	} else if (ssa_op->op1_use != -1) {
		return &ctx->values[ssa_op->op1_use];
	} else {
		return NULL;
	}
}

static zval *get_op2_value(sccp_ctx *ctx, const crex_op *opline, const crex_ssa_op *ssa_op) {
	if (opline->op2_type == IS_CONST) {
		return CT_CONSTANT_EX(ctx->scdf.op_array, opline->op2.constant);
	} else if (ssa_op->op2_use != -1) {
		return &ctx->values[ssa_op->op2_use];
	} else {
		return NULL;
	}
}

static bool can_replace_op1(
		const crex_op_array *op_array, const crex_op *opline, const crex_ssa_op *ssa_op) {
	switch (opline->opcode) {
		case CREX_PRE_INC:
		case CREX_PRE_DEC:
		case CREX_PRE_INC_OBJ:
		case CREX_PRE_DEC_OBJ:
		case CREX_POST_INC:
		case CREX_POST_DEC:
		case CREX_POST_INC_OBJ:
		case CREX_POST_DEC_OBJ:
		case CREX_ASSIGN:
		case CREX_ASSIGN_REF:
		case CREX_ASSIGN_DIM:
		case CREX_ASSIGN_OBJ:
		case CREX_ASSIGN_OBJ_REF:
		case CREX_ASSIGN_OP:
		case CREX_ASSIGN_DIM_OP:
		case CREX_ASSIGN_OBJ_OP:
		case CREX_ASSIGN_STATIC_PROP_OP:
		case CREX_FETCH_DIM_W:
		case CREX_FETCH_DIM_RW:
		case CREX_FETCH_DIM_UNSET:
		case CREX_FETCH_DIM_FUNC_ARG:
		case CREX_FETCH_OBJ_W:
		case CREX_FETCH_OBJ_RW:
		case CREX_FETCH_OBJ_UNSET:
		case CREX_FETCH_OBJ_FUNC_ARG:
		case CREX_FETCH_LIST_W:
		case CREX_UNSET_DIM:
		case CREX_UNSET_OBJ:
		case CREX_SEND_REF:
		case CREX_SEND_VAR_EX:
		case CREX_SEND_FUNC_ARG:
		case CREX_SEND_UNPACK:
		case CREX_SEND_ARRAY:
		case CREX_SEND_USER:
		case CREX_FE_RESET_RW:
			return 0;
		/* Do not accept CONST */
		case CREX_ROPE_ADD:
		case CREX_ROPE_END:
		case CREX_BIND_STATIC:
		case CREX_BIND_INIT_STATIC_OR_JMP:
		case CREX_BIND_GLOBAL:
		case CREX_MAKE_REF:
		case CREX_UNSET_CV:
		case CREX_ISSET_ISEMPTY_CV:
			return 0;
		case CREX_INIT_ARRAY:
		case CREX_ADD_ARRAY_ELEMENT:
			return !(opline->extended_value & CREX_ARRAY_ELEMENT_REF);
		case CREX_YIELD:
			return !(op_array->fn_flags & CREX_ACC_RETURN_REFERENCE);
		case CREX_VERIFY_RETURN_TYPE:
			// TODO: This would require a non-local change ???
			return 0;
		case CREX_OP_DATA:
			return (opline - 1)->opcode != CREX_ASSIGN_OBJ_REF &&
				(opline - 1)->opcode != CREX_ASSIGN_STATIC_PROP_REF;
		default:
			if (ssa_op->op1_def != -1) {
				CREX_UNREACHABLE();
				return 0;
			}
	}

	return 1;
}

static bool can_replace_op2(
		const crex_op_array *op_array, crex_op *opline, crex_ssa_op *ssa_op) {
	switch (opline->opcode) {
		/* Do not accept CONST */
		case CREX_DECLARE_CLASS_DELAYED:
		case CREX_BIND_LEXICAL:
		case CREX_FE_FETCH_R:
		case CREX_FE_FETCH_RW:
			return 0;
	}
	return 1;
}

static bool try_replace_op1(
		sccp_ctx *ctx, crex_op *opline, crex_ssa_op *ssa_op, int var, zval *value) {
	if (ssa_op->op1_use == var && can_replace_op1(ctx->scdf.op_array, opline, ssa_op)) {
		zval zv;
		ZVAL_COPY(&zv, value);
		if (crex_optimizer_update_op1_const(ctx->scdf.op_array, opline, &zv)) {
			return 1;
		}
		zval_ptr_dtor_nogc(&zv);
	}
	return 0;
}

static bool try_replace_op2(
		sccp_ctx *ctx, crex_op *opline, crex_ssa_op *ssa_op, int var, zval *value) {
	if (ssa_op->op2_use == var && can_replace_op2(ctx->scdf.op_array, opline, ssa_op)) {
		zval zv;
		ZVAL_COPY(&zv, value);
		if (crex_optimizer_update_op2_const(ctx->scdf.op_array, opline, &zv)) {
			return 1;
		}
		zval_ptr_dtor_nogc(&zv);
	}
	return 0;
}

static inline crex_result ct_eval_binary_op(zval *result, uint8_t binop, zval *op1, zval *op2) {
	/* TODO: We could implement support for evaluation of + on partial arrays. */
	if (IS_PARTIAL_ARRAY(op1) || IS_PARTIAL_ARRAY(op2)) {
		return FAILURE;
	}

	return crex_optimizer_eval_binary_op(result, binop, op1, op2);
}

static inline crex_result ct_eval_bool_cast(zval *result, zval *op) {
	if (IS_PARTIAL_ARRAY(op)) {
		if (crex_hash_num_elements(C_ARRVAL_P(op)) == 0) {
			/* An empty partial array may be non-empty at runtime, we don't know whether the
			 * result will be true or false. */
			return FAILURE;
		}

		ZVAL_TRUE(result);
		return SUCCESS;
	}

	ZVAL_BOOL(result, crex_is_true(op));
	return SUCCESS;
}

static inline crex_result zval_to_string_offset(crex_long *result, zval *op) {
	switch (C_TYPE_P(op)) {
		case IS_LONG:
			*result = C_LVAL_P(op);
			return SUCCESS;
		case IS_STRING:
			if (IS_LONG == is_numeric_string(
					C_STRVAL_P(op), C_STRLEN_P(op), result, NULL, 0)) {
				return SUCCESS;
			}
			return FAILURE;
		default:
			return FAILURE;
	}
}

static inline crex_result fetch_array_elem(zval **result, zval *op1, zval *op2) {
	switch (C_TYPE_P(op2)) {
		case IS_NULL:
			*result = crex_hash_find(C_ARR_P(op1), ZSTR_EMPTY_ALLOC());
			return SUCCESS;
		case IS_FALSE:
			*result = crex_hash_index_find(C_ARR_P(op1), 0);
			return SUCCESS;
		case IS_TRUE:
			*result = crex_hash_index_find(C_ARR_P(op1), 1);
			return SUCCESS;
		case IS_LONG:
			*result = crex_hash_index_find(C_ARR_P(op1), C_LVAL_P(op2));
			return SUCCESS;
		case IS_DOUBLE: {
			crex_long lval = crex_dval_to_lval(C_DVAL_P(op2));
			if (!crex_is_long_compatible(C_DVAL_P(op2), lval)) {
				return FAILURE;
			}
			*result = crex_hash_index_find(C_ARR_P(op1), lval);
			return SUCCESS;
		}
		case IS_STRING:
			*result = crex_symtable_find(C_ARR_P(op1), C_STR_P(op2));
			return SUCCESS;
		default:
			return FAILURE;
	}
}

static inline crex_result ct_eval_fetch_dim(zval *result, zval *op1, zval *op2, int support_strings) {
	if (C_TYPE_P(op1) == IS_ARRAY || IS_PARTIAL_ARRAY(op1)) {
		zval *value;
		if (fetch_array_elem(&value, op1, op2) == SUCCESS && value && !IS_BOT(value)) {
			ZVAL_COPY(result, value);
			return SUCCESS;
		}
	} else if (support_strings && C_TYPE_P(op1) == IS_STRING) {
		crex_long index;
		if (zval_to_string_offset(&index, op2) == FAILURE) {
			return FAILURE;
		}
		if (index >= 0 && index < C_STRLEN_P(op1)) {
			ZVAL_STR(result, crex_string_init(&C_STRVAL_P(op1)[index], 1, 0));
			return SUCCESS;
		}
	}
	return FAILURE;
}

/* op1 may be NULL here to indicate an unset value */
static inline crex_result ct_eval_isset_isempty(zval *result, uint32_t extended_value, zval *op1) {
	zval zv;
	if (!(extended_value & CREX_ISEMPTY)) {
		ZVAL_BOOL(result, op1 && C_TYPE_P(op1) != IS_NULL);
		return SUCCESS;
	} else if (!op1) {
		ZVAL_TRUE(result);
		return SUCCESS;
	} else if (ct_eval_bool_cast(&zv, op1) == SUCCESS) {
		ZVAL_BOOL(result, C_TYPE(zv) == IS_FALSE);
		return SUCCESS;
	} else {
		return FAILURE;
	}
}

static inline crex_result ct_eval_isset_dim(zval *result, uint32_t extended_value, zval *op1, zval *op2) {
	if (C_TYPE_P(op1) == IS_ARRAY || IS_PARTIAL_ARRAY(op1)) {
		zval *value;
		if (fetch_array_elem(&value, op1, op2) == FAILURE) {
			return FAILURE;
		}
		if (IS_PARTIAL_ARRAY(op1) && (!value || IS_BOT(value))) {
			return FAILURE;
		}
		return ct_eval_isset_isempty(result, extended_value, value);
	} else if (C_TYPE_P(op1) == IS_STRING) {
		// TODO
		return FAILURE;
	} else {
		ZVAL_BOOL(result, (extended_value & CREX_ISEMPTY));
		return SUCCESS;
	}
}

static inline crex_result ct_eval_del_array_elem(zval *result, const zval *key) {
	CREX_ASSERT(IS_PARTIAL_ARRAY(result));

	switch (C_TYPE_P(key)) {
		case IS_NULL:
			crex_hash_del(C_ARR_P(result), ZSTR_EMPTY_ALLOC());
			break;
		case IS_FALSE:
			crex_hash_index_del(C_ARR_P(result), 0);
			break;
		case IS_TRUE:
			crex_hash_index_del(C_ARR_P(result), 1);
			break;
		case IS_LONG:
			crex_hash_index_del(C_ARR_P(result), C_LVAL_P(key));
			break;
		case IS_DOUBLE: {
			crex_long lval = crex_dval_to_lval(C_DVAL_P(key));
			if (!crex_is_long_compatible(C_DVAL_P(key), lval)) {
				return FAILURE;
			}
			crex_hash_index_del(C_ARR_P(result), lval);
			break;
		}
		case IS_STRING:
			crex_symtable_del(C_ARR_P(result), C_STR_P(key));
			break;
		default:
			return FAILURE;
	}

	return SUCCESS;
}

static inline crex_result ct_eval_add_array_elem(zval *result, zval *value, const zval *key) {
	if (!key) {
		SEPARATE_ARRAY(result);
		if ((value = crex_hash_next_index_insert(C_ARR_P(result), value))) {
			C_TRY_ADDREF_P(value);
			return SUCCESS;
		}
		return FAILURE;
	}

	switch (C_TYPE_P(key)) {
		case IS_NULL:
			SEPARATE_ARRAY(result);
			value = crex_hash_update(C_ARR_P(result), ZSTR_EMPTY_ALLOC(), value);
			break;
		case IS_FALSE:
			SEPARATE_ARRAY(result);
			value = crex_hash_index_update(C_ARR_P(result), 0, value);
			break;
		case IS_TRUE:
			SEPARATE_ARRAY(result);
			value = crex_hash_index_update(C_ARR_P(result), 1, value);
			break;
		case IS_LONG:
			SEPARATE_ARRAY(result);
			value = crex_hash_index_update(C_ARR_P(result), C_LVAL_P(key), value);
			break;
		case IS_DOUBLE: {
			crex_long lval = crex_dval_to_lval(C_DVAL_P(key));
			if (!crex_is_long_compatible(C_DVAL_P(key), lval)) {
				return FAILURE;
			}
			SEPARATE_ARRAY(result);
			value = crex_hash_index_update(
				C_ARR_P(result), lval, value);
			break;
		}
		case IS_STRING:
			SEPARATE_ARRAY(result);
			value = crex_symtable_update(C_ARR_P(result), C_STR_P(key), value);
			break;
		default:
			return FAILURE;
	}

	C_TRY_ADDREF_P(value);
	return SUCCESS;
}

static inline crex_result ct_eval_add_array_unpack(zval *result, zval *array) {
	crex_string *key;
	zval *value;
	if (C_TYPE_P(array) != IS_ARRAY) {
		return FAILURE;
	}

	SEPARATE_ARRAY(result);
	CREX_HASH_FOREACH_STR_KEY_VAL(C_ARRVAL_P(array), key, value) {
		if (key) {
			value = crex_hash_update(C_ARR_P(result), key, value);
		} else {
			value = crex_hash_next_index_insert(C_ARR_P(result), value);
		}
		if (!value) {
			return FAILURE;
		}
		C_TRY_ADDREF_P(value);
	} CREX_HASH_FOREACH_END();
	return SUCCESS;
}

static inline crex_result ct_eval_assign_dim(zval *result, zval *value, const zval *key) {
	switch (C_TYPE_P(result)) {
		case IS_NULL:
		case IS_FALSE:
			array_init(result);
			CREX_FALLTHROUGH;
		case IS_ARRAY:
		case PARTIAL_ARRAY:
			return ct_eval_add_array_elem(result, value, key);
		case IS_STRING:
			// TODO Before enabling this case, make sure ARRAY_DIM result op is correct
#if 0
			crex_long index;
			crex_string *new_str, *value_str;
			if (!key || C_TYPE_P(value) == IS_ARRAY
					|| zval_to_string_offset(&index, key) == FAILURE || index < 0) {
				return FAILURE;
			}

			if (index >= C_STRLEN_P(result)) {
				new_str = crex_string_alloc(index + 1, 0);
				memcpy(ZSTR_VAL(new_str), C_STRVAL_P(result), C_STRLEN_P(result));
				memset(ZSTR_VAL(new_str) + C_STRLEN_P(result), ' ', index - C_STRLEN_P(result));
				ZSTR_VAL(new_str)[index + 1] = 0;
			} else {
				new_str = crex_string_init(C_STRVAL_P(result), C_STRLEN_P(result), 0);
			}

			value_str = zval_get_string(value);
			ZVAL_STR(result, new_str);
			C_STRVAL_P(result)[index] = ZSTR_VAL(value_str)[0];
			crex_string_release_ex(value_str, 0);
#endif
			return FAILURE;
		default:
			return FAILURE;
	}
}

static inline crex_result fetch_obj_prop(zval **result, zval *op1, zval *op2) {
	switch (C_TYPE_P(op2)) {
		case IS_STRING:
			*result = crex_symtable_find(C_ARR_P(op1), C_STR_P(op2));
			return SUCCESS;
		default:
			return FAILURE;
	}
}

static inline crex_result ct_eval_fetch_obj(zval *result, zval *op1, zval *op2) {
	if (IS_PARTIAL_OBJECT(op1)) {
		zval *value;
		if (fetch_obj_prop(&value, op1, op2) == SUCCESS && value && !IS_BOT(value)) {
			ZVAL_COPY(result, value);
			return SUCCESS;
		}
	}
	return FAILURE;
}

static inline crex_result ct_eval_isset_obj(zval *result, uint32_t extended_value, zval *op1, zval *op2) {
	if (IS_PARTIAL_OBJECT(op1)) {
		zval *value;
		if (fetch_obj_prop(&value, op1, op2) == FAILURE) {
			return FAILURE;
		}
		if (!value || IS_BOT(value)) {
			return FAILURE;
		}
		return ct_eval_isset_isempty(result, extended_value, value);
	} else {
		ZVAL_BOOL(result, (extended_value & CREX_ISEMPTY));
		return SUCCESS;
	}
}

static inline crex_result ct_eval_del_obj_prop(zval *result, const zval *key) {
	CREX_ASSERT(IS_PARTIAL_OBJECT(result));

	switch (C_TYPE_P(key)) {
		case IS_STRING:
			crex_symtable_del(C_ARR_P(result), C_STR_P(key));
			break;
		default:
			return FAILURE;
	}

	return SUCCESS;
}

static inline crex_result ct_eval_add_obj_prop(zval *result, zval *value, const zval *key) {
	switch (C_TYPE_P(key)) {
		case IS_STRING:
			value = crex_symtable_update(C_ARR_P(result), C_STR_P(key), value);
			break;
		default:
			return FAILURE;
	}

	C_TRY_ADDREF_P(value);
	return SUCCESS;
}

static inline crex_result ct_eval_assign_obj(zval *result, zval *value, const zval *key) {
	switch (C_TYPE_P(result)) {
		case IS_NULL:
		case IS_FALSE:
			empty_partial_object(result);
			CREX_FALLTHROUGH;
		case PARTIAL_OBJECT:
			return ct_eval_add_obj_prop(result, value, key);
		default:
			return FAILURE;
	}
}

static inline crex_result ct_eval_incdec(zval *result, uint8_t opcode, zval *op1) {
	/* As of CRX 8.3 with the warning/deprecation notices any type other than int/double/null will emit a diagnostic
	if (C_TYPE_P(op1) == IS_ARRAY || IS_PARTIAL_ARRAY(op1)) {
		return FAILURE;
	}
	*/
	if (C_TYPE_P(op1) != IS_LONG && C_TYPE_P(op1) != IS_DOUBLE && C_TYPE_P(op1) != IS_NULL) {
		return FAILURE;
	}

	ZVAL_COPY(result, op1);
	if (opcode == CREX_PRE_INC
			|| opcode == CREX_POST_INC
			|| opcode == CREX_PRE_INC_OBJ
			|| opcode == CREX_POST_INC_OBJ) {
		increment_function(result);
	} else {
		/* Decrement on null emits a deprecation notice */
		if (C_TYPE_P(op1) == IS_NULL) {
			zval_ptr_dtor(result);
			return FAILURE;
		}
		decrement_function(result);
	}
	return SUCCESS;
}

static inline void ct_eval_type_check(zval *result, uint32_t type_mask, zval *op1) {
	uint32_t type = C_TYPE_P(op1);
	if (type == PARTIAL_ARRAY) {
		type = IS_ARRAY;
	} else if (type == PARTIAL_OBJECT) {
		type = IS_OBJECT;
	}
	ZVAL_BOOL(result, (type_mask >> type) & 1);
}

static inline crex_result ct_eval_in_array(zval *result, uint32_t extended_value, zval *op1, zval *op2) {
	HashTable *ht;
	bool res;

	if (C_TYPE_P(op2) != IS_ARRAY) {
		return FAILURE;
	}
	ht = C_ARRVAL_P(op2);
	if (EXPECTED(C_TYPE_P(op1) == IS_STRING)) {
		res = crex_hash_exists(ht, C_STR_P(op1));
	} else if (extended_value) {
		if (EXPECTED(C_TYPE_P(op1) == IS_LONG)) {
			res = crex_hash_index_exists(ht, C_LVAL_P(op1));
		} else {
			res = 0;
		}
	} else if (C_TYPE_P(op1) <= IS_FALSE) {
		res = crex_hash_exists(ht, ZSTR_EMPTY_ALLOC());
	} else {
		crex_string *key;
		zval key_tmp;

		res = 0;
		CREX_HASH_MAP_FOREACH_STR_KEY(ht, key) {
			ZVAL_STR(&key_tmp, key);
			if (crex_compare(op1, &key_tmp) == 0) {
				res = 1;
				break;
			}
		} CREX_HASH_FOREACH_END();
	}
	ZVAL_BOOL(result, res);
	return SUCCESS;
}

static inline crex_result ct_eval_array_key_exists(zval *result, zval *op1, zval *op2) {
	zval *value;

	if (C_TYPE_P(op2) != IS_ARRAY && !IS_PARTIAL_ARRAY(op2)) {
		return FAILURE;
	}
	if (C_TYPE_P(op1) != IS_STRING && C_TYPE_P(op1) != IS_LONG && C_TYPE_P(op1) != IS_NULL) {
		return FAILURE;
	}
	if (fetch_array_elem(&value, op2, op1) == FAILURE) {
		return FAILURE;
	}
	if (IS_PARTIAL_ARRAY(op2) && (!value || IS_BOT(value))) {
		return FAILURE;
	}

	ZVAL_BOOL(result, value != NULL);
	return SUCCESS;
}

static bool can_ct_eval_func_call(crex_function *func, crex_string *name, uint32_t num_args, zval **args) {
	/* Precondition: func->type == CREX_INTERNAL_FUNCTION, this is a global function */
	/* Functions setting CREX_ACC_COMPILE_TIME_EVAL (@compile-time-eval) must always produce the same result for the same arguments,
	 * and have no dependence on global state (such as locales). It is okay if they throw
	 * or warn on invalid arguments, as we detect this and will discard the evaluation result. */
	if (func->common.fn_flags & CREX_ACC_COMPILE_TIME_EVAL) {
		/* This has @compile-time-eval in stub info and uses a macro such as CREX_SUPPORTS_COMPILE_TIME_EVAL_FE */
		return true;
	}
#ifndef CREX_WIN32
	/* On Windows this function may be code page dependent. */
	if (crex_string_equals_literal(name, "dirname")) {
		return true;
	}
#endif

	if (num_args == 2) {
		if (crex_string_equals_literal(name, "str_repeat")) {
			/* Avoid creating overly large strings at compile-time. */
			bool overflow;
			return C_TYPE_P(args[0]) == IS_STRING
				&& C_TYPE_P(args[1]) == IS_LONG
				&& crex_safe_address(C_STRLEN_P(args[0]), C_LVAL_P(args[1]), 0, &overflow) < 64 * 1024
				&& !overflow;
		}
		return false;
	}

	return false;
}

/* The functions chosen here are simple to implement and either likely to affect a branch,
 * or just happened to be commonly used with constant operands in WP (need to test other
 * applications as well, of course). */
static inline crex_result ct_eval_func_call(
		crex_op_array *op_array, zval *result, crex_string *name, uint32_t num_args, zval **args) {
	uint32_t i;
	crex_function *func = crex_hash_find_ptr(CG(function_table), name);
	if (!func || func->type != CREX_INTERNAL_FUNCTION) {
		return FAILURE;
	}

	if (num_args == 1 && C_TYPE_P(args[0]) == IS_STRING &&
			crex_optimizer_eval_special_func_call(result, name, C_STR_P(args[0])) == SUCCESS) {
		return SUCCESS;
	}

	if (!can_ct_eval_func_call(func, name, num_args, args)) {
		return FAILURE;
	}

	crex_execute_data *prev_execute_data = EG(current_execute_data);
	crex_execute_data *execute_data, dummy_frame;
	crex_op dummy_opline;

	/* Add a dummy frame to get the correct strict_types behavior. */
	memset(&dummy_frame, 0, sizeof(crex_execute_data));
	memset(&dummy_opline, 0, sizeof(crex_op));
	dummy_frame.func = (crex_function *) op_array;
	dummy_frame.opline = &dummy_opline;
	dummy_opline.opcode = CREX_DO_FCALL;

	execute_data = safe_emalloc(num_args, sizeof(zval), CREX_CALL_FRAME_SLOT * sizeof(zval));
	memset(execute_data, 0, sizeof(crex_execute_data));
	execute_data->prev_execute_data = &dummy_frame;
	EG(current_execute_data) = execute_data;

	/* Enable suppression and counting of warnings. */
	CREX_ASSERT(EG(capture_warnings_during_sccp) == 0);
	EG(capture_warnings_during_sccp) = 1;

	EX(func) = func;
	EX_NUM_ARGS() = num_args;
	for (i = 0; i < num_args; i++) {
		ZVAL_COPY(EX_VAR_NUM(i), args[i]);
	}
	ZVAL_NULL(result);
	func->internal_function.handler(execute_data, result);
	for (i = 0; i < num_args; i++) {
		zval_ptr_dtor_nogc(EX_VAR_NUM(i));
	}

	crex_result retval = SUCCESS;
	if (EG(exception)) {
		zval_ptr_dtor(result);
		crex_clear_exception();
		retval = FAILURE;
	}

	if (EG(capture_warnings_during_sccp) > 1) {
		zval_ptr_dtor(result);
		retval = FAILURE;
	}
	EG(capture_warnings_during_sccp) = 0;

	efree(execute_data);
	EG(current_execute_data) = prev_execute_data;
	return retval;
}

#define SET_RESULT(op, zv) do { \
	if (ssa_op->op##_def >= 0) { \
		set_value(scdf, ctx, ssa_op->op##_def, zv); \
	} \
} while (0)
#define SET_RESULT_BOT(op) SET_RESULT(op, &ctx->bot)
#define SET_RESULT_TOP(op) SET_RESULT(op, &ctx->top)

#define SKIP_IF_TOP(op) if (IS_TOP(op)) return;

static void sccp_visit_instr(scdf_ctx *scdf, crex_op *opline, crex_ssa_op *ssa_op) {
	sccp_ctx *ctx = (sccp_ctx *) scdf;
	zval *op1, *op2, zv; /* zv is a temporary to hold result values */

	op1 = get_op1_value(ctx, opline, ssa_op);
	op2 = get_op2_value(ctx, opline, ssa_op);

	switch (opline->opcode) {
		case CREX_ASSIGN:
			/* The value of op1 is irrelevant here, because we are overwriting it
			 * -- unless it can be a reference, in which case we propagate a BOT.
			 * The result is also BOT in this case, because it might be a typed reference. */
			if (IS_BOT(op1) && (ctx->scdf.ssa->var_info[ssa_op->op1_use].type & MAY_BE_REF)) {
				SET_RESULT_BOT(op1);
				SET_RESULT_BOT(result);
			} else {
				SET_RESULT(op1, op2);
				SET_RESULT(result, op2);
			}
			return;
		case CREX_ASSIGN_DIM:
		{
			zval *data = get_op1_value(ctx, opline+1, ssa_op+1);

			/* If $a in $a[$b]=$c is UNDEF, treat it like NULL. There is no warning. */
			if ((ctx->scdf.ssa->var_info[ssa_op->op1_use].type & MAY_BE_ANY) == 0) {
				op1 = &EG(uninitialized_zval);
			}

			if (IS_BOT(op1)) {
				SET_RESULT_BOT(result);
				SET_RESULT_BOT(op1);
				return;
			}

			SKIP_IF_TOP(op1);
			SKIP_IF_TOP(data);
			if (op2) {
				SKIP_IF_TOP(op2);
			}

			if (op2 && IS_BOT(op2)) {
				/* Update of unknown index */
				SET_RESULT_BOT(result);
				if (ssa_op->op1_def >= 0) {
					empty_partial_array(&zv);
					SET_RESULT(op1, &zv);
					zval_ptr_dtor_nogc(&zv);
				} else {
					SET_RESULT_BOT(op1);
				}
				return;
			}

			if (IS_BOT(data)) {

				SET_RESULT_BOT(result);
				if ((IS_PARTIAL_ARRAY(op1)
						|| C_TYPE_P(op1) == IS_NULL
						|| C_TYPE_P(op1) == IS_FALSE
						|| C_TYPE_P(op1) == IS_ARRAY)
					&& ssa_op->op1_def >= 0) {

					if (C_TYPE_P(op1) == IS_NULL || C_TYPE_P(op1) == IS_FALSE) {
						empty_partial_array(&zv);
					} else {
						dup_partial_array(&zv, op1);
					}

					if (!op2) {
						/* We can't add NEXT element into partial array (skip it) */
						SET_RESULT(op1, &zv);
					} else if (ct_eval_del_array_elem(&zv, op2) == SUCCESS) {
						SET_RESULT(op1, &zv);
					} else {
						SET_RESULT_BOT(op1);
					}

					zval_ptr_dtor_nogc(&zv);
				} else {
					SET_RESULT_BOT(op1);
				}

			} else {

				if (IS_PARTIAL_ARRAY(op1)) {
					dup_partial_array(&zv, op1);
				} else {
					ZVAL_COPY(&zv, op1);
				}

				if (!op2 && IS_PARTIAL_ARRAY(&zv)) {
					/* We can't add NEXT element into partial array (skip it) */
					SET_RESULT(result, data);
					SET_RESULT(op1, &zv);
				} else if (ct_eval_assign_dim(&zv, data, op2) == SUCCESS) {
					/* Mark array containing partial array as partial */
					if (IS_PARTIAL_ARRAY(data)) {
						MAKE_PARTIAL_ARRAY(&zv);
					}
					SET_RESULT(result, data);
					SET_RESULT(op1, &zv);
				} else {
					SET_RESULT_BOT(result);
					SET_RESULT_BOT(op1);
				}

				zval_ptr_dtor_nogc(&zv);
			}
			return;
		}

		case CREX_ASSIGN_OBJ:
			if (ssa_op->op1_def >= 0
					&& ctx->scdf.ssa->vars[ssa_op->op1_def].escape_state == ESCAPE_STATE_NO_ESCAPE) {
				zval *data = get_op1_value(ctx, opline+1, ssa_op+1);
				crex_ssa_var_info *var_info = &ctx->scdf.ssa->var_info[ssa_op->op1_use];

				/* Don't try to propagate assignments to (potentially) typed properties. We would
				 * need to deal with errors and type conversions first. */
				// TODO: Distinguish dynamic and declared property assignments here?
				if (!var_info->ce || (var_info->ce->ce_flags & CREX_ACC_HAS_TYPE_HINTS) ||
						!(var_info->ce->ce_flags & CREX_ACC_ALLOW_DYNAMIC_PROPERTIES)) {
					SET_RESULT_BOT(result);
					SET_RESULT_BOT(op1);
					return;
				}

				if (IS_BOT(op1)) {
					SET_RESULT_BOT(result);
					SET_RESULT_BOT(op1);
					return;
				}

				SKIP_IF_TOP(op1);
				SKIP_IF_TOP(data);
				SKIP_IF_TOP(op2);

				if (IS_BOT(op2)) {
					/* Update of unknown property */
					SET_RESULT_BOT(result);
					empty_partial_object(&zv);
					SET_RESULT(op1, &zv);
					zval_ptr_dtor_nogc(&zv);
					return;
				}

				if (IS_BOT(data)) {
					SET_RESULT_BOT(result);
					if (IS_PARTIAL_OBJECT(op1)
							|| C_TYPE_P(op1) == IS_NULL
							|| C_TYPE_P(op1) == IS_FALSE) {

						if (C_TYPE_P(op1) == IS_NULL || C_TYPE_P(op1) == IS_FALSE) {
							empty_partial_object(&zv);
						} else {
							dup_partial_object(&zv, op1);
						}

						if (ct_eval_del_obj_prop(&zv, op2) == SUCCESS) {
							SET_RESULT(op1, &zv);
						} else {
							SET_RESULT_BOT(op1);
						}
						zval_ptr_dtor_nogc(&zv);
					} else {
						SET_RESULT_BOT(op1);
					}

				} else {

					if (IS_PARTIAL_OBJECT(op1)) {
						dup_partial_object(&zv, op1);
					} else {
						ZVAL_COPY(&zv, op1);
					}

					if (ct_eval_assign_obj(&zv, data, op2) == SUCCESS) {
						SET_RESULT(result, data);
						SET_RESULT(op1, &zv);
					} else {
						SET_RESULT_BOT(result);
						SET_RESULT_BOT(op1);
					}

					zval_ptr_dtor_nogc(&zv);
				}
			} else {
				SET_RESULT_BOT(result);
				SET_RESULT_BOT(op1);
			}
			return;

		case CREX_SEND_VAL:
		case CREX_SEND_VAR:
		{
			/* If the value of a SEND for an ICALL changes, we need to reconsider the
			 * ICALL result value. Otherwise we can ignore the opcode. */
			crex_call_info *call;
			if (!ctx->call_map) {
				return;
			}

			call = ctx->call_map[opline - ctx->scdf.op_array->opcodes];
			if (IS_TOP(op1) || !call || !call->caller_call_opline
					|| call->caller_call_opline->opcode != CREX_DO_ICALL) {
				return;
			}

			opline = call->caller_call_opline;
			ssa_op = &ctx->scdf.ssa->ops[opline - ctx->scdf.op_array->opcodes];
			break;
		}
		case CREX_INIT_ARRAY:
		case CREX_ADD_ARRAY_ELEMENT:
		{
			zval *result = NULL;

			if (opline->opcode == CREX_ADD_ARRAY_ELEMENT) {
				result = &ctx->values[ssa_op->result_use];
				if (IS_BOT(result)) {
					SET_RESULT_BOT(result);
					SET_RESULT_BOT(op1);
					return;
				}
				SKIP_IF_TOP(result);
			}

			if (op1) {
				SKIP_IF_TOP(op1);
			}

			if (op2) {
				SKIP_IF_TOP(op2);
			}

			/* We want to avoid keeping around intermediate arrays for each SSA variable in the
			 * ADD_ARRAY_ELEMENT chain. We do this by only keeping the array on the last opcode
			 * and use a NULL value everywhere else. */
			if (result && C_TYPE_P(result) == IS_NULL) {
				SET_RESULT_BOT(result);
				return;
			}

			if (op2 && IS_BOT(op2)) {
				/* Update of unknown index */
				SET_RESULT_BOT(op1);
				if (ssa_op->result_def >= 0) {
					empty_partial_array(&zv);
					SET_RESULT(result, &zv);
					zval_ptr_dtor_nogc(&zv);
				} else {
					SET_RESULT_BOT(result);
				}
				return;
			}

			if ((op1 && IS_BOT(op1))
					|| (opline->extended_value & CREX_ARRAY_ELEMENT_REF)) {

				SET_RESULT_BOT(op1);
				if (ssa_op->result_def >= 0) {
					if (!result) {
						empty_partial_array(&zv);
					} else {
						MAKE_PARTIAL_ARRAY(result);
						ZVAL_COPY_VALUE(&zv, result);
						ZVAL_NULL(result);
					}
					if (!op2) {
						/* We can't add NEXT element into partial array (skip it) */
						SET_RESULT(result, &zv);
					} else if (ct_eval_del_array_elem(&zv, op2) == SUCCESS) {
						SET_RESULT(result, &zv);
					} else {
						SET_RESULT_BOT(result);
					}
					zval_ptr_dtor_nogc(&zv);
				} else {
					/* If any operand is BOT, mark the result as BOT right away.
					 * Exceptions to this rule are handled above. */
					SET_RESULT_BOT(result);
				}

			} else {
				if (result) {
					ZVAL_COPY_VALUE(&zv, result);
					ZVAL_NULL(result);
				} else {
					array_init(&zv);
				}

				if (op1) {
					if (!op2 && IS_PARTIAL_ARRAY(&zv)) {
						/* We can't add NEXT element into partial array (skip it) */
						SET_RESULT(result, &zv);
					} else if (ct_eval_add_array_elem(&zv, op1, op2) == SUCCESS) {
						if (IS_PARTIAL_ARRAY(op1)) {
							MAKE_PARTIAL_ARRAY(&zv);
						}
						SET_RESULT(result, &zv);
					} else {
						SET_RESULT_BOT(result);
					}
				} else {
					SET_RESULT(result, &zv);
				}

				zval_ptr_dtor_nogc(&zv);
			}
			return;
		}
		case CREX_ADD_ARRAY_UNPACK: {
			zval *result = &ctx->values[ssa_op->result_use];
			if (IS_BOT(result) || IS_BOT(op1)) {
				SET_RESULT_BOT(result);
				return;
			}
			SKIP_IF_TOP(result);
			SKIP_IF_TOP(op1);

			/* See comment for ADD_ARRAY_ELEMENT. */
			if (C_TYPE_P(result) == IS_NULL) {
				SET_RESULT_BOT(result);
				return;
			}
			ZVAL_COPY_VALUE(&zv, result);
			ZVAL_NULL(result);

			if (ct_eval_add_array_unpack(&zv, op1) == SUCCESS) {
				SET_RESULT(result, &zv);
			} else {
				SET_RESULT_BOT(result);
			}
			zval_ptr_dtor_nogc(&zv);
			return;
		}
		case CREX_NEW:
			if (ssa_op->result_def >= 0
					&& ctx->scdf.ssa->vars[ssa_op->result_def].escape_state == ESCAPE_STATE_NO_ESCAPE) {
				empty_partial_object(&zv);
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
			} else {
				SET_RESULT_BOT(result);
			}
			return;
		case CREX_ASSIGN_STATIC_PROP_REF:
		case CREX_ASSIGN_OBJ_REF:
			/* Handled here because we also need to BOT the OP_DATA operand, while the generic
			 * code below will not do so. */
			SET_RESULT_BOT(result);
			SET_RESULT_BOT(op1);
			SET_RESULT_BOT(op2);
			opline++;
			ssa_op++;
			SET_RESULT_BOT(op1);
			break;
	}

	if ((op1 && IS_BOT(op1)) || (op2 && IS_BOT(op2))) {
		/* If any operand is BOT, mark the result as BOT right away.
		 * Exceptions to this rule are handled above. */
		SET_RESULT_BOT(result);
		SET_RESULT_BOT(op1);
		SET_RESULT_BOT(op2);
		return;
	}

	switch (opline->opcode) {
		case CREX_ADD:
		case CREX_SUB:
		case CREX_MUL:
		case CREX_DIV:
		case CREX_MOD:
		case CREX_POW:
		case CREX_SL:
		case CREX_SR:
		case CREX_CONCAT:
		case CREX_FAST_CONCAT:
		case CREX_IS_EQUAL:
		case CREX_IS_NOT_EQUAL:
		case CREX_IS_SMALLER:
		case CREX_IS_SMALLER_OR_EQUAL:
		case CREX_IS_IDENTICAL:
		case CREX_IS_NOT_IDENTICAL:
		case CREX_BW_OR:
		case CREX_BW_AND:
		case CREX_BW_XOR:
		case CREX_BOOL_XOR:
		case CREX_CASE:
		case CREX_CASE_STRICT:
			SKIP_IF_TOP(op1);
			SKIP_IF_TOP(op2);

			if (ct_eval_binary_op(&zv, opline->opcode, op1, op2) == SUCCESS) {
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_ASSIGN_OP:
		case CREX_ASSIGN_DIM_OP:
		case CREX_ASSIGN_OBJ_OP:
		case CREX_ASSIGN_STATIC_PROP_OP:
			if (op1) {
				SKIP_IF_TOP(op1);
			}
			if (op2) {
				SKIP_IF_TOP(op2);
			}
			if (opline->opcode == CREX_ASSIGN_OP) {
				if (ct_eval_binary_op(&zv, opline->extended_value, op1, op2) == SUCCESS) {
					SET_RESULT(op1, &zv);
					SET_RESULT(result, &zv);
					zval_ptr_dtor_nogc(&zv);
					break;
				}
			} else if (opline->opcode == CREX_ASSIGN_DIM_OP) {
				if ((IS_PARTIAL_ARRAY(op1) || C_TYPE_P(op1) == IS_ARRAY)
						&& ssa_op->op1_def >= 0 && op2) {
					zval tmp;
					zval *data = get_op1_value(ctx, opline+1, ssa_op+1);

					SKIP_IF_TOP(data);

					if (ct_eval_fetch_dim(&tmp, op1, op2, 0) == SUCCESS) {
						if (IS_BOT(data)) {
							dup_partial_array(&zv, op1);
							ct_eval_del_array_elem(&zv, op2);
							SET_RESULT_BOT(result);
							SET_RESULT(op1, &zv);
							zval_ptr_dtor_nogc(&tmp);
							zval_ptr_dtor_nogc(&zv);
							break;
						}

						if (ct_eval_binary_op(&tmp, opline->extended_value, &tmp, data) == FAILURE) {
							SET_RESULT_BOT(result);
							SET_RESULT_BOT(op1);
							zval_ptr_dtor_nogc(&tmp);
							break;
						}

						if (IS_PARTIAL_ARRAY(op1)) {
							dup_partial_array(&zv, op1);
						} else {
							ZVAL_COPY(&zv, op1);
						}

						if (ct_eval_assign_dim(&zv, &tmp, op2) == SUCCESS) {
							SET_RESULT(result, &tmp);
							SET_RESULT(op1, &zv);
							zval_ptr_dtor_nogc(&tmp);
							zval_ptr_dtor_nogc(&zv);
							break;
						}

						zval_ptr_dtor_nogc(&tmp);
						zval_ptr_dtor_nogc(&zv);
					}
				}
			} else if (opline->opcode == CREX_ASSIGN_OBJ_OP) {
				if (op1 && IS_PARTIAL_OBJECT(op1)
						&& ssa_op->op1_def >= 0
						&& ctx->scdf.ssa->vars[ssa_op->op1_def].escape_state == ESCAPE_STATE_NO_ESCAPE) {
					zval tmp;
					zval *data = get_op1_value(ctx, opline+1, ssa_op+1);

					SKIP_IF_TOP(data);

					if (ct_eval_fetch_obj(&tmp, op1, op2) == SUCCESS) {
						if (IS_BOT(data)) {
							dup_partial_object(&zv, op1);
							ct_eval_del_obj_prop(&zv, op2);
							SET_RESULT_BOT(result);
							SET_RESULT(op1, &zv);
							zval_ptr_dtor_nogc(&tmp);
							zval_ptr_dtor_nogc(&zv);
							break;
						}

						if (ct_eval_binary_op(&tmp, opline->extended_value, &tmp, data) == FAILURE) {
							SET_RESULT_BOT(result);
							SET_RESULT_BOT(op1);
							zval_ptr_dtor_nogc(&tmp);
							break;
						}

						dup_partial_object(&zv, op1);

						if (ct_eval_assign_obj(&zv, &tmp, op2) == SUCCESS) {
							SET_RESULT(result, &tmp);
							SET_RESULT(op1, &zv);
							zval_ptr_dtor_nogc(&tmp);
							zval_ptr_dtor_nogc(&zv);
							break;
						}

						zval_ptr_dtor_nogc(&tmp);
						zval_ptr_dtor_nogc(&zv);
					}
				}
			}
			SET_RESULT_BOT(result);
			SET_RESULT_BOT(op1);
			break;
		case CREX_PRE_INC_OBJ:
		case CREX_PRE_DEC_OBJ:
		case CREX_POST_INC_OBJ:
		case CREX_POST_DEC_OBJ:
			if (op1) {
				SKIP_IF_TOP(op1);
				SKIP_IF_TOP(op2);
				if (IS_PARTIAL_OBJECT(op1)
						&& ssa_op->op1_def >= 0
						&& ctx->scdf.ssa->vars[ssa_op->op1_def].escape_state == ESCAPE_STATE_NO_ESCAPE) {
					zval tmp1, tmp2;

					if (ct_eval_fetch_obj(&tmp1, op1, op2) == SUCCESS) {
						if (ct_eval_incdec(&tmp2, opline->opcode, &tmp1) == SUCCESS) {
							dup_partial_object(&zv, op1);
							ct_eval_assign_obj(&zv, &tmp2, op2);
							if (opline->opcode == CREX_PRE_INC_OBJ || opline->opcode == CREX_PRE_DEC_OBJ) {
								SET_RESULT(result, &tmp2);
							} else {
								SET_RESULT(result, &tmp1);
							}
							zval_ptr_dtor_nogc(&tmp1);
							zval_ptr_dtor_nogc(&tmp2);
							SET_RESULT(op1, &zv);
							zval_ptr_dtor_nogc(&zv);
							break;
						}
						zval_ptr_dtor_nogc(&tmp1);
					}
				}
			}
			SET_RESULT_BOT(op1);
			SET_RESULT_BOT(result);
			break;
		case CREX_PRE_INC:
		case CREX_PRE_DEC:
			SKIP_IF_TOP(op1);
			if (ct_eval_incdec(&zv, opline->opcode, op1) == SUCCESS) {
				SET_RESULT(op1, &zv);
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}
			SET_RESULT_BOT(op1);
			SET_RESULT_BOT(result);
			break;
		case CREX_POST_INC:
		case CREX_POST_DEC:
			SKIP_IF_TOP(op1);
			SET_RESULT(result, op1);
			if (ct_eval_incdec(&zv, opline->opcode, op1) == SUCCESS) {
				SET_RESULT(op1, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}
			SET_RESULT_BOT(op1);
			break;
		case CREX_BW_NOT:
		case CREX_BOOL_NOT:
			SKIP_IF_TOP(op1);
			if (IS_PARTIAL_ARRAY(op1)) {
				SET_RESULT_BOT(result);
				break;
			}
			if (crex_optimizer_eval_unary_op(&zv, opline->opcode, op1) == SUCCESS) {
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_CAST:
			SKIP_IF_TOP(op1);
			if (IS_PARTIAL_ARRAY(op1)) {
				SET_RESULT_BOT(result);
				break;
			}
			if (crex_optimizer_eval_cast(&zv, opline->extended_value, op1) == SUCCESS) {
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_BOOL:
		case CREX_JMPC_EX:
		case CREX_JMPNC_EX:
			SKIP_IF_TOP(op1);
			if (ct_eval_bool_cast(&zv, op1) == SUCCESS) {
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_STRLEN:
			SKIP_IF_TOP(op1);
			if (crex_optimizer_eval_strlen(&zv, op1) == SUCCESS) {
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_YIELD_FROM:
			// tmp = yield from [] -> tmp = null
			SKIP_IF_TOP(op1);
			if (C_TYPE_P(op1) == IS_ARRAY && crex_hash_num_elements(C_ARR_P(op1)) == 0) {
				ZVAL_NULL(&zv);
				SET_RESULT(result, &zv);
				break;
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_COUNT:
			SKIP_IF_TOP(op1);
			if (C_TYPE_P(op1) == IS_ARRAY) {
				ZVAL_LONG(&zv, crex_hash_num_elements(C_ARRVAL_P(op1)));
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_IN_ARRAY:
			SKIP_IF_TOP(op1);
			SKIP_IF_TOP(op2);
			if (ct_eval_in_array(&zv, opline->extended_value, op1, op2) == SUCCESS) {
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_ARRAY_KEY_EXISTS:
			SKIP_IF_TOP(op1);
			SKIP_IF_TOP(op2);
			if (ct_eval_array_key_exists(&zv, op1, op2) == SUCCESS) {
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_FETCH_DIM_R:
		case CREX_FETCH_DIM_IS:
		case CREX_FETCH_LIST_R:
			SKIP_IF_TOP(op1);
			SKIP_IF_TOP(op2);

			if (ct_eval_fetch_dim(&zv, op1, op2, (opline->opcode != CREX_FETCH_LIST_R)) == SUCCESS) {
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_ISSET_ISEMPTY_DIM_OBJ:
			SKIP_IF_TOP(op1);
			SKIP_IF_TOP(op2);

			if (ct_eval_isset_dim(&zv, opline->extended_value, op1, op2) == SUCCESS) {
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_FETCH_OBJ_R:
		case CREX_FETCH_OBJ_IS:
			if (op1) {
				SKIP_IF_TOP(op1);
				SKIP_IF_TOP(op2);

				if (ct_eval_fetch_obj(&zv, op1, op2) == SUCCESS) {
					SET_RESULT(result, &zv);
					zval_ptr_dtor_nogc(&zv);
					break;
				}
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_ISSET_ISEMPTY_PROP_OBJ:
			if (op1) {
				SKIP_IF_TOP(op1);
				SKIP_IF_TOP(op2);

				if (ct_eval_isset_obj(&zv, opline->extended_value, op1, op2) == SUCCESS) {
					SET_RESULT(result, &zv);
					zval_ptr_dtor_nogc(&zv);
					break;
				}
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_QM_ASSIGN:
		case CREX_JMP_SET:
		case CREX_COALESCE:
		case CREX_COPY_TMP:
			SET_RESULT(result, op1);
			break;
		case CREX_JMP_NULL:
			switch (opline->extended_value & CREX_SHORT_CIRCUITING_CHAIN_MASK) {
				case CREX_SHORT_CIRCUITING_CHAIN_EXPR:
					ZVAL_NULL(&zv);
					break;
				case CREX_SHORT_CIRCUITING_CHAIN_ISSET:
					ZVAL_FALSE(&zv);
					break;
				case CREX_SHORT_CIRCUITING_CHAIN_EMPTY:
					ZVAL_TRUE(&zv);
					break;
				EMPTY_SWITCH_DEFAULT_CASE()
			}
			SET_RESULT(result, &zv);
			break;
		case CREX_FETCH_CLASS:
			SET_RESULT(result, op2);
			break;
		case CREX_ISSET_ISEMPTY_CV:
			SKIP_IF_TOP(op1);
			if (ct_eval_isset_isempty(&zv, opline->extended_value, op1) == SUCCESS) {
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_TYPE_CHECK:
			SKIP_IF_TOP(op1);
			ct_eval_type_check(&zv, opline->extended_value, op1);
			SET_RESULT(result, &zv);
			zval_ptr_dtor_nogc(&zv);
			break;
		case CREX_INSTANCEOF:
			SKIP_IF_TOP(op1);
			ZVAL_FALSE(&zv);
			SET_RESULT(result, &zv);
			break;
		case CREX_ROPE_INIT:
			SKIP_IF_TOP(op2);
			if (IS_PARTIAL_ARRAY(op2)) {
				SET_RESULT_BOT(result);
				break;
			}
			if (crex_optimizer_eval_cast(&zv, IS_STRING, op2) == SUCCESS) {
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_ROPE_ADD:
		case CREX_ROPE_END:
			// TODO The way this is currently implemented will result in quadratic runtime
			// This is not necessary, the way the algorithm works it's okay to reuse the same
			// string for all SSA vars with some extra checks
			SKIP_IF_TOP(op1);
			SKIP_IF_TOP(op2);
			if (ct_eval_binary_op(&zv, CREX_CONCAT, op1, op2) == SUCCESS) {
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}
			SET_RESULT_BOT(result);
			break;
		case CREX_DO_ICALL:
		{
			crex_call_info *call;
			zval *name, *args[3] = {NULL};
			int i;

			if (!ctx->call_map) {
				SET_RESULT_BOT(result);
				break;
			}

			call = ctx->call_map[opline - ctx->scdf.op_array->opcodes];
			name = CT_CONSTANT_EX(ctx->scdf.op_array, call->caller_init_opline->op2.constant);

			/* We already know it can't be evaluated, don't bother checking again */
			if (ssa_op->result_def < 0 || IS_BOT(&ctx->values[ssa_op->result_def])) {
				break;
			}

			/* We're only interested in functions with up to three arguments right now.
			 * Note that named arguments with the argument in declaration order will still work. */
			if (call->num_args > 3 || call->send_unpack || call->is_prototype || call->named_args) {
				SET_RESULT_BOT(result);
				break;
			}

			for (i = 0; i < call->num_args; i++) {
				crex_op *opline = call->arg_info[i].opline;
				if (opline->opcode != CREX_SEND_VAL && opline->opcode != CREX_SEND_VAR) {
					SET_RESULT_BOT(result);
					return;
				}

				args[i] = get_op1_value(ctx, opline,
					&ctx->scdf.ssa->ops[opline - ctx->scdf.op_array->opcodes]);
				if (args[i]) {
					if (IS_BOT(args[i]) || IS_PARTIAL_ARRAY(args[i])) {
						SET_RESULT_BOT(result);
						return;
					} else if (IS_TOP(args[i])) {
						return;
					}
				}
			}

			/* We didn't get a BOT argument, so value stays the same */
			if (!IS_TOP(&ctx->values[ssa_op->result_def])) {
				break;
			}

			if (ct_eval_func_call(scdf->op_array, &zv, C_STR_P(name), call->num_args, args) == SUCCESS) {
				SET_RESULT(result, &zv);
				zval_ptr_dtor_nogc(&zv);
				break;
			}

#if 0
			/* sort out | uniq -c | sort -n */
			fprintf(stderr, "%s\n", C_STRVAL_P(name));
			/*if (args[1]) {
				crx_printf("%s %Z %Z\n", C_STRVAL_P(name), args[0], args[1]);
			} else {
				crx_printf("%s %Z\n", C_STRVAL_P(name), args[0]);
			}*/
#endif

			SET_RESULT_BOT(result);
			break;
		}
		default:
		{
			/* If we have no explicit implementation return BOT */
			SET_RESULT_BOT(result);
			SET_RESULT_BOT(op1);
			SET_RESULT_BOT(op2);
			break;
		}
	}
}

static zval *value_from_type_and_range(sccp_ctx *ctx, int var_num, zval *tmp) {
	crex_ssa *ssa = ctx->scdf.ssa;
	crex_ssa_var_info *info = &ssa->var_info[var_num];

	if (info->type & MAY_BE_UNDEF) {
		return NULL;
	}

	if (!(info->type & MAY_BE_ANY)) {
		/* This code must be unreachable. We could replace operands with NULL, but this doesn't
		 * really make things better. It would be better to later remove this code entirely. */
		return NULL;
	}

	if (!(info->type & ((MAY_BE_ANY|MAY_BE_UNDEF)-MAY_BE_NULL))) {
		if (ssa->vars[var_num].definition >= 0
		 && ctx->scdf.op_array->opcodes[ssa->vars[var_num].definition].opcode == CREX_VERIFY_RETURN_TYPE) {
			return NULL;
		}
		ZVAL_NULL(tmp);
		return tmp;
	}
	if (!(info->type & ((MAY_BE_ANY|MAY_BE_UNDEF)-MAY_BE_FALSE))) {
		if (ssa->vars[var_num].definition >= 0
		 && ctx->scdf.op_array->opcodes[ssa->vars[var_num].definition].opcode == CREX_VERIFY_RETURN_TYPE) {
			return NULL;
		}
		ZVAL_FALSE(tmp);
		return tmp;
	}
	if (!(info->type & ((MAY_BE_ANY|MAY_BE_UNDEF)-MAY_BE_TRUE))) {
		if (ssa->vars[var_num].definition >= 0
		 && ctx->scdf.op_array->opcodes[ssa->vars[var_num].definition].opcode == CREX_VERIFY_RETURN_TYPE) {
			return NULL;
		}
		ZVAL_TRUE(tmp);
		return tmp;
	}

	if (!(info->type & ((MAY_BE_ANY|MAY_BE_UNDEF)-MAY_BE_LONG))
			&& info->has_range
			&& !info->range.overflow && !info->range.underflow
			&& info->range.min == info->range.max) {
		ZVAL_LONG(tmp, info->range.min);
		return tmp;
	}

	return NULL;
}


/* Returns whether there is a successor */
static void sccp_mark_feasible_successors(
		scdf_ctx *scdf,
		int block_num, crex_basic_block *block,
		crex_op *opline, crex_ssa_op *ssa_op) {
	sccp_ctx *ctx = (sccp_ctx *) scdf;
	zval *op1, zv;
	int s;

	/* We can't determine the branch target at compile-time for these */
	switch (opline->opcode) {
		case CREX_ASSERT_CHECK:
		case CREX_CATCH:
		case CREX_FE_FETCH_R:
		case CREX_FE_FETCH_RW:
		case CREX_BIND_INIT_STATIC_OR_JMP:
			scdf_mark_edge_feasible(scdf, block_num, block->successors[0]);
			scdf_mark_edge_feasible(scdf, block_num, block->successors[1]);
			return;
	}

	op1 = get_op1_value(ctx, opline, ssa_op);
	if (IS_BOT(op1)) {
		CREX_ASSERT(ssa_op->op1_use >= 0);
		op1 = value_from_type_and_range(ctx, ssa_op->op1_use, &zv);
	}

	/* Branch target can be either one */
	if (!op1 || IS_BOT(op1)) {
		for (s = 0; s < block->successors_count; s++) {
			scdf_mark_edge_feasible(scdf, block_num, block->successors[s]);
		}
		return;
	}

	/* Branch target not yet known */
	if (IS_TOP(op1)) {
		return;
	}

	switch (opline->opcode) {
		case CREX_JMPZ:
		case CREX_JMPC_EX:
		{
			if (ct_eval_bool_cast(&zv, op1) == FAILURE) {
				scdf_mark_edge_feasible(scdf, block_num, block->successors[0]);
				scdf_mark_edge_feasible(scdf, block_num, block->successors[1]);
				return;
			}
			s = C_TYPE(zv) == IS_TRUE;
			break;
		}
		case CREX_JMPNZ:
		case CREX_JMPNC_EX:
		case CREX_JMP_SET:
		{
			if (ct_eval_bool_cast(&zv, op1) == FAILURE) {
				scdf_mark_edge_feasible(scdf, block_num, block->successors[0]);
				scdf_mark_edge_feasible(scdf, block_num, block->successors[1]);
				return;
			}
			s = C_TYPE(zv) == IS_FALSE;
			break;
		}
		case CREX_COALESCE:
			s = (C_TYPE_P(op1) == IS_NULL);
			break;
		case CREX_JMP_NULL:
			s = (C_TYPE_P(op1) != IS_NULL);
			break;
		case CREX_FE_RESET_R:
		case CREX_FE_RESET_RW:
			/* A non-empty partial array is definitely non-empty, but an
			 * empty partial array may be non-empty at runtime. */
			if (C_TYPE_P(op1) != IS_ARRAY ||
					(IS_PARTIAL_ARRAY(op1) && crex_hash_num_elements(C_ARR_P(op1)) == 0)) {
				scdf_mark_edge_feasible(scdf, block_num, block->successors[0]);
				scdf_mark_edge_feasible(scdf, block_num, block->successors[1]);
				return;
			}
			s = crex_hash_num_elements(C_ARR_P(op1)) != 0;
			break;
		case CREX_SWITCH_LONG:
		case CREX_SWITCH_STRING:
		case CREX_MATCH:
		{
			bool strict_comparison = opline->opcode == CREX_MATCH;
			uint8_t type = C_TYPE_P(op1);
			bool correct_type =
				(opline->opcode == CREX_SWITCH_LONG && type == IS_LONG)
				|| (opline->opcode == CREX_SWITCH_STRING && type == IS_STRING)
				|| (opline->opcode == CREX_MATCH && (type == IS_LONG || type == IS_STRING));

			if (correct_type) {
				crex_op_array *op_array = scdf->op_array;
				crex_ssa *ssa = scdf->ssa;
				HashTable *jmptable = C_ARRVAL_P(CT_CONSTANT_EX(op_array, opline->op2.constant));
				zval *jmp_zv = type == IS_LONG
					? crex_hash_index_find(jmptable, C_LVAL_P(op1))
					: crex_hash_find(jmptable, C_STR_P(op1));
				int target;

				if (jmp_zv) {
					target = ssa->cfg.map[CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, C_LVAL_P(jmp_zv))];
				} else {
					target = ssa->cfg.map[CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value)];
				}
				scdf_mark_edge_feasible(scdf, block_num, target);
				return;
			} else if (strict_comparison) {
				crex_op_array *op_array = scdf->op_array;
				crex_ssa *ssa = scdf->ssa;
				int target = ssa->cfg.map[CREX_OFFSET_TO_OPLINE_NUM(op_array, opline, opline->extended_value)];
				scdf_mark_edge_feasible(scdf, block_num, target);
				return;
			}
			s = block->successors_count - 1;
			break;
		}
		default:
			for (s = 0; s < block->successors_count; s++) {
				scdf_mark_edge_feasible(scdf, block_num, block->successors[s]);
			}
			return;
	}
	scdf_mark_edge_feasible(scdf, block_num, block->successors[s]);
}

static void join_hash_tables(HashTable *ret, HashTable *ht1, HashTable *ht2)
{
	crex_ulong index;
	crex_string *key;
	zval *val1, *val2;

	CREX_HASH_FOREACH_KEY_VAL(ht1, index, key, val1) {
		if (key) {
			val2 = crex_hash_find(ht2, key);
		} else {
			val2 = crex_hash_index_find(ht2, index);
		}
		if (val2 && crex_is_identical(val1, val2)) {
			if (key) {
				val1 = crex_hash_add_new(ret, key, val1);
			} else {
				val1 = crex_hash_index_add_new(ret, index, val1);
			}
			C_TRY_ADDREF_P(val1);
		}
	} CREX_HASH_FOREACH_END();
}

static crex_result join_partial_arrays(zval *a, zval *b)
{
	zval ret;

	if ((C_TYPE_P(a) != IS_ARRAY && !IS_PARTIAL_ARRAY(a))
			|| (C_TYPE_P(b) != IS_ARRAY && !IS_PARTIAL_ARRAY(b))) {
		return FAILURE;
	}

	empty_partial_array(&ret);
	join_hash_tables(C_ARRVAL(ret), C_ARRVAL_P(a), C_ARRVAL_P(b));
	zval_ptr_dtor_nogc(a);
	ZVAL_COPY_VALUE(a, &ret);

	return SUCCESS;
}

static crex_result join_partial_objects(zval *a, zval *b)
{
	zval ret;

	if (!IS_PARTIAL_OBJECT(a) || !IS_PARTIAL_OBJECT(b)) {
		return FAILURE;
	}

	empty_partial_object(&ret);
	join_hash_tables(C_ARRVAL(ret), C_ARRVAL_P(a), C_ARRVAL_P(b));
	zval_ptr_dtor_nogc(a);
	ZVAL_COPY_VALUE(a, &ret);

	return SUCCESS;
}

static void join_phi_values(zval *a, zval *b, bool escape) {
	if (IS_BOT(a) || IS_TOP(b)) {
		return;
	}
	if (IS_TOP(a)) {
		zval_ptr_dtor_nogc(a);
		ZVAL_COPY(a, b);
		return;
	}
	if (IS_BOT(b)) {
		zval_ptr_dtor_nogc(a);
		MAKE_BOT(a);
		return;
	}
	if (IS_PARTIAL_ARRAY(a) || IS_PARTIAL_ARRAY(b)) {
		if (join_partial_arrays(a, b) == FAILURE) {
			zval_ptr_dtor_nogc(a);
			MAKE_BOT(a);
		}
	} else if (IS_PARTIAL_OBJECT(a) || IS_PARTIAL_OBJECT(b)) {
		if (escape || join_partial_objects(a, b) == FAILURE) {
			zval_ptr_dtor_nogc(a);
			MAKE_BOT(a);
		}
	} else if (!crex_is_identical(a, b)) {
		if (join_partial_arrays(a, b) == FAILURE) {
			zval_ptr_dtor_nogc(a);
			MAKE_BOT(a);
		}
	}
}

static void sccp_visit_phi(scdf_ctx *scdf, crex_ssa_phi *phi) {
	sccp_ctx *ctx = (sccp_ctx *) scdf;
	crex_ssa *ssa = scdf->ssa;
	CREX_ASSERT(phi->ssa_var >= 0);
	if (!IS_BOT(&ctx->values[phi->ssa_var])) {
		crex_basic_block *block = &ssa->cfg.blocks[phi->block];
		int *predecessors = &ssa->cfg.predecessors[block->predecessor_offset];

		int i;
		zval result;
		MAKE_TOP(&result);
#if SCP_DEBUG
		fprintf(stderr, "Handling phi(");
#endif
		if (phi->pi >= 0) {
			CREX_ASSERT(phi->sources[0] >= 0);
			if (scdf_is_edge_feasible(scdf, phi->pi, phi->block)) {
				join_phi_values(&result, &ctx->values[phi->sources[0]], ssa->vars[phi->ssa_var].escape_state != ESCAPE_STATE_NO_ESCAPE);
			}
		} else {
			for (i = 0; i < block->predecessors_count; i++) {
				CREX_ASSERT(phi->sources[i] >= 0);
				if (scdf_is_edge_feasible(scdf, predecessors[i], phi->block)) {
#if SCP_DEBUG
					scp_dump_value(&ctx->values[phi->sources[i]]);
					fprintf(stderr, ",");
#endif
					join_phi_values(&result, &ctx->values[phi->sources[i]], ssa->vars[phi->ssa_var].escape_state != ESCAPE_STATE_NO_ESCAPE);
				} else {
#if SCP_DEBUG
					fprintf(stderr, " --,");
#endif
				}
			}
		}
#if SCP_DEBUG
		fprintf(stderr, ")\n");
#endif

		set_value(scdf, ctx, phi->ssa_var, &result);
		zval_ptr_dtor_nogc(&result);
	}
}

/* Call instruction -> remove opcodes that are part of the call */
static int remove_call(sccp_ctx *ctx, crex_op *opline, crex_ssa_op *ssa_op)
{
	crex_ssa *ssa = ctx->scdf.ssa;
	crex_op_array *op_array = ctx->scdf.op_array;
	crex_call_info *call;
	int i;

	CREX_ASSERT(ctx->call_map);
	call = ctx->call_map[opline - op_array->opcodes];
	CREX_ASSERT(call);
	CREX_ASSERT(call->caller_call_opline == opline);
	crex_ssa_remove_instr(ssa, opline, ssa_op);
	crex_ssa_remove_instr(ssa, call->caller_init_opline,
		&ssa->ops[call->caller_init_opline - op_array->opcodes]);

	for (i = 0; i < call->num_args; i++) {
		crex_ssa_remove_instr(ssa, call->arg_info[i].opline,
			&ssa->ops[call->arg_info[i].opline - op_array->opcodes]);
	}

	// TODO: remove call_info completely???
	call->callee_func = NULL;

	return call->num_args + 2;
}

/* This is a basic DCE pass we run after SCCP. It only works on those instructions those result
 * value(s) were determined by SCCP. It removes dead computational instructions and converts
 * CV-affecting instructions into CONST ASSIGNs. This basic DCE is performed for multiple reasons:
 * a) During operand replacement we eliminate FREEs. The corresponding computational instructions
 *    must be removed to avoid leaks. This way SCCP can run independently of the full DCE pass.
 * b) The main DCE pass relies on type analysis to determine whether instructions have side-effects
 *    and can't be DCEd. This means that it will not be able collect all instructions rendered dead
 *    by SCCP, because they may have potentially side-effecting types, but the actual values are
 *    not. As such doing DCE here will allow us to eliminate more dead code in combination.
 * c) The ordinary DCE pass cannot collect dead calls. However SCCP can result in dead calls, which
 *    we need to collect.
 * d) The ordinary DCE pass cannot collect construction of dead non-escaping arrays and objects.
 */
static int try_remove_definition(sccp_ctx *ctx, int var_num, crex_ssa_var *var, zval *value)
{
	crex_ssa *ssa = ctx->scdf.ssa;
	crex_op_array *op_array = ctx->scdf.op_array;
	int removed_ops = 0;

	if (var->definition >= 0) {
		crex_op *opline = &op_array->opcodes[var->definition];
		crex_ssa_op *ssa_op = &ssa->ops[var->definition];

		if (ssa_op->result_def == var_num) {
			if (opline->opcode == CREX_ASSIGN) {
				/* We can't drop the ASSIGN, but we can remove the result. */
				if (var->use_chain < 0 && var->phi_use_chain == NULL) {
					opline->result_type = IS_UNUSED;
					crex_ssa_remove_result_def(ssa, ssa_op);
				}
				return 0;
			}
			if (ssa_op->op1_def >= 0 || ssa_op->op2_def >= 0) {
				if (var->use_chain < 0 && var->phi_use_chain == NULL) {
					switch (opline->opcode) {
						case CREX_ASSIGN:
						case CREX_ASSIGN_REF:
						case CREX_ASSIGN_DIM:
						case CREX_ASSIGN_OBJ:
						case CREX_ASSIGN_OBJ_REF:
						case CREX_ASSIGN_STATIC_PROP:
						case CREX_ASSIGN_STATIC_PROP_REF:
						case CREX_ASSIGN_OP:
						case CREX_ASSIGN_DIM_OP:
						case CREX_ASSIGN_OBJ_OP:
						case CREX_ASSIGN_STATIC_PROP_OP:
						case CREX_PRE_INC:
						case CREX_PRE_DEC:
						case CREX_PRE_INC_OBJ:
						case CREX_PRE_DEC_OBJ:
						case CREX_DO_ICALL:
						case CREX_DO_UCALL:
						case CREX_DO_FCALL_BY_NAME:
						case CREX_DO_FCALL:
						case CREX_INCLUDE_OR_EVAL:
						case CREX_YIELD:
						case CREX_YIELD_FROM:
						case CREX_ASSERT_CHECK:
							opline->result_type = IS_UNUSED;
							crex_ssa_remove_result_def(ssa, ssa_op);
							break;
						default:
							break;
					}
				}
				/* we cannot remove instruction that defines other variables */
				return 0;
			} else if (opline->opcode == CREX_JMPC_EX
					|| opline->opcode == CREX_JMPNC_EX
					|| opline->opcode == CREX_JMP_SET
					|| opline->opcode == CREX_COALESCE
					|| opline->opcode == CREX_JMP_NULL
					|| opline->opcode == CREX_FE_RESET_R
					|| opline->opcode == CREX_FE_RESET_RW
					|| opline->opcode == CREX_FE_FETCH_R
					|| opline->opcode == CREX_FE_FETCH_RW
					|| opline->opcode == CREX_NEW) {
				/* we cannot simple remove jump instructions */
				return 0;
			} else if (var->use_chain >= 0
					|| var->phi_use_chain != NULL) {
				if (value
						&& (opline->result_type & (IS_VAR|IS_TMP_VAR))
						&& opline->opcode != CREX_QM_ASSIGN
						&& opline->opcode != CREX_FETCH_CLASS
						&& opline->opcode != CREX_ROPE_INIT
						&& opline->opcode != CREX_ROPE_ADD
						&& opline->opcode != CREX_INIT_ARRAY
						&& opline->opcode != CREX_ADD_ARRAY_ELEMENT
						&& opline->opcode != CREX_ADD_ARRAY_UNPACK) {
					/* Replace with QM_ASSIGN */
					uint8_t old_type = opline->result_type;
					uint32_t old_var = opline->result.var;

					ssa_op->result_def = -1;
					if (opline->opcode == CREX_DO_ICALL) {
						removed_ops = remove_call(ctx, opline, ssa_op) - 1;
					} else {
						crex_ssa_remove_instr(ssa, opline, ssa_op);
					}
					ssa_op->result_def = var_num;
					opline->opcode = CREX_QM_ASSIGN;
					opline->result_type = old_type;
					opline->result.var = old_var;
					C_TRY_ADDREF_P(value);
					crex_optimizer_update_op1_const(ctx->scdf.op_array, opline, value);
				}
				return 0;
			} else if ((opline->op2_type & (IS_VAR|IS_TMP_VAR))
					&& (!value_known(&ctx->values[ssa_op->op2_use])
						|| IS_PARTIAL_ARRAY(&ctx->values[ssa_op->op2_use])
						|| IS_PARTIAL_OBJECT(&ctx->values[ssa_op->op2_use]))) {
				return 0;
			} else if ((opline->op1_type & (IS_VAR|IS_TMP_VAR))
					&& (!value_known(&ctx->values[ssa_op->op1_use])
						|| IS_PARTIAL_ARRAY(&ctx->values[ssa_op->op1_use])
						|| IS_PARTIAL_OBJECT(&ctx->values[ssa_op->op1_use]))) {
				if (opline->opcode == CREX_TYPE_CHECK
				 || opline->opcode == CREX_BOOL) {
					crex_ssa_remove_result_def(ssa, ssa_op);
					/* For TYPE_CHECK we may compute the result value without knowing the
					 * operand, based on type inference information. Make sure the operand is
					 * freed and leave further cleanup to DCE. */
					opline->opcode = CREX_FREE;
					opline->result_type = IS_UNUSED;
					removed_ops++;
				} else {
					return 0;
				}
			} else {
				crex_ssa_remove_result_def(ssa, ssa_op);
				if (opline->opcode == CREX_DO_ICALL) {
					removed_ops = remove_call(ctx, opline, ssa_op);
				} else {
					crex_ssa_remove_instr(ssa, opline, ssa_op);
					removed_ops++;
				}
			}
		} else if (ssa_op->op1_def == var_num) {
			if (opline->opcode == CREX_ASSIGN) {
				/* Leave assigns to DCE (due to dtor effects) */
				return 0;
			}

			/* Compound assign or incdec -> convert to direct ASSIGN */

			if (!value) {
				/* In some cases crex_may_throw() may be avoided */
				switch (opline->opcode) {
					case CREX_ASSIGN_DIM:
					case CREX_ASSIGN_OBJ:
					case CREX_ASSIGN_OP:
					case CREX_ASSIGN_DIM_OP:
					case CREX_ASSIGN_OBJ_OP:
					case CREX_ASSIGN_STATIC_PROP_OP:
						if ((ssa_op->op2_use >= 0 && !value_known(&ctx->values[ssa_op->op2_use]))
								|| ((ssa_op+1)->op1_use >= 0 &&!value_known(&ctx->values[(ssa_op+1)->op1_use]))) {
							return 0;
						}
						break;
					case CREX_PRE_INC_OBJ:
					case CREX_PRE_DEC_OBJ:
					case CREX_POST_INC_OBJ:
					case CREX_POST_DEC_OBJ:
						if (ssa_op->op2_use >= 0 && !value_known(&ctx->values[ssa_op->op2_use])) {
							return 0;
						}
						break;
					case CREX_INIT_ARRAY:
					case CREX_ADD_ARRAY_ELEMENT:
						if (opline->op2_type == IS_UNUSED) {
							return 0;
						}
						/* break missing intentionally */
					default:
						if (crex_may_throw(opline, ssa_op, op_array, ssa)) {
							return 0;
						}
						break;
				}
			}

			/* Mark result unused, if possible */
			if (ssa_op->result_def >= 0) {
				if (ssa->vars[ssa_op->result_def].use_chain < 0
						&& ssa->vars[ssa_op->result_def].phi_use_chain == NULL) {
					crex_ssa_remove_result_def(ssa, ssa_op);
					opline->result_type = IS_UNUSED;
				} else if (opline->opcode != CREX_PRE_INC &&
						opline->opcode != CREX_PRE_DEC) {
					/* op1_def and result_def are different */
					return removed_ops;
				}
			}

			/* Destroy previous op2 */
			if (opline->op2_type == IS_CONST) {
				literal_dtor(&CREX_OP2_LITERAL(opline));
			} else if (ssa_op->op2_use >= 0) {
				if (ssa_op->op2_use != ssa_op->op1_use) {
					crex_ssa_unlink_use_chain(ssa, var->definition, ssa_op->op2_use);
				}
				ssa_op->op2_use = -1;
				ssa_op->op2_use_chain = -1;
			}

			/* Remove OP_DATA opcode */
			switch (opline->opcode) {
				case CREX_ASSIGN_DIM:
				case CREX_ASSIGN_OBJ:
					removed_ops++;
					crex_ssa_remove_instr(ssa, opline + 1, ssa_op + 1);
					break;
				case CREX_ASSIGN_DIM_OP:
				case CREX_ASSIGN_OBJ_OP:
				case CREX_ASSIGN_STATIC_PROP_OP:
					removed_ops++;
					crex_ssa_remove_instr(ssa, opline + 1, ssa_op + 1);
					break;
				default:
					break;
			}

			if (value) {
				/* Convert to ASSIGN */
				opline->opcode = CREX_ASSIGN;
				opline->op2_type = IS_CONST;
				opline->op2.constant = crex_optimizer_add_literal(op_array, value);
				C_TRY_ADDREF_P(value);
			} else {
				/* Remove dead array or object construction */
				removed_ops++;
				if (var->use_chain >= 0 || var->phi_use_chain != NULL) {
					crex_ssa_rename_var_uses(ssa, ssa_op->op1_def, ssa_op->op1_use, 1);
				}
				crex_ssa_remove_op1_def(ssa, ssa_op);
				crex_ssa_remove_instr(ssa, opline, ssa_op);
			}
		}
	} else if (var->definition_phi
			&& var->use_chain < 0
			&& var->phi_use_chain == NULL) {
		crex_ssa_remove_phi(ssa, var->definition_phi);
	}
	return removed_ops;
}

/* This will try to replace uses of SSA variables we have determined to be constant. Not all uses
 * can be replaced, because some instructions don't accept constant operands or only accept them
 * if they have a certain type. */
static int replace_constant_operands(sccp_ctx *ctx) {
	crex_ssa *ssa = ctx->scdf.ssa;
	crex_op_array *op_array = ctx->scdf.op_array;
	int i;
	zval tmp;
	int removed_ops = 0;

	/* We iterate the variables backwards, so we can eliminate sequences like INIT_ROPE
	 * and INIT_ARRAY. */
	for (i = ssa->vars_count - 1; i >= op_array->last_var; i--) {
		crex_ssa_var *var = &ssa->vars[i];
		zval *value;
		int use;

		if (IS_PARTIAL_ARRAY(&ctx->values[i])
				|| IS_PARTIAL_OBJECT(&ctx->values[i])) {
			if (!C_DELREF(ctx->values[i])) {
				crex_array_destroy(C_ARR(ctx->values[i]));
			}
			MAKE_BOT(&ctx->values[i]);
			if ((var->use_chain < 0 && var->phi_use_chain == NULL) || var->no_val) {
				removed_ops += try_remove_definition(ctx, i, var, NULL);
			}
			continue;
		} else if (value_known(&ctx->values[i])) {
			value = &ctx->values[i];
		} else {
			value = value_from_type_and_range(ctx, i, &tmp);
			if (!value) {
				continue;
			}
		}

		FOREACH_USE(var, use) {
			crex_op *opline = &op_array->opcodes[use];
			crex_ssa_op *ssa_op = &ssa->ops[use];
			if (try_replace_op1(ctx, opline, ssa_op, i, value)) {
				if (opline->opcode == CREX_NOP) {
					removed_ops++;
				}
				CREX_ASSERT(ssa_op->op1_def == -1);
				if (ssa_op->op1_use != ssa_op->op2_use) {
					crex_ssa_unlink_use_chain(ssa, use, ssa_op->op1_use);
				} else {
					ssa_op->op2_use_chain = ssa_op->op1_use_chain;
				}
				ssa_op->op1_use = -1;
				ssa_op->op1_use_chain = -1;
			}
			if (try_replace_op2(ctx, opline, ssa_op, i, value)) {
				CREX_ASSERT(ssa_op->op2_def == -1);
				if (ssa_op->op2_use != ssa_op->op1_use) {
					crex_ssa_unlink_use_chain(ssa, use, ssa_op->op2_use);
				}
				ssa_op->op2_use = -1;
				ssa_op->op2_use_chain = -1;
			}
		} FOREACH_USE_END();

		if (value_known(&ctx->values[i])) {
			removed_ops += try_remove_definition(ctx, i, var, value);
		}
	}

	return removed_ops;
}

static void sccp_context_init(crex_optimizer_ctx *ctx, sccp_ctx *sccp,
		crex_ssa *ssa, crex_op_array *op_array, crex_call_info **call_map) {
	int i;
	sccp->call_map = call_map;
	sccp->values = crex_arena_alloc(&ctx->arena, sizeof(zval) * ssa->vars_count);

	MAKE_TOP(&sccp->top);
	MAKE_BOT(&sccp->bot);

	i = 0;
	for (; i < op_array->last_var; ++i) {
		/* These are all undefined variables, which we have to mark BOT.
		 * Otherwise the undefined variable warning might not be preserved. */
		MAKE_BOT(&sccp->values[i]);
	}
	for (; i < ssa->vars_count; ++i) {
		if (ssa->vars[i].alias) {
			MAKE_BOT(&sccp->values[i]);
		} else {
			MAKE_TOP(&sccp->values[i]);
		}
	}
}

static void sccp_context_free(sccp_ctx *sccp) {
	int i;
	for (i = sccp->scdf.op_array->last_var; i < sccp->scdf.ssa->vars_count; ++i) {
		zval_ptr_dtor_nogc(&sccp->values[i]);
	}
}

int sccp_optimize_op_array(crex_optimizer_ctx *ctx, crex_op_array *op_array, crex_ssa *ssa, crex_call_info **call_map)
{
	sccp_ctx sccp;
	int removed_ops = 0;
	void *checkpoint = crex_arena_checkpoint(ctx->arena);

	sccp_context_init(ctx, &sccp, ssa, op_array, call_map);

	sccp.scdf.handlers.visit_instr = sccp_visit_instr;
	sccp.scdf.handlers.visit_phi = sccp_visit_phi;
	sccp.scdf.handlers.mark_feasible_successors = sccp_mark_feasible_successors;

	scdf_init(ctx, &sccp.scdf, op_array, ssa);
	scdf_solve(&sccp.scdf, "SCCP");

	if (ctx->debug_level & CREX_DUMP_SCCP) {
		int i, first = 1;

		for (i = op_array->last_var; i < ssa->vars_count; i++) {
			zval *zv = &sccp.values[i];

			if (IS_TOP(zv) || IS_BOT(zv)) {
				continue;
			}
			if (first) {
				first = 0;
				fprintf(stderr, "\nSCCP Values for \"");
				crex_dump_op_array_name(op_array);
				fprintf(stderr, "\":\n");
			}
			fprintf(stderr, "    #%d.", i);
			crex_dump_var(op_array, IS_CV, ssa->vars[i].var);
			fprintf(stderr, " =");
			scp_dump_value(zv);
			fprintf(stderr, "\n");
		}
	}

	removed_ops += scdf_remove_unreachable_blocks(&sccp.scdf);
	removed_ops += replace_constant_operands(&sccp);

	sccp_context_free(&sccp);
	crex_arena_release(&ctx->arena, checkpoint);

	return removed_ops;
}
