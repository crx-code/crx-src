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
   | Authors: George Schlossnagle <george@omniti.com>                     |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_REFLECTION_H
#define CRX_REFLECTION_H

#include "crx.h"

extern crex_module_entry reflection_module_entry;
#define crxext_reflection_ptr &reflection_module_entry

#define CRX_REFLECTION_VERSION CRX_VERSION

BEGIN_EXTERN_C()

/* Class entry pointers */
extern CRXAPI crex_class_entry *reflector_ptr;
extern CRXAPI crex_class_entry *reflection_exception_ptr;
extern CRXAPI crex_class_entry *reflection_ptr;
extern CRXAPI crex_class_entry *reflection_function_abstract_ptr;
extern CRXAPI crex_class_entry *reflection_function_ptr;
extern CRXAPI crex_class_entry *reflection_parameter_ptr;
extern CRXAPI crex_class_entry *reflection_type_ptr;
extern CRXAPI crex_class_entry *reflection_named_type_ptr;
extern CRXAPI crex_class_entry *reflection_class_ptr;
extern CRXAPI crex_class_entry *reflection_object_ptr;
extern CRXAPI crex_class_entry *reflection_method_ptr;
extern CRXAPI crex_class_entry *reflection_property_ptr;
extern CRXAPI crex_class_entry *reflection_extension_ptr;
extern CRXAPI crex_class_entry *reflection_crex_extension_ptr;
extern CRXAPI crex_class_entry *reflection_reference_ptr;
extern CRXAPI crex_class_entry *reflection_attribute_ptr;
extern CRXAPI crex_class_entry *reflection_enum_ptr;
extern CRXAPI crex_class_entry *reflection_enum_unit_case_ptr;
extern CRXAPI crex_class_entry *reflection_enum_backed_case_ptr;
extern CRXAPI crex_class_entry *reflection_fiber_ptr;

CRXAPI void crex_reflection_class_factory(crex_class_entry *ce, zval *object);

END_EXTERN_C()

#endif /* CRX_REFLECTION_H */
