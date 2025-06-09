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

/*  Based on the public domain algorithm found at
	http://www.isthe.com/chongo/tech/comp/fnv/index.html */

#include "crx_hash.h"
#include "crx_hash_fnv.h"

const crx_hash_ops crx_hash_fnv132_ops = {
	"fnv132",
	(crx_hash_init_func_t) CRX_FNV132Init,
	(crx_hash_update_func_t) CRX_FNV132Update,
	(crx_hash_final_func_t) CRX_FNV132Final,
	crx_hash_copy,
	crx_hash_serialize,
	crx_hash_unserialize,
	CRX_FNV132_SPEC,
	4,
	4,
	sizeof(CRX_FNV132_CTX),
	0
};

const crx_hash_ops crx_hash_fnv1a32_ops = {
	"fnv1a32",
	(crx_hash_init_func_t) CRX_FNV132Init,
	(crx_hash_update_func_t) CRX_FNV1a32Update,
	(crx_hash_final_func_t) CRX_FNV132Final,
	crx_hash_copy,
	crx_hash_serialize,
	crx_hash_unserialize,
	CRX_FNV132_SPEC,
	4,
	4,
	sizeof(CRX_FNV132_CTX),
	0
};

const crx_hash_ops crx_hash_fnv164_ops = {
	"fnv164",
	(crx_hash_init_func_t) CRX_FNV164Init,
	(crx_hash_update_func_t) CRX_FNV164Update,
	(crx_hash_final_func_t) CRX_FNV164Final,
	crx_hash_copy,
	crx_hash_serialize,
	crx_hash_unserialize,
	CRX_FNV164_SPEC,
	8,
	4,
	sizeof(CRX_FNV164_CTX),
	0
};

const crx_hash_ops crx_hash_fnv1a64_ops = {
	"fnv1a64",
	(crx_hash_init_func_t) CRX_FNV164Init,
	(crx_hash_update_func_t) CRX_FNV1a64Update,
	(crx_hash_final_func_t) CRX_FNV164Final,
	crx_hash_copy,
	crx_hash_serialize,
	crx_hash_unserialize,
	CRX_FNV164_SPEC,
	8,
	4,
	sizeof(CRX_FNV164_CTX),
	0
};

/* {{{ CRX_FNV132Init
 * 32-bit FNV-1 hash initialisation
 */
CRX_HASH_API void CRX_FNV132Init(CRX_FNV132_CTX *context, CREX_ATTRIBUTE_UNUSED HashTable *args)
{
	context->state = CRX_FNV1_32_INIT;
}
/* }}} */

CRX_HASH_API void CRX_FNV132Update(CRX_FNV132_CTX *context, const unsigned char *input,
		size_t inputLen)
{
	context->state = fnv_32_buf((void *)input, inputLen, context->state, 0);
}

CRX_HASH_API void CRX_FNV1a32Update(CRX_FNV132_CTX *context, const unsigned char *input,
		size_t inputLen)
{
	context->state = fnv_32_buf((void *)input, inputLen, context->state, 1);
}

CRX_HASH_API void CRX_FNV132Final(unsigned char digest[4], CRX_FNV132_CTX * context)
{
#ifdef WORDS_BIGENDIAN
	memcpy(digest, &context->state, 4);
#else
	int i = 0;
	unsigned char *c = (unsigned char *) &context->state;

	for (i = 0; i < 4; i++) {
		digest[i] = c[3 - i];
	}
#endif
}

/* {{{ CRX_FNV164Init
 * 64-bit FNV-1 hash initialisation
 */
CRX_HASH_API void CRX_FNV164Init(CRX_FNV164_CTX *context, CREX_ATTRIBUTE_UNUSED HashTable *args)
{
	context->state = CRX_FNV1_64_INIT;
}
/* }}} */

CRX_HASH_API void CRX_FNV164Update(CRX_FNV164_CTX *context, const unsigned char *input,
		size_t inputLen)
{
	context->state = fnv_64_buf((void *)input, inputLen, context->state, 0);
}

CRX_HASH_API void CRX_FNV1a64Update(CRX_FNV164_CTX *context, const unsigned char *input,
		size_t inputLen)
{
	context->state = fnv_64_buf((void *)input, inputLen, context->state, 1);
}

CRX_HASH_API void CRX_FNV164Final(unsigned char digest[8], CRX_FNV164_CTX * context)
{
#ifdef WORDS_BIGENDIAN
	memcpy(digest, &context->state, 8);
#else
	int i = 0;
	unsigned char *c = (unsigned char *) &context->state;

	for (i = 0; i < 8; i++) {
		digest[i] = c[7 - i];
	}
#endif
}


/*
 * fnv_32_buf - perform a 32 bit Fowler/Noll/Vo hash on a buffer
 *
 * input:
 *  buf - start of buffer to hash
 *  len - length of buffer in octets
 *  hval	- previous hash value or 0 if first call
 *  alternate - if > 0 use the alternate version
 *
 * returns:
 *  32-bit hash as a static hash type
 */
static uint32_t
fnv_32_buf(void *buf, size_t len, uint32_t hval, int alternate)
{
	unsigned char *bp = (unsigned char *)buf;   /* start of buffer */
	unsigned char *be = bp + len;	   /* beyond end of buffer */

	/*
	 * FNV-1 hash each octet in the buffer
	 */
	if (alternate == 0) {
		while (bp < be) {
			/* multiply by the 32 bit FNV magic prime mod 2^32 */
			hval *= CRX_FNV_32_PRIME;

			/* xor the bottom with the current octet */
			hval ^= (uint32_t)*bp++;
		}
	} else {
		while (bp < be) {
			/* xor the bottom with the current octet */
			hval ^= (uint32_t)*bp++;

			/* multiply by the 32 bit FNV magic prime mod 2^32 */
			hval *= CRX_FNV_32_PRIME;
		}
	}

	/* return our new hash value */
	return hval;
}

/*
 * fnv_64_buf - perform a 64 bit Fowler/Noll/Vo hash on a buffer
 *
 * input:
 *  buf - start of buffer to hash
 *  len - length of buffer in octets
 *  hval	- previous hash value or 0 if first call
 *  alternate - if > 0 use the alternate version
 *
 * returns:
 *  64-bit hash as a static hash type
 */
static uint64_t
fnv_64_buf(void *buf, size_t len, uint64_t hval, int alternate)
{
	unsigned char *bp = (unsigned char *)buf;   /* start of buffer */
	unsigned char *be = bp + len;	   /* beyond end of buffer */

	/*
	 * FNV-1 hash each octet of the buffer
	 */

	if (alternate == 0) {
		while (bp < be) {
			/* multiply by the 64 bit FNV magic prime mod 2^64 */
			hval *= CRX_FNV_64_PRIME;

			/* xor the bottom with the current octet */
			hval ^= (uint64_t)*bp++;
		}
	 } else {
		while (bp < be) {
			/* xor the bottom with the current octet */
			hval ^= (uint64_t)*bp++;

			/* multiply by the 64 bit FNV magic prime mod 2^64 */
			hval *= CRX_FNV_64_PRIME;
		 }
	}

	/* return our new hash value */
	return hval;
}
