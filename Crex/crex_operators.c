/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include <ctype.h>

#include "crex.h"
#include "crex_operators.h"
#include "crex_variables.h"
#include "crex_globals.h"
#include "crex_list.h"
#include "crex_API.h"
#include "crex_strtod.h"
#include "crex_exceptions.h"
#include "crex_closures.h"

#include <locale.h>
#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif

#ifdef CREX_INTRIN_AVX2_NATIVE
#include <immintrin.h>
#endif
#ifdef __SSE2__
#include <emmintrin.h>
#endif
#if defined(__aarch64__) || defined(_M_ARM64)
#include <arm_neon.h>
#endif

#if defined(CREX_WIN32) && !defined(ZTS) && defined(_MSC_VER)
/* This performance improvement of tolower() on Windows gives 10-18% on bench.crx */
#define CREX_USE_TOLOWER_L 1
#endif

#ifdef CREX_USE_TOLOWER_L
static _locale_t current_locale = NULL;
/* this is true global! may lead to strange effects on ZTS, but so may setlocale() */
#define crex_tolower(c) _tolower_l(c, current_locale)
#else
#define crex_tolower(c) tolower(c)
#endif

#define TYPE_PAIR(t1,t2) (((t1) << 4) | (t2))

#ifdef CREX_INTRIN_AVX2_NATIVE
#define HAVE_BLOCKCONV

#define BLOCKCONV_INIT_RANGE(start, end) \
	const __m256i blconv_offset = _mm256_set1_epi8((signed char)(SCHAR_MIN - start)); \
	const __m256i blconv_threshold = _mm256_set1_epi8(SCHAR_MIN + (end - start) + 1);

#define BLOCKCONV_STRIDE sizeof(__m256i)

#define BLOCKCONV_INIT_DELTA(delta) \
	const __m256i blconv_delta = _mm256_set1_epi8(delta);

#define BLOCKCONV_LOAD(input) \
	__m256i blconv_operand = _mm256_loadu_si256((__m256i*)(input)); \
	__m256i blconv_mask = _mm256_cmpgt_epi8(blconv_threshold, _mm256_add_epi8(blconv_operand, blconv_offset));

#define BLOCKCONV_FOUND() _mm256_movemask_epi8(blconv_mask)

#define BLOCKCONV_STORE(dest) \
	__m256i blconv_add = _mm256_and_si256(blconv_mask, blconv_delta); \
	__m256i blconv_result = _mm256_add_epi8(blconv_operand, blconv_add); \
	_mm256_storeu_si256((__m256i*)(dest), blconv_result);

#elif __SSE2__
#define HAVE_BLOCKCONV

/* Common code for SSE2 accelerated character case conversion */

#define BLOCKCONV_INIT_RANGE(start, end) \
	const __m128i blconv_offset = _mm_set1_epi8((signed char)(SCHAR_MIN - start)); \
	const __m128i blconv_threshold = _mm_set1_epi8(SCHAR_MIN + (end - start) + 1);

#define BLOCKCONV_STRIDE sizeof(__m128i)

#define BLOCKCONV_INIT_DELTA(delta) \
	const __m128i blconv_delta = _mm_set1_epi8(delta);

#define BLOCKCONV_LOAD(input) \
	__m128i blconv_operand = _mm_loadu_si128((__m128i*)(input)); \
	__m128i blconv_mask = _mm_cmplt_epi8(_mm_add_epi8(blconv_operand, blconv_offset), blconv_threshold);

#define BLOCKCONV_FOUND() _mm_movemask_epi8(blconv_mask)

#define BLOCKCONV_STORE(dest) \
	__m128i blconv_add = _mm_and_si128(blconv_mask, blconv_delta); \
	__m128i blconv_result = _mm_add_epi8(blconv_operand, blconv_add); \
	_mm_storeu_si128((__m128i *)(dest), blconv_result);

#elif defined(__aarch64__) || defined(_M_ARM64)
#define HAVE_BLOCKCONV

#define BLOCKCONV_INIT_RANGE(start, end) \
	const int8x16_t blconv_offset = vdupq_n_s8((signed char)(SCHAR_MIN - start)); \
	const int8x16_t blconv_threshold = vdupq_n_s8(SCHAR_MIN + (end - start) + 1);

#define BLOCKCONV_STRIDE sizeof(int8x16_t)

#define BLOCKCONV_INIT_DELTA(delta) \
	const int8x16_t blconv_delta = vdupq_n_s8(delta);

#define BLOCKCONV_LOAD(input) \
	int8x16_t blconv_operand = vld1q_s8((const int8_t*)(input)); \
	uint8x16_t blconv_mask = vcltq_s8(vaddq_s8(blconv_operand, blconv_offset), blconv_threshold);

#define BLOCKCONV_FOUND() vmaxvq_u8(blconv_mask)

#define BLOCKCONV_STORE(dest) \
	int8x16_t blconv_add = vandq_s8(vreinterpretq_s8_u8(blconv_mask), blconv_delta); \
	int8x16_t blconv_result = vaddq_s8(blconv_operand, blconv_add); \
	vst1q_s8((int8_t *)(dest), blconv_result);

#endif /* defined(__aarch64__) || defined(_M_ARM64) */

CREX_API const unsigned char crex_tolower_map[256] = {
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
0x40,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x5b,0x5c,0x5d,0x5e,0x5f,
0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};

CREX_API const unsigned char crex_toupper_map[256] = {
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
0x60,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x7b,0x7c,0x7d,0x7e,0x7f,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};


/**
 * Functions using locale lowercase:
 	 	crex_binary_strncasecmp_l
 	 	crex_binary_strcasecmp_l
 * Functions using ascii lowercase:
		string_compare_function_ex
		string_case_compare_function
  		crex_str_tolower_copy
		crex_str_tolower_dup
		crex_str_tolower
		crex_binary_strcasecmp
		crex_binary_strncasecmp
 */

static crex_long CREX_FASTCALL crex_atol_internal(const char *str, size_t str_len) /* {{{ */
{
	if (!str_len) {
		str_len = strlen(str);
	}

	/* Perform following multiplications on unsigned to avoid overflow UB.
	 * For now overflow is silently ignored -- not clear what else can be
	 * done here, especially as the final result of this function may be
	 * used in an unsigned context (e.g. "memory_limit=3G", which overflows
	 * crex_long on 32-bit, but not size_t). */
	crex_ulong retval = (crex_ulong) CREX_STRTOL(str, NULL, 0);
	if (str_len>0) {
		switch (str[str_len-1]) {
			case 'g':
			case 'G':
				retval *= 1024;
				CREX_FALLTHROUGH;
			case 'm':
			case 'M':
				retval *= 1024;
				CREX_FALLTHROUGH;
			case 'k':
			case 'K':
				retval *= 1024;
				break;
		}
	}
	return (crex_long) retval;
}
/* }}} */

CREX_API crex_long CREX_FASTCALL crex_atol(const char *str, size_t str_len)
{
	return crex_atol_internal(str, str_len);
}

CREX_API int CREX_FASTCALL crex_atoi(const char *str, size_t str_len)
{
	return (int) crex_atol_internal(str, str_len);
}

/* {{{ convert_object_to_type: dst will be either ctype or UNDEF */
#define convert_object_to_type(op, dst, ctype)									\
	ZVAL_UNDEF(dst);																		\
	if (C_OBJ_HT_P(op)->cast_object(C_OBJ_P(op), dst, ctype) == FAILURE) {					\
		crex_error(E_WARNING,																\
			"Object of class %s could not be converted to %s", ZSTR_VAL(C_OBJCE_P(op)->name),\
		crex_get_type_by_const(ctype));														\
	} 																						\

/* }}} */

CREX_API void CREX_FASTCALL convert_scalar_to_number(zval *op) /* {{{ */
{
try_again:
	switch (C_TYPE_P(op)) {
		case IS_REFERENCE:
			crex_unwrap_reference(op);
			goto try_again;
		case IS_STRING:
			{
				crex_string *str;

				str = C_STR_P(op);
				if ((C_TYPE_INFO_P(op)=is_numeric_string(ZSTR_VAL(str), ZSTR_LEN(str), &C_LVAL_P(op), &C_DVAL_P(op), 1)) == 0) {
					ZVAL_LONG(op, 0);
				}
				crex_string_release_ex(str, 0);
				break;
			}
		case IS_NULL:
		case IS_FALSE:
			ZVAL_LONG(op, 0);
			break;
		case IS_TRUE:
			ZVAL_LONG(op, 1);
			break;
		case IS_RESOURCE:
			{
				crex_long l = C_RES_HANDLE_P(op);
				zval_ptr_dtor(op);
				ZVAL_LONG(op, l);
			}
			break;
		case IS_OBJECT:
			{
				zval dst;

				convert_object_to_type(op, &dst, _IS_NUMBER);
				zval_ptr_dtor(op);

				if (C_TYPE(dst) == IS_LONG || C_TYPE(dst) == IS_DOUBLE) {
					ZVAL_COPY_VALUE(op, &dst);
				} else {
					ZVAL_LONG(op, 1);
				}
			}
			break;
	}
}
/* }}} */

static crex_never_inline zval* CREX_FASTCALL _crexi_convert_scalar_to_number_silent(zval *op, zval *holder) /* {{{ */
{
	switch (C_TYPE_P(op)) {
		case IS_NULL:
		case IS_FALSE:
			ZVAL_LONG(holder, 0);
			return holder;
		case IS_TRUE:
			ZVAL_LONG(holder, 1);
			return holder;
		case IS_STRING:
			if ((C_TYPE_INFO_P(holder) = is_numeric_string(C_STRVAL_P(op), C_STRLEN_P(op), &C_LVAL_P(holder), &C_DVAL_P(holder), 1)) == 0) {
				ZVAL_LONG(holder, 0);
			}
			return holder;
		case IS_RESOURCE:
			ZVAL_LONG(holder, C_RES_HANDLE_P(op));
			return holder;
		case IS_OBJECT:
			convert_object_to_type(op, holder, _IS_NUMBER);
			if (UNEXPECTED(EG(exception)) ||
			    UNEXPECTED(C_TYPE_P(holder) != IS_LONG && C_TYPE_P(holder) != IS_DOUBLE)) {
				ZVAL_LONG(holder, 1);
			}
			return holder;
		case IS_LONG:
		case IS_DOUBLE:
		default:
			return op;
	}
}
/* }}} */

static crex_never_inline crex_result CREX_FASTCALL _crexi_try_convert_scalar_to_number(zval *op, zval *holder) /* {{{ */
{
	switch (C_TYPE_P(op)) {
		case IS_NULL:
		case IS_FALSE:
			ZVAL_LONG(holder, 0);
			return SUCCESS;
		case IS_TRUE:
			ZVAL_LONG(holder, 1);
			return SUCCESS;
		case IS_STRING:
		{
			bool trailing_data = false;
			/* For BC reasons we allow errors so that we can warn on leading numeric string */
			if (0 == (C_TYPE_INFO_P(holder) = is_numeric_string_ex(C_STRVAL_P(op), C_STRLEN_P(op),
					&C_LVAL_P(holder), &C_DVAL_P(holder),  /* allow errors */ true, NULL, &trailing_data))) {
				/* Will lead to invalid OP type error */
				return FAILURE;
			}
			if (UNEXPECTED(trailing_data)) {
				crex_error(E_WARNING, "A non-numeric value encountered");
				if (UNEXPECTED(EG(exception))) {
					return FAILURE;
				}
			}
			return SUCCESS;
		}
		case IS_OBJECT:
			if (C_OBJ_HT_P(op)->cast_object(C_OBJ_P(op), holder, _IS_NUMBER) == FAILURE
					|| EG(exception)) {
				return FAILURE;
			}
			CREX_ASSERT(C_TYPE_P(holder) == IS_LONG || C_TYPE_P(holder) == IS_DOUBLE);
			return SUCCESS;
		case IS_RESOURCE:
		case IS_ARRAY:
			return FAILURE;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}
/* }}} */

static crex_always_inline crex_result crexi_try_convert_scalar_to_number(zval *op, zval *holder) /* {{{ */
{
	if (C_TYPE_P(op) == IS_LONG || C_TYPE_P(op) == IS_DOUBLE) {
		ZVAL_COPY_VALUE(holder, op);
		return SUCCESS;
	} else {
		return _crexi_try_convert_scalar_to_number(op, holder);
	}
}
/* }}} */

static crex_never_inline crex_long CREX_FASTCALL crexi_try_get_long(const zval *op, bool *failed) /* {{{ */
{
	*failed = 0;
	switch (C_TYPE_P(op)) {
		case IS_NULL:
		case IS_FALSE:
			return 0;
		case IS_TRUE:
			return 1;
		case IS_DOUBLE: {
			double dval = C_DVAL_P(op);
			crex_long lval = crex_dval_to_lval(dval);
			if (!crex_is_long_compatible(dval, lval)) {
				crex_incompatible_double_to_long_error(dval);
				if (UNEXPECTED(EG(exception))) {
					*failed = 1;
				}
			}
			return lval;
		}
		case IS_STRING:
			{
				uint8_t type;
				crex_long lval;
				double dval;
				bool trailing_data = false;

				/* For BC reasons we allow errors so that we can warn on leading numeric string */
				type = is_numeric_string_ex(C_STRVAL_P(op), C_STRLEN_P(op), &lval, &dval,
					/* allow errors */ true, NULL, &trailing_data);
				if (type == 0) {
					*failed = 1;
					return 0;
				}
				if (UNEXPECTED(trailing_data)) {
					crex_error(E_WARNING, "A non-numeric value encountered");
					if (UNEXPECTED(EG(exception))) {
						*failed = 1;
					}
				}
				if (EXPECTED(type == IS_LONG)) {
					return lval;
				} else {
					/* Previously we used strtol here, not is_numeric_string,
					 * and strtol gives you LONG_MAX/_MIN on overflow.
					 * We use use saturating conversion to emulate strtol()'s
					 * behaviour.
					 */
					lval = crex_dval_to_lval_cap(dval);
					if (!crex_is_long_compatible(dval, lval)) {
						crex_incompatible_string_to_long_error(C_STR_P(op));
						if (UNEXPECTED(EG(exception))) {
							*failed = 1;
						}
					}
					return lval;
				}
			}
		case IS_OBJECT:
			{
				zval dst;
				if (C_OBJ_HT_P(op)->cast_object(C_OBJ_P(op), &dst, IS_LONG) == FAILURE
						|| EG(exception)) {
					*failed = 1;
					return 0;
				}
				CREX_ASSERT(C_TYPE(dst) == IS_LONG);
				return C_LVAL(dst);
			}
		case IS_RESOURCE:
		case IS_ARRAY:
			*failed = 1;
			return 0;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}
/* }}} */

CREX_API crex_long CREX_FASTCALL zval_try_get_long(const zval *op, bool *failed)
{
	if (EXPECTED(C_TYPE_P(op) == IS_LONG)) {
		*failed = false;
		return C_LVAL_P(op);
	}
	return crexi_try_get_long(op, failed);
}

#define CREX_TRY_BINARY_OP1_OBJECT_OPERATION(opcode) \
	if (UNEXPECTED(C_TYPE_P(op1) == IS_OBJECT) \
		&& UNEXPECTED(C_OBJ_HANDLER_P(op1, do_operation))) { \
		if (EXPECTED(SUCCESS == C_OBJ_HANDLER_P(op1, do_operation)(opcode, result, op1, op2))) { \
			return SUCCESS; \
		} \
	}

#define CREX_TRY_BINARY_OP2_OBJECT_OPERATION(opcode) \
	if (UNEXPECTED(C_TYPE_P(op2) == IS_OBJECT) \
		&& UNEXPECTED(C_OBJ_HANDLER_P(op2, do_operation)) \
		&& EXPECTED(SUCCESS == C_OBJ_HANDLER_P(op2, do_operation)(opcode, result, op1, op2))) { \
		return SUCCESS; \
	}

#define CREX_TRY_BINARY_OBJECT_OPERATION(opcode) \
	CREX_TRY_BINARY_OP1_OBJECT_OPERATION(opcode) \
	else \
	CREX_TRY_BINARY_OP2_OBJECT_OPERATION(opcode)

#define CREX_TRY_UNARY_OBJECT_OPERATION(opcode) \
	if (UNEXPECTED(C_TYPE_P(op1) == IS_OBJECT) \
		&& UNEXPECTED(C_OBJ_HANDLER_P(op1, do_operation)) \
		&& EXPECTED(SUCCESS == C_OBJ_HANDLER_P(op1, do_operation)(opcode, result, op1, NULL))) { \
		return SUCCESS; \
	}

#define convert_op1_op2_long(op1, op1_lval, op2, op2_lval, result, opcode, sigil) \
	do {																\
		if (UNEXPECTED(C_TYPE_P(op1) != IS_LONG)) {						\
			bool failed;											\
			if (C_ISREF_P(op1)) {										\
				op1 = C_REFVAL_P(op1);									\
				if (C_TYPE_P(op1) == IS_LONG) {							\
					op1_lval = C_LVAL_P(op1);							\
					break;												\
				}														\
			}															\
			CREX_TRY_BINARY_OP1_OBJECT_OPERATION(opcode);				\
			op1_lval = crexi_try_get_long(op1, &failed);				\
			if (UNEXPECTED(failed)) {									\
				crex_binop_error(sigil, op1, op2);						\
				if (result != op1) {									\
					ZVAL_UNDEF(result);									\
				}														\
				return FAILURE;											\
			}															\
		} else {														\
			op1_lval = C_LVAL_P(op1);									\
		}																\
	} while (0);														\
	do {																\
		if (UNEXPECTED(C_TYPE_P(op2) != IS_LONG)) {						\
			bool failed;											\
			if (C_ISREF_P(op2)) {										\
				op2 = C_REFVAL_P(op2);									\
				if (C_TYPE_P(op2) == IS_LONG) {							\
					op2_lval = C_LVAL_P(op2);							\
					break;												\
				}														\
			}															\
			CREX_TRY_BINARY_OP2_OBJECT_OPERATION(opcode);				\
			op2_lval = crexi_try_get_long(op2, &failed);				\
			if (UNEXPECTED(failed)) {									\
				crex_binop_error(sigil, op1, op2);						\
				if (result != op1) {									\
					ZVAL_UNDEF(result);									\
				}														\
				return FAILURE;											\
			}															\
		} else {														\
			op2_lval = C_LVAL_P(op2);									\
		}																\
	} while (0);

CREX_API void CREX_FASTCALL convert_to_long(zval *op) /* {{{ */
{
	crex_long tmp;

try_again:
	switch (C_TYPE_P(op)) {
		case IS_NULL:
		case IS_FALSE:
			ZVAL_LONG(op, 0);
			break;
		case IS_TRUE:
			ZVAL_LONG(op, 1);
			break;
		case IS_RESOURCE:
			tmp = C_RES_HANDLE_P(op);
			zval_ptr_dtor(op);
			ZVAL_LONG(op, tmp);
			break;
		case IS_LONG:
			break;
		case IS_DOUBLE:
			ZVAL_LONG(op, crex_dval_to_lval(C_DVAL_P(op)));
			break;
		case IS_STRING:
			{
				crex_string *str = C_STR_P(op);
				ZVAL_LONG(op, zval_get_long(op));
				crex_string_release_ex(str, 0);
			}
			break;
		case IS_ARRAY:
			tmp = (crex_hash_num_elements(C_ARRVAL_P(op))?1:0);
			zval_ptr_dtor(op);
			ZVAL_LONG(op, tmp);
			break;
		case IS_OBJECT:
			{
				zval dst;

				convert_object_to_type(op, &dst, IS_LONG);
				zval_ptr_dtor(op);

				if (C_TYPE(dst) == IS_LONG) {
					ZVAL_LONG(op, C_LVAL(dst));
				} else {
					ZVAL_LONG(op, 1);
				}
				return;
			}
		case IS_REFERENCE:
			crex_unwrap_reference(op);
			goto try_again;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}
/* }}} */

CREX_API void CREX_FASTCALL convert_to_double(zval *op) /* {{{ */
{
	double tmp;

try_again:
	switch (C_TYPE_P(op)) {
		case IS_NULL:
		case IS_FALSE:
			ZVAL_DOUBLE(op, 0.0);
			break;
		case IS_TRUE:
			ZVAL_DOUBLE(op, 1.0);
			break;
		case IS_RESOURCE: {
				double d = (double) C_RES_HANDLE_P(op);
				zval_ptr_dtor(op);
				ZVAL_DOUBLE(op, d);
			}
			break;
		case IS_LONG:
			ZVAL_DOUBLE(op, (double) C_LVAL_P(op));
			break;
		case IS_DOUBLE:
			break;
		case IS_STRING:
			{
				crex_string *str = C_STR_P(op);

				ZVAL_DOUBLE(op, crex_strtod(ZSTR_VAL(str), NULL));
				crex_string_release_ex(str, 0);
			}
			break;
		case IS_ARRAY:
			tmp = (crex_hash_num_elements(C_ARRVAL_P(op))?1:0);
			zval_ptr_dtor(op);
			ZVAL_DOUBLE(op, tmp);
			break;
		case IS_OBJECT:
			{
				zval dst;

				convert_object_to_type(op, &dst, IS_DOUBLE);
				zval_ptr_dtor(op);

				if (C_TYPE(dst) == IS_DOUBLE) {
					ZVAL_DOUBLE(op, C_DVAL(dst));
				} else {
					ZVAL_DOUBLE(op, 1.0);
				}
				break;
			}
		case IS_REFERENCE:
			crex_unwrap_reference(op);
			goto try_again;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}
/* }}} */

CREX_API void CREX_FASTCALL convert_to_null(zval *op) /* {{{ */
{
	zval_ptr_dtor(op);
	ZVAL_NULL(op);
}
/* }}} */

CREX_API void CREX_FASTCALL convert_to_boolean(zval *op) /* {{{ */
{
	bool tmp;

try_again:
	switch (C_TYPE_P(op)) {
		case IS_FALSE:
		case IS_TRUE:
			break;
		case IS_NULL:
			ZVAL_FALSE(op);
			break;
		case IS_RESOURCE: {
				crex_long l = (C_RES_HANDLE_P(op) ? 1 : 0);

				zval_ptr_dtor(op);
				ZVAL_BOOL(op, l);
			}
			break;
		case IS_LONG:
			ZVAL_BOOL(op, C_LVAL_P(op) ? 1 : 0);
			break;
		case IS_DOUBLE:
			ZVAL_BOOL(op, C_DVAL_P(op) ? 1 : 0);
			break;
		case IS_STRING:
			{
				crex_string *str = C_STR_P(op);

				if (ZSTR_LEN(str) == 0
					|| (ZSTR_LEN(str) == 1 && ZSTR_VAL(str)[0] == '0')) {
					ZVAL_FALSE(op);
				} else {
					ZVAL_TRUE(op);
				}
				crex_string_release_ex(str, 0);
			}
			break;
		case IS_ARRAY:
			tmp = (crex_hash_num_elements(C_ARRVAL_P(op))?1:0);
			zval_ptr_dtor(op);
			ZVAL_BOOL(op, tmp);
			break;
		case IS_OBJECT:
			{
				zval dst;

				convert_object_to_type(op, &dst, _IS_BOOL);
				zval_ptr_dtor(op);

				if (C_TYPE_INFO(dst) == IS_FALSE || C_TYPE_INFO(dst) == IS_TRUE) {
					C_TYPE_INFO_P(op) = C_TYPE_INFO(dst);
				} else {
					ZVAL_TRUE(op);
				}
				break;
			}
		case IS_REFERENCE:
			crex_unwrap_reference(op);
			goto try_again;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}
/* }}} */

CREX_API void CREX_FASTCALL _convert_to_string(zval *op) /* {{{ */
{
try_again:
	switch (C_TYPE_P(op)) {
		case IS_UNDEF:
		case IS_NULL:
		case IS_FALSE: {
			ZVAL_EMPTY_STRING(op);
			break;
		}
		case IS_TRUE:
			ZVAL_CHAR(op, '1');
			break;
		case IS_STRING:
			break;
		case IS_RESOURCE: {
			crex_string *str = crex_strpprintf(0, "Resource id #" CREX_LONG_FMT, (crex_long)C_RES_HANDLE_P(op));
			zval_ptr_dtor(op);
			ZVAL_NEW_STR(op, str);
			break;
		}
		case IS_LONG:
			ZVAL_STR(op, crex_long_to_str(C_LVAL_P(op)));
			break;
		case IS_DOUBLE:
			ZVAL_NEW_STR(op, crex_double_to_str(C_DVAL_P(op)));
			break;
		case IS_ARRAY:
			crex_error(E_WARNING, "Array to string conversion");
			zval_ptr_dtor(op);
			ZVAL_INTERNED_STR(op, ZSTR_KNOWN(CREX_STR_ARRAY_CAPITALIZED));
			break;
		case IS_OBJECT: {
			zval tmp;
			if (C_OBJ_HT_P(op)->cast_object(C_OBJ_P(op), &tmp, IS_STRING) == SUCCESS) {
				zval_ptr_dtor(op);
				ZVAL_COPY_VALUE(op, &tmp);
				return;
			}
			if (!EG(exception)) {
				crex_throw_error(NULL, "Object of class %s could not be converted to string", ZSTR_VAL(C_OBJCE_P(op)->name));
			}
			zval_ptr_dtor(op);
			ZVAL_EMPTY_STRING(op);
			break;
		}
		case IS_REFERENCE:
			crex_unwrap_reference(op);
			goto try_again;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}
/* }}} */

CREX_API bool CREX_FASTCALL _try_convert_to_string(zval *op) /* {{{ */
{
	crex_string *str;

	CREX_ASSERT(C_TYPE_P(op) != IS_STRING);
	str = zval_try_get_string_func(op);
	if (UNEXPECTED(!str)) {
		return 0;
	}
	zval_ptr_dtor(op);
	ZVAL_STR(op, str);
	return 1;
}
/* }}} */

static void convert_scalar_to_array(zval *op) /* {{{ */
{
	HashTable *ht = crex_new_array(1);
	crex_hash_index_add_new(ht, 0, op);
	ZVAL_ARR(op, ht);
}
/* }}} */

CREX_API void CREX_FASTCALL convert_to_array(zval *op) /* {{{ */
{
try_again:
	switch (C_TYPE_P(op)) {
		case IS_ARRAY:
			break;
/* OBJECTS_OPTIMIZE */
		case IS_OBJECT:
			if (C_OBJCE_P(op) == crex_ce_closure) {
				convert_scalar_to_array(op);
			} else if (C_OBJ_P(op)->properties == NULL
			 && C_OBJ_HT_P(op)->get_properties_for == NULL
			 && C_OBJ_HT_P(op)->get_properties == crex_std_get_properties) {
				/* Optimized version without rebuilding properties HashTable */
				HashTable *ht = crex_std_build_object_properties_array(C_OBJ_P(op));
				OBJ_RELEASE(C_OBJ_P(op));
				ZVAL_ARR(op, ht);
			} else {
				HashTable *obj_ht = crex_get_properties_for(op, CREX_PROP_PURPOSE_ARRAY_CAST);
				if (obj_ht) {
					HashTable *new_obj_ht = crex_proptable_to_symtable(obj_ht,
						(C_OBJCE_P(op)->default_properties_count ||
						 C_OBJ_P(op)->handlers != &std_object_handlers ||
						 GC_IS_RECURSIVE(obj_ht)));
					zval_ptr_dtor(op);
					ZVAL_ARR(op, new_obj_ht);
					crex_release_properties(obj_ht);
				} else {
					zval_ptr_dtor(op);
					/*ZVAL_EMPTY_ARRAY(op);*/
					array_init(op);
				}
			}
			break;
		case IS_NULL:
			/*ZVAL_EMPTY_ARRAY(op);*/
			array_init(op);
			break;
		case IS_REFERENCE:
			crex_unwrap_reference(op);
			goto try_again;
		default:
			convert_scalar_to_array(op);
			break;
	}
}
/* }}} */

CREX_API void CREX_FASTCALL convert_to_object(zval *op) /* {{{ */
{
try_again:
	switch (C_TYPE_P(op)) {
		case IS_ARRAY:
			{
				HashTable *ht = crex_symtable_to_proptable(C_ARR_P(op));
				crex_object *obj;

				if (GC_FLAGS(ht) & IS_ARRAY_IMMUTABLE) {
					/* TODO: try not to duplicate immutable arrays as well ??? */
					ht = crex_array_dup(ht);
				} else if (ht != C_ARR_P(op)) {
					zval_ptr_dtor(op);
				} else {
					GC_DELREF(ht);
				}
				obj = crex_objects_new(crex_standard_class_def);
				obj->properties = ht;
				ZVAL_OBJ(op, obj);
				break;
			}
		case IS_OBJECT:
			break;
		case IS_NULL:
			object_init(op);
			break;
		case IS_REFERENCE:
			crex_unwrap_reference(op);
			goto try_again;
		default: {
			zval tmp;
			ZVAL_COPY_VALUE(&tmp, op);
			object_init(op);
			crex_hash_add_new(C_OBJPROP_P(op), ZSTR_KNOWN(CREX_STR_SCALAR), &tmp);
			break;
		}
	}
}
/* }}} */

CREX_API void CREX_COLD crex_incompatible_double_to_long_error(double d)
{
	crex_error_unchecked(E_DEPRECATED, "Implicit conversion from float %.*H to int loses precision", -1, d);
}
CREX_API void CREX_COLD crex_incompatible_string_to_long_error(const crex_string *s)
{
	crex_error(E_DEPRECATED, "Implicit conversion from float-string \"%s\" to int loses precision", ZSTR_VAL(s));
}

CREX_API crex_long CREX_FASTCALL zval_get_long_func(const zval *op, bool is_strict) /* {{{ */
{
try_again:
	switch (C_TYPE_P(op)) {
		case IS_UNDEF:
		case IS_NULL:
		case IS_FALSE:
			return 0;
		case IS_TRUE:
			return 1;
		case IS_RESOURCE:
			return C_RES_HANDLE_P(op);
		case IS_LONG:
			return C_LVAL_P(op);
		case IS_DOUBLE: {
			double dval = C_DVAL_P(op);
			crex_long lval = crex_dval_to_lval(dval);
			if (UNEXPECTED(is_strict)) {
				if (!crex_is_long_compatible(dval, lval)) {
					crex_incompatible_double_to_long_error(dval);
				}
			}
			return lval;
		}
		case IS_STRING:
			{
				uint8_t type;
				crex_long lval;
				double dval;
				if (0 == (type = is_numeric_string(C_STRVAL_P(op), C_STRLEN_P(op), &lval, &dval, true))) {
					return 0;
				} else if (EXPECTED(type == IS_LONG)) {
					return lval;
				} else {
					/* Previously we used strtol here, not is_numeric_string,
					 * and strtol gives you LONG_MAX/_MIN on overflow.
					 * We use saturating conversion to emulate strtol()'s
					 * behaviour.
					 */
					 /* Most usages are expected to not be (int) casts */
					lval = crex_dval_to_lval_cap(dval);
					if (UNEXPECTED(is_strict)) {
						if (!crex_is_long_compatible(dval, lval)) {
							crex_incompatible_string_to_long_error(C_STR_P(op));
						}
					}
					return lval;
				}
			}
		case IS_ARRAY:
			return crex_hash_num_elements(C_ARRVAL_P(op)) ? 1 : 0;
		case IS_OBJECT:
			{
				zval dst;
				convert_object_to_type(op, &dst, IS_LONG);
				if (C_TYPE(dst) == IS_LONG) {
					return C_LVAL(dst);
				} else {
					return 1;
				}
			}
		case IS_REFERENCE:
			op = C_REFVAL_P(op);
			goto try_again;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
	return 0;
}
/* }}} */

CREX_API double CREX_FASTCALL zval_get_double_func(const zval *op) /* {{{ */
{
try_again:
	switch (C_TYPE_P(op)) {
		case IS_NULL:
		case IS_FALSE:
			return 0.0;
		case IS_TRUE:
			return 1.0;
		case IS_RESOURCE:
			return (double) C_RES_HANDLE_P(op);
		case IS_LONG:
			return (double) C_LVAL_P(op);
		case IS_DOUBLE:
			return C_DVAL_P(op);
		case IS_STRING:
			return crex_strtod(C_STRVAL_P(op), NULL);
		case IS_ARRAY:
			return crex_hash_num_elements(C_ARRVAL_P(op)) ? 1.0 : 0.0;
		case IS_OBJECT:
			{
				zval dst;
				convert_object_to_type(op, &dst, IS_DOUBLE);

				if (C_TYPE(dst) == IS_DOUBLE) {
					return C_DVAL(dst);
				} else {
					return 1.0;
				}
			}
		case IS_REFERENCE:
			op = C_REFVAL_P(op);
			goto try_again;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
	return 0.0;
}
/* }}} */

static crex_always_inline crex_string* __zval_get_string_func(zval *op, bool try) /* {{{ */
{
try_again:
	switch (C_TYPE_P(op)) {
		case IS_UNDEF:
		case IS_NULL:
		case IS_FALSE:
			return ZSTR_EMPTY_ALLOC();
		case IS_TRUE:
			return ZSTR_CHAR('1');
		case IS_RESOURCE:
			return crex_strpprintf(0, "Resource id #" CREX_LONG_FMT, (crex_long)C_RES_HANDLE_P(op));
		case IS_LONG:
			return crex_long_to_str(C_LVAL_P(op));
		case IS_DOUBLE:
			return crex_double_to_str(C_DVAL_P(op));
		case IS_ARRAY:
			crex_error(E_WARNING, "Array to string conversion");
			return (try && UNEXPECTED(EG(exception))) ?
				NULL : ZSTR_KNOWN(CREX_STR_ARRAY_CAPITALIZED);
		case IS_OBJECT: {
			zval tmp;
			if (C_OBJ_HT_P(op)->cast_object(C_OBJ_P(op), &tmp, IS_STRING) == SUCCESS) {
				return C_STR(tmp);
			}
			if (!EG(exception)) {
				crex_throw_error(NULL, "Object of class %s could not be converted to string", ZSTR_VAL(C_OBJCE_P(op)->name));
			}
			return try ? NULL : ZSTR_EMPTY_ALLOC();
		}
		case IS_REFERENCE:
			op = C_REFVAL_P(op);
			goto try_again;
		case IS_STRING:
			return crex_string_copy(C_STR_P(op));
		EMPTY_SWITCH_DEFAULT_CASE()
	}
	return NULL;
}
/* }}} */

CREX_API crex_string* CREX_FASTCALL zval_get_string_func(zval *op) /* {{{ */
{
	return __zval_get_string_func(op, 0);
}
/* }}} */

CREX_API crex_string* CREX_FASTCALL zval_try_get_string_func(zval *op) /* {{{ */
{
	return __zval_get_string_func(op, 1);
}
/* }}} */

static CREX_COLD crex_never_inline void CREX_FASTCALL crex_binop_error(const char *operator, zval *op1, zval *op2) /* {{{ */ {
	if (EG(exception)) {
		return;
	}

	crex_type_error("Unsupported operand types: %s %s %s",
		crex_zval_type_name(op1), operator, crex_zval_type_name(op2));
}
/* }}} */

static crex_never_inline void CREX_FASTCALL add_function_array(zval *result, zval *op1, zval *op2) /* {{{ */
{
	if (result == op1 && C_ARR_P(op1) == C_ARR_P(op2)) {
		/* $a += $a */
		return;
	}
	if (result != op1) {
		ZVAL_ARR(result, crex_array_dup(C_ARR_P(op1)));
	} else {
		SEPARATE_ARRAY(result);
	}
	crex_hash_merge(C_ARRVAL_P(result), C_ARRVAL_P(op2), zval_add_ref, 0);
}
/* }}} */

static crex_always_inline crex_result add_function_fast(zval *result, zval *op1, zval *op2) /* {{{ */
{
	uint8_t type_pair = TYPE_PAIR(C_TYPE_P(op1), C_TYPE_P(op2));

	if (EXPECTED(type_pair == TYPE_PAIR(IS_LONG, IS_LONG))) {
		fast_long_add_function(result, op1, op2);
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_DOUBLE, IS_DOUBLE))) {
		ZVAL_DOUBLE(result, C_DVAL_P(op1) + C_DVAL_P(op2));
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_LONG, IS_DOUBLE))) {
		ZVAL_DOUBLE(result, ((double)C_LVAL_P(op1)) + C_DVAL_P(op2));
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_DOUBLE, IS_LONG))) {
		ZVAL_DOUBLE(result, C_DVAL_P(op1) + ((double)C_LVAL_P(op2)));
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_ARRAY, IS_ARRAY))) {
		add_function_array(result, op1, op2);
		return SUCCESS;
	} else {
		return FAILURE;
	}
} /* }}} */

static crex_never_inline crex_result CREX_FASTCALL add_function_slow(zval *result, zval *op1, zval *op2) /* {{{ */
{
	ZVAL_DEREF(op1);
	ZVAL_DEREF(op2);
	if (add_function_fast(result, op1, op2) == SUCCESS) {
		return SUCCESS;
	}

	CREX_TRY_BINARY_OBJECT_OPERATION(CREX_ADD);

	zval op1_copy, op2_copy;
	if (UNEXPECTED(crexi_try_convert_scalar_to_number(op1, &op1_copy) == FAILURE)
			|| UNEXPECTED(crexi_try_convert_scalar_to_number(op2, &op2_copy) == FAILURE)) {
		crex_binop_error("+", op1, op2);
		if (result != op1) {
			ZVAL_UNDEF(result);
		}
		return FAILURE;
	}

	if (result == op1) {
		zval_ptr_dtor(result);
	}

	if (add_function_fast(result, &op1_copy, &op2_copy) == SUCCESS) {
		return SUCCESS;
	}

	CREX_ASSERT(0 && "Operation must succeed");
	return FAILURE;
} /* }}} */

CREX_API crex_result CREX_FASTCALL add_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	if (add_function_fast(result, op1, op2) == SUCCESS) {
		return SUCCESS;
	} else {
		return add_function_slow(result, op1, op2);
	}
}
/* }}} */

static crex_always_inline crex_result sub_function_fast(zval *result, zval *op1, zval *op2) /* {{{ */
{
	uint8_t type_pair = TYPE_PAIR(C_TYPE_P(op1), C_TYPE_P(op2));

	if (EXPECTED(type_pair == TYPE_PAIR(IS_LONG, IS_LONG))) {
		fast_long_sub_function(result, op1, op2);
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_DOUBLE, IS_DOUBLE))) {
		ZVAL_DOUBLE(result, C_DVAL_P(op1) - C_DVAL_P(op2));
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_LONG, IS_DOUBLE))) {
		ZVAL_DOUBLE(result, ((double)C_LVAL_P(op1)) - C_DVAL_P(op2));
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_DOUBLE, IS_LONG))) {
		ZVAL_DOUBLE(result, C_DVAL_P(op1) - ((double)C_LVAL_P(op2)));
		return SUCCESS;
	} else {
		return FAILURE;
	}
}
/* }}} */

static crex_never_inline crex_result CREX_FASTCALL sub_function_slow(zval *result, zval *op1, zval *op2) /* {{{ */
{
	ZVAL_DEREF(op1);
	ZVAL_DEREF(op2);
	if (sub_function_fast(result, op1, op2) == SUCCESS) {
		return SUCCESS;
	}

	CREX_TRY_BINARY_OBJECT_OPERATION(CREX_SUB);

	zval op1_copy, op2_copy;
	if (UNEXPECTED(crexi_try_convert_scalar_to_number(op1, &op1_copy) == FAILURE)
			|| UNEXPECTED(crexi_try_convert_scalar_to_number(op2, &op2_copy) == FAILURE)) {
		crex_binop_error("-", op1, op2);
		if (result != op1) {
			ZVAL_UNDEF(result);
		}
		return FAILURE;
	}

	if (result == op1) {
		zval_ptr_dtor(result);
	}

	if (sub_function_fast(result, &op1_copy, &op2_copy) == SUCCESS) {
		return SUCCESS;
	}

	CREX_ASSERT(0 && "Operation must succeed");
	return FAILURE;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL sub_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	if (sub_function_fast(result, op1, op2) == SUCCESS) {
		return SUCCESS;
	} else {
		return sub_function_slow(result, op1, op2);
	}
}
/* }}} */

static crex_always_inline crex_result mul_function_fast(zval *result, zval *op1, zval *op2) /* {{{ */
{
	uint8_t type_pair = TYPE_PAIR(C_TYPE_P(op1), C_TYPE_P(op2));

	if (EXPECTED(type_pair == TYPE_PAIR(IS_LONG, IS_LONG))) {
		crex_long overflow;
		CREX_SIGNED_MULTIPLY_LONG(
			C_LVAL_P(op1), C_LVAL_P(op2),
			C_LVAL_P(result), C_DVAL_P(result), overflow);
		C_TYPE_INFO_P(result) = overflow ? IS_DOUBLE : IS_LONG;
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_DOUBLE, IS_DOUBLE))) {
		ZVAL_DOUBLE(result, C_DVAL_P(op1) * C_DVAL_P(op2));
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_LONG, IS_DOUBLE))) {
		ZVAL_DOUBLE(result, ((double)C_LVAL_P(op1)) * C_DVAL_P(op2));
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_DOUBLE, IS_LONG))) {
		ZVAL_DOUBLE(result, C_DVAL_P(op1) * ((double)C_LVAL_P(op2)));
		return SUCCESS;
	} else {
		return FAILURE;
	}
}
/* }}} */

static crex_never_inline crex_result CREX_FASTCALL mul_function_slow(zval *result, zval *op1, zval *op2) /* {{{ */
{
	ZVAL_DEREF(op1);
	ZVAL_DEREF(op2);
	if (mul_function_fast(result, op1, op2) == SUCCESS) {
		return SUCCESS;
	}

	CREX_TRY_BINARY_OBJECT_OPERATION(CREX_MUL);

	zval op1_copy, op2_copy;
	if (UNEXPECTED(crexi_try_convert_scalar_to_number(op1, &op1_copy) == FAILURE)
			|| UNEXPECTED(crexi_try_convert_scalar_to_number(op2, &op2_copy) == FAILURE)) {
		crex_binop_error("*", op1, op2);
		if (result != op1) {
			ZVAL_UNDEF(result);
		}
		return FAILURE;
	}

	if (result == op1) {
		zval_ptr_dtor(result);
	}

	if (mul_function_fast(result, &op1_copy, &op2_copy) == SUCCESS) {
		return SUCCESS;
	}

	CREX_ASSERT(0 && "Operation must succeed");
	return FAILURE;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL mul_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	if (mul_function_fast(result, op1, op2) == SUCCESS) {
		return SUCCESS;
	} else {
		return mul_function_slow(result, op1, op2);
	}
}
/* }}} */

static crex_result CREX_FASTCALL pow_function_base(zval *result, zval *op1, zval *op2) /* {{{ */
{
	uint8_t type_pair = TYPE_PAIR(C_TYPE_P(op1), C_TYPE_P(op2));

	if (EXPECTED(type_pair == TYPE_PAIR(IS_LONG, IS_LONG))) {
		if (C_LVAL_P(op2) >= 0) {
			crex_long l1 = 1, l2 = C_LVAL_P(op1), i = C_LVAL_P(op2);

			if (i == 0) {
				ZVAL_LONG(result, 1L);
				return SUCCESS;
			} else if (l2 == 0) {
				ZVAL_LONG(result, 0);
				return SUCCESS;
			}

			while (i >= 1) {
				crex_long overflow;
				double dval = 0.0;

				if (i % 2) {
					--i;
					CREX_SIGNED_MULTIPLY_LONG(l1, l2, l1, dval, overflow);
					if (overflow) {
						ZVAL_DOUBLE(result, dval * pow(l2, i));
						return SUCCESS;
					}
				} else {
					i /= 2;
					CREX_SIGNED_MULTIPLY_LONG(l2, l2, l2, dval, overflow);
					if (overflow) {
						ZVAL_DOUBLE(result, (double)l1 * pow(dval, i));
						return SUCCESS;
					}
				}
			}
			/* i == 0 */
			ZVAL_LONG(result, l1);
		} else {
			ZVAL_DOUBLE(result, pow((double)C_LVAL_P(op1), (double)C_LVAL_P(op2)));
		}
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_DOUBLE, IS_DOUBLE))) {
		ZVAL_DOUBLE(result, pow(C_DVAL_P(op1), C_DVAL_P(op2)));
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_LONG, IS_DOUBLE))) {
		ZVAL_DOUBLE(result, pow((double)C_LVAL_P(op1), C_DVAL_P(op2)));
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_DOUBLE, IS_LONG))) {
		ZVAL_DOUBLE(result, pow(C_DVAL_P(op1), (double)C_LVAL_P(op2)));
		return SUCCESS;
	} else {
		return FAILURE;
	}
}
/* }}} */

CREX_API crex_result CREX_FASTCALL pow_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	ZVAL_DEREF(op1);
	ZVAL_DEREF(op2);
	if (pow_function_base(result, op1, op2) == SUCCESS) {
		return SUCCESS;
	}

	CREX_TRY_BINARY_OBJECT_OPERATION(CREX_POW);

	zval op1_copy, op2_copy;
	if (UNEXPECTED(crexi_try_convert_scalar_to_number(op1, &op1_copy) == FAILURE)
			|| UNEXPECTED(crexi_try_convert_scalar_to_number(op2, &op2_copy) == FAILURE)) {
		crex_binop_error("**", op1, op2);
		if (result != op1) {
			ZVAL_UNDEF(result);
		}
		return FAILURE;
	}

	if (result == op1) {
		zval_ptr_dtor(result);
	}

	if (pow_function_base(result, &op1_copy, &op2_copy) == SUCCESS) {
		return SUCCESS;
	}

	CREX_ASSERT(0 && "Operation must succeed");
	return FAILURE;
}
/* }}} */

/* Returns SUCCESS/TYPES_NOT_HANDLED/DIV_BY_ZERO */
#define TYPES_NOT_HANDLED 1
#define DIV_BY_ZERO 2
static int CREX_FASTCALL div_function_base(zval *result, zval *op1, zval *op2) /* {{{ */
{
	uint8_t type_pair = TYPE_PAIR(C_TYPE_P(op1), C_TYPE_P(op2));

	if (EXPECTED(type_pair == TYPE_PAIR(IS_LONG, IS_LONG))) {
		if (C_LVAL_P(op2) == 0) {
			return DIV_BY_ZERO;
		} else if (C_LVAL_P(op2) == -1 && C_LVAL_P(op1) == CREX_LONG_MIN) {
			/* Prevent overflow error/crash */
			ZVAL_DOUBLE(result, (double) CREX_LONG_MIN / -1);
			return SUCCESS;
		}
		if (C_LVAL_P(op1) % C_LVAL_P(op2) == 0) { /* integer */
			ZVAL_LONG(result, C_LVAL_P(op1) / C_LVAL_P(op2));
		} else {
			ZVAL_DOUBLE(result, ((double) C_LVAL_P(op1)) / C_LVAL_P(op2));
		}
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_DOUBLE, IS_DOUBLE))) {
		if (C_DVAL_P(op2) == 0) {
			return DIV_BY_ZERO;
		}
		ZVAL_DOUBLE(result, C_DVAL_P(op1) / C_DVAL_P(op2));
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_DOUBLE, IS_LONG))) {
		if (C_LVAL_P(op2) == 0) {
			return DIV_BY_ZERO;
		}
		ZVAL_DOUBLE(result, C_DVAL_P(op1) / (double)C_LVAL_P(op2));
		return SUCCESS;
	} else if (EXPECTED(type_pair == TYPE_PAIR(IS_LONG, IS_DOUBLE))) {
		if (C_DVAL_P(op2) == 0) {
			return DIV_BY_ZERO;
		}
		ZVAL_DOUBLE(result, (double)C_LVAL_P(op1) / C_DVAL_P(op2));
		return SUCCESS;
	} else {
		return TYPES_NOT_HANDLED;
	}
}
/* }}} */

CREX_API crex_result CREX_FASTCALL div_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	ZVAL_DEREF(op1);
	ZVAL_DEREF(op2);

	int retval = div_function_base(result, op1, op2);
	if (EXPECTED(retval == SUCCESS)) {
		return SUCCESS;
	}

	if (UNEXPECTED(retval == DIV_BY_ZERO)) {
		goto div_by_zero;
	}

	CREX_TRY_BINARY_OBJECT_OPERATION(CREX_DIV);

	zval result_copy, op1_copy, op2_copy;
	if (UNEXPECTED(crexi_try_convert_scalar_to_number(op1, &op1_copy) == FAILURE)
			|| UNEXPECTED(crexi_try_convert_scalar_to_number(op2, &op2_copy) == FAILURE)) {
		crex_binop_error("/", op1, op2);
		if (result != op1) {
			ZVAL_UNDEF(result);
		}
		return FAILURE;
	}

	retval = div_function_base(&result_copy, &op1_copy, &op2_copy);
	if (retval == SUCCESS) {
		if (result == op1) {
			zval_ptr_dtor(result);
		}
		ZVAL_COPY_VALUE(result, &result_copy);
		return SUCCESS;
	}

div_by_zero:
	CREX_ASSERT(retval == DIV_BY_ZERO && "TYPES_NOT_HANDLED should not occur here");
	if (result != op1) {
		ZVAL_UNDEF(result);
	}
	crex_throw_error(crex_ce_division_by_zero_error, "Division by zero");
	return FAILURE;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL mod_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	crex_long op1_lval, op2_lval;

	convert_op1_op2_long(op1, op1_lval, op2, op2_lval, result, CREX_MOD, "%");

	if (op2_lval == 0) {
		/* modulus by zero */
		if (EG(current_execute_data) && !CG(in_compilation)) {
			crex_throw_exception_ex(crex_ce_division_by_zero_error, 0, "Modulo by zero");
		} else {
			crex_error_noreturn(E_ERROR, "Modulo by zero");
		}
		if (op1 != result) {
			ZVAL_UNDEF(result);
		}
		return FAILURE;
	}

	if (op1 == result) {
		zval_ptr_dtor(result);
	}

	if (op2_lval == -1) {
		/* Prevent overflow error/crash if op1==LONG_MIN */
		ZVAL_LONG(result, 0);
		return SUCCESS;
	}

	ZVAL_LONG(result, op1_lval % op2_lval);
	return SUCCESS;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL boolean_xor_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	int op1_val, op2_val;

	do {
		if (C_TYPE_P(op1) == IS_FALSE) {
			op1_val = 0;
		} else if (EXPECTED(C_TYPE_P(op1) == IS_TRUE)) {
			op1_val = 1;
		} else {
			if (C_ISREF_P(op1)) {
				op1 = C_REFVAL_P(op1);
				if (C_TYPE_P(op1) == IS_FALSE) {
					op1_val = 0;
					break;
				} else if (EXPECTED(C_TYPE_P(op1) == IS_TRUE)) {
					op1_val = 1;
					break;
				}
			}
			CREX_TRY_BINARY_OP1_OBJECT_OPERATION(CREX_BOOL_XOR);
			op1_val = zval_is_true(op1);
		}
	} while (0);
	do {
		if (C_TYPE_P(op2) == IS_FALSE) {
			op2_val = 0;
		} else if (EXPECTED(C_TYPE_P(op2) == IS_TRUE)) {
			op2_val = 1;
		} else {
			if (C_ISREF_P(op2)) {
				op2 = C_REFVAL_P(op2);
				if (C_TYPE_P(op2) == IS_FALSE) {
					op2_val = 0;
					break;
				} else if (EXPECTED(C_TYPE_P(op2) == IS_TRUE)) {
					op2_val = 1;
					break;
				}
			}
			CREX_TRY_BINARY_OP2_OBJECT_OPERATION(CREX_BOOL_XOR);
			op2_val = zval_is_true(op2);
		}
	} while (0);

	ZVAL_BOOL(result, op1_val ^ op2_val);
	return SUCCESS;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL boolean_not_function(zval *result, zval *op1) /* {{{ */
{
	if (C_TYPE_P(op1) < IS_TRUE) {
		ZVAL_TRUE(result);
	} else if (EXPECTED(C_TYPE_P(op1) == IS_TRUE)) {
		ZVAL_FALSE(result);
	} else {
		if (C_ISREF_P(op1)) {
			op1 = C_REFVAL_P(op1);
			if (C_TYPE_P(op1) < IS_TRUE) {
				ZVAL_TRUE(result);
				return SUCCESS;
			} else if (EXPECTED(C_TYPE_P(op1) == IS_TRUE)) {
				ZVAL_FALSE(result);
				return SUCCESS;
			}
		}
		CREX_TRY_UNARY_OBJECT_OPERATION(CREX_BOOL_NOT);

		ZVAL_BOOL(result, !zval_is_true(op1));
	}
	return SUCCESS;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL bitwise_not_function(zval *result, zval *op1) /* {{{ */
{
try_again:
	switch (C_TYPE_P(op1)) {
		case IS_LONG:
			ZVAL_LONG(result, ~C_LVAL_P(op1));
			return SUCCESS;
		case IS_DOUBLE: {
			crex_long lval = crex_dval_to_lval(C_DVAL_P(op1));
			if (!crex_is_long_compatible(C_DVAL_P(op1), lval)) {
				crex_incompatible_double_to_long_error(C_DVAL_P(op1));
				if (EG(exception)) {
					if (result != op1) {
						ZVAL_UNDEF(result);
					}
					return FAILURE;
				}
			}
			ZVAL_LONG(result, ~lval);
			return SUCCESS;
		}
		case IS_STRING: {
			size_t i;

			if (C_STRLEN_P(op1) == 1) {
				crex_uchar not = (crex_uchar) ~*C_STRVAL_P(op1);
				ZVAL_CHAR(result, not);
			} else {
				ZVAL_NEW_STR(result, crex_string_alloc(C_STRLEN_P(op1), 0));
				for (i = 0; i < C_STRLEN_P(op1); i++) {
					C_STRVAL_P(result)[i] = ~C_STRVAL_P(op1)[i];
				}
				C_STRVAL_P(result)[i] = 0;
			}
			return SUCCESS;
		}
		case IS_REFERENCE:
			op1 = C_REFVAL_P(op1);
			goto try_again;
		default:
			CREX_TRY_UNARY_OBJECT_OPERATION(CREX_BW_NOT);

			if (result != op1) {
				ZVAL_UNDEF(result);
			}
			crex_type_error("Cannot perform bitwise not on %s", crex_zval_value_name(op1));
			return FAILURE;
	}
}
/* }}} */

CREX_API crex_result CREX_FASTCALL bitwise_or_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	crex_long op1_lval, op2_lval;

	if (EXPECTED(C_TYPE_P(op1) == IS_LONG) && EXPECTED(C_TYPE_P(op2) == IS_LONG)) {
		ZVAL_LONG(result, C_LVAL_P(op1) | C_LVAL_P(op2));
		return SUCCESS;
	}

	ZVAL_DEREF(op1);
	ZVAL_DEREF(op2);

	if (C_TYPE_P(op1) == IS_STRING && EXPECTED(C_TYPE_P(op2) == IS_STRING)) {
		zval *longer, *shorter;
		crex_string *str;
		size_t i;

		if (EXPECTED(C_STRLEN_P(op1) >= C_STRLEN_P(op2))) {
			if (EXPECTED(C_STRLEN_P(op1) == C_STRLEN_P(op2)) && C_STRLEN_P(op1) == 1) {
				crex_uchar or = (crex_uchar) (*C_STRVAL_P(op1) | *C_STRVAL_P(op2));
				if (result==op1) {
					zval_ptr_dtor_str(result);
				}
				ZVAL_CHAR(result, or);
				return SUCCESS;
			}
			longer = op1;
			shorter = op2;
		} else {
			longer = op2;
			shorter = op1;
		}

		str = crex_string_alloc(C_STRLEN_P(longer), 0);
		for (i = 0; i < C_STRLEN_P(shorter); i++) {
			ZSTR_VAL(str)[i] = C_STRVAL_P(longer)[i] | C_STRVAL_P(shorter)[i];
		}
		memcpy(ZSTR_VAL(str) + i, C_STRVAL_P(longer) + i, C_STRLEN_P(longer) - i + 1);
		if (result==op1) {
			zval_ptr_dtor_str(result);
		}
		ZVAL_NEW_STR(result, str);
		return SUCCESS;
	}

	if (UNEXPECTED(C_TYPE_P(op1) != IS_LONG)) {
		bool failed;
		CREX_TRY_BINARY_OP1_OBJECT_OPERATION(CREX_BW_OR);
		op1_lval = crexi_try_get_long(op1, &failed);
		if (UNEXPECTED(failed)) {
			crex_binop_error("|", op1, op2);
			if (result != op1) {
				ZVAL_UNDEF(result);
			}
			return FAILURE;
		}
	} else {
		op1_lval = C_LVAL_P(op1);
	}
	if (UNEXPECTED(C_TYPE_P(op2) != IS_LONG)) {
		bool failed;
		CREX_TRY_BINARY_OP2_OBJECT_OPERATION(CREX_BW_OR);
		op2_lval = crexi_try_get_long(op2, &failed);
		if (UNEXPECTED(failed)) {
			crex_binop_error("|", op1, op2);
			if (result != op1) {
				ZVAL_UNDEF(result);
			}
			return FAILURE;
		}
	} else {
		op2_lval = C_LVAL_P(op2);
	}

	if (op1 == result) {
		zval_ptr_dtor(result);
	}
	ZVAL_LONG(result, op1_lval | op2_lval);
	return SUCCESS;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL bitwise_and_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	crex_long op1_lval, op2_lval;

	if (EXPECTED(C_TYPE_P(op1) == IS_LONG) && EXPECTED(C_TYPE_P(op2) == IS_LONG)) {
		ZVAL_LONG(result, C_LVAL_P(op1) & C_LVAL_P(op2));
		return SUCCESS;
	}

	ZVAL_DEREF(op1);
	ZVAL_DEREF(op2);

	if (C_TYPE_P(op1) == IS_STRING && C_TYPE_P(op2) == IS_STRING) {
		zval *longer, *shorter;
		crex_string *str;
		size_t i;

		if (EXPECTED(C_STRLEN_P(op1) >= C_STRLEN_P(op2))) {
			if (EXPECTED(C_STRLEN_P(op1) == C_STRLEN_P(op2)) && C_STRLEN_P(op1) == 1) {
				crex_uchar and = (crex_uchar) (*C_STRVAL_P(op1) & *C_STRVAL_P(op2));
				if (result==op1) {
					zval_ptr_dtor_str(result);
				}
				ZVAL_CHAR(result, and);
				return SUCCESS;
			}
			longer = op1;
			shorter = op2;
		} else {
			longer = op2;
			shorter = op1;
		}

		str = crex_string_alloc(C_STRLEN_P(shorter), 0);
		for (i = 0; i < C_STRLEN_P(shorter); i++) {
			ZSTR_VAL(str)[i] = C_STRVAL_P(shorter)[i] & C_STRVAL_P(longer)[i];
		}
		ZSTR_VAL(str)[i] = 0;
		if (result==op1) {
			zval_ptr_dtor_str(result);
		}
		ZVAL_NEW_STR(result, str);
		return SUCCESS;
	}

	if (UNEXPECTED(C_TYPE_P(op1) != IS_LONG)) {
		bool failed;
		CREX_TRY_BINARY_OP1_OBJECT_OPERATION(CREX_BW_AND);
		op1_lval = crexi_try_get_long(op1, &failed);
		if (UNEXPECTED(failed)) {
			crex_binop_error("&", op1, op2);
			if (result != op1) {
				ZVAL_UNDEF(result);
			}
			return FAILURE;
		}
	} else {
		op1_lval = C_LVAL_P(op1);
	}
	if (UNEXPECTED(C_TYPE_P(op2) != IS_LONG)) {
		bool failed;
		CREX_TRY_BINARY_OP2_OBJECT_OPERATION(CREX_BW_AND);
		op2_lval = crexi_try_get_long(op2, &failed);
		if (UNEXPECTED(failed)) {
			crex_binop_error("&", op1, op2);
			if (result != op1) {
				ZVAL_UNDEF(result);
			}
			return FAILURE;
		}
	} else {
		op2_lval = C_LVAL_P(op2);
	}

	if (op1 == result) {
		zval_ptr_dtor(result);
	}
	ZVAL_LONG(result, op1_lval & op2_lval);
	return SUCCESS;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL bitwise_xor_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	crex_long op1_lval, op2_lval;

	if (EXPECTED(C_TYPE_P(op1) == IS_LONG) && EXPECTED(C_TYPE_P(op2) == IS_LONG)) {
		ZVAL_LONG(result, C_LVAL_P(op1) ^ C_LVAL_P(op2));
		return SUCCESS;
	}

	ZVAL_DEREF(op1);
	ZVAL_DEREF(op2);

	if (C_TYPE_P(op1) == IS_STRING && C_TYPE_P(op2) == IS_STRING) {
		zval *longer, *shorter;
		crex_string *str;
		size_t i;

		if (EXPECTED(C_STRLEN_P(op1) >= C_STRLEN_P(op2))) {
			if (EXPECTED(C_STRLEN_P(op1) == C_STRLEN_P(op2)) && C_STRLEN_P(op1) == 1) {
				crex_uchar xor = (crex_uchar) (*C_STRVAL_P(op1) ^ *C_STRVAL_P(op2));
				if (result==op1) {
					zval_ptr_dtor_str(result);
				}
				ZVAL_CHAR(result, xor);
				return SUCCESS;
			}
			longer = op1;
			shorter = op2;
		} else {
			longer = op2;
			shorter = op1;
		}

		str = crex_string_alloc(C_STRLEN_P(shorter), 0);
		for (i = 0; i < C_STRLEN_P(shorter); i++) {
			ZSTR_VAL(str)[i] = C_STRVAL_P(shorter)[i] ^ C_STRVAL_P(longer)[i];
		}
		ZSTR_VAL(str)[i] = 0;
		if (result==op1) {
			zval_ptr_dtor_str(result);
		}
		ZVAL_NEW_STR(result, str);
		return SUCCESS;
	}

	if (UNEXPECTED(C_TYPE_P(op1) != IS_LONG)) {
		bool failed;
		CREX_TRY_BINARY_OP1_OBJECT_OPERATION(CREX_BW_XOR);
		op1_lval = crexi_try_get_long(op1, &failed);
		if (UNEXPECTED(failed)) {
			crex_binop_error("^", op1, op2);
			if (result != op1) {
				ZVAL_UNDEF(result);
			}
			return FAILURE;
		}
	} else {
		op1_lval = C_LVAL_P(op1);
	}
	if (UNEXPECTED(C_TYPE_P(op2) != IS_LONG)) {
		bool failed;
		CREX_TRY_BINARY_OP2_OBJECT_OPERATION(CREX_BW_XOR);
		op2_lval = crexi_try_get_long(op2, &failed);
		if (UNEXPECTED(failed)) {
			crex_binop_error("^", op1, op2);
			if (result != op1) {
				ZVAL_UNDEF(result);
			}
			return FAILURE;
		}
	} else {
		op2_lval = C_LVAL_P(op2);
	}

	if (op1 == result) {
		zval_ptr_dtor(result);
	}
	ZVAL_LONG(result, op1_lval ^ op2_lval);
	return SUCCESS;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL shift_left_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	crex_long op1_lval, op2_lval;

	convert_op1_op2_long(op1, op1_lval, op2, op2_lval, result, CREX_SL, "<<");

	/* prevent wrapping quirkiness on some processors where << 64 + x == << x */
	if (UNEXPECTED((crex_ulong)op2_lval >= SIZEOF_CREX_LONG * 8)) {
		if (EXPECTED(op2_lval > 0)) {
			if (op1 == result) {
				zval_ptr_dtor(result);
			}
			ZVAL_LONG(result, 0);
			return SUCCESS;
		} else {
			if (EG(current_execute_data) && !CG(in_compilation)) {
				crex_throw_exception_ex(crex_ce_arithmetic_error, 0, "Bit shift by negative number");
			} else {
				crex_error_noreturn(E_ERROR, "Bit shift by negative number");
			}
			if (op1 != result) {
				ZVAL_UNDEF(result);
			}
			return FAILURE;
		}
	}

	if (op1 == result) {
		zval_ptr_dtor(result);
	}

	/* Perform shift on unsigned numbers to get well-defined wrap behavior. */
	ZVAL_LONG(result, (crex_long) ((crex_ulong) op1_lval << op2_lval));
	return SUCCESS;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL shift_right_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	crex_long op1_lval, op2_lval;

	convert_op1_op2_long(op1, op1_lval, op2, op2_lval, result, CREX_SR, ">>");

	/* prevent wrapping quirkiness on some processors where >> 64 + x == >> x */
	if (UNEXPECTED((crex_ulong)op2_lval >= SIZEOF_CREX_LONG * 8)) {
		if (EXPECTED(op2_lval > 0)) {
			if (op1 == result) {
				zval_ptr_dtor(result);
			}
			ZVAL_LONG(result, (op1_lval < 0) ? -1 : 0);
			return SUCCESS;
		} else {
			if (EG(current_execute_data) && !CG(in_compilation)) {
				crex_throw_exception_ex(crex_ce_arithmetic_error, 0, "Bit shift by negative number");
			} else {
				crex_error_noreturn(E_ERROR, "Bit shift by negative number");
			}
			if (op1 != result) {
				ZVAL_UNDEF(result);
			}
			return FAILURE;
		}
	}

	if (op1 == result) {
		zval_ptr_dtor(result);
	}

	ZVAL_LONG(result, op1_lval >> op2_lval);
	return SUCCESS;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL concat_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	zval *orig_op1 = op1;
	crex_string *op1_string, *op2_string;
	bool free_op1_string = false;
	bool free_op2_string = false;

	do {
		if (EXPECTED(C_TYPE_P(op1) == IS_STRING)) {
			op1_string = C_STR_P(op1);
		} else {
	 		if (C_ISREF_P(op1)) {
	 			op1 = C_REFVAL_P(op1);
				if (C_TYPE_P(op1) == IS_STRING) {
					op1_string = C_STR_P(op1);
					break;
				}
	 		}
			CREX_TRY_BINARY_OBJECT_OPERATION(CREX_CONCAT);
			op1_string = zval_get_string_func(op1);
			if (UNEXPECTED(EG(exception))) {
				crex_string_release(op1_string);
				if (orig_op1 != result) {
					ZVAL_UNDEF(result);
				}
				return FAILURE;
			}
			free_op1_string = true;
			if (result == op1) {
				if (UNEXPECTED(op1 == op2)) {
					op2_string = op1_string;
					goto has_op2_string;
				}
			}
		}
	} while (0);
	do {
		if (EXPECTED(C_TYPE_P(op2) == IS_STRING)) {
			op2_string = C_STR_P(op2);
		} else {
			if (C_ISREF_P(op2)) {
				op2 = C_REFVAL_P(op2);
				if (C_TYPE_P(op2) == IS_STRING) {
					op2_string = C_STR_P(op2);
					break;
				}
			}
			/* hold an additional reference because a userland function could free this */
			if (!free_op1_string) {
				op1_string = crex_string_copy(op1_string);
				free_op1_string = true;
			}
			CREX_TRY_BINARY_OP2_OBJECT_OPERATION(CREX_CONCAT);
			op2_string = zval_get_string_func(op2);
			if (UNEXPECTED(EG(exception))) {
				crex_string_release(op1_string);
				crex_string_release(op2_string);
				if (orig_op1 != result) {
					ZVAL_UNDEF(result);
				}
				return FAILURE;
			}
			free_op2_string = true;
		}
	} while (0);

has_op2_string:;
	if (UNEXPECTED(ZSTR_LEN(op1_string) == 0)) {
		if (EXPECTED(result != op2 || C_TYPE_P(result) != IS_STRING)) {
			if (result == orig_op1) {
				i_zval_ptr_dtor(result);
			}
			if (free_op2_string) {
				/* transfer ownership of op2_string */
				ZVAL_STR(result, op2_string);
				free_op2_string = false;
			} else {
				ZVAL_STR_COPY(result, op2_string);
			}
		}
	} else if (UNEXPECTED(ZSTR_LEN(op2_string) == 0)) {
		if (EXPECTED(result != op1 || C_TYPE_P(result) != IS_STRING)) {
			if (result == orig_op1) {
				i_zval_ptr_dtor(result);
			}
			if (free_op1_string) {
				/* transfer ownership of op1_string */
				ZVAL_STR(result, op1_string);
				free_op1_string = false;
			} else {
				ZVAL_STR_COPY(result, op1_string);
			}
		}
	} else {
		size_t op1_len = ZSTR_LEN(op1_string);
		size_t op2_len = ZSTR_LEN(op2_string);
		size_t result_len = op1_len + op2_len;
		crex_string *result_str;
		uint32_t flags = ZSTR_GET_COPYABLE_CONCAT_PROPERTIES_BOTH(op1_string, op2_string);

		if (UNEXPECTED(op1_len > ZSTR_MAX_LEN - op2_len)) {
			if (free_op1_string) crex_string_release(op1_string);
			if (free_op2_string) crex_string_release(op2_string);
			crex_throw_error(NULL, "String size overflow");
			if (orig_op1 != result) {
				ZVAL_UNDEF(result);
			}
			return FAILURE;
		}

		if (result == op1) {
			/* Destroy the old result first to drop the refcount, such that $x .= ...; may happen in-place. */
			if (free_op1_string) {
				/* op1_string will be used as the result, so we should not free it */
				i_zval_ptr_dtor(result);
				/* Set it to NULL in case that the extension will throw an out-of-memory error.
				 * Otherwise the shutdown sequence will try to free this again. */
				ZVAL_NULL(result);
				free_op1_string = false;
			}
			/* special case, perform operations on result */
			result_str = crex_string_extend(op1_string, result_len, 0);
			/* account for the case where result_str == op1_string == op2_string and the realloc is done */
			if (op1_string == op2_string) {
				if (free_op2_string) {
					crex_string_release(op2_string);
					free_op2_string = false;
				}
				op2_string = result_str;
			}
		} else {
			result_str = crex_string_alloc(result_len, 0);
			memcpy(ZSTR_VAL(result_str), ZSTR_VAL(op1_string), op1_len);
			if (result == orig_op1) {
				i_zval_ptr_dtor(result);
			}
		}
		GC_ADD_FLAGS(result_str, flags);

		ZVAL_NEW_STR(result, result_str);
		memcpy(ZSTR_VAL(result_str) + op1_len, ZSTR_VAL(op2_string), op2_len);
		ZSTR_VAL(result_str)[result_len] = '\0';
	}

	if (free_op1_string) crex_string_release(op1_string);
	if (free_op2_string) crex_string_release(op2_string);

	return SUCCESS;
}
/* }}} */

CREX_API int CREX_FASTCALL string_compare_function_ex(zval *op1, zval *op2, bool case_insensitive) /* {{{ */
{
	crex_string *tmp_str1, *tmp_str2;
	crex_string *str1 = zval_get_tmp_string(op1, &tmp_str1);
	crex_string *str2 = zval_get_tmp_string(op2, &tmp_str2);
	int ret;

	if (case_insensitive) {
		ret = crex_binary_strcasecmp(ZSTR_VAL(str1), ZSTR_LEN(str1), ZSTR_VAL(str2), ZSTR_LEN(str2));
	} else {
		ret = crex_binary_strcmp(ZSTR_VAL(str1), ZSTR_LEN(str1), ZSTR_VAL(str2), ZSTR_LEN(str2));
	}

	crex_tmp_string_release(tmp_str1);
	crex_tmp_string_release(tmp_str2);
	return ret;
}
/* }}} */

CREX_API int CREX_FASTCALL string_compare_function(zval *op1, zval *op2) /* {{{ */
{
	if (EXPECTED(C_TYPE_P(op1) == IS_STRING) &&
	    EXPECTED(C_TYPE_P(op2) == IS_STRING)) {
		if (C_STR_P(op1) == C_STR_P(op2)) {
			return 0;
		} else {
			return crex_binary_strcmp(C_STRVAL_P(op1), C_STRLEN_P(op1), C_STRVAL_P(op2), C_STRLEN_P(op2));
		}
	} else {
		crex_string *tmp_str1, *tmp_str2;
		crex_string *str1 = zval_get_tmp_string(op1, &tmp_str1);
		crex_string *str2 = zval_get_tmp_string(op2, &tmp_str2);
		int ret = crex_binary_strcmp(ZSTR_VAL(str1), ZSTR_LEN(str1), ZSTR_VAL(str2), ZSTR_LEN(str2));

		crex_tmp_string_release(tmp_str1);
		crex_tmp_string_release(tmp_str2);
		return ret;
	}
}
/* }}} */

CREX_API int CREX_FASTCALL string_case_compare_function(zval *op1, zval *op2) /* {{{ */
{
	if (EXPECTED(C_TYPE_P(op1) == IS_STRING) &&
	    EXPECTED(C_TYPE_P(op2) == IS_STRING)) {
		if (C_STR_P(op1) == C_STR_P(op2)) {
			return 0;
		} else {
			return crex_binary_strcasecmp(C_STRVAL_P(op1), C_STRLEN_P(op1), C_STRVAL_P(op2), C_STRLEN_P(op2));
		}
	} else {
		crex_string *tmp_str1, *tmp_str2;
		crex_string *str1 = zval_get_tmp_string(op1, &tmp_str1);
		crex_string *str2 = zval_get_tmp_string(op2, &tmp_str2);
		int ret = crex_binary_strcasecmp(ZSTR_VAL(str1), ZSTR_LEN(str1), ZSTR_VAL(str2), ZSTR_LEN(str2));

		crex_tmp_string_release(tmp_str1);
		crex_tmp_string_release(tmp_str2);
		return ret;
	}
}
/* }}} */

CREX_API int CREX_FASTCALL string_locale_compare_function(zval *op1, zval *op2) /* {{{ */
{
	crex_string *tmp_str1, *tmp_str2;
	crex_string *str1 = zval_get_tmp_string(op1, &tmp_str1);
	crex_string *str2 = zval_get_tmp_string(op2, &tmp_str2);
	int ret = strcoll(ZSTR_VAL(str1), ZSTR_VAL(str2));

	crex_tmp_string_release(tmp_str1);
	crex_tmp_string_release(tmp_str2);
	return ret;
}
/* }}} */

CREX_API int CREX_FASTCALL numeric_compare_function(zval *op1, zval *op2) /* {{{ */
{
	double d1, d2;

	d1 = zval_get_double(op1);
	d2 = zval_get_double(op2);

	return CREX_THREEWAY_COMPARE(d1, d2);
}
/* }}} */

CREX_API crex_result CREX_FASTCALL compare_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	ZVAL_LONG(result, crex_compare(op1, op2));
	return SUCCESS;
}
/* }}} */

static int compare_long_to_string(crex_long lval, crex_string *str) /* {{{ */
{
	crex_long str_lval;
	double str_dval;
	uint8_t type = is_numeric_string(ZSTR_VAL(str), ZSTR_LEN(str), &str_lval, &str_dval, 0);

	if (type == IS_LONG) {
		return lval > str_lval ? 1 : lval < str_lval ? -1 : 0;
	}

	if (type == IS_DOUBLE) {
		return CREX_THREEWAY_COMPARE((double) lval, str_dval);
	}

	crex_string *lval_as_str = crex_long_to_str(lval);
	int cmp_result = crex_binary_strcmp(
		ZSTR_VAL(lval_as_str), ZSTR_LEN(lval_as_str), ZSTR_VAL(str), ZSTR_LEN(str));
	crex_string_release(lval_as_str);
	return CREX_NORMALIZE_BOOL(cmp_result);
}
/* }}} */

static int compare_double_to_string(double dval, crex_string *str) /* {{{ */
{
	crex_long str_lval;
	double str_dval;
	uint8_t type = is_numeric_string(ZSTR_VAL(str), ZSTR_LEN(str), &str_lval, &str_dval, 0);

	if (type == IS_LONG) {
		return CREX_THREEWAY_COMPARE(dval, (double) str_lval);
	}

	if (type == IS_DOUBLE) {
		return CREX_THREEWAY_COMPARE(dval, str_dval);
	}

	crex_string *dval_as_str = crex_double_to_str(dval);
	int cmp_result = crex_binary_strcmp(
		ZSTR_VAL(dval_as_str), ZSTR_LEN(dval_as_str), ZSTR_VAL(str), ZSTR_LEN(str));
	crex_string_release(dval_as_str);
	return CREX_NORMALIZE_BOOL(cmp_result);
}
/* }}} */

CREX_API int CREX_FASTCALL crex_compare(zval *op1, zval *op2) /* {{{ */
{
	int converted = 0;
	zval op1_copy, op2_copy;

	while (1) {
		switch (TYPE_PAIR(C_TYPE_P(op1), C_TYPE_P(op2))) {
			case TYPE_PAIR(IS_LONG, IS_LONG):
				return C_LVAL_P(op1)>C_LVAL_P(op2)?1:(C_LVAL_P(op1)<C_LVAL_P(op2)?-1:0);

			case TYPE_PAIR(IS_DOUBLE, IS_LONG):
				return CREX_THREEWAY_COMPARE(C_DVAL_P(op1), (double)C_LVAL_P(op2));

			case TYPE_PAIR(IS_LONG, IS_DOUBLE):
				return CREX_THREEWAY_COMPARE((double)C_LVAL_P(op1), C_DVAL_P(op2));

			case TYPE_PAIR(IS_DOUBLE, IS_DOUBLE):
				return CREX_THREEWAY_COMPARE(C_DVAL_P(op1), C_DVAL_P(op2));

			case TYPE_PAIR(IS_ARRAY, IS_ARRAY):
				return crex_compare_arrays(op1, op2);

			case TYPE_PAIR(IS_NULL, IS_NULL):
			case TYPE_PAIR(IS_NULL, IS_FALSE):
			case TYPE_PAIR(IS_FALSE, IS_NULL):
			case TYPE_PAIR(IS_FALSE, IS_FALSE):
			case TYPE_PAIR(IS_TRUE, IS_TRUE):
				return 0;

			case TYPE_PAIR(IS_NULL, IS_TRUE):
				return -1;

			case TYPE_PAIR(IS_TRUE, IS_NULL):
				return 1;

			case TYPE_PAIR(IS_STRING, IS_STRING):
				if (C_STR_P(op1) == C_STR_P(op2)) {
					return 0;
				}
				return crexi_smart_strcmp(C_STR_P(op1), C_STR_P(op2));

			case TYPE_PAIR(IS_NULL, IS_STRING):
				return C_STRLEN_P(op2) == 0 ? 0 : -1;

			case TYPE_PAIR(IS_STRING, IS_NULL):
				return C_STRLEN_P(op1) == 0 ? 0 : 1;

			case TYPE_PAIR(IS_LONG, IS_STRING):
				return compare_long_to_string(C_LVAL_P(op1), C_STR_P(op2));

			case TYPE_PAIR(IS_STRING, IS_LONG):
				return -compare_long_to_string(C_LVAL_P(op2), C_STR_P(op1));

			case TYPE_PAIR(IS_DOUBLE, IS_STRING):
				if (crex_isnan(C_DVAL_P(op1))) {
					return 1;
				}

				return compare_double_to_string(C_DVAL_P(op1), C_STR_P(op2));

			case TYPE_PAIR(IS_STRING, IS_DOUBLE):
				if (crex_isnan(C_DVAL_P(op2))) {
					return 1;
				}

				return -compare_double_to_string(C_DVAL_P(op2), C_STR_P(op1));

			case TYPE_PAIR(IS_OBJECT, IS_NULL):
				return 1;

			case TYPE_PAIR(IS_NULL, IS_OBJECT):
				return -1;

			default:
				if (C_ISREF_P(op1)) {
					op1 = C_REFVAL_P(op1);
					continue;
				} else if (C_ISREF_P(op2)) {
					op2 = C_REFVAL_P(op2);
					continue;
				}

				if (C_TYPE_P(op1) == IS_OBJECT
				 && C_TYPE_P(op2) == IS_OBJECT
				 && C_OBJ_P(op1) == C_OBJ_P(op2)) {
					return 0;
				} else if (C_TYPE_P(op1) == IS_OBJECT) {
					return C_OBJ_HANDLER_P(op1, compare)(op1, op2);
				} else if (C_TYPE_P(op2) == IS_OBJECT) {
					return C_OBJ_HANDLER_P(op2, compare)(op1, op2);
				}

				if (!converted) {
					if (C_TYPE_P(op1) < IS_TRUE) {
						return zval_is_true(op2) ? -1 : 0;
					} else if (C_TYPE_P(op1) == IS_TRUE) {
						return zval_is_true(op2) ? 0 : 1;
					} else if (C_TYPE_P(op2) < IS_TRUE) {
						return zval_is_true(op1) ? 1 : 0;
					} else if (C_TYPE_P(op2) == IS_TRUE) {
						return zval_is_true(op1) ? 0 : -1;
					} else {
						op1 = _crexi_convert_scalar_to_number_silent(op1, &op1_copy);
						op2 = _crexi_convert_scalar_to_number_silent(op2, &op2_copy);
						if (EG(exception)) {
							return 1; /* to stop comparison of arrays */
						}
						converted = 1;
					}
				} else if (C_TYPE_P(op1)==IS_ARRAY) {
					return 1;
				} else if (C_TYPE_P(op2)==IS_ARRAY) {
					return -1;
				} else {
					CREX_UNREACHABLE();
					crex_throw_error(NULL, "Unsupported operand types");
					return 1;
				}
		}
	}
}
/* }}} */

/* return int to be compatible with compare_func_t */
static int hash_zval_identical_function(zval *z1, zval *z2) /* {{{ */
{
	/* is_identical_function() returns 1 in case of identity and 0 in case
	 * of a difference;
	 * whereas this comparison function is expected to return 0 on identity,
	 * and non zero otherwise.
	 */
	ZVAL_DEREF(z1);
	ZVAL_DEREF(z2);
	return fast_is_not_identical_function(z1, z2);
}
/* }}} */

CREX_API bool CREX_FASTCALL crex_is_identical(const zval *op1, const zval *op2) /* {{{ */
{
	if (C_TYPE_P(op1) != C_TYPE_P(op2)) {
		return 0;
	}
	switch (C_TYPE_P(op1)) {
		case IS_NULL:
		case IS_FALSE:
		case IS_TRUE:
			return 1;
		case IS_LONG:
			return (C_LVAL_P(op1) == C_LVAL_P(op2));
		case IS_RESOURCE:
			return (C_RES_P(op1) == C_RES_P(op2));
		case IS_DOUBLE:
			return (C_DVAL_P(op1) == C_DVAL_P(op2));
		case IS_STRING:
			return crex_string_equals(C_STR_P(op1), C_STR_P(op2));
		case IS_ARRAY:
			return (C_ARRVAL_P(op1) == C_ARRVAL_P(op2) ||
				crex_hash_compare(C_ARRVAL_P(op1), C_ARRVAL_P(op2), (compare_func_t) hash_zval_identical_function, 1) == 0);
		case IS_OBJECT:
			return (C_OBJ_P(op1) == C_OBJ_P(op2));
		default:
			return 0;
	}
}
/* }}} */

CREX_API crex_result CREX_FASTCALL is_identical_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	ZVAL_BOOL(result, crex_is_identical(op1, op2));
	return SUCCESS;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL is_not_identical_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	ZVAL_BOOL(result, !crex_is_identical(op1, op2));
	return SUCCESS;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL is_equal_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	ZVAL_BOOL(result, crex_compare(op1, op2) == 0);
	return SUCCESS;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL is_not_equal_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	ZVAL_BOOL(result, (crex_compare(op1, op2) != 0));
	return SUCCESS;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL is_smaller_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	ZVAL_BOOL(result, (crex_compare(op1, op2) < 0));
	return SUCCESS;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL is_smaller_or_equal_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	ZVAL_BOOL(result, (crex_compare(op1, op2) <= 0));
	return SUCCESS;
}
/* }}} */

CREX_API bool CREX_FASTCALL crex_class_implements_interface(const crex_class_entry *class_ce, const crex_class_entry *interface_ce) /* {{{ */
{
	uint32_t i;
	CREX_ASSERT(interface_ce->ce_flags & CREX_ACC_INTERFACE);

	if (class_ce->num_interfaces) {
		CREX_ASSERT(class_ce->ce_flags & CREX_ACC_RESOLVED_INTERFACES);
		for (i = 0; i < class_ce->num_interfaces; i++) {
			if (class_ce->interfaces[i] == interface_ce) {
				return 1;
			}
		}
	}
	return 0;
}
/* }}} */

CREX_API bool CREX_FASTCALL instanceof_function_slow(const crex_class_entry *instance_ce, const crex_class_entry *ce) /* {{{ */
{
	CREX_ASSERT(instance_ce != ce && "Should have been checked already");
	if (ce->ce_flags & CREX_ACC_INTERFACE) {
		uint32_t i;

		if (instance_ce->num_interfaces) {
			CREX_ASSERT(instance_ce->ce_flags & CREX_ACC_RESOLVED_INTERFACES);
			for (i = 0; i < instance_ce->num_interfaces; i++) {
				if (instance_ce->interfaces[i] == ce) {
					return 1;
				}
			}
		}
		return 0;
	} else {
		while (1) {
			instance_ce = instance_ce->parent;
			if (instance_ce == ce) {
				return 1;
			}
			if (instance_ce == NULL) {
				return 0;
			}
		}
	}
}
/* }}} */

#define LOWER_CASE 1
#define UPPER_CASE 2
#define NUMERIC 3

CREX_API bool crex_string_only_has_ascii_alphanumeric(const crex_string *str)
{
	const char *p = ZSTR_VAL(str);
	const char *e = ZSTR_VAL(str) + ZSTR_LEN(str);
	while (p < e) {
		char c = *p++;
		if (UNEXPECTED( c < '0' || c > 'z' || (c < 'a' && c > 'Z') || (c < 'A' && c > '9') ) ) {
			return false;
		}
	}
	return true;
}

static bool CREX_FASTCALL increment_string(zval *str) /* {{{ */
{
	int carry=0;
	size_t pos=C_STRLEN_P(str)-1;
	char *s;
	crex_string *t;
	int last=0; /* Shut up the compiler warning */
	int ch;

	if (UNEXPECTED(C_STRLEN_P(str) == 0)) {
		crex_error(E_DEPRECATED, "Increment on non-alphanumeric string is deprecated");
		if (EG(exception)) {
			return false;
		}
		/* A userland error handler can change the type from string to something else */
		zval_ptr_dtor(str);
		ZVAL_CHAR(str, '1');
		return true;
	}

	if (UNEXPECTED(!crex_string_only_has_ascii_alphanumeric(C_STR_P(str)))) {
		crex_string *zstr = C_STR_P(str);
		crex_string_addref(zstr);
		crex_error(E_DEPRECATED, "Increment on non-alphanumeric string is deprecated");
		if (EG(exception)) {
			crex_string_release(zstr);
			return false;
		}
		zval_ptr_dtor(str);
		ZVAL_STR(str, zstr);
	}

	if (!C_REFCOUNTED_P(str)) {
		C_STR_P(str) = crex_string_init(C_STRVAL_P(str), C_STRLEN_P(str), 0);
		C_TYPE_INFO_P(str) = IS_STRING_EX;
	} else if (C_REFCOUNT_P(str) > 1) {
		/* Only release string after allocation succeeded. */
		crex_string *orig_str = C_STR_P(str);
		C_STR_P(str) = crex_string_init(C_STRVAL_P(str), C_STRLEN_P(str), 0);
		GC_DELREF(orig_str);
	} else {
		crex_string_forget_hash_val(C_STR_P(str));
	}
	s = C_STRVAL_P(str);

	do {
		ch = s[pos];
		if (ch >= 'a' && ch <= 'z') {
			if (ch == 'z') {
				s[pos] = 'a';
				carry=1;
			} else {
				s[pos]++;
				carry=0;
			}
			last=LOWER_CASE;
		} else if (ch >= 'A' && ch <= 'Z') {
			if (ch == 'Z') {
				s[pos] = 'A';
				carry=1;
			} else {
				s[pos]++;
				carry=0;
			}
			last=UPPER_CASE;
		} else if (ch >= '0' && ch <= '9') {
			if (ch == '9') {
				s[pos] = '0';
				carry=1;
			} else {
				s[pos]++;
				carry=0;
			}
			last = NUMERIC;
		} else {
			carry=0;
			break;
		}
		if (carry == 0) {
			break;
		}
	} while (pos-- > 0);

	if (carry) {
		t = crex_string_alloc(C_STRLEN_P(str)+1, 0);
		memcpy(ZSTR_VAL(t) + 1, C_STRVAL_P(str), C_STRLEN_P(str));
		ZSTR_VAL(t)[C_STRLEN_P(str) + 1] = '\0';
		switch (last) {
			case NUMERIC:
				ZSTR_VAL(t)[0] = '1';
				break;
			case UPPER_CASE:
				ZSTR_VAL(t)[0] = 'A';
				break;
			case LOWER_CASE:
				ZSTR_VAL(t)[0] = 'a';
				break;
		}
		crex_string_free(C_STR_P(str));
		ZVAL_NEW_STR(str, t);
	}
	return true;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL increment_function(zval *op1) /* {{{ */
{
try_again:
	switch (C_TYPE_P(op1)) {
		case IS_LONG:
			fast_long_increment_function(op1);
			break;
		case IS_DOUBLE:
			C_DVAL_P(op1) = C_DVAL_P(op1) + 1;
			break;
		case IS_NULL:
			ZVAL_LONG(op1, 1);
			break;
		case IS_STRING: {
				crex_long lval;
				double dval;

				switch (is_numeric_str_function(C_STR_P(op1), &lval, &dval)) {
					case IS_LONG:
						zval_ptr_dtor_str(op1);
						if (lval == CREX_LONG_MAX) {
							/* switch to double */
							double d = (double)lval;
							ZVAL_DOUBLE(op1, d+1);
						} else {
							ZVAL_LONG(op1, lval+1);
						}
						break;
					case IS_DOUBLE:
						zval_ptr_dtor_str(op1);
						ZVAL_DOUBLE(op1, dval+1);
						break;
					default:
						/* Perl style string increment */
						increment_string(op1);
						if (EG(exception)) {
							return FAILURE;
						}
						break;
				}
			}
			break;
		case IS_FALSE:
		case IS_TRUE: {
			/* Error handler can undef/change type of op1, save it and reset it in case those cases */
			zval copy;
			ZVAL_COPY_VALUE(&copy, op1);
			crex_error(E_WARNING, "Increment on type bool has no effect, this will change in the next major version of CRX");
			zval_ptr_dtor(op1);
			ZVAL_COPY_VALUE(op1, &copy);
			if (EG(exception)) {
				return FAILURE;
			}
			break;
		}
		case IS_REFERENCE:
			op1 = C_REFVAL_P(op1);
			goto try_again;
		case IS_OBJECT: {
			if (C_OBJ_HANDLER_P(op1, do_operation)) {
				zval op2;
				ZVAL_LONG(&op2, 1);
				if (C_OBJ_HANDLER_P(op1, do_operation)(CREX_ADD, op1, op1, &op2) == SUCCESS) {
					return SUCCESS;
				}
			}
			zval tmp;
			if (C_OBJ_HT_P(op1)->cast_object(C_OBJ_P(op1), &tmp, _IS_NUMBER) == SUCCESS) {
				CREX_ASSERT(C_TYPE(tmp) == IS_LONG || C_TYPE(tmp) == IS_DOUBLE);
				zval_ptr_dtor(op1);
				ZVAL_COPY_VALUE(op1, &tmp);
				goto try_again;
			}
			CREX_FALLTHROUGH;
		}
		case IS_RESOURCE:
		case IS_ARRAY:
			crex_type_error("Cannot increment %s", crex_zval_value_name(op1));
			return FAILURE;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
	return SUCCESS;
}
/* }}} */

CREX_API crex_result CREX_FASTCALL decrement_function(zval *op1) /* {{{ */
{
	crex_long lval;
	double dval;

try_again:
	switch (C_TYPE_P(op1)) {
		case IS_LONG:
			fast_long_decrement_function(op1);
			break;
		case IS_DOUBLE:
			C_DVAL_P(op1) = C_DVAL_P(op1) - 1;
			break;
		case IS_STRING:		/* Like perl we only support string increment */
			if (C_STRLEN_P(op1) == 0) { /* consider as 0 */
				crex_error(E_DEPRECATED, "Decrement on empty string is deprecated as non-numeric");
				if (EG(exception)) {
					return FAILURE;
				}
				/* A userland error handler can change the type from string to something else */
				zval_ptr_dtor(op1);
				ZVAL_LONG(op1, -1);
				break;
			}
			switch (is_numeric_str_function(C_STR_P(op1), &lval, &dval)) {
				case IS_LONG:
					zval_ptr_dtor_str(op1);
					if (lval == CREX_LONG_MIN) {
						double d = (double)lval;
						ZVAL_DOUBLE(op1, d-1);
					} else {
						ZVAL_LONG(op1, lval-1);
					}
					break;
				case IS_DOUBLE:
					zval_ptr_dtor_str(op1);
					ZVAL_DOUBLE(op1, dval - 1);
					break;
				default: {
					/* Error handler can unset the variable */
					crex_string *zstr = C_STR_P(op1);
					crex_string_addref(zstr);
					crex_error(E_DEPRECATED, "Decrement on non-numeric string has no effect and is deprecated");
					if (EG(exception)) {
						crex_string_release(zstr);
						return FAILURE;
					}
					zval_ptr_dtor(op1);
					ZVAL_STR(op1, zstr);
				}
			}
			break;
		case IS_NULL: {
			/* Error handler can undef/change type of op1, save it and reset it in case those cases */
			zval copy;
			ZVAL_COPY_VALUE(&copy, op1);
			crex_error(E_WARNING, "Decrement on type null has no effect, this will change in the next major version of CRX");
			zval_ptr_dtor(op1);
			ZVAL_COPY_VALUE(op1, &copy);
			if (EG(exception)) {
				return FAILURE;
			}
			break;
		}
		case IS_FALSE:
		case IS_TRUE: {
			/* Error handler can undef/change type of op1, save it and reset it in case those cases */
			zval copy;
			ZVAL_COPY_VALUE(&copy, op1);
			crex_error(E_WARNING, "Decrement on type bool has no effect, this will change in the next major version of CRX");
			zval_ptr_dtor(op1);
			ZVAL_COPY_VALUE(op1, &copy);
			if (EG(exception)) {
				return FAILURE;
			}
			break;
		}
		case IS_REFERENCE:
			op1 = C_REFVAL_P(op1);
			goto try_again;
		case IS_OBJECT: {
			if (C_OBJ_HANDLER_P(op1, do_operation)) {
				zval op2;
				ZVAL_LONG(&op2, 1);
				if (C_OBJ_HANDLER_P(op1, do_operation)(CREX_SUB, op1, op1, &op2) == SUCCESS) {
					return SUCCESS;
				}
			}
			zval tmp;
			if (C_OBJ_HT_P(op1)->cast_object(C_OBJ_P(op1), &tmp, _IS_NUMBER) == SUCCESS) {
				CREX_ASSERT(C_TYPE(tmp) == IS_LONG || C_TYPE(tmp) == IS_DOUBLE);
				zval_ptr_dtor(op1);
				ZVAL_COPY_VALUE(op1, &tmp);
				goto try_again;
			}
			CREX_FALLTHROUGH;
		}
		case IS_RESOURCE:
		case IS_ARRAY:
			crex_type_error("Cannot decrement %s", crex_zval_value_name(op1));
			return FAILURE;
		EMPTY_SWITCH_DEFAULT_CASE()
	}

	return SUCCESS;
}
/* }}} */

CREX_API int CREX_FASTCALL crex_is_true(const zval *op) /* {{{ */
{
	return (int) i_crex_is_true(op);
}
/* }}} */

CREX_API bool CREX_FASTCALL crex_object_is_true(const zval *op) /* {{{ */
{
	crex_object *zobj = C_OBJ_P(op);
	zval tmp;
	if (zobj->handlers->cast_object(zobj, &tmp, _IS_BOOL) == SUCCESS) {
		return C_TYPE(tmp) == IS_TRUE;
	}
	crex_error(E_RECOVERABLE_ERROR, "Object of class %s could not be converted to bool", ZSTR_VAL(zobj->ce->name));
	return false;
}
/* }}} */

CREX_API void crex_update_current_locale(void) /* {{{ */
{
#ifdef CREX_USE_TOLOWER_L
# if defined(CREX_WIN32) && defined(_MSC_VER)
	current_locale = _get_current_locale();
# else
	current_locale = uselocale(0);
# endif
#endif
#if defined(CREX_WIN32) && defined(_MSC_VER)
	if (MB_CUR_MAX > 1) {
		unsigned int cp = ___lc_codepage_func();
		CG(variable_width_locale) = 1;
		// TODO: EUC-* are also ASCII compatible ???
		CG(ascii_compatible_locale) =
			cp == 65001; /* UTF-8 */
	} else {
		CG(variable_width_locale) = 0;
		CG(ascii_compatible_locale) = 1;
	}
#elif defined(MB_CUR_MAX)
	/* Check if current locale uses variable width encoding */
	if (MB_CUR_MAX > 1) {
#if HAVE_NL_LANGINFO
		const char *charmap = nl_langinfo(CODESET);
#else
		char buf[16];
		const char *charmap = NULL;
		const char *locale = setlocale(LC_CTYPE, NULL);

		if (locale) {
			const char *dot = strchr(locale, '.');
			const char *modifier;

			if (dot) {
				dot++;
				modifier = strchr(dot, '@');
				if (!modifier) {
					charmap = dot;
				} else if (modifier - dot < sizeof(buf)) {
					memcpy(buf, dot, modifier - dot);
                    buf[modifier - dot] = '\0';
                    charmap = buf;
				}
			}
		}
#endif
		CG(variable_width_locale) = 1;
		CG(ascii_compatible_locale) = 0;

		if (charmap) {
			size_t len = strlen(charmap);
			static const char *ascii_compatible_charmaps[] = {
				"utf-8",
				"utf8",
				// TODO: EUC-* are also ASCII compatible ???
				NULL
			};
			const char **p;
			/* Check if current locale is ASCII compatible */
			for (p = ascii_compatible_charmaps; *p; p++) {
				if (crex_binary_strcasecmp(charmap, len, *p, strlen(*p)) == 0) {
					CG(ascii_compatible_locale) = 1;
					break;
				}
			}
		}

	} else {
		CG(variable_width_locale) = 0;
		CG(ascii_compatible_locale) = 1;
	}
#else
	/* We can't determine current charset. Assume the worst case */
	CG(variable_width_locale) = 1;
	CG(ascii_compatible_locale) = 0;
#endif
}
/* }}} */

CREX_API void crex_reset_lc_ctype_locale(void)
{
	/* Use the C.UTF-8 locale so that readline can process UTF-8 input, while not interfering
	 * with single-byte locale-dependent functions used by CRX. */
	if (!setlocale(LC_CTYPE, "C.UTF-8")) {
		setlocale(LC_CTYPE, "C");
	}
}

static crex_always_inline void crex_str_tolower_impl(char *dest, const char *str, size_t length) /* {{{ */ {
	unsigned char *p = (unsigned char*)str;
	unsigned char *q = (unsigned char*)dest;
	unsigned char *end = p + length;
#ifdef HAVE_BLOCKCONV
	if (length >= BLOCKCONV_STRIDE) {
		BLOCKCONV_INIT_RANGE('A', 'Z');
		BLOCKCONV_INIT_DELTA('a' - 'A');
		do {
			BLOCKCONV_LOAD(p);
			BLOCKCONV_STORE(q);
			p += BLOCKCONV_STRIDE;
			q += BLOCKCONV_STRIDE;
		} while (p + BLOCKCONV_STRIDE <= end);
	}
#endif
	while (p < end) {
		*q++ = crex_tolower_ascii(*p++);
	}
}
/* }}} */

static crex_always_inline void crex_str_toupper_impl(char *dest, const char *str, size_t length) /* {{{ */ {
	unsigned char *p = (unsigned char*)str;
	unsigned char *q = (unsigned char*)dest;
	unsigned char *end = p + length;
#ifdef HAVE_BLOCKCONV
	if (length >= BLOCKCONV_STRIDE) {
		BLOCKCONV_INIT_RANGE('a', 'z');
		BLOCKCONV_INIT_DELTA('A' - 'a');
		do {
			BLOCKCONV_LOAD(p);
			BLOCKCONV_STORE(q);
			p += BLOCKCONV_STRIDE;
			q += BLOCKCONV_STRIDE;
		} while (p + BLOCKCONV_STRIDE <= end);
	}
#endif
	while (p < end) {
		*q++ = crex_toupper_ascii(*p++);
	}
}
/* }}} */

CREX_API char* CREX_FASTCALL crex_str_tolower_copy(char *dest, const char *source, size_t length) /* {{{ */
{
	crex_str_tolower_impl(dest, source, length);
	dest[length] = '\0';
	return dest;
}
/* }}} */

CREX_API char* CREX_FASTCALL crex_str_toupper_copy(char *dest, const char *source, size_t length) /* {{{ */
{
	crex_str_toupper_impl(dest, source, length);
	dest[length] = '\0';
	return dest;
}
/* }}} */

CREX_API char* CREX_FASTCALL crex_str_tolower_dup(const char *source, size_t length) /* {{{ */
{
	return crex_str_tolower_copy((char *)emalloc(length+1), source, length);
}
/* }}} */

CREX_API char* CREX_FASTCALL crex_str_toupper_dup(const char *source, size_t length) /* {{{ */
{
	return crex_str_toupper_copy((char *)emalloc(length+1), source, length);
}
/* }}} */

CREX_API void CREX_FASTCALL crex_str_tolower(char *str, size_t length) /* {{{ */
{
	crex_str_tolower_impl(str, (const char*)str, length);
}
/* }}} */

CREX_API void CREX_FASTCALL crex_str_toupper(char *str, size_t length) /* {{{ */
{
	crex_str_toupper_impl(str, (const char*)str, length);
}
/* }}} */


CREX_API char* CREX_FASTCALL crex_str_tolower_dup_ex(const char *source, size_t length) /* {{{ */
{
	const unsigned char *p = (const unsigned char*)source;
	const unsigned char *end = p + length;

	while (p < end) {
		if (*p != crex_tolower_ascii(*p)) {
			char *res = (char*)emalloc(length + 1);
			unsigned char *r;

			if (p != (const unsigned char*)source) {
				memcpy(res, source, p - (const unsigned char*)source);
			}
			r = (unsigned char*)p + (res - source);
			crex_str_tolower_impl((char *)r, (const char*)p, end - p);
			res[length] = '\0';
			return res;
		}
		p++;
	}
	return NULL;
}
/* }}} */

CREX_API char* CREX_FASTCALL crex_str_toupper_dup_ex(const char *source, size_t length) /* {{{ */
{
	const unsigned char *p = (const unsigned char*)source;
	const unsigned char *end = p + length;

	while (p < end) {
		if (*p != crex_toupper_ascii(*p)) {
			char *res = (char*)emalloc(length + 1);
			unsigned char *r;

			if (p != (const unsigned char*)source) {
				memcpy(res, source, p - (const unsigned char*)source);
			}
			r = (unsigned char*)p + (res - source);
			crex_str_toupper_impl((char *)r, (const char*)p, end - p);
			res[length] = '\0';
			return res;
		}
		p++;
	}
	return NULL;
}
/* }}} */

CREX_API crex_string* CREX_FASTCALL crex_string_tolower_ex(crex_string *str, bool persistent) /* {{{ */
{
	size_t length = ZSTR_LEN(str);
	unsigned char *p = (unsigned char *) ZSTR_VAL(str);
	unsigned char *end = p + length;

#ifdef HAVE_BLOCKCONV
	BLOCKCONV_INIT_RANGE('A', 'Z');
	while (p + BLOCKCONV_STRIDE <= end) {
		BLOCKCONV_LOAD(p);
		if (BLOCKCONV_FOUND()) {
			crex_string *res = crex_string_alloc(length, persistent);
			memcpy(ZSTR_VAL(res), ZSTR_VAL(str), p - (unsigned char *) ZSTR_VAL(str));
			unsigned char *q = (unsigned char*) ZSTR_VAL(res) + (p - (unsigned char*) ZSTR_VAL(str));

			/* Lowercase the chunk we already compared. */
			BLOCKCONV_INIT_DELTA('a' - 'A');
			BLOCKCONV_STORE(q);

			/* Lowercase the rest of the string. */
			p += BLOCKCONV_STRIDE;
			q += BLOCKCONV_STRIDE;
			crex_str_tolower_impl((char *) q, (const char *) p, end - p);
			ZSTR_VAL(res)[length] = '\0';
			return res;
		}
		p += BLOCKCONV_STRIDE;
	}
#endif

	while (p < end) {
		if (*p != crex_tolower_ascii(*p)) {
			crex_string *res = crex_string_alloc(length, persistent);
			memcpy(ZSTR_VAL(res), ZSTR_VAL(str), p - (unsigned char*) ZSTR_VAL(str));

			unsigned char *q = (unsigned char*) ZSTR_VAL(res) + (p - (unsigned char*) ZSTR_VAL(str));
			while (p < end) {
				*q++ = crex_tolower_ascii(*p++);
			}
			ZSTR_VAL(res)[length] = '\0';
			return res;
		}
		p++;
	}

	return crex_string_copy(str);
}
/* }}} */

CREX_API crex_string* CREX_FASTCALL crex_string_toupper_ex(crex_string *str, bool persistent) /* {{{ */
{
	size_t length = ZSTR_LEN(str);
	unsigned char *p = (unsigned char *) ZSTR_VAL(str);
	unsigned char *end = p + length;

#ifdef HAVE_BLOCKCONV
	BLOCKCONV_INIT_RANGE('a', 'z');
	while (p + BLOCKCONV_STRIDE <= end) {
		BLOCKCONV_LOAD(p);
		if (BLOCKCONV_FOUND()) {
			crex_string *res = crex_string_alloc(length, persistent);
			memcpy(ZSTR_VAL(res), ZSTR_VAL(str), p - (unsigned char *) ZSTR_VAL(str));
			unsigned char *q = (unsigned char *) ZSTR_VAL(res) + (p - (unsigned char *) ZSTR_VAL(str));

			/* Uppercase the chunk we already compared. */
			BLOCKCONV_INIT_DELTA('A' - 'a');
			BLOCKCONV_STORE(q);

			/* Uppercase the rest of the string. */
			p += BLOCKCONV_STRIDE;
			q += BLOCKCONV_STRIDE;
			crex_str_toupper_impl((char *) q, (const char *) p, end - p);
			ZSTR_VAL(res)[length] = '\0';
			return res;
		}
		p += BLOCKCONV_STRIDE;
	}
#endif

	while (p < end) {
		if (*p != crex_toupper_ascii(*p)) {
			crex_string *res = crex_string_alloc(length, persistent);
			memcpy(ZSTR_VAL(res), ZSTR_VAL(str), p - (unsigned char*) ZSTR_VAL(str));

			unsigned char *q = (unsigned char *) ZSTR_VAL(res) + (p - (unsigned char *) ZSTR_VAL(str));
			while (p < end) {
				*q++ = crex_toupper_ascii(*p++);
			}
			ZSTR_VAL(res)[length] = '\0';
			return res;
		}
		p++;
	}

	return crex_string_copy(str);
}
/* }}} */

CREX_API int CREX_FASTCALL crex_binary_strcmp(const char *s1, size_t len1, const char *s2, size_t len2) /* {{{ */
{
	int retval;

	if (s1 == s2) {
		return 0;
	}
	retval = memcmp(s1, s2, MIN(len1, len2));
	if (!retval) {
		return CREX_THREEWAY_COMPARE(len1, len2);
	} else {
		return retval;
	}
}
/* }}} */

CREX_API int CREX_FASTCALL crex_binary_strncmp(const char *s1, size_t len1, const char *s2, size_t len2, size_t length) /* {{{ */
{
	int retval;

	if (s1 == s2) {
		return 0;
	}
	retval = memcmp(s1, s2, MIN(length, MIN(len1, len2)));
	if (!retval) {
		return CREX_THREEWAY_COMPARE(MIN(length, len1), MIN(length, len2));
	} else {
		return retval;
	}
}
/* }}} */

CREX_API int CREX_FASTCALL crex_binary_strcasecmp(const char *s1, size_t len1, const char *s2, size_t len2) /* {{{ */
{
	size_t len;
	int c1, c2;

	if (s1 == s2) {
		return 0;
	}

	len = MIN(len1, len2);
	while (len--) {
		c1 = crex_tolower_ascii(*(unsigned char *)s1++);
		c2 = crex_tolower_ascii(*(unsigned char *)s2++);
		if (c1 != c2) {
			return c1 - c2;
		}
	}

	return CREX_THREEWAY_COMPARE(len1, len2);
}
/* }}} */

CREX_API int CREX_FASTCALL crex_binary_strncasecmp(const char *s1, size_t len1, const char *s2, size_t len2, size_t length) /* {{{ */
{
	size_t len;
	int c1, c2;

	if (s1 == s2) {
		return 0;
	}
	len = MIN(length, MIN(len1, len2));
	while (len--) {
		c1 = crex_tolower_ascii(*(unsigned char *)s1++);
		c2 = crex_tolower_ascii(*(unsigned char *)s2++);
		if (c1 != c2) {
			return c1 - c2;
		}
	}

	return CREX_THREEWAY_COMPARE(MIN(length, len1), MIN(length, len2));
}
/* }}} */

CREX_API int CREX_FASTCALL crex_binary_strcasecmp_l(const char *s1, size_t len1, const char *s2, size_t len2) /* {{{ */
{
	size_t len;
	int c1, c2;

	if (s1 == s2) {
		return 0;
	}

	len = MIN(len1, len2);
	while (len--) {
		c1 = crex_tolower((int)*(unsigned char *)s1++);
		c2 = crex_tolower((int)*(unsigned char *)s2++);
		if (c1 != c2) {
			return c1 - c2;
		}
	}

	return CREX_THREEWAY_COMPARE(len1, len2);
}
/* }}} */

CREX_API int CREX_FASTCALL crex_binary_strncasecmp_l(const char *s1, size_t len1, const char *s2, size_t len2, size_t length) /* {{{ */
{
	size_t len;
	int c1, c2;

	if (s1 == s2) {
		return 0;
	}
	len = MIN(length, MIN(len1, len2));
	while (len--) {
		c1 = crex_tolower((int)*(unsigned char *)s1++);
		c2 = crex_tolower((int)*(unsigned char *)s2++);
		if (c1 != c2) {
			return c1 - c2;
		}
	}

	return CREX_THREEWAY_COMPARE(MIN(length, len1), MIN(length, len2));
}
/* }}} */

CREX_API int CREX_FASTCALL crex_binary_zval_strcmp(zval *s1, zval *s2) /* {{{ */
{
	return crex_binary_strcmp(C_STRVAL_P(s1), C_STRLEN_P(s1), C_STRVAL_P(s2), C_STRLEN_P(s2));
}
/* }}} */

CREX_API int CREX_FASTCALL crex_binary_zval_strncmp(zval *s1, zval *s2, zval *s3) /* {{{ */
{
	return crex_binary_strncmp(C_STRVAL_P(s1), C_STRLEN_P(s1), C_STRVAL_P(s2), C_STRLEN_P(s2), C_LVAL_P(s3));
}
/* }}} */

CREX_API bool CREX_FASTCALL crexi_smart_streq(crex_string *s1, crex_string *s2) /* {{{ */
{
	uint8_t ret1, ret2;
	int oflow1, oflow2;
	crex_long lval1 = 0, lval2 = 0;
	double dval1 = 0.0, dval2 = 0.0;

	if ((ret1 = is_numeric_string_ex(s1->val, s1->len, &lval1, &dval1, false, &oflow1, NULL)) &&
		(ret2 = is_numeric_string_ex(s2->val, s2->len, &lval2, &dval2, false, &oflow2, NULL))) {
#if CREX_ULONG_MAX == 0xFFFFFFFF
		if (oflow1 != 0 && oflow1 == oflow2 && dval1 - dval2 == 0. &&
			((oflow1 == 1 && dval1 > 9007199254740991. /*0x1FFFFFFFFFFFFF*/)
			|| (oflow1 == -1 && dval1 < -9007199254740991.))) {
#else
		if (oflow1 != 0 && oflow1 == oflow2 && dval1 - dval2 == 0.) {
#endif
			/* both values are integers overflown to the same side, and the
			 * double comparison may have resulted in crucial accuracy lost */
			goto string_cmp;
		}
		if ((ret1 == IS_DOUBLE) || (ret2 == IS_DOUBLE)) {
			if (ret1 != IS_DOUBLE) {
				if (oflow2) {
					/* 2nd operand is integer > LONG_MAX (oflow2==1) or < LONG_MIN (-1) */
					return 0;
				}
				dval1 = (double) lval1;
			} else if (ret2 != IS_DOUBLE) {
				if (oflow1) {
					return 0;
				}
				dval2 = (double) lval2;
			} else if (dval1 == dval2 && !crex_finite(dval1)) {
				/* Both values overflowed and have the same sign,
				 * so a numeric comparison would be inaccurate */
				goto string_cmp;
			}
			return dval1 == dval2;
		} else { /* they both have to be long's */
			return lval1 == lval2;
		}
	} else {
string_cmp:
		return crex_string_equal_content(s1, s2);
	}
}
/* }}} */

CREX_API int CREX_FASTCALL crexi_smart_strcmp(crex_string *s1, crex_string *s2) /* {{{ */
{
	uint8_t ret1, ret2;
	int oflow1, oflow2;
	crex_long lval1 = 0, lval2 = 0;
	double dval1 = 0.0, dval2 = 0.0;

	if ((ret1 = is_numeric_string_ex(s1->val, s1->len, &lval1, &dval1, false, &oflow1, NULL)) &&
		(ret2 = is_numeric_string_ex(s2->val, s2->len, &lval2, &dval2, false, &oflow2, NULL))) {
#if CREX_ULONG_MAX == 0xFFFFFFFF
		if (oflow1 != 0 && oflow1 == oflow2 && dval1 - dval2 == 0. &&
			((oflow1 == 1 && dval1 > 9007199254740991. /*0x1FFFFFFFFFFFFF*/)
			|| (oflow1 == -1 && dval1 < -9007199254740991.))) {
#else
		if (oflow1 != 0 && oflow1 == oflow2 && dval1 - dval2 == 0.) {
#endif
			/* both values are integers overflowed to the same side, and the
			 * double comparison may have resulted in crucial accuracy lost */
			goto string_cmp;
		}
		if ((ret1 == IS_DOUBLE) || (ret2 == IS_DOUBLE)) {
			if (ret1 != IS_DOUBLE) {
				if (oflow2) {
					/* 2nd operand is integer > LONG_MAX (oflow2==1) or < LONG_MIN (-1) */
					return -1 * oflow2;
				}
				dval1 = (double) lval1;
			} else if (ret2 != IS_DOUBLE) {
				if (oflow1) {
					return oflow1;
				}
				dval2 = (double) lval2;
			} else if (dval1 == dval2 && !crex_finite(dval1)) {
				/* Both values overflowed and have the same sign,
				 * so a numeric comparison would be inaccurate */
				goto string_cmp;
			}
			dval1 = dval1 - dval2;
			return CREX_NORMALIZE_BOOL(dval1);
		} else { /* they both have to be long's */
			return lval1 > lval2 ? 1 : (lval1 < lval2 ? -1 : 0);
		}
	} else {
		int strcmp_ret;
string_cmp:
		strcmp_ret = crex_binary_strcmp(s1->val, s1->len, s2->val, s2->len);
		return CREX_NORMALIZE_BOOL(strcmp_ret);
	}
}
/* }}} */

static int hash_zval_compare_function(zval *z1, zval *z2) /* {{{ */
{
	return crex_compare(z1, z2);
}
/* }}} */

CREX_API int CREX_FASTCALL crex_compare_symbol_tables(HashTable *ht1, HashTable *ht2) /* {{{ */
{
	return ht1 == ht2 ? 0 : crex_hash_compare(ht1, ht2, (compare_func_t) hash_zval_compare_function, 0);
}
/* }}} */

CREX_API int CREX_FASTCALL crex_compare_arrays(zval *a1, zval *a2) /* {{{ */
{
	return crex_compare_symbol_tables(C_ARRVAL_P(a1), C_ARRVAL_P(a2));
}
/* }}} */

CREX_API int CREX_FASTCALL crex_compare_objects(zval *o1, zval *o2) /* {{{ */
{
	if (C_OBJ_P(o1) == C_OBJ_P(o2)) {
		return 0;
	}

	if (C_OBJ_HT_P(o1)->compare == NULL) {
		return 1;
	} else {
		return C_OBJ_HT_P(o1)->compare(o1, o2);
	}
}
/* }}} */

CREX_API crex_string* CREX_FASTCALL crex_long_to_str(crex_long num) /* {{{ */
{
	if ((crex_ulong)num <= 9) {
		return ZSTR_CHAR((crex_uchar)'0' + (crex_uchar)num);
	} else {
		char buf[MAX_LENGTH_OF_LONG + 1];
		char *res = crex_print_long_to_buf(buf + sizeof(buf) - 1, num);
		crex_string *str =  crex_string_init(res, buf + sizeof(buf) - 1 - res, 0);
		GC_ADD_FLAGS(str, IS_STR_VALID_UTF8);
		return str;
	}
}
/* }}} */

CREX_API crex_string* CREX_FASTCALL crex_ulong_to_str(crex_ulong num)
{
	if (num <= 9) {
		return ZSTR_CHAR((crex_uchar)'0' + (crex_uchar)num);
	} else {
		char buf[MAX_LENGTH_OF_LONG + 1];
		char *res = crex_print_ulong_to_buf(buf + sizeof(buf) - 1, num);
		crex_string *str =  crex_string_init(res, buf + sizeof(buf) - 1 - res, 0);
		GC_ADD_FLAGS(str, IS_STR_VALID_UTF8);
		return str;
	}
}

/* buf points to the END of the buffer */
static crex_always_inline char *crex_print_u64_to_buf(char *buf, uint64_t num64) {
#if SIZEOF_CREX_LONG == 8
	return crex_print_ulong_to_buf(buf, num64);
#else
	*buf = '\0';
	while (num64 > CREX_ULONG_MAX) {
		*--buf = (char) (num64 % 10) + '0';
		num64 /= 10;
	}

	crex_ulong num = (crex_ulong) num64;
	do {
		*--buf = (char) (num % 10) + '0';
		num /= 10;
	} while (num > 0);
	return buf;
#endif
}

/* buf points to the END of the buffer */
static crex_always_inline char *crex_print_i64_to_buf(char *buf, int64_t num) {
	if (num < 0) {
	    char *result = crex_print_u64_to_buf(buf, ~((uint64_t) num) + 1);
	    *--result = '-';
		return result;
	} else {
	    return crex_print_u64_to_buf(buf, num);
	}
}

CREX_API crex_string* CREX_FASTCALL crex_u64_to_str(uint64_t num)
{
	if (num <= 9) {
		return ZSTR_CHAR((crex_uchar)'0' + (crex_uchar)num);
	} else {
		char buf[20 + 1];
		char *res = crex_print_u64_to_buf(buf + sizeof(buf) - 1, num);
		crex_string *str =  crex_string_init(res, buf + sizeof(buf) - 1 - res, 0);
		GC_ADD_FLAGS(str, IS_STR_VALID_UTF8);
		return str;
	}
}

CREX_API crex_string* CREX_FASTCALL crex_i64_to_str(int64_t num)
{
	if ((uint64_t)num <= 9) {
		return ZSTR_CHAR((crex_uchar)'0' + (crex_uchar)num);
	} else {
		char buf[20 + 1];
		char *res = crex_print_i64_to_buf(buf + sizeof(buf) - 1, num);
		crex_string *str =  crex_string_init(res, buf + sizeof(buf) - 1 - res, 0);
		GC_ADD_FLAGS(str, IS_STR_VALID_UTF8);
		return str;
	}
}

CREX_API crex_string* CREX_FASTCALL crex_double_to_str(double num)
{
	char buf[CREX_DOUBLE_MAX_LENGTH];
	/* Model snprintf precision behavior. */
	int precision = (int) EG(precision);
	crex_gcvt(num, precision ? precision : 1, '.', 'E', buf);
	crex_string *str =  crex_string_init(buf, strlen(buf), 0);
	GC_ADD_FLAGS(str, IS_STR_VALID_UTF8);
	return str;
}

CREX_API uint8_t CREX_FASTCALL is_numeric_str_function(const crex_string *str, crex_long *lval, double *dval) /* {{{ */
{
	return is_numeric_string(ZSTR_VAL(str), ZSTR_LEN(str), lval, dval, false);
}
/* }}} */

CREX_API uint8_t CREX_FASTCALL _is_numeric_string_ex(const char *str, size_t length, crex_long *lval,
	double *dval, bool allow_errors, int *oflow_info, bool *trailing_data) /* {{{ */
{
	const char *ptr;
	int digits = 0, dp_or_e = 0;
	double local_dval = 0.0;
	uint8_t type;
	crex_ulong tmp_lval = 0;
	int neg = 0;

	if (!length) {
		return 0;
	}

	if (oflow_info != NULL) {
		*oflow_info = 0;
	}
	if (trailing_data != NULL) {
		*trailing_data = false;
	}

	/* Skip any whitespace
	 * This is much faster than the isspace() function */
	while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r' || *str == '\v' || *str == '\f') {
		str++;
		length--;
	}
	ptr = str;

	if (*ptr == '-') {
		neg = 1;
		ptr++;
	} else if (*ptr == '+') {
		ptr++;
	}

	if (CREX_IS_DIGIT(*ptr)) {
		/* Skip any leading 0s */
		while (*ptr == '0') {
			ptr++;
		}

		/* Count the number of digits. If a decimal point/exponent is found,
		 * it's a double. Otherwise, if there's a dval or no need to check for
		 * a full match, stop when there are too many digits for a long */
		for (type = IS_LONG; !(digits >= MAX_LENGTH_OF_LONG && (dval || allow_errors)); digits++, ptr++) {
check_digits:
			if (CREX_IS_DIGIT(*ptr)) {
				tmp_lval = tmp_lval * 10 + (*ptr) - '0';
				continue;
			} else if (*ptr == '.' && dp_or_e < 1) {
				goto process_double;
			} else if ((*ptr == 'e' || *ptr == 'E') && dp_or_e < 2) {
				const char *e = ptr + 1;

				if (*e == '-' || *e == '+') {
					ptr = e++;
				}
				if (CREX_IS_DIGIT(*e)) {
					goto process_double;
				}
			}

			break;
		}

		if (digits >= MAX_LENGTH_OF_LONG) {
			if (oflow_info != NULL) {
				*oflow_info = *str == '-' ? -1 : 1;
			}
			dp_or_e = -1;
			goto process_double;
		}
	} else if (*ptr == '.' && CREX_IS_DIGIT(ptr[1])) {
process_double:
		type = IS_DOUBLE;

		/* If there's a dval, do the conversion; else continue checking
		 * the digits if we need to check for a full match */
		if (dval) {
			local_dval = crex_strtod(str, &ptr);
		} else if (!allow_errors && dp_or_e != -1) {
			dp_or_e = (*ptr++ == '.') ? 1 : 2;
			goto check_digits;
		}
	} else {
		return 0;
	}

	if (ptr != str + length) {
		const char *endptr = ptr;
		while (*endptr == ' ' || *endptr == '\t' || *endptr == '\n' || *endptr == '\r' || *endptr == '\v' || *endptr == '\f') {
			endptr++;
			length--;
		}
		if (ptr != str + length) {
			if (!allow_errors) {
				return 0;
			}
			if (trailing_data != NULL) {
				*trailing_data = true;
			}
		}
	}

	if (type == IS_LONG) {
		if (digits == MAX_LENGTH_OF_LONG - 1) {
			int cmp = strcmp(&ptr[-digits], long_min_digits);

			if (!(cmp < 0 || (cmp == 0 && *str == '-'))) {
				if (dval) {
					*dval = crex_strtod(str, NULL);
				}
				if (oflow_info != NULL) {
					*oflow_info = *str == '-' ? -1 : 1;
				}

				return IS_DOUBLE;
			}
		}

		if (lval) {
			if (neg) {
				tmp_lval = -tmp_lval;
			}
			*lval = (crex_long) tmp_lval;
		}

		return IS_LONG;
	} else {
		if (dval) {
			*dval = local_dval;
		}

		return IS_DOUBLE;
	}
}
/* }}} */

/*
 * String matching - Sunday algorithm
 * http://www.iti.fh-flensburg.de/lang/algorithmen/pattern/sundayen.htm
 */
static crex_always_inline void crex_memnstr_ex_pre(unsigned int td[], const char *needle, size_t needle_len, int reverse) /* {{{ */ {
	int i;

	for (i = 0; i < 256; i++) {
		td[i] = needle_len + 1;
	}

	if (reverse) {
		for (i = needle_len - 1; i >= 0; i--) {
			td[(unsigned char)needle[i]] = i + 1;
		}
	} else {
		size_t i;

		for (i = 0; i < needle_len; i++) {
			td[(unsigned char)needle[i]] = (int)needle_len - i;
		}
	}
}
/* }}} */

CREX_API const char* CREX_FASTCALL crex_memnstr_ex(const char *haystack, const char *needle, size_t needle_len, const char *end) /* {{{ */
{
	unsigned int td[256];
	size_t i;
	const char *p;

	if (needle_len == 0 || (end - haystack) < needle_len) {
		return NULL;
	}

	crex_memnstr_ex_pre(td, needle, needle_len, 0);

	p = haystack;
	end -= needle_len;

	while (p <= end) {
		for (i = 0; i < needle_len; i++) {
			if (needle[i] != p[i]) {
				break;
			}
		}
		if (i == needle_len) {
			return p;
		}
		if (UNEXPECTED(p == end)) {
			return NULL;
		}
		p += td[(unsigned char)(p[needle_len])];
	}

	return NULL;
}
/* }}} */

CREX_API const char* CREX_FASTCALL crex_memnrstr_ex(const char *haystack, const char *needle, size_t needle_len, const char *end) /* {{{ */
{
	unsigned int td[256];
	size_t i;
	const char *p;

	if (needle_len == 0 || (end - haystack) < needle_len) {
		return NULL;
	}

	crex_memnstr_ex_pre(td, needle, needle_len, 1);

	p = end;
	p -= needle_len;

	while (p >= haystack) {
		for (i = 0; i < needle_len; i++) {
			if (needle[i] != p[i]) {
				break;
			}
		}

		if (i == needle_len) {
			return (const char *)p;
		}

		if (UNEXPECTED(p == haystack)) {
			return NULL;
		}

		p -= td[(unsigned char)(p[-1])];
	}

	return NULL;
}
/* }}} */

#if SIZEOF_CREX_LONG == 4
CREX_API crex_long CREX_FASTCALL crex_dval_to_lval_slow(double d) /* {{{ */
{
	double	two_pow_32 = pow(2., 32.),
			dmod;

	dmod = fmod(d, two_pow_32);
	if (dmod < 0) {
		/* we're going to make this number positive; call ceil()
		 * to simulate rounding towards 0 of the negative number */
		dmod = ceil(dmod) + two_pow_32;
	}
	return (crex_long)(crex_ulong)dmod;
}
#else
CREX_API crex_long CREX_FASTCALL crex_dval_to_lval_slow(double d)
{
	double	two_pow_64 = pow(2., 64.),
			dmod;

	dmod = fmod(d, two_pow_64);
	if (dmod < 0) {
		/* no need to call ceil; original double must have had no
		 * fractional part, hence dmod does not have one either */
		dmod += two_pow_64;
	}
	return (crex_long)(crex_ulong)dmod;
}
/* }}} */
#endif
