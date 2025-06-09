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
  | Author: John Coggeshall <john@crx.net>                               |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_TIDY_H
#define CRX_TIDY_H

extern crex_module_entry tidy_module_entry;
#define crxext_tidy_ptr &tidy_module_entry

#include "crx_version.h"
#define CRX_TIDY_VERSION CRX_VERSION

CREX_BEGIN_MODULE_GLOBALS(tidy)
	char *default_config;
	bool clean_output;
CREX_END_MODULE_GLOBALS(tidy)

#define TG(v) CREX_MODULE_GLOBALS_ACCESSOR(tidy, v)

#if defined(ZTS) && defined(COMPILE_DL_TIDY)
CREX_TSRMLS_CACHE_EXTERN()
#endif

#endif
