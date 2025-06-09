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

#ifndef CREX_LLIST_H
#define CREX_LLIST_H

typedef struct _crex_llist_element {
	struct _crex_llist_element *next;
	struct _crex_llist_element *prev;
	char data[1]; /* Needs to always be last in the struct */
} crex_llist_element;

typedef void (*llist_dtor_func_t)(void *);
typedef int (*llist_compare_func_t)(const crex_llist_element **, const crex_llist_element **);
typedef void (*llist_apply_with_args_func_t)(void *data, int num_args, va_list args);
typedef void (*llist_apply_with_arg_func_t)(void *data, void *arg);
typedef void (*llist_apply_func_t)(void *);

typedef struct _crex_llist {
	crex_llist_element *head;
	crex_llist_element *tail;
	size_t count;
	size_t size;
	llist_dtor_func_t dtor;
	unsigned char persistent;
	crex_llist_element *traverse_ptr;
} crex_llist;

typedef crex_llist_element* crex_llist_position;

BEGIN_EXTERN_C()
CREX_API void crex_llist_init(crex_llist *l, size_t size, llist_dtor_func_t dtor, unsigned char persistent);
CREX_API void crex_llist_add_element(crex_llist *l, const void *element);
CREX_API void crex_llist_prepend_element(crex_llist *l, const void *element);
CREX_API void crex_llist_del_element(crex_llist *l, void *element, int (*compare)(void *element1, void *element2));
CREX_API void crex_llist_destroy(crex_llist *l);
CREX_API void crex_llist_clean(crex_llist *l);
CREX_API void crex_llist_remove_tail(crex_llist *l);
CREX_API void crex_llist_copy(crex_llist *dst, crex_llist *src);
CREX_API void crex_llist_apply(crex_llist *l, llist_apply_func_t func);
CREX_API void crex_llist_apply_with_del(crex_llist *l, int (*func)(void *data));
CREX_API void crex_llist_apply_with_argument(crex_llist *l, llist_apply_with_arg_func_t func, void *arg);
CREX_API void crex_llist_apply_with_arguments(crex_llist *l, llist_apply_with_args_func_t func, int num_args, ...);
CREX_API size_t crex_llist_count(crex_llist *l);
CREX_API void crex_llist_sort(crex_llist *l, llist_compare_func_t comp_func);

/* traversal */
CREX_API void *crex_llist_get_first_ex(crex_llist *l, crex_llist_position *pos);
CREX_API void *crex_llist_get_last_ex(crex_llist *l, crex_llist_position *pos);
CREX_API void *crex_llist_get_next_ex(crex_llist *l, crex_llist_position *pos);
CREX_API void *crex_llist_get_prev_ex(crex_llist *l, crex_llist_position *pos);

static crex_always_inline void *crex_llist_get_first(crex_llist *l)
{
	return crex_llist_get_first_ex(l, NULL);
}

static crex_always_inline void *crex_llist_get_last(crex_llist *l)
{
	return crex_llist_get_last_ex(l, NULL);
}

static crex_always_inline void *crex_llist_get_next(crex_llist *l)
{
	return crex_llist_get_next_ex(l, NULL);
}

static crex_always_inline void *crex_llist_get_prev(crex_llist *l)
{
	return crex_llist_get_prev_ex(l, NULL);
}

END_EXTERN_C()

#endif /* CREX_LLIST_H */
