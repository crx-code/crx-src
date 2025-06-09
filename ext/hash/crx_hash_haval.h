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
   | Author: Sara Golemon <pollita@crx.net>                               |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_HASH_HAVAL_H
#define CRX_HASH_HAVAL_H

#include "ext/standard/basic_functions.h"
/* HAVAL context. */
typedef struct {
	uint32_t state[8];
	uint32_t count[2];
	unsigned char buffer[128];

	char passes;
	short output;
	void (*Transform)(uint32_t state[8], const unsigned char block[128]);
} CRX_HAVAL_CTX;
#define CRX_HAVAL_SPEC "l8l2b128"

#define CRX_HASH_HAVAL_INIT_DECL(p,b)	CRX_HASH_API void CRX_##p##HAVAL##b##Init(CRX_HAVAL_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *); \
										CRX_HASH_API void CRX_HAVAL##b##Final(unsigned char*, CRX_HAVAL_CTX *);

CRX_HASH_API void CRX_HAVALUpdate(CRX_HAVAL_CTX *, const unsigned char *, size_t);

CRX_HASH_HAVAL_INIT_DECL(3,128)
CRX_HASH_HAVAL_INIT_DECL(3,160)
CRX_HASH_HAVAL_INIT_DECL(3,192)
CRX_HASH_HAVAL_INIT_DECL(3,224)
CRX_HASH_HAVAL_INIT_DECL(3,256)

CRX_HASH_HAVAL_INIT_DECL(4,128)
CRX_HASH_HAVAL_INIT_DECL(4,160)
CRX_HASH_HAVAL_INIT_DECL(4,192)
CRX_HASH_HAVAL_INIT_DECL(4,224)
CRX_HASH_HAVAL_INIT_DECL(4,256)

CRX_HASH_HAVAL_INIT_DECL(5,128)
CRX_HASH_HAVAL_INIT_DECL(5,160)
CRX_HASH_HAVAL_INIT_DECL(5,192)
CRX_HASH_HAVAL_INIT_DECL(5,224)
CRX_HASH_HAVAL_INIT_DECL(5,256)

#endif
