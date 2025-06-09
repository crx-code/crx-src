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
   | Author: Sascha Schumann <sascha@schumann.cx>                         |
   |         Xinchen Hui <laruence@crx.net>                               |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_SMART_STRING_H
#define CRX_SMART_STRING_H

#include "crex_smart_string_public.h"

#include <stdlib.h>
#include <crex.h>

/* wrapper */

#define smart_string_appends_ex(str, src, what) \
	smart_string_appendl_ex((str), (src), strlen(src), (what))
#define smart_string_appends(str, src) \
	smart_string_appendl((str), (src), strlen(src))
#define smart_string_append_ex(str, src, what) \
	smart_string_appendl_ex((str), ((smart_string *)(src))->c, \
		((smart_string *)(src))->len, (what));
#define smart_string_sets(str, src) \
	smart_string_setl((str), (src), strlen(src));

#define smart_string_appendc(str, c) \
	smart_string_appendc_ex((str), (c), 0)
#define smart_string_free(s) \
	smart_string_free_ex((s), 0)
#define smart_string_appendl(str, src, len) \
	smart_string_appendl_ex((str), (src), (len), 0)
#define smart_string_append(str, src) \
	smart_string_append_ex((str), (src), 0)
#define smart_string_append_long(str, val) \
	smart_string_append_long_ex((str), (val), 0)
#define smart_string_append_unsigned(str, val) \
	smart_string_append_unsigned_ex((str), (val), 0)

CREX_API void CREX_FASTCALL _smart_string_alloc_persistent(smart_string *str, size_t len);
CREX_API void CREX_FASTCALL _smart_string_alloc(smart_string *str, size_t len);

static crex_always_inline size_t smart_string_alloc(smart_string *str, size_t len, bool persistent) {
	if (UNEXPECTED(!str->c) || UNEXPECTED(len >= str->a - str->len)) {
		if (persistent) {
			_smart_string_alloc_persistent(str, len);
		} else {
			_smart_string_alloc(str, len);
		}
	}
	return str->len + len;
}

static crex_always_inline void smart_string_free_ex(smart_string *str, bool persistent) {
	if (str->c) {
		pefree(str->c, persistent);
		str->c = NULL;
	}
	str->a = str->len = 0;
}

static crex_always_inline void smart_string_0(smart_string *str) {
	if (str->c) {
		str->c[str->len] = '\0';
	}
}

static crex_always_inline void smart_string_appendc_ex(smart_string *dest, char ch, bool persistent) {
	dest->len = smart_string_alloc(dest, 1, persistent);
	dest->c[dest->len - 1] = ch;
}

static crex_always_inline void smart_string_appendl_ex(smart_string *dest, const char *str, size_t len, bool persistent) {
	size_t new_len = smart_string_alloc(dest, len, persistent);
	memcpy(dest->c + dest->len, str, len);
	dest->len = new_len;

}

static crex_always_inline void smart_string_append_long_ex(smart_string *dest, crex_long num, bool persistent) {
	char buf[32];
	char *result = crex_print_long_to_buf(buf + sizeof(buf) - 1, num);
	smart_string_appendl_ex(dest, result, buf + sizeof(buf) - 1 - result, persistent);
}

static crex_always_inline void smart_string_append_unsigned_ex(smart_string *dest, crex_ulong num, bool persistent) {
	char buf[32];
	char *result = crex_print_ulong_to_buf(buf + sizeof(buf) - 1, num);
	smart_string_appendl_ex(dest, result, buf + sizeof(buf) - 1 - result, persistent);
}

static crex_always_inline void smart_string_setl(smart_string *dest, char *src, size_t len) {
	dest->len = len;
	dest->a = len + 1;
	dest->c = src;
}

static crex_always_inline void smart_string_reset(smart_string *str) {
	str->len = 0;
}

#endif
