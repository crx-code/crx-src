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
#include "crx_hash_adler32.h"

CRX_HASH_API void CRX_ADLER32Init(CRX_ADLER32_CTX *context, CREX_ATTRIBUTE_UNUSED HashTable *args)
{
	context->state = 1;
}

CRX_HASH_API void CRX_ADLER32Update(CRX_ADLER32_CTX *context, const unsigned char *input, size_t len)
{
	uint32_t i, s[2];

	s[0] = context->state & 0xffff;
	s[1] = (context->state >> 16) & 0xffff;
	for (i = 0; i < len; ++i) {
		s[0] += input[i];
		s[1] += s[0];
		if (s[1]>=0x7fffffff)
		{
			s[0] = s[0] % 65521;
			s[1] = s[1] % 65521;
		}
	}
	s[0] = s[0] % 65521;
	s[1] = s[1] % 65521;
	context->state = s[0] + (s[1] << 16);
}

CRX_HASH_API void CRX_ADLER32Final(unsigned char digest[4], CRX_ADLER32_CTX *context)
{
	digest[0] = (unsigned char) ((context->state >> 24) & 0xff);
	digest[1] = (unsigned char) ((context->state >> 16) & 0xff);
	digest[2] = (unsigned char) ((context->state >> 8) & 0xff);
	digest[3] = (unsigned char) (context->state & 0xff);
	context->state = 0;
}

CRX_HASH_API int CRX_ADLER32Copy(const crx_hash_ops *ops, CRX_ADLER32_CTX *orig_context, CRX_ADLER32_CTX *copy_context)
{
	copy_context->state = orig_context->state;
	return SUCCESS;
}

const crx_hash_ops crx_hash_adler32_ops = {
	"adler32",
	(crx_hash_init_func_t) CRX_ADLER32Init,
	(crx_hash_update_func_t) CRX_ADLER32Update,
	(crx_hash_final_func_t) CRX_ADLER32Final,
	(crx_hash_copy_func_t) CRX_ADLER32Copy,
	crx_hash_serialize,
	crx_hash_unserialize,
	CRX_ADLER32_SPEC,
	4, /* what to say here? */
	4,
	sizeof(CRX_ADLER32_CTX),
	0
};
