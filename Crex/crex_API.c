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
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Andrei Zmievski <andrei@crx.net>                            |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_execute.h"
#include "crex_API.h"
#include "crex_modules.h"
#include "crex_extensions.h"
#include "crex_constants.h"
#include "crex_interfaces.h"
#include "crex_exceptions.h"
#include "crex_closures.h"
#include "crex_inheritance.h"
#include "crex_ini.h"
#include "crex_enum.h"
#include "crex_observer.h"

#include <stdarg.h>

/* these variables are true statics/globals, and have to be mutex'ed on every access */
CREX_API HashTable module_registry;

static crex_module_entry **module_request_startup_handlers;
static crex_module_entry **module_request_shutdown_handlers;
static crex_module_entry **module_post_deactivate_handlers;

static crex_class_entry  **class_cleanup_handlers;

CREX_API crex_result crex_get_parameters_array_ex(uint32_t param_count, zval *argument_array) /* {{{ */
{
	zval *param_ptr;
	uint32_t arg_count;

	param_ptr = CREX_CALL_ARG(EG(current_execute_data), 1);
	arg_count = CREX_CALL_NUM_ARGS(EG(current_execute_data));

	if (param_count>arg_count) {
		return FAILURE;
	}

	while (param_count-->0) {
		ZVAL_COPY_VALUE(argument_array, param_ptr);
		argument_array++;
		param_ptr++;
	}

	return SUCCESS;
}
/* }}} */

CREX_API crex_result crex_copy_parameters_array(uint32_t param_count, zval *argument_array) /* {{{ */
{
	zval *param_ptr;
	uint32_t arg_count;

	param_ptr = CREX_CALL_ARG(EG(current_execute_data), 1);
	arg_count = CREX_CALL_NUM_ARGS(EG(current_execute_data));

	if (param_count>arg_count) {
		return FAILURE;
	}

	while (param_count-->0) {
		C_TRY_ADDREF_P(param_ptr);
		crex_hash_next_index_insert_new(C_ARRVAL_P(argument_array), param_ptr);
		param_ptr++;
	}

	return SUCCESS;
}
/* }}} */

CREX_API CREX_COLD void crex_wrong_param_count(void) /* {{{ */
{
	const char *space;
	const char *class_name = get_active_class_name(&space);

	crex_argument_count_error("Wrong parameter count for %s%s%s()", class_name, space, get_active_function_name());
}
/* }}} */

CREX_API CREX_COLD void crex_wrong_property_read(zval *object, zval *property)
{
	crex_string *tmp_property_name;
	crex_string *property_name = zval_get_tmp_string(property, &tmp_property_name);
	crex_error(E_WARNING, "Attempt to read property \"%s\" on %s", ZSTR_VAL(property_name), crex_zval_value_name(object));
	crex_tmp_string_release(tmp_property_name);
}

/* Argument parsing API -- andrei */
CREX_API const char *crex_get_type_by_const(int type) /* {{{ */
{
	switch(type) {
		case IS_FALSE:
		case IS_TRUE:
		case _IS_BOOL:
			return "bool";
		case IS_LONG:
			return "int";
		case IS_DOUBLE:
			return "float";
		case IS_STRING:
			return "string";
		case IS_OBJECT:
			return "object";
		case IS_RESOURCE:
			return "resource";
		case IS_NULL:
			return "null";
		case IS_CALLABLE:
			return "callable";
		case IS_ITERABLE:
			return "iterable";
		case IS_ARRAY:
			return "array";
		case IS_VOID:
			return "void";
		case IS_MIXED:
			return "mixed";
		case _IS_NUMBER:
			return "int|float";
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}
/* }}} */

CREX_API const char *crex_zval_value_name(const zval *arg)
{
	ZVAL_DEREF(arg);

	if (C_ISUNDEF_P(arg)) {
		return "null";
	}

	if (C_TYPE_P(arg) == IS_OBJECT) {
		return ZSTR_VAL(C_OBJCE_P(arg)->name);
	} else if (C_TYPE_P(arg) == IS_FALSE) {
		return "false";
	} else if  (C_TYPE_P(arg) == IS_TRUE) {
		return "true";
	}

	return crex_get_type_by_const(C_TYPE_P(arg));
}

CREX_API const char *crex_zval_type_name(const zval *arg)
{
	ZVAL_DEREF(arg);

	if (C_ISUNDEF_P(arg)) {
		return "null";
	}

	if (C_TYPE_P(arg) == IS_OBJECT) {
		return ZSTR_VAL(C_OBJCE_P(arg)->name);
	}

	return crex_get_type_by_const(C_TYPE_P(arg));
}

/* This API exists *only* for use in gettype().
 * For anything else, you likely want crex_zval_type_name(). */
CREX_API crex_string *crex_zval_get_legacy_type(const zval *arg) /* {{{ */
{
	switch (C_TYPE_P(arg)) {
		case IS_NULL:
			return ZSTR_KNOWN(CREX_STR_NULL);
		case IS_FALSE:
		case IS_TRUE:
			return ZSTR_KNOWN(CREX_STR_BOOLEAN);
		case IS_LONG:
			return ZSTR_KNOWN(CREX_STR_INTEGER);
		case IS_DOUBLE:
			return ZSTR_KNOWN(CREX_STR_DOUBLE);
		case IS_STRING:
			return ZSTR_KNOWN(CREX_STR_STRING);
		case IS_ARRAY:
			return ZSTR_KNOWN(CREX_STR_ARRAY);
		case IS_OBJECT:
			return ZSTR_KNOWN(CREX_STR_OBJECT);
		case IS_RESOURCE:
			if (crex_rsrc_list_get_rsrc_type(C_RES_P(arg))) {
				return ZSTR_KNOWN(CREX_STR_RESOURCE);
			} else {
				return ZSTR_KNOWN(CREX_STR_CLOSED_RESOURCE);
			}
		default:
			return NULL;
	}
}
/* }}} */

CREX_API CREX_COLD void CREX_FASTCALL crex_wrong_parameters_none_error(void) /* {{{ */
{
	int num_args = CREX_CALL_NUM_ARGS(EG(current_execute_data));
	crex_string *func_name = get_active_function_or_method_name();

	crex_argument_count_error("%s() expects exactly 0 arguments, %d given", ZSTR_VAL(func_name), num_args);

	crex_string_release(func_name);
}
/* }}} */

CREX_API CREX_COLD void CREX_FASTCALL crex_wrong_parameters_count_error(uint32_t min_num_args, uint32_t max_num_args) /* {{{ */
{
	uint32_t num_args = CREX_CALL_NUM_ARGS(EG(current_execute_data));
	crex_string *func_name = get_active_function_or_method_name();

	crex_argument_count_error(
		"%s() expects %s %d argument%s, %d given",
		ZSTR_VAL(func_name),
		min_num_args == max_num_args ? "exactly" : num_args < min_num_args ? "at least" : "at most",
		num_args < min_num_args ? min_num_args : max_num_args,
		(num_args < min_num_args ? min_num_args : max_num_args) == 1 ? "" : "s",
		num_args
	);

	crex_string_release(func_name);
}
/* }}} */

CREX_API CREX_COLD void CREX_FASTCALL crex_wrong_parameter_error(int error_code, uint32_t num, char *name, crex_expected_type expected_type, zval *arg) /* {{{ */
{
	switch (error_code) {
		case ZPP_ERROR_WRONG_CALLBACK:
			crex_wrong_callback_error(num, name);
			break;
		case ZPP_ERROR_WRONG_CALLBACK_OR_NULL:
			crex_wrong_callback_or_null_error(num, name);
			break;
		case ZPP_ERROR_WRONG_CLASS:
			crex_wrong_parameter_class_error(num, name, arg);
			break;
		case ZPP_ERROR_WRONG_CLASS_OR_NULL:
			crex_wrong_parameter_class_or_null_error(num, name, arg);
			break;
		case ZPP_ERROR_WRONG_CLASS_OR_STRING:
			crex_wrong_parameter_class_or_string_error(num, name, arg);
			break;
		case ZPP_ERROR_WRONG_CLASS_OR_STRING_OR_NULL:
			crex_wrong_parameter_class_or_string_or_null_error(num, name, arg);
			break;
		case ZPP_ERROR_WRONG_CLASS_OR_LONG:
			crex_wrong_parameter_class_or_long_error(num, name, arg);
			break;
		case ZPP_ERROR_WRONG_CLASS_OR_LONG_OR_NULL:
			crex_wrong_parameter_class_or_long_or_null_error(num, name, arg);
			break;
		case ZPP_ERROR_WRONG_ARG:
			crex_wrong_parameter_type_error(num, expected_type, arg);
			break;
		case ZPP_ERROR_UNEXPECTED_EXTRA_NAMED:
			crex_unexpected_extra_named_error();
			break;
		case ZPP_ERROR_FAILURE:
			CREX_ASSERT(EG(exception) && "Should have produced an error already");
			break;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}
/* }}} */

CREX_API CREX_COLD void CREX_FASTCALL crex_wrong_parameter_type_error(uint32_t num, crex_expected_type expected_type, zval *arg) /* {{{ */
{
	static const char * const expected_error[] = {
		C_EXPECTED_TYPES(C_EXPECTED_TYPE_STR)
		NULL
	};

	if (EG(exception)) {
		return;
	}

	if ((expected_type == C_EXPECTED_PATH || expected_type == C_EXPECTED_PATH_OR_NULL)
			&& C_TYPE_P(arg) == IS_STRING) {
		crex_argument_value_error(num, "must not contain any null bytes");
		return;
	}

	crex_argument_type_error(num, "must be %s, %s given", expected_error[expected_type], crex_zval_value_name(arg));
}
/* }}} */

CREX_API CREX_COLD void CREX_FASTCALL crex_wrong_parameter_class_error(uint32_t num, const char *name, zval *arg) /* {{{ */
{
	if (EG(exception)) {
		return;
	}

	crex_argument_type_error(num, "must be of type %s, %s given", name, crex_zval_value_name(arg));
}
/* }}} */

CREX_API CREX_COLD void CREX_FASTCALL crex_wrong_parameter_class_or_null_error(uint32_t num, const char *name, zval *arg) /* {{{ */
{
	if (EG(exception)) {
		return;
	}

	crex_argument_type_error(num, "must be of type ?%s, %s given", name, crex_zval_value_name(arg));
}
/* }}} */

CREX_API CREX_COLD void CREX_FASTCALL crex_wrong_parameter_class_or_long_error(uint32_t num, const char *name, zval *arg) /* {{{ */
{
	if (EG(exception)) {
		return;
	}

	crex_argument_type_error(num, "must be of type %s|int, %s given", name, crex_zval_value_name(arg));
}
/* }}} */

CREX_API CREX_COLD void CREX_FASTCALL crex_wrong_parameter_class_or_long_or_null_error(uint32_t num, const char *name, zval *arg) /* {{{ */
{
	if (EG(exception)) {
		return;
	}

	crex_argument_type_error(num, "must be of type %s|int|null, %s given", name, crex_zval_value_name(arg));
}
/* }}} */

CREX_API CREX_COLD void CREX_FASTCALL crex_wrong_parameter_class_or_string_error(uint32_t num, const char *name, zval *arg) /* {{{ */
{
	if (EG(exception)) {
		return;
	}

	crex_argument_type_error(num, "must be of type %s|string, %s given", name, crex_zval_value_name(arg));
}
/* }}} */

CREX_API CREX_COLD void CREX_FASTCALL crex_wrong_parameter_class_or_string_or_null_error(uint32_t num, const char *name, zval *arg) /* {{{ */
{
	if (EG(exception)) {
		return;
	}

	crex_argument_type_error(num, "must be of type %s|string|null, %s given", name, crex_zval_value_name(arg));
}
/* }}} */

CREX_API CREX_COLD void CREX_FASTCALL crex_wrong_callback_error(uint32_t num, char *error) /* {{{ */
{
	if (!EG(exception)) {
		crex_argument_type_error(num, "must be a valid callback, %s", error);
	}
	efree(error);
}
/* }}} */

CREX_API CREX_COLD void CREX_FASTCALL crex_wrong_callback_or_null_error(uint32_t num, char *error) /* {{{ */
{
	if (!EG(exception)) {
		crex_argument_type_error(num, "must be a valid callback or null, %s", error);
	}
	efree(error);
}
/* }}} */

CREX_API CREX_COLD void CREX_FASTCALL crex_unexpected_extra_named_error(void)
{
	const char *space;
	const char *class_name = get_active_class_name(&space);
	crex_argument_count_error("%s%s%s() does not accept unknown named parameters",
		class_name, space, get_active_function_name());
}

CREX_API CREX_COLD void CREX_FASTCALL crex_argument_error_variadic(crex_class_entry *error_ce, uint32_t arg_num, const char *format, va_list va) /* {{{ */
{
	crex_string *func_name;
	const char *arg_name;
	char *message = NULL;
	if (EG(exception)) {
		return;
	}

	func_name = get_active_function_or_method_name();
	arg_name = get_active_function_arg_name(arg_num);

	crex_vspprintf(&message, 0, format, va);
	crex_throw_error(error_ce, "%s(): Argument #%d%s%s%s %s",
		ZSTR_VAL(func_name), arg_num,
		arg_name ? " ($" : "", arg_name ? arg_name : "", arg_name ? ")" : "", message
	);
	efree(message);
	crex_string_release(func_name);
}
/* }}} */

CREX_API CREX_COLD void crex_argument_error(crex_class_entry *error_ce, uint32_t arg_num, const char *format, ...) /* {{{ */
{
	va_list va;

	va_start(va, format);
	crex_argument_error_variadic(error_ce, arg_num, format, va);
	va_end(va);
}
/* }}} */

CREX_API CREX_COLD void crex_argument_type_error(uint32_t arg_num, const char *format, ...) /* {{{ */
{
	va_list va;

	va_start(va, format);
	crex_argument_error_variadic(crex_ce_type_error, arg_num, format, va);
	va_end(va);
}
/* }}} */

CREX_API CREX_COLD void crex_argument_value_error(uint32_t arg_num, const char *format, ...) /* {{{ */
{
	va_list va;

	va_start(va, format);
	crex_argument_error_variadic(crex_ce_value_error, arg_num, format, va);
	va_end(va);
}
/* }}} */

CREX_API bool CREX_FASTCALL crex_parse_arg_class(zval *arg, crex_class_entry **pce, uint32_t num, bool check_null) /* {{{ */
{
	crex_class_entry *ce_base = *pce;

	if (check_null && C_TYPE_P(arg) == IS_NULL) {
		*pce = NULL;
		return 1;
	}
	if (!try_convert_to_string(arg)) {
		*pce = NULL;
		return 0;
	}

	*pce = crex_lookup_class(C_STR_P(arg));
	if (ce_base) {
		if ((!*pce || !instanceof_function(*pce, ce_base))) {
			crex_argument_type_error(num, "must be a class name derived from %s, %s given", ZSTR_VAL(ce_base->name), C_STRVAL_P(arg));
			*pce = NULL;
			return 0;
		}
	}
	if (!*pce) {
		crex_argument_type_error(num, "must be a valid class name, %s given", C_STRVAL_P(arg));
		return 0;
	}
	return 1;
}
/* }}} */

static CREX_COLD bool crex_null_arg_deprecated(const char *fallback_type, uint32_t arg_num) {
	crex_function *func = EG(current_execute_data)->func;
	CREX_ASSERT(arg_num > 0);
	uint32_t arg_offset = arg_num - 1;
	if (arg_offset >= func->common.num_args) {
		CREX_ASSERT(func->common.fn_flags & CREX_ACC_VARIADIC);
		arg_offset = func->common.num_args;
	}

	crex_arg_info *arg_info = &func->common.arg_info[arg_offset];
	crex_string *func_name = get_active_function_or_method_name();
	const char *arg_name = get_active_function_arg_name(arg_num);

	/* If no type is specified in arginfo, use the specified fallback_type determined through
	 * crex_parse_parameters instead. */
	crex_string *type_str = crex_type_to_string(arg_info->type);
	const char *type = type_str ? ZSTR_VAL(type_str) : fallback_type;
	crex_error(E_DEPRECATED,
		"%s(): Passing null to parameter #%" PRIu32 "%s%s%s of type %s is deprecated",
		ZSTR_VAL(func_name), arg_num,
		arg_name ? " ($" : "", arg_name ? arg_name : "", arg_name ? ")" : "",
		type);
	crex_string_release(func_name);
	if (type_str) {
		crex_string_release(type_str);
	}
	return !EG(exception);
}

CREX_API bool CREX_FASTCALL crex_parse_arg_bool_weak(const zval *arg, bool *dest, uint32_t arg_num) /* {{{ */
{
	if (EXPECTED(C_TYPE_P(arg) <= IS_STRING)) {
		if (UNEXPECTED(C_TYPE_P(arg) == IS_NULL) && !crex_null_arg_deprecated("bool", arg_num)) {
			return 0;
		}
		*dest = crex_is_true(arg);
	} else {
		return 0;
	}
	return 1;
}
/* }}} */

CREX_API bool CREX_FASTCALL crex_parse_arg_bool_slow(const zval *arg, bool *dest, uint32_t arg_num) /* {{{ */
{
	if (UNEXPECTED(CREX_ARG_USES_STRICT_TYPES())) {
		return 0;
	}
	return crex_parse_arg_bool_weak(arg, dest, arg_num);
}
/* }}} */

CREX_API bool CREX_FASTCALL crex_parse_arg_long_weak(const zval *arg, crex_long *dest, uint32_t arg_num) /* {{{ */
{
	if (EXPECTED(C_TYPE_P(arg) == IS_DOUBLE)) {
		if (UNEXPECTED(crex_isnan(C_DVAL_P(arg)))) {
			return 0;
		}
		if (UNEXPECTED(!CREX_DOUBLE_FITS_LONG(C_DVAL_P(arg)))) {
			return 0;
		} else {
			crex_long lval = crex_dval_to_lval(C_DVAL_P(arg));
			if (UNEXPECTED(!crex_is_long_compatible(C_DVAL_P(arg), lval))) {
				/* Check arg_num is not (uint32_t)-1, as otherwise its called by
				 * crex_verify_weak_scalar_type_hint_no_sideeffect() */
				if (arg_num != (uint32_t)-1) {
					crex_incompatible_double_to_long_error(C_DVAL_P(arg));
				}
				if (UNEXPECTED(EG(exception))) {
					return 0;
				}
			}
			*dest = lval;
		}
	} else if (EXPECTED(C_TYPE_P(arg) == IS_STRING)) {
		double d;
		uint8_t type;

		if (UNEXPECTED((type = is_numeric_str_function(C_STR_P(arg), dest, &d)) != IS_LONG)) {
			if (EXPECTED(type != 0)) {
				crex_long lval;
				if (UNEXPECTED(crex_isnan(d))) {
					return 0;
				}
				if (UNEXPECTED(!CREX_DOUBLE_FITS_LONG(d))) {
					return 0;
				}

				lval = crex_dval_to_lval(d);
				/* This only checks for a fractional part as if doesn't fit it already throws a TypeError */
				if (UNEXPECTED(!crex_is_long_compatible(d, lval))) {
					/* Check arg_num is not (uint32_t)-1, as otherwise its called by
					 * crex_verify_weak_scalar_type_hint_no_sideeffect() */
					if (arg_num != (uint32_t)-1) {
						crex_incompatible_string_to_long_error(C_STR_P(arg));
					}
					if (UNEXPECTED(EG(exception))) {
						return 0;
					}
				}
				*dest = lval;
			} else {
				return 0;
			}
		}
		if (UNEXPECTED(EG(exception))) {
			return 0;
		}
	} else if (EXPECTED(C_TYPE_P(arg) < IS_TRUE)) {
		if (UNEXPECTED(C_TYPE_P(arg) == IS_NULL) && !crex_null_arg_deprecated("int", arg_num)) {
			return 0;
		}
		*dest = 0;
	} else if (EXPECTED(C_TYPE_P(arg) == IS_TRUE)) {
		*dest = 1;
	} else {
		return 0;
	}
	return 1;
}
/* }}} */

CREX_API bool CREX_FASTCALL crex_parse_arg_long_slow(const zval *arg, crex_long *dest, uint32_t arg_num) /* {{{ */
{
	if (UNEXPECTED(CREX_ARG_USES_STRICT_TYPES())) {
		return 0;
	}
	return crex_parse_arg_long_weak(arg, dest, arg_num);
}
/* }}} */

CREX_API bool CREX_FASTCALL crex_parse_arg_double_weak(const zval *arg, double *dest, uint32_t arg_num) /* {{{ */
{
	if (EXPECTED(C_TYPE_P(arg) == IS_LONG)) {
		*dest = (double)C_LVAL_P(arg);
	} else if (EXPECTED(C_TYPE_P(arg) == IS_STRING)) {
		crex_long l;
		uint8_t type;

		if (UNEXPECTED((type = is_numeric_str_function(C_STR_P(arg), &l, dest)) != IS_DOUBLE)) {
			if (EXPECTED(type != 0)) {
				*dest = (double)(l);
			} else {
				return 0;
			}
		}
		if (UNEXPECTED(EG(exception))) {
			return 0;
		}
	} else if (EXPECTED(C_TYPE_P(arg) < IS_TRUE)) {
		if (UNEXPECTED(C_TYPE_P(arg) == IS_NULL) && !crex_null_arg_deprecated("float", arg_num)) {
			return 0;
		}
		*dest = 0.0;
	} else if (EXPECTED(C_TYPE_P(arg) == IS_TRUE)) {
		*dest = 1.0;
	} else {
		return 0;
	}
	return 1;
}
/* }}} */

CREX_API bool CREX_FASTCALL crex_parse_arg_double_slow(const zval *arg, double *dest, uint32_t arg_num) /* {{{ */
{
	if (EXPECTED(C_TYPE_P(arg) == IS_LONG)) {
		/* SSTH Exception: IS_LONG may be accepted instead as IS_DOUBLE */
		*dest = (double)C_LVAL_P(arg);
	} else if (UNEXPECTED(CREX_ARG_USES_STRICT_TYPES())) {
		return 0;
	}
	return crex_parse_arg_double_weak(arg, dest, arg_num);
}
/* }}} */

CREX_API bool CREX_FASTCALL crex_parse_arg_number_slow(zval *arg, zval **dest, uint32_t arg_num) /* {{{ */
{
	if (UNEXPECTED(CREX_ARG_USES_STRICT_TYPES())) {
		return 0;
	}
	if (C_TYPE_P(arg) == IS_STRING) {
		crex_string *str = C_STR_P(arg);
		crex_long lval;
		double dval;
		uint8_t type = is_numeric_str_function(str, &lval, &dval);
		if (type == IS_LONG) {
			ZVAL_LONG(arg, lval);
		} else if (type == IS_DOUBLE) {
			ZVAL_DOUBLE(arg, dval);
		} else {
			return 0;
		}
		crex_string_release(str);
	} else if (C_TYPE_P(arg) < IS_TRUE) {
		if (UNEXPECTED(C_TYPE_P(arg) == IS_NULL) && !crex_null_arg_deprecated("int|float", arg_num)) {
			return 0;
		}
		ZVAL_LONG(arg, 0);
	} else if (C_TYPE_P(arg) == IS_TRUE) {
		ZVAL_LONG(arg, 1);
	} else {
		return 0;
	}
	*dest = arg;
	return 1;
}
/* }}} */


CREX_API bool CREX_FASTCALL crex_parse_arg_number_or_str_slow(zval *arg, zval **dest, uint32_t arg_num) /* {{{ */
{
	if (UNEXPECTED(CREX_ARG_USES_STRICT_TYPES())) {
		return false;
	}
	if (C_TYPE_P(arg) < IS_TRUE) {
		if (UNEXPECTED(C_TYPE_P(arg) == IS_NULL) && !crex_null_arg_deprecated("string|int|float", arg_num)) {
			return false;
		}
		ZVAL_LONG(arg, 0);
	} else if (C_TYPE_P(arg) == IS_TRUE) {
		ZVAL_LONG(arg, 1);
	} else if (UNEXPECTED(C_TYPE_P(arg) == IS_OBJECT)) {
		crex_object *zobj = C_OBJ_P(arg);
		zval obj;
		if (zobj->handlers->cast_object(zobj, &obj, IS_STRING) == SUCCESS) {
			OBJ_RELEASE(zobj);
			ZVAL_COPY_VALUE(arg, &obj);
			*dest = arg;
			return true;
		}
		return false;
	} else {
		return false;
	}
	*dest = arg;
	return true;
}

CREX_API bool CREX_FASTCALL crex_parse_arg_str_weak(zval *arg, crex_string **dest, uint32_t arg_num) /* {{{ */
{
	if (EXPECTED(C_TYPE_P(arg) < IS_STRING)) {
		if (UNEXPECTED(C_TYPE_P(arg) == IS_NULL) && !crex_null_arg_deprecated("string", arg_num)) {
			return 0;
		}
		convert_to_string(arg);
		*dest = C_STR_P(arg);
	} else if (UNEXPECTED(C_TYPE_P(arg) == IS_OBJECT)) {
		crex_object *zobj = C_OBJ_P(arg);
		zval obj;
		if (zobj->handlers->cast_object(zobj, &obj, IS_STRING) == SUCCESS) {
			OBJ_RELEASE(zobj);
			ZVAL_COPY_VALUE(arg, &obj);
			*dest = C_STR_P(arg);
			return 1;
		}
		return 0;
	} else {
		return 0;
	}
	return 1;
}
/* }}} */

CREX_API bool CREX_FASTCALL crex_parse_arg_str_slow(zval *arg, crex_string **dest, uint32_t arg_num) /* {{{ */
{
	if (UNEXPECTED(CREX_ARG_USES_STRICT_TYPES())) {
		return 0;
	}
	return crex_parse_arg_str_weak(arg, dest, arg_num);
}
/* }}} */

CREX_API bool CREX_FASTCALL crex_parse_arg_str_or_long_slow(zval *arg, crex_string **dest_str, crex_long *dest_long, uint32_t arg_num) /* {{{ */
{
	if (UNEXPECTED(CREX_ARG_USES_STRICT_TYPES())) {
		return 0;
	}
	if (crex_parse_arg_long_weak(arg, dest_long, arg_num)) {
		*dest_str = NULL;
		return 1;
	} else if (crex_parse_arg_str_weak(arg, dest_str, arg_num)) {
		*dest_long = 0;
		return 1;
	} else {
		return 0;
	}
}
/* }}} */

static const char *crex_parse_arg_impl(zval *arg, va_list *va, const char **spec, char **error, uint32_t arg_num) /* {{{ */
{
	const char *spec_walk = *spec;
	char c = *spec_walk++;
	bool check_null = 0;
	bool separate = 0;
	zval *real_arg = arg;

	/* scan through modifiers */
	ZVAL_DEREF(arg);
	while (1) {
		if (*spec_walk == '/') {
			SEPARATE_ZVAL_NOREF(arg);
			real_arg = arg;
			separate = 1;
		} else if (*spec_walk == '!') {
			check_null = 1;
		} else {
			break;
		}
		spec_walk++;
	}

	switch (c) {
		case 'l':
			{
				crex_long *p = va_arg(*va, crex_long *);
				bool *is_null = NULL;

				if (check_null) {
					is_null = va_arg(*va, bool *);
				}

				if (!crex_parse_arg_long(arg, p, is_null, check_null, arg_num)) {
					return check_null ? "?int" : "int";
				}
			}
			break;

		case 'd':
			{
				double *p = va_arg(*va, double *);
				bool *is_null = NULL;

				if (check_null) {
					is_null = va_arg(*va, bool *);
				}

				if (!crex_parse_arg_double(arg, p, is_null, check_null, arg_num)) {
					return check_null ? "?float" : "float";
				}
			}
			break;

		case 'n':
			{
				zval **p = va_arg(*va, zval **);

				if (!crex_parse_arg_number(arg, p, check_null, arg_num)) {
					return check_null ? "int|float|null" : "int|float";
				}
			}
			break;

		case 's':
			{
				char **p = va_arg(*va, char **);
				size_t *pl = va_arg(*va, size_t *);
				if (!crex_parse_arg_string(arg, p, pl, check_null, arg_num)) {
					return check_null ? "?string" : "string";
				}
			}
			break;

		case 'p':
			{
				char **p = va_arg(*va, char **);
				size_t *pl = va_arg(*va, size_t *);
				if (!crex_parse_arg_path(arg, p, pl, check_null, arg_num)) {
					if (C_TYPE_P(arg) == IS_STRING) {
						crex_spprintf(error, 0, "must not contain any null bytes");
						return "";
					} else {
						return check_null ? "?string" : "string";
					}
				}
			}
			break;

		case 'P':
			{
				crex_string **str = va_arg(*va, crex_string **);
				if (!crex_parse_arg_path_str(arg, str, check_null, arg_num)) {
					if (C_TYPE_P(arg) == IS_STRING) {
						crex_spprintf(error, 0, "must not contain any null bytes");
						return "";
					} else {
						return check_null ? "?string" : "string";
					}
				}
			}
			break;

		case 'S':
			{
				crex_string **str = va_arg(*va, crex_string **);
				if (!crex_parse_arg_str(arg, str, check_null, arg_num)) {
					return check_null ? "?string" : "string";
				}
			}
			break;

		case 'b':
			{
				bool *p = va_arg(*va, bool *);
				bool *is_null = NULL;

				if (check_null) {
					is_null = va_arg(*va, bool *);
				}

				if (!crex_parse_arg_bool(arg, p, is_null, check_null, arg_num)) {
					return check_null ? "?bool" : "bool";
				}
			}
			break;

		case 'r':
			{
				zval **p = va_arg(*va, zval **);

				if (!crex_parse_arg_resource(arg, p, check_null)) {
					return check_null ? "resource or null" : "resource";
				}
			}
			break;

		case 'A':
		case 'a':
			{
				zval **p = va_arg(*va, zval **);

				if (!crex_parse_arg_array(arg, p, check_null, c == 'A')) {
					return check_null ? "?array" : "array";
				}
			}
			break;

		case 'H':
		case 'h':
			{
				HashTable **p = va_arg(*va, HashTable **);

				if (!crex_parse_arg_array_ht(arg, p, check_null, c == 'H', separate)) {
					return check_null ? "?array" : "array";
				}
			}
			break;

		case 'o':
			{
				zval **p = va_arg(*va, zval **);

				if (!crex_parse_arg_object(arg, p, NULL, check_null)) {
					return check_null ? "?object" : "object";
				}
			}
			break;

		case 'O':
			{
				zval **p = va_arg(*va, zval **);
				crex_class_entry *ce = va_arg(*va, crex_class_entry *);

				if (!crex_parse_arg_object(arg, p, ce, check_null)) {
					if (ce) {
						if (check_null) {
							crex_spprintf(error, 0, "must be of type ?%s, %s given", ZSTR_VAL(ce->name), crex_zval_value_name(arg));
							return "";
						} else {
							return ZSTR_VAL(ce->name);
						}
					} else {
						return check_null ? "?object" : "object";
					}
				}
			}
			break;

		case 'C':
			{
				crex_class_entry *lookup, **pce = va_arg(*va, crex_class_entry **);
				crex_class_entry *ce_base = *pce;

				if (check_null && C_TYPE_P(arg) == IS_NULL) {
					*pce = NULL;
					break;
				}
				if (!try_convert_to_string(arg)) {
					*pce = NULL;
					return ""; /* try_convert_to_string() throws an exception */
				}

				if ((lookup = crex_lookup_class(C_STR_P(arg))) == NULL) {
					*pce = NULL;
				} else {
					*pce = lookup;
				}
				if (ce_base) {
					if ((!*pce || !instanceof_function(*pce, ce_base))) {
						crex_spprintf(error, 0, "must be a class name derived from %s%s, %s given",
							ZSTR_VAL(ce_base->name), check_null ? " or null" : "", C_STRVAL_P(arg));
						*pce = NULL;
						return "";
					}
				}
				if (!*pce) {
					crex_spprintf(error, 0, "must be a valid class name%s, %s given",
						check_null ? " or null" : "", C_STRVAL_P(arg));
					return "";
				}
				break;

			}
			break;

		case 'f':
			{
				crex_fcall_info *fci = va_arg(*va, crex_fcall_info *);
				crex_fcall_info_cache *fcc = va_arg(*va, crex_fcall_info_cache *);
				char *is_callable_error = NULL;

				if (check_null && C_TYPE_P(arg) == IS_NULL) {
					fci->size = 0;
					fcc->function_handler = 0;
					break;
				}

				if (crex_fcall_info_init(arg, 0, fci, fcc, NULL, &is_callable_error) == SUCCESS) {
					CREX_ASSERT(!is_callable_error);
					/* Release call trampolines: The function may not get called, in which case
					 * the trampoline will leak. Force it to be refetched during
					 * crex_call_function instead. */
					crex_release_fcall_info_cache(fcc);
					break;
				}

				if (is_callable_error) {
					crex_spprintf(error, 0, "must be a valid callback%s, %s", check_null ? " or null" : "", is_callable_error);
					efree(is_callable_error);
					return "";
				} else {
					return check_null ? "a valid callback or null" : "a valid callback";
				}
			}

		case 'z':
			{
				zval **p = va_arg(*va, zval **);

				crex_parse_arg_zval_deref(real_arg, p, check_null);
			}
			break;

		case 'Z': /* replace with 'z' */
		case 'L': /* replace with 'l' */
			CREX_ASSERT(0 && "ZPP modifier no longer supported");
			CREX_FALLTHROUGH;
		default:
			return "unknown";
	}

	*spec = spec_walk;

	return NULL;
}
/* }}} */

static crex_result crex_parse_arg(uint32_t arg_num, zval *arg, va_list *va, const char **spec, int flags) /* {{{ */
{
	const char *expected_type = NULL;
	char *error = NULL;

	expected_type = crex_parse_arg_impl(arg, va, spec, &error, arg_num);
	if (expected_type) {
		if (EG(exception)) {
			return FAILURE;
		}
		if (!(flags & CREX_PARSE_PARAMS_QUIET) && (*expected_type || error)) {
			if (error) {
				if (strcmp(error, "must not contain any null bytes") == 0) {
					crex_argument_value_error(arg_num, "%s", error);
				} else {
					crex_argument_type_error(arg_num, "%s", error);
				}
				efree(error);
			} else {
				crex_argument_type_error(arg_num, "must be of type %s, %s given", expected_type, crex_zval_value_name(arg));
			}
		} else if (error) {
			efree(error);
		}

		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

CREX_API crex_result crex_parse_parameter(int flags, uint32_t arg_num, zval *arg, const char *spec, ...)
{
	va_list va;
	crex_result ret;

	va_start(va, spec);
	ret = crex_parse_arg(arg_num, arg, &va, &spec, flags);
	va_end(va);

	return ret;
}

static CREX_COLD void crex_parse_parameters_debug_error(const char *msg) {
	crex_function *active_function = EG(current_execute_data)->func;
	const char *class_name = active_function->common.scope
		? ZSTR_VAL(active_function->common.scope->name) : "";
	crex_error_noreturn(E_CORE_ERROR, "%s%s%s(): %s",
		class_name, class_name[0] ? "::" : "",
		ZSTR_VAL(active_function->common.function_name), msg);
}

static crex_result crex_parse_va_args(uint32_t num_args, const char *type_spec, va_list *va, int flags) /* {{{ */
{
	const  char *spec_walk;
	char c;
	uint32_t i;
	uint32_t min_num_args = 0;
	uint32_t max_num_args = 0;
	uint32_t post_varargs = 0;
	zval *arg;
	bool have_varargs = 0;
	bool have_optional_args = 0;
	zval **varargs = NULL;
	uint32_t *n_varargs = NULL;

	for (spec_walk = type_spec; *spec_walk; spec_walk++) {
		c = *spec_walk;
		switch (c) {
			case 'l': case 'd':
			case 's': case 'b':
			case 'r': case 'a':
			case 'o': case 'O':
			case 'z': case 'Z':
			case 'C': case 'h':
			case 'f': case 'A':
			case 'H': case 'p':
			case 'S': case 'P':
			case 'L': case 'n':
				max_num_args++;
				break;

			case '|':
				min_num_args = max_num_args;
				have_optional_args = 1;
				break;

			case '/':
			case '!':
				/* Pass */
				break;

			case '*':
			case '+':
				if (have_varargs) {
					crex_parse_parameters_debug_error(
						"only one varargs specifier (* or +) is permitted");
					return FAILURE;
				}
				have_varargs = 1;
				/* we expect at least one parameter in varargs */
				if (c == '+') {
					max_num_args++;
				}
				/* mark the beginning of varargs */
				post_varargs = max_num_args;

				if (CREX_CALL_INFO(EG(current_execute_data)) & CREX_CALL_HAS_EXTRA_NAMED_PARAMS) {
					crex_unexpected_extra_named_error();
					return FAILURE;
				}
				break;

			default:
				crex_parse_parameters_debug_error("bad type specifier while parsing parameters");
				return FAILURE;
		}
	}

	/* with no optional arguments the minimum number of arguments must be the same as the maximum */
	if (!have_optional_args) {
		min_num_args = max_num_args;
	}

	if (have_varargs) {
		/* calculate how many required args are at the end of the specifier list */
		post_varargs = max_num_args - post_varargs;
		max_num_args = UINT32_MAX;
	}

	if (num_args < min_num_args || num_args > max_num_args) {
		if (!(flags & CREX_PARSE_PARAMS_QUIET)) {
			crex_string *func_name = get_active_function_or_method_name();

			crex_argument_count_error("%s() expects %s %d argument%s, %d given",
				ZSTR_VAL(func_name),
				min_num_args == max_num_args ? "exactly" : num_args < min_num_args ? "at least" : "at most",
				num_args < min_num_args ? min_num_args : max_num_args,
				(num_args < min_num_args ? min_num_args : max_num_args) == 1 ? "" : "s",
				num_args
			);

			crex_string_release(func_name);
		}
		return FAILURE;
	}

	if (num_args > CREX_CALL_NUM_ARGS(EG(current_execute_data))) {
		crex_parse_parameters_debug_error("could not obtain parameters for parsing");
		return FAILURE;
	}

	i = 0;
	while (num_args-- > 0) {
		if (*type_spec == '|') {
			type_spec++;
		}

		if (*type_spec == '*' || *type_spec == '+') {
			uint32_t num_varargs = num_args + 1 - post_varargs;

			/* eat up the passed in storage even if it won't be filled in with varargs */
			varargs = va_arg(*va, zval **);
			n_varargs = va_arg(*va, uint32_t *);
			type_spec++;

			if (num_varargs > 0) {
				*n_varargs = num_varargs;
				*varargs = CREX_CALL_ARG(EG(current_execute_data), i + 1);
				/* adjust how many args we have left and restart loop */
				num_args += 1 - num_varargs;
				i += num_varargs;
				continue;
			} else {
				*varargs = NULL;
				*n_varargs = 0;
			}
		}

		arg = CREX_CALL_ARG(EG(current_execute_data), i + 1);

		if (crex_parse_arg(i+1, arg, va, &type_spec, flags) == FAILURE) {
			/* clean up varargs array if it was used */
			if (varargs && *varargs) {
				*varargs = NULL;
			}
			return FAILURE;
		}
		i++;
	}

	return SUCCESS;
}
/* }}} */

CREX_API crex_result crex_parse_parameters_ex(int flags, uint32_t num_args, const char *type_spec, ...) /* {{{ */
{
	va_list va;
	crex_result retval;

	va_start(va, type_spec);
	retval = crex_parse_va_args(num_args, type_spec, &va, flags);
	va_end(va);

	return retval;
}
/* }}} */

CREX_API crex_result crex_parse_parameters(uint32_t num_args, const char *type_spec, ...) /* {{{ */
{
	va_list va;
	crex_result retval;
	int flags = 0;

	va_start(va, type_spec);
	retval = crex_parse_va_args(num_args, type_spec, &va, flags);
	va_end(va);

	return retval;
}
/* }}} */

CREX_API crex_result crex_parse_method_parameters(uint32_t num_args, zval *this_ptr, const char *type_spec, ...) /* {{{ */
{
	va_list va;
	crex_result retval;
	int flags = 0;
	const char *p = type_spec;
	zval **object;
	crex_class_entry *ce;

	/* Just checking this_ptr is not enough, because fcall_common_helper does not set
	 * C_OBJ(EG(This)) to NULL when calling an internal function with common.scope == NULL.
	 * In that case EG(This) would still be the $this from the calling code and we'd take the
	 * wrong branch here. */
	bool is_method = EG(current_execute_data)->func->common.scope != NULL;

	if (!is_method || !this_ptr || C_TYPE_P(this_ptr) != IS_OBJECT) {
		va_start(va, type_spec);
		retval = crex_parse_va_args(num_args, type_spec, &va, flags);
		va_end(va);
	} else {
		p++;

		va_start(va, type_spec);

		object = va_arg(va, zval **);
		ce = va_arg(va, crex_class_entry *);
		*object = this_ptr;

		if (ce && !instanceof_function(C_OBJCE_P(this_ptr), ce)) {
			crex_error_noreturn(E_CORE_ERROR, "%s::%s() must be derived from %s::%s()",
				ZSTR_VAL(C_OBJCE_P(this_ptr)->name), get_active_function_name(), ZSTR_VAL(ce->name), get_active_function_name());
		}

		retval = crex_parse_va_args(num_args, p, &va, flags);
		va_end(va);
	}
	return retval;
}
/* }}} */

CREX_API crex_result crex_parse_method_parameters_ex(int flags, uint32_t num_args, zval *this_ptr, const char *type_spec, ...) /* {{{ */
{
	va_list va;
	crex_result retval;
	const char *p = type_spec;
	zval **object;
	crex_class_entry *ce;

	if (!this_ptr) {
		va_start(va, type_spec);
		retval = crex_parse_va_args(num_args, type_spec, &va, flags);
		va_end(va);
	} else {
		p++;
		va_start(va, type_spec);

		object = va_arg(va, zval **);
		ce = va_arg(va, crex_class_entry *);
		*object = this_ptr;

		if (ce && !instanceof_function(C_OBJCE_P(this_ptr), ce)) {
			if (!(flags & CREX_PARSE_PARAMS_QUIET)) {
				crex_error_noreturn(E_CORE_ERROR, "%s::%s() must be derived from %s::%s()",
					ZSTR_VAL(ce->name), get_active_function_name(), ZSTR_VAL(C_OBJCE_P(this_ptr)->name), get_active_function_name());
			}
			va_end(va);
			return FAILURE;
		}

		retval = crex_parse_va_args(num_args, p, &va, flags);
		va_end(va);
	}
	return retval;
}
/* }}} */

/* This function should be called after the constructor has been called
 * because it may call __set from the uninitialized object otherwise. */
CREX_API void crex_merge_properties(zval *obj, HashTable *properties) /* {{{ */
{
	crex_object *zobj = C_OBJ_P(obj);
	crex_object_write_property_t write_property = zobj->handlers->write_property;
	crex_class_entry *old_scope = EG(fake_scope);
	crex_string *key;
	zval *value;

	if (HT_IS_PACKED(properties)) {
		return;
	}
	EG(fake_scope) = C_OBJCE_P(obj);
	CREX_HASH_MAP_FOREACH_STR_KEY_VAL(properties, key, value) {
		if (key) {
			write_property(zobj, key, value, NULL);
		}
	} CREX_HASH_FOREACH_END();
	EG(fake_scope) = old_scope;
}
/* }}} */

static crex_class_mutable_data *crex_allocate_mutable_data(crex_class_entry *class_type) /* {{{ */
{
	crex_class_mutable_data *mutable_data;

	CREX_ASSERT(CREX_MAP_PTR(class_type->mutable_data) != NULL);
	CREX_ASSERT(CREX_MAP_PTR_GET_IMM(class_type->mutable_data) == NULL);

	mutable_data = crex_arena_alloc(&CG(arena), sizeof(crex_class_mutable_data));
	memset(mutable_data, 0, sizeof(crex_class_mutable_data));
	mutable_data->ce_flags = class_type->ce_flags;
	CREX_MAP_PTR_SET_IMM(class_type->mutable_data, mutable_data);

	return mutable_data;
}
/* }}} */

CREX_API HashTable *crex_separate_class_constants_table(crex_class_entry *class_type) /* {{{ */
{
	crex_class_mutable_data *mutable_data;
	HashTable *constants_table;
	crex_string *key;
	crex_class_constant *new_c, *c;

	constants_table = crex_arena_alloc(&CG(arena), sizeof(HashTable));
	crex_hash_init(constants_table, crex_hash_num_elements(&class_type->constants_table), NULL, NULL, 0);
	crex_hash_extend(constants_table, crex_hash_num_elements(&class_type->constants_table), 0);

	CREX_HASH_MAP_FOREACH_STR_KEY_PTR(&class_type->constants_table, key, c) {
		if (c->ce == class_type) {
			if (C_TYPE(c->value) == IS_CONSTANT_AST) {
				new_c = crex_arena_alloc(&CG(arena), sizeof(crex_class_constant));
				memcpy(new_c, c, sizeof(crex_class_constant));
				c = new_c;
			}
			C_TRY_ADDREF(c->value);
		} else {
			if (C_TYPE(c->value) == IS_CONSTANT_AST) {
				c = crex_hash_find_ptr(CE_CONSTANTS_TABLE(c->ce), key);
				CREX_ASSERT(c);
			}
		}
		_crex_hash_append_ptr(constants_table, key, c);
	} CREX_HASH_FOREACH_END();

	CREX_ASSERT(CREX_MAP_PTR(class_type->mutable_data) != NULL);

	mutable_data = CREX_MAP_PTR_GET_IMM(class_type->mutable_data);
	if (!mutable_data) {
		mutable_data = crex_allocate_mutable_data(class_type);
	}

	mutable_data->constants_table = constants_table;

	return constants_table;
}

static crex_result update_property(zval *val, crex_property_info *prop_info) {
	if (CREX_TYPE_IS_SET(prop_info->type)) {
		zval tmp;

		ZVAL_COPY(&tmp, val);
		if (UNEXPECTED(zval_update_constant_ex(&tmp, prop_info->ce) != SUCCESS)) {
			zval_ptr_dtor(&tmp);
			return FAILURE;
		}
		/* property initializers must always be evaluated with strict types */;
		if (UNEXPECTED(!crex_verify_property_type(prop_info, &tmp, /* strict */ 1))) {
			zval_ptr_dtor(&tmp);
			return FAILURE;
		}
		zval_ptr_dtor(val);
		ZVAL_COPY_VALUE(val, &tmp);
		return SUCCESS;
	}
	return zval_update_constant_ex(val, prop_info->ce);
}

CREX_API crex_result crex_update_class_constant(crex_class_constant *c, const crex_string *name, crex_class_entry *scope)
{
	CREX_ASSERT(C_TYPE(c->value) == IS_CONSTANT_AST);

	if (EXPECTED(!CREX_TYPE_IS_SET(c->type) || CREX_TYPE_PURE_MASK(c->type) == MAY_BE_ANY)) {
		return zval_update_constant_ex(&c->value, scope);
	}

	zval tmp;

	ZVAL_COPY(&tmp, &c->value);
	crex_result result = zval_update_constant_ex(&tmp, scope);
	if (result == FAILURE) {
		zval_ptr_dtor(&tmp);
		return FAILURE;
	}

	if (UNEXPECTED(!crex_verify_class_constant_type(c, name, &tmp))) {
		zval_ptr_dtor(&tmp);
		return FAILURE;
	}

	zval_ptr_dtor(&c->value);
	ZVAL_COPY_VALUE(&c->value, &tmp);

	return SUCCESS;
}

CREX_API crex_result crex_update_class_constants(crex_class_entry *class_type) /* {{{ */
{
	crex_class_mutable_data *mutable_data = NULL;
	zval *default_properties_table = NULL;
	zval *static_members_table = NULL;
	crex_class_constant *c;
	zval *val;
	uint32_t ce_flags;

	ce_flags = class_type->ce_flags;

	if (ce_flags & CREX_ACC_CONSTANTS_UPDATED) {
		return SUCCESS;
	}

	bool uses_mutable_data = CREX_MAP_PTR(class_type->mutable_data) != NULL;
	if (uses_mutable_data) {
		mutable_data = CREX_MAP_PTR_GET_IMM(class_type->mutable_data);
		if (mutable_data) {
			ce_flags = mutable_data->ce_flags;
			if (ce_flags & CREX_ACC_CONSTANTS_UPDATED) {
				return SUCCESS;
			}
		} else {
			mutable_data = crex_allocate_mutable_data(class_type);
		}
	}

	if (class_type->parent) {
		if (UNEXPECTED(crex_update_class_constants(class_type->parent) != SUCCESS)) {
			return FAILURE;
		}
	}

	if (ce_flags & CREX_ACC_HAS_AST_CONSTANTS) {
		HashTable *constants_table;

		if (uses_mutable_data) {
			constants_table = mutable_data->constants_table;
			if (!constants_table) {
				constants_table = crex_separate_class_constants_table(class_type);
			}
		} else {
			constants_table = &class_type->constants_table;
		}

		crex_string *name;
		CREX_HASH_MAP_FOREACH_STR_KEY_VAL(constants_table, name, val) {
			c = C_PTR_P(val);
			if (C_TYPE(c->value) == IS_CONSTANT_AST) {
				if (c->ce != class_type) {
					C_PTR_P(val) = c = crex_hash_find_ptr(CE_CONSTANTS_TABLE(c->ce), name);
					if (C_TYPE(c->value) != IS_CONSTANT_AST) {
						continue;
					}
				}

				val = &c->value;
				if (UNEXPECTED(crex_update_class_constant(c, name, c->ce) != SUCCESS)) {
					return FAILURE;
				}
			}
		} CREX_HASH_FOREACH_END();
	}

	if (class_type->default_static_members_count) {
		static_members_table = CE_STATIC_MEMBERS(class_type);
		if (!static_members_table) {
			crex_class_init_statics(class_type);
			static_members_table = CE_STATIC_MEMBERS(class_type);
		}
	}

	default_properties_table = class_type->default_properties_table;
	if (uses_mutable_data && (ce_flags & CREX_ACC_HAS_AST_PROPERTIES)) {
		zval *src, *dst, *end;

		default_properties_table = mutable_data->default_properties_table;
		if (!default_properties_table) {
			default_properties_table = crex_arena_alloc(&CG(arena), sizeof(zval) * class_type->default_properties_count);
			src = class_type->default_properties_table;
			dst = default_properties_table;
			end = dst + class_type->default_properties_count;
			do {
				ZVAL_COPY_PROP(dst, src);
				src++;
				dst++;
			} while (dst != end);
			mutable_data->default_properties_table = default_properties_table;
		}
	}

	if (ce_flags & (CREX_ACC_HAS_AST_PROPERTIES|CREX_ACC_HAS_AST_STATICS)) {
		crex_property_info *prop_info;

		/* Use the default properties table to also update initializers of private properties
		 * that have been shadowed in a child class. */
		for (uint32_t i = 0; i < class_type->default_properties_count; i++) {
			val = &default_properties_table[i];
			prop_info = class_type->properties_info_table[i];
			if (C_TYPE_P(val) == IS_CONSTANT_AST
					&& UNEXPECTED(update_property(val, prop_info) != SUCCESS)) {
				return FAILURE;
			}
		}

		if (class_type->default_static_members_count) {
			CREX_HASH_MAP_FOREACH_PTR(&class_type->properties_info, prop_info) {
				if (prop_info->flags & CREX_ACC_STATIC) {
					val = static_members_table + prop_info->offset;
					if (C_TYPE_P(val) == IS_CONSTANT_AST
							&& UNEXPECTED(update_property(val, prop_info) != SUCCESS)) {
						return FAILURE;
					}
				}
			} CREX_HASH_FOREACH_END();
		}
	}

	if (class_type->type == CREX_USER_CLASS && class_type->ce_flags & CREX_ACC_ENUM && class_type->enum_backing_type != IS_UNDEF) {
		if (crex_enum_build_backed_enum_table(class_type) == FAILURE) {
			return FAILURE;
		}
	}

	ce_flags |= CREX_ACC_CONSTANTS_UPDATED;
	ce_flags &= ~CREX_ACC_HAS_AST_CONSTANTS;
	ce_flags &= ~CREX_ACC_HAS_AST_PROPERTIES;
	ce_flags &= ~CREX_ACC_HAS_AST_STATICS;
	if (uses_mutable_data) {
		mutable_data->ce_flags = ce_flags;
	} else {
		class_type->ce_flags = ce_flags;
	}

	return SUCCESS;
}
/* }}} */

static crex_always_inline void _object_properties_init(crex_object *object, crex_class_entry *class_type) /* {{{ */
{
	if (class_type->default_properties_count) {
		zval *src = CE_DEFAULT_PROPERTIES_TABLE(class_type);
		zval *dst = object->properties_table;
		zval *end = src + class_type->default_properties_count;

		if (UNEXPECTED(class_type->type == CREX_INTERNAL_CLASS)) {
			do {
				ZVAL_COPY_OR_DUP_PROP(dst, src);
				src++;
				dst++;
			} while (src != end);
		} else {
			do {
				ZVAL_COPY_PROP(dst, src);
				src++;
				dst++;
			} while (src != end);
		}
	}
}
/* }}} */

CREX_API void object_properties_init(crex_object *object, crex_class_entry *class_type) /* {{{ */
{
	object->properties = NULL;
	_object_properties_init(object, class_type);
}
/* }}} */

CREX_API void object_properties_init_ex(crex_object *object, HashTable *properties) /* {{{ */
{
	object->properties = properties;
	if (object->ce->default_properties_count) {
		zval *prop;
		crex_string *key;
		crex_property_info *property_info;

		CREX_HASH_MAP_FOREACH_STR_KEY_VAL(properties, key, prop) {
			property_info = crex_get_property_info(object->ce, key, 1);
			if (property_info != CREX_WRONG_PROPERTY_INFO &&
			    property_info &&
			    (property_info->flags & CREX_ACC_STATIC) == 0) {
				zval *slot = OBJ_PROP(object, property_info->offset);

				if (UNEXPECTED(CREX_TYPE_IS_SET(property_info->type))) {
					zval tmp;

					ZVAL_COPY_VALUE(&tmp, prop);
					if (UNEXPECTED(!crex_verify_property_type(property_info, &tmp, 0))) {
						continue;
					}
					ZVAL_COPY_VALUE(slot, &tmp);
				} else {
					ZVAL_COPY_VALUE(slot, prop);
				}
				ZVAL_INDIRECT(prop, slot);
			}
		} CREX_HASH_FOREACH_END();
	}
}
/* }}} */

CREX_API void object_properties_load(crex_object *object, HashTable *properties) /* {{{ */
{
	zval *prop, tmp;
	crex_string *key;
	crex_long h;
	crex_property_info *property_info;

	CREX_HASH_FOREACH_KEY_VAL(properties, h, key, prop) {
		if (key) {
			if (ZSTR_VAL(key)[0] == '\0') {
				const char *class_name, *prop_name;
				size_t prop_name_len;
				if (crex_unmangle_property_name_ex(key, &class_name, &prop_name, &prop_name_len) == SUCCESS) {
					crex_string *pname = crex_string_init(prop_name, prop_name_len, 0);
					crex_class_entry *prev_scope = EG(fake_scope);
					if (class_name && class_name[0] != '*') {
						crex_string *cname = crex_string_init(class_name, strlen(class_name), 0);
						EG(fake_scope) = crex_lookup_class(cname);
						crex_string_release_ex(cname, 0);
					}
					property_info = crex_get_property_info(object->ce, pname, 1);
					crex_string_release_ex(pname, 0);
					EG(fake_scope) = prev_scope;
				} else {
					property_info = CREX_WRONG_PROPERTY_INFO;
				}
			} else {
				property_info = crex_get_property_info(object->ce, key, 1);
			}
			if (property_info != CREX_WRONG_PROPERTY_INFO &&
				property_info &&
				(property_info->flags & CREX_ACC_STATIC) == 0) {
				zval *slot = OBJ_PROP(object, property_info->offset);
				zval_ptr_dtor(slot);
				ZVAL_COPY_VALUE(slot, prop);
				zval_add_ref(slot);
				if (object->properties) {
					ZVAL_INDIRECT(&tmp, slot);
					crex_hash_update(object->properties, key, &tmp);
				}
			} else {
				if (UNEXPECTED(object->ce->ce_flags & CREX_ACC_NO_DYNAMIC_PROPERTIES)) {
					crex_throw_error(NULL, "Cannot create dynamic property %s::$%s",
						ZSTR_VAL(object->ce->name), property_info != CREX_WRONG_PROPERTY_INFO ? crex_get_unmangled_property_name(key): "");
					return;
				} else if (!(object->ce->ce_flags & CREX_ACC_ALLOW_DYNAMIC_PROPERTIES)) {
					crex_error(E_DEPRECATED, "Creation of dynamic property %s::$%s is deprecated",
						ZSTR_VAL(object->ce->name), property_info != CREX_WRONG_PROPERTY_INFO ? crex_get_unmangled_property_name(key): "");
				}

				if (!object->properties) {
					rebuild_object_properties(object);
				}
				prop = crex_hash_update(object->properties, key, prop);
				zval_add_ref(prop);
			}
		} else {
			if (UNEXPECTED(object->ce->ce_flags & CREX_ACC_NO_DYNAMIC_PROPERTIES)) {
				crex_throw_error(NULL, "Cannot create dynamic property %s::$" CREX_LONG_FMT, ZSTR_VAL(object->ce->name), h);
				return;
			} else if (!(object->ce->ce_flags & CREX_ACC_ALLOW_DYNAMIC_PROPERTIES)) {
				crex_error(E_DEPRECATED, "Creation of dynamic property %s::$" CREX_LONG_FMT " is deprecated",
					ZSTR_VAL(object->ce->name), h);
			}

			if (!object->properties) {
				rebuild_object_properties(object);
			}
			prop = crex_hash_index_update(object->properties, h, prop);
			zval_add_ref(prop);
		}
	} CREX_HASH_FOREACH_END();
}
/* }}} */

/* This function requires 'properties' to contain all props declared in the
 * class and all props being public. If only a subset is given or the class
 * has protected members then you need to merge the properties separately by
 * calling crex_merge_properties(). */
static crex_always_inline crex_result _object_and_properties_init(zval *arg, crex_class_entry *class_type, HashTable *properties) /* {{{ */
{
	if (UNEXPECTED(class_type->ce_flags & (CREX_ACC_INTERFACE|CREX_ACC_TRAIT|CREX_ACC_IMPLICIT_ABSTRACT_CLASS|CREX_ACC_EXPLICIT_ABSTRACT_CLASS|CREX_ACC_ENUM))) {
		if (class_type->ce_flags & CREX_ACC_INTERFACE) {
			crex_throw_error(NULL, "Cannot instantiate interface %s", ZSTR_VAL(class_type->name));
		} else if (class_type->ce_flags & CREX_ACC_TRAIT) {
			crex_throw_error(NULL, "Cannot instantiate trait %s", ZSTR_VAL(class_type->name));
		} else if (class_type->ce_flags & CREX_ACC_ENUM) {
			crex_throw_error(NULL, "Cannot instantiate enum %s", ZSTR_VAL(class_type->name));
		} else {
			crex_throw_error(NULL, "Cannot instantiate abstract class %s", ZSTR_VAL(class_type->name));
		}
		ZVAL_NULL(arg);
		C_OBJ_P(arg) = NULL;
		return FAILURE;
	}

	if (UNEXPECTED(!(class_type->ce_flags & CREX_ACC_CONSTANTS_UPDATED))) {
		if (UNEXPECTED(crex_update_class_constants(class_type) != SUCCESS)) {
			ZVAL_NULL(arg);
			C_OBJ_P(arg) = NULL;
			return FAILURE;
		}
	}

	if (class_type->create_object == NULL) {
		crex_object *obj = crex_objects_new(class_type);

		ZVAL_OBJ(arg, obj);
		if (properties) {
			object_properties_init_ex(obj, properties);
		} else {
			_object_properties_init(obj, class_type);
		}
	} else {
		ZVAL_OBJ(arg, class_type->create_object(class_type));
	}
	return SUCCESS;
}
/* }}} */

CREX_API crex_result object_and_properties_init(zval *arg, crex_class_entry *class_type, HashTable *properties) /* {{{ */
{
	return _object_and_properties_init(arg, class_type, properties);
}
/* }}} */

CREX_API crex_result object_init_ex(zval *arg, crex_class_entry *class_type) /* {{{ */
{
	return _object_and_properties_init(arg, class_type, NULL);
}
/* }}} */

CREX_API void object_init(zval *arg) /* {{{ */
{
	ZVAL_OBJ(arg, crex_objects_new(crex_standard_class_def));
}
/* }}} */

CREX_API void add_assoc_long_ex(zval *arg, const char *key, size_t key_len, crex_long n) /* {{{ */
{
	zval tmp;

	ZVAL_LONG(&tmp, n);
	crex_symtable_str_update(C_ARRVAL_P(arg), key, key_len, &tmp);
}
/* }}} */

CREX_API void add_assoc_null_ex(zval *arg, const char *key, size_t key_len) /* {{{ */
{
	zval tmp;

	ZVAL_NULL(&tmp);
	crex_symtable_str_update(C_ARRVAL_P(arg), key, key_len, &tmp);
}
/* }}} */

CREX_API void add_assoc_bool_ex(zval *arg, const char *key, size_t key_len, bool b) /* {{{ */
{
	zval tmp;

	ZVAL_BOOL(&tmp, b);
	crex_symtable_str_update(C_ARRVAL_P(arg), key, key_len, &tmp);
}
/* }}} */

CREX_API void add_assoc_resource_ex(zval *arg, const char *key, size_t key_len, crex_resource *r) /* {{{ */
{
	zval tmp;

	ZVAL_RES(&tmp, r);
	crex_symtable_str_update(C_ARRVAL_P(arg), key, key_len, &tmp);
}
/* }}} */

CREX_API void add_assoc_double_ex(zval *arg, const char *key, size_t key_len, double d) /* {{{ */
{
	zval tmp;

	ZVAL_DOUBLE(&tmp, d);
	crex_symtable_str_update(C_ARRVAL_P(arg), key, key_len, &tmp);
}
/* }}} */

CREX_API void add_assoc_str_ex(zval *arg, const char *key, size_t key_len, crex_string *str) /* {{{ */
{
	zval tmp;

	ZVAL_STR(&tmp, str);
	crex_symtable_str_update(C_ARRVAL_P(arg), key, key_len, &tmp);
}
/* }}} */

CREX_API void add_assoc_string_ex(zval *arg, const char *key, size_t key_len, const char *str) /* {{{ */
{
	zval tmp;

	ZVAL_STRING(&tmp, str);
	crex_symtable_str_update(C_ARRVAL_P(arg), key, key_len, &tmp);
}
/* }}} */

CREX_API void add_assoc_stringl_ex(zval *arg, const char *key, size_t key_len, const char *str, size_t length) /* {{{ */
{
	zval tmp;

	ZVAL_STRINGL(&tmp, str, length);
	crex_symtable_str_update(C_ARRVAL_P(arg), key, key_len, &tmp);
}
/* }}} */

CREX_API void add_assoc_array_ex(zval *arg, const char *key, size_t key_len, crex_array *arr) /* {{{ */
{
	zval tmp;

	ZVAL_ARR(&tmp, arr);
	crex_symtable_str_update(C_ARRVAL_P(arg), key, key_len, &tmp);
}
/* }}} */

CREX_API void add_assoc_object_ex(zval *arg, const char *key, size_t key_len, crex_object *obj) /* {{{ */
{
	zval tmp;

	ZVAL_OBJ(&tmp, obj);
	crex_symtable_str_update(C_ARRVAL_P(arg), key, key_len, &tmp);
}
/* }}} */

CREX_API void add_assoc_reference_ex(zval *arg, const char *key, size_t key_len, crex_reference *ref) /* {{{ */
{
	zval tmp;

	ZVAL_REF(&tmp, ref);
	crex_symtable_str_update(C_ARRVAL_P(arg), key, key_len, &tmp);
}
/* }}} */

CREX_API void add_assoc_zval_ex(zval *arg, const char *key, size_t key_len, zval *value) /* {{{ */
{
	crex_symtable_str_update(C_ARRVAL_P(arg), key, key_len, value);
}
/* }}} */

CREX_API void add_index_long(zval *arg, crex_ulong index, crex_long n) /* {{{ */
{
	zval tmp;

	ZVAL_LONG(&tmp, n);
	crex_hash_index_update(C_ARRVAL_P(arg), index, &tmp);
}
/* }}} */

CREX_API void add_index_null(zval *arg, crex_ulong index) /* {{{ */
{
	zval tmp;

	ZVAL_NULL(&tmp);
	crex_hash_index_update(C_ARRVAL_P(arg), index, &tmp);
}
/* }}} */

CREX_API void add_index_bool(zval *arg, crex_ulong index, bool b) /* {{{ */
{
	zval tmp;

	ZVAL_BOOL(&tmp, b);
	crex_hash_index_update(C_ARRVAL_P(arg), index, &tmp);
}
/* }}} */

CREX_API void add_index_resource(zval *arg, crex_ulong index, crex_resource *r) /* {{{ */
{
	zval tmp;

	ZVAL_RES(&tmp, r);
	crex_hash_index_update(C_ARRVAL_P(arg), index, &tmp);
}
/* }}} */

CREX_API void add_index_double(zval *arg, crex_ulong index, double d) /* {{{ */
{
	zval tmp;

	ZVAL_DOUBLE(&tmp, d);
	crex_hash_index_update(C_ARRVAL_P(arg), index, &tmp);
}
/* }}} */

CREX_API void add_index_str(zval *arg, crex_ulong index, crex_string *str) /* {{{ */
{
	zval tmp;

	ZVAL_STR(&tmp, str);
	crex_hash_index_update(C_ARRVAL_P(arg), index, &tmp);
}
/* }}} */

CREX_API void add_index_string(zval *arg, crex_ulong index, const char *str) /* {{{ */
{
	zval tmp;

	ZVAL_STRING(&tmp, str);
	crex_hash_index_update(C_ARRVAL_P(arg), index, &tmp);
}
/* }}} */

CREX_API void add_index_stringl(zval *arg, crex_ulong index, const char *str, size_t length) /* {{{ */
{
	zval tmp;

	ZVAL_STRINGL(&tmp, str, length);
	crex_hash_index_update(C_ARRVAL_P(arg), index, &tmp);
}
/* }}} */

CREX_API void add_index_array(zval *arg, crex_ulong index, crex_array *arr) /* {{{ */
{
	zval tmp;

	ZVAL_ARR(&tmp, arr);
	crex_hash_index_update(C_ARRVAL_P(arg), index, &tmp);
}
/* }}} */

CREX_API void add_index_object(zval *arg, crex_ulong index, crex_object *obj) /* {{{ */
{
	zval tmp;

	ZVAL_OBJ(&tmp, obj);
	crex_hash_index_update(C_ARRVAL_P(arg), index, &tmp);
}
/* }}} */

CREX_API void add_index_reference(zval *arg, crex_ulong index, crex_reference *ref) /* {{{ */
{
	zval tmp;

	ZVAL_REF(&tmp, ref);
	crex_hash_index_update(C_ARRVAL_P(arg), index, &tmp);
}
/* }}} */

CREX_API crex_result add_next_index_long(zval *arg, crex_long n) /* {{{ */
{
	zval tmp;

	ZVAL_LONG(&tmp, n);
	return crex_hash_next_index_insert(C_ARRVAL_P(arg), &tmp) ? SUCCESS : FAILURE;
}
/* }}} */

CREX_API crex_result add_next_index_null(zval *arg) /* {{{ */
{
	zval tmp;

	ZVAL_NULL(&tmp);
	return crex_hash_next_index_insert(C_ARRVAL_P(arg), &tmp) ? SUCCESS : FAILURE;
}
/* }}} */

CREX_API crex_result add_next_index_bool(zval *arg, bool b) /* {{{ */
{
	zval tmp;

	ZVAL_BOOL(&tmp, b);
	return crex_hash_next_index_insert(C_ARRVAL_P(arg), &tmp) ? SUCCESS : FAILURE;
}
/* }}} */

CREX_API crex_result add_next_index_resource(zval *arg, crex_resource *r) /* {{{ */
{
	zval tmp;

	ZVAL_RES(&tmp, r);
	return crex_hash_next_index_insert(C_ARRVAL_P(arg), &tmp) ? SUCCESS : FAILURE;
}
/* }}} */

CREX_API crex_result add_next_index_double(zval *arg, double d) /* {{{ */
{
	zval tmp;

	ZVAL_DOUBLE(&tmp, d);
	return crex_hash_next_index_insert(C_ARRVAL_P(arg), &tmp) ? SUCCESS : FAILURE;
}
/* }}} */

CREX_API crex_result add_next_index_str(zval *arg, crex_string *str) /* {{{ */
{
	zval tmp;

	ZVAL_STR(&tmp, str);
	return crex_hash_next_index_insert(C_ARRVAL_P(arg), &tmp) ? SUCCESS : FAILURE;
}
/* }}} */

CREX_API crex_result add_next_index_string(zval *arg, const char *str) /* {{{ */
{
	zval tmp;

	ZVAL_STRING(&tmp, str);
	return crex_hash_next_index_insert(C_ARRVAL_P(arg), &tmp) ? SUCCESS : FAILURE;
}
/* }}} */

CREX_API crex_result add_next_index_stringl(zval *arg, const char *str, size_t length) /* {{{ */
{
	zval tmp;

	ZVAL_STRINGL(&tmp, str, length);
	return crex_hash_next_index_insert(C_ARRVAL_P(arg), &tmp) ? SUCCESS : FAILURE;
}
/* }}} */

CREX_API crex_result add_next_index_array(zval *arg, crex_array *arr) /* {{{ */
{
	zval tmp;

	ZVAL_ARR(&tmp, arr);
	return crex_hash_next_index_insert(C_ARRVAL_P(arg), &tmp) ? SUCCESS : FAILURE;
}
/* }}} */

CREX_API crex_result add_next_index_object(zval *arg, crex_object *obj) /* {{{ */
{
	zval tmp;

	ZVAL_OBJ(&tmp, obj);
	return crex_hash_next_index_insert(C_ARRVAL_P(arg), &tmp) ? SUCCESS : FAILURE;
}
/* }}} */

CREX_API crex_result add_next_index_reference(zval *arg, crex_reference *ref) /* {{{ */
{
	zval tmp;

	ZVAL_REF(&tmp, ref);
	return crex_hash_next_index_insert(C_ARRVAL_P(arg), &tmp) ? SUCCESS : FAILURE;
}
/* }}} */

CREX_API crex_result array_set_zval_key(HashTable *ht, zval *key, zval *value) /* {{{ */
{
	zval *result;

	switch (C_TYPE_P(key)) {
		case IS_STRING:
			result = crex_symtable_update(ht, C_STR_P(key), value);
			break;
		case IS_NULL:
			result = crex_hash_update(ht, ZSTR_EMPTY_ALLOC(), value);
			break;
		case IS_RESOURCE:
			crex_use_resource_as_offset(key);
			result = crex_hash_index_update(ht, C_RES_HANDLE_P(key), value);
			break;
		case IS_FALSE:
			result = crex_hash_index_update(ht, 0, value);
			break;
		case IS_TRUE:
			result = crex_hash_index_update(ht, 1, value);
			break;
		case IS_LONG:
			result = crex_hash_index_update(ht, C_LVAL_P(key), value);
			break;
		case IS_DOUBLE:
			result = crex_hash_index_update(ht, crex_dval_to_lval_safe(C_DVAL_P(key)), value);
			break;
		default:
			crex_illegal_container_offset(ZSTR_KNOWN(CREX_STR_ARRAY), key, BP_VAR_W);
			result = NULL;
	}

	if (result) {
		C_TRY_ADDREF_P(result);
		return SUCCESS;
	} else {
		return FAILURE;
	}
}
/* }}} */

CREX_API void add_property_long_ex(zval *arg, const char *key, size_t key_len, crex_long n) /* {{{ */
{
	zval tmp;

	ZVAL_LONG(&tmp, n);
	add_property_zval_ex(arg, key, key_len, &tmp);
}
/* }}} */

CREX_API void add_property_bool_ex(zval *arg, const char *key, size_t key_len, crex_long b) /* {{{ */
{
	zval tmp;

	ZVAL_BOOL(&tmp, b);
	add_property_zval_ex(arg, key, key_len, &tmp);
}
/* }}} */

CREX_API void add_property_null_ex(zval *arg, const char *key, size_t key_len) /* {{{ */
{
	zval tmp;

	ZVAL_NULL(&tmp);
	add_property_zval_ex(arg, key, key_len, &tmp);
}
/* }}} */

CREX_API void add_property_resource_ex(zval *arg, const char *key, size_t key_len, crex_resource *r) /* {{{ */
{
	zval tmp;

	ZVAL_RES(&tmp, r);
	add_property_zval_ex(arg, key, key_len, &tmp);
	zval_ptr_dtor(&tmp); /* write_property will add 1 to refcount */
}
/* }}} */

CREX_API void add_property_double_ex(zval *arg, const char *key, size_t key_len, double d) /* {{{ */
{
	zval tmp;

	ZVAL_DOUBLE(&tmp, d);
	add_property_zval_ex(arg, key, key_len, &tmp);
}
/* }}} */

CREX_API void add_property_str_ex(zval *arg, const char *key, size_t key_len, crex_string *str) /* {{{ */
{
	zval tmp;

	ZVAL_STR(&tmp, str);
	add_property_zval_ex(arg, key, key_len, &tmp);
	zval_ptr_dtor(&tmp); /* write_property will add 1 to refcount */
}
/* }}} */

CREX_API void add_property_string_ex(zval *arg, const char *key, size_t key_len, const char *str) /* {{{ */
{
	zval tmp;

	ZVAL_STRING(&tmp, str);
	add_property_zval_ex(arg, key, key_len, &tmp);
	zval_ptr_dtor(&tmp); /* write_property will add 1 to refcount */
}
/* }}} */

CREX_API void add_property_stringl_ex(zval *arg, const char *key, size_t key_len, const char *str, size_t length) /* {{{ */
{
	zval tmp;

	ZVAL_STRINGL(&tmp, str, length);
	add_property_zval_ex(arg, key, key_len, &tmp);
	zval_ptr_dtor(&tmp); /* write_property will add 1 to refcount */
}
/* }}} */

CREX_API void add_property_array_ex(zval *arg, const char *key, size_t key_len, crex_array *arr) /* {{{ */
{
	zval tmp;

	ZVAL_ARR(&tmp, arr);
	add_property_zval_ex(arg, key, key_len, &tmp);
	zval_ptr_dtor(&tmp); /* write_property will add 1 to refcount */
}
/* }}} */

CREX_API void add_property_object_ex(zval *arg, const char *key, size_t key_len, crex_object *obj) /* {{{ */
{
	zval tmp;

	ZVAL_OBJ(&tmp, obj);
	add_property_zval_ex(arg, key, key_len, &tmp);
	zval_ptr_dtor(&tmp); /* write_property will add 1 to refcount */
}
/* }}} */

CREX_API void add_property_reference_ex(zval *arg, const char *key, size_t key_len, crex_reference *ref) /* {{{ */
{
	zval tmp;

	ZVAL_REF(&tmp, ref);
	add_property_zval_ex(arg, key, key_len, &tmp);
	zval_ptr_dtor(&tmp); /* write_property will add 1 to refcount */
}
/* }}} */

CREX_API void add_property_zval_ex(zval *arg, const char *key, size_t key_len, zval *value) /* {{{ */
{
	crex_string *str;

	str = crex_string_init(key, key_len, 0);
	C_OBJ_HANDLER_P(arg, write_property)(C_OBJ_P(arg), str, value, NULL);
	crex_string_release_ex(str, 0);
}
/* }}} */

CREX_API crex_result crex_startup_module_ex(crex_module_entry *module) /* {{{ */
{
	size_t name_len;
	crex_string *lcname;

	if (module->module_started) {
		return SUCCESS;
	}
	module->module_started = 1;

	/* Check module dependencies */
	if (module->deps) {
		const crex_module_dep *dep = module->deps;

		while (dep->name) {
			if (dep->type == MODULE_DEP_REQUIRED) {
				crex_module_entry *req_mod;

				name_len = strlen(dep->name);
				lcname = crex_string_alloc(name_len, 0);
				crex_str_tolower_copy(ZSTR_VAL(lcname), dep->name, name_len);

				if ((req_mod = crex_hash_find_ptr(&module_registry, lcname)) == NULL || !req_mod->module_started) {
					crex_string_efree(lcname);
					/* TODO: Check version relationship */
					crex_error(E_CORE_WARNING, "Cannot load module \"%s\" because required module \"%s\" is not loaded", module->name, dep->name);
					module->module_started = 0;
					return FAILURE;
				}
				crex_string_efree(lcname);
			}
			++dep;
		}
	}

	/* Initialize module globals */
	if (module->globals_size) {
#ifdef ZTS
		ts_allocate_id(module->globals_id_ptr, module->globals_size, (ts_allocate_ctor) module->globals_ctor, (ts_allocate_dtor) module->globals_dtor);
#else
		if (module->globals_ctor) {
			module->globals_ctor(module->globals_ptr);
		}
#endif
	}
	if (module->module_startup_func) {
		EG(current_module) = module;
		if (module->module_startup_func(module->type, module->module_number)==FAILURE) {
			crex_error_noreturn(E_CORE_ERROR,"Unable to start %s module", module->name);
			EG(current_module) = NULL;
			return FAILURE;
		}
		EG(current_module) = NULL;
	}
	return SUCCESS;
}
/* }}} */

static int crex_startup_module_zval(zval *zv) /* {{{ */
{
	crex_module_entry *module = C_PTR_P(zv);

	return (crex_startup_module_ex(module) == SUCCESS) ? CREX_HASH_APPLY_KEEP : CREX_HASH_APPLY_REMOVE;
}
/* }}} */

static void crex_sort_modules(void *base, size_t count, size_t siz, compare_func_t compare, swap_func_t swp) /* {{{ */
{
	Bucket *b1 = base;
	Bucket *b2;
	Bucket *end = b1 + count;
	Bucket tmp;
	crex_module_entry *m, *r;

	while (b1 < end) {
try_again:
		m = (crex_module_entry*)C_PTR(b1->val);
		if (!m->module_started && m->deps) {
			const crex_module_dep *dep = m->deps;
			while (dep->name) {
				if (dep->type == MODULE_DEP_REQUIRED || dep->type == MODULE_DEP_OPTIONAL) {
					b2 = b1 + 1;
					while (b2 < end) {
						r = (crex_module_entry*)C_PTR(b2->val);
						if (strcasecmp(dep->name, r->name) == 0) {
							tmp = *b1;
							*b1 = *b2;
							*b2 = tmp;
							goto try_again;
						}
						b2++;
					}
				}
				dep++;
			}
		}
		b1++;
	}
}
/* }}} */

CREX_API void crex_collect_module_handlers(void) /* {{{ */
{
	crex_module_entry *module;
	int startup_count = 0;
	int shutdown_count = 0;
	int post_deactivate_count = 0;
	crex_class_entry *ce;
	int class_count = 0;

	/* Collect extensions with request startup/shutdown handlers */
	CREX_HASH_MAP_FOREACH_PTR(&module_registry, module) {
		if (module->request_startup_func) {
			startup_count++;
		}
		if (module->request_shutdown_func) {
			shutdown_count++;
		}
		if (module->post_deactivate_func) {
			post_deactivate_count++;
		}
	} CREX_HASH_FOREACH_END();
	module_request_startup_handlers = (crex_module_entry**)realloc(
		module_request_startup_handlers,
	    sizeof(crex_module_entry*) *
		(startup_count + 1 +
		 shutdown_count + 1 +
		 post_deactivate_count + 1));
	module_request_startup_handlers[startup_count] = NULL;
	module_request_shutdown_handlers = module_request_startup_handlers + startup_count + 1;
	module_request_shutdown_handlers[shutdown_count] = NULL;
	module_post_deactivate_handlers = module_request_shutdown_handlers + shutdown_count + 1;
	module_post_deactivate_handlers[post_deactivate_count] = NULL;
	startup_count = 0;

	CREX_HASH_MAP_FOREACH_PTR(&module_registry, module) {
		if (module->request_startup_func) {
			module_request_startup_handlers[startup_count++] = module;
		}
		if (module->request_shutdown_func) {
			module_request_shutdown_handlers[--shutdown_count] = module;
		}
		if (module->post_deactivate_func) {
			module_post_deactivate_handlers[--post_deactivate_count] = module;
		}
	} CREX_HASH_FOREACH_END();

	/* Collect internal classes with static members */
	CREX_HASH_MAP_FOREACH_PTR(CG(class_table), ce) {
		if (ce->type == CREX_INTERNAL_CLASS &&
		    ce->default_static_members_count > 0) {
		    class_count++;
		}
	} CREX_HASH_FOREACH_END();

	class_cleanup_handlers = (crex_class_entry**)realloc(
		class_cleanup_handlers,
		sizeof(crex_class_entry*) *
		(class_count + 1));
	class_cleanup_handlers[class_count] = NULL;

	if (class_count) {
		CREX_HASH_MAP_FOREACH_PTR(CG(class_table), ce) {
			if (ce->type == CREX_INTERNAL_CLASS &&
			    ce->default_static_members_count > 0) {
			    class_cleanup_handlers[--class_count] = ce;
			}
		} CREX_HASH_FOREACH_END();
	}
}
/* }}} */

CREX_API void crex_startup_modules(void) /* {{{ */
{
	crex_hash_sort_ex(&module_registry, crex_sort_modules, NULL, 0);
	crex_hash_apply(&module_registry, crex_startup_module_zval);
}
/* }}} */

CREX_API void crex_destroy_modules(void) /* {{{ */
{
	free(class_cleanup_handlers);
	class_cleanup_handlers = NULL;
	free(module_request_startup_handlers);
	module_request_startup_handlers = NULL;
	crex_hash_graceful_reverse_destroy(&module_registry);
}
/* }}} */

CREX_API crex_module_entry* crex_register_module_ex(crex_module_entry *module) /* {{{ */
{
	size_t name_len;
	crex_string *lcname;
	crex_module_entry *module_ptr;

	if (!module) {
		return NULL;
	}

#if 0
	crex_printf("%s: Registering module %d\n", module->name, module->module_number);
#endif

	/* Check module dependencies */
	if (module->deps) {
		const crex_module_dep *dep = module->deps;

		while (dep->name) {
			if (dep->type == MODULE_DEP_CONFLICTS) {
				name_len = strlen(dep->name);
				lcname = crex_string_alloc(name_len, 0);
				crex_str_tolower_copy(ZSTR_VAL(lcname), dep->name, name_len);

				if (crex_hash_exists(&module_registry, lcname) || crex_get_extension(dep->name)) {
					crex_string_efree(lcname);
					/* TODO: Check version relationship */
					crex_error(E_CORE_WARNING, "Cannot load module \"%s\" because conflicting module \"%s\" is already loaded", module->name, dep->name);
					return NULL;
				}
				crex_string_efree(lcname);
			}
			++dep;
		}
	}

	name_len = strlen(module->name);
	lcname = crex_string_alloc(name_len, module->type == MODULE_PERSISTENT);
	crex_str_tolower_copy(ZSTR_VAL(lcname), module->name, name_len);

	lcname = crex_new_interned_string(lcname);
	if ((module_ptr = crex_hash_add_ptr(&module_registry, lcname, module)) == NULL) {
		crex_error(E_CORE_WARNING, "Module \"%s\" is already loaded", module->name);
		crex_string_release(lcname);
		return NULL;
	}
	module = module_ptr;
	EG(current_module) = module;

	if (module->functions && crex_register_functions(NULL, module->functions, NULL, module->type)==FAILURE) {
		crex_hash_del(&module_registry, lcname);
		crex_string_release(lcname);
		EG(current_module) = NULL;
		crex_error(E_CORE_WARNING,"%s: Unable to register functions, unable to load", module->name);
		return NULL;
	}

	EG(current_module) = NULL;
	crex_string_release(lcname);
	return module;
}
/* }}} */

CREX_API crex_module_entry* crex_register_internal_module(crex_module_entry *module) /* {{{ */
{
	module->module_number = crex_next_free_module();
	module->type = MODULE_PERSISTENT;
	return crex_register_module_ex(module);
}
/* }}} */

static void crex_check_magic_method_args(
		uint32_t num_args, const crex_class_entry *ce, const crex_function *fptr, int error_type)
{
	if (fptr->common.num_args != num_args) {
		if (num_args == 0) {
			crex_error(error_type, "Method %s::%s() cannot take arguments",
				ZSTR_VAL(ce->name), ZSTR_VAL(fptr->common.function_name));
		} else if (num_args == 1) {
			crex_error(error_type, "Method %s::%s() must take exactly 1 argument",
				ZSTR_VAL(ce->name), ZSTR_VAL(fptr->common.function_name));
		} else {
			crex_error(error_type, "Method %s::%s() must take exactly %" PRIu32 " arguments",
				ZSTR_VAL(ce->name), ZSTR_VAL(fptr->common.function_name), num_args);
		}
		return;
	}
	for (uint32_t i = 0; i < num_args; i++) {
		if (QUICK_ARG_SHOULD_BE_SENT_BY_REF(fptr, i + 1)) {
			crex_error(error_type, "Method %s::%s() cannot take arguments by reference",
				ZSTR_VAL(ce->name), ZSTR_VAL(fptr->common.function_name));
			return;
		}
	}
}

static void crex_check_magic_method_arg_type(uint32_t arg_num, const crex_class_entry *ce, const crex_function *fptr, int error_type, int arg_type)
{
		if (
			CREX_TYPE_IS_SET(fptr->common.arg_info[arg_num].type)
			 && !(CREX_TYPE_FULL_MASK(fptr->common.arg_info[arg_num].type) & arg_type)
		) {
			crex_error(error_type, "%s::%s(): Parameter #%d ($%s) must be of type %s when declared",
				ZSTR_VAL(ce->name), ZSTR_VAL(fptr->common.function_name),
				arg_num + 1, ZSTR_VAL(fptr->common.arg_info[arg_num].name),
				ZSTR_VAL(crex_type_to_string((crex_type) CREX_TYPE_INIT_MASK(arg_type))));
		}
}

static void crex_check_magic_method_return_type(const crex_class_entry *ce, const crex_function *fptr, int error_type, int return_type)
{
	if (!(fptr->common.fn_flags & CREX_ACC_HAS_RETURN_TYPE)) {
		/* For backwards compatibility reasons, do not enforce the return type if it is not set. */
		return;
	}

	if (CREX_TYPE_PURE_MASK(fptr->common.arg_info[-1].type) & MAY_BE_NEVER) {
		/* It is always legal to specify the never type. */
		return;
	}

	bool is_complex_type = CREX_TYPE_IS_COMPLEX(fptr->common.arg_info[-1].type);
	uint32_t extra_types = CREX_TYPE_PURE_MASK(fptr->common.arg_info[-1].type) & ~return_type;
	if (extra_types & MAY_BE_STATIC) {
		extra_types &= ~MAY_BE_STATIC;
		is_complex_type = true;
	}

	if (extra_types || (is_complex_type && return_type != MAY_BE_OBJECT)) {
		crex_error(error_type, "%s::%s(): Return type must be %s when declared",
			ZSTR_VAL(ce->name), ZSTR_VAL(fptr->common.function_name),
			ZSTR_VAL(crex_type_to_string((crex_type) CREX_TYPE_INIT_MASK(return_type))));
	}
}

static void crex_check_magic_method_non_static(
		const crex_class_entry *ce, const crex_function *fptr, int error_type)
{
	if (fptr->common.fn_flags & CREX_ACC_STATIC) {
		crex_error(error_type, "Method %s::%s() cannot be static",
			ZSTR_VAL(ce->name), ZSTR_VAL(fptr->common.function_name));
	}
}

static void crex_check_magic_method_static(
		const crex_class_entry *ce, const crex_function *fptr, int error_type)
{
	if (!(fptr->common.fn_flags & CREX_ACC_STATIC)) {
		crex_error(error_type, "Method %s::%s() must be static",
			ZSTR_VAL(ce->name), ZSTR_VAL(fptr->common.function_name));
	}
}

static void crex_check_magic_method_public(
		const crex_class_entry *ce, const crex_function *fptr, int error_type)
{
	// TODO: Remove this warning after adding proper visibility handling.
	if (!(fptr->common.fn_flags & CREX_ACC_PUBLIC)) {
		crex_error(E_WARNING, "The magic method %s::%s() must have public visibility",
			ZSTR_VAL(ce->name), ZSTR_VAL(fptr->common.function_name));
	}
}

static void crex_check_magic_method_no_return_type(
		const crex_class_entry *ce, const crex_function *fptr, int error_type)
{
	if (fptr->common.fn_flags & CREX_ACC_HAS_RETURN_TYPE) {
		crex_error_noreturn(error_type, "Method %s::%s() cannot declare a return type",
			ZSTR_VAL(ce->name), ZSTR_VAL(fptr->common.function_name));
	}
}

CREX_API void crex_check_magic_method_implementation(const crex_class_entry *ce, const crex_function *fptr, crex_string *lcname, int error_type) /* {{{ */
{
	if (ZSTR_VAL(fptr->common.function_name)[0] != '_'
	 || ZSTR_VAL(fptr->common.function_name)[1] != '_') {
		return;
	}

	if (crex_string_equals_literal(lcname, CREX_CONSTRUCTOR_FUNC_NAME)) {
		crex_check_magic_method_non_static(ce, fptr, error_type);
		crex_check_magic_method_no_return_type(ce, fptr, error_type);
	} else if (crex_string_equals_literal(lcname, CREX_DESTRUCTOR_FUNC_NAME)) {
		crex_check_magic_method_args(0, ce, fptr, error_type);
		crex_check_magic_method_non_static(ce, fptr, error_type);
		crex_check_magic_method_no_return_type(ce, fptr, error_type);
	} else if (crex_string_equals_literal(lcname, CREX_CLONE_FUNC_NAME)) {
		crex_check_magic_method_args(0, ce, fptr, error_type);
		crex_check_magic_method_non_static(ce, fptr, error_type);
		crex_check_magic_method_return_type(ce, fptr, error_type, MAY_BE_VOID);
	} else if (crex_string_equals_literal(lcname, CREX_GET_FUNC_NAME)) {
		crex_check_magic_method_args(1, ce, fptr, error_type);
		crex_check_magic_method_non_static(ce, fptr, error_type);
		crex_check_magic_method_public(ce, fptr, error_type);
		crex_check_magic_method_arg_type(0, ce, fptr, error_type, MAY_BE_STRING);
	} else if (crex_string_equals_literal(lcname, CREX_SET_FUNC_NAME)) {
		crex_check_magic_method_args(2, ce, fptr, error_type);
		crex_check_magic_method_non_static(ce, fptr, error_type);
		crex_check_magic_method_public(ce, fptr, error_type);
		crex_check_magic_method_arg_type(0, ce, fptr, error_type, MAY_BE_STRING);
		crex_check_magic_method_return_type(ce, fptr, error_type, MAY_BE_VOID);
	} else if (crex_string_equals_literal(lcname, CREX_UNSET_FUNC_NAME)) {
		crex_check_magic_method_args(1, ce, fptr, error_type);
		crex_check_magic_method_non_static(ce, fptr, error_type);
		crex_check_magic_method_public(ce, fptr, error_type);
		crex_check_magic_method_arg_type(0, ce, fptr, error_type, MAY_BE_STRING);
		crex_check_magic_method_return_type(ce, fptr, error_type, MAY_BE_VOID);
	} else if (crex_string_equals_literal(lcname, CREX_ISSET_FUNC_NAME)) {
		crex_check_magic_method_args(1, ce, fptr, error_type);
		crex_check_magic_method_non_static(ce, fptr, error_type);
		crex_check_magic_method_public(ce, fptr, error_type);
		crex_check_magic_method_arg_type(0, ce, fptr, error_type, MAY_BE_STRING);
		crex_check_magic_method_return_type(ce, fptr, error_type, MAY_BE_BOOL);
	} else if (crex_string_equals_literal(lcname, CREX_CALL_FUNC_NAME)) {
		crex_check_magic_method_args(2, ce, fptr, error_type);
		crex_check_magic_method_non_static(ce, fptr, error_type);
		crex_check_magic_method_public(ce, fptr, error_type);
		crex_check_magic_method_arg_type(0, ce, fptr, error_type, MAY_BE_STRING);
		crex_check_magic_method_arg_type(1, ce, fptr, error_type, MAY_BE_ARRAY);
	} else if (crex_string_equals_literal(lcname, CREX_CALLSTATIC_FUNC_NAME)) {
		crex_check_magic_method_args(2, ce, fptr, error_type);
		crex_check_magic_method_static(ce, fptr, error_type);
		crex_check_magic_method_public(ce, fptr, error_type);
		crex_check_magic_method_arg_type(0, ce, fptr, error_type, MAY_BE_STRING);
		crex_check_magic_method_arg_type(1, ce, fptr, error_type, MAY_BE_ARRAY);
	} else if (crex_string_equals_literal(lcname, CREX_TOSTRING_FUNC_NAME)) {
		crex_check_magic_method_args(0, ce, fptr, error_type);
		crex_check_magic_method_non_static(ce, fptr, error_type);
		crex_check_magic_method_public(ce, fptr, error_type);
		crex_check_magic_method_return_type(ce, fptr, error_type, MAY_BE_STRING);
	} else if (crex_string_equals_literal(lcname, CREX_DEBUGINFO_FUNC_NAME)) {
		crex_check_magic_method_args(0, ce, fptr, error_type);
		crex_check_magic_method_non_static(ce, fptr, error_type);
		crex_check_magic_method_public(ce, fptr, error_type);
		crex_check_magic_method_return_type(ce, fptr, error_type, (MAY_BE_ARRAY | MAY_BE_NULL));
	} else if (crex_string_equals_literal(lcname, "__serialize")) {
		crex_check_magic_method_args(0, ce, fptr, error_type);
		crex_check_magic_method_non_static(ce, fptr, error_type);
		crex_check_magic_method_public(ce, fptr, error_type);
		crex_check_magic_method_return_type(ce, fptr, error_type, MAY_BE_ARRAY);
	} else if (crex_string_equals_literal(lcname, "__unserialize")) {
		crex_check_magic_method_args(1, ce, fptr, error_type);
		crex_check_magic_method_non_static(ce, fptr, error_type);
		crex_check_magic_method_public(ce, fptr, error_type);
		crex_check_magic_method_arg_type(0, ce, fptr, error_type, MAY_BE_ARRAY);
		crex_check_magic_method_return_type(ce, fptr, error_type, MAY_BE_VOID);
	} else if (crex_string_equals_literal(lcname, "__set_state")) {
		crex_check_magic_method_args(1, ce, fptr, error_type);
		crex_check_magic_method_static(ce, fptr, error_type);
		crex_check_magic_method_public(ce, fptr, error_type);
		crex_check_magic_method_arg_type(0, ce, fptr, error_type, MAY_BE_ARRAY);
		crex_check_magic_method_return_type(ce, fptr, error_type, MAY_BE_OBJECT);
	} else if (crex_string_equals(lcname, ZSTR_KNOWN(CREX_STR_MAGIC_INVOKE))) {
		crex_check_magic_method_non_static(ce, fptr, error_type);
		crex_check_magic_method_public(ce, fptr, error_type);
	} else if (crex_string_equals(lcname, ZSTR_KNOWN(CREX_STR_SLEEP))) {
		crex_check_magic_method_args(0, ce, fptr, error_type);
		crex_check_magic_method_non_static(ce, fptr, error_type);
		crex_check_magic_method_public(ce, fptr, error_type);
		crex_check_magic_method_return_type(ce, fptr, error_type, MAY_BE_ARRAY);
	} else if (crex_string_equals(lcname, ZSTR_KNOWN(CREX_STR_WAKEUP))) {
		crex_check_magic_method_args(0, ce, fptr, error_type);
		crex_check_magic_method_non_static(ce, fptr, error_type);
		crex_check_magic_method_public(ce, fptr, error_type);
		crex_check_magic_method_return_type(ce, fptr, error_type, MAY_BE_VOID);
	}
}
/* }}} */

CREX_API void crex_add_magic_method(crex_class_entry *ce, crex_function *fptr, crex_string *lcname)
{
	if (ZSTR_VAL(lcname)[0] != '_' || ZSTR_VAL(lcname)[1] != '_') {
		/* pass */
	} else if (crex_string_equals_literal(lcname, CREX_CLONE_FUNC_NAME)) {
		ce->clone = fptr;
	} else if (crex_string_equals_literal(lcname, CREX_CONSTRUCTOR_FUNC_NAME)) {
		ce->constructor = fptr;
		ce->constructor->common.fn_flags |= CREX_ACC_CTOR;
	} else if (crex_string_equals_literal(lcname, CREX_DESTRUCTOR_FUNC_NAME)) {
		ce->destructor = fptr;
	} else if (crex_string_equals_literal(lcname, CREX_GET_FUNC_NAME)) {
		ce->__get = fptr;
		ce->ce_flags |= CREX_ACC_USE_GUARDS;
	} else if (crex_string_equals_literal(lcname, CREX_SET_FUNC_NAME)) {
		ce->__set = fptr;
		ce->ce_flags |= CREX_ACC_USE_GUARDS;
	} else if (crex_string_equals_literal(lcname, CREX_CALL_FUNC_NAME)) {
		ce->__call = fptr;
	} else if (crex_string_equals_literal(lcname, CREX_UNSET_FUNC_NAME)) {
		ce->__unset = fptr;
		ce->ce_flags |= CREX_ACC_USE_GUARDS;
	} else if (crex_string_equals_literal(lcname, CREX_ISSET_FUNC_NAME)) {
		ce->__isset = fptr;
		ce->ce_flags |= CREX_ACC_USE_GUARDS;
	} else if (crex_string_equals_literal(lcname, CREX_CALLSTATIC_FUNC_NAME)) {
		ce->__callstatic = fptr;
	} else if (crex_string_equals_literal(lcname, CREX_TOSTRING_FUNC_NAME)) {
		ce->__tostring = fptr;
	} else if (crex_string_equals_literal(lcname, CREX_DEBUGINFO_FUNC_NAME)) {
		ce->__debugInfo = fptr;
		ce->ce_flags |= CREX_ACC_USE_GUARDS;
	} else if (crex_string_equals_literal(lcname, "__serialize")) {
		ce->__serialize = fptr;
	} else if (crex_string_equals_literal(lcname, "__unserialize")) {
		ce->__unserialize = fptr;
	}
}

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arg_info_toString, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

static crex_always_inline void crex_normalize_internal_type(crex_type *type) {
	CREX_ASSERT(!CREX_TYPE_HAS_LITERAL_NAME(*type));
	crex_type *current;
	CREX_TYPE_FOREACH(*type, current) {
		if (CREX_TYPE_HAS_NAME(*current)) {
			crex_string *name = crex_new_interned_string(CREX_TYPE_NAME(*current));
			crex_alloc_ce_cache(name);
			CREX_TYPE_SET_PTR(*current, name);
		} else if (CREX_TYPE_HAS_LIST(*current)) {
			crex_type *inner;
			CREX_TYPE_FOREACH(*current, inner) {
				CREX_ASSERT(!CREX_TYPE_HAS_LITERAL_NAME(*inner) && !CREX_TYPE_HAS_LIST(*inner));
				if (CREX_TYPE_HAS_NAME(*inner)) {
					crex_string *name = crex_new_interned_string(CREX_TYPE_NAME(*inner));
					crex_alloc_ce_cache(name);
					CREX_TYPE_SET_PTR(*inner, name);
				}
			} CREX_TYPE_FOREACH_END();
		}
	} CREX_TYPE_FOREACH_END();
}

/* registers all functions in *library_functions in the function hash */
CREX_API crex_result crex_register_functions(crex_class_entry *scope, const crex_function_entry *functions, HashTable *function_table, int type) /* {{{ */
{
	const crex_function_entry *ptr = functions;
	crex_function function;
	crex_internal_function *reg_function, *internal_function = (crex_internal_function *)&function;
	int count=0, unload=0;
	HashTable *target_function_table = function_table;
	int error_type;
	crex_string *lowercase_name;
	size_t fname_len;

	if (type==MODULE_PERSISTENT) {
		error_type = E_CORE_WARNING;
	} else {
		error_type = E_WARNING;
	}

	if (!target_function_table) {
		target_function_table = CG(function_table);
	}
	internal_function->type = CREX_INTERNAL_FUNCTION;
	internal_function->module = EG(current_module);
	internal_function->T = 0;
	memset(internal_function->reserved, 0, CREX_MAX_RESERVED_RESOURCES * sizeof(void*));

	while (ptr->fname) {
		fname_len = strlen(ptr->fname);
		internal_function->handler = ptr->handler;
		internal_function->function_name = crex_string_init_interned(ptr->fname, fname_len, 1);
		internal_function->scope = scope;
		internal_function->prototype = NULL;
		internal_function->attributes = NULL;
		if (EG(active)) { // at run-time: this ought to only happen if registered with dl() or somehow temporarily at runtime
			CREX_MAP_PTR_INIT(internal_function->run_time_cache, crex_arena_calloc(&CG(arena), 1, crex_internal_run_time_cache_reserved_size()));
		} else {
			CREX_MAP_PTR_NEW(internal_function->run_time_cache);
		}
		if (ptr->flags) {
			if (!(ptr->flags & CREX_ACC_PPP_MASK)) {
				if (ptr->flags != CREX_ACC_DEPRECATED && scope) {
					crex_error(error_type, "Invalid access level for %s::%s() - access must be exactly one of public, protected or private", ZSTR_VAL(scope->name), ptr->fname);
				}
				internal_function->fn_flags = CREX_ACC_PUBLIC | ptr->flags;
			} else {
				internal_function->fn_flags = ptr->flags;
			}
		} else {
			internal_function->fn_flags = CREX_ACC_PUBLIC;
		}

		if (ptr->arg_info) {
			crex_internal_function_info *info = (crex_internal_function_info*)ptr->arg_info;
			internal_function->arg_info = (crex_internal_arg_info*)ptr->arg_info+1;
			internal_function->num_args = ptr->num_args;
			/* Currently you cannot denote that the function can accept less arguments than num_args */
			if (info->required_num_args == (uintptr_t)-1) {
				internal_function->required_num_args = ptr->num_args;
			} else {
				internal_function->required_num_args = info->required_num_args;
			}
			if (CREX_ARG_SEND_MODE(info)) {
				internal_function->fn_flags |= CREX_ACC_RETURN_REFERENCE;
			}
			if (CREX_ARG_IS_VARIADIC(&ptr->arg_info[ptr->num_args])) {
				internal_function->fn_flags |= CREX_ACC_VARIADIC;
				/* Don't count the variadic argument */
				internal_function->num_args--;
			}
			if (CREX_TYPE_IS_SET(info->type)) {
				if (CREX_TYPE_HAS_NAME(info->type)) {
					const char *type_name = CREX_TYPE_LITERAL_NAME(info->type);
					if (!scope && (!strcasecmp(type_name, "self") || !strcasecmp(type_name, "parent"))) {
						crex_error_noreturn(E_CORE_ERROR, "Cannot declare a return type of %s outside of a class scope", type_name);
					}
				}

				internal_function->fn_flags |= CREX_ACC_HAS_RETURN_TYPE;
			}
		} else {
			crex_error(E_CORE_WARNING, "Missing arginfo for %s%s%s()",
				 scope ? ZSTR_VAL(scope->name) : "", scope ? "::" : "", ptr->fname);

			internal_function->arg_info = NULL;
			internal_function->num_args = 0;
			internal_function->required_num_args = 0;
		}

		/* If not specified, add __toString() return type for compatibility with Stringable
		 * interface. */
		if (scope && crex_string_equals_literal_ci(internal_function->function_name, "__tostring") &&
				!(internal_function->fn_flags & CREX_ACC_HAS_RETURN_TYPE)) {
			crex_error(E_CORE_WARNING, "%s::__toString() implemented without string return type",
				ZSTR_VAL(scope->name));
			internal_function->arg_info = (crex_internal_arg_info *) arg_info_toString + 1;
			internal_function->fn_flags |= CREX_ACC_HAS_RETURN_TYPE;
			internal_function->num_args = internal_function->required_num_args = 0;
		}


		crex_set_function_arg_flags((crex_function*)internal_function);
		if (ptr->flags & CREX_ACC_ABSTRACT) {
			if (scope) {
				/* This is a class that must be abstract itself. Here we set the check info. */
				scope->ce_flags |= CREX_ACC_IMPLICIT_ABSTRACT_CLASS;
				if (!(scope->ce_flags & CREX_ACC_INTERFACE)) {
					/* Since the class is not an interface it needs to be declared as a abstract class. */
					/* Since here we are handling internal functions only we can add the keyword flag. */
					/* This time we set the flag for the keyword 'abstract'. */
					scope->ce_flags |= CREX_ACC_EXPLICIT_ABSTRACT_CLASS;
				}
			}
			if ((ptr->flags & CREX_ACC_STATIC) && (!scope || !(scope->ce_flags & CREX_ACC_INTERFACE))) {
				crex_error(error_type, "Static function %s%s%s() cannot be abstract", scope ? ZSTR_VAL(scope->name) : "", scope ? "::" : "", ptr->fname);
			}
		} else {
			if (scope && (scope->ce_flags & CREX_ACC_INTERFACE)) {
				crex_error(error_type, "Interface %s cannot contain non abstract method %s()", ZSTR_VAL(scope->name), ptr->fname);
				return FAILURE;
			}
			if (!internal_function->handler) {
				crex_error(error_type, "Method %s%s%s() cannot be a NULL function", scope ? ZSTR_VAL(scope->name) : "", scope ? "::" : "", ptr->fname);
				crex_unregister_functions(functions, count, target_function_table);
				return FAILURE;
			}
		}
		lowercase_name = crex_string_tolower_ex(internal_function->function_name, type == MODULE_PERSISTENT);
		lowercase_name = crex_new_interned_string(lowercase_name);
		reg_function = malloc(sizeof(crex_internal_function));
		memcpy(reg_function, &function, sizeof(crex_internal_function));
		if (crex_hash_add_ptr(target_function_table, lowercase_name, reg_function) == NULL) {
			unload=1;
			free(reg_function);
			crex_string_release(lowercase_name);
			break;
		}

		/* Get parameter count including variadic parameter. */
		uint32_t num_args = reg_function->num_args;
		if (reg_function->fn_flags & CREX_ACC_VARIADIC) {
			num_args++;
		}

		/* If types of arguments have to be checked */
		if (reg_function->arg_info && num_args) {
			uint32_t i;
			for (i = 0; i < num_args; i++) {
				crex_internal_arg_info *arg_info = &reg_function->arg_info[i];
				CREX_ASSERT(arg_info->name && "Parameter must have a name");
				if (CREX_TYPE_IS_SET(arg_info->type)) {
				    reg_function->fn_flags |= CREX_ACC_HAS_TYPE_HINTS;
				}
#if CREX_DEBUG
				for (uint32_t j = 0; j < i; j++) {
					if (!strcmp(arg_info->name, reg_function->arg_info[j].name)) {
						crex_error_noreturn(E_CORE_ERROR,
							"Duplicate parameter name $%s for function %s%s%s()", arg_info->name,
							scope ? ZSTR_VAL(scope->name) : "", scope ? "::" : "", ptr->fname);
					}
				}
#endif
			}
		}

		/* Rebuild arginfos if parameter/property types and/or a return type are used */
		if (reg_function->arg_info &&
		    (reg_function->fn_flags & (CREX_ACC_HAS_RETURN_TYPE|CREX_ACC_HAS_TYPE_HINTS))) {
			/* convert "const char*" class type names into "crex_string*" */
			uint32_t i;
			crex_internal_arg_info *arg_info = reg_function->arg_info - 1;
			crex_internal_arg_info *new_arg_info;

			/* Treat return type as an extra argument */
			num_args++;
			new_arg_info = malloc(sizeof(crex_internal_arg_info) * num_args);
			memcpy(new_arg_info, arg_info, sizeof(crex_internal_arg_info) * num_args);
			reg_function->arg_info = new_arg_info + 1;
			for (i = 0; i < num_args; i++) {
				if (CREX_TYPE_HAS_LITERAL_NAME(new_arg_info[i].type)) {
					// gen_stubs.crx does not support codegen for DNF types in arg infos.
					// As a temporary workaround, we split the type name on `|` characters,
					// converting it to an union type if necessary.
					const char *class_name = CREX_TYPE_LITERAL_NAME(new_arg_info[i].type);
					new_arg_info[i].type.type_mask &= ~_CREX_TYPE_LITERAL_NAME_BIT;

					size_t num_types = 1;
					const char *p = class_name;
					while ((p = strchr(p, '|'))) {
						num_types++;
						p++;
					}

					if (num_types == 1) {
						/* Simple class type */
						crex_string *str = crex_string_init_interned(class_name, strlen(class_name), 1);
						crex_alloc_ce_cache(str);
						CREX_TYPE_SET_PTR(new_arg_info[i].type, str);
						new_arg_info[i].type.type_mask |= _CREX_TYPE_NAME_BIT;
					} else {
						/* Union type */
						crex_type_list *list = malloc(CREX_TYPE_LIST_SIZE(num_types));
						list->num_types = num_types;
						CREX_TYPE_SET_LIST(new_arg_info[i].type, list);
						CREX_TYPE_FULL_MASK(new_arg_info[i].type) |= _CREX_TYPE_UNION_BIT;

						const char *start = class_name;
						uint32_t j = 0;
						while (true) {
							const char *end = strchr(start, '|');
							crex_string *str = crex_string_init_interned(start, end ? end - start : strlen(start), 1);
							crex_alloc_ce_cache(str);
							list->types[j] = (crex_type) CREX_TYPE_INIT_CLASS(str, 0, 0);
							if (!end) {
								break;
							}
							start = end + 1;
							j++;
						}
					}
				}
				if (CREX_TYPE_IS_ITERABLE_FALLBACK(new_arg_info[i].type)) {
					/* Warning generated an extension load warning which is emitted for every test
					crex_error(E_CORE_WARNING, "iterable type is now a compile time alias for array|Traversable,"
						" regenerate the argument info via the crx-src gen_stub build script");
					*/
					crex_type legacy_iterable = CREX_TYPE_INIT_CLASS_MASK(
						ZSTR_KNOWN(CREX_STR_TRAVERSABLE),
						(new_arg_info[i].type.type_mask | MAY_BE_ARRAY)
					);
					new_arg_info[i].type = legacy_iterable;
				}

				crex_normalize_internal_type(&new_arg_info[i].type);
			}
		}

		if (scope) {
			crex_check_magic_method_implementation(
				scope, (crex_function *)reg_function, lowercase_name, E_CORE_ERROR);
			crex_add_magic_method(scope, (crex_function *)reg_function, lowercase_name);
		}
		ptr++;
		count++;
		crex_string_release(lowercase_name);
	}
	if (unload) { /* before unloading, display all remaining bad function in the module */
		while (ptr->fname) {
			fname_len = strlen(ptr->fname);
			lowercase_name = crex_string_alloc(fname_len, 0);
			crex_str_tolower_copy(ZSTR_VAL(lowercase_name), ptr->fname, fname_len);
			if (crex_hash_exists(target_function_table, lowercase_name)) {
				crex_error(error_type, "Function registration failed - duplicate name - %s%s%s", scope ? ZSTR_VAL(scope->name) : "", scope ? "::" : "", ptr->fname);
			}
			crex_string_efree(lowercase_name);
			ptr++;
		}
		crex_unregister_functions(functions, count, target_function_table);
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

/* count=-1 means erase all functions, otherwise,
 * erase the first count functions
 */
CREX_API void crex_unregister_functions(const crex_function_entry *functions, int count, HashTable *function_table) /* {{{ */
{
	const crex_function_entry *ptr = functions;
	int i=0;
	HashTable *target_function_table = function_table;
	crex_string *lowercase_name;
	size_t fname_len;

	if (!target_function_table) {
		target_function_table = CG(function_table);
	}
	while (ptr->fname) {
		if (count!=-1 && i>=count) {
			break;
		}
		fname_len = strlen(ptr->fname);
		lowercase_name = crex_string_alloc(fname_len, 0);
		crex_str_tolower_copy(ZSTR_VAL(lowercase_name), ptr->fname, fname_len);
		crex_hash_del(target_function_table, lowercase_name);
		crex_string_efree(lowercase_name);
		ptr++;
		i++;
	}
}
/* }}} */

CREX_API crex_result crex_startup_module(crex_module_entry *module) /* {{{ */
{
	if ((module = crex_register_internal_module(module)) != NULL && crex_startup_module_ex(module) == SUCCESS) {
		return SUCCESS;
	}
	return FAILURE;
}
/* }}} */

CREX_API crex_result crex_get_module_started(const char *module_name) /* {{{ */
{
	crex_module_entry *module;

	module = crex_hash_str_find_ptr(&module_registry, module_name, strlen(module_name));
	return (module && module->module_started) ? SUCCESS : FAILURE;
}
/* }}} */

static int clean_module_class(zval *el, void *arg) /* {{{ */
{
	crex_class_entry *ce = (crex_class_entry *)C_PTR_P(el);
	int module_number = *(int *)arg;
	if (ce->type == CREX_INTERNAL_CLASS && ce->info.internal.module->module_number == module_number) {
		return CREX_HASH_APPLY_REMOVE;
	} else {
		return CREX_HASH_APPLY_KEEP;
	}
}
/* }}} */

static void clean_module_classes(int module_number) /* {{{ */
{
	crex_hash_apply_with_argument(EG(class_table), clean_module_class, (void *) &module_number);
}
/* }}} */

static int clean_module_function(zval *el, void *arg) /* {{{ */
{
	crex_function *fe = (crex_function *) C_PTR_P(el);
	crex_module_entry *module = (crex_module_entry *) arg;
	if (fe->common.type == CREX_INTERNAL_FUNCTION && fe->internal_function.module == module) {
		return CREX_HASH_APPLY_REMOVE;
	} else {
		return CREX_HASH_APPLY_KEEP;
	}
}
/* }}} */

static void clean_module_functions(crex_module_entry *module) /* {{{ */
{
	crex_hash_apply_with_argument(CG(function_table), clean_module_function, module);
}
/* }}} */

void module_destructor(crex_module_entry *module) /* {{{ */
{
#if CREX_RC_DEBUG
	bool orig_rc_debug = crex_rc_debug;
#endif

	if (module->type == MODULE_TEMPORARY) {
#if CREX_RC_DEBUG
		/* FIXME: Loading extensions during the request breaks some invariants.
		 * In particular, it will create persistent interned strings, which is
		 * not allowed at this stage. */
		crex_rc_debug = false;
#endif
		crex_clean_module_rsrc_dtors(module->module_number);
		clean_module_constants(module->module_number);
		clean_module_classes(module->module_number);
	}

	if (module->module_started && module->module_shutdown_func) {
#if 0
		crex_printf("%s: Module shutdown\n", module->name);
#endif
		module->module_shutdown_func(module->type, module->module_number);
	}

	if (module->module_started
	 && !module->module_shutdown_func
	 && module->type == MODULE_TEMPORARY) {
		crex_unregister_ini_entries_ex(module->module_number, module->type);
	}

	/* Deinitialize module globals */
	if (module->globals_size) {
#ifdef ZTS
		if (*module->globals_id_ptr) {
			ts_free_id(*module->globals_id_ptr);
		}
#else
		if (module->globals_dtor) {
			module->globals_dtor(module->globals_ptr);
		}
#endif
	}

	module->module_started=0;
	if (module->type == MODULE_TEMPORARY && module->functions) {
		crex_unregister_functions(module->functions, -1, NULL);
		/* Clean functions registered separately from module->functions */
		clean_module_functions(module);
	}

#if HAVE_LIBDL
	if (module->handle && !getenv("CREX_DONT_UNLOAD_MODULES")) {
		DL_UNLOAD(module->handle);
	}
#endif

#if CREX_RC_DEBUG
	crex_rc_debug = orig_rc_debug;
#endif
}
/* }}} */

CREX_API void crex_activate_modules(void) /* {{{ */
{
	crex_module_entry **p = module_request_startup_handlers;

	while (*p) {
		crex_module_entry *module = *p;

		if (module->request_startup_func(module->type, module->module_number)==FAILURE) {
			crex_error(E_WARNING, "request_startup() for %s module failed", module->name);
			exit(1);
		}
		p++;
	}
}
/* }}} */

CREX_API void crex_deactivate_modules(void) /* {{{ */
{
	EG(current_execute_data) = NULL; /* we're no longer executing anything */

	if (EG(full_tables_cleanup)) {
		crex_module_entry *module;

		CREX_HASH_MAP_REVERSE_FOREACH_PTR(&module_registry, module) {
			if (module->request_shutdown_func) {
				crex_try {
					module->request_shutdown_func(module->type, module->module_number);
				} crex_end_try();
			}
		} CREX_HASH_FOREACH_END();
	} else {
		crex_module_entry **p = module_request_shutdown_handlers;

		while (*p) {
			crex_module_entry *module = *p;
			crex_try {
				module->request_shutdown_func(module->type, module->module_number);
			} crex_end_try();
			p++;
		}
	}
}
/* }}} */

CREX_API void crex_post_deactivate_modules(void) /* {{{ */
{
	if (EG(full_tables_cleanup)) {
		crex_module_entry *module;
		zval *zv;
		crex_string *key;

		CREX_HASH_MAP_FOREACH_PTR(&module_registry, module) {
			if (module->post_deactivate_func) {
				module->post_deactivate_func();
			}
		} CREX_HASH_FOREACH_END();
		CREX_HASH_MAP_REVERSE_FOREACH_STR_KEY_VAL(&module_registry, key, zv) {
			module = C_PTR_P(zv);
			if (module->type != MODULE_TEMPORARY) {
				break;
			}
			module_destructor(module);
			crex_string_release_ex(key, 0);
		} CREX_HASH_MAP_FOREACH_END_DEL();
	} else {
		crex_module_entry **p = module_post_deactivate_handlers;

		while (*p) {
			crex_module_entry *module = *p;

			module->post_deactivate_func();
			p++;
		}
	}
}
/* }}} */

/* return the next free module number */
CREX_API int crex_next_free_module(void) /* {{{ */
{
	return crex_hash_num_elements(&module_registry) + 1;
}
/* }}} */

static crex_class_entry *do_register_internal_class(crex_class_entry *orig_class_entry, uint32_t ce_flags) /* {{{ */
{
	crex_class_entry *class_entry = malloc(sizeof(crex_class_entry));
	crex_string *lowercase_name;
	*class_entry = *orig_class_entry;

	class_entry->type = CREX_INTERNAL_CLASS;
	crex_initialize_class_data(class_entry, 0);
	crex_alloc_ce_cache(class_entry->name);
	class_entry->ce_flags = orig_class_entry->ce_flags | ce_flags | CREX_ACC_CONSTANTS_UPDATED | CREX_ACC_LINKED | CREX_ACC_RESOLVED_PARENT | CREX_ACC_RESOLVED_INTERFACES;
	class_entry->info.internal.module = EG(current_module);

	if (class_entry->info.internal.builtin_functions) {
		crex_register_functions(class_entry, class_entry->info.internal.builtin_functions, &class_entry->function_table, EG(current_module)->type);
	}

	lowercase_name = crex_string_tolower_ex(orig_class_entry->name, EG(current_module)->type == MODULE_PERSISTENT);
	lowercase_name = crex_new_interned_string(lowercase_name);
	crex_hash_update_ptr(CG(class_table), lowercase_name, class_entry);
	crex_string_release_ex(lowercase_name, 1);

	if (class_entry->__tostring && !crex_string_equals_literal(class_entry->name, "Stringable")
			&& !(class_entry->ce_flags & CREX_ACC_TRAIT)) {
		CREX_ASSERT(crex_ce_stringable
			&& "Should be registered before first class using __toString()");
		crex_do_implement_interface(class_entry, crex_ce_stringable);
	}
	return class_entry;
}
/* }}} */

/* If parent_ce is not NULL then it inherits from parent_ce
 * If parent_ce is NULL and parent_name isn't then it looks for the parent and inherits from it
 * If both parent_ce and parent_name are NULL it does a regular class registration
 * If parent_name is specified but not found NULL is returned
 */
CREX_API crex_class_entry *crex_register_internal_class_ex(crex_class_entry *class_entry, crex_class_entry *parent_ce) /* {{{ */
{
	crex_class_entry *register_class;

	register_class = crex_register_internal_class(class_entry);

	if (parent_ce) {
		crex_do_inheritance(register_class, parent_ce);
		crex_build_properties_info_table(register_class);
	}

	return register_class;
}
/* }}} */

CREX_API void crex_class_implements(crex_class_entry *class_entry, int num_interfaces, ...) /* {{{ */
{
	crex_class_entry *interface_entry;
	va_list interface_list;
	va_start(interface_list, num_interfaces);

	while (num_interfaces--) {
		interface_entry = va_arg(interface_list, crex_class_entry *);
		if (interface_entry == crex_ce_stringable
				&& crex_class_implements_interface(class_entry, crex_ce_stringable)) {
			/* Stringable is implemented automatically,
			 * silently ignore an explicit implementation. */
			continue;
		}

		crex_do_implement_interface(class_entry, interface_entry);
	}

	va_end(interface_list);
}
/* }}} */

/* A class that contains at least one abstract method automatically becomes an abstract class.
 */
CREX_API crex_class_entry *crex_register_internal_class(crex_class_entry *orig_class_entry) /* {{{ */
{
	return do_register_internal_class(orig_class_entry, 0);
}
/* }}} */

CREX_API crex_class_entry *crex_register_internal_interface(crex_class_entry *orig_class_entry) /* {{{ */
{
	return do_register_internal_class(orig_class_entry, CREX_ACC_INTERFACE);
}
/* }}} */

CREX_API crex_result crex_register_class_alias_ex(const char *name, size_t name_len, crex_class_entry *ce, bool persistent) /* {{{ */
{
	crex_string *lcname;
	zval zv, *ret;

	/* TODO: Move this out of here in 7.4. */
	if (persistent && EG(current_module) && EG(current_module)->type == MODULE_TEMPORARY) {
		persistent = 0;
	}

	if (name[0] == '\\') {
		lcname = crex_string_alloc(name_len-1, persistent);
		crex_str_tolower_copy(ZSTR_VAL(lcname), name+1, name_len-1);
	} else {
		lcname = crex_string_alloc(name_len, persistent);
		crex_str_tolower_copy(ZSTR_VAL(lcname), name, name_len);
	}

	crex_assert_valid_class_name(lcname);

	lcname = crex_new_interned_string(lcname);

	/* We cannot increase the refcount of an internal class during request time.
	 * Instead of having to deal with differentiating between class types and lifetimes,
	 * we simply don't increase the refcount of a class entry for aliases.
	 */
	ZVAL_ALIAS_PTR(&zv, ce);

	ret = crex_hash_add(CG(class_table), lcname, &zv);
	crex_string_release_ex(lcname, 0);
	if (ret) {
		// avoid notifying at MINIT time
		if (ce->type == CREX_USER_CLASS) {
			crex_observer_class_linked_notify(ce, lcname);
		}
		return SUCCESS;
	}
	return FAILURE;
}
/* }}} */

// TODO num_symbol_tables as unsigned int?
CREX_API crex_result crex_set_hash_symbol(zval *symbol, const char *name, size_t name_length, bool is_ref, int num_symbol_tables, ...) /* {{{ */
{
	HashTable *symbol_table;
	va_list symbol_table_list;

	if (num_symbol_tables <= 0) return FAILURE;

	if (is_ref) {
		ZVAL_MAKE_REF(symbol);
	}

	va_start(symbol_table_list, num_symbol_tables);
	while (num_symbol_tables-- > 0) {
		symbol_table = va_arg(symbol_table_list, HashTable *);
		crex_hash_str_update(symbol_table, name, name_length, symbol);
		C_TRY_ADDREF_P(symbol);
	}
	va_end(symbol_table_list);
	return SUCCESS;
}
/* }}} */

/* Disabled functions support */

static void crex_disable_function(const char *function_name, size_t function_name_length)
{
	crex_hash_str_del(CG(function_table), function_name, function_name_length);
}

CREX_API void crex_disable_functions(const char *function_list) /* {{{ */
{
	if (!function_list || !*function_list) {
		return;
	}

	const char *s = NULL, *e = function_list;
	while (*e) {
		switch (*e) {
			case ' ':
			case ',':
				if (s) {
					crex_disable_function(s, e - s);
					s = NULL;
				}
				break;
			default:
				if (!s) {
					s = e;
				}
				break;
		}
		e++;
	}
	if (s) {
		crex_disable_function(s, e - s);
	}

	/* Rehash the function table after deleting functions. This ensures that all internal
	 * functions are contiguous, which means we don't need to perform full table cleanup
	 * on shutdown. */
	crex_hash_rehash(CG(function_table));
}
/* }}} */

#ifdef CREX_WIN32
#pragma optimize("", off)
#endif
static CREX_COLD crex_object *display_disabled_class(crex_class_entry *class_type) /* {{{ */
{
	crex_object *intern;

	intern = crex_objects_new(class_type);

	/* Initialize default properties */
	if (EXPECTED(class_type->default_properties_count != 0)) {
		zval *p = intern->properties_table;
		zval *end = p + class_type->default_properties_count;
		do {
			ZVAL_UNDEF(p);
			p++;
		} while (p != end);
	}

	crex_error(E_WARNING, "%s() has been disabled for security reasons", ZSTR_VAL(class_type->name));
	return intern;
}
#ifdef CREX_WIN32
#pragma optimize("", on)
#endif
/* }}} */

static const crex_function_entry disabled_class_new[] = {
	CREX_FE_END
};

CREX_API crex_result crex_disable_class(const char *class_name, size_t class_name_length) /* {{{ */
{
	crex_class_entry *disabled_class;
	crex_string *key;
	crex_function *fn;
	crex_property_info *prop;

	key = crex_string_alloc(class_name_length, 0);
	crex_str_tolower_copy(ZSTR_VAL(key), class_name, class_name_length);
	disabled_class = crex_hash_find_ptr(CG(class_table), key);
	crex_string_release_ex(key, 0);
	if (!disabled_class) {
		return FAILURE;
	}

	/* Will be reset by INIT_CLASS_ENTRY. */
	free(disabled_class->interfaces);

	INIT_CLASS_ENTRY_INIT_METHODS((*disabled_class), disabled_class_new);
	disabled_class->create_object = display_disabled_class;

	CREX_HASH_MAP_FOREACH_PTR(&disabled_class->function_table, fn) {
		if ((fn->common.fn_flags & (CREX_ACC_HAS_RETURN_TYPE|CREX_ACC_HAS_TYPE_HINTS)) &&
			fn->common.scope == disabled_class) {
			crex_free_internal_arg_info(&fn->internal_function);
		}
	} CREX_HASH_FOREACH_END();
	crex_hash_clean(&disabled_class->function_table);
	CREX_HASH_MAP_FOREACH_PTR(&disabled_class->properties_info, prop) {
		if (prop->ce == disabled_class) {
			crex_string_release(prop->name);
			crex_type_release(prop->type, /* persistent */ 1);
			free(prop);
		}
	} CREX_HASH_FOREACH_END();
	crex_hash_clean(&disabled_class->properties_info);
	return SUCCESS;
}
/* }}} */

static crex_always_inline crex_class_entry *get_scope(crex_execute_data *frame)
{
	return frame && frame->func ? frame->func->common.scope : NULL;
}

static bool crex_is_callable_check_class(crex_string *name, crex_class_entry *scope, crex_execute_data *frame, crex_fcall_info_cache *fcc, bool *strict_class, char **error, bool suppress_deprecation) /* {{{ */
{
	bool ret = 0;
	crex_class_entry *ce;
	size_t name_len = ZSTR_LEN(name);
	crex_string *lcname;
	ALLOCA_FLAG(use_heap);

	ZSTR_ALLOCA_ALLOC(lcname, name_len, use_heap);
	crex_str_tolower_copy(ZSTR_VAL(lcname), ZSTR_VAL(name), name_len);

	*strict_class = 0;
	if (crex_string_equals_literal(lcname, "self")) {
		if (!scope) {
			if (error) *error = estrdup("cannot access \"self\" when no class scope is active");
		} else {
			if (!suppress_deprecation) {
				crex_error(E_DEPRECATED, "Use of \"self\" in callables is deprecated");
			}
			fcc->called_scope = crex_get_called_scope(frame);
			if (!fcc->called_scope || !instanceof_function(fcc->called_scope, scope)) {
				fcc->called_scope = scope;
			}
			fcc->calling_scope = scope;
			if (!fcc->object) {
				fcc->object = crex_get_this_object(frame);
			}
			ret = 1;
		}
	} else if (crex_string_equals_literal(lcname, "parent")) {
		if (!scope) {
			if (error) *error = estrdup("cannot access \"parent\" when no class scope is active");
		} else if (!scope->parent) {
			if (error) *error = estrdup("cannot access \"parent\" when current class scope has no parent");
		} else {
			if (!suppress_deprecation) {
				crex_error(E_DEPRECATED, "Use of \"parent\" in callables is deprecated");
			}
			fcc->called_scope = crex_get_called_scope(frame);
			if (!fcc->called_scope || !instanceof_function(fcc->called_scope, scope->parent)) {
				fcc->called_scope = scope->parent;
			}
			fcc->calling_scope = scope->parent;
			if (!fcc->object) {
				fcc->object = crex_get_this_object(frame);
			}
			*strict_class = 1;
			ret = 1;
		}
	} else if (crex_string_equals(lcname, ZSTR_KNOWN(CREX_STR_STATIC))) {
		crex_class_entry *called_scope = crex_get_called_scope(frame);

		if (!called_scope) {
			if (error) *error = estrdup("cannot access \"static\" when no class scope is active");
		} else {
			if (!suppress_deprecation) {
				crex_error(E_DEPRECATED, "Use of \"static\" in callables is deprecated");
			}
			fcc->called_scope = called_scope;
			fcc->calling_scope = called_scope;
			if (!fcc->object) {
				fcc->object = crex_get_this_object(frame);
			}
			*strict_class = 1;
			ret = 1;
		}
	} else if ((ce = crex_lookup_class(name)) != NULL) {
		crex_class_entry *scope = get_scope(frame);
		fcc->calling_scope = ce;
		if (scope && !fcc->object) {
			crex_object *object = crex_get_this_object(frame);

			if (object &&
			    instanceof_function(object->ce, scope) &&
			    instanceof_function(scope, ce)) {
				fcc->object = object;
				fcc->called_scope = object->ce;
			} else {
				fcc->called_scope = ce;
			}
		} else {
			fcc->called_scope = fcc->object ? fcc->object->ce : ce;
		}
		*strict_class = 1;
		ret = 1;
	} else {
		if (error) crex_spprintf(error, 0, "class \"%.*s\" not found", (int)name_len, ZSTR_VAL(name));
	}
	ZSTR_ALLOCA_FREE(lcname, use_heap);
	return ret;
}
/* }}} */

CREX_API void crex_release_fcall_info_cache(crex_fcall_info_cache *fcc) {
	if (fcc->function_handler &&
		(fcc->function_handler->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE)) {
		if (fcc->function_handler->common.function_name) {
			crex_string_release_ex(fcc->function_handler->common.function_name, 0);
		}
		crex_free_trampoline(fcc->function_handler);
		fcc->function_handler = NULL;
	}
}

static crex_always_inline bool crex_is_callable_check_func(zval *callable, crex_execute_data *frame, crex_fcall_info_cache *fcc, bool strict_class, char **error, bool suppress_deprecation) /* {{{ */
{
	crex_class_entry *ce_org = fcc->calling_scope;
	bool retval = 0;
	crex_string *mname, *cname;
	crex_string *lmname;
	const char *colon;
	size_t clen;
	HashTable *ftable;
	int call_via_handler = 0;
	crex_class_entry *scope;
	zval *zv;
	ALLOCA_FLAG(use_heap)

	fcc->calling_scope = NULL;

	if (!ce_org) {
		crex_function *func;
		crex_string *lmname;

		/* Check if function with given name exists.
		 * This may be a compound name that includes namespace name */
		if (UNEXPECTED(C_STRVAL_P(callable)[0] == '\\')) {
			/* Skip leading \ */
			ZSTR_ALLOCA_ALLOC(lmname, C_STRLEN_P(callable) - 1, use_heap);
			crex_str_tolower_copy(ZSTR_VAL(lmname), C_STRVAL_P(callable) + 1, C_STRLEN_P(callable) - 1);
			func = crex_fetch_function(lmname);
			ZSTR_ALLOCA_FREE(lmname, use_heap);
		} else {
			lmname = C_STR_P(callable);
			func = crex_fetch_function(lmname);
			if (!func) {
				ZSTR_ALLOCA_ALLOC(lmname, C_STRLEN_P(callable), use_heap);
				crex_str_tolower_copy(ZSTR_VAL(lmname), C_STRVAL_P(callable), C_STRLEN_P(callable));
				func = crex_fetch_function(lmname);
				ZSTR_ALLOCA_FREE(lmname, use_heap);
			}
		}
		if (EXPECTED(func != NULL)) {
			fcc->function_handler = func;
			return 1;
		}
	}

	/* Split name into class/namespace and method/function names */
	if ((colon = crex_memrchr(C_STRVAL_P(callable), ':', C_STRLEN_P(callable))) != NULL &&
		colon > C_STRVAL_P(callable) &&
		*(colon-1) == ':'
	) {
		size_t mlen;

		colon--;
		clen = colon - C_STRVAL_P(callable);
		mlen = C_STRLEN_P(callable) - clen - 2;

		if (colon == C_STRVAL_P(callable)) {
			if (error) *error = estrdup("invalid function name");
			return 0;
		}

		/* This is a compound name.
		 * Try to fetch class and then find static method. */
		if (ce_org) {
			scope = ce_org;
		} else {
			scope = get_scope(frame);
		}

		cname = crex_string_init_interned(C_STRVAL_P(callable), clen, 0);
		if (ZSTR_HAS_CE_CACHE(cname) && ZSTR_GET_CE_CACHE(cname)) {
			fcc->calling_scope = ZSTR_GET_CE_CACHE(cname);
			if (scope && !fcc->object) {
				crex_object *object = crex_get_this_object(frame);

				if (object &&
				    instanceof_function(object->ce, scope) &&
				    instanceof_function(scope, fcc->calling_scope)) {
					fcc->object = object;
					fcc->called_scope = object->ce;
				} else {
					fcc->called_scope = fcc->calling_scope;
				}
			} else {
				fcc->called_scope = fcc->object ? fcc->object->ce : fcc->calling_scope;
			}
			strict_class = 1;
		} else if (!crex_is_callable_check_class(cname, scope, frame, fcc, &strict_class, error, suppress_deprecation || ce_org != NULL)) {
			crex_string_release_ex(cname, 0);
			return 0;
		}
		crex_string_release_ex(cname, 0);

		ftable = &fcc->calling_scope->function_table;
		if (ce_org && !instanceof_function(ce_org, fcc->calling_scope)) {
			if (error) crex_spprintf(error, 0, "class %s is not a subclass of %s", ZSTR_VAL(ce_org->name), ZSTR_VAL(fcc->calling_scope->name));
			return 0;
		}
		if (ce_org && !suppress_deprecation) {
			crex_error(E_DEPRECATED,
				"Callables of the form [\"%s\", \"%s\"] are deprecated",
				ZSTR_VAL(ce_org->name), C_STRVAL_P(callable));
		}
		mname = crex_string_init(C_STRVAL_P(callable) + clen + 2, mlen, 0);
	} else if (ce_org) {
		/* Try to fetch find static method of given class. */
		mname = C_STR_P(callable);
		crex_string_addref(mname);
		ftable = &ce_org->function_table;
		fcc->calling_scope = ce_org;
	} else {
		/* We already checked for plain function before. */
		if (error) {
			crex_spprintf(error, 0, "function \"%s\" not found or invalid function name", C_STRVAL_P(callable));
		}
		return 0;
	}

	lmname = crex_string_tolower(mname);
	if (strict_class &&
	    fcc->calling_scope &&
		crex_string_equals_literal(lmname, CREX_CONSTRUCTOR_FUNC_NAME)) {
		fcc->function_handler = fcc->calling_scope->constructor;
		if (fcc->function_handler) {
			retval = 1;
		}
	} else if ((zv = crex_hash_find(ftable, lmname)) != NULL) {
		fcc->function_handler = C_PTR_P(zv);
		retval = 1;
		if ((fcc->function_handler->op_array.fn_flags & CREX_ACC_CHANGED) &&
		    !strict_class) {
			scope = get_scope(frame);
			if (scope &&
			    instanceof_function(fcc->function_handler->common.scope, scope)) {

				zv = crex_hash_find(&scope->function_table, lmname);
				if (zv != NULL) {
					crex_function *priv_fbc = C_PTR_P(zv);

					if ((priv_fbc->common.fn_flags & CREX_ACC_PRIVATE)
					 && priv_fbc->common.scope == scope) {
						fcc->function_handler = priv_fbc;
					}
				}
			}
		}
		if (!(fcc->function_handler->common.fn_flags & CREX_ACC_PUBLIC) &&
		    (fcc->calling_scope &&
		     ((fcc->object && fcc->calling_scope->__call) ||
		      (!fcc->object && fcc->calling_scope->__callstatic)))) {
			scope = get_scope(frame);
			if (fcc->function_handler->common.scope != scope) {
				if ((fcc->function_handler->common.fn_flags & CREX_ACC_PRIVATE)
				 || !crex_check_protected(crex_get_function_root_class(fcc->function_handler), scope)) {
					retval = 0;
					fcc->function_handler = NULL;
					goto get_function_via_handler;
				}
			}
		}
	} else {
get_function_via_handler:
		if (fcc->object && fcc->calling_scope == ce_org) {
			if (strict_class && ce_org->__call) {
				fcc->function_handler = crex_get_call_trampoline_func(ce_org, mname, 0);
				call_via_handler = 1;
				retval = 1;
			} else {
				fcc->function_handler = fcc->object->handlers->get_method(&fcc->object, mname, NULL);
				if (fcc->function_handler) {
					if (strict_class &&
					    (!fcc->function_handler->common.scope ||
					     !instanceof_function(ce_org, fcc->function_handler->common.scope))) {
						crex_release_fcall_info_cache(fcc);
					} else {
						retval = 1;
						call_via_handler = (fcc->function_handler->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE) != 0;
					}
				}
			}
		} else if (fcc->calling_scope) {
			if (fcc->calling_scope->get_static_method) {
				fcc->function_handler = fcc->calling_scope->get_static_method(fcc->calling_scope, mname);
			} else {
				fcc->function_handler = crex_std_get_static_method(fcc->calling_scope, mname, NULL);
			}
			if (fcc->function_handler) {
				retval = 1;
				call_via_handler = (fcc->function_handler->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE) != 0;
				if (call_via_handler && !fcc->object) {
					crex_object *object = crex_get_this_object(frame);
					if (object &&
					    instanceof_function(object->ce, fcc->calling_scope)) {
						fcc->object = object;
					}
				}
			}
		}
	}

	if (retval) {
		if (fcc->calling_scope && !call_via_handler) {
			if (fcc->function_handler->common.fn_flags & CREX_ACC_ABSTRACT) {
				retval = 0;
				if (error) {
					crex_spprintf(error, 0, "cannot call abstract method %s::%s()", ZSTR_VAL(fcc->calling_scope->name), ZSTR_VAL(fcc->function_handler->common.function_name));
				}
			} else if (!fcc->object && !(fcc->function_handler->common.fn_flags & CREX_ACC_STATIC)) {
				retval = 0;
				if (error) {
					crex_spprintf(error, 0, "non-static method %s::%s() cannot be called statically", ZSTR_VAL(fcc->calling_scope->name), ZSTR_VAL(fcc->function_handler->common.function_name));
				}
			}
			if (retval
			 && !(fcc->function_handler->common.fn_flags & CREX_ACC_PUBLIC)) {
				scope = get_scope(frame);
				if (fcc->function_handler->common.scope != scope) {
					if ((fcc->function_handler->common.fn_flags & CREX_ACC_PRIVATE)
					 || (!crex_check_protected(crex_get_function_root_class(fcc->function_handler), scope))) {
						if (error) {
							if (*error) {
								efree(*error);
							}
							crex_spprintf(error, 0, "cannot access %s method %s::%s()", crex_visibility_string(fcc->function_handler->common.fn_flags), ZSTR_VAL(fcc->calling_scope->name), ZSTR_VAL(fcc->function_handler->common.function_name));
						}
						retval = 0;
					}
				}
			}
		}
	} else if (error) {
		if (fcc->calling_scope) {
			crex_spprintf(error, 0, "class %s does not have a method \"%s\"", ZSTR_VAL(fcc->calling_scope->name), ZSTR_VAL(mname));
		} else {
			crex_spprintf(error, 0, "function %s() does not exist", ZSTR_VAL(mname));
		}
	}
	crex_string_release_ex(lmname, 0);
	crex_string_release_ex(mname, 0);

	if (fcc->object) {
		fcc->called_scope = fcc->object->ce;
		if (fcc->function_handler
		 && (fcc->function_handler->common.fn_flags & CREX_ACC_STATIC)) {
			fcc->object = NULL;
		}
	}
	return retval;
}
/* }}} */

CREX_API crex_string *crex_get_callable_name_ex(zval *callable, crex_object *object) /* {{{ */
{
try_again:
	switch (C_TYPE_P(callable)) {
		case IS_STRING:
			if (object) {
				return crex_create_member_string(object->ce->name, C_STR_P(callable));
			}
			return crex_string_copy(C_STR_P(callable));

		case IS_ARRAY:
		{
			zval *method = NULL;
			zval *obj = NULL;

			if (crex_hash_num_elements(C_ARRVAL_P(callable)) == 2) {
				obj = crex_hash_index_find_deref(C_ARRVAL_P(callable), 0);
				method = crex_hash_index_find_deref(C_ARRVAL_P(callable), 1);
			}

			if (obj == NULL || method == NULL || C_TYPE_P(method) != IS_STRING) {
				return ZSTR_KNOWN(CREX_STR_ARRAY_CAPITALIZED);
			}

			if (C_TYPE_P(obj) == IS_STRING) {
				return crex_create_member_string(C_STR_P(obj), C_STR_P(method));
			} else if (C_TYPE_P(obj) == IS_OBJECT) {
				return crex_create_member_string(C_OBJCE_P(obj)->name, C_STR_P(method));
			} else {
				return ZSTR_KNOWN(CREX_STR_ARRAY_CAPITALIZED);
			}
		}
		case IS_OBJECT:
		{
			crex_class_entry *ce = C_OBJCE_P(callable);
			return crex_string_concat2(
				ZSTR_VAL(ce->name), ZSTR_LEN(ce->name),
				"::__invoke", sizeof("::__invoke") - 1);
		}
		case IS_REFERENCE:
			callable = C_REFVAL_P(callable);
			goto try_again;
		default:
			return zval_get_string_func(callable);
	}
}
/* }}} */

CREX_API crex_string *crex_get_callable_name(zval *callable) /* {{{ */
{
	return crex_get_callable_name_ex(callable, NULL);
}
/* }}} */

CREX_API bool crex_is_callable_at_frame(
		zval *callable, crex_object *object, crex_execute_data *frame,
		uint32_t check_flags, crex_fcall_info_cache *fcc, char **error) /* {{{ */
{
	bool ret;
	crex_fcall_info_cache fcc_local;
	bool strict_class = 0;

	if (fcc == NULL) {
		fcc = &fcc_local;
	}
	if (error) {
		*error = NULL;
	}

	fcc->calling_scope = NULL;
	fcc->called_scope = NULL;
	fcc->function_handler = NULL;
	fcc->object = NULL;
	fcc->closure = NULL;

again:
	switch (C_TYPE_P(callable)) {
		case IS_STRING:
			if (object) {
				fcc->object = object;
				fcc->calling_scope = object->ce;
			}

			if (check_flags & IS_CALLABLE_CHECK_SYNTAX_ONLY) {
				fcc->called_scope = fcc->calling_scope;
				return 1;
			}

check_func:
			ret = crex_is_callable_check_func(callable, frame, fcc, strict_class, error, check_flags & IS_CALLABLE_SUPPRESS_DEPRECATIONS);
			if (fcc == &fcc_local) {
				crex_release_fcall_info_cache(fcc);
			}
			return ret;

		case IS_ARRAY:
			{
				if (crex_hash_num_elements(C_ARRVAL_P(callable)) != 2) {
					if (error) *error = estrdup("array callback must have exactly two members");
					return 0;
				}

				zval *obj = crex_hash_index_find(C_ARRVAL_P(callable), 0);
				zval *method = crex_hash_index_find(C_ARRVAL_P(callable), 1);
				if (!obj || !method) {
					if (error) *error = estrdup("array callback has to contain indices 0 and 1");
					return 0;
				}

				ZVAL_DEREF(obj);
				if (C_TYPE_P(obj) != IS_STRING && C_TYPE_P(obj) != IS_OBJECT) {
					if (error) *error = estrdup("first array member is not a valid class name or object");
					return 0;
				}

				ZVAL_DEREF(method);
				if (C_TYPE_P(method) != IS_STRING) {
					if (error) *error = estrdup("second array member is not a valid method");
					return 0;
				}

				if (C_TYPE_P(obj) == IS_STRING) {
					if (check_flags & IS_CALLABLE_CHECK_SYNTAX_ONLY) {
						return 1;
					}

					if (!crex_is_callable_check_class(C_STR_P(obj), get_scope(frame), frame, fcc, &strict_class, error, check_flags & IS_CALLABLE_SUPPRESS_DEPRECATIONS)) {
						return 0;
					}
				} else {
					CREX_ASSERT(C_TYPE_P(obj) == IS_OBJECT);
					fcc->calling_scope = C_OBJCE_P(obj); /* TBFixed: what if it's overloaded? */
					fcc->object = C_OBJ_P(obj);

					if (check_flags & IS_CALLABLE_CHECK_SYNTAX_ONLY) {
						fcc->called_scope = fcc->calling_scope;
						return 1;
					}
				}

				callable = method;
				goto check_func;
			}
			return 0;
		case IS_OBJECT:
			if (C_OBJ_HANDLER_P(callable, get_closure) && C_OBJ_HANDLER_P(callable, get_closure)(C_OBJ_P(callable), &fcc->calling_scope, &fcc->function_handler, &fcc->object, 1) == SUCCESS) {
				fcc->called_scope = fcc->calling_scope;
				fcc->closure = C_OBJ_P(callable);
				if (fcc == &fcc_local) {
					crex_release_fcall_info_cache(fcc);
				}
				return 1;
			}
			if (error) *error = estrdup("no array or string given");
			return 0;
		case IS_REFERENCE:
			callable = C_REFVAL_P(callable);
			goto again;
		default:
			if (error) *error = estrdup("no array or string given");
			return 0;
	}
}
/* }}} */

CREX_API bool crex_is_callable_ex(zval *callable, crex_object *object, uint32_t check_flags, crex_string **callable_name, crex_fcall_info_cache *fcc, char **error) /* {{{ */
{
	/* Determine callability at the first parent user frame. */
	crex_execute_data *frame = EG(current_execute_data);
	while (frame && (!frame->func || !CREX_USER_CODE(frame->func->type))) {
		frame = frame->prev_execute_data;
	}

	bool ret = crex_is_callable_at_frame(callable, object, frame, check_flags, fcc, error);
	if (callable_name) {
		*callable_name = crex_get_callable_name_ex(callable, object);
	}
	return ret;
}

CREX_API bool crex_is_callable(zval *callable, uint32_t check_flags, crex_string **callable_name) /* {{{ */
{
	return crex_is_callable_ex(callable, NULL, check_flags, callable_name, NULL, NULL);
}
/* }}} */

CREX_API bool crex_make_callable(zval *callable, crex_string **callable_name) /* {{{ */
{
	crex_fcall_info_cache fcc;

	if (crex_is_callable_ex(callable, NULL, IS_CALLABLE_SUPPRESS_DEPRECATIONS, callable_name, &fcc, NULL)) {
		if (C_TYPE_P(callable) == IS_STRING && fcc.calling_scope) {
			zval_ptr_dtor_str(callable);
			array_init(callable);
			add_next_index_str(callable, crex_string_copy(fcc.calling_scope->name));
			add_next_index_str(callable, crex_string_copy(fcc.function_handler->common.function_name));
		}
		crex_release_fcall_info_cache(&fcc);
		return 1;
	}
	return 0;
}
/* }}} */

CREX_API crex_result crex_fcall_info_init(zval *callable, uint32_t check_flags, crex_fcall_info *fci, crex_fcall_info_cache *fcc, crex_string **callable_name, char **error) /* {{{ */
{
	if (!crex_is_callable_ex(callable, NULL, check_flags, callable_name, fcc, error)) {
		return FAILURE;
	}

	fci->size = sizeof(*fci);
	fci->object = fcc->object;
	ZVAL_COPY_VALUE(&fci->function_name, callable);
	fci->retval = NULL;
	fci->param_count = 0;
	fci->params = NULL;
	fci->named_params = NULL;

	return SUCCESS;
}
/* }}} */

CREX_API void crex_fcall_info_args_clear(crex_fcall_info *fci, bool free_mem) /* {{{ */
{
	if (fci->params) {
		zval *p = fci->params;
		zval *end = p + fci->param_count;

		while (p != end) {
			i_zval_ptr_dtor(p);
			p++;
		}
		if (free_mem) {
			efree(fci->params);
			fci->params = NULL;
		}
	}
	fci->param_count = 0;
}
/* }}} */

CREX_API void crex_fcall_info_args_save(crex_fcall_info *fci, uint32_t *param_count, zval **params) /* {{{ */
{
	*param_count = fci->param_count;
	*params = fci->params;
	fci->param_count = 0;
	fci->params = NULL;
}
/* }}} */

CREX_API void crex_fcall_info_args_restore(crex_fcall_info *fci, uint32_t param_count, zval *params) /* {{{ */
{
	crex_fcall_info_args_clear(fci, 1);
	fci->param_count = param_count;
	fci->params = params;
}
/* }}} */

CREX_API crex_result crex_fcall_info_args_ex(crex_fcall_info *fci, crex_function *func, zval *args) /* {{{ */
{
	zval *arg, *params;
	uint32_t n = 1;

	crex_fcall_info_args_clear(fci, !args);

	if (!args) {
		return SUCCESS;
	}

	if (C_TYPE_P(args) != IS_ARRAY) {
		return FAILURE;
	}

	fci->param_count = crex_hash_num_elements(C_ARRVAL_P(args));
	fci->params = params = (zval *) erealloc(fci->params, fci->param_count * sizeof(zval));

	CREX_HASH_FOREACH_VAL(C_ARRVAL_P(args), arg) {
		if (func && !C_ISREF_P(arg) && ARG_SHOULD_BE_SENT_BY_REF(func, n)) {
			ZVAL_NEW_REF(params, arg);
			C_TRY_ADDREF_P(arg);
		} else {
			ZVAL_COPY(params, arg);
		}
		params++;
		n++;
	} CREX_HASH_FOREACH_END();

	return SUCCESS;
}
/* }}} */

CREX_API crex_result crex_fcall_info_args(crex_fcall_info *fci, zval *args) /* {{{ */
{
	return crex_fcall_info_args_ex(fci, NULL, args);
}
/* }}} */

CREX_API void crex_fcall_info_argp(crex_fcall_info *fci, uint32_t argc, zval *argv) /* {{{ */
{
	crex_fcall_info_args_clear(fci, !argc);

	if (argc) {
		fci->param_count = argc;
		fci->params = (zval *) erealloc(fci->params, fci->param_count * sizeof(zval));

		for (uint32_t i = 0; i < argc; ++i) {
			ZVAL_COPY(&fci->params[i], &argv[i]);
		}
	}
}
/* }}} */

CREX_API void crex_fcall_info_argv(crex_fcall_info *fci, uint32_t argc, va_list *argv) /* {{{ */
{
	crex_fcall_info_args_clear(fci, !argc);

	if (argc) {
		zval *arg;
		fci->param_count = argc;
		fci->params = (zval *) erealloc(fci->params, fci->param_count * sizeof(zval));

		for (uint32_t i = 0; i < argc; ++i) {
			arg = va_arg(*argv, zval *);
			ZVAL_COPY(&fci->params[i], arg);
		}
	}
}
/* }}} */

CREX_API void crex_fcall_info_argn(crex_fcall_info *fci, uint32_t argc, ...) /* {{{ */
{
	va_list argv;

	va_start(argv, argc);
	crex_fcall_info_argv(fci, argc, &argv);
	va_end(argv);
}
/* }}} */

CREX_API crex_result crex_fcall_info_call(crex_fcall_info *fci, crex_fcall_info_cache *fcc, zval *retval_ptr, zval *args) /* {{{ */
{
	zval retval, *org_params = NULL;
	uint32_t org_count = 0;
	crex_result result;

	fci->retval = retval_ptr ? retval_ptr : &retval;
	if (args) {
		crex_fcall_info_args_save(fci, &org_count, &org_params);
		crex_fcall_info_args(fci, args);
	}
	result = crex_call_function(fci, fcc);

	if (!retval_ptr && C_TYPE(retval) != IS_UNDEF) {
		zval_ptr_dtor(&retval);
	}
	if (args) {
		crex_fcall_info_args_restore(fci, org_count, org_params);
	}
	return result;
}
/* }}} */

CREX_API void crex_get_callable_zval_from_fcc(const crex_fcall_info_cache *fcc, zval *callable)
{
	if (fcc->closure) {
		ZVAL_OBJ_COPY(callable, fcc->closure);
	} else if (fcc->function_handler->common.scope) {
		array_init(callable);
		if (fcc->object) {
			GC_ADDREF(fcc->object);
			add_next_index_object(callable, fcc->object);
		} else {
			add_next_index_str(callable, crex_string_copy(fcc->calling_scope->name));
		}
		add_next_index_str(callable, crex_string_copy(fcc->function_handler->common.function_name));
	} else {
		ZVAL_STR_COPY(callable, fcc->function_handler->common.function_name);
	}
}

CREX_API const char *crex_get_module_version(const char *module_name) /* {{{ */
{
	crex_string *lname;
	size_t name_len = strlen(module_name);
	crex_module_entry *module;

	lname = crex_string_alloc(name_len, 0);
	crex_str_tolower_copy(ZSTR_VAL(lname), module_name, name_len);
	module = crex_hash_find_ptr(&module_registry, lname);
	crex_string_efree(lname);
	return module ? module->version : NULL;
}
/* }}} */

static crex_always_inline bool is_persistent_class(crex_class_entry *ce) {
	return (ce->type & CREX_INTERNAL_CLASS)
		&& ce->info.internal.module->type == MODULE_PERSISTENT;
}

CREX_API crex_property_info *crex_declare_typed_property(crex_class_entry *ce, crex_string *name, zval *property, int access_type, crex_string *doc_comment, crex_type type) /* {{{ */
{
	crex_property_info *property_info, *property_info_ptr;

	if (CREX_TYPE_IS_SET(type)) {
		ce->ce_flags |= CREX_ACC_HAS_TYPE_HINTS;

		if (access_type & CREX_ACC_READONLY) {
			ce->ce_flags |= CREX_ACC_HAS_READONLY_PROPS;
		}
	}

	if (ce->type == CREX_INTERNAL_CLASS) {
		property_info = pemalloc(sizeof(crex_property_info), 1);
	} else {
		property_info = crex_arena_alloc(&CG(arena), sizeof(crex_property_info));
		if (C_TYPE_P(property) == IS_CONSTANT_AST) {
			ce->ce_flags &= ~CREX_ACC_CONSTANTS_UPDATED;
			if (access_type & CREX_ACC_STATIC) {
				ce->ce_flags |= CREX_ACC_HAS_AST_STATICS;
			} else {
				ce->ce_flags |= CREX_ACC_HAS_AST_PROPERTIES;
			}
		}
	}

	if (C_TYPE_P(property) == IS_STRING && !ZSTR_IS_INTERNED(C_STR_P(property))) {
		zval_make_interned_string(property);
	}

	if (!(access_type & CREX_ACC_PPP_MASK)) {
		access_type |= CREX_ACC_PUBLIC;
	}
	if (access_type & CREX_ACC_STATIC) {
		if ((property_info_ptr = crex_hash_find_ptr(&ce->properties_info, name)) != NULL &&
		    (property_info_ptr->flags & CREX_ACC_STATIC) != 0) {
			property_info->offset = property_info_ptr->offset;
			zval_ptr_dtor(&ce->default_static_members_table[property_info->offset]);
			if (property_info_ptr->doc_comment && property_info_ptr->ce == ce) {
				crex_string_release(property_info_ptr->doc_comment);
			}
			crex_hash_del(&ce->properties_info, name);
		} else {
			property_info->offset = ce->default_static_members_count++;
			ce->default_static_members_table = perealloc(ce->default_static_members_table, sizeof(zval) * ce->default_static_members_count, ce->type == CREX_INTERNAL_CLASS);
		}
		ZVAL_COPY_VALUE(&ce->default_static_members_table[property_info->offset], property);
		if (!CREX_MAP_PTR(ce->static_members_table)) {
			if (ce->type == CREX_INTERNAL_CLASS &&
					ce->info.internal.module->type == MODULE_PERSISTENT) {
				CREX_MAP_PTR_NEW(ce->static_members_table);
			}
		}
	} else {
		zval *property_default_ptr;
		if ((property_info_ptr = crex_hash_find_ptr(&ce->properties_info, name)) != NULL &&
		    (property_info_ptr->flags & CREX_ACC_STATIC) == 0) {
			property_info->offset = property_info_ptr->offset;
			zval_ptr_dtor(&ce->default_properties_table[OBJ_PROP_TO_NUM(property_info->offset)]);
			if (property_info_ptr->doc_comment && property_info_ptr->ce == ce) {
				crex_string_release_ex(property_info_ptr->doc_comment, 1);
			}
			crex_hash_del(&ce->properties_info, name);

			CREX_ASSERT(ce->type == CREX_INTERNAL_CLASS);
			CREX_ASSERT(ce->properties_info_table != NULL);
			ce->properties_info_table[OBJ_PROP_TO_NUM(property_info->offset)] = property_info;
		} else {
			property_info->offset = OBJ_PROP_TO_OFFSET(ce->default_properties_count);
			ce->default_properties_count++;
			ce->default_properties_table = perealloc(ce->default_properties_table, sizeof(zval) * ce->default_properties_count, ce->type == CREX_INTERNAL_CLASS);

			/* For user classes this is handled during linking */
			if (ce->type == CREX_INTERNAL_CLASS) {
				ce->properties_info_table = perealloc(ce->properties_info_table, sizeof(crex_property_info *) * ce->default_properties_count, 1);
				ce->properties_info_table[ce->default_properties_count - 1] = property_info;
			}
		}
		property_default_ptr = &ce->default_properties_table[OBJ_PROP_TO_NUM(property_info->offset)];
		ZVAL_COPY_VALUE(property_default_ptr, property);
		C_PROP_FLAG_P(property_default_ptr) = C_ISUNDEF_P(property) ? IS_PROP_UNINIT : 0;
	}
	if (ce->type & CREX_INTERNAL_CLASS) {
		/* Must be interned to avoid ZTS data races */
		if (is_persistent_class(ce)) {
			name = crex_new_interned_string(crex_string_copy(name));
		}

		if (C_REFCOUNTED_P(property)) {
			crex_error_noreturn(E_CORE_ERROR, "Internal zvals cannot be refcounted");
		}
	}

	if (access_type & CREX_ACC_PUBLIC) {
		property_info->name = crex_string_copy(name);
	} else if (access_type & CREX_ACC_PRIVATE) {
		property_info->name = crex_mangle_property_name(ZSTR_VAL(ce->name), ZSTR_LEN(ce->name), ZSTR_VAL(name), ZSTR_LEN(name), is_persistent_class(ce));
	} else {
		CREX_ASSERT(access_type & CREX_ACC_PROTECTED);
		property_info->name = crex_mangle_property_name("*", 1, ZSTR_VAL(name), ZSTR_LEN(name), is_persistent_class(ce));
	}

	property_info->name = crex_new_interned_string(property_info->name);
	property_info->flags = access_type;
	property_info->doc_comment = doc_comment;
	property_info->attributes = NULL;
	property_info->ce = ce;
	property_info->type = type;

	if (is_persistent_class(ce)) {
		crex_normalize_internal_type(&property_info->type);
	}

	crex_hash_update_ptr(&ce->properties_info, name, property_info);

	return property_info;
}
/* }}} */

CREX_API crex_result crex_try_assign_typed_ref_ex(crex_reference *ref, zval *val, bool strict) /* {{{ */
{
	if (UNEXPECTED(!crex_verify_ref_assignable_zval(ref, val, strict))) {
		zval_ptr_dtor(val);
		return FAILURE;
	} else {
		zval_ptr_dtor(&ref->val);
		ZVAL_COPY_VALUE(&ref->val, val);
		return SUCCESS;
	}
}
/* }}} */

CREX_API crex_result crex_try_assign_typed_ref(crex_reference *ref, zval *val) /* {{{ */
{
	return crex_try_assign_typed_ref_ex(ref, val, CREX_ARG_USES_STRICT_TYPES());
}
/* }}} */

CREX_API crex_result crex_try_assign_typed_ref_null(crex_reference *ref) /* {{{ */
{
	zval tmp;

	ZVAL_NULL(&tmp);
	return crex_try_assign_typed_ref(ref, &tmp);
}
/* }}} */

CREX_API crex_result crex_try_assign_typed_ref_bool(crex_reference *ref, bool val) /* {{{ */
{
	zval tmp;

	ZVAL_BOOL(&tmp, val);
	return crex_try_assign_typed_ref(ref, &tmp);
}
/* }}} */

CREX_API crex_result crex_try_assign_typed_ref_long(crex_reference *ref, crex_long lval) /* {{{ */
{
	zval tmp;

	ZVAL_LONG(&tmp, lval);
	return crex_try_assign_typed_ref(ref, &tmp);
}
/* }}} */

CREX_API crex_result crex_try_assign_typed_ref_double(crex_reference *ref, double dval) /* {{{ */
{
	zval tmp;

	ZVAL_DOUBLE(&tmp, dval);
	return crex_try_assign_typed_ref(ref, &tmp);
}
/* }}} */

CREX_API crex_result crex_try_assign_typed_ref_empty_string(crex_reference *ref) /* {{{ */
{
	zval tmp;

	ZVAL_EMPTY_STRING(&tmp);
	return crex_try_assign_typed_ref(ref, &tmp);
}
/* }}} */

CREX_API crex_result crex_try_assign_typed_ref_str(crex_reference *ref, crex_string *str) /* {{{ */
{
	zval tmp;

	ZVAL_STR(&tmp, str);
	return crex_try_assign_typed_ref(ref, &tmp);
}
/* }}} */

CREX_API crex_result crex_try_assign_typed_ref_string(crex_reference *ref, const char *string) /* {{{ */
{
	zval tmp;

	ZVAL_STRING(&tmp, string);
	return crex_try_assign_typed_ref(ref, &tmp);
}
/* }}} */

CREX_API crex_result crex_try_assign_typed_ref_stringl(crex_reference *ref, const char *string, size_t len) /* {{{ */
{
	zval tmp;

	ZVAL_STRINGL(&tmp, string, len);
	return crex_try_assign_typed_ref(ref, &tmp);
}
/* }}} */

CREX_API crex_result crex_try_assign_typed_ref_arr(crex_reference *ref, crex_array *arr) /* {{{ */
{
	zval tmp;

	ZVAL_ARR(&tmp, arr);
	return crex_try_assign_typed_ref(ref, &tmp);
}
/* }}} */

CREX_API crex_result crex_try_assign_typed_ref_res(crex_reference *ref, crex_resource *res) /* {{{ */
{
	zval tmp;

	ZVAL_RES(&tmp, res);
	return crex_try_assign_typed_ref(ref, &tmp);
}
/* }}} */

CREX_API crex_result crex_try_assign_typed_ref_zval(crex_reference *ref, zval *zv) /* {{{ */
{
	zval tmp;

	ZVAL_COPY_VALUE(&tmp, zv);
	return crex_try_assign_typed_ref(ref, &tmp);
}
/* }}} */

CREX_API crex_result crex_try_assign_typed_ref_zval_ex(crex_reference *ref, zval *zv, bool strict) /* {{{ */
{
	zval tmp;

	ZVAL_COPY_VALUE(&tmp, zv);
	return crex_try_assign_typed_ref_ex(ref, &tmp, strict);
}
/* }}} */

CREX_API void crex_declare_property_ex(crex_class_entry *ce, crex_string *name, zval *property, int access_type, crex_string *doc_comment) /* {{{ */
{
	crex_declare_typed_property(ce, name, property, access_type, doc_comment, (crex_type) CREX_TYPE_INIT_NONE(0));
}
/* }}} */

CREX_API void crex_declare_property(crex_class_entry *ce, const char *name, size_t name_length, zval *property, int access_type) /* {{{ */
{
	crex_string *key = crex_string_init(name, name_length, is_persistent_class(ce));
	crex_declare_property_ex(ce, key, property, access_type, NULL);
	crex_string_release(key);
}
/* }}} */

CREX_API void crex_declare_property_null(crex_class_entry *ce, const char *name, size_t name_length, int access_type) /* {{{ */
{
	zval property;

	ZVAL_NULL(&property);
	crex_declare_property(ce, name, name_length, &property, access_type);
}
/* }}} */

CREX_API void crex_declare_property_bool(crex_class_entry *ce, const char *name, size_t name_length, crex_long value, int access_type) /* {{{ */
{
	zval property;

	ZVAL_BOOL(&property, value);
	crex_declare_property(ce, name, name_length, &property, access_type);
}
/* }}} */

CREX_API void crex_declare_property_long(crex_class_entry *ce, const char *name, size_t name_length, crex_long value, int access_type) /* {{{ */
{
	zval property;

	ZVAL_LONG(&property, value);
	crex_declare_property(ce, name, name_length, &property, access_type);
}
/* }}} */

CREX_API void crex_declare_property_double(crex_class_entry *ce, const char *name, size_t name_length, double value, int access_type) /* {{{ */
{
	zval property;

	ZVAL_DOUBLE(&property, value);
	crex_declare_property(ce, name, name_length, &property, access_type);
}
/* }}} */

CREX_API void crex_declare_property_string(crex_class_entry *ce, const char *name, size_t name_length, const char *value, int access_type) /* {{{ */
{
	zval property;

	ZVAL_NEW_STR(&property, crex_string_init(value, strlen(value), ce->type & CREX_INTERNAL_CLASS));
	crex_declare_property(ce, name, name_length, &property, access_type);
}
/* }}} */

CREX_API void crex_declare_property_stringl(crex_class_entry *ce, const char *name, size_t name_length, const char *value, size_t value_len, int access_type) /* {{{ */
{
	zval property;

	ZVAL_NEW_STR(&property, crex_string_init(value, value_len, ce->type & CREX_INTERNAL_CLASS));
	crex_declare_property(ce, name, name_length, &property, access_type);
}
/* }}} */

CREX_API crex_class_constant *crex_declare_typed_class_constant(crex_class_entry *ce, crex_string *name, zval *value, int flags, crex_string *doc_comment, crex_type type) /* {{{ */
{
	crex_class_constant *c;

	if (ce->ce_flags & CREX_ACC_INTERFACE) {
		if (!(flags & CREX_ACC_PUBLIC)) {
			crex_error_noreturn(E_COMPILE_ERROR, "Access type for interface constant %s::%s must be public", ZSTR_VAL(ce->name), ZSTR_VAL(name));
		}
	}

	if (crex_string_equals_ci(name, ZSTR_KNOWN(CREX_STR_CLASS))) {
		crex_error_noreturn(ce->type == CREX_INTERNAL_CLASS ? E_CORE_ERROR : E_COMPILE_ERROR,
				"A class constant must not be called 'class'; it is reserved for class name fetching");
	}

	if (C_TYPE_P(value) == IS_STRING && !ZSTR_IS_INTERNED(C_STR_P(value))) {
		zval_make_interned_string(value);
	}

	if (ce->type == CREX_INTERNAL_CLASS) {
		c = pemalloc(sizeof(crex_class_constant), 1);
	} else {
		c = crex_arena_alloc(&CG(arena), sizeof(crex_class_constant));
	}
	ZVAL_COPY_VALUE(&c->value, value);
	CREX_CLASS_CONST_FLAGS(c) = flags;
	c->doc_comment = doc_comment;
	c->attributes = NULL;
	c->ce = ce;
	c->type = type;

	if (C_TYPE_P(value) == IS_CONSTANT_AST) {
		ce->ce_flags &= ~CREX_ACC_CONSTANTS_UPDATED;
		ce->ce_flags |= CREX_ACC_HAS_AST_CONSTANTS;
		if (ce->type == CREX_INTERNAL_CLASS && !CREX_MAP_PTR(ce->mutable_data)) {
			CREX_MAP_PTR_NEW(ce->mutable_data);
		}
	}

	if (!crex_hash_add_ptr(&ce->constants_table, name, c)) {
		crex_error_noreturn(ce->type == CREX_INTERNAL_CLASS ? E_CORE_ERROR : E_COMPILE_ERROR,
			"Cannot redefine class constant %s::%s", ZSTR_VAL(ce->name), ZSTR_VAL(name));
	}

	return c;
}

CREX_API crex_class_constant *crex_declare_class_constant_ex(crex_class_entry *ce, crex_string *name, zval *value, int flags, crex_string *doc_comment)
{
	return crex_declare_typed_class_constant(ce, name, value, flags, doc_comment, (crex_type) CREX_TYPE_INIT_NONE(0));
}

CREX_API void crex_declare_class_constant(crex_class_entry *ce, const char *name, size_t name_length, zval *value) /* {{{ */
{
	crex_string *key;

	if (ce->type == CREX_INTERNAL_CLASS) {
		key = crex_string_init_interned(name, name_length, 1);
	} else {
		key = crex_string_init(name, name_length, 0);
	}
	crex_declare_class_constant_ex(ce, key, value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(key);
}
/* }}} */

CREX_API void crex_declare_class_constant_null(crex_class_entry *ce, const char *name, size_t name_length) /* {{{ */
{
	zval constant;

	ZVAL_NULL(&constant);
	crex_declare_class_constant(ce, name, name_length, &constant);
}
/* }}} */

CREX_API void crex_declare_class_constant_long(crex_class_entry *ce, const char *name, size_t name_length, crex_long value) /* {{{ */
{
	zval constant;

	ZVAL_LONG(&constant, value);
	crex_declare_class_constant(ce, name, name_length, &constant);
}
/* }}} */

CREX_API void crex_declare_class_constant_bool(crex_class_entry *ce, const char *name, size_t name_length, bool value) /* {{{ */
{
	zval constant;

	ZVAL_BOOL(&constant, value);
	crex_declare_class_constant(ce, name, name_length, &constant);
}
/* }}} */

CREX_API void crex_declare_class_constant_double(crex_class_entry *ce, const char *name, size_t name_length, double value) /* {{{ */
{
	zval constant;

	ZVAL_DOUBLE(&constant, value);
	crex_declare_class_constant(ce, name, name_length, &constant);
}
/* }}} */

CREX_API void crex_declare_class_constant_stringl(crex_class_entry *ce, const char *name, size_t name_length, const char *value, size_t value_length) /* {{{ */
{
	zval constant;

	ZVAL_NEW_STR(&constant, crex_string_init(value, value_length, ce->type & CREX_INTERNAL_CLASS));
	crex_declare_class_constant(ce, name, name_length, &constant);
}
/* }}} */

CREX_API void crex_declare_class_constant_string(crex_class_entry *ce, const char *name, size_t name_length, const char *value) /* {{{ */
{
	crex_declare_class_constant_stringl(ce, name, name_length, value, strlen(value));
}
/* }}} */

CREX_API void crex_update_property_ex(crex_class_entry *scope, crex_object *object, crex_string *name, zval *value) /* {{{ */
{
	crex_class_entry *old_scope = EG(fake_scope);

	EG(fake_scope) = scope;

	object->handlers->write_property(object, name, value, NULL);

	EG(fake_scope) = old_scope;
}
/* }}} */

CREX_API void crex_update_property(crex_class_entry *scope, crex_object *object, const char *name, size_t name_length, zval *value) /* {{{ */
{
	crex_string *property;
	crex_class_entry *old_scope = EG(fake_scope);

	EG(fake_scope) = scope;

	property = crex_string_init(name, name_length, 0);
	object->handlers->write_property(object, property, value, NULL);
	crex_string_release_ex(property, 0);

	EG(fake_scope) = old_scope;
}
/* }}} */

CREX_API void crex_update_property_null(crex_class_entry *scope, crex_object *object, const char *name, size_t name_length) /* {{{ */
{
	zval tmp;

	ZVAL_NULL(&tmp);
	crex_update_property(scope, object, name, name_length, &tmp);
}
/* }}} */

CREX_API void crex_unset_property(crex_class_entry *scope, crex_object *object, const char *name, size_t name_length) /* {{{ */
{
	crex_string *property;
	crex_class_entry *old_scope = EG(fake_scope);

	EG(fake_scope) = scope;

	property = crex_string_init(name, name_length, 0);
	object->handlers->unset_property(object, property, 0);
	crex_string_release_ex(property, 0);

	EG(fake_scope) = old_scope;
}
/* }}} */

CREX_API void crex_update_property_bool(crex_class_entry *scope, crex_object *object, const char *name, size_t name_length, crex_long value) /* {{{ */
{
	zval tmp;

	ZVAL_BOOL(&tmp, value);
	crex_update_property(scope, object, name, name_length, &tmp);
}
/* }}} */

CREX_API void crex_update_property_long(crex_class_entry *scope, crex_object *object, const char *name, size_t name_length, crex_long value) /* {{{ */
{
	zval tmp;

	ZVAL_LONG(&tmp, value);
	crex_update_property(scope, object, name, name_length, &tmp);
}
/* }}} */

CREX_API void crex_update_property_double(crex_class_entry *scope, crex_object *object, const char *name, size_t name_length, double value) /* {{{ */
{
	zval tmp;

	ZVAL_DOUBLE(&tmp, value);
	crex_update_property(scope, object, name, name_length, &tmp);
}
/* }}} */

CREX_API void crex_update_property_str(crex_class_entry *scope, crex_object *object, const char *name, size_t name_length, crex_string *value) /* {{{ */
{
	zval tmp;

	ZVAL_STR(&tmp, value);
	crex_update_property(scope, object, name, name_length, &tmp);
}
/* }}} */

CREX_API void crex_update_property_string(crex_class_entry *scope, crex_object *object, const char *name, size_t name_length, const char *value) /* {{{ */
{
	zval tmp;

	ZVAL_STRING(&tmp, value);
	C_SET_REFCOUNT(tmp, 0);
	crex_update_property(scope, object, name, name_length, &tmp);
}
/* }}} */

CREX_API void crex_update_property_stringl(crex_class_entry *scope, crex_object *object, const char *name, size_t name_length, const char *value, size_t value_len) /* {{{ */
{
	zval tmp;

	ZVAL_STRINGL(&tmp, value, value_len);
	C_SET_REFCOUNT(tmp, 0);
	crex_update_property(scope, object, name, name_length, &tmp);
}
/* }}} */

CREX_API crex_result crex_update_static_property_ex(crex_class_entry *scope, crex_string *name, zval *value) /* {{{ */
{
	zval *property, tmp;
	crex_property_info *prop_info;
	crex_class_entry *old_scope = EG(fake_scope);

	if (UNEXPECTED(!(scope->ce_flags & CREX_ACC_CONSTANTS_UPDATED))) {
		if (UNEXPECTED(crex_update_class_constants(scope) != SUCCESS)) {
			return FAILURE;
		}
	}

	EG(fake_scope) = scope;
	property = crex_std_get_static_property_with_info(scope, name, BP_VAR_W, &prop_info);
	EG(fake_scope) = old_scope;

	if (!property) {
		return FAILURE;
	}

	CREX_ASSERT(!C_ISREF_P(value));
	C_TRY_ADDREF_P(value);
	if (CREX_TYPE_IS_SET(prop_info->type)) {
		ZVAL_COPY_VALUE(&tmp, value);
		if (!crex_verify_property_type(prop_info, &tmp, /* strict */ 0)) {
			C_TRY_DELREF_P(value);
			return FAILURE;
		}
		value = &tmp;
	}

	crex_assign_to_variable(property, value, IS_TMP_VAR, /* strict */ 0);
	return SUCCESS;
}
/* }}} */

CREX_API crex_result crex_update_static_property(crex_class_entry *scope, const char *name, size_t name_length, zval *value) /* {{{ */
{
	crex_string *key = crex_string_init(name, name_length, 0);
	crex_result retval = crex_update_static_property_ex(scope, key, value);
	crex_string_efree(key);
	return retval;
}
/* }}} */

CREX_API crex_result crex_update_static_property_null(crex_class_entry *scope, const char *name, size_t name_length) /* {{{ */
{
	zval tmp;

	ZVAL_NULL(&tmp);
	return crex_update_static_property(scope, name, name_length, &tmp);
}
/* }}} */

CREX_API crex_result crex_update_static_property_bool(crex_class_entry *scope, const char *name, size_t name_length, crex_long value) /* {{{ */
{
	zval tmp;

	ZVAL_BOOL(&tmp, value);
	return crex_update_static_property(scope, name, name_length, &tmp);
}
/* }}} */

CREX_API crex_result crex_update_static_property_long(crex_class_entry *scope, const char *name, size_t name_length, crex_long value) /* {{{ */
{
	zval tmp;

	ZVAL_LONG(&tmp, value);
	return crex_update_static_property(scope, name, name_length, &tmp);
}
/* }}} */

CREX_API crex_result crex_update_static_property_double(crex_class_entry *scope, const char *name, size_t name_length, double value) /* {{{ */
{
	zval tmp;

	ZVAL_DOUBLE(&tmp, value);
	return crex_update_static_property(scope, name, name_length, &tmp);
}
/* }}} */

CREX_API crex_result crex_update_static_property_string(crex_class_entry *scope, const char *name, size_t name_length, const char *value) /* {{{ */
{
	zval tmp;

	ZVAL_STRING(&tmp, value);
	C_SET_REFCOUNT(tmp, 0);
	return crex_update_static_property(scope, name, name_length, &tmp);
}
/* }}} */

CREX_API crex_result crex_update_static_property_stringl(crex_class_entry *scope, const char *name, size_t name_length, const char *value, size_t value_len) /* {{{ */
{
	zval tmp;

	ZVAL_STRINGL(&tmp, value, value_len);
	C_SET_REFCOUNT(tmp, 0);
	return crex_update_static_property(scope, name, name_length, &tmp);
}
/* }}} */

CREX_API zval *crex_read_property_ex(crex_class_entry *scope, crex_object *object, crex_string *name, bool silent, zval *rv) /* {{{ */
{
	zval *value;
	crex_class_entry *old_scope = EG(fake_scope);

	EG(fake_scope) = scope;

	value = object->handlers->read_property(object, name, silent?BP_VAR_IS:BP_VAR_R, NULL, rv);

	EG(fake_scope) = old_scope;
	return value;
}
/* }}} */

CREX_API zval *crex_read_property(crex_class_entry *scope, crex_object *object, const char *name, size_t name_length, bool silent, zval *rv) /* {{{ */
{
	zval *value;
	crex_string *str;

	str = crex_string_init(name, name_length, 0);
	value = crex_read_property_ex(scope, object, str, silent, rv);
	crex_string_release_ex(str, 0);
	return value;
}
/* }}} */

CREX_API zval *crex_read_static_property_ex(crex_class_entry *scope, crex_string *name, bool silent) /* {{{ */
{
	zval *property;
	crex_class_entry *old_scope = EG(fake_scope);

	EG(fake_scope) = scope;
	property = crex_std_get_static_property(scope, name, silent ? BP_VAR_IS : BP_VAR_R);
	EG(fake_scope) = old_scope;

	return property;
}
/* }}} */

CREX_API zval *crex_read_static_property(crex_class_entry *scope, const char *name, size_t name_length, bool silent) /* {{{ */
{
	crex_string *key = crex_string_init(name, name_length, 0);
	zval *property = crex_read_static_property_ex(scope, key, silent);
	crex_string_efree(key);
	return property;
}
/* }}} */

CREX_API void crex_save_error_handling(crex_error_handling *current) /* {{{ */
{
	current->handling = EG(error_handling);
	current->exception = EG(exception_class);
}
/* }}} */

CREX_API void crex_replace_error_handling(crex_error_handling_t error_handling, crex_class_entry *exception_class, crex_error_handling *current) /* {{{ */
{
	if (current) {
		crex_save_error_handling(current);
	}
	CREX_ASSERT(error_handling == EH_THROW || exception_class == NULL);
	EG(error_handling) = error_handling;
	EG(exception_class) = exception_class;
}
/* }}} */

CREX_API void crex_restore_error_handling(crex_error_handling *saved) /* {{{ */
{
	EG(error_handling) = saved->handling;
	EG(exception_class) = saved->exception;
}
/* }}} */

CREX_API CREX_COLD const char *crex_get_object_type_case(const crex_class_entry *ce, bool upper_case) /* {{{ */
{
	if (ce->ce_flags & CREX_ACC_TRAIT) {
		return upper_case ? "Trait" : "trait";
	} else if (ce->ce_flags & CREX_ACC_INTERFACE) {
		return upper_case ? "Interface" : "interface";
	} else if (ce->ce_flags & CREX_ACC_ENUM) {
		return upper_case ? "Enum" : "enum";
	} else {
		return upper_case ? "Class" : "class";
	}
}
/* }}} */

CREX_API bool crex_is_iterable(const zval *iterable) /* {{{ */
{
	switch (C_TYPE_P(iterable)) {
		case IS_ARRAY:
			return 1;
		case IS_OBJECT:
			return crex_class_implements_interface(C_OBJCE_P(iterable), crex_ce_traversable);
		default:
			return 0;
	}
}
/* }}} */

CREX_API bool crex_is_countable(const zval *countable) /* {{{ */
{
	switch (C_TYPE_P(countable)) {
		case IS_ARRAY:
			return 1;
		case IS_OBJECT:
			if (C_OBJ_HT_P(countable)->count_elements) {
				return 1;
			}

			return crex_class_implements_interface(C_OBJCE_P(countable), crex_ce_countable);
		default:
			return 0;
	}
}
/* }}} */

static crex_result get_default_via_ast(zval *default_value_zval, const char *default_value) {
	crex_ast *ast;
	crex_arena *ast_arena;

	crex_string *code = crex_string_concat3(
		"<?crx ", sizeof("<?crx ") - 1, default_value, strlen(default_value), ";", 1);

	ast = crex_compile_string_to_ast(code, &ast_arena, ZSTR_EMPTY_ALLOC());
	crex_string_release(code);

	if (!ast) {
		return FAILURE;
	}

	crex_ast_list *statement_list = crex_ast_get_list(ast);
	crex_ast **const_expr_ast_ptr = &statement_list->child[0];

	crex_arena *original_ast_arena = CG(ast_arena);
	uint32_t original_compiler_options = CG(compiler_options);
	crex_file_context original_file_context;
	CG(ast_arena) = ast_arena;
	/* Disable constant substitution, to make getDefaultValueConstant() work. */
	CG(compiler_options) |= CREX_COMPILE_NO_CONSTANT_SUBSTITUTION | CREX_COMPILE_NO_PERSISTENT_CONSTANT_SUBSTITUTION;
	crex_file_context_begin(&original_file_context);
	crex_const_expr_to_zval(default_value_zval, const_expr_ast_ptr, /* allow_dynamic */ true);
	CG(ast_arena) = original_ast_arena;
	CG(compiler_options) = original_compiler_options;
	crex_file_context_end(&original_file_context);

	crex_ast_destroy(ast);
	crex_arena_destroy(ast_arena);

	return SUCCESS;
}

static crex_string *try_parse_string(const char *str, size_t len, char quote) {
	if (len == 0) {
		return ZSTR_EMPTY_ALLOC();
	}

	for (size_t i = 0; i < len; i++) {
		if (str[i] == '\\' || str[i] == quote) {
			return NULL;
		}
	}
	return crex_string_init(str, len, 0);
}

CREX_API crex_result crex_get_default_from_internal_arg_info(
		zval *default_value_zval, crex_internal_arg_info *arg_info)
{
	const char *default_value = arg_info->default_value;
	if (!default_value) {
		return FAILURE;
	}

	/* Avoid going through the full AST machinery for some simple and common cases. */
	size_t default_value_len = strlen(default_value);
	crex_ulong lval;
	if (default_value_len == sizeof("null")-1
			&& !memcmp(default_value, "null", sizeof("null")-1)) {
		ZVAL_NULL(default_value_zval);
		return SUCCESS;
	} else if (default_value_len == sizeof("true")-1
			&& !memcmp(default_value, "true", sizeof("true")-1)) {
		ZVAL_TRUE(default_value_zval);
		return SUCCESS;
	} else if (default_value_len == sizeof("false")-1
			&& !memcmp(default_value, "false", sizeof("false")-1)) {
		ZVAL_FALSE(default_value_zval);
		return SUCCESS;
	} else if (default_value_len >= 2
			&& (default_value[0] == '\'' || default_value[0] == '"')
			&& default_value[default_value_len - 1] == default_value[0]) {
		crex_string *str = try_parse_string(
			default_value + 1, default_value_len - 2, default_value[0]);
		if (str) {
			ZVAL_STR(default_value_zval, str);
			return SUCCESS;
		}
	} else if (default_value_len == sizeof("[]")-1
			&& !memcmp(default_value, "[]", sizeof("[]")-1)) {
		ZVAL_EMPTY_ARRAY(default_value_zval);
		return SUCCESS;
	} else if (CREX_HANDLE_NUMERIC_STR(default_value, default_value_len, lval)) {
		ZVAL_LONG(default_value_zval, lval);
		return SUCCESS;
	}

#if 0
	fprintf(stderr, "Evaluating %s via AST\n", default_value);
#endif
	return get_default_via_ast(default_value_zval, default_value);
}
