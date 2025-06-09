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
   | Author: Andi Gutmans <andi@crx.net>                                  |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"

#ifdef HAVE_BCMATH

#include "crx_ini.h"
#include "crex_exceptions.h"
#include "bcmath_arginfo.h"
#include "ext/standard/info.h"
#include "crx_bcmath.h"
#include "libbcmath/src/bcmath.h"

CREX_DECLARE_MODULE_GLOBALS(bcmath)
static CRX_GINIT_FUNCTION(bcmath);
static CRX_GSHUTDOWN_FUNCTION(bcmath);
static CRX_MINIT_FUNCTION(bcmath);
static CRX_MSHUTDOWN_FUNCTION(bcmath);
static CRX_MINFO_FUNCTION(bcmath);

crex_module_entry bcmath_module_entry = {
	STANDARD_MODULE_HEADER,
	"bcmath",
	ext_functions,
	CRX_MINIT(bcmath),
	CRX_MSHUTDOWN(bcmath),
	NULL,
	NULL,
	CRX_MINFO(bcmath),
	CRX_BCMATH_VERSION,
	CRX_MODULE_GLOBALS(bcmath),
	CRX_GINIT(bcmath),
	CRX_GSHUTDOWN(bcmath),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_BCMATH
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(bcmath)
#endif

CREX_INI_MH(OnUpdateScale)
{
	int *p;
	crex_long tmp;

	tmp = crex_ini_parse_quantity_warn(new_value, entry->name);
	if (tmp < 0 || tmp > INT_MAX) {
		return FAILURE;
	}

	p = (int *) CREX_INI_GET_ADDR();
	*p = (int) tmp;

	return SUCCESS;
}

/* {{{ CRX_INI */
CRX_INI_BEGIN()
	STD_CRX_INI_ENTRY("bcmath.scale", "0", CRX_INI_ALL, OnUpdateScale, bc_precision, crex_bcmath_globals, bcmath_globals)
CRX_INI_END()
/* }}} */

/* {{{ CRX_GINIT_FUNCTION */
static CRX_GINIT_FUNCTION(bcmath)
{
#if defined(COMPILE_DL_BCMATH) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	bcmath_globals->bc_precision = 0;
	bc_init_numbers();
}
/* }}} */

/* {{{ CRX_GSHUTDOWN_FUNCTION */
static CRX_GSHUTDOWN_FUNCTION(bcmath)
{
	_bc_free_num_ex(&bcmath_globals->_zero_, 1);
	_bc_free_num_ex(&bcmath_globals->_one_, 1);
	_bc_free_num_ex(&bcmath_globals->_two_, 1);
}
/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(bcmath)
{
	REGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(bcmath)
{
	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(bcmath)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "BCMath support", "enabled");
	crx_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ crx_str2num
   Convert to bc_num detecting scale */
static crex_result crx_str2num(bc_num *num, char *str)
{
	char *p;

	if (!(p = strchr(str, '.'))) {
		if (!bc_str2num(num, str, 0)) {
			return FAILURE;
		}

		return SUCCESS;
	}

	if (!bc_str2num(num, str, strlen(p+1))) {
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ Returns the sum of two arbitrary precision numbers */
CRX_FUNCTION(bcadd)
{
	crex_string *left, *right;
	crex_long scale_param;
	bool scale_param_is_null = 1;
	bc_num first, second, result;
	int scale;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(left)
		C_PARAM_STR(right)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(scale_param, scale_param_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (scale_param_is_null) {
		scale = BCG(bc_precision);
	} else if (scale_param < 0 || scale_param > INT_MAX) {
		crex_argument_value_error(3, "must be between 0 and %d", INT_MAX);
		RETURN_THROWS();
	} else {
		scale = (int) scale_param;
	}

	bc_init_num(&first);
	bc_init_num(&second);
	bc_init_num(&result);

	if (crx_str2num(&first, ZSTR_VAL(left)) == FAILURE) {
		crex_argument_value_error(1, "is not well-formed");
		goto cleanup;
	}

	if (crx_str2num(&second, ZSTR_VAL(right)) == FAILURE) {
		crex_argument_value_error(2, "is not well-formed");
		goto cleanup;
	}

	bc_add (first, second, &result, scale);

	RETVAL_STR(bc_num2str_ex(result, scale));

	cleanup: {
		bc_free_num(&first);
		bc_free_num(&second);
		bc_free_num(&result);
	};
}
/* }}} */

/* {{{ Returns the difference between two arbitrary precision numbers */
CRX_FUNCTION(bcsub)
{
	crex_string *left, *right;
	crex_long scale_param;
	bool scale_param_is_null = 1;
	bc_num first, second, result;
	int scale;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(left)
		C_PARAM_STR(right)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(scale_param, scale_param_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (scale_param_is_null) {
		scale = BCG(bc_precision);
	} else if (scale_param < 0 || scale_param > INT_MAX) {
		crex_argument_value_error(3, "must be between 0 and %d", INT_MAX);
		RETURN_THROWS();
	} else {
		scale = (int) scale_param;
	}

	bc_init_num(&first);
	bc_init_num(&second);
	bc_init_num(&result);

	if (crx_str2num(&first, ZSTR_VAL(left)) == FAILURE) {
		crex_argument_value_error(1, "is not well-formed");
		goto cleanup;
	}

	if (crx_str2num(&second, ZSTR_VAL(right)) == FAILURE) {
		crex_argument_value_error(2, "is not well-formed");
		goto cleanup;
	}

	bc_sub (first, second, &result, scale);

	RETVAL_STR(bc_num2str_ex(result, scale));

	cleanup: {
		bc_free_num(&first);
		bc_free_num(&second);
		bc_free_num(&result);
	};
}
/* }}} */

/* {{{ Returns the multiplication of two arbitrary precision numbers */
CRX_FUNCTION(bcmul)
{
	crex_string *left, *right;
	crex_long scale_param;
	bool scale_param_is_null = 1;
	bc_num first, second, result;
	int scale;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(left)
		C_PARAM_STR(right)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(scale_param, scale_param_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (scale_param_is_null) {
		scale = BCG(bc_precision);
	} else if (scale_param < 0 || scale_param > INT_MAX) {
		crex_argument_value_error(3, "must be between 0 and %d", INT_MAX);
		RETURN_THROWS();
	} else {
		scale = (int) scale_param;
	}

	bc_init_num(&first);
	bc_init_num(&second);
	bc_init_num(&result);

	if (crx_str2num(&first, ZSTR_VAL(left)) == FAILURE) {
		crex_argument_value_error(1, "is not well-formed");
		goto cleanup;
	}

	if (crx_str2num(&second, ZSTR_VAL(right)) == FAILURE) {
		crex_argument_value_error(2, "is not well-formed");
		goto cleanup;
	}

	bc_multiply (first, second, &result, scale);

	RETVAL_STR(bc_num2str_ex(result, scale));

	cleanup: {
		bc_free_num(&first);
		bc_free_num(&second);
		bc_free_num(&result);
	};
}
/* }}} */

/* {{{ Returns the quotient of two arbitrary precision numbers (division) */
CRX_FUNCTION(bcdiv)
{
	crex_string *left, *right;
	crex_long scale_param;
	bool scale_param_is_null = 1;
	bc_num first, second, result;
	int scale = BCG(bc_precision);

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(left)
		C_PARAM_STR(right)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(scale_param, scale_param_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (scale_param_is_null) {
		scale = BCG(bc_precision);
	} else if (scale_param < 0 || scale_param > INT_MAX) {
		crex_argument_value_error(3, "must be between 0 and %d", INT_MAX);
		RETURN_THROWS();
	} else {
		scale = (int) scale_param;
	}

	bc_init_num(&first);
	bc_init_num(&second);
	bc_init_num(&result);

	if (crx_str2num(&first, ZSTR_VAL(left)) == FAILURE) {
		crex_argument_value_error(1, "is not well-formed");
		goto cleanup;
	}

	if (crx_str2num(&second, ZSTR_VAL(right)) == FAILURE) {
		crex_argument_value_error(2, "is not well-formed");
		goto cleanup;
	}

	if (!bc_divide(first, second, &result, scale)) {
		crex_throw_exception_ex(crex_ce_division_by_zero_error, 0, "Division by zero");
		goto cleanup;
	}

	RETVAL_STR(bc_num2str_ex(result, scale));

	cleanup: {
		bc_free_num(&first);
		bc_free_num(&second);
		bc_free_num(&result);
	};
}
/* }}} */

/* {{{ Returns the modulus of the two arbitrary precision operands */
CRX_FUNCTION(bcmod)
{
	crex_string *left, *right;
	crex_long scale_param;
	bool scale_param_is_null = 1;
	bc_num first, second, result;
	int scale = BCG(bc_precision);

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(left)
		C_PARAM_STR(right)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(scale_param, scale_param_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (scale_param_is_null) {
		scale = BCG(bc_precision);
	} else if (scale_param < 0 || scale_param > INT_MAX) {
		crex_argument_value_error(3, "must be between 0 and %d", INT_MAX);
		RETURN_THROWS();
	} else {
		scale = (int) scale_param;
	}

	bc_init_num(&first);
	bc_init_num(&second);
	bc_init_num(&result);

	if (crx_str2num(&first, ZSTR_VAL(left)) == FAILURE) {
		crex_argument_value_error(1, "is not well-formed");
		goto cleanup;
	}

	if (crx_str2num(&second, ZSTR_VAL(right)) == FAILURE) {
		crex_argument_value_error(2, "is not well-formed");
		goto cleanup;
	}

	if (!bc_modulo(first, second, &result, scale)) {
		crex_throw_exception_ex(crex_ce_division_by_zero_error, 0, "Modulo by zero");
		goto cleanup;
	}

	RETVAL_STR(bc_num2str_ex(result, scale));

	cleanup: {
		bc_free_num(&first);
		bc_free_num(&second);
		bc_free_num(&result);
	};
}
/* }}} */

/* {{{ Returns the value of an arbitrary precision number raised to the power of another reduced by a modulus */
CRX_FUNCTION(bcpowmod)
{
	crex_string *base_str, *exponent_str, *modulus_str;
	crex_long scale_param;
	bool scale_param_is_null = 1;
	bc_num bc_base, bc_expo, bc_modulus, result;
	int scale = BCG(bc_precision);

	CREX_PARSE_PARAMETERS_START(3, 4)
		C_PARAM_STR(base_str)
		C_PARAM_STR(exponent_str)
		C_PARAM_STR(modulus_str)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(scale_param, scale_param_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (scale_param_is_null) {
		scale = BCG(bc_precision);
	} else if (scale_param < 0 || scale_param > INT_MAX) {
		crex_argument_value_error(4, "must be between 0 and %d", INT_MAX);
		RETURN_THROWS();
	} else {
		scale = (int) scale_param;
	}

	bc_init_num(&bc_base);
	bc_init_num(&bc_expo);
	bc_init_num(&bc_modulus);
	bc_init_num(&result);

	if (crx_str2num(&bc_base, ZSTR_VAL(base_str)) == FAILURE) {
		crex_argument_value_error(1, "is not well-formed");
		goto cleanup;
	}

	if (crx_str2num(&bc_expo, ZSTR_VAL(exponent_str)) == FAILURE) {
		crex_argument_value_error(2, "is not well-formed");
		goto cleanup;
	}

	if (crx_str2num(&bc_modulus, ZSTR_VAL(modulus_str)) == FAILURE) {
		crex_argument_value_error(3, "is not well-formed");
		goto cleanup;
	}

	raise_mod_status status = bc_raisemod(bc_base, bc_expo, bc_modulus, &result, scale);
	switch (status) {
		case BASE_HAS_FRACTIONAL:
			crex_argument_value_error(1, "cannot have a fractional part");
			goto cleanup;
		case EXPO_HAS_FRACTIONAL:
			crex_argument_value_error(2, "cannot have a fractional part");
			goto cleanup;
		case EXPO_IS_NEGATIVE:
			crex_argument_value_error(2, "must be greater than or equal to 0");
			goto cleanup;
		case MOD_HAS_FRACTIONAL:
			crex_argument_value_error(3, "cannot have a fractional part");
			goto cleanup;
		case MOD_IS_ZERO:
			crex_throw_exception_ex(crex_ce_division_by_zero_error, 0, "Modulo by zero");
			goto cleanup;
		case OK:
			RETVAL_STR(bc_num2str_ex(result, scale));
			break;
		EMPTY_SWITCH_DEFAULT_CASE();
	}

	cleanup: {
		bc_free_num(&bc_base);
		bc_free_num(&bc_expo);
		bc_free_num(&bc_modulus);
		bc_free_num(&result);
	};
}
/* }}} */

/* {{{ Returns the value of an arbitrary precision number raised to the power of another */
CRX_FUNCTION(bcpow)
{
	crex_string *base_str, *exponent_str;
	crex_long scale_param;
	bool scale_param_is_null = 1;
	bc_num first, bc_exponent, result;
	int scale = BCG(bc_precision);

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(base_str)
		C_PARAM_STR(exponent_str)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(scale_param, scale_param_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (scale_param_is_null) {
		scale = BCG(bc_precision);
	} else if (scale_param < 0 || scale_param > INT_MAX) {
		crex_argument_value_error(3, "must be between 0 and %d", INT_MAX);
		RETURN_THROWS();
	} else {
		scale = (int) scale_param;
	}

	bc_init_num(&first);
	bc_init_num(&bc_exponent);
	bc_init_num(&result);

	if (crx_str2num(&first, ZSTR_VAL(base_str)) == FAILURE) {
		crex_argument_value_error(1, "is not well-formed");
		goto cleanup;
	}

	if (crx_str2num(&bc_exponent, ZSTR_VAL(exponent_str)) == FAILURE) {
		crex_argument_value_error(2, "is not well-formed");
		goto cleanup;
	}

	/* Check the exponent for scale digits and convert to a long. */
	if (bc_exponent->n_scale != 0) {
		crex_argument_value_error(2, "cannot have a fractional part");
		goto cleanup;
	}
	long exponent = bc_num2long(bc_exponent);
	if (exponent == 0 && (bc_exponent->n_len > 1 || bc_exponent->n_value[0] != 0)) {
		crex_argument_value_error(2, "is too large");
		goto cleanup;
	}

	bc_raise(first, exponent, &result, scale);

	RETVAL_STR(bc_num2str_ex(result, scale));

	cleanup: {
		bc_free_num(&first);
		bc_free_num(&bc_exponent);
		bc_free_num(&result);
	};
}
/* }}} */

/* {{{ Returns the square root of an arbitrary precision number */
CRX_FUNCTION(bcsqrt)
{
	crex_string *left;
	crex_long scale_param;
	bool scale_param_is_null = 1;
	bc_num result;
	int scale = BCG(bc_precision);

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(left)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(scale_param, scale_param_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (scale_param_is_null) {
		scale = BCG(bc_precision);
	} else if (scale_param < 0 || scale_param > INT_MAX) {
		crex_argument_value_error(2, "must be between 0 and %d", INT_MAX);
		RETURN_THROWS();
	} else {
		scale = (int) scale_param;
	}

	bc_init_num(&result);

	if (crx_str2num(&result, ZSTR_VAL(left)) == FAILURE) {
		crex_argument_value_error(1, "is not well-formed");
		goto cleanup;
	}

	if (bc_sqrt (&result, scale) != 0) {
		RETVAL_STR(bc_num2str_ex(result, scale));
	} else {
		crex_argument_value_error(1, "must be greater than or equal to 0");
	}

	cleanup: {
		bc_free_num(&result);
	};
}
/* }}} */

/* {{{ Compares two arbitrary precision numbers */
CRX_FUNCTION(bccomp)
{
	crex_string *left, *right;
	crex_long scale_param;
	bool scale_param_is_null = 1;
	bc_num first, second;
	int scale = BCG(bc_precision);

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(left)
		C_PARAM_STR(right)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(scale_param, scale_param_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (scale_param_is_null) {
		scale = BCG(bc_precision);
	} else if (scale_param < 0 || scale_param > INT_MAX) {
		crex_argument_value_error(3, "must be between 0 and %d", INT_MAX);
		RETURN_THROWS();
	} else {
		scale = (int) scale_param;
	}

	bc_init_num(&first);
	bc_init_num(&second);

	if (!bc_str2num(&first, ZSTR_VAL(left), scale)) {
		crex_argument_value_error(1, "is not well-formed");
		goto cleanup;
	}

	if (!bc_str2num(&second, ZSTR_VAL(right), scale)) {
		crex_argument_value_error(2, "is not well-formed");
		goto cleanup;
	}

	RETVAL_LONG(bc_compare(first, second));

	cleanup: {
		bc_free_num(&first);
		bc_free_num(&second);
	};
}
/* }}} */

/* {{{ Sets default scale parameter for all bc math functions */
CRX_FUNCTION(bcscale)
{
	crex_long old_scale, new_scale;
	bool new_scale_is_null = 1;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(new_scale, new_scale_is_null)
	CREX_PARSE_PARAMETERS_END();

	old_scale = BCG(bc_precision);

	if (!new_scale_is_null) {
		if (new_scale < 0 || new_scale > INT_MAX) {
			crex_argument_value_error(1, "must be between 0 and %d", INT_MAX);
			RETURN_THROWS();
		}

		crex_string *ini_name = ZSTR_INIT_LITERAL("bcmath.scale", 0);
		crex_string *new_scale_str = crex_long_to_str(new_scale);
		crex_alter_ini_entry(ini_name, new_scale_str, CRX_INI_USER, CRX_INI_STAGE_RUNTIME);
		crex_string_release(new_scale_str);
		crex_string_release(ini_name);
	}

	RETURN_LONG(old_scale);
}
/* }}} */


#endif
