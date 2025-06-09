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

#ifndef CRX_MAIN_H
#define CRX_MAIN_H

#include "crex_globals.h"
#include "crx_globals.h"
#include "SAPI.h"

BEGIN_EXTERN_C()

/* Returns the CRX version the engine was built with. This is useful for
 * extensions which want to know the version of CRX at run-time, rather than
 * the version they were built with at compile-time.
 */
CRXAPI const char *crx_version(void);

/* Returns the CRX version id the engine was built with. This is useful for
 * extensions which want to know the version of CRX at run-time, rather than
 * the version they were built with at compile-time.
 */
CRXAPI unsigned int crx_version_id(void);

CRXAPI crex_result crx_request_startup(void);
CRXAPI void crx_request_shutdown(void *dummy);
CRXAPI crex_result crx_module_startup(sapi_module_struct *sf, crex_module_entry *additional_module);
CRXAPI void crx_module_shutdown(void);
CRXAPI int crx_module_shutdown_wrapper(sapi_module_struct *sapi_globals);

CRXAPI crex_result crx_register_extensions(crex_module_entry * const * ptr, int count);

CRXAPI bool crx_execute_script(crex_file_handle *primary_file);
CRXAPI int crx_execute_simple_script(crex_file_handle *primary_file, zval *ret);
CRXAPI crex_result crx_lint_script(crex_file_handle *file);

CRXAPI void crx_handle_aborted_connection(void);
CRXAPI int crx_handle_auth_data(const char *auth);

CRXAPI void crx_html_puts(const char *str, size_t siz);
CRXAPI crex_result crx_stream_open_for_crex_ex(crex_file_handle *handle, int mode);

/* environment module */
extern int crx_init_environ(void);
extern int crx_shutdown_environ(void);

#ifdef ZTS
CRXAPI void crx_reserve_tsrm_memory(void);
CRXAPI bool crx_tsrm_startup_ex(int expected_threads);
CRXAPI bool crx_tsrm_startup(void);

#define CRX_ZTS 1
#else
#define CRX_ZTS 0
#endif

#ifdef CRX_WIN32
#define CRX_OS_STR "WINNT"
#else
#define CRX_OS_STR CRX_OS
#endif

END_EXTERN_C()

#endif
