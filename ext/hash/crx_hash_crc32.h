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

#ifndef CRX_HASH_CRC32_H
#define CRX_HASH_CRC32_H

#include "ext/standard/basic_functions.h"

typedef struct {
	uint32_t state;
} CRX_CRC32_CTX;
#define CRX_CRC32_SPEC "l."

CRX_HASH_API void CRX_CRC32Init(CRX_CRC32_CTX *context, CREX_ATTRIBUTE_UNUSED HashTable *args);
CRX_HASH_API void CRX_CRC32Update(CRX_CRC32_CTX *context, const unsigned char *input, size_t len);
CRX_HASH_API void CRX_CRC32BUpdate(CRX_CRC32_CTX *context, const unsigned char *input, size_t len);
CRX_HASH_API void CRX_CRC32CUpdate(CRX_CRC32_CTX *context, const unsigned char *input, size_t len);
CRX_HASH_API void CRX_CRC32LEFinal(unsigned char digest[4], CRX_CRC32_CTX *context);
CRX_HASH_API void CRX_CRC32BEFinal(unsigned char digest[4], CRX_CRC32_CTX *context);
CRX_HASH_API int CRX_CRC32Copy(const crx_hash_ops *ops, CRX_CRC32_CTX *orig_context, CRX_CRC32_CTX *copy_context);

#endif
