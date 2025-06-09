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
  | Author: Martin Jansen <mj@crx.net>                                   |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_HASH_JOAAT_H
#define CRX_HASH_JOAAT_H

typedef struct {
	uint32_t state;
} CRX_JOAAT_CTX;
#define CRX_JOAAT_SPEC "l."

CRX_HASH_API void CRX_JOAATInit(CRX_JOAAT_CTX *context, CREX_ATTRIBUTE_UNUSED HashTable *args);
CRX_HASH_API void CRX_JOAATUpdate(CRX_JOAAT_CTX *context, const unsigned char *input, size_t inputLen);
CRX_HASH_API void CRX_JOAATFinal(unsigned char digest[4], CRX_JOAAT_CTX * context);

static uint32_t joaat_buf(void *buf, size_t len, uint32_t hval);

#endif
