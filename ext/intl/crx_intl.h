/*
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the CRX license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.crx.net/license/3_01.txt                                 |
   | If you did not receive a copy of the CRX license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crx.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Vadim Savchuk <vsavchuk@productengine.com>                  |
   |          Dmitry Lakhtyuk <dlakhtyuk@productengine.com>               |
   |          Stanislav Malyshev <stas@crex.com>                          |
   |          Kirti Velankar <kirtig@yahoo-inc.com>                       |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_INTL_H
#define CRX_INTL_H

#include <crx.h>

/* Even if we're included from C++, don't introduce C++ definitions
 * because we were included with extern "C". The effect would be that
 * when the headers defined any method, they would do so with C linkage */
#undef U_SHOW_CPLUSPLUS_API
#define U_SHOW_CPLUSPLUS_API 0
#include "collator/collator_sort.h"
#include <unicode/ubrk.h>
#include "intl_error.h"
#include "Crex/crex_exceptions.h"

extern crex_module_entry intl_module_entry;
#define crxext_intl_ptr &intl_module_entry

#ifdef CRX_WIN32
#define CRX_INTL_API __declspec(dllexport)
#else
#define CRX_INTL_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

CREX_BEGIN_MODULE_GLOBALS(intl)
	struct UCollator *current_collator;
	char* default_locale;
	collator_compare_func_t compare_func;
	UBreakIterator* grapheme_iterator;
	intl_error g_error;
	crex_long error_level;
	bool use_exceptions;
CREX_END_MODULE_GLOBALS(intl)

#if defined(ZTS) && defined(COMPILE_DL_INTL)
CREX_TSRMLS_CACHE_EXTERN()
#endif

CREX_EXTERN_MODULE_GLOBALS(intl)
/* Macro to access request-wide global variables. */
#define INTL_G(v) CREX_MODULE_GLOBALS_ACCESSOR(intl, v)

CRX_MINIT_FUNCTION(intl);
CRX_MSHUTDOWN_FUNCTION(intl);
CRX_RINIT_FUNCTION(intl);
CRX_RSHUTDOWN_FUNCTION(intl);
CRX_MINFO_FUNCTION(intl);

const char *intl_locale_get_default( void );

#define CRX_INTL_VERSION CRX_VERSION

#endif  /* CRX_INTL_H */
