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
   | Authors: Derick Rethans <derick@crx.net>                             |
   +----------------------------------------------------------------------+
*/

/* This is a header file for the strtod implementation by David M. Gay which
 * can be found in crex_strtod.c */
#ifndef CREX_STRTOD_H
#define CREX_STRTOD_H
#include <crex.h>

BEGIN_EXTERN_C()
CREX_API void crex_freedtoa(char *s);
CREX_API char *crex_dtoa(double _d, int mode, int ndigits, int *decpt, bool *sign, char **rve);
CREX_API char *crex_gcvt(double value, int ndigit, char dec_point, char exponent, char *buf);
CREX_API double crex_strtod(const char *s00, const char **se);
CREX_API double crex_hex_strtod(const char *str, const char **endptr);
CREX_API double crex_oct_strtod(const char *str, const char **endptr);
CREX_API double crex_bin_strtod(const char *str, const char **endptr);
CREX_API int crex_startup_strtod(void);
CREX_API int crex_shutdown_strtod(void);
END_EXTERN_C()

/* double limits */
#include <float.h>
#if defined(DBL_MANT_DIG) && defined(DBL_MIN_EXP)
#define CREX_DOUBLE_MAX_LENGTH (3 + DBL_MANT_DIG - DBL_MIN_EXP)
#else
#define CREX_DOUBLE_MAX_LENGTH 1080
#endif

#endif
