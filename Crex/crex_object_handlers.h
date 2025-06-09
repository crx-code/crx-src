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

#ifndef CREX_OBJECT_HANDLERS_H
#define CREX_OBJECT_HANDLERS_H

#include <stdint.h>

struct _crex_property_info;

#define CREX_WRONG_PROPERTY_INFO \
	((struct _crex_property_info*)((intptr_t)-1))

#define CREX_DYNAMIC_PROPERTY_OFFSET               ((uintptr_t)(intptr_t)(-1))

#define IS_VALID_PROPERTY_OFFSET(offset)           ((intptr_t)(offset) > 0)
#define IS_WRONG_PROPERTY_OFFSET(offset)           ((intptr_t)(offset) == 0)
#define IS_DYNAMIC_PROPERTY_OFFSET(offset)         ((intptr_t)(offset) < 0)

#define IS_UNKNOWN_DYNAMIC_PROPERTY_OFFSET(offset) (offset == CREX_DYNAMIC_PROPERTY_OFFSET)
#define CREX_DECODE_DYN_PROP_OFFSET(offset)        ((uintptr_t)(-(intptr_t)(offset) - 2))
#define CREX_ENCODE_DYN_PROP_OFFSET(offset)        ((uintptr_t)(-((intptr_t)(offset) + 2)))


/* Used to fetch property from the object, read-only */
typedef zval *(*crex_object_read_property_t)(crex_object *object, crex_string *member, int type, void **cache_slot, zval *rv);

/* Used to fetch dimension from the object, read-only */
typedef zval *(*crex_object_read_dimension_t)(crex_object *object, zval *offset, int type, zval *rv);


/* Used to set property of the object
   You must return the final value of the assigned property.
*/
typedef zval *(*crex_object_write_property_t)(crex_object *object, crex_string *member, zval *value, void **cache_slot);

/* Used to set dimension of the object */
typedef void (*crex_object_write_dimension_t)(crex_object *object, zval *offset, zval *value);


/* Used to create pointer to the property of the object, for future direct r/w access.
 * May return one of:
 *  * A zval pointer, without incrementing the reference count.
 *  * &EG(error_zval), if an exception has been thrown.
 *  * NULL, if acquiring a direct pointer is not possible.
 *    In this case, the VM will fall back to using read_property and write_property.
 */
typedef zval *(*crex_object_get_property_ptr_ptr_t)(crex_object *object, crex_string *member, int type, void **cache_slot);

/* Used to check if a property of the object exists */
/* param has_set_exists:
 * 0 (has) whether property exists and is not NULL
 * 1 (set) whether property exists and is true
 * 2 (exists) whether property exists
 */
typedef int (*crex_object_has_property_t)(crex_object *object, crex_string *member, int has_set_exists, void **cache_slot);

/* Used to check if a dimension of the object exists */
typedef int (*crex_object_has_dimension_t)(crex_object *object, zval *member, int check_empty);

/* Used to remove a property of the object */
typedef void (*crex_object_unset_property_t)(crex_object *object, crex_string *member, void **cache_slot);

/* Used to remove a dimension of the object */
typedef void (*crex_object_unset_dimension_t)(crex_object *object, zval *offset);

/* Used to get hash of the properties of the object, as hash of zval's */
typedef HashTable *(*crex_object_get_properties_t)(crex_object *object);

typedef HashTable *(*crex_object_get_debug_info_t)(crex_object *object, int *is_temp);

typedef enum _crex_prop_purpose {
	/* Used for debugging. Supersedes get_debug_info handler. */
	CREX_PROP_PURPOSE_DEBUG,
	/* Used for (array) casts. */
	CREX_PROP_PURPOSE_ARRAY_CAST,
	/* Used for serialization using the "O" scheme.
	 * Unserialization will use __wakeup(). */
	CREX_PROP_PURPOSE_SERIALIZE,
	/* Used for var_export().
	 * The data will be passed to __set_state() when evaluated. */
	CREX_PROP_PURPOSE_VAR_EXPORT,
	/* Used for json_encode(). */
	CREX_PROP_PURPOSE_JSON,
	/* Dummy member to ensure that "default" is specified. */
	_CREX_PROP_PURPOSE_NON_EXHAUSTIVE_ENUM
} crex_prop_purpose;

/* The return value must be released using crex_release_properties(). */
typedef crex_array *(*crex_object_get_properties_for_t)(crex_object *object, crex_prop_purpose purpose);

/* Used to call methods */
/* args on stack! */
/* Andi - EX(fbc) (function being called) needs to be initialized already in the INIT fcall opcode so that the parameters can be parsed the right way. We need to add another callback for this.
 */
typedef crex_function *(*crex_object_get_method_t)(crex_object **object, crex_string *method, const zval *key);
typedef crex_function *(*crex_object_get_constructor_t)(crex_object *object);

/* free_obj should release any resources the object holds, without freeing the
 * object structure itself. The object does not need to be in a valid state after
 * free_obj finishes running.
 *
 * free_obj will always be invoked, even if the object leaks or a fatal error
 * occurs. However, during shutdown it may be called once the executor is no
 * longer active, in which case execution of user code may be skipped.
 */
typedef void (*crex_object_free_obj_t)(crex_object *object);

/* dtor_obj is called before free_obj. The object must remain in a valid state
 * after dtor_obj finishes running. Unlike free_obj, it is run prior to
 * deactivation of the executor during shutdown, which allows user code to run.
 *
 * This handler is not guaranteed to be called (e.g. on fatal error), and as
 * such should not be used to release resources or deallocate memory. Furthermore,
 * releasing resources in this handler can break detection of memory leaks, as
 * cycles may be broken early.
 *
 * dtor_obj should be used *only* to call user destruction hooks, such as __destruct.
 */
typedef void (*crex_object_dtor_obj_t)(crex_object *object);

typedef crex_object* (*crex_object_clone_obj_t)(crex_object *object);

/* Get class name for display in var_dump and other debugging functions.
 * Must be defined and must return a non-NULL value. */
typedef crex_string *(*crex_object_get_class_name_t)(const crex_object *object);

typedef int (*crex_object_compare_t)(zval *object1, zval *object2);

/* Cast an object to some other type.
 * readobj and retval must point to distinct zvals.
 */
typedef crex_result (*crex_object_cast_t)(crex_object *readobj, zval *retval, int type);

/* updates *count to hold the number of elements present and returns SUCCESS.
 * Returns FAILURE if the object does not have any sense of overloaded dimensions */
typedef crex_result (*crex_object_count_elements_t)(crex_object *object, crex_long *count);

typedef crex_result (*crex_object_get_closure_t)(crex_object *obj, crex_class_entry **ce_ptr, crex_function **fptr_ptr, crex_object **obj_ptr, bool check_only);

typedef HashTable *(*crex_object_get_gc_t)(crex_object *object, zval **table, int *n);

typedef crex_result (*crex_object_do_operation_t)(uint8_t opcode, zval *result, zval *op1, zval *op2);

struct _crex_object_handlers {
	/* offset of real object header (usually zero) */
	int										offset;
	/* object handlers */
	crex_object_free_obj_t					free_obj;             /* required */
	crex_object_dtor_obj_t					dtor_obj;             /* required */
	crex_object_clone_obj_t					clone_obj;            /* optional */
	crex_object_read_property_t				read_property;        /* required */
	crex_object_write_property_t			write_property;       /* required */
	crex_object_read_dimension_t			read_dimension;       /* required */
	crex_object_write_dimension_t			write_dimension;      /* required */
	crex_object_get_property_ptr_ptr_t		get_property_ptr_ptr; /* required */
	crex_object_has_property_t				has_property;         /* required */
	crex_object_unset_property_t			unset_property;       /* required */
	crex_object_has_dimension_t				has_dimension;        /* required */
	crex_object_unset_dimension_t			unset_dimension;      /* required */
	crex_object_get_properties_t			get_properties;       /* required */
	crex_object_get_method_t				get_method;           /* required */
	crex_object_get_constructor_t			get_constructor;      /* required */
	crex_object_get_class_name_t			get_class_name;       /* required */
	crex_object_cast_t						cast_object;          /* required */
	crex_object_count_elements_t			count_elements;       /* optional */
	crex_object_get_debug_info_t			get_debug_info;       /* optional */
	crex_object_get_closure_t				get_closure;          /* optional */
	crex_object_get_gc_t					get_gc;               /* required */
	crex_object_do_operation_t				do_operation;         /* optional */
	crex_object_compare_t					compare;              /* required */
	crex_object_get_properties_for_t		get_properties_for;   /* optional */
};

BEGIN_EXTERN_C()
extern const CREX_API crex_object_handlers std_object_handlers;

#define crex_get_std_object_handlers() \
	(&std_object_handlers)

#define crex_get_function_root_class(fbc) \
	((fbc)->common.prototype ? (fbc)->common.prototype->common.scope : (fbc)->common.scope)

#define CREX_PROPERTY_ISSET     0x0          /* Property exists and is not NULL */
#define CREX_PROPERTY_NOT_EMPTY CREX_ISEMPTY /* Property is not empty */
#define CREX_PROPERTY_EXISTS    0x2          /* Property exists */

CREX_API void crex_class_init_statics(crex_class_entry *ce);
CREX_API crex_function *crex_std_get_static_method(crex_class_entry *ce, crex_string *function_name_strval, const zval *key);
CREX_API zval *crex_std_get_static_property_with_info(crex_class_entry *ce, crex_string *property_name, int type, struct _crex_property_info **prop_info);
CREX_API zval *crex_std_get_static_property(crex_class_entry *ce, crex_string *property_name, int type);
CREX_API CREX_COLD bool crex_std_unset_static_property(crex_class_entry *ce, crex_string *property_name);
CREX_API crex_function *crex_std_get_constructor(crex_object *object);
CREX_API struct _crex_property_info *crex_get_property_info(const crex_class_entry *ce, crex_string *member, int silent);
CREX_API HashTable *crex_std_get_properties(crex_object *object);
CREX_API HashTable *crex_std_get_gc(crex_object *object, zval **table, int *n);
CREX_API HashTable *crex_std_get_debug_info(crex_object *object, int *is_temp);
CREX_API crex_result crex_std_cast_object_tostring(crex_object *object, zval *writeobj, int type);
CREX_API zval *crex_std_get_property_ptr_ptr(crex_object *object, crex_string *member, int type, void **cache_slot);
CREX_API zval *crex_std_read_property(crex_object *object, crex_string *member, int type, void **cache_slot, zval *rv);
CREX_API zval *crex_std_write_property(crex_object *object, crex_string *member, zval *value, void **cache_slot);
CREX_API int crex_std_has_property(crex_object *object, crex_string *member, int has_set_exists, void **cache_slot);
CREX_API void crex_std_unset_property(crex_object *object, crex_string *member, void **cache_slot);
CREX_API zval *crex_std_read_dimension(crex_object *object, zval *offset, int type, zval *rv);
CREX_API void crex_std_write_dimension(crex_object *object, zval *offset, zval *value);
CREX_API int crex_std_has_dimension(crex_object *object, zval *offset, int check_empty);
CREX_API void crex_std_unset_dimension(crex_object *object, zval *offset);
CREX_API crex_function *crex_std_get_method(crex_object **obj_ptr, crex_string *method_name, const zval *key);
CREX_API crex_string *crex_std_get_class_name(const crex_object *zobj);
CREX_API int crex_std_compare_objects(zval *o1, zval *o2);
CREX_API crex_result crex_std_get_closure(crex_object *obj, crex_class_entry **ce_ptr, crex_function **fptr_ptr, crex_object **obj_ptr, bool check_only);
CREX_API void rebuild_object_properties(crex_object *zobj);

CREX_API HashTable *crex_std_build_object_properties_array(crex_object *zobj);

/* Handler for objects that cannot be meaningfully compared.
 * Only objects with the same identity will be considered equal. */
CREX_API int crex_objects_not_comparable(zval *o1, zval *o2);

CREX_API bool crex_check_protected(const crex_class_entry *ce, const crex_class_entry *scope);

CREX_API crex_result crex_check_property_access(const crex_object *zobj, crex_string *prop_info_name, bool is_dynamic);

CREX_API crex_function *crex_get_call_trampoline_func(const crex_class_entry *ce, crex_string *method_name, bool is_static);

CREX_API uint32_t *crex_get_property_guard(crex_object *zobj, crex_string *member);

CREX_API uint32_t *crex_get_property_guard(crex_object *zobj, crex_string *member);

CREX_API uint32_t *crex_get_recursion_guard(crex_object *zobj);

/* Default behavior for get_properties_for. For use as a fallback in custom
 * get_properties_for implementations. */
CREX_API HashTable *crex_std_get_properties_for(crex_object *obj, crex_prop_purpose purpose);

/* Will call get_properties_for handler or use default behavior. For use by
 * consumers of the get_properties_for API. */
CREX_API HashTable *crex_get_properties_for(zval *obj, crex_prop_purpose purpose);

#define crex_release_properties(ht) do { \
	if ((ht) && !(GC_FLAGS(ht) & GC_IMMUTABLE) && !GC_DELREF(ht)) { \
		crex_array_destroy(ht); \
	} \
} while (0)

#define crex_free_trampoline(func) do { \
		if ((func) == &EG(trampoline)) { \
			EG(trampoline).common.function_name = NULL; \
		} else { \
			efree(func); \
		} \
	} while (0)

/* Fallback to default comparison implementation if the arguments aren't both objects
 * and have the same compare() handler. You'll likely want to use this unless you
 * explicitly wish to support comparisons between objects and non-objects. */
#define CREX_COMPARE_OBJECTS_FALLBACK(op1, op2) \
	if (C_TYPE_P(op1) != IS_OBJECT || \
			C_TYPE_P(op2) != IS_OBJECT || \
			C_OBJ_HT_P(op1)->compare != C_OBJ_HT_P(op2)->compare) { \
		return crex_std_compare_objects(op1, op2); \
	}

END_EXTERN_C()

#endif
