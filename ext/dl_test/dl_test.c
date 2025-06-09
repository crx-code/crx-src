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
  | Author: Arnaud Le Blanc <arnaud.lb@gmail.com>                        |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "crx.h"
#include "ext/standard/info.h"
#include "crx_dl_test.h"
#include "dl_test_arginfo.h"

CREX_DECLARE_MODULE_GLOBALS(dl_test)

/* {{{ void dl_test_test1() */
CRX_FUNCTION(dl_test_test1)
{
	CREX_PARSE_PARAMETERS_NONE();

	crx_printf("The extension %s is loaded and working!\r\n", "dl_test");
}
/* }}} */

/* {{{ string dl_test_test2( [ string $var ] ) */
CRX_FUNCTION(dl_test_test2)
{
	char *var = "World";
	size_t var_len = sizeof("World") - 1;
	crex_string *retval;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_STRING(var, var_len)
	CREX_PARSE_PARAMETERS_END();

	retval = strpprintf(0, "Hello %s", var);

	RETURN_STR(retval);
}
/* }}}*/

/* {{{ CRX_DL_TEST_USE_REGISTER_FUNCTIONS_DIRECTLY */
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dl_test_use_register_functions_directly, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CRX_FUNCTION(dl_test_use_register_functions_directly)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_STRING("OK");
}

static const crex_function_entry crx_dl_test_use_register_functions_directly_functions[] = {
	CREX_FENTRY(dl_test_use_register_functions_directly, CREX_FN(dl_test_use_register_functions_directly), arginfo_dl_test_use_register_functions_directly,  0)
	CREX_FE_END
};
/* }}} */

/* {{{ INI */
CRX_INI_BEGIN()
	STD_CRX_INI_ENTRY("dl_test.long",       "0", CRX_INI_ALL, OnUpdateLong,   long_value,   crex_dl_test_globals, dl_test_globals)
	STD_CRX_INI_ENTRY("dl_test.string", "hello", CRX_INI_ALL, OnUpdateString, string_value, crex_dl_test_globals, dl_test_globals)
CRX_INI_END()
/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(dl_test)
{
	/* Test backwards compatibility */
	if (getenv("CRX_DL_TEST_USE_OLD_REGISTER_INI_ENTRIES")) {
		crex_register_ini_entries(ini_entries, module_number);
	} else {
		REGISTER_INI_ENTRIES();
	}

	if (getenv("CRX_DL_TEST_USE_REGISTER_FUNCTIONS_DIRECTLY")) {
		crex_register_functions(NULL, crx_dl_test_use_register_functions_directly_functions, NULL, type);
	}

	if (getenv("CRX_DL_TEST_MODULE_DEBUG")) {
		fprintf(stderr, "DL TEST MINIT\n");
	}

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
static CRX_MSHUTDOWN_FUNCTION(dl_test)
{
	/* Test backwards compatibility */
	if (getenv("CRX_DL_TEST_USE_OLD_REGISTER_INI_ENTRIES")) {
		crex_unregister_ini_entries(module_number);
	} else {
		UNREGISTER_INI_ENTRIES();
	}

	if (getenv("CRX_DL_TEST_MODULE_DEBUG")) {
		fprintf(stderr, "DL TEST MSHUTDOWN\n");
	}

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_RINIT_FUNCTION */
CRX_RINIT_FUNCTION(dl_test)
{
#if defined(ZTS) && defined(COMPILE_DL_DL_TEST)
	CREX_TSRMLS_CACHE_UPDATE();
#endif

	if (getenv("CRX_DL_TEST_MODULE_DEBUG")) {
		fprintf(stderr, "DL TEST RINIT\n");
	}

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_RSHUTDOWN_FUNCTION */
CRX_RSHUTDOWN_FUNCTION(dl_test)
{
	if (getenv("CRX_DL_TEST_MODULE_DEBUG")) {
		fprintf(stderr, "DL TEST RSHUTDOWN\n");
	}

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(dl_test)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "dl_test support", "enabled");
	crx_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ CRX_GINIT_FUNCTION */
static CRX_GINIT_FUNCTION(dl_test)
{
#if defined(COMPILE_DL_DL_TEST) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	memset(dl_test_globals, 0, sizeof(*dl_test_globals));
}
/* }}} */

/* {{{ dl_test_module_entry */
crex_module_entry dl_test_module_entry = {
	STANDARD_MODULE_HEADER,
	"dl_test",
	ext_functions,
	CRX_MINIT(dl_test),
	CRX_MSHUTDOWN(dl_test),
	CRX_RINIT(dl_test),
	CRX_RSHUTDOWN(dl_test),
	CRX_MINFO(dl_test),
	CRX_DL_TEST_VERSION,
	CRX_MODULE_GLOBALS(dl_test),
	CRX_GINIT(dl_test),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_DL_TEST
# ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
# endif
CREX_GET_MODULE(dl_test)
#endif
