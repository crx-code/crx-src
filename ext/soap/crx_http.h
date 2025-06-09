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
  | Authors: Brad Lafountain <rodif_bl@yahoo.com>                        |
  |          Shane Caraveo <shane@caraveo.com>                           |
  |          Dmitry Stogov <dmitry@crx.net>                              |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_HTTP_H
#define CRX_HTTP_H

int make_http_soap_request(zval        *this_ptr,
                           crex_string *request,
                           char        *location,
                           char        *soapaction,
                           int          soap_version,
                           zval        *response);

int proxy_authentication(zval* this_ptr, smart_str* soap_headers);
int basic_authentication(zval* this_ptr, smart_str* soap_headers);
void http_context_headers(crx_stream_context* context,
                          bool has_authorization,
                          bool has_proxy_authorization,
                          bool has_cookies,
                          smart_str* soap_headers);
#endif
