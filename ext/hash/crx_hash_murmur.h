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
   | Author: Anatol Belski <ab@crx.net>                                   |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_HASH_MURMUR_H
#define CRX_HASH_MURMUR_H

typedef struct {
	uint32_t h;
	uint32_t carry;
	uint32_t len;
} CRX_MURMUR3A_CTX;
#define CRX_MURMUR3A_SPEC "lll"

CRX_HASH_API void CRX_MURMUR3AInit(CRX_MURMUR3A_CTX *ctx, HashTable *args);
CRX_HASH_API void CRX_MURMUR3AUpdate(CRX_MURMUR3A_CTX *ctx, const unsigned char *in, size_t len);
CRX_HASH_API void CRX_MURMUR3AFinal(unsigned char digest[4], CRX_MURMUR3A_CTX *ctx);
CRX_HASH_API int CRX_MURMUR3ACopy(const crx_hash_ops *ops, CRX_MURMUR3A_CTX *orig_context, CRX_MURMUR3A_CTX *copy_context);

typedef struct {
	uint32_t h[4];
	uint32_t carry[4];
	uint32_t len;
} CRX_MURMUR3C_CTX;
#define CRX_MURMUR3C_SPEC "lllllllll"

CRX_HASH_API void CRX_MURMUR3CInit(CRX_MURMUR3C_CTX *ctx, HashTable *args);
CRX_HASH_API void CRX_MURMUR3CUpdate(CRX_MURMUR3C_CTX *ctx, const unsigned char *in, size_t len);
CRX_HASH_API void CRX_MURMUR3CFinal(unsigned char digest[16], CRX_MURMUR3C_CTX *ctx);
CRX_HASH_API int CRX_MURMUR3CCopy(const crx_hash_ops *ops, CRX_MURMUR3C_CTX *orig_context, CRX_MURMUR3C_CTX *copy_context);

typedef struct {
	uint64_t h[2];
	uint64_t carry[2];
	uint32_t len;
} CRX_MURMUR3F_CTX;
#define CRX_MURMUR3F_SPEC "qqqql"

CRX_HASH_API void CRX_MURMUR3FInit(CRX_MURMUR3F_CTX *ctx, HashTable *args);
CRX_HASH_API void CRX_MURMUR3FUpdate(CRX_MURMUR3F_CTX *ctx, const unsigned char *in, size_t len);
CRX_HASH_API void CRX_MURMUR3FFinal(unsigned char digest[16], CRX_MURMUR3F_CTX *ctx);
CRX_HASH_API int CRX_MURMUR3FCopy(const crx_hash_ops *ops, CRX_MURMUR3F_CTX *orig_context, CRX_MURMUR3F_CTX *copy_context);

#endif /* CRX_HASH_MURMUR_H */

