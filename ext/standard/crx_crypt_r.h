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
   | Authors: Pierre Alain Joye  <pajoye@crx.net                          |
   +----------------------------------------------------------------------+
 */

#ifndef _CRYPT_WIHN32_H_
#define _CRYPT_WIHN32_H_

BEGIN_EXTERN_C()
#include "crypt_freesec.h"

void crx_init_crypt_r(void);
void crx_shutdown_crypt_r(void);

extern void _crypt_extended_init_r(void);

CRXAPI char *crx_crypt_r (const char *__key, const char *__salt, struct crx_crypt_extended_data * __data);

#define MD5_HASH_MAX_LEN 120

#include "crypt_blowfish.h"

extern char * crx_md5_crypt_r(const char *pw, const char *salt, char *out);
extern char * crx_sha512_crypt_r (const char *key, const char *salt, char *buffer, int buflen);
extern char * crx_sha256_crypt_r (const char *key, const char *salt, char *buffer, int buflen);
END_EXTERN_C()

#endif /* _CRYPT_WIHN32_H_ */
