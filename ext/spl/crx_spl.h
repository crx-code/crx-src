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
   | Authors: Marcus Boerger <helly@crx.net>                              |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_SPL_H
#define CRX_SPL_H

#include "crx.h"
#include <stdarg.h>

#define CRX_SPL_VERSION CRX_VERSION

extern crex_module_entry spl_module_entry;
#define crxext_spl_ptr &spl_module_entry

#ifdef CRX_WIN32
#	ifdef SPL_EXPORTS
#		define SPL_API __declspec(dllexport)
#	elif defined(COMPILE_DL_SPL)
#		define SPL_API __declspec(dllimport)
#	else
#		define SPL_API /* nothing */
#	endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define SPL_API __attribute__ ((visibility("default")))
#else
#	define SPL_API
#endif

#if defined(CRX_WIN32) && !defined(COMPILE_DL_SPL)
#undef crxext_spl
#define crxext_spl NULL
#endif

CRX_MINIT_FUNCTION(spl);
CRX_MSHUTDOWN_FUNCTION(spl);
CRX_RINIT_FUNCTION(spl);
CRX_RSHUTDOWN_FUNCTION(spl);
CRX_MINFO_FUNCTION(spl);

CRXAPI crex_string *crx_spl_object_hash(crex_object *obj);

#endif /* CRX_SPL_H */
