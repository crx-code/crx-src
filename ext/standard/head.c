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
   | Author: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                        |
   +----------------------------------------------------------------------+
 */

#include <stdio.h>
#include "crx.h"
#include "ext/standard/crx_standard.h"
#include "ext/date/crx_date.h"
#include "SAPI.h"
#include "crx_main.h"
#include "head.h"
#include <time.h>

#include "crx_globals.h"
#include "crex_smart_str.h"


/* Implementation of the language Header() function */
/* {{{ Sends a raw HTTP header */
CRX_FUNCTION(header)
{
	bool rep = 1;
	sapi_header_line ctr = {0};
	char *line;
	size_t len;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_STRING(line, len)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(rep)
		C_PARAM_LONG(ctr.response_code)
	CREX_PARSE_PARAMETERS_END();

	ctr.line = line;
	ctr.line_len = (uint32_t)len;
	sapi_header_op(rep ? SAPI_HEADER_REPLACE:SAPI_HEADER_ADD, &ctr);
}
/* }}} */

/* {{{ Removes an HTTP header previously set using header() */
CRX_FUNCTION(header_remove)
{
	sapi_header_line ctr = {0};
	char *line = NULL;
	size_t len = 0;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_STRING_OR_NULL(line, len)
	CREX_PARSE_PARAMETERS_END();

	ctr.line = line;
	ctr.line_len = (uint32_t)len;
	sapi_header_op(line == NULL ? SAPI_HEADER_DELETE_ALL : SAPI_HEADER_DELETE, &ctr);
}
/* }}} */

CRXAPI int crx_header(void)
{
	if (sapi_send_headers()==FAILURE || SG(request_info).headers_only) {
		return 0; /* don't allow output */
	} else {
		return 1; /* allow output */
	}
}

#define ILLEGAL_COOKIE_CHARACTER "\",\", \";\", \" \", \"\\t\", \"\\r\", \"\\n\", \"\\013\", or \"\\014\""
CRXAPI crex_result crx_setcookie(crex_string *name, crex_string *value, time_t expires,
	crex_string *path, crex_string *domain, bool secure, bool httponly,
	crex_string *samesite, bool url_encode)
{
	crex_string *dt;
	sapi_header_line ctr = {0};
	crex_result result;
	smart_str buf = {0};

	if (!ZSTR_LEN(name)) {
		crex_argument_value_error(1, "cannot be empty");
		return FAILURE;
	}
	if (strpbrk(ZSTR_VAL(name), "=,; \t\r\n\013\014") != NULL) {   /* man isspace for \013 and \014 */
		crex_argument_value_error(1, "cannot contain \"=\", " ILLEGAL_COOKIE_CHARACTER);
		return FAILURE;
	}
	if (!url_encode && value &&
			strpbrk(ZSTR_VAL(value), ",; \t\r\n\013\014") != NULL) { /* man isspace for \013 and \014 */
		crex_argument_value_error(2, "cannot contain " ILLEGAL_COOKIE_CHARACTER);
		return FAILURE;
	}

	if (path && strpbrk(ZSTR_VAL(path), ",; \t\r\n\013\014") != NULL) { /* man isspace for \013 and \014 */
		crex_value_error("%s(): \"path\" option cannot contain " ILLEGAL_COOKIE_CHARACTER,
			get_active_function_name());
		return FAILURE;
	}
	if (domain && strpbrk(ZSTR_VAL(domain), ",; \t\r\n\013\014") != NULL) { /* man isspace for \013 and \014 */
		crex_value_error("%s(): \"domain\" option cannot contain " ILLEGAL_COOKIE_CHARACTER,
			get_active_function_name());
		return FAILURE;
	}
#ifdef CREX_ENABLE_ZVAL_LONG64
	if (expires >= 253402300800) {
		crex_value_error("%s(): \"expires\" option cannot have a year greater than 9999",
			get_active_function_name());
		return FAILURE;
	}
#endif

	/* Should check value of SameSite? */

	if (value == NULL || ZSTR_LEN(value) == 0) {
		/*
		 * MSIE doesn't delete a cookie when you set it to a null value
		 * so in order to force cookies to be deleted, even on MSIE, we
		 * pick an expiry date in the past
		 */
		dt = crx_format_date("D, d M Y H:i:s \\G\\M\\T", sizeof("D, d M Y H:i:s \\G\\M\\T")-1, 1, 0);
		smart_str_appends(&buf, "Set-Cookie: ");
		smart_str_append(&buf, name);
		smart_str_appends(&buf, "=deleted; expires=");
		smart_str_append(&buf, dt);
		smart_str_appends(&buf, "; Max-Age=0");
		crex_string_free(dt);
	} else {
		smart_str_appends(&buf, "Set-Cookie: ");
		smart_str_append(&buf, name);
		smart_str_appendc(&buf, '=');
		if (url_encode) {
			crex_string *encoded_value = crx_raw_url_encode(ZSTR_VAL(value), ZSTR_LEN(value));
			smart_str_append(&buf, encoded_value);
			crex_string_release_ex(encoded_value, 0);
		} else {
			smart_str_append(&buf, value);
		}

		if (expires > 0) {
			double diff;

			smart_str_appends(&buf, COOKIE_EXPIRES);
			dt = crx_format_date("D, d M Y H:i:s \\G\\M\\T", sizeof("D, d M Y H:i:s \\G\\M\\T")-1, expires, 0);

			smart_str_append(&buf, dt);
			crex_string_free(dt);

			diff = difftime(expires, crx_time());
			if (diff < 0) {
				diff = 0;
			}

			smart_str_appends(&buf, COOKIE_MAX_AGE);
			smart_str_append_long(&buf, (crex_long) diff);
		}
	}

	if (path && ZSTR_LEN(path)) {
		smart_str_appends(&buf, COOKIE_PATH);
		smart_str_append(&buf, path);
	}
	if (domain && ZSTR_LEN(domain)) {
		smart_str_appends(&buf, COOKIE_DOMAIN);
		smart_str_append(&buf, domain);
	}
	if (secure) {
		smart_str_appends(&buf, COOKIE_SECURE);
	}
	if (httponly) {
		smart_str_appends(&buf, COOKIE_HTTPONLY);
	}
	if (samesite && ZSTR_LEN(samesite)) {
		smart_str_appends(&buf, COOKIE_SAMESITE);
		smart_str_append(&buf, samesite);
	}

	ctr.line = ZSTR_VAL(buf.s);
	ctr.line_len = (uint32_t) ZSTR_LEN(buf.s);

	result = sapi_header_op(SAPI_HEADER_ADD, &ctr);
	crex_string_release(buf.s);
	return result;
}

static crex_result crx_head_parse_cookie_options_array(HashTable *options, crex_long *expires, crex_string **path,
		crex_string **domain, bool *secure, bool *httponly, crex_string **samesite)
{
	crex_string *key;
	zval *value;

	CREX_HASH_FOREACH_STR_KEY_VAL(options, key, value) {
		if (!key) {
			crex_value_error("%s(): option array cannot have numeric keys", get_active_function_name());
			return FAILURE;
		}
		if (crex_string_equals_literal_ci(key, "expires")) {
			*expires = zval_get_long(value);
		} else if (crex_string_equals_literal_ci(key, "path")) {
			*path = zval_get_string(value);
		} else if (crex_string_equals_literal_ci(key, "domain")) {
			*domain = zval_get_string(value);
		} else if (crex_string_equals_literal_ci(key, "secure")) {
			*secure = zval_is_true(value);
		} else if (crex_string_equals_literal_ci(key, "httponly")) {
			*httponly = zval_is_true(value);
		} else if (crex_string_equals_literal_ci(key, "samesite")) {
			*samesite = zval_get_string(value);
		} else {
			crex_value_error("%s(): option \"%s\" is invalid", get_active_function_name(), ZSTR_VAL(key));
			return FAILURE;
		}
	} CREX_HASH_FOREACH_END();
	return SUCCESS;
}

static void crx_setcookie_common(INTERNAL_FUNCTION_PARAMETERS, bool is_raw)
{
	HashTable *options = NULL;
	crex_long expires = 0;
	crex_string *name, *value = NULL, *path = NULL, *domain = NULL, *samesite = NULL;
	bool secure = 0, httponly = 0;

	CREX_PARSE_PARAMETERS_START(1, 7)
		C_PARAM_STR(name)
		C_PARAM_OPTIONAL
		C_PARAM_STR(value)
		C_PARAM_ARRAY_HT_OR_LONG(options, expires)
		C_PARAM_STR(path)
		C_PARAM_STR(domain)
		C_PARAM_BOOL(secure)
		C_PARAM_BOOL(httponly)
	CREX_PARSE_PARAMETERS_END();

	if (options) {
		if (UNEXPECTED(CREX_NUM_ARGS() > 3)) {
			crex_argument_count_error("%s(): Expects exactly 3 arguments when argument #3 "
				"($expires_or_options) is an array", get_active_function_name());
			RETURN_THROWS();
		}

		if (FAILURE == crx_head_parse_cookie_options_array(options, &expires, &path,
			&domain, &secure, &httponly, &samesite)
		) {
			goto cleanup;
		}
	}

	if (crx_setcookie(name, value, expires, path, domain, secure, httponly, samesite, !is_raw) == SUCCESS) {
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}

	if (options) {
cleanup:
		if (path) {
			crex_string_release(path);
		}
		if (domain) {
			crex_string_release(domain);
		}
		if (samesite) {
			crex_string_release(samesite);
		}
	}
}

/* {{{ setcookie(string name [, string value [, array options]])
   Send a cookie */
CRX_FUNCTION(setcookie)
{
	crx_setcookie_common(INTERNAL_FUNCTION_PARAM_PASSTHRU, false);
}
/* }}} */

/* {{{ setrawcookie(string name [, string value [, array options]])
   Send a cookie with no url encoding of the value */
CRX_FUNCTION(setrawcookie)
{
	crx_setcookie_common(INTERNAL_FUNCTION_PARAM_PASSTHRU, true);
}
/* }}} */


/* {{{ Returns true if headers have already been sent, false otherwise */
CRX_FUNCTION(headers_sent)
{
	zval *arg1 = NULL, *arg2 = NULL;
	const char *file="";
	int line=0;

	CREX_PARSE_PARAMETERS_START(0, 2)
		C_PARAM_OPTIONAL
		C_PARAM_ZVAL(arg1)
		C_PARAM_ZVAL(arg2)
	CREX_PARSE_PARAMETERS_END();

	if (SG(headers_sent)) {
		line = crx_output_get_start_lineno();
		file = crx_output_get_start_filename();
	}

	switch(CREX_NUM_ARGS()) {
	case 2:
		CREX_TRY_ASSIGN_REF_LONG(arg2, line);
		CREX_FALLTHROUGH;
	case 1:
		if (file) {
			CREX_TRY_ASSIGN_REF_STRING(arg1, file);
		} else {
			CREX_TRY_ASSIGN_REF_EMPTY_STRING(arg1);
		}
		break;
	}

	if (SG(headers_sent)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ crx_head_apply_header_list_to_hash
   Turn an llist of sapi_header_struct headers into a numerically indexed zval hash */
static void crx_head_apply_header_list_to_hash(void *data, void *arg)
{
	sapi_header_struct *sapi_header = (sapi_header_struct *)data;

	if (arg && sapi_header) {
		add_next_index_string((zval *)arg, (char *)(sapi_header->header));
	}
}

/* {{{ Return list of headers to be sent / already sent */
CRX_FUNCTION(headers_list)
{
	CREX_PARSE_PARAMETERS_NONE();

	array_init(return_value);
	crex_llist_apply_with_argument(&SG(sapi_headers).headers, crx_head_apply_header_list_to_hash, return_value);
}
/* }}} */

/* {{{ Sets a response code, or returns the current HTTP response code */
CRX_FUNCTION(http_response_code)
{
	crex_long response_code = 0;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(response_code)
	CREX_PARSE_PARAMETERS_END();

	if (response_code)
	{
		if (SG(headers_sent) && !SG(request_info).no_headers) {
			const char *output_start_filename = crx_output_get_start_filename();
			int output_start_lineno = crx_output_get_start_lineno();

			if (output_start_filename) {
				crx_error_docref(NULL, E_WARNING, "Cannot set response code - headers already sent "
					"(output started at %s:%d)", output_start_filename, output_start_lineno);
			} else {
				crx_error_docref(NULL, E_WARNING, "Cannot set response code - headers already sent");
			}
			RETURN_FALSE;
		}
		crex_long old_response_code;

		old_response_code = SG(sapi_headers).http_response_code;
		SG(sapi_headers).http_response_code = (int)response_code;

		if (old_response_code) {
			RETURN_LONG(old_response_code);
		}

		RETURN_TRUE;
	}

	if (!SG(sapi_headers).http_response_code) {
		RETURN_FALSE;
	}

	RETURN_LONG(SG(sapi_headers).http_response_code);
}
/* }}} */
