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
   | Authors: Jim Winstead <jimw@crx.net>                                 |
   |          Stig Sæther Bakken <ssb@crx.net>                            |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_MATH_H
#define CRX_MATH_H

CRXAPI double _crx_math_round(double value, int places, int mode);
CRXAPI crex_string *_crx_math_number_format(double d, int dec, char dec_point, char thousand_sep);
CRXAPI crex_string *_crx_math_number_format_ex(double d, int dec, const char *dec_point, size_t dec_point_len, const char *thousand_sep, size_t thousand_sep_len);
CRXAPI crex_string *_crx_math_number_format_long(crex_long num, crex_long dec, const char *dec_point, size_t dec_point_len, const char *thousand_sep, size_t thousand_sep_len);
CRXAPI crex_string * _crx_math_longtobase(crex_long arg, int base);
CRXAPI crex_long _crx_math_basetolong(zval *arg, int base);
CRXAPI void _crx_math_basetozval(crex_string *str, int base, zval *ret);
CRXAPI crex_string * _crx_math_zvaltobase(zval *arg, int base);

#include <math.h>

#ifndef M_E
#define M_E            2.7182818284590452354   /* e */
#endif

#ifndef M_LOG2E
#define M_LOG2E        1.4426950408889634074   /* log_2 e */
#endif

#ifndef M_LOG10E
#define M_LOG10E       0.43429448190325182765  /* log_10 e */
#endif

#ifndef M_LN2
#define M_LN2          0.69314718055994530942  /* log_e 2 */
#endif

#ifndef M_LN10
#define M_LN10         2.30258509299404568402  /* log_e 10 */
#endif

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

#ifndef M_PI_2
#define M_PI_2         1.57079632679489661923  /* pi/2 */
#endif

#ifndef M_PI_4
#define M_PI_4         0.78539816339744830962  /* pi/4 */
#endif

#ifndef M_1_PI
#define M_1_PI         0.31830988618379067154  /* 1/pi */
#endif

#ifndef M_2_PI
#define M_2_PI         0.63661977236758134308  /* 2/pi */
#endif

#ifndef M_SQRTPI
#define M_SQRTPI       1.77245385090551602729  /* sqrt(pi) */
#endif

#ifndef M_2_SQRTPI
#define M_2_SQRTPI     1.12837916709551257390  /* 2/sqrt(pi) */
#endif

#ifndef M_LNPI
#define M_LNPI         1.14472988584940017414  /* ln(pi) */
#endif

#ifndef M_EULER
#define M_EULER        0.57721566490153286061 /* Euler constant */
#endif

#ifndef M_SQRT2
#define M_SQRT2        1.41421356237309504880  /* sqrt(2) */
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2      0.70710678118654752440  /* 1/sqrt(2) */
#endif

#ifndef M_SQRT3
#define M_SQRT3	       1.73205080756887729352  /* sqrt(3) */
#endif

/* Define rounding modes (all are round-to-nearest) */
#ifndef CRX_ROUND_HALF_UP
#define CRX_ROUND_HALF_UP        0x01    /* Arithmetic rounding, up == away from zero */
#endif

#ifndef CRX_ROUND_HALF_DOWN
#define CRX_ROUND_HALF_DOWN      0x02    /* Down == towards zero */
#endif

#ifndef CRX_ROUND_HALF_EVEN
#define CRX_ROUND_HALF_EVEN      0x03    /* Banker's rounding */
#endif

#ifndef CRX_ROUND_HALF_ODD
#define CRX_ROUND_HALF_ODD       0x04
#endif

#endif /* CRX_MATH_H */
