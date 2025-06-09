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
   | Author: Stefan Esser <sesser@crx.net>                                |
   +----------------------------------------------------------------------+
*/

#ifndef SHA1_H
#define SHA1_H

#include "ext/standard/basic_functions.h"

/* SHA1 context. */
typedef struct {
	uint32_t state[5];		/* state (ABCD) */
	uint32_t count[2];		/* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];	/* input buffer */
} CRX_SHA1_CTX;
#define CRX_SHA1_SPEC "l5l2b64."

#define CRX_SHA1Init(ctx) CRX_SHA1InitArgs(ctx, NULL)
CRXAPI void CRX_SHA1InitArgs(CRX_SHA1_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *);
CRXAPI void CRX_SHA1Update(CRX_SHA1_CTX *, const unsigned char *, size_t);
CRXAPI void CRX_SHA1Final(unsigned char[20], CRX_SHA1_CTX *);
CRXAPI void make_sha1_digest(char *sha1str, const unsigned char *digest);

#endif
