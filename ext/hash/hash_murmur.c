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

#include "crx_hash.h"
#include "crx_hash_murmur.h"

#include "murmur/PMurHash.h"
#include "murmur/PMurHash128.h"


const crx_hash_ops crx_hash_murmur3a_ops = {
	"murmur3a",
	(crx_hash_init_func_t) CRX_MURMUR3AInit,
	(crx_hash_update_func_t) CRX_MURMUR3AUpdate,
	(crx_hash_final_func_t) CRX_MURMUR3AFinal,
	(crx_hash_copy_func_t) CRX_MURMUR3ACopy,
	crx_hash_serialize,
	crx_hash_unserialize,
	CRX_MURMUR3A_SPEC,
	4,
	4,
	sizeof(CRX_MURMUR3A_CTX),
	0
};

CRX_HASH_API void CRX_MURMUR3AInit(CRX_MURMUR3A_CTX *ctx, HashTable *args)
{
	if (args) {
		zval *seed = crex_hash_str_find_deref(args, "seed", sizeof("seed") - 1);
		/* This might be a bit too restrictive, but thinking that a seed might be set
			once and for all, it should be done a clean way. */
		if (seed && IS_LONG == C_TYPE_P(seed)) {
			ctx->h = (uint32_t)C_LVAL_P(seed);
		} else {
			ctx->h = 0;
		}
	} else {
		ctx->h = 0;
	}
	ctx->carry = 0;
	ctx->len = 0;
}

CRX_HASH_API void CRX_MURMUR3AUpdate(CRX_MURMUR3A_CTX *ctx, const unsigned char *in, size_t len)
{
	ctx->len += len;
	PMurHash32_Process(&ctx->h, &ctx->carry, in, len);
}

CRX_HASH_API void CRX_MURMUR3AFinal(unsigned char digest[4], CRX_MURMUR3A_CTX *ctx)
{
	ctx->h = PMurHash32_Result(ctx->h, ctx->carry, ctx->len);

	digest[0] = (unsigned char)((ctx->h >> 24) & 0xff);
	digest[1] = (unsigned char)((ctx->h >> 16) & 0xff);
	digest[2] = (unsigned char)((ctx->h >> 8) & 0xff);
	digest[3] = (unsigned char)(ctx->h & 0xff);
}

CRX_HASH_API int CRX_MURMUR3ACopy(const crx_hash_ops *ops, CRX_MURMUR3A_CTX *orig_context, CRX_MURMUR3A_CTX *copy_context)
{
	copy_context->h = orig_context->h;
	copy_context->carry = orig_context->carry;
	copy_context->len = orig_context->len;
	return SUCCESS;
}

const crx_hash_ops crx_hash_murmur3c_ops = {
	"murmur3c",
	(crx_hash_init_func_t) CRX_MURMUR3CInit,
	(crx_hash_update_func_t) CRX_MURMUR3CUpdate,
	(crx_hash_final_func_t) CRX_MURMUR3CFinal,
	(crx_hash_copy_func_t) CRX_MURMUR3CCopy,
	crx_hash_serialize,
	crx_hash_unserialize,
	CRX_MURMUR3C_SPEC,
	16,
	4,
	sizeof(CRX_MURMUR3C_CTX),
	0
};

CRX_HASH_API void CRX_MURMUR3CInit(CRX_MURMUR3C_CTX *ctx, HashTable *args)
{
	if (args) {
		zval *seed = crex_hash_str_find_deref(args, "seed", sizeof("seed") - 1);
		/* This might be a bit too restrictive, but thinking that a seed might be set
			once and for all, it should be done a clean way. */
		if (seed && IS_LONG == C_TYPE_P(seed)) {
			uint32_t _seed = (uint32_t)C_LVAL_P(seed);
			ctx->h[0] = _seed;
			ctx->h[1] = _seed;
			ctx->h[2] = _seed;
			ctx->h[3] = _seed;
		} else {
			memset(&ctx->h, 0, sizeof ctx->h);
		}
	} else {
		memset(&ctx->h, 0, sizeof ctx->h);
	}
	memset(&ctx->carry, 0, sizeof ctx->carry);
	ctx->len = 0;
}

CRX_HASH_API void CRX_MURMUR3CUpdate(CRX_MURMUR3C_CTX *ctx, const unsigned char *in, size_t len)
{
	ctx->len += len;
	PMurHash128x86_Process(ctx->h, ctx->carry, in, len);
}

CRX_HASH_API void CRX_MURMUR3CFinal(unsigned char digest[16], CRX_MURMUR3C_CTX *ctx)
{
	uint32_t h[4] = {0, 0, 0, 0};
	PMurHash128x86_Result(ctx->h, ctx->carry, ctx->len, h);

	digest[0]  = (unsigned char)((h[0] >> 24) & 0xff);
	digest[1]  = (unsigned char)((h[0] >> 16) & 0xff);
	digest[2]  = (unsigned char)((h[0] >> 8) & 0xff);
	digest[3]  = (unsigned char)(h[0] & 0xff);
	digest[4]  = (unsigned char)((h[1] >> 24) & 0xff);
	digest[5]  = (unsigned char)((h[1] >> 16) & 0xff);
	digest[6]  = (unsigned char)((h[1] >> 8) & 0xff);
	digest[7]  = (unsigned char)(h[1] & 0xff);
	digest[8]  = (unsigned char)((h[2] >> 24) & 0xff);
	digest[9]  = (unsigned char)((h[2] >> 16) & 0xff);
	digest[10] = (unsigned char)((h[2] >> 8) & 0xff);
	digest[11] = (unsigned char)(h[2] & 0xff);
	digest[12] = (unsigned char)((h[3] >> 24) & 0xff);
	digest[13] = (unsigned char)((h[3] >> 16) & 0xff);
	digest[14] = (unsigned char)((h[3] >> 8) & 0xff);
	digest[15] = (unsigned char)(h[3] & 0xff);
}

CRX_HASH_API int CRX_MURMUR3CCopy(const crx_hash_ops *ops, CRX_MURMUR3C_CTX *orig_context, CRX_MURMUR3C_CTX *copy_context)
{
	memcpy(&copy_context->h, &orig_context->h, sizeof orig_context->h);
	memcpy(&copy_context->carry, &orig_context->carry, sizeof orig_context->carry);
	copy_context->len = orig_context->len;
	return SUCCESS;
}

const crx_hash_ops crx_hash_murmur3f_ops = {
	"murmur3f",
	(crx_hash_init_func_t) CRX_MURMUR3FInit,
	(crx_hash_update_func_t) CRX_MURMUR3FUpdate,
	(crx_hash_final_func_t) CRX_MURMUR3FFinal,
	(crx_hash_copy_func_t) CRX_MURMUR3FCopy,
	crx_hash_serialize,
	crx_hash_unserialize,
	CRX_MURMUR3F_SPEC,
	16,
	8,
	sizeof(CRX_MURMUR3F_CTX),
	0
};

CRX_HASH_API void CRX_MURMUR3FInit(CRX_MURMUR3F_CTX *ctx, HashTable *args)
{
	if (args) {
		zval *seed = crex_hash_str_find_deref(args, "seed", sizeof("seed") - 1);
		/* This might be a bit too restrictive, but thinking that a seed might be set
			once and for all, it should be done a clean way. */
		if (seed && IS_LONG == C_TYPE_P(seed)) {
			uint64_t _seed = (uint64_t)C_LVAL_P(seed);
			ctx->h[0] = _seed;
			ctx->h[1] = _seed;
		} else {
			memset(&ctx->h, 0, sizeof ctx->h);
		}
	} else {
		memset(&ctx->h, 0, sizeof ctx->h);
	}
	memset(&ctx->carry, 0, sizeof ctx->carry);
	ctx->len = 0;
}

CRX_HASH_API void CRX_MURMUR3FUpdate(CRX_MURMUR3F_CTX *ctx, const unsigned char *in, size_t len)
{
	ctx->len += len;
	PMurHash128x64_Process(ctx->h, ctx->carry, in, len);
}

CRX_HASH_API void CRX_MURMUR3FFinal(unsigned char digest[16], CRX_MURMUR3F_CTX *ctx)
{
	uint64_t h[2] = {0, 0};
	PMurHash128x64_Result(ctx->h, ctx->carry, ctx->len, h);

	digest[0]  = (unsigned char)((h[0] >> 56) & 0xff);
	digest[1]  = (unsigned char)((h[0] >> 48) & 0xff);
	digest[2]  = (unsigned char)((h[0] >> 40) & 0xff);
	digest[3]  = (unsigned char)((h[0] >> 32) & 0xff);
	digest[4]  = (unsigned char)((h[0] >> 24) & 0xff);
	digest[5]  = (unsigned char)((h[0] >> 16) & 0xff);
	digest[6]  = (unsigned char)((h[0] >> 8) & 0xff);
	digest[7]  = (unsigned char)(h[0] & 0xff);
	digest[8]  = (unsigned char)((h[1] >> 56) & 0xff);
	digest[9]  = (unsigned char)((h[1] >> 48) & 0xff);
	digest[10] = (unsigned char)((h[1] >> 40) & 0xff);
	digest[11] = (unsigned char)((h[1] >> 32) & 0xff);
	digest[12] = (unsigned char)((h[1] >> 24) & 0xff);
	digest[13] = (unsigned char)((h[1] >> 16) & 0xff);
	digest[14] = (unsigned char)((h[1] >> 8) & 0xff);
	digest[15] = (unsigned char)(h[1] & 0xff);
}

CRX_HASH_API int CRX_MURMUR3FCopy(const crx_hash_ops *ops, CRX_MURMUR3F_CTX *orig_context, CRX_MURMUR3F_CTX *copy_context)
{
	memcpy(&copy_context->h, &orig_context->h, sizeof orig_context->h);
	memcpy(&copy_context->carry, &orig_context->carry, sizeof orig_context->carry);
	copy_context->len = orig_context->len;
	return SUCCESS;
}
