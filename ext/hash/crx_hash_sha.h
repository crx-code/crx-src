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
   | SHA1 Author: Stefan Esser <sesser@crx.net>                           |
   | SHA256 Author: Sara Golemon <pollita@crx.net>                        |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_HASH_SHA_H
#define CRX_HASH_SHA_H

#include "ext/standard/sha1.h"
#include "ext/standard/basic_functions.h"

/* SHA224 context. */
typedef struct {
	uint32_t state[8];		/* state */
	uint32_t count[2];		/* number of bits, modulo 2^64 */
	unsigned char buffer[64];	/* input buffer */
} CRX_SHA224_CTX;
#define CRX_SHA224_SPEC "l8l2b64."

#define CRX_SHA224Init(ctx) CRX_SHA224InitArgs(ctx, NULL)
CRX_HASH_API void CRX_SHA224InitArgs(CRX_SHA224_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *);
CRX_HASH_API void CRX_SHA224Update(CRX_SHA224_CTX *, const unsigned char *, size_t);
CRX_HASH_API void CRX_SHA224Final(unsigned char[28], CRX_SHA224_CTX *);

/* SHA256 context. */
typedef struct {
	uint32_t state[8];		/* state */
	uint32_t count[2];		/* number of bits, modulo 2^64 */
	unsigned char buffer[64];	/* input buffer */
} CRX_SHA256_CTX;
#define CRX_SHA256_SPEC "l8l2b64."

#define CRX_SHA256Init(ctx) CRX_SHA256InitArgs(ctx, NULL)
CRX_HASH_API void CRX_SHA256InitArgs(CRX_SHA256_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *);
CRX_HASH_API void CRX_SHA256Update(CRX_SHA256_CTX *, const unsigned char *, size_t);
CRX_HASH_API void CRX_SHA256Final(unsigned char[32], CRX_SHA256_CTX *);

/* SHA384 context */
typedef struct {
	uint64_t state[8];	/* state */
	uint64_t count[2];	/* number of bits, modulo 2^128 */
	unsigned char buffer[128];	/* input buffer */
} CRX_SHA384_CTX;
#define CRX_SHA384_SPEC "q8q2b128."

#define CRX_SHA384Init(ctx) CRX_SHA384InitArgs(ctx, NULL)
CRX_HASH_API void CRX_SHA384InitArgs(CRX_SHA384_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *);
CRX_HASH_API void CRX_SHA384Update(CRX_SHA384_CTX *, const unsigned char *, size_t);
CRX_HASH_API void CRX_SHA384Final(unsigned char[48], CRX_SHA384_CTX *);

/* SHA512 context */
typedef struct {
	uint64_t state[8];	/* state */
	uint64_t count[2];	/* number of bits, modulo 2^128 */
	unsigned char buffer[128];	/* input buffer */
} CRX_SHA512_CTX;
#define CRX_SHA512_SPEC "q8q2b128."

#define CRX_SHA512Init(ctx) CRX_SHA512InitArgs(ctx, NULL)
CRX_HASH_API void CRX_SHA512InitArgs(CRX_SHA512_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *);
CRX_HASH_API void CRX_SHA512Update(CRX_SHA512_CTX *, const unsigned char *, size_t);
CRX_HASH_API void CRX_SHA512Final(unsigned char[64], CRX_SHA512_CTX *);

#define CRX_SHA512_256Init(ctx) CRX_SHA512_256InitArgs(ctx, NULL)
CRX_HASH_API void CRX_SHA512_256InitArgs(CRX_SHA512_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *);
#define CRX_SHA512_256Update CRX_SHA512Update
CRX_HASH_API void CRX_SHA512_256Final(unsigned char[32], CRX_SHA512_CTX *);

#define CRX_SHA512_224Init(ctx) CRX_SHA512_224InitArgs(ctx, NULL)
CRX_HASH_API void CRX_SHA512_224InitArgs(CRX_SHA512_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *);
#define CRX_SHA512_224Update CRX_SHA512Update
CRX_HASH_API void CRX_SHA512_224Final(unsigned char[28], CRX_SHA512_CTX *);

#endif /* CRX_HASH_SHA_H */
