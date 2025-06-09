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
  | Author: Wez Furlong <wez@crx.net>                                    |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_PDO_H
#define CRX_PDO_H

#include "crex.h"

extern crex_module_entry pdo_module_entry;
#define crxext_pdo_ptr &pdo_module_entry

#include "crx_version.h"
#define CRX_PDO_VERSION CRX_VERSION

#ifdef CRX_WIN32
#	if defined(PDO_EXPORTS) || (!defined(COMPILE_DL_PDO))
#		define PDO_API __declspec(dllexport)
#	elif defined(COMPILE_DL_PDO)
#		define PDO_API __declspec(dllimport)
#	else
#		define PDO_API /* nothing special */
#	endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PDO_API __attribute__ ((visibility("default")))
#else
#	define PDO_API /* nothing special */
#endif

#ifdef ZTS
# include "TSRM.h"
#endif

CRX_MINIT_FUNCTION(pdo);
CRX_MSHUTDOWN_FUNCTION(pdo);
CRX_MINFO_FUNCTION(pdo);

#define REGISTER_PDO_CLASS_CONST_LONG(const_name, value) \
	crex_declare_class_constant_long(crx_pdo_get_dbh_ce(), const_name, sizeof(const_name)-1, (crex_long)value);

#define REGISTER_PDO_CLASS_CONST_STRING(const_name, value) \
	crex_declare_class_constant_stringl(crx_pdo_get_dbh_ce(), const_name, sizeof(const_name)-1, value, sizeof(value)-1);

#define LONG_CONST(c) (crex_long) c

#define PDO_CONSTRUCT_CHECK \
	if (!dbh->driver) { \
		crex_throw_error(NULL, "PDO object is not initialized, constructor was not called"); \
		RETURN_THROWS(); \
	} \


#endif	/* CRX_PDO_H */
