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
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_OBJECTS_H
#define CREX_OBJECTS_H

#include "crex.h"

BEGIN_EXTERN_C()
CREX_API void CREX_FASTCALL crex_object_std_init(crex_object *object, crex_class_entry *ce);
CREX_API crex_object* CREX_FASTCALL crex_objects_new(crex_class_entry *ce);
CREX_API void CREX_FASTCALL crex_objects_clone_members(crex_object *new_object, crex_object *old_object);

CREX_API void crex_object_std_dtor(crex_object *object);
CREX_API void crex_objects_destroy_object(crex_object *object);
CREX_API crex_object *crex_objects_clone_obj(crex_object *object);
END_EXTERN_C()

#endif /* CREX_OBJECTS_H */
