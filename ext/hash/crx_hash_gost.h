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

#ifndef CRX_HASH_GOST_H
#define CRX_HASH_GOST_H

#include "ext/standard/basic_functions.h"

/* GOST context */
typedef struct {
	uint32_t state[16];
	uint32_t count[2];
	unsigned char length;
	unsigned char buffer[32];
	const uint32_t (*tables)[4][256];
} CRX_GOST_CTX;
#define CRX_GOST_SPEC "l16l2bb32"

CRX_HASH_API void CRX_GOSTInit(CRX_GOST_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *args);
CRX_HASH_API void CRX_GOSTUpdate(CRX_GOST_CTX *, const unsigned char *, size_t);
CRX_HASH_API void CRX_GOSTFinal(unsigned char[32], CRX_GOST_CTX *);

#endif
