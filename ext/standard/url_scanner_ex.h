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
  | Author: Sascha Schumann <sascha@schumann.cx>                         |
  +----------------------------------------------------------------------+
*/

#ifndef URL_SCANNER_EX_H
#define URL_SCANNER_EX_H

CRX_MINIT_FUNCTION(url_scanner_ex);
CRX_MSHUTDOWN_FUNCTION(url_scanner_ex);

CRX_RINIT_FUNCTION(url_scanner_ex);
CRX_RSHUTDOWN_FUNCTION(url_scanner_ex);

CRXAPI char *crx_url_scanner_adapt_single_url(const char *url, size_t urllen, const char *name, const char *value, size_t *newlen, int encode);
CRXAPI int crx_url_scanner_add_session_var(const char *name, size_t name_len, const char *value, size_t value_len, int encode);
CRXAPI int crx_url_scanner_reset_session_var(crex_string *name, int encode);
CRXAPI int crx_url_scanner_reset_session_vars(void);
CRXAPI int crx_url_scanner_add_var(const char *name, size_t name_len, const char *value, size_t value_len, int encode);
CRXAPI int crx_url_scanner_reset_var(crex_string *name, int encode);
CRXAPI int crx_url_scanner_reset_vars(void);

#include "crex_smart_str_public.h"

typedef struct {
	/* Used by the mainloop of the scanner */
	smart_str tag; /* read only */
	smart_str arg; /* read only */
	smart_str val; /* read only */
	smart_str buf;

	/* The result buffer */
	smart_str result;

	/* The data which is appended to each relative URL/FORM */
	smart_str form_app, url_app;

	int active;

	char *lookup_data;
	int state;

	int type;
	smart_str attr_val;
	int tag_type;
	int attr_type;

	/* Everything above is zeroed in RINIT */
	HashTable *tags;
} url_adapt_state_ex_t;

#endif
