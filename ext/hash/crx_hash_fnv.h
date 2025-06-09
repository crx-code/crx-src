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
  | Author: Michael Maclean <mgdm@crx.net>                               |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_HASH_FNV_H
#define CRX_HASH_FNV_H

#define CRX_FNV1_32_INIT ((uint32_t)0x811c9dc5)

#define CRX_FNV_32_PRIME ((uint32_t)0x01000193)

#define CRX_FNV1_64_INIT ((uint64_t)0xcbf29ce484222325ULL)

#define CRX_FNV_64_PRIME ((uint64_t)0x100000001b3ULL)


/*
 * hash types
 */
enum crx_fnv_type {
	CRX_FNV_NONE  = 0,	/* invalid FNV hash type */
	CRX_FNV0_32   = 1,	/* FNV-0 32 bit hash */
	CRX_FNV1_32   = 2,	/* FNV-1 32 bit hash */
	CRX_FNV1a_32  = 3,	/* FNV-1a 32 bit hash */
	CRX_FNV0_64   = 4,	/* FNV-0 64 bit hash */
	CRX_FNV1_64   = 5,	/* FNV-1 64 bit hash */
	CRX_FNV1a_64  = 6,	/* FNV-1a 64 bit hash */
};

typedef struct {
	uint32_t state;
} CRX_FNV132_CTX;
#define CRX_FNV132_SPEC "l."

typedef struct {
	uint64_t state;
} CRX_FNV164_CTX;
#define CRX_FNV164_SPEC "q."


CRX_HASH_API void CRX_FNV132Init(CRX_FNV132_CTX *context, CREX_ATTRIBUTE_UNUSED HashTable *args);
CRX_HASH_API void CRX_FNV132Update(CRX_FNV132_CTX *context, const unsigned char *input, size_t inputLen);
CRX_HASH_API void CRX_FNV1a32Update(CRX_FNV132_CTX *context, const unsigned char *input, size_t inputLen);
CRX_HASH_API void CRX_FNV132Final(unsigned char digest[4], CRX_FNV132_CTX * context);

CRX_HASH_API void CRX_FNV164Init(CRX_FNV164_CTX *context, CREX_ATTRIBUTE_UNUSED HashTable *args);
CRX_HASH_API void CRX_FNV164Update(CRX_FNV164_CTX *context, const unsigned char *input, size_t inputLen);
CRX_HASH_API void CRX_FNV1a64Update(CRX_FNV164_CTX *context, const unsigned char *input, size_t inputLen);
CRX_HASH_API void CRX_FNV164Final(unsigned char digest[8], CRX_FNV164_CTX * context);

static uint32_t fnv_32_buf(void *buf, size_t len, uint32_t hval, int alternate);
static uint64_t fnv_64_buf(void *buf, size_t len, uint64_t hval, int alternate);

#endif
