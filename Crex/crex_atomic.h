/*
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the CRX license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.crx.net/license/3_01.txt                                 |
   | If you did not receive a copy of the CRX license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crx.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Levi Morrison <morrison.levi@gmail.com>                     |
   +----------------------------------------------------------------------+
 */

#ifndef CREX_ATOMIC_H
#define CREX_ATOMIC_H

#include "crex_portability.h"

#include <stdbool.h>

#define CREX_GCC_PREREQ(x, y) \
	((__GNUC__ == (x) && __GNUC_MINOR__ >= (y)) || (__GNUC__ > (x)))

/* Builtins are used to avoid library linkage */
#if __has_feature(c_atomic)
#define	HAVE_C11_ATOMICS 1
#elif CREX_GCC_PREREQ(4, 7)
#define	HAVE_GNUC_ATOMICS 1
#elif defined(__GNUC__)
#define	HAVE_SYNC_ATOMICS 1
#elif !defined(CREX_WIN32)
#define HAVE_NO_ATOMICS 1
#endif

#undef CREX_GCC_PREREQ

/* Treat crex_atomic_* types as opaque. They have definitions only for size
 * and alignment purposes.
 */

#if defined(CREX_WIN32) || defined(HAVE_SYNC_ATOMICS)
typedef struct crex_atomic_bool_s {
	volatile char value;
} crex_atomic_bool;
#elif defined(HAVE_C11_ATOMICS)
typedef struct crex_atomic_bool_s {
	_Atomic(bool) value;
} crex_atomic_bool;
#else
typedef struct crex_atomic_bool_s {
	volatile bool value;
} crex_atomic_bool;
#endif

BEGIN_EXTERN_C()

#ifdef CREX_WIN32

#ifndef InterlockedExchange8
#define InterlockedExchange8 _InterlockedExchange8
#endif
#ifndef InterlockedOr8
#define InterlockedOr8 _InterlockedOr8
#endif

#define CREX_ATOMIC_BOOL_INIT(obj, desired) ((obj)->value = (desired))

static crex_always_inline bool crex_atomic_bool_exchange_ex(crex_atomic_bool *obj, bool desired) {
	return InterlockedExchange8(&obj->value, desired);
}

/* On this platform it is non-const due to Iterlocked API*/
static crex_always_inline bool crex_atomic_bool_load_ex(crex_atomic_bool *obj) {
	/* Or'ing with false won't change the value. */
	return InterlockedOr8(&obj->value, false);
}

static crex_always_inline void crex_atomic_bool_store_ex(crex_atomic_bool *obj, bool desired) {
	(void)InterlockedExchange8(&obj->value, desired);
}

#elif defined(HAVE_C11_ATOMICS)

#define CREX_ATOMIC_BOOL_INIT(obj, desired) __c11_atomic_init(&(obj)->value, (desired))

static crex_always_inline bool crex_atomic_bool_exchange_ex(crex_atomic_bool *obj, bool desired) {
	return __c11_atomic_exchange(&obj->value, desired, __ATOMIC_SEQ_CST);
}

static crex_always_inline bool crex_atomic_bool_load_ex(const crex_atomic_bool *obj) {
	return __c11_atomic_load(&obj->value, __ATOMIC_SEQ_CST);
}

static crex_always_inline void crex_atomic_bool_store_ex(crex_atomic_bool *obj, bool desired) {
	__c11_atomic_store(&obj->value, desired, __ATOMIC_SEQ_CST);
}

#elif defined(HAVE_GNUC_ATOMICS)

#define CREX_ATOMIC_BOOL_INIT(obj, desired) ((obj)->value = (desired))

static crex_always_inline bool crex_atomic_bool_exchange_ex(crex_atomic_bool *obj, bool desired) {
	bool prev = false;
	__atomic_exchange(&obj->value, &desired, &prev, __ATOMIC_SEQ_CST);
	return prev;
}

static crex_always_inline bool crex_atomic_bool_load_ex(const crex_atomic_bool *obj) {
	bool prev = false;
	__atomic_load(&obj->value, &prev, __ATOMIC_SEQ_CST);
	return prev;
}

static crex_always_inline void crex_atomic_bool_store_ex(crex_atomic_bool *obj, bool desired) {
	__atomic_store(&obj->value, &desired, __ATOMIC_SEQ_CST);
}

#elif defined(HAVE_SYNC_ATOMICS)

#define CREX_ATOMIC_BOOL_INIT(obj, desired) ((obj)->value = (desired))

static crex_always_inline bool crex_atomic_bool_exchange_ex(crex_atomic_bool *obj, bool desired) {
	bool prev = __sync_lock_test_and_set(&obj->value, desired);

	/* __sync_lock_test_and_set only does an acquire barrier, so sync
	 * immediately after.
	 */
	__sync_synchronize();
	return prev;
}

static crex_always_inline bool crex_atomic_bool_load_ex(crex_atomic_bool *obj) {
	/* Or'ing false won't change the value */
	return __sync_fetch_and_or(&obj->value, false);
}

static crex_always_inline void crex_atomic_bool_store_ex(crex_atomic_bool *obj, bool desired) {
	__sync_synchronize();
	obj->value = desired;
	__sync_synchronize();
}

#elif defined(HAVE_NO_ATOMICS)

#warning No atomics support detected. Please open an issue with platform details.

#define CREX_ATOMIC_BOOL_INIT(obj, desired) ((obj)->value = (desired))

static crex_always_inline void crex_atomic_bool_store_ex(crex_atomic_bool *obj, bool desired) {
	obj->value = desired;
}

static crex_always_inline bool crex_atomic_bool_load_ex(const crex_atomic_bool *obj) {
	return obj->value;
}

static crex_always_inline bool crex_atomic_bool_exchange_ex(crex_atomic_bool *obj, bool desired) {
	bool prev = obj->value;
	obj->value = desired;
	return prev;
}

#endif

CREX_API void crex_atomic_bool_init(crex_atomic_bool *obj, bool desired);
CREX_API bool crex_atomic_bool_exchange(crex_atomic_bool *obj, bool desired);
CREX_API void crex_atomic_bool_store(crex_atomic_bool *obj, bool desired);

#if defined(CREX_WIN32) || defined(HAVE_SYNC_ATOMICS)
/* On these platforms it is non-const due to underlying APIs. */
CREX_API bool crex_atomic_bool_load(crex_atomic_bool *obj);
#else
CREX_API bool crex_atomic_bool_load(const crex_atomic_bool *obj);
#endif

END_EXTERN_C()

#endif
