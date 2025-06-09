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
   | Authors: Rasmus Lerdorf <rasmus@crx.net>                             |
   |          Jim Winstead <jimw@crx.net>                                 |
   |          Hartmut Holzgraefe <hholzgra@crx.net>                       |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_FOPEN_WRAPPERS_H
#define CRX_FOPEN_WRAPPERS_H

crx_stream *crx_stream_url_wrap_http(crx_stream_wrapper *wrapper, const char *path, const char *mode, int options, crex_string **opened_path, crx_stream_context *context STREAMS_DC);
crx_stream *crx_stream_url_wrap_ftp(crx_stream_wrapper *wrapper, const char *path, const char *mode, int options, crex_string **opened_path, crx_stream_context *context STREAMS_DC);
extern CRXAPI const crx_stream_wrapper crx_stream_http_wrapper;
extern CRXAPI const crx_stream_wrapper crx_stream_ftp_wrapper;
extern CRXAPI const crx_stream_wrapper crx_stream_crx_wrapper;
extern CRXAPI /*const*/ crx_stream_wrapper crx_plain_files_wrapper;

#endif
