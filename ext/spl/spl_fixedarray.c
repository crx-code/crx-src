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
  | Author: Antony Dovgal <tony@daylessday.org>                          |
  |         Etienne Kneuss <colder@crx.net>                              |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "crex_exceptions.h"

#include "crx_spl.h"
#include "spl_fixedarray_arginfo.h"
#include "spl_functions.h"
#include "spl_engine.h"
#include "spl_fixedarray.h"
#include "spl_exceptions.h"
#include "spl_iterators.h"
#include "ext/json/crx_json.h"

crex_object_handlers spl_handler_SplFixedArray;
CRXAPI crex_class_entry *spl_ce_SplFixedArray;

#ifdef COMPILE_DL_SPL_FIXEDARRAY
CREX_GET_MODULE(spl_fixedarray)
#endif

/* Check if the object is an instance of a subclass of SplFixedArray that overrides method's implementation.
 * Expect subclassing SplFixedArray to be rare and check that first. */
#define HAS_FIXEDARRAY_ARRAYACCESS_OVERRIDE(object, method) UNEXPECTED((object)->ce != spl_ce_SplFixedArray && (object)->ce->arrayaccess_funcs_ptr->method->common.scope != spl_ce_SplFixedArray)

typedef struct _spl_fixedarray {
	crex_long size;
	/* It is possible to resize this, so this can't be combined with the object */
	zval *elements;
	/* If positive, it's a resize within a resize and the value gives the desired size. If -1, it's not. */
	crex_long cached_resize;
} spl_fixedarray;

typedef struct _spl_fixedarray_object {
	spl_fixedarray          array;
	crex_function          *fptr_count;
	crex_object             std;
} spl_fixedarray_object;

typedef struct _spl_fixedarray_it {
	crex_object_iterator intern;
	crex_long            current;
} spl_fixedarray_it;

static spl_fixedarray_object *spl_fixed_array_from_obj(crex_object *obj)
{
	return (spl_fixedarray_object*)((char*)(obj) - XtOffsetOf(spl_fixedarray_object, std));
}

#define C_SPLFIXEDARRAY_P(zv)  spl_fixed_array_from_obj(C_OBJ_P((zv)))

/* Helps enforce the invariants in debug mode:
 *   - if size == 0, then elements == NULL
 *   - if size > 0, then elements != NULL
 *   - size is not less than 0
 */
static bool spl_fixedarray_empty(spl_fixedarray *array)
{
	if (array->elements) {
		CREX_ASSERT(array->size > 0);
		return false;
	}
	CREX_ASSERT(array->size == 0);
	return true;
}

static void spl_fixedarray_default_ctor(spl_fixedarray *array)
{
	array->size = 0;
	array->elements = NULL;
}

/* Initializes the range [from, to) to null. Does not dtor existing elements. */
static void spl_fixedarray_init_elems(spl_fixedarray *array, crex_long from, crex_long to)
{
	CREX_ASSERT(from <= to);
	zval *begin = array->elements + from, *end = array->elements + to;

	while (begin != end) {
		ZVAL_NULL(begin++);
	}
}

static void spl_fixedarray_init_non_empty_struct(spl_fixedarray *array, crex_long size)
{
	array->size = 0; /* reset size in case ecalloc() fails */
	array->elements = size ? safe_emalloc(size, sizeof(zval), 0) : NULL;
	array->size = size;
}

static void spl_fixedarray_init(spl_fixedarray *array, crex_long size)
{
	if (size > 0) {
		spl_fixedarray_init_non_empty_struct(array, size);
		spl_fixedarray_init_elems(array, 0, size);
	} else {
		spl_fixedarray_default_ctor(array);
	}
	array->cached_resize = -1;
}

/* Copies the range [begin, end) into the fixedarray, beginning at `offset`.
 * Does not dtor the existing elements.
 */
static void spl_fixedarray_copy_range(spl_fixedarray *array, crex_long offset, zval *begin, zval *end)
{
	CREX_ASSERT(offset >= 0);
	CREX_ASSERT(array->size - offset >= end - begin);

	zval *to = &array->elements[offset];
	while (begin != end) {
		ZVAL_COPY(to++, begin++);
	}
}

static void spl_fixedarray_copy_ctor(spl_fixedarray *to, spl_fixedarray *from)
{
	crex_long size = from->size;
	spl_fixedarray_init(to, size);
	if (size != 0) {
		zval *begin = from->elements, *end = from->elements + size;
		spl_fixedarray_copy_range(to, 0, begin, end);
	}
}

/* Destructs the elements in the range [from, to).
 * Caller is expected to bounds check.
 */
static void spl_fixedarray_dtor_range(spl_fixedarray *array, crex_long from, crex_long to)
{
	array->size = from;
	zval *begin = array->elements + from, *end = array->elements + to;
	while (begin != end) {
		zval_ptr_dtor(begin++);
	}
}

/* Destructs and frees contents but not the array itself.
 * If you want to re-use the array then you need to re-initialize it.
 */
static void spl_fixedarray_dtor(spl_fixedarray *array)
{
	if (!spl_fixedarray_empty(array)) {
		zval *begin = array->elements, *end = array->elements + array->size;
		array->elements = NULL;
		array->size = 0;
		while (begin != end) {
			zval_ptr_dtor(--end);
		}
		efree(begin);
	}
}

static void spl_fixedarray_resize(spl_fixedarray *array, crex_long size)
{
	if (size == array->size) {
		/* nothing to do */
		return;
	}

	/* first initialization */
	if (array->size == 0) {
		spl_fixedarray_init(array, size);
		return;
	}

	if (UNEXPECTED(array->cached_resize >= 0)) {
		/* We're already resizing, so just remember the desired size.
		 * The resize will happen later. */
		array->cached_resize = size;
		return;
	}
	array->cached_resize = size;

	/* clearing the array */
	if (size == 0) {
		spl_fixedarray_dtor(array);
		array->elements = NULL;
		array->size = 0;
	} else if (size > array->size) {
		array->elements = safe_erealloc(array->elements, size, sizeof(zval), 0);
		spl_fixedarray_init_elems(array, array->size, size);
		array->size = size;
	} else { /* size < array->size */
		/* Size set in spl_fixedarray_dtor_range() */
		spl_fixedarray_dtor_range(array, size, array->size);
		array->elements = erealloc(array->elements, sizeof(zval) * size);
	}

	/* If resized within the destructor, take the last resize command and perform it */
	crex_long cached_resize = array->cached_resize;
	array->cached_resize = -1;
	if (cached_resize != size) {
		spl_fixedarray_resize(array, cached_resize);
	}
}

static HashTable* spl_fixedarray_object_get_gc(crex_object *obj, zval **table, int *n)
{
	spl_fixedarray_object *intern = spl_fixed_array_from_obj(obj);
	HashTable *ht = crex_std_get_properties(obj);

	*table = intern->array.elements;
	*n = (int)intern->array.size;

	return ht;
}

static HashTable* spl_fixedarray_object_get_properties_for(crex_object *obj, crex_prop_purpose purpose)
{
	/* This has __serialize, so the purpose is not CREX_PROP_PURPOSE_SERIALIZE, which would expect a non-null return value */
	CREX_ASSERT(purpose != CREX_PROP_PURPOSE_SERIALIZE);

	const spl_fixedarray_object *intern = spl_fixed_array_from_obj(obj);
	/*
	 * SplFixedArray can be subclassed or have dynamic properties (With or without AllowDynamicProperties in subclasses).
	 * Instances of subclasses with declared properties may have properties but not yet have a property table.
	 */
	HashTable *source_properties = obj->properties ? obj->properties : (obj->ce->default_properties_count ? crex_std_get_properties(obj) : NULL);

	const crex_long size = intern->array.size;
	if (size == 0 && (!source_properties || !crex_hash_num_elements(source_properties))) {
		return NULL;
	}
	zval *const elements = intern->array.elements;
	HashTable *ht = crex_new_array(size);

	for (crex_long i = 0; i < size; i++) {
		C_TRY_ADDREF_P(&elements[i]);
		crex_hash_next_index_insert(ht, &elements[i]);
	}
	if (source_properties && crex_hash_num_elements(source_properties) > 0) {
		crex_long nkey;
		crex_string *skey;
		zval *value;
		CREX_HASH_MAP_FOREACH_KEY_VAL_IND(source_properties, nkey, skey, value) {
			C_TRY_ADDREF_P(value);
			if (skey) {
				crex_hash_add_new(ht, skey, value);
			} else {
				crex_hash_index_update(ht, nkey, value);
			}
		} CREX_HASH_FOREACH_END();
	}

	return ht;
}

static void spl_fixedarray_object_free_storage(crex_object *object)
{
	spl_fixedarray_object *intern = spl_fixed_array_from_obj(object);
	spl_fixedarray_dtor(&intern->array);
	crex_object_std_dtor(&intern->std);
}

static crex_object *spl_fixedarray_object_new_ex(crex_class_entry *class_type, crex_object *orig, bool clone_orig)
{
	spl_fixedarray_object *intern;
	crex_class_entry      *parent = class_type;
	bool                   inherited = false;

	intern = crex_object_alloc(sizeof(spl_fixedarray_object), parent);

	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	if (orig && clone_orig) {
		spl_fixedarray_object *other = spl_fixed_array_from_obj(orig);
		spl_fixedarray_copy_ctor(&intern->array, &other->array);
	}

	while (parent) {
		if (parent == spl_ce_SplFixedArray) {
			break;
		}

		parent = parent->parent;
		inherited = true;
	}

	CREX_ASSERT(parent);

	if (UNEXPECTED(inherited)) {
		/* Find count() method */
		crex_function *fptr_count = crex_hash_find_ptr(&class_type->function_table, ZSTR_KNOWN(CREX_STR_COUNT));
		if (fptr_count->common.scope == parent) {
			fptr_count = NULL;
		}
		intern->fptr_count = fptr_count;
	}

	return &intern->std;
}

static crex_object *spl_fixedarray_new(crex_class_entry *class_type)
{
	return spl_fixedarray_object_new_ex(class_type, NULL, 0);
}

static crex_object *spl_fixedarray_object_clone(crex_object *old_object)
{
	crex_object *new_object = spl_fixedarray_object_new_ex(old_object->ce, old_object, 1);

	crex_objects_clone_members(new_object, old_object);

	return new_object;
}

static crex_long spl_offset_convert_to_long(zval *offset) /* {{{ */
{
	try_again:
	switch (C_TYPE_P(offset)) {
		case IS_STRING: {
			crex_ulong index;
			if (CREX_HANDLE_NUMERIC(C_STR_P(offset), index)) {
				return (crex_long) index;
			}
			break;
		}
		case IS_DOUBLE:
			return crex_dval_to_lval_safe(C_DVAL_P(offset));
		case IS_LONG:
			return C_LVAL_P(offset);
		case IS_FALSE:
			return 0;
		case IS_TRUE:
			return 1;
		case IS_REFERENCE:
			offset = C_REFVAL_P(offset);
			goto try_again;
		case IS_RESOURCE:
			crex_use_resource_as_offset(offset);
			return C_RES_HANDLE_P(offset);
	}

	/* Use SplFixedArray name from the CE */
	crex_illegal_container_offset(spl_ce_SplFixedArray->name, offset, BP_VAR_R);
	return 0;
}

static zval *spl_fixedarray_object_read_dimension_helper(spl_fixedarray_object *intern, zval *offset)
{
	crex_long index;

	/* we have to return NULL on error here to avoid memleak because of
	 * ZE duplicating uninitialized_zval_ptr */
	if (!offset) {
		crex_throw_error(NULL, "[] operator not supported for SplFixedArray");
		return NULL;
	}

	index = spl_offset_convert_to_long(offset);
	if (EG(exception)) {
		return NULL;
	}

	if (index < 0 || index >= intern->array.size) {
		// TODO Change error message and use OutOfBound SPL Exception?
		crex_throw_exception(spl_ce_RuntimeException, "Index invalid or out of range", 0);
		return NULL;
	} else {
		return &intern->array.elements[index];
	}
}

static int spl_fixedarray_object_has_dimension(crex_object *object, zval *offset, int check_empty);

static zval *spl_fixedarray_object_read_dimension(crex_object *object, zval *offset, int type, zval *rv)
{
	if (type == BP_VAR_IS && !spl_fixedarray_object_has_dimension(object, offset, 0)) {
		return &EG(uninitialized_zval);
	}

	if (HAS_FIXEDARRAY_ARRAYACCESS_OVERRIDE(object, zf_offsetget)) {
		zval tmp;
		if (!offset) {
			ZVAL_NULL(&tmp);
			offset = &tmp;
		}
		crex_call_known_instance_method_with_1_params(object->ce->arrayaccess_funcs_ptr->zf_offsetget, object, rv, offset);
		if (!C_ISUNDEF_P(rv)) {
			return rv;
		}
		return &EG(uninitialized_zval);
	}

	spl_fixedarray_object *intern = spl_fixed_array_from_obj(object);
	return spl_fixedarray_object_read_dimension_helper(intern, offset);
}

static void spl_fixedarray_object_write_dimension_helper(spl_fixedarray_object *intern, zval *offset, zval *value)
{
	crex_long index;

	if (!offset) {
		/* '$array[] = value' syntax is not supported */
		crex_throw_error(NULL, "[] operator not supported for SplFixedArray");
		return;
	}

	index = spl_offset_convert_to_long(offset);
	if (EG(exception)) {
		return;
	}

	if (index < 0 || index >= intern->array.size) {
		// TODO Change error message and use OutOfBound SPL Exception?
		crex_throw_exception(spl_ce_RuntimeException, "Index invalid or out of range", 0);
		return;
	} else {
		/* Fix #81429 */
		zval *ptr = &(intern->array.elements[index]);
		zval tmp;
		ZVAL_COPY_VALUE(&tmp, ptr);
		ZVAL_COPY_DEREF(ptr, value);
		zval_ptr_dtor(&tmp);
	}
}

static void spl_fixedarray_object_write_dimension(crex_object *object, zval *offset, zval *value)
{
	if (HAS_FIXEDARRAY_ARRAYACCESS_OVERRIDE(object, zf_offsetset)) {
		zval tmp;

		if (!offset) {
			ZVAL_NULL(&tmp);
			offset = &tmp;
		}
		crex_call_known_instance_method_with_2_params(object->ce->arrayaccess_funcs_ptr->zf_offsetset, object, NULL, offset, value);
		return;
	}

	spl_fixedarray_object *intern = spl_fixed_array_from_obj(object);
	spl_fixedarray_object_write_dimension_helper(intern, offset, value);
}

static void spl_fixedarray_object_unset_dimension_helper(spl_fixedarray_object *intern, zval *offset)
{
	crex_long index;

	index = spl_offset_convert_to_long(offset);
	if (EG(exception)) {
		return;
	}

	if (index < 0 || index >= intern->array.size) {
		// TODO Change error message and use OutOfBound SPL Exception?
		crex_throw_exception(spl_ce_RuntimeException, "Index invalid or out of range", 0);
		return;
	} else {
		zval_ptr_dtor(&(intern->array.elements[index]));
		ZVAL_NULL(&intern->array.elements[index]);
	}
}

static void spl_fixedarray_object_unset_dimension(crex_object *object, zval *offset)
{
	if (UNEXPECTED(HAS_FIXEDARRAY_ARRAYACCESS_OVERRIDE(object, zf_offsetunset))) {
		crex_call_known_instance_method_with_1_params(object->ce->arrayaccess_funcs_ptr->zf_offsetunset, object, NULL, offset);
		return;
	}

	spl_fixedarray_object *intern = spl_fixed_array_from_obj(object);
	spl_fixedarray_object_unset_dimension_helper(intern, offset);
}

static bool spl_fixedarray_object_has_dimension_helper(spl_fixedarray_object *intern, zval *offset, bool check_empty)
{
	crex_long index;

	index = spl_offset_convert_to_long(offset);
	if (EG(exception)) {
		return false;
	}

	if (index < 0 || index >= intern->array.size) {
		return false;
	}

	if (check_empty) {
		return crex_is_true(&intern->array.elements[index]);
	}

	return C_TYPE(intern->array.elements[index]) != IS_NULL;
}

static int spl_fixedarray_object_has_dimension(crex_object *object, zval *offset, int check_empty)
{
	if (HAS_FIXEDARRAY_ARRAYACCESS_OVERRIDE(object, zf_offsetexists)) {
		zval rv;

		crex_call_known_instance_method_with_1_params(object->ce->arrayaccess_funcs_ptr->zf_offsetexists, object, &rv, offset);
		bool result = crex_is_true(&rv);
		zval_ptr_dtor(&rv);
		return result;
	}

	spl_fixedarray_object *intern = spl_fixed_array_from_obj(object);

	return spl_fixedarray_object_has_dimension_helper(intern, offset, check_empty);
}

static crex_result spl_fixedarray_object_count_elements(crex_object *object, crex_long *count)
{
	spl_fixedarray_object *intern;

	intern = spl_fixed_array_from_obj(object);
	if (UNEXPECTED(intern->fptr_count)) {
		zval rv;
		crex_call_known_instance_method_with_0_params(intern->fptr_count, object, &rv);
		if (!C_ISUNDEF(rv)) {
			*count = zval_get_long(&rv);
			zval_ptr_dtor(&rv);
		} else {
			*count = 0;
		}
	} else {
		*count = intern->array.size;
	}
	return SUCCESS;
}

CRX_METHOD(SplFixedArray, __main)
{
	zval *object = CREX_THIS;
	spl_fixedarray_object *intern;
	crex_long size = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|l", &size) == FAILURE) {
		RETURN_THROWS();
	}

	if (size < 0) {
		crex_argument_value_error(1, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	intern = C_SPLFIXEDARRAY_P(object);

	if (!spl_fixedarray_empty(&intern->array)) {
		/* called __main() twice, bail out */
		return;
	}

	spl_fixedarray_init(&intern->array, size);
}

CRX_METHOD(SplFixedArray, __wakeup)
{
	spl_fixedarray_object *intern = C_SPLFIXEDARRAY_P(CREX_THIS);
	HashTable *intern_ht = crex_std_get_properties(C_OBJ_P(CREX_THIS));
	zval *data;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (intern->array.size == 0) {
		int index = 0;
		int size = crex_hash_num_elements(intern_ht);

		spl_fixedarray_init(&intern->array, size);

		CREX_HASH_FOREACH_VAL(intern_ht, data) {
			ZVAL_COPY(&intern->array.elements[index], data);
			index++;
		} CREX_HASH_FOREACH_END();

		/* Remove the unserialised properties, since we now have the elements
		 * within the spl_fixedarray_object structure. */
		crex_hash_clean(intern_ht);
	}
}

CRX_METHOD(SplFixedArray, __serialize)
{
	spl_fixedarray_object *intern = C_SPLFIXEDARRAY_P(CREX_THIS);
	zval *current;
	crex_string *key;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	HashTable *ht = crex_std_get_properties(&intern->std);
	uint32_t num_properties = crex_hash_num_elements(ht);
	array_init_size(return_value, intern->array.size + num_properties);

	/* elements */
	for (crex_long i = 0; i < intern->array.size; i++) {
		current = &intern->array.elements[i];
		crex_hash_next_index_insert(C_ARRVAL_P(return_value), current);
		C_TRY_ADDREF_P(current);
	}

	/* members */
	CREX_HASH_FOREACH_STR_KEY_VAL_IND(ht, key, current) {
		/* If the properties table was already rebuild, it will also contain the
		 * array elements. The array elements are already added in the above loop.
		 * We can detect array elements by the fact that their key == NULL. */
		if (key != NULL) {
			crex_hash_add_new(C_ARRVAL_P(return_value), key, current);
			C_TRY_ADDREF_P(current);
		}
	} CREX_HASH_FOREACH_END();
}

CRX_METHOD(SplFixedArray, __unserialize)
{
	spl_fixedarray_object *intern = C_SPLFIXEDARRAY_P(CREX_THIS);
	HashTable *data;
	zval members_zv, *elem;
	crex_string *key;
	crex_long size;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "h", &data) == FAILURE) {
		RETURN_THROWS();
	}

	if (intern->array.size == 0) {
		size = crex_hash_num_elements(data);
		spl_fixedarray_init_non_empty_struct(&intern->array, size);
		if (!size) {
			return;
		}
		array_init(&members_zv);

		intern->array.size = 0;
		CREX_HASH_FOREACH_STR_KEY_VAL(data, key, elem) {
			if (key == NULL) {
				ZVAL_COPY(&intern->array.elements[intern->array.size], elem);
				intern->array.size++;
			} else {
				C_TRY_ADDREF_P(elem);
				crex_hash_add(C_ARRVAL(members_zv), key, elem);
			}
		} CREX_HASH_FOREACH_END();

		if (intern->array.size != size) {
			if (intern->array.size) {
				intern->array.elements = erealloc(intern->array.elements, sizeof(zval) * intern->array.size);
			} else {
				efree(intern->array.elements);
				intern->array.elements = NULL;
			}
		}

		object_properties_load(&intern->std, C_ARRVAL(members_zv));
		zval_ptr_dtor(&members_zv);
	}
}

CRX_METHOD(SplFixedArray, count)
{
	zval *object = CREX_THIS;
	spl_fixedarray_object *intern;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLFIXEDARRAY_P(object);
	RETURN_LONG(intern->array.size);
}

CRX_METHOD(SplFixedArray, toArray)
{
	spl_fixedarray_object *intern;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLFIXEDARRAY_P(CREX_THIS);

	if (!spl_fixedarray_empty(&intern->array)) {
		array_init(return_value);
		for (crex_long i = 0; i < intern->array.size; i++) {
			crex_hash_index_update(C_ARRVAL_P(return_value), i, &intern->array.elements[i]);
			C_TRY_ADDREF(intern->array.elements[i]);
		}
	} else {
		RETURN_EMPTY_ARRAY();
	}
}

CRX_METHOD(SplFixedArray, fromArray)
{
	zval *data;
	spl_fixedarray array;
	spl_fixedarray_object *intern;
	int num;
	bool save_indexes = 1;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "a|b", &data, &save_indexes) == FAILURE) {
		RETURN_THROWS();
	}

	num = crex_hash_num_elements(C_ARRVAL_P(data));

	if (num > 0 && save_indexes) {
		zval *element;
		crex_string *str_index;
		crex_ulong num_index, max_index = 0;
		crex_long tmp;

		CREX_HASH_FOREACH_KEY(C_ARRVAL_P(data), num_index, str_index) {
			if (str_index != NULL || (crex_long)num_index < 0) {
				crex_throw_exception_ex(spl_ce_InvalidArgumentException, 0, "array must contain only positive integer keys");
				RETURN_THROWS();
			}

			if (num_index > max_index) {
				max_index = num_index;
			}
		} CREX_HASH_FOREACH_END();

		tmp = max_index + 1;
		if (tmp <= 0) {
			crex_throw_exception_ex(spl_ce_InvalidArgumentException, 0, "integer overflow detected");
			RETURN_THROWS();
		}
		spl_fixedarray_init(&array, tmp);

		CREX_HASH_FOREACH_KEY_VAL(C_ARRVAL_P(data), num_index, str_index, element) {
			ZVAL_COPY_DEREF(&array.elements[num_index], element);
		} CREX_HASH_FOREACH_END();

	} else if (num > 0 && !save_indexes) {
		zval *element;
		crex_long i = 0;

		spl_fixedarray_init(&array, num);

		CREX_HASH_FOREACH_VAL(C_ARRVAL_P(data), element) {
			ZVAL_COPY_DEREF(&array.elements[i], element);
			i++;
		} CREX_HASH_FOREACH_END();
	} else {
		spl_fixedarray_init(&array, 0);
	}

	object_init_ex(return_value, spl_ce_SplFixedArray);

	intern = C_SPLFIXEDARRAY_P(return_value);
	intern->array = array;
}

CRX_METHOD(SplFixedArray, getSize)
{
	zval *object = CREX_THIS;
	spl_fixedarray_object *intern;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLFIXEDARRAY_P(object);
	RETURN_LONG(intern->array.size);
}

CRX_METHOD(SplFixedArray, setSize)
{
	zval *object = CREX_THIS;
	spl_fixedarray_object *intern;
	crex_long size;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &size) == FAILURE) {
		RETURN_THROWS();
	}

	if (size < 0) {
		crex_argument_value_error(1, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	intern = C_SPLFIXEDARRAY_P(object);

	spl_fixedarray_resize(&intern->array, size);
	RETURN_TRUE;
}

/* Returns whether the requested $index exists. */
CRX_METHOD(SplFixedArray, offsetExists)
{
	zval                  *zindex;
	spl_fixedarray_object  *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "z", &zindex) == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLFIXEDARRAY_P(CREX_THIS);

	RETURN_BOOL(spl_fixedarray_object_has_dimension_helper(intern, zindex, 0));
}

/* Returns the value at the specified $index. */
CRX_METHOD(SplFixedArray, offsetGet)
{
	zval *zindex, *value;
	spl_fixedarray_object  *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "z", &zindex) == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLFIXEDARRAY_P(CREX_THIS);
	value = spl_fixedarray_object_read_dimension_helper(intern, zindex);

	if (value) {
		RETURN_COPY_DEREF(value);
	} else {
		RETURN_NULL();
	}
}

/* Sets the value at the specified $index to $newval. */
CRX_METHOD(SplFixedArray, offsetSet)
{
	zval                  *zindex, *value;
	spl_fixedarray_object  *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "zz", &zindex, &value) == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLFIXEDARRAY_P(CREX_THIS);
	spl_fixedarray_object_write_dimension_helper(intern, zindex, value);

}

/* Unsets the value at the specified $index. */
CRX_METHOD(SplFixedArray, offsetUnset)
{
	zval                  *zindex;
	spl_fixedarray_object  *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "z", &zindex) == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLFIXEDARRAY_P(CREX_THIS);
	spl_fixedarray_object_unset_dimension_helper(intern, zindex);

}

/* Create a new iterator from a SplFixedArray instance. */
CRX_METHOD(SplFixedArray, getIterator)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	crex_create_internal_iterator_zval(return_value, CREX_THIS);
}

CRX_METHOD(SplFixedArray, jsonSerialize)
{
	CREX_PARSE_PARAMETERS_NONE();

	spl_fixedarray_object *intern = C_SPLFIXEDARRAY_P(CREX_THIS);
	array_init_size(return_value, intern->array.size);
	for (crex_long i = 0; i < intern->array.size; i++) {
		crex_hash_next_index_insert_new(C_ARR_P(return_value), &intern->array.elements[i]);
		C_TRY_ADDREF(intern->array.elements[i]);
	}
}

static void spl_fixedarray_it_dtor(crex_object_iterator *iter)
{
	zval_ptr_dtor(&iter->data);
}

static void spl_fixedarray_it_rewind(crex_object_iterator *iter)
{
	((spl_fixedarray_it*)iter)->current = 0;
}

static int spl_fixedarray_it_valid(crex_object_iterator *iter)
{
	spl_fixedarray_it     *iterator = (spl_fixedarray_it*)iter;
	spl_fixedarray_object *object   = C_SPLFIXEDARRAY_P(&iter->data);

	if (iterator->current >= 0 && iterator->current < object->array.size) {
		return SUCCESS;
	}

	return FAILURE;
}

static zval *spl_fixedarray_it_get_current_data(crex_object_iterator *iter)
{
	zval zindex, *data;
	spl_fixedarray_it     *iterator = (spl_fixedarray_it*)iter;
	spl_fixedarray_object *object   = C_SPLFIXEDARRAY_P(&iter->data);

	ZVAL_LONG(&zindex, iterator->current);
	data = spl_fixedarray_object_read_dimension_helper(object, &zindex);

	if (data == NULL) {
		data = &EG(uninitialized_zval);
	}
	return data;
}

static void spl_fixedarray_it_get_current_key(crex_object_iterator *iter, zval *key)
{
	ZVAL_LONG(key, ((spl_fixedarray_it*)iter)->current);
}

static void spl_fixedarray_it_move_forward(crex_object_iterator *iter)
{
	((spl_fixedarray_it*)iter)->current++;
}

/* iterator handler table */
static const crex_object_iterator_funcs spl_fixedarray_it_funcs = {
	spl_fixedarray_it_dtor,
	spl_fixedarray_it_valid,
	spl_fixedarray_it_get_current_data,
	spl_fixedarray_it_get_current_key,
	spl_fixedarray_it_move_forward,
	spl_fixedarray_it_rewind,
	NULL,
	NULL, /* get_gc */
};

static crex_object_iterator *spl_fixedarray_get_iterator(crex_class_entry *ce, zval *object, int by_ref)
{
	spl_fixedarray_it *iterator;

	if (by_ref) {
		crex_throw_error(NULL, "An iterator cannot be used with foreach by reference");
		return NULL;
	}

	iterator = emalloc(sizeof(spl_fixedarray_it));

	crex_iterator_init((crex_object_iterator*)iterator);

	ZVAL_OBJ_COPY(&iterator->intern.data, C_OBJ_P(object));
	iterator->intern.funcs = &spl_fixedarray_it_funcs;

	return &iterator->intern;
}

CRX_MINIT_FUNCTION(spl_fixedarray)
{
	spl_ce_SplFixedArray = register_class_SplFixedArray(
		crex_ce_aggregate, crex_ce_arrayaccess, crex_ce_countable, crx_json_serializable_ce);
	spl_ce_SplFixedArray->create_object = spl_fixedarray_new;
	spl_ce_SplFixedArray->default_object_handlers = &spl_handler_SplFixedArray;
	spl_ce_SplFixedArray->get_iterator = spl_fixedarray_get_iterator;

	memcpy(&spl_handler_SplFixedArray, &std_object_handlers, sizeof(crex_object_handlers));

	spl_handler_SplFixedArray.offset          = XtOffsetOf(spl_fixedarray_object, std);
	spl_handler_SplFixedArray.clone_obj       = spl_fixedarray_object_clone;
	spl_handler_SplFixedArray.read_dimension  = spl_fixedarray_object_read_dimension;
	spl_handler_SplFixedArray.write_dimension = spl_fixedarray_object_write_dimension;
	spl_handler_SplFixedArray.unset_dimension = spl_fixedarray_object_unset_dimension;
	spl_handler_SplFixedArray.has_dimension   = spl_fixedarray_object_has_dimension;
	spl_handler_SplFixedArray.count_elements  = spl_fixedarray_object_count_elements;
	spl_handler_SplFixedArray.get_properties_for = spl_fixedarray_object_get_properties_for;
	spl_handler_SplFixedArray.get_gc          = spl_fixedarray_object_get_gc;
	spl_handler_SplFixedArray.free_obj        = spl_fixedarray_object_free_storage;

	return SUCCESS;
}
