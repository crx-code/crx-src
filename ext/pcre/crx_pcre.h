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
   | Author: Andrei Zmievski <andrei@crx.net>                             |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_PCRE_H
#define CRX_PCRE_H

#ifdef HAVE_BUNDLED_PCRE
#include "pcre2lib/pcre2.h"
#else
#include "pcre2.h"
#endif

#include <locale.h>

CRXAPI crex_string *crx_pcre_replace(crex_string *regex, crex_string *subject_str, const char *subject, size_t subject_len, crex_string *replace_str, size_t limit, size_t *replace_count);
CRXAPI pcre2_code* pcre_get_compiled_regex(crex_string *regex, uint32_t *capture_count);

extern crex_module_entry pcre_module_entry;
#define pcre_module_ptr &pcre_module_entry

#include "crx_version.h"
#define CRX_PCRE_VERSION CRX_VERSION

typedef struct _pcre_cache_entry pcre_cache_entry;

typedef enum {
    CRX_PCRE_NO_ERROR = 0,
    CRX_PCRE_INTERNAL_ERROR,
    CRX_PCRE_BACKTRACK_LIMIT_ERROR,
    CRX_PCRE_RECURSION_LIMIT_ERROR,
    CRX_PCRE_BAD_UTF8_ERROR,
    CRX_PCRE_BAD_UTF8_OFFSET_ERROR,
    CRX_PCRE_JIT_STACKLIMIT_ERROR
} crx_pcre_error_code;

CRXAPI pcre_cache_entry* pcre_get_compiled_regex_cache(crex_string *regex);
CRXAPI pcre_cache_entry* pcre_get_compiled_regex_cache_ex(crex_string *regex, int locale_aware);

CRXAPI void  crx_pcre_match_impl(pcre_cache_entry *pce, crex_string *subject_str, zval *return_value,
	zval *subpats, int global, int use_flags, crex_long flags, crex_off_t start_offset);

CRXAPI crex_string *crx_pcre_replace_impl(pcre_cache_entry *pce, crex_string *subject_str, const char *subject, size_t subject_len, crex_string *replace_str,
	size_t limit, size_t *replace_count);

CRXAPI void  crx_pcre_split_impl(  pcre_cache_entry *pce, crex_string *subject_str, zval *return_value,
	crex_long limit_val, crex_long flags);

CRXAPI void  crx_pcre_grep_impl(   pcre_cache_entry *pce, zval *input, zval *return_value,
	crex_long flags);

CRXAPI pcre2_match_context *crx_pcre_mctx(void);
CRXAPI pcre2_general_context *crx_pcre_gctx(void);
CRXAPI pcre2_compile_context *crx_pcre_cctx(void);
CRXAPI void crx_pcre_pce_incref(pcre_cache_entry *);
CRXAPI void crx_pcre_pce_decref(pcre_cache_entry *);
CRXAPI pcre2_code *crx_pcre_pce_re(pcre_cache_entry *);
/* capture_count can be ignored, re is required. */
CRXAPI pcre2_match_data *crx_pcre_create_match_data(uint32_t, pcre2_code *);
CRXAPI void crx_pcre_free_match_data(pcre2_match_data *);

CREX_BEGIN_MODULE_GLOBALS(pcre)
	HashTable pcre_cache;
	crex_long backtrack_limit;
	crex_long recursion_limit;
#ifdef HAVE_PCRE_JIT_SUPPORT
	bool jit;
#endif
	bool per_request_cache;
	crx_pcre_error_code error_code;
	/* Used for unmatched subpatterns in OFFSET_CAPTURE mode */
	zval unmatched_null_pair;
	zval unmatched_empty_pair;
	/* General context using per-request allocator (ZMM). */
	pcre2_general_context *gctx_zmm;
CREX_END_MODULE_GLOBALS(pcre)

CRXAPI CREX_EXTERN_MODULE_GLOBALS(pcre)
#define PCRE_G(v) CREX_MODULE_GLOBALS_ACCESSOR(pcre, v)

#define crxext_pcre_ptr pcre_module_ptr

#endif /* CRX_PCRE_H */
