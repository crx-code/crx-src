/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: David Soria Parra <david.soriaparra@sun.com>                |
   +----------------------------------------------------------------------+
*/

#ifndef	_CREX_DTRACE_H
#define	_CREX_DTRACE_H

#ifndef CREX_WIN32
# include <unistd.h>
#endif

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef HAVE_DTRACE
CREX_API extern crex_op_array *(*crex_dtrace_compile_file)(crex_file_handle *file_handle, int type);
CREX_API extern void (*crex_dtrace_execute)(crex_op_array *op_array);
CREX_API extern void (*crex_dtrace_execute_internal)(crex_execute_data *execute_data, zval *return_value);

CREX_API crex_op_array *dtrace_compile_file(crex_file_handle *file_handle, int type);
CREX_API void dtrace_execute_ex(crex_execute_data *execute_data);
CREX_API void dtrace_execute_internal(crex_execute_data *execute_data, zval *return_value);
#include <crex_dtrace_gen.h>

void dtrace_error_notify_cb(int type, crex_string *error_filename, uint32_t error_lineno, crex_string *message);

#endif /* HAVE_DTRACE */

#ifdef	__cplusplus
}
#endif

#endif	/* _CREX_DTRACE_H */
