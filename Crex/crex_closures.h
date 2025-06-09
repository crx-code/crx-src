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
   | Authors: Christian Seiler <chris_se@gmx.net>                         |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_CLOSURES_H
#define CREX_CLOSURES_H

BEGIN_EXTERN_C()

/* This macro depends on crex_closure structure layout */
#define CREX_CLOSURE_OBJECT(op_array) \
	((crex_object*)((char*)(op_array) - sizeof(crex_object)))

void crex_register_closure_ce(void);
void crex_closure_bind_var(zval *closure_zv, crex_string *var_name, zval *var);
void crex_closure_bind_var_ex(zval *closure_zv, uint32_t offset, zval *val);
void crex_closure_from_frame(zval *closure_zv, crex_execute_data *frame);

extern CREX_API crex_class_entry *crex_ce_closure;

CREX_API void crex_create_closure(zval *res, crex_function *op_array, crex_class_entry *scope, crex_class_entry *called_scope, zval *this_ptr);
CREX_API void crex_create_fake_closure(zval *res, crex_function *op_array, crex_class_entry *scope, crex_class_entry *called_scope, zval *this_ptr);
CREX_API crex_function *crex_get_closure_invoke_method(crex_object *obj);
CREX_API const crex_function *crex_get_closure_method_def(crex_object *obj);
CREX_API zval* crex_get_closure_this_ptr(zval *obj);

END_EXTERN_C()

#endif
