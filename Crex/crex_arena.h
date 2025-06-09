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
   | Authors: Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef _CREX_ARENA_H_
#define _CREX_ARENA_H_

#include "crex.h"

#ifndef CREX_TRACK_ARENA_ALLOC

typedef struct _crex_arena crex_arena;

struct _crex_arena {
	char		*ptr;
	char		*end;
	crex_arena  *prev;
};

static crex_always_inline crex_arena* crex_arena_create(size_t size)
{
	crex_arena *arena = (crex_arena*)emalloc(size);

	arena->ptr = (char*) arena + CREX_MM_ALIGNED_SIZE(sizeof(crex_arena));
	arena->end = (char*) arena + size;
	arena->prev = NULL;
	return arena;
}

static crex_always_inline void crex_arena_destroy(crex_arena *arena)
{
	do {
		crex_arena *prev = arena->prev;
		efree(arena);
		arena = prev;
	} while (arena);
}

static crex_always_inline void* crex_arena_alloc(crex_arena **arena_ptr, size_t size)
{
	crex_arena *arena = *arena_ptr;
	char *ptr = arena->ptr;

	size = CREX_MM_ALIGNED_SIZE(size);

	if (EXPECTED(size <= (size_t)(arena->end - ptr))) {
		arena->ptr = ptr + size;
	} else {
		size_t arena_size =
			UNEXPECTED((size + CREX_MM_ALIGNED_SIZE(sizeof(crex_arena))) > (size_t)(arena->end - (char*) arena)) ?
				(size + CREX_MM_ALIGNED_SIZE(sizeof(crex_arena))) :
				(size_t)(arena->end - (char*) arena);
		crex_arena *new_arena = (crex_arena*)emalloc(arena_size);

		ptr = (char*) new_arena + CREX_MM_ALIGNED_SIZE(sizeof(crex_arena));
		new_arena->ptr = (char*) new_arena + CREX_MM_ALIGNED_SIZE(sizeof(crex_arena)) + size;
		new_arena->end = (char*) new_arena + arena_size;
		new_arena->prev = arena;
		*arena_ptr = new_arena;
	}

	return (void*) ptr;
}

static crex_always_inline void* crex_arena_calloc(crex_arena **arena_ptr, size_t count, size_t unit_size)
{
	bool overflow;
	size_t size;
	void *ret;

	size = crex_safe_address(unit_size, count, 0, &overflow);
	if (UNEXPECTED(overflow)) {
		crex_error(E_ERROR, "Possible integer overflow in crex_arena_calloc() (%zu * %zu)", unit_size, count);
	}
	ret = crex_arena_alloc(arena_ptr, size);
	memset(ret, 0, size);
	return ret;
}

static crex_always_inline void* crex_arena_checkpoint(crex_arena *arena)
{
	return arena->ptr;
}

static crex_always_inline void crex_arena_release(crex_arena **arena_ptr, void *checkpoint)
{
	crex_arena *arena = *arena_ptr;

	while (UNEXPECTED((char*)checkpoint > arena->end) ||
	       UNEXPECTED((char*)checkpoint <= (char*)arena)) {
		crex_arena *prev = arena->prev;
		efree(arena);
		*arena_ptr = arena = prev;
	}
	CREX_ASSERT((char*)checkpoint > (char*)arena && (char*)checkpoint <= arena->end);
	arena->ptr = (char*)checkpoint;
}

static crex_always_inline bool crex_arena_contains(crex_arena *arena, void *ptr)
{
	while (arena) {
		if ((char*)ptr > (char*)arena && (char*)ptr <= arena->ptr) {
			return 1;
		}
		arena = arena->prev;
	}
	return 0;
}

#else

/* Use normal allocations and keep track of them for mass-freeing.
 * This is intended for use with asan/valgrind. */

typedef struct _crex_arena crex_arena;

struct _crex_arena {
	void **ptr;
	void **end;
	struct _crex_arena *prev;
	void *ptrs[0];
};

#define CREX_TRACKED_ARENA_SIZE 1000

static crex_always_inline crex_arena *crex_arena_create(size_t _size)
{
	crex_arena *arena = (crex_arena*) emalloc(
		sizeof(crex_arena) + sizeof(void *) * CREX_TRACKED_ARENA_SIZE);
	arena->ptr = &arena->ptrs[0];
	arena->end = &arena->ptrs[CREX_TRACKED_ARENA_SIZE];
	arena->prev = NULL;
	return arena;
}

static crex_always_inline void crex_arena_destroy(crex_arena *arena)
{
	do {
		crex_arena *prev = arena->prev;
		void **ptr;
		for (ptr = arena->ptrs; ptr < arena->ptr; ptr++) {
			efree(*ptr);
		}
		efree(arena);
		arena = prev;
	} while (arena);
}

static crex_always_inline void *crex_arena_alloc(crex_arena **arena_ptr, size_t size)
{
	crex_arena *arena = *arena_ptr;
	if (arena->ptr == arena->end) {
		*arena_ptr = crex_arena_create(0);
		(*arena_ptr)->prev = arena;
		arena = *arena_ptr;
	}

	return *arena->ptr++ = emalloc(size);
}

static crex_always_inline void* crex_arena_calloc(crex_arena **arena_ptr, size_t count, size_t unit_size)
{
	bool overflow;
	size_t size;
	void *ret;

	size = crex_safe_address(unit_size, count, 0, &overflow);
	if (UNEXPECTED(overflow)) {
		crex_error(E_ERROR, "Possible integer overflow in crex_arena_calloc() (%zu * %zu)", unit_size, count);
	}
	ret = crex_arena_alloc(arena_ptr, size);
	memset(ret, 0, size);
	return ret;
}

static crex_always_inline void* crex_arena_checkpoint(crex_arena *arena)
{
	return arena->ptr;
}

static crex_always_inline void crex_arena_release(crex_arena **arena_ptr, void *checkpoint)
{
	while (1) {
		crex_arena *arena = *arena_ptr;
		crex_arena *prev = arena->prev;
		while (1) {
			if (arena->ptr == (void **) checkpoint) {
				return;
			}
			if (arena->ptr == arena->ptrs) {
				break;
			}
			arena->ptr--;
			efree(*arena->ptr);
		}
		efree(arena);
		*arena_ptr = prev;
		CREX_ASSERT(*arena_ptr);
	}
}

static crex_always_inline bool crex_arena_contains(crex_arena *arena, void *ptr)
{
	/* TODO: Dummy */
	return 1;
}

#endif

#endif /* _CREX_ARENA_H_ */
