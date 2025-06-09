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
   | Authors: Frank Denis <jedisct1@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_LIBSODIUM_H
#define CRX_LIBSODIUM_H

extern crex_module_entry sodium_module_entry;
#define crxext_sodium_ptr &sodium_module_entry

#define CRX_SODIUM_VERSION CRX_VERSION

#ifdef ZTS
# include "TSRM.h"
#endif

#define SODIUM_LIBRARY_VERSION() (char *) (void *) sodium_version_string()

#define SODIUM_CRYPTO_BOX_KEYPAIRBYTES() crypto_box_SECRETKEYBYTES + crypto_box_PUBLICKEYBYTES

#define SODIUM_CRYPTO_KX_KEYPAIRBYTES() crypto_kx_SECRETKEYBYTES + crypto_kx_PUBLICKEYBYTES

#define SODIUM_CRYPTO_SIGN_KEYPAIRBYTES() crypto_sign_SECRETKEYBYTES + crypto_sign_PUBLICKEYBYTES

CRX_MINIT_FUNCTION(sodium);
CRX_MINIT_FUNCTION(sodium_password_hash);
CRX_MSHUTDOWN_FUNCTION(sodium);
CRX_RINIT_FUNCTION(sodium);
CRX_RSHUTDOWN_FUNCTION(sodium);
CRX_MINFO_FUNCTION(sodium);

#endif	/* CRX_LIBSODIUM_H */
