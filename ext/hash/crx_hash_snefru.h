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

#ifndef CRX_HASH_SNEFRU_H
#define CRX_HASH_SNEFRU_H

/* SNEFRU-2.5a with 8 passes and 256 bit hash output
 * AKA "Xerox Secure Hash Function"
 */

#include "ext/standard/basic_functions.h"

/* SNEFRU context */
typedef struct {
	uint32_t state[16];
	uint32_t count[2];
	unsigned char length;
	unsigned char buffer[32];
} CRX_SNEFRU_CTX;
#define CRX_SNEFRU_SPEC "l16l2bb32"

CRX_HASH_API void CRX_SNEFRUInit(CRX_SNEFRU_CTX *, CREX_ATTRIBUTE_UNUSED HashTable *);
CRX_HASH_API void CRX_SNEFRUUpdate(CRX_SNEFRU_CTX *, const unsigned char *, size_t);
CRX_HASH_API void CRX_SNEFRUFinal(unsigned char[32], CRX_SNEFRU_CTX *);

#endif
