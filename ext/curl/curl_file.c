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
   | Author: Stanislav Malyshev <stas@crx.net>                            |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "crx.h"
#include "Crex/crex_exceptions.h"
#include "curl_private.h"
#include "curl_file_arginfo.h"

CRX_CURL_API crex_class_entry *curl_CURLFile_class;
CRX_CURL_API crex_class_entry *curl_CURLStringFile_class;

static void curlfile_ctor(INTERNAL_FUNCTION_PARAMETERS)
{
	crex_string *fname, *mime = NULL, *postname = NULL;
	zval *cf = return_value;

	CREX_PARSE_PARAMETERS_START(1,3)
		C_PARAM_PATH_STR(fname)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(mime)
		C_PARAM_STR_OR_NULL(postname)
	CREX_PARSE_PARAMETERS_END();

	crex_update_property_str(curl_CURLFile_class, C_OBJ_P(cf), "name", sizeof("name")-1, fname);

	if (mime) {
		crex_update_property_str(curl_CURLFile_class, C_OBJ_P(cf), "mime", sizeof("mime")-1, mime);
	}

	if (postname) {
		crex_update_property_str(curl_CURLFile_class, C_OBJ_P(cf), "postname", sizeof("postname")-1, postname);
	}
}

/* {{{ Create the CURLFile object */
CREX_METHOD(CURLFile, __main)
{
	return_value = CREX_THIS;
	curlfile_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Create the CURLFile object */
CRX_FUNCTION(curl_file_create)
{
	object_init_ex( return_value, curl_CURLFile_class );
	curlfile_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

static void curlfile_get_property(const char *name, size_t name_len, INTERNAL_FUNCTION_PARAMETERS)
{
	zval *res, rv;

	CREX_PARSE_PARAMETERS_NONE();
	res = crex_read_property(curl_CURLFile_class, C_OBJ_P(CREX_THIS), name, name_len, 1, &rv);
	RETURN_COPY_DEREF(res);
}

static void curlfile_set_property(const char *name, size_t name_len, INTERNAL_FUNCTION_PARAMETERS)
{
	crex_string *arg;

	CREX_PARSE_PARAMETERS_START(1,1)
		C_PARAM_STR(arg)
	CREX_PARSE_PARAMETERS_END();

	crex_update_property_str(curl_CURLFile_class, C_OBJ_P(CREX_THIS), name, name_len, arg);
}

/* {{{ Get file name */
CREX_METHOD(CURLFile, getFilename)
{
	curlfile_get_property("name", sizeof("name")-1, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Get MIME type */
CREX_METHOD(CURLFile, getMimeType)
{
	curlfile_get_property("mime", sizeof("mime")-1, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Get file name for POST */
CREX_METHOD(CURLFile, getPostFilename)
{
	curlfile_get_property("postname", sizeof("postname")-1, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Set MIME type */
CREX_METHOD(CURLFile, setMimeType)
{
	curlfile_set_property("mime", sizeof("mime")-1, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ Set file name for POST */
CREX_METHOD(CURLFile, setPostFilename)
{
	curlfile_set_property("postname", sizeof("postname")-1, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

CREX_METHOD(CURLStringFile, __main)
{
	crex_string *data, *postname, *mime = NULL;
	zval *object;

	object = CREX_THIS;

	CREX_PARSE_PARAMETERS_START(2,3)
		C_PARAM_STR(data)
		C_PARAM_STR(postname)
		C_PARAM_OPTIONAL
		C_PARAM_STR(mime)
	CREX_PARSE_PARAMETERS_END();

	crex_update_property_str(curl_CURLStringFile_class, C_OBJ_P(object), "data", sizeof("data") - 1, data);
	crex_update_property_str(curl_CURLStringFile_class, C_OBJ_P(object), "postname", sizeof("postname")-1, postname);
	if (mime) {
		crex_update_property_str(curl_CURLStringFile_class, C_OBJ_P(object), "mime", sizeof("mime")-1, mime);
	} else {
		crex_update_property_string(curl_CURLStringFile_class, C_OBJ_P(object), "mime", sizeof("mime")-1, "application/octet-stream");
	}
}

void curlfile_register_class(void)
{
	curl_CURLFile_class = register_class_CURLFile();

	curl_CURLStringFile_class = register_class_CURLStringFile();
}
