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
   | Authors: Anthony Ferrara <ircmaxell@crx.net>                         |
   |          Charles R. Portwood II <charlesportwoodii@erianna.com>      |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_PASSWORD_H
#define CRX_PASSWORD_H

CRX_MINIT_FUNCTION(password);
CRX_MSHUTDOWN_FUNCTION(password);

#define CRX_PASSWORD_DEFAULT    CRX_PASSWORD_BCRYPT
#define CRX_PASSWORD_BCRYPT_COST 10

#ifdef HAVE_ARGON2LIB
/**
 * When updating these values, synchronize ext/sodium/sodium_pwhash.c values.
 * Note that libargon expresses memlimit in KB, while libsoidum uses bytes.
 */
#define CRX_PASSWORD_ARGON2_MEMORY_COST (64 << 10)
#define CRX_PASSWORD_ARGON2_TIME_COST 4
#define CRX_PASSWORD_ARGON2_THREADS 1
#endif

typedef struct _crx_password_algo {
	const char *name;
	crex_string *(*hash)(const crex_string *password, crex_array *options);
	bool (*verify)(const crex_string *password, const crex_string *hash);
	bool (*needs_rehash)(const crex_string *password, crex_array *options);
	int (*get_info)(zval *return_value, const crex_string *hash);
	bool (*valid)(const crex_string *hash);
} crx_password_algo;

extern const crx_password_algo crx_password_algo_bcrypt;
#ifdef HAVE_ARGON2LIB
extern const crx_password_algo crx_password_algo_argon2i;
extern const crx_password_algo crx_password_algo_argon2id;
#endif

CRXAPI int crx_password_algo_register(const char*, const crx_password_algo*);
CRXAPI void crx_password_algo_unregister(const char*);
CRXAPI const crx_password_algo* crx_password_algo_default(void);
CRXAPI crex_string *crx_password_algo_extract_ident(const crex_string*);
CRXAPI const crx_password_algo* crx_password_algo_find(const crex_string*);

CRXAPI const crx_password_algo* crx_password_algo_identify_ex(const crex_string*, const crx_password_algo*);
static inline const crx_password_algo* crx_password_algo_identify(const crex_string *hash) {
	return crx_password_algo_identify_ex(hash, crx_password_algo_default());
}


#endif
