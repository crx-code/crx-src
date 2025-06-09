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
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_globals.h"
#include "crex_variables.h"
#include "crex_API.h"
#include "crex_objects_API.h"
#include "crex_fibers.h"

CREX_API void CREX_FASTCALL crex_objects_store_init(crex_objects_store *objects, uint32_t init_size)
{
	objects->object_buckets = (crex_object **) emalloc(init_size * sizeof(crex_object*));
	objects->top = 1; /* Skip 0 so that handles are true */
	objects->size = init_size;
	objects->free_list_head = -1;
	memset(&objects->object_buckets[0], 0, sizeof(crex_object*));
}

CREX_API void CREX_FASTCALL crex_objects_store_destroy(crex_objects_store *objects)
{
	efree(objects->object_buckets);
	objects->object_buckets = NULL;
}

CREX_API void CREX_FASTCALL crex_objects_store_call_destructors(crex_objects_store *objects)
{
	EG(flags) |= EG_FLAGS_OBJECT_STORE_NO_REUSE;
	if (objects->top > 1) {
		crex_fiber_switch_block();

		uint32_t i;
		for (i = 1; i < objects->top; i++) {
			crex_object *obj = objects->object_buckets[i];
			if (IS_OBJ_VALID(obj)) {
				if (!(OBJ_FLAGS(obj) & IS_OBJ_DESTRUCTOR_CALLED)) {
					GC_ADD_FLAGS(obj, IS_OBJ_DESTRUCTOR_CALLED);

					if (obj->handlers->dtor_obj != crex_objects_destroy_object
							|| obj->ce->destructor) {
						GC_ADDREF(obj);
						obj->handlers->dtor_obj(obj);
						GC_DELREF(obj);
					}
				}
			}
		}

		crex_fiber_switch_unblock();
	}
}

CREX_API void CREX_FASTCALL crex_objects_store_mark_destructed(crex_objects_store *objects)
{
	if (objects->object_buckets && objects->top > 1) {
		crex_object **obj_ptr = objects->object_buckets + 1;
		crex_object **end = objects->object_buckets + objects->top;

		do {
			crex_object *obj = *obj_ptr;

			if (IS_OBJ_VALID(obj)) {
				GC_ADD_FLAGS(obj, IS_OBJ_DESTRUCTOR_CALLED);
			}
			obj_ptr++;
		} while (obj_ptr != end);
	}
}

CREX_API void CREX_FASTCALL crex_objects_store_free_object_storage(crex_objects_store *objects, bool fast_shutdown)
{
	crex_object **obj_ptr, **end, *obj;

	if (objects->top <= 1) {
		return;
	}

	/* Free object contents, but don't free objects themselves, so they show up as leaks.
	 * Also add a ref to all objects, so the object can't be freed by something else later. */
	end = objects->object_buckets + 1;
	obj_ptr = objects->object_buckets + objects->top;

	if (fast_shutdown) {
		do {
			obj_ptr--;
			obj = *obj_ptr;
			if (IS_OBJ_VALID(obj)) {
				if (!(OBJ_FLAGS(obj) & IS_OBJ_FREE_CALLED)) {
					GC_ADD_FLAGS(obj, IS_OBJ_FREE_CALLED);
					if (obj->handlers->free_obj != crex_object_std_dtor) {
						GC_ADDREF(obj);
						obj->handlers->free_obj(obj);
					}
				}
			}
		} while (obj_ptr != end);
	} else {
		do {
			obj_ptr--;
			obj = *obj_ptr;
			if (IS_OBJ_VALID(obj)) {
				if (!(OBJ_FLAGS(obj) & IS_OBJ_FREE_CALLED)) {
					GC_ADD_FLAGS(obj, IS_OBJ_FREE_CALLED);
					GC_ADDREF(obj);
					obj->handlers->free_obj(obj);
				}
			}
		} while (obj_ptr != end);
	}
}


/* Store objects API */
static CREX_COLD crex_never_inline void CREX_FASTCALL crex_objects_store_put_cold(crex_object *object)
{
	int handle;
	uint32_t new_size = 2 * EG(objects_store).size;

	EG(objects_store).object_buckets = (crex_object **) erealloc(EG(objects_store).object_buckets, new_size * sizeof(crex_object*));
	/* Assign size after realloc, in case it fails */
	EG(objects_store).size = new_size;
	handle = EG(objects_store).top++;
	object->handle = handle;
	EG(objects_store).object_buckets[handle] = object;
}

CREX_API void CREX_FASTCALL crex_objects_store_put(crex_object *object)
{
	int handle;

	/* When in shutdown sequence - do not reuse previously freed handles, to make sure
	 * the dtors for newly created objects are called in crex_objects_store_call_destructors() loop
	 */
	if (EG(objects_store).free_list_head != -1 && EXPECTED(!(EG(flags) & EG_FLAGS_OBJECT_STORE_NO_REUSE))) {
		handle = EG(objects_store).free_list_head;
		EG(objects_store).free_list_head = GET_OBJ_BUCKET_NUMBER(EG(objects_store).object_buckets[handle]);
	} else if (UNEXPECTED(EG(objects_store).top == EG(objects_store).size)) {
		crex_objects_store_put_cold(object);
		return;
	} else {
		handle = EG(objects_store).top++;
	}
	object->handle = handle;
	EG(objects_store).object_buckets[handle] = object;
}

CREX_API void CREX_FASTCALL crex_objects_store_del(crex_object *object) /* {{{ */
{
	CREX_ASSERT(GC_REFCOUNT(object) == 0);

	/* GC might have released this object already. */
	if (UNEXPECTED(GC_TYPE(object) == IS_NULL)) {
		return;
	}

	/*	Make sure we hold a reference count during the destructor call
		otherwise, when the destructor ends the storage might be freed
		when the refcount reaches 0 a second time
	 */
	if (!(OBJ_FLAGS(object) & IS_OBJ_DESTRUCTOR_CALLED)) {
		GC_ADD_FLAGS(object, IS_OBJ_DESTRUCTOR_CALLED);

		if (object->handlers->dtor_obj != crex_objects_destroy_object
				|| object->ce->destructor) {
			crex_fiber_switch_block();
			GC_SET_REFCOUNT(object, 1);
			object->handlers->dtor_obj(object);
			GC_DELREF(object);
			crex_fiber_switch_unblock();
		}
	}

	if (GC_REFCOUNT(object) == 0) {
		uint32_t handle = object->handle;
		void *ptr;

		CREX_ASSERT(EG(objects_store).object_buckets != NULL);
		CREX_ASSERT(IS_OBJ_VALID(EG(objects_store).object_buckets[handle]));
		EG(objects_store).object_buckets[handle] = SET_OBJ_INVALID(object);
		if (!(OBJ_FLAGS(object) & IS_OBJ_FREE_CALLED)) {
			GC_ADD_FLAGS(object, IS_OBJ_FREE_CALLED);
			GC_SET_REFCOUNT(object, 1);
			object->handlers->free_obj(object);
		}
		ptr = ((char*)object) - object->handlers->offset;
		GC_REMOVE_FROM_BUFFER(object);
		efree(ptr);
		CREX_OBJECTS_STORE_ADD_TO_FREE_LIST(handle);
	}
}
/* }}} */
