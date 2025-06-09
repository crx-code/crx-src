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

#include "crex.h"
#include "crex_llist.h"
#include "crex_sort.h"

CREX_API void crex_llist_init(crex_llist *l, size_t size, llist_dtor_func_t dtor, unsigned char persistent)
{
	l->head  = NULL;
	l->tail  = NULL;
	l->count = 0;
	l->size  = size;
	l->dtor  = dtor;
	l->persistent = persistent;
}

CREX_API void crex_llist_add_element(crex_llist *l, const void *element)
{
	crex_llist_element *tmp = pemalloc(sizeof(crex_llist_element)+l->size-1, l->persistent);

	tmp->prev = l->tail;
	tmp->next = NULL;
	if (l->tail) {
		l->tail->next = tmp;
	} else {
		l->head = tmp;
	}
	l->tail = tmp;
	memcpy(tmp->data, element, l->size);

	++l->count;
}


CREX_API void crex_llist_prepend_element(crex_llist *l, const void *element)
{
	crex_llist_element *tmp = pemalloc(sizeof(crex_llist_element)+l->size-1, l->persistent);

	tmp->next = l->head;
	tmp->prev = NULL;
	if (l->head) {
		l->head->prev = tmp;
	} else {
		l->tail = tmp;
	}
	l->head = tmp;
	memcpy(tmp->data, element, l->size);

	++l->count;
}


#define DEL_LLIST_ELEMENT(current, l) \
			if ((current)->prev) {\
				(current)->prev->next = (current)->next;\
			} else {\
				(l)->head = (current)->next;\
			}\
			if ((current)->next) {\
				(current)->next->prev = (current)->prev;\
			} else {\
				(l)->tail = (current)->prev;\
			}\
			if ((l)->dtor) {\
				(l)->dtor((current)->data);\
			}\
			pefree((current), (l)->persistent);\
			--l->count;


CREX_API void crex_llist_del_element(crex_llist *l, void *element, int (*compare)(void *element1, void *element2))
{
	crex_llist_element *current=l->head;

	while (current) {
		if (compare(current->data, element)) {
			DEL_LLIST_ELEMENT(current, l);
			break;
		}
		current = current->next;
	}
}


CREX_API void crex_llist_destroy(crex_llist *l)
{
	crex_llist_element *current=l->head, *next;

	while (current) {
		next = current->next;
		if (l->dtor) {
			l->dtor(current->data);
		}
		pefree(current, l->persistent);
		current = next;
	}

	l->head  = NULL;
	l->tail  = NULL;
	l->count = 0;
}


CREX_API void crex_llist_clean(crex_llist *l)
{
	crex_llist_destroy(l);
	l->head = l->tail = NULL;
}


CREX_API void crex_llist_remove_tail(crex_llist *l)
{
	crex_llist_element *old_tail = l->tail;
	if (!old_tail) {
		return;
	}

	if (old_tail->prev) {
		old_tail->prev->next = NULL;
	} else {
		l->head = NULL;
	}

	l->tail = old_tail->prev;
	--l->count;

	if (l->dtor) {
		l->dtor(old_tail->data);
	}
	pefree(old_tail, l->persistent);
}


CREX_API void crex_llist_copy(crex_llist *dst, crex_llist *src)
{
	crex_llist_element *ptr;

	crex_llist_init(dst, src->size, src->dtor, src->persistent);
	ptr = src->head;
	while (ptr) {
		crex_llist_add_element(dst, ptr->data);
		ptr = ptr->next;
	}
}


CREX_API void crex_llist_apply_with_del(crex_llist *l, int (*func)(void *data))
{
	crex_llist_element *element, *next;

	element=l->head;
	while (element) {
		next = element->next;
		if (func(element->data)) {
			DEL_LLIST_ELEMENT(element, l);
		}
		element = next;
	}
}


CREX_API void crex_llist_apply(crex_llist *l, llist_apply_func_t func)
{
	crex_llist_element *element;

	for (element=l->head; element; element=element->next) {
		func(element->data);
	}
}

static void crex_llist_swap(crex_llist_element **p, crex_llist_element **q)
{
	crex_llist_element *t;
	t = *p;
	*p = *q;
	*q = t;
}

CREX_API void crex_llist_sort(crex_llist *l, llist_compare_func_t comp_func)
{
	size_t i;

	crex_llist_element **elements;
	crex_llist_element *element, **ptr;

	if (l->count == 0) {
		return;
	}

	elements = (crex_llist_element **) emalloc(l->count * sizeof(crex_llist_element *));

	ptr = &elements[0];

	for (element=l->head; element; element=element->next) {
		*ptr++ = element;
	}

	crex_sort(elements, l->count, sizeof(crex_llist_element *),
			(compare_func_t) comp_func, (swap_func_t) crex_llist_swap);

	l->head = elements[0];
	elements[0]->prev = NULL;

	for (i = 1; i < l->count; i++) {
		elements[i]->prev = elements[i-1];
		elements[i-1]->next = elements[i];
	}
	elements[i-1]->next = NULL;
	l->tail = elements[i-1];
	efree(elements);
}


CREX_API void crex_llist_apply_with_argument(crex_llist *l, llist_apply_with_arg_func_t func, void *arg)
{
	crex_llist_element *element;

	for (element=l->head; element; element=element->next) {
		func(element->data, arg);
	}
}


CREX_API void crex_llist_apply_with_arguments(crex_llist *l, llist_apply_with_args_func_t func, int num_args, ...)
{
	crex_llist_element *element;
	va_list args;

	va_start(args, num_args);
	for (element=l->head; element; element=element->next) {
		func(element->data, num_args, args);
	}
	va_end(args);
}


CREX_API size_t crex_llist_count(crex_llist *l)
{
	return l->count;
}


CREX_API void *crex_llist_get_first_ex(crex_llist *l, crex_llist_position *pos)
{
	crex_llist_position *current = pos ? pos : &l->traverse_ptr;

	*current = l->head;
	if (*current) {
		return (*current)->data;
	} else {
		return NULL;
	}
}


CREX_API void *crex_llist_get_last_ex(crex_llist *l, crex_llist_position *pos)
{
	crex_llist_position *current = pos ? pos : &l->traverse_ptr;

	*current = l->tail;
	if (*current) {
		return (*current)->data;
	} else {
		return NULL;
	}
}


CREX_API void *crex_llist_get_next_ex(crex_llist *l, crex_llist_position *pos)
{
	crex_llist_position *current = pos ? pos : &l->traverse_ptr;

	if (*current) {
		*current = (*current)->next;
		if (*current) {
			return (*current)->data;
		}
	}
	return NULL;
}


CREX_API void *crex_llist_get_prev_ex(crex_llist *l, crex_llist_position *pos)
{
	crex_llist_position *current = pos ? pos : &l->traverse_ptr;

	if (*current) {
		*current = (*current)->prev;
		if (*current) {
			return (*current)->data;
		}
	}
	return NULL;
}
