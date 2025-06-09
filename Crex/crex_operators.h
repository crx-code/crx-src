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

#ifndef CREX_OPERATORS_H
#define CREX_OPERATORS_H

#include <errno.h>
#include <math.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif

#include "crex_portability.h"
#include "crex_strtod.h"
#include "crex_multiply.h"
#include "crex_object_handlers.h"

#define LONG_SIGN_MASK CREX_LONG_MIN

BEGIN_EXTERN_C()
CREX_API crex_result CREX_FASTCALL add_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL sub_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL mul_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL pow_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL div_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL mod_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL boolean_xor_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL boolean_not_function(zval *result, zval *op1);
CREX_API crex_result CREX_FASTCALL bitwise_not_function(zval *result, zval *op1);
CREX_API crex_result CREX_FASTCALL bitwise_or_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL bitwise_and_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL bitwise_xor_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL shift_left_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL shift_right_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL concat_function(zval *result, zval *op1, zval *op2);

CREX_API bool CREX_FASTCALL crex_is_identical(const zval *op1, const zval *op2);

CREX_API crex_result CREX_FASTCALL is_equal_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL is_identical_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL is_not_identical_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL is_not_equal_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL is_smaller_function(zval *result, zval *op1, zval *op2);
CREX_API crex_result CREX_FASTCALL is_smaller_or_equal_function(zval *result, zval *op1, zval *op2);

CREX_API bool CREX_FASTCALL crex_class_implements_interface(const crex_class_entry *class_ce, const crex_class_entry *interface_ce);
CREX_API bool CREX_FASTCALL instanceof_function_slow(const crex_class_entry *instance_ce, const crex_class_entry *ce);

static crex_always_inline bool instanceof_function(
		const crex_class_entry *instance_ce, const crex_class_entry *ce) {
	return instance_ce == ce || instanceof_function_slow(instance_ce, ce);
}

CREX_API bool crex_string_only_has_ascii_alphanumeric(const crex_string *str);

/**
 * Checks whether the string "str" with length "length" is numeric. The value
 * of allow_errors determines whether it's required to be entirely numeric, or
 * just its prefix. Leading whitespace is allowed.
 *
 * The function returns 0 if the string did not contain a valid number; IS_LONG
 * if it contained a number that fits within the range of a long; or IS_DOUBLE
 * if the number was out of long range or contained a decimal point/exponent.
 * The number's value is returned into the respective pointer, *lval or *dval,
 * if that pointer is not NULL.
 *
 * This variant also gives information if a string that represents an integer
 * could not be represented as such due to overflow. It writes 1 to oflow_info
 * if the integer is larger than CREX_LONG_MAX and -1 if it's smaller than CREX_LONG_MIN.
 */
CREX_API uint8_t CREX_FASTCALL _is_numeric_string_ex(const char *str, size_t length, crex_long *lval,
	double *dval, bool allow_errors, int *oflow_info, bool *trailing_data);

CREX_API const char* CREX_FASTCALL crex_memnstr_ex(const char *haystack, const char *needle, size_t needle_len, const char *end);
CREX_API const char* CREX_FASTCALL crex_memnrstr_ex(const char *haystack, const char *needle, size_t needle_len, const char *end);

#if SIZEOF_CREX_LONG == 4
#	define CREX_DOUBLE_FITS_LONG(d) (!((d) > (double)CREX_LONG_MAX || (d) < (double)CREX_LONG_MIN))
#else
	/* >= as (double)CREX_LONG_MAX is outside signed range */
#	define CREX_DOUBLE_FITS_LONG(d) (!((d) >= (double)CREX_LONG_MAX || (d) < (double)CREX_LONG_MIN))
#endif

CREX_API crex_long CREX_FASTCALL crex_dval_to_lval_slow(double d);

static crex_always_inline crex_long crex_dval_to_lval(double d)
{
	if (UNEXPECTED(!crex_finite(d)) || UNEXPECTED(crex_isnan(d))) {
		return 0;
	} else if (!CREX_DOUBLE_FITS_LONG(d)) {
		return crex_dval_to_lval_slow(d);
	}
	return (crex_long)d;
}

/* Used to convert a string float to integer during an (int) cast */
static crex_always_inline crex_long crex_dval_to_lval_cap(double d)
{
	if (UNEXPECTED(!crex_finite(d)) || UNEXPECTED(crex_isnan(d))) {
		return 0;
	} else if (!CREX_DOUBLE_FITS_LONG(d)) {
		return (d > 0 ? CREX_LONG_MAX : CREX_LONG_MIN);
	}
	return (crex_long)d;
}
/* }}} */

static crex_always_inline bool crex_is_long_compatible(double d, crex_long l) {
	return (double)l == d;
}

CREX_API void crex_incompatible_double_to_long_error(double d);
CREX_API void crex_incompatible_string_to_long_error(const crex_string *s);

static crex_always_inline crex_long crex_dval_to_lval_safe(double d)
{
	crex_long l = crex_dval_to_lval(d);
	if (!crex_is_long_compatible(d, l)) {
		crex_incompatible_double_to_long_error(d);
	}
	return l;
}

#define CREX_IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define CREX_IS_XDIGIT(c) (((c) >= 'A' && (c) <= 'F') || ((c) >= 'a' && (c) <= 'f'))

static crex_always_inline uint8_t is_numeric_string_ex(const char *str, size_t length, crex_long *lval,
	double *dval, bool allow_errors, int *oflow_info, bool *trailing_data)
{
	if (*str > '9') {
		return 0;
	}
	return _is_numeric_string_ex(str, length, lval, dval, allow_errors, oflow_info, trailing_data);
}

static crex_always_inline uint8_t is_numeric_string(const char *str, size_t length, crex_long *lval, double *dval, bool allow_errors) {
    return is_numeric_string_ex(str, length, lval, dval, allow_errors, NULL, NULL);
}

CREX_API uint8_t CREX_FASTCALL is_numeric_str_function(const crex_string *str, crex_long *lval, double *dval);

static crex_always_inline const char *
crex_memnstr(const char *haystack, const char *needle, size_t needle_len, const char *end)
{
	const char *p = haystack;
	size_t off_s;

	CREX_ASSERT(end >= p);

	if (needle_len == 1) {
		return (const char *)memchr(p, *needle, (end-p));
	} else if (UNEXPECTED(needle_len == 0)) {
		return p;
	}

	off_s = (size_t)(end - p);

	if (needle_len > off_s) {
		return NULL;
	}

	if (EXPECTED(off_s < 1024 || needle_len < 9)) {	/* glibc memchr is faster when needle is too short */
		const char ne = needle[needle_len-1];
		end -= needle_len;

		while (p <= end) {
			if ((p = (const char *)memchr(p, *needle, (end-p+1)))) {
				if (ne == p[needle_len-1] && !memcmp(needle+1, p+1, needle_len-2)) {
					return p;
				}
			} else {
				return NULL;
			}
			p++;
		}

		return NULL;
	} else {
		return crex_memnstr_ex(haystack, needle, needle_len, end);
	}
}

static crex_always_inline const void *crex_memrchr(const void *s, int c, size_t n)
{
#if defined(HAVE_MEMRCHR) && !defined(i386)
	/* On x86 memrchr() doesn't use SSE/AVX, so inlined version is faster */
	return (const void*)memrchr(s, c, n);
#else
	const unsigned char *e;
	if (0 == n) {
		return NULL;
	}

	for (e = (const unsigned char *)s + n - 1; e >= (const unsigned char *)s; e--) {
		if (*e == (unsigned char)c) {
			return (const void *)e;
		}
	}
	return NULL;
#endif
}


static crex_always_inline const char *
crex_memnrstr(const char *haystack, const char *needle, size_t needle_len, const char *end)
{
    const char *p = end;
    ptrdiff_t off_p;
    size_t off_s;

	if (needle_len == 0) {
		return p;
	}

    if (needle_len == 1) {
        return (const char *)crex_memrchr(haystack, *needle, (p - haystack));
    }

    off_p = end - haystack;
    off_s = (off_p > 0) ? (size_t)off_p : 0;

    if (needle_len > off_s) {
        return NULL;
    }

	if (EXPECTED(off_s < 1024 || needle_len < 3)) {
		const char ne = needle[needle_len-1];
		p -= needle_len;

		do {
			p = (const char *)crex_memrchr(haystack, *needle, (p - haystack) + 1);
			if (!p) {
				return NULL;
			}
			if (ne == p[needle_len-1] && !memcmp(needle + 1, p + 1, needle_len - 2)) {
				return p;
			}
		} while (p-- >= haystack);

		return NULL;
	} else {
		return crex_memnrstr_ex(haystack, needle, needle_len, end);
	}
}

static crex_always_inline size_t crex_strnlen(const char* s, size_t maxlen)
{
#if defined(HAVE_STRNLEN)
	return strnlen(s, maxlen);
#else
	const char *p = memchr(s, '\0', maxlen);
	return p ? p-s : maxlen;
#endif
}

CREX_API crex_result CREX_FASTCALL increment_function(zval *op1);
CREX_API crex_result CREX_FASTCALL decrement_function(zval *op2);

CREX_API void CREX_FASTCALL convert_scalar_to_number(zval *op);
CREX_API void CREX_FASTCALL _convert_to_string(zval *op);
CREX_API void CREX_FASTCALL convert_to_long(zval *op);
CREX_API void CREX_FASTCALL convert_to_double(zval *op);
CREX_API void CREX_FASTCALL convert_to_null(zval *op);
CREX_API void CREX_FASTCALL convert_to_boolean(zval *op);
CREX_API void CREX_FASTCALL convert_to_array(zval *op);
CREX_API void CREX_FASTCALL convert_to_object(zval *op);

CREX_API crex_long    CREX_FASTCALL zval_get_long_func(const zval *op, bool is_strict);
CREX_API crex_long    CREX_FASTCALL zval_try_get_long(const zval *op, bool *failed);
CREX_API double       CREX_FASTCALL zval_get_double_func(const zval *op);
CREX_API crex_string* CREX_FASTCALL zval_get_string_func(zval *op);
CREX_API crex_string* CREX_FASTCALL zval_try_get_string_func(zval *op);

static crex_always_inline crex_long zval_get_long(const zval *op) {
	return EXPECTED(C_TYPE_P(op) == IS_LONG) ? C_LVAL_P(op) : zval_get_long_func(op, false);
}
static crex_always_inline crex_long zval_get_long_ex(const zval *op, bool is_strict) {
	return EXPECTED(C_TYPE_P(op) == IS_LONG) ? C_LVAL_P(op) : zval_get_long_func(op, is_strict);
}
static crex_always_inline double zval_get_double(const zval *op) {
	return EXPECTED(C_TYPE_P(op) == IS_DOUBLE) ? C_DVAL_P(op) : zval_get_double_func(op);
}
static crex_always_inline crex_string *zval_get_string(zval *op) {
	return EXPECTED(C_TYPE_P(op) == IS_STRING) ? crex_string_copy(C_STR_P(op)) : zval_get_string_func(op);
}

static crex_always_inline crex_string *zval_get_tmp_string(zval *op, crex_string **tmp) {
	if (EXPECTED(C_TYPE_P(op) == IS_STRING)) {
		*tmp = NULL;
		return C_STR_P(op);
	} else {
		return *tmp = zval_get_string_func(op);
	}
}
static crex_always_inline void crex_tmp_string_release(crex_string *tmp) {
	if (UNEXPECTED(tmp)) {
		crex_string_release_ex(tmp, 0);
	}
}

/* Like zval_get_string, but returns NULL if the conversion fails with an exception. */
static crex_always_inline crex_string *zval_try_get_string(zval *op) {
	if (EXPECTED(C_TYPE_P(op) == IS_STRING)) {
		crex_string *ret = crex_string_copy(C_STR_P(op));
		CREX_ASSUME(ret != NULL);
		return ret;
	} else {
		return zval_try_get_string_func(op);
	}
}

/* Like zval_get_tmp_string, but returns NULL if the conversion fails with an exception. */
static crex_always_inline crex_string *zval_try_get_tmp_string(zval *op, crex_string **tmp) {
	if (EXPECTED(C_TYPE_P(op) == IS_STRING)) {
		crex_string *ret = C_STR_P(op);
		*tmp = NULL;
		CREX_ASSUME(ret != NULL);
		return ret;
	} else {
		return *tmp = zval_try_get_string_func(op);
	}
}

/* Like convert_to_string(), but returns whether the conversion succeeded and does not modify the
 * zval in-place if it fails. */
CREX_API bool CREX_FASTCALL _try_convert_to_string(zval *op);
static crex_always_inline bool try_convert_to_string(zval *op) {
	if (C_TYPE_P(op) == IS_STRING) {
		return 1;
	}
	return _try_convert_to_string(op);
}

/* Compatibility macros for 7.2 and below */
#define _zval_get_long(op) zval_get_long(op)
#define _zval_get_double(op) zval_get_double(op)
#define _zval_get_string(op) zval_get_string(op)
#define _zval_get_long_func(op) zval_get_long_func(op)
#define _zval_get_double_func(op) zval_get_double_func(op)
#define _zval_get_string_func(op) zval_get_string_func(op)

#define convert_to_string(op) if (C_TYPE_P(op) != IS_STRING) { _convert_to_string((op)); }


CREX_API int CREX_FASTCALL crex_is_true(const zval *op);
CREX_API bool CREX_FASTCALL crex_object_is_true(const zval *op);

#define zval_is_true(op) \
	crex_is_true(op)

static crex_always_inline bool i_crex_is_true(const zval *op)
{
	bool result = 0;

again:
	switch (C_TYPE_P(op)) {
		case IS_TRUE:
			result = 1;
			break;
		case IS_LONG:
			if (C_LVAL_P(op)) {
				result = 1;
			}
			break;
		case IS_DOUBLE:
			if (C_DVAL_P(op)) {
				result = 1;
			}
			break;
		case IS_STRING:
			if (C_STRLEN_P(op) > 1 || (C_STRLEN_P(op) && C_STRVAL_P(op)[0] != '0')) {
				result = 1;
			}
			break;
		case IS_ARRAY:
			if (crex_hash_num_elements(C_ARRVAL_P(op))) {
				result = 1;
			}
			break;
		case IS_OBJECT:
			if (EXPECTED(C_OBJ_HT_P(op)->cast_object == crex_std_cast_object_tostring)) {
				result = 1;
			} else {
				result = crex_object_is_true(op);
			}
			break;
		case IS_RESOURCE:
			if (EXPECTED(C_RES_HANDLE_P(op))) {
				result = 1;
			}
			break;
		case IS_REFERENCE:
			op = C_REFVAL_P(op);
			goto again;
			break;
		default:
			break;
	}
	return result;
}

/* Indicate that two values cannot be compared. This value should be returned for both orderings
 * of the operands. This implies that all of ==, <, <= and >, >= will return false, because we
 * canonicalize >/>= to </<= with swapped operands. */
// TODO: Use a different value to allow an actual distinction here.
#define CREX_UNCOMPARABLE 1

CREX_API int CREX_FASTCALL crex_compare(zval *op1, zval *op2);

CREX_API crex_result CREX_FASTCALL compare_function(zval *result, zval *op1, zval *op2);

CREX_API int CREX_FASTCALL numeric_compare_function(zval *op1, zval *op2);
CREX_API int CREX_FASTCALL string_compare_function_ex(zval *op1, zval *op2, bool case_insensitive);
CREX_API int CREX_FASTCALL string_compare_function(zval *op1, zval *op2);
CREX_API int CREX_FASTCALL string_case_compare_function(zval *op1, zval *op2);
CREX_API int CREX_FASTCALL string_locale_compare_function(zval *op1, zval *op2);

CREX_API extern const unsigned char crex_tolower_map[256];
CREX_API extern const unsigned char crex_toupper_map[256];

#define crex_tolower_ascii(c) (crex_tolower_map[(unsigned char)(c)])
#define crex_toupper_ascii(c) (crex_toupper_map[(unsigned char)(c)])

CREX_API void         CREX_FASTCALL crex_str_tolower(char *str, size_t length);
CREX_API void         CREX_FASTCALL crex_str_toupper(char *str, size_t length);
CREX_API char*        CREX_FASTCALL crex_str_tolower_copy(char *dest, const char *source, size_t length);
CREX_API char*        CREX_FASTCALL crex_str_toupper_copy(char *dest, const char *source, size_t length);
CREX_API char*        CREX_FASTCALL crex_str_tolower_dup(const char *source, size_t length);
CREX_API char*        CREX_FASTCALL crex_str_toupper_dup(const char *source, size_t length);
CREX_API char*        CREX_FASTCALL crex_str_tolower_dup_ex(const char *source, size_t length);
CREX_API char*        CREX_FASTCALL crex_str_toupper_dup_ex(const char *source, size_t length);
CREX_API crex_string* CREX_FASTCALL crex_string_tolower_ex(crex_string *str, bool persistent);
CREX_API crex_string* CREX_FASTCALL crex_string_toupper_ex(crex_string *str, bool persistent);

static crex_always_inline crex_string* crex_string_tolower(crex_string *str) {
	return crex_string_tolower_ex(str, false);
}
static crex_always_inline crex_string* crex_string_toupper(crex_string *str) {
	return crex_string_toupper_ex(str, false);
}

CREX_API int CREX_FASTCALL crex_binary_zval_strcmp(zval *s1, zval *s2);
CREX_API int CREX_FASTCALL crex_binary_zval_strncmp(zval *s1, zval *s2, zval *s3);
CREX_API int CREX_FASTCALL crex_binary_strcmp(const char *s1, size_t len1, const char *s2, size_t len2);
CREX_API int CREX_FASTCALL crex_binary_strncmp(const char *s1, size_t len1, const char *s2, size_t len2, size_t length);
CREX_API int CREX_FASTCALL crex_binary_strcasecmp(const char *s1, size_t len1, const char *s2, size_t len2);
CREX_API int CREX_FASTCALL crex_binary_strncasecmp(const char *s1, size_t len1, const char *s2, size_t len2, size_t length);
CREX_API int CREX_FASTCALL crex_binary_strcasecmp_l(const char *s1, size_t len1, const char *s2, size_t len2);
CREX_API int CREX_FASTCALL crex_binary_strncasecmp_l(const char *s1, size_t len1, const char *s2, size_t len2, size_t length);

CREX_API bool CREX_FASTCALL crexi_smart_streq(crex_string *s1, crex_string *s2);
CREX_API int CREX_FASTCALL crexi_smart_strcmp(crex_string *s1, crex_string *s2);
CREX_API int CREX_FASTCALL crex_compare_symbol_tables(HashTable *ht1, HashTable *ht2);
CREX_API int CREX_FASTCALL crex_compare_arrays(zval *a1, zval *a2);
CREX_API int CREX_FASTCALL crex_compare_objects(zval *o1, zval *o2);

/** Deprecatd in favor of CREX_STRTOL() */
CREX_ATTRIBUTE_DEPRECATED CREX_API int CREX_FASTCALL crex_atoi(const char *str, size_t str_len);

/** Deprecatd in favor of CREX_STRTOL() */
CREX_ATTRIBUTE_DEPRECATED CREX_API crex_long CREX_FASTCALL crex_atol(const char *str, size_t str_len);

#define convert_to_null_ex(zv) convert_to_null(zv)
#define convert_to_boolean_ex(zv) convert_to_boolean(zv)
#define convert_to_long_ex(zv) convert_to_long(zv)
#define convert_to_double_ex(zv) convert_to_double(zv)
#define convert_to_string_ex(zv) convert_to_string(zv)
#define convert_to_array_ex(zv) convert_to_array(zv)
#define convert_to_object_ex(zv) convert_to_object(zv)
#define convert_scalar_to_number_ex(zv) convert_scalar_to_number(zv)

CREX_API void crex_update_current_locale(void);

CREX_API void crex_reset_lc_ctype_locale(void);

/* The offset in bytes between the value and type fields of a zval */
#define ZVAL_OFFSETOF_TYPE	\
	(offsetof(zval, u1.type_info) - offsetof(zval, value))

#if defined(HAVE_ASM_GOTO) && !__has_feature(memory_sanitizer)
# define CREX_USE_ASM_ARITHMETIC 1
#else
# define CREX_USE_ASM_ARITHMETIC 0
#endif

static crex_always_inline void fast_long_increment_function(zval *op1)
{
#if CREX_USE_ASM_ARITHMETIC && defined(__i386__) && !(4 == __GNUC__ && 8 == __GNUC_MINOR__)
	__asm__ goto(
		"addl $1,(%0)\n\t"
		"jo  %l1\n"
		:
		: "r"(&op1->value)
		: "cc", "memory"
		: overflow);
	return;
overflow: CREX_ATTRIBUTE_COLD_LABEL
	ZVAL_DOUBLE(op1, (double)CREX_LONG_MAX + 1.0);
#elif CREX_USE_ASM_ARITHMETIC && defined(__x86_64__)
	__asm__ goto(
		"addq $1,(%0)\n\t"
		"jo  %l1\n"
		:
		: "r"(&op1->value)
		: "cc", "memory"
		: overflow);
	return;
overflow: CREX_ATTRIBUTE_COLD_LABEL
	ZVAL_DOUBLE(op1, (double)CREX_LONG_MAX + 1.0);
#elif CREX_USE_ASM_ARITHMETIC && defined(__aarch64__)
	__asm__ goto (
		"ldr x5, [%0]\n\t"
		"adds x5, x5, 1\n\t"
		"bvs %l1\n"
		"str x5, [%0]"
		:
		: "r"(&op1->value)
		: "x5", "cc", "memory"
		: overflow);
	return;
overflow: CREX_ATTRIBUTE_COLD_LABEL
	ZVAL_DOUBLE(op1, (double)CREX_LONG_MAX + 1.0);
#elif CRX_HAVE_BUILTIN_SADDL_OVERFLOW && SIZEOF_LONG == SIZEOF_CREX_LONG
	long lresult;
	if (UNEXPECTED(__builtin_saddl_overflow(C_LVAL_P(op1), 1, &lresult))) {
		/* switch to double */
		ZVAL_DOUBLE(op1, (double)CREX_LONG_MAX + 1.0);
	} else {
		C_LVAL_P(op1) = lresult;
	}
#elif CRX_HAVE_BUILTIN_SADDLL_OVERFLOW && SIZEOF_LONG_LONG == SIZEOF_CREX_LONG
	long long llresult;
	if (UNEXPECTED(__builtin_saddll_overflow(C_LVAL_P(op1), 1, &llresult))) {
		/* switch to double */
		ZVAL_DOUBLE(op1, (double)CREX_LONG_MAX + 1.0);
	} else {
		C_LVAL_P(op1) = llresult;
	}
#else
	if (UNEXPECTED(C_LVAL_P(op1) == CREX_LONG_MAX)) {
		/* switch to double */
		ZVAL_DOUBLE(op1, (double)CREX_LONG_MAX + 1.0);
	} else {
		C_LVAL_P(op1)++;
	}
#endif
}

static crex_always_inline void fast_long_decrement_function(zval *op1)
{
#if CREX_USE_ASM_ARITHMETIC && defined(__i386__) && !(4 == __GNUC__ && 8 == __GNUC_MINOR__)
	__asm__ goto(
		"subl $1,(%0)\n\t"
		"jo  %l1\n"
		:
		: "r"(&op1->value)
		: "cc", "memory"
		: overflow);
	return;
overflow: CREX_ATTRIBUTE_COLD_LABEL
	ZVAL_DOUBLE(op1, (double)CREX_LONG_MIN - 1.0);
#elif CREX_USE_ASM_ARITHMETIC && defined(__x86_64__)
	__asm__ goto(
		"subq $1,(%0)\n\t"
		"jo  %l1\n"
		:
		: "r"(&op1->value)
		: "cc", "memory"
		: overflow);
	return;
overflow: CREX_ATTRIBUTE_COLD_LABEL
	ZVAL_DOUBLE(op1, (double)CREX_LONG_MIN - 1.0);
#elif CREX_USE_ASM_ARITHMETIC && defined(__aarch64__)
	__asm__ goto (
		"ldr x5, [%0]\n\t"
		"subs x5 ,x5, 1\n\t"
		"bvs %l1\n"
		"str x5, [%0]"
		:
		: "r"(&op1->value)
		: "x5", "cc", "memory"
		: overflow);
	return;
overflow: CREX_ATTRIBUTE_COLD_LABEL
	ZVAL_DOUBLE(op1, (double)CREX_LONG_MIN - 1.0);
#elif CRX_HAVE_BUILTIN_SSUBL_OVERFLOW && SIZEOF_LONG == SIZEOF_CREX_LONG
	long lresult;
	if (UNEXPECTED(__builtin_ssubl_overflow(C_LVAL_P(op1), 1, &lresult))) {
		/* switch to double */
		ZVAL_DOUBLE(op1, (double)CREX_LONG_MIN - 1.0);
	} else {
		C_LVAL_P(op1) = lresult;
	}
#elif CRX_HAVE_BUILTIN_SSUBLL_OVERFLOW && SIZEOF_LONG_LONG == SIZEOF_CREX_LONG
	long long llresult;
	if (UNEXPECTED(__builtin_ssubll_overflow(C_LVAL_P(op1), 1, &llresult))) {
		/* switch to double */
		ZVAL_DOUBLE(op1, (double)CREX_LONG_MIN - 1.0);
	} else {
		C_LVAL_P(op1) = llresult;
	}
#else
	if (UNEXPECTED(C_LVAL_P(op1) == CREX_LONG_MIN)) {
		/* switch to double */
		ZVAL_DOUBLE(op1, (double)CREX_LONG_MIN - 1.0);
	} else {
		C_LVAL_P(op1)--;
	}
#endif
}

static crex_always_inline void fast_long_add_function(zval *result, zval *op1, zval *op2)
{
#if CREX_USE_ASM_ARITHMETIC && defined(__i386__) && !(4 == __GNUC__ && 8 == __GNUC_MINOR__)
	__asm__ goto(
		"movl	(%1), %%eax\n\t"
		"addl   (%2), %%eax\n\t"
		"jo     %l5\n\t"
		"movl   %%eax, (%0)\n\t"
		"movl   %3, %c4(%0)\n"
		:
		: "r"(&result->value),
		  "r"(&op1->value),
		  "r"(&op2->value),
		  "n"(IS_LONG),
		  "n"(ZVAL_OFFSETOF_TYPE)
		: "eax","cc", "memory"
		: overflow);
	return;
overflow: CREX_ATTRIBUTE_COLD_LABEL
	ZVAL_DOUBLE(result, (double) C_LVAL_P(op1) + (double) C_LVAL_P(op2));
#elif CREX_USE_ASM_ARITHMETIC && defined(__x86_64__)
	__asm__ goto(
		"movq	(%1), %%rax\n\t"
		"addq   (%2), %%rax\n\t"
		"jo     %l5\n\t"
		"movq   %%rax, (%0)\n\t"
		"movl   %3, %c4(%0)\n"
		:
		: "r"(&result->value),
		  "r"(&op1->value),
		  "r"(&op2->value),
		  "n"(IS_LONG),
		  "n"(ZVAL_OFFSETOF_TYPE)
		: "rax","cc", "memory"
		: overflow);
	return;
overflow: CREX_ATTRIBUTE_COLD_LABEL
	ZVAL_DOUBLE(result, (double) C_LVAL_P(op1) + (double) C_LVAL_P(op2));
#elif CREX_USE_ASM_ARITHMETIC && defined(__aarch64__)
	__asm__ goto(
		"ldr    x5, [%1]\n\t"
		"ldr    x6, [%2]\n\t"
		"adds	x5, x5, x6\n\t"
		"bvs	%l5\n\t"
		"mov	w6, %3\n\t"
		"str	x5, [%0]\n\t"
		"str	w6, [%0, %c4]\n"
		:
		: "r"(&result->value),
		  "r"(&op1->value),
		  "r"(&op2->value),
		  "n"(IS_LONG),
		  "n"(ZVAL_OFFSETOF_TYPE)
		: "x5", "x6", "cc", "memory"
		: overflow);
	return;
overflow: CREX_ATTRIBUTE_COLD_LABEL
	ZVAL_DOUBLE(result, (double) C_LVAL_P(op1) + (double) C_LVAL_P(op2));
#elif CRX_HAVE_BUILTIN_SADDL_OVERFLOW && SIZEOF_LONG == SIZEOF_CREX_LONG
	long lresult;
	if (UNEXPECTED(__builtin_saddl_overflow(C_LVAL_P(op1), C_LVAL_P(op2), &lresult))) {
		ZVAL_DOUBLE(result, (double) C_LVAL_P(op1) + (double) C_LVAL_P(op2));
	} else {
		ZVAL_LONG(result, lresult);
	}
#elif CRX_HAVE_BUILTIN_SADDLL_OVERFLOW && SIZEOF_LONG_LONG == SIZEOF_CREX_LONG
	long long llresult;
	if (UNEXPECTED(__builtin_saddll_overflow(C_LVAL_P(op1), C_LVAL_P(op2), &llresult))) {
		ZVAL_DOUBLE(result, (double) C_LVAL_P(op1) + (double) C_LVAL_P(op2));
	} else {
		ZVAL_LONG(result, llresult);
	}
#else
	/*
	 * 'result' may alias with op1 or op2, so we need to
	 * ensure that 'result' is not updated until after we
	 * have read the values of op1 and op2.
	 */

	if (UNEXPECTED((C_LVAL_P(op1) & LONG_SIGN_MASK) == (C_LVAL_P(op2) & LONG_SIGN_MASK)
		&& (C_LVAL_P(op1) & LONG_SIGN_MASK) != ((C_LVAL_P(op1) + C_LVAL_P(op2)) & LONG_SIGN_MASK))) {
		ZVAL_DOUBLE(result, (double) C_LVAL_P(op1) + (double) C_LVAL_P(op2));
	} else {
		ZVAL_LONG(result, C_LVAL_P(op1) + C_LVAL_P(op2));
	}
#endif
}

static crex_always_inline void fast_long_sub_function(zval *result, zval *op1, zval *op2)
{
#if CREX_USE_ASM_ARITHMETIC && defined(__i386__) && !(4 == __GNUC__ && 8 == __GNUC_MINOR__)
	__asm__ goto(
		"movl	(%1), %%eax\n\t"
		"subl   (%2), %%eax\n\t"
		"jo     %l5\n\t"
		"movl   %%eax, (%0)\n\t"
		"movl   %3, %c4(%0)\n"
		:
		: "r"(&result->value),
		  "r"(&op1->value),
		  "r"(&op2->value),
		  "n"(IS_LONG),
		  "n"(ZVAL_OFFSETOF_TYPE)
		: "eax","cc", "memory"
		: overflow);
	return;
overflow: CREX_ATTRIBUTE_COLD_LABEL
	ZVAL_DOUBLE(result, (double) C_LVAL_P(op1) - (double) C_LVAL_P(op2));
#elif CREX_USE_ASM_ARITHMETIC && defined(__x86_64__)
	__asm__ goto(
		"movq	(%1), %%rax\n\t"
		"subq   (%2), %%rax\n\t"
		"jo     %l5\n\t"
		"movq   %%rax, (%0)\n\t"
		"movl   %3, %c4(%0)\n"
		:
		: "r"(&result->value),
		  "r"(&op1->value),
		  "r"(&op2->value),
		  "n"(IS_LONG),
		  "n"(ZVAL_OFFSETOF_TYPE)
		: "rax","cc", "memory"
		: overflow);
	return;
overflow: CREX_ATTRIBUTE_COLD_LABEL
	ZVAL_DOUBLE(result, (double) C_LVAL_P(op1) - (double) C_LVAL_P(op2));
#elif CREX_USE_ASM_ARITHMETIC && defined(__aarch64__)
	__asm__ goto(
		"ldr    x5, [%1]\n\t"
		"ldr    x6, [%2]\n\t"
		"subs	x5, x5, x6\n\t"
		"bvs	%l5\n\t"
		"mov	w6, %3\n\t"
		"str	x5, [%0]\n\t"
		"str	w6, [%0, %c4]\n"
		:
		: "r"(&result->value),
		  "r"(&op1->value),
		  "r"(&op2->value),
		  "n"(IS_LONG),
		  "n"(ZVAL_OFFSETOF_TYPE)
		: "x5", "x6", "cc", "memory"
		: overflow);
	return;
overflow: CREX_ATTRIBUTE_COLD_LABEL
	ZVAL_DOUBLE(result, (double) C_LVAL_P(op1) - (double) C_LVAL_P(op2));
#elif CRX_HAVE_BUILTIN_SSUBL_OVERFLOW && SIZEOF_LONG == SIZEOF_CREX_LONG
	long lresult;
	if (UNEXPECTED(__builtin_ssubl_overflow(C_LVAL_P(op1), C_LVAL_P(op2), &lresult))) {
		ZVAL_DOUBLE(result, (double) C_LVAL_P(op1) - (double) C_LVAL_P(op2));
	} else {
		ZVAL_LONG(result, lresult);
	}
#elif CRX_HAVE_BUILTIN_SSUBLL_OVERFLOW && SIZEOF_LONG_LONG == SIZEOF_CREX_LONG
	long long llresult;
	if (UNEXPECTED(__builtin_ssubll_overflow(C_LVAL_P(op1), C_LVAL_P(op2), &llresult))) {
		ZVAL_DOUBLE(result, (double) C_LVAL_P(op1) - (double) C_LVAL_P(op2));
	} else {
		ZVAL_LONG(result, llresult);
	}
#else
	ZVAL_LONG(result, C_LVAL_P(op1) - C_LVAL_P(op2));

	if (UNEXPECTED((C_LVAL_P(op1) & LONG_SIGN_MASK) != (C_LVAL_P(op2) & LONG_SIGN_MASK)
		&& (C_LVAL_P(op1) & LONG_SIGN_MASK) != (C_LVAL_P(result) & LONG_SIGN_MASK))) {
		ZVAL_DOUBLE(result, (double) C_LVAL_P(op1) - (double) C_LVAL_P(op2));
	}
#endif
}

static crex_always_inline bool crex_fast_equal_strings(crex_string *s1, crex_string *s2)
{
	if (s1 == s2) {
		return 1;
	} else if (ZSTR_VAL(s1)[0] > '9' || ZSTR_VAL(s2)[0] > '9') {
		return crex_string_equal_content(s1, s2);
	} else {
		return crexi_smart_streq(s1, s2);
	}
}

static crex_always_inline bool fast_equal_check_function(zval *op1, zval *op2)
{
	if (EXPECTED(C_TYPE_P(op1) == IS_LONG)) {
		if (EXPECTED(C_TYPE_P(op2) == IS_LONG)) {
			return C_LVAL_P(op1) == C_LVAL_P(op2);
		} else if (EXPECTED(C_TYPE_P(op2) == IS_DOUBLE)) {
			return ((double)C_LVAL_P(op1)) == C_DVAL_P(op2);
		}
	} else if (EXPECTED(C_TYPE_P(op1) == IS_DOUBLE)) {
		if (EXPECTED(C_TYPE_P(op2) == IS_DOUBLE)) {
			return C_DVAL_P(op1) == C_DVAL_P(op2);
		} else if (EXPECTED(C_TYPE_P(op2) == IS_LONG)) {
			return C_DVAL_P(op1) == ((double)C_LVAL_P(op2));
		}
	} else if (EXPECTED(C_TYPE_P(op1) == IS_STRING)) {
		if (EXPECTED(C_TYPE_P(op2) == IS_STRING)) {
			return crex_fast_equal_strings(C_STR_P(op1), C_STR_P(op2));
		}
	}
	return crex_compare(op1, op2) == 0;
}

static crex_always_inline bool fast_equal_check_long(zval *op1, zval *op2)
{
	if (EXPECTED(C_TYPE_P(op2) == IS_LONG)) {
		return C_LVAL_P(op1) == C_LVAL_P(op2);
	}
	return crex_compare(op1, op2) == 0;
}

static crex_always_inline bool fast_equal_check_string(zval *op1, zval *op2)
{
	if (EXPECTED(C_TYPE_P(op2) == IS_STRING)) {
		return crex_fast_equal_strings(C_STR_P(op1), C_STR_P(op2));
	}
	return crex_compare(op1, op2) == 0;
}

static crex_always_inline bool fast_is_identical_function(zval *op1, zval *op2)
{
	if (C_TYPE_P(op1) != C_TYPE_P(op2)) {
		return 0;
	} else if (C_TYPE_P(op1) <= IS_TRUE) {
		return 1;
	}
	return crex_is_identical(op1, op2);
}

static crex_always_inline bool fast_is_not_identical_function(zval *op1, zval *op2)
{
	if (C_TYPE_P(op1) != C_TYPE_P(op2)) {
		return 1;
	} else if (C_TYPE_P(op1) <= IS_TRUE) {
		return 0;
	}
	return !crex_is_identical(op1, op2);
}

/* buf points to the END of the buffer */
static crex_always_inline char *crex_print_ulong_to_buf(char *buf, crex_ulong num) {
	*buf = '\0';
	do {
		*--buf = (char) (num % 10) + '0';
		num /= 10;
	} while (num > 0);
	return buf;
}

/* buf points to the END of the buffer */
static crex_always_inline char *crex_print_long_to_buf(char *buf, crex_long num) {
	if (num < 0) {
	    char *result = crex_print_ulong_to_buf(buf, ~((crex_ulong) num) + 1);
	    *--result = '-';
		return result;
	} else {
	    return crex_print_ulong_to_buf(buf, num);
	}
}

CREX_API crex_string* CREX_FASTCALL crex_long_to_str(crex_long num);
CREX_API crex_string* CREX_FASTCALL crex_ulong_to_str(crex_ulong num);
CREX_API crex_string* CREX_FASTCALL crex_u64_to_str(uint64_t num);
CREX_API crex_string* CREX_FASTCALL crex_i64_to_str(int64_t num);
CREX_API crex_string* CREX_FASTCALL crex_double_to_str(double num);

static crex_always_inline void crex_unwrap_reference(zval *op) /* {{{ */
{
	if (C_REFCOUNT_P(op) == 1) {
		ZVAL_UNREF(op);
	} else {
		C_DELREF_P(op);
		ZVAL_COPY(op, C_REFVAL_P(op));
	}
}
/* }}} */

static crex_always_inline bool crex_strnieq(const char *ptr1, const char *ptr2, size_t num)
{
	const char *end = ptr1 + num;
	while (ptr1 < end) {
		if (crex_tolower_ascii(*ptr1++) != crex_tolower_ascii(*ptr2++)) {
			return 0;
		}
	}
	return 1;
}

static crex_always_inline const char *
crex_memnistr(const char *haystack, const char *needle, size_t needle_len, const char *end)
{
	CREX_ASSERT(end >= haystack);

	if (UNEXPECTED(needle_len == 0)) {
		return haystack;
	}

	if (UNEXPECTED(needle_len > (size_t)(end - haystack))) {
		return NULL;
	}

	const char first_lower = crex_tolower_ascii(*needle);
	const char first_upper = crex_toupper_ascii(*needle);
	const char *p_lower = (const char *)memchr(haystack, first_lower, end - haystack);
	const char *p_upper = NULL;
	if (first_lower != first_upper) {
		// If the needle length is 1 we don't need to look beyond p_lower as it is a guaranteed match
		size_t upper_search_length = needle_len == 1 && p_lower != NULL ? p_lower - haystack : end - haystack;
		p_upper = (const char *)memchr(haystack, first_upper, upper_search_length);
	}
	const char *p = !p_upper || (p_lower && p_lower < p_upper) ? p_lower : p_upper;

	if (needle_len == 1) {
		return p;
	}

	const char needle_end_lower = crex_tolower_ascii(needle[needle_len - 1]);
	const char needle_end_upper = crex_toupper_ascii(needle[needle_len - 1]);
	end -= needle_len;

	while (p && p <= end) {
		if (needle_end_lower == p[needle_len - 1] || needle_end_upper == p[needle_len - 1]) {
			if (crex_strnieq(needle + 1, p + 1, needle_len - 2)) {
				return p;
			}
		}
		if (p_lower == p) {
			p_lower = (const char *)memchr(p_lower + 1, first_lower, end - p_lower);
		}
		if (p_upper == p) {
			p_upper = (const char *)memchr(p_upper + 1, first_upper, end - p_upper);
		}
		p = !p_upper || (p_lower && p_lower < p_upper) ? p_lower : p_upper;
	}

	return NULL;
}


END_EXTERN_C()

#endif
