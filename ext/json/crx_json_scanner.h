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
  | Author: Jakub Zelenka <bukka@crx.net>                                |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_JSON_SCANNER_H
#define	CRX_JSON_SCANNER_H

#include "crx.h"
#include "crx_json.h"

typedef unsigned char crx_json_ctype;

typedef struct _crx_json_scanner {
	crx_json_ctype *cursor;         /* cursor position */
	crx_json_ctype *token;          /* token position */
	crx_json_ctype *limit;          /* the last read character + 1 position */
	crx_json_ctype *marker;         /* marker position for backtracking */
	crx_json_ctype *ctxmarker;      /* marker position for context backtracking */
	crx_json_ctype *str_start;      /* start position of the string */
	crx_json_ctype *pstr;           /* string pointer for escapes conversion */
	zval value;                     /* value */
	int str_esc;                    /* number of extra characters for escaping */
	int state;                      /* condition state */
	int options;                    /* options */
	crx_json_error_code errcode;    /* error type if there is an error */
	int utf8_invalid;               /* whether utf8 is invalid */
	int utf8_invalid_count;         /* number of extra character for invalid utf8 */
} crx_json_scanner;


void crx_json_scanner_init(crx_json_scanner *scanner, const char *str, size_t str_len, int options);
int crx_json_scan(crx_json_scanner *s);

#endif	/* CRX_JSON_SCANNER_H */
