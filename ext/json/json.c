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
  | Author: Omar Kilani <omar@crx.net>                                   |
  |         Jakub Zelenka <bukka@crx.net>                                |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/html.h"
#include "crex_smart_str.h"
#include "crx_json.h"
#include "crx_json_encoder.h"
#include "crx_json_parser.h"
#include "json_arginfo.h"
#include <crex_exceptions.h>

static CRX_MINFO_FUNCTION(json);

CRX_JSON_API crex_class_entry *crx_json_serializable_ce;
CRX_JSON_API crex_class_entry *crx_json_exception_ce;

CRX_JSON_API CREX_DECLARE_MODULE_GLOBALS(json)

static int crx_json_implement_json_serializable(crex_class_entry *interface, crex_class_entry *class_type)
{
	class_type->ce_flags |= CREX_ACC_USE_GUARDS;
	return SUCCESS;
}

/* {{{ MINIT */
static CRX_MINIT_FUNCTION(json)
{
	crx_json_serializable_ce = register_class_JsonSerializable();
	crx_json_serializable_ce->interface_gets_implemented = crx_json_implement_json_serializable;

	crx_json_exception_ce = register_class_JsonException(crex_ce_exception);

	register_json_symbols(module_number);

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_GINIT_FUNCTION */
static CRX_GINIT_FUNCTION(json)
{
#if defined(COMPILE_DL_JSON) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	json_globals->encoder_depth = 0;
	json_globals->error_code = 0;
	json_globals->encode_max_depth = CRX_JSON_PARSER_DEFAULT_DEPTH;
}
/* }}} */

static CRX_RINIT_FUNCTION(json)
{
	JSON_G(error_code) = 0;
	return SUCCESS;
}

/* {{{ json_module_entry */
crex_module_entry json_module_entry = {
	STANDARD_MODULE_HEADER,
	"json",
	ext_functions,
	CRX_MINIT(json),
	NULL,
	CRX_RINIT(json),
	NULL,
	CRX_MINFO(json),
	CRX_JSON_VERSION,
	CRX_MODULE_GLOBALS(json),
	CRX_GINIT(json),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_JSON
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(json)
#endif

/* {{{ CRX_MINFO_FUNCTION */
static CRX_MINFO_FUNCTION(json)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "json support", "enabled");
	crx_info_print_table_end();
}
/* }}} */

CRX_JSON_API crex_string *crx_json_encode_string(const char *s, size_t len, int options)
{
	smart_str buf = {0};
	crx_json_encoder encoder;

	crx_json_encode_init(&encoder);

	if (crx_json_escape_string(&buf, s, len, options, &encoder) == FAILURE) {
		smart_str_free(&buf);
		return NULL;
	}

	return smart_str_extract(&buf);
}

CRX_JSON_API crex_result crx_json_encode_ex(smart_str *buf, zval *val, int options, crex_long depth) /* {{{ */
{
	crx_json_encoder encoder;
	crex_result return_code;

	crx_json_encode_init(&encoder);
	encoder.max_depth = depth;

	return_code = crx_json_encode_zval(buf, val, options, &encoder);
	JSON_G(error_code) = encoder.error_code;

	return return_code;
}
/* }}} */

CRX_JSON_API crex_result crx_json_encode(smart_str *buf, zval *val, int options) /* {{{ */
{
	return crx_json_encode_ex(buf, val, options, JSON_G(encode_max_depth));
}
/* }}} */

static const char *crx_json_get_error_msg(crx_json_error_code error_code) /* {{{ */
{
	switch(error_code) {
		case CRX_JSON_ERROR_NONE:
			return "No error";
		case CRX_JSON_ERROR_DEPTH:
			return "Maximum stack depth exceeded";
		case CRX_JSON_ERROR_STATE_MISMATCH:
			return "State mismatch (invalid or malformed JSON)";
		case CRX_JSON_ERROR_CTRL_CHAR:
			return "Control character error, possibly incorrectly encoded";
		case CRX_JSON_ERROR_SYNTAX:
			return "Syntax error";
		case CRX_JSON_ERROR_UTF8:
			return "Malformed UTF-8 characters, possibly incorrectly encoded";
		case CRX_JSON_ERROR_RECURSION:
			return "Recursion detected";
		case CRX_JSON_ERROR_INF_OR_NAN:
			return "Inf and NaN cannot be JSON encoded";
		case CRX_JSON_ERROR_UNSUPPORTED_TYPE:
			return "Type is not supported";
		case CRX_JSON_ERROR_INVALID_PROPERTY_NAME:
			return "The decoded property name is invalid";
		case CRX_JSON_ERROR_UTF16:
			return "Single unpaired UTF-16 surrogate in unicode escape";
		case CRX_JSON_ERROR_NON_BACKED_ENUM:
			return "Non-backed enums have no default serialization";
		default:
			return "Unknown error";
	}
}
/* }}} */

CRX_JSON_API crex_result crx_json_decode_ex(zval *return_value, const char *str, size_t str_len, crex_long options, crex_long depth) /* {{{ */
{
	crx_json_parser parser;

	crx_json_parser_init(&parser, return_value, str, str_len, (int)options, (int)depth);

	if (crx_json_yyparse(&parser)) {
		crx_json_error_code error_code = crx_json_parser_error_code(&parser);
		if (!(options & CRX_JSON_THROW_ON_ERROR)) {
			JSON_G(error_code) = error_code;
		} else {
			crex_throw_exception(crx_json_exception_ce, crx_json_get_error_msg(error_code), error_code);
		}
		RETVAL_NULL();
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ */
CRX_JSON_API bool crx_json_validate_ex(const char *str, size_t str_len, crex_long options, crex_long depth)
{
	crx_json_parser parser;
	zval tmp;
	const crx_json_parser_methods* parser_validate_methods = crx_json_get_validate_methods();
	crx_json_parser_init_ex(&parser, &tmp, str, str_len, (int)options, (int)depth, parser_validate_methods);

	if (crx_json_yyparse(&parser)) {
		crx_json_error_code error_code = crx_json_parser_error_code(&parser);
		JSON_G(error_code) = error_code;
		return false;
	}

	return true;
}
/* }}} */

/* {{{ Returns the JSON representation of a value */
CRX_FUNCTION(json_encode)
{
	zval *parameter;
	crx_json_encoder encoder;
	smart_str buf = {0};
	crex_long options = 0;
	crex_long depth = CRX_JSON_PARSER_DEFAULT_DEPTH;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_ZVAL(parameter)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(options)
		C_PARAM_LONG(depth)
	CREX_PARSE_PARAMETERS_END();

	crx_json_encode_init(&encoder);
	encoder.max_depth = (int)depth;
	crx_json_encode_zval(&buf, parameter, (int)options, &encoder);

	if (!(options & CRX_JSON_THROW_ON_ERROR) || (options & CRX_JSON_PARTIAL_OUTPUT_ON_ERROR)) {
		JSON_G(error_code) = encoder.error_code;
		if (encoder.error_code != CRX_JSON_ERROR_NONE && !(options & CRX_JSON_PARTIAL_OUTPUT_ON_ERROR)) {
			smart_str_free(&buf);
			RETURN_FALSE;
		}
	} else {
		if (encoder.error_code != CRX_JSON_ERROR_NONE) {
			smart_str_free(&buf);
			crex_throw_exception(crx_json_exception_ce, crx_json_get_error_msg(encoder.error_code), encoder.error_code);
			RETURN_THROWS();
		}
	}

	RETURN_STR(smart_str_extract(&buf));
}
/* }}} */

/* {{{ Decodes the JSON representation into a CRX value */
CRX_FUNCTION(json_decode)
{
	char *str;
	size_t str_len;
	bool assoc = 0; /* return JS objects as CRX objects by default */
	bool assoc_null = 1;
	crex_long depth = CRX_JSON_PARSER_DEFAULT_DEPTH;
	crex_long options = 0;

	CREX_PARSE_PARAMETERS_START(1, 4)
		C_PARAM_STRING(str, str_len)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL_OR_NULL(assoc, assoc_null)
		C_PARAM_LONG(depth)
		C_PARAM_LONG(options)
	CREX_PARSE_PARAMETERS_END();

	if (!(options & CRX_JSON_THROW_ON_ERROR)) {
		JSON_G(error_code) = CRX_JSON_ERROR_NONE;
	}

	if (!str_len) {
		if (!(options & CRX_JSON_THROW_ON_ERROR)) {
			JSON_G(error_code) = CRX_JSON_ERROR_SYNTAX;
		} else {
			crex_throw_exception(crx_json_exception_ce, crx_json_get_error_msg(CRX_JSON_ERROR_SYNTAX), CRX_JSON_ERROR_SYNTAX);
		}
		RETURN_NULL();
	}

	if (depth <= 0) {
		crex_argument_value_error(3, "must be greater than 0");
		RETURN_THROWS();
	}

	if (depth > INT_MAX) {
		crex_argument_value_error(3, "must be less than %d", INT_MAX);
		RETURN_THROWS();
	}

	/* For BC reasons, the bool $assoc overrides the long $options bit for CRX_JSON_OBJECT_AS_ARRAY */
	if (!assoc_null) {
		if (assoc) {
			options |=  CRX_JSON_OBJECT_AS_ARRAY;
		} else {
			options &= ~CRX_JSON_OBJECT_AS_ARRAY;
		}
	}

	crx_json_decode_ex(return_value, str, str_len, options, depth);
}
/* }}} */

/* {{{ Validates if a string contains a valid json */
CRX_FUNCTION(json_validate)
{
	char *str;
	size_t str_len;
	crex_long depth = CRX_JSON_PARSER_DEFAULT_DEPTH;
	crex_long options = 0;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_STRING(str, str_len)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(depth)
		C_PARAM_LONG(options)
	CREX_PARSE_PARAMETERS_END();


	if ((options != 0) && (options != CRX_JSON_INVALID_UTF8_IGNORE)) {
		crex_argument_value_error(3, "must be a valid flag (allowed flags: JSON_INVALID_UTF8_IGNORE)");
		RETURN_THROWS();
	}

	if (!str_len) {
		JSON_G(error_code) = CRX_JSON_ERROR_SYNTAX;
		RETURN_FALSE;
	}

	JSON_G(error_code) = CRX_JSON_ERROR_NONE;
	
	if (depth <= 0) {
		crex_argument_value_error(2, "must be greater than 0");
		RETURN_THROWS();
	}

	if (depth > INT_MAX) {
		crex_argument_value_error(2, "must be less than %d", INT_MAX);
		RETURN_THROWS();
	}

	RETURN_BOOL(crx_json_validate_ex(str, str_len, options, depth));
}
/* }}} */

/* {{{ Returns the error code of the last json_encode() or json_decode() call. */
CRX_FUNCTION(json_last_error)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_LONG(JSON_G(error_code));
}
/* }}} */

/* {{{ Returns the error string of the last json_encode() or json_decode() call. */
CRX_FUNCTION(json_last_error_msg)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_STRING(crx_json_get_error_msg(JSON_G(error_code)));
}
/* }}} */
