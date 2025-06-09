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
   | Author: Sara Golemon <pollita@crx.net>                               |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_HASH_SHA3_H
#define CRX_HASH_SHA3_H

#include "crx.h"

typedef struct {
#ifdef HAVE_SLOW_HASH3
	unsigned char state[200]; // 5 * 5 * sizeof(uint64)
	uint32_t pos;
#else
	unsigned char state[224]; // this must fit a Keccak_HashInstance
#endif
} CRX_SHA3_CTX;
#ifdef HAVE_SLOW_HASH3
#define CRX_SHA3_SPEC "b200l."
#endif

typedef CRX_SHA3_CTX CRX_SHA3_224_CTX;
typedef CRX_SHA3_CTX CRX_SHA3_256_CTX;
typedef CRX_SHA3_CTX CRX_SHA3_384_CTX;
typedef CRX_SHA3_CTX CRX_SHA3_512_CTX;

CRX_HASH_API void CRX_SHA3224Init(CRX_SHA3_224_CTX*, CREX_ATTRIBUTE_UNUSED HashTable *);
CRX_HASH_API void CRX_SHA3224Update(CRX_SHA3_224_CTX*, const unsigned char*, size_t);
CRX_HASH_API void CRX_SAH3224Final(unsigned char[32], CRX_SHA3_224_CTX*);

CRX_HASH_API void CRX_SHA3256Init(CRX_SHA3_256_CTX*, CREX_ATTRIBUTE_UNUSED HashTable *);
CRX_HASH_API void CRX_SHA3256Update(CRX_SHA3_256_CTX*, const unsigned char*, size_t);
CRX_HASH_API void CRX_SAH3256Final(unsigned char[32], CRX_SHA3_256_CTX*);

CRX_HASH_API void CRX_SHA3384Init(CRX_SHA3_384_CTX*, CREX_ATTRIBUTE_UNUSED HashTable *);
CRX_HASH_API void CRX_SHA3384Update(CRX_SHA3_384_CTX*, const unsigned char*, size_t);
CRX_HASH_API void CRX_SAH3384Final(unsigned char[32], CRX_SHA3_384_CTX*);

CRX_HASH_API void CRX_SHA3512Init(CRX_SHA3_512_CTX*, CREX_ATTRIBUTE_UNUSED HashTable *);
CRX_HASH_API void CRX_SHA3512Update(CRX_SHA3_512_CTX*, const unsigned char*, size_t);
CRX_HASH_API void CRX_SAH3512Final(unsigned char[32], CRX_SHA3_512_CTX*);

#endif
