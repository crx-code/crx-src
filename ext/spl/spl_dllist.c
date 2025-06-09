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
   | Authors: Etienne Kneuss <colder@crx.net>                             |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "crx.h"
#include "crex_exceptions.h"
#include "crex_hash.h"

#include "crx_spl.h"
#include "ext/standard/info.h"
#include "ext/standard/crx_var.h"
#include "crex_smart_str.h"
#include "spl_functions.h"
#include "spl_engine.h"
#include "spl_iterators.h"
#include "spl_dllist.h"
#include "spl_dllist_arginfo.h"
#include "spl_exceptions.h"

crex_object_handlers spl_handler_SplDoublyLinkedList;
CRXAPI crex_class_entry  *spl_ce_SplDoublyLinkedList;
CRXAPI crex_class_entry  *spl_ce_SplQueue;
CRXAPI crex_class_entry  *spl_ce_SplStack;

#define SPL_LLIST_RC(elem) C_EXTRA((elem)->data)

#define SPL_LLIST_DELREF(elem) if (!--SPL_LLIST_RC(elem)) { \
	efree(elem); \
}

#define SPL_LLIST_CHECK_DELREF(elem) if ((elem) && !--SPL_LLIST_RC(elem)) { \
	efree(elem); \
}

#define SPL_LLIST_ADDREF(elem) SPL_LLIST_RC(elem)++
#define SPL_LLIST_CHECK_ADDREF(elem) if (elem) SPL_LLIST_RC(elem)++

#ifdef accept
#undef accept
#endif

typedef struct _spl_ptr_llist_element {
	struct _spl_ptr_llist_element *prev;
	struct _spl_ptr_llist_element *next;
	zval                           data;
} spl_ptr_llist_element;

typedef struct _spl_ptr_llist {
	spl_ptr_llist_element *head;
	spl_ptr_llist_element *tail;
	int count;
} spl_ptr_llist;

typedef struct _spl_dllist_object spl_dllist_object;
typedef struct _spl_dllist_it spl_dllist_it;

struct _spl_dllist_object {
	spl_ptr_llist         *llist;
	spl_ptr_llist_element *traverse_pointer;
	int                    traverse_position;
	int                    flags;
	crex_function         *fptr_offset_get;
	crex_function         *fptr_offset_set;
	crex_function         *fptr_offset_has;
	crex_function         *fptr_offset_del;
	crex_function         *fptr_count;
	crex_class_entry      *ce_get_iterator;
	crex_object            std;
};

/* define an overloaded iterator structure */
struct _spl_dllist_it {
	crex_object_iterator   intern;
	spl_ptr_llist_element *traverse_pointer;
	int                    traverse_position;
	int                    flags;
};

static inline spl_dllist_object *spl_dllist_from_obj(crex_object *obj) /* {{{ */ {
	return (spl_dllist_object*)((char*)(obj) - XtOffsetOf(spl_dllist_object, std));
}
/* }}} */

#define C_SPLDLLIST_P(zv)  spl_dllist_from_obj(C_OBJ_P((zv)))

static spl_ptr_llist *spl_ptr_llist_init(void) /* {{{ */
{
	spl_ptr_llist *llist = emalloc(sizeof(spl_ptr_llist));

	llist->head  = NULL;
	llist->tail  = NULL;
	llist->count = 0;

	return llist;
}
/* }}} */

static crex_long spl_ptr_llist_count(spl_ptr_llist *llist) /* {{{ */
{
	return (crex_long)llist->count;
}
/* }}} */

static void spl_ptr_llist_destroy(spl_ptr_llist *llist) /* {{{ */
{
	spl_ptr_llist_element *current = llist->head, *next;

	while (current) {
		next = current->next;
		zval_ptr_dtor(&current->data);
		SPL_LLIST_DELREF(current);
		current = next;
	}

	efree(llist);
}
/* }}} */

static spl_ptr_llist_element *spl_ptr_llist_offset(spl_ptr_llist *llist, crex_long offset, int backward) /* {{{ */
{

	spl_ptr_llist_element *current;
	int pos = 0;

	if (backward) {
		current = llist->tail;
	} else {
		current = llist->head;
	}

	while (current && pos < offset) {
		pos++;
		if (backward) {
			current = current->prev;
		} else {
			current = current->next;
		}
	}

	return current;
}
/* }}} */

static void spl_ptr_llist_unshift(spl_ptr_llist *llist, zval *data) /* {{{ */
{
	spl_ptr_llist_element *elem = emalloc(sizeof(spl_ptr_llist_element));

	elem->prev = NULL;
	elem->next = llist->head;
	ZVAL_COPY(&elem->data, data);
	SPL_LLIST_RC(elem) = 1;

	if (llist->head) {
		llist->head->prev = elem;
	} else {
		llist->tail = elem;
	}

	llist->head = elem;
	llist->count++;
}
/* }}} */

static void spl_ptr_llist_push(spl_ptr_llist *llist, zval *data) /* {{{ */
{
	spl_ptr_llist_element *elem = emalloc(sizeof(spl_ptr_llist_element));

	elem->prev = llist->tail;
	elem->next = NULL;
	ZVAL_COPY(&elem->data, data);
	SPL_LLIST_RC(elem) = 1;

	if (llist->tail) {
		llist->tail->next = elem;
	} else {
		llist->head = elem;
	}

	llist->tail = elem;
	llist->count++;
}
/* }}} */

static void spl_ptr_llist_pop(spl_ptr_llist *llist, zval *ret) /* {{{ */
{
	spl_ptr_llist_element    *tail = llist->tail;

	if (tail == NULL) {
		ZVAL_UNDEF(ret);
		return;
	}

	if (tail->prev) {
		tail->prev->next = NULL;
	} else {
		llist->head = NULL;
	}

	llist->tail = tail->prev;
	llist->count--;
	ZVAL_COPY_VALUE(ret, &tail->data);
	ZVAL_UNDEF(&tail->data);

	tail->prev = NULL;

	SPL_LLIST_DELREF(tail);
}
/* }}} */

static zval *spl_ptr_llist_last(spl_ptr_llist *llist) /* {{{ */
{
	spl_ptr_llist_element *tail = llist->tail;

	if (tail == NULL) {
		return NULL;
	} else {
		return &tail->data;
	}
}
/* }}} */

static zval *spl_ptr_llist_first(spl_ptr_llist *llist) /* {{{ */
{
	spl_ptr_llist_element *head = llist->head;

	if (head == NULL) {
		return NULL;
	} else {
		return &head->data;
	}
}
/* }}} */

static void spl_ptr_llist_shift(spl_ptr_llist *llist, zval *ret) /* {{{ */
{
	spl_ptr_llist_element   *head = llist->head;

	if (head == NULL) {
		ZVAL_UNDEF(ret);
		return;
	}

	if (head->next) {
		head->next->prev = NULL;
	} else {
		llist->tail = NULL;
	}

	llist->head = head->next;
	llist->count--;
	ZVAL_COPY_VALUE(ret, &head->data);
	ZVAL_UNDEF(&head->data);

	head->next = NULL;

	SPL_LLIST_DELREF(head);
}
/* }}} */

static void spl_ptr_llist_copy(spl_ptr_llist *from, spl_ptr_llist *to) /* {{{ */
{
	spl_ptr_llist_element *current = from->head, *next;

	while (current) {
		next = current->next;
		spl_ptr_llist_push(to, &current->data);
		current = next;
	}

}
/* }}} */

/* }}} */

static void spl_dllist_object_free_storage(crex_object *object) /* {{{ */
{
	spl_dllist_object *intern = spl_dllist_from_obj(object);
	zval tmp;

	crex_object_std_dtor(&intern->std);

	if (intern->llist) {
		while (intern->llist->count > 0) {
			spl_ptr_llist_pop(intern->llist, &tmp);
			zval_ptr_dtor(&tmp);
		}
		spl_ptr_llist_destroy(intern->llist);
	}
	SPL_LLIST_CHECK_DELREF(intern->traverse_pointer);
}
/* }}} */

static crex_object *spl_dllist_object_new_ex(crex_class_entry *class_type, crex_object *orig, int clone_orig) /* {{{ */
{
	spl_dllist_object *intern;
	crex_class_entry  *parent = class_type;
	int                inherited = 0;

	intern = crex_object_alloc(sizeof(spl_dllist_object), parent);

	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->flags = 0;
	intern->traverse_position = 0;

	if (orig) {
		spl_dllist_object *other = spl_dllist_from_obj(orig);
		intern->ce_get_iterator = other->ce_get_iterator;

		if (clone_orig) {
			intern->llist = spl_ptr_llist_init();
			spl_ptr_llist_copy(other->llist, intern->llist);
			intern->traverse_pointer  = intern->llist->head;
			SPL_LLIST_CHECK_ADDREF(intern->traverse_pointer);
		} else {
			intern->llist = other->llist;
			intern->traverse_pointer  = intern->llist->head;
			SPL_LLIST_CHECK_ADDREF(intern->traverse_pointer);
		}

		intern->flags = other->flags;
	} else {
		intern->llist = spl_ptr_llist_init();
		intern->traverse_pointer  = intern->llist->head;
		SPL_LLIST_CHECK_ADDREF(intern->traverse_pointer);
	}

	while (parent) {
		if (parent == spl_ce_SplStack) {
			intern->flags |= (SPL_DLLIST_IT_FIX | SPL_DLLIST_IT_LIFO);
		} else if (parent == spl_ce_SplQueue) {
			intern->flags |= SPL_DLLIST_IT_FIX;
		}

		if (parent == spl_ce_SplDoublyLinkedList) {
			break;
		}

		parent = parent->parent;
		inherited = 1;
	}

	CREX_ASSERT(parent);

	if (inherited) {
		intern->fptr_offset_get = crex_hash_str_find_ptr(&class_type->function_table, "offsetget", sizeof("offsetget") - 1);
		if (intern->fptr_offset_get->common.scope == parent) {
			intern->fptr_offset_get = NULL;
		}
		intern->fptr_offset_set = crex_hash_str_find_ptr(&class_type->function_table, "offsetset", sizeof("offsetset") - 1);
		if (intern->fptr_offset_set->common.scope == parent) {
			intern->fptr_offset_set = NULL;
		}
		intern->fptr_offset_has = crex_hash_str_find_ptr(&class_type->function_table, "offsetexists", sizeof("offsetexists") - 1);
		if (intern->fptr_offset_has->common.scope == parent) {
			intern->fptr_offset_has = NULL;
		}
		intern->fptr_offset_del = crex_hash_str_find_ptr(&class_type->function_table, "offsetunset", sizeof("offsetunset") - 1);
		if (intern->fptr_offset_del->common.scope == parent) {
			intern->fptr_offset_del = NULL;
		}
		/* Find count() method */
		intern->fptr_count = crex_hash_find_ptr(&class_type->function_table, ZSTR_KNOWN(CREX_STR_COUNT));
		if (intern->fptr_count->common.scope == parent) {
			intern->fptr_count = NULL;
		}
	}

	return &intern->std;
}
/* }}} */

static crex_object *spl_dllist_object_new(crex_class_entry *class_type) /* {{{ */
{
	return spl_dllist_object_new_ex(class_type, NULL, 0);
}
/* }}} */

static crex_object *spl_dllist_object_clone(crex_object *old_object) /* {{{ */
{
	crex_object *new_object = spl_dllist_object_new_ex(old_object->ce, old_object, 1);

	crex_objects_clone_members(new_object, old_object);

	return new_object;
}
/* }}} */

static crex_result spl_dllist_object_count_elements(crex_object *object, crex_long *count) /* {{{ */
{
	spl_dllist_object *intern = spl_dllist_from_obj(object);

	if (intern->fptr_count) {
		zval rv;
		crex_call_method_with_0_params(object, intern->std.ce, &intern->fptr_count, "count", &rv);
		if (!C_ISUNDEF(rv)) {
			*count = zval_get_long(&rv);
			zval_ptr_dtor(&rv);
			return SUCCESS;
		}
		*count = 0;
		return FAILURE;
	}

	*count = spl_ptr_llist_count(intern->llist);
	return SUCCESS;
}
/* }}} */

static inline HashTable* spl_dllist_object_get_debug_info(crex_object *obj) /* {{{{ */
{
	spl_dllist_object     *intern  = spl_dllist_from_obj(obj);
	spl_ptr_llist_element *current = intern->llist->head, *next;
	zval tmp, dllist_array;
	crex_string *pnstr;
	int  i = 0;
	HashTable *debug_info;

	if (!intern->std.properties) {
		rebuild_object_properties(&intern->std);
	}

	debug_info = crex_new_array(1);
	crex_hash_copy(debug_info, intern->std.properties, (copy_ctor_func_t) zval_add_ref);

	pnstr = spl_gen_private_prop_name(spl_ce_SplDoublyLinkedList, "flags", sizeof("flags")-1);
	ZVAL_LONG(&tmp, intern->flags);
	crex_hash_add(debug_info, pnstr, &tmp);
	crex_string_release_ex(pnstr, 0);

	array_init(&dllist_array);

	while (current) {
		next = current->next;

		add_index_zval(&dllist_array, i, &current->data);
		if (C_REFCOUNTED(current->data)) {
			C_ADDREF(current->data);
		}
		i++;

		current = next;
	}

	pnstr = spl_gen_private_prop_name(spl_ce_SplDoublyLinkedList, "dllist", sizeof("dllist")-1);
	crex_hash_add(debug_info, pnstr, &dllist_array);
	crex_string_release_ex(pnstr, 0);

	return debug_info;
}
/* }}}} */

static HashTable *spl_dllist_object_get_gc(crex_object *obj, zval **gc_data, int *gc_data_count) /* {{{ */
{
	spl_dllist_object *intern = spl_dllist_from_obj(obj);
	crex_get_gc_buffer *gc_buffer = crex_get_gc_buffer_create();
	spl_ptr_llist_element *current = intern->llist->head;

	while (current) {
		crex_get_gc_buffer_add_zval(gc_buffer, &current->data);
		current = current->next;
	}

	crex_get_gc_buffer_use(gc_buffer, gc_data, gc_data_count);
	return crex_std_get_properties(obj);
}
/* }}} */

/* {{{ Push $value on the SplDoublyLinkedList */
CRX_METHOD(SplDoublyLinkedList, push)
{
	zval *value;
	spl_dllist_object *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "z", &value) == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLDLLIST_P(CREX_THIS);
	spl_ptr_llist_push(intern->llist, value);
}
/* }}} */

/* {{{ Unshift $value on the SplDoublyLinkedList */
CRX_METHOD(SplDoublyLinkedList, unshift)
{
	zval *value;
	spl_dllist_object *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "z", &value) == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLDLLIST_P(CREX_THIS);
	spl_ptr_llist_unshift(intern->llist, value);
}
/* }}} */

/* {{{ Pop an element out of the SplDoublyLinkedList */
CRX_METHOD(SplDoublyLinkedList, pop)
{
	spl_dllist_object *intern;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLDLLIST_P(CREX_THIS);
	spl_ptr_llist_pop(intern->llist, return_value);

	if (C_ISUNDEF_P(return_value)) {
		crex_throw_exception(spl_ce_RuntimeException, "Can't pop from an empty datastructure", 0);
		RETURN_THROWS();
	}
}
/* }}} */

/* {{{ Shift an element out of the SplDoublyLinkedList */
CRX_METHOD(SplDoublyLinkedList, shift)
{
	spl_dllist_object *intern;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLDLLIST_P(CREX_THIS);
	spl_ptr_llist_shift(intern->llist, return_value);

	if (C_ISUNDEF_P(return_value)) {
		crex_throw_exception(spl_ce_RuntimeException, "Can't shift from an empty datastructure", 0);
		RETURN_THROWS();
	}
}
/* }}} */

/* {{{ Peek at the top element of the SplDoublyLinkedList */
CRX_METHOD(SplDoublyLinkedList, top)
{
	zval *value;
	spl_dllist_object *intern;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLDLLIST_P(CREX_THIS);
	value = spl_ptr_llist_last(intern->llist);

	if (value == NULL || C_ISUNDEF_P(value)) {
		crex_throw_exception(spl_ce_RuntimeException, "Can't peek at an empty datastructure", 0);
		RETURN_THROWS();
	}

	RETURN_COPY_DEREF(value);
}
/* }}} */

/* {{{ Peek at the bottom element of the SplDoublyLinkedList */
CRX_METHOD(SplDoublyLinkedList, bottom)
{
	zval *value;
	spl_dllist_object *intern;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLDLLIST_P(CREX_THIS);
	value  = spl_ptr_llist_first(intern->llist);

	if (value == NULL || C_ISUNDEF_P(value)) {
		crex_throw_exception(spl_ce_RuntimeException, "Can't peek at an empty datastructure", 0);
		RETURN_THROWS();
	}

	RETURN_COPY_DEREF(value);
}
/* }}} */

/* {{{ Return the number of elements in the datastructure. */
CRX_METHOD(SplDoublyLinkedList, count)
{
	crex_long count;
	spl_dllist_object *intern = C_SPLDLLIST_P(CREX_THIS);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	count = spl_ptr_llist_count(intern->llist);
	RETURN_LONG(count);
}
/* }}} */

/* {{{ Return true if the SplDoublyLinkedList is empty. */
CRX_METHOD(SplDoublyLinkedList, isEmpty)
{
	crex_long count;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	spl_dllist_object_count_elements(C_OBJ_P(CREX_THIS), &count);
	RETURN_BOOL(count == 0);
}
/* }}} */

/* {{{ Set the mode of iteration */
CRX_METHOD(SplDoublyLinkedList, setIteratorMode)
{
	crex_long value;
	spl_dllist_object *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &value) == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLDLLIST_P(CREX_THIS);

	if ((intern->flags & SPL_DLLIST_IT_FIX)
		&& (intern->flags & SPL_DLLIST_IT_LIFO) != (value & SPL_DLLIST_IT_LIFO)) {
		crex_throw_exception(spl_ce_RuntimeException, "Iterators' LIFO/FIFO modes for SplStack/SplQueue objects are frozen", 0);
		RETURN_THROWS();
	}

	intern->flags = (value & SPL_DLLIST_IT_MASK) | (intern->flags & SPL_DLLIST_IT_FIX);

	RETURN_LONG(intern->flags);
}
/* }}} */

/* {{{ Return the mode of iteration */
CRX_METHOD(SplDoublyLinkedList, getIteratorMode)
{
	spl_dllist_object *intern;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLDLLIST_P(CREX_THIS);

	RETURN_LONG(intern->flags);
}
/* }}} */

/* {{{ Returns whether the requested $index exists. */
CRX_METHOD(SplDoublyLinkedList, offsetExists)
{
	spl_dllist_object *intern;
	crex_long index;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLDLLIST_P(CREX_THIS);

	RETURN_BOOL(index >= 0 && index < intern->llist->count);
} /* }}} */

/* {{{ Returns the value at the specified $index. */
CRX_METHOD(SplDoublyLinkedList, offsetGet)
{
	crex_long                   index;
	spl_dllist_object     *intern;
	spl_ptr_llist_element *element;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLDLLIST_P(CREX_THIS);

	if (index < 0 || index >= intern->llist->count) {
		crex_argument_error(spl_ce_OutOfRangeException, 1, "is out of range");
		RETURN_THROWS();
	}

	element = spl_ptr_llist_offset(intern->llist, index, intern->flags & SPL_DLLIST_IT_LIFO);
	if (element == NULL) {
		crex_argument_error(spl_ce_OutOfRangeException, 1, "is an invalid offset");
		RETURN_THROWS();
	}

	RETURN_COPY_DEREF(&element->data);
} /* }}} */

/* {{{ Sets the value at the specified $index to $newval. */
CRX_METHOD(SplDoublyLinkedList, offsetSet)
{
	crex_long index;
	bool index_is_null = 1;
	zval *value;
	spl_dllist_object *intern;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l!z", &index, &index_is_null, &value) == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLDLLIST_P(CREX_THIS);

	if (index_is_null) {
		/* $obj[] = ... */
		spl_ptr_llist_push(intern->llist, value);
	} else {
		/* $obj[$foo] = ... */
		spl_ptr_llist_element *element;

		if (index < 0 || index >= intern->llist->count) {
			crex_argument_error(spl_ce_OutOfRangeException, 1, "is out of range");
			RETURN_THROWS();
		}

		element = spl_ptr_llist_offset(intern->llist, index, intern->flags & SPL_DLLIST_IT_LIFO);

		if (element != NULL) {
			/* the element is replaced, delref the old one as in
			 * SplDoublyLinkedList::pop() */
			zval_ptr_dtor(&element->data);
			ZVAL_COPY(&element->data, value);
		} else {
			zval_ptr_dtor(value);
			crex_argument_error(spl_ce_OutOfRangeException, 1, "is an invalid offset");
			RETURN_THROWS();
		}
	}
} /* }}} */

/* {{{ Unsets the value at the specified $index. */
CRX_METHOD(SplDoublyLinkedList, offsetUnset)
{
	crex_long             index;
	spl_dllist_object     *intern;
	spl_ptr_llist_element *element;
	spl_ptr_llist         *llist;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &index) == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLDLLIST_P(CREX_THIS);
	llist  = intern->llist;

	if (index < 0 || index >= intern->llist->count) {
		crex_argument_error(spl_ce_OutOfRangeException, 1, "is out of range");
		RETURN_THROWS();
	}

	element = spl_ptr_llist_offset(intern->llist, index, intern->flags & SPL_DLLIST_IT_LIFO);

	if (element != NULL) {
		/* connect the neightbors */
		if (element->prev) {
			element->prev->next = element->next;
		}

		if (element->next) {
			element->next->prev = element->prev;
		}

		/* take care of head/tail */
		if (element == llist->head) {
			llist->head = element->next;
		}

		if (element == llist->tail) {
			llist->tail = element->prev;
		}

		/* finally, delete the element */
		llist->count--;

		if (intern->traverse_pointer == element) {
			SPL_LLIST_DELREF(element);
			intern->traverse_pointer = NULL;
		}

		zval_ptr_dtor(&element->data);
		ZVAL_UNDEF(&element->data);

		SPL_LLIST_DELREF(element);
	} else {
		crex_argument_error(spl_ce_OutOfRangeException, 1, "is an invalid offset");
		RETURN_THROWS();
	}
} /* }}} */

static void spl_dllist_it_dtor(crex_object_iterator *iter) /* {{{ */
{
	spl_dllist_it *iterator = (spl_dllist_it *)iter;

	SPL_LLIST_CHECK_DELREF(iterator->traverse_pointer);

	zval_ptr_dtor(&iterator->intern.data);
}
/* }}} */

static void spl_dllist_it_helper_rewind(spl_ptr_llist_element **traverse_pointer_ptr, int *traverse_position_ptr, spl_ptr_llist *llist, int flags) /* {{{ */
{
	SPL_LLIST_CHECK_DELREF(*traverse_pointer_ptr);

	if (flags & SPL_DLLIST_IT_LIFO) {
		*traverse_position_ptr = llist->count-1;
		*traverse_pointer_ptr  = llist->tail;
	} else {
		*traverse_position_ptr = 0;
		*traverse_pointer_ptr  = llist->head;
	}

	SPL_LLIST_CHECK_ADDREF(*traverse_pointer_ptr);
}
/* }}} */

static void spl_dllist_it_helper_move_forward(spl_ptr_llist_element **traverse_pointer_ptr, int *traverse_position_ptr, spl_ptr_llist *llist, int flags) /* {{{ */
{
	if (*traverse_pointer_ptr) {
		spl_ptr_llist_element *old = *traverse_pointer_ptr;

		if (flags & SPL_DLLIST_IT_LIFO) {
			*traverse_pointer_ptr = old->prev;
			(*traverse_position_ptr)--;

			if (flags & SPL_DLLIST_IT_DELETE) {
				zval prev;
				spl_ptr_llist_pop(llist, &prev);

				zval_ptr_dtor(&prev);
			}
		} else {
			*traverse_pointer_ptr = old->next;

			if (flags & SPL_DLLIST_IT_DELETE) {
				zval prev;
				spl_ptr_llist_shift(llist, &prev);

				zval_ptr_dtor(&prev);
			} else {
				(*traverse_position_ptr)++;
			}
		}

		SPL_LLIST_DELREF(old);
		SPL_LLIST_CHECK_ADDREF(*traverse_pointer_ptr);
	}
}
/* }}} */

static void spl_dllist_it_rewind(crex_object_iterator *iter) /* {{{ */
{
	spl_dllist_it *iterator = (spl_dllist_it *)iter;
	spl_dllist_object *object = C_SPLDLLIST_P(&iter->data);
	spl_ptr_llist *llist = object->llist;

	spl_dllist_it_helper_rewind(&iterator->traverse_pointer, &iterator->traverse_position, llist, iterator->flags);
}
/* }}} */

static int spl_dllist_it_valid(crex_object_iterator *iter) /* {{{ */
{
	spl_dllist_it         *iterator = (spl_dllist_it *)iter;
	spl_ptr_llist_element *element  = iterator->traverse_pointer;

	return (element != NULL ? SUCCESS : FAILURE);
}
/* }}} */

static zval *spl_dllist_it_get_current_data(crex_object_iterator *iter) /* {{{ */
{
	spl_dllist_it         *iterator = (spl_dllist_it *)iter;
	spl_ptr_llist_element *element  = iterator->traverse_pointer;

	if (element == NULL || C_ISUNDEF(element->data)) {
		return NULL;
	}

	return &element->data;
}
/* }}} */

static void spl_dllist_it_get_current_key(crex_object_iterator *iter, zval *key) /* {{{ */
{
	spl_dllist_it *iterator = (spl_dllist_it *)iter;

	ZVAL_LONG(key, iterator->traverse_position);
}
/* }}} */

static void spl_dllist_it_move_forward(crex_object_iterator *iter) /* {{{ */
{
	spl_dllist_it *iterator = (spl_dllist_it *)iter;
	spl_dllist_object *object = C_SPLDLLIST_P(&iter->data);

	spl_dllist_it_helper_move_forward(&iterator->traverse_pointer, &iterator->traverse_position, object->llist, iterator->flags);
}
/* }}} */

/* {{{ Return current array key */
CRX_METHOD(SplDoublyLinkedList, key)
{
	spl_dllist_object *intern = C_SPLDLLIST_P(CREX_THIS);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	RETURN_LONG(intern->traverse_position);
}
/* }}} */

/* {{{ Move to next entry */
CRX_METHOD(SplDoublyLinkedList, prev)
{
	spl_dllist_object *intern = C_SPLDLLIST_P(CREX_THIS);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	spl_dllist_it_helper_move_forward(&intern->traverse_pointer, &intern->traverse_position, intern->llist, intern->flags ^ SPL_DLLIST_IT_LIFO);
}
/* }}} */

/* {{{ Move to next entry */
CRX_METHOD(SplDoublyLinkedList, next)
{
	spl_dllist_object *intern = C_SPLDLLIST_P(CREX_THIS);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	spl_dllist_it_helper_move_forward(&intern->traverse_pointer, &intern->traverse_position, intern->llist, intern->flags);
}
/* }}} */

/* {{{ Check whether the datastructure contains more entries */
CRX_METHOD(SplDoublyLinkedList, valid)
{
	spl_dllist_object *intern = C_SPLDLLIST_P(CREX_THIS);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	RETURN_BOOL(intern->traverse_pointer != NULL);
}
/* }}} */

/* {{{ Rewind the datastructure back to the start */
CRX_METHOD(SplDoublyLinkedList, rewind)
{
	spl_dllist_object *intern = C_SPLDLLIST_P(CREX_THIS);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	spl_dllist_it_helper_rewind(&intern->traverse_pointer, &intern->traverse_position, intern->llist, intern->flags);
}
/* }}} */

/* {{{ Return current datastructure entry */
CRX_METHOD(SplDoublyLinkedList, current)
{
	spl_dllist_object     *intern  = C_SPLDLLIST_P(CREX_THIS);
	spl_ptr_llist_element *element = intern->traverse_pointer;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (element == NULL || C_ISUNDEF(element->data)) {
		RETURN_NULL();
	} else {
		RETURN_COPY_DEREF(&element->data);
	}
}
/* }}} */

/* {{{ Serializes storage */
CRX_METHOD(SplDoublyLinkedList, serialize)
{
	spl_dllist_object     *intern   = C_SPLDLLIST_P(CREX_THIS);
	smart_str              buf      = {0};
	spl_ptr_llist_element *current  = intern->llist->head, *next;
	zval                   flags;
	crx_serialize_data_t   var_hash;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CRX_VAR_SERIALIZE_INIT(var_hash);

	/* flags */
	ZVAL_LONG(&flags, intern->flags);
	crx_var_serialize(&buf, &flags, &var_hash);

	/* elements */
	while (current) {
		smart_str_appendc(&buf, ':');
		next = current->next;

		crx_var_serialize(&buf, &current->data, &var_hash);

		current = next;
	}

	/* done */
	CRX_VAR_SERIALIZE_DESTROY(var_hash);

	RETURN_STR(smart_str_extract(&buf));
} /* }}} */

/* {{{ Unserializes storage */
CRX_METHOD(SplDoublyLinkedList, unserialize)
{
	spl_dllist_object *intern = C_SPLDLLIST_P(CREX_THIS);
	zval *flags, *elem;
	char *buf;
	size_t buf_len;
	const unsigned char *p, *s;
	crx_unserialize_data_t var_hash;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &buf, &buf_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (buf_len == 0) {
		return;
	}

	while (intern->llist->count > 0) {
		zval tmp;
		spl_ptr_llist_pop(intern->llist, &tmp);
		zval_ptr_dtor(&tmp);
	}

	s = p = (const unsigned char*)buf;
	CRX_VAR_UNSERIALIZE_INIT(var_hash);

	/* flags */
	flags = var_tmp_var(&var_hash);
	if (!crx_var_unserialize(flags, &p, s + buf_len, &var_hash) || C_TYPE_P(flags) != IS_LONG) {
		goto error;
	}

	intern->flags = (int)C_LVAL_P(flags);

	/* elements */
	while(*p == ':') {
		++p;
		elem = var_tmp_var(&var_hash);
		if (!crx_var_unserialize(elem, &p, s + buf_len, &var_hash)) {
			goto error;
		}
		var_push_dtor(&var_hash, elem);

		spl_ptr_llist_push(intern->llist, elem);
	}

	if (*p != '\0') {
		goto error;
	}

	CRX_VAR_UNSERIALIZE_DESTROY(var_hash);
	return;

error:
	CRX_VAR_UNSERIALIZE_DESTROY(var_hash);
	crex_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Error at offset %zd of %zd bytes", ((char*)p - buf), buf_len);
	RETURN_THROWS();

} /* }}} */

/* {{{ */
CRX_METHOD(SplDoublyLinkedList, __serialize)
{
	spl_dllist_object *intern = C_SPLDLLIST_P(CREX_THIS);
	spl_ptr_llist_element *current = intern->llist->head;
	zval tmp;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);

	/* flags */
	ZVAL_LONG(&tmp, intern->flags);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &tmp);

	/* elements */
	array_init_size(&tmp, intern->llist->count);
	while (current) {
		crex_hash_next_index_insert(C_ARRVAL(tmp), &current->data);
		C_TRY_ADDREF(current->data);
		current = current->next;
	}
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &tmp);

	/* members */
	ZVAL_ARR(&tmp, crex_proptable_to_symtable(
		crex_std_get_properties(&intern->std), /* always_duplicate */ 1));
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &tmp);
} /* }}} */

/* {{{ */
CRX_METHOD(SplDoublyLinkedList, __unserialize) {
	spl_dllist_object *intern = C_SPLDLLIST_P(CREX_THIS);
	HashTable *data;
	zval *flags_zv, *storage_zv, *members_zv, *elem;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "h", &data) == FAILURE) {
		RETURN_THROWS();
	}

	flags_zv = crex_hash_index_find(data, 0);
	storage_zv = crex_hash_index_find(data, 1);
	members_zv = crex_hash_index_find(data, 2);
	if (!flags_zv || !storage_zv || !members_zv ||
			C_TYPE_P(flags_zv) != IS_LONG || C_TYPE_P(storage_zv) != IS_ARRAY ||
			C_TYPE_P(members_zv) != IS_ARRAY) {
		crex_throw_exception(spl_ce_UnexpectedValueException,
			"Incomplete or ill-typed serialization data", 0);
		RETURN_THROWS();
	}

	intern->flags = (int) C_LVAL_P(flags_zv);

	CREX_HASH_FOREACH_VAL(C_ARRVAL_P(storage_zv), elem) {
		spl_ptr_llist_push(intern->llist, elem);
	} CREX_HASH_FOREACH_END();

	object_properties_load(&intern->std, C_ARRVAL_P(members_zv));
} /* }}} */

/* {{{ Inserts a new entry before the specified $index consisting of $newval. */
CRX_METHOD(SplDoublyLinkedList, add)
{
	zval                  *value;
	spl_dllist_object     *intern;
	spl_ptr_llist_element *element;
	crex_long                  index;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "lz", &index, &value) == FAILURE) {
		RETURN_THROWS();
	}

	intern = C_SPLDLLIST_P(CREX_THIS);

	if (index < 0 || index > intern->llist->count) {
		crex_argument_error(spl_ce_OutOfRangeException, 1, "is out of range");
		RETURN_THROWS();
	}

	if (index == intern->llist->count) {
		/* If index is the last entry+1 then we do a push because we're not inserting before any entry */
		spl_ptr_llist_push(intern->llist, value);
	} else {
		/* Create the new element we want to insert */
		spl_ptr_llist_element *elem = emalloc(sizeof(spl_ptr_llist_element));

		/* Get the element we want to insert before */
		element = spl_ptr_llist_offset(intern->llist, index, intern->flags & SPL_DLLIST_IT_LIFO);
		CREX_ASSERT(element != NULL);

		ZVAL_COPY(&elem->data, value);
		SPL_LLIST_RC(elem) = 1;
		/* connect to the neighbours */
		elem->next = element;
		elem->prev = element->prev;

		/* connect the neighbours to this new element */
		if (elem->prev == NULL) {
			intern->llist->head = elem;
		} else {
			element->prev->next = elem;
		}
		element->prev = elem;

		intern->llist->count++;
	}
} /* }}} */

/* {{{ */
CRX_METHOD(SplDoublyLinkedList, __debugInfo)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	RETURN_ARR(spl_dllist_object_get_debug_info(C_OBJ_P(CREX_THIS)));
} /* }}} */

/* {{{ iterator handler table */
static const crex_object_iterator_funcs spl_dllist_it_funcs = {
	spl_dllist_it_dtor,
	spl_dllist_it_valid,
	spl_dllist_it_get_current_data,
	spl_dllist_it_get_current_key,
	spl_dllist_it_move_forward,
	spl_dllist_it_rewind,
	NULL,
	NULL, /* get_gc */
}; /* }}} */

static crex_object_iterator *spl_dllist_get_iterator(crex_class_entry *ce, zval *object, int by_ref) /* {{{ */
{
	spl_dllist_object *dllist_object = C_SPLDLLIST_P(object);

	if (by_ref) {
		crex_throw_error(NULL, "An iterator cannot be used with foreach by reference");
		return NULL;
	}

	spl_dllist_it *iterator = emalloc(sizeof(spl_dllist_it));

	crex_iterator_init(&iterator->intern);

	ZVAL_OBJ_COPY(&iterator->intern.data, C_OBJ_P(object));
	iterator->intern.funcs       = &spl_dllist_it_funcs;
	iterator->traverse_position  = dllist_object->traverse_position;
	iterator->traverse_pointer   = dllist_object->traverse_pointer;
	iterator->flags              = dllist_object->flags & SPL_DLLIST_IT_MASK;

	SPL_LLIST_CHECK_ADDREF(iterator->traverse_pointer);

	return &iterator->intern;
}
/* }}} */

CRX_MINIT_FUNCTION(spl_dllist) /* {{{ */
{
	spl_ce_SplDoublyLinkedList = register_class_SplDoublyLinkedList(
		crex_ce_iterator, crex_ce_countable, crex_ce_arrayaccess, crex_ce_serializable
	);
	spl_ce_SplDoublyLinkedList->create_object = spl_dllist_object_new;
	spl_ce_SplDoublyLinkedList->default_object_handlers = &spl_handler_SplDoublyLinkedList;
	spl_ce_SplDoublyLinkedList->get_iterator = spl_dllist_get_iterator;

	memcpy(&spl_handler_SplDoublyLinkedList, &std_object_handlers, sizeof(crex_object_handlers));

	spl_handler_SplDoublyLinkedList.offset = XtOffsetOf(spl_dllist_object, std);
	spl_handler_SplDoublyLinkedList.clone_obj = spl_dllist_object_clone;
	spl_handler_SplDoublyLinkedList.count_elements = spl_dllist_object_count_elements;
	spl_handler_SplDoublyLinkedList.get_gc = spl_dllist_object_get_gc;
	spl_handler_SplDoublyLinkedList.free_obj = spl_dllist_object_free_storage;

	spl_ce_SplQueue = register_class_SplQueue(spl_ce_SplDoublyLinkedList);
	spl_ce_SplQueue->create_object = spl_dllist_object_new;
	spl_ce_SplQueue->get_iterator = spl_dllist_get_iterator;

	spl_ce_SplStack = register_class_SplStack(spl_ce_SplDoublyLinkedList);
	spl_ce_SplStack->create_object = spl_dllist_object_new;
	spl_ce_SplStack->get_iterator = spl_dllist_get_iterator;

	return SUCCESS;
}
/* }}} */
