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
   | Author: Stanislav Malyshev <stas@crx.net>                            |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_GMP_H
#define CRX_GMP_H

#include <gmp.h>

extern crex_module_entry gmp_module_entry;
#define crxext_gmp_ptr &gmp_module_entry

#include "crx_version.h"
#define CRX_GMP_VERSION CRX_VERSION

CREX_MODULE_STARTUP_D(gmp);
CREX_MODULE_DEACTIVATE_D(gmp);
CREX_MODULE_INFO_D(gmp);

CREX_BEGIN_MODULE_GLOBALS(gmp)
	bool rand_initialized;
	gmp_randstate_t rand_state;
CREX_END_MODULE_GLOBALS(gmp)

#define GMPG(v) CREX_MODULE_GLOBALS_ACCESSOR(gmp, v)

#if defined(ZTS) && defined(COMPILE_DL_GMP)
CREX_TSRMLS_CACHE_EXTERN()
#endif

#endif	/* CRX_GMP_H */
