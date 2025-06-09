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
   | Author: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                        |
   +----------------------------------------------------------------------+
*/

#ifndef HEAD_H
#define HEAD_H

#define COOKIE_EXPIRES    "; expires="
#define COOKIE_MAX_AGE    "; Max-Age="
#define COOKIE_DOMAIN     "; domain="
#define COOKIE_PATH       "; path="
#define COOKIE_SECURE     "; secure"
#define COOKIE_HTTPONLY   "; HttpOnly"
#define COOKIE_SAMESITE   "; SameSite="

extern CRX_RINIT_FUNCTION(head);

CRXAPI int crx_header(void);
CRXAPI crex_result crx_setcookie(crex_string *name, crex_string *value, time_t expires,
	crex_string *path, crex_string *domain, bool secure, bool httponly,
	crex_string *samesite, bool url_encode);

#endif
