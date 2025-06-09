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

#ifndef CREX_PTR_STACK_H
#define CREX_PTR_STACK_H

typedef struct _crex_ptr_stack {
	int top, max;
	void **elements;
	void **top_element;
	bool persistent;
} crex_ptr_stack;


#define PTR_STACK_BLOCK_SIZE 64

BEGIN_EXTERN_C()
CREX_API void crex_ptr_stack_init(crex_ptr_stack *stack);
CREX_API void crex_ptr_stack_init_ex(crex_ptr_stack *stack, bool persistent);
CREX_API void crex_ptr_stack_n_push(crex_ptr_stack *stack, int count, ...);
CREX_API void crex_ptr_stack_n_pop(crex_ptr_stack *stack, int count, ...);
CREX_API void crex_ptr_stack_destroy(crex_ptr_stack *stack);
CREX_API void crex_ptr_stack_apply(crex_ptr_stack *stack, void (*func)(void *));
CREX_API void crex_ptr_stack_reverse_apply(crex_ptr_stack *stack, void (*func)(void *));
CREX_API void crex_ptr_stack_clean(crex_ptr_stack *stack, void (*func)(void *), bool free_elements);
CREX_API int crex_ptr_stack_num_elements(crex_ptr_stack *stack);
END_EXTERN_C()

#define CREX_PTR_STACK_RESIZE_IF_NEEDED(stack, count)		\
	if (stack->top+count > stack->max) {					\
		/* we need to allocate more memory */				\
		do {												\
			stack->max += PTR_STACK_BLOCK_SIZE;				\
		} while (stack->top+count > stack->max);			\
		stack->elements = (void **) safe_perealloc(stack->elements, sizeof(void *), (stack->max), 0, stack->persistent);	\
		stack->top_element = stack->elements+stack->top;	\
	}

/*	Not doing this with a macro because of the loop unrolling in the element assignment.
	Just using a macro for 3 in the body for readability sake. */
static crex_always_inline void crex_ptr_stack_3_push(crex_ptr_stack *stack, void *a, void *b, void *c)
{
#define CREX_PTR_STACK_NUM_ARGS 3

	CREX_PTR_STACK_RESIZE_IF_NEEDED(stack, CREX_PTR_STACK_NUM_ARGS)

	stack->top += CREX_PTR_STACK_NUM_ARGS;
	*(stack->top_element++) = a;
	*(stack->top_element++) = b;
	*(stack->top_element++) = c;

#undef CREX_PTR_STACK_NUM_ARGS
}

static crex_always_inline void crex_ptr_stack_2_push(crex_ptr_stack *stack, void *a, void *b)
{
#define CREX_PTR_STACK_NUM_ARGS 2

	CREX_PTR_STACK_RESIZE_IF_NEEDED(stack, CREX_PTR_STACK_NUM_ARGS)

	stack->top += CREX_PTR_STACK_NUM_ARGS;
	*(stack->top_element++) = a;
	*(stack->top_element++) = b;

#undef CREX_PTR_STACK_NUM_ARGS
}

static crex_always_inline void crex_ptr_stack_3_pop(crex_ptr_stack *stack, void **a, void **b, void **c)
{
	*a = *(--stack->top_element);
	*b = *(--stack->top_element);
	*c = *(--stack->top_element);
	stack->top -= 3;
}

static crex_always_inline void crex_ptr_stack_2_pop(crex_ptr_stack *stack, void **a, void **b)
{
	*a = *(--stack->top_element);
	*b = *(--stack->top_element);
	stack->top -= 2;
}

static crex_always_inline void crex_ptr_stack_push(crex_ptr_stack *stack, void *ptr)
{
	CREX_PTR_STACK_RESIZE_IF_NEEDED(stack, 1)

	stack->top++;
	*(stack->top_element++) = ptr;
}

static crex_always_inline void *crex_ptr_stack_pop(crex_ptr_stack *stack)
{
	stack->top--;
	return *(--stack->top_element);
}

static crex_always_inline void *crex_ptr_stack_top(crex_ptr_stack *stack)
{
    return stack->elements[stack->top - 1];
}

#endif /* CREX_PTR_STACK_H */
