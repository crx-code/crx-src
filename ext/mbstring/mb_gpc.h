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
   | Author: Rui Hirokawa <hirokawa@crx.net>                              |
   |         Moriyoshi Koizumi <moriyoshi@crx.net>                        |
   +----------------------------------------------------------------------+
 */

/* {{{ includes */
#include "crx.h"
/* }}} */

/* {{{ typedefs */
typedef struct _crx_mb_encoding_handler_info_t {
	const char *separator;
	const mbfl_encoding *to_encoding;
	const mbfl_encoding **from_encodings;
	size_t num_from_encodings;
	int data_type;
	bool report_errors;
} crx_mb_encoding_handler_info_t;

/* }}}*/

/* {{{ prototypes */
SAPI_POST_HANDLER_FUNC(crx_mb_post_handler);
MBSTRING_API SAPI_TREAT_DATA_FUNC(mbstr_treat_data);

int _crx_mb_enable_encoding_translation(int flag);
const mbfl_encoding *_crx_mb_encoding_handler_ex(const crx_mb_encoding_handler_info_t *info, zval *arg, char *res);
/* }}} */
