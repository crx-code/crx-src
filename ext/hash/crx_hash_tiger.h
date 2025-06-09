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

#ifndef CRX_HASH_TIGER_H
#define CRX_HASH_TIGER_H

/* TIGER context */
typedef struct {
	uint64_t state[3];
	uint64_t passed;
	unsigned char buffer[64];
	uint32_t length;
	unsigned int passes:1;
} CRX_TIGER_CTX;
#define CRX_TIGER_SPEC "q3qb64l"

CRX_HASH_API void CRX_3TIGERInit(CRX_TIGER_CTX *context, CREX_ATTRIBUTE_UNUSED HashTable *args);
CRX_HASH_API void CRX_4TIGERInit(CRX_TIGER_CTX *context, CREX_ATTRIBUTE_UNUSED HashTable *args);
CRX_HASH_API void CRX_TIGERUpdate(CRX_TIGER_CTX *context, const unsigned char *input, size_t len);
CRX_HASH_API void CRX_TIGER128Final(unsigned char digest[16], CRX_TIGER_CTX *context);
CRX_HASH_API void CRX_TIGER160Final(unsigned char digest[20], CRX_TIGER_CTX *context);
CRX_HASH_API void CRX_TIGER192Final(unsigned char digest[24], CRX_TIGER_CTX *context);

#endif
