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
   | Author: Moriyoshi Koizumi <moriyoshi@crx.net>                        |
   +----------------------------------------------------------------------+
 */

#ifndef _CRX_MBREGEX_H
#define _CRX_MBREGEX_H

#ifdef HAVE_MBREGEX

#include "crx.h"
#include "crex.h"

#define CRX_MBREGEX_MAXCACHE 50

CRX_MINIT_FUNCTION(mb_regex);
CRX_MSHUTDOWN_FUNCTION(mb_regex);
CRX_RINIT_FUNCTION(mb_regex);
CRX_RSHUTDOWN_FUNCTION(mb_regex);
CRX_MINFO_FUNCTION(mb_regex);

extern char crx_mb_oniguruma_version[256];

typedef struct _crex_mb_regex_globals crex_mb_regex_globals;

crex_mb_regex_globals *crx_mb_regex_globals_alloc(void);
void crx_mb_regex_globals_free(crex_mb_regex_globals *pglobals);
int crx_mb_regex_set_mbctype(const char *enc);
int crx_mb_regex_set_default_mbctype(const char *encname);
const char *crx_mb_regex_get_mbctype(void);
const char *crx_mb_regex_get_default_mbctype(void);

#endif /* HAVE_MBREGEX */

#endif /* _CRX_MBREGEX_H */
