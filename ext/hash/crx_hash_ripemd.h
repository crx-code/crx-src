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

#ifndef CRX_HASH_RIPEMD_H
#define CRX_HASH_RIPEMD_H
#include "ext/standard/basic_functions.h"

/* RIPEMD context. */
typedef struct {
	uint32_t state[4];		/* state (ABCD) */
	uint32_t count[2];		/* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];	/* input buffer */
} CRX_RIPEMD128_CTX;
#define CRX_RIPEMD128_SPEC "l4l2b64."

typedef struct {
	uint32_t state[5];		/* state (ABCD) */
	uint32_t count[2];		/* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];	/* input buffer */
} CRX_RIPEMD160_CTX;
#define CRX_RIPEMD160_SPEC "l5l2b64."

typedef struct {
	uint32_t state[8];		/* state (ABCD) */
	uint32_t count[2];		/* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];	/* input buffer */
} CRX_RIPEMD256_CTX;
#define CRX_RIPEMD256_SPEC "l8l2b64."

typedef struct {
	uint32_t state[10];		/* state (ABCD) */
	uint32_t count[2];		/* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];	/* input buffer */
} CRX_RIPEMD320_CTX;
#define CRX_RIPEMD320_SPEC "l10l2b64."

CRX_HASH_API void CRX_RIPEMD128Init(CRX_RIPEMD128_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *);
CRX_HASH_API void CRX_RIPEMD128Update(CRX_RIPEMD128_CTX *, const unsigned char *, size_t);
CRX_HASH_API void CRX_RIPEMD128Final(unsigned char[16], CRX_RIPEMD128_CTX *);

CRX_HASH_API void CRX_RIPEMD160Init(CRX_RIPEMD160_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *);
CRX_HASH_API void CRX_RIPEMD160Update(CRX_RIPEMD160_CTX *, const unsigned char *, size_t);
CRX_HASH_API void CRX_RIPEMD160Final(unsigned char[20], CRX_RIPEMD160_CTX *);

CRX_HASH_API void CRX_RIPEMD256Init(CRX_RIPEMD256_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *);
CRX_HASH_API void CRX_RIPEMD256Update(CRX_RIPEMD256_CTX *, const unsigned char *, size_t);
CRX_HASH_API void CRX_RIPEMD256Final(unsigned char[32], CRX_RIPEMD256_CTX *);

CRX_HASH_API void CRX_RIPEMD320Init(CRX_RIPEMD320_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *);
CRX_HASH_API void CRX_RIPEMD320Update(CRX_RIPEMD320_CTX *, const unsigned char *, size_t);
CRX_HASH_API void CRX_RIPEMD320Final(unsigned char[40], CRX_RIPEMD320_CTX *);

#endif /* CRX_HASH_RIPEMD_H */
