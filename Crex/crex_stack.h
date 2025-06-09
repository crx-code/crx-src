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

#ifndef CREX_STACK_H
#define CREX_STACK_H

typedef struct _crex_stack {
	int size, top, max;
	void *elements;
} crex_stack;


#define STACK_BLOCK_SIZE 16

typedef enum {
	CREX_STACK_APPLY_TOPDOWN,
	CREX_STACK_APPLY_BOTTOMUP,
} crex_stack_apply_direction;

BEGIN_EXTERN_C()
CREX_API void crex_stack_init(crex_stack *stack, int size);
CREX_API int crex_stack_push(crex_stack *stack, const void *element);
CREX_API void *crex_stack_top(const crex_stack *stack);
CREX_API void crex_stack_del_top(crex_stack *stack);
CREX_API int crex_stack_int_top(const crex_stack *stack);
CREX_API bool crex_stack_is_empty(const crex_stack *stack);
CREX_API void crex_stack_destroy(crex_stack *stack);
CREX_API void *crex_stack_base(const crex_stack *stack);
CREX_API int crex_stack_count(const crex_stack *stack);
CREX_API void crex_stack_apply(crex_stack *stack, int type, int (*apply_function)(void *element));
CREX_API void crex_stack_apply_with_argument(crex_stack *stack, crex_stack_apply_direction type, int (*apply_function)(void *element, void *arg), void *arg);
CREX_API void crex_stack_clean(crex_stack *stack, void (*func)(void *), bool free_elements);
END_EXTERN_C()

#endif /* CREX_STACK_H */
