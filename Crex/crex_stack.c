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
#include "crex_stack.h"

#define CREX_STACK_ELEMENT(stack, n) ((void *)((char *) (stack)->elements + (stack)->size * (n)))

CREX_API void crex_stack_init(crex_stack *stack, int size)
{
	stack->size = size;
	stack->top = 0;
	stack->max = 0;
	stack->elements = NULL;
}

CREX_API int crex_stack_push(crex_stack *stack, const void *element)
{
	/* We need to allocate more memory */
	if (stack->top >= stack->max) {
		stack->max += STACK_BLOCK_SIZE;
		stack->elements = safe_erealloc(stack->elements, stack->size, stack->max, 0);
	}
	memcpy(CREX_STACK_ELEMENT(stack, stack->top), element, stack->size);
	return stack->top++;
}


CREX_API void *crex_stack_top(const crex_stack *stack)
{
	if (stack->top > 0) {
		return CREX_STACK_ELEMENT(stack, stack->top - 1);
	} else {
		return NULL;
	}
}


CREX_API void crex_stack_del_top(crex_stack *stack)
{
	--stack->top;
}


CREX_API int crex_stack_int_top(const crex_stack *stack)
{
	int *e = crex_stack_top(stack);
	if (e) {
		return *e;
	} else {
		return FAILURE;
	}
}


CREX_API bool crex_stack_is_empty(const crex_stack *stack)
{
	return stack->top == 0;
}


CREX_API void crex_stack_destroy(crex_stack *stack)
{
	if (stack->elements) {
		efree(stack->elements);
		stack->elements = NULL;
	}
}


CREX_API void *crex_stack_base(const crex_stack *stack)
{
	return stack->elements;
}


CREX_API int crex_stack_count(const crex_stack *stack)
{
	return stack->top;
}


CREX_API void crex_stack_apply(crex_stack *stack, int type, int (*apply_function)(void *element))
{
	int i;

	switch (type) {
		case CREX_STACK_APPLY_TOPDOWN:
			for (i=stack->top-1; i>=0; i--) {
				if (apply_function(CREX_STACK_ELEMENT(stack, i))) {
					break;
				}
			}
			break;
		case CREX_STACK_APPLY_BOTTOMUP:
			for (i=0; i<stack->top; i++) {
				if (apply_function(CREX_STACK_ELEMENT(stack, i))) {
					break;
				}
			}
			break;
	}
}


CREX_API void crex_stack_apply_with_argument(crex_stack *stack, crex_stack_apply_direction type, int (*apply_function)(void *element, void *arg), void *arg)
{
	int i;

	switch (type) {
		case CREX_STACK_APPLY_TOPDOWN:
			for (i=stack->top-1; i>=0; i--) {
				if (apply_function(CREX_STACK_ELEMENT(stack, i), arg)) {
					break;
				}
			}
			break;
		case CREX_STACK_APPLY_BOTTOMUP:
			for (i=0; i<stack->top; i++) {
				if (apply_function(CREX_STACK_ELEMENT(stack, i), arg)) {
					break;
				}
			}
			break;
	}
}

CREX_API void crex_stack_clean(crex_stack *stack, void (*func)(void *), bool free_elements)
{
	int i;

	if (func) {
		for (i = 0; i < stack->top; i++) {
			func(CREX_STACK_ELEMENT(stack, i));
		}
	}
	if (free_elements) {
		if (stack->elements) {
			efree(stack->elements);
			stack->elements = NULL;
		}
		stack->top = stack->max = 0;
	}
}
