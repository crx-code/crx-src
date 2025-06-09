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

#ifndef CREX_INFERENCE_H
#define CREX_INFERENCE_H

#include "crex_optimizer.h"
#include "crex_ssa.h"
#include "crex_bitset.h"

/* Bitmask for type inference (crex_ssa_var_info.type) */
#include "crex_type_info.h"

#include <stdint.h>

#define MAY_BE_PACKED_GUARD         (1<<27) /* needs packed array guard */
#define MAY_BE_CLASS_GUARD          (1<<27) /* needs class guard */
#define MAY_BE_GUARD                (1<<28) /* needs type guard */

#define MAY_HAVE_DTOR \
	(MAY_BE_OBJECT|MAY_BE_RESOURCE \
	|MAY_BE_ARRAY_OF_ARRAY|MAY_BE_ARRAY_OF_OBJECT|MAY_BE_ARRAY_OF_RESOURCE)

#define DEFINE_SSA_OP_HAS_RANGE(opN) \
	static crex_always_inline bool _ssa_##opN##_has_range(const crex_op_array *op_array, const crex_ssa *ssa, const crex_op *opline, const crex_ssa_op *ssa_op) \
	{ \
		if (opline->opN##_type == IS_CONST) { \
			zval *zv = CRT_CONSTANT(opline->opN); \
			return (C_TYPE_P(zv) == IS_LONG); \
		} else { \
			return (opline->opN##_type != IS_UNUSED && \
		        ssa->var_info && \
		        ssa_op->opN##_use >= 0 && \
			    ssa->var_info[ssa_op->opN##_use].has_range); \
		} \
		return 0; \
	} \

#define DEFINE_SSA_OP_MIN_RANGE(opN) \
	static crex_always_inline crex_long _ssa_##opN##_min_range(const crex_op_array *op_array, const crex_ssa *ssa, const crex_op *opline, const crex_ssa_op *ssa_op) \
	{ \
		if (opline->opN##_type == IS_CONST) { \
			zval *zv = CRT_CONSTANT(opline->opN); \
			if (C_TYPE_P(zv) == IS_LONG) { \
				return C_LVAL_P(zv); \
			} \
		} else if (opline->opN##_type != IS_UNUSED && \
		    ssa->var_info && \
		    ssa_op->opN##_use >= 0 && \
		    ssa->var_info[ssa_op->opN##_use].has_range) { \
			return ssa->var_info[ssa_op->opN##_use].range.min; \
		} \
		return CREX_LONG_MIN; \
	} \

#define DEFINE_SSA_OP_MAX_RANGE(opN) \
	static crex_always_inline crex_long _ssa_##opN##_max_range(const crex_op_array *op_array, const crex_ssa *ssa, const crex_op *opline, const crex_ssa_op *ssa_op) \
	{ \
		if (opline->opN##_type == IS_CONST) { \
			zval *zv = CRT_CONSTANT(opline->opN); \
			if (C_TYPE_P(zv) == IS_LONG) { \
				return C_LVAL_P(zv); \
			} \
		} else if (opline->opN##_type != IS_UNUSED && \
		    ssa->var_info && \
		    ssa_op->opN##_use >= 0 && \
		    ssa->var_info[ssa_op->opN##_use].has_range) { \
			return ssa->var_info[ssa_op->opN##_use].range.max; \
		} \
		return CREX_LONG_MAX; \
	} \

#define DEFINE_SSA_OP_RANGE_UNDERFLOW(opN) \
	static crex_always_inline char _ssa_##opN##_range_underflow(const crex_op_array *op_array, const crex_ssa *ssa, const crex_op *opline, const crex_ssa_op *ssa_op) \
	{ \
		if (opline->opN##_type == IS_CONST) { \
			zval *zv = CRT_CONSTANT(opline->opN); \
			if (C_TYPE_P(zv) == IS_LONG) { \
				return 0; \
			} \
		} else if (opline->opN##_type != IS_UNUSED && \
		    ssa->var_info && \
		    ssa_op->opN##_use >= 0 && \
		    ssa->var_info[ssa_op->opN##_use].has_range) { \
			return ssa->var_info[ssa_op->opN##_use].range.underflow; \
		} \
		return 1; \
	} \

#define DEFINE_SSA_OP_RANGE_OVERFLOW(opN) \
	static crex_always_inline char _ssa_##opN##_range_overflow(const crex_op_array *op_array, const crex_ssa *ssa, const crex_op *opline, const crex_ssa_op *ssa_op) \
	{ \
		if (opline->opN##_type == IS_CONST) { \
			zval *zv = CRT_CONSTANT(opline->opN); \
			if (C_TYPE_P(zv) == IS_LONG) { \
				return 0; \
			} \
		} else if (opline->opN##_type != IS_UNUSED && \
		    ssa->var_info && \
		    ssa_op->opN##_use >= 0 && \
		    ssa->var_info[ssa_op->opN##_use].has_range) { \
			return ssa->var_info[ssa_op->opN##_use].range.overflow; \
		} \
		return 1; \
	} \

DEFINE_SSA_OP_HAS_RANGE(op1)
DEFINE_SSA_OP_MIN_RANGE(op1)
DEFINE_SSA_OP_MAX_RANGE(op1)
DEFINE_SSA_OP_RANGE_UNDERFLOW(op1)
DEFINE_SSA_OP_RANGE_OVERFLOW(op1)
DEFINE_SSA_OP_HAS_RANGE(op2)
DEFINE_SSA_OP_MIN_RANGE(op2)
DEFINE_SSA_OP_MAX_RANGE(op2)
DEFINE_SSA_OP_RANGE_UNDERFLOW(op2)
DEFINE_SSA_OP_RANGE_OVERFLOW(op2)

#define OP1_HAS_RANGE()       (_ssa_op1_has_range (op_array, ssa, opline, ssa_op))
#define OP1_MIN_RANGE()       (_ssa_op1_min_range (op_array, ssa, opline, ssa_op))
#define OP1_MAX_RANGE()       (_ssa_op1_max_range (op_array, ssa, opline, ssa_op))
#define OP1_RANGE_UNDERFLOW() (_ssa_op1_range_underflow (op_array, ssa, opline, ssa_op))
#define OP1_RANGE_OVERFLOW()  (_ssa_op1_range_overflow (op_array, ssa, opline, ssa_op))
#define OP2_HAS_RANGE()       (_ssa_op2_has_range (op_array, ssa, opline, ssa_op))
#define OP2_MIN_RANGE()       (_ssa_op2_min_range (op_array, ssa, opline, ssa_op))
#define OP2_MAX_RANGE()       (_ssa_op2_max_range (op_array, ssa, opline, ssa_op))
#define OP2_RANGE_UNDERFLOW() (_ssa_op2_range_underflow (op_array, ssa, opline, ssa_op))
#define OP2_RANGE_OVERFLOW()  (_ssa_op2_range_overflow (op_array, ssa, opline, ssa_op))

BEGIN_EXTERN_C()
CREX_API uint32_t CREX_FASTCALL crex_array_type_info(const zval *zv);
END_EXTERN_C()

static crex_always_inline uint32_t _const_op_type(const zval *zv) {
	if (C_TYPE_P(zv) == IS_CONSTANT_AST) {
		return MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY;
	} else if (C_TYPE_P(zv) == IS_ARRAY) {
		return crex_array_type_info(zv);
	} else {
		uint32_t tmp = (1 << C_TYPE_P(zv));

		if (C_REFCOUNTED_P(zv)) {
			tmp |= MAY_BE_RC1 | MAY_BE_RCN;
		} else if (C_TYPE_P(zv) == IS_STRING) {
			tmp |= MAY_BE_RCN;
		}
		return tmp;
	}
}

static crex_always_inline uint32_t get_ssa_var_info(const crex_ssa *ssa, int ssa_var_num)
{
	if (ssa->var_info && ssa_var_num >= 0) {
		return ssa->var_info[ssa_var_num].type;
	} else {
		return MAY_BE_UNDEF | MAY_BE_RC1 | MAY_BE_RCN | MAY_BE_REF | MAY_BE_INDIRECT | MAY_BE_ANY | MAY_BE_ARRAY_KEY_ANY | MAY_BE_ARRAY_OF_ANY | MAY_BE_ARRAY_OF_REF;
	}
}

#define DEFINE_SSA_OP_INFO(opN) \
	static crex_always_inline uint32_t _ssa_##opN##_info(const crex_op_array *op_array, const crex_ssa *ssa, const crex_op *opline, const crex_ssa_op *ssa_op) \
	{																		\
		if (opline->opN##_type == IS_CONST) {							\
			return _const_op_type(CRT_CONSTANT(opline->opN)); \
		} else { \
			return get_ssa_var_info(ssa, ssa->var_info ? ssa_op->opN##_use : -1); \
		} \
	} \

#define DEFINE_SSA_OP_DEF_INFO(opN) \
	static crex_always_inline uint32_t _ssa_##opN##_def_info(const crex_op_array *op_array, const crex_ssa *ssa, const crex_op *opline, const crex_ssa_op *ssa_op) \
	{ \
		return get_ssa_var_info(ssa, ssa->var_info ? ssa_op->opN##_def : -1); \
	} \


DEFINE_SSA_OP_INFO(op1)
DEFINE_SSA_OP_INFO(op2)
DEFINE_SSA_OP_INFO(result)
DEFINE_SSA_OP_DEF_INFO(op1)
DEFINE_SSA_OP_DEF_INFO(op2)
DEFINE_SSA_OP_DEF_INFO(result)

#define OP1_INFO()           (_ssa_op1_info(op_array, ssa, opline, ssa_op))
#define OP2_INFO()           (_ssa_op2_info(op_array, ssa, opline, ssa_op))
#define OP1_DATA_INFO()      (_ssa_op1_info(op_array, ssa, (opline+1), (ssa_op+1)))
#define OP2_DATA_INFO()      (_ssa_op2_info(op_array, ssa, (opline+1), (ssa_op+1)))
#define RES_USE_INFO()       (_ssa_result_info(op_array, ssa, opline, ssa_op))
#define OP1_DEF_INFO()       (_ssa_op1_def_info(op_array, ssa, opline, ssa_op))
#define OP2_DEF_INFO()       (_ssa_op2_def_info(op_array, ssa, opline, ssa_op))
#define OP1_DATA_DEF_INFO()  (_ssa_op1_def_info(op_array, ssa, (opline+1), (ssa_op+1)))
#define OP2_DATA_DEF_INFO()  (_ssa_op2_def_info(op_array, ssa, (opline+1), (ssa_op+1)))
#define RES_INFO()           (_ssa_result_def_info(op_array, ssa, opline, ssa_op))

static crex_always_inline bool crex_add_will_overflow(crex_long a, crex_long b) {
	return (b > 0 && a > CREX_LONG_MAX - b)
		|| (b < 0 && a < CREX_LONG_MIN - b);
}
static crex_always_inline bool crex_sub_will_overflow(crex_long a, crex_long b) {
	return (b > 0 && a < CREX_LONG_MIN + b)
		|| (b < 0 && a > CREX_LONG_MAX + b);
}

BEGIN_EXTERN_C()

CREX_API void crex_ssa_find_false_dependencies(const crex_op_array *op_array, crex_ssa *ssa);
CREX_API void crex_ssa_find_sccs(const crex_op_array *op_array, crex_ssa *ssa);
CREX_API crex_result crex_ssa_inference(crex_arena **raena, const crex_op_array *op_array, const crex_script *script, crex_ssa *ssa, crex_long optimization_level);

CREX_API uint32_t crex_array_element_type(uint32_t t1, uint8_t op_type, int write, int insert);

CREX_API bool crex_inference_propagate_range(const crex_op_array *op_array, const crex_ssa *ssa, const crex_op *opline, const crex_ssa_op* ssa_op, int var, crex_ssa_range *tmp);

CREX_API uint32_t crex_fetch_arg_info_type(
	const crex_script *script, const crex_arg_info *arg_info, crex_class_entry **pce);
CREX_API void crex_init_func_return_info(
	const crex_op_array *op_array, const crex_script *script, crex_ssa_var_info *ret);
uint32_t crex_get_return_info_from_signature_only(
		const crex_function *func, const crex_script *script,
		crex_class_entry **ce, bool *ce_is_instanceof, bool use_tentative_return_info);

CREX_API bool crex_may_throw_ex(const crex_op *opline, const crex_ssa_op *ssa_op, const crex_op_array *op_array, const crex_ssa *ssa, uint32_t t1, uint32_t t2);
CREX_API bool crex_may_throw(const crex_op *opline, const crex_ssa_op *ssa_op, const crex_op_array *op_array, const crex_ssa *ssa);

CREX_API crex_result crex_update_type_info(
	const crex_op_array *op_array, crex_ssa *ssa, const crex_script *script,
	crex_op *opline, crex_ssa_op *ssa_op, const crex_op **ssa_opcodes,
	crex_long optimization_level);

END_EXTERN_C()

#endif /* CREX_INFERENCE_H */
