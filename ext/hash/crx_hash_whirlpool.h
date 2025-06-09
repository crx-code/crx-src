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
   | Author: Michael Wallner <mike@crx.net>                               |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_HASH_WHIRLPOOL_H
#define CRX_HASH_WHIRLPOOL_H

/* WHIRLPOOL context */
typedef struct {
	uint64_t state[8];
	unsigned char bitlength[32];
	struct {
		int pos;
		int bits;
		unsigned char data[64];
	} buffer;
} CRX_WHIRLPOOL_CTX;
#define CRX_WHIRLPOOL_SPEC "q8b32iib64."

CRX_HASH_API void CRX_WHIRLPOOLInit(CRX_WHIRLPOOL_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *);
CRX_HASH_API void CRX_WHIRLPOOLUpdate(CRX_WHIRLPOOL_CTX *, const unsigned char *, size_t);
CRX_HASH_API void CRX_WHIRLPOOLFinal(unsigned char[64], CRX_WHIRLPOOL_CTX *);

#endif
