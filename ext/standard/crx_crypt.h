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
   | Authors: Stig Bakken <ssb@crx.net>                                   |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Rasmus Lerdorf <rasmus@crx.net>                             |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_CRYPT_H
#define CRX_CRYPT_H

CRXAPI crex_string *crx_crypt(const char *password, const int pass_len, const char *salt, int salt_len, bool quiet);
CRX_MINIT_FUNCTION(crypt);
CRX_MSHUTDOWN_FUNCTION(crypt);
CRX_RINIT_FUNCTION(crypt);

/* sha512 crypt has the maximal salt length of 123 characters */
#define CRX_MAX_SALT_LEN 123

#endif
