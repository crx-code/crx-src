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
   | Authors: Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_STRING_H
#define CREX_STRING_H

#include "crex.h"

BEGIN_EXTERN_C()

typedef void (*crex_string_copy_storage_func_t)(void);
typedef crex_string *(CREX_FASTCALL *crex_new_interned_string_func_t)(crex_string *str);
typedef crex_string *(CREX_FASTCALL *crex_string_init_interned_func_t)(const char *str, size_t size, bool permanent);
typedef crex_string *(CREX_FASTCALL *crex_string_init_existing_interned_func_t)(const char *str, size_t size, bool permanent);

CREX_API extern crex_new_interned_string_func_t crex_new_interned_string;
CREX_API extern crex_string_init_interned_func_t crex_string_init_interned;
/* Init an interned string if it already exists, but do not create a new one if it does not. */
CREX_API extern crex_string_init_existing_interned_func_t crex_string_init_existing_interned;

CREX_API crex_ulong CREX_FASTCALL crex_string_hash_func(crex_string *str);
CREX_API crex_ulong CREX_FASTCALL crex_hash_func(const char *str, size_t len);
CREX_API crex_string* CREX_FASTCALL crex_interned_string_find_permanent(crex_string *str);

CREX_API crex_string *crex_string_concat2(
	const char *str1, size_t str1_len,
	const char *str2, size_t str2_len);
CREX_API crex_string *crex_string_concat3(
	const char *str1, size_t str1_len,
	const char *str2, size_t str2_len,
	const char *str3, size_t str3_len);

CREX_API void crex_interned_strings_init(void);
CREX_API void crex_interned_strings_dtor(void);
CREX_API void crex_interned_strings_activate(void);
CREX_API void crex_interned_strings_deactivate(void);
CREX_API void crex_interned_strings_set_request_storage_handlers(
	crex_new_interned_string_func_t handler,
	crex_string_init_interned_func_t init_handler,
	crex_string_init_existing_interned_func_t init_existing_handler);
CREX_API void crex_interned_strings_switch_storage(bool request);

CREX_API extern crex_string  *crex_empty_string;
CREX_API extern crex_string  *crex_one_char_string[256];
CREX_API extern crex_string **crex_known_strings;

END_EXTERN_C()

/* Shortcuts */

#define ZSTR_VAL(zstr)  (zstr)->val
#define ZSTR_LEN(zstr)  (zstr)->len
#define ZSTR_H(zstr)    (zstr)->h
#define ZSTR_HASH(zstr) crex_string_hash_val(zstr)

/* Compatibility macros */

#define IS_INTERNED(s)	ZSTR_IS_INTERNED(s)
#define STR_EMPTY_ALLOC()	ZSTR_EMPTY_ALLOC()
#define _STR_HEADER_SIZE _ZSTR_HEADER_SIZE
#define STR_ALLOCA_ALLOC(str, _len, use_heap) ZSTR_ALLOCA_ALLOC(str, _len, use_heap)
#define STR_ALLOCA_INIT(str, s, len, use_heap) ZSTR_ALLOCA_INIT(str, s, len, use_heap)
#define STR_ALLOCA_FREE(str, use_heap) ZSTR_ALLOCA_FREE(str, use_heap)

/*---*/

#define ZSTR_IS_INTERNED(s)					(GC_FLAGS(s) & IS_STR_INTERNED)
#define ZSTR_IS_VALID_UTF8(s)				(GC_FLAGS(s) & IS_STR_VALID_UTF8)

/* These are properties, encoded as flags, that will hold on the resulting string
 * after concatenating two strings that have these property.
 * Example: concatenating two UTF-8 strings yields another UTF-8 string. */
#define ZSTR_COPYABLE_CONCAT_PROPERTIES		(IS_STR_VALID_UTF8)

#define ZSTR_GET_COPYABLE_CONCAT_PROPERTIES(s) 				(GC_FLAGS(s) & ZSTR_COPYABLE_CONCAT_PROPERTIES)
/* This macro returns the copyable concat properties which hold on both strings. */
#define ZSTR_GET_COPYABLE_CONCAT_PROPERTIES_BOTH(s1, s2)	(GC_FLAGS(s1) & GC_FLAGS(s2) & ZSTR_COPYABLE_CONCAT_PROPERTIES)

#define ZSTR_COPY_CONCAT_PROPERTIES(out, in) do { \
	crex_string *_out = (out); \
	uint32_t properties = ZSTR_GET_COPYABLE_CONCAT_PROPERTIES((in)); \
	GC_ADD_FLAGS(_out, properties); \
} while (0)

#define ZSTR_COPY_CONCAT_PROPERTIES_BOTH(out, in1, in2) do { \
	crex_string *_out = (out); \
	uint32_t properties = ZSTR_GET_COPYABLE_CONCAT_PROPERTIES_BOTH((in1), (in2)); \
	GC_ADD_FLAGS(_out, properties); \
} while (0)

#define ZSTR_EMPTY_ALLOC() crex_empty_string
#define ZSTR_CHAR(c) crex_one_char_string[c]
#define ZSTR_KNOWN(idx) crex_known_strings[idx]

#define _ZSTR_HEADER_SIZE XtOffsetOf(crex_string, val)

#define _ZSTR_STRUCT_SIZE(len) (_ZSTR_HEADER_SIZE + len + 1)

#define ZSTR_MAX_OVERHEAD (CREX_MM_ALIGNED_SIZE(_ZSTR_HEADER_SIZE + 1))
#define ZSTR_MAX_LEN (SIZE_MAX - ZSTR_MAX_OVERHEAD)

#define ZSTR_ALLOCA_ALLOC(str, _len, use_heap) do { \
	(str) = (crex_string *)do_alloca(CREX_MM_ALIGNED_SIZE_EX(_ZSTR_STRUCT_SIZE(_len), 8), (use_heap)); \
	GC_SET_REFCOUNT(str, 1); \
	GC_TYPE_INFO(str) = GC_STRING; \
	ZSTR_H(str) = 0; \
	ZSTR_LEN(str) = _len; \
} while (0)

#define ZSTR_ALLOCA_INIT(str, s, len, use_heap) do { \
	ZSTR_ALLOCA_ALLOC(str, len, use_heap); \
	memcpy(ZSTR_VAL(str), (s), (len)); \
	ZSTR_VAL(str)[(len)] = '\0'; \
} while (0)

#define ZSTR_ALLOCA_FREE(str, use_heap) free_alloca(str, use_heap)

#define ZSTR_INIT_LITERAL(s, persistent) (crex_string_init((s), strlen(s), (persistent)))

/*---*/

static crex_always_inline crex_ulong crex_string_hash_val(crex_string *s)
{
	return ZSTR_H(s) ? ZSTR_H(s) : crex_string_hash_func(s);
}

static crex_always_inline void crex_string_forget_hash_val(crex_string *s)
{
	ZSTR_H(s) = 0;
	GC_DEL_FLAGS(s, IS_STR_VALID_UTF8);
}

static crex_always_inline uint32_t crex_string_refcount(const crex_string *s)
{
	if (!ZSTR_IS_INTERNED(s)) {
		return GC_REFCOUNT(s);
	}
	return 1;
}

static crex_always_inline uint32_t crex_string_addref(crex_string *s)
{
	if (!ZSTR_IS_INTERNED(s)) {
		return GC_ADDREF(s);
	}
	return 1;
}

static crex_always_inline uint32_t crex_string_delref(crex_string *s)
{
	if (!ZSTR_IS_INTERNED(s)) {
		return GC_DELREF(s);
	}
	return 1;
}

static crex_always_inline crex_string *crex_string_alloc(size_t len, bool persistent)
{
	crex_string *ret = (crex_string *)pemalloc(CREX_MM_ALIGNED_SIZE(_ZSTR_STRUCT_SIZE(len)), persistent);

	GC_SET_REFCOUNT(ret, 1);
	GC_TYPE_INFO(ret) = GC_STRING | ((persistent ? IS_STR_PERSISTENT : 0) << GC_FLAGS_SHIFT);
	ZSTR_H(ret) = 0;
	ZSTR_LEN(ret) = len;
	return ret;
}

static crex_always_inline crex_string *crex_string_safe_alloc(size_t n, size_t m, size_t l, bool persistent)
{
	crex_string *ret = (crex_string *)safe_pemalloc(n, m, CREX_MM_ALIGNED_SIZE(_ZSTR_STRUCT_SIZE(l)), persistent);

	GC_SET_REFCOUNT(ret, 1);
	GC_TYPE_INFO(ret) = GC_STRING | ((persistent ? IS_STR_PERSISTENT : 0) << GC_FLAGS_SHIFT);
	ZSTR_H(ret) = 0;
	ZSTR_LEN(ret) = (n * m) + l;
	return ret;
}

static crex_always_inline crex_string *crex_string_init(const char *str, size_t len, bool persistent)
{
	crex_string *ret = crex_string_alloc(len, persistent);

	memcpy(ZSTR_VAL(ret), str, len);
	ZSTR_VAL(ret)[len] = '\0';
	return ret;
}

static crex_always_inline crex_string *crex_string_init_fast(const char *str, size_t len)
{
	if (len > 1) {
		return crex_string_init(str, len, 0);
	} else if (len == 0) {
		return crex_empty_string;
	} else /* if (len == 1) */ {
		return ZSTR_CHAR((crex_uchar) *str);
	}
}

static crex_always_inline crex_string *crex_string_copy(crex_string *s)
{
	if (!ZSTR_IS_INTERNED(s)) {
		GC_ADDREF(s);
	}
	return s;
}

static crex_always_inline crex_string *crex_string_dup(crex_string *s, bool persistent)
{
	if (ZSTR_IS_INTERNED(s)) {
		return s;
	} else {
		return crex_string_init(ZSTR_VAL(s), ZSTR_LEN(s), persistent);
	}
}

static crex_always_inline crex_string *crex_string_separate(crex_string *s, bool persistent)
{
	if (ZSTR_IS_INTERNED(s) || GC_REFCOUNT(s) > 1) {
		if (!ZSTR_IS_INTERNED(s)) {
			GC_DELREF(s);
		}
		return crex_string_init(ZSTR_VAL(s), ZSTR_LEN(s), persistent);
	}

	crex_string_forget_hash_val(s);
	return s;
}

static crex_always_inline crex_string *crex_string_realloc(crex_string *s, size_t len, bool persistent)
{
	crex_string *ret;

	if (!ZSTR_IS_INTERNED(s)) {
		if (EXPECTED(GC_REFCOUNT(s) == 1)) {
			ret = (crex_string *)perealloc(s, CREX_MM_ALIGNED_SIZE(_ZSTR_STRUCT_SIZE(len)), persistent);
			ZSTR_LEN(ret) = len;
			crex_string_forget_hash_val(ret);
			return ret;
		}
	}
	ret = crex_string_alloc(len, persistent);
	memcpy(ZSTR_VAL(ret), ZSTR_VAL(s), MIN(len, ZSTR_LEN(s)) + 1);
	if (!ZSTR_IS_INTERNED(s)) {
		GC_DELREF(s);
	}
	return ret;
}

static crex_always_inline crex_string *crex_string_extend(crex_string *s, size_t len, bool persistent)
{
	crex_string *ret;

	CREX_ASSERT(len >= ZSTR_LEN(s));
	if (!ZSTR_IS_INTERNED(s)) {
		if (EXPECTED(GC_REFCOUNT(s) == 1)) {
			ret = (crex_string *)perealloc(s, CREX_MM_ALIGNED_SIZE(_ZSTR_STRUCT_SIZE(len)), persistent);
			ZSTR_LEN(ret) = len;
			crex_string_forget_hash_val(ret);
			return ret;
		}
	}
	ret = crex_string_alloc(len, persistent);
	memcpy(ZSTR_VAL(ret), ZSTR_VAL(s), ZSTR_LEN(s) + 1);
	if (!ZSTR_IS_INTERNED(s)) {
		GC_DELREF(s);
	}
	return ret;
}

static crex_always_inline crex_string *crex_string_truncate(crex_string *s, size_t len, bool persistent)
{
	crex_string *ret;

	CREX_ASSERT(len <= ZSTR_LEN(s));
	if (!ZSTR_IS_INTERNED(s)) {
		if (EXPECTED(GC_REFCOUNT(s) == 1)) {
			ret = (crex_string *)perealloc(s, CREX_MM_ALIGNED_SIZE(_ZSTR_STRUCT_SIZE(len)), persistent);
			ZSTR_LEN(ret) = len;
			crex_string_forget_hash_val(ret);
			return ret;
		}
	}
	ret = crex_string_alloc(len, persistent);
	memcpy(ZSTR_VAL(ret), ZSTR_VAL(s), len + 1);
	if (!ZSTR_IS_INTERNED(s)) {
		GC_DELREF(s);
	}
	return ret;
}

static crex_always_inline crex_string *crex_string_safe_realloc(crex_string *s, size_t n, size_t m, size_t l, bool persistent)
{
	crex_string *ret;

	if (!ZSTR_IS_INTERNED(s)) {
		if (GC_REFCOUNT(s) == 1) {
			ret = (crex_string *)safe_perealloc(s, n, m, CREX_MM_ALIGNED_SIZE(_ZSTR_STRUCT_SIZE(l)), persistent);
			ZSTR_LEN(ret) = (n * m) + l;
			crex_string_forget_hash_val(ret);
			return ret;
		}
	}
	ret = crex_string_safe_alloc(n, m, l, persistent);
	memcpy(ZSTR_VAL(ret), ZSTR_VAL(s), MIN((n * m) + l, ZSTR_LEN(s)) + 1);
	if (!ZSTR_IS_INTERNED(s)) {
		GC_DELREF(s);
	}
	return ret;
}

static crex_always_inline void crex_string_free(crex_string *s)
{
	if (!ZSTR_IS_INTERNED(s)) {
		CREX_ASSERT(GC_REFCOUNT(s) <= 1);
		pefree(s, GC_FLAGS(s) & IS_STR_PERSISTENT);
	}
}

static crex_always_inline void crex_string_efree(crex_string *s)
{
	CREX_ASSERT(!ZSTR_IS_INTERNED(s));
	CREX_ASSERT(GC_REFCOUNT(s) <= 1);
	CREX_ASSERT(!(GC_FLAGS(s) & IS_STR_PERSISTENT));
	efree(s);
}

static crex_always_inline void crex_string_release(crex_string *s)
{
	if (!ZSTR_IS_INTERNED(s)) {
		if (GC_DELREF(s) == 0) {
			pefree(s, GC_FLAGS(s) & IS_STR_PERSISTENT);
		}
	}
}

static crex_always_inline void crex_string_release_ex(crex_string *s, bool persistent)
{
	if (!ZSTR_IS_INTERNED(s)) {
		if (GC_DELREF(s) == 0) {
			if (persistent) {
				CREX_ASSERT(GC_FLAGS(s) & IS_STR_PERSISTENT);
				free(s);
			} else {
				CREX_ASSERT(!(GC_FLAGS(s) & IS_STR_PERSISTENT));
				efree(s);
			}
		}
	}
}

static crex_always_inline bool crex_string_equals_cstr(const crex_string *s1, const char *s2, size_t s2_length)
{
	return ZSTR_LEN(s1) == s2_length && !memcmp(ZSTR_VAL(s1), s2, s2_length);
}

#if defined(__GNUC__) && (defined(__i386__) || (defined(__x86_64__) && !defined(__ILP32__)))
BEGIN_EXTERN_C()
CREX_API bool CREX_FASTCALL crex_string_equal_val(const crex_string *s1, const crex_string *s2);
END_EXTERN_C()
#else
static crex_always_inline bool crex_string_equal_val(const crex_string *s1, const crex_string *s2)
{
	return !memcmp(ZSTR_VAL(s1), ZSTR_VAL(s2), ZSTR_LEN(s1));
}
#endif

static crex_always_inline bool crex_string_equal_content(const crex_string *s1, const crex_string *s2)
{
	return ZSTR_LEN(s1) == ZSTR_LEN(s2) && crex_string_equal_val(s1, s2);
}

static crex_always_inline bool crex_string_equals(const crex_string *s1, const crex_string *s2)
{
	return s1 == s2 || crex_string_equal_content(s1, s2);
}

#define crex_string_equals_ci(s1, s2) \
	(ZSTR_LEN(s1) == ZSTR_LEN(s2) && !crex_binary_strcasecmp(ZSTR_VAL(s1), ZSTR_LEN(s1), ZSTR_VAL(s2), ZSTR_LEN(s2)))

#define crex_string_equals_literal_ci(str, c) \
	(ZSTR_LEN(str) == sizeof("" c) - 1 && !crex_binary_strcasecmp(ZSTR_VAL(str), ZSTR_LEN(str), (c), sizeof(c) - 1))

#define crex_string_equals_literal(str, literal) \
	crex_string_equals_cstr(str, "" literal, sizeof(literal) - 1)

static crex_always_inline bool crex_string_starts_with_cstr(const crex_string *str, const char *prefix, size_t prefix_length)
{
	return ZSTR_LEN(str) >= prefix_length && !memcmp(ZSTR_VAL(str), prefix, prefix_length);
}

static crex_always_inline bool crex_string_starts_with(const crex_string *str, const crex_string *prefix)
{
	return crex_string_starts_with_cstr(str, ZSTR_VAL(prefix), ZSTR_LEN(prefix));
}

#define crex_string_starts_with_literal(str, prefix) \
	crex_string_starts_with_cstr(str, prefix, strlen(prefix))

static crex_always_inline bool crex_string_starts_with_cstr_ci(const crex_string *str, const char *prefix, size_t prefix_length)
{
	return ZSTR_LEN(str) >= prefix_length && !strncasecmp(ZSTR_VAL(str), prefix, prefix_length);
}

static crex_always_inline bool crex_string_starts_with_ci(const crex_string *str, const crex_string *prefix)
{
	return crex_string_starts_with_cstr_ci(str, ZSTR_VAL(prefix), ZSTR_LEN(prefix));
}

#define crex_string_starts_with_literal_ci(str, prefix) \
	crex_string_starts_with_cstr(str, prefix, strlen(prefix))

/*
 * DJBX33A (Daniel J. Bernstein, Times 33 with Addition)
 *
 * This is Daniel J. Bernstein's popular `times 33' hash function as
 * posted by him years ago on comp.lang.c. It basically uses a function
 * like ``hash(i) = hash(i-1) * 33 + str[i]''. This is one of the best
 * known hash functions for strings. Because it is both computed very
 * fast and distributes very well.
 *
 * The magic of number 33, i.e. why it works better than many other
 * constants, prime or not, has never been adequately explained by
 * anyone. So I try an explanation: if one experimentally tests all
 * multipliers between 1 and 256 (as RSE did now) one detects that even
 * numbers are not usable at all. The remaining 128 odd numbers
 * (except for the number 1) work more or less all equally well. They
 * all distribute in an acceptable way and this way fill a hash table
 * with an average percent of approx. 86%.
 *
 * If one compares the Chi^2 values of the variants, the number 33 not
 * even has the best value. But the number 33 and a few other equally
 * good numbers like 17, 31, 63, 127 and 129 have nevertheless a great
 * advantage to the remaining numbers in the large set of possible
 * multipliers: their multiply operation can be replaced by a faster
 * operation based on just one shift plus either a single addition
 * or subtraction operation. And because a hash function has to both
 * distribute good _and_ has to be very fast to compute, those few
 * numbers should be preferred and seems to be the reason why Daniel J.
 * Bernstein also preferred it.
 *
 *
 *                  -- Ralf S. Engelschall <rse@engelschall.com>
 */

static crex_always_inline crex_ulong crex_inline_hash_func(const char *str, size_t len)
{
	crex_ulong hash = C_UL(5381);

#if defined(_WIN32) || defined(__i386__) || defined(__x86_64__) || defined(__aarch64__)
	/* Version with multiplication works better on modern CPU */
	for (; len >= 8; len -= 8, str += 8) {
# if defined(__aarch64__) && !defined(WORDS_BIGENDIAN)
		/* On some architectures it is beneficial to load 8 bytes at a
		   time and extract each byte with a bit field extract instr. */
		uint64_t chunk;

		memcpy(&chunk, str, sizeof(chunk));
		hash =
			hash                        * 33 * 33 * 33 * 33 +
			((chunk >> (8 * 0)) & 0xff) * 33 * 33 * 33 +
			((chunk >> (8 * 1)) & 0xff) * 33 * 33 +
			((chunk >> (8 * 2)) & 0xff) * 33 +
			((chunk >> (8 * 3)) & 0xff);
		hash =
			hash                        * 33 * 33 * 33 * 33 +
			((chunk >> (8 * 4)) & 0xff) * 33 * 33 * 33 +
			((chunk >> (8 * 5)) & 0xff) * 33 * 33 +
			((chunk >> (8 * 6)) & 0xff) * 33 +
			((chunk >> (8 * 7)) & 0xff);
# else
		hash =
			hash   * C_L(33 * 33 * 33 * 33) +
			str[0] * C_L(33 * 33 * 33) +
			str[1] * C_L(33 * 33) +
			str[2] * C_L(33) +
			str[3];
		hash =
			hash   * C_L(33 * 33 * 33 * 33) +
			str[4] * C_L(33 * 33 * 33) +
			str[5] * C_L(33 * 33) +
			str[6] * C_L(33) +
			str[7];
# endif
	}
	if (len >= 4) {
		hash =
			hash   * C_L(33 * 33 * 33 * 33) +
			str[0] * C_L(33 * 33 * 33) +
			str[1] * C_L(33 * 33) +
			str[2] * C_L(33) +
			str[3];
		len -= 4;
		str += 4;
	}
	if (len >= 2) {
		if (len > 2) {
			hash =
				hash   * C_L(33 * 33 * 33) +
				str[0] * C_L(33 * 33) +
				str[1] * C_L(33) +
				str[2];
		} else {
			hash =
				hash   * C_L(33 * 33) +
				str[0] * C_L(33) +
				str[1];
		}
	} else if (len != 0) {
		hash = hash * C_L(33) + *str;
	}
#else
	/* variant with the hash unrolled eight times */
	for (; len >= 8; len -= 8) {
		hash = ((hash << 5) + hash) + *str++;
		hash = ((hash << 5) + hash) + *str++;
		hash = ((hash << 5) + hash) + *str++;
		hash = ((hash << 5) + hash) + *str++;
		hash = ((hash << 5) + hash) + *str++;
		hash = ((hash << 5) + hash) + *str++;
		hash = ((hash << 5) + hash) + *str++;
		hash = ((hash << 5) + hash) + *str++;
	}
	switch (len) {
		case 7: hash = ((hash << 5) + hash) + *str++; /* fallthrough... */
		case 6: hash = ((hash << 5) + hash) + *str++; /* fallthrough... */
		case 5: hash = ((hash << 5) + hash) + *str++; /* fallthrough... */
		case 4: hash = ((hash << 5) + hash) + *str++; /* fallthrough... */
		case 3: hash = ((hash << 5) + hash) + *str++; /* fallthrough... */
		case 2: hash = ((hash << 5) + hash) + *str++; /* fallthrough... */
		case 1: hash = ((hash << 5) + hash) + *str++; break;
		case 0: break;
EMPTY_SWITCH_DEFAULT_CASE()
	}
#endif

	/* Hash value can't be zero, so we always set the high bit */
#if SIZEOF_CREX_LONG == 8
	return hash | C_UL(0x8000000000000000);
#elif SIZEOF_CREX_LONG == 4
	return hash | C_UL(0x80000000);
#else
# error "Unknown SIZEOF_CREX_LONG"
#endif
}

#define CREX_KNOWN_STRINGS(_) \
	_(CREX_STR_FILE,                   "file") \
	_(CREX_STR_LINE,                   "line") \
	_(CREX_STR_FUNCTION,               "function") \
	_(CREX_STR_CLASS,                  "class") \
	_(CREX_STR_OBJECT,                 "object") \
	_(CREX_STR_TYPE,                   "type") \
	_(CREX_STR_OBJECT_OPERATOR,        "->") \
	_(CREX_STR_PAAMAYIM_NEKUDOTAYIM,   "::") \
	_(CREX_STR_ARGS,                   "args") \
	_(CREX_STR_UNKNOWN,                "unknown") \
	_(CREX_STR_UNKNOWN_CAPITALIZED,    "Unknown") \
	_(CREX_STR_EVAL,                   "eval") \
	_(CREX_STR_INCLUDE,                "include") \
	_(CREX_STR_REQUIRE,                "require") \
	_(CREX_STR_INCLUDE_ONCE,           "include_once") \
	_(CREX_STR_REQUIRE_ONCE,           "require_once") \
	_(CREX_STR_SCALAR,                 "scalar") \
	_(CREX_STR_ERROR_REPORTING,        "error_reporting") \
	_(CREX_STR_STATIC,                 "static") \
	_(CREX_STR_THIS,                   "this") \
	_(CREX_STR_VALUE,                  "value") \
	_(CREX_STR_KEY,                    "key") \
	_(CREX_STR_MAGIC_INVOKE,           "__invoke") \
	_(CREX_STR_PREVIOUS,               "previous") \
	_(CREX_STR_CODE,                   "code") \
	_(CREX_STR_MESSAGE,                "message") \
	_(CREX_STR_SEVERITY,               "severity") \
	_(CREX_STR_STRING,                 "string") \
	_(CREX_STR_TRACE,                  "trace") \
	_(CREX_STR_SCHEME,                 "scheme") \
	_(CREX_STR_HOST,                   "host") \
	_(CREX_STR_PORT,                   "port") \
	_(CREX_STR_USER,                   "user") \
	_(CREX_STR_PASS,                   "pass") \
	_(CREX_STR_PATH,                   "path") \
	_(CREX_STR_QUERY,                  "query") \
	_(CREX_STR_FRAGMENT,               "fragment") \
	_(CREX_STR_NULL,                   "NULL") \
	_(CREX_STR_BOOLEAN,                "boolean") \
	_(CREX_STR_INTEGER,                "integer") \
	_(CREX_STR_DOUBLE,                 "double") \
	_(CREX_STR_ARRAY,                  "array") \
	_(CREX_STR_RESOURCE,               "resource") \
	_(CREX_STR_CLOSED_RESOURCE,        "resource (closed)") \
	_(CREX_STR_NAME,                   "name") \
	_(CREX_STR_ARGV,                   "argv") \
	_(CREX_STR_ARGC,                   "argc") \
	_(CREX_STR_ARRAY_CAPITALIZED,      "Array") \
	_(CREX_STR_BOOL,                   "bool") \
	_(CREX_STR_INT,                    "int") \
	_(CREX_STR_FLOAT,                  "float") \
	_(CREX_STR_CALLABLE,               "callable") \
	_(CREX_STR_ITERABLE,               "iterable") \
	_(CREX_STR_VOID,                   "void") \
	_(CREX_STR_NEVER,                  "never") \
	_(CREX_STR_FALSE,                  "false") \
	_(CREX_STR_TRUE,                   "true") \
	_(CREX_STR_NULL_LOWERCASE,         "null") \
	_(CREX_STR_MIXED,                  "mixed") \
	_(CREX_STR_TRAVERSABLE,            "Traversable") \
	_(CREX_STR_SLEEP,                  "__sleep") \
	_(CREX_STR_WAKEUP,                 "__wakeup") \
	_(CREX_STR_CASES,                  "cases") \
	_(CREX_STR_FROM,                   "from") \
	_(CREX_STR_TRYFROM,                "tryFrom") \
	_(CREX_STR_TRYFROM_LOWERCASE,      "tryfrom") \
	_(CREX_STR_AUTOGLOBAL_SERVER,      "_SERVER") \
	_(CREX_STR_AUTOGLOBAL_ENV,         "_ENV") \
	_(CREX_STR_AUTOGLOBAL_REQUEST,     "_REQUEST") \
	_(CREX_STR_COUNT,                  "count") \
	_(CREX_STR_SENSITIVEPARAMETER,     "SensitiveParameter") \
	_(CREX_STR_CONST_EXPR_PLACEHOLDER, "[constant expression]") \


typedef enum _crex_known_string_id {
#define _CREX_STR_ID(id, str) id,
CREX_KNOWN_STRINGS(_CREX_STR_ID)
#undef _CREX_STR_ID
	CREX_STR_LAST_KNOWN
} crex_known_string_id;

#endif /* CREX_STRING_H */
