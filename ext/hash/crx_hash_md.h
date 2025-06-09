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
   | Original Author: Rasmus Lerdorf <rasmus@lerdorf.on.ca>               |
   | Modified for pHASH by: Sara Golemon <pollita@crx.net>
   +----------------------------------------------------------------------+
*/

#ifndef CRX_HASH_MD_H
#define CRX_HASH_MD_H

#include "ext/standard/md5.h"

/* MD4 context */
typedef struct {
	uint32_t state[4];
	uint32_t count[2];
	unsigned char buffer[64];
} CRX_MD4_CTX;
#define CRX_MD4_SPEC "l4l2b64."

#define CRX_MD4Init(ctx) CRX_MD4InitArgs(ctx, NULL)
CRX_HASH_API void CRX_MD4InitArgs(CRX_MD4_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *);
CRX_HASH_API void CRX_MD4Update(CRX_MD4_CTX *context, const unsigned char *, size_t);
CRX_HASH_API void CRX_MD4Final(unsigned char[16], CRX_MD4_CTX *);

/* MD2 context */
typedef struct {
	unsigned char state[48];
	unsigned char checksum[16];
	unsigned char buffer[16];
	unsigned char in_buffer;
} CRX_MD2_CTX;
#define CRX_MD2_SPEC "b48b16b16b."

#define CRX_MD2Init(ctx) CRX_MD2InitArgs(ctx, NULL)
CRX_HASH_API void CRX_MD2InitArgs(CRX_MD2_CTX *context, CREX_ATTRIBUTE_UNUSED HashTable *args);
CRX_HASH_API void CRX_MD2Update(CRX_MD2_CTX *context, const unsigned char *, size_t);
CRX_HASH_API void CRX_MD2Final(unsigned char[16], CRX_MD2_CTX *);

#endif
