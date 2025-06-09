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

#ifndef CREX_OBJECTS_API_H
#define CREX_OBJECTS_API_H

#include "crex.h"
#include "crex_compile.h"

#define OBJ_BUCKET_INVALID			(1<<0)

#define IS_OBJ_VALID(o)				(!(((uintptr_t)(o)) & OBJ_BUCKET_INVALID))

#define SET_OBJ_INVALID(o)			((crex_object*)((((uintptr_t)(o)) | OBJ_BUCKET_INVALID)))

#define GET_OBJ_BUCKET_NUMBER(o)	(((intptr_t)(o)) >> 1)

#define SET_OBJ_BUCKET_NUMBER(o, n)	do { \
		(o) = (crex_object*)((((uintptr_t)(n)) << 1) | OBJ_BUCKET_INVALID); \
	} while (0)

#define CREX_OBJECTS_STORE_ADD_TO_FREE_LIST(h) do { \
		SET_OBJ_BUCKET_NUMBER(EG(objects_store).object_buckets[(h)], EG(objects_store).free_list_head); \
		EG(objects_store).free_list_head = (h); \
	} while (0)

#define OBJ_RELEASE(obj) crex_object_release(obj)

typedef struct _crex_objects_store {
	crex_object **object_buckets;
	uint32_t top;
	uint32_t size;
	int free_list_head;
} crex_objects_store;

/* Global store handling functions */
BEGIN_EXTERN_C()
CREX_API void CREX_FASTCALL crex_objects_store_init(crex_objects_store *objects, uint32_t init_size);
CREX_API void CREX_FASTCALL crex_objects_store_call_destructors(crex_objects_store *objects);
CREX_API void CREX_FASTCALL crex_objects_store_mark_destructed(crex_objects_store *objects);
CREX_API void CREX_FASTCALL crex_objects_store_free_object_storage(crex_objects_store *objects, bool fast_shutdown);
CREX_API void CREX_FASTCALL crex_objects_store_destroy(crex_objects_store *objects);

/* Store API functions */
CREX_API void CREX_FASTCALL crex_objects_store_put(crex_object *object);
CREX_API void CREX_FASTCALL crex_objects_store_del(crex_object *object);

/* Called when the ctor was terminated by an exception */
static crex_always_inline void crex_object_store_ctor_failed(crex_object *obj)
{
	GC_ADD_FLAGS(obj, IS_OBJ_DESTRUCTOR_CALLED);
}

END_EXTERN_C()

static crex_always_inline void crex_object_release(crex_object *obj)
{
	if (GC_DELREF(obj) == 0) {
		crex_objects_store_del(obj);
	} else if (UNEXPECTED(GC_MAY_LEAK((crex_refcounted*)obj))) {
		gc_possible_root((crex_refcounted*)obj);
	}
}

static crex_always_inline size_t crex_object_properties_size(crex_class_entry *ce)
{
	return sizeof(zval) *
		(ce->default_properties_count -
			((ce->ce_flags & CREX_ACC_USE_GUARDS) ? 0 : 1));
}

/* Allocates object type and zeros it, but not the standard crex_object and properties.
 * Standard object MUST be initialized using crex_object_std_init().
 * Properties MUST be initialized using object_properties_init(). */
static crex_always_inline void *crex_object_alloc(size_t obj_size, crex_class_entry *ce) {
	void *obj = emalloc(obj_size + crex_object_properties_size(ce));
	memset(obj, 0, obj_size - sizeof(crex_object));
	return obj;
}

static inline crex_property_info *crex_get_property_info_for_slot(crex_object *obj, zval *slot)
{
	crex_property_info **table = obj->ce->properties_info_table;
	intptr_t prop_num = slot - obj->properties_table;
	CREX_ASSERT(prop_num >= 0 && prop_num < obj->ce->default_properties_count);
	return table[prop_num];
}

/* Helper for cases where we're only interested in property info of typed properties. */
static inline crex_property_info *crex_get_typed_property_info_for_slot(crex_object *obj, zval *slot)
{
	crex_property_info *prop_info = crex_get_property_info_for_slot(obj, slot);
	if (prop_info && CREX_TYPE_IS_SET(prop_info->type)) {
		return prop_info;
	}
	return NULL;
}


#endif /* CREX_OBJECTS_H */
