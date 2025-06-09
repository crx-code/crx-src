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
   +----------------------------------------------------------------------+
 */

#ifndef CRX_ICONV_H
#define CRX_ICONV_H

#ifdef CRX_WIN32
# ifdef CRX_ICONV_EXPORTS
#  define CRX_ICONV_API __declspec(dllexport)
# else
#  define CRX_ICONV_API __declspec(dllimport)
# endif
#elif defined(__GNUC__) && __GNUC__ >= 4
# define CRX_ICONV_API __attribute__ ((visibility("default")))
#else
# define CRX_ICONV_API
#endif

#include "crx_version.h"
#define CRX_ICONV_VERSION CRX_VERSION

#ifdef HAVE_ICONV
extern crex_module_entry iconv_module_entry;
#define iconv_module_ptr &iconv_module_entry

CRX_MINIT_FUNCTION(miconv);
CRX_MSHUTDOWN_FUNCTION(miconv);
CRX_MINFO_FUNCTION(miconv);

CREX_BEGIN_MODULE_GLOBALS(iconv)
	char *input_encoding;
	char *internal_encoding;
	char *output_encoding;
CREX_END_MODULE_GLOBALS(iconv)

#define ICONVG(v) CREX_MODULE_GLOBALS_ACCESSOR(iconv, v)

#if defined(ZTS) && defined(COMPILE_DL_ICONV)
CREX_TSRMLS_CACHE_EXTERN()
#endif

#ifdef HAVE_IBM_ICONV
# define ICONV_ASCII_ENCODING "IBM-850"
# define ICONV_UCS4_ENCODING "UCS-4"
#else
# define ICONV_ASCII_ENCODING "ASCII"
# define ICONV_UCS4_ENCODING "UCS-4LE"
#endif

#ifndef ICONV_CSNMAXLEN
#define ICONV_CSNMAXLEN 64
#endif

/* {{{ typedef enum crx_iconv_err_t */
typedef enum _crx_iconv_err_t {
	CRX_ICONV_ERR_SUCCESS           = SUCCESS,
	CRX_ICONV_ERR_CONVERTER         = 1,
	CRX_ICONV_ERR_WRONG_CHARSET     = 2,
	CRX_ICONV_ERR_TOO_BIG           = 3,
	CRX_ICONV_ERR_ILLEGAL_SEQ       = 4,
	CRX_ICONV_ERR_ILLEGAL_CHAR      = 5,
	CRX_ICONV_ERR_UNKNOWN           = 6,
	CRX_ICONV_ERR_MALFORMED         = 7,
	CRX_ICONV_ERR_ALLOC             = 8,
	CRX_ICONV_ERR_OUT_BY_BOUNDS     = 9
} crx_iconv_err_t;
/* }}} */

CRX_ICONV_API crx_iconv_err_t crx_iconv_string(const char * in_p, size_t in_len, crex_string **out, const char *out_charset, const char *in_charset);

#else

#define iconv_module_ptr NULL

#endif /* HAVE_ICONV */

#define crxext_iconv_ptr iconv_module_ptr

#endif	/* CRX_ICONV_H */
