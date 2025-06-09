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
   | Authors: Anatol Belski <ab@crx.net>                                  |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_LONG_H
#define CREX_LONG_H

#include <inttypes.h>
#include <stdint.h>

/* This is the heart of the whole int64 enablement in zval. */
#if defined(__x86_64__) || defined(__LP64__) || defined(_LP64) || defined(_WIN64)
# define CREX_ENABLE_ZVAL_LONG64 1
#endif

/* Integer types. */
#ifdef CREX_ENABLE_ZVAL_LONG64
typedef int64_t crex_long;
typedef uint64_t crex_ulong;
typedef int64_t crex_off_t;
# define CREX_LONG_MAX INT64_MAX
# define CREX_LONG_MIN INT64_MIN
# define CREX_ULONG_MAX UINT64_MAX
# define C_L(i) INT64_C(i)
# define C_UL(i) UINT64_C(i)
# define SIZEOF_CREX_LONG 8
#else
typedef int32_t crex_long;
typedef uint32_t crex_ulong;
typedef int32_t crex_off_t;
# define CREX_LONG_MAX INT32_MAX
# define CREX_LONG_MIN INT32_MIN
# define CREX_ULONG_MAX UINT32_MAX
# define C_L(i) INT32_C(i)
# define C_UL(i) UINT32_C(i)
# define SIZEOF_CREX_LONG 4
#endif


/* Conversion macros. */
#define CREX_LTOA_BUF_LEN 65

#ifdef CREX_ENABLE_ZVAL_LONG64
# define CREX_LONG_FMT "%" PRId64
# define CREX_ULONG_FMT "%" PRIu64
# define CREX_XLONG_FMT "%" PRIx64
# define CREX_LONG_FMT_SPEC PRId64
# define CREX_ULONG_FMT_SPEC PRIu64
# ifdef CREX_WIN32
#  define CREX_LTOA(i, s, len) _i64toa_s((i), (s), (len), 10)
#  define CREX_ATOL(s) _atoi64((s))
#  define CREX_STRTOL(s0, s1, base) _strtoi64((s0), (s1), (base))
#  define CREX_STRTOUL(s0, s1, base) _strtoui64((s0), (s1), (base))
#  define CREX_STRTOL_PTR _strtoi64
#  define CREX_STRTOUL_PTR _strtoui64
#  define CREX_ABS _abs64
# else
#  define CREX_LTOA(i, s, len) \
	do { \
		int st = snprintf((s), (len), CREX_LONG_FMT, (i)); \
		(s)[st] = '\0'; \
 	} while (0)
#  define CREX_ATOL(s) atoll((s))
#  define CREX_STRTOL(s0, s1, base) strtoll((s0), (s1), (base))
#  define CREX_STRTOUL(s0, s1, base) strtoull((s0), (s1), (base))
#  define CREX_STRTOL_PTR strtoll
#  define CREX_STRTOUL_PTR strtoull
#  define CREX_ABS imaxabs
# endif
#else
# define CREX_STRTOL(s0, s1, base) strtol((s0), (s1), (base))
# define CREX_STRTOUL(s0, s1, base) strtoul((s0), (s1), (base))
# define CREX_LONG_FMT "%" PRId32
# define CREX_ULONG_FMT "%" PRIu32
# define CREX_XLONG_FMT "%" PRIx32
# define CREX_LONG_FMT_SPEC PRId32
# define CREX_ULONG_FMT_SPEC PRIu32
# ifdef CREX_WIN32
#  define CREX_LTOA(i, s, len) _ltoa_s((i), (s), (len), 10)
#  define CREX_ATOL(s) atol((s))
# else
#  define CREX_LTOA(i, s, len) \
	do { \
		int st = snprintf((s), (len), CREX_LONG_FMT, (i)); \
		(s)[st] = '\0'; \
 	} while (0)
#  define CREX_ATOL(s) atol((s))
# endif
# define CREX_STRTOL_PTR strtol
# define CREX_STRTOUL_PTR strtoul
# define CREX_ABS abs
#endif

#if SIZEOF_CREX_LONG == 4
# define MAX_LENGTH_OF_LONG 11
# define LONG_MIN_DIGITS "2147483648"
#elif SIZEOF_CREX_LONG == 8
# define MAX_LENGTH_OF_LONG 20
# define LONG_MIN_DIGITS "9223372036854775808"
#else
# error "Unknown SIZEOF_CREX_LONG"
#endif

static const char long_min_digits[] = LONG_MIN_DIGITS;

#if SIZEOF_SIZE_T == 4
# define CREX_ADDR_FMT "0x%08zx"
#elif SIZEOF_SIZE_T == 8
# define CREX_ADDR_FMT "0x%016zx"
#else
# error "Unknown SIZEOF_SIZE_T"
#endif

#endif /* CREX_LONG_H */
