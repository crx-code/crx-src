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
   | Author: Michael Wallner <mike@crx.net>                               |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_HASH_ADLER32_H
#define CRX_HASH_ADLER32_H

#include "ext/standard/basic_functions.h"

typedef struct {
	uint32_t state;
} CRX_ADLER32_CTX;
#define CRX_ADLER32_SPEC "l."

CRX_HASH_API void CRX_ADLER32Init(CRX_ADLER32_CTX *context, CREX_ATTRIBUTE_UNUSED HashTable *args);
CRX_HASH_API void CRX_ADLER32Update(CRX_ADLER32_CTX *context, const unsigned char *input, size_t len);
CRX_HASH_API void CRX_ADLER32Final(unsigned char digest[4], CRX_ADLER32_CTX *context);
CRX_HASH_API int CRX_ADLER32Copy(const crx_hash_ops *ops, CRX_ADLER32_CTX *orig_context, CRX_ADLER32_CTX *copy_context);

#endif
