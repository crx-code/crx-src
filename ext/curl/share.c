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
   | Author: Pierrick Charron <pierrick@crx.net>                          |
   +----------------------------------------------------------------------+
*/

#define CREX_INCLUDE_FULL_WINDOWS_HEADERS

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"

#include "curl_private.h"

#include <curl/curl.h>

#define SAVE_CURLSH_ERROR(__handle, __err) (__handle)->err.no = (int) __err;

/* {{{ Initialize a share curl handle */
CRX_FUNCTION(curl_share_init)
{
	crx_curlsh *sh;

	CREX_PARSE_PARAMETERS_NONE();

	object_init_ex(return_value, curl_share_ce);
	sh = C_CURL_SHARE_P(return_value);

	sh->share = curl_share_init();
}
/* }}} */

/* {{{ Close a set of cURL handles */
CRX_FUNCTION(curl_share_close)
{
	zval *z_sh;

	CREX_PARSE_PARAMETERS_START(1,1)
		C_PARAM_OBJECT_OF_CLASS(z_sh, curl_share_ce)
	CREX_PARSE_PARAMETERS_END();
}
/* }}} */

static bool _crx_curl_share_setopt(crx_curlsh *sh, crex_long option, zval *zvalue, zval *return_value) /* {{{ */
{
	CURLSHcode error = CURLSHE_OK;

	switch (option) {
		case CURLSHOPT_SHARE:
		case CURLSHOPT_UNSHARE:
			error = curl_share_setopt(sh->share, option, zval_get_long(zvalue));
			break;

		default:
			crex_argument_value_error(2, "is not a valid cURL share option");
			error = CURLSHE_BAD_OPTION;
			break;
	}

	SAVE_CURLSH_ERROR(sh, error);

	return error == CURLSHE_OK;
}
/* }}} */

/* {{{ Set an option for a cURL transfer */
CRX_FUNCTION(curl_share_setopt)
{
	zval       *z_sh, *zvalue;
	crex_long        options;
	crx_curlsh *sh;

	CREX_PARSE_PARAMETERS_START(3,3)
		C_PARAM_OBJECT_OF_CLASS(z_sh, curl_share_ce)
		C_PARAM_LONG(options)
		C_PARAM_ZVAL(zvalue)
	CREX_PARSE_PARAMETERS_END();

	sh = C_CURL_SHARE_P(z_sh);

	if (_crx_curl_share_setopt(sh, options, zvalue, return_value)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Return an integer containing the last share curl error number */
CRX_FUNCTION(curl_share_errno)
{
	zval        *z_sh;
	crx_curlsh  *sh;

	CREX_PARSE_PARAMETERS_START(1,1)
		C_PARAM_OBJECT_OF_CLASS(z_sh, curl_share_ce)
	CREX_PARSE_PARAMETERS_END();

	sh = C_CURL_SHARE_P(z_sh);

	RETURN_LONG(sh->err.no);
}
/* }}} */


/* {{{ return string describing error code */
CRX_FUNCTION(curl_share_strerror)
{
	crex_long code;
	const char *str;

	CREX_PARSE_PARAMETERS_START(1,1)
		C_PARAM_LONG(code)
	CREX_PARSE_PARAMETERS_END();

	str = curl_share_strerror(code);
	if (str) {
		RETURN_STRING(str);
	} else {
		RETURN_NULL();
	}
}
/* }}} */

/* CurlShareHandle class */

static crex_object *curl_share_create_object(crex_class_entry *class_type) {
	crx_curlsh *intern = crex_object_alloc(sizeof(crx_curlsh), class_type);

	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	return &intern->std;
}

static crex_function *curl_share_get_constructor(crex_object *object) {
	crex_throw_error(NULL, "Cannot directly construct CurlShareHandle, use curl_share_init() instead");
	return NULL;
}

void curl_share_free_obj(crex_object *object)
{
	crx_curlsh *sh = curl_share_from_obj(object);

	curl_share_cleanup(sh->share);
	crex_object_std_dtor(&sh->std);
}

static crex_object_handlers curl_share_handlers;

void curl_share_register_handlers(void) {
	curl_share_ce->create_object = curl_share_create_object;
	curl_share_ce->default_object_handlers = &curl_share_handlers;

	memcpy(&curl_share_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	curl_share_handlers.offset = XtOffsetOf(crx_curlsh, std);
	curl_share_handlers.free_obj = curl_share_free_obj;
	curl_share_handlers.get_constructor = curl_share_get_constructor;
	curl_share_handlers.clone_obj = NULL;
	curl_share_handlers.compare = crex_objects_not_comparable;
}
