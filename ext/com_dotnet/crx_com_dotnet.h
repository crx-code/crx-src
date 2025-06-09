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
   | Author: Wez Furlong <wez@thebrainroom.com>                           |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_COM_DOTNET_H
#define CRX_COM_DOTNET_H

extern crex_module_entry com_dotnet_module_entry;
#define crxext_com_dotnet_ptr &com_dotnet_module_entry

#ifdef ZTS
#include "TSRM.h"
#endif

#define CRX_COM_DOTNET_API __declspec(dllexport)

#include "crx_version.h"
#define CRX_COM_DOTNET_VERSION CRX_VERSION

CRX_MINIT_FUNCTION(com_dotnet);
CRX_MSHUTDOWN_FUNCTION(com_dotnet);
CRX_RINIT_FUNCTION(com_dotnet);
CRX_RSHUTDOWN_FUNCTION(com_dotnet);
CRX_MINFO_FUNCTION(com_dotnet);

CREX_BEGIN_MODULE_GLOBALS(com_dotnet)
	bool allow_dcom;
	bool autoreg_verbose;
	bool autoreg_on;
	bool autoreg_case_sensitive;
	void *dotnet_runtime_stuff; /* opaque to avoid cluttering up other modules */
	int code_page; /* default code_page if left unspecified */
	bool rshutdown_started;
CREX_END_MODULE_GLOBALS(com_dotnet)

#if defined(ZTS) && defined(COMPILE_DL_COM_DOTNET)
CREX_TSRMLS_CACHE_EXTERN()
#endif

CREX_EXTERN_MODULE_GLOBALS(com_dotnet)
#define COMG(v) CREX_MODULE_GLOBALS_ACCESSOR(com_dotnet, v)

#endif	/* CRX_COM_DOTNET_H */
