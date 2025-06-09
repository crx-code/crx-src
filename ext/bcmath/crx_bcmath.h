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
   | Author: Andi Gutmans <andi@crx.net>                                  |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_BCMATH_H
#define CRX_BCMATH_H

#include "libbcmath/src/bcmath.h"
#include "crex_API.h"

extern crex_module_entry bcmath_module_entry;
#define crxext_bcmath_ptr &bcmath_module_entry

#include "crx_version.h"
#define CRX_BCMATH_VERSION CRX_VERSION

CREX_BEGIN_MODULE_GLOBALS(bcmath)
	bc_num _zero_;
	bc_num _one_;
	bc_num _two_;
	int bc_precision;
CREX_END_MODULE_GLOBALS(bcmath)

#if defined(ZTS) && defined(COMPILE_DL_BCMATH)
CREX_TSRMLS_CACHE_EXTERN()
#endif

CREX_EXTERN_MODULE_GLOBALS(bcmath)
#define BCG(v) CREX_MODULE_GLOBALS_ACCESSOR(bcmath, v)

#endif /* CRX_BCMATH_H */
