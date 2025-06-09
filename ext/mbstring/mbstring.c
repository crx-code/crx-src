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
   | Author: Tsukada Takuya <tsukada@fminn.nagano.nagano.jp>              |
   |         Rui Hirokawa <hirokawa@crx.net>                              |
   |         Hironori Sato <satoh@jpnnet.com>                             |
   |         Shigeru Kanemoto <sgk@happysize.co.jp>                       |
   +----------------------------------------------------------------------+
*/

/* {{{ includes */
#include <limits.h>

#include "libmbfl/config.h"
#include "crx.h"
#include "crx_ini.h"
#include "crx_variables.h"
#include "mbstring.h"
#include "ext/standard/crx_string.h"
#include "ext/standard/crx_mail.h"
#include "ext/standard/exec.h"
#include "ext/standard/url.h"
#include "main/crx_output.h"
#include "ext/standard/info.h"
#include "ext/pcre/crx_pcre.h"

#include "libmbfl/mbfl/mbfilter_8bit.h"
#include "libmbfl/mbfl/mbfilter_pass.h"
#include "libmbfl/mbfl/mbfilter_wchar.h"
#include "libmbfl/mbfl/eaw_table.h"
#include "libmbfl/filters/mbfilter_base64.h"
#include "libmbfl/filters/mbfilter_qprint.h"
#include "libmbfl/filters/mbfilter_htmlent.h"
#include "libmbfl/filters/mbfilter_uuencode.h"
#include "libmbfl/filters/mbfilter_ucs4.h"
#include "libmbfl/filters/mbfilter_utf8.h"
#include "libmbfl/filters/mbfilter_utf16.h"
#include "libmbfl/filters/mbfilter_singlebyte.h"
#include "libmbfl/filters/translit_kana_jisx0201_jisx0208.h"
#include "libmbfl/filters/unicode_prop.h"

#include "crx_variables.h"
#include "crx_globals.h"
#include "rfc1867.h"
#include "crx_content_types.h"
#include "SAPI.h"
#include "crx_unicode.h"
#include "TSRM.h"

#include "mb_gpc.h"

#ifdef HAVE_MBREGEX
# include "crx_mbregex.h"
#endif

#include "crex_smart_str.h"
#include "crex_multibyte.h"
#include "mbstring_arginfo.h"

#include "rare_cp_bitvec.h"

/* }}} */

/* {{{ prototypes */
CREX_DECLARE_MODULE_GLOBALS(mbstring)

static CRX_GINIT_FUNCTION(mbstring);
static CRX_GSHUTDOWN_FUNCTION(mbstring);

static void crx_mb_populate_current_detect_order_list(void);

static int crx_mb_encoding_translation(void);

static void crx_mb_gpc_get_detect_order(const crex_encoding ***list, size_t *list_size);

static void crx_mb_gpc_set_input_encoding(const crex_encoding *encoding);

static inline bool crx_mb_is_unsupported_no_encoding(enum mbfl_no_encoding no_enc);

static inline bool crx_mb_is_no_encoding_utf8(enum mbfl_no_encoding no_enc);

static bool mb_check_str_encoding(crex_string *str, const mbfl_encoding *encoding);

static const mbfl_encoding* mb_guess_encoding(unsigned char *in, size_t in_len, const mbfl_encoding **elist, unsigned int elist_size, bool strict, bool order_significant);

static crex_string* mb_mime_header_encode(crex_string *input, const mbfl_encoding *incode, const mbfl_encoding *outcode, bool base64, char *linefeed, size_t linefeed_len, crex_long indent);

/* See mbfilter_cp5022x.c */
uint32_t mb_convert_kana_codepoint(uint32_t c, uint32_t next, bool *consumed, uint32_t *second, int mode);
/* }}} */

/* {{{ crx_mb_default_identify_list */
typedef struct _crx_mb_nls_ident_list {
	enum mbfl_no_language lang;
	const enum mbfl_no_encoding *list;
	size_t list_size;
} crx_mb_nls_ident_list;

static const enum mbfl_no_encoding crx_mb_default_identify_list_ja[] = {
	mbfl_no_encoding_ascii,
	mbfl_no_encoding_jis,
	mbfl_no_encoding_utf8,
	mbfl_no_encoding_euc_jp,
	mbfl_no_encoding_sjis
};

static const enum mbfl_no_encoding crx_mb_default_identify_list_cn[] = {
	mbfl_no_encoding_ascii,
	mbfl_no_encoding_utf8,
	mbfl_no_encoding_euc_cn,
	mbfl_no_encoding_cp936
};

static const enum mbfl_no_encoding crx_mb_default_identify_list_tw_hk[] = {
	mbfl_no_encoding_ascii,
	mbfl_no_encoding_utf8,
	mbfl_no_encoding_euc_tw,
	mbfl_no_encoding_big5
};

static const enum mbfl_no_encoding crx_mb_default_identify_list_kr[] = {
	mbfl_no_encoding_ascii,
	mbfl_no_encoding_utf8,
	mbfl_no_encoding_euc_kr,
	mbfl_no_encoding_uhc
};

static const enum mbfl_no_encoding crx_mb_default_identify_list_ru[] = {
	mbfl_no_encoding_ascii,
	mbfl_no_encoding_utf8,
	mbfl_no_encoding_koi8r,
	mbfl_no_encoding_cp1251,
	mbfl_no_encoding_cp866
};

static const enum mbfl_no_encoding crx_mb_default_identify_list_hy[] = {
	mbfl_no_encoding_ascii,
	mbfl_no_encoding_utf8,
	mbfl_no_encoding_armscii8
};

static const enum mbfl_no_encoding crx_mb_default_identify_list_tr[] = {
	mbfl_no_encoding_ascii,
	mbfl_no_encoding_utf8,
	mbfl_no_encoding_cp1254,
	mbfl_no_encoding_8859_9
};

static const enum mbfl_no_encoding crx_mb_default_identify_list_ua[] = {
	mbfl_no_encoding_ascii,
	mbfl_no_encoding_utf8,
	mbfl_no_encoding_koi8u
};

static const enum mbfl_no_encoding crx_mb_default_identify_list_neut[] = {
	mbfl_no_encoding_ascii,
	mbfl_no_encoding_utf8
};


static const crx_mb_nls_ident_list crx_mb_default_identify_list[] = {
	{ mbfl_no_language_japanese, crx_mb_default_identify_list_ja, sizeof(crx_mb_default_identify_list_ja) / sizeof(crx_mb_default_identify_list_ja[0]) },
	{ mbfl_no_language_korean, crx_mb_default_identify_list_kr, sizeof(crx_mb_default_identify_list_kr) / sizeof(crx_mb_default_identify_list_kr[0]) },
	{ mbfl_no_language_traditional_chinese, crx_mb_default_identify_list_tw_hk, sizeof(crx_mb_default_identify_list_tw_hk) / sizeof(crx_mb_default_identify_list_tw_hk[0]) },
	{ mbfl_no_language_simplified_chinese, crx_mb_default_identify_list_cn, sizeof(crx_mb_default_identify_list_cn) / sizeof(crx_mb_default_identify_list_cn[0]) },
	{ mbfl_no_language_russian, crx_mb_default_identify_list_ru, sizeof(crx_mb_default_identify_list_ru) / sizeof(crx_mb_default_identify_list_ru[0]) },
	{ mbfl_no_language_armenian, crx_mb_default_identify_list_hy, sizeof(crx_mb_default_identify_list_hy) / sizeof(crx_mb_default_identify_list_hy[0]) },
	{ mbfl_no_language_turkish, crx_mb_default_identify_list_tr, sizeof(crx_mb_default_identify_list_tr) / sizeof(crx_mb_default_identify_list_tr[0]) },
	{ mbfl_no_language_ukrainian, crx_mb_default_identify_list_ua, sizeof(crx_mb_default_identify_list_ua) / sizeof(crx_mb_default_identify_list_ua[0]) },
	{ mbfl_no_language_neutral, crx_mb_default_identify_list_neut, sizeof(crx_mb_default_identify_list_neut) / sizeof(crx_mb_default_identify_list_neut[0]) }
};

/* }}} */

/* {{{ mbstring_deps[] */
static const crex_module_dep mbstring_deps[] = {
	CREX_MOD_REQUIRED("pcre")
	CREX_MOD_END
};
/* }}} */

/* {{{ crex_module_entry mbstring_module_entry */
crex_module_entry mbstring_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	mbstring_deps,
	"mbstring",
	ext_functions,
	CRX_MINIT(mbstring),
	CRX_MSHUTDOWN(mbstring),
	CRX_RINIT(mbstring),
	CRX_RSHUTDOWN(mbstring),
	CRX_MINFO(mbstring),
	CRX_MBSTRING_VERSION,
	CRX_MODULE_GLOBALS(mbstring),
	CRX_GINIT(mbstring),
	CRX_GSHUTDOWN(mbstring),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

/* {{{ static sapi_post_entry crx_post_entries[] */
static const sapi_post_entry crx_post_entries[] = {
	{ DEFAULT_POST_CONTENT_TYPE, sizeof(DEFAULT_POST_CONTENT_TYPE)-1, sapi_read_standard_form_data,	crx_std_post_handler },
	{ MULTIPART_CONTENT_TYPE,    sizeof(MULTIPART_CONTENT_TYPE)-1,    NULL,                         rfc1867_post_handler },
	{ NULL, 0, NULL, NULL }
};
/* }}} */

#ifdef COMPILE_DL_MBSTRING
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(mbstring)
#endif

/* {{{ static sapi_post_entry mbstr_post_entries[] */
static const sapi_post_entry mbstr_post_entries[] = {
	{ DEFAULT_POST_CONTENT_TYPE, sizeof(DEFAULT_POST_CONTENT_TYPE)-1, sapi_read_standard_form_data, crx_mb_post_handler },
	{ MULTIPART_CONTENT_TYPE,    sizeof(MULTIPART_CONTENT_TYPE)-1,    NULL,                         rfc1867_post_handler },
	{ NULL, 0, NULL, NULL }
};
/* }}} */

static const mbfl_encoding *crx_mb_get_encoding(crex_string *encoding_name, uint32_t arg_num) {
	if (encoding_name) {
		const mbfl_encoding *encoding;
		crex_string *last_encoding_name = MBSTRG(last_used_encoding_name);
		if (last_encoding_name && (last_encoding_name == encoding_name
				|| crex_string_equals_ci(encoding_name, last_encoding_name))) {
			return MBSTRG(last_used_encoding);
		}

		encoding = mbfl_name2encoding(ZSTR_VAL(encoding_name));
		if (!encoding) {
			crex_argument_value_error(arg_num, "must be a valid encoding, \"%s\" given", ZSTR_VAL(encoding_name));
			return NULL;
		} else if (encoding->no_encoding <= mbfl_no_encoding_qprint) {
			if (encoding == &mbfl_encoding_base64) {
				crx_error_docref(NULL, E_DEPRECATED, "Handling Base64 via mbstring is deprecated; use base64_encode/base64_decode instead");
			} else if (encoding == &mbfl_encoding_qprint) {
				crx_error_docref(NULL, E_DEPRECATED, "Handling QPrint via mbstring is deprecated; use quoted_printable_encode/quoted_printable_decode instead");
			} else if (encoding == &mbfl_encoding_html_ent) {
				crx_error_docref(NULL, E_DEPRECATED, "Handling HTML entities via mbstring is deprecated; use htmlspecialchars, htmlentities, or mb_encode_numericentity/mb_decode_numericentity instead");
			} else if (encoding == &mbfl_encoding_uuencode) {
				crx_error_docref(NULL, E_DEPRECATED, "Handling Uuencode via mbstring is deprecated; use convert_uuencode/convert_uudecode instead");
			}
		}

		if (last_encoding_name) {
			crex_string_release(last_encoding_name);
		}
		MBSTRG(last_used_encoding_name) = crex_string_copy(encoding_name);
		MBSTRG(last_used_encoding) = encoding;
		return encoding;
	} else {
		return MBSTRG(current_internal_encoding);
	}
}

static const mbfl_encoding *crx_mb_get_encoding_or_pass(const char *encoding_name) {
	if (strcmp(encoding_name, "pass") == 0) {
		return &mbfl_encoding_pass;
	}

	return mbfl_name2encoding(encoding_name);
}

static size_t count_commas(const char *p, const char *end) {
	size_t count = 0;
	while ((p = memchr(p, ',', end - p))) {
		count++;
		p++;
	}
	return count;
}

/* {{{ static crex_result crx_mb_parse_encoding_list()
 *  Return FAILURE if input contains any illegal encoding, otherwise SUCCESS.
 * 	Emits a ValueError in function context and a warning in INI context, in INI context arg_num must be 0.
 */
static crex_result crx_mb_parse_encoding_list(const char *value, size_t value_length,
	const mbfl_encoding ***return_list, size_t *return_size, bool persistent, uint32_t arg_num,
	bool allow_pass_encoding)
{
	if (value == NULL || value_length == 0) {
		*return_list = NULL;
		*return_size = 0;
		return SUCCESS;
	} else {
		bool included_auto;
		size_t n, size;
		char *p1, *endp, *tmpstr;
		const mbfl_encoding **entry, **list;

		/* copy the value string for work */
		if (value[0]=='"' && value[value_length-1]=='"' && value_length>2) {
			tmpstr = (char *)estrndup(value+1, value_length-2);
			value_length -= 2;
		} else {
			tmpstr = (char *)estrndup(value, value_length);
		}

		endp = tmpstr + value_length;
		size = 1 + count_commas(tmpstr, endp) + MBSTRG(default_detect_order_list_size);
		list = (const mbfl_encoding **)pecalloc(size, sizeof(mbfl_encoding*), persistent);
		entry = list;
		n = 0;
		included_auto = 0;
		p1 = tmpstr;
		while (1) {
			char *comma = memchr(p1, ',', endp - p1);
			char *p = comma ? comma : endp;
			*p = '\0';
			/* trim spaces */
			while (p1 < p && (*p1 == ' ' || *p1 == '\t')) {
				p1++;
			}
			p--;
			while (p > p1 && (*p == ' ' || *p == '\t')) {
				*p = '\0';
				p--;
			}
			/* convert to the encoding number and check encoding */
			if (strcasecmp(p1, "auto") == 0) {
				if (!included_auto) {
					const enum mbfl_no_encoding *src = MBSTRG(default_detect_order_list);
					const size_t identify_list_size = MBSTRG(default_detect_order_list_size);
					size_t i;
					included_auto = 1;
					for (i = 0; i < identify_list_size; i++) {
						*entry++ = mbfl_no2encoding(*src++);
						n++;
					}
				}
			} else {
				const mbfl_encoding *encoding =
					allow_pass_encoding ? crx_mb_get_encoding_or_pass(p1) : mbfl_name2encoding(p1);
				if (!encoding) {
					/* Called from an INI setting modification */
					if (arg_num == 0) {
						crx_error_docref("ref.mbstring", E_WARNING, "INI setting contains invalid encoding \"%s\"", p1);
					} else {
						crex_argument_value_error(arg_num, "contains invalid encoding \"%s\"", p1);
					}
					efree(tmpstr);
					pefree(CREX_VOIDP(list), persistent);
					return FAILURE;
				}

				*entry++ = encoding;
				n++;
			}
			if (n >= size || comma == NULL) {
				break;
			}
			p1 = comma + 1;
		}
		*return_list = list;
		*return_size = n;
		efree(tmpstr);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ static int crx_mb_parse_encoding_array()
 *  Return FAILURE if input contains any illegal encoding, otherwise SUCCESS.
 * 	Emits a ValueError in function context and a warning in INI context, in INI context arg_num must be 0.
 */
static int crx_mb_parse_encoding_array(HashTable *target_hash, const mbfl_encoding ***return_list,
	size_t *return_size, uint32_t arg_num)
{
	/* Allocate enough space to include the default detect order if "auto" is used. */
	size_t size = crex_hash_num_elements(target_hash) + MBSTRG(default_detect_order_list_size);
	const mbfl_encoding **list = ecalloc(size, sizeof(mbfl_encoding*));
	const mbfl_encoding **entry = list;
	bool included_auto = 0;
	size_t n = 0;
	zval *hash_entry;
	CREX_HASH_FOREACH_VAL(target_hash, hash_entry) {
		crex_string *encoding_str = zval_try_get_string(hash_entry);
		if (UNEXPECTED(!encoding_str)) {
			efree(CREX_VOIDP(list));
			return FAILURE;
		}

		if (crex_string_equals_literal_ci(encoding_str, "auto")) {
			if (!included_auto) {
				const enum mbfl_no_encoding *src = MBSTRG(default_detect_order_list);
				const size_t identify_list_size = MBSTRG(default_detect_order_list_size);
				size_t j;

				included_auto = 1;
				for (j = 0; j < identify_list_size; j++) {
					*entry++ = mbfl_no2encoding(*src++);
					n++;
				}
			}
		} else {
			const mbfl_encoding *encoding = mbfl_name2encoding(ZSTR_VAL(encoding_str));
			if (encoding) {
				*entry++ = encoding;
				n++;
			} else {
				crex_argument_value_error(arg_num, "contains invalid encoding \"%s\"", ZSTR_VAL(encoding_str));
				crex_string_release(encoding_str);
				efree(CREX_VOIDP(list));
				return FAILURE;
			}
		}
		crex_string_release(encoding_str);
	} CREX_HASH_FOREACH_END();
	*return_list = list;
	*return_size = n;
	return SUCCESS;
}
/* }}} */

/* {{{ crex_multibyte interface */
static const crex_encoding* crx_mb_crex_encoding_fetcher(const char *encoding_name)
{
	return (const crex_encoding*)mbfl_name2encoding(encoding_name);
}

static const char *crx_mb_crex_encoding_name_getter(const crex_encoding *encoding)
{
	return ((const mbfl_encoding *)encoding)->name;
}

static bool crx_mb_crex_encoding_lexer_compatibility_checker(const crex_encoding *_encoding)
{
	const mbfl_encoding *encoding = (const mbfl_encoding*)_encoding;
	return !(encoding->flag & MBFL_ENCTYPE_GL_UNSAFE);
}

static const crex_encoding *crx_mb_crex_encoding_detector(const unsigned char *arg_string, size_t arg_length, const crex_encoding **list, size_t list_size)
{
	if (!list) {
		list = (const crex_encoding**)MBSTRG(current_detect_order_list);
		list_size = MBSTRG(current_detect_order_list_size);
	}

	return (const crex_encoding*)mb_guess_encoding((unsigned char*)arg_string, arg_length, (const mbfl_encoding **)list, list_size, false, false);
}

static size_t crx_mb_crex_encoding_converter(unsigned char **to, size_t *to_length, const unsigned char *from, size_t from_length, const crex_encoding *encoding_to, const crex_encoding *encoding_from)
{
	unsigned int num_errors = 0;
	crex_string *result = mb_fast_convert((unsigned char*)from, from_length, (const mbfl_encoding*)encoding_from, (const mbfl_encoding*)encoding_to, MBSTRG(current_filter_illegal_substchar), MBSTRG(current_filter_illegal_mode), &num_errors);

	*to_length = ZSTR_LEN(result);
	*to = emalloc(ZSTR_LEN(result) + 1); /* Include terminating null byte */
	memcpy(*to, ZSTR_VAL(result), ZSTR_LEN(result) + 1);
	crex_string_free(result);

	return from_length;
}

static crex_result crx_mb_crex_encoding_list_parser(const char *encoding_list, size_t encoding_list_len, const crex_encoding ***return_list, size_t *return_size, bool persistent)
{
	return crx_mb_parse_encoding_list(
		encoding_list, encoding_list_len,
		(const mbfl_encoding ***)return_list, return_size,
		persistent, /* arg_num */ 0, /* allow_pass_encoding */ 1);
}

static const crex_encoding *crx_mb_crex_internal_encoding_getter(void)
{
	return (const crex_encoding *)MBSTRG(internal_encoding);
}

static crex_result crx_mb_crex_internal_encoding_setter(const crex_encoding *encoding)
{
	MBSTRG(internal_encoding) = (const mbfl_encoding *)encoding;
	return SUCCESS;
}

static crex_multibyte_functions crx_mb_crex_multibyte_functions = {
	"mbstring",
	crx_mb_crex_encoding_fetcher,
	crx_mb_crex_encoding_name_getter,
	crx_mb_crex_encoding_lexer_compatibility_checker,
	crx_mb_crex_encoding_detector,
	crx_mb_crex_encoding_converter,
	crx_mb_crex_encoding_list_parser,
	crx_mb_crex_internal_encoding_getter,
	crx_mb_crex_internal_encoding_setter
};
/* }}} */

/* {{{ _crx_mb_compile_regex */
static void *_crx_mb_compile_regex(const char *pattern)
{
	pcre2_code *retval;
	PCRE2_SIZE err_offset;
	int errnum;

	if (!(retval = pcre2_compile((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED,
			PCRE2_CASELESS, &errnum, &err_offset, crx_pcre_cctx()))) {
		PCRE2_UCHAR err_str[128];
		pcre2_get_error_message(errnum, err_str, sizeof(err_str));
		crx_error_docref(NULL, E_WARNING, "%s (offset=%zu): %s", pattern, err_offset, err_str);
	}
	return retval;
}
/* }}} */

/* {{{ _crx_mb_match_regex */
static int _crx_mb_match_regex(void *opaque, const char *str, size_t str_len)
{
	int res;

	pcre2_match_data *match_data = crx_pcre_create_match_data(0, opaque);
	if (NULL == match_data) {
		pcre2_code_free(opaque);
		crx_error_docref(NULL, E_WARNING, "Cannot allocate match data");
		return FAILURE;
	}
	res = pcre2_match(opaque, (PCRE2_SPTR)str, str_len, 0, 0, match_data, crx_pcre_mctx()) >= 0;
	crx_pcre_free_match_data(match_data);

	return res;
}
/* }}} */

/* {{{ _crx_mb_free_regex */
static void _crx_mb_free_regex(void *opaque)
{
	pcre2_code_free(opaque);
}
/* }}} */

/* {{{ crx_mb_nls_get_default_detect_order_list */
static int crx_mb_nls_get_default_detect_order_list(enum mbfl_no_language lang, enum mbfl_no_encoding **plist, size_t *plist_size)
{
	size_t i;

	*plist = (enum mbfl_no_encoding *) crx_mb_default_identify_list_neut;
	*plist_size = sizeof(crx_mb_default_identify_list_neut) / sizeof(crx_mb_default_identify_list_neut[0]);

	for (i = 0; i < sizeof(crx_mb_default_identify_list) / sizeof(crx_mb_default_identify_list[0]); i++) {
		if (crx_mb_default_identify_list[i].lang == lang) {
			*plist = (enum mbfl_no_encoding *)crx_mb_default_identify_list[i].list;
			*plist_size = crx_mb_default_identify_list[i].list_size;
			return 1;
		}
	}
	return 0;
}
/* }}} */

static char *crx_mb_rfc1867_substring_conf(const crex_encoding *encoding, char *start, size_t len, char quote)
{
	char *result = emalloc(len + 2);
	char *resp = result;
	size_t i;

	for (i = 0; i < len && start[i] != quote; ++i) {
		if (start[i] == '\\' && (start[i + 1] == '\\' || (quote && start[i + 1] == quote))) {
			*resp++ = start[++i];
		} else {
			size_t j = crx_mb_mbchar_bytes(start+i, (const mbfl_encoding *)encoding);

			while (j-- > 0 && i < len) {
				*resp++ = start[i++];
			}
			--i;
		}
	}

	*resp = '\0';
	return result;
}

static char *crx_mb_rfc1867_getword(const crex_encoding *encoding, char **line, char stop) /* {{{ */
{
	char *pos = *line, quote;
	char *res;

	while (*pos && *pos != stop) {
		if ((quote = *pos) == '"' || quote == '\'') {
			++pos;
			while (*pos && *pos != quote) {
				if (*pos == '\\' && pos[1] && pos[1] == quote) {
					pos += 2;
				} else {
					++pos;
				}
			}
			if (*pos) {
				++pos;
			}
		} else {
			pos += crx_mb_mbchar_bytes(pos, (const mbfl_encoding *)encoding);

		}
	}
	if (*pos == '\0') {
		res = estrdup(*line);
		*line += strlen(*line);
		return res;
	}

	res = estrndup(*line, pos - *line);

	while (*pos == stop) {
		pos += crx_mb_mbchar_bytes(pos, (const mbfl_encoding *)encoding);
	}

	*line = pos;
	return res;
}
/* }}} */

static char *crx_mb_rfc1867_getword_conf(const crex_encoding *encoding, char *str) /* {{{ */
{
	while (*str && isspace(*(unsigned char *)str)) {
		++str;
	}

	if (!*str) {
		return estrdup("");
	}

	if (*str == '"' || *str == '\'') {
		char quote = *str;

		str++;
		return crx_mb_rfc1867_substring_conf(encoding, str, strlen(str), quote);
	} else {
		char *strend = str;

		while (*strend && !isspace(*(unsigned char *)strend)) {
			++strend;
		}
		return crx_mb_rfc1867_substring_conf(encoding, str, strend - str, 0);
	}
}
/* }}} */

static char *crx_mb_rfc1867_basename(const crex_encoding *encoding, char *filename) /* {{{ */
{
	char *s, *s2;
	const size_t filename_len = strlen(filename);

	/* The \ check should technically be needed for win32 systems only where
	 * it is a valid path separator. However, IE in all it's wisdom always sends
	 * the full path of the file on the user's filesystem, which means that unless
	 * the user does basename() they get a bogus file name. Until IE's user base drops
	 * to nill or problem is fixed this code must remain enabled for all systems. */
	s = crx_mb_safe_strrchr(filename, '\\', filename_len, (const mbfl_encoding *)encoding);
	s2 = crx_mb_safe_strrchr(filename, '/', filename_len, (const mbfl_encoding *)encoding);

	if (s && s2) {
		if (s > s2) {
			return ++s;
		} else {
			return ++s2;
		}
	} else if (s) {
		return ++s;
	} else if (s2) {
		return ++s2;
	} else {
		return filename;
	}
}
/* }}} */

/* {{{ crx.ini directive handler */
/* {{{ static CRX_INI_MH(OnUpdate_mbstring_language) */
static CRX_INI_MH(OnUpdate_mbstring_language)
{
	enum mbfl_no_language no_language;

	no_language = mbfl_name2no_language(ZSTR_VAL(new_value));
	if (no_language == mbfl_no_language_invalid) {
		MBSTRG(language) = mbfl_no_language_neutral;
		return FAILURE;
	}
	MBSTRG(language) = no_language;
	crx_mb_nls_get_default_detect_order_list(no_language, &MBSTRG(default_detect_order_list), &MBSTRG(default_detect_order_list_size));
	return SUCCESS;
}
/* }}} */

/* {{{ static CRX_INI_MH(OnUpdate_mbstring_detect_order) */
static CRX_INI_MH(OnUpdate_mbstring_detect_order)
{
	const mbfl_encoding **list;
	size_t size;

	if (!new_value) {
		if (MBSTRG(detect_order_list)) {
			pefree(CREX_VOIDP(MBSTRG(detect_order_list)), 1);
		}
		MBSTRG(detect_order_list) = NULL;
		MBSTRG(detect_order_list_size) = 0;
		return SUCCESS;
	}

	if (FAILURE == crx_mb_parse_encoding_list(ZSTR_VAL(new_value), ZSTR_LEN(new_value), &list, &size, /* persistent */ 1, /* arg_num */ 0, /* allow_pass_encoding */ 0) || size == 0) {
		return FAILURE;
	}

	if (MBSTRG(detect_order_list)) {
		pefree(CREX_VOIDP(MBSTRG(detect_order_list)), 1);
	}
	MBSTRG(detect_order_list) = list;
	MBSTRG(detect_order_list_size) = size;
	return SUCCESS;
}
/* }}} */

static int _crx_mb_ini_mbstring_http_input_set(const char *new_value, size_t new_value_length) {
	const mbfl_encoding **list;
	size_t size;
	if (FAILURE == crx_mb_parse_encoding_list(new_value, new_value_length, &list, &size, /* persistent */ 1, /* arg_num */ 0, /* allow_pass_encoding */ 1) || size == 0) {
		return FAILURE;
	}
	if (MBSTRG(http_input_list)) {
		pefree(CREX_VOIDP(MBSTRG(http_input_list)), 1);
	}
	MBSTRG(http_input_list) = list;
	MBSTRG(http_input_list_size) = size;
	return SUCCESS;
}

/* {{{ static CRX_INI_MH(OnUpdate_mbstring_http_input) */
static CRX_INI_MH(OnUpdate_mbstring_http_input)
{
	if (new_value) {
		crx_error_docref("ref.mbstring", E_DEPRECATED, "Use of mbstring.http_input is deprecated");
	}

	if (!new_value || !ZSTR_LEN(new_value)) {
		const char *encoding = crx_get_input_encoding();
		MBSTRG(http_input_set) = 0;
		_crx_mb_ini_mbstring_http_input_set(encoding, strlen(encoding));
		return SUCCESS;
	}

	MBSTRG(http_input_set) = 1;
	return _crx_mb_ini_mbstring_http_input_set(ZSTR_VAL(new_value), ZSTR_LEN(new_value));
}
/* }}} */

static int _crx_mb_ini_mbstring_http_output_set(const char *new_value) {
	const mbfl_encoding *encoding = crx_mb_get_encoding_or_pass(new_value);
	if (!encoding) {
		return FAILURE;
	}

	MBSTRG(http_output_encoding) = encoding;
	MBSTRG(current_http_output_encoding) = encoding;
	return SUCCESS;
}

/* {{{ static CRX_INI_MH(OnUpdate_mbstring_http_output) */
static CRX_INI_MH(OnUpdate_mbstring_http_output)
{
	if (new_value) {
		crx_error_docref("ref.mbstring", E_DEPRECATED, "Use of mbstring.http_output is deprecated");
	}

	if (new_value == NULL || ZSTR_LEN(new_value) == 0) {
		MBSTRG(http_output_set) = 0;
		_crx_mb_ini_mbstring_http_output_set(crx_get_output_encoding());
		return SUCCESS;
	}

	MBSTRG(http_output_set) = 1;
	return _crx_mb_ini_mbstring_http_output_set(ZSTR_VAL(new_value));
}
/* }}} */

/* {{{ static _crx_mb_ini_mbstring_internal_encoding_set */
static int _crx_mb_ini_mbstring_internal_encoding_set(const char *new_value, size_t new_value_length)
{
	const mbfl_encoding *encoding;

	if (!new_value || !new_value_length || !(encoding = mbfl_name2encoding(new_value))) {
		/* falls back to UTF-8 if an unknown encoding name is given */
		if (new_value) {
			crx_error_docref("ref.mbstring", E_WARNING, "Unknown encoding \"%s\" in ini setting", new_value);
		}
		encoding = &mbfl_encoding_utf8;
	}
	MBSTRG(internal_encoding) = encoding;
	MBSTRG(current_internal_encoding) = encoding;
#ifdef HAVE_MBREGEX
	{
		const char *enc_name = new_value;
		if (FAILURE == crx_mb_regex_set_default_mbctype(enc_name)) {
			/* falls back to UTF-8 if an unknown encoding name is given */
			enc_name = "UTF-8";
			crx_mb_regex_set_default_mbctype(enc_name);
		}
		crx_mb_regex_set_mbctype(new_value);
	}
#endif
	return SUCCESS;
}
/* }}} */

/* {{{ static CRX_INI_MH(OnUpdate_mbstring_internal_encoding) */
static CRX_INI_MH(OnUpdate_mbstring_internal_encoding)
{
	if (new_value) {
		crx_error_docref("ref.mbstring", E_DEPRECATED, "Use of mbstring.internal_encoding is deprecated");
	}

	if (OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage) == FAILURE) {
		return FAILURE;
	}

	if (new_value && ZSTR_LEN(new_value)) {
		MBSTRG(internal_encoding_set) = 1;
		return _crx_mb_ini_mbstring_internal_encoding_set(ZSTR_VAL(new_value), ZSTR_LEN(new_value));
	} else {
		const char *encoding = crx_get_internal_encoding();
		MBSTRG(internal_encoding_set) = 0;
		return _crx_mb_ini_mbstring_internal_encoding_set(encoding, strlen(encoding));
	}
}
/* }}} */

/* {{{ static CRX_INI_MH(OnUpdate_mbstring_substitute_character) */
static CRX_INI_MH(OnUpdate_mbstring_substitute_character)
{
	int c;
	char *endptr = NULL;

	if (new_value != NULL) {
		if (crex_string_equals_literal_ci(new_value, "none")) {
			MBSTRG(filter_illegal_mode) = MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE;
			MBSTRG(current_filter_illegal_mode) = MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE;
		} else if (crex_string_equals_literal_ci(new_value, "long")) {
			MBSTRG(filter_illegal_mode) = MBFL_OUTPUTFILTER_ILLEGAL_MODE_LONG;
			MBSTRG(current_filter_illegal_mode) = MBFL_OUTPUTFILTER_ILLEGAL_MODE_LONG;
		} else if (crex_string_equals_literal_ci(new_value, "entity")) {
			MBSTRG(filter_illegal_mode) = MBFL_OUTPUTFILTER_ILLEGAL_MODE_ENTITY;
			MBSTRG(current_filter_illegal_mode) = MBFL_OUTPUTFILTER_ILLEGAL_MODE_ENTITY;
		} else {
			MBSTRG(filter_illegal_mode) = MBFL_OUTPUTFILTER_ILLEGAL_MODE_CHAR;
			MBSTRG(current_filter_illegal_mode) = MBFL_OUTPUTFILTER_ILLEGAL_MODE_CHAR;
			if (ZSTR_LEN(new_value) > 0) {
				c = strtol(ZSTR_VAL(new_value), &endptr, 0);
				if (*endptr == '\0') {
					MBSTRG(filter_illegal_substchar) = c;
					MBSTRG(current_filter_illegal_substchar) = c;
				}
			}
		}
	} else {
		MBSTRG(filter_illegal_mode) = MBFL_OUTPUTFILTER_ILLEGAL_MODE_CHAR;
		MBSTRG(current_filter_illegal_mode) = MBFL_OUTPUTFILTER_ILLEGAL_MODE_CHAR;
		MBSTRG(filter_illegal_substchar) = '?';
		MBSTRG(current_filter_illegal_substchar) = '?';
	}

	return SUCCESS;
}
/* }}} */

/* {{{ static CRX_INI_MH(OnUpdate_mbstring_encoding_translation) */
static CRX_INI_MH(OnUpdate_mbstring_encoding_translation)
{
	if (new_value == NULL) {
		return FAILURE;
	}

	OnUpdateBool(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);

	if (MBSTRG(encoding_translation)) {
		sapi_unregister_post_entry(crx_post_entries);
		sapi_register_post_entries(mbstr_post_entries);
	} else {
		sapi_unregister_post_entry(mbstr_post_entries);
		sapi_register_post_entries(crx_post_entries);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ static CRX_INI_MH(OnUpdate_mbstring_http_output_conv_mimetypes */
static CRX_INI_MH(OnUpdate_mbstring_http_output_conv_mimetypes)
{
	crex_string *tmp;
	void *re = NULL;

	if (!new_value) {
		new_value = entry->orig_value;
	}
	tmp = crx_trim(new_value, NULL, 0, 3);

	if (ZSTR_LEN(tmp) > 0) {
		if (!(re = _crx_mb_compile_regex(ZSTR_VAL(tmp)))) {
			crex_string_release_ex(tmp, 0);
			return FAILURE;
		}
	}

	if (MBSTRG(http_output_conv_mimetypes)) {
		_crx_mb_free_regex(MBSTRG(http_output_conv_mimetypes));
	}

	MBSTRG(http_output_conv_mimetypes) = re;

	crex_string_release_ex(tmp, 0);
	return SUCCESS;
}
/* }}} */
/* }}} */

/* {{{ crx.ini directive registration */
CRX_INI_BEGIN()
	CRX_INI_ENTRY("mbstring.language", "neutral", CRX_INI_ALL, OnUpdate_mbstring_language)
	CRX_INI_ENTRY("mbstring.detect_order", NULL, CRX_INI_ALL, OnUpdate_mbstring_detect_order)
	CRX_INI_ENTRY("mbstring.http_input", NULL, CRX_INI_ALL, OnUpdate_mbstring_http_input)
	CRX_INI_ENTRY("mbstring.http_output", NULL, CRX_INI_ALL, OnUpdate_mbstring_http_output)
	STD_CRX_INI_ENTRY("mbstring.internal_encoding", NULL, CRX_INI_ALL, OnUpdate_mbstring_internal_encoding, internal_encoding_name, crex_mbstring_globals, mbstring_globals)
	CRX_INI_ENTRY("mbstring.substitute_character", NULL, CRX_INI_ALL, OnUpdate_mbstring_substitute_character)

	STD_CRX_INI_BOOLEAN("mbstring.encoding_translation", "0",
		CRX_INI_SYSTEM | CRX_INI_PERDIR,
		OnUpdate_mbstring_encoding_translation,
		encoding_translation, crex_mbstring_globals, mbstring_globals)
	CRX_INI_ENTRY("mbstring.http_output_conv_mimetypes",
		"^(text/|application/xhtml\\+xml)",
		CRX_INI_ALL,
		OnUpdate_mbstring_http_output_conv_mimetypes)

	STD_CRX_INI_BOOLEAN("mbstring.strict_detection", "0",
		CRX_INI_ALL,
		OnUpdateBool,
		strict_detection, crex_mbstring_globals, mbstring_globals)
#ifdef HAVE_MBREGEX
	STD_CRX_INI_ENTRY("mbstring.regex_stack_limit", "100000",CRX_INI_ALL, OnUpdateLong, regex_stack_limit, crex_mbstring_globals, mbstring_globals)
	STD_CRX_INI_ENTRY("mbstring.regex_retry_limit", "1000000",CRX_INI_ALL, OnUpdateLong, regex_retry_limit, crex_mbstring_globals, mbstring_globals)
#endif
CRX_INI_END()
/* }}} */

static void mbstring_internal_encoding_changed_hook(void) {
	/* One of the internal_encoding / input_encoding / output_encoding ini settings changed. */
	if (!MBSTRG(internal_encoding_set)) {
		const char *encoding = crx_get_internal_encoding();
		_crx_mb_ini_mbstring_internal_encoding_set(encoding, strlen(encoding));
	}

	if (!MBSTRG(http_output_set)) {
		const char *encoding = crx_get_output_encoding();
		_crx_mb_ini_mbstring_http_output_set(encoding);
	}

	if (!MBSTRG(http_input_set)) {
		const char *encoding = crx_get_input_encoding();
		_crx_mb_ini_mbstring_http_input_set(encoding, strlen(encoding));
	}
}

/* {{{ module global initialize handler */
static CRX_GINIT_FUNCTION(mbstring)
{
#if defined(COMPILE_DL_MBSTRING) && defined(ZTS)
CREX_TSRMLS_CACHE_UPDATE();
#endif

	mbstring_globals->language = mbfl_no_language_uni;
	mbstring_globals->internal_encoding = NULL;
	mbstring_globals->current_internal_encoding = mbstring_globals->internal_encoding;
	mbstring_globals->http_output_encoding = &mbfl_encoding_pass;
	mbstring_globals->current_http_output_encoding = &mbfl_encoding_pass;
	mbstring_globals->http_input_identify = NULL;
	mbstring_globals->http_input_identify_get = NULL;
	mbstring_globals->http_input_identify_post = NULL;
	mbstring_globals->http_input_identify_cookie = NULL;
	mbstring_globals->http_input_identify_string = NULL;
	mbstring_globals->http_input_list = NULL;
	mbstring_globals->http_input_list_size = 0;
	mbstring_globals->detect_order_list = NULL;
	mbstring_globals->detect_order_list_size = 0;
	mbstring_globals->current_detect_order_list = NULL;
	mbstring_globals->current_detect_order_list_size = 0;
	mbstring_globals->default_detect_order_list = (enum mbfl_no_encoding *) crx_mb_default_identify_list_neut;
	mbstring_globals->default_detect_order_list_size = sizeof(crx_mb_default_identify_list_neut) / sizeof(crx_mb_default_identify_list_neut[0]);
	mbstring_globals->filter_illegal_mode = MBFL_OUTPUTFILTER_ILLEGAL_MODE_CHAR;
	mbstring_globals->filter_illegal_substchar = '?';
	mbstring_globals->current_filter_illegal_mode = MBFL_OUTPUTFILTER_ILLEGAL_MODE_CHAR;
	mbstring_globals->current_filter_illegal_substchar = '?';
	mbstring_globals->illegalchars = 0;
	mbstring_globals->encoding_translation = 0;
	mbstring_globals->strict_detection = 0;
	mbstring_globals->outconv_enabled = false;
	mbstring_globals->outconv_state = 0;
	mbstring_globals->http_output_conv_mimetypes = NULL;
#ifdef HAVE_MBREGEX
	mbstring_globals->mb_regex_globals = crx_mb_regex_globals_alloc();
#endif
	mbstring_globals->last_used_encoding_name = NULL;
	mbstring_globals->last_used_encoding = NULL;
	mbstring_globals->internal_encoding_set = 0;
	mbstring_globals->http_output_set = 0;
	mbstring_globals->http_input_set = 0;
	mbstring_globals->all_encodings_list = NULL;
}
/* }}} */

/* {{{ CRX_GSHUTDOWN_FUNCTION */
static CRX_GSHUTDOWN_FUNCTION(mbstring)
{
	if (mbstring_globals->http_input_list) {
		free(CREX_VOIDP(mbstring_globals->http_input_list));
	}
	if (mbstring_globals->detect_order_list) {
		free(CREX_VOIDP(mbstring_globals->detect_order_list));
	}
	if (mbstring_globals->http_output_conv_mimetypes) {
		_crx_mb_free_regex(mbstring_globals->http_output_conv_mimetypes);
	}
#ifdef HAVE_MBREGEX
	crx_mb_regex_globals_free(mbstring_globals->mb_regex_globals);
#endif
}
/* }}} */

#ifdef CREX_INTRIN_AVX2_FUNC_PTR
static void init_check_utf8(void);
#endif

/* {{{ CRX_MINIT_FUNCTION(mbstring) */
CRX_MINIT_FUNCTION(mbstring)
{
#if defined(COMPILE_DL_MBSTRING) && defined(ZTS)
CREX_TSRMLS_CACHE_UPDATE();
#endif

	REGISTER_INI_ENTRIES();

	/* We assume that we're the only user of the hook. */
	CREX_ASSERT(crx_internal_encoding_changed == NULL);
	crx_internal_encoding_changed = mbstring_internal_encoding_changed_hook;
	mbstring_internal_encoding_changed_hook();

	/* This is a global handler. Should not be set in a per-request handler. */
	sapi_register_treat_data(mbstr_treat_data);

	/* Post handlers are stored in the thread-local context. */
	if (MBSTRG(encoding_translation)) {
		sapi_register_post_entries(mbstr_post_entries);
	}

#ifdef HAVE_MBREGEX
	CRX_MINIT(mb_regex) (INIT_FUNC_ARGS_PASSTHRU);
#endif

	register_mbstring_symbols(module_number);

	if (FAILURE == crex_multibyte_set_functions(&crx_mb_crex_multibyte_functions)) {
		return FAILURE;
	}

	crx_rfc1867_set_multibyte_callbacks(
		crx_mb_encoding_translation,
		crx_mb_gpc_get_detect_order,
		crx_mb_gpc_set_input_encoding,
		crx_mb_rfc1867_getword,
		crx_mb_rfc1867_getword_conf,
		crx_mb_rfc1867_basename);

#ifdef CREX_INTRIN_AVX2_FUNC_PTR
	init_check_utf8();
	init_convert_utf16();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION(mbstring) */
CRX_MSHUTDOWN_FUNCTION(mbstring)
{
	UNREGISTER_INI_ENTRIES();

	crex_multibyte_restore_functions();

#ifdef HAVE_MBREGEX
	CRX_MSHUTDOWN(mb_regex) (INIT_FUNC_ARGS_PASSTHRU);
#endif

	crx_internal_encoding_changed = NULL;

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_RINIT_FUNCTION(mbstring) */
CRX_RINIT_FUNCTION(mbstring)
{
	MBSTRG(current_internal_encoding) = MBSTRG(internal_encoding);
	MBSTRG(current_http_output_encoding) = MBSTRG(http_output_encoding);
	MBSTRG(current_filter_illegal_mode) = MBSTRG(filter_illegal_mode);
	MBSTRG(current_filter_illegal_substchar) = MBSTRG(filter_illegal_substchar);

	MBSTRG(illegalchars) = 0;

	crx_mb_populate_current_detect_order_list();

#ifdef HAVE_MBREGEX
	CRX_RINIT(mb_regex) (INIT_FUNC_ARGS_PASSTHRU);
#endif
	crex_multibyte_set_internal_encoding((const crex_encoding *)MBSTRG(internal_encoding));

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_RSHUTDOWN_FUNCTION(mbstring) */
CRX_RSHUTDOWN_FUNCTION(mbstring)
{
	if (MBSTRG(current_detect_order_list) != NULL) {
		efree(CREX_VOIDP(MBSTRG(current_detect_order_list)));
		MBSTRG(current_detect_order_list) = NULL;
		MBSTRG(current_detect_order_list_size) = 0;
	}

	/* clear http input identification. */
	MBSTRG(http_input_identify) = NULL;
	MBSTRG(http_input_identify_post) = NULL;
	MBSTRG(http_input_identify_get) = NULL;
	MBSTRG(http_input_identify_cookie) = NULL;
	MBSTRG(http_input_identify_string) = NULL;

	if (MBSTRG(last_used_encoding_name)) {
		crex_string_release(MBSTRG(last_used_encoding_name));
		MBSTRG(last_used_encoding_name) = NULL;
	}

	MBSTRG(internal_encoding_set) = 0;
	MBSTRG(http_output_set) = 0;
	MBSTRG(http_input_set) = 0;

	MBSTRG(outconv_enabled) = false;
	MBSTRG(outconv_state) = 0;

	if (MBSTRG(all_encodings_list)) {
		GC_DELREF(MBSTRG(all_encodings_list));
		crex_array_destroy(MBSTRG(all_encodings_list));
		MBSTRG(all_encodings_list) = NULL;
	}

#ifdef HAVE_MBREGEX
	CRX_RSHUTDOWN(mb_regex) (INIT_FUNC_ARGS_PASSTHRU);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION(mbstring) */
CRX_MINFO_FUNCTION(mbstring)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "Multibyte Support", "enabled");
	crx_info_print_table_row(2, "Multibyte string engine", "libmbfl");
	crx_info_print_table_row(2, "HTTP input encoding translation", MBSTRG(encoding_translation) ? "enabled": "disabled");
	{
		char tmp[256];
		snprintf(tmp, sizeof(tmp), "%d.%d.%d", MBFL_VERSION_MAJOR, MBFL_VERSION_MINOR, MBFL_VERSION_TEENY);
		crx_info_print_table_row(2, "libmbfl version", tmp);
	}
	crx_info_print_table_end();

	crx_info_print_table_start();
	crx_info_print_table_header(1, "mbstring extension makes use of \"streamable kanji code filter and converter\", which is distributed under the GNU Lesser General Public License version 2.1.");
	crx_info_print_table_end();

#ifdef HAVE_MBREGEX
	CRX_MINFO(mb_regex)(CREX_MODULE_INFO_FUNC_ARGS_PASSTHRU);
#endif

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ Sets the current language or Returns the current language as a string */
CRX_FUNCTION(mb_language)
{
	crex_string *name = NULL;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(name)
	CREX_PARSE_PARAMETERS_END();

	if (name == NULL) {
		RETVAL_STRING((char *)mbfl_no_language2name(MBSTRG(language)));
	} else {
		crex_string *ini_name = ZSTR_INIT_LITERAL("mbstring.language", 0);
		if (FAILURE == crex_alter_ini_entry(ini_name, name, CRX_INI_USER, CRX_INI_STAGE_RUNTIME)) {
			crex_argument_value_error(1, "must be a valid language, \"%s\" given", ZSTR_VAL(name));
			crex_string_release_ex(ini_name, 0);
			RETURN_THROWS();
		}
		// TODO Make return void
		RETVAL_TRUE;
		crex_string_release_ex(ini_name, 0);
	}
}
/* }}} */

/* {{{ Sets the current internal encoding or Returns the current internal encoding as a string */
CRX_FUNCTION(mb_internal_encoding)
{
	char *name = NULL;
	size_t name_len;
	const mbfl_encoding *encoding;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_STRING_OR_NULL(name, name_len)
	CREX_PARSE_PARAMETERS_END();

	if (name == NULL) {
		CREX_ASSERT(MBSTRG(current_internal_encoding));
		RETURN_STRING(MBSTRG(current_internal_encoding)->name);
	} else {
		encoding = mbfl_name2encoding(name);
		if (!encoding) {
			crex_argument_value_error(1, "must be a valid encoding, \"%s\" given", name);
			RETURN_THROWS();
		} else {
			MBSTRG(current_internal_encoding) = encoding;
			MBSTRG(internal_encoding_set) = 1;
			/* TODO Return old encoding */
			RETURN_TRUE;
		}
	}
}
/* }}} */

/* {{{ Returns the input encoding */
CRX_FUNCTION(mb_http_input)
{
	char *type = NULL;
	size_t type_len = 0, n;
	const mbfl_encoding **entry;
	const mbfl_encoding *encoding;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_STRING_OR_NULL(type, type_len)
	CREX_PARSE_PARAMETERS_END();

	if (type == NULL) {
		encoding = MBSTRG(http_input_identify);
	} else {
		switch (*type) {
		case 'G':
		case 'g':
			encoding = MBSTRG(http_input_identify_get);
			break;
		case 'P':
		case 'p':
			encoding = MBSTRG(http_input_identify_post);
			break;
		case 'C':
		case 'c':
			encoding = MBSTRG(http_input_identify_cookie);
			break;
		case 'S':
		case 's':
			encoding = MBSTRG(http_input_identify_string);
			break;
		case 'I':
		case 'i':
			entry = MBSTRG(http_input_list);
			n = MBSTRG(http_input_list_size);
			array_init(return_value);
			for (size_t i = 0; i < n; i++, entry++) {
				add_next_index_string(return_value, (*entry)->name);
			}
			return;
		case 'L':
		case 'l':
			entry = MBSTRG(http_input_list);
			n = MBSTRG(http_input_list_size);
			if (n == 0) {
				RETURN_FALSE;
			}

			smart_str result = {0};
			for (size_t i = 0; i < n; i++, entry++) {
				if (i > 0) {
					smart_str_appendc(&result, ',');
				}
				smart_str_appends(&result, (*entry)->name);
			}
			RETURN_STR(smart_str_extract(&result));
		default:
			crex_argument_value_error(1,
				"must be one of \"G\", \"P\", \"C\", \"S\", \"I\", or \"L\"");
			RETURN_THROWS();
		}
	}

	if (encoding) {
		RETURN_STRING(encoding->name);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Sets the current output_encoding or returns the current output_encoding as a string */
CRX_FUNCTION(mb_http_output)
{
	char *name = NULL;
	size_t name_len;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_STRING_OR_NULL(name, name_len)
	CREX_PARSE_PARAMETERS_END();

	if (name == NULL) {
		CREX_ASSERT(MBSTRG(current_http_output_encoding));
		RETURN_STRING(MBSTRG(current_http_output_encoding)->name);
	} else {
		const mbfl_encoding *encoding = crx_mb_get_encoding_or_pass(name);
		if (!encoding) {
			crex_argument_value_error(1, "must be a valid encoding, \"%s\" given", name);
			RETURN_THROWS();
		} else {
			MBSTRG(http_output_set) = 1;
			MBSTRG(current_http_output_encoding) = encoding;
			/* TODO Return previous encoding? */
			RETURN_TRUE;
		}
	}
}
/* }}} */

/* {{{ Sets the current detect_order or Return the current detect_order as an array */
CRX_FUNCTION(mb_detect_order)
{
	crex_string *order_str = NULL;
	HashTable *order_ht = NULL;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_STR_OR_NULL(order_ht, order_str)
	CREX_PARSE_PARAMETERS_END();

	if (!order_str && !order_ht) {
		size_t n = MBSTRG(current_detect_order_list_size);
		const mbfl_encoding **entry = MBSTRG(current_detect_order_list);
		array_init(return_value);
		for (size_t i = 0; i < n; i++) {
			add_next_index_string(return_value, (*entry)->name);
			entry++;
		}
	} else {
		const mbfl_encoding **list;
		size_t size;
		if (order_ht) {
			if (FAILURE == crx_mb_parse_encoding_array(order_ht, &list, &size, 1)) {
				RETURN_THROWS();
			}
		} else {
			if (FAILURE == crx_mb_parse_encoding_list(ZSTR_VAL(order_str), ZSTR_LEN(order_str), &list, &size, /* persistent */ 0, /* arg_num */ 1, /* allow_pass_encoding */ 0)) {
				RETURN_THROWS();
			}
		}

		if (size == 0) {
			efree(CREX_VOIDP(list));
			crex_argument_value_error(1, "must specify at least one encoding");
			RETURN_THROWS();
		}

		if (MBSTRG(current_detect_order_list)) {
			efree(CREX_VOIDP(MBSTRG(current_detect_order_list)));
		}
		MBSTRG(current_detect_order_list) = list;
		MBSTRG(current_detect_order_list_size) = size;
		RETURN_TRUE;
	}
}
/* }}} */

static inline int crx_mb_check_code_point(crex_long cp)
{
	if (cp < 0 || cp >= 0x110000) {
		/* Out of Unicode range */
		return 0;
	}

	if (cp >= 0xd800 && cp <= 0xdfff) {
		/* Surrogate code-point. These are never valid on their own and we only allow a single
		 * substitute character. */
		return 0;
	}

	/* As we do not know the target encoding of the conversion operation that is going to
	 * use the substitution character, we cannot check whether the codepoint is actually mapped
	 * in the given encoding at this point. Thus we have to accept everything. */
	return 1;
}

/* {{{ Sets the current substitute_character or returns the current substitute_character */
CRX_FUNCTION(mb_substitute_character)
{
	crex_string *substitute_character = NULL;
	crex_long substitute_codepoint;
	bool substitute_is_null = 1;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_LONG_OR_NULL(substitute_character, substitute_codepoint, substitute_is_null)
	CREX_PARSE_PARAMETERS_END();

	if (substitute_is_null) {
		if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE) {
			RETURN_STRING("none");
		}
		if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_LONG) {
			RETURN_STRING("long");
		}
		if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_ENTITY) {
			RETURN_STRING("entity");
		}
		RETURN_LONG(MBSTRG(current_filter_illegal_substchar));
	}

	if (substitute_character != NULL) {
		if (crex_string_equals_literal_ci(substitute_character, "none")) {
			MBSTRG(current_filter_illegal_mode) = MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE;
			RETURN_TRUE;
		}
		if (crex_string_equals_literal_ci(substitute_character, "long")) {
			MBSTRG(current_filter_illegal_mode) = MBFL_OUTPUTFILTER_ILLEGAL_MODE_LONG;
			RETURN_TRUE;
		}
		if (crex_string_equals_literal_ci(substitute_character, "entity")) {
			MBSTRG(current_filter_illegal_mode) = MBFL_OUTPUTFILTER_ILLEGAL_MODE_ENTITY;
			RETURN_TRUE;
		}
		/* Invalid string value */
		crex_argument_value_error(1, "must be \"none\", \"long\", \"entity\" or a valid codepoint");
		RETURN_THROWS();
	}
	/* Integer codepoint passed */
	if (!crx_mb_check_code_point(substitute_codepoint)) {
		crex_argument_value_error(1, "is not a valid codepoint");
		RETURN_THROWS();
	}

	MBSTRG(current_filter_illegal_mode) = MBFL_OUTPUTFILTER_ILLEGAL_MODE_CHAR;
	MBSTRG(current_filter_illegal_substchar) = substitute_codepoint;
	RETURN_TRUE;
}
/* }}} */

/* {{{ Return the preferred MIME name (charset) as a string */
CRX_FUNCTION(mb_preferred_mime_name)
{
	char *name = NULL;
	size_t name_len;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STRING(name, name_len)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = mbfl_name2encoding(name);
	if (enc == NULL) {
		crex_argument_value_error(1, "must be a valid encoding, \"%s\" given", name);
		RETURN_THROWS();
	}

	const char *preferred_name = mbfl_encoding_preferred_mime_name(enc);
	if (preferred_name == NULL || *preferred_name == '\0') {
		crx_error_docref(NULL, E_WARNING, "No MIME preferred name corresponding to \"%s\"", name);
		RETVAL_FALSE;
	} else {
		RETVAL_STRING((char *)preferred_name);
	}
}
/* }}} */

/* {{{ Parses GET/POST/COOKIE data and sets global variables */
CRX_FUNCTION(mb_parse_str)
{
	zval *track_vars_array = NULL;
	char *encstr;
	size_t encstr_len;
	crx_mb_encoding_handler_info_t info;
	const mbfl_encoding *detected;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STRING(encstr, encstr_len)
		C_PARAM_ZVAL(track_vars_array)
	CREX_PARSE_PARAMETERS_END();

	track_vars_array = crex_try_array_init(track_vars_array);
	if (!track_vars_array) {
		RETURN_THROWS();
	}

	encstr = estrndup(encstr, encstr_len);

	info.data_type              = PARSE_STRING;
	info.separator              = PG(arg_separator).input;
	info.report_errors          = true;
	info.to_encoding            = MBSTRG(current_internal_encoding);
	info.from_encodings         = MBSTRG(http_input_list);
	info.num_from_encodings     = MBSTRG(http_input_list_size);

	detected = _crx_mb_encoding_handler_ex(&info, track_vars_array, encstr);

	MBSTRG(http_input_identify) = detected;

	RETVAL_BOOL(detected);

	if (encstr != NULL) efree(encstr);
}
/* }}} */

CRX_FUNCTION(mb_output_handler)
{
	crex_string *str;
	crex_long arg_status;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STR(str)
		C_PARAM_LONG(arg_status)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *encoding = MBSTRG(current_http_output_encoding);
	if (encoding == &mbfl_encoding_pass) {
		RETURN_STR_COPY(str);
	}

	if (arg_status & CRX_OUTPUT_HANDLER_START) {
		bool free_mimetype = false;
		char *mimetype = NULL;

		/* Analyze mime type */
		if (SG(sapi_headers).mimetype && _crx_mb_match_regex(MBSTRG(http_output_conv_mimetypes), SG(sapi_headers).mimetype, strlen(SG(sapi_headers).mimetype))) {
			char *s;
			if ((s = strchr(SG(sapi_headers).mimetype, ';')) == NULL) {
				mimetype = estrdup(SG(sapi_headers).mimetype);
			} else {
				mimetype = estrndup(SG(sapi_headers).mimetype, s - SG(sapi_headers).mimetype);
			}
			free_mimetype = true;
		} else if (SG(sapi_headers).send_default_content_type) {
			mimetype = SG(default_mimetype) ? SG(default_mimetype) : SAPI_DEFAULT_MIMETYPE;
		}

		/* If content-type is not yet set, set it and enable conversion */
		if (SG(sapi_headers).send_default_content_type || free_mimetype) {
			const char *charset = encoding->mime_name;
			if (charset) {
				char *p;
				size_t len = spprintf(&p, 0, "Content-Type: %s; charset=%s",  mimetype, charset);
				if (sapi_add_header(p, len, 0) != FAILURE) {
					SG(sapi_headers).send_default_content_type = 0;
				}
			}

			MBSTRG(outconv_enabled) = true;
		}

		if (free_mimetype) {
			efree(mimetype);
		}
	}

	if (!MBSTRG(outconv_enabled)) {
		RETURN_STR_COPY(str);
	}

	mb_convert_buf buf;
	mb_convert_buf_init(&buf, ZSTR_LEN(str), MBSTRG(current_filter_illegal_substchar), MBSTRG(current_filter_illegal_mode));

	uint32_t wchar_buf[128];
	unsigned char *in = (unsigned char*)ZSTR_VAL(str);
	size_t in_len = ZSTR_LEN(str);
	bool last_feed = ((arg_status & CRX_OUTPUT_HANDLER_END) != 0);

	while (in_len) {
		size_t out_len = MBSTRG(current_internal_encoding)->to_wchar(&in, &in_len, wchar_buf, 128, &MBSTRG(outconv_state));
		CREX_ASSERT(out_len <= 128);
		encoding->from_wchar(wchar_buf, out_len, &buf, !in_len && last_feed);
	}

	MBSTRG(illegalchars) += buf.errors;
	RETVAL_STR(mb_convert_buf_result_raw(&buf));

	if (last_feed) {
		MBSTRG(outconv_enabled) = false;
		MBSTRG(outconv_state) = 0;
	}
}

CRX_FUNCTION(mb_str_split)
{
	crex_string *str, *encoding = NULL;
	crex_long split_len = 1;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(split_len)
		C_PARAM_STR_OR_NULL(encoding)
	CREX_PARSE_PARAMETERS_END();

	if (split_len <= 0) {
		crex_argument_value_error(2, "must be greater than 0");
		RETURN_THROWS();
	} else if (split_len > UINT_MAX / 4) {
		crex_argument_value_error(2, "is too large");
		RETURN_THROWS();
	}

	const mbfl_encoding *enc = crx_mb_get_encoding(encoding, 3);
	if (!enc) {
		RETURN_THROWS();
	}

	if (ZSTR_LEN(str) == 0) {
		RETURN_EMPTY_ARRAY();
	}

	unsigned char *p = (unsigned char*)ZSTR_VAL(str), *e = p + ZSTR_LEN(str);

	unsigned int char_len = enc->flag & (MBFL_ENCTYPE_SBCS | MBFL_ENCTYPE_WCS2 | MBFL_ENCTYPE_WCS4);
	if (char_len) {
		unsigned int chunk_len = char_len * split_len;
		unsigned int chunks = ((ZSTR_LEN(str) / chunk_len) + split_len - 1) / split_len; /* round up */
		array_init_size(return_value, chunks);
		while (p < e) {
			add_next_index_stringl(return_value, (const char*)p, MIN(chunk_len, e - p));
			p += chunk_len;
		}
	} else if (enc->mblen_table) {
		unsigned char const *mbtab = enc->mblen_table;

		/* Assume that we have 1-byte characters */
		array_init_size(return_value, (ZSTR_LEN(str) + split_len - 1) / split_len);

		while (p < e) {
			unsigned char *chunk = p; /* start of chunk */

			for (int char_count = 0; char_count < split_len && p < e; char_count++) {
				p += mbtab[*p];
			}
			if (p > e) {
				p = e; /* ensure chunk is in bounds */
			}
			add_next_index_stringl(return_value, (const char*)chunk, p - chunk);
		}
	} else {
		/* Assume that we have 1-byte characters */
		array_init_size(return_value, (ZSTR_LEN(str) + split_len - 1) / split_len);

		uint32_t wchar_buf[128];
		size_t in_len = ZSTR_LEN(str);
		unsigned int state = 0, char_count = 0;

		mb_convert_buf buf;

		while (in_len) {
			size_t out_len = enc->to_wchar(&p, &in_len, wchar_buf, 128, &state);
			CREX_ASSERT(out_len <= 128);
			size_t i = 0;

			/* Is there some output remaining from the previous iteration? */
			if (char_count) {
				if (out_len >= split_len - char_count) {
					/* Finish off an incomplete chunk from previous iteration
					 * ('buf' was already initialized; we don't need to do it again) */
					enc->from_wchar(wchar_buf, split_len - char_count, &buf, true);
					i += split_len - char_count;
					char_count = 0;
					add_next_index_str(return_value, mb_convert_buf_result(&buf, enc));
				} else {
					/* Output from this iteration is not enough to finish the next chunk;
					 * output what we can, and leave 'buf' to be used again on next iteration */
					enc->from_wchar(wchar_buf, out_len, &buf, !in_len);
					char_count += out_len;
					continue;
				}
			}

			while (i < out_len) {
				/* Prepare for the next chunk */
				mb_convert_buf_init(&buf, split_len, MBSTRG(current_filter_illegal_substchar), MBSTRG(current_filter_illegal_mode));

				if (out_len - i >= split_len) {
					enc->from_wchar(wchar_buf + i, split_len, &buf, true);
					i += split_len;
					add_next_index_str(return_value, mb_convert_buf_result(&buf, enc));
				} else {
					/* The remaining codepoints in wchar_buf aren't enough to finish a chunk;
					 * leave them for the next iteration */
					enc->from_wchar(wchar_buf + i, out_len - i, &buf, !in_len);
					char_count = out_len - i;
					break;
				}
			}
		}

		if (char_count) {
			/* The main loop above has finished processing the input string, but
			 * has left a partial chunk in 'buf' */
			add_next_index_str(return_value, mb_convert_buf_result(&buf, enc));
		}
	}
}

#ifdef __SSE2__
/* Thanks to StackOverflow user 'Paul R' (https://stackoverflow.com/users/253056/paul-r)
 * From: https://stackoverflow.com/questions/36998538/fastest-way-to-horizontally-sum-sse-unsigned-byte-vector
 * Takes a 128-bit XMM register, treats each byte as an 8-bit integer, and sums up all
 * 16 of them, returning the sum in an ordinary scalar register */
static inline uint32_t _mm_sum_epu8(const __m128i v)
{
	/* We don't have any dedicated instruction to sum up 8-bit values from a 128-bit register
	 * _mm_sad_epu8 takes the differences between corresponding bytes of two different XMM registers,
	 * sums up those differences, and stores them as two 16-byte integers in the top and bottom
	 * halves of the destination XMM register
	 * By using a zeroed-out XMM register as one operand, we ensure the "differences" which are
	 * summed up will actually just be the 8-bit values from `v` */
	__m128i vsum = _mm_sad_epu8(v, _mm_setzero_si128());
	/* If _mm_sad_epu8 had stored the sum of those bytes as a single integer, we would just have
	 * to extract it here; but it stored the sum as two different 16-bit values
	 * _mm_cvtsi128_si32 extracts one of those values into a scalar register
	 * _mm_extract_epi16 extracts the other one into another scalar register; then we just add them */
	return _mm_cvtsi128_si32(vsum) + _mm_extract_epi16(vsum, 4);
}
#endif

/* This assumes that `string` is valid UTF-8
 * In UTF-8, the only bytes which do not start a new codepoint are 0x80-0xBF (continuation bytes)
 * Interpreted as signed integers, those are all byte values less than -64
 * A fast way to get the length of a UTF-8 string is to start with its byte length,
 * then subtract off the number of continuation bytes */
static size_t mb_fast_strlen_utf8(unsigned char *p, size_t len)
{
	unsigned char *e = p + len;

#ifdef __SSE2__
	if (len >= sizeof(__m128i)) {
		e -= sizeof(__m128i);

		const __m128i threshold = _mm_set1_epi8(-64);
		const __m128i delta = _mm_set1_epi8(1);
		__m128i counter = _mm_setzero_si128(); /* Vector of 16 continuation-byte counters */

		int reset_counter = 255;
		do {
			__m128i operand = _mm_loadu_si128((__m128i*)p); /* Load 16 bytes */
			__m128i lt = _mm_cmplt_epi8(operand, threshold); /* Find all which are continuation bytes */
			counter = _mm_add_epi8(counter, _mm_and_si128(lt, delta)); /* Update the 16 counters */

			/* The counters can only go up to 255, so every 255 iterations, fold them into `len`
			 * and reset them to zero */
			if (--reset_counter == 0) {
				len -= _mm_sum_epu8(counter);
				counter = _mm_setzero_si128();
				reset_counter = 255;
			}

			p += sizeof(__m128i);
		} while (p <= e);

		e += sizeof(__m128i);
		len -= _mm_sum_epu8(counter); /* Fold in any remaining non-zero values in the 16 counters */
	}
#endif

	/* Check for continuation bytes in the 0-15 remaining bytes at the end of the string */
	while (p < e) {
		signed char c = *p++;
		if (c < -64) {
			len--;
		}
	}

	return len;
}

static size_t mb_get_strlen(crex_string *string, const mbfl_encoding *encoding)
{
	unsigned int char_len = encoding->flag & (MBFL_ENCTYPE_SBCS | MBFL_ENCTYPE_WCS2 | MBFL_ENCTYPE_WCS4);
	if (char_len) {
		return ZSTR_LEN(string) / char_len;
	} else if (crx_mb_is_no_encoding_utf8(encoding->no_encoding) && GC_FLAGS(string) & IS_STR_VALID_UTF8) {
		return mb_fast_strlen_utf8((unsigned char*)ZSTR_VAL(string), ZSTR_LEN(string));
	}

	uint32_t wchar_buf[128];
	unsigned char *in = (unsigned char*)ZSTR_VAL(string);
	size_t in_len = ZSTR_LEN(string);
	unsigned int state = 0;
	size_t len = 0;

	while (in_len) {
		len += encoding->to_wchar(&in, &in_len, wchar_buf, 128, &state);
	}

	return len;
}

/* {{{ Get character numbers of a string */
CRX_FUNCTION(mb_strlen)
{
	crex_string *string, *enc_name = NULL;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(string)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(enc_name)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = crx_mb_get_encoding(enc_name, 2);
	if (!enc) {
		RETURN_THROWS();
	}

	RETVAL_LONG(mb_get_strlen(string, enc));
}
/* }}} */

/* See mbfl_no_encoding definition for list of UTF-8 encodings */
static inline bool crx_mb_is_no_encoding_utf8(enum mbfl_no_encoding no_enc)
{
	return (no_enc >= mbfl_no_encoding_utf8 && no_enc <= mbfl_no_encoding_utf8_sb);
}

static unsigned char* offset_to_pointer_utf8(unsigned char *str, unsigned char *end, ssize_t offset) {
	if (offset < 0) {
		unsigned char *pos = end;
		while (offset < 0) {
			if (pos <= str) {
				return NULL;
			}

			unsigned char c = *--pos;
			if (c < 0x80 || (c & 0xC0) != 0x80) {
				offset++;
			}
		}
		return pos;
	} else {
		const unsigned char *u8_tbl = mbfl_encoding_utf8.mblen_table;
		unsigned char *pos = str;
		while (offset-- > 0) {
			if (pos >= end) {
				return NULL;
			}
			pos += u8_tbl[*pos];
		}
		return pos;
	}
}

static size_t pointer_to_offset_utf8(unsigned char *start, unsigned char *pos) {
	return mb_fast_strlen_utf8(start, pos - start);
}

static size_t mb_find_strpos(crex_string *haystack, crex_string *needle, const mbfl_encoding *enc, ssize_t offset, bool reverse)
{
	size_t result;
	crex_string *haystack_u8 = NULL, *needle_u8 = NULL;
	unsigned char *offset_pointer;

	if (!crx_mb_is_no_encoding_utf8(enc->no_encoding)) {
		unsigned int num_errors = 0;
		haystack_u8 = mb_fast_convert((unsigned char*)ZSTR_VAL(haystack), ZSTR_LEN(haystack), enc, &mbfl_encoding_utf8, 0, MBFL_OUTPUTFILTER_ILLEGAL_MODE_BADUTF8, &num_errors);
		needle_u8 = mb_fast_convert((unsigned char*)ZSTR_VAL(needle), ZSTR_LEN(needle), enc, &mbfl_encoding_utf8, 0, MBFL_OUTPUTFILTER_ILLEGAL_MODE_BADUTF8, &num_errors);
	} else {
		haystack_u8 = haystack;
		needle_u8 = needle;
	}

	offset_pointer = offset_to_pointer_utf8((unsigned char*)ZSTR_VAL(haystack_u8), (unsigned char*)ZSTR_VAL(haystack_u8) + ZSTR_LEN(haystack_u8), offset);
	if (!offset_pointer) {
		result = MBFL_ERROR_OFFSET;
		goto out;
	}

	result = MBFL_ERROR_NOT_FOUND;
	if (ZSTR_LEN(haystack_u8) < ZSTR_LEN(needle_u8)) {
		goto out;
	}

	const char *found_pos;
	if (!reverse) {
		found_pos = crex_memnstr((const char*)offset_pointer, ZSTR_VAL(needle_u8), ZSTR_LEN(needle_u8), ZSTR_VAL(haystack_u8) + ZSTR_LEN(haystack_u8));
	} else if (offset >= 0) {
		found_pos = crex_memnrstr((const char*)offset_pointer, ZSTR_VAL(needle_u8), ZSTR_LEN(needle_u8), ZSTR_VAL(haystack_u8) + ZSTR_LEN(haystack_u8));
	} else {
		size_t needle_len = pointer_to_offset_utf8((unsigned char*)ZSTR_VAL(needle), (unsigned char*)ZSTR_VAL(needle) + ZSTR_LEN(needle));
		offset_pointer = offset_to_pointer_utf8(offset_pointer, (unsigned char*)ZSTR_VAL(haystack_u8) + ZSTR_LEN(haystack_u8), needle_len);
		if (!offset_pointer) {
			offset_pointer = (unsigned char*)ZSTR_VAL(haystack_u8) + ZSTR_LEN(haystack_u8);
		}

		found_pos = crex_memnrstr(ZSTR_VAL(haystack_u8), ZSTR_VAL(needle_u8), ZSTR_LEN(needle_u8), (const char*)offset_pointer);
	}

	if (found_pos) {
		result = pointer_to_offset_utf8((unsigned char*)ZSTR_VAL(haystack_u8), (unsigned char*)found_pos);
	}

out:
	if (haystack_u8 != haystack) {
		crex_string_free(haystack_u8);
	}
	if (needle_u8 != needle) {
		crex_string_free(needle_u8);
	}
	return result;
}

static void handle_strpos_error(size_t error) {
	switch (error) {
	case MBFL_ERROR_NOT_FOUND:
		break;
	case MBFL_ERROR_ENCODING:
		crx_error_docref(NULL, E_WARNING, "Conversion error");
		break;
	case MBFL_ERROR_OFFSET:
		crex_argument_value_error(3, "must be contained in argument #1 ($haystack)");
		break;
	default:
		crex_value_error("mb_strpos(): Unknown error");
		break;
	}
}

CRX_FUNCTION(mb_strpos)
{
	crex_long offset = 0;
	crex_string *needle, *haystack;
	crex_string *enc_name = NULL;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(offset)
		C_PARAM_STR_OR_NULL(enc_name)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = crx_mb_get_encoding(enc_name, 4);
	if (!enc) {
		RETURN_THROWS();
	}

	size_t n = mb_find_strpos(haystack, needle, enc, offset, false);
	if (!mbfl_is_error(n)) {
		RETVAL_LONG(n);
	} else {
		handle_strpos_error(n);
		RETVAL_FALSE;
	}
}

/* {{{ Find position of last occurrence of a string within another */
CRX_FUNCTION(mb_strrpos)
{
	crex_long offset = 0;
	crex_string *needle, *haystack;
	crex_string *enc_name = NULL;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(offset)
		C_PARAM_STR_OR_NULL(enc_name)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = crx_mb_get_encoding(enc_name, 4);
	if (!enc) {
		RETURN_THROWS();
	}

	size_t n = mb_find_strpos(haystack, needle, enc, offset, true);
	if (!mbfl_is_error(n)) {
		RETVAL_LONG(n);
	} else {
		handle_strpos_error(n);
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ Finds position of first occurrence of a string within another, case insensitive */
CRX_FUNCTION(mb_stripos)
{
	crex_long offset = 0;
	crex_string *haystack, *needle;
	crex_string *from_encoding = NULL;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(offset)
		C_PARAM_STR_OR_NULL(from_encoding)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = crx_mb_get_encoding(from_encoding, 4);
	if (!enc) {
		RETURN_THROWS();
	}

	size_t n = crx_mb_stripos(false, haystack, needle, offset, enc);

	if (!mbfl_is_error(n)) {
		RETVAL_LONG(n);
	} else {
		handle_strpos_error(n);
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ Finds position of last occurrence of a string within another, case insensitive */
CRX_FUNCTION(mb_strripos)
{
	crex_long offset = 0;
	crex_string *haystack, *needle;
	crex_string *from_encoding = NULL;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(offset)
		C_PARAM_STR_OR_NULL(from_encoding)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = crx_mb_get_encoding(from_encoding, 4);
	if (!enc) {
		RETURN_THROWS();
	}

	size_t n = crx_mb_stripos(true, haystack, needle, offset, enc);

	if (!mbfl_is_error(n)) {
		RETVAL_LONG(n);
	} else {
		handle_strpos_error(n);
		RETVAL_FALSE;
	}
}
/* }}} */

static crex_string* mb_get_substr_slow(unsigned char *in, size_t in_len, size_t from, size_t len, const mbfl_encoding *enc)
{
	uint32_t wchar_buf[128];
	unsigned int state = 0;

	mb_convert_buf buf;
	mb_convert_buf_init(&buf, MIN(len, in_len - from), MBSTRG(current_filter_illegal_substchar), MBSTRG(current_filter_illegal_mode));

	while (in_len && len) {
		size_t out_len = enc->to_wchar(&in, &in_len, wchar_buf, 128, &state);
		CREX_ASSERT(out_len <= 128);

		if (from >= out_len) {
			from -= out_len;
		} else {
			enc->from_wchar(wchar_buf + from, MIN(out_len - from, len), &buf, !in_len || out_len >= len);
			from = 0;
			len -= MIN(out_len, len);
		}
	}

	return mb_convert_buf_result(&buf, enc);
}

static crex_string* mb_get_substr(crex_string *input, size_t from, size_t len, const mbfl_encoding *enc)
{
	unsigned char *in = (unsigned char*)ZSTR_VAL(input);
	size_t in_len = ZSTR_LEN(input);

	if (from >= in_len) {
		/* No supported text encoding decodes to more than one codepoint per byte
		 * So if the number of codepoints to skip >= number of input bytes,
		 * then definitely the output should be empty */
		return crex_empty_string;
	}

	/* Does each codepoint have a fixed byte width? */
	unsigned int flag = enc->flag & (MBFL_ENCTYPE_SBCS | MBFL_ENCTYPE_WCS2 | MBFL_ENCTYPE_WCS4);
	if (flag) {
		/* The value of the flag is 2 if each codepoint takes 2 bytes, or 4 if 4 bytes */
		from *= flag;
		len *= flag;
		if (from >= in_len) {
			return crex_empty_string;
		}
		in += from;
		in_len -= from;
		if (len > in_len) {
			len = in_len;
		}
		return crex_string_init_fast((const char*)in, len);
	} else if (enc->mblen_table) {
		/* The use of the `mblen_table` means that for encodings like MacJapanese,
		 * we treat each character in its native charset as "1 character", even if it
		 * maps to a sequence of several codepoints */
		const unsigned char *mbtab = enc->mblen_table;
		unsigned char *limit = in + in_len;
		while (from && in < limit) {
			in += mbtab[*in];
			from--;
		}
		if (in >= limit) {
			return crex_empty_string;
		} else if (len == MBFL_SUBSTR_UNTIL_END) {
			return crex_string_init_fast((const char*)in, limit - in);
		}
		unsigned char *end = in;
		while (len && end < limit) {
			end += mbtab[*end];
			len--;
		}
		if (end > limit) {
			end = limit;
		}
		return crex_string_init_fast((const char*)in, end - in);
	}

	return mb_get_substr_slow(in, in_len, from, len, enc);
}

#define MB_STRSTR 1
#define MB_STRRCHR 2
#define MB_STRISTR 3
#define MB_STRRICHR 4

static void crx_mb_strstr_variants(INTERNAL_FUNCTION_PARAMETERS, unsigned int variant)
{
	bool reverse_mode = false, part = false;
	size_t n;
	crex_string *haystack, *needle;
	crex_string *encoding_name = NULL;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(part)
		C_PARAM_STR_OR_NULL(encoding_name)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = crx_mb_get_encoding(encoding_name, 4);
	if (!enc) {
		RETURN_THROWS();
	}

	if (variant == MB_STRRCHR || variant == MB_STRRICHR) {
		reverse_mode = true;
	}

	if (variant == MB_STRISTR || variant == MB_STRRICHR) {
		n = crx_mb_stripos(reverse_mode, haystack, needle, 0, enc);
	} else {
		n = mb_find_strpos(haystack, needle, enc, 0, reverse_mode);
	}

	if (!mbfl_is_error(n)) {
		if (part) {
			RETVAL_STR(mb_get_substr(haystack, 0, n, enc));
		} else {
			RETVAL_STR(mb_get_substr(haystack, n, MBFL_SUBSTR_UNTIL_END, enc));
		}
	} else {
		// FIXME use handle_strpos_error(n)
		RETVAL_FALSE;
	}
}

/* {{{ Finds first occurrence of a string within another */
CRX_FUNCTION(mb_strstr)
{
	crx_mb_strstr_variants(INTERNAL_FUNCTION_PARAM_PASSTHRU, MB_STRSTR);
}
/* }}} */

/* {{{ Finds the last occurrence of a character in a string within another */
CRX_FUNCTION(mb_strrchr)
{
	crx_mb_strstr_variants(INTERNAL_FUNCTION_PARAM_PASSTHRU, MB_STRRCHR);
}
/* }}} */

/* {{{ Finds first occurrence of a string within another, case insensitive */
CRX_FUNCTION(mb_stristr)
{
	crx_mb_strstr_variants(INTERNAL_FUNCTION_PARAM_PASSTHRU, MB_STRISTR);
}
/* }}} */

/* {{{ Finds the last occurrence of a character in a string within another, case insensitive */
CRX_FUNCTION(mb_strrichr)
{
	crx_mb_strstr_variants(INTERNAL_FUNCTION_PARAM_PASSTHRU, MB_STRRICHR);
}
/* }}} */

#undef MB_STRSTR
#undef MB_STRRCHR
#undef MB_STRISTR
#undef MB_STRRICHR

CRX_FUNCTION(mb_substr_count)
{
	crex_string *haystack, *needle, *enc_name = NULL, *haystack_u8 = NULL, *needle_u8 = NULL;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(haystack)
		C_PARAM_STR(needle)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(enc_name)
	CREX_PARSE_PARAMETERS_END();

	if (ZSTR_LEN(needle) == 0) {
		crex_argument_value_error(2, "must not be empty");
		RETURN_THROWS();
	}

	const mbfl_encoding *enc = crx_mb_get_encoding(enc_name, 3);
	if (!enc) {
		RETURN_THROWS();
	}

	if (crx_mb_is_no_encoding_utf8(enc->no_encoding)) {
		/* No need to do any conversion if haystack/needle are already known-valid UTF-8
		 * (If they are not valid, then not passing them through conversion filters could affect output) */
		if (GC_FLAGS(haystack) & IS_STR_VALID_UTF8) {
			haystack_u8 = haystack;
		} else {
			unsigned int num_errors = 0;
			haystack_u8 = mb_fast_convert((unsigned char*)ZSTR_VAL(haystack), ZSTR_LEN(haystack), enc, &mbfl_encoding_utf8, 0, MBFL_OUTPUTFILTER_ILLEGAL_MODE_BADUTF8, &num_errors);
			if (!num_errors && !ZSTR_IS_INTERNED(haystack)) {
				GC_ADD_FLAGS(haystack, IS_STR_VALID_UTF8);
			}
		}

		if (GC_FLAGS(needle) & IS_STR_VALID_UTF8) {
			needle_u8 = needle;
		} else {
			unsigned int num_errors = 0;
			needle_u8 = mb_fast_convert((unsigned char*)ZSTR_VAL(needle), ZSTR_LEN(needle), enc, &mbfl_encoding_utf8, 0, MBFL_OUTPUTFILTER_ILLEGAL_MODE_BADUTF8, &num_errors);
			if (!num_errors && !ZSTR_IS_INTERNED(needle)) {
				GC_ADD_FLAGS(needle, IS_STR_VALID_UTF8);
			}
		}
	} else {
		unsigned int num_errors = 0;
		haystack_u8 = mb_fast_convert((unsigned char*)ZSTR_VAL(haystack), ZSTR_LEN(haystack), enc, &mbfl_encoding_utf8, 0, MBFL_OUTPUTFILTER_ILLEGAL_MODE_BADUTF8, &num_errors);
		needle_u8 = mb_fast_convert((unsigned char*)ZSTR_VAL(needle), ZSTR_LEN(needle), enc, &mbfl_encoding_utf8, 0, MBFL_OUTPUTFILTER_ILLEGAL_MODE_BADUTF8, &num_errors);
		/* A string with >0 bytes may convert to 0 codepoints; for example, the contents
		 * may be only escape sequences */
		if (ZSTR_LEN(needle_u8) == 0) {
			crex_string_free(haystack_u8);
			crex_string_free(needle_u8);
			crex_argument_value_error(2, "must not be empty");
			RETURN_THROWS();
		}
	}

	size_t result = 0;

	if (ZSTR_LEN(haystack_u8) < ZSTR_LEN(needle_u8)) {
		goto out;
	}

	const char *p = ZSTR_VAL(haystack_u8), *e = p + ZSTR_LEN(haystack_u8);
	while (true) {
		p = crex_memnstr(p, ZSTR_VAL(needle_u8), ZSTR_LEN(needle_u8), e);
		if (!p) {
			break;
		}
		p += ZSTR_LEN(needle_u8);
		result++;
	}

out:
	if (haystack_u8 != haystack) {
		crex_string_free(haystack_u8);
	}
	if (needle_u8 != needle) {
		crex_string_free(needle_u8);
	}

	RETVAL_LONG(result);
}

/* {{{ Returns part of a string */
CRX_FUNCTION(mb_substr)
{
	crex_string *str, *encoding = NULL;
	crex_long from, len;
	size_t real_from, real_len;
	bool len_is_null = true;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_STR(str)
		C_PARAM_LONG(from)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(len, len_is_null)
		C_PARAM_STR_OR_NULL(encoding)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = crx_mb_get_encoding(encoding, 4);
	if (!enc) {
		RETURN_THROWS();
	}

	size_t mblen = 0;
	if (from < 0 || (!len_is_null && len < 0)) {
		if (enc->mblen_table) {
			/* Because we use the `mblen_table` when iterating over the string and
			 * extracting the requested part, we also need to use it here for counting
			 * the "length" of the string
			 * Otherwise, we can get wrong results for text encodings like MacJapanese,
			 * where one native 'character' can map to a sequence of several codepoints */
			const unsigned char *mbtab = enc->mblen_table;
			unsigned char *p = (unsigned char*)ZSTR_VAL(str), *e = p + ZSTR_LEN(str);
			while (p < e) {
				p += mbtab[*p];
				mblen++;
			}
		} else {
			mblen = mb_get_strlen(str, enc);
		}
	}

	/* if "from" position is negative, count start position from the end
	 * of the string */
	if (from >= 0) {
		real_from = (size_t) from;
	} else if (-from < mblen) {
		real_from = mblen + from;
	} else {
		real_from = 0;
	}

	/* if "length" position is negative, set it to the length
	 * needed to stop that many chars from the end of the string */
	if (len_is_null) {
		real_len = MBFL_SUBSTR_UNTIL_END;
	} else if (len >= 0) {
		real_len = (size_t) len;
	} else if (real_from < mblen && -len < mblen - real_from) {
		real_len = (mblen - real_from) + len;
	} else {
		real_len = 0;
	}

	RETVAL_STR(mb_get_substr(str, real_from, real_len, enc));
}
/* }}} */

/* {{{ Returns part of a string */
CRX_FUNCTION(mb_strcut)
{
	crex_string *encoding = NULL;
	char *string_val;
	crex_long from, len;
	bool len_is_null = 1;
	mbfl_string string, result, *ret;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_STRING(string_val, string.len)
		C_PARAM_LONG(from)
		C_PARAM_OPTIONAL
		C_PARAM_LONG_OR_NULL(len, len_is_null)
		C_PARAM_STR_OR_NULL(encoding)
	CREX_PARSE_PARAMETERS_END();

	string.val = (unsigned char*)string_val;
	string.encoding = crx_mb_get_encoding(encoding, 4);
	if (!string.encoding) {
		RETURN_THROWS();
	}

	if (len_is_null) {
		len = string.len;
	}

	/* if "from" position is negative, count start position from the end
	 * of the string
	 */
	if (from < 0) {
		from = string.len + from;
		if (from < 0) {
			from = 0;
		}
	}

	/* if "length" position is negative, set it to the length
	 * needed to stop that many chars from the end of the string
	 */
	if (len < 0) {
		len = (string.len - from) + len;
		if (len < 0) {
			len = 0;
		}
	}

	if (from > string.len) {
		RETURN_EMPTY_STRING();
	}

	ret = mbfl_strcut(&string, &result, from, len);
	CREX_ASSERT(ret != NULL);

	// TODO: avoid reallocation ???
	RETVAL_STRINGL((char *)ret->val, ret->len); /* the string is already strdup()'ed */
	efree(ret->val);
}
/* }}} */

/* Some East Asian characters, when printed at a terminal (or the like), require double
 * the usual amount of horizontal space. We call these "fullwidth" characters. */
static size_t character_width(uint32_t c)
{
	if (c < FIRST_DOUBLEWIDTH_CODEPOINT) {
		return 1;
	}

	/* Do a binary search to see if we fall in any of the fullwidth ranges */
	int lo = 0, hi = sizeof(mbfl_eaw_table) / sizeof(mbfl_eaw_table[0]);
	while (lo < hi) {
		int probe = (lo + hi) / 2;
		if (c < mbfl_eaw_table[probe].begin) {
			hi = probe;
		} else if (c > mbfl_eaw_table[probe].end) {
			lo = probe + 1;
		} else {
			return 2;
		}
	}

	return 1;
}

static size_t mb_get_strwidth(crex_string *string, const mbfl_encoding *enc)
{
	size_t width = 0;
	uint32_t wchar_buf[128];
	unsigned char *in = (unsigned char*)ZSTR_VAL(string);
	size_t in_len = ZSTR_LEN(string);
	unsigned int state = 0;

	while (in_len) {
		size_t out_len = enc->to_wchar(&in, &in_len, wchar_buf, 128, &state);
		CREX_ASSERT(out_len <= 128);

		while (out_len) {
			/* NOTE: 'bad input' marker will be counted as 1 unit of width
			 * If text conversion is performed with an ordinary ASCII character as
			 * the 'replacement character', this will give us the correct display width. */
			width += character_width(wchar_buf[--out_len]);
		}
	}

	return width;
}

/* Gets terminal width of a string */
CRX_FUNCTION(mb_strwidth)
{
	crex_string *string, *enc_name = NULL;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(string)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(enc_name)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = crx_mb_get_encoding(enc_name, 2);
	if (!enc) {
		RETURN_THROWS();
	}

	RETVAL_LONG(mb_get_strwidth(string, enc));
}

static crex_string* mb_trim_string(crex_string *input, crex_string *marker, const mbfl_encoding *enc, unsigned int from, int width)
{
	uint32_t wchar_buf[128];
	unsigned char *in = (unsigned char*)ZSTR_VAL(input);
	size_t in_len = ZSTR_LEN(input);
	unsigned int state = 0;
	int remaining_width = width;
	unsigned int to_skip = from;
	size_t out_len = 0;
	bool first_call = true, input_err = false;
	mb_convert_buf buf;

	while (in_len) {
		out_len = enc->to_wchar(&in, &in_len, wchar_buf, 128, &state);
		CREX_ASSERT(out_len <= 128);

		if (out_len <= to_skip) {
			to_skip -= out_len;
		} else {
			for (int i = to_skip; i < out_len; i++) {
				uint32_t w = wchar_buf[i];
				input_err |= (w == MBFL_BAD_INPUT);
				remaining_width -= character_width(w);
				if (remaining_width < 0) {
					/* We need to truncate string and append trim marker */
					width -= mb_get_strwidth(marker, enc);
					/* 'width' is now the amount we want to take from 'input' */
					if (width <= 0) {
						return crex_string_copy(marker);
					}
					mb_convert_buf_init(&buf, width, MBSTRG(current_filter_illegal_substchar), MBSTRG(current_filter_illegal_mode));

					if (first_call) {
						/* We can use the buffer of wchars which we have right now;
						 * no need to convert again */
						goto dont_restart_conversion;
					} else {
						goto restart_conversion;
					}
				}
			}
			to_skip = 0;
		}
		first_call = false;
	}

	/* The input string fits in the requested width; we don't need to append the trim marker
	 * However, if the string contains erroneous byte sequences, those should be converted
	 * to error markers */
	if (!input_err) {
		if (from == 0) {
			/* This just increments the string's refcount; it doesn't really 'copy' it */
			return crex_string_copy(input);
		} else {
			return mb_get_substr(input, from, MBFL_SUBSTR_UNTIL_END, enc);
		}
	} else {
		/* We can't use `mb_get_substr`, because it uses the fastest method possible of
		 * picking out a substring, which may not include converting erroneous byte
		 * sequences to error markers */
		return mb_get_substr_slow((unsigned char*)ZSTR_VAL(input), ZSTR_LEN(input), from, MBFL_SUBSTR_UNTIL_END, enc);
	}

	/* The input string is too wide; we need to build a new string which
	 * includes some portion of the input string, with the trim marker
	 * concatenated onto it */
restart_conversion:
	in = (unsigned char*)ZSTR_VAL(input);
	in_len = ZSTR_LEN(input);
	state = 0;

	while (true) {
		out_len = enc->to_wchar(&in, &in_len, wchar_buf, 128, &state);
		CREX_ASSERT(out_len <= 128);

dont_restart_conversion:
		if (out_len <= from) {
			from -= out_len;
		} else {
			for (int i = from; i < out_len; i++) {
				width -= character_width(wchar_buf[i]);
				if (width < 0) {
					enc->from_wchar(wchar_buf + from, i - from, &buf, true);
					goto append_trim_marker;
				}
			}
			CREX_ASSERT(in_len > 0);
			enc->from_wchar(wchar_buf + from, out_len - from, &buf, false);
			from = 0;
		}
	}

append_trim_marker:
	if (ZSTR_LEN(marker) > 0) {
		MB_CONVERT_BUF_ENSURE((&buf), buf.out, buf.limit, ZSTR_LEN(marker));
		memcpy(buf.out, ZSTR_VAL(marker), ZSTR_LEN(marker));
		buf.out += ZSTR_LEN(marker);
	}

	/* Even if `enc` is UTF-8, don't mark the output string as valid UTF-8, because
	 * we have no guarantee that the trim marker string is valid UTF-8 */
	return mb_convert_buf_result_raw(&buf);
}

/* Trim the string to terminal width; optional, add a 'trim marker' if it was truncated */
CRX_FUNCTION(mb_strimwidth)
{
	crex_string *str, *trimmarker = crex_empty_string, *encoding = NULL;
	crex_long from, width;

	CREX_PARSE_PARAMETERS_START(3, 5)
		C_PARAM_STR(str)
		C_PARAM_LONG(from)
		C_PARAM_LONG(width)
		C_PARAM_OPTIONAL
		C_PARAM_STR(trimmarker)
		C_PARAM_STR_OR_NULL(encoding)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = crx_mb_get_encoding(encoding, 5);
	if (!enc) {
		RETURN_THROWS();
	}

	if (from != 0) {
		size_t str_len = mb_get_strlen(str, enc);
		if (from < 0) {
			from += str_len;
		}
		if (from < 0 || from > str_len) {
			crex_argument_value_error(2, "is out of range");
			RETURN_THROWS();
		}
	}

	if (width < 0) {
		crx_error_docref(NULL, E_DEPRECATED,
			"passing a negative integer to argument #3 ($width) is deprecated");
		width += mb_get_strwidth(str, enc);

		if (from > 0) {
			crex_string *trimmed = mb_get_substr(str, 0, from, enc);
			width -= mb_get_strwidth(trimmed, enc);
			crex_string_free(trimmed);
		}

		if (width < 0) {
			crex_argument_value_error(3, "is out of range");
			RETURN_THROWS();
		}
	}

	RETVAL_STR(mb_trim_string(str, trimmarker, enc, from, width));
}


/* See mbfl_no_encoding definition for list of unsupported encodings */
static inline bool crx_mb_is_unsupported_no_encoding(enum mbfl_no_encoding no_enc)
{
	return ((no_enc >= mbfl_no_encoding_invalid && no_enc <= mbfl_no_encoding_qprint)
			|| (no_enc >= mbfl_no_encoding_utf7 && no_enc <= mbfl_no_encoding_utf7imap)
			|| (no_enc >= mbfl_no_encoding_jis && no_enc <= mbfl_no_encoding_2022jpms)
			|| (no_enc >= mbfl_no_encoding_cp50220 && no_enc <= mbfl_no_encoding_cp50222));
}

MBSTRING_API crex_string* crx_mb_convert_encoding_ex(const char *input, size_t length, const mbfl_encoding *to_encoding, const mbfl_encoding *from_encoding)
{
	unsigned int num_errors = 0;
	crex_string *result = mb_fast_convert((unsigned char*)input, length, from_encoding, to_encoding, MBSTRG(current_filter_illegal_substchar), MBSTRG(current_filter_illegal_mode), &num_errors);
	MBSTRG(illegalchars) += num_errors;
	return result;
}

MBSTRING_API crex_string* crx_mb_convert_encoding(const char *input, size_t length, const mbfl_encoding *to_encoding, const mbfl_encoding **from_encodings, size_t num_from_encodings)
{
	const mbfl_encoding *from_encoding;

	/* pre-conversion encoding */
	CREX_ASSERT(num_from_encodings >= 1);
	if (num_from_encodings == 1) {
		from_encoding = *from_encodings;
	} else {
		/* auto detect */
		from_encoding = mb_guess_encoding((unsigned char*)input, length, from_encodings, num_from_encodings, MBSTRG(strict_detection), true);
		if (!from_encoding) {
			crx_error_docref(NULL, E_WARNING, "Unable to detect character encoding");
			return NULL;
		}
	}

	return crx_mb_convert_encoding_ex(input, length, to_encoding, from_encoding);
}

MBSTRING_API HashTable *crx_mb_convert_encoding_recursive(HashTable *input, const mbfl_encoding *to_encoding, const mbfl_encoding **from_encodings, size_t num_from_encodings)
{
	HashTable *output, *chash;
	crex_long idx;
	crex_string *key;
	zval *entry, entry_tmp;

	if (!input) {
		return NULL;
	}

	if (GC_IS_RECURSIVE(input)) {
		GC_UNPROTECT_RECURSION(input);
		crx_error_docref(NULL, E_WARNING, "Cannot convert recursively referenced values");
		return NULL;
	}
	GC_TRY_PROTECT_RECURSION(input);
	output = crex_new_array(crex_hash_num_elements(input));
	CREX_HASH_FOREACH_KEY_VAL(input, idx, key, entry) {
		/* convert key */
		if (key) {
			crex_string *converted_key = crx_mb_convert_encoding(ZSTR_VAL(key), ZSTR_LEN(key), to_encoding, from_encodings, num_from_encodings);
			if (!converted_key) {
				continue;
			}
			key = converted_key;
		}
		/* convert value */
		CREX_ASSERT(entry);
try_again:
		switch(C_TYPE_P(entry)) {
			case IS_STRING: {
				crex_string *converted_key = crx_mb_convert_encoding(C_STRVAL_P(entry), C_STRLEN_P(entry), to_encoding, from_encodings, num_from_encodings);
				if (!converted_key) {
					if (key) {
						crex_string_release(key);
					}
					continue;
				}
				ZVAL_STR(&entry_tmp, converted_key);
				break;
			}
			case IS_NULL:
			case IS_TRUE:
			case IS_FALSE:
			case IS_LONG:
			case IS_DOUBLE:
				ZVAL_COPY(&entry_tmp, entry);
				break;
			case IS_ARRAY:
				chash = crx_mb_convert_encoding_recursive(
					C_ARRVAL_P(entry), to_encoding, from_encodings, num_from_encodings);
				if (chash) {
					ZVAL_ARR(&entry_tmp, chash);
				} else {
					ZVAL_EMPTY_ARRAY(&entry_tmp);
				}
				break;
			case IS_REFERENCE:
				entry = C_REFVAL_P(entry);
				goto try_again;
			case IS_OBJECT:
			default:
				if (key) {
					crex_string_release(key);
				}
				crx_error_docref(NULL, E_WARNING, "Object is not supported");
				continue;
		}
		if (key) {
			crex_hash_add(output, key, &entry_tmp);
			crex_string_release(key);
		} else {
			crex_hash_index_add(output, idx, &entry_tmp);
		}
	} CREX_HASH_FOREACH_END();
	GC_TRY_UNPROTECT_RECURSION(input);

	return output;
}
/* }}} */

static void remove_non_encodings_from_elist(const mbfl_encoding **elist, size_t *size)
{
	/* mbstring supports some 'text encodings' which aren't really text encodings
	 * at all, but really 'byte encodings', like Base64, QPrint, and so on.
	 * These should never be returned by `mb_detect_encoding`. */
	int shift = 0;
	for (int i = 0; i < *size; i++) {
		const mbfl_encoding *encoding = elist[i];
		if (encoding->no_encoding <= mbfl_no_encoding_charset_min) {
			shift++; /* Remove this encoding from the list */
		} else if (shift) {
			elist[i - shift] = encoding;
		}
	}
	*size -= shift;
}

/* {{{ Returns converted string in desired encoding */
CRX_FUNCTION(mb_convert_encoding)
{
	crex_string *to_encoding_name;
	crex_string *input_str, *from_encodings_str = NULL;
	HashTable *input_ht, *from_encodings_ht = NULL;
	const mbfl_encoding **from_encodings;
	size_t num_from_encodings;
	bool free_from_encodings = false;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_ARRAY_HT_OR_STR(input_ht, input_str)
		C_PARAM_STR(to_encoding_name)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_STR_OR_NULL(from_encodings_ht, from_encodings_str)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *to_encoding = crx_mb_get_encoding(to_encoding_name, 2);
	if (!to_encoding) {
		RETURN_THROWS();
	}

	if (from_encodings_ht) {
		if (crx_mb_parse_encoding_array(from_encodings_ht, &from_encodings, &num_from_encodings, 3) == FAILURE) {
			RETURN_THROWS();
		}
		free_from_encodings = true;
	} else if (from_encodings_str) {
		if (crx_mb_parse_encoding_list(ZSTR_VAL(from_encodings_str), ZSTR_LEN(from_encodings_str),
				&from_encodings, &num_from_encodings,
				/* persistent */ 0, /* arg_num */ 3, /* allow_pass_encoding */ 0) == FAILURE) {
			RETURN_THROWS();
		}
		free_from_encodings = true;
	} else {
		from_encodings = &MBSTRG(current_internal_encoding);
		num_from_encodings = 1;
	}

	if (num_from_encodings > 1) {
		remove_non_encodings_from_elist(from_encodings, &num_from_encodings);
	}

	if (!num_from_encodings) {
		efree(CREX_VOIDP(from_encodings));
		crex_argument_value_error(3, "must specify at least one encoding");
		RETURN_THROWS();
	}

	if (input_str) {
		crex_string *ret = crx_mb_convert_encoding(ZSTR_VAL(input_str), ZSTR_LEN(input_str), to_encoding, from_encodings, num_from_encodings);
		if (ret != NULL) {
			RETVAL_STR(ret);
		} else {
			RETVAL_FALSE;
		}
	} else {
		HashTable *tmp;
		tmp = crx_mb_convert_encoding_recursive(
			input_ht, to_encoding, from_encodings, num_from_encodings);
		RETVAL_ARR(tmp);
	}

	if (free_from_encodings) {
		efree(CREX_VOIDP(from_encodings));
	}
}
/* }}} */

static crex_string *mbstring_convert_case(crx_case_mode case_mode, const char *str, size_t str_len, const mbfl_encoding *enc)
{
	return crx_unicode_convert_case(case_mode, str, str_len, enc, enc, MBSTRG(current_filter_illegal_mode), MBSTRG(current_filter_illegal_substchar));
}

CRX_FUNCTION(mb_convert_case)
{
	crex_string *str, *from_encoding = NULL;
	crex_long case_mode = 0;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(str)
		C_PARAM_LONG(case_mode)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(from_encoding)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = crx_mb_get_encoding(from_encoding, 3);
	if (!enc) {
		RETURN_THROWS();
	}

	if (case_mode < 0 || case_mode >= CRX_UNICODE_CASE_MODE_MAX) {
		crex_argument_value_error(2, "must be one of the MB_CASE_* constants");
		RETURN_THROWS();
	}

	RETURN_STR(mbstring_convert_case(case_mode, ZSTR_VAL(str), ZSTR_LEN(str), enc));
}

CRX_FUNCTION(mb_strtoupper)
{
	crex_string *str, *from_encoding = NULL;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(from_encoding)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = crx_mb_get_encoding(from_encoding, 2);
	if (!enc) {
		RETURN_THROWS();
	}

	RETURN_STR(mbstring_convert_case(CRX_UNICODE_CASE_UPPER, ZSTR_VAL(str), ZSTR_LEN(str), enc));
}

CRX_FUNCTION(mb_strtolower)
{
	crex_string *str, *from_encoding = NULL;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(from_encoding)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = crx_mb_get_encoding(from_encoding, 2);
	if (!enc) {
		RETURN_THROWS();
	}

	RETURN_STR(mbstring_convert_case(CRX_UNICODE_CASE_LOWER, ZSTR_VAL(str), ZSTR_LEN(str), enc));
}

static const mbfl_encoding **duplicate_elist(const mbfl_encoding **elist, size_t size)
{
	const mbfl_encoding **new_elist = safe_emalloc(size, sizeof(mbfl_encoding*), 0);
	memcpy(CREX_VOIDP(new_elist), elist, size * sizeof(mbfl_encoding*));
	return new_elist;
}

static unsigned int estimate_demerits(uint32_t w)
{
	/* Receive wchars decoded from input string using candidate encoding.
	 * Give the candidate many 'demerits' for each 'rare' codepoint found,
	 * a smaller number for each ASCII punctuation character, and 1 for
	 * all other codepoints.
	 *
	 * The 'common' codepoints should cover the vast majority of
	 * codepoints we are likely to see in practice, while only covering
	 * a small minority of the entire Unicode encoding space. Why?
	 * Well, if the test string happens to be valid in an incorrect
	 * candidate encoding, the bogus codepoints which it decodes to will
	 * be more or less random. By treating the majority of codepoints as
	 * 'rare', we ensure that in almost all such cases, the bogus
	 * codepoints will include plenty of 'rares', thus giving the
	 * incorrect candidate encoding lots of demerits. See
	 * common_codepoints.txt for the actual list used.
	 *
	 * So, why give extra demerits for ASCII punctuation characters? It's
	 * because there are some text encodings, like UTF-7, HZ, and ISO-2022,
	 * which deliberately only use bytes in the ASCII range. When
	 * misinterpreted as ASCII/UTF-8, strings in these encodings will
	 * have an unusually high number of ASCII punctuation characters.
	 * So giving extra demerits for such characters will improve
	 * detection accuracy for UTF-7 and similar encodings.
	 *
	 * Finally, why 1 demerit for all other characters? That penalizes
	 * long strings, meaning we will tend to choose a candidate encoding
	 * in which the test string decodes to a smaller number of
	 * codepoints. That prevents single-byte encodings in which almost
	 * every possible input byte decodes to a 'common' codepoint from
	 * being favored too much. */
	if (w > 0xFFFF) {
		return 40;
	} else if (w >= 0x21 && w <= 0x2F) {
		return 6;
	} else if ((rare_codepoint_bitvec[w >> 5] >> (w & 0x1F)) & 1) {
		return 30;
	} else {
		return 1;
	}
	return 0;
}

struct candidate {
	const mbfl_encoding *enc;
	const unsigned char *in;
	size_t in_len;
	uint64_t demerits; /* Wide bit size to prevent overflow */
	unsigned int state;
	float multiplier;
};

static size_t init_candidate_array(struct candidate *array, size_t length, const mbfl_encoding **encodings, const unsigned char **in, size_t *in_len, size_t n, bool strict, bool order_significant)
{
	size_t j = 0;

	for (size_t i = 0; i < length; i++) {
		const mbfl_encoding *enc = encodings[i];

		array[j].enc = enc;
		array[j].state = 0;
		array[j].demerits = 0;

		/* If any candidate encodings have specialized validation functions, use them
		 * to eliminate as many candidates as possible */
		if (enc->check != NULL) {
			for (size_t k = 0; k < n; k++) {
				if (!enc->check((unsigned char*)in[k], in_len[k])) {
					if (strict) {
						goto skip_to_next;
					} else {
						array[j].demerits += 500;
					}
				}
			}
		}

		/* This multiplier can optionally be used to make candidate encodings listed
		 * first more likely to be chosen. It is a weight factor which multiplies
		 * the number of demerits counted for each candidate. */
		array[j].multiplier = order_significant ? 1.0 + ((0.3 * i) / length) : 1.0;
		j++;
skip_to_next: ;
	}

	return j;
}

static void start_string(struct candidate *array, size_t length, const unsigned char *in, size_t in_len)
{
	for (size_t i = 0; i < length; i++) {
		const mbfl_encoding *enc = array[i].enc;

		array[i].in = in;
		array[i].in_len = in_len;

		/* Skip byte order mark for UTF-8, UTF-16BE, or UTF-16LE */
		if (enc == &mbfl_encoding_utf8) {
			if (in_len >= 3 && in[0] == 0xEF && in[1] == 0xBB && in[2] == 0xBF) {
				array[i].in_len -= 3;
				array[i].in += 3;
			}
		} else if (enc == &mbfl_encoding_utf16be) {
			if (in_len >= 2 && in[0] == 0xFE && in[1] == 0xFF) {
				array[i].in_len -= 2;
				array[i].in += 2;
			}
		} else if (enc == &mbfl_encoding_utf16le) {
			if (in_len >= 2 && in[0] == 0xFF && in[1] == 0xFE) {
				array[i].in_len -= 2;
				array[i].in += 2;
			}
		}
	}
}

static size_t count_demerits(struct candidate *array, size_t length, bool strict)
{
	uint32_t wchar_buf[128];
	unsigned int finished = 0; /* For how many candidate encodings have we processed all the input? */

	for (size_t i = 0; i < length; i++) {
		if (array[i].in_len == 0) {
			finished++;
		}
	}

	while ((strict || length > 1) && finished < length) {
		/* Iterate in reverse order to avoid moving candidates that can be eliminated. */
		for (size_t i = length - 1; i != (size_t)-1; i--) {
			/* Do we still have more input to process for this candidate encoding? */
			if (array[i].in_len) {
				const mbfl_encoding *enc = array[i].enc;
				size_t out_len = enc->to_wchar((unsigned char**)&array[i].in, &array[i].in_len, wchar_buf, 128, &array[i].state);
				CREX_ASSERT(out_len <= 128);
				/* Check this batch of decoded codepoints; are there any error markers?
				 * Also sum up the number of demerits */
				while (out_len) {
					uint32_t w = wchar_buf[--out_len];
					if (w == MBFL_BAD_INPUT) {
						if (strict) {
							/* This candidate encoding is not valid, eliminate it from consideration */
							length--;
							if (i < length) {
								/* The eliminated candidate was the last valid one in the list */
								memmove(&array[i], &array[i+1], (length - i) * sizeof(struct candidate));
							}
							goto try_next_encoding;
						} else {
							array[i].demerits += 1000;
						}
					} else {
						array[i].demerits += estimate_demerits(w);
					}
				}
				if (array[i].in_len == 0) {
					finished++;
				}
			}
try_next_encoding:;
		}
	}

	for (size_t i = 0; i < length; i++) {
		array[i].demerits *= array[i].multiplier;
	}

	return length;
}

MBSTRING_API const mbfl_encoding* mb_guess_encoding_for_strings(const unsigned char **strings, size_t *str_lengths, size_t n, const mbfl_encoding **elist, unsigned int elist_size, bool strict, bool order_significant)
{
	if (elist_size == 0) {
		return NULL;
	}
	if (elist_size == 1) {
		if (strict) {
			while (n--) {
				if (!crx_mb_check_encoding((const char*)strings[n], str_lengths[n], *elist)) {
					return NULL;
				}
			}
		}
		return *elist;
	}
	if (n == 1 && *str_lengths == 0) {
		return *elist;
	}

	/* Allocate on stack; when we return, this array is automatically freed */
	struct candidate *array = alloca(elist_size * sizeof(struct candidate));
	elist_size = init_candidate_array(array, elist_size, elist, strings, str_lengths, n, strict, order_significant);

	while (n--) {
		start_string(array, elist_size, strings[n], str_lengths[n]);
		elist_size = count_demerits(array, elist_size, strict);
		if (elist_size == 0) {
			/* All candidates were eliminated */
			return NULL;
		}
	}

	/* See which remaining candidate encoding has the least demerits */
	unsigned int best = 0;
	for (unsigned int i = 1; i < elist_size; i++) {
		if (array[i].demerits < array[best].demerits) {
			best = i;
		}
	}
	return array[best].enc;
}

/* When doing 'strict' detection, any string which is invalid in the candidate encoding
 * is rejected. With non-strict detection, we just continue, but apply demerits for
 * each invalid byte sequence */
static const mbfl_encoding* mb_guess_encoding(unsigned char *in, size_t in_len, const mbfl_encoding **elist, unsigned int elist_size, bool strict, bool order_significant)
{
	return mb_guess_encoding_for_strings((const unsigned char**)&in, &in_len, 1, elist, elist_size, strict, order_significant);
}

/* {{{ Encodings of the given string is returned (as a string) */
CRX_FUNCTION(mb_detect_encoding)
{
	crex_string *str, *encoding_str = NULL;
	HashTable *encoding_ht = NULL;
	bool strict = false;
	const mbfl_encoding *ret, **elist;
	size_t size;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_STR_OR_NULL(encoding_ht, encoding_str)
		C_PARAM_BOOL(strict)
	CREX_PARSE_PARAMETERS_END();

	/* Should we pay attention to the order of the provided candidate encodings and prefer
	 * the earlier ones (if more than one candidate encoding matches)?
	 * If the entire list of supported encodings returned by `mb_list_encodings` is passed
	 * in, then don't treat the order as significant */
	bool order_significant = true;

	/* make encoding list */
	if (encoding_ht) {
		if (encoding_ht == MBSTRG(all_encodings_list)) {
			order_significant = false;
		}
		if (FAILURE == crx_mb_parse_encoding_array(encoding_ht, &elist, &size, 2)) {
			RETURN_THROWS();
		}
	} else if (encoding_str) {
		if (FAILURE == crx_mb_parse_encoding_list(ZSTR_VAL(encoding_str), ZSTR_LEN(encoding_str), &elist, &size, /* persistent */ 0, /* arg_num */ 2, /* allow_pass_encoding */ 0)) {
			RETURN_THROWS();
		}
	} else {
		elist = duplicate_elist(MBSTRG(current_detect_order_list), MBSTRG(current_detect_order_list_size));
		size = MBSTRG(current_detect_order_list_size);
	}

	if (size == 0) {
		efree(CREX_VOIDP(elist));
		crex_argument_value_error(2, "must specify at least one encoding");
		RETURN_THROWS();
	}

	remove_non_encodings_from_elist(elist, &size);
	if (size == 0) {
		efree(CREX_VOIDP(elist));
		RETURN_FALSE;
	}

	if (CREX_NUM_ARGS() < 3) {
		strict = MBSTRG(strict_detection);
	}

	if (size == 1 && *elist == &mbfl_encoding_utf8 && (GC_FLAGS(str) & IS_STR_VALID_UTF8)) {
		ret = &mbfl_encoding_utf8;
	} else {
		ret = mb_guess_encoding((unsigned char*)ZSTR_VAL(str), ZSTR_LEN(str), elist, size, strict, order_significant);
	}

	efree(CREX_VOIDP(elist));

	if (ret == NULL) {
		RETURN_FALSE;
	}

	RETVAL_STRING((char *)ret->name);
}
/* }}} */

/* {{{ Returns an array of all supported entity encodings */
CRX_FUNCTION(mb_list_encodings)
{
	CREX_PARSE_PARAMETERS_NONE();

	if (MBSTRG(all_encodings_list) == NULL) {
		/* Initialize shared array of supported encoding names
		 * This is done so that we can check if `mb_list_encodings()` is being
		 * passed to other mbstring functions using a cheap pointer equality check */
		HashTable *array = emalloc(sizeof(HashTable));
		crex_hash_init(array, 80, NULL, zval_ptr_dtor_str, false);
		for (const mbfl_encoding **encodings = mbfl_get_supported_encodings(); *encodings; encodings++) {
			zval tmp;
			ZVAL_STRING(&tmp, (*encodings)->name);
			crex_hash_next_index_insert(array, &tmp);
		}
		MBSTRG(all_encodings_list) = array;
	}

	GC_ADDREF(MBSTRG(all_encodings_list));
	RETURN_ARR(MBSTRG(all_encodings_list));
}
/* }}} */

/* {{{ Returns an array of the aliases of a given encoding name */
CRX_FUNCTION(mb_encoding_aliases)
{
	const mbfl_encoding *encoding;
	crex_string *encoding_name = NULL;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(encoding_name)
	CREX_PARSE_PARAMETERS_END();

	encoding = crx_mb_get_encoding(encoding_name, 1);
	if (!encoding) {
		RETURN_THROWS();
	}

	array_init(return_value);
	if (encoding->aliases != NULL) {
		for (const char **alias = encoding->aliases; *alias; ++alias) {
			add_next_index_string(return_value, (char *)*alias);
		}
	}
}
/* }}} */

static crex_string* jp_kana_convert(crex_string *input, const mbfl_encoding *encoding, unsigned int mode)
{
	/* Each wchar may potentially expand to 2 when we perform kana conversion...
	 * if we are converting zenkaku kana to hankaku kana
	 * Make the buffer for converted kana big enough that we never need to
	 * perform bounds checks */
	uint32_t wchar_buf[64], converted_buf[64 * 2];
	unsigned int buf_offset = 0;
	unsigned int state = 0;
	unsigned char *in = (unsigned char*)ZSTR_VAL(input);
	size_t in_len = ZSTR_LEN(input);

	mb_convert_buf buf;
	mb_convert_buf_init(&buf, in_len, MBSTRG(current_filter_illegal_substchar), MBSTRG(current_filter_illegal_mode));

	while (in_len) {
		uint32_t *converted = converted_buf;
		/* If one codepoint has been left in wchar_buf[0] to be reprocessed from the
		 * previous iteration, don't overwrite it */
		size_t out_len = encoding->to_wchar(&in, &in_len, wchar_buf + buf_offset, 64 - buf_offset, &state);
		out_len += buf_offset;
		CREX_ASSERT(out_len <= 64);

		if (!out_len) {
			continue;
		}

		for (int i = 0; i < out_len-1; i++) {
			uint32_t second = 0;
			bool consumed = false;
			*converted++ = mb_convert_kana_codepoint(wchar_buf[i], wchar_buf[i+1], &consumed, &second, mode);
			if (second) {
				*converted++ = second;
			}
			if (consumed) {
				i++;
				if (i == out_len-1) {
					/* We consumed two codepoints at the very end of the wchar buffer
					 * So there is nothing remaining to reprocess on the next iteration */
					buf_offset = 0;
					goto emit_converted_kana;
				}
			}
		}

		if (!in_len) {
			/* This is the last iteration, so we need to process the final codepoint now */
			uint32_t second = 0;
			*converted++ = mb_convert_kana_codepoint(wchar_buf[out_len-1], 0, NULL, &second, mode);
			if (second) {
				*converted++ = second;
			}
		} else {
			/* Reprocess the last codepoint on the next iteration */
			wchar_buf[0] = wchar_buf[out_len-1];
			buf_offset = 1;
		}

emit_converted_kana:
		encoding->from_wchar(converted_buf, converted - converted_buf, &buf, !in_len);
	}

	return mb_convert_buf_result(&buf, encoding);
}

char mb_convert_kana_flags[17] = {
	'A', 'R', 'N', 'S', 'K', 'H', 'M', 'C',
	'a', 'r', 'n', 's', 'k', 'h', 'm', 'c',
	'V'
};

/* Conversion between full-width characters and half-width characters (Japanese) */
CRX_FUNCTION(mb_convert_kana)
{
	unsigned int opt;
	char *optstr = NULL;
	size_t optstr_len;
	crex_string *encname = NULL, *str;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_STRING(optstr, optstr_len)
		C_PARAM_STR_OR_NULL(encname)
	CREX_PARSE_PARAMETERS_END();

	if (optstr != NULL) {
		char *p = optstr, *e = p + optstr_len;
		opt = 0;
next_option:
		while (p < e) {
			/* Walk through option string and convert to bit vector
			 * See translit_kana_jisx0201_jisx0208.h for the values used */
			char c = *p++;
			if (c == 'A') {
				opt |= MBFL_HAN2ZEN_ALL | MBFL_HAN2ZEN_ALPHA | MBFL_HAN2ZEN_NUMERIC;
			} else if (c == 'a') {
				opt |= MBFL_ZEN2HAN_ALL | MBFL_ZEN2HAN_ALPHA | MBFL_ZEN2HAN_NUMERIC;
			} else {
				for (int i = 0; i < sizeof(mb_convert_kana_flags) / sizeof(char); i++) {
					if (c == mb_convert_kana_flags[i]) {
						opt |= (1 << i);
						goto next_option;
					}
				}

				crex_argument_value_error(2, "contains invalid flag: '%c'", c);
				RETURN_THROWS();
			}
		}

		/* Check for illegal combinations of options */
		if (((opt & 0xFF00) >> 8) & opt) {
			/* It doesn't make sense to convert the same type of characters from halfwidth to
			 * fullwidth and then back to halfwidth again. Neither does it make sense to convert
			 * FW hiragana to FW katakana and then back again. */
			int badflag = ((opt & 0xFF00) >> 8) & opt, i;
			for (i = 0; (badflag & 1) == 0; badflag >>= 1, i++);
			char flag1 = mb_convert_kana_flags[i], flag2 = mb_convert_kana_flags[i+8];
			if ((flag1 == 'R' || flag1 == 'N') && (opt & MBFL_HAN2ZEN_ALL))
				flag1 = 'A';
			if ((flag2 == 'r' || flag2 == 'n') && (opt & MBFL_ZEN2HAN_ALL))
				flag2 = 'a';
			crex_argument_value_error(2, "must not combine '%c' and '%c' flags", flag1, flag2);
			RETURN_THROWS();
		}

		if ((opt & MBFL_HAN2ZEN_HIRAGANA) && (opt & MBFL_HAN2ZEN_KATAKANA)) {
			/* We can either convert all HW kana to FW hiragana, or to FW katakana, but not both */
			crex_argument_value_error(2, "must not combine 'H' and 'K' flags");
			RETURN_THROWS();
		}

		/* We can either convert all FW kana to HW hiragana, or all FW kana to HW katakana,
		 * or all FW hiragana to FW katakana, or all FW katakana to FW hiragana, but not
		 * more than one of these */
		if (opt & MBFL_ZEN2HAN_HIRAGANA) {
			if (opt & MBFL_ZENKAKU_HIRA2KATA) {
				crex_argument_value_error(2, "must not combine 'h' and 'C' flags");
				RETURN_THROWS();
			} else if (opt & MBFL_ZENKAKU_KATA2HIRA) {
				crex_argument_value_error(2, "must not combine 'h' and 'c' flags");
				RETURN_THROWS();
			}
		} else if (opt & MBFL_ZEN2HAN_KATAKANA) {
			if (opt & MBFL_ZENKAKU_HIRA2KATA) {
				crex_argument_value_error(2, "must not combine 'k' and 'C' flags");
				RETURN_THROWS();
			} else if (opt & MBFL_ZENKAKU_KATA2HIRA) {
				crex_argument_value_error(2, "must not combine 'k' and 'c' flags");
				RETURN_THROWS();
			}
		}
	} else {
		opt = MBFL_HAN2ZEN_KATAKANA | MBFL_HAN2ZEN_GLUE;
	}

	const mbfl_encoding *enc = crx_mb_get_encoding(encname, 3);
	if (!enc) {
		RETURN_THROWS();
	}

	RETVAL_STR(jp_kana_convert(str, enc, opt));
}

static unsigned int mb_recursive_count_strings(zval *var)
{
	unsigned int count = 0;
	ZVAL_DEREF(var);

	if (C_TYPE_P(var) == IS_STRING) {
		count++;
	} else if (C_TYPE_P(var) == IS_ARRAY || C_TYPE_P(var) == IS_OBJECT) {
		if (C_REFCOUNTED_P(var)) {
			if (C_IS_RECURSIVE_P(var)) {
				return count;
			}
			C_PROTECT_RECURSION_P(var);
		}

		HashTable *ht = HASH_OF(var);
		if (ht != NULL) {
			zval *entry;
			CREX_HASH_FOREACH_VAL_IND(ht, entry) {
				count += mb_recursive_count_strings(entry);
			} CREX_HASH_FOREACH_END();
		}

		if (C_REFCOUNTED_P(var)) {
			C_UNPROTECT_RECURSION_P(var);
		}
	}

	return count;
}

static bool mb_recursive_find_strings(zval *var, const unsigned char **val_list, size_t *len_list, unsigned int *count)
{
	ZVAL_DEREF(var);

	if (C_TYPE_P(var) == IS_STRING) {
		val_list[*count] = (const unsigned char*)C_STRVAL_P(var);
		len_list[*count] = C_STRLEN_P(var);
		(*count)++;
	} else if (C_TYPE_P(var) == IS_ARRAY || C_TYPE_P(var) == IS_OBJECT) {
		if (C_REFCOUNTED_P(var)) {
			if (C_IS_RECURSIVE_P(var)) {
				return true;
			}
			C_PROTECT_RECURSION_P(var);
		}

		HashTable *ht = HASH_OF(var);
		if (ht != NULL) {
			zval *entry;
			CREX_HASH_FOREACH_VAL_IND(ht, entry) {
				if (mb_recursive_find_strings(entry, val_list, len_list, count)) {
					if (C_REFCOUNTED_P(var)) {
						C_UNPROTECT_RECURSION_P(var);
						return true;
					}
				}
			} CREX_HASH_FOREACH_END();
		}

		if (C_REFCOUNTED_P(var)) {
			C_UNPROTECT_RECURSION_P(var);
		}
	}

	return false;
}

static bool mb_recursive_convert_variable(zval *var, const mbfl_encoding* from_encoding, const mbfl_encoding* to_encoding)
{
	zval *entry, *orig_var;

	orig_var = var;
	ZVAL_DEREF(var);

	if (C_TYPE_P(var) == IS_STRING) {
		crex_string *ret = crx_mb_convert_encoding_ex(C_STRVAL_P(var), C_STRLEN_P(var), to_encoding, from_encoding);
		zval_ptr_dtor(orig_var);
		ZVAL_STR(orig_var, ret);
	} else if (C_TYPE_P(var) == IS_ARRAY || C_TYPE_P(var) == IS_OBJECT) {
		if (C_TYPE_P(var) == IS_ARRAY) {
			SEPARATE_ARRAY(var);
		}
		if (C_REFCOUNTED_P(var)) {
			if (C_IS_RECURSIVE_P(var)) {
				return true;
			}
			C_PROTECT_RECURSION_P(var);
		}

		HashTable *ht = HASH_OF(var);
		if (ht != NULL) {
			CREX_HASH_FOREACH_VAL_IND(ht, entry) {
				if (mb_recursive_convert_variable(entry, from_encoding, to_encoding)) {
					if (C_REFCOUNTED_P(var)) {
						C_UNPROTECT_RECURSION_P(var);
					}
					return true;
				}
			} CREX_HASH_FOREACH_END();
		}

		if (C_REFCOUNTED_P(var)) {
			C_UNPROTECT_RECURSION_P(var);
		}
	}

	return false;
}

CRX_FUNCTION(mb_convert_variables)
{
	zval *args;
	crex_string *to_enc_str;
	crex_string *from_enc_str;
	HashTable *from_enc_ht;
	const mbfl_encoding *from_encoding, *to_encoding;
	uint32_t argc;
	size_t elistsz;
	const mbfl_encoding **elist;

	CREX_PARSE_PARAMETERS_START(3, -1)
		C_PARAM_STR(to_enc_str)
		C_PARAM_ARRAY_HT_OR_STR(from_enc_ht, from_enc_str)
		C_PARAM_VARIADIC('+', args, argc)
	CREX_PARSE_PARAMETERS_END();

	/* new encoding */
	to_encoding = crx_mb_get_encoding(to_enc_str, 1);
	if (!to_encoding) {
		RETURN_THROWS();
	}

	from_encoding = MBSTRG(current_internal_encoding);

	bool order_significant = true;

	/* pre-conversion encoding */
	if (from_enc_ht) {
		if (from_enc_ht == MBSTRG(all_encodings_list)) {
			/* If entire list of supported encodings returned by `mb_list_encodings` is passed
			 * in, then don't treat the order of the list as significant */
			order_significant = false;
		}
		if (crx_mb_parse_encoding_array(from_enc_ht, &elist, &elistsz, 2) == FAILURE) {
			RETURN_THROWS();
		}
	} else {
		if (crx_mb_parse_encoding_list(ZSTR_VAL(from_enc_str), ZSTR_LEN(from_enc_str), &elist, &elistsz, /* persistent */ 0, /* arg_num */ 2, /* allow_pass_encoding */ 0) == FAILURE) {
			RETURN_THROWS();
		}
	}

	if (elistsz == 0) {
		efree(CREX_VOIDP(elist));
		crex_argument_value_error(2, "must specify at least one encoding");
		RETURN_THROWS();
	}

	if (elistsz == 1) {
		from_encoding = *elist;
	} else {
		/* auto detect */
		unsigned int num = 0;
		for (size_t n = 0; n < argc; n++) {
			zval *zv = &args[n];
			num += mb_recursive_count_strings(zv);
		}
		const unsigned char **val_list = (const unsigned char**)ecalloc(num, sizeof(char *));
		size_t *len_list = (size_t*)ecalloc(num, sizeof(size_t));
		unsigned int i = 0;
		for (size_t n = 0; n < argc; n++) {
			zval *zv = &args[n];
			if (mb_recursive_find_strings(zv, val_list, len_list, &i)) {
				efree(CREX_VOIDP(elist));
				efree(CREX_VOIDP(val_list));
				efree(len_list);
				crx_error_docref(NULL, E_WARNING, "Cannot handle recursive references");
				RETURN_FALSE;
			}
		}
		from_encoding = mb_guess_encoding_for_strings(val_list, len_list, num, elist, elistsz, MBSTRG(strict_detection), order_significant);
		efree(CREX_VOIDP(val_list));
		efree(len_list);
		if (!from_encoding) {
			crx_error_docref(NULL, E_WARNING, "Unable to detect encoding");
			efree(CREX_VOIDP(elist));
			RETURN_FALSE;
		}

	}

	efree(CREX_VOIDP(elist));

	/* convert */
	for (size_t n = 0; n < argc; n++) {
		zval *zv = &args[n];
		ZVAL_DEREF(zv);
		if (mb_recursive_convert_variable(zv, from_encoding, to_encoding)) {
			crx_error_docref(NULL, E_WARNING, "Cannot handle recursive references");
			RETURN_FALSE;
		}
	}

	RETURN_STRING(from_encoding->name);
}

/* HTML numeric entities */

/* Convert CRX array to data structure required by mbfl_html_numeric_entity */
static uint32_t *make_conversion_map(HashTable *target_hash, int *convmap_size)
{
	zval *hash_entry;

	int n_elems = crex_hash_num_elements(target_hash);
	if (n_elems % 4 != 0) {
		crex_argument_value_error(2, "must have a multiple of 4 elements");
		return NULL;
	}

	uint32_t *convmap = (uint32_t*)safe_emalloc(n_elems, sizeof(uint32_t), 0);
	uint32_t *mapelm = convmap;

	CREX_HASH_FOREACH_VAL(target_hash, hash_entry) {
		*mapelm++ = zval_get_long(hash_entry);
	} CREX_HASH_FOREACH_END();

	*convmap_size = n_elems / 4;
	return convmap;
}

static bool html_numeric_entity_convert(uint32_t w, uint32_t *convmap, int mapsize, uint32_t *retval)
{
	uint32_t *convmap_end = convmap + (mapsize * 4);

	for (uint32_t *mapelm = convmap; mapelm < convmap_end; mapelm += 4) {
		uint32_t lo_code = mapelm[0];
		uint32_t hi_code = mapelm[1];
		uint32_t offset  = mapelm[2];
		uint32_t mask    = mapelm[3];

		if (w >= lo_code && w <= hi_code) {
			/* This wchar falls inside one of the ranges which should be
			 * converted to HTML entities */
			*retval = (w + offset) & mask;
			return true;
		}
	}

	/* None of the ranges matched */
	return false;
}

static crex_string* html_numeric_entity_encode(crex_string *input, const mbfl_encoding *encoding, uint32_t *convmap, int mapsize, bool hex)
{
	/* Each wchar which we get from decoding the input string may become up to
	 * 13 wchars when we convert it to an HTML entity */
	uint32_t wchar_buf[32], converted_buf[32 * 13];
	unsigned char entity[16]; /* For converting wchars to hex/decimal string */

	unsigned int state = 0;
	unsigned char *in = (unsigned char*)ZSTR_VAL(input);
	size_t in_len = ZSTR_LEN(input);

	mb_convert_buf buf;
	mb_convert_buf_init(&buf, in_len, MBSTRG(current_filter_illegal_substchar), MBSTRG(current_filter_illegal_mode));

	while (in_len) {
		/* Convert input string to wchars, up to 32 at a time */
		size_t out_len = encoding->to_wchar(&in, &in_len, wchar_buf, 32, &state);
		CREX_ASSERT(out_len <= 32);
		uint32_t *converted = converted_buf;

		/* Run through wchars and see if any of them fall into the ranges
		 * which we want to convert to HTML entities */
		for (int i = 0; i < out_len; i++) {
			uint32_t w = wchar_buf[i];

			if (html_numeric_entity_convert(w, convmap, mapsize, &w)) {
				*converted++ = '&';
				*converted++ = '#';
				if (hex) {
					*converted++ = 'x';
				}

				/* Convert wchar to decimal/hex string */
				if (w == 0) {
					*converted++ = '0';
				} else {
					unsigned char *p = entity + sizeof(entity);
					if (hex) {
						while (w > 0) {
							*(--p) = "0123456789ABCDEF"[w & 0xF];
							w >>= 4;
						}
					} else {
						while (w > 0) {
							*(--p) = "0123456789"[w % 10];
							w /= 10;
						}
					}
					while (p < entity + sizeof(entity)) {
						*converted++ = *p++;
					}
				}

				*converted++ = ';';
			} else {
				*converted++ = w;
			}
		}

		CREX_ASSERT(converted <= converted_buf + sizeof(converted_buf)/sizeof(*converted_buf));
		encoding->from_wchar(converted_buf, converted - converted_buf, &buf, !in_len);
	}

	return mb_convert_buf_result(&buf, encoding);
}

/* {{{ Converts specified characters to HTML numeric entities */
CRX_FUNCTION(mb_encode_numericentity)
{
	crex_string *encoding = NULL, *str;
	int mapsize;
	HashTable *target_hash;
	bool is_hex = false;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_STR(str)
		C_PARAM_ARRAY_HT(target_hash)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(encoding)
		C_PARAM_BOOL(is_hex)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = crx_mb_get_encoding(encoding, 3);
	if (!enc) {
		RETURN_THROWS();
	}

	uint32_t *convmap = make_conversion_map(target_hash, &mapsize);
	if (convmap == NULL) {
		RETURN_THROWS();
	}

	RETVAL_STR(html_numeric_entity_encode(str, enc, convmap, mapsize, is_hex));
	efree(convmap);
}
/* }}} */

static bool html_numeric_entity_deconvert(uint32_t number, uint32_t *convmap, int mapsize, uint32_t *retval)
{
	uint32_t *convmap_end = convmap + (mapsize * 4);

	for (uint32_t *mapelm = convmap; mapelm < convmap_end; mapelm += 4) {
		uint32_t lo_code = mapelm[0];
		uint32_t hi_code = mapelm[1];
		uint32_t offset  = mapelm[2];
		uint32_t codepoint = number - offset;
		if (codepoint >= lo_code && codepoint <= hi_code) {
			*retval = codepoint;
			return true;
		}
	}

	return false;
}

#define DEC_ENTITY_MINLEN 3  /* For "&#" and 1 decimal digit */
#define HEX_ENTITY_MINLEN 4  /* For "&#x" and 1 hexadecimal digit */
#define DEC_ENTITY_MAXLEN 12 /* For "&#" and 10 decimal digits */
#define HEX_ENTITY_MAXLEN 11 /* For "&#x" and 8 hexadecimal digits */

static crex_string* html_numeric_entity_decode(crex_string *input, const mbfl_encoding *encoding, uint32_t *convmap, int mapsize)
{
	uint32_t wchar_buf[128], converted_buf[128];

	unsigned int state = 0;
	unsigned char *in = (unsigned char*)ZSTR_VAL(input);
	size_t in_len = ZSTR_LEN(input);

	mb_convert_buf buf;
	mb_convert_buf_init(&buf, in_len, MBSTRG(current_filter_illegal_substchar), MBSTRG(current_filter_illegal_mode));

	/* Decode input string from bytes to wchars one 128-wchar buffer at a time, then deconvert HTML entities,
	 * copying the deconverted wchars to a second buffer, then convert back to original encoding from the
	 * 2nd 'converted' buffer.
	 *
	 * Tricky part: an HTML entity might be truncated at the end of the wchar buffer; the remaining
	 * part could come in the next buffer of wchars. To deal with this problem, when we find what looks
	 * like an HTML entity, we scan to see if it terminates before the end of the wchar buffer or not.
	 * If not, we copy it to the beginning of the wchar buffer, and tell the input conversion routine
	 * to store the next batch of wchars after it.
	 *
	 * Optimization: Scan for &, and if we don't find it anywhere, don't even bother copying the
	 * wchars from the 1st buffer to the 2nd one.
	 *
	 * 'converted_buf' is big enough that the deconverted wchars will *always* fit in it, so we don't
	 * have to do bounds checks when writing wchars into it.
	 */

	unsigned int wchar_buf_offset = 0;

	while (in_len) {
		/* Leave space for sentinel at the end of the buffer */
		size_t out_len = encoding->to_wchar(&in, &in_len, wchar_buf + wchar_buf_offset, 127 - wchar_buf_offset, &state);
		out_len += wchar_buf_offset;
		CREX_ASSERT(out_len <= 127);
		wchar_buf[out_len] = '&'; /* Sentinel, to avoid bounds checks */

		uint32_t *p, *converted;

		/* Scan for & first; however, if `wchar_buf_offset` > 0, then definitely & will
		 * be there (in `wchar_buf[0]`), so don't bother in that case */
		if (wchar_buf_offset == 0) {
			p = wchar_buf;
			while (*p != '&')
				p++;
			if (p == wchar_buf + out_len) {
				/* No HTML entities in this buffer */
				encoding->from_wchar(wchar_buf, out_len, &buf, !in_len);
				continue;
			}

			/* Copy over the prefix with no & which we already scanned */
			memcpy(converted_buf, wchar_buf, (p - wchar_buf) * 4);
			converted = converted_buf + (p - wchar_buf);
		} else {
			p = wchar_buf;
			converted = converted_buf;
		}

found_ampersand:
		CREX_ASSERT(*p == '&');
		uint32_t *p2 = p;

		/* These tests can't overrun end of buffer, because we have a '&' sentinel there */
		if (*++p2 == '#') {
			if (*++p2 == 'x') {
				/* Possible hex entity */
				uint32_t w = *++p2;
				while ((w >= '0' && w <= '9') || (w >= 'A' && w <= 'F') || (w >= 'a' && w <= 'f'))
					w = *++p2;
				if ((p2 == wchar_buf + out_len) && in_len && (p2 - p) <= HEX_ENTITY_MAXLEN) {
					/* We hit the end of the buffer while reading digits, and
					 * more wchars are still coming in the next buffer
					 * Reprocess this identity on next iteration */
					memmove(wchar_buf, p, (p2 - p) * 4);
					wchar_buf_offset = p2 - p;
					goto process_converted_wchars;
				} else if ((p2 - p) < HEX_ENTITY_MINLEN || (p2 - p) > HEX_ENTITY_MAXLEN) {
					/* Invalid entity (too long or "&#x" only) */
					memcpy(converted, p, (p2 - p) * 4);
					converted += p2 - p;
				} else {
					/* Valid hexadecimal entity */
					uint32_t value = 0, *p3 = p + 3;
					while (p3 < p2) {
						w = *p3++;
						if (w <= '9') {
							value = (value * 16) + (w - '0');
						} else if (w >= 'a') {
							value = (value * 16) + 10 + (w - 'a');
						} else {
							value = (value * 16) + 10 + (w - 'A');
						}
					}
					if (html_numeric_entity_deconvert(value, convmap, mapsize, converted)) {
						converted++;
						if (*p2 == ';')
							p2++;
					} else {
						memcpy(converted, p, (p2 - p) * 4);
						converted += p2 - p;
					}
				}
			} else {
				/* Possible decimal entity */
				uint32_t w = *p2;
				while (w >= '0' && w <= '9')
					w = *++p2;
				if ((p2 == wchar_buf + out_len) && in_len && (p2 - p) <= DEC_ENTITY_MAXLEN) {
					/* The number of digits was legal (no more than 10 decimal digits)
					 * Reprocess this identity on next iteration of main loop */
					memmove(wchar_buf, p, (p2 - p) * 4);
					wchar_buf_offset = p2 - p;
					goto process_converted_wchars;
				} else if ((p2 - p) < DEC_ENTITY_MINLEN || (p2 - p) > DEC_ENTITY_MAXLEN) {
					/* Invalid entity (too long or "&#" only) */
					memcpy(converted, p, (p2 - p) * 4);
					converted += p2 - p;
				} else {
					/* Valid decimal entity */
					uint32_t value = 0, *p3 = p + 2;
					while (p3 < p2) {
						/* If unsigned integer overflow would occur in the below
						 * multiplication by 10, this entity is no good
						 * 0x19999999 is 1/10th of 0xFFFFFFFF */
						if (value > 0x19999999) {
							memcpy(converted, p, (p2 - p) * 4);
							converted += p2 - p;
							goto decimal_entity_too_big;
						}
						value = (value * 10) + (*p3++ - '0');
					}
					if (html_numeric_entity_deconvert(value, convmap, mapsize, converted)) {
						converted++;
						if (*p2 == ';')
							p2++;
					} else {
						memcpy(converted, p, (p2 - p) * 4);
						converted += p2 - p;
					}
				}
			}
		} else if ((p2 == wchar_buf + out_len) && in_len) {
			/* Corner case: & at end of buffer */
			wchar_buf[0] = '&';
			wchar_buf_offset = 1;
			goto process_converted_wchars;
		} else {
			*converted++ = '&';
		}
decimal_entity_too_big:

		/* Starting to scan a new section of the wchar buffer
		 * 'p2' is pointing at the next wchar which needs to be processed */
		p = p2;
		while (*p2 != '&')
			p2++;

		if (p2 > p) {
			memcpy(converted, p, (p2 - p) * 4);
			converted += p2 - p;
			p = p2;
		}

		if (p < wchar_buf + out_len)
			goto found_ampersand;

		/* We do not have any wchars remaining at the end of this buffer which
		 * we need to reprocess on the next call */
		wchar_buf_offset = 0;
process_converted_wchars:
		CREX_ASSERT(converted <= converted_buf + 128);
		encoding->from_wchar(converted_buf, converted - converted_buf, &buf, !in_len);
	}

	return mb_convert_buf_result(&buf, encoding);
}

/* {{{ Converts HTML numeric entities to character code */
CRX_FUNCTION(mb_decode_numericentity)
{
	crex_string *encoding = NULL, *str;
	int mapsize;
	HashTable *target_hash;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(str)
		C_PARAM_ARRAY_HT(target_hash)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(encoding)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = crx_mb_get_encoding(encoding, 3);
	if (!enc) {
		RETURN_THROWS();
	}

	uint32_t *convmap = make_conversion_map(target_hash, &mapsize);
	if (convmap == NULL) {
		RETURN_THROWS();
	}

	RETVAL_STR(html_numeric_entity_decode(str, enc, convmap, mapsize));
	efree(convmap);
}
/* }}} */

/* {{{ Sends an email message with MIME scheme */
#define CRLF "\r\n"

static int _crx_mbstr_parse_mail_headers(HashTable *ht, const char *str, size_t str_len)
{
	const char *ps;
	size_t icnt;
	int state = 0;
	int crlf_state = -1;
	char *token = NULL;
	size_t token_pos = 0;
	crex_string *fld_name, *fld_val;

	ps = str;
	icnt = str_len;
	fld_name = fld_val = NULL;

	/*
	 *             C o n t e n t - T y p e :   t e x t / h t m l \r\n
	 *             ^ ^^^^^^^^^^^^^^^^^^^^^ ^^^ ^^^^^^^^^^^^^^^^^ ^^^^
	 *      state  0            1           2          3
	 *
	 *             C o n t e n t - T y p e :   t e x t / h t m l \r\n
	 *             ^ ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ^^^^
	 * crlf_state -1                       0                     1 -1
	 *
	 */

	while (icnt > 0) {
		switch (*ps) {
			case ':':
				if (crlf_state == 1) {
					token_pos++;
				}

				if (state == 0 || state == 1) {
					if(token && token_pos > 0) {
						fld_name = crex_string_init(token, token_pos, 0);
					}
					state = 2;
				} else {
					token_pos++;
				}

				crlf_state = 0;
				break;

			case '\n':
				if (crlf_state == -1) {
					goto out;
				}
				crlf_state = -1;
				break;

			case '\r':
				if (crlf_state == 1) {
					token_pos++;
				} else {
					crlf_state = 1;
				}
				break;

			case ' ': case '\t':
				if (crlf_state == -1) {
					if (state == 3) {
						/* continuing from the previous line */
						state = 4;
					} else {
						/* simply skipping this new line */
						state = 5;
					}
				} else {
					if (crlf_state == 1) {
						token_pos++;
					}
					if (state == 1 || state == 3) {
						token_pos++;
					}
				}
				crlf_state = 0;
				break;

			default:
				switch (state) {
					case 0:
						token = (char*)ps;
						token_pos = 0;
						state = 1;
						break;

					case 2:
						if (crlf_state != -1) {
							token = (char*)ps;
							token_pos = 0;

							state = 3;
							break;
						}
						CREX_FALLTHROUGH;

					case 3:
						if (crlf_state == -1) {
							if(token && token_pos > 0) {
								fld_val = crex_string_init(token, token_pos, 0);
							}

							if (fld_name != NULL && fld_val != NULL) {
								zval val;
								crex_str_tolower(ZSTR_VAL(fld_name), ZSTR_LEN(fld_name));
								ZVAL_STR(&val, fld_val);

								crex_hash_update(ht, fld_name, &val);

								crex_string_release_ex(fld_name, 0);
							}

							fld_name = fld_val = NULL;
							token = (char*)ps;
							token_pos = 0;

							state = 1;
						}
						break;

					case 4:
						token_pos++;
						state = 3;
						break;
				}

				if (crlf_state == 1) {
					token_pos++;
				}

				token_pos++;

				crlf_state = 0;
				break;
		}
		ps++, icnt--;
	}
out:
	if (state == 2) {
		token = "";
		token_pos = 0;

		state = 3;
	}
	if (state == 3) {
		if(token && token_pos > 0) {
			fld_val = crex_string_init(token, token_pos, 0);
		}
		if (fld_name != NULL && fld_val != NULL) {
			zval val;
			crex_str_tolower(ZSTR_VAL(fld_name), ZSTR_LEN(fld_name));
			ZVAL_STR(&val, fld_val);
			crex_hash_update(ht, fld_name, &val);

			crex_string_release_ex(fld_name, 0);
		}
	}
	return state;
}

CRX_FUNCTION(mb_send_mail)
{
	char *to;
	size_t to_len;
	char *message;
	size_t message_len;
	crex_string *subject;
	crex_string *extra_cmd = NULL;
	HashTable *headers_ht = NULL;
	crex_string *str_headers = NULL;
	size_t i;
	char *to_r = NULL;
	char *force_extra_parameters = INI_STR("mail.force_extra_parameters");
	bool suppress_content_type = false;
	bool suppress_content_transfer_encoding = false;

	char *p;
	enum mbfl_no_encoding;
	const mbfl_encoding *tran_cs,	/* transfer text charset */
						*head_enc,	/* header transfer encoding */
						*body_enc;	/* body transfer encoding */
	const mbfl_language *lang;
	HashTable ht_headers;
	zval *s;

	/* character-set, transfer-encoding */
	tran_cs = &mbfl_encoding_utf8;
	head_enc = &mbfl_encoding_base64;
	body_enc = &mbfl_encoding_base64;
	lang = mbfl_no2language(MBSTRG(language));
	if (lang != NULL) {
		tran_cs = mbfl_no2encoding(lang->mail_charset);
		head_enc = mbfl_no2encoding(lang->mail_header_encoding);
		body_enc = mbfl_no2encoding(lang->mail_body_encoding);
	}

	CREX_PARSE_PARAMETERS_START(3, 5)
		C_PARAM_PATH(to, to_len)
		C_PARAM_PATH_STR(subject)
		C_PARAM_PATH(message, message_len)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_STR(headers_ht, str_headers)
		C_PARAM_PATH_STR_OR_NULL(extra_cmd)
	CREX_PARSE_PARAMETERS_END();

	if (str_headers) {
		if (strlen(ZSTR_VAL(str_headers)) != ZSTR_LEN(str_headers)) {
			crex_argument_value_error(4, "must not contain any null bytes");
			RETURN_THROWS();
		}
		str_headers = crx_trim(str_headers, NULL, 0, 2);
	} else if (headers_ht) {
		str_headers = crx_mail_build_headers(headers_ht);
		if (EG(exception)) {
			RETURN_THROWS();
		}
	}

	crex_hash_init(&ht_headers, 0, NULL, ZVAL_PTR_DTOR, 0);

	if (str_headers != NULL) {
		_crx_mbstr_parse_mail_headers(&ht_headers, ZSTR_VAL(str_headers), ZSTR_LEN(str_headers));
	}

	if ((s = crex_hash_str_find(&ht_headers, "content-type", sizeof("content-type") - 1))) {
		char *tmp;
		char *param_name;
		char *charset = NULL;

		CREX_ASSERT(C_TYPE_P(s) == IS_STRING);
		p = strchr(C_STRVAL_P(s), ';');

		if (p != NULL) {
			/* skipping the padded spaces */
			do {
				++p;
			} while (*p == ' ' || *p == '\t');

			if (*p != '\0') {
				if ((param_name = crx_strtok_r(p, "= ", &tmp)) != NULL) {
					if (strcasecmp(param_name, "charset") == 0) {
						const mbfl_encoding *_tran_cs = tran_cs;

						charset = crx_strtok_r(NULL, "= \"", &tmp);
						if (charset != NULL) {
							_tran_cs = mbfl_name2encoding(charset);
						}

						if (!_tran_cs) {
							crx_error_docref(NULL, E_WARNING, "Unsupported charset \"%s\" - will be regarded as ascii", charset);
							_tran_cs = &mbfl_encoding_ascii;
						}
						tran_cs = _tran_cs;
					}
				}
			}
		}
		suppress_content_type = true;
	}

	if ((s = crex_hash_str_find(&ht_headers, "content-transfer-encoding", sizeof("content-transfer-encoding") - 1))) {
		const mbfl_encoding *_body_enc;

		CREX_ASSERT(C_TYPE_P(s) == IS_STRING);
		_body_enc = mbfl_name2encoding(C_STRVAL_P(s));
		switch (_body_enc ? _body_enc->no_encoding : mbfl_no_encoding_invalid) {
			case mbfl_no_encoding_base64:
			case mbfl_no_encoding_7bit:
			case mbfl_no_encoding_8bit:
				body_enc = _body_enc;
				break;

			default:
				crx_error_docref(NULL, E_WARNING, "Unsupported transfer encoding \"%s\" - will be regarded as 8bit", C_STRVAL_P(s));
				body_enc =	&mbfl_encoding_8bit;
				break;
		}
		suppress_content_transfer_encoding = true;
	}

	/* To: */
	if (to_len > 0) {
		to_r = estrndup(to, to_len);
		for (; to_len; to_len--) {
			if (!isspace((unsigned char) to_r[to_len - 1])) {
				break;
			}
			to_r[to_len - 1] = '\0';
		}
		for (i = 0; to_r[i]; i++) {
			if (iscntrl((unsigned char) to_r[i])) {
				/* According to RFC 822, section 3.1.1 long headers may be separated into
				 * parts using CRLF followed at least one linear-white-space character ('\t' or ' ').
				 * To prevent these separators from being replaced with a space, we skip over them. */
				if (to_r[i] == '\r' && to_r[i + 1] == '\n' && (to_r[i + 2] == ' ' || to_r[i + 2] == '\t')) {
					i += 2;
					while (to_r[i + 1] == ' ' || to_r[i + 1] == '\t') {
						i++;
					}
					continue;
				}

				to_r[i] = ' ';
			}
		}
	} else {
		to_r = to;
	}

	/* Subject: */
	const mbfl_encoding *enc = MBSTRG(current_internal_encoding);
	if (enc == &mbfl_encoding_pass) {
		enc = mb_guess_encoding((unsigned char*)ZSTR_VAL(subject), ZSTR_LEN(subject), MBSTRG(current_detect_order_list), MBSTRG(current_detect_order_list_size), MBSTRG(strict_detection), false);
	}
	const char *line_sep = PG(mail_mixed_lf_and_crlf) ? "\n" : CRLF;
	size_t line_sep_len = strlen(line_sep);

	subject = mb_mime_header_encode(subject, enc, tran_cs, head_enc == &mbfl_encoding_base64, (char*)line_sep, line_sep_len, strlen("Subject: [CRX-jp nnnnnnnn]") + line_sep_len);

	/* message body */
	const mbfl_encoding *msg_enc = MBSTRG(current_internal_encoding);
	if (msg_enc == &mbfl_encoding_pass) {
		msg_enc = mb_guess_encoding((unsigned char*)message, message_len, MBSTRG(current_detect_order_list), MBSTRG(current_detect_order_list_size), MBSTRG(strict_detection), false);
	}

	unsigned int num_errors = 0;
	crex_string *tmpstr = mb_fast_convert((unsigned char*)message, message_len, msg_enc, tran_cs, '?', MBFL_OUTPUTFILTER_ILLEGAL_MODE_CHAR, &num_errors);
	crex_string *conv = mb_fast_convert((unsigned char*)ZSTR_VAL(tmpstr), ZSTR_LEN(tmpstr), &mbfl_encoding_8bit, body_enc, '?', MBFL_OUTPUTFILTER_ILLEGAL_MODE_CHAR, &num_errors);
	crex_string_free(tmpstr);
	message = ZSTR_VAL(conv);

	/* other headers */
#define CRX_MBSTR_MAIL_MIME_HEADER1 "MIME-Version: 1.0"
#define CRX_MBSTR_MAIL_MIME_HEADER2 "Content-Type: text/plain"
#define CRX_MBSTR_MAIL_MIME_HEADER3 "; charset="
#define CRX_MBSTR_MAIL_MIME_HEADER4 "Content-Transfer-Encoding: "

	smart_str str = {0};
	bool empty = true;

	if (str_headers != NULL) {
		/* Strip trailing CRLF from `str_headers`; we will add CRLF back if necessary */
		size_t len = ZSTR_LEN(str_headers);
		if (ZSTR_VAL(str_headers)[len-1] == '\n') {
			len--;
		}
		if (ZSTR_VAL(str_headers)[len-1] == '\r') {
			len--;
		}
		smart_str_appendl(&str, ZSTR_VAL(str_headers), len);
		empty = false;
		crex_string_release_ex(str_headers, 0);
	}

	if (!crex_hash_str_exists(&ht_headers, "mime-version", sizeof("mime-version") - 1)) {
		if (!empty) {
			smart_str_appendl(&str, line_sep, line_sep_len);
		}
		smart_str_appendl(&str, CRX_MBSTR_MAIL_MIME_HEADER1, sizeof(CRX_MBSTR_MAIL_MIME_HEADER1) - 1);
		empty = false;
	}

	if (!suppress_content_type) {
		if (!empty) {
			smart_str_appendl(&str, line_sep, line_sep_len);
		}
		smart_str_appendl(&str, CRX_MBSTR_MAIL_MIME_HEADER2, sizeof(CRX_MBSTR_MAIL_MIME_HEADER2) - 1);

		p = (char *)mbfl_encoding_preferred_mime_name(tran_cs);
		if (p != NULL) {
			smart_str_appendl(&str, CRX_MBSTR_MAIL_MIME_HEADER3, sizeof(CRX_MBSTR_MAIL_MIME_HEADER3) - 1);
			smart_str_appends(&str, p);
		}
		empty = false;
	}

	if (!suppress_content_transfer_encoding) {
		if (!empty) {
			smart_str_appendl(&str, line_sep, line_sep_len);
		}
		smart_str_appendl(&str, CRX_MBSTR_MAIL_MIME_HEADER4, sizeof(CRX_MBSTR_MAIL_MIME_HEADER4) - 1);
		p = (char *)mbfl_encoding_preferred_mime_name(body_enc);
		if (p == NULL) {
			p = "7bit";
		}
		smart_str_appends(&str, p);
	}

	str_headers = smart_str_extract(&str);

	if (force_extra_parameters) {
		extra_cmd = crx_escape_shell_cmd(force_extra_parameters);
	} else if (extra_cmd) {
		extra_cmd = crx_escape_shell_cmd(ZSTR_VAL(extra_cmd));
	}

	RETVAL_BOOL(crx_mail(to_r, ZSTR_VAL(subject), message, ZSTR_VAL(str_headers), extra_cmd ? ZSTR_VAL(extra_cmd) : NULL));

	if (extra_cmd) {
		crex_string_release_ex(extra_cmd, 0);
	}
	if (to_r != to) {
		efree(to_r);
	}
	crex_string_release(subject);
	crex_string_free(conv);
	crex_hash_destroy(&ht_headers);
	if (str_headers) {
		crex_string_release_ex(str_headers, 0);
	}
}

#undef CRLF
#undef MAIL_ASCIIC_CHECK_MBSTRING
#undef CRX_MBSTR_MAIL_MIME_HEADER1
#undef CRX_MBSTR_MAIL_MIME_HEADER2
#undef CRX_MBSTR_MAIL_MIME_HEADER3
#undef CRX_MBSTR_MAIL_MIME_HEADER4
/* }}} */

/* {{{ Returns the current settings of mbstring */
CRX_FUNCTION(mb_get_info)
{
	crex_string *type = NULL;
	size_t n;
	char *name;
	zval row;
	const mbfl_language *lang = mbfl_no2language(MBSTRG(language));
	const mbfl_encoding **entry;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_STR(type)
	CREX_PARSE_PARAMETERS_END();

	if (!type || crex_string_equals_literal_ci(type, "all")) {
		array_init(return_value);
		if (MBSTRG(current_internal_encoding)) {
			add_assoc_string(return_value, "internal_encoding", (char *)MBSTRG(current_internal_encoding)->name);
		}
		if (MBSTRG(http_input_identify)) {
			add_assoc_string(return_value, "http_input", (char *)MBSTRG(http_input_identify)->name);
		}
		if (MBSTRG(current_http_output_encoding)) {
			add_assoc_string(return_value, "http_output", (char *)MBSTRG(current_http_output_encoding)->name);
		}
		if ((name = (char *)crex_ini_string("mbstring.http_output_conv_mimetypes", sizeof("mbstring.http_output_conv_mimetypes") - 1, 0)) != NULL) {
			add_assoc_string(return_value, "http_output_conv_mimetypes", name);
		}
		if (lang != NULL) {
			if ((name = (char *)mbfl_no_encoding2name(lang->mail_charset)) != NULL) {
				add_assoc_string(return_value, "mail_charset", name);
			}
			if ((name = (char *)mbfl_no_encoding2name(lang->mail_header_encoding)) != NULL) {
				add_assoc_string(return_value, "mail_header_encoding", name);
			}
			if ((name = (char *)mbfl_no_encoding2name(lang->mail_body_encoding)) != NULL) {
				add_assoc_string(return_value, "mail_body_encoding", name);
			}
		}
		add_assoc_long(return_value, "illegal_chars", MBSTRG(illegalchars));
		if (MBSTRG(encoding_translation)) {
			add_assoc_string(return_value, "encoding_translation", "On");
		} else {
			add_assoc_string(return_value, "encoding_translation", "Off");
		}
		if ((name = (char *)mbfl_no_language2name(MBSTRG(language))) != NULL) {
			add_assoc_string(return_value, "language", name);
		}
		n = MBSTRG(current_detect_order_list_size);
		entry = MBSTRG(current_detect_order_list);
		if (n > 0) {
			size_t i;
			array_init(&row);
			for (i = 0; i < n; i++) {
				add_next_index_string(&row, (*entry)->name);
				entry++;
			}
			add_assoc_zval(return_value, "detect_order", &row);
		}
		if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE) {
			add_assoc_string(return_value, "substitute_character", "none");
		} else if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_LONG) {
			add_assoc_string(return_value, "substitute_character", "long");
		} else if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_ENTITY) {
			add_assoc_string(return_value, "substitute_character", "entity");
		} else {
			add_assoc_long(return_value, "substitute_character", MBSTRG(current_filter_illegal_substchar));
		}
		if (MBSTRG(strict_detection)) {
			add_assoc_string(return_value, "strict_detection", "On");
		} else {
			add_assoc_string(return_value, "strict_detection", "Off");
		}
	} else if (crex_string_equals_literal_ci(type, "internal_encoding")) {
		if (MBSTRG(current_internal_encoding)) {
			RETVAL_STRING((char *)MBSTRG(current_internal_encoding)->name);
		}
	} else if (crex_string_equals_literal_ci(type, "http_input")) {
		if (MBSTRG(http_input_identify)) {
			RETVAL_STRING((char *)MBSTRG(http_input_identify)->name);
		}
	} else if (crex_string_equals_literal_ci(type, "http_output")) {
		if (MBSTRG(current_http_output_encoding)) {
			RETVAL_STRING((char *)MBSTRG(current_http_output_encoding)->name);
		}
	} else if (crex_string_equals_literal_ci(type, "http_output_conv_mimetypes")) {
		if ((name = (char *)crex_ini_string("mbstring.http_output_conv_mimetypes", sizeof("mbstring.http_output_conv_mimetypes") - 1, 0)) != NULL) {
			RETVAL_STRING(name);
		}
	} else if (crex_string_equals_literal_ci(type, "mail_charset")) {
		if (lang != NULL && (name = (char *)mbfl_no_encoding2name(lang->mail_charset)) != NULL) {
			RETVAL_STRING(name);
		}
	} else if (crex_string_equals_literal_ci(type, "mail_header_encoding")) {
		if (lang != NULL && (name = (char *)mbfl_no_encoding2name(lang->mail_header_encoding)) != NULL) {
			RETVAL_STRING(name);
		}
	} else if (crex_string_equals_literal_ci(type, "mail_body_encoding")) {
		if (lang != NULL && (name = (char *)mbfl_no_encoding2name(lang->mail_body_encoding)) != NULL) {
			RETVAL_STRING(name);
		}
	} else if (crex_string_equals_literal_ci(type, "illegal_chars")) {
		RETVAL_LONG(MBSTRG(illegalchars));
	} else if (crex_string_equals_literal_ci(type, "encoding_translation")) {
		if (MBSTRG(encoding_translation)) {
			RETVAL_STRING("On");
		} else {
			RETVAL_STRING("Off");
		}
	} else if (crex_string_equals_literal_ci(type, "language")) {
		if ((name = (char *)mbfl_no_language2name(MBSTRG(language))) != NULL) {
			RETVAL_STRING(name);
		}
	} else if (crex_string_equals_literal_ci(type, "detect_order")) {
		n = MBSTRG(current_detect_order_list_size);
		entry = MBSTRG(current_detect_order_list);
		if (n > 0) {
			size_t i;
			array_init(return_value);
			for (i = 0; i < n; i++) {
				add_next_index_string(return_value, (*entry)->name);
				entry++;
			}
		}
	} else if (crex_string_equals_literal_ci(type, "substitute_character")) {
		if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE) {
			RETVAL_STRING("none");
		} else if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_LONG) {
			RETVAL_STRING("long");
		} else if (MBSTRG(current_filter_illegal_mode) == MBFL_OUTPUTFILTER_ILLEGAL_MODE_ENTITY) {
			RETVAL_STRING("entity");
		} else {
			RETVAL_LONG(MBSTRG(current_filter_illegal_substchar));
		}
	} else if (crex_string_equals_literal_ci(type, "strict_detection")) {
		if (MBSTRG(strict_detection)) {
			RETVAL_STRING("On");
		} else {
			RETVAL_STRING("Off");
		}
	} else {
		// TODO Convert to ValueError
		RETURN_FALSE;
	}
}
/* }}} */

MBSTRING_API bool crx_mb_check_encoding(const char *input, size_t length, const mbfl_encoding *encoding)
{
	uint32_t wchar_buf[128];
	unsigned char *in = (unsigned char*)input;
	unsigned int state = 0;

	if (encoding->check != NULL) {
		return encoding->check(in, length);
	}

	/* If the input string is not encoded in the given encoding, there is a significant chance
	 * that this will be seen in the first bytes. Therefore, rather than converting an entire
	 * buffer of 128 codepoints, convert and check just a few codepoints first */
	size_t out_len = encoding->to_wchar(&in, &length, wchar_buf, 8, &state);
	CREX_ASSERT(out_len <= 8);
	for (int i = 0; i < out_len; i++) {
		if (wchar_buf[i] == MBFL_BAD_INPUT) {
			return false;
		}
	}

	while (length) {
		out_len = encoding->to_wchar(&in, &length, wchar_buf, 128, &state);
		CREX_ASSERT(out_len <= 128);
		for (int i = 0; i < out_len; i++) {
			if (wchar_buf[i] == MBFL_BAD_INPUT) {
				return false;
			}
		}
	}

	return true;
}

/* MSVC 32-bit has issues with 64-bit intrinsics.
 * (Bad 7/8-byte UTF-8 strings would be wrongly passed through as 'valid')
 * It seems this is caused by a bug in MS Visual C++
 * Ref: https://stackoverflow.com/questions/37509129/potential-bug-in-visual-studio-c-compiler-or-in-intel-intrinsics-avx2-mm256-s */
#if defined(CRX_WIN32) && !defined(__clang__) && defined(_MSC_VER) && defined(_M_IX86)
# define MBSTRING_BROKEN_X86_MSVC_INTRINSICS
#endif

/* If we are building an AVX2-only binary, don't compile the next function */
#ifndef CREX_INTRIN_AVX2_NATIVE

/* SSE2-based function for validating UTF-8 strings
 * A faster implementation which uses AVX2 instructions follows */
static bool mb_fast_check_utf8_default(crex_string *str)
{
	unsigned char *p = (unsigned char*)ZSTR_VAL(str);
# ifdef __SSE2__
	/* `e` points 1 byte past the last full 16-byte block of string content
	 * Note that we include the terminating null byte which is included in each crex_string
	 * as part of the content to check; this ensures that multi-byte characters which are
	 * truncated abruptly at the end of the string will be detected as invalid */
	unsigned char *e = p + ((ZSTR_LEN(str) + 1) & ~(sizeof(__m128i) - 1));

	/* For checking for illegal bytes 0xF5-FF */
	const __m128i over_f5 = _mm_set1_epi8(-117);
	/* For checking for overlong 3-byte code units and reserved codepoints U+D800-DFFF */
	const __m128i over_9f = _mm_set1_epi8(-97);
	/* For checking for overlong 4-byte code units and invalid codepoints > U+10FFFF */
	const __m128i over_8f = _mm_set1_epi8(-113);
	/* For checking for illegal bytes 0xC0-C1 */
	const __m128i find_c0 = _mm_set1_epi8(-64);
	const __m128i c0_to_c1 = _mm_set1_epi8(-126);
	/* For checking structure of continuation bytes */
	const __m128i find_e0 = _mm_set1_epi8(-32);
	const __m128i find_f0 = _mm_set1_epi8(-16);

	__m128i last_block = _mm_setzero_si128();
	__m128i operand;

	while (p < e) {
		operand = _mm_loadu_si128((__m128i*)p); /* Load 16 bytes */

check_operand:
		/* If all 16 bytes are single-byte characters, then a number of checks can be skipped */
		if (!_mm_movemask_epi8(operand)) {
			/* Even if this block only contains single-byte characters, there may have been a
			 * multi-byte character at the end of the previous block, which was supposed to
			 * have continuation bytes in this block
			 * This bitmask will pick out a 2/3/4-byte character starting from the last byte of
			 * the previous block, a 3/4-byte starting from the 2nd last, or a 4-byte starting
			 * from the 3rd last */
			__m128i bad_mask = _mm_set_epi8(-64, -32, -16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
			__m128i bad = _mm_cmpeq_epi8(_mm_and_si128(last_block, bad_mask), bad_mask);
			if (_mm_movemask_epi8(bad)) {
				return false;
			}

			/* Consume as many full blocks of single-byte characters as we can */
			while (true) {
				p += sizeof(__m128i);
				if (p >= e) {
					goto finish_up_remaining_bytes;
				}
				operand = _mm_loadu_si128((__m128i*)p);
				if (_mm_movemask_epi8(operand)) {
					break;
				}
			}
		}

		/* Check for >= 0xF5, which are illegal byte values in UTF-8
		 * AVX512 has instructions for vectorized unsigned compare, but SSE2 only has signed compare
		 * So we add an offset to shift 0xF5-FF to the far low end of the signed byte range
		 * Then a single signed compare will pick out any bad bytes
		 * `bad` is a vector of 16 good/bad values, where 0x00 means good and 0xFF means bad */
		__m128i bad = _mm_cmplt_epi8(_mm_add_epi8(operand, over_f5), over_f5);

		/* Check for overlong 3-byte code units AND reserved codepoints U+D800-DFFF
		 * 0xE0 followed by a byte < 0xA0 indicates an overlong 3-byte code unit, and
		 * 0xED followed by a byte >= 0xA0 indicates a reserved codepoint
		 * We can check for both problems at once by generating a vector where each byte < 0xA0
		 * is mapped to 0xE0, and each byte >= 0xA0 is mapped to 0xED
		 * Shift the original block right by one byte, and compare the shifted block with the bitmask */
		__m128i operand2 = _mm_or_si128(_mm_slli_si128(operand, 1), _mm_srli_si128(last_block, 15));
		__m128i mask1 = _mm_or_si128(find_e0, _mm_and_si128(_mm_set1_epi8(0xD), _mm_cmpgt_epi8(operand, over_9f)));
		bad = _mm_or_si128(bad, _mm_cmpeq_epi8(operand2, mask1));

		/* Check for overlong 4-byte code units AND invalid codepoints > U+10FFFF
		 * Similar to the previous check; 0xF0 followed by < 0x90 indicates an overlong 4-byte
		 * code unit, and 0xF4 followed by >= 0x90 indicates a codepoint over U+10FFFF
		 * Build the bitmask and compare it with the shifted block */
		__m128i mask2 = _mm_or_si128(find_f0, _mm_and_si128(_mm_set1_epi8(0x4), _mm_cmpgt_epi8(operand, over_8f)));
		bad = _mm_or_si128(bad, _mm_cmpeq_epi8(operand2, mask2));

		/* Check for overlong 2-byte code units
		 * Any 0xC0 or 0xC1 byte can only be the first byte of an overlong 2-byte code unit
		 * Same deal as before; add an offset to shift 0xC0-C1 to the far low end of the signed
		 * byte range, do a signed compare to pick out any bad bytes */
		bad = _mm_or_si128(bad, _mm_cmplt_epi8(_mm_add_epi8(operand, find_c0), c0_to_c1));

		/* Check structure of continuation bytes
		 * A UTF-8 byte should be a continuation byte if, and only if, it is:
		 * 1) 1 byte after the start of a 2-byte, 3-byte, or 4-byte character
		 * 2) 2 bytes after the start of a 3-byte or 4-byte character
		 * 3) 3 bytes after the start of a 4-byte character
		 * We build 3 bitmasks with 0xFF in each such position, and OR them together to
		 * get a single bitmask with 0xFF in each position where a continuation byte should be */
		__m128i cont_mask = _mm_cmpeq_epi8(_mm_and_si128(operand2, find_c0), find_c0);
		__m128i operand3 = _mm_or_si128(_mm_slli_si128(operand, 2), _mm_srli_si128(last_block, 14));
		cont_mask = _mm_or_si128(cont_mask, _mm_cmpeq_epi8(_mm_and_si128(operand3, find_e0), find_e0));
		__m128i operand4 = _mm_or_si128(_mm_slli_si128(operand, 3), _mm_srli_si128(last_block, 13));
		cont_mask = _mm_or_si128(cont_mask, _mm_cmpeq_epi8(_mm_and_si128(operand4, find_f0), find_f0));

		/* Now, use a signed comparison to get another bitmask with 0xFF in each position where
		 * a continuation byte actually is
		 * XOR those two bitmasks together; if everything is good, the result should be zero
		 * However, if a byte which should have been a continuation wasn't, or if a byte which
		 * shouldn't have been a continuation was, we will get 0xFF in that position */
		__m128i continuation = _mm_cmplt_epi8(operand, find_c0);
		bad = _mm_or_si128(bad, _mm_xor_si128(continuation, cont_mask));

		/* Pick out the high bit of each byte in `bad` as a 16-bit value (into a scalar register)
		 * If that value is non-zero, then we found a bad byte somewhere! */
		if (_mm_movemask_epi8(bad)) {
			return false;
		}

		last_block = operand;
		p += sizeof(__m128i);
	}

finish_up_remaining_bytes:
	/* Finish up 1-15 remaining bytes */
	if (p == e) {
		uint8_t remaining_bytes = ZSTR_LEN(str) & (sizeof(__m128i) - 1); /* Not including terminating null */

		/* Crazy hack here for cases where 9 or more bytes are remaining...
		 * We want to use the above vectorized code to check a block of less than 16 bytes,
		 * but there is no good way to read a variable number of bytes into an XMM register
		 * However, we know that these bytes are part of a crex_string, and a crex_string has some
		 * 'header' fields which occupy the memory just before its content
		 * And, those header fields occupy more than 16 bytes...
		 * So if we go back 16 bytes from the end of the crex_string content, and load 16 bytes from there,
		 * we may pick up some 'junk' bytes from the crex_string header fields, but we will get the 1-15
		 * bytes we wanted in the tail end of our XMM register, and this will never cause a segfault.
		 * Then, we do a left shift to get rid of the unwanted bytes
		 * Conveniently, the same left shift also zero-fills the tail end of the XMM register
		 *
		 * The following `switch` looks useless, but it's not
		 * The PSRLDQ instruction used for the 128-bit left shift requires an immediate (literal)
		 * shift distance, so the compiler will choke on _mm_srli_si128(operand, shift_dist)
		 */
		switch (remaining_bytes) {
		case 0: ;
			__m128i bad_mask = _mm_set_epi8(-64, -32, -16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
			__m128i bad = _mm_cmpeq_epi8(_mm_and_si128(last_block, bad_mask), bad_mask);
			return _mm_movemask_epi8(bad) == 0;
		case 1:
		case 2:
			operand = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, *((uint16_t*)p));
			goto check_operand;
		case 3:
		case 4:
			operand = _mm_set_epi32(0, 0, 0, *((uint32_t*)p));
			goto check_operand;
		case 5:
			operand = _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 10)), 10);
			goto check_operand;
		case 6:
			operand = _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 9)), 9);
			goto check_operand;
		case 7:
		case 8:
#ifdef MBSTRING_BROKEN_X86_MSVC_INTRINSICS
			operand = _mm_set_epi32(0, 0, ((int32_t*)p)[1], ((int32_t*)p)[0]);
#else
			operand = _mm_set_epi64x(0, *((uint64_t*)p));
#endif
			goto check_operand;
		case 9:
			operand = _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 6)), 6);
			goto check_operand;
		case 10:
			operand = _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 5)), 5);
			goto check_operand;
		case 11:
			operand = _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 4)), 4);
			goto check_operand;
		case 12:
			operand = _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 3)), 3);
			goto check_operand;
		case 13:
			operand = _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 2)), 2);
			goto check_operand;
		case 14:
			operand = _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 1)), 1);
			goto check_operand;
		case 15:
			/* No trailing bytes are left which need to be checked
			 * We get 15 because we did not include the terminating null when
			 * calculating `remaining_bytes`, so the value wraps around */
			return true;
		}

		CREX_UNREACHABLE();
	}

	return true;
# else
	/* This UTF-8 validation function is derived from PCRE2 */
	size_t length = ZSTR_LEN(str);
	/* Table of the number of extra bytes, indexed by the first byte masked with
	0x3f. The highest number for a valid UTF-8 first byte is in fact 0x3d. */
	static const uint8_t utf8_table[] = {
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
		3,3,3,3,3,3,3,3
	};

	for (; length > 0; p++) {
		uint32_t d;
		unsigned char c = *p;
		length--;

		if (c < 128) {
			/* ASCII character */
			continue;
		}

		if (c < 0xc0) {
			/* Isolated 10xx xxxx byte */
			return false;
		}

		if (c >= 0xf5) {
			return false;
		}

		uint32_t ab = utf8_table[c & 0x3f]; /* Number of additional bytes (1-3) */
		if (length < ab) {
			/* Missing bytes */
			return false;
		}
		length -= ab;

		/* Check top bits in the second byte */
		if (((d = *(++p)) & 0xc0) != 0x80) {
			return false;
		}

		/* For each length, check that the remaining bytes start with the 0x80 bit
		 * set and not the 0x40 bit. Then check for an overlong sequence, and for the
		 * excluded range 0xd800 to 0xdfff. */
		switch (ab) {
		case 1:
			/* 2-byte character. No further bytes to check for 0x80. Check first byte
			 * for xx00 000x (overlong sequence). */
			if ((c & 0x3e) == 0) {
				return false;
			}
			break;

		case 2:
			/* 3-byte character. Check third byte for 0x80. Then check first 2 bytes for
			 * 1110 0000, xx0x xxxx (overlong sequence) or 1110 1101, 1010 xxxx (0xd800-0xdfff) */
			if ((*(++p) & 0xc0) != 0x80 || (c == 0xe0 && (d & 0x20) == 0) || (c == 0xed && d >= 0xa0)) {
				return false;
			}
			break;

		case 3:
			/* 4-byte character. Check 3rd and 4th bytes for 0x80. Then check first 2
			 * bytes for 1111 0000, xx00 xxxx (overlong sequence), then check for a
			 * character greater than 0x0010ffff (f4 8f bf bf) */
			if ((*(++p) & 0xc0) != 0x80 || (*(++p) & 0xc0) != 0x80 || (c == 0xf0 && (d & 0x30) == 0) || (c > 0xf4 || (c == 0xf4 && d > 0x8f))) {
				return false;
			}
			break;

			EMPTY_SWITCH_DEFAULT_CASE();
		}
	}

	return true;
# endif
}

#endif /* #ifndef CREX_INTRIN_AVX2_NATIVE */

#ifdef CREX_INTRIN_AVX2_NATIVE

/* We are building AVX2-only binary */
# include <immintrin.h>
# define mb_fast_check_utf8 mb_fast_check_utf8_avx2

#elif defined(CREX_INTRIN_AVX2_RESOLVER)

/* We are building binary which works with or without AVX2; whether or not to use
 * AVX2-accelerated functions will be determined at runtime */
# include <immintrin.h>
# include "Crex/crex_cpuinfo.h"

# ifdef CREX_INTRIN_AVX2_FUNC_PROTO
/* Dynamic linker will decide whether or not to use AVX2-based functions and
 * resolve symbols accordingly */

CREX_INTRIN_AVX2_FUNC_DECL(bool mb_fast_check_utf8_avx2(crex_string *str));

bool mb_fast_check_utf8(crex_string *str) __attribute__((ifunc("resolve_check_utf8")));

typedef bool (*check_utf8_func_t)(crex_string*);

CREX_NO_SANITIZE_ADDRESS
CREX_ATTRIBUTE_UNUSED
static check_utf8_func_t resolve_check_utf8(void)
{
	if (crex_cpu_supports_avx2()) {
		return mb_fast_check_utf8_avx2;
	}
	return mb_fast_check_utf8_default;
}

# else /* CREX_INTRIN_AVX2_FUNC_PTR */
/* We are compiling for a target where the dynamic linker will not be able to
 * resolve symbols according to whether the host supports AVX2 or not; so instead,
 * we can make calls go through a function pointer and set the function pointer
 * on module load */

#ifdef HAVE_FUNC_ATTRIBUTE_TARGET
static bool mb_fast_check_utf8_avx2(crex_string *str) __attribute__((target("avx2")));
#else
static bool mb_fast_check_utf8_avx2(crex_string *str);
#endif

static bool (*check_utf8_ptr)(crex_string *str) = NULL;

static bool mb_fast_check_utf8(crex_string *str)
{
	return check_utf8_ptr(str);
}

static void init_check_utf8(void)
{
	if (crex_cpu_supports_avx2()) {
		check_utf8_ptr = mb_fast_check_utf8_avx2;
	} else {
		check_utf8_ptr = mb_fast_check_utf8_default;
	}
}
# endif

#else

/* No AVX2 support */
#define mb_fast_check_utf8 mb_fast_check_utf8_default

#endif

#if defined(CREX_INTRIN_AVX2_NATIVE) || defined(CREX_INTRIN_AVX2_RESOLVER)

/* GCC prior to version 8 does not define all intrinsics. See GH-11514.
 * Use a workaround from https://stackoverflow.com/questions/32630458/setting-m256i-to-the-value-of-two-m128i-values */
#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER) && __GNUC__ < 8
# define _mm256_set_m128i(v0, v1)  _mm256_insertf128_si256(_mm256_castsi128_si256(v1), (v0), 1)
#endif

/* Take (256-bit) `hi` and `lo` as a 512-bit value, shift down by some
 * number of bytes, then take the low 256 bits
 * This is used to take some number of trailing bytes from the previous 32-byte
 * block followed by some number of leading bytes from the current 32-byte block
 *
 * _mm256_alignr_epi8 (VPALIGNR) is used to shift out bytes from a 256-bit
 * YMM register while shifting in bytes from another YMM register... but
 * it works separately on respective 128-bit halves of the YMM registers,
 * which is not what we want.
 * To make it work as desired, we first do _mm256_permute2x128_si256
 * (VPERM2I128) to combine the low 128 bits from the previous block and
 * the high 128 bits of the current block in one YMM register.
 * Then VPALIGNR will do what is needed. */
#define _mm256_shift_epi8(hi, lo, shift) _mm256_alignr_epi8(lo, _mm256_permute2x128_si256(hi, lo, 33), 16 - shift)

/* AVX2-based UTF-8 validation function; validates text in 32-byte chunks
 *
 * Some parts of this function are the same as `mb_fast_check_utf8`; code comments
 * are not repeated, so consult `mb_fast_check_utf8` for information on uncommented
 * sections. */
#ifdef CREX_INTRIN_AVX2_FUNC_PROTO
CREX_API bool mb_fast_check_utf8_avx2(crex_string *str)
#else
static bool mb_fast_check_utf8_avx2(crex_string *str)
#endif
{
	unsigned char *p = (unsigned char*)ZSTR_VAL(str);
	unsigned char *e = p + ((ZSTR_LEN(str) + 1) & ~(sizeof(__m256i) - 1));

	/* The algorithm used here for UTF-8 validation is partially adapted from the
	 * paper "Validating UTF-8 In Less Than One Instruction Per Byte", by John Keiser
	 * and Daniel Lemire.
	 * Ref: https://arxiv.org/pdf/2010.03090.pdf
	 *
	 * Most types of invalid UTF-8 text can be detected by examining pairs of
	 * successive bytes. Specifically:
	 *
	 * • Overlong 2-byte code units start with 0xC0 or 0xC1.
	 *   No valid UTF-8 string ever uses these byte values.
	 * • Overlong 3-byte code units start with 0xE0, followed by a byte < 0xA0.
	 * • Overlong 4-byte code units start with 0xF0, followed by a byte < 0x90.
	 * • 5-byte or 6-byte code units, which should never be used, start with
	 *   0xF8-FE.
	 * • A codepoint value higher than U+10FFFF, which is the highest value for
	 *   any Unicode codepoint, would either start with 0xF4, followed by a
	 *   byte >= 0x90, or else would start with 0xF5-F7, followed by any value.
	 * • A codepoint value from U+D800-DFFF, which are reserved and should never
	 *   be used, would start with 0xED, followed by a byte >= 0xA0.
	 * • The byte value 0xFF is also illegal and is never used in valid UTF-8.
	 *
	 * To detect all these problems, for each pair of successive bytes, we do
	 * table lookups using the high nibble of the first byte, the low nibble of
	 * the first byte, and the high nibble of the second byte. Each table lookup
	 * retrieves a bitmask, in which each 1 bit indicates a possible invalid
	 * combination; AND those three bitmasks together, and any 1 bit in the result
	 * will indicate an actual invalid byte combination was found.
	 */

#define BAD_BYTE 0x1
#define OVERLONG_2BYTE 0x2
#define _1BYTE (BAD_BYTE | OVERLONG_2BYTE)
#define OVERLONG_3BYTE 0x4
#define SURROGATE 0x8
#define OVERLONG_4BYTE 0x10
#define INVALID_CP 0x20

	/* Each of these are 16-entry tables, repeated twice; this is required by the
	 * VPSHUFB instruction which we use to perform 32 table lookups in parallel
	 * The first entry is for 0xF, the second is for 0xE, and so on down to 0x0
	 *
	 * So, for example, notice that the 4th entry in the 1st table is OVERLONG_2BYTE;
	 * that means that high nibble 0xC is consistent with the byte pair being part of
	 * an overlong 2-byte code unit */
	const __m256i bad_hi_nibble2 = _mm256_set_epi8(
		BAD_BYTE | OVERLONG_4BYTE | INVALID_CP, OVERLONG_3BYTE | SURROGATE, 0, OVERLONG_2BYTE,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		BAD_BYTE | OVERLONG_4BYTE | INVALID_CP, OVERLONG_3BYTE | SURROGATE, 0, OVERLONG_2BYTE,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0);
	const __m256i bad_lo_nibble2 = _mm256_set_epi8(
		BAD_BYTE, BAD_BYTE, BAD_BYTE | SURROGATE, BAD_BYTE,
		BAD_BYTE, BAD_BYTE, BAD_BYTE, BAD_BYTE,
		BAD_BYTE, BAD_BYTE, BAD_BYTE, INVALID_CP,
		0, 0, OVERLONG_2BYTE, OVERLONG_2BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE,
		BAD_BYTE, BAD_BYTE, BAD_BYTE | SURROGATE, BAD_BYTE,
		BAD_BYTE, BAD_BYTE, BAD_BYTE, BAD_BYTE,
		BAD_BYTE, BAD_BYTE, BAD_BYTE, INVALID_CP,
		0, 0, OVERLONG_2BYTE, OVERLONG_2BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE);
	const __m256i bad_hi_nibble = _mm256_set_epi8(
		_1BYTE | SURROGATE | INVALID_CP, _1BYTE | SURROGATE | INVALID_CP,
		_1BYTE | SURROGATE | INVALID_CP, _1BYTE | SURROGATE | INVALID_CP,
		_1BYTE | SURROGATE | INVALID_CP, _1BYTE | SURROGATE | INVALID_CP,
		_1BYTE | OVERLONG_3BYTE | INVALID_CP, _1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE,
		_1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE, _1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE,
		_1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE, _1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE,
		_1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE, _1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE,
		_1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE, _1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE,
		_1BYTE | SURROGATE | INVALID_CP, _1BYTE | SURROGATE | INVALID_CP,
		_1BYTE | SURROGATE | INVALID_CP, _1BYTE | SURROGATE | INVALID_CP,
		_1BYTE | SURROGATE | INVALID_CP, _1BYTE | SURROGATE | INVALID_CP,
		_1BYTE | OVERLONG_3BYTE | INVALID_CP, _1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE,
		_1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE, _1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE,
		_1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE, _1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE,
		_1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE, _1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE,
		_1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE, _1BYTE | OVERLONG_3BYTE | OVERLONG_4BYTE);

	const __m256i find_continuation = _mm256_set1_epi8(-64);
	const __m256i _b = _mm256_set1_epi8(0xB);
	const __m256i _d = _mm256_set1_epi8(0xD);
	const __m256i _f = _mm256_set1_epi8(0xF);

	__m256i last_hi_nibbles = _mm256_setzero_si256(), last_lo_nibbles = _mm256_setzero_si256();
	__m256i operand;

	while (p < e) {
		operand = _mm256_loadu_si256((__m256i*)p);

check_operand:
		if (!_mm256_movemask_epi8(operand)) {
			/* Entire 32-byte block is ASCII characters; the only thing we need to validate is that
			 * the previous block didn't end with an incomplete multi-byte character
			 * (This will also confirm that the previous block didn't end with a bad byte like 0xFF) */
			__m256i bad_mask = _mm256_set_epi8(0xB, 0xD, 0xE, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127);
			__m256i bad = _mm256_cmpgt_epi8(last_hi_nibbles, bad_mask);
			if (_mm256_movemask_epi8(bad)) {
				return false;
			}

			/* Consume as many full blocks of single-byte characters as we can */
			while (true) {
				p += sizeof(__m256i);
				if (p >= e) {
					goto finish_up_remaining_bytes;
				}
				operand = _mm256_loadu_si256((__m256i*)p);
				if (_mm256_movemask_epi8(operand)) {
					break;
				}
			}
		}

		__m256i hi_nibbles = _mm256_and_si256(_mm256_srli_epi16(operand, 4), _f);
		__m256i lo_nibbles = _mm256_and_si256(operand, _f);

		__m256i lo_nibbles2 = _mm256_shift_epi8(last_lo_nibbles, lo_nibbles, 1);
		__m256i hi_nibbles2 = _mm256_shift_epi8(last_hi_nibbles, hi_nibbles, 1);

		/* Do parallel table lookups in all 3 tables */
		__m256i bad = _mm256_cmpgt_epi8(
			_mm256_and_si256(
				_mm256_and_si256(
					_mm256_shuffle_epi8(bad_lo_nibble2, lo_nibbles2),
					_mm256_shuffle_epi8(bad_hi_nibble2, hi_nibbles2)),
				_mm256_shuffle_epi8(bad_hi_nibble, hi_nibbles)),
			_mm256_setzero_si256());

		__m256i cont_mask = _mm256_cmpgt_epi8(hi_nibbles2, _b);
		__m256i hi_nibbles3 = _mm256_shift_epi8(last_hi_nibbles, hi_nibbles, 2);
		cont_mask = _mm256_or_si256(cont_mask, _mm256_cmpgt_epi8(hi_nibbles3, _d));
		__m256i hi_nibbles4 = _mm256_shift_epi8(last_hi_nibbles, hi_nibbles, 3);
		cont_mask = _mm256_or_si256(cont_mask, _mm256_cmpeq_epi8(hi_nibbles4, _f));

		__m256i continuation = _mm256_cmpgt_epi8(find_continuation, operand);
		bad = _mm256_or_si256(bad, _mm256_xor_si256(continuation, cont_mask));

		if (_mm256_movemask_epi8(bad)) {
			return false;
		}

		last_hi_nibbles = hi_nibbles;
		last_lo_nibbles = lo_nibbles;
		p += sizeof(__m256i);
	}

finish_up_remaining_bytes:
	if (p == e) {
		uint8_t remaining_bytes = ZSTR_LEN(str) & (sizeof(__m256i) - 1); /* Not including terminating null */

		switch (remaining_bytes) {
		case 0: ;
			/* No actual data bytes are remaining */
			__m256i bad_mask = _mm256_set_epi8(0xB, 0xD, 0xE, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127);
			__m256i bad = _mm256_cmpgt_epi8(last_hi_nibbles, bad_mask);
			return _mm256_movemask_epi8(bad) == 0;
		case 1:
		case 2:
			operand = _mm256_set_epi16(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, *((int16_t*)p));
			goto check_operand;
		case 3:
		case 4:
			operand = _mm256_set_epi32(0, 0, 0, 0, 0, 0, 0, *((int32_t*)p));
			goto check_operand;
		case 5:
			operand = _mm256_set_m128i(_mm_setzero_si128(), _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 10)), 10));
			goto check_operand;
		case 6:
			operand = _mm256_set_m128i(_mm_setzero_si128(), _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 9)), 9));
			goto check_operand;
		case 7:
		case 8:
#ifdef MBSTRING_BROKEN_X86_MSVC_INTRINSICS
			operand = _mm256_set_epi32(0, 0, 0, 0, 0, 0, ((int32_t*)p)[1], ((int32_t*)p)[0]);
#else
			operand = _mm256_set_epi64x(0, 0, 0, *((int64_t*)p));
#endif
			goto check_operand;
		case 9:
			operand = _mm256_set_m128i(_mm_setzero_si128(), _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 6)), 6));
			goto check_operand;
		case 10:
			operand = _mm256_set_m128i(_mm_setzero_si128(), _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 5)), 5));
			goto check_operand;
		case 11:
			operand = _mm256_set_m128i(_mm_setzero_si128(), _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 4)), 4));
			goto check_operand;
		case 12:
			operand = _mm256_set_m128i(_mm_setzero_si128(), _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 3)), 3));
			goto check_operand;
		case 13:
			operand = _mm256_set_m128i(_mm_setzero_si128(), _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 2)), 2));
			goto check_operand;
		case 14:
			operand = _mm256_set_m128i(_mm_setzero_si128(), _mm_srli_si128(_mm_loadu_si128((__m128i*)(p - 1)), 1));
			goto check_operand;
		case 15:
		case 16:
			operand = _mm256_set_m128i(_mm_setzero_si128(), _mm_loadu_si128((__m128i*)p));
			goto check_operand;
		case 17:
			operand = _mm256_set_m128i(_mm_srli_si128(_mm_loadu_si128((__m128i*)(p + 2)), 14), _mm_loadu_si128((__m128i*)p));
			goto check_operand;
		case 18:
			operand = _mm256_set_m128i(_mm_srli_si128(_mm_loadu_si128((__m128i*)(p + 3)), 13), _mm_loadu_si128((__m128i*)p));
			goto check_operand;
		case 19:
			operand = _mm256_set_m128i(_mm_srli_si128(_mm_loadu_si128((__m128i*)(p + 4)), 12), _mm_loadu_si128((__m128i*)p));
			goto check_operand;
		case 20:
			operand = _mm256_set_m128i(_mm_srli_si128(_mm_loadu_si128((__m128i*)(p + 5)), 11), _mm_loadu_si128((__m128i*)p));
			goto check_operand;
		case 21:
			operand = _mm256_set_m128i(_mm_srli_si128(_mm_loadu_si128((__m128i*)(p + 6)), 10), _mm_loadu_si128((__m128i*)p));
			goto check_operand;
		case 22:
			operand = _mm256_set_m128i(_mm_srli_si128(_mm_loadu_si128((__m128i*)(p + 7)), 9), _mm_loadu_si128((__m128i*)p));
			goto check_operand;
		case 23:
			operand = _mm256_set_m128i(_mm_srli_si128(_mm_loadu_si128((__m128i*)(p + 8)), 8), _mm_loadu_si128((__m128i*)p));
			goto check_operand;
		case 24:
			operand = _mm256_set_m128i(_mm_srli_si128(_mm_loadu_si128((__m128i*)(p + 9)), 7), _mm_loadu_si128((__m128i*)p));
			goto check_operand;
		case 25:
			operand = _mm256_set_m128i(_mm_srli_si128(_mm_loadu_si128((__m128i*)(p + 10)), 6), _mm_loadu_si128((__m128i*)p));
			goto check_operand;
		case 26:
			operand = _mm256_set_m128i(_mm_srli_si128(_mm_loadu_si128((__m128i*)(p + 11)), 5), _mm_loadu_si128((__m128i*)p));
			goto check_operand;
		case 27:
			operand = _mm256_set_m128i(_mm_srli_si128(_mm_loadu_si128((__m128i*)(p + 12)), 4), _mm_loadu_si128((__m128i*)p));
			goto check_operand;
		case 28:
			operand = _mm256_set_m128i(_mm_srli_si128(_mm_loadu_si128((__m128i*)(p + 13)), 3), _mm_loadu_si128((__m128i*)p));
			goto check_operand;
		case 29:
			operand = _mm256_set_m128i(_mm_srli_si128(_mm_loadu_si128((__m128i*)(p + 14)), 2), _mm_loadu_si128((__m128i*)p));
			goto check_operand;
		case 30:
			operand = _mm256_set_m128i(_mm_srli_si128(_mm_loadu_si128((__m128i*)(p + 15)), 1), _mm_loadu_si128((__m128i*)p));
			goto check_operand;
		case 31:
			return true;
		}

		CREX_UNREACHABLE();
	}

	return true;
}

#endif /* defined(CREX_INTRIN_AVX2_NATIVE) || defined(CREX_INTRIN_AVX2_RESOLVER) */

static bool mb_check_str_encoding(crex_string *str, const mbfl_encoding *encoding)
{
	if (encoding == &mbfl_encoding_utf8) {
		if (GC_FLAGS(str) & IS_STR_VALID_UTF8) {
			return true;
		}
		bool result = mb_fast_check_utf8(str);
		if (result && !ZSTR_IS_INTERNED(str)) {
			GC_ADD_FLAGS(str, IS_STR_VALID_UTF8);
		}
		return result;
	} else {
		return crx_mb_check_encoding(ZSTR_VAL(str), ZSTR_LEN(str), encoding);
	}
}

static int crx_mb_check_encoding_recursive(HashTable *vars, const mbfl_encoding *encoding)
{
	crex_long idx;
	crex_string *key;
	zval *entry;
	int valid = 1;

	(void)(idx); /* Suppress spurious compiler warning that `idx` is not used */

	if (GC_IS_RECURSIVE(vars)) {
		crx_error_docref(NULL, E_WARNING, "Cannot not handle circular references");
		return 0;
	}
	GC_TRY_PROTECT_RECURSION(vars);
	CREX_HASH_FOREACH_KEY_VAL(vars, idx, key, entry) {
		ZVAL_DEREF(entry);
		if (key) {
			if (!mb_check_str_encoding(key, encoding)) {
				valid = 0;
				break;
			}
		}
		switch (C_TYPE_P(entry)) {
			case IS_STRING:
				if (!mb_check_str_encoding(C_STR_P(entry), encoding)) {
					valid = 0;
					break;
				}
				break;
			case IS_ARRAY:
				if (!crx_mb_check_encoding_recursive(C_ARRVAL_P(entry), encoding)) {
					valid = 0;
					break;
				}
				break;
			case IS_LONG:
			case IS_DOUBLE:
			case IS_NULL:
			case IS_TRUE:
			case IS_FALSE:
				break;
			default:
				/* Other types are error. */
				valid = 0;
				break;
		}
	} CREX_HASH_FOREACH_END();
	GC_TRY_UNPROTECT_RECURSION(vars);
	return valid;
}

/* {{{ Check if the string is valid for the specified encoding */
CRX_FUNCTION(mb_check_encoding)
{
	crex_string *input_str = NULL, *enc = NULL;
	HashTable *input_ht = NULL;
	const mbfl_encoding *encoding;

	CREX_PARSE_PARAMETERS_START(0, 2)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_STR_OR_NULL(input_ht, input_str)
		C_PARAM_STR_OR_NULL(enc)
	CREX_PARSE_PARAMETERS_END();

	encoding = crx_mb_get_encoding(enc, 2);
	if (!encoding) {
		RETURN_THROWS();
	}

	if (input_ht) {
		RETURN_BOOL(crx_mb_check_encoding_recursive(input_ht, encoding));
	} else if (input_str) {
		RETURN_BOOL(mb_check_str_encoding(input_str, encoding));
	} else {
		crx_error_docref(NULL, E_DEPRECATED,
			"Calling mb_check_encoding() without argument is deprecated");

		/* FIXME: Actually check all inputs, except $_FILES file content. */
		RETURN_BOOL(MBSTRG(illegalchars) == 0);
	}
}
/* }}} */

static inline crex_long crx_mb_ord(const char *str, size_t str_len, crex_string *enc_name,
	const uint32_t enc_name_arg_num)
{
	const mbfl_encoding *enc;
	enum mbfl_no_encoding no_enc;

	CREX_ASSERT(str_len > 0);

	enc = crx_mb_get_encoding(enc_name, enc_name_arg_num);
	if (!enc) {
		return -2;
	}

	no_enc = enc->no_encoding;
	if (crx_mb_is_unsupported_no_encoding(no_enc)) {
		crex_value_error("mb_ord() does not support the \"%s\" encoding", enc->name);
		return -2;
	}

	/* Some legacy text encodings have a minimum required wchar buffer size;
	 * the ones which need the most are SJIS-Mac, UTF-7, and UTF7-IMAP */
	uint32_t wchar_buf[MBSTRING_MIN_WCHAR_BUFSIZE];
	unsigned int state = 0;
	size_t out_len = enc->to_wchar((unsigned char**)&str, &str_len, wchar_buf, MBSTRING_MIN_WCHAR_BUFSIZE, &state);
	CREX_ASSERT(out_len <= MBSTRING_MIN_WCHAR_BUFSIZE);

	if (!out_len || wchar_buf[0] == MBFL_BAD_INPUT) {
		return -1;
	}
	return wchar_buf[0];
}

/* {{{ */
CRX_FUNCTION(mb_ord)
{
	char *str;
	size_t str_len;
	crex_string *enc = NULL;
	crex_long cp;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STRING(str, str_len)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(enc)
	CREX_PARSE_PARAMETERS_END();

	if (str_len == 0) {
		crex_argument_value_error(1, "must not be empty");
		RETURN_THROWS();
	}

	cp = crx_mb_ord(str, str_len, enc, 2);

	if (0 > cp) {
		if (cp == -2) {
			RETURN_THROWS();
		}
		RETURN_FALSE;
	}

	RETURN_LONG(cp);
}
/* }}} */

static inline crex_string *crx_mb_chr(crex_long cp, crex_string *enc_name, uint32_t enc_name_arg_num)
{
	const mbfl_encoding *enc;
	enum mbfl_no_encoding no_enc;
	crex_string *ret;
	char buf[4];

	enc = crx_mb_get_encoding(enc_name, enc_name_arg_num);
	if (!enc) {
		return NULL;
	}

	no_enc = enc->no_encoding;
	if (crx_mb_is_unsupported_no_encoding(no_enc)) {
		crex_value_error("mb_chr() does not support the \"%s\" encoding", enc->name);
		return NULL;
	}

	if (cp < 0 || cp > 0x10ffff) {
		return NULL;
	}

	if (crx_mb_is_no_encoding_utf8(no_enc)) {
		if (cp > 0xd7ff && 0xe000 > cp) {
			return NULL;
		}

		if (cp < 0x80) {
			ret = ZSTR_CHAR(cp);
		} else if (cp < 0x800) {
			ret = crex_string_alloc(2, 0);
			ZSTR_VAL(ret)[0] = 0xc0 | (cp >> 6);
			ZSTR_VAL(ret)[1] = 0x80 | (cp & 0x3f);
			ZSTR_VAL(ret)[2] = 0;
		} else if (cp < 0x10000) {
			ret = crex_string_alloc(3, 0);
			ZSTR_VAL(ret)[0] = 0xe0 | (cp >> 12);
			ZSTR_VAL(ret)[1] = 0x80 | ((cp >> 6) & 0x3f);
			ZSTR_VAL(ret)[2] = 0x80 | (cp & 0x3f);
			ZSTR_VAL(ret)[3] = 0;
		} else {
			ret = crex_string_alloc(4, 0);
			ZSTR_VAL(ret)[0] = 0xf0 | (cp >> 18);
			ZSTR_VAL(ret)[1] = 0x80 | ((cp >> 12) & 0x3f);
			ZSTR_VAL(ret)[2] = 0x80 | ((cp >> 6) & 0x3f);
			ZSTR_VAL(ret)[3] = 0x80 | (cp & 0x3f);
			ZSTR_VAL(ret)[4] = 0;
		}

		return ret;
	}

	buf[0] = (cp >> 24) & 0xff;
	buf[1] = (cp >> 16) & 0xff;
	buf[2] = (cp >>  8) & 0xff;
	buf[3] = cp & 0xff;

	long orig_illegalchars = MBSTRG(illegalchars);
	MBSTRG(illegalchars) = 0;
	ret = crx_mb_convert_encoding_ex(buf, 4, enc, &mbfl_encoding_ucs4be);

	if (MBSTRG(illegalchars) != 0) {
		crex_string_release(ret);
		ret = NULL;
	}

	MBSTRG(illegalchars) = orig_illegalchars;
	return ret;
}

/* {{{ */
CRX_FUNCTION(mb_chr)
{
	crex_long cp;
	crex_string *enc = NULL;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_LONG(cp)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(enc)
	CREX_PARSE_PARAMETERS_END();

	crex_string* ret = crx_mb_chr(cp, enc, 2);
	if (ret == NULL) {
		RETURN_FALSE;
	}

	RETURN_STR(ret);
}
/* }}} */

CRX_FUNCTION(mb_str_pad)
{
	crex_string *input, *encoding_str = NULL, *pad = ZSTR_CHAR(' ');
	crex_long pad_to_length;
	crex_long pad_type_val = CRX_STR_PAD_RIGHT;

	CREX_PARSE_PARAMETERS_START(2, 5)
		C_PARAM_STR(input)
		C_PARAM_LONG(pad_to_length)
		C_PARAM_OPTIONAL
		C_PARAM_STR(pad)
		C_PARAM_LONG(pad_type_val)
		C_PARAM_STR_OR_NULL(encoding_str)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *encoding = crx_mb_get_encoding(encoding_str, 5);
	if (!encoding) {
		RETURN_THROWS();
	}

	size_t input_length = mb_get_strlen(input, encoding);

	/* If resulting string turns out to be shorter than input string,
	   we simply copy the input and return. */
	if (pad_to_length < 0 || (size_t)pad_to_length <= input_length) {
		RETURN_STR_COPY(input);
	}

	if (ZSTR_LEN(pad) == 0) {
		crex_argument_value_error(3, "must be a non-empty string");
		RETURN_THROWS();
	}

	if (pad_type_val < CRX_STR_PAD_LEFT || pad_type_val > CRX_STR_PAD_BOTH) {
		crex_argument_value_error(4, "must be STR_PAD_LEFT, STR_PAD_RIGHT, or STR_PAD_BOTH");
		RETURN_THROWS();
	}

	size_t pad_length = mb_get_strlen(pad, encoding);

	size_t num_mb_pad_chars = pad_to_length - input_length;

	/* We need to figure out the left/right padding lengths. */
	size_t left_pad = 0, right_pad = 0; /* Initialize here to silence compiler warnings. */
	switch (pad_type_val) {
		case CRX_STR_PAD_RIGHT:
			right_pad = num_mb_pad_chars;
			break;

		case CRX_STR_PAD_LEFT:
			left_pad = num_mb_pad_chars;
			break;

		case CRX_STR_PAD_BOTH:
			left_pad = num_mb_pad_chars / 2;
			right_pad = num_mb_pad_chars - left_pad;
			break;
	}

	/* How many full block copies need to happen, and how many characters are then left over? */
	size_t full_left_pad_copies = left_pad / pad_length;
	size_t full_right_pad_copies = right_pad / pad_length;
	size_t remaining_left_pad_chars = left_pad % pad_length;
	size_t remaining_right_pad_chars = right_pad % pad_length;

	if (UNEXPECTED(full_left_pad_copies > SIZE_MAX / ZSTR_LEN(pad) || full_right_pad_copies > SIZE_MAX / ZSTR_LEN(pad))) {
		goto overflow_no_release;
	}

	/* Compute the number of bytes required for the padding */
	size_t full_left_pad_bytes = full_left_pad_copies * ZSTR_LEN(pad);
	size_t full_right_pad_bytes = full_right_pad_copies * ZSTR_LEN(pad);

	/* No special fast-path handling necessary for zero-length pads because these functions will not
	 * allocate memory in case a zero-length pad is required. */
	crex_string *remaining_left_pad_str = mb_get_substr(pad, 0, remaining_left_pad_chars, encoding);
	crex_string *remaining_right_pad_str = mb_get_substr(pad, 0, remaining_right_pad_chars, encoding);

	if (UNEXPECTED(full_left_pad_bytes > ZSTR_MAX_LEN - ZSTR_LEN(remaining_left_pad_str)
		|| full_right_pad_bytes > ZSTR_MAX_LEN - ZSTR_LEN(remaining_right_pad_str))) {
		goto overflow;
	}

	size_t left_pad_bytes = full_left_pad_bytes + ZSTR_LEN(remaining_left_pad_str);
	size_t right_pad_bytes = full_right_pad_bytes + ZSTR_LEN(remaining_right_pad_str);

	if (UNEXPECTED(left_pad_bytes > ZSTR_MAX_LEN - right_pad_bytes
		|| ZSTR_LEN(input) > ZSTR_MAX_LEN - left_pad_bytes - right_pad_bytes)) {
		goto overflow;
	}

	crex_string *result = crex_string_alloc(ZSTR_LEN(input) + left_pad_bytes + right_pad_bytes, false);
	char *buffer = ZSTR_VAL(result);

	/* First we pad the left. */
	for (size_t i = 0; i < full_left_pad_copies; i++, buffer += ZSTR_LEN(pad)) {
		memcpy(buffer, ZSTR_VAL(pad), ZSTR_LEN(pad));
	}
	memcpy(buffer, ZSTR_VAL(remaining_left_pad_str), ZSTR_LEN(remaining_left_pad_str));
	buffer += ZSTR_LEN(remaining_left_pad_str);

	/* Then we copy the input string. */
	memcpy(buffer, ZSTR_VAL(input), ZSTR_LEN(input));
	buffer += ZSTR_LEN(input);

	/* Finally, we pad on the right. */
	for (size_t i = 0; i < full_right_pad_copies; i++, buffer += ZSTR_LEN(pad)) {
		memcpy(buffer, ZSTR_VAL(pad), ZSTR_LEN(pad));
	}
	memcpy(buffer, ZSTR_VAL(remaining_right_pad_str), ZSTR_LEN(remaining_right_pad_str));

	ZSTR_VAL(result)[ZSTR_LEN(result)] = '\0';

	crex_string_release_ex(remaining_left_pad_str, false);
	crex_string_release_ex(remaining_right_pad_str, false);

	RETURN_NEW_STR(result);

overflow:
	crex_string_release_ex(remaining_left_pad_str, false);
	crex_string_release_ex(remaining_right_pad_str, false);
overflow_no_release:
	crex_throw_error(NULL, "String size overflow");
	RETURN_THROWS();
}

/* {{{ */
CRX_FUNCTION(mb_scrub)
{
	crex_string *str, *enc_name = NULL;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(enc_name)
	CREX_PARSE_PARAMETERS_END();

	const mbfl_encoding *enc = crx_mb_get_encoding(enc_name, 2);
	if (!enc) {
		RETURN_THROWS();
	}

	if (enc == &mbfl_encoding_utf8 && (GC_FLAGS(str) & IS_STR_VALID_UTF8)) {
		/* A valid UTF-8 string will not be changed by mb_scrub; so just increment the refcount and return it */
		RETURN_STR_COPY(str);
	}

	RETURN_STR(crx_mb_convert_encoding_ex(ZSTR_VAL(str), ZSTR_LEN(str), enc, enc));
}
/* }}} */

/* {{{ crx_mb_populate_current_detect_order_list */
static void crx_mb_populate_current_detect_order_list(void)
{
	const mbfl_encoding **entry = 0;
	size_t nentries;

	if (MBSTRG(detect_order_list) && MBSTRG(detect_order_list_size)) {
		nentries = MBSTRG(detect_order_list_size);
		entry = (const mbfl_encoding **)safe_emalloc(nentries, sizeof(mbfl_encoding*), 0);
		memcpy(CREX_VOIDP(entry), MBSTRG(detect_order_list), sizeof(mbfl_encoding*) * nentries);
	} else {
		const enum mbfl_no_encoding *src = MBSTRG(default_detect_order_list);
		size_t i;
		nentries = MBSTRG(default_detect_order_list_size);
		entry = (const mbfl_encoding **)safe_emalloc(nentries, sizeof(mbfl_encoding*), 0);
		for (i = 0; i < nentries; i++) {
			entry[i] = mbfl_no2encoding(src[i]);
		}
	}
	MBSTRG(current_detect_order_list) = entry;
	MBSTRG(current_detect_order_list_size) = nentries;
}
/* }}} */

/* {{{ static int crx_mb_encoding_translation() */
static int crx_mb_encoding_translation(void)
{
	return MBSTRG(encoding_translation);
}
/* }}} */

MBSTRING_API size_t crx_mb_mbchar_bytes(const char *s, const mbfl_encoding *enc)
{
	if (enc) {
		if (enc->mblen_table) {
			if (s) {
				return enc->mblen_table[*(unsigned char *)s];
			}
		} else if (enc->flag & MBFL_ENCTYPE_WCS2) {
			return 2;
		} else if (enc->flag & MBFL_ENCTYPE_WCS4) {
			return 4;
		}
	}
	return 1;
}

MBSTRING_API char *crx_mb_safe_strrchr(const char *s, unsigned int c, size_t nbytes, const mbfl_encoding *enc)
{
	const char *p = s;
	char *last=NULL;

	if (nbytes == (size_t)-1) {
		size_t nb = 0;

		while (*p != '\0') {
			if (nb == 0) {
				if ((unsigned char)*p == (unsigned char)c) {
					last = (char *)p;
				}
				nb = crx_mb_mbchar_bytes(p, enc);
				if (nb == 0) {
					return NULL; /* something is going wrong! */
				}
			}
			--nb;
			++p;
		}
	} else {
		size_t bcnt = nbytes;
		size_t nbytes_char;
		while (bcnt > 0) {
			if ((unsigned char)*p == (unsigned char)c) {
				last = (char *)p;
			}
			nbytes_char = crx_mb_mbchar_bytes(p, enc);
			if (bcnt < nbytes_char) {
				return NULL;
			}
			p += nbytes_char;
			bcnt -= nbytes_char;
		}
	}
	return last;
}

MBSTRING_API size_t crx_mb_stripos(bool mode, crex_string *haystack, crex_string *needle, crex_long offset, const mbfl_encoding *enc)
{
	/* We're using simple case-folding here, because we'd have to deal with remapping of
	 * offsets otherwise. */
	crex_string *haystack_conv = crx_unicode_convert_case(CRX_UNICODE_CASE_FOLD_SIMPLE, ZSTR_VAL(haystack), ZSTR_LEN(haystack), enc, &mbfl_encoding_utf8, MBFL_OUTPUTFILTER_ILLEGAL_MODE_BADUTF8, 0);
	crex_string *needle_conv = crx_unicode_convert_case(CRX_UNICODE_CASE_FOLD_SIMPLE, ZSTR_VAL(needle), ZSTR_LEN(needle), enc, &mbfl_encoding_utf8, MBFL_OUTPUTFILTER_ILLEGAL_MODE_BADUTF8, 0);

	size_t n = mb_find_strpos(haystack_conv, needle_conv, &mbfl_encoding_utf8, offset, mode);

	crex_string_free(haystack_conv);
	crex_string_free(needle_conv);

	return n;
}

static void crx_mb_gpc_get_detect_order(const crex_encoding ***list, size_t *list_size) /* {{{ */
{
	*list = (const crex_encoding **)MBSTRG(http_input_list);
	*list_size = MBSTRG(http_input_list_size);
}
/* }}} */

static void crx_mb_gpc_set_input_encoding(const crex_encoding *encoding) /* {{{ */
{
	MBSTRG(http_input_identify) = (const mbfl_encoding*)encoding;
}
/* }}} */

static const unsigned char base64_table[] = {
 /* 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', */
   0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,
 /* 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', */
   0x4e,0x4f,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,
 /* 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', */
   0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,
 /* 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', */
   0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,
 /* '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0' */
   0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x2b,0x2f,0x00
};

static size_t transfer_encoded_size(mb_convert_buf *tmpbuf, bool base64)
{
	if (base64) {
		return ((mb_convert_buf_len(tmpbuf) + 2) / 3) * 4;
	} else {
		size_t enc_size = 0;
		unsigned char *p = (unsigned char*)ZSTR_VAL(tmpbuf->str);
		while (p < tmpbuf->out) {
			unsigned char c = *p++;
			enc_size += (c > 0x7F || c == '=' || mime_char_needs_qencode[c]) ? 3 : 1;
		}
		return enc_size;
	}
}

static void transfer_encode_mime_bytes(mb_convert_buf *tmpbuf, mb_convert_buf *outbuf, bool base64)
{
	unsigned char *out, *limit;
	MB_CONVERT_BUF_LOAD(outbuf, out, limit);
	unsigned char *p = (unsigned char*)ZSTR_VAL(tmpbuf->str), *e = tmpbuf->out;

	if (base64) {
		MB_CONVERT_BUF_ENSURE(outbuf, out, limit, ((e - p) + 2) / 3 * 4);
		while ((e - p) >= 3) {
			unsigned char a = *p++;
			unsigned char b = *p++;
			unsigned char c = *p++;
			uint32_t bits = (a << 16) | (b << 8) | c;
			out = mb_convert_buf_add4(out,
				base64_table[(bits >> 18) & 0x3F],
				base64_table[(bits >> 12) & 0x3F],
				base64_table[(bits >> 6) & 0x3F],
				base64_table[bits & 0x3F]);
		}
		if (p != e) {
			if ((e - p) == 1) {
				uint32_t bits = *p++;
				out = mb_convert_buf_add4(out, base64_table[(bits >> 2) & 0x3F], base64_table[(bits & 0x3) << 4], '=', '=');
			} else {
				unsigned char a = *p++;
				unsigned char b = *p++;
				uint32_t bits = (a << 8) | b;
				out = mb_convert_buf_add4(out, base64_table[(bits >> 10) & 0x3F], base64_table[(bits >> 4) & 0x3F], base64_table[(bits & 0xF) << 2], '=');
			}
		}
	} else {
		MB_CONVERT_BUF_ENSURE(outbuf, out, limit, (e - p) * 3);
		while (p < e) {
			unsigned char c = *p++;
			if (c > 0x7F || c == '=' || mime_char_needs_qencode[c]) {
				out = mb_convert_buf_add3(out, '=', "0123456789ABCDEF"[(c >> 4) & 0xF], "0123456789ABCDEF"[c & 0xF]);
			} else {
				out = mb_convert_buf_add(out, c);
			}
		}
	}

	mb_convert_buf_reset(tmpbuf, 0);
	MB_CONVERT_BUF_STORE(outbuf, out, limit);
}

static crex_string* mb_mime_header_encode(crex_string *input, const mbfl_encoding *incode, const mbfl_encoding *outcode, bool base64, char *linefeed, size_t linefeed_len, crex_long indent)
{
	unsigned char *in = (unsigned char*)ZSTR_VAL(input);
	size_t in_len = ZSTR_LEN(input);

	if (!in_len) {
		return crex_empty_string;
	}

	if (indent < 0 || indent >= 74) {
		indent = 0;
	}

	if (linefeed_len > 8) {
		linefeed_len = 8;
	}
	/* Maintain legacy behavior as regards embedded NUL (zero) bytes in linefeed string */
	for (size_t i = 0; i < linefeed_len; i++) {
		if (linefeed[i] == '\0') {
			linefeed_len = i;
			break;
		}
	}

	unsigned int state = 0;
	/* wchar_buf should be big enough that when it is full, we definitely have enough
	 * wchars to fill an entire line of output */
	uint32_t wchar_buf[80];
	uint32_t *p, *e;
	/* What part of wchar_buf is filled with still-unprocessed data which should not
	 * be overwritten? */
	unsigned int offset = 0;
	size_t line_start = 0;

	/* If the entire input string is ASCII with no spaces (except possibly leading
	 * spaces), just pass it through unchanged */
	bool checking_leading_spaces = true;
	while (in_len) {
		size_t out_len = incode->to_wchar(&in, &in_len, wchar_buf, 80, &state);
		p = wchar_buf;
		e = wchar_buf + out_len;

		while (p < e) {
			uint32_t w = *p++;
			if (checking_leading_spaces) {
				if (w == ' ') {
					continue;
				} else {
					checking_leading_spaces = false;
				}
			}
			if (w < 0x21 || w > 0x7E || w == '=' || w == '?' || w == '_') {
				/* We cannot simply pass input string through unchanged; start again */
				in = (unsigned char*)ZSTR_VAL(input);
				in_len = ZSTR_LEN(input);
				goto no_passthrough;
			}
		}
	}

	return crex_string_copy(input); /* This just increments refcount */

no_passthrough: ;

	mb_convert_buf buf;
	mb_convert_buf_init(&buf, in_len, '?', MBFL_OUTPUTFILTER_ILLEGAL_MODE_CHAR);

	/* Encode some prefix of the input string as plain ASCII if possible
	 * If we find it necessary to switch to Base64/QPrint encoding, we will
	 * do so all the way to the end of the string */
	while (in_len) {
		/* Decode part of the input string, refill wchar_buf */
		CREX_ASSERT(offset < 80);
		size_t out_len = incode->to_wchar(&in, &in_len, wchar_buf + offset, 80 - offset, &state);
		CREX_ASSERT(out_len <= 80 - offset);
		p = wchar_buf;
		e = wchar_buf + offset + out_len;
		/* ASCII output is broken into space-delimited 'words'
		 * If we find a non-ASCII character in the middle of a word, we will
		 * transfer-encode the entire word */
		uint32_t *word_start = p;

		/* Don't consider adding line feed for spaces at the beginning of a word */
		while (p < e && *p == ' ' && (p - word_start) <= 74) {
			p++;
		}

		while (p < e) {
			uint32_t w = *p++;

			if (w < 0x20 || w > 0x7E || w == '?' || w == '=' || w == '_' || (w == ' ' && (p - word_start) > 74)) {
				/* Non-ASCII character (or line too long); switch to Base64/QPrint encoding
				 * If we are already too far along on a line to include Base64/QPrint encoded data
				 * on the same line (without overrunning max line length), then add a line feed
				 * right now */
				if (mb_convert_buf_len(&buf) - line_start + indent + strlen(outcode->mime_name) > 55) {
					MB_CONVERT_BUF_ENSURE(&buf, buf.out, buf.limit, (e - word_start) + linefeed_len + 1);
					buf.out = mb_convert_buf_appendn(buf.out, linefeed, linefeed_len);
					buf.out = mb_convert_buf_add(buf.out, ' ');
					indent = 0;
					line_start = mb_convert_buf_len(&buf);
				} else if (mb_convert_buf_len(&buf) > 0) {
					MB_CONVERT_BUF_ENSURE(&buf, buf.out, buf.limit, 1);
					buf.out = mb_convert_buf_add(buf.out, ' ');
				}
				p = word_start; /* Back up to where MIME encoding of input chars should start */
				goto mime_encoding_needed;
			} else if (w == ' ') {
				/* When we see a space, check whether we should insert a line break */
				if (mb_convert_buf_len(&buf) - line_start + (p - word_start) + indent > 75) {
					MB_CONVERT_BUF_ENSURE(&buf, buf.out, buf.limit, (e - word_start) + linefeed_len + 1);
					buf.out = mb_convert_buf_appendn(buf.out, linefeed, linefeed_len);
					buf.out = mb_convert_buf_add(buf.out, ' ');
					indent = 0;
					line_start = mb_convert_buf_len(&buf);
				} else if (mb_convert_buf_len(&buf) > 0) {
					MB_CONVERT_BUF_ENSURE(&buf, buf.out, buf.limit, (e - word_start) + 1);
					buf.out = mb_convert_buf_add(buf.out, ' ');
				}
				/* Output one (space-delimited) word as plain ASCII */
				while (word_start < p-1) {
					buf.out = mb_convert_buf_add(buf.out, *word_start++ & 0xFF);
				}
				word_start++;
				while (p < e && *p == ' ') {
					p++;
				}
			}
		}

		if (in_len) {
			/* Copy chars which are part of an incomplete 'word' to the beginning
			 * of wchar_buf and reprocess them on the next iteration */
			offset = e - word_start;
			if (offset) {
				memmove(wchar_buf, word_start, offset * sizeof(uint32_t));
			}
		} else {
			/* We have reached the end of the input string while still in 'ASCII mode';
			 * process any trailing ASCII chars which were not followed by a space */
			if (word_start < e && mb_convert_buf_len(&buf) > 0) {
				/* The whole input string was not just one big ASCII 'word' with no spaces
				 * consider adding a line feed if necessary to prevent output lines from
				 * being too long */
				if (mb_convert_buf_len(&buf) - line_start + (p - word_start) + indent > 74) {
					MB_CONVERT_BUF_ENSURE(&buf, buf.out, buf.limit, (e - word_start) + linefeed_len + 1);
					buf.out = mb_convert_buf_appendn(buf.out, linefeed, linefeed_len);
					buf.out = mb_convert_buf_add(buf.out, ' ');
				} else {
					MB_CONVERT_BUF_ENSURE(&buf, buf.out, buf.limit, (e - word_start) + 1);
					buf.out = mb_convert_buf_add(buf.out, ' ');
				}
			}
			while (word_start < e) {
				buf.out = mb_convert_buf_add(buf.out, *word_start++ & 0xFF);
			}
		}
	}

	/* Ensure output string is marked as valid UTF-8 (ASCII strings are always 'valid UTF-8') */
	return mb_convert_buf_result(&buf, &mbfl_encoding_utf8);

mime_encoding_needed: ;

	/* We will generate the output line by line, first converting wchars to bytes
	 * in the requested output encoding, then transfer-encoding those bytes as
	 * Base64 or QPrint
	 * 'tmpbuf' will receive the bytes which need to be transfer-encoded before
	 * sending them to 'buf' */
	mb_convert_buf tmpbuf;
	mb_convert_buf_init(&tmpbuf, in_len, '?', MBFL_OUTPUTFILTER_ILLEGAL_MODE_CHAR);

	/* Do we need to refill wchar_buf to make sure we don't run out of wchars
	 * in the middle of a line? */
	if (p == wchar_buf) {
		goto start_new_line;
	}
	offset = e - p;
	memmove(wchar_buf, p, offset * sizeof(uint32_t));

	while(true) {
refill_wchar_buf: ;
		CREX_ASSERT(offset < 80);
		size_t out_len = incode->to_wchar(&in, &in_len, wchar_buf + offset, 80 - offset, &state);
		CREX_ASSERT(out_len <= 80 - offset);
		p = wchar_buf;
		e = wchar_buf + offset + out_len;

start_new_line: ;
		MB_CONVERT_BUF_ENSURE(&buf, buf.out, buf.limit, strlen(outcode->mime_name) + 5);
		buf.out = mb_convert_buf_add2(buf.out, '=', '?');
		buf.out = mb_convert_buf_appends(buf.out, outcode->mime_name);
		buf.out = mb_convert_buf_add3(buf.out, '?', base64 ? 'B' : 'Q', '?');

		/* How many wchars should we try converting to Base64/QPrint-encoded bytes?
		 * We do something like a 'binary search' to find the greatest number which
		 * can be included on this line without exceeding max line length */
		unsigned int n = 12;
		size_t space_available = 73 - indent - (mb_convert_buf_len(&buf) - line_start);

		while (true) {
			CREX_ASSERT(p < e);

			/* Remember where we were in process of generating output, so we can back
			 * up if necessary */
			size_t tmppos = mb_convert_buf_len(&tmpbuf);
			unsigned int tmpstate = tmpbuf.state;

			/* Try encoding 'n' wchars in output text encoding and sending output
			 * bytes to 'tmpbuf'. Hopefully this is not too many to fit on the
			 * current line. */
			n = MIN(n, e - p);
			outcode->from_wchar(p, n, &tmpbuf, false);

			/* For some output text encodings, there may be a few ending bytes
			 * which need to be emitted to output before we break a line.
			 * Again, remember where we were so we can back up */
			size_t tmppos2 = mb_convert_buf_len(&tmpbuf);
			unsigned int tmpstate2 = tmpbuf.state;
			outcode->from_wchar(NULL, 0, &tmpbuf, true);

			if (transfer_encoded_size(&tmpbuf, base64) <= space_available || (n == 1 && tmppos == 0)) {
				/* If we convert 'n' more wchars on the current line, it will not
				 * overflow the maximum line length */
				p += n;

				if (p == e) {
					/* We are done; we shouldn't reach here if there is more remaining
					 * of the input string which needs to be processed */
					CREX_ASSERT(!in_len);
					transfer_encode_mime_bytes(&tmpbuf, &buf, base64);
					MB_CONVERT_BUF_ENSURE(&buf, buf.out, buf.limit, 2);
					buf.out = mb_convert_buf_add2(buf.out, '?', '=');
					mb_convert_buf_free(&tmpbuf);
					return mb_convert_buf_result(&buf, &mbfl_encoding_utf8);
				} else {
					/* It's possible that more chars might fit on the current line,
					 * so back up to where we were before emitting any ending bytes */
					mb_convert_buf_reset(&tmpbuf, tmppos2);
					tmpbuf.state = tmpstate2;
				}
			} else {
				/* Converting 'n' more wchars on this line would be too much.
				 * Back up to where we were before we tried that. */
				mb_convert_buf_reset(&tmpbuf, tmppos);
				tmpbuf.state = tmpstate;

				if (n == 1) {
					/* We have found the exact number of chars which will fit on the
					 * current line. Finish up and move to a new line. */
					outcode->from_wchar(NULL, 0, &tmpbuf, true);
					transfer_encode_mime_bytes(&tmpbuf, &buf, base64);
					tmpbuf.state = 0;

					MB_CONVERT_BUF_ENSURE(&buf, buf.out, buf.limit, 3 + linefeed_len);
					buf.out = mb_convert_buf_add2(buf.out, '?', '=');

					indent = 0; /* Indent argument must only affect the first line */

					if (in_len) {
						/* We still have more of input string remaining to decode */
						buf.out = mb_convert_buf_appendn(buf.out, linefeed, linefeed_len);
						buf.out = mb_convert_buf_add(buf.out, ' ');
						line_start = mb_convert_buf_len(&buf);
						/* Copy remaining wchars to beginning of buffer so they will be
						 * processed on the next iteration of outer 'do' loop */
						offset = e - p;
						memmove(wchar_buf, p, offset * sizeof(uint32_t));
						goto refill_wchar_buf;
					} else if (p < e) {
						/* Input string is finished, but we still have trailing wchars
						 * remaining to be processed in wchar_buf */
						buf.out = mb_convert_buf_appendn(buf.out, linefeed, linefeed_len);
						buf.out = mb_convert_buf_add(buf.out, ' ');
						line_start = mb_convert_buf_len(&buf);
						goto start_new_line;
					} else {
						/* We are done! */
						mb_convert_buf_free(&tmpbuf);
						return mb_convert_buf_result(&buf, &mbfl_encoding_utf8);
					}
				} else {
					/* Try a smaller number of wchars */
					n = MAX(n >> 1, 1);
				}
			}
		}
	}
}

CRX_FUNCTION(mb_encode_mimeheader)
{
	const mbfl_encoding *charset = &mbfl_encoding_pass;
	crex_string *str, *charset_name = NULL, *transenc_name = NULL;
	char *linefeed = "\r\n";
	size_t linefeed_len = 2;
	crex_long indent = 0;
	bool base64 = true;

	CREX_PARSE_PARAMETERS_START(1, 5)
		C_PARAM_STR(str)
		C_PARAM_OPTIONAL
		C_PARAM_STR(charset_name)
		C_PARAM_STR(transenc_name)
		C_PARAM_STRING(linefeed, linefeed_len)
		C_PARAM_LONG(indent)
	CREX_PARSE_PARAMETERS_END();

	if (charset_name != NULL) {
		charset = crx_mb_get_encoding(charset_name, 2);
		if (!charset) {
			RETURN_THROWS();
		} else if (charset->mime_name == NULL || charset->mime_name[0] == '\0') {
			crex_argument_value_error(2, "\"%s\" cannot be used for MIME header encoding", ZSTR_VAL(charset_name));
			RETURN_THROWS();
		}
	} else {
		const mbfl_language *lang = mbfl_no2language(MBSTRG(language));
		if (lang != NULL) {
			charset = mbfl_no2encoding(lang->mail_charset);
			const mbfl_encoding *transenc = mbfl_no2encoding(lang->mail_header_encoding);
			char t = transenc->name[0];
			if (t == 'Q' || t == 'q') {
				base64 = false;
			}
		}
	}

	if (transenc_name != NULL && ZSTR_LEN(transenc_name) > 0) {
		char t = ZSTR_VAL(transenc_name)[0];
		if (t == 'Q' || t == 'q') {
			base64 = false;
		}
	}

	RETURN_STR(mb_mime_header_encode(str, MBSTRG(current_internal_encoding), charset, base64, linefeed, linefeed_len, indent));
}

static int8_t decode_base64(unsigned char c)
{
	if (c >= 'A' && c <= 'Z') {
		return c - 'A';
	} else if (c >= 'a' && c <= 'z') {
		return c - 'a' + 26;
	} else if (c >= '0' && c <= '9') {
		return c - '0' + 52;
	} else if (c == '+') {
		return 62;
	} else if (c == '/') {
		return 63;
	}
	return -1;
}

static int8_t qprint_map[] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
	-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

/* Decode MIME encoded word as defined in RFC 2047 */
static unsigned char* mime_header_decode_encoded_word(unsigned char *p, unsigned char *e, const mbfl_encoding *outcode, mb_convert_buf *outbuf, unsigned int *state)
{
	if ((e - p) < 6) {
		return NULL;
	}

	CREX_ASSERT(p[0] == '=');
	CREX_ASSERT(p[1] == '?');
	p += 2;

	unsigned char *charset = p;
	unsigned char *charset_end = memchr(charset, '?', e - charset);
	if (charset_end == NULL) {
		return NULL;
	}

	unsigned char *encoding = charset_end + 1;
	p = encoding + 1;
	if (p >= e || *p++ != '?') {
		return NULL;
	}

	char *charset_name = estrndup((const char*)charset, charset_end - charset);
	const mbfl_encoding *incode = mbfl_name2encoding(charset_name);
	efree(charset_name);
	if (incode == NULL) {
		return NULL;
	}

	unsigned char *end_marker = (unsigned char*)crex_memnstr((const char*)p, "?=", 2, (const char*)e);
	if (end_marker) {
		e = end_marker;
	} else if (p < e && *(e-1) == '?') {
		/* If encoded word is not properly terminated, but last byte is '?',
		 * take that as a terminator (legacy behavior) */
		e--;
	}

	unsigned char *buf = emalloc(e - p), *bufp = buf;
	if (*encoding == 'Q' || *encoding == 'q') {
		/* Fill `buf` with bytes from decoding QPrint */
		while (p < e) {
			unsigned char c = *p++;
			if (c == '_') {
				*bufp++ = ' ';
				continue;
			} else if (c == '=' && (e - p) >= 2) {
				unsigned char c2 = *p++;
				unsigned char c3 = *p++;
				if (qprint_map[c2] >= 0 && qprint_map[c3] >= 0) {
					*bufp++ = (qprint_map[c2] << 4) | (qprint_map[c3] & 0xF);
					continue;
				} else if (c2 == '\r') {
					if (c3 != '\n') {
						p--;
					}
					continue;
				} else if (c2 == '\n') {
					p--;
					continue;
				}
			}
			*bufp++ = c;
		}
	} else if (*encoding == 'B' || *encoding == 'b') {
		/* Fill `buf` with bytes from decoding Base64 */
		unsigned int bits = 0, cache = 0;
		while (p < e) {
			unsigned char c = *p++;
			if (c == '\r' || c == '\n' || c == ' ' || c == '\t' || c == '=') {
				continue;
			}
			int8_t decoded = decode_base64(c);
			if (decoded == -1) {
				*bufp++ = '?';
				continue;
			}
			bits += 6;
			cache = (cache << 6) | (decoded & 0x3F);
			if (bits == 24) {
				*bufp++ = (cache >> 16) & 0xFF;
				*bufp++ = (cache >> 8) & 0xFF;
				*bufp++ = cache & 0xFF;
				bits = cache = 0;
			}
		}
		if (bits == 18) {
			*bufp++ = (cache >> 10) & 0xFF;
			*bufp++ = (cache >> 2) & 0xFF;
		} else if (bits == 12) {
			*bufp++ = (cache >> 4) & 0xFF;
		}
	} else {
		efree(buf);
		return NULL;
	}

	size_t in_len = bufp - buf;
	uint32_t wchar_buf[128];

	bufp = buf;
	while (in_len) {
		size_t out_len = incode->to_wchar(&bufp, &in_len, wchar_buf, 128, state);
		CREX_ASSERT(out_len <= 128);
		outcode->from_wchar(wchar_buf, out_len, outbuf, false);
	}

	efree(buf);
	return e + 2;
}

static crex_string* mb_mime_header_decode(crex_string *input, const mbfl_encoding *outcode)
{
	unsigned char *p = (unsigned char*)ZSTR_VAL(input), *e = p + ZSTR_LEN(input);
	unsigned int state = 0;
	bool space_pending = false;

	mb_convert_buf buf;
	mb_convert_buf_init(&buf, ZSTR_LEN(input), '?', MBFL_OUTPUTFILTER_ILLEGAL_MODE_CHAR);

	while (p < e) {
		unsigned char c = *p;

		if (c == '=' && *(p + 1) == '?' && (e - p) >= 6) {
			/* Does this look like a MIME encoded word? If so, try to decode it as one */
			unsigned char *incode_end = memchr(p + 2, '?', e - p - 2);
			if (incode_end && (e - incode_end) >= 3) {
				unsigned char *temp = mime_header_decode_encoded_word(p, e, outcode, &buf, &state);
				if (temp) {
					p = temp;
					/* Decoding of MIME encoded word was successful;
					 * Try to collapse a run of whitespace */
					if (p < e && (*p == '\n' || *p == '\r')) {
						do {
							p++;
						} while (p < e && (*p == '\n' || *p == '\r' || *p == '\t' || *p == ' '));
						/* We will only actually output a space if this is not immediately followed
						 * by another valid encoded word */
						space_pending = true;
					}
					continue;
				}
			}
		}

		if (space_pending) {
			uint32_t space = ' ';
			outcode->from_wchar(&space, 1, &buf, false);
			space_pending = false;
		}

		/* Consume a run of plain ASCII characters */
		if (c != '\n' && c != '\r') {
			unsigned char *end = p + 1;
			while (end < e && (*end != '=' && *end != '\n' && *end != '\r')) {
				end++;
			}
			uint32_t wchar_buf[128];
			size_t in_len = end - p;
			while (in_len) {
				size_t out_len = mbfl_encoding_ascii.to_wchar(&p, &in_len, wchar_buf, 128, &state);
				CREX_ASSERT(out_len <= 128);
				outcode->from_wchar(wchar_buf, out_len, &buf, false);
			}
		}
		/* Collapse a run of whitespace into a single space */
		if (p < e && (*p == '\n' || *p == '\r')) {
			do {
				p++;
			} while (p < e && (*p == '\n' || *p == '\r' || *p == '\t' || *p == ' '));
			if (p < e) {
				/* Emulating legacy behavior of mb_decode_mimeheader here;
				 * a run of whitespace is not converted to a space at the very
				 * end of the input string */
				uint32_t space = ' ';
				outcode->from_wchar(&space, 1, &buf, false);
			}
		}
	}

	outcode->from_wchar(NULL, 0, &buf, true);

	return mb_convert_buf_result(&buf, outcode);
}

CRX_FUNCTION(mb_decode_mimeheader)
{
	crex_string *str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	RETURN_STR(mb_mime_header_decode(str, MBSTRG(current_internal_encoding)));
}
