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
   | Author: Wez Furlong <wez@thebrainroom.com>                           |
   |         Marcus Boerger <helly@crx.net>                               |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_API.h"

static crex_class_entry crex_iterator_class_entry;

static void iter_wrapper_free(crex_object *object);
static void iter_wrapper_dtor(crex_object *object);
static HashTable *iter_wrapper_get_gc(crex_object *object, zval **table, int *n);

static const crex_object_handlers iterator_object_handlers = {
	0,
	iter_wrapper_free,
	iter_wrapper_dtor,
	NULL, /* clone_obj */
	NULL, /* prop read */
	NULL, /* prop write */
	NULL, /* read dim */
	NULL, /* write dim */
	NULL, /* get_property_ptr_ptr */
	NULL, /* has prop */
	NULL, /* unset prop */
	NULL, /* has dim */
	NULL, /* unset dim */
	NULL, /* props get */
	NULL, /* method get */
	NULL, /* get ctor */
	NULL, /* get class name */
	NULL, /* cast */
	NULL, /* count */
	NULL, /* get_debug_info */
	NULL, /* get_closure */
	iter_wrapper_get_gc,
	NULL, /* do_operation */
	NULL, /* compare */
	NULL  /* get_properties_for */
};

CREX_API void crex_register_iterator_wrapper(void)
{
	INIT_CLASS_ENTRY(crex_iterator_class_entry, "__iterator_wrapper", NULL);
	crex_iterator_class_entry.default_object_handlers = &iterator_object_handlers;
}

static void iter_wrapper_free(crex_object *object)
{
	crex_object_iterator *iter = (crex_object_iterator*)object;
	iter->funcs->dtor(iter);
}

static void iter_wrapper_dtor(crex_object *object)
{
}

static HashTable *iter_wrapper_get_gc(crex_object *object, zval **table, int *n) {
	crex_object_iterator *iter = (crex_object_iterator*)object;
	if (iter->funcs->get_gc) {
		return iter->funcs->get_gc(iter, table, n);
	}

	*table = NULL;
	*n = 0;
	return NULL;
}

CREX_API void crex_iterator_init(crex_object_iterator *iter)
{
	crex_object_std_init(&iter->std, &crex_iterator_class_entry);
}

CREX_API void crex_iterator_dtor(crex_object_iterator *iter)
{
	if (GC_DELREF(&iter->std) > 0) {
		return;
	}

	crex_objects_store_del(&iter->std);
}

CREX_API crex_object_iterator* crex_iterator_unwrap(zval *array_ptr)
{
	CREX_ASSERT(C_TYPE_P(array_ptr) == IS_OBJECT);
	if (C_OBJ_HT_P(array_ptr) == &iterator_object_handlers) {
		return (crex_object_iterator *)C_OBJ_P(array_ptr);
	}
	return NULL;
}
