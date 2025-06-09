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

/* These iterators were designed to operate within the foreach()
 * structures provided by the engine, but could be extended for use
 * with other iterative engine opcodes.
 * These methods have similar semantics to the crex_hash API functions
 * with similar names.
 * */

typedef struct _crex_object_iterator crex_object_iterator;

typedef struct _crex_object_iterator_funcs {
	/* release all resources associated with this iterator instance */
	void (*dtor)(crex_object_iterator *iter);

	/* check for end of iteration (FAILURE or SUCCESS if data is valid) */
	int (*valid)(crex_object_iterator *iter);

	/* fetch the item data for the current element */
	zval *(*get_current_data)(crex_object_iterator *iter);

	/* fetch the key for the current element (optional, may be NULL). The key
	 * should be written into the provided zval* using the ZVAL_* macros. If
	 * this handler is not provided auto-incrementing integer keys will be
	 * used. */
	void (*get_current_key)(crex_object_iterator *iter, zval *key);

	/* step forwards to next element */
	void (*move_forward)(crex_object_iterator *iter);

	/* rewind to start of data (optional, may be NULL) */
	void (*rewind)(crex_object_iterator *iter);

	/* invalidate current value/key (optional, may be NULL) */
	void (*invalidate_current)(crex_object_iterator *iter);

	/* Expose owned values to GC.
	 * This has the same semantics as the corresponding object handler. */
	HashTable *(*get_gc)(crex_object_iterator *iter, zval **table, int *n);
} crex_object_iterator_funcs;

struct _crex_object_iterator {
	crex_object std;
	zval data;
	const crex_object_iterator_funcs *funcs;
	crex_ulong index; /* private to fe_reset/fe_fetch opcodes */
};

typedef struct _crex_class_iterator_funcs {
	crex_function *zf_new_iterator;
	crex_function *zf_valid;
	crex_function *zf_current;
	crex_function *zf_key;
	crex_function *zf_next;
	crex_function *zf_rewind;
} crex_class_iterator_funcs;

typedef struct _crex_class_arrayaccess_funcs {
	crex_function *zf_offsetget;
	crex_function *zf_offsetexists;
	crex_function *zf_offsetset;
	crex_function *zf_offsetunset;
} crex_class_arrayaccess_funcs;

BEGIN_EXTERN_C()
/* given a zval, returns stuff that can be used to iterate it. */
CREX_API crex_object_iterator* crex_iterator_unwrap(zval *array_ptr);

/* given an iterator, wrap it up as a zval for use by the engine opcodes */
CREX_API void crex_iterator_init(crex_object_iterator *iter);
CREX_API void crex_iterator_dtor(crex_object_iterator *iter);

CREX_API void crex_register_iterator_wrapper(void);
END_EXTERN_C()
