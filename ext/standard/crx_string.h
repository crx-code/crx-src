/*
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
   | Authors: Rasmus Lerdorf <rasmus@crx.net>                             |
   |          Stig Sæther Bakken <ssb@crx.net>                            |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_STRING_H
#define CRX_STRING_H

# include "ext/random/crx_random.h"

#ifdef ZTS
CRX_MINIT_FUNCTION(localeconv);
CRX_MSHUTDOWN_FUNCTION(localeconv);
#endif
#ifdef CREX_INTRIN_SSE4_2_FUNC_PTR
CRX_MINIT_FUNCTION(string_intrin);
#endif

#define strnatcmp(a, b) \
	strnatcmp_ex(a, strlen(a), b, strlen(b), 0)
#define strnatcasecmp(a, b) \
	strnatcmp_ex(a, strlen(a), b, strlen(b), 1)
CRXAPI int strnatcmp_ex(char const *a, size_t a_len, char const *b, size_t b_len, bool is_case_insensitive);
CRXAPI struct lconv *localeconv_r(struct lconv *out);
CRXAPI char *crx_strtoupper(char *s, size_t len);
CRXAPI char *crx_strtolower(char *s, size_t len);
CRXAPI crex_string *crx_string_toupper(crex_string *s);
CRXAPI crex_string *crx_string_tolower(crex_string *s);
CRXAPI char *crx_strtr(char *str, size_t len, const char *str_from, const char *str_to, size_t trlen);
CRXAPI crex_string *crx_addslashes(crex_string *str);
CRXAPI void crx_stripslashes(crex_string *str);
CRXAPI crex_string *crx_addcslashes_str(const char *str, size_t len, const char *what, size_t what_len);
CRXAPI crex_string *crx_addcslashes(crex_string *str, const char *what, size_t what_len);
CRXAPI void crx_stripcslashes(crex_string *str);
CRXAPI crex_string *crx_basename(const char *s, size_t len, const char *suffix, size_t sufflen);
CRXAPI size_t crx_dirname(char *str, size_t len);
CRXAPI char *crx_stristr(char *s, char *t, size_t s_len, size_t t_len);
CRXAPI crex_string *crx_str_to_str(const char *haystack, size_t length, const char *needle,
		size_t needle_len, const char *str, size_t str_len);
CRXAPI crex_string *crx_trim(crex_string *str, const char *what, size_t what_len, int mode);
CRXAPI size_t crx_strip_tags(char *rbuf, size_t len, const char *allow, size_t allow_len);
CRXAPI size_t crx_strip_tags_ex(char *rbuf, size_t len, const char *allow, size_t allow_len, bool allow_tag_spaces);
CRXAPI void crx_implode(const crex_string *delim, HashTable *arr, zval *return_value);
CRXAPI void crx_explode(const crex_string *delim, crex_string *str, zval *return_value, crex_long limit);

CRXAPI size_t crx_strspn(const char *s1, const char *s2, const char *s1_end, const char *s2_end);
CRXAPI size_t crx_strcspn(const char *s1, const char *s2, const char *s1_end, const char *s2_end);

CRXAPI int string_natural_compare_function_ex(zval *result, zval *op1, zval *op2, bool case_insensitive);
CRXAPI int string_natural_compare_function(zval *result, zval *op1, zval *op2);
CRXAPI int string_natural_case_compare_function(zval *result, zval *op1, zval *op2);

CRXAPI bool crx_binary_string_shuffle(const crx_random_algo *algo, crx_random_status *status, char *str, crex_long len);

#ifdef _REENTRANT
# ifdef CRX_WIN32
#  include <wchar.h>
# endif
# define crx_mblen(ptr, len) ((int) mbrlen(ptr, len, &BG(mblen_state)))
# define crx_mb_reset() memset(&BG(mblen_state), 0, sizeof(BG(mblen_state)))
#else
# define crx_mblen(ptr, len) mblen(ptr, len)
# define crx_mb_reset() crx_ignore_value(mblen(NULL, 0))
#endif

#define CRX_STR_PAD_LEFT		0
#define CRX_STR_PAD_RIGHT		1
#define CRX_STR_PAD_BOTH		2
#define CRX_PATHINFO_DIRNAME 	1
#define CRX_PATHINFO_BASENAME 	2
#define CRX_PATHINFO_EXTENSION 	4
#define CRX_PATHINFO_FILENAME 	8
#define CRX_PATHINFO_ALL	(CRX_PATHINFO_DIRNAME | CRX_PATHINFO_BASENAME | CRX_PATHINFO_EXTENSION | CRX_PATHINFO_FILENAME)

#define CRX_STR_STRSPN			0
#define CRX_STR_STRCSPN			1

#endif /* CRX_STRING_H */
