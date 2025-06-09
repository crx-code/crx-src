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
   | Authors: Sara Golemon <pollita@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_HTTP_H
#define CRX_HTTP_H

#include "crx.h"
#include "crex_types.h" /* for crex_string */
#include "crex_smart_str.h"

CRXAPI void crx_url_encode_hash_ex(HashTable *ht, smart_str *formstr,
				const char *num_prefix, size_t num_prefix_len,
				const crex_string *key_prefix,
				zval *type, const crex_string *arg_sep, int enc_type);

#endif
