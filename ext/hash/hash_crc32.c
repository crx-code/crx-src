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
  | Authors: Michael Wallner <mike@crx.net>                              |
  |          Sara Golemon <pollita@crx.net>                              |
  +----------------------------------------------------------------------+
*/

#include "crx_hash.h"
#include "crx_hash_crc32.h"
#include "crx_hash_crc32_tables.h"
#include "ext/standard/crc32_x86.h"

CRX_HASH_API void CRX_CRC32Init(CRX_CRC32_CTX *context, CREX_ATTRIBUTE_UNUSED HashTable *args)
{
	context->state = ~0;
}

CRX_HASH_API void CRX_CRC32Update(CRX_CRC32_CTX *context, const unsigned char *input, size_t len)
{
	size_t i = 0;

#if CREX_INTRIN_SSE4_2_PCLMUL_NATIVE || CREX_INTRIN_SSE4_2_PCLMUL_RESOLVER
	i += crc32_x86_simd_update(X86_CRC32, &context->state, input, len);
#endif

	for (; i < len; ++i) {
		context->state = (context->state << 8) ^ crc32_table[(context->state >> 24) ^ (input[i] & 0xff)];
	}
}

CRX_HASH_API void CRX_CRC32BUpdate(CRX_CRC32_CTX *context, const unsigned char *input, size_t len)
{
	size_t i = 0;

#if CREX_INTRIN_SSE4_2_PCLMUL_NATIVE || CREX_INTRIN_SSE4_2_PCLMUL_RESOLVER
	i += crc32_x86_simd_update(X86_CRC32B, &context->state, input, len);
#endif

	for (; i < len; ++i) {
		context->state = (context->state >> 8) ^ crc32b_table[(context->state ^ input[i]) & 0xff];
	}
}

CRX_HASH_API void CRX_CRC32CUpdate(CRX_CRC32_CTX *context, const unsigned char *input, size_t len)
{
	size_t i = 0;

#if CREX_INTRIN_SSE4_2_PCLMUL_NATIVE || CREX_INTRIN_SSE4_2_PCLMUL_RESOLVER
	i += crc32_x86_simd_update(X86_CRC32C, &context->state, input, len);
#endif

	for (; i < len; ++i) {
		context->state = (context->state >> 8) ^ crc32c_table[(context->state ^ input[i]) & 0xff];
	}
}

CRX_HASH_API void CRX_CRC32LEFinal(unsigned char digest[4], CRX_CRC32_CTX *context)
{
	context->state=~context->state;
	digest[3] = (unsigned char) ((context->state >> 24) & 0xff);
	digest[2] = (unsigned char) ((context->state >> 16) & 0xff);
	digest[1] = (unsigned char) ((context->state >> 8) & 0xff);
	digest[0] = (unsigned char) (context->state & 0xff);
	context->state = 0;
}

CRX_HASH_API void CRX_CRC32BEFinal(unsigned char digest[4], CRX_CRC32_CTX *context)
{
	context->state=~context->state;
	digest[0] = (unsigned char) ((context->state >> 24) & 0xff);
	digest[1] = (unsigned char) ((context->state >> 16) & 0xff);
	digest[2] = (unsigned char) ((context->state >> 8) & 0xff);
	digest[3] = (unsigned char) (context->state & 0xff);
	context->state = 0;
}

CRX_HASH_API int CRX_CRC32Copy(const crx_hash_ops *ops, CRX_CRC32_CTX *orig_context, CRX_CRC32_CTX *copy_context)
{
	copy_context->state = orig_context->state;
	return SUCCESS;
}

const crx_hash_ops crx_hash_crc32_ops = {
	"crc32",
	(crx_hash_init_func_t) CRX_CRC32Init,
	(crx_hash_update_func_t) CRX_CRC32Update,
	(crx_hash_final_func_t) CRX_CRC32LEFinal,
	(crx_hash_copy_func_t) CRX_CRC32Copy,
	crx_hash_serialize,
	crx_hash_unserialize,
	CRX_CRC32_SPEC,
	4, /* what to say here? */
	4,
	sizeof(CRX_CRC32_CTX),
	0
};

const crx_hash_ops crx_hash_crc32b_ops = {
	"crc32b",
	(crx_hash_init_func_t) CRX_CRC32Init,
	(crx_hash_update_func_t) CRX_CRC32BUpdate,
	(crx_hash_final_func_t) CRX_CRC32BEFinal,
	(crx_hash_copy_func_t) CRX_CRC32Copy,
	crx_hash_serialize,
	crx_hash_unserialize,
	CRX_CRC32_SPEC,
	4, /* what to say here? */
	4,
	sizeof(CRX_CRC32_CTX),
	0
};

const crx_hash_ops crx_hash_crc32c_ops = {
	"crc32c",
	(crx_hash_init_func_t) CRX_CRC32Init,
	(crx_hash_update_func_t) CRX_CRC32CUpdate,
	(crx_hash_final_func_t) CRX_CRC32BEFinal,
	(crx_hash_copy_func_t) CRX_CRC32Copy,
	crx_hash_serialize,
	crx_hash_unserialize,
	CRX_CRC32_SPEC,
	4, /* what to say here? */
	4,
	sizeof(CRX_CRC32_CTX),
	0
};
