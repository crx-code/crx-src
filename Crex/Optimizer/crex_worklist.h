/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
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
   | Authors: Andy Wingo <wingo@igalia.com>                               |
   +----------------------------------------------------------------------+
*/

#ifndef _CREX_WORKLIST_H_
#define _CREX_WORKLIST_H_

#include "crex_arena.h"
#include "crex_bitset.h"

typedef struct _crex_worklist_stack {
	int *buf;
	int len;
	int capacity;
} crex_worklist_stack;

#define CREX_WORKLIST_STACK_ALLOCA(s, _len, use_heap) do { \
		(s)->buf = (int*)do_alloca(sizeof(int) * _len, use_heap); \
		(s)->len = 0; \
		(s)->capacity = _len; \
	} while (0)

#define CREX_WORKLIST_STACK_FREE_ALLOCA(s, use_heap) \
	free_alloca((s)->buf, use_heap)

static inline void crex_worklist_stack_prepare(crex_arena **arena, crex_worklist_stack *stack, int len)
{
	CREX_ASSERT(len >= 0);

	stack->buf = (int*)crex_arena_calloc(arena, sizeof(*stack->buf), len);
	stack->len = 0;
	stack->capacity = len;
}

static inline void crex_worklist_stack_push(crex_worklist_stack *stack, int i)
{
	CREX_ASSERT(stack->len < stack->capacity);
	stack->buf[stack->len++] = i;
}

static inline int crex_worklist_stack_peek(const crex_worklist_stack *stack)
{
	CREX_ASSERT(stack->len);
	return stack->buf[stack->len - 1];
}

static inline int crex_worklist_stack_pop(crex_worklist_stack *stack)
{
	CREX_ASSERT(stack->len);
	return stack->buf[--stack->len];
}

typedef struct _crex_worklist {
	crex_bitset visited;
	crex_worklist_stack stack;
} crex_worklist;

#define CREX_WORKLIST_ALLOCA(w, _len, use_heap) do { \
		(w)->stack.buf = (int*)do_alloca(CREX_MM_ALIGNED_SIZE(sizeof(int) * _len) + sizeof(crex_ulong) * crex_bitset_len(_len), use_heap); \
		(w)->stack.len = 0; \
		(w)->stack.capacity = _len; \
		(w)->visited = (crex_bitset)((char*)(w)->stack.buf + CREX_MM_ALIGNED_SIZE(sizeof(int) * _len)); \
		memset((w)->visited, 0, sizeof(crex_ulong) * crex_bitset_len(_len)); \
	} while (0)

#define CREX_WORKLIST_FREE_ALLOCA(w, use_heap) \
	free_alloca((w)->stack.buf, use_heap)

static inline void crex_worklist_prepare(crex_arena **arena, crex_worklist *worklist, int len)
{
	CREX_ASSERT(len >= 0);
	worklist->visited = (crex_bitset)crex_arena_calloc(arena, sizeof(crex_ulong), crex_bitset_len(len));
	crex_worklist_stack_prepare(arena, &worklist->stack, len);
}

static inline int crex_worklist_len(const crex_worklist *worklist)
{
	return worklist->stack.len;
}

static inline bool crex_worklist_push(crex_worklist *worklist, int i)
{
	CREX_ASSERT(i >= 0 && i < worklist->stack.capacity);

	if (crex_bitset_in(worklist->visited, i)) {
		return 0;
	}

	crex_bitset_incl(worklist->visited, i);
	crex_worklist_stack_push(&worklist->stack, i);
	return 1;
}

static inline int crex_worklist_peek(const crex_worklist *worklist)
{
	return crex_worklist_stack_peek(&worklist->stack);
}

static inline int crex_worklist_pop(crex_worklist *worklist)
{
	/* Does not clear visited flag */
	return crex_worklist_stack_pop(&worklist->stack);
}

#endif /* _CREX_WORKLIST_H_ */
