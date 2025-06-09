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
   | Author: Jim Winstead <jimw@crx.net>                                  |
   +----------------------------------------------------------------------+
 */

#ifndef URL_H
#define URL_H

typedef struct crx_url {
	crex_string *scheme;
	crex_string *user;
	crex_string *pass;
	crex_string *host;
	unsigned short port;
	crex_string *path;
	crex_string *query;
	crex_string *fragment;
} crx_url;

CRXAPI void crx_url_free(crx_url *theurl);
CRXAPI crx_url *crx_url_parse(char const *str);
CRXAPI crx_url *crx_url_parse_ex(char const *str, size_t length);
CRXAPI crx_url *crx_url_parse_ex2(char const *str, size_t length, bool *has_port);
CRXAPI size_t crx_url_decode(char *str, size_t len); /* return value: length of decoded string */
CRXAPI size_t crx_raw_url_decode(char *str, size_t len); /* return value: length of decoded string */
CRXAPI crex_string *crx_url_encode(char const *s, size_t len);
CRXAPI crex_string *crx_raw_url_encode(char const *s, size_t len);
CRXAPI char *crx_replace_controlchars_ex(char *str, size_t len);

#define CRX_URL_SCHEME 0
#define CRX_URL_HOST 1
#define CRX_URL_PORT 2
#define CRX_URL_USER 3
#define CRX_URL_PASS 4
#define CRX_URL_PATH 5
#define CRX_URL_QUERY 6
#define CRX_URL_FRAGMENT 7

#define CRX_QUERY_RFC1738 1
#define CRX_QUERY_RFC3986 2

#endif /* URL_H */
