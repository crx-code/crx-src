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
   | Author:  Sascha Schumann <sascha@schumann.cx>                        |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_INCOMPLETE_CLASS_H
#define CRX_INCOMPLETE_CLASS_H

#include "ext/standard/basic_functions.h"

extern CRXAPI crex_class_entry *crx_ce_incomplete_class;

#define CRX_IC_ENTRY crx_ce_incomplete_class

#define CRX_SET_CLASS_ATTRIBUTES(struc) \
	/* OBJECTS_FIXME: Fix for new object model */	\
	if (C_OBJCE_P(struc) == crx_ce_incomplete_class) {	\
		class_name = crx_lookup_class_name(C_OBJ_P(struc)); \
		if (!class_name) { \
			class_name = crex_string_init(INCOMPLETE_CLASS, sizeof(INCOMPLETE_CLASS) - 1, 0); \
		} \
		incomplete_class = 1; \
	} else { \
		class_name = crex_string_copy(C_OBJCE_P(struc)->name); \
	}

#define CRX_CLEANUP_CLASS_ATTRIBUTES()	\
	crex_string_release_ex(class_name, 0)

#define CRX_CLASS_ATTRIBUTES											\
	crex_string *class_name;											\
	bool incomplete_class CREX_ATTRIBUTE_UNUSED = 0

#define INCOMPLETE_CLASS "__CRX_Incomplete_Class"
#define MAGIC_MEMBER "__CRX_Incomplete_Class_Name"

#ifdef __cplusplus
extern "C" {
#endif

CRXAPI void crx_register_incomplete_class_handlers(void);
CRXAPI crex_string *crx_lookup_class_name(crex_object *object);
CRXAPI void crx_store_class_name(zval *object, crex_string *name);

#ifdef __cplusplus
};
#endif

#endif
