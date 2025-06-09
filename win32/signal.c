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
   | Author: Anatol Belski <ab@crx.net>                                   |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "SAPI.h"

#include "win32/console.h"

/* true globals; only used from main thread and from kernel callback */
static zval ctrl_handler;
static DWORD ctrl_evt = (DWORD)-1;
static crex_atomic_bool *vm_interrupt_flag = NULL;

static void (*orig_interrupt_function)(crex_execute_data *execute_data);

static void crx_win32_signal_ctrl_interrupt_function(crex_execute_data *execute_data)
{/*{{{*/
	if (IS_UNDEF != C_TYPE(ctrl_handler)) {
		zval retval, params[1];

		ZVAL_LONG(&params[0], ctrl_evt);

		/* If the function returns, */
		call_user_function(NULL, NULL, &ctrl_handler, &retval, 1, params);
		zval_ptr_dtor(&retval);
	}

	if (orig_interrupt_function) {
		orig_interrupt_function(execute_data);
	}
}/*}}}*/

CRX_WINUTIL_API void crx_win32_signal_ctrl_handler_init(void)
{/*{{{*/
	/* We are in the main thread! */
	if (!crx_win32_console_is_cli_sapi()) {
		return;
	}

	orig_interrupt_function = crex_interrupt_function;
	crex_interrupt_function = crx_win32_signal_ctrl_interrupt_function;
	vm_interrupt_flag = &EG(vm_interrupt);
	ZVAL_UNDEF(&ctrl_handler);

	REGISTER_MAIN_LONG_CONSTANT("CRX_WINDOWS_EVENT_CTRL_C", CTRL_C_EVENT, CONST_PERSISTENT);
	REGISTER_MAIN_LONG_CONSTANT("CRX_WINDOWS_EVENT_CTRL_BREAK", CTRL_BREAK_EVENT, CONST_PERSISTENT);
}/*}}}*/

CRX_WINUTIL_API void crx_win32_signal_ctrl_handler_shutdown(void)
{/*{{{*/
	if (!crx_win32_console_is_cli_sapi()) {
		return;
	}

	crex_interrupt_function = orig_interrupt_function;
	orig_interrupt_function = NULL;
	vm_interrupt_flag = NULL;
	ZVAL_UNDEF(&ctrl_handler);
}/*}}}*/

static BOOL WINAPI crx_win32_signal_system_ctrl_handler(DWORD evt)
{/*{{{*/
	if (CTRL_C_EVENT != evt && CTRL_BREAK_EVENT != evt) {
		return FALSE;
	}

	crex_atomic_bool_store_ex(vm_interrupt_flag, true);

	ctrl_evt = evt;

	return TRUE;
}/*}}}*/

/* {{{ Assigns a CTRL signal handler to a CRX function */
CRX_FUNCTION(sapi_windows_set_ctrl_handler)
{
	crex_fcall_info fci;
	crex_fcall_info_cache fcc;
	bool add = 1;


	/* callable argument corresponds to the CTRL handler */
	if (crex_parse_parameters(CREX_NUM_ARGS(), "f!|b", &fci, &fcc, &add) == FAILURE) {
		RETURN_THROWS();
	}

#if ZTS
	if (!tsrm_is_main_thread()) {
		crex_throw_error(NULL, "CTRL events can only be received on the main thread");
		RETURN_THROWS();
	}
#endif

	if (!crx_win32_console_is_cli_sapi()) {
		crex_throw_error(NULL, "CTRL events trapping is only supported on console");
		RETURN_THROWS();
	}

	if (!CREX_FCI_INITIALIZED(fci)) {
		zval_ptr_dtor(&ctrl_handler);
		ZVAL_UNDEF(&ctrl_handler);
		if (!SetConsoleCtrlHandler(NULL, add)) {
			RETURN_FALSE;
		}
		RETURN_TRUE;
	}

	if (!SetConsoleCtrlHandler(NULL, FALSE) || !SetConsoleCtrlHandler(crx_win32_signal_system_ctrl_handler, add)) {
		crex_string *func_name = crex_get_callable_name(&fci.function_name);
		crx_error_docref(NULL, E_WARNING, "Unable to attach %s as a CTRL handler", ZSTR_VAL(func_name));
		crex_string_release_ex(func_name, 0);
		RETURN_FALSE;
	}

	zval_ptr_dtor_nogc(&ctrl_handler);
	ZVAL_COPY(&ctrl_handler, &fci.function_name);

	RETURN_TRUE;
}/*}}}*/

CRX_FUNCTION(sapi_windows_generate_ctrl_event)
{/*{{{*/
	crex_long evt, pid = 0;
	bool ret = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l|l", &evt, &pid) == FAILURE) {
		RETURN_THROWS();
	}

	if (!crx_win32_console_is_cli_sapi()) {
		crex_throw_error(NULL, "CTRL events trapping is only supported on console");
		return;
	}

	SetConsoleCtrlHandler(NULL, TRUE);

	ret = (GenerateConsoleCtrlEvent(evt, pid) != 0);

	if (IS_UNDEF != C_TYPE(ctrl_handler)) {
		ret = ret && SetConsoleCtrlHandler(crx_win32_signal_system_ctrl_handler, TRUE);
	}

	RETURN_BOOL(ret);
}/*}}}*/
