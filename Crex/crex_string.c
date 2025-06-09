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

#include "crex.h"
#include "crex_globals.h"

#ifdef HAVE_VALGRIND
# include "valgrind/callgrind.h"
#endif

CREX_API crex_new_interned_string_func_t crex_new_interned_string;
CREX_API crex_string_init_interned_func_t crex_string_init_interned;
CREX_API crex_string_init_existing_interned_func_t crex_string_init_existing_interned;

static crex_string* CREX_FASTCALL crex_new_interned_string_permanent(crex_string *str);
static crex_string* CREX_FASTCALL crex_new_interned_string_request(crex_string *str);
static crex_string* CREX_FASTCALL crex_string_init_interned_permanent(const char *str, size_t size, bool permanent);
static crex_string* CREX_FASTCALL crex_string_init_existing_interned_permanent(const char *str, size_t size, bool permanent);
static crex_string* CREX_FASTCALL crex_string_init_interned_request(const char *str, size_t size, bool permanent);
static crex_string* CREX_FASTCALL crex_string_init_existing_interned_request(const char *str, size_t size, bool permanent);

/* Any strings interned in the startup phase. Common to all the threads,
   won't be free'd until process exit. If we want an ability to
   add permanent strings even after startup, it would be still
   possible on costs of locking in the thread safe builds. */
static HashTable interned_strings_permanent;

static crex_new_interned_string_func_t interned_string_request_handler = crex_new_interned_string_request;
static crex_string_init_interned_func_t interned_string_init_request_handler = crex_string_init_interned_request;
static crex_string_init_existing_interned_func_t interned_string_init_existing_request_handler = crex_string_init_existing_interned_request;

CREX_API crex_string  *crex_empty_string = NULL;
CREX_API crex_string  *crex_one_char_string[256];
CREX_API crex_string **crex_known_strings = NULL;

CREX_API crex_ulong CREX_FASTCALL crex_string_hash_func(crex_string *str)
{
	return ZSTR_H(str) = crex_hash_func(ZSTR_VAL(str), ZSTR_LEN(str));
}

CREX_API crex_ulong CREX_FASTCALL crex_hash_func(const char *str, size_t len)
{
	return crex_inline_hash_func(str, len);
}

static void _str_dtor(zval *zv)
{
	crex_string *str = C_STR_P(zv);
	pefree(str, GC_FLAGS(str) & IS_STR_PERSISTENT);
}

static const char *known_strings[] = {
#define _CREX_STR_DSC(id, str) str,
CREX_KNOWN_STRINGS(_CREX_STR_DSC)
#undef _CREX_STR_DSC
	NULL
};

static crex_always_inline void crex_init_interned_strings_ht(HashTable *interned_strings, bool permanent)
{
	crex_hash_init(interned_strings, 1024, NULL, _str_dtor, permanent);
	if (permanent) {
		crex_hash_real_init_mixed(interned_strings);
	}
}

CREX_API void crex_interned_strings_init(void)
{
	char s[2];
	unsigned int i;
	crex_string *str;

	interned_string_request_handler = crex_new_interned_string_request;
	interned_string_init_request_handler = crex_string_init_interned_request;
	interned_string_init_existing_request_handler = crex_string_init_existing_interned_request;

	crex_empty_string = NULL;
	crex_known_strings = NULL;

	crex_init_interned_strings_ht(&interned_strings_permanent, 1);

	crex_new_interned_string = crex_new_interned_string_permanent;
	crex_string_init_interned = crex_string_init_interned_permanent;
	crex_string_init_existing_interned = crex_string_init_existing_interned_permanent;

	/* interned empty string */
	str = crex_string_alloc(sizeof("")-1, 1);
	ZSTR_VAL(str)[0] = '\000';
	crex_empty_string = crex_new_interned_string_permanent(str);
	GC_ADD_FLAGS(crex_empty_string, IS_STR_VALID_UTF8);

	s[1] = 0;
	for (i = 0; i < 256; i++) {
		s[0] = i;
		crex_one_char_string[i] = crex_new_interned_string_permanent(crex_string_init(s, 1, 1));
		if (i < 0x80) {
			GC_ADD_FLAGS(crex_one_char_string[i], IS_STR_VALID_UTF8);
		}
	}

	/* known strings */
	crex_known_strings = pemalloc(sizeof(crex_string*) * ((sizeof(known_strings) / sizeof(known_strings[0]) - 1)), 1);
	for (i = 0; i < (sizeof(known_strings) / sizeof(known_strings[0])) - 1; i++) {
		str = crex_string_init(known_strings[i], strlen(known_strings[i]), 1);
		crex_known_strings[i] = crex_new_interned_string_permanent(str);
		GC_ADD_FLAGS(crex_known_strings[i], IS_STR_VALID_UTF8);
	}
}

CREX_API void crex_interned_strings_dtor(void)
{
	crex_hash_destroy(&interned_strings_permanent);

	free(crex_known_strings);
	crex_known_strings = NULL;
}

static crex_always_inline crex_string *crex_interned_string_ht_lookup_ex(crex_ulong h, const char *str, size_t size, HashTable *interned_strings)
{
	uint32_t nIndex;
	uint32_t idx;
	Bucket *p;

	nIndex = h | interned_strings->nTableMask;
	idx = HT_HASH(interned_strings, nIndex);
	while (idx != HT_INVALID_IDX) {
		p = HT_HASH_TO_BUCKET(interned_strings, idx);
		if ((p->h == h) && crex_string_equals_cstr(p->key, str, size)) {
			return p->key;
		}
		idx = C_NEXT(p->val);
	}

	return NULL;
}

static crex_always_inline crex_string *crex_interned_string_ht_lookup(crex_string *str, HashTable *interned_strings)
{
	crex_ulong h = ZSTR_H(str);
	uint32_t nIndex;
	uint32_t idx;
	Bucket *p;

	nIndex = h | interned_strings->nTableMask;
	idx = HT_HASH(interned_strings, nIndex);
	while (idx != HT_INVALID_IDX) {
		p = HT_HASH_TO_BUCKET(interned_strings, idx);
		if ((p->h == h) && crex_string_equal_content(p->key, str)) {
			return p->key;
		}
		idx = C_NEXT(p->val);
	}

	return NULL;
}

/* This function might be not thread safe at least because it would update the
   hash val in the passed string. Be sure it is called in the appropriate context. */
static crex_always_inline crex_string *crex_add_interned_string(crex_string *str, HashTable *interned_strings, uint32_t flags)
{
	zval val;

	GC_SET_REFCOUNT(str, 1);
	GC_ADD_FLAGS(str, IS_STR_INTERNED | flags);

	ZVAL_INTERNED_STR(&val, str);

	crex_hash_add_new(interned_strings, str, &val);

	return str;
}

CREX_API crex_string* CREX_FASTCALL crex_interned_string_find_permanent(crex_string *str)
{
	crex_string_hash_val(str);
	return crex_interned_string_ht_lookup(str, &interned_strings_permanent);
}

static crex_string* CREX_FASTCALL crex_init_string_for_interning(crex_string *str, bool persistent)
{
	uint32_t flags = ZSTR_GET_COPYABLE_CONCAT_PROPERTIES(str);
	crex_ulong h = ZSTR_H(str);
	crex_string_delref(str);
	str = crex_string_init(ZSTR_VAL(str), ZSTR_LEN(str), persistent);
	GC_ADD_FLAGS(str, flags);
	ZSTR_H(str) = h;
	return str;
}

static crex_string* CREX_FASTCALL crex_new_interned_string_permanent(crex_string *str)
{
	crex_string *ret;

	if (ZSTR_IS_INTERNED(str)) {
		return str;
	}

	crex_string_hash_val(str);
	ret = crex_interned_string_ht_lookup(str, &interned_strings_permanent);
	if (ret) {
		crex_string_release(str);
		return ret;
	}

	CREX_ASSERT(GC_FLAGS(str) & GC_PERSISTENT);
	if (GC_REFCOUNT(str) > 1) {
		str = crex_init_string_for_interning(str, true);
	}

	return crex_add_interned_string(str, &interned_strings_permanent, IS_STR_PERMANENT);
}

static crex_string* CREX_FASTCALL crex_new_interned_string_request(crex_string *str)
{
	crex_string *ret;

	if (ZSTR_IS_INTERNED(str)) {
		return str;
	}

	crex_string_hash_val(str);

	/* Check for permanent strings, the table is readonly at this point. */
	ret = crex_interned_string_ht_lookup(str, &interned_strings_permanent);
	if (ret) {
		crex_string_release(str);
		return ret;
	}

	ret = crex_interned_string_ht_lookup(str, &CG(interned_strings));
	if (ret) {
		crex_string_release(str);
		return ret;
	}

	/* Create a short living interned, freed after the request. */
#if CREX_RC_DEBUG
	if (crex_rc_debug) {
		/* CRX shouldn't create persistent interned string during request,
		 * but at least dl() may do this */
		CREX_ASSERT(!(GC_FLAGS(str) & GC_PERSISTENT));
	}
#endif
	if (GC_REFCOUNT(str) > 1) {
		str = crex_init_string_for_interning(str, false);
	}

	ret = crex_add_interned_string(str, &CG(interned_strings), 0);

	return ret;
}

static crex_string* CREX_FASTCALL crex_string_init_interned_permanent(const char *str, size_t size, bool permanent)
{
	crex_string *ret;
	crex_ulong h = crex_inline_hash_func(str, size);

	ret = crex_interned_string_ht_lookup_ex(h, str, size, &interned_strings_permanent);
	if (ret) {
		return ret;
	}

	CREX_ASSERT(permanent);
	ret = crex_string_init(str, size, permanent);
	ZSTR_H(ret) = h;
	return crex_add_interned_string(ret, &interned_strings_permanent, IS_STR_PERMANENT);
}

static crex_string* CREX_FASTCALL crex_string_init_existing_interned_permanent(const char *str, size_t size, bool permanent)
{
	crex_ulong h = crex_inline_hash_func(str, size);
	crex_string *ret = crex_interned_string_ht_lookup_ex(h, str, size, &interned_strings_permanent);
	if (ret) {
		return ret;
	}

	CREX_ASSERT(permanent);
	ret = crex_string_init(str, size, permanent);
	ZSTR_H(ret) = h;
	return ret;
}

static crex_string* CREX_FASTCALL crex_string_init_interned_request(const char *str, size_t size, bool permanent)
{
	crex_string *ret;
	crex_ulong h = crex_inline_hash_func(str, size);

	/* Check for permanent strings, the table is readonly at this point. */
	ret = crex_interned_string_ht_lookup_ex(h, str, size, &interned_strings_permanent);
	if (ret) {
		return ret;
	}

	ret = crex_interned_string_ht_lookup_ex(h, str, size, &CG(interned_strings));
	if (ret) {
		return ret;
	}

#if CREX_RC_DEBUG
	if (crex_rc_debug) {
		/* CRX shouldn't create persistent interned string during request,
		 * but at least dl() may do this */
		CREX_ASSERT(!permanent);
	}
#endif
	ret = crex_string_init(str, size, permanent);
	ZSTR_H(ret) = h;

	/* Create a short living interned, freed after the request. */
	return crex_add_interned_string(ret, &CG(interned_strings), 0);
}

static crex_string* CREX_FASTCALL crex_string_init_existing_interned_request(const char *str, size_t size, bool permanent)
{
	crex_ulong h = crex_inline_hash_func(str, size);
	crex_string *ret = crex_interned_string_ht_lookup_ex(h, str, size, &interned_strings_permanent);
	if (ret) {
		return ret;
	}

	ret = crex_interned_string_ht_lookup_ex(h, str, size, &CG(interned_strings));
	if (ret) {
		return ret;
	}

	CREX_ASSERT(!permanent);
	ret = crex_string_init(str, size, permanent);
	ZSTR_H(ret) = h;
	return ret;
}

CREX_API void crex_interned_strings_activate(void)
{
	crex_init_interned_strings_ht(&CG(interned_strings), 0);
}

CREX_API void crex_interned_strings_deactivate(void)
{
	crex_hash_destroy(&CG(interned_strings));
}

CREX_API void crex_interned_strings_set_request_storage_handlers(crex_new_interned_string_func_t handler, crex_string_init_interned_func_t init_handler, crex_string_init_existing_interned_func_t init_existing_handler)
{
	interned_string_request_handler = handler;
	interned_string_init_request_handler = init_handler;
	interned_string_init_existing_request_handler = init_existing_handler;
}

CREX_API void crex_interned_strings_switch_storage(bool request)
{
	if (request) {
		crex_new_interned_string = interned_string_request_handler;
		crex_string_init_interned = interned_string_init_request_handler;
		crex_string_init_existing_interned = interned_string_init_existing_request_handler;
	} else {
		crex_new_interned_string = crex_new_interned_string_permanent;
		crex_string_init_interned = crex_string_init_interned_permanent;
		crex_string_init_existing_interned = crex_string_init_existing_interned_permanent;
	}
}

/* Even if we don't build with valgrind support, include the symbol so that valgrind available
 * only at runtime will not result in false positives. */
#ifndef I_REPLACE_SONAME_FNNAME_ZU
# define I_REPLACE_SONAME_FNNAME_ZU(soname, fnname) _vgr00000ZU_ ## soname ## _ ## fnname
#endif

/* See GH-9068 */
#if defined(__GNUC__) && (__GNUC__ >= 11 || defined(__clang__)) && __has_attribute(no_caller_saved_registers)
# define NO_CALLER_SAVED_REGISTERS __attribute__((no_caller_saved_registers))
# ifndef __clang__
#  pragma GCC push_options
#  pragma GCC target ("general-regs-only")
#  define POP_OPTIONS
# endif
#else
# define NO_CALLER_SAVED_REGISTERS
#endif

CREX_API bool CREX_FASTCALL NO_CALLER_SAVED_REGISTERS I_REPLACE_SONAME_FNNAME_ZU(NONE,crex_string_equal_val)(const crex_string *s1, const crex_string *s2)
{
	return !memcmp(ZSTR_VAL(s1), ZSTR_VAL(s2), ZSTR_LEN(s1));
}

#ifdef POP_OPTIONS
# pragma GCC pop_options
# undef POP_OPTIONS
#endif

#if defined(__GNUC__) && defined(__i386__)
CREX_API bool CREX_FASTCALL crex_string_equal_val(const crex_string *s1, const crex_string *s2)
{
	const char *ptr = ZSTR_VAL(s1);
	size_t delta = (const char*)s2 - (const char*)s1;
	size_t len = ZSTR_LEN(s1);
	crex_ulong ret;

	__asm__ (
		".LL0%=:\n\t"
		"movl (%2,%3), %0\n\t"
		"xorl (%2), %0\n\t"
		"jne .LL1%=\n\t"
		"addl $0x4, %2\n\t"
		"subl $0x4, %1\n\t"
		"ja .LL0%=\n\t"
		"movl $0x1, %0\n\t"
		"jmp .LL3%=\n\t"
		".LL1%=:\n\t"
		"cmpl $0x4,%1\n\t"
		"jb .LL2%=\n\t"
		"xorl %0, %0\n\t"
		"jmp .LL3%=\n\t"
		".LL2%=:\n\t"
		"negl %1\n\t"
		"lea 0x20(,%1,8), %1\n\t"
		"shll %b1, %0\n\t"
		"sete %b0\n\t"
		"movzbl %b0, %0\n\t"
		".LL3%=:\n"
		: "=&a"(ret),
		  "+c"(len),
		  "+r"(ptr)
		: "r"(delta)
		: "cc");
	return ret;
}

#elif defined(__GNUC__) && defined(__x86_64__) && !defined(__ILP32__)
CREX_API bool CREX_FASTCALL crex_string_equal_val(const crex_string *s1, const crex_string *s2)
{
	const char *ptr = ZSTR_VAL(s1);
	size_t delta = (const char*)s2 - (const char*)s1;
	size_t len = ZSTR_LEN(s1);
	crex_ulong ret;

	__asm__ (
		".LL0%=:\n\t"
		"movq (%2,%3), %0\n\t"
		"xorq (%2), %0\n\t"
		"jne .LL1%=\n\t"
		"addq $0x8, %2\n\t"
		"subq $0x8, %1\n\t"
		"ja .LL0%=\n\t"
		"movq $0x1, %0\n\t"
		"jmp .LL3%=\n\t"
		".LL1%=:\n\t"
		"cmpq $0x8,%1\n\t"
		"jb .LL2%=\n\t"
		"xorq %0, %0\n\t"
		"jmp .LL3%=\n\t"
		".LL2%=:\n\t"
		"negq %1\n\t"
		"lea 0x40(,%1,8), %1\n\t"
		"shlq %b1, %0\n\t"
		"sete %b0\n\t"
		"movzbq %b0, %0\n\t"
		".LL3%=:\n"
		: "=&a"(ret),
		  "+c"(len),
		  "+r"(ptr)
		: "r"(delta)
		: "cc");
	return ret;
}
#endif

CREX_API crex_string *crex_string_concat2(
		const char *str1, size_t str1_len,
		const char *str2, size_t str2_len)
{
	size_t len = str1_len + str2_len;
	crex_string *res = crex_string_alloc(len, 0);

	memcpy(ZSTR_VAL(res), str1, str1_len);
	memcpy(ZSTR_VAL(res) + str1_len, str2, str2_len);
	ZSTR_VAL(res)[len] = '\0';

	return res;
}

CREX_API crex_string *crex_string_concat3(
		const char *str1, size_t str1_len,
		const char *str2, size_t str2_len,
		const char *str3, size_t str3_len)
{
	size_t len = str1_len + str2_len + str3_len;
	crex_string *res = crex_string_alloc(len, 0);

	memcpy(ZSTR_VAL(res), str1, str1_len);
	memcpy(ZSTR_VAL(res) + str1_len, str2, str2_len);
	memcpy(ZSTR_VAL(res) + str1_len + str2_len, str3, str3_len);
	ZSTR_VAL(res)[len] = '\0';

	return res;
}
