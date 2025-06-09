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
	| Authors: Andi Gutmans <andi@crx.net>                                 |
	|          Zeev Suraski <zeev@crx.net>                                 |
	+----------------------------------------------------------------------+
*/

/* {{{ includes */
#include "crx.h"
#include "crx_main.h"
#include "crex_modules.h"
#include "crex_compile.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "ext/standard/dl.h"
#include "ext/standard/file.h"
#include "ext/standard/fsock.h"
#include "ext/standard/head.h"
#include "ext/standard/pack.h"
#include "ext/standard/crx_browscap.h"
#include "ext/standard/crx_crypt.h"
#include "ext/standard/crx_dir.h"
#include "ext/standard/crx_filestat.h"
#include "ext/standard/crx_mail.h"
#include "ext/standard/crx_ext_syslog.h"
#include "ext/standard/crx_standard.h"
#include "ext/standard/crx_array.h"
#include "ext/standard/crx_assert.h"
#include "ext/reflection/crx_reflection.h"
#include "ext/random/crx_random.h"
#if HAVE_BCMATH
#include "ext/bcmath/crx_bcmath.h"
#endif
#if HAVE_CALENDAR
#include "ext/calendar/crx_calendar.h"
#endif
#if HAVE_CTYPE
#include "ext/ctype/crx_ctype.h"
#endif
#include "ext/date/crx_date.h"
#if HAVE_FTP
#include "ext/ftp/crx_ftp.h"
#endif
#if HAVE_ICONV
#include "ext/iconv/crx_iconv.h"
#endif
#include "ext/standard/reg.h"
#include "ext/pcre/crx_pcre.h"
#if HAVE_UODBC
#include "ext/odbc/crx_odbc.h"
#endif
#if HAVE_CRX_SESSION
#include "ext/session/crx_session.h"
#endif
#if HAVE_MBSTRING
#include "ext/mbstring/mbstring.h"
#endif
#if HAVE_TOKENIZER
#include "ext/tokenizer/crx_tokenizer.h"
#endif
#if HAVE_ZLIB
#include "ext/zlib/crx_zlib.h"
#endif
#if HAVE_LIBXML
#include "ext/libxml/crx_libxml.h"
#if HAVE_DOM
#include "ext/dom/crx_dom.h"
#endif
#if HAVE_SIMPLEXML
#include "ext/simplexml/crx_simplexml.h"
#endif
#endif
#if HAVE_XML
#include "ext/xml/crx_xml.h"
#endif
#include "ext/com_dotnet/crx_com_dotnet.h"
#include "ext/spl/crx_spl.h"
#if HAVE_XML && HAVE_XMLREADER
#include "ext/xmlreader/crx_xmlreader.h"
#endif
#if HAVE_XML && HAVE_XMLWRITER
#include "ext/xmlwriter/crx_xmlwriter.h"
#endif
/* }}} */

/* {{{ crx_builtin_extensions[] */
static crex_module_entry * const crx_builtin_extensions[] = {
	crxext_standard_ptr
#if HAVE_BCMATH
	,crxext_bcmath_ptr
#endif
#if HAVE_CALENDAR
	,crxext_calendar_ptr
#endif
	,crxext_com_dotnet_ptr
#if HAVE_CTYPE
	,crxext_ctype_ptr
#endif
	,crxext_date_ptr
#if HAVE_FTP
	,crxext_ftp_ptr
#endif
	,crxext_hash_ptr
#if HAVE_ICONV
	,crxext_iconv_ptr
#endif
#if HAVE_MBSTRING
	,crxext_mbstring_ptr
#endif
#if HAVE_UODBC
	,crxext_odbc_ptr
#endif
	,crxext_pcre_ptr
	,crxext_reflection_ptr
#if HAVE_CRX_SESSION
	,crxext_session_ptr
#endif
#if HAVE_TOKENIZER
	,crxext_tokenizer_ptr
#endif
#if HAVE_ZLIB
	,crxext_zlib_ptr
#endif
#if HAVE_LIBXML
	,crxext_libxml_ptr
#if HAVE_DOM
	,crxext_dom_ptr
#endif
#if HAVE_SIMPLEXML
	,crxext_simplexml_ptr
#endif
#endif
#if HAVE_XML
	,crxext_xml_ptr
#endif
	,crxext_spl_ptr
#if HAVE_XML && HAVE_XMLREADER
	,crxext_xmlreader_ptr
#endif
#if HAVE_XML && HAVE_XMLWRITER
	,crxext_xmlwriter_ptr
#endif
};
/* }}} */

#define EXTCOUNT (sizeof(crx_builtin_extensions)/sizeof(crex_module_entry *))

CRXAPI int crx_register_internal_extensions(void)
{
	return crx_register_extensions(crx_builtin_extensions, EXTCOUNT);
}
