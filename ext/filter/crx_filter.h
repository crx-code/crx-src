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
  | Authors: Rasmus Lerdorf <rasmus@crx.net>                             |
  |          Derick Rethans <derick@crx.net>                             |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_FILTER_H
#define CRX_FILTER_H

#include "SAPI.h"
#include "crex_API.h"
#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/crx_string.h"
#include "ext/standard/html.h"
#include "crx_variables.h"

extern crex_module_entry filter_module_entry;
#define crxext_filter_ptr &filter_module_entry

#ifdef ZTS
#include "TSRM.h"
#endif

#define CRX_FILTER_VERSION CRX_VERSION

CRX_MINIT_FUNCTION(filter);
CRX_MSHUTDOWN_FUNCTION(filter);
CRX_RINIT_FUNCTION(filter);
CRX_RSHUTDOWN_FUNCTION(filter);
CRX_MINFO_FUNCTION(filter);

CREX_BEGIN_MODULE_GLOBALS(filter)
	zval post_array;
	zval get_array;
	zval cookie_array;
	zval env_array;
	zval server_array;
#if 0
	zval session_array;
#endif
	crex_long default_filter;
	crex_long default_filter_flags;
CREX_END_MODULE_GLOBALS(filter)

#if defined(COMPILE_DL_FILTER) && defined(ZTS)
CREX_TSRMLS_CACHE_EXTERN()
#endif

#define IF_G(v) CREX_MODULE_GLOBALS_ACCESSOR(filter, v)

#define CRX_INPUT_FILTER_PARAM_DECL zval *value, crex_long flags, zval *option_array, char *charset
void crx_filter_int(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_boolean(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_float(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_validate_regexp(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_validate_domain(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_validate_url(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_validate_email(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_validate_ip(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_validate_mac(CRX_INPUT_FILTER_PARAM_DECL);

void crx_filter_string(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_encoded(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_special_chars(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_full_special_chars(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_unsafe_raw(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_email(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_url(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_number_int(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_number_float(CRX_INPUT_FILTER_PARAM_DECL);
void crx_filter_add_slashes(CRX_INPUT_FILTER_PARAM_DECL);

void crx_filter_callback(CRX_INPUT_FILTER_PARAM_DECL);

#endif	/* FILTER_H */
