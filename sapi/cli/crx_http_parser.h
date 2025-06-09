/* Copyright 2009,2010 Ryan Dahl <ry@tinyclouds.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
/* modified by Moriyoshi Koizumi <moriyoshi@crx.net> to make it fit to CRX source tree. */
#ifndef crx_http_parser_h
#define crx_http_parser_h
#ifdef __cplusplus
extern "C" {
#endif


#include <sys/types.h>
#if defined(_WIN32) && !defined(__MINGW32__)
# include <windows.h>
# include "config.w32.h"
#else
# include "crx_config.h"
#endif

#include <stdint.h>

/* Compile with -DCRX_HTTP_PARSER_STRICT=0 to make less checks, but run
 * faster
 */
#ifndef CRX_HTTP_PARSER_STRICT
# define CRX_HTTP_PARSER_STRICT 1
#else
# define CRX_HTTP_PARSER_STRICT 0
#endif


/* Maximum header size allowed */
#define CRX_HTTP_MAX_HEADER_SIZE (80*1024)


typedef struct crx_http_parser crx_http_parser;
typedef struct crx_http_parser_settings crx_http_parser_settings;


/* Callbacks should return non-zero to indicate an error. The parser will
 * then halt execution.
 *
 * The one exception is on_headers_complete. In a CRX_HTTP_RESPONSE parser
 * returning '1' from on_headers_complete will tell the parser that it
 * should not expect a body. This is used when receiving a response to a
 * HEAD request which may contain 'Content-Length' or 'Transfer-Encoding:
 * chunked' headers that indicate the presence of a body.
 *
 * http_data_cb does not return data chunks. It will be call arbitrarily
 * many times for each string. E.G. you might get 10 callbacks for "on_path"
 * each providing just a few characters more data.
 */
typedef int (*crx_http_data_cb) (crx_http_parser*, const char *at, size_t length);
typedef int (*crx_http_cb) (crx_http_parser*);


/* Request Methods */
enum crx_http_method
  { CRX_HTTP_DELETE    = 0
  , CRX_HTTP_GET
  , CRX_HTTP_HEAD
  , CRX_HTTP_POST
  , CRX_HTTP_PUT
  , CRX_HTTP_PATCH
  /* pathological */
  , CRX_HTTP_CONNECT
  , CRX_HTTP_OPTIONS
  , CRX_HTTP_TRACE
  /* webdav */
  , CRX_HTTP_COPY
  , CRX_HTTP_LOCK
  , CRX_HTTP_MKCOL
  , CRX_HTTP_MOVE
  , CRX_HTTP_MKCALENDAR
  , CRX_HTTP_PROPFIND
  , CRX_HTTP_PROPPATCH
  , CRX_HTTP_SEARCH
  , CRX_HTTP_UNLOCK
  /* subversion */
  , CRX_HTTP_REPORT
  , CRX_HTTP_MKACTIVITY
  , CRX_HTTP_CHECKOUT
  , CRX_HTTP_MERGE
  /* upnp */
  , CRX_HTTP_MSEARCH
  , CRX_HTTP_NOTIFY
  , CRX_HTTP_SUBSCRIBE
  , CRX_HTTP_UNSUBSCRIBE
  /* unknown, not implemented */
  , CRX_HTTP_NOT_IMPLEMENTED
  };


enum crx_http_parser_type { CRX_HTTP_REQUEST, CRX_HTTP_RESPONSE, CRX_HTTP_BOTH };

enum state
  { s_dead = 1 /* important that this is > 0 */

  , s_start_req_or_res
  , s_res_or_resp_H
  , s_start_res
  , s_res_H
  , s_res_HT
  , s_res_HTT
  , s_res_HTTP
  , s_res_first_http_major
  , s_res_http_major
  , s_res_first_http_minor
  , s_res_http_minor
  , s_res_first_status_code
  , s_res_status_code
  , s_res_status
  , s_res_line_almost_done

  , s_start_req

  , s_req_method
  , s_req_spaces_before_url
  , s_req_schema
  , s_req_schema_slash
  , s_req_schema_slash_slash
  , s_req_host
  , s_req_port
  , s_req_path
  , s_req_query_string_start
  , s_req_query_string
  , s_req_fragment_start
  , s_req_fragment
  , s_req_http_start
  , s_req_http_H
  , s_req_http_HT
  , s_req_http_HTT
  , s_req_http_HTTP
  , s_req_first_http_major
  , s_req_http_major
  , s_req_first_http_minor
  , s_req_http_minor
  , s_req_line_almost_done

  , s_header_field_start
  , s_header_field
  , s_header_value_start
  , s_header_value

  , s_header_almost_done

  , s_headers_almost_done
  /* Important: 's_headers_almost_done' must be the last 'header' state. All
   * states beyond this must be 'body' states. It is used for overflow
   * checking. See the PARSING_HEADER() macro.
   */
  , s_chunk_size_start
  , s_chunk_size
  , s_chunk_size_almost_done
  , s_chunk_parameters
  , s_chunk_data
  , s_chunk_data_almost_done
  , s_chunk_data_done

  , s_body_identity
  , s_body_identity_eof
  };

struct crx_http_parser {
  /** PRIVATE **/
  unsigned char type : 2;
  unsigned char flags : 6;
  unsigned char state;
  unsigned char header_state;
  unsigned char index;

  uint32_t nread;
  ssize_t  content_length;

  /** READ-ONLY **/
  unsigned short http_major;
  unsigned short http_minor;
  unsigned short status_code; /* responses only */
  unsigned char method;    /* requests only */

  /* 1 = Upgrade header was present and the parser has exited because of that.
   * 0 = No upgrade header present.
   * Should be checked when http_parser_execute() returns in addition to
   * error checking.
   */
  char upgrade;

  /** PUBLIC **/
  void *data; /* A pointer to get hook to the "connection" or "socket" object */
};


struct crx_http_parser_settings {
  crx_http_cb      on_message_begin;
  crx_http_data_cb on_path;
  crx_http_data_cb on_query_string;
  crx_http_data_cb on_url;
  crx_http_data_cb on_fragment;
  crx_http_data_cb on_header_field;
  crx_http_data_cb on_header_value;
  crx_http_cb      on_headers_complete;
  crx_http_data_cb on_body;
  crx_http_cb      on_message_complete;
};


void crx_http_parser_init(crx_http_parser *parser, enum crx_http_parser_type type);


size_t crx_http_parser_execute(crx_http_parser *parser,
                           const crx_http_parser_settings *settings,
                           const char *data,
                           size_t len);


/* If crx_http_should_keep_alive() in the on_headers_complete or
 * on_message_complete callback returns true, then this will be should be
 * the last message on the connection.
 * If you are the server, respond with the "Connection: close" header.
 * If you are the client, close the connection.
 */
int crx_http_should_keep_alive(crx_http_parser *parser);

/* Returns a string version of the HTTP method. */
const char *crx_http_method_str(enum crx_http_method);

#ifdef __cplusplus
}
#endif
#endif
