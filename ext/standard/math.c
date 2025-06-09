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
   |          Zeev Suraski <zeev@crx.net>                                 |
   | CRX 4.0 patches by Thies C. Arntzen <thies@thieso.net>               |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "crx_math.h"
#include "crex_bitset.h"
#include "crex_exceptions.h"
#include "crex_multiply.h"
#include "crex_portability.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "basic_functions.h"

/* {{{ crx_intlog10abs
   Returns floor(log10(fabs(val))), uses fast binary search */
static inline int crx_intlog10abs(double value) {
	value = fabs(value);

	if (value < 1e-8 || value > 1e22) {
		return (int)floor(log10(value));
	} else {
		/* Do a binary search with 5 steps */
		int result = 15;
		static const double values[] = {
				1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1, 1e0, 1e1, 1e2,
				1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13,
				1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};

		if (value < values[result]) {
			result -= 8;
		} else {
			result += 8;
		}
		if (value < values[result]) {
			result -= 4;
		} else {
			result += 4;
		}
		if (value < values[result]) {
			result -= 2;
		} else {
			result += 2;
		}
		if (value < values[result]) {
			result -= 1;
		} else {
			result += 1;
		}
		if (value < values[result]) {
			result -= 1;
		}
		result -= 8;
		return result;
	}
}
/* }}} */

/* {{{ crx_intpow10
       Returns pow(10.0, (double)power), uses fast lookup table for exact powers */
static inline double crx_intpow10(int power) {
	/* Not in lookup table */
	if (power < 0 || power > 22) {
		return pow(10.0, (double)power);
	}

	static const double powers[] = {
			1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11,
			1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};

	return powers[power];
}
/* }}} */

/* {{{ crx_round_helper
       Actually performs the rounding of a value to integer in a certain mode */
static inline double crx_round_helper(double value, int mode) {
	double tmp_value;

	if (value >= 0.0) {
		tmp_value = floor(value + 0.5);
		if ((mode == CRX_ROUND_HALF_DOWN && value == (-0.5 + tmp_value)) ||
			(mode == CRX_ROUND_HALF_EVEN && value == (0.5 + 2 * floor(tmp_value/2.0))) ||
			(mode == CRX_ROUND_HALF_ODD  && value == (0.5 + 2 * floor(tmp_value/2.0) - 1.0)))
		{
			tmp_value = tmp_value - 1.0;
		}
	} else {
		tmp_value = ceil(value - 0.5);
		if ((mode == CRX_ROUND_HALF_DOWN && value == (0.5 + tmp_value)) ||
			(mode == CRX_ROUND_HALF_EVEN && value == (-0.5 + 2 * ceil(tmp_value/2.0))) ||
			(mode == CRX_ROUND_HALF_ODD  && value == (-0.5 + 2 * ceil(tmp_value/2.0) + 1.0)))
		{
			tmp_value = tmp_value + 1.0;
		}
	}

	return tmp_value;
}
/* }}} */

/* {{{ _crx_math_round */
/*
 * Rounds a number to a certain number of decimal places in a certain rounding
 * mode. For the specifics of the algorithm, see http://wiki.crx.net/rfc/rounding
 */
CRXAPI double _crx_math_round(double value, int places, int mode) {
	double f1, f2;
	double tmp_value;
	int precision_places;

	if (!crex_finite(value) || value == 0.0) {
		return value;
	}

	places = places < INT_MIN+1 ? INT_MIN+1 : places;
	precision_places = 14 - crx_intlog10abs(value);

	f1 = crx_intpow10(abs(places));

	/* If the decimal precision guaranteed by FP arithmetic is higher than
	   the requested places BUT is small enough to make sure a non-zero value
	   is returned, pre-round the result to the precision */
	if (precision_places > places && precision_places - 15 < places) {
		int64_t use_precision = precision_places < INT_MIN+1 ? INT_MIN+1 : precision_places;

		f2 = crx_intpow10(abs((int)use_precision));
		if (use_precision >= 0) {
			tmp_value = value * f2;
		} else {
			tmp_value = value / f2;
		}
		/* preround the result (tmp_value will always be something * 1e14,
		   thus never larger than 1e15 here) */
		tmp_value = crx_round_helper(tmp_value, mode);

		use_precision = places - precision_places;
		use_precision = use_precision < INT_MIN+1 ? INT_MIN+1 : use_precision;
		/* now correctly move the decimal point */
		f2 = crx_intpow10(abs((int)use_precision));
		/* because places < precision_places */
		tmp_value = tmp_value / f2;
	} else {
		/* adjust the value */
		if (places >= 0) {
			tmp_value = value * f1;
		} else {
			tmp_value = value / f1;
		}
		/* This value is beyond our precision, so rounding it is pointless */
		if (fabs(tmp_value) >= 1e15) {
			return value;
		}
	}

	/* round the temp value */
	tmp_value = crx_round_helper(tmp_value, mode);

	/* see if it makes sense to use simple division to round the value */
	if (abs(places) < 23) {
		if (places > 0) {
			tmp_value = tmp_value / f1;
		} else {
			tmp_value = tmp_value * f1;
		}
	} else {
		/* Simple division can't be used since that will cause wrong results.
		   Instead, the number is converted to a string and back again using
		   strtod(). strtod() will return the nearest possible FP value for
		   that string. */

		/* 40 Bytes should be more than enough for this format string. The
		   float won't be larger than 1e15 anyway. But just in case, use
		   snprintf() and make sure the buffer is zero-terminated */
		char buf[40];
		snprintf(buf, 39, "%15fe%d", tmp_value, -places);
		buf[39] = '\0';
		tmp_value = crex_strtod(buf, NULL);
		/* couldn't convert to string and back */
		if (!crex_finite(tmp_value) || crex_isnan(tmp_value)) {
			tmp_value = value;
		}
	}

	return tmp_value;
}
/* }}} */

/* {{{ Return the absolute value of the number */
CRX_FUNCTION(abs)
{
	zval *value;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_NUMBER(value)
	CREX_PARSE_PARAMETERS_END();

	if (C_TYPE_P(value) == IS_DOUBLE) {
		RETURN_DOUBLE(fabs(C_DVAL_P(value)));
	} else if (C_TYPE_P(value) == IS_LONG) {
		if (C_LVAL_P(value) == CREX_LONG_MIN) {
			RETURN_DOUBLE(-(double)CREX_LONG_MIN);
		} else {
			RETURN_LONG(C_LVAL_P(value) < 0 ? -C_LVAL_P(value) : C_LVAL_P(value));
		}
	} else {
		CREX_ASSERT(0 && "Unexpected type");
	}
}
/* }}} */

/* {{{ Returns the next highest integer value of the number */
CRX_FUNCTION(ceil)
{
	zval *value;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_NUMBER(value)
	CREX_PARSE_PARAMETERS_END();

	if (C_TYPE_P(value) == IS_DOUBLE) {
		RETURN_DOUBLE(ceil(C_DVAL_P(value)));
	} else if (C_TYPE_P(value) == IS_LONG) {
		RETURN_DOUBLE(zval_get_double(value));
	} else {
		CREX_ASSERT(0 && "Unexpected type");
	}
}
/* }}} */

/* {{{ Returns the next lowest integer value from the number */
CRX_FUNCTION(floor)
{
	zval *value;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_NUMBER(value)
	CREX_PARSE_PARAMETERS_END();

	if (C_TYPE_P(value) == IS_DOUBLE) {
		RETURN_DOUBLE(floor(C_DVAL_P(value)));
	} else if (C_TYPE_P(value) == IS_LONG) {
		RETURN_DOUBLE(zval_get_double(value));
	} else {
		CREX_ASSERT(0 && "Unexpected type");
	}
}
/* }}} */

/* {{{ Returns the number rounded to specified precision */
CRX_FUNCTION(round)
{
	zval *value;
	int places = 0;
	crex_long precision = 0;
	crex_long mode = CRX_ROUND_HALF_UP;
	double return_val;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_NUMBER(value)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(precision)
		C_PARAM_LONG(mode)
	CREX_PARSE_PARAMETERS_END();

	if (CREX_NUM_ARGS() >= 2) {
		if (precision >= 0) {
			places = CREX_LONG_INT_OVFL(precision) ? INT_MAX : (int)precision;
		} else {
			places = CREX_LONG_INT_UDFL(precision) ? INT_MIN : (int)precision;
		}
	}

	switch (C_TYPE_P(value)) {
		case IS_LONG:
			/* Simple case - long that doesn't need to be rounded. */
			if (places >= 0) {
				RETURN_DOUBLE((double) C_LVAL_P(value));
			}
			CREX_FALLTHROUGH;

		case IS_DOUBLE:
			return_val = (C_TYPE_P(value) == IS_LONG) ? (double)C_LVAL_P(value) : C_DVAL_P(value);
			return_val = _crx_math_round(return_val, (int)places, (int)mode);
			RETURN_DOUBLE(return_val);
			break;

		EMPTY_SWITCH_DEFAULT_CASE()
	}
}
/* }}} */

/* {{{ Returns the sine of the number in radians */
CRX_FUNCTION(sin)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();
	RETURN_DOUBLE(sin(num));
}
/* }}} */

/* {{{ Returns the cosine of the number in radians */
CRX_FUNCTION(cos)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();
	RETURN_DOUBLE(cos(num));
}
/* }}} */

/* {{{ Returns the tangent of the number in radians */
CRX_FUNCTION(tan)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();
	RETURN_DOUBLE(tan(num));
}
/* }}} */

/* {{{ Returns the arc sine of the number in radians */
CRX_FUNCTION(asin)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();
	RETURN_DOUBLE(asin(num));
}
/* }}} */

/* {{{ Return the arc cosine of the number in radians */
CRX_FUNCTION(acos)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();
	RETURN_DOUBLE(acos(num));
}
/* }}} */

/* {{{ Returns the arc tangent of the number in radians */
CRX_FUNCTION(atan)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();
	RETURN_DOUBLE(atan(num));
}
/* }}} */

/* {{{ Returns the arc tangent of y/x, with the resulting quadrant determined by the signs of y and x */
CRX_FUNCTION(atan2)
{
	double num1, num2;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_DOUBLE(num1)
		C_PARAM_DOUBLE(num2)
	CREX_PARSE_PARAMETERS_END();
	RETURN_DOUBLE(atan2(num1, num2));
}
/* }}} */

/* {{{ Returns the hyperbolic sine of the number, defined as (exp(number) - exp(-number))/2 */
CRX_FUNCTION(sinh)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();
	RETURN_DOUBLE(sinh(num));
}
/* }}} */

/* {{{ Returns the hyperbolic cosine of the number, defined as (exp(number) + exp(-number))/2 */
CRX_FUNCTION(cosh)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();
	RETURN_DOUBLE(cosh(num));
}
/* }}} */

/* {{{ Returns the hyperbolic tangent of the number, defined as sinh(number)/cosh(number) */
CRX_FUNCTION(tanh)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();
	RETURN_DOUBLE(tanh(num));
}
/* }}} */

/* {{{ Returns the inverse hyperbolic sine of the number, i.e. the value whose hyperbolic sine is number */
CRX_FUNCTION(asinh)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();
	RETURN_DOUBLE(asinh(num));
}
/* }}} */

/* {{{ Returns the inverse hyperbolic cosine of the number, i.e. the value whose hyperbolic cosine is number */
CRX_FUNCTION(acosh)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();
	RETURN_DOUBLE(acosh(num));
}
/* }}} */

/* {{{ Returns the inverse hyperbolic tangent of the number, i.e. the value whose hyperbolic tangent is number */
CRX_FUNCTION(atanh)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();
	RETURN_DOUBLE(atanh(num));
}
/* }}} */

/* {{{ Returns an approximation of pi */
CRX_FUNCTION(pi)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_DOUBLE(M_PI);
}
/* }}} */

/* {{{ Returns whether argument is finite */
CRX_FUNCTION(is_finite)
{
	double dval;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(dval)
	CREX_PARSE_PARAMETERS_END();
	RETURN_BOOL(crex_finite(dval));
}
/* }}} */

/* {{{ Returns whether argument is infinite */
CRX_FUNCTION(is_infinite)
{
	double dval;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(dval)
	CREX_PARSE_PARAMETERS_END();
	RETURN_BOOL(crex_isinf(dval));
}
/* }}} */

/* {{{ Returns whether argument is not a number */
CRX_FUNCTION(is_nan)
{
	double dval;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(dval)
	CREX_PARSE_PARAMETERS_END();
	RETURN_BOOL(crex_isnan(dval));
}
/* }}} */

/* {{{ Returns base raised to the power of exponent. Returns integer result when possible */
CRX_FUNCTION(pow)
{
	zval *zbase, *zexp;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_ZVAL(zbase)
		C_PARAM_ZVAL(zexp)
	CREX_PARSE_PARAMETERS_END();

	pow_function(return_value, zbase, zexp);
}
/* }}} */

/* {{{ Returns e raised to the power of the number */
CRX_FUNCTION(exp)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();

	RETURN_DOUBLE(exp(num));
}
/* }}} */

/* {{{ Returns exp(number) - 1, computed in a way that accurate even when the value of number is close to zero */
CRX_FUNCTION(expm1)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();

	RETURN_DOUBLE(expm1(num));
}
/* }}} */

/* {{{ Returns log(1 + number), computed in a way that accurate even when the value of number is close to zero */
CRX_FUNCTION(log1p)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();

	RETURN_DOUBLE(log1p(num));
}
/* }}} */

/* {{{ Returns the natural logarithm of the number, or the base log if base is specified */
CRX_FUNCTION(log)
{
	double num, base = 0;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_DOUBLE(num)
		C_PARAM_OPTIONAL
		C_PARAM_DOUBLE(base)
	CREX_PARSE_PARAMETERS_END();

	if (CREX_NUM_ARGS() == 1) {
		RETURN_DOUBLE(log(num));
	}

	if (base == 2.0) {
		RETURN_DOUBLE(log2(num));
	}

	if (base == 10.0) {
		RETURN_DOUBLE(log10(num));
	}

	if (base == 1.0) {
		RETURN_DOUBLE(CREX_NAN);
	}

	if (base <= 0.0) {
		crex_argument_value_error(2, "must be greater than 0");
		RETURN_THROWS();
	}

	RETURN_DOUBLE(log(num) / log(base));
}
/* }}} */

/* {{{ Returns the base-10 logarithm of the number */
CRX_FUNCTION(log10)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();

	RETURN_DOUBLE(log10(num));
}
/* }}} */

/* {{{ Returns the square root of the number */
CRX_FUNCTION(sqrt)
{
	double num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(num)
	CREX_PARSE_PARAMETERS_END();

	RETURN_DOUBLE(sqrt(num));
}
/* }}} */

/* {{{ Returns sqrt(num1*num1 + num2*num2) */
CRX_FUNCTION(hypot)
{
	double num1, num2;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_DOUBLE(num1)
		C_PARAM_DOUBLE(num2)
	CREX_PARSE_PARAMETERS_END();

	RETURN_DOUBLE(hypot(num1, num2));
}
/* }}} */

/* {{{ Converts the number in degrees to the radian equivalent */
CRX_FUNCTION(deg2rad)
{
	double deg;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(deg)
	CREX_PARSE_PARAMETERS_END();
	RETURN_DOUBLE((deg / 180.0) * M_PI);
}
/* }}} */

/* {{{ Converts the radian number to the equivalent number in degrees */
CRX_FUNCTION(rad2deg)
{
	double rad;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_DOUBLE(rad)
	CREX_PARSE_PARAMETERS_END();

	RETURN_DOUBLE((rad / M_PI) * 180);
}
/* }}} */

/* {{{ _crx_math_basetolong */
/*
 * Convert a string representation of a base(2-36) number to a long.
 */
CRXAPI crex_long _crx_math_basetolong(zval *arg, int base)
{
	crex_long num = 0, digit, onum;
	crex_long i;
	char c, *s;

	if (C_TYPE_P(arg) != IS_STRING || base < 2 || base > 36) {
		return 0;
	}

	s = C_STRVAL_P(arg);

	for (i = C_STRLEN_P(arg); i > 0; i--) {
		c = *s++;

		digit = (c >= '0' && c <= '9') ? c - '0'
			: (c >= 'A' && c <= 'Z') ? c - 'A' + 10
			: (c >= 'a' && c <= 'z') ? c - 'a' + 10
			: base;

		if (digit >= base) {
			continue;
		}

		onum = num;
		num = num * base + digit;
		if (num > onum)
			continue;

		{

			crx_error_docref(NULL, E_WARNING, "Number %s is too big to fit in long", s);
			return CREX_LONG_MAX;
		}
	}

	return num;
}
/* }}} */

/* {{{ _crx_math_basetozval */
/*
 * Convert a string representation of a base(2-36) number to a zval.
 */
CRXAPI void _crx_math_basetozval(crex_string *str, int base, zval *ret)
{
	crex_long num = 0;
	double fnum = 0;
	int mode = 0;
	char c, *s, *e;
	crex_long cutoff;
	int cutlim;
	int invalidchars = 0;

	s = ZSTR_VAL(str);
	e = s + ZSTR_LEN(str);

	/* Skip leading whitespace */
	while (s < e && isspace(*s)) s++;
	/* Skip trailing whitespace */
	while (s < e && isspace(*(e-1))) e--;

	if (e - s >= 2) {
		if (base == 16 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;
		if (base == 8 && s[0] == '0' && (s[1] == 'o' || s[1] == 'O')) s += 2;
		if (base == 2 && s[0] == '0' && (s[1] == 'b' || s[1] == 'B')) s += 2;
	}

	cutoff = CREX_LONG_MAX / base;
	cutlim = CREX_LONG_MAX % base;

	while (s < e) {
		c = *s++;

		/* might not work for EBCDIC */
		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'A' && c <= 'Z')
			c -= 'A' - 10;
		else if (c >= 'a' && c <= 'z')
			c -= 'a' - 10;
		else {
			invalidchars++;
			continue;
		}

		if (c >= base) {
			invalidchars++;
			continue;
		}

		switch (mode) {
		case 0: /* Integer */
			if (num < cutoff || (num == cutoff && c <= cutlim)) {
				num = num * base + c;
				break;
			} else {
				fnum = (double)num;
				mode = 1;
			}
			CREX_FALLTHROUGH;
		case 1: /* Float */
			fnum = fnum * base + c;
		}
	}

	if (invalidchars > 0) {
		crex_error(E_DEPRECATED, "Invalid characters passed for attempted conversion, these have been ignored");
	}

	if (mode == 1) {
		ZVAL_DOUBLE(ret, fnum);
	} else {
		ZVAL_LONG(ret, num);
	}
}
/* }}} */

/* {{{ _crx_math_longtobase */
/*
 * Convert a long to a string containing a base(2-36) representation of
 * the number.
 */
CRXAPI crex_string * _crx_math_longtobase(crex_long arg, int base)
{
	static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char buf[(sizeof(crex_ulong) << 3) + 1];
	char *ptr, *end;
	crex_ulong value;

	if (base < 2 || base > 36) {
		return ZSTR_EMPTY_ALLOC();
	}

	value = arg;

	end = ptr = buf + sizeof(buf) - 1;
	*ptr = '\0';

	do {
		CREX_ASSERT(ptr > buf);
		*--ptr = digits[value % base];
		value /= base;
	} while (value);

	return crex_string_init(ptr, end - ptr, 0);
}
/* }}} */

/* {{{ _crx_math_longtobase_pwr2 */
/*
 * Convert a long to a string containing a base(2,4,6,16,32) representation of
 * the number.
 */
static crex_always_inline crex_string * _crx_math_longtobase_pwr2(crex_long arg, int base_log2)
{
	static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	crex_ulong value;
	size_t len;
	crex_string *ret;
	char *ptr;

	value = arg;

	if (value == 0) {
		len = 1;
	} else {
		len = ((sizeof(value) * 8 - crex_ulong_nlz(value)) + (base_log2 - 1)) / base_log2;
	}

	ret = crex_string_alloc(len, 0);
	ptr = ZSTR_VAL(ret) + len;
	*ptr = '\0';

	do {
		CREX_ASSERT(ptr > ZSTR_VAL(ret));
		*--ptr = digits[value & ((1 << base_log2) - 1)];
		value >>= base_log2;
	} while (value);

	return ret;
}
/* }}} */

/* {{{ _crx_math_zvaltobase */
/*
 * Convert a zval to a string containing a base(2-36) representation of
 * the number.
 */
CRXAPI crex_string * _crx_math_zvaltobase(zval *arg, int base)
{
	static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

	if ((C_TYPE_P(arg) != IS_LONG && C_TYPE_P(arg) != IS_DOUBLE) || base < 2 || base > 36) {
		return ZSTR_EMPTY_ALLOC();
	}

	if (C_TYPE_P(arg) == IS_DOUBLE) {
		double fvalue = floor(C_DVAL_P(arg)); /* floor it just in case */
		char *ptr, *end;
		char buf[(sizeof(double) << 3) + 1];

		/* Don't try to convert +/- infinity */
		if (fvalue == CREX_INFINITY || fvalue == -CREX_INFINITY) {
			crex_value_error("An infinite value cannot be converted to base %d", base);
			return NULL;
		}

		end = ptr = buf + sizeof(buf) - 1;
		*ptr = '\0';

		do {
			*--ptr = digits[(int) fmod(fvalue, base)];
			fvalue /= base;
		} while (ptr > buf && fabs(fvalue) >= 1);

		return crex_string_init(ptr, end - ptr, 0);
	}

	return _crx_math_longtobase(C_LVAL_P(arg), base);
}
/* }}} */

/* {{{ Returns the decimal equivalent of the binary number */
CRX_FUNCTION(bindec)
{
	crex_string *arg;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(arg)
	CREX_PARSE_PARAMETERS_END();

	_crx_math_basetozval(arg, 2, return_value);
}
/* }}} */

/* {{{ Returns the decimal equivalent of the hexadecimal number */
CRX_FUNCTION(hexdec)
{
	crex_string *arg;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(arg)
	CREX_PARSE_PARAMETERS_END();

	_crx_math_basetozval(arg, 16, return_value);
}
/* }}} */

/* {{{ Returns the decimal equivalent of an octal string */
CRX_FUNCTION(octdec)
{
	crex_string *arg;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(arg)
	CREX_PARSE_PARAMETERS_END();

	_crx_math_basetozval(arg, 8, return_value);
}
/* }}} */

/* {{{ Returns a string containing a binary representation of the number */
CRX_FUNCTION(decbin)
{
	crex_long arg;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(arg)
	CREX_PARSE_PARAMETERS_END();

	RETURN_STR(_crx_math_longtobase_pwr2(arg, 1));
}
/* }}} */

/* {{{ Returns a string containing an octal representation of the given number */
CRX_FUNCTION(decoct)
{
	crex_long arg;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(arg)
	CREX_PARSE_PARAMETERS_END();

	RETURN_STR(_crx_math_longtobase_pwr2(arg, 3));
}
/* }}} */

/* {{{ Returns a string containing a hexadecimal representation of the given number */
CRX_FUNCTION(dechex)
{
	crex_long arg;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(arg)
	CREX_PARSE_PARAMETERS_END();

	RETURN_STR(_crx_math_longtobase_pwr2(arg, 4));
}
/* }}} */

/* {{{ Converts a number in a string from any base <= 36 to any base <= 36 */
CRX_FUNCTION(base_convert)
{
	zval temp;
	crex_string *number;
	crex_long frombase, tobase;
	crex_string *result;

	CREX_PARSE_PARAMETERS_START(3, 3)
		C_PARAM_STR(number)
		C_PARAM_LONG(frombase)
		C_PARAM_LONG(tobase)
	CREX_PARSE_PARAMETERS_END();

	if (frombase < 2 || frombase > 36) {
		crex_argument_value_error(2, "must be between 2 and 36 (inclusive)");
		RETURN_THROWS();
	}
	if (tobase < 2 || tobase > 36) {
		crex_argument_value_error(3, "must be between 2 and 36 (inclusive)");
		RETURN_THROWS();
	}

	_crx_math_basetozval(number, (int)frombase, &temp);
	result = _crx_math_zvaltobase(&temp, (int)tobase);
	if (!result) {
		RETURN_THROWS();
	}

	RETVAL_STR(result);
}
/* }}} */

/* {{{ _crx_math_number_format */
CRXAPI crex_string *_crx_math_number_format(double d, int dec, char dec_point, char thousand_sep)
{
	return _crx_math_number_format_ex(d, dec, &dec_point, 1, &thousand_sep, 1);
}

CRXAPI crex_string *_crx_math_number_format_ex(double d, int dec, const char *dec_point,
		size_t dec_point_len, const char *thousand_sep, size_t thousand_sep_len)
{
	crex_string *res;
	crex_string *tmpbuf;
	char *s, *t;  /* source, target */
	char *dp;
	size_t integral;
	size_t reslen = 0;
	int count = 0;
	int is_negative = 0;

	if (d < 0) {
		is_negative = 1;
		d = -d;
	}

	d = _crx_math_round(d, dec, CRX_ROUND_HALF_UP);
	dec = MAX(0, dec);
	tmpbuf = strpprintf(0, "%.*F", dec, d);
	if (tmpbuf == NULL) {
		return NULL;
	} else if (!isdigit((int)ZSTR_VAL(tmpbuf)[0])) {
		return tmpbuf;
	}

	/* Check if the number is no longer negative after rounding */
	if (is_negative && d == 0) {
		is_negative = 0;
	}

	/* find decimal point, if expected */
	if (dec) {
		dp = strpbrk(ZSTR_VAL(tmpbuf), ".,");
	} else {
		dp = NULL;
	}

	/* calculate the length of the return buffer */
	if (dp) {
		integral = (dp - ZSTR_VAL(tmpbuf));
	} else {
		/* no decimal point was found */
		integral = ZSTR_LEN(tmpbuf);
	}

	/* allow for thousand separators */
	if (thousand_sep) {
		integral = crex_safe_addmult((integral-1)/3, thousand_sep_len, integral, "number formatting");
	}

	reslen = integral;

	if (dec) {
		reslen += dec;

		if (dec_point) {
			reslen = crex_safe_addmult(reslen, 1, dec_point_len, "number formatting");
		}
	}

	/* add a byte for minus sign */
	if (is_negative) {
		reslen++;
	}
	res = crex_string_alloc(reslen, 0);

	s = ZSTR_VAL(tmpbuf) + ZSTR_LEN(tmpbuf) - 1;
	t = ZSTR_VAL(res) + reslen;
	*t-- = '\0';

	/* copy the decimal places.
	 * Take care, as the sprintf implementation may return less places than
	 * we requested due to internal buffer limitations */
	if (dec) {
		size_t declen = (dp ? s - dp : 0);
		size_t topad = (size_t)dec > declen ? dec - declen : 0;

		/* pad with '0's */
		while (topad--) {
			*t-- = '0';
		}

		if (dp) {
			s -= declen + 1; /* +1 to skip the point */
			t -= declen;

			/* now copy the chars after the point */
			memcpy(t + 1, dp + 1, declen);
		}

		/* add decimal point */
		if (dec_point) {
			t -= dec_point_len;
			memcpy(t + 1, dec_point, dec_point_len);
		}
	}

	/* copy the numbers before the decimal point, adding thousand
	 * separator every three digits */
	while (s >= ZSTR_VAL(tmpbuf)) {
		*t-- = *s--;
		if (thousand_sep && (++count%3)==0 && s >= ZSTR_VAL(tmpbuf)) {
			t -= thousand_sep_len;
			memcpy(t + 1, thousand_sep, thousand_sep_len);
		}
	}

	/* and a minus sign, if needed */
	if (is_negative) {
		*t-- = '-';
	}

	ZSTR_LEN(res) = reslen;
	crex_string_release_ex(tmpbuf, 0);
	return res;
}

CRXAPI crex_string *_crx_math_number_format_long(crex_long num, crex_long dec, const char *dec_point,
		size_t dec_point_len, const char *thousand_sep, size_t thousand_sep_len)
{
	static const crex_ulong powers[] = {
		1, 10, 100, 1000, 10000,
		100000, 1000000, 10000000, 100000000, 1000000000,
#if SIZEOF_CREX_LONG == 8
		10000000000, 100000000000, 1000000000000, 10000000000000, 100000000000000,
		1000000000000000, 10000000000000000, 100000000000000000, 1000000000000000000, 10000000000000000000ul
#elif SIZEOF_CREX_LONG > 8
# error "Unknown SIZEOF_CREX_LONG"
#endif
	};

	int is_negative = 0;
	crex_ulong tmpnum;
	crex_ulong power;
	crex_ulong power_half;
	crex_ulong rest;

	crex_string *tmpbuf;
	crex_string *res;
	size_t reslen;
	char *s, *t;  /* source, target */
	int count = 0;
	size_t topad;

	// unsigned absolute number and memorize negative sign
	if (num < 0) {
		is_negative = 1;
		tmpnum = ((crex_ulong)-(num + 1)) + 1;
	} else {
		tmpnum = (crex_ulong)num;
	}

	// rounding the number
	if (dec < 0) {
		// Check rounding to more negative places than possible
		if (dec < -(sizeof(powers) / sizeof(powers[0]) - 1)) {
			tmpnum = 0;
		} else {
			power = powers[-dec];
			power_half = power / 2;
			rest = tmpnum % power;
			tmpnum = tmpnum / power;

			if (rest >= power_half) {
				tmpnum = tmpnum * power + power;
			} else {
				tmpnum = tmpnum * power;
			}
		}

		// prevent resulting in negative zero
		if (tmpnum == 0) {
			is_negative = 0;
		}
	}

	tmpbuf = strpprintf(0, CREX_ULONG_FMT, tmpnum);
	reslen = ZSTR_LEN(tmpbuf);

	/* allow for thousand separators */
	if (thousand_sep) {
		reslen = crex_safe_addmult((reslen-1)/3, thousand_sep_len, reslen, "number formatting");
	}

	reslen += is_negative;

	if (dec > 0) {
		reslen += dec;

		if (dec_point) {
			reslen = crex_safe_addmult(reslen, 1, dec_point_len, "number formatting");
		}
	}

	res = crex_string_alloc(reslen, 0);

	s = ZSTR_VAL(tmpbuf) + ZSTR_LEN(tmpbuf) - 1;
	t = ZSTR_VAL(res) + reslen;
	*t-- = '\0';

	/* copy the decimal places. */
	if (dec > 0) {
		topad = (size_t)dec;

		/* pad with '0's */
		while (topad--) {
			*t-- = '0';
		}

		/* add decimal point */
		if (dec_point) {
			t -= dec_point_len;
			memcpy(t + 1, dec_point, dec_point_len);
		}
	}

	/* copy the numbers before the decimal point, adding thousand
	 * separator every three digits */
	while (s >= ZSTR_VAL(tmpbuf)) {
		*t-- = *s--;
		if (thousand_sep && (++count % 3) == 0 && s >= ZSTR_VAL(tmpbuf)) {
			t -= thousand_sep_len;
			memcpy(t + 1, thousand_sep, thousand_sep_len);
		}
	}

	if (is_negative) {
		*t-- = '-';
	}

	ZSTR_LEN(res) = reslen;
	crex_string_release_ex(tmpbuf, 0);
	return res;
}

/* {{{ Formats a number with grouped thousands */
CRX_FUNCTION(number_format)
{
	zval* num;
	crex_long dec = 0;
	int dec_int;
	char *thousand_sep = NULL, *dec_point = NULL;
	size_t thousand_sep_len = 0, dec_point_len = 0;

	CREX_PARSE_PARAMETERS_START(1, 4)
		C_PARAM_NUMBER(num)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(dec)
		C_PARAM_STRING_OR_NULL(dec_point, dec_point_len)
		C_PARAM_STRING_OR_NULL(thousand_sep, thousand_sep_len)
	CREX_PARSE_PARAMETERS_END();

	if (dec_point == NULL) {
		dec_point = ".";
		dec_point_len = 1;
	}
	if (thousand_sep == NULL) {
		thousand_sep = ",";
		thousand_sep_len = 1;
	}

	switch (C_TYPE_P(num)) {
		case IS_LONG:
			RETURN_STR(_crx_math_number_format_long(C_LVAL_P(num), dec, dec_point, dec_point_len, thousand_sep, thousand_sep_len));
			break;

		case IS_DOUBLE:
			if (dec >= 0) {
				dec_int = CREX_LONG_INT_OVFL(dec) ? INT_MAX : (int)dec;
			} else {
				dec_int = CREX_LONG_INT_UDFL(dec) ? INT_MIN : (int)dec;
			}
			RETURN_STR(_crx_math_number_format_ex(C_DVAL_P(num), dec_int, dec_point, dec_point_len, thousand_sep, thousand_sep_len));
			break;

		EMPTY_SWITCH_DEFAULT_CASE()
	}
}
/* }}} */

/* {{{ Returns the remainder of dividing x by y as a float */
CRX_FUNCTION(fmod)
{
	double num1, num2;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_DOUBLE(num1)
		C_PARAM_DOUBLE(num2)
	CREX_PARSE_PARAMETERS_END();

	RETURN_DOUBLE(fmod(num1, num2));
}
/* }}} */

/* {{{ Perform floating-point division of dividend / divisor
   with IEEE-754 semantics for division by zero. */
#ifdef __clang__
__attribute__((no_sanitize("float-divide-by-zero")))
#endif
CRX_FUNCTION(fdiv)
{
	double dividend, divisor;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_DOUBLE(dividend)
		C_PARAM_DOUBLE(divisor)
	CREX_PARSE_PARAMETERS_END();

	RETURN_DOUBLE(dividend / divisor);
}
/* }}} */

/* {{{ Returns the integer quotient of the division of dividend by divisor */
CRX_FUNCTION(intdiv)
{
	crex_long dividend, divisor;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_LONG(dividend)
		C_PARAM_LONG(divisor)
	CREX_PARSE_PARAMETERS_END();

	if (divisor == 0) {
		crex_throw_exception_ex(crex_ce_division_by_zero_error, 0, "Division by zero");
		RETURN_THROWS();
	} else if (divisor == -1 && dividend == CREX_LONG_MIN) {
		/* Prevent overflow error/crash ... really should not happen:
		   We don't return a float here as that violates function contract */
		crex_throw_exception_ex(crex_ce_arithmetic_error, 0, "Division of CRX_INT_MIN by -1 is not an integer");
		RETURN_THROWS();
	}

	RETURN_LONG(dividend / divisor);
}
/* }}} */
