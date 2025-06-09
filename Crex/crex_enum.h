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
   | Authors: Ilija Tovilo <ilutov@crx.net>                               |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_ENUM_H
#define CREX_ENUM_H

#include "crex.h"
#include "crex_types.h"

#include <stdint.h>

BEGIN_EXTERN_C()

extern CREX_API crex_class_entry *crex_ce_unit_enum;
extern CREX_API crex_class_entry *crex_ce_backed_enum;
extern CREX_API crex_object_handlers crex_enum_object_handlers;

void crex_register_enum_ce(void);
void crex_enum_add_interfaces(crex_class_entry *ce);
crex_result crex_enum_build_backed_enum_table(crex_class_entry *ce);
crex_object *crex_enum_new(zval *result, crex_class_entry *ce, crex_string *case_name, zval *backing_value_zv);
void crex_verify_enum(crex_class_entry *ce);
void crex_enum_register_funcs(crex_class_entry *ce);
void crex_enum_register_props(crex_class_entry *ce);

CREX_API crex_class_entry *crex_register_internal_enum(
	const char *name, uint8_t type, const crex_function_entry *functions);
CREX_API void crex_enum_add_case(crex_class_entry *ce, crex_string *case_name, zval *value);
CREX_API void crex_enum_add_case_cstr(crex_class_entry *ce, const char *name, zval *value);
CREX_API crex_object *crex_enum_get_case(crex_class_entry *ce, crex_string *name);
CREX_API crex_object *crex_enum_get_case_cstr(crex_class_entry *ce, const char *name);
CREX_API crex_result crex_enum_get_case_by_value(crex_object **result, crex_class_entry *ce, crex_long long_key, crex_string *string_key, bool try);

static crex_always_inline zval *crex_enum_fetch_case_name(crex_object *zobj)
{
	CREX_ASSERT(zobj->ce->ce_flags & CREX_ACC_ENUM);
	return OBJ_PROP_NUM(zobj, 0);
}

static crex_always_inline zval *crex_enum_fetch_case_value(crex_object *zobj)
{
	CREX_ASSERT(zobj->ce->ce_flags & CREX_ACC_ENUM);
	CREX_ASSERT(zobj->ce->enum_backing_type != IS_UNDEF);
	return OBJ_PROP_NUM(zobj, 1);
}

END_EXTERN_C()

#endif /* CREX_ENUM_H */
