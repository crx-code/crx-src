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

#ifndef CRX_HASH_XXHASH_H
#define CRX_HASH_XXHASH_H

#define XXH_INLINE_ALL 1
#include "xxhash.h"

typedef struct {
	XXH32_state_t s;
} CRX_XXH32_CTX;
#define CRX_XXH32_SPEC "llllllllllll"

CRX_HASH_API void CRX_XXH32Init(CRX_XXH32_CTX *ctx, HashTable *args);
CRX_HASH_API void CRX_XXH32Update(CRX_XXH32_CTX *ctx, const unsigned char *in, size_t len);
CRX_HASH_API void CRX_XXH32Final(unsigned char digest[4], CRX_XXH32_CTX *ctx);
CRX_HASH_API int CRX_XXH32Copy(const crx_hash_ops *ops, CRX_XXH32_CTX *orig_context, CRX_XXH32_CTX *copy_context);

typedef struct {
	XXH64_state_t s;
} CRX_XXH64_CTX;
#define CRX_XXH64_SPEC "qqqqqqqqqllq"

CRX_HASH_API void CRX_XXH64Init(CRX_XXH64_CTX *ctx, HashTable *args);
CRX_HASH_API void CRX_XXH64Update(CRX_XXH64_CTX *ctx, const unsigned char *in, size_t len);
CRX_HASH_API void CRX_XXH64Final(unsigned char digest[8], CRX_XXH64_CTX *ctx);
CRX_HASH_API int CRX_XXH64Copy(const crx_hash_ops *ops, CRX_XXH64_CTX *orig_context, CRX_XXH64_CTX *copy_context);

#define CRX_XXH3_SECRET_SIZE_MIN XXH3_SECRET_SIZE_MIN
#define CRX_XXH3_SECRET_SIZE_MAX 256

typedef struct {
	XXH3_state_t s;
	/* The value must survive the whole streaming cycle from init to final.

	   A more flexible mechanism would be to carry crex_string* passed through
	   the options. However, that will require to introduce a destructor
	   handler for ctx, so then it wolud be automatically called from the
	   object destructor. Until that is given, the viable way is to use a
	   plausible max secret length. */
	const unsigned char secret[CRX_XXH3_SECRET_SIZE_MAX];
} CRX_XXH3_CTX;

typedef CRX_XXH3_CTX CRX_XXH3_64_CTX;

CRX_HASH_API void CRX_XXH3_64_Init(CRX_XXH3_64_CTX *ctx, HashTable *args);
CRX_HASH_API void CRX_XXH3_64_Update(CRX_XXH3_64_CTX *ctx, const unsigned char *in, size_t len);
CRX_HASH_API void CRX_XXH3_64_Final(unsigned char digest[8], CRX_XXH3_64_CTX *ctx);
CRX_HASH_API int CRX_XXH3_64_Copy(const crx_hash_ops *ops, CRX_XXH3_64_CTX *orig_context, CRX_XXH3_64_CTX *copy_context);

typedef CRX_XXH3_CTX CRX_XXH3_128_CTX;

CRX_HASH_API void CRX_XXH3_128_Init(CRX_XXH3_128_CTX *ctx, HashTable *args);
CRX_HASH_API void CRX_XXH3_128_Update(CRX_XXH3_128_CTX *ctx, const unsigned char *in, size_t len);
CRX_HASH_API void CRX_XXH3_128_Final(unsigned char digest[16], CRX_XXH3_128_CTX *ctx);
CRX_HASH_API int CRX_XXH3_128_Copy(const crx_hash_ops *ops, CRX_XXH3_128_CTX *orig_context, CRX_XXH3_128_CTX *copy_context);

#endif /* CRX_HASH_XXHASH_H */

