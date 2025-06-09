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
   | Author: Kristian Koehntopp <kris@koehntopp.de>                       |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_POSIX_H
#define CRX_POSIX_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_POSIX
#ifndef DLEXPORT
#define DLEXPORT
#endif

extern crex_module_entry posix_module_entry;
#define posix_module_ptr &posix_module_entry

#include "crx_version.h"
#define CRX_POSIX_VERSION CRX_VERSION

CREX_BEGIN_MODULE_GLOBALS(posix)
	int last_error;
CREX_END_MODULE_GLOBALS(posix)

#if defined(ZTS) && defined(COMPILE_DL_POSIX)
CREX_TSRMLS_CACHE_EXTERN()
#endif

CREX_EXTERN_MODULE_GLOBALS(posix)
#define POSIX_G(v) CREX_MODULE_GLOBALS_ACCESSOR(posix, v)

#else

#define posix_module_ptr NULL

#endif

#define crxext_posix_ptr posix_module_ptr

#endif /* CRX_POSIX_H */
