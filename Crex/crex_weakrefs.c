/*
   +----------------------------------------------------------------------+
   | Copyright (c) The CRX Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: krakjoe@crx.net                                             |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_interfaces.h"
#include "crex_objects_API.h"
#include "crex_types.h"
#include "crex_weakrefs.h"
#include "crex_weakrefs_arginfo.h"

typedef struct _crex_weakref {
	crex_object *referent;
	crex_object std;
} crex_weakref;

typedef struct _crex_weakmap {
	HashTable ht;
	crex_object std;
} crex_weakmap;

typedef struct _crex_weakmap_iterator {
	crex_object_iterator it;
	uint32_t ht_iter;
} crex_weakmap_iterator;

/* EG(weakrefs) is a map from a key corresponding to a crex_object pointer to all the WeakReference and/or WeakMap entries relating to that pointer.
 *
 * 1. For a single WeakReference,
 *    the HashTable's corresponding value's tag is a CREX_WEAKREF_TAG_REF and the pointer is a singleton WeakReference instance (crex_weakref *) for that crex_object pointer (from WeakReference::create()).
 * 2. For a single WeakMap, the HashTable's corresponding value's tag is a CREX_WEAKREF_TAG_MAP and the pointer is a WeakMap instance (crex_weakmap *).
 * 3. For multiple values associated with the same crex_object pointer, the HashTable entry's tag is a CREX_WEAKREF_TAG_HT with a HashTable mapping
 *    tagged pointers of at most 1 WeakReference and 1 or more WeakMaps to the same tagged pointer.
 *
 * CREX_MM_ALIGNED_OFFSET_LOG2 is at least 2 on supported architectures (pointers to the objects in question are aligned to 4 bytes (1<<2) even on 32-bit systems),
 * i.e. the least two significant bits of the pointer can be used as a tag (CREX_WEAKREF_TAG_*). */
#define CREX_WEAKREF_TAG_REF 0
#define CREX_WEAKREF_TAG_MAP 1
#define CREX_WEAKREF_TAG_HT  2
#define CREX_WEAKREF_GET_TAG(p) (((uintptr_t) (p)) & 3)
#define CREX_WEAKREF_GET_PTR(p) ((void *) (((uintptr_t) (p)) & ~3))
#define CREX_WEAKREF_ENCODE(p, t) ((void *) (((uintptr_t) (p)) | (t)))

crex_class_entry *crex_ce_weakref;
crex_class_entry *crex_ce_weakmap;
static crex_object_handlers crex_weakref_handlers;
static crex_object_handlers crex_weakmap_handlers;

#define crex_weakref_from(o) ((crex_weakref*)(((char*) o) - XtOffsetOf(crex_weakref, std)))
#define crex_weakref_fetch(z) crex_weakref_from(C_OBJ_P(z))

#define crex_weakmap_from(o) ((crex_weakmap*)(((char*) o) - XtOffsetOf(crex_weakmap, std)))
#define crex_weakmap_fetch(z) crex_weakmap_from(C_OBJ_P(z))

static inline void crex_weakref_unref_single(
		void *ptr, uintptr_t tag, crex_object *object)
{
	if (tag == CREX_WEAKREF_TAG_REF) {
		/* Unreferencing WeakReference (at ptr) singleton that pointed to object. */
		crex_weakref *wr = ptr;
		wr->referent = NULL;
	} else {
		/* unreferencing WeakMap entry (at ptr) with a key of object. */
		CREX_ASSERT(tag == CREX_WEAKREF_TAG_MAP);
		crex_hash_index_del((HashTable *) ptr, crex_object_to_weakref_key(object));
	}
}

static void crex_weakref_unref(crex_object *object, void *tagged_ptr) {
	void *ptr = CREX_WEAKREF_GET_PTR(tagged_ptr);
	uintptr_t tag = CREX_WEAKREF_GET_TAG(tagged_ptr);
	if (tag == CREX_WEAKREF_TAG_HT) {
		HashTable *ht = ptr;
		CREX_HASH_MAP_FOREACH_PTR(ht, tagged_ptr) {
			crex_weakref_unref_single(
				CREX_WEAKREF_GET_PTR(tagged_ptr), CREX_WEAKREF_GET_TAG(tagged_ptr), object);
		} CREX_HASH_FOREACH_END();
		crex_hash_destroy(ht);
		FREE_HASHTABLE(ht);
	} else {
		crex_weakref_unref_single(ptr, tag, object);
	}
}

static void crex_weakref_register(crex_object *object, void *payload) {
	GC_ADD_FLAGS(object, IS_OBJ_WEAKLY_REFERENCED);

	crex_ulong obj_key = crex_object_to_weakref_key(object);
	zval *zv = crex_hash_index_lookup(&EG(weakrefs), obj_key);
	if (C_TYPE_P(zv) == IS_NULL) {
		ZVAL_PTR(zv, payload);
		return;
	}

	void *tagged_ptr = C_PTR_P(zv);
	if (CREX_WEAKREF_GET_TAG(tagged_ptr) == CREX_WEAKREF_TAG_HT) {
		HashTable *ht = CREX_WEAKREF_GET_PTR(tagged_ptr);
		crex_hash_index_add_new_ptr(ht, (crex_ulong) payload, payload);
		return;
	}

	/* Convert simple pointer to hashtable. */
	HashTable *ht = emalloc(sizeof(HashTable));
	crex_hash_init(ht, 0, NULL, NULL, 0);
	crex_hash_index_add_new_ptr(ht, (crex_ulong) tagged_ptr, tagged_ptr);
	crex_hash_index_add_new_ptr(ht, (crex_ulong) payload, payload);
	/* Replace the single WeakMap or WeakReference entry in EG(weakrefs) with a HashTable with 2 entries in place. */
	ZVAL_PTR(zv, CREX_WEAKREF_ENCODE(ht, CREX_WEAKREF_TAG_HT));
}

static void crex_weakref_unregister(crex_object *object, void *payload, bool weakref_free) {
	crex_ulong obj_key = crex_object_to_weakref_key(object);
	void *tagged_ptr = crex_hash_index_find_ptr(&EG(weakrefs), obj_key);
	CREX_ASSERT(tagged_ptr && "Weakref not registered?");

	void *ptr = CREX_WEAKREF_GET_PTR(tagged_ptr);
	uintptr_t tag = CREX_WEAKREF_GET_TAG(tagged_ptr);
	if (tag != CREX_WEAKREF_TAG_HT) {
		CREX_ASSERT(tagged_ptr == payload);
		crex_hash_index_del(&EG(weakrefs), obj_key);
		GC_DEL_FLAGS(object, IS_OBJ_WEAKLY_REFERENCED);

		/* Do this last, as it may destroy the object. */
		if (weakref_free) {
			crex_weakref_unref_single(ptr, tag, object);
		} else {
			/* The optimization of skipping unref is only used in the destructor of WeakMap */
			CREX_ASSERT(CREX_WEAKREF_GET_TAG(payload) == CREX_WEAKREF_TAG_MAP);
		}
		return;
	}

	HashTable *ht = ptr;
#if CREX_DEBUG
	void *old_payload = crex_hash_index_find_ptr(ht, (crex_ulong) payload);
	CREX_ASSERT(old_payload && "Weakref not registered?");
	CREX_ASSERT(old_payload == payload);
#endif
	crex_hash_index_del(ht, (crex_ulong) payload);
	if (crex_hash_num_elements(ht) == 0) {
		GC_DEL_FLAGS(object, IS_OBJ_WEAKLY_REFERENCED);
		crex_hash_destroy(ht);
		FREE_HASHTABLE(ht);
		crex_hash_index_del(&EG(weakrefs), obj_key);
	}

	/* Do this last, as it may destroy the object. */
	if (weakref_free)  {
		crex_weakref_unref_single(
			CREX_WEAKREF_GET_PTR(payload), CREX_WEAKREF_GET_TAG(payload), object);
	} else {
		/* The optimization of skipping unref is only used in the destructor of WeakMap */
		CREX_ASSERT(CREX_WEAKREF_GET_TAG(payload) == CREX_WEAKREF_TAG_MAP);
	}
}

CREX_API zval *crex_weakrefs_hash_add(HashTable *ht, crex_object *key, zval *pData) {
	zval *zv = crex_hash_index_add(ht, crex_object_to_weakref_key(key), pData);
	if (zv) {
		crex_weakref_register(key, CREX_WEAKREF_ENCODE(ht, CREX_WEAKREF_TAG_MAP));
	}
	return zv;
}

CREX_API crex_result crex_weakrefs_hash_del(HashTable *ht, crex_object *key) {
	zval *zv = crex_hash_index_find(ht, crex_object_to_weakref_key(key));
	if (zv) {
		crex_weakref_unregister(key, CREX_WEAKREF_ENCODE(ht, CREX_WEAKREF_TAG_MAP), 1);
		return SUCCESS;
	}
	return FAILURE;
}

void crex_weakrefs_init(void) {
	crex_hash_init(&EG(weakrefs), 8, NULL, NULL, 0);
}

/* This is called when the object is garbage collected
 * to remove all WeakReference and WeakMap entries weakly referencing that object. */
void crex_weakrefs_notify(crex_object *object) {
	/* Annoyingly we can't use the HT destructor here, because we need access to the key (which
	 * is the object address), which is not provided to the dtor. */
	const crex_ulong obj_key = crex_object_to_weakref_key(object);
	void *tagged_ptr = crex_hash_index_find_ptr(&EG(weakrefs), obj_key);
#if CREX_DEBUG
	CREX_ASSERT(tagged_ptr && "Tracking of the IS_OBJ_WEAKLY_REFERENCE flag should be precise");
#endif
	if (tagged_ptr) {
		crex_weakref_unref(object, tagged_ptr);
		crex_hash_index_del(&EG(weakrefs), obj_key);
	}
}

void crex_weakrefs_shutdown(void) {
	crex_hash_destroy(&EG(weakrefs));
}

static crex_object* crex_weakref_new(crex_class_entry *ce) {
	crex_weakref *wr = crex_object_alloc(sizeof(crex_weakref), crex_ce_weakref);

	crex_object_std_init(&wr->std, crex_ce_weakref);
	return &wr->std;
}

static crex_always_inline bool crex_weakref_find(crex_object *referent, zval *return_value) {
	void *tagged_ptr = crex_hash_index_find_ptr(&EG(weakrefs), crex_object_to_weakref_key(referent));
	if (!tagged_ptr) {
		return 0;
	}

	void *ptr = CREX_WEAKREF_GET_PTR(tagged_ptr);
	uintptr_t tag = CREX_WEAKREF_GET_TAG(tagged_ptr);
	if (tag == CREX_WEAKREF_TAG_REF) {
		crex_weakref *wr;
found_weakref:
		wr = ptr;
		RETVAL_OBJ_COPY(&wr->std);
		return 1;
	}

	if (tag == CREX_WEAKREF_TAG_HT) {
		CREX_HASH_MAP_FOREACH_PTR(ptr, tagged_ptr) {
			if (CREX_WEAKREF_GET_TAG(tagged_ptr) == CREX_WEAKREF_TAG_REF) {
				ptr = CREX_WEAKREF_GET_PTR(tagged_ptr);
				goto found_weakref;
			}
		} CREX_HASH_FOREACH_END();
	}

	return 0;
}

static crex_always_inline void crex_weakref_create(crex_object *referent, zval *return_value) {
	crex_weakref *wr;

	object_init_ex(return_value, crex_ce_weakref);

	wr = crex_weakref_fetch(return_value);
	wr->referent = referent;

	crex_weakref_register(wr->referent, CREX_WEAKREF_ENCODE(wr, CREX_WEAKREF_TAG_REF));
}

static crex_always_inline void crex_weakref_get(zval *weakref, zval *return_value) {
	crex_weakref *wr = crex_weakref_fetch(weakref);

	if (wr->referent) {
		RETVAL_OBJ_COPY(wr->referent);
	}
}

static void crex_weakref_free(crex_object *zo) {
	crex_weakref *wr = crex_weakref_from(zo);

	if (wr->referent) {
		crex_weakref_unregister(wr->referent, CREX_WEAKREF_ENCODE(wr, CREX_WEAKREF_TAG_REF), 1);
	}

	crex_object_std_dtor(&wr->std);
}

CREX_COLD CREX_METHOD(WeakReference, __main)
{
	crex_throw_error(NULL, "Direct instantiation of WeakReference is not allowed, use WeakReference::create instead");
}

CREX_METHOD(WeakReference, create)
{
	crex_object *referent;

	CREX_PARSE_PARAMETERS_START(1,1)
		C_PARAM_OBJ(referent)
	CREX_PARSE_PARAMETERS_END();

	if (crex_weakref_find(referent, return_value)) {
	    return;
	}

	crex_weakref_create(referent, return_value);
}

CREX_METHOD(WeakReference, get)
{
	CREX_PARSE_PARAMETERS_NONE();

	crex_weakref_get(getThis(), return_value);
}

static crex_object *crex_weakmap_create_object(crex_class_entry *ce)
{
	crex_weakmap *wm = crex_object_alloc(sizeof(crex_weakmap), ce);
	crex_object_std_init(&wm->std, ce);

	crex_hash_init(&wm->ht, 0, NULL, ZVAL_PTR_DTOR, 0);
	return &wm->std;
}

static void crex_weakmap_free_obj(crex_object *object)
{
	crex_weakmap *wm = crex_weakmap_from(object);
	crex_ulong obj_key;
	CREX_HASH_MAP_FOREACH_NUM_KEY(&wm->ht, obj_key) {
		/* Optimization: Don't call crex_weakref_unref_single to free individual entries from wm->ht when unregistering (which would do a hash table lookup, call crex_hash_index_del, and skip over any bucket collisions).
		 * Let freeing the corresponding values for WeakMap entries be done in crex_hash_destroy, freeing objects sequentially.
		 * The performance difference is notable for larger WeakMaps with worse cache locality. */
		crex_weakref_unregister(
			crex_weakref_key_to_object(obj_key), CREX_WEAKREF_ENCODE(&wm->ht, CREX_WEAKREF_TAG_MAP), 0);
	} CREX_HASH_FOREACH_END();
	crex_hash_destroy(&wm->ht);
	crex_object_std_dtor(&wm->std);
}

static zval *crex_weakmap_read_dimension(crex_object *object, zval *offset, int type, zval *rv)
{
	if (offset == NULL) {
		crex_throw_error(NULL, "Cannot append to WeakMap");
		return NULL;
	}

	ZVAL_DEREF(offset);
	if (C_TYPE_P(offset) != IS_OBJECT) {
		crex_type_error("WeakMap key must be an object");
		return NULL;
	}

	crex_weakmap *wm = crex_weakmap_from(object);
	crex_object *obj_addr = C_OBJ_P(offset);
	zval *zv = crex_hash_index_find(&wm->ht, crex_object_to_weakref_key(obj_addr));
	if (zv == NULL) {
		if (type != BP_VAR_IS) {
			crex_throw_error(NULL,
				"Object %s#%d not contained in WeakMap", ZSTR_VAL(obj_addr->ce->name), obj_addr->handle);
			return NULL;
		}
		return NULL;
	}

	if (type == BP_VAR_W || type == BP_VAR_RW) {
		ZVAL_MAKE_REF(zv);
	}
	return zv;
}

static void crex_weakmap_write_dimension(crex_object *object, zval *offset, zval *value)
{
	if (offset == NULL) {
		crex_throw_error(NULL, "Cannot append to WeakMap");
		return;
	}

	ZVAL_DEREF(offset);
	if (C_TYPE_P(offset) != IS_OBJECT) {
		crex_type_error("WeakMap key must be an object");
		return;
	}

	crex_weakmap *wm = crex_weakmap_from(object);
	crex_object *obj_addr = C_OBJ_P(offset);
	crex_ulong obj_key = crex_object_to_weakref_key(obj_addr);
	C_TRY_ADDREF_P(value);

	zval *zv = crex_hash_index_find(&wm->ht, obj_key);
	if (zv) {
		/* Because the destructors can have side effects such as resizing or rehashing the WeakMap storage,
		 * free the zval only after overwriting the original value. */
		zval zv_orig;
		ZVAL_COPY_VALUE(&zv_orig, zv);
		ZVAL_COPY_VALUE(zv, value);
		zval_ptr_dtor(&zv_orig);
		return;
	}

	crex_weakref_register(obj_addr, CREX_WEAKREF_ENCODE(&wm->ht, CREX_WEAKREF_TAG_MAP));
	crex_hash_index_add_new(&wm->ht, obj_key, value);
}

/* int return and check_empty due to Object Handler API */
static int crex_weakmap_has_dimension(crex_object *object, zval *offset, int check_empty)
{
	ZVAL_DEREF(offset);
	if (C_TYPE_P(offset) != IS_OBJECT) {
		crex_type_error("WeakMap key must be an object");
		return 0;
	}

	crex_weakmap *wm = crex_weakmap_from(object);
	zval *zv = crex_hash_index_find(&wm->ht, crex_object_to_weakref_key(C_OBJ_P(offset)));
	if (!zv) {
		return 0;
	}

	if (check_empty) {
		return i_crex_is_true(zv);
	}
	return C_TYPE_P(zv) != IS_NULL;
}

static void crex_weakmap_unset_dimension(crex_object *object, zval *offset)
{
	ZVAL_DEREF(offset);
	if (C_TYPE_P(offset) != IS_OBJECT) {
		crex_type_error("WeakMap key must be an object");
		return;
	}

	crex_weakmap *wm = crex_weakmap_from(object);
	crex_object *obj_addr = C_OBJ_P(offset);
	if (!crex_hash_index_exists(&wm->ht, crex_object_to_weakref_key(obj_addr))) {
		/* Object not in WeakMap, do nothing. */
		return;
	}

	crex_weakref_unregister(obj_addr, CREX_WEAKREF_ENCODE(&wm->ht, CREX_WEAKREF_TAG_MAP), 1);
}

static crex_result crex_weakmap_count_elements(crex_object *object, crex_long *count)
{
	crex_weakmap *wm = crex_weakmap_from(object);
	*count = crex_hash_num_elements(&wm->ht);
	return SUCCESS;
}

static HashTable *crex_weakmap_get_properties_for(crex_object *object, crex_prop_purpose purpose)
{
	if (purpose != CREX_PROP_PURPOSE_DEBUG) {
		return NULL;
	}

	crex_weakmap *wm = crex_weakmap_from(object);
	HashTable *ht;
	ALLOC_HASHTABLE(ht);
	crex_hash_init(ht, crex_hash_num_elements(&wm->ht), NULL, ZVAL_PTR_DTOR, 0);

	crex_ulong obj_key;
	zval *val;
	CREX_HASH_MAP_FOREACH_NUM_KEY_VAL(&wm->ht, obj_key, val) {
		crex_object *obj = crex_weakref_key_to_object(obj_key);
		zval pair;
		array_init(&pair);

		GC_ADDREF(obj);
		add_assoc_object(&pair, "key", obj);
		C_TRY_ADDREF_P(val);
		add_assoc_zval(&pair, "value", val);

		crex_hash_next_index_insert_new(ht, &pair);
	} CREX_HASH_FOREACH_END();

	return ht;
}

HashTable *crex_weakmap_get_gc(crex_object *object, zval **table, int *n)
{
	crex_weakmap *wm = crex_weakmap_from(object);
	crex_get_gc_buffer *gc_buffer = crex_get_gc_buffer_create();
	zval *val;
	CREX_HASH_MAP_FOREACH_VAL(&wm->ht, val) {
		crex_get_gc_buffer_add_zval(gc_buffer, val);
	} CREX_HASH_FOREACH_END();
	crex_get_gc_buffer_use(gc_buffer, table, n);
	return NULL;
}

HashTable *crex_weakmap_get_key_entry_gc(crex_object *object, zval **table, int *n)
{
	crex_weakmap *wm = crex_weakmap_from(object);
	crex_get_gc_buffer *gc_buffer = crex_get_gc_buffer_create();
	crex_ulong h;
	zval *val;
	CREX_HASH_MAP_FOREACH_NUM_KEY_VAL(&wm->ht, h, val) {
		crex_object *key = crex_weakref_key_to_object(h);
		crex_get_gc_buffer_add_obj(gc_buffer, key);
		crex_get_gc_buffer_add_ptr(gc_buffer, val);
	} CREX_HASH_FOREACH_END();
	crex_get_gc_buffer_use(gc_buffer, table, n);
	return NULL;
}

HashTable *crex_weakmap_get_entry_gc(crex_object *object, zval **table, int *n)
{
	crex_weakmap *wm = crex_weakmap_from(object);
	crex_get_gc_buffer *gc_buffer = crex_get_gc_buffer_create();
	zval *val;
	CREX_HASH_MAP_FOREACH_VAL(&wm->ht, val) {
		crex_get_gc_buffer_add_ptr(gc_buffer, val);
	} CREX_HASH_FOREACH_END();
	crex_get_gc_buffer_use(gc_buffer, table, n);
	return NULL;
}

HashTable *crex_weakmap_get_object_key_entry_gc(crex_object *object, zval **table, int *n)
{
	crex_get_gc_buffer *gc_buffer = crex_get_gc_buffer_create();
	const crex_ulong obj_key = crex_object_to_weakref_key(object);
	void *tagged_ptr = crex_hash_index_find_ptr(&EG(weakrefs), obj_key);
#if CREX_DEBUG
	CREX_ASSERT(tagged_ptr && "Tracking of the IS_OBJ_WEAKLY_REFERENCE flag should be precise");
#endif
	void *ptr = CREX_WEAKREF_GET_PTR(tagged_ptr);
	uintptr_t tag = CREX_WEAKREF_GET_TAG(tagged_ptr);

	if (tag == CREX_WEAKREF_TAG_HT) {
		HashTable *ht = ptr;
		CREX_HASH_MAP_FOREACH_PTR(ht, tagged_ptr) {
			if (CREX_WEAKREF_GET_TAG(tagged_ptr) == CREX_WEAKREF_TAG_MAP) {
				crex_weakmap *wm = (crex_weakmap*) CREX_WEAKREF_GET_PTR(tagged_ptr);
				zval *zv = crex_hash_index_find(&wm->ht, obj_key);
				CREX_ASSERT(zv);
				crex_get_gc_buffer_add_ptr(gc_buffer, zv);
				crex_get_gc_buffer_add_obj(gc_buffer, &wm->std);
			}
		} CREX_HASH_FOREACH_END();
	} else if (tag == CREX_WEAKREF_TAG_MAP) {
		crex_weakmap *wm = (crex_weakmap*) ptr;
		zval *zv = crex_hash_index_find(&wm->ht, obj_key);
		CREX_ASSERT(zv);
		crex_get_gc_buffer_add_ptr(gc_buffer, zv);
		crex_get_gc_buffer_add_obj(gc_buffer, &wm->std);
	}

	crex_get_gc_buffer_use(gc_buffer, table, n);

	return NULL;
}

HashTable *crex_weakmap_get_object_entry_gc(crex_object *object, zval **table, int *n)
{
	crex_get_gc_buffer *gc_buffer = crex_get_gc_buffer_create();
	const crex_ulong obj_key = crex_object_to_weakref_key(object);
	void *tagged_ptr = crex_hash_index_find_ptr(&EG(weakrefs), obj_key);
#if CREX_DEBUG
	CREX_ASSERT(tagged_ptr && "Tracking of the IS_OBJ_WEAKLY_REFERENCE flag should be precise");
#endif
	void *ptr = CREX_WEAKREF_GET_PTR(tagged_ptr);
	uintptr_t tag = CREX_WEAKREF_GET_TAG(tagged_ptr);

	if (tag == CREX_WEAKREF_TAG_HT) {
		HashTable *ht = ptr;
		CREX_HASH_MAP_FOREACH_PTR(ht, tagged_ptr) {
			if (CREX_WEAKREF_GET_TAG(tagged_ptr) == CREX_WEAKREF_TAG_MAP) {
				crex_weakmap *wm = (crex_weakmap*) CREX_WEAKREF_GET_PTR(tagged_ptr);
				zval *zv = crex_hash_index_find(&wm->ht, obj_key);
				CREX_ASSERT(zv);
				crex_get_gc_buffer_add_ptr(gc_buffer, zv);
			}
		} CREX_HASH_FOREACH_END();
	} else if (tag == CREX_WEAKREF_TAG_MAP) {
		crex_weakmap *wm = (crex_weakmap*) ptr;
		zval *zv = crex_hash_index_find(&wm->ht, obj_key);
		CREX_ASSERT(zv);
		crex_get_gc_buffer_add_ptr(gc_buffer, zv);
	}

	crex_get_gc_buffer_use(gc_buffer, table, n);

	return NULL;
}

static crex_object *crex_weakmap_clone_obj(crex_object *old_object)
{
	crex_object *new_object = crex_weakmap_create_object(crex_ce_weakmap);
	crex_weakmap *old_wm = crex_weakmap_from(old_object);
	crex_weakmap *new_wm = crex_weakmap_from(new_object);
	crex_hash_copy(&new_wm->ht, &old_wm->ht, NULL);

	crex_ulong obj_key;
	zval *val;
	CREX_HASH_MAP_FOREACH_NUM_KEY_VAL(&new_wm->ht, obj_key, val) {
		crex_weakref_register(
			crex_weakref_key_to_object(obj_key), CREX_WEAKREF_ENCODE(new_wm, CREX_WEAKREF_TAG_MAP));
		zval_add_ref(val);
	} CREX_HASH_FOREACH_END();
	return new_object;
}

static HashPosition *crex_weakmap_iterator_get_pos_ptr(crex_weakmap_iterator *iter) {
	CREX_ASSERT(iter->ht_iter != (uint32_t) -1);
	return &EG(ht_iterators)[iter->ht_iter].pos;
}

static void crex_weakmap_iterator_dtor(crex_object_iterator *obj_iter)
{
	crex_weakmap_iterator *iter = (crex_weakmap_iterator *) obj_iter;
	crex_hash_iterator_del(iter->ht_iter);
	zval_ptr_dtor(&iter->it.data);
}

static int crex_weakmap_iterator_valid(crex_object_iterator *obj_iter)
{
	crex_weakmap_iterator *iter = (crex_weakmap_iterator *) obj_iter;
	crex_weakmap *wm = crex_weakmap_fetch(&iter->it.data);
	HashPosition *pos = crex_weakmap_iterator_get_pos_ptr(iter);
	return crex_hash_has_more_elements_ex(&wm->ht, pos);
}

static zval *crex_weakmap_iterator_get_current_data(crex_object_iterator *obj_iter)
{
	crex_weakmap_iterator *iter = (crex_weakmap_iterator *) obj_iter;
	crex_weakmap *wm = crex_weakmap_fetch(&iter->it.data);
	HashPosition *pos = crex_weakmap_iterator_get_pos_ptr(iter);
	return crex_hash_get_current_data_ex(&wm->ht, pos);
}

static void crex_weakmap_iterator_get_current_key(crex_object_iterator *obj_iter, zval *key)
{
	crex_weakmap_iterator *iter = (crex_weakmap_iterator *) obj_iter;
	crex_weakmap *wm = crex_weakmap_fetch(&iter->it.data);
	HashPosition *pos = crex_weakmap_iterator_get_pos_ptr(iter);

	crex_string *string_key;
	crex_ulong num_key;
	int key_type = crex_hash_get_current_key_ex(&wm->ht, &string_key, &num_key, pos);
	if (key_type != HASH_KEY_IS_LONG) {
		CREX_ASSERT(0 && "Must have integer key");
	}

	ZVAL_OBJ_COPY(key, crex_weakref_key_to_object(num_key));
}

static void crex_weakmap_iterator_move_forward(crex_object_iterator *obj_iter)
{
	crex_weakmap_iterator *iter = (crex_weakmap_iterator *) obj_iter;
	crex_weakmap *wm = crex_weakmap_fetch(&iter->it.data);
	HashPosition *pos = crex_weakmap_iterator_get_pos_ptr(iter);
	crex_hash_move_forward_ex(&wm->ht, pos);
}

static void crex_weakmap_iterator_rewind(crex_object_iterator *obj_iter)
{
	crex_weakmap_iterator *iter = (crex_weakmap_iterator *) obj_iter;
	crex_weakmap *wm = crex_weakmap_fetch(&iter->it.data);
	HashPosition *pos = crex_weakmap_iterator_get_pos_ptr(iter);
	crex_hash_internal_pointer_reset_ex(&wm->ht, pos);
}

static const crex_object_iterator_funcs crex_weakmap_iterator_funcs = {
	crex_weakmap_iterator_dtor,
	crex_weakmap_iterator_valid,
	crex_weakmap_iterator_get_current_data,
	crex_weakmap_iterator_get_current_key,
	crex_weakmap_iterator_move_forward,
	crex_weakmap_iterator_rewind,
	NULL,
	NULL, /* get_gc */
};

/* by_ref is int due to Iterator API */
static crex_object_iterator *crex_weakmap_get_iterator(
		crex_class_entry *ce, zval *object, int by_ref)
{
	crex_weakmap *wm = crex_weakmap_fetch(object);
	crex_weakmap_iterator *iter = emalloc(sizeof(crex_weakmap_iterator));
	crex_iterator_init(&iter->it);
	iter->it.funcs = &crex_weakmap_iterator_funcs;
	ZVAL_COPY(&iter->it.data, object);
	iter->ht_iter = crex_hash_iterator_add(&wm->ht, 0);
	return &iter->it;
}

CREX_METHOD(WeakMap, offsetGet)
{
	zval *key;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "z", &key) == FAILURE) {
		return;
	}

	zval *zv = crex_weakmap_read_dimension(C_OBJ_P(CREX_THIS), key, BP_VAR_R, NULL);
	if (!zv) {
		return;
	}

	ZVAL_COPY(return_value, zv);
}

CREX_METHOD(WeakMap, offsetSet)
{
	zval *key, *value;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "zz", &key, &value) == FAILURE) {
		return;
	}

	crex_weakmap_write_dimension(C_OBJ_P(CREX_THIS), key, value);
}

CREX_METHOD(WeakMap, offsetExists)
{
	zval *key;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "z", &key) == FAILURE) {
		return;
	}

	RETURN_BOOL(crex_weakmap_has_dimension(C_OBJ_P(CREX_THIS), key, /* check_empty */ 0));
}

CREX_METHOD(WeakMap, offsetUnset)
{
	zval *key;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "z", &key) == FAILURE) {
		return;
	}

	crex_weakmap_unset_dimension(C_OBJ_P(CREX_THIS), key);
}

CREX_METHOD(WeakMap, count)
{
	if (crex_parse_parameters_none() == FAILURE) {
		return;
	}

	crex_long count;
	crex_weakmap_count_elements(C_OBJ_P(CREX_THIS), &count);
	RETURN_LONG(count);
}

CREX_METHOD(WeakMap, getIterator)
{
	if (crex_parse_parameters_none() == FAILURE) {
		return;
	}

	crex_create_internal_iterator_zval(return_value, CREX_THIS);
}

void crex_register_weakref_ce(void) /* {{{ */
{
	crex_ce_weakref = register_class_WeakReference();

	crex_ce_weakref->create_object = crex_weakref_new;
	crex_ce_weakref->default_object_handlers = &crex_weakref_handlers;

	memcpy(&crex_weakref_handlers, crex_get_std_object_handlers(), sizeof(crex_object_handlers));
	crex_weakref_handlers.offset = XtOffsetOf(crex_weakref, std);

	crex_weakref_handlers.free_obj = crex_weakref_free;
	crex_weakref_handlers.clone_obj = NULL;

	crex_ce_weakmap = register_class_WeakMap(crex_ce_arrayaccess, crex_ce_countable, crex_ce_aggregate);

	crex_ce_weakmap->create_object = crex_weakmap_create_object;
	crex_ce_weakmap->get_iterator = crex_weakmap_get_iterator;
	crex_ce_weakmap->default_object_handlers = &crex_weakmap_handlers;

	memcpy(&crex_weakmap_handlers, crex_get_std_object_handlers(), sizeof(crex_object_handlers));
	crex_weakmap_handlers.offset = XtOffsetOf(crex_weakmap, std);
	crex_weakmap_handlers.free_obj = crex_weakmap_free_obj;
	crex_weakmap_handlers.read_dimension = crex_weakmap_read_dimension;
	crex_weakmap_handlers.write_dimension = crex_weakmap_write_dimension;
	crex_weakmap_handlers.has_dimension = crex_weakmap_has_dimension;
	crex_weakmap_handlers.unset_dimension = crex_weakmap_unset_dimension;
	crex_weakmap_handlers.count_elements = crex_weakmap_count_elements;
	crex_weakmap_handlers.get_properties_for = crex_weakmap_get_properties_for;
	crex_weakmap_handlers.get_gc = crex_weakmap_get_gc;
	crex_weakmap_handlers.clone_obj = crex_weakmap_clone_obj;
}
/* }}} */

