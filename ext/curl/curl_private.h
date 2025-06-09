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
   | Author: Sterling Hughes <sterling@crx.net>                           |
   |         Wez Furlong <wez@thebrainroom.com>                           |
   +----------------------------------------------------------------------+
*/

#ifndef _CRX_CURL_PRIVATE_H
#define _CRX_CURL_PRIVATE_H

#include "crx_curl.h"

#define CRX_CURL_DEBUG 0

#include "crx_version.h"
#define CRX_CURL_VERSION CRX_VERSION

#include <curl/curl.h>
#include <curl/multi.h>

#define CURLOPT_RETURNTRANSFER 19913
#define CURLOPT_BINARYTRANSFER 19914 /* For Backward compatibility */
#define CRX_CURL_STDOUT 0
#define CRX_CURL_FILE   1
#define CRX_CURL_USER   2
#define CRX_CURL_DIRECT 3
#define CRX_CURL_RETURN 4
#define CRX_CURL_IGNORE 7

#define SAVE_CURL_ERROR(__handle, __err) \
    do { (__handle)->err.no = (int) __err; } while (0)

CRX_MINIT_FUNCTION(curl);
CRX_MSHUTDOWN_FUNCTION(curl);
CRX_MINFO_FUNCTION(curl);

typedef struct {
	zval                  func_name;
	crex_fcall_info_cache fci_cache;
	FILE                 *fp;
	smart_str             buf;
	int                   method;
	zval					stream;
} crx_curl_write;

typedef struct {
	zval                  func_name;
	crex_fcall_info_cache fci_cache;
	FILE                 *fp;
	crex_resource        *res;
	int                   method;
	zval                  stream;
} crx_curl_read;

typedef struct {
	zval                  func_name;
	crex_fcall_info_cache fci_cache;
} crx_curl_callback;

typedef struct {
	crx_curl_write    *write;
	crx_curl_write    *write_header;
	crx_curl_read     *read;
	zval               std_err;
	crx_curl_callback *progress;
#if LIBCURL_VERSION_NUM >= 0x072000
	crx_curl_callback  *xferinfo;
#endif
	crx_curl_callback  *fnmatch;
#if LIBCURL_VERSION_NUM >= 0x075400
	crx_curl_callback  *sshhostkey;
#endif
} crx_curl_handlers;

struct _crx_curl_error  {
	char str[CURL_ERROR_SIZE + 1];
	int  no;
};

struct _crx_curl_send_headers {
	crex_string *str;
};

struct _crx_curl_free {
	crex_llist post;
	crex_llist stream;
#if LIBCURL_VERSION_NUM < 0x073800 /* 7.56.0 */
	crex_llist buffers;
#endif
	HashTable *slist;
};

typedef struct {
	CURL                         *cp;
	crx_curl_handlers             handlers;
	struct _crx_curl_free        *to_free;
	struct _crx_curl_send_headers header;
	struct _crx_curl_error        err;
	bool                     in_callback;
	uint32_t*                     clone;
	zval                          postfields;
	/* For CURLOPT_PRIVATE */
	zval private_data;
	/* CurlShareHandle object set using CURLOPT_SHARE. */
	struct _crx_curlsh *share;
	crex_object                   std;
} crx_curl;

#define CURLOPT_SAFE_UPLOAD -1

typedef struct {
	crx_curl_callback	*server_push;
} crx_curlm_handlers;

typedef struct {
	CURLM      *multi;
	crex_llist  easyh;
	crx_curlm_handlers handlers;
	struct {
		int no;
	} err;
	crex_object std;
} crx_curlm;

typedef struct _crx_curlsh {
	CURLSH                   *share;
	struct {
		int no;
	} err;
	crex_object std;
} crx_curlsh;

crx_curl *init_curl_handle_into_zval(zval *curl);
void init_curl_handle(crx_curl *ch);
void _crx_curl_cleanup_handle(crx_curl *);
void _crx_curl_multi_cleanup_list(void *data);
void _crx_curl_verify_handlers(crx_curl *ch, bool reporterror);
void _crx_setup_easy_copy_handlers(crx_curl *ch, crx_curl *source);

static inline crx_curl *curl_from_obj(crex_object *obj) {
	return (crx_curl *)((char *)(obj) - XtOffsetOf(crx_curl, std));
}

#define C_CURL_P(zv) curl_from_obj(C_OBJ_P(zv))

static inline crx_curlsh *curl_share_from_obj(crex_object *obj) {
	return (crx_curlsh *)((char *)(obj) - XtOffsetOf(crx_curlsh, std));
}

#define C_CURL_SHARE_P(zv) curl_share_from_obj(C_OBJ_P(zv))

void curl_multi_register_handlers(void);
void curl_share_register_handlers(void);
void curlfile_register_class(void);
crex_result curl_cast_object(crex_object *obj, zval *result, int type);

#endif  /* _CRX_CURL_PRIVATE_H */
