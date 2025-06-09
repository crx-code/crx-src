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
#include "crex_ptr_stack.h"
#include <stdarg.h>

CREX_API void crex_ptr_stack_init_ex(crex_ptr_stack *stack, bool persistent)
{
	stack->top_element = stack->elements = NULL;
	stack->top = stack->max = 0;
	stack->persistent = persistent;
}

CREX_API void crex_ptr_stack_init(crex_ptr_stack *stack)
{
	crex_ptr_stack_init_ex(stack, 0);
}


CREX_API void crex_ptr_stack_n_push(crex_ptr_stack *stack, int count, ...)
{
	va_list ptr;
	void *elem;

	CREX_PTR_STACK_RESIZE_IF_NEEDED(stack, count)

	va_start(ptr, count);
	while (count>0) {
		elem = va_arg(ptr, void *);
		stack->top++;
		*(stack->top_element++) = elem;
		count--;
	}
	va_end(ptr);
}


CREX_API void crex_ptr_stack_n_pop(crex_ptr_stack *stack, int count, ...)
{
	va_list ptr;
	void **elem;

	va_start(ptr, count);
	while (count>0) {
		elem = va_arg(ptr, void **);
		*elem = *(--stack->top_element);
		stack->top--;
		count--;
	}
	va_end(ptr);
}



CREX_API void crex_ptr_stack_destroy(crex_ptr_stack *stack)
{
	if (stack->elements) {
		pefree(stack->elements, stack->persistent);
	}
}


CREX_API void crex_ptr_stack_apply(crex_ptr_stack *stack, void (*func)(void *))
{
	int i = stack->top;

	while (--i >= 0) {
		func(stack->elements[i]);
	}
}

CREX_API void crex_ptr_stack_reverse_apply(crex_ptr_stack *stack, void (*func)(void *))
{
	int i = 0;

	while (i < stack->top) {
		func(stack->elements[i++]);
	}
}


CREX_API void crex_ptr_stack_clean(crex_ptr_stack *stack, void (*func)(void *), bool free_elements)
{
	crex_ptr_stack_apply(stack, func);
	if (free_elements) {
		int i = stack->top;

		while (--i >= 0) {
			pefree(stack->elements[i], stack->persistent);
		}
	}
	stack->top = 0;
	stack->top_element = stack->elements;
}


CREX_API int crex_ptr_stack_num_elements(crex_ptr_stack *stack)
{
	return stack->top;
}
