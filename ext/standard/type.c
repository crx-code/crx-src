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
   | Author: Rasmus Lerdorf <rasmus@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "crx_incomplete_class.h"

/* {{{ Returns the type of the variable */
CRX_FUNCTION(gettype)
{
	zval *arg;
	crex_string *type;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(arg)
	CREX_PARSE_PARAMETERS_END();

	type = crex_zval_get_legacy_type(arg);
	if (EXPECTED(type)) {
		RETURN_INTERNED_STR(type);
	} else {
		RETURN_STRING("unknown type");
	}
}
/* }}} */

/* {{{ Returns the type of the variable resolving class names */
CRX_FUNCTION(get_debug_type)
{
	zval *arg;
	const char *name;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(arg)
	CREX_PARSE_PARAMETERS_END();

	switch (C_TYPE_P(arg)) {
		case IS_NULL:
			RETURN_INTERNED_STR(ZSTR_KNOWN(CREX_STR_NULL_LOWERCASE));
		case IS_FALSE:
		case IS_TRUE:
			RETURN_INTERNED_STR(ZSTR_KNOWN(CREX_STR_BOOL));
		case IS_LONG:
			RETURN_INTERNED_STR(ZSTR_KNOWN(CREX_STR_INT));
		case IS_DOUBLE:
			RETURN_INTERNED_STR(ZSTR_KNOWN(CREX_STR_FLOAT));
		case IS_STRING:
			RETURN_INTERNED_STR(ZSTR_KNOWN(CREX_STR_STRING));
		case IS_ARRAY:
			RETURN_INTERNED_STR(ZSTR_KNOWN(CREX_STR_ARRAY));
		case IS_OBJECT:
			if (C_OBJ_P(arg)->ce->ce_flags & CREX_ACC_ANON_CLASS) {
				name = ZSTR_VAL(C_OBJ_P(arg)->ce->name);
				RETURN_NEW_STR(crex_string_init(name, strlen(name), 0));
			} else {
				RETURN_STR_COPY(C_OBJ_P(arg)->ce->name);
			}
		case IS_RESOURCE:
			name = crex_rsrc_list_get_rsrc_type(C_RES_P(arg));
			if (name) {
				RETURN_NEW_STR(crex_strpprintf(0, "resource (%s)", name));
			} else {
				RETURN_INTERNED_STR(ZSTR_KNOWN(CREX_STR_CLOSED_RESOURCE));
			}
		default:
			RETURN_INTERNED_STR(ZSTR_KNOWN(CREX_STR_UNKNOWN));
	}
}
/* }}} */


/* {{{ Set the type of the variable */
CRX_FUNCTION(settype)
{
	zval *var;
	crex_string *type;
	zval tmp, *ptr;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_ZVAL(var)
		C_PARAM_STR(type)
	CREX_PARSE_PARAMETERS_END();

	CREX_ASSERT(C_ISREF_P(var));
	if (UNEXPECTED(CREX_REF_HAS_TYPE_SOURCES(C_REF_P(var)))) {
		ZVAL_COPY(&tmp, C_REFVAL_P(var));
		ptr = &tmp;
	} else {
		ptr = C_REFVAL_P(var);
	}
	if (crex_string_equals_ci(type, ZSTR_KNOWN(CREX_STR_INTEGER))) {
		convert_to_long(ptr);
	} else if (crex_string_equals_ci(type, ZSTR_KNOWN(CREX_STR_INT))) {
		convert_to_long(ptr);
	} else if (crex_string_equals_ci(type, ZSTR_KNOWN(CREX_STR_FLOAT))) {
		convert_to_double(ptr);
	} else if (crex_string_equals_ci(type, ZSTR_KNOWN(CREX_STR_DOUBLE))) { /* deprecated */
		convert_to_double(ptr);
	} else if (crex_string_equals_ci(type, ZSTR_KNOWN(CREX_STR_STRING))) {
		convert_to_string(ptr);
	} else if (crex_string_equals_ci(type, ZSTR_KNOWN(CREX_STR_ARRAY))) {
		convert_to_array(ptr);
	} else if (crex_string_equals_ci(type, ZSTR_KNOWN(CREX_STR_OBJECT))) {
		convert_to_object(ptr);
	} else if (crex_string_equals_ci(type, ZSTR_KNOWN(CREX_STR_BOOL))) {
		convert_to_boolean(ptr);
	} else if (crex_string_equals_ci(type, ZSTR_KNOWN(CREX_STR_BOOLEAN))) {
		convert_to_boolean(ptr);
	} else if (crex_string_equals_ci(type, ZSTR_KNOWN(CREX_STR_NULL_LOWERCASE))) {
		convert_to_null(ptr);
	} else {
		if (ptr == &tmp) {
			zval_ptr_dtor(&tmp);
		}
		if (crex_string_equals_ci(type, ZSTR_KNOWN(CREX_STR_RESOURCE))) {
			crex_value_error("Cannot convert to resource type");
		} else {
			crex_argument_value_error(2, "must be a valid type");
		}
		RETURN_THROWS();
	}

	if (ptr == &tmp) {
		crex_try_assign_typed_ref(C_REF_P(var), &tmp);
	}
	RETVAL_TRUE;
}
/* }}} */

/* {{{ Get the integer value of a variable using the optional base for the conversion */
CRX_FUNCTION(intval)
{
	zval *num;
	crex_long base = 10;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_ZVAL(num)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(base)
	CREX_PARSE_PARAMETERS_END();

	if (C_TYPE_P(num) != IS_STRING || base == 10) {
		RETVAL_LONG(zval_get_long(num));
		return;
	}


	if (base == 0 || base == 2) {
		char *strval = C_STRVAL_P(num);
		size_t strlen = C_STRLEN_P(num);

		while (isspace(*strval) && strlen) {
			strval++;
			strlen--;
		}

		/* Length of 3+ covers "0b#" and "-0b" (which results in 0) */
		if (strlen > 2) {
			int offset = 0;
			if (strval[0] == '-' || strval[0] == '+') {
				offset = 1;
			}

			if (strval[offset] == '0' && (strval[offset + 1] == 'b' || strval[offset + 1] == 'B')) {
				char *tmpval;
				strlen -= 2; /* Removing "0b" */
				tmpval = emalloc(strlen + 1);

				/* Place the unary symbol at pos 0 if there was one */
				if (offset) {
					tmpval[0] = strval[0];
				}

				/* Copy the data from after "0b" to the end of the buffer */
				memcpy(tmpval + offset, strval + offset + 2, strlen - offset);
				tmpval[strlen] = 0;

				RETVAL_LONG(CREX_STRTOL(tmpval, NULL, 2));
				efree(tmpval);
				return;
			}
		}
	}

	RETVAL_LONG(CREX_STRTOL(C_STRVAL_P(num), NULL, base));
}
/* }}} */

/* {{{ Get the float value of a variable */
CRX_FUNCTION(floatval)
{
	zval *num;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(num)
	CREX_PARSE_PARAMETERS_END();

	RETURN_DOUBLE(zval_get_double(num));
}
/* }}} */

/* {{{ Get the boolean value of a variable */
CRX_FUNCTION(boolval)
{
	zval *value;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(value)
	CREX_PARSE_PARAMETERS_END();

	RETURN_BOOL(crex_is_true(value));
}
/* }}} */

/* {{{ Get the string value of a variable */
CRX_FUNCTION(strval)
{
	zval *value;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(value)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_STR(zval_get_string(value));
}
/* }}} */

static inline void crx_is_type(INTERNAL_FUNCTION_PARAMETERS, int type)
{
	zval *arg;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(arg)
	CREX_PARSE_PARAMETERS_END();

	if (C_TYPE_P(arg) == type) {
		if (type == IS_RESOURCE) {
			const char *type_name = crex_rsrc_list_get_rsrc_type(C_RES_P(arg));
			if (!type_name) {
				RETURN_FALSE;
			}
		}
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}


/* {{{ Returns true if variable is null
   Warning: This function is special-cased by crex_compile.c and so is usually bypassed */
CRX_FUNCTION(is_null)
{
	crx_is_type(INTERNAL_FUNCTION_PARAM_PASSTHRU, IS_NULL);
}
/* }}} */

/* {{{ Returns true if variable is a resource
   Warning: This function is special-cased by crex_compile.c and so is usually bypassed */
CRX_FUNCTION(is_resource)
{
	crx_is_type(INTERNAL_FUNCTION_PARAM_PASSTHRU, IS_RESOURCE);
}
/* }}} */

/* {{{ Returns true if variable is a boolean
   Warning: This function is special-cased by crex_compile.c and so is usually bypassed */
CRX_FUNCTION(is_bool)
{
	zval *arg;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(arg)
	CREX_PARSE_PARAMETERS_END();

	RETURN_BOOL(C_TYPE_P(arg) == IS_FALSE || C_TYPE_P(arg) == IS_TRUE);
}
/* }}} */

/* {{{ Returns true if variable is an integer
   Warning: This function is special-cased by crex_compile.c and so is usually bypassed */
CRX_FUNCTION(is_int)
{
	crx_is_type(INTERNAL_FUNCTION_PARAM_PASSTHRU, IS_LONG);
}
/* }}} */

/* {{{ Returns true if variable is float point
   Warning: This function is special-cased by crex_compile.c and so is usually bypassed */
CRX_FUNCTION(is_float)
{
	crx_is_type(INTERNAL_FUNCTION_PARAM_PASSTHRU, IS_DOUBLE);
}
/* }}} */

/* {{{ Returns true if variable is a string
   Warning: This function is special-cased by crex_compile.c and so is usually bypassed */
CRX_FUNCTION(is_string)
{
	crx_is_type(INTERNAL_FUNCTION_PARAM_PASSTHRU, IS_STRING);
}
/* }}} */

/* {{{ Returns true if variable is an array
   Warning: This function is special-cased by crex_compile.c and so is usually bypassed */
CRX_FUNCTION(is_array)
{
	crx_is_type(INTERNAL_FUNCTION_PARAM_PASSTHRU, IS_ARRAY);
}
/* }}} */

/* {{{ Returns true if $array is an array whose keys are all numeric, sequential, and start at 0 */
CRX_FUNCTION(array_is_list)
{
	HashTable *array;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ARRAY_HT(array)
	CREX_PARSE_PARAMETERS_END();

	RETURN_BOOL(crex_array_is_list(array));
}
/* }}} */

/* {{{ Returns true if variable is an object
   Warning: This function is special-cased by crex_compile.c and so is usually bypassed */
CRX_FUNCTION(is_object)
{
	crx_is_type(INTERNAL_FUNCTION_PARAM_PASSTHRU, IS_OBJECT);
}
/* }}} */

/* {{{ Returns true if value is a number or a numeric string */
CRX_FUNCTION(is_numeric)
{
	zval *arg;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(arg)
	CREX_PARSE_PARAMETERS_END();

	switch (C_TYPE_P(arg)) {
		case IS_LONG:
		case IS_DOUBLE:
			RETURN_TRUE;
			break;

		case IS_STRING:
			if (is_numeric_string(C_STRVAL_P(arg), C_STRLEN_P(arg), NULL, NULL, 0)) {
				RETURN_TRUE;
			} else {
				RETURN_FALSE;
			}
			break;

		default:
			RETURN_FALSE;
			break;
	}
}
/* }}} */

/* {{{ Returns true if value is a scalar */
CRX_FUNCTION(is_scalar)
{
	zval *arg;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(arg)
	CREX_PARSE_PARAMETERS_END();

	switch (C_TYPE_P(arg)) {
		case IS_FALSE:
		case IS_TRUE:
		case IS_DOUBLE:
		case IS_LONG:
		case IS_STRING:
			RETURN_TRUE;
			break;

		default:
			RETURN_FALSE;
			break;
	}
}
/* }}} */

/* {{{ Returns true if var is callable. */
CRX_FUNCTION(is_callable)
{
	zval *var, *callable_name = NULL;
	crex_string *name;
	bool retval;
	bool syntax_only = 0;
	int check_flags = 0;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_ZVAL(var)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(syntax_only)
		C_PARAM_ZVAL(callable_name)
	CREX_PARSE_PARAMETERS_END();

	if (syntax_only) {
		check_flags |= IS_CALLABLE_CHECK_SYNTAX_ONLY;
	}
	if (CREX_NUM_ARGS() > 2) {
		retval = crex_is_callable_ex(var, NULL, check_flags, &name, NULL, NULL);
		CREX_TRY_ASSIGN_REF_STR(callable_name, name);
	} else {
		retval = crex_is_callable_ex(var, NULL, check_flags, NULL, NULL, NULL);
	}

	RETURN_BOOL(retval);
}
/* }}} */

/* {{{ Returns true if var is iterable (array or instance of Traversable). */
CRX_FUNCTION(is_iterable)
{
	zval *var;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(var)
	CREX_PARSE_PARAMETERS_END();

	RETURN_BOOL(crex_is_iterable(var));
}
/* }}} */

/* {{{ Returns true if var is countable (array or instance of Countable). */
CRX_FUNCTION(is_countable)
{
	zval *var;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(var)
	CREX_PARSE_PARAMETERS_END();

	RETURN_BOOL(crex_is_countable(var));
}
/* }}} */
