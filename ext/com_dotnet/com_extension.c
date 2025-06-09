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
   | Author: Wez Furlong  <wez@thebrainroom.com>                          |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <intsafe.h>

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "crx_com_dotnet.h"
#include "crx_com_dotnet_internal.h"
#include "Crex/crex_exceptions.h"
#include "Crex/crex_interfaces.h"

#if SIZEOF_CREX_LONG == 8
#define CRX_DISP_E_DIVBYZERO ((crex_long) (ULONG) DISP_E_DIVBYZERO)
#define CRX_DISP_E_OVERFLOW ((crex_long) (ULONG) DISP_E_OVERFLOW)
#define CRX_DISP_E_BADINDEX ((crex_long) (ULONG) DISP_E_BADINDEX)
#define CRX_DISP_E_PARAMNOTFOUND ((crex_long) (ULONG) DISP_E_PARAMNOTFOUND)
#define CRX_MK_E_UNAVAILABLE ((crex_long) (ULONG) MK_E_UNAVAILABLE)
#else
#define CRX_DISP_E_DIVBYZERO DISP_E_DIVBYZERO
#define CRX_DISP_E_OVERFLOW DISP_E_OVERFLOW
#define CRX_DISP_E_BADINDEX DISP_E_BADINDEX
#define CRX_DISP_E_PARAMNOTFOUND DISP_E_PARAMNOTFOUND
#define CRX_MK_E_UNAVAILABLE MK_E_UNAVAILABLE
#endif

#include "com_extension_arginfo.h"

CREX_DECLARE_MODULE_GLOBALS(com_dotnet)
static CRX_GINIT_FUNCTION(com_dotnet);

crex_class_entry
	*crx_com_variant_class_entry,
   	*crx_com_exception_class_entry,
	*crx_com_saproxy_class_entry;

/* {{{ com_dotnet_module_entry */
crex_module_entry com_dotnet_module_entry = {
	STANDARD_MODULE_HEADER,
	"com_dotnet",
	ext_functions,
	CRX_MINIT(com_dotnet),
	CRX_MSHUTDOWN(com_dotnet),
	CRX_RINIT(com_dotnet),
	CRX_RSHUTDOWN(com_dotnet),
	CRX_MINFO(com_dotnet),
	CRX_COM_DOTNET_VERSION,
	CRX_MODULE_GLOBALS(com_dotnet),
	CRX_GINIT(com_dotnet),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_COM_DOTNET
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(com_dotnet)
#endif

/* {{{ CRX_INI */

/* com.typelib_file is the path to a file containing a
 * list of typelibraries to register *persistently*.
 * lines starting with ; are comments
 * append #cis to end of typelib name to cause its constants
 * to be loaded case insensitively */
static CRX_INI_MH(OnTypeLibFileUpdate)
{
	FILE *typelib_file;
	char *typelib_name_buffer;
	char *strtok_buf = NULL;

	if (NULL == new_value || !new_value->val[0] || (typelib_file = VCWD_FOPEN(new_value->val, "r"))==NULL) {
		return FAILURE;
	}

	typelib_name_buffer = (char *) emalloc(sizeof(char)*1024);

	while (fgets(typelib_name_buffer, 1024, typelib_file)) {
		ITypeLib *pTL;
		char *typelib_name;
		char *modifier, *ptr;

		if (typelib_name_buffer[0]==';') {
			continue;
		}
		typelib_name = crx_strtok_r(typelib_name_buffer, "\r\n", &strtok_buf); /* get rid of newlines */
		if (typelib_name == NULL) {
			continue;
		}
		typelib_name = crx_strtok_r(typelib_name, "#", &strtok_buf);
		modifier = crx_strtok_r(NULL, "#", &strtok_buf);
		if (modifier != NULL) {
			if (!strcmp(modifier, "cis") || !strcmp(modifier, "case_insensitive")) {
				crx_error_docref("com.configuration", E_WARNING, "Declaration of case-insensitive constants is no longer supported; #cis modifier ignored");
			}
		}

		/* Remove leading/training white spaces on search_string */
		while (isspace(*typelib_name)) {/* Ends on '\0' in worst case */
			typelib_name ++;
		}
		ptr = typelib_name + strlen(typelib_name) - 1;
		while ((ptr != typelib_name) && isspace(*ptr)) {
			*ptr = '\0';
			ptr--;
		}

		if ((pTL = crx_com_load_typelib_via_cache(typelib_name, COMG(code_page))) != NULL) {
			crx_com_import_typelib(pTL, CONST_PERSISTENT, COMG(code_page));
			ITypeLib_Release(pTL);
		}
	}

	efree(typelib_name_buffer);
	fclose(typelib_file);

	return SUCCESS;
}

static CREX_INI_MH(OnAutoregisterCasesensitive)
{
	if (!crex_ini_parse_bool(new_value)) {
		crx_error_docref("com.configuration", E_WARNING, "Declaration of case-insensitive constants is no longer supported");
		return FAILURE;
	}
	return OnUpdateBool(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
}

CRX_INI_BEGIN()
	STD_CRX_INI_BOOLEAN("com.allow_dcom",				"0", CRX_INI_SYSTEM, OnUpdateBool, allow_dcom, crex_com_dotnet_globals, com_dotnet_globals)
	STD_CRX_INI_BOOLEAN("com.autoregister_verbose",	"0", CRX_INI_ALL, OnUpdateBool, autoreg_verbose, crex_com_dotnet_globals, com_dotnet_globals)
	STD_CRX_INI_BOOLEAN("com.autoregister_typelib",	"0", CRX_INI_ALL, OnUpdateBool, autoreg_on, crex_com_dotnet_globals, com_dotnet_globals)
	STD_CRX_INI_ENTRY("com.autoregister_casesensitive",	"1", CRX_INI_ALL, OnAutoregisterCasesensitive, autoreg_case_sensitive, crex_com_dotnet_globals, com_dotnet_globals)
	STD_CRX_INI_ENTRY("com.code_page", "", CRX_INI_ALL, OnUpdateLong, code_page, crex_com_dotnet_globals, com_dotnet_globals)
	CRX_INI_ENTRY("com.typelib_file", "", CRX_INI_SYSTEM, OnTypeLibFileUpdate)
	CRX_INI_ENTRY("com.dotnet_version", NULL, CRX_INI_SYSTEM, NULL)
CRX_INI_END()
/* }}} */

/* {{{ CRX_GINIT_FUNCTION */
static CRX_GINIT_FUNCTION(com_dotnet)
{
#if defined(COMPILE_DL_COM_DOTNET) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	memset(com_dotnet_globals, 0, sizeof(*com_dotnet_globals));
	com_dotnet_globals->code_page = CP_ACP;
}
/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(com_dotnet)
{
	crex_class_entry *tmp;

	crx_com_wrapper_minit(INIT_FUNC_ARGS_PASSTHRU);
	crx_com_persist_minit(INIT_FUNC_ARGS_PASSTHRU);

	crx_com_exception_class_entry = register_class_com_exception(crex_ce_exception);
/*	crx_com_exception_class_entry->constructor->common.fn_flags |= CREX_ACC_PROTECTED; */

	crx_com_saproxy_class_entry = register_class_com_safearray_proxy();
/*	crx_com_saproxy_class_entry->constructor->common.fn_flags |= CREX_ACC_PROTECTED; */
	crx_com_saproxy_class_entry->default_object_handlers = &crx_com_saproxy_handlers;
	crx_com_saproxy_class_entry->get_iterator = crx_com_saproxy_iter_get;

	crx_com_variant_class_entry = register_class_variant();
	crx_com_variant_class_entry->default_object_handlers = &crx_com_object_handlers;
	crx_com_variant_class_entry->create_object = crx_com_object_new;
	crx_com_variant_class_entry->get_iterator = crx_com_iter_get;

	tmp = register_class_com(crx_com_variant_class_entry);
	tmp->default_object_handlers = &crx_com_object_handlers;
	tmp->create_object = crx_com_object_new;
	tmp->get_iterator = crx_com_iter_get;

#if HAVE_MSCOREE_H
	tmp = register_class_dotnet(crx_com_variant_class_entry);
	tmp->default_object_handlers = &crx_com_object_handlers;
	tmp->create_object = crx_com_object_new;
	tmp->get_iterator = crx_com_iter_get;
#endif

	REGISTER_INI_ENTRIES();

	register_com_extension_symbols(module_number);

	CRX_MINIT(com_typeinfo)(INIT_FUNC_ARGS_PASSTHRU);

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(com_dotnet)
{
	UNREGISTER_INI_ENTRIES();
#if HAVE_MSCOREE_H
	if (COMG(dotnet_runtime_stuff)) {
		crx_com_dotnet_mshutdown();
	}
#endif

	CRX_MSHUTDOWN(com_typeinfo)(INIT_FUNC_ARGS_PASSTHRU);

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_RINIT_FUNCTION */
CRX_RINIT_FUNCTION(com_dotnet)
{
	COMG(rshutdown_started) = 0;
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_RSHUTDOWN_FUNCTION */
CRX_RSHUTDOWN_FUNCTION(com_dotnet)
{
#if HAVE_MSCOREE_H
	if (COMG(dotnet_runtime_stuff)) {
		crx_com_dotnet_rshutdown();
	}
#endif
	COMG(rshutdown_started) = 1;
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(com_dotnet)
{
	crx_info_print_table_start();

	crx_info_print_table_row(2, "COM support", "enabled");
	crx_info_print_table_row(2, "DCOM support", COMG(allow_dcom) ? "enabled" : "disabled");

#if HAVE_MSCOREE_H
	crx_info_print_table_row(2, ".Net support", "enabled");
#else
	crx_info_print_table_row(2, ".Net support", "not present in this build");
#endif

	crx_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */
