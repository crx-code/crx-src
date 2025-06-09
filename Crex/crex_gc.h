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
   | Authors: David Wang <planetbeing@gmail.com>                          |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_GC_H
#define CREX_GC_H

#include "crex_hrtime.h"

#ifndef GC_BENCH
# define GC_BENCH 0
#endif

BEGIN_EXTERN_C()

typedef struct _crex_gc_status {
	bool active;
	bool gc_protected;
	bool full;
	uint32_t runs;
	uint32_t collected;
	uint32_t threshold;
	uint32_t buf_size;
	uint32_t num_roots;
	crex_hrtime_t application_time;
	crex_hrtime_t collector_time;
	crex_hrtime_t dtor_time;
	crex_hrtime_t free_time;
} crex_gc_status;

CREX_API extern int (*gc_collect_cycles)(void);

CREX_API void CREX_FASTCALL gc_possible_root(crex_refcounted *ref);
CREX_API void CREX_FASTCALL gc_remove_from_buffer(crex_refcounted *ref);

/* enable/disable automatic start of GC collection */
CREX_API bool gc_enable(bool enable);
CREX_API bool gc_enabled(void);

/* enable/disable possible root additions */
CREX_API bool gc_protect(bool protect);
CREX_API bool gc_protected(void);

#if GC_BENCH
void gc_bench_print(void);
#endif

/* The default implementation of the gc_collect_cycles callback. */
CREX_API int  crex_gc_collect_cycles(void);

CREX_API void crex_gc_get_status(crex_gc_status *status);

void gc_globals_ctor(void);
void gc_globals_dtor(void);
void gc_reset(void);

#ifdef ZTS
size_t crex_gc_globals_size(void);
#endif

#define GC_REMOVE_FROM_BUFFER(p) do { \
		crex_refcounted *_p = (crex_refcounted*)(p); \
		if (GC_TYPE_INFO(_p) & GC_INFO_MASK) { \
			gc_remove_from_buffer(_p); \
		} \
	} while (0)

#define GC_MAY_LEAK(ref) \
	((GC_TYPE_INFO(ref) & \
		(GC_INFO_MASK | (GC_NOT_COLLECTABLE << GC_FLAGS_SHIFT))) == 0)

static crex_always_inline void gc_check_possible_root(crex_refcounted *ref)
{
	if (EXPECTED(GC_TYPE_INFO(ref) == GC_REFERENCE)) {
		zval *zv = &((crex_reference*)ref)->val;

		if (!C_COLLECTABLE_P(zv)) {
			return;
		}
		ref = C_COUNTED_P(zv);
	}
	if (UNEXPECTED(GC_MAY_LEAK(ref))) {
		gc_possible_root(ref);
	}
}

static crex_always_inline void gc_check_possible_root_no_ref(crex_refcounted *ref)
{
	CREX_ASSERT(GC_TYPE_INFO(ref) != GC_REFERENCE);
	if (UNEXPECTED(GC_MAY_LEAK(ref))) {
		gc_possible_root(ref);
	}
}

/* These APIs can be used to simplify object get_gc implementations
 * over heterogeneous structures. See crex_generator_get_gc() for
 * a usage example. */

typedef struct {
	zval *cur;
	zval *end;
	zval *start;
} crex_get_gc_buffer;

CREX_API crex_get_gc_buffer *crex_get_gc_buffer_create(void);
CREX_API void crex_get_gc_buffer_grow(crex_get_gc_buffer *gc_buffer);

static crex_always_inline void crex_get_gc_buffer_add_zval(
		crex_get_gc_buffer *gc_buffer, zval *zv) {
	if (C_REFCOUNTED_P(zv)) {
		if (UNEXPECTED(gc_buffer->cur == gc_buffer->end)) {
			crex_get_gc_buffer_grow(gc_buffer);
		}
		ZVAL_COPY_VALUE(gc_buffer->cur, zv);
		gc_buffer->cur++;
	}
}

static crex_always_inline void crex_get_gc_buffer_add_obj(
		crex_get_gc_buffer *gc_buffer, crex_object *obj) {
	if (UNEXPECTED(gc_buffer->cur == gc_buffer->end)) {
		crex_get_gc_buffer_grow(gc_buffer);
	}
	ZVAL_OBJ(gc_buffer->cur, obj);
	gc_buffer->cur++;
}

static crex_always_inline void crex_get_gc_buffer_add_ptr(
		crex_get_gc_buffer *gc_buffer, void *ptr) {
	if (UNEXPECTED(gc_buffer->cur == gc_buffer->end)) {
		crex_get_gc_buffer_grow(gc_buffer);
	}
	ZVAL_PTR(gc_buffer->cur, ptr);
	gc_buffer->cur++;
}

static crex_always_inline void crex_get_gc_buffer_use(
		crex_get_gc_buffer *gc_buffer, zval **table, int *n) {
	*table = gc_buffer->start;
	*n = gc_buffer->cur - gc_buffer->start;
}

END_EXTERN_C()

#endif /* CREX_GC_H */
