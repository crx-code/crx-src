%HEADER%

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "crx.h"
#include "ext/standard/info.h"
#include "crx_%EXTNAME%.h"
#include "%EXTNAME%_arginfo.h"

/* For compatibility with older CRX versions */
#ifndef CREX_PARSE_PARAMETERS_NONE
#define CREX_PARSE_PARAMETERS_NONE() \
	CREX_PARSE_PARAMETERS_START(0, 0) \
	CREX_PARSE_PARAMETERS_END()
#endif

/* {{{ void test1() */
CRX_FUNCTION(test1)
{
	CREX_PARSE_PARAMETERS_NONE();

	crx_printf("The extension %s is loaded and working!\r\n", "%EXTNAME%");
}
/* }}} */

/* {{{ string test2( [ string $var ] ) */
CRX_FUNCTION(test2)
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

/* {{{ CRX_RINIT_FUNCTION */
CRX_RINIT_FUNCTION(%EXTNAME%)
{
#if defined(ZTS) && defined(COMPILE_DL_%EXTNAMECAPS%)
	CREX_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(%EXTNAME%)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "%EXTNAME% support", "enabled");
	crx_info_print_table_end();
}
/* }}} */

/* {{{ %EXTNAME%_module_entry */
crex_module_entry %EXTNAME%_module_entry = {
	STANDARD_MODULE_HEADER,
	"%EXTNAME%",					/* Extension name */
	ext_functions,					/* crex_function_entry */
	NULL,							/* CRX_MINIT - Module initialization */
	NULL,							/* CRX_MSHUTDOWN - Module shutdown */
	CRX_RINIT(%EXTNAME%),			/* CRX_RINIT - Request initialization */
	NULL,							/* CRX_RSHUTDOWN - Request shutdown */
	CRX_MINFO(%EXTNAME%),			/* CRX_MINFO - Module info */
	CRX_%EXTNAMECAPS%_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_%EXTNAMECAPS%
# ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
# endif
CREX_GET_MODULE(%EXTNAME%)
#endif
