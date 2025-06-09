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
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_DL_TEST_H
# define CRX_DL_TEST_H

extern crex_module_entry dl_test_module_entry;
# define crxext_dl_test_ptr &dl_test_module_entry

# define CRX_DL_TEST_VERSION CRX_VERSION

# if defined(ZTS) && defined(COMPILE_DL_DL_TEST)
CREX_TSRMLS_CACHE_EXTERN()
# endif

CREX_BEGIN_MODULE_GLOBALS(dl_test)
	crex_long long_value;
	char *string_value;
CREX_END_MODULE_GLOBALS(dl_test);

#define DT_G(v) CREX_MODULE_GLOBALS_ACCESSOR(dl_test, v)

#endif	/* CRX_DL_TEST_H */
