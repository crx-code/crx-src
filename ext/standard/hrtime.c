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
   | Author: Niklas Keller <kelunik@crx.net>                              |
   | Author: Anatol Belski <ab@crx.net>                                   |
   +----------------------------------------------------------------------+
 */

#include "crx.h"
#include "crex_hrtime.h"

#ifdef CREX_ENABLE_ZVAL_LONG64
#define CRX_RETURN_HRTIME(t) RETURN_LONG((crex_long)t)
#else
#ifdef _WIN32
# define HRTIME_U64A(i, s, len) _ui64toa_s(i, s, len, 10)
#else
# define HRTIME_U64A(i, s, len) \
	do { \
		int st = snprintf(s, len, "%llu", i); \
		s[st] = '\0'; \
	} while (0)
#endif
#define CRX_RETURN_HRTIME(t) do { \
	char _a[CREX_LTOA_BUF_LEN]; \
	double _d; \
	HRTIME_U64A(t, _a, CREX_LTOA_BUF_LEN); \
	_d = crex_strtod(_a, NULL); \
	RETURN_DOUBLE(_d); \
	} while (0)
#endif

/* {{{ Returns an array of integers in form [seconds, nanoseconds] counted
	from an arbitrary point in time. If an optional boolean argument is
	passed, returns an integer on 64-bit platforms or float on 32-bit
	containing the current high-resolution time in nanoseconds. The
	delivered timestamp is monotonic and cannot be adjusted. */
CRX_FUNCTION(hrtime)
{
#if CREX_HRTIME_AVAILABLE
	bool get_as_num = 0;
	crex_hrtime_t t = crex_hrtime();

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(get_as_num)
	CREX_PARSE_PARAMETERS_END();

	if (UNEXPECTED(get_as_num)) {
		CRX_RETURN_HRTIME(t);
	} else {
		array_init_size(return_value, 2);
		crex_hash_real_init_packed(C_ARRVAL_P(return_value));
		add_next_index_long(return_value, (crex_long)(t / (crex_hrtime_t)CREX_NANO_IN_SEC));
		add_next_index_long(return_value, (crex_long)(t % (crex_hrtime_t)CREX_NANO_IN_SEC));
	}
#else
	RETURN_FALSE;
#endif
}
/* }}} */
