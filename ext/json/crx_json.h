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

#ifndef CRX_JSON_H
#define CRX_JSON_H

#include "crx_version.h"
#include "crex_smart_str_public.h"

#define CRX_JSON_VERSION CRX_VERSION

extern crex_module_entry json_module_entry;
#define crxext_json_ptr &json_module_entry

#if defined(CRX_WIN32) && defined(JSON_EXPORTS)
#define CRX_JSON_API __declspec(dllexport)
#else
#define CRX_JSON_API CRXAPI
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

extern CRX_JSON_API crex_class_entry *crx_json_serializable_ce;

/* error codes */
typedef enum {
	CRX_JSON_ERROR_NONE = 0,
	CRX_JSON_ERROR_DEPTH,
	CRX_JSON_ERROR_STATE_MISMATCH,
	CRX_JSON_ERROR_CTRL_CHAR,
	CRX_JSON_ERROR_SYNTAX,
	CRX_JSON_ERROR_UTF8,
	CRX_JSON_ERROR_RECURSION,
	CRX_JSON_ERROR_INF_OR_NAN,
	CRX_JSON_ERROR_UNSUPPORTED_TYPE,
	CRX_JSON_ERROR_INVALID_PROPERTY_NAME,
	CRX_JSON_ERROR_UTF16,
	CRX_JSON_ERROR_NON_BACKED_ENUM,
} crx_json_error_code;

/* json_decode() options */
#define CRX_JSON_OBJECT_AS_ARRAY            (1<<0)
#define CRX_JSON_BIGINT_AS_STRING           (1<<1)

/* json_encode() options */
#define CRX_JSON_HEX_TAG                    (1<<0)
#define CRX_JSON_HEX_AMP                    (1<<1)
#define CRX_JSON_HEX_APOS                   (1<<2)
#define CRX_JSON_HEX_QUOT                   (1<<3)
#define CRX_JSON_FORCE_OBJECT               (1<<4)
#define CRX_JSON_NUMERIC_CHECK              (1<<5)
#define CRX_JSON_UNESCAPED_SLASHES          (1<<6)
#define CRX_JSON_PRETTY_PRINT               (1<<7)
#define CRX_JSON_UNESCAPED_UNICODE          (1<<8)
#define CRX_JSON_PARTIAL_OUTPUT_ON_ERROR    (1<<9)
#define CRX_JSON_PRESERVE_ZERO_FRACTION     (1<<10)
#define CRX_JSON_UNESCAPED_LINE_TERMINATORS (1<<11)

/* json_validate(), json_decode() and json_encode() common options */
#define CRX_JSON_INVALID_UTF8_IGNORE        (1<<20)

/* json_decode() and json_encode() common options */
#define CRX_JSON_INVALID_UTF8_SUBSTITUTE    (1<<21)
#define CRX_JSON_THROW_ON_ERROR             (1<<22)

/* Internal flags */
#define CRX_JSON_OUTPUT_ARRAY	0
#define CRX_JSON_OUTPUT_OBJECT	1

/* default depth */
#define CRX_JSON_PARSER_DEFAULT_DEPTH 512

CREX_BEGIN_MODULE_GLOBALS(json)
	int encoder_depth;
	int encode_max_depth;
	crx_json_error_code error_code;
CREX_END_MODULE_GLOBALS(json)

CRX_JSON_API CREX_EXTERN_MODULE_GLOBALS(json)
#define JSON_G(v) CREX_MODULE_GLOBALS_ACCESSOR(json, v)

#if defined(ZTS) && defined(COMPILE_DL_JSON)
CREX_TSRMLS_CACHE_EXTERN()
#endif

CRX_JSON_API crex_string *crx_json_encode_string(const char *s, size_t len, int options);

CRX_JSON_API crex_result crx_json_encode_ex(smart_str *buf, zval *val, int options, crex_long depth);
CRX_JSON_API crex_result crx_json_encode(smart_str *buf, zval *val, int options);
CRX_JSON_API crex_result crx_json_decode_ex(zval *return_value, const char *str, size_t str_len, crex_long options, crex_long depth);
CRX_JSON_API bool crx_json_validate_ex(const char *str, size_t str_len, crex_long options, crex_long depth);

static inline crex_result crx_json_decode(zval *return_value, const char *str, size_t str_len, bool assoc, crex_long depth)
{
	return crx_json_decode_ex(return_value, str, str_len, assoc ? CRX_JSON_OBJECT_AS_ARRAY : 0, depth);
}

#endif  /* CRX_JSON_H */
