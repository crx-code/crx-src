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
  | Taken from: ext/standard/md5.c                                       |
  +----------------------------------------------------------------------+
*/

#include "crx_hash.h"
#include "crx_hash_md.h"

const crx_hash_ops crx_hash_md5_ops = {
	"md5",
	(crx_hash_init_func_t) CRX_MD5InitArgs,
	(crx_hash_update_func_t) CRX_MD5Update,
	(crx_hash_final_func_t) CRX_MD5Final,
	crx_hash_copy,
	crx_hash_serialize,
	crx_hash_unserialize,
	CRX_MD5_SPEC,
	16,
	64,
	sizeof(CRX_MD5_CTX),
	1
};

const crx_hash_ops crx_hash_md4_ops = {
	"md4",
	(crx_hash_init_func_t) CRX_MD4InitArgs,
	(crx_hash_update_func_t) CRX_MD4Update,
	(crx_hash_final_func_t) CRX_MD4Final,
	crx_hash_copy,
	crx_hash_serialize,
	crx_hash_unserialize,
	CRX_MD4_SPEC,
	16,
	64,
	sizeof(CRX_MD4_CTX),
	1
};

static int crx_md2_unserialize(crx_hashcontext_object *hash, crex_long magic, const zval *zv);

const crx_hash_ops crx_hash_md2_ops = {
	"md2",
	(crx_hash_init_func_t) CRX_MD2InitArgs,
	(crx_hash_update_func_t) CRX_MD2Update,
	(crx_hash_final_func_t) CRX_MD2Final,
	crx_hash_copy,
	crx_hash_serialize,
	crx_md2_unserialize,
	CRX_MD2_SPEC,
	16,
	16,
	sizeof(CRX_MD2_CTX),
	1
};

/* MD common stuff */

static const unsigned char PADDING[64] =
{
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* {{{ Encode
   Encodes input (uint32_t) into output (unsigned char). Assumes len is
   a multiple of 4.
 */
static void Encode(unsigned char *output, uint32_t *input, unsigned int len)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4) {
		output[j] = (unsigned char) (input[i] & 0xff);
		output[j + 1] = (unsigned char) ((input[i] >> 8) & 0xff);
		output[j + 2] = (unsigned char) ((input[i] >> 16) & 0xff);
		output[j + 3] = (unsigned char) ((input[i] >> 24) & 0xff);
	}
}
/* }}} */

/* {{{ Decode
   Decodes input (unsigned char) into output (uint32_t). Assumes len is
   a multiple of 4.
 */
static void Decode(uint32_t *output, const unsigned char *input, unsigned int len)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4)
		output[i] = ((uint32_t) input[j]) | (((uint32_t) input[j + 1]) << 8) |
			(((uint32_t) input[j + 2]) << 16) | (((uint32_t) input[j + 3]) << 24);
}
/* }}} */

/* MD4 */

#define MD4_F(x,y,z)			((z) ^ ((x) & ((y) ^ (z))))
#define MD4_G(x,y,z)			(((x) & ((y) | (z))) | ((y) & (z)))
#define MD4_H(x,y,z)			((x) ^ (y) ^ (z))

#define ROTL32(s,v)				(((v) << (s)) | ((v) >> (32 - (s))))

#define MD4_R1(a,b,c,d,k,s)		a = ROTL32(s, a + MD4_F(b,c,d) + x[k])
#define MD4_R2(a,b,c,d,k,s)		a = ROTL32(s, a + MD4_G(b,c,d) + x[k] + 0x5A827999)
#define MD4_R3(a,b,c,d,k,s)		a = ROTL32(s, a + MD4_H(b,c,d) + x[k] + 0x6ED9EBA1)

static void MD4Transform(uint32_t state[4], const unsigned char block[64])
{
	uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];

	Decode(x, block, 64);

	/* Round 1 */
	MD4_R1(a,b,c,d, 0, 3);
	MD4_R1(d,a,b,c, 1, 7);
	MD4_R1(c,d,a,b, 2,11);
	MD4_R1(b,c,d,a, 3,19);
	MD4_R1(a,b,c,d, 4, 3);
	MD4_R1(d,a,b,c, 5, 7);
	MD4_R1(c,d,a,b, 6,11);
	MD4_R1(b,c,d,a, 7,19);
	MD4_R1(a,b,c,d, 8, 3);
	MD4_R1(d,a,b,c, 9, 7);
	MD4_R1(c,d,a,b,10,11);
	MD4_R1(b,c,d,a,11,19);
	MD4_R1(a,b,c,d,12, 3);
	MD4_R1(d,a,b,c,13, 7);
	MD4_R1(c,d,a,b,14,11);
	MD4_R1(b,c,d,a,15,19);

	/* Round 2 */
	MD4_R2(a,b,c,d, 0, 3);
	MD4_R2(d,a,b,c, 4, 5);
	MD4_R2(c,d,a,b, 8, 9);
	MD4_R2(b,c,d,a,12,13);
	MD4_R2(a,b,c,d, 1, 3);
	MD4_R2(d,a,b,c, 5, 5);
	MD4_R2(c,d,a,b, 9, 9);
	MD4_R2(b,c,d,a,13,13);
	MD4_R2(a,b,c,d, 2, 3);
	MD4_R2(d,a,b,c, 6, 5);
	MD4_R2(c,d,a,b,10, 9);
	MD4_R2(b,c,d,a,14,13);
	MD4_R2(a,b,c,d, 3, 3);
	MD4_R2(d,a,b,c, 7, 5);
	MD4_R2(c,d,a,b,11, 9);
	MD4_R2(b,c,d,a,15,13);

	/* Round 3 */
	MD4_R3(a,b,c,d, 0, 3);
	MD4_R3(d,a,b,c, 8, 9);
	MD4_R3(c,d,a,b, 4,11);
	MD4_R3(b,c,d,a,12,15);
	MD4_R3(a,b,c,d, 2, 3);
	MD4_R3(d,a,b,c,10, 9);
	MD4_R3(c,d,a,b, 6,11);
	MD4_R3(b,c,d,a,14,15);
	MD4_R3(a,b,c,d, 1, 3);
	MD4_R3(d,a,b,c, 9, 9);
	MD4_R3(c,d,a,b, 5,11);
	MD4_R3(b,c,d,a,13,15);
	MD4_R3(a,b,c,d, 3, 3);
	MD4_R3(d,a,b,c,11, 9);
	MD4_R3(c,d,a,b, 7,11);
	MD4_R3(b,c,d,a,15,15);

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
}

/* {{{ CRX_MD4InitArgs
 * MD4 initialization. Begins an MD4 operation, writing a new context.
 */
CRX_HASH_API void CRX_MD4InitArgs(CRX_MD4_CTX * context, CREX_ATTRIBUTE_UNUSED HashTable *args)
{
	context->count[0] = context->count[1] = 0;
	/* Load magic initialization constants.
	 */
	context->state[0] = 0x67452301;
	context->state[1] = 0xefcdab89;
	context->state[2] = 0x98badcfe;
	context->state[3] = 0x10325476;
}
/* }}} */

/* {{{ CRX_MD4Update
   MD4 block update operation. Continues an MD4 message-digest
   operation, processing another message block, and updating the
   context.
 */
CRX_HASH_API void CRX_MD4Update(CRX_MD4_CTX * context, const unsigned char *input, size_t inputLen)
{
	unsigned int i, index, partLen;

	/* Compute number of bytes mod 64 */
	index = (unsigned int) ((context->count[0] >> 3) & 0x3F);

	/* Update number of bits */
	if ((context->count[0] += ((uint32_t) inputLen << 3))
		< ((uint32_t) inputLen << 3))
		context->count[1]++;
	context->count[1] += ((uint32_t) inputLen >> 29);

	partLen = 64 - index;

	/* Transform as many times as possible.
	 */
	if (inputLen >= partLen) {
		memcpy((unsigned char*) & context->buffer[index], (unsigned char*) input, partLen);
		MD4Transform(context->state, context->buffer);

		for (i = partLen; i + 63 < inputLen; i += 64) {
			MD4Transform(context->state, &input[i]);
		}

		index = 0;
	} else {
		i = 0;
	}

	/* Buffer remaining input */
	memcpy((unsigned char*) & context->buffer[index], (unsigned char*) & input[i], inputLen - i);
}
/* }}} */

/* {{{ CRX_MD4Final
   MD4 finalization. Ends an MD4 message-digest operation, writing
   the message digest and zeroizing the context.
 */
CRX_HASH_API void CRX_MD4Final(unsigned char digest[16], CRX_MD4_CTX * context)
{
	unsigned char bits[8];
	unsigned int index, padLen;

	/* Save number of bits */
	Encode(bits, context->count, 8);

	/* Pad out to 56 mod 64.
	 */
	index = (unsigned int) ((context->count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	CRX_MD4Update(context, PADDING, padLen);

	/* Append length (before padding) */
	CRX_MD4Update(context, bits, 8);

	/* Store state in digest */
	Encode(digest, context->state, 16);

	/* Zeroize sensitive information.
	 */
	CREX_SECURE_ZERO((unsigned char*) context, sizeof(*context));
}
/* }}} */

/* MD2 */

static const unsigned char MD2_S[256] = {
	 41,  46,  67, 201, 162, 216, 124,   1,  61,  54,  84, 161, 236, 240,   6,  19,
	 98, 167,   5, 243, 192, 199, 115, 140, 152, 147,  43, 217, 188,  76, 130, 202,
	 30, 155,  87,  60, 253, 212, 224,  22, 103,  66, 111,  24, 138,  23, 229,  18,
	190,  78, 196, 214, 218, 158, 222,  73, 160, 251, 245, 142, 187,  47, 238, 122,
	169, 104, 121, 145,  21, 178,   7,  63, 148, 194,  16, 137,  11,  34,  95,  33,
	128, 127,  93, 154,  90, 144,  50,  39,  53,  62, 204, 231, 191, 247, 151,   3,
	255,  25,  48, 179,  72, 165, 181, 209, 215,  94, 146,  42, 172,  86, 170, 198,
	 79, 184,  56, 210, 150, 164, 125, 182, 118, 252, 107, 226, 156, 116,   4, 241,
	 69, 157, 112,  89, 100, 113, 135,  32, 134,  91, 207, 101, 230,  45, 168,   2,
	 27,  96,  37, 173, 174, 176, 185, 246,  28,  70,  97, 105,  52,  64, 126,  15,
	 85,  71, 163,  35, 221,  81, 175,  58, 195,  92, 249, 206, 186, 197, 234,  38,
	 44,  83,  13, 110, 133,  40, 132,   9, 211, 223, 205, 244,  65, 129,  77,  82,
	106, 220,  55, 200, 108, 193, 171, 250,  36, 225, 123,   8,  12, 189, 177,  74,
	120, 136, 149, 139, 227,  99, 232, 109, 233, 203, 213, 254,  59,   0,  29,  57,
	242, 239, 183,  14, 102,  88, 208, 228, 166, 119, 114, 248, 235, 117,  75,  10,
	 49,  68,  80, 180, 143, 237,  31,  26, 219, 153, 141,  51, 159,  17, 131,  20 };

CRX_HASH_API void CRX_MD2InitArgs(CRX_MD2_CTX *context, CREX_ATTRIBUTE_UNUSED HashTable *args)
{
	memset(context, 0, sizeof(CRX_MD2_CTX));
}

static void MD2_Transform(CRX_MD2_CTX *context, const unsigned char *block)
{
	unsigned char i,j,t = 0;

	for(i = 0; i < 16; i++) {
		context->state[16+i] = block[i];
		context->state[32+i] = (context->state[16+i] ^ context->state[i]);
	}

	for(i = 0; i < 18; i++) {
		for(j = 0; j < 48; j++) {
			t = context->state[j] = context->state[j] ^ MD2_S[t];
		}
		t += i;
	}

	/* Update checksum -- must be after transform to avoid fouling up last message block */
	t = context->checksum[15];
	for(i = 0; i < 16; i++) {
		t = context->checksum[i] ^= MD2_S[block[i] ^ t];
	}
}

CRX_HASH_API void CRX_MD2Update(CRX_MD2_CTX *context, const unsigned char *buf, size_t len)
{
	const unsigned char *p = buf, *e = buf + len;

	if (context->in_buffer) {
		if (context->in_buffer + len < 16) {
			/* Not enough for block, just pass into buffer */
			memcpy(context->buffer + context->in_buffer, p, len);
			context->in_buffer += (char) len;
			return;
		}
		/* Put buffered data together with inbound for a single block */
		memcpy(context->buffer + context->in_buffer, p, 16 - context->in_buffer);
		MD2_Transform(context, context->buffer);
		p += 16 - context->in_buffer;
		context->in_buffer = 0;
	}

	/* Process as many whole blocks as remain */
	while ((p + 16) <= e) {
		MD2_Transform(context, p);
		p += 16;
	}

	/* Copy remaining data to buffer */
	if (p < e) {
		memcpy(context->buffer, p, e - p);
		context->in_buffer = (char) (e - p);
	}
}

CRX_HASH_API void CRX_MD2Final(unsigned char output[16], CRX_MD2_CTX *context)
{
	memset(context->buffer + context->in_buffer, 16 - context->in_buffer, 16 - context->in_buffer);
	MD2_Transform(context, context->buffer);
	MD2_Transform(context, context->checksum);

	memcpy(output, context->state, 16);
}

static int crx_md2_unserialize(crx_hashcontext_object *hash, crex_long magic, const zval *zv)
{
	CRX_MD2_CTX *ctx = (CRX_MD2_CTX *) hash->context;
	int r = FAILURE;
	if (magic == CRX_HASH_SERIALIZE_MAGIC_SPEC
		&& (r = crx_hash_unserialize_spec(hash, zv, CRX_MD2_SPEC)) == SUCCESS
		&& (unsigned char) ctx->in_buffer < sizeof(ctx->buffer)) {
		return SUCCESS;
	} else {
		return r != SUCCESS ? r : -2000;
	}
}
