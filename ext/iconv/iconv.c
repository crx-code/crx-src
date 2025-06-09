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
   | Authors: Rui Hirokawa <rui_hirokawa@ybb.ne.jp>                       |
   |          Stig Bakken <ssb@crx.net>                                   |
   |          Moriyoshi Koizumi <moriyoshi@crx.net>                       |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_globals.h"
#include "ext/standard/info.h"
#include "main/crx_output.h"
#include "SAPI.h"
#include "crx_ini.h"

#include <stdlib.h>
#include <errno.h>

#include "crx_iconv.h"

#ifdef HAVE_ICONV

#include <iconv.h>

#ifdef HAVE_GLIBC_ICONV
#include <gnu/libc-version.h>
#endif

#ifdef HAVE_LIBICONV
#undef iconv
#endif

#if defined(__NetBSD__)
// unfortunately, netbsd has still the old non posix conformant signature
// libiconv tends to match the eventual system's iconv too.
#define ICONV_CONST const
#else
#define ICONV_CONST
#endif

#include "crex_smart_str.h"
#include "ext/standard/base64.h"
#include "ext/standard/quot_print.h"

#ifdef CRX_ICONV_IMPL
#define CRX_ICONV_IMPL_VALUE CRX_ICONV_IMPL
#elif HAVE_LIBICONV
#define CRX_ICONV_IMPL_VALUE "libiconv"
#else
#define CRX_ICONV_IMPL_VALUE "unknown"
#endif

char *get_iconv_version(void) {
	char *version = "unknown";

#ifdef HAVE_LIBICONV
	static char buf[16];
	snprintf(buf, sizeof(buf), "%d.%d", _libiconv_version >> 8, _libiconv_version & 0xff);
	version = buf;
#elif HAVE_GLIBC_ICONV
	version = (char *) gnu_get_libc_version();
#endif

	return version;
}

#define CRX_ICONV_MIME_DECODE_STRICT            (1<<0)
#define CRX_ICONV_MIME_DECODE_CONTINUE_ON_ERROR (1<<1)

#include "iconv_arginfo.h"

#define _crx_iconv_memequal(a, b, c) \
	(memcmp(a, b, c) == 0)

CREX_DECLARE_MODULE_GLOBALS(iconv)
static CRX_GINIT_FUNCTION(iconv);

/* {{{ iconv_module_entry */
crex_module_entry iconv_module_entry = {
	STANDARD_MODULE_HEADER,
	"iconv",
	ext_functions,
	CRX_MINIT(miconv),
	CRX_MSHUTDOWN(miconv),
	NULL,
	NULL,
	CRX_MINFO(miconv),
	CRX_ICONV_VERSION,
	CRX_MODULE_GLOBALS(iconv),
	CRX_GINIT(iconv),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_ICONV
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(iconv)
#endif

/* {{{ CRX_GINIT_FUNCTION */
static CRX_GINIT_FUNCTION(iconv)
{
#if defined(COMPILE_DL_ICONV) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	iconv_globals->input_encoding = NULL;
	iconv_globals->output_encoding = NULL;
	iconv_globals->internal_encoding = NULL;
}
/* }}} */

#if defined(HAVE_LIBICONV) && defined(ICONV_ALIASED_LIBICONV)
#define iconv libiconv
#endif

/* {{{ typedef enum crx_iconv_enc_scheme_t */
typedef enum _crx_iconv_enc_scheme_t {
	CRX_ICONV_ENC_SCHEME_BASE64,
	CRX_ICONV_ENC_SCHEME_QPRINT
} crx_iconv_enc_scheme_t;
/* }}} */

/* {{{ prototypes */
static crx_iconv_err_t _crx_iconv_appendl(smart_str *d, const char *s, size_t l, iconv_t cd);
static crx_iconv_err_t _crx_iconv_appendc(smart_str *d, const char c, iconv_t cd);

static void _crx_iconv_show_error(crx_iconv_err_t err, const char *out_charset, const char *in_charset);

static crx_iconv_err_t _crx_iconv_strlen(size_t *pretval, const char *str, size_t nbytes, const char *enc);

static crx_iconv_err_t _crx_iconv_substr(smart_str *pretval, const char *str, size_t nbytes, crex_long offset, crex_long len, const char *enc);

static crx_iconv_err_t _crx_iconv_mime_encode(smart_str *pretval, const char *fname, size_t fname_nbytes, const char *fval, size_t fval_nbytes, size_t max_line_len, const char *lfchars, crx_iconv_enc_scheme_t enc_scheme, const char *out_charset, const char *enc);

static crx_iconv_err_t _crx_iconv_mime_decode(smart_str *pretval, const char *str, size_t str_nbytes, const char *enc, const char **next_pos, int mode);

static crx_iconv_err_t crx_iconv_stream_filter_register_factory(void);
static crx_iconv_err_t crx_iconv_stream_filter_unregister_factory(void);

static int crx_iconv_output_conflict(const char *handler_name, size_t handler_name_len);
static crx_output_handler *crx_iconv_output_handler_init(const char *name, size_t name_len, size_t chunk_size, int flags);
static int crx_iconv_output_handler(void **nothing, crx_output_context *output_context);
/* }}} */

/* {{{ static globals */
static const char _generic_superset_name[] = ICONV_UCS4_ENCODING;
#define GENERIC_SUPERSET_NAME _generic_superset_name
#define GENERIC_SUPERSET_NBYTES 4
/* }}} */


static CRX_INI_MH(OnUpdateInputEncoding)
{
	if (ZSTR_LEN(new_value) >= ICONV_CSNMAXLEN) {
		return FAILURE;
	}
	if (stage & (CRX_INI_STAGE_ACTIVATE | CRX_INI_STAGE_RUNTIME)) {
		crx_error_docref("ref.iconv", E_DEPRECATED, "Use of iconv.input_encoding is deprecated");
	}
	OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
	return SUCCESS;
}


static CRX_INI_MH(OnUpdateOutputEncoding)
{
	if (ZSTR_LEN(new_value) >= ICONV_CSNMAXLEN) {
		return FAILURE;
	}
	if (stage & (CRX_INI_STAGE_ACTIVATE | CRX_INI_STAGE_RUNTIME)) {
		crx_error_docref("ref.iconv", E_DEPRECATED, "Use of iconv.output_encoding is deprecated");
	}
	OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
	return SUCCESS;
}


static CRX_INI_MH(OnUpdateInternalEncoding)
{
	if (ZSTR_LEN(new_value) >= ICONV_CSNMAXLEN) {
		return FAILURE;
	}
	if (stage & (CRX_INI_STAGE_ACTIVATE | CRX_INI_STAGE_RUNTIME)) {
		crx_error_docref("ref.iconv", E_DEPRECATED, "Use of iconv.internal_encoding is deprecated");
	}
	OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
	return SUCCESS;
}


/* {{{ CRX_INI */
CRX_INI_BEGIN()
	STD_CRX_INI_ENTRY("iconv.input_encoding",    "", CRX_INI_ALL, OnUpdateInputEncoding,    input_encoding,    crex_iconv_globals, iconv_globals)
	STD_CRX_INI_ENTRY("iconv.output_encoding",   "", CRX_INI_ALL, OnUpdateOutputEncoding,   output_encoding,   crex_iconv_globals, iconv_globals)
	STD_CRX_INI_ENTRY("iconv.internal_encoding", "", CRX_INI_ALL, OnUpdateInternalEncoding, internal_encoding, crex_iconv_globals, iconv_globals)
CRX_INI_END()
/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(miconv)
{
	REGISTER_INI_ENTRIES();

	if (crx_iconv_stream_filter_register_factory() != CRX_ICONV_ERR_SUCCESS) {
		return FAILURE;
	}

	register_iconv_symbols(module_number);

	crx_output_handler_alias_register(CREX_STRL("ob_iconv_handler"), crx_iconv_output_handler_init);
	crx_output_handler_conflict_register(CREX_STRL("ob_iconv_handler"), crx_iconv_output_conflict);

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(miconv)
{
	crx_iconv_stream_filter_unregister_factory();
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(miconv)
{
	zval *iconv_impl, *iconv_ver;

	iconv_impl = crex_get_constant_str("ICONV_IMPL", sizeof("ICONV_IMPL")-1);
	iconv_ver = crex_get_constant_str("ICONV_VERSION", sizeof("ICONV_VERSION")-1);

	crx_info_print_table_start();
	crx_info_print_table_row(2, "iconv support", "enabled");
	crx_info_print_table_row(2, "iconv implementation", C_STRVAL_P(iconv_impl));
	crx_info_print_table_row(2, "iconv library version", C_STRVAL_P(iconv_ver));
	crx_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

static const char *get_internal_encoding(void) {
	if (ICONVG(internal_encoding) && ICONVG(internal_encoding)[0]) {
		return ICONVG(internal_encoding);
	}
	return crx_get_internal_encoding();
}

static const char *get_input_encoding(void) {
	if (ICONVG(input_encoding) && ICONVG(input_encoding)[0]) {
		return ICONVG(input_encoding);
	}
	return crx_get_input_encoding();
}

static const char *get_output_encoding(void) {
	if (ICONVG(output_encoding) && ICONVG(output_encoding)[0]) {
		return ICONVG(output_encoding);
	}
	return crx_get_output_encoding();
}


static int crx_iconv_output_conflict(const char *handler_name, size_t handler_name_len)
{
	if (crx_output_get_level()) {
		if (crx_output_handler_conflict(handler_name, handler_name_len, CREX_STRL("ob_iconv_handler"))
		||	crx_output_handler_conflict(handler_name, handler_name_len, CREX_STRL("mb_output_handler"))) {
			return FAILURE;
		}
	}
	return SUCCESS;
}

static crx_output_handler *crx_iconv_output_handler_init(const char *handler_name, size_t handler_name_len, size_t chunk_size, int flags)
{
	return crx_output_handler_create_internal(handler_name, handler_name_len, crx_iconv_output_handler, chunk_size, flags);
}

static int crx_iconv_output_handler(void **nothing, crx_output_context *output_context)
{
	char *s, *content_type, *mimetype = NULL;
	int output_status, mimetype_len = 0;

	if (output_context->op & CRX_OUTPUT_HANDLER_START) {
		output_status = crx_output_get_status();
		if (output_status & CRX_OUTPUT_SENT) {
			return FAILURE;
		}

		if (SG(sapi_headers).mimetype && !strncasecmp(SG(sapi_headers).mimetype, "text/", 5)) {
			if ((s = strchr(SG(sapi_headers).mimetype,';')) == NULL){
				mimetype = SG(sapi_headers).mimetype;
			} else {
				mimetype = SG(sapi_headers).mimetype;
				mimetype_len = (int)(s - SG(sapi_headers).mimetype);
			}
		} else if (SG(sapi_headers).send_default_content_type) {
			mimetype = SG(default_mimetype) ? SG(default_mimetype) : SAPI_DEFAULT_MIMETYPE;
		}

		if (mimetype != NULL && (!(output_context->op & CRX_OUTPUT_HANDLER_CLEAN) || ((output_context->op & CRX_OUTPUT_HANDLER_START) && !(output_context->op & CRX_OUTPUT_HANDLER_FINAL)))) {
			size_t len;
			char *p = strstr(get_output_encoding(), "//");

			if (p) {
				len = spprintf(&content_type, 0, "Content-Type:%.*s; charset=%.*s", mimetype_len ? mimetype_len : (int) strlen(mimetype), mimetype, (int) (p - get_output_encoding()), get_output_encoding());
			} else {
				len = spprintf(&content_type, 0, "Content-Type:%.*s; charset=%s", mimetype_len ? mimetype_len : (int) strlen(mimetype), mimetype, get_output_encoding());
			}
			if (content_type && SUCCESS == sapi_add_header(content_type, len, 0)) {
				SG(sapi_headers).send_default_content_type = 0;
				crx_output_handler_hook(CRX_OUTPUT_HANDLER_HOOK_IMMUTABLE, NULL);
			}
		}
	}

	if (output_context->in.used) {
		crex_string *out;
		output_context->out.free = 1;
		_crx_iconv_show_error(crx_iconv_string(output_context->in.data, output_context->in.used, &out, get_output_encoding(), get_internal_encoding()), get_output_encoding(), get_internal_encoding());
		if (out) {
			output_context->out.data = estrndup(ZSTR_VAL(out), ZSTR_LEN(out));
			output_context->out.used = ZSTR_LEN(out);
			crex_string_efree(out);
		} else {
			output_context->out.data = NULL;
			output_context->out.used = 0;
		}
	}

	return SUCCESS;
}

/* {{{ _crx_iconv_appendl() */
static crx_iconv_err_t _crx_iconv_appendl(smart_str *d, const char *s, size_t l, iconv_t cd)
{
	const char *in_p = s;
	size_t in_left = l;
	char *out_p;
	size_t out_left = 0;
	size_t buf_growth = 128;

	if (in_p != NULL) {
		while (in_left > 0) {
			out_left = buf_growth;
			smart_str_alloc(d, out_left, 0);

			out_p = ZSTR_VAL((d)->s) + ZSTR_LEN((d)->s);

			if (iconv(cd, (ICONV_CONST char **)&in_p, &in_left, (char **) &out_p, &out_left) == (size_t)-1) {
				switch (errno) {
					case EINVAL:
						return CRX_ICONV_ERR_ILLEGAL_CHAR;

					case EILSEQ:
						return CRX_ICONV_ERR_ILLEGAL_SEQ;

					case E2BIG:
						break;

					default:
						return CRX_ICONV_ERR_UNKNOWN;
				}
			}
			ZSTR_LEN((d)->s) += (buf_growth - out_left);
			buf_growth <<= 1;
		}
	} else {
		for (;;) {
			out_left = buf_growth;
			smart_str_alloc(d, out_left, 0);

			out_p = ZSTR_VAL((d)->s) + ZSTR_LEN((d)->s);

			if (iconv(cd, NULL, NULL, (char **) &out_p, &out_left) == (size_t)0) {
				ZSTR_LEN((d)->s) += (buf_growth - out_left);
				break;
			} else {
				if (errno != E2BIG) {
					return CRX_ICONV_ERR_UNKNOWN;
				}
			}
			ZSTR_LEN((d)->s) += (buf_growth - out_left);
			buf_growth <<= 1;
		}
	}
	return CRX_ICONV_ERR_SUCCESS;
}
/* }}} */

/* {{{ _crx_iconv_appendc() */
static crx_iconv_err_t _crx_iconv_appendc(smart_str *d, const char c, iconv_t cd)
{
	return _crx_iconv_appendl(d, &c, 1, cd);
}
/* }}} */

/* {{{ */
#ifdef ICONV_BROKEN_IGNORE
static int _crx_check_ignore(const char *charset)
{
	size_t clen = strlen(charset);
	if (clen >= 9 && strcmp("//IGNORE", charset+clen-8) == 0) {
		return 1;
	}
	if (clen >= 19 && strcmp("//IGNORE//TRANSLIT", charset+clen-18) == 0) {
		return 1;
	}
	return 0;
}
#else
#define _crx_check_ignore(x) (0)
#endif
/* }}} */

/* {{{ crx_iconv_string() */
CRX_ICONV_API crx_iconv_err_t crx_iconv_string(const char *in_p, size_t in_len, crex_string **out, const char *out_charset, const char *in_charset)
{
	iconv_t cd;
	size_t in_left, out_size, out_left;
	char *out_p;
	size_t bsz, result = 0;
	crx_iconv_err_t retval = CRX_ICONV_ERR_SUCCESS;
	crex_string *out_buf;
	int ignore_ilseq = _crx_check_ignore(out_charset);

	*out = NULL;

	cd = iconv_open(out_charset, in_charset);

	if (cd == (iconv_t)(-1)) {
		if (errno == EINVAL) {
			return CRX_ICONV_ERR_WRONG_CHARSET;
		} else {
			return CRX_ICONV_ERR_CONVERTER;
		}
	}
	in_left= in_len;
	out_left = in_len + 32; /* Avoid realloc() most cases */
	out_size = 0;
	bsz = out_left;
	out_buf = crex_string_alloc(bsz, 0);
	out_p = ZSTR_VAL(out_buf);

	while (in_left > 0) {
		result = iconv(cd, (ICONV_CONST char **) &in_p, &in_left, (char **) &out_p, &out_left);
		out_size = bsz - out_left;
		if (result == (size_t)(-1)) {
			if (ignore_ilseq && errno == EILSEQ) {
				if (in_left <= 1) {
					result = 0;
				} else {
					errno = 0;
					in_p++;
					in_left--;
					continue;
				}
			}

			if (errno == E2BIG && in_left > 0) {
				/* converted string is longer than out buffer */
				bsz += in_len;

				out_buf = crex_string_extend(out_buf, bsz, 0);
				out_p = ZSTR_VAL(out_buf);
				out_p += out_size;
				out_left = bsz - out_size;
				continue;
			}
		}
		break;
	}

	if (result != (size_t)(-1)) {
		/* flush the shift-out sequences */
		for (;;) {
		   	result = iconv(cd, NULL, NULL, (char **) &out_p, &out_left);
			out_size = bsz - out_left;

			if (result != (size_t)(-1)) {
				break;
			}

			if (errno == E2BIG) {
				bsz += 16;
				out_buf = crex_string_extend(out_buf, bsz, 0);
				out_p = ZSTR_VAL(out_buf);
				out_p += out_size;
				out_left = bsz - out_size;
			} else {
				break;
			}
		}
	}

	iconv_close(cd);

	if (result == (size_t)(-1)) {
		switch (errno) {
			case EINVAL:
				retval = CRX_ICONV_ERR_ILLEGAL_CHAR;
				break;

			case EILSEQ:
				retval = CRX_ICONV_ERR_ILLEGAL_SEQ;
				break;

			case E2BIG:
				/* should not happen */
				retval = CRX_ICONV_ERR_TOO_BIG;
				break;

			default:
				/* other error */
				crex_string_efree(out_buf);
				return CRX_ICONV_ERR_UNKNOWN;
		}
	}
	*out_p = '\0';
	ZSTR_LEN(out_buf) = out_size;
	*out = out_buf;
	return retval;
}
/* }}} */

/* {{{ _crx_iconv_strlen() */
static crx_iconv_err_t _crx_iconv_strlen(size_t *pretval, const char *str, size_t nbytes, const char *enc)
{
	char buf[GENERIC_SUPERSET_NBYTES*2];

	crx_iconv_err_t err = CRX_ICONV_ERR_SUCCESS;

	iconv_t cd;

	const char *in_p;
	size_t in_left;

	char *out_p;
	size_t out_left;

	size_t cnt;
	int more;

	*pretval = (size_t)-1;

	cd = iconv_open(GENERIC_SUPERSET_NAME, enc);

	if (cd == (iconv_t)(-1)) {
		if (errno == EINVAL) {
			return CRX_ICONV_ERR_WRONG_CHARSET;
		} else {
			return CRX_ICONV_ERR_CONVERTER;
		}
	}

	errno = 0;
	out_left = 0;
	more = nbytes > 0;

	for (in_p = str, in_left = nbytes, cnt = 0; more;) {
		out_p = buf;
		out_left = sizeof(buf);

		more = in_left > 0;

		iconv(cd, more ? (ICONV_CONST char **)&in_p : NULL, more ? &in_left : NULL, (char **) &out_p, &out_left);
		if (out_left == sizeof(buf)) {
			break;
		} else {
			CREX_ASSERT((sizeof(buf) - out_left) % GENERIC_SUPERSET_NBYTES == 0);
			cnt += (sizeof(buf) - out_left) / GENERIC_SUPERSET_NBYTES;
		}
	}

	switch (errno) {
		case EINVAL:
			err = CRX_ICONV_ERR_ILLEGAL_CHAR;
			break;

		case EILSEQ:
			err = CRX_ICONV_ERR_ILLEGAL_SEQ;
			break;

		case E2BIG:
		case 0:
			*pretval = cnt;
			break;

		default:
			err = CRX_ICONV_ERR_UNKNOWN;
			break;
	}

	iconv_close(cd);

	return err;
}

/* }}} */

/* {{{ _crx_iconv_substr() */
static crx_iconv_err_t _crx_iconv_substr(smart_str *pretval,
	const char *str, size_t nbytes, crex_long offset, crex_long len, const char *enc)
{
	char buf[GENERIC_SUPERSET_NBYTES];

	crx_iconv_err_t err = CRX_ICONV_ERR_SUCCESS;

	iconv_t cd1, cd2;

	const char *in_p;
	size_t in_left;

	char *out_p;
	size_t out_left;

	size_t cnt;
	size_t total_len;
	int more;

	err = _crx_iconv_strlen(&total_len, str, nbytes, enc);
	if (err != CRX_ICONV_ERR_SUCCESS) {
		return err;
	}

	if (offset < 0) {
		if ((offset += total_len) < 0) {
			offset = 0;
		}
	} else if ((size_t)offset > total_len) {
		offset = total_len;
	}

	if (len < 0) {
		if ((len += (total_len - offset)) < 0) {
			len = 0;
		}
	} else if ((size_t)len > total_len) {
		len = total_len;
	}

	if ((size_t)(offset + len) > total_len ) {
		/* trying to compute the length */
		len = total_len - offset;
	}

	if (len == 0) {
		smart_str_appendl(pretval, "", 0);
		smart_str_0(pretval);
		return CRX_ICONV_ERR_SUCCESS;
	}

	cd1 = iconv_open(GENERIC_SUPERSET_NAME, enc);

	if (cd1 == (iconv_t)(-1)) {
		if (errno == EINVAL) {
			return CRX_ICONV_ERR_WRONG_CHARSET;
		} else {
			return CRX_ICONV_ERR_CONVERTER;
		}
	}

	cd2 = (iconv_t)NULL;
	errno = 0;
	more = nbytes > 0 && len > 0;

	for (in_p = str, in_left = nbytes, cnt = 0; more; ++cnt) {
		out_p = buf;
		out_left = sizeof(buf);

		more = in_left > 0 && len > 0;

		iconv(cd1, more ? (ICONV_CONST char **)&in_p : NULL, more ? &in_left : NULL, (char **) &out_p, &out_left);
		if (out_left == sizeof(buf)) {
			break;
		}

		if ((crex_long)cnt >= offset) {
			if (cd2 == (iconv_t)NULL) {
				cd2 = iconv_open(enc, GENERIC_SUPERSET_NAME);

				if (cd2 == (iconv_t)(-1)) {
					cd2 = (iconv_t)NULL;
					if (errno == EINVAL) {
						err = CRX_ICONV_ERR_WRONG_CHARSET;
					} else {
						err = CRX_ICONV_ERR_CONVERTER;
					}
					break;
				}
			}

			if (_crx_iconv_appendl(pretval, buf, sizeof(buf), cd2) != CRX_ICONV_ERR_SUCCESS) {
				break;
			}
			--len;
		}

	}

	switch (errno) {
		case EINVAL:
			err = CRX_ICONV_ERR_ILLEGAL_CHAR;
			break;

		case EILSEQ:
			err = CRX_ICONV_ERR_ILLEGAL_SEQ;
			break;

		case E2BIG:
			break;
	}
	if (err == CRX_ICONV_ERR_SUCCESS) {
		if (cd2 != (iconv_t)NULL) {
			_crx_iconv_appendl(pretval, NULL, 0, cd2);
		}
		smart_str_0(pretval);
	}

	if (cd1 != (iconv_t)NULL) {
		iconv_close(cd1);
	}

	if (cd2 != (iconv_t)NULL) {
		iconv_close(cd2);
	}
	return err;
}

/* }}} */

/* {{{ _crx_iconv_strpos() */
static crx_iconv_err_t _crx_iconv_strpos(size_t *pretval,
	const char *haystk, size_t haystk_nbytes,
	const char *ndl, size_t ndl_nbytes,
	size_t offset, const char *enc, bool reverse)
{
	char buf[GENERIC_SUPERSET_NBYTES];

	crx_iconv_err_t err = CRX_ICONV_ERR_SUCCESS;

	iconv_t cd;

	const char *in_p;
	size_t in_left;

	char *out_p;
	size_t out_left;

	size_t cnt;

	crex_string *ndl_buf;
	const char *ndl_buf_p;
	size_t ndl_buf_left;

	size_t match_ofs;
	int more;
	size_t iconv_ret;

	*pretval = (size_t)-1;

	err = crx_iconv_string(ndl, ndl_nbytes, &ndl_buf, GENERIC_SUPERSET_NAME, enc);

	if (err != CRX_ICONV_ERR_SUCCESS) {
		if (ndl_buf != NULL) {
			crex_string_efree(ndl_buf);
		}
		return err;
	}

	cd = iconv_open(GENERIC_SUPERSET_NAME, enc);

	if (cd == (iconv_t)(-1)) {
		if (ndl_buf != NULL) {
			crex_string_efree(ndl_buf);
		}
		if (errno == EINVAL) {
			return CRX_ICONV_ERR_WRONG_CHARSET;
		} else {
			return CRX_ICONV_ERR_CONVERTER;
		}
	}

	ndl_buf_p = ZSTR_VAL(ndl_buf);
	ndl_buf_left = ZSTR_LEN(ndl_buf);
	match_ofs = (size_t)-1;
	more = haystk_nbytes > 0;

	for (in_p = haystk, in_left = haystk_nbytes, cnt = 0; more; ++cnt) {
		out_p = buf;
		out_left = sizeof(buf);

		more = in_left > 0;

		iconv_ret = iconv(cd, more ? (ICONV_CONST char **)&in_p : NULL, more ? &in_left : NULL, (char **) &out_p, &out_left);
		if (out_left == sizeof(buf)) {
			break;
		}
		if (iconv_ret == (size_t)-1) {
			switch (errno) {
				case EINVAL:
					err = CRX_ICONV_ERR_ILLEGAL_CHAR;
					break;

				case EILSEQ:
					err = CRX_ICONV_ERR_ILLEGAL_SEQ;
					break;

				case E2BIG:
					break;

				default:
					err = CRX_ICONV_ERR_UNKNOWN;
					break;
			}
		}
		if (cnt >= offset) {
			if (_crx_iconv_memequal(buf, ndl_buf_p, sizeof(buf))) {
				if (match_ofs == (size_t)-1) {
					match_ofs = cnt;
				}
				ndl_buf_p += GENERIC_SUPERSET_NBYTES;
				ndl_buf_left -= GENERIC_SUPERSET_NBYTES;
				if (ndl_buf_left == 0) {
					*pretval = match_ofs;
					if (reverse) {
						/* If searching backward, continue trying to find a later match. */
						ndl_buf_p = ZSTR_VAL(ndl_buf);
						ndl_buf_left = ZSTR_LEN(ndl_buf);
						match_ofs = -1;
					} else {
						/* If searching forward, stop at first match. */
						break;
					}
				}
			} else {
				size_t i, j, lim;

				i = 0;
				j = GENERIC_SUPERSET_NBYTES;
				lim = (size_t)(ndl_buf_p - ZSTR_VAL(ndl_buf));

				while (j < lim) {
					if (_crx_iconv_memequal(&ZSTR_VAL(ndl_buf)[j], &ZSTR_VAL(ndl_buf)[i],
							   GENERIC_SUPERSET_NBYTES)) {
						i += GENERIC_SUPERSET_NBYTES;
					} else {
						j -= i;
						i = 0;
					}
					j += GENERIC_SUPERSET_NBYTES;
				}

				if (_crx_iconv_memequal(buf, &ZSTR_VAL(ndl_buf)[i], sizeof(buf))) {
					match_ofs += (lim - i) / GENERIC_SUPERSET_NBYTES;
					i += GENERIC_SUPERSET_NBYTES;
					ndl_buf_p = &ZSTR_VAL(ndl_buf)[i];
					ndl_buf_left = ZSTR_LEN(ndl_buf) - i;
				} else {
					match_ofs = (size_t)-1;
					ndl_buf_p = ZSTR_VAL(ndl_buf);
					ndl_buf_left = ZSTR_LEN(ndl_buf);
				}
			}
		}
	}

	if (ndl_buf) {
		crex_string_efree(ndl_buf);
	}

	iconv_close(cd);

	if (err == CRX_ICONV_ERR_SUCCESS && offset > cnt) {
		return CRX_ICONV_ERR_OUT_BY_BOUNDS;
	}

	return err;
}
/* }}} */

/* {{{ _crx_iconv_mime_encode() */
static crx_iconv_err_t _crx_iconv_mime_encode(smart_str *pretval, const char *fname, size_t fname_nbytes, const char *fval, size_t fval_nbytes, size_t max_line_len, const char *lfchars, crx_iconv_enc_scheme_t enc_scheme, const char *out_charset, const char *enc)
{
	crx_iconv_err_t err = CRX_ICONV_ERR_SUCCESS;
	iconv_t cd = (iconv_t)(-1), cd_pl = (iconv_t)(-1);
	size_t char_cnt = 0;
	size_t out_charset_len;
	size_t lfchars_len;
	char *buf = NULL;
	const char *in_p;
	size_t in_left;
	char *out_p;
	size_t out_left;
	crex_string *encoded = NULL;
	static const int qp_table[256] = {
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0x00 */
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0x10 */
		3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x20 */
		1, 1, 1, 1, 1, 1, 1 ,1, 1, 1, 1, 1, 1, 3, 1, 3, /* 0x30 */
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x40 */
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, /* 0x50 */
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x60 */
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, /* 0x70 */
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0x80 */
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0x90 */
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0xA0 */
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0xB0 */
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0xC0 */
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0xD0 */
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0xE0 */
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3  /* 0xF0 */
	};

	out_charset_len = strlen(out_charset);
	lfchars_len = strlen(lfchars);

	if ((fname_nbytes + 2) >= max_line_len
		|| (out_charset_len + 12) >= max_line_len) {
		/* field name is too long */
		err = CRX_ICONV_ERR_TOO_BIG;
		goto out;
	}

	cd_pl = iconv_open(ICONV_ASCII_ENCODING, enc);
	if (cd_pl == (iconv_t)(-1)) {
		if (errno == EINVAL) {
			err = CRX_ICONV_ERR_WRONG_CHARSET;
		} else {
			err = CRX_ICONV_ERR_CONVERTER;
		}
		goto out;
	}

	cd = iconv_open(out_charset, enc);
	if (cd == (iconv_t)(-1)) {
		if (errno == EINVAL) {
			err = CRX_ICONV_ERR_WRONG_CHARSET;
		} else {
			err = CRX_ICONV_ERR_CONVERTER;
		}
		goto out;
	}

	buf = safe_emalloc(1, max_line_len, 5);

	char_cnt = max_line_len;

	_crx_iconv_appendl(pretval, fname, fname_nbytes, cd_pl);
	char_cnt -= fname_nbytes;
	smart_str_appendl(pretval, ": ", sizeof(": ") - 1);
	char_cnt -= 2;

	in_p = fval;
	in_left = fval_nbytes;

	do {
		size_t prev_in_left;
		size_t out_size;
		size_t encoded_word_min_len = sizeof("=\?\?X\?\?=")-1 + out_charset_len + (enc_scheme == CRX_ICONV_ENC_SCHEME_BASE64 ? 4 : 3);

		if (char_cnt < encoded_word_min_len + lfchars_len + 1) {
			/* lfchars must be encoded in ASCII here*/
			smart_str_appendl(pretval, lfchars, lfchars_len);
			smart_str_appendc(pretval, ' ');
			char_cnt = max_line_len - 1;
		}

		smart_str_appendl(pretval, "=?", sizeof("=?") - 1);
		char_cnt -= 2;
		smart_str_appendl(pretval, out_charset, out_charset_len);
		char_cnt -= out_charset_len;
		smart_str_appendc(pretval, '?');
		char_cnt --;

		switch (enc_scheme) {
			case CRX_ICONV_ENC_SCHEME_BASE64: {
				size_t ini_in_left;
				const char *ini_in_p;
				size_t out_reserved = 4;

				smart_str_appendc(pretval, 'B');
				char_cnt--;
				smart_str_appendc(pretval, '?');
				char_cnt--;

				prev_in_left = ini_in_left = in_left;
				ini_in_p = in_p;

				out_size = (char_cnt - 2) / 4 * 3;

				for (;;) {
					out_p = buf;

					if (out_size <= out_reserved) {
						err = CRX_ICONV_ERR_TOO_BIG;
						goto out;
					}

					out_left = out_size - out_reserved;

					if (iconv(cd, (ICONV_CONST char **)&in_p, &in_left, (char **) &out_p, &out_left) == (size_t)-1) {
						switch (errno) {
							case EINVAL:
								err = CRX_ICONV_ERR_ILLEGAL_CHAR;
								goto out;

							case EILSEQ:
								err = CRX_ICONV_ERR_ILLEGAL_SEQ;
								goto out;

							case E2BIG:
								if (prev_in_left == in_left) {
									err = CRX_ICONV_ERR_TOO_BIG;
									goto out;
								}
								break;

							default:
								err = CRX_ICONV_ERR_UNKNOWN;
								goto out;
						}
					}

					out_left += out_reserved;

					if (iconv(cd, NULL, NULL, (char **) &out_p, &out_left) == (size_t)-1) {
						if (errno != E2BIG) {
							err = CRX_ICONV_ERR_UNKNOWN;
							goto out;
						}
					} else {
						break;
					}

					if (iconv(cd, NULL, NULL, NULL, NULL) == (size_t)-1) {
						err = CRX_ICONV_ERR_UNKNOWN;
						goto out;
					}

					out_reserved += 4;
					in_left = ini_in_left;
					in_p = ini_in_p;
				}

				prev_in_left = in_left;

				encoded = crx_base64_encode((unsigned char *) buf, (out_size - out_left));

				if (char_cnt < ZSTR_LEN(encoded)) {
					/* something went wrong! */
					err = CRX_ICONV_ERR_UNKNOWN;
					goto out;
				}

				smart_str_appendl(pretval, ZSTR_VAL(encoded), ZSTR_LEN(encoded));
				char_cnt -= ZSTR_LEN(encoded);
				smart_str_appendl(pretval, "?=", sizeof("?=") - 1);
				char_cnt -= 2;

				crex_string_release_ex(encoded, 0);
				encoded = NULL;
			} break; /* case CRX_ICONV_ENC_SCHEME_BASE64: */

			case CRX_ICONV_ENC_SCHEME_QPRINT: {
				size_t ini_in_left;
				const char *ini_in_p;
				const unsigned char *p;
				size_t nbytes_required;

				smart_str_appendc(pretval, 'Q');
				char_cnt--;
				smart_str_appendc(pretval, '?');
				char_cnt--;

				prev_in_left = ini_in_left = in_left;
				ini_in_p = in_p;

				for (out_size = (char_cnt - 2); out_size > 0;) {

					nbytes_required = 0;

					out_p = buf;
					out_left = out_size;

					if (iconv(cd, (ICONV_CONST char **)&in_p, &in_left, (char **) &out_p, &out_left) == (size_t)-1) {
						switch (errno) {
							case EINVAL:
								err = CRX_ICONV_ERR_ILLEGAL_CHAR;
								goto out;

							case EILSEQ:
								err = CRX_ICONV_ERR_ILLEGAL_SEQ;
								goto out;

							case E2BIG:
								if (prev_in_left == in_left) {
									err = CRX_ICONV_ERR_UNKNOWN;
									goto out;
								}
								break;

							default:
								err = CRX_ICONV_ERR_UNKNOWN;
								goto out;
						}
					}
					if (iconv(cd, NULL, NULL, (char **) &out_p, &out_left) == (size_t)-1) {
						if (errno != E2BIG) {
							err = CRX_ICONV_ERR_UNKNOWN;
							goto out;
						}
					}

					for (p = (unsigned char *)buf; p < (unsigned char *)out_p; p++) {
						nbytes_required += qp_table[*p];
					}

					if (nbytes_required <= char_cnt - 2) {
						break;
					}

					out_size -= ((nbytes_required - (char_cnt - 2)) + 2) / 3;
					in_left = ini_in_left;
					in_p = ini_in_p;
				}

				for (p = (unsigned char *)buf; p < (unsigned char *)out_p; p++) {
					if (qp_table[*p] == 1) {
						smart_str_appendc(pretval, *(char *)p);
						char_cnt--;
					} else {
						static const char qp_digits[] = "0123456789ABCDEF";
						smart_str_appendc(pretval, '=');
						smart_str_appendc(pretval, qp_digits[(*p >> 4) & 0x0f]);
						smart_str_appendc(pretval, qp_digits[(*p & 0x0f)]);
						char_cnt -= 3;
					}
				}

				smart_str_appendl(pretval, "?=", sizeof("?=") - 1);
				char_cnt -= 2;

				if (iconv(cd, NULL, NULL, NULL, NULL) == (size_t)-1) {
					err = CRX_ICONV_ERR_UNKNOWN;
					goto out;
				}

			} break; /* case CRX_ICONV_ENC_SCHEME_QPRINT: */
		}
	} while (in_left > 0);

	smart_str_0(pretval);

out:
	if (cd != (iconv_t)(-1)) {
		iconv_close(cd);
	}
	if (cd_pl != (iconv_t)(-1)) {
		iconv_close(cd_pl);
	}
	if (encoded != NULL) {
		crex_string_release_ex(encoded, 0);
	}
	if (buf != NULL) {
		efree(buf);
	}
	return err;
}
/* }}} */

/* {{{ _crx_iconv_mime_decode() */
static crx_iconv_err_t _crx_iconv_mime_decode(smart_str *pretval, const char *str, size_t str_nbytes, const char *enc, const char **next_pos, int mode)
{
	crx_iconv_err_t err = CRX_ICONV_ERR_SUCCESS;

	iconv_t cd = (iconv_t)(-1), cd_pl = (iconv_t)(-1);

	const char *p1;
	size_t str_left;
	unsigned int scan_stat = 0;
	const char *csname = NULL;
	size_t csname_len;
	const char *encoded_text = NULL;
	size_t encoded_text_len = 0;
	const char *encoded_word = NULL;
	const char *spaces = NULL;

	crx_iconv_enc_scheme_t enc_scheme = CRX_ICONV_ENC_SCHEME_BASE64;

	if (next_pos != NULL) {
		*next_pos = NULL;
	}

	cd_pl = iconv_open(enc, ICONV_ASCII_ENCODING);

	if (cd_pl == (iconv_t)(-1)) {
		if (errno == EINVAL) {
			err = CRX_ICONV_ERR_WRONG_CHARSET;
		} else {
			err = CRX_ICONV_ERR_CONVERTER;
		}
		goto out;
	}

	p1 = str;
	for (str_left = str_nbytes; str_left > 0; str_left--, p1++) {
		int eos = 0;

		switch (scan_stat) {
			case 0: /* expecting any character */
				switch (*p1) {
					case '\r': /* part of an EOL sequence? */
						scan_stat = 7;
						break;

					case '\n':
						scan_stat = 8;
						break;

					case '=': /* first letter of an encoded chunk */
						encoded_word = p1;
						scan_stat = 1;
						break;

					case ' ': case '\t': /* a chunk of whitespaces */
						spaces = p1;
						scan_stat = 11;
						break;

					default: /* first letter of a non-encoded word */
						err = _crx_iconv_appendc(pretval, *p1, cd_pl);
						if (err != CRX_ICONV_ERR_SUCCESS) {
							if (mode & CRX_ICONV_MIME_DECODE_CONTINUE_ON_ERROR) {
								err = CRX_ICONV_ERR_SUCCESS;
							} else {
								goto out;
							}
						}
						encoded_word = NULL;
						if ((mode & CRX_ICONV_MIME_DECODE_STRICT)) {
							scan_stat = 12;
						}
						break;
				}
				break;

			case 1: /* expecting a delimiter */
				if (*p1 != '?') {
					if (*p1 == '\r' || *p1 == '\n') {
						--p1;
					}
					err = _crx_iconv_appendl(pretval, encoded_word, (size_t)((p1 + 1) - encoded_word), cd_pl);
					if (err != CRX_ICONV_ERR_SUCCESS) {
						goto out;
					}
					encoded_word = NULL;
					if ((mode & CRX_ICONV_MIME_DECODE_STRICT)) {
						scan_stat = 12;
					} else {
						scan_stat = 0;
					}
					break;
				}
				csname = p1 + 1;
				scan_stat = 2;
				break;

			case 2: /* expecting a charset name */
				switch (*p1) {
					case '?': /* normal delimiter: encoding scheme follows */
						scan_stat = 3;
						break;

					case '*': /* new style delimiter: locale id follows */
						scan_stat = 10;
						break;

					case '\r': case '\n': /* not an encoded-word */
						--p1;
						_crx_iconv_appendc(pretval, '=', cd_pl);
						_crx_iconv_appendc(pretval, '?', cd_pl);
						err = _crx_iconv_appendl(pretval, csname, (size_t)((p1 + 1) - csname), cd_pl);
						if (err != CRX_ICONV_ERR_SUCCESS) {
							goto out;
						}
						csname = NULL;
						if ((mode & CRX_ICONV_MIME_DECODE_STRICT)) {
							scan_stat = 12;
						}
						else {
							scan_stat = 0;
						}
						continue;
				}
				if (scan_stat != 2) {
					char tmpbuf[80];

					if (csname == NULL) {
						err = CRX_ICONV_ERR_MALFORMED;
						goto out;
					}

					csname_len = (size_t)(p1 - csname);

					if (csname_len > sizeof(tmpbuf) - 1) {
						if ((mode & CRX_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
							err = _crx_iconv_appendl(pretval, encoded_word, (size_t)((p1 + 1) - encoded_word), cd_pl);
							if (err != CRX_ICONV_ERR_SUCCESS) {
								goto out;
							}
							encoded_word = NULL;
							if ((mode & CRX_ICONV_MIME_DECODE_STRICT)) {
								scan_stat = 12;
							} else {
								scan_stat = 0;
							}
							break;
						} else {
							err = CRX_ICONV_ERR_MALFORMED;
							goto out;
						}
					}

					memcpy(tmpbuf, csname, csname_len);
					tmpbuf[csname_len] = '\0';

					if (cd != (iconv_t)(-1)) {
						iconv_close(cd);
					}

					cd = iconv_open(enc, tmpbuf);

					if (cd == (iconv_t)(-1)) {
						if ((mode & CRX_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
							/* Bad character set, but the user wants us to
							 * press on. In this case, we'll just insert the
							 * undecoded encoded word, since there isn't really
							 * a more sensible behaviour available; the only
							 * other options are to swallow the encoded word
							 * entirely or decode it with an arbitrarily chosen
							 * single byte encoding, both of which seem to have
							 * a higher WTF factor than leaving it undecoded.
							 *
							 * Given this approach, we need to skip ahead to
							 * the end of the encoded word. */
							int qmarks = 2;
							while (qmarks > 0 && str_left > 1) {
								if (*(++p1) == '?') {
									--qmarks;
								}
								--str_left;
							}

							/* Look ahead to check for the terminating = that
							 * should be there as well; if it's there, we'll
							 * also include that. If it's not, there isn't much
							 * we can do at this point. */
							if (*(p1 + 1) == '=') {
								++p1;
								if (str_left > 1) {
									--str_left;
								}
							}

							err = _crx_iconv_appendl(pretval, encoded_word, (size_t)((p1 + 1) - encoded_word), cd_pl);
							if (err != CRX_ICONV_ERR_SUCCESS) {
								goto out;
							}

							/* Let's go back and see if there are further
							 * encoded words or bare content, and hope they
							 * might actually have a valid character set. */
							scan_stat = 12;
							break;
						} else {
							if (errno == EINVAL) {
								err = CRX_ICONV_ERR_WRONG_CHARSET;
							} else {
								err = CRX_ICONV_ERR_CONVERTER;
							}
							goto out;
						}
					}
				}
				break;

			case 3: /* expecting a encoding scheme specifier */
				switch (*p1) {
					case 'b':
					case 'B':
						enc_scheme = CRX_ICONV_ENC_SCHEME_BASE64;
						scan_stat = 4;
						break;

					case 'q':
					case 'Q':
						enc_scheme = CRX_ICONV_ENC_SCHEME_QPRINT;
						scan_stat = 4;
						break;

					default:
						if ((mode & CRX_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
							err = _crx_iconv_appendl(pretval, encoded_word, (size_t)((p1 + 1) - encoded_word), cd_pl);
							if (err != CRX_ICONV_ERR_SUCCESS) {
								goto out;
							}
							encoded_word = NULL;
							if ((mode & CRX_ICONV_MIME_DECODE_STRICT)) {
								scan_stat = 12;
							} else {
								scan_stat = 0;
							}
							break;
						} else {
							err = CRX_ICONV_ERR_MALFORMED;
							goto out;
						}
				}
				break;

			case 4: /* expecting a delimiter */
				if (*p1 != '?') {
					if ((mode & CRX_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
						/* pass the entire chunk through the converter */
						err = _crx_iconv_appendl(pretval, encoded_word, (size_t)((p1 + 1) - encoded_word), cd_pl);
						if (err != CRX_ICONV_ERR_SUCCESS) {
							goto out;
						}
						encoded_word = NULL;
						if ((mode & CRX_ICONV_MIME_DECODE_STRICT)) {
							scan_stat = 12;
						} else {
							scan_stat = 0;
						}
						break;
					} else {
						err = CRX_ICONV_ERR_MALFORMED;
						goto out;
					}
				}
				encoded_text = p1 + 1;
				scan_stat = 5;
				break;

			case 5: /* expecting an encoded portion */
				if (*p1 == '?') {
					encoded_text_len = (size_t)(p1 - encoded_text);
					scan_stat = 6;
				}
				break;

			case 7: /* expecting a "\n" character */
				if (*p1 == '\n') {
					scan_stat = 8;
				} else {
					/* bare CR */
					_crx_iconv_appendc(pretval, '\r', cd_pl);
					_crx_iconv_appendc(pretval, *p1, cd_pl);
					scan_stat = 0;
				}
				break;

			case 8: /* checking whether the following line is part of a
					   folded header */
				if (*p1 != ' ' && *p1 != '\t') {
					--p1;
					str_left = 1; /* quit_loop */
					break;
				}
				if (encoded_word == NULL) {
					_crx_iconv_appendc(pretval, ' ', cd_pl);
				}
				spaces = NULL;
				scan_stat = 11;
				break;

			case 6: /* expecting a End-Of-Chunk character "=" */
				if (*p1 != '=') {
					if ((mode & CRX_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
						/* pass the entire chunk through the converter */
						err = _crx_iconv_appendl(pretval, encoded_word, (size_t)((p1 + 1) - encoded_word), cd_pl);
						if (err != CRX_ICONV_ERR_SUCCESS) {
							goto out;
						}
						encoded_word = NULL;
						if ((mode & CRX_ICONV_MIME_DECODE_STRICT)) {
							scan_stat = 12;
						} else {
							scan_stat = 0;
						}
						break;
					} else {
						err = CRX_ICONV_ERR_MALFORMED;
						goto out;
					}
				}
				scan_stat = 9;
				if (str_left == 1) {
					eos = 1;
				} else {
					break;
				}
				/* TODO might want to rearrange logic so this is more obvious */
				CREX_FALLTHROUGH;

			case 9: /* choice point, seeing what to do next.*/
				switch (*p1) {
					default:
						/* Handle non-RFC-compliant formats
						 *
						 * RFC2047 requires the character that comes right
						 * after an encoded word (chunk) to be a whitespace,
						 * while there are lots of broken implementations that
						 * generate such malformed headers that don't fulfill
						 * that requirement.
						 */
						if (!eos) {
							if ((mode & CRX_ICONV_MIME_DECODE_STRICT)) {
								/* pass the entire chunk through the converter */
								err = _crx_iconv_appendl(pretval, encoded_word, (size_t)((p1 + 1) - encoded_word), cd_pl);
								if (err != CRX_ICONV_ERR_SUCCESS) {
									goto out;
								}
								scan_stat = 12;
								break;
							}
						}
						CREX_FALLTHROUGH;

					case '\r': case '\n': case ' ': case '\t': {
						crex_string *decoded_text;

						switch (enc_scheme) {
							case CRX_ICONV_ENC_SCHEME_BASE64:
								decoded_text = crx_base64_decode((unsigned char*)encoded_text, encoded_text_len);
								break;

							case CRX_ICONV_ENC_SCHEME_QPRINT:
								decoded_text = crx_quot_print_decode((unsigned char*)encoded_text, encoded_text_len, 1);
								break;
							default:
								decoded_text = NULL;
								break;
						}

						if (decoded_text == NULL) {
							if ((mode & CRX_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
								/* pass the entire chunk through the converter */
								err = _crx_iconv_appendl(pretval, encoded_word, (size_t)((p1 + 1) - encoded_word), cd_pl);
								if (err != CRX_ICONV_ERR_SUCCESS) {
									goto out;
								}
								encoded_word = NULL;
								if ((mode & CRX_ICONV_MIME_DECODE_STRICT)) {
									scan_stat = 12;
								} else {
									scan_stat = 0;
								}
								break;
							} else {
								err = CRX_ICONV_ERR_UNKNOWN;
								goto out;
							}
						}

						err = _crx_iconv_appendl(pretval, ZSTR_VAL(decoded_text), ZSTR_LEN(decoded_text), cd);
						if (err == CRX_ICONV_ERR_SUCCESS) {
							err = _crx_iconv_appendl(pretval, NULL, 0, cd);
						}
						crex_string_release_ex(decoded_text, 0);

						if (err != CRX_ICONV_ERR_SUCCESS) {
							if ((mode & CRX_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
								/* pass the entire chunk through the converter */
								err = _crx_iconv_appendl(pretval, encoded_word, (size_t)(p1 - encoded_word), cd_pl);
								encoded_word = NULL;
								if (err != CRX_ICONV_ERR_SUCCESS) {
									break;
								}
							} else {
								goto out;
							}
						}

						if (eos) { /* reached end-of-string. done. */
							scan_stat = 0;
							break;
						}

						switch (*p1) {
							case '\r': /* part of an EOL sequence? */
								scan_stat = 7;
								break;

							case '\n':
								scan_stat = 8;
								break;

							case '=': /* first letter of an encoded chunk */
								scan_stat = 1;
								break;

							case ' ': case '\t': /* medial whitespaces */
								spaces = p1;
								scan_stat = 11;
								break;

							default: /* first letter of a non-encoded word */
								_crx_iconv_appendc(pretval, *p1, cd_pl);
								scan_stat = 12;
								break;
						}
					} break;
				}
				break;

			case 10: /* expects a language specifier. dismiss it for now */
				if (*p1 == '?') {
					scan_stat = 3;
				}
				break;

			case 11: /* expecting a chunk of whitespaces */
				switch (*p1) {
					case '\r': /* part of an EOL sequence? */
						scan_stat = 7;
						break;

					case '\n':
						scan_stat = 8;
						break;

					case '=': /* first letter of an encoded chunk */
						if (spaces != NULL && encoded_word == NULL) {
							_crx_iconv_appendl(pretval, spaces, (size_t)(p1 - spaces), cd_pl);
							spaces = NULL;
						}
						encoded_word = p1;
						scan_stat = 1;
						break;

					case ' ': case '\t':
						break;

					default: /* first letter of a non-encoded word */
						if (spaces != NULL) {
							_crx_iconv_appendl(pretval, spaces, (size_t)(p1 - spaces), cd_pl);
							spaces = NULL;
						}
						_crx_iconv_appendc(pretval, *p1, cd_pl);
						encoded_word = NULL;
						if ((mode & CRX_ICONV_MIME_DECODE_STRICT)) {
							scan_stat = 12;
						} else {
							scan_stat = 0;
						}
						break;
				}
				break;

			case 12: /* expecting a non-encoded word */
				switch (*p1) {
					case '\r': /* part of an EOL sequence? */
						scan_stat = 7;
						break;

					case '\n':
						scan_stat = 8;
						break;

					case ' ': case '\t':
						spaces = p1;
						scan_stat = 11;
						break;

					case '=': /* first letter of an encoded chunk */
						if (!(mode & CRX_ICONV_MIME_DECODE_STRICT)) {
							encoded_word = p1;
							scan_stat = 1;
							break;
						}
						CREX_FALLTHROUGH;

					default:
						_crx_iconv_appendc(pretval, *p1, cd_pl);
						break;
				}
				break;
		}
	}
	switch (scan_stat) {
		case 0: case 8: case 11: case 12:
			break;
		default:
			if ((mode & CRX_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
				if (scan_stat == 1) {
					_crx_iconv_appendc(pretval, '=', cd_pl);
				}
				err = CRX_ICONV_ERR_SUCCESS;
			} else {
				err = CRX_ICONV_ERR_MALFORMED;
				goto out;
			}
	}

	if (next_pos != NULL) {
		*next_pos = p1;
	}

	smart_str_0(pretval);
out:
	if (cd != (iconv_t)(-1)) {
		iconv_close(cd);
	}
	if (cd_pl != (iconv_t)(-1)) {
		iconv_close(cd_pl);
	}
	return err;
}
/* }}} */

/* {{{ crx_iconv_show_error() */
static void _crx_iconv_show_error(crx_iconv_err_t err, const char *out_charset, const char *in_charset)
{
	switch (err) {
		case CRX_ICONV_ERR_SUCCESS:
			break;

		case CRX_ICONV_ERR_CONVERTER:
			crx_error_docref(NULL, E_WARNING, "Cannot open converter");
			break;

		case CRX_ICONV_ERR_WRONG_CHARSET:
			crx_error_docref(NULL, E_WARNING, "Wrong encoding, conversion from \"%s\" to \"%s\" is not allowed",
			          in_charset, out_charset);
			break;

		case CRX_ICONV_ERR_ILLEGAL_CHAR:
			crx_error_docref(NULL, E_NOTICE, "Detected an incomplete multibyte character in input string");
			break;

		case CRX_ICONV_ERR_ILLEGAL_SEQ:
			crx_error_docref(NULL, E_NOTICE, "Detected an illegal character in input string");
			break;

		case CRX_ICONV_ERR_TOO_BIG:
			/* should not happen */
			crx_error_docref(NULL, E_WARNING, "Buffer length exceeded");
			break;

		case CRX_ICONV_ERR_MALFORMED:
			crx_error_docref(NULL, E_WARNING, "Malformed string");
			break;

		case CRX_ICONV_ERR_OUT_BY_BOUNDS:
			crex_argument_value_error(3, "must be contained in argument #1 ($haystack)");
			break;

		default:
			/* other error */
			crx_error_docref(NULL, E_NOTICE, "Unknown error (%d)", errno);
			break;
	}
}
/* }}} */

/* {{{ Returns the character count of str */
CRX_FUNCTION(iconv_strlen)
{
	const char *charset = NULL;
	size_t charset_len;
	crex_string *str;

	crx_iconv_err_t err;

	size_t retval;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S|s!",
		&str, &charset, &charset_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (charset == NULL) {
	 	charset = get_internal_encoding();
	} else if (charset_len >= ICONV_CSNMAXLEN) {
		crx_error_docref(NULL, E_WARNING, "Encoding parameter exceeds the maximum allowed length of %d characters", ICONV_CSNMAXLEN);
		RETURN_FALSE;
	}

	err = _crx_iconv_strlen(&retval, ZSTR_VAL(str), ZSTR_LEN(str), charset);
	_crx_iconv_show_error(err, GENERIC_SUPERSET_NAME, charset);
	if (err == CRX_ICONV_ERR_SUCCESS) {
		RETVAL_LONG(retval);
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ Returns specified part of a string */
CRX_FUNCTION(iconv_substr)
{
	const char *charset = NULL;
	size_t charset_len;
	crex_string *str;
	crex_long offset, length = 0;
	bool len_is_null = 1;

	crx_iconv_err_t err;

	smart_str retval = {0};

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Sl|l!s!",
		&str, &offset, &length, &len_is_null,
		&charset, &charset_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (charset == NULL) {
	 	charset = get_internal_encoding();
	} else if (charset_len >= ICONV_CSNMAXLEN) {
		crx_error_docref(NULL, E_WARNING, "Encoding parameter exceeds the maximum allowed length of %d characters", ICONV_CSNMAXLEN);
		RETURN_FALSE;
	}

	if (len_is_null) {
		length = ZSTR_LEN(str);
	}

	err = _crx_iconv_substr(&retval, ZSTR_VAL(str), ZSTR_LEN(str), offset, length, charset);
	_crx_iconv_show_error(err, GENERIC_SUPERSET_NAME, charset);

	if (err == CRX_ICONV_ERR_SUCCESS && retval.s != NULL) {
		RETURN_STR(smart_str_extract(&retval));
	}
	smart_str_free(&retval);
	RETURN_FALSE;
}
/* }}} */

/* {{{ Finds position of first occurrence of needle within part of haystack beginning with offset */
CRX_FUNCTION(iconv_strpos)
{
	const char *charset = NULL;
	size_t charset_len, haystk_len;
	crex_string *haystk;
	crex_string *ndl;
	crex_long offset = 0;

	crx_iconv_err_t err;

	size_t retval;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "SS|ls!",
		&haystk, &ndl,
		&offset, &charset, &charset_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (charset == NULL) {
	 	charset = get_internal_encoding();
	} else if (charset_len >= ICONV_CSNMAXLEN) {
		crx_error_docref(NULL, E_WARNING, "Encoding parameter exceeds the maximum allowed length of %d characters", ICONV_CSNMAXLEN);
		RETURN_FALSE;
	}

	if (offset < 0) {
		/* Convert negative offset (counted from the end of string) */
		err = _crx_iconv_strlen(&haystk_len, ZSTR_VAL(haystk), ZSTR_LEN(haystk), charset);
		if (err != CRX_ICONV_ERR_SUCCESS) {
			_crx_iconv_show_error(err, GENERIC_SUPERSET_NAME, charset);
			RETURN_FALSE;
		}
		offset += haystk_len;
		if (offset < 0) { /* If offset before start */
			crex_argument_value_error(3, "must be contained in argument #1 ($haystack)");
			RETURN_THROWS();
		}
	}

	if (ZSTR_LEN(ndl) < 1) {
		// TODO: Support empty needles!
		RETURN_FALSE;
	}

	err = _crx_iconv_strpos(
		&retval, ZSTR_VAL(haystk), ZSTR_LEN(haystk), ZSTR_VAL(ndl), ZSTR_LEN(ndl),
		offset, charset, /* reverse */ false);
	_crx_iconv_show_error(err, GENERIC_SUPERSET_NAME, charset);

	if (err == CRX_ICONV_ERR_SUCCESS && retval != (size_t)-1) {
		RETVAL_LONG((crex_long)retval);
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ Finds position of last occurrence of needle within part of haystack beginning with offset */
CRX_FUNCTION(iconv_strrpos)
{
	const char *charset = NULL;
	size_t charset_len;
	crex_string *haystk;
	crex_string *ndl;

	crx_iconv_err_t err;

	size_t retval;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "SS|s!",
		&haystk, &ndl,
		&charset, &charset_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (ZSTR_LEN(ndl) < 1) {
		RETURN_FALSE;
	}

	if (charset == NULL) {
	 	charset = get_internal_encoding();
	} else if (charset_len >= ICONV_CSNMAXLEN) {
		crx_error_docref(NULL, E_WARNING, "Encoding parameter exceeds the maximum allowed length of %d characters", ICONV_CSNMAXLEN);
		RETURN_FALSE;
	}

	err = _crx_iconv_strpos(
		&retval, ZSTR_VAL(haystk), ZSTR_LEN(haystk), ZSTR_VAL(ndl), ZSTR_LEN(ndl),
		/* offset */ 0, charset, /* reserve */ true);
	_crx_iconv_show_error(err, GENERIC_SUPERSET_NAME, charset);

	if (err == CRX_ICONV_ERR_SUCCESS && retval != (size_t)-1) {
		RETVAL_LONG((crex_long)retval);
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ Composes a mime header field with field_name and field_value in a specified scheme */
CRX_FUNCTION(iconv_mime_encode)
{
	crex_string *field_name = NULL;
	crex_string *field_value = NULL;
	crex_string *tmp_str = NULL;
	zval *pref = NULL;
	smart_str retval = {0};
	crx_iconv_err_t err;

	const char *in_charset = get_internal_encoding();
	const char *out_charset = in_charset;
	crex_long line_len = 76;
	const char *lfchars = "\r\n";
	crx_iconv_enc_scheme_t scheme_id = CRX_ICONV_ENC_SCHEME_BASE64;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "SS|a",
		&field_name, &field_value,
		&pref) == FAILURE) {

		RETURN_THROWS();
	}

	if (pref != NULL) {
		zval *pzval;

		if ((pzval = crex_hash_find_deref(C_ARRVAL_P(pref), ZSTR_KNOWN(CREX_STR_SCHEME))) != NULL) {
			if (C_TYPE_P(pzval) == IS_STRING && C_STRLEN_P(pzval) > 0) {
				switch (C_STRVAL_P(pzval)[0]) {
					case 'B': case 'b':
						scheme_id = CRX_ICONV_ENC_SCHEME_BASE64;
						break;

					case 'Q': case 'q':
						scheme_id = CRX_ICONV_ENC_SCHEME_QPRINT;
						break;
				}
			}
		}

		if ((pzval = crex_hash_str_find_deref(C_ARRVAL_P(pref), "input-charset", sizeof("input-charset") - 1)) != NULL && C_TYPE_P(pzval) == IS_STRING) {
			if (C_STRLEN_P(pzval) >= ICONV_CSNMAXLEN) {
				crx_error_docref(NULL, E_WARNING, "Encoding parameter exceeds the maximum allowed length of %d characters", ICONV_CSNMAXLEN);
				RETURN_FALSE;
			}

			if (C_STRLEN_P(pzval) > 0) {
				in_charset = C_STRVAL_P(pzval);
			}
		}


		if ((pzval = crex_hash_str_find_deref(C_ARRVAL_P(pref), "output-charset", sizeof("output-charset") - 1)) != NULL && C_TYPE_P(pzval) == IS_STRING) {
			if (C_STRLEN_P(pzval) >= ICONV_CSNMAXLEN) {
				crx_error_docref(NULL, E_WARNING, "Encoding parameter exceeds the maximum allowed length of %d characters", ICONV_CSNMAXLEN);
				RETURN_FALSE;
			}

			if (C_STRLEN_P(pzval) > 0) {
				out_charset = C_STRVAL_P(pzval);
			}
		}

		if ((pzval = crex_hash_str_find_deref(C_ARRVAL_P(pref), "line-length", sizeof("line-length") - 1)) != NULL) {
			line_len = zval_get_long(pzval);
		}

		if ((pzval = crex_hash_str_find_deref(C_ARRVAL_P(pref), "line-break-chars", sizeof("line-break-chars") - 1)) != NULL) {
			if (C_TYPE_P(pzval) != IS_STRING) {
				tmp_str = zval_try_get_string_func(pzval);
				if (UNEXPECTED(!tmp_str)) {
					return;
				}
				lfchars = ZSTR_VAL(tmp_str);
			} else {
				lfchars = C_STRVAL_P(pzval);
			}
		}
	}

	err = _crx_iconv_mime_encode(&retval, ZSTR_VAL(field_name), ZSTR_LEN(field_name),
		ZSTR_VAL(field_value), ZSTR_LEN(field_value), line_len, lfchars, scheme_id,
		out_charset, in_charset);
	_crx_iconv_show_error(err, out_charset, in_charset);

	if (err == CRX_ICONV_ERR_SUCCESS) {
		RETVAL_STR(smart_str_extract(&retval));
	} else {
		smart_str_free(&retval);
		RETVAL_FALSE;
	}

	if (tmp_str) {
		crex_string_release_ex(tmp_str, 0);
	}
}
/* }}} */

/* {{{ Decodes a mime header field */
CRX_FUNCTION(iconv_mime_decode)
{
	crex_string *encoded_str;
	const char *charset = NULL;
	size_t charset_len;
	crex_long mode = 0;

	smart_str retval = {0};

	crx_iconv_err_t err;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S|ls!",
		&encoded_str, &mode, &charset, &charset_len) == FAILURE) {

		RETURN_THROWS();
	}

	if (charset == NULL) {
	 	charset = get_internal_encoding();
	} else if (charset_len >= ICONV_CSNMAXLEN) {
		crx_error_docref(NULL, E_WARNING, "Encoding parameter exceeds the maximum allowed length of %d characters", ICONV_CSNMAXLEN);
		RETURN_FALSE;
	}

	err = _crx_iconv_mime_decode(&retval, ZSTR_VAL(encoded_str), ZSTR_LEN(encoded_str), charset, NULL, (int)mode);
	_crx_iconv_show_error(err, charset, "???");

	if (err == CRX_ICONV_ERR_SUCCESS) {
		RETVAL_STR(smart_str_extract(&retval));
	} else {
		smart_str_free(&retval);
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ Decodes multiple mime header fields */
CRX_FUNCTION(iconv_mime_decode_headers)
{
	crex_string *encoded_str;
	const char *charset = NULL;
	size_t charset_len;
	crex_long mode = 0;
	char *enc_str_tmp;
	size_t enc_str_len_tmp;

	crx_iconv_err_t err = CRX_ICONV_ERR_SUCCESS;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S|ls!",
		&encoded_str, &mode, &charset, &charset_len) == FAILURE) {

		RETURN_THROWS();
	}

	if (charset == NULL) {
	 	charset = get_internal_encoding();
	} else if (charset_len >= ICONV_CSNMAXLEN) {
		crx_error_docref(NULL, E_WARNING, "Encoding parameter exceeds the maximum allowed length of %d characters", ICONV_CSNMAXLEN);
		RETURN_FALSE;
	}

	array_init(return_value);

	enc_str_tmp = ZSTR_VAL(encoded_str);
	enc_str_len_tmp = ZSTR_LEN(encoded_str);
	while (enc_str_len_tmp > 0) {
		smart_str decoded_header = {0};
		char *header_name = NULL;
		size_t header_name_len = 0;
		char *header_value = NULL;
		size_t header_value_len = 0;
		char *p, *limit;
		const char *next_pos;

		if (CRX_ICONV_ERR_SUCCESS != (err = _crx_iconv_mime_decode(&decoded_header, enc_str_tmp, enc_str_len_tmp, charset, &next_pos, (int)mode))) {
			smart_str_free(&decoded_header);
			break;
		}

		if (decoded_header.s == NULL) {
			break;
		}

		limit = ZSTR_VAL(decoded_header.s) + ZSTR_LEN(decoded_header.s);
		for (p = ZSTR_VAL(decoded_header.s); p < limit; p++) {
			if (*p == ':') {
				*p = '\0';
				header_name = ZSTR_VAL(decoded_header.s);
				header_name_len = p - ZSTR_VAL(decoded_header.s);

				while (++p < limit) {
					if (*p != ' ' && *p != '\t') {
						break;
					}
				}

				header_value = p;
				header_value_len = limit - p;

				break;
			}
		}

		if (header_name != NULL) {
			zval *elem;

			if ((elem = crex_hash_str_find(C_ARRVAL_P(return_value), header_name, header_name_len)) != NULL) {
				if (C_TYPE_P(elem) != IS_ARRAY) {
					zval new_elem;

					array_init(&new_elem);
					C_ADDREF_P(elem);
					add_next_index_zval(&new_elem, elem);

					elem = crex_hash_str_update(C_ARRVAL_P(return_value), header_name, header_name_len, &new_elem);
				}
				add_next_index_stringl(elem, header_value, header_value_len);
			} else {
				add_assoc_stringl_ex(return_value, header_name, header_name_len, header_value, header_value_len);
			}
		}
		enc_str_len_tmp -= next_pos - enc_str_tmp;
		enc_str_tmp = (char *)next_pos;

		smart_str_free(&decoded_header);
	}

	if (err != CRX_ICONV_ERR_SUCCESS) {
		_crx_iconv_show_error(err, charset, "???");
		crex_array_destroy(C_ARR_P(return_value));
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ Returns str converted to the out_charset character set */
CRX_FUNCTION(iconv)
{
	char *in_charset, *out_charset;
	crex_string *in_buffer;
	size_t in_charset_len = 0, out_charset_len = 0;
	crx_iconv_err_t err;
	crex_string *out_buffer;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ssS",
		&in_charset, &in_charset_len, &out_charset, &out_charset_len, &in_buffer) == FAILURE) {
		RETURN_THROWS();
	}

	if (in_charset_len >= ICONV_CSNMAXLEN || out_charset_len >= ICONV_CSNMAXLEN) {
		crx_error_docref(NULL, E_WARNING, "Encoding parameter exceeds the maximum allowed length of %d characters", ICONV_CSNMAXLEN);
		RETURN_FALSE;
	}

	err = crx_iconv_string(ZSTR_VAL(in_buffer), (size_t)ZSTR_LEN(in_buffer), &out_buffer, out_charset, in_charset);
	_crx_iconv_show_error(err, out_charset, in_charset);
	if (err == CRX_ICONV_ERR_SUCCESS && out_buffer != NULL) {
		RETVAL_NEW_STR(out_buffer);
	} else {
		if (out_buffer != NULL) {
			crex_string_efree(out_buffer);
		}
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Sets internal encoding and output encoding for ob_iconv_handler() */
CRX_FUNCTION(iconv_set_encoding)
{
	crex_string *type;
	crex_string *charset;
	crex_result retval;
	crex_string *name;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "SS", &type, &charset) == FAILURE) {
		RETURN_THROWS();
	}

	if (ZSTR_LEN(charset) >= ICONV_CSNMAXLEN) {
		crx_error_docref(NULL, E_WARNING, "Encoding parameter exceeds the maximum allowed length of %d characters", ICONV_CSNMAXLEN);
		RETURN_FALSE;
	}

	if(crex_string_equals_literal_ci(type, "input_encoding")) {
		name = ZSTR_INIT_LITERAL("iconv.input_encoding", 0);
	} else if(crex_string_equals_literal_ci(type, "output_encoding")) {
		name = ZSTR_INIT_LITERAL("iconv.output_encoding", 0);
	} else if(crex_string_equals_literal_ci(type, "internal_encoding")) {
		name = ZSTR_INIT_LITERAL("iconv.internal_encoding", 0);
	} else {
		RETURN_FALSE;
	}

	retval = crex_alter_ini_entry(name, charset, CRX_INI_USER, CRX_INI_STAGE_RUNTIME);
	crex_string_release_ex(name, 0);

	if (retval == SUCCESS) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Get internal encoding and output encoding for ob_iconv_handler() */
CRX_FUNCTION(iconv_get_encoding)
{
	crex_string *type = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|S", &type) == FAILURE) {
		RETURN_THROWS();
	}

	if (!type || crex_string_equals_literal_ci(type, "all")) {
		array_init(return_value);
		add_assoc_string(return_value, "input_encoding",    get_input_encoding());
		add_assoc_string(return_value, "output_encoding",   get_output_encoding());
		add_assoc_string(return_value, "internal_encoding", get_internal_encoding());
	} else if (crex_string_equals_literal_ci(type, "input_encoding")) {
		RETVAL_STRING(get_input_encoding());
	} else if (crex_string_equals_literal_ci(type, "output_encoding")) {
		RETVAL_STRING(get_output_encoding());
	} else if (crex_string_equals_literal_ci(type, "internal_encoding")) {
		RETVAL_STRING(get_internal_encoding());
	} else {
		/* TODO Warning/ValueError? */
		RETURN_FALSE;
	}

}
/* }}} */

/* {{{ iconv stream filter */
typedef struct _crx_iconv_stream_filter {
	iconv_t cd;
	int persistent;
	char *to_charset;
	size_t to_charset_len;
	char *from_charset;
	size_t from_charset_len;
	char stub[128];
	size_t stub_len;
} crx_iconv_stream_filter;
/* }}} iconv stream filter */

/* {{{ crx_iconv_stream_filter_dtor */
static void crx_iconv_stream_filter_dtor(crx_iconv_stream_filter *self)
{
	iconv_close(self->cd);
	pefree(self->to_charset, self->persistent);
	pefree(self->from_charset, self->persistent);
}
/* }}} */

/* {{{ crx_iconv_stream_filter_ctor() */
static crx_iconv_err_t crx_iconv_stream_filter_ctor(crx_iconv_stream_filter *self,
		const char *to_charset, size_t to_charset_len,
		const char *from_charset, size_t from_charset_len, int persistent)
{
	self->to_charset = pemalloc(to_charset_len + 1, persistent);
	self->to_charset_len = to_charset_len;
	self->from_charset = pemalloc(from_charset_len + 1, persistent);
	self->from_charset_len = from_charset_len;

	memcpy(self->to_charset, to_charset, to_charset_len);
	self->to_charset[to_charset_len] = '\0';
	memcpy(self->from_charset, from_charset, from_charset_len);
	self->from_charset[from_charset_len] = '\0';

	if ((iconv_t)-1 == (self->cd = iconv_open(self->to_charset, self->from_charset))) {
		pefree(self->from_charset, persistent);
		pefree(self->to_charset, persistent);
		return CRX_ICONV_ERR_UNKNOWN;
	}
	self->persistent = persistent;
	self->stub_len = 0;
	return CRX_ICONV_ERR_SUCCESS;
}
/* }}} */

/* {{{ crx_iconv_stream_filter_append_bucket */
static int crx_iconv_stream_filter_append_bucket(
		crx_iconv_stream_filter *self,
		crx_stream *stream, crx_stream_filter *filter,
		crx_stream_bucket_brigade *buckets_out,
		const char *ps, size_t buf_len, size_t *consumed,
		int persistent)
{
	crx_stream_bucket *new_bucket;
	char *out_buf = NULL;
	size_t out_buf_size;
	char *pd, *pt;
	size_t ocnt, prev_ocnt, icnt, tcnt;
	size_t initial_out_buf_size;

	if (ps == NULL) {
		initial_out_buf_size = 64;
		icnt = 1;
	} else {
		initial_out_buf_size = buf_len;
		icnt = buf_len;
	}

	out_buf_size = ocnt = prev_ocnt = initial_out_buf_size;
	out_buf = pemalloc(out_buf_size, persistent);

	pd = out_buf;

	if (self->stub_len > 0) {
		pt = self->stub;
		tcnt = self->stub_len;

		while (tcnt > 0) {
			if (iconv(self->cd, (ICONV_CONST char **)&pt, &tcnt, &pd, &ocnt) == (size_t)-1) {
				switch (errno) {
					case EILSEQ:
						crx_error_docref(NULL, E_WARNING, "iconv stream filter (\"%s\"=>\"%s\"): invalid multibyte sequence", self->from_charset, self->to_charset);
						goto out_failure;

					case EINVAL:
						if (ps != NULL) {
							if (icnt > 0) {
								if (self->stub_len >= sizeof(self->stub)) {
									crx_error_docref(NULL, E_WARNING, "iconv stream filter (\"%s\"=>\"%s\"): insufficient buffer", self->from_charset, self->to_charset);
									goto out_failure;
								}
								self->stub[self->stub_len++] = *(ps++);
								icnt--;
								pt = self->stub;
								tcnt = self->stub_len;
							} else {
								tcnt = 0;
								break;
							}
						} else {
						    crx_error_docref(NULL, E_WARNING, "iconv stream filter (\"%s\"=>\"%s\"): invalid multibyte sequence", self->from_charset, self->to_charset);
						    goto out_failure;
						}
						break;

					case E2BIG: {
						char *new_out_buf;
						size_t new_out_buf_size;

						new_out_buf_size = out_buf_size << 1;

						if (new_out_buf_size < out_buf_size) {
							/* whoa! no bigger buckets are sold anywhere... */
							if (NULL == (new_bucket = crx_stream_bucket_new(stream, out_buf, (out_buf_size - ocnt), 1, persistent))) {
								goto out_failure;
							}

							crx_stream_bucket_append(buckets_out, new_bucket);

							out_buf_size = ocnt = initial_out_buf_size;
							out_buf = pemalloc(out_buf_size, persistent);
							pd = out_buf;
						} else {
							new_out_buf = perealloc(out_buf, new_out_buf_size, persistent);
							pd = new_out_buf + (pd - out_buf);
							ocnt += (new_out_buf_size - out_buf_size);
							out_buf = new_out_buf;
							out_buf_size = new_out_buf_size;
						}
					} break;

					default:
						crx_error_docref(NULL, E_WARNING, "iconv stream filter (\"%s\"=>\"%s\"): unknown error", self->from_charset, self->to_charset);
						goto out_failure;
				}
			}
			prev_ocnt = ocnt;
		}
		memmove(self->stub, pt, tcnt);
		self->stub_len = tcnt;
	}

	while (icnt > 0) {
		if ((ps == NULL ? iconv(self->cd, NULL, NULL, &pd, &ocnt):
					iconv(self->cd, (ICONV_CONST char **)&ps, &icnt, &pd, &ocnt)) == (size_t)-1) {
			switch (errno) {
				case EILSEQ:
					crx_error_docref(NULL, E_WARNING, "iconv stream filter (\"%s\"=>\"%s\"): invalid multibyte sequence", self->from_charset, self->to_charset);
					goto out_failure;

				case EINVAL:
					if (ps != NULL) {
						if (icnt > sizeof(self->stub)) {
							crx_error_docref(NULL, E_WARNING, "iconv stream filter (\"%s\"=>\"%s\"): insufficient buffer", self->from_charset, self->to_charset);
							goto out_failure;
						}
						memcpy(self->stub, ps, icnt);
						self->stub_len = icnt;
						ps += icnt;
						icnt = 0;
					} else {
						crx_error_docref(NULL, E_WARNING, "iconv stream filter (\"%s\"=>\"%s\"): unexpected octet values", self->from_charset, self->to_charset);
						goto out_failure;
					}
					break;

				case E2BIG: {
					char *new_out_buf;
					size_t new_out_buf_size;

					new_out_buf_size = out_buf_size << 1;

					if (new_out_buf_size < out_buf_size) {
						/* whoa! no bigger buckets are sold anywhere... */
						if (NULL == (new_bucket = crx_stream_bucket_new(stream, out_buf, (out_buf_size - ocnt), 1, persistent))) {
							goto out_failure;
						}

						crx_stream_bucket_append(buckets_out, new_bucket);

						out_buf_size = ocnt = initial_out_buf_size;
						out_buf = pemalloc(out_buf_size, persistent);
						pd = out_buf;
					} else {
						new_out_buf = perealloc(out_buf, new_out_buf_size, persistent);
						pd = new_out_buf + (pd - out_buf);
						ocnt += (new_out_buf_size - out_buf_size);
						out_buf = new_out_buf;
						out_buf_size = new_out_buf_size;
					}
				} break;

				default:
					crx_error_docref(NULL, E_WARNING, "iconv stream filter (\"%s\"=>\"%s\"): unknown error", self->from_charset, self->to_charset);
					goto out_failure;
			}
		} else {
			if (ps == NULL) {
				break;
			}
		}
		prev_ocnt = ocnt;
	}

	if (out_buf_size > ocnt) {
		if (NULL == (new_bucket = crx_stream_bucket_new(stream, out_buf, (out_buf_size - ocnt), 1, persistent))) {
			goto out_failure;
		}
		crx_stream_bucket_append(buckets_out, new_bucket);
	} else {
		pefree(out_buf, persistent);
	}
	*consumed += buf_len - icnt;

	return SUCCESS;

out_failure:
	pefree(out_buf, persistent);
	return FAILURE;
}
/* }}} crx_iconv_stream_filter_append_bucket */

/* {{{ crx_iconv_stream_filter_do_filter */
static crx_stream_filter_status_t crx_iconv_stream_filter_do_filter(
		crx_stream *stream, crx_stream_filter *filter,
		crx_stream_bucket_brigade *buckets_in,
		crx_stream_bucket_brigade *buckets_out,
		size_t *bytes_consumed, int flags)
{
	crx_stream_bucket *bucket = NULL;
	size_t consumed = 0;
	crx_iconv_stream_filter *self = (crx_iconv_stream_filter *)C_PTR(filter->abstract);

	while (buckets_in->head != NULL) {
		bucket = buckets_in->head;

		crx_stream_bucket_unlink(bucket);

		if (crx_iconv_stream_filter_append_bucket(self, stream, filter,
				buckets_out, bucket->buf, bucket->buflen, &consumed,
				crx_stream_is_persistent(stream)) != SUCCESS) {
			goto out_failure;
		}

		crx_stream_bucket_delref(bucket);
	}

	if (flags != PSFS_FLAG_NORMAL) {
		if (crx_iconv_stream_filter_append_bucket(self, stream, filter,
				buckets_out, NULL, 0, &consumed,
				crx_stream_is_persistent(stream)) != SUCCESS) {
			goto out_failure;
		}
	}

	if (bytes_consumed != NULL) {
		*bytes_consumed = consumed;
	}

	return PSFS_PASS_ON;

out_failure:
	if (bucket != NULL) {
		crx_stream_bucket_delref(bucket);
	}
	return PSFS_ERR_FATAL;
}
/* }}} */

/* {{{ crx_iconv_stream_filter_cleanup */
static void crx_iconv_stream_filter_cleanup(crx_stream_filter *filter)
{
	crx_iconv_stream_filter_dtor((crx_iconv_stream_filter *)C_PTR(filter->abstract));
	pefree(C_PTR(filter->abstract), ((crx_iconv_stream_filter *)C_PTR(filter->abstract))->persistent);
}
/* }}} */

static const crx_stream_filter_ops crx_iconv_stream_filter_ops = {
	crx_iconv_stream_filter_do_filter,
	crx_iconv_stream_filter_cleanup,
	"convert.iconv.*"
};

/* {{{ crx_iconv_stream_filter_create */
static crx_stream_filter *crx_iconv_stream_filter_factory_create(const char *name, zval *params, uint8_t persistent)
{
	crx_stream_filter *retval = NULL;
	crx_iconv_stream_filter *inst;
	char *from_charset = NULL, *to_charset = NULL;
	size_t from_charset_len, to_charset_len;

	if ((from_charset = strchr(name, '.')) == NULL) {
		return NULL;
	}
	++from_charset;
	if ((from_charset = strchr(from_charset, '.')) == NULL) {
		return NULL;
	}
	++from_charset;
	if ((to_charset = strpbrk(from_charset, "/.")) == NULL) {
		return NULL;
	}
	from_charset_len = to_charset - from_charset;
	++to_charset;
	to_charset_len = strlen(to_charset);

	if (from_charset_len >= ICONV_CSNMAXLEN || to_charset_len >= ICONV_CSNMAXLEN) {
		return NULL;
	}

	inst = pemalloc(sizeof(crx_iconv_stream_filter), persistent);

	if (crx_iconv_stream_filter_ctor(inst, to_charset, to_charset_len, from_charset, from_charset_len, persistent) != CRX_ICONV_ERR_SUCCESS) {
		pefree(inst, persistent);
		return NULL;
	}

	if (NULL == (retval = crx_stream_filter_alloc(&crx_iconv_stream_filter_ops, inst, persistent))) {
		crx_iconv_stream_filter_dtor(inst);
		pefree(inst, persistent);
	}

	return retval;
}
/* }}} */

/* {{{ crx_iconv_stream_register_factory */
static crx_iconv_err_t crx_iconv_stream_filter_register_factory(void)
{
	static const crx_stream_filter_factory filter_factory = {
		crx_iconv_stream_filter_factory_create
	};

	if (FAILURE == crx_stream_filter_register_factory(
				crx_iconv_stream_filter_ops.label,
				&filter_factory)) {
		return CRX_ICONV_ERR_UNKNOWN;
	}
	return CRX_ICONV_ERR_SUCCESS;
}
/* }}} */

/* {{{ crx_iconv_stream_unregister_factory */
static crx_iconv_err_t crx_iconv_stream_filter_unregister_factory(void)
{
	if (FAILURE == crx_stream_filter_unregister_factory(
				crx_iconv_stream_filter_ops.label)) {
		return CRX_ICONV_ERR_UNKNOWN;
	}
	return CRX_ICONV_ERR_SUCCESS;
}
/* }}} */
/* }}} */
#endif
