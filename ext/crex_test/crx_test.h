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

#ifndef CRX_TEST_H
#define CRX_TEST_H

#include "fiber.h"

extern crex_module_entry crex_test_module_entry;
#define crxext_crex_test_ptr &crex_test_module_entry

#define CRX_CREX_TEST_VERSION "0.1.0"

#ifdef ZTS
#include "TSRM.h"
#endif

#if defined(ZTS) && defined(COMPILE_DL_CREX_TEST)
CREX_TSRMLS_CACHE_EXTERN()
#endif

CREX_BEGIN_MODULE_GLOBALS(crex_test)
	int observer_enabled;
	int observer_show_output;
	int observer_observe_all;
	int observer_observe_includes;
	int observer_observe_functions;
	int observer_observe_declaring;
	crex_array *observer_observe_function_names;
	int observer_show_return_type;
	int observer_show_return_value;
	int observer_show_init_backtrace;
	int observer_show_opcode;
	char *observer_show_opcode_in_user_handler;
	int observer_nesting_depth;
	int observer_fiber_init;
	int observer_fiber_switch;
	int observer_fiber_destroy;
	int observer_execute_internal;
	HashTable global_weakmap;
	int replace_crex_execute_ex;
	int register_passes;
	bool print_stderr_mshutdown;
	crex_long limit_copy_file_range;
	crex_test_fiber *active_fiber;
	crex_long quantity_value;
	crex_string *str_test;
	crex_string *not_empty_str_test;
CREX_END_MODULE_GLOBALS(crex_test)

extern CREX_DECLARE_MODULE_GLOBALS(crex_test)

#define ZT_G(v) CREX_MODULE_GLOBALS_ACCESSOR(crex_test, v)

struct bug79096 {
	uint64_t a;
	uint64_t b;
};

#ifdef CRX_WIN32
#	define CRX_CREX_TEST_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define CRX_CREX_TEST_API __attribute__ ((visibility("default")))
#else
#	define CRX_CREX_TEST_API
#endif

CRX_CREX_TEST_API int CREX_FASTCALL bug78270(const char *str, size_t str_len);

CRX_CREX_TEST_API struct bug79096 bug79096(void);
CRX_CREX_TEST_API void bug79532(off_t *array, size_t elems);

extern CRX_CREX_TEST_API int *(*bug79177_cb)(void);
CRX_CREX_TEST_API void bug79177(void);

#endif
