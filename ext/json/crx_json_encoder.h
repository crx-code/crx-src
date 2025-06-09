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
  | Author: Jakub Zelenka <bukka@crx.net>                                |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_JSON_ENCODER_H
#define	CRX_JSON_ENCODER_H

#include "crx.h"
#include "crex_smart_str.h"

typedef struct _crx_json_encoder crx_json_encoder;

struct _crx_json_encoder {
	int depth;
	int max_depth;
	crx_json_error_code error_code;
};

static inline void crx_json_encode_init(crx_json_encoder *encoder)
{
	memset(encoder, 0, sizeof(crx_json_encoder));
}

crex_result crx_json_encode_zval(smart_str *buf, zval *val, int options, crx_json_encoder *encoder);

crex_result crx_json_escape_string(smart_str *buf, const char *s, size_t len, int options, crx_json_encoder *encoder);

#endif	/* CRX_JSON_ENCODER_H */
