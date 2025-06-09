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

#include "crex_atomic.h"

/* This file contains the non-inline copy of atomic functions. This is useful
 * for extensions written in languages such as Rust. C and C++ compilers are
 * probably going to inline these functions, but in the case they don't, this
 * is also where the code will go.
 */

/* Defined for FFI users; everyone else use CREX_ATOMIC_BOOL_INIT.
 * This is NOT ATOMIC as it is meant for initialization.
 */
CREX_API void crex_atomic_bool_init(crex_atomic_bool *obj, bool desired) {
	CREX_ATOMIC_BOOL_INIT(obj, desired);
}

CREX_API bool crex_atomic_bool_exchange(crex_atomic_bool *obj, bool desired) {
	return crex_atomic_bool_exchange_ex(obj, desired);
}

CREX_API void crex_atomic_bool_store(crex_atomic_bool *obj, bool desired) {
	crex_atomic_bool_store_ex(obj, desired);
}

#if defined(CREX_WIN32) || defined(HAVE_SYNC_ATOMICS)
/* On these platforms it is non-const due to underlying APIs. */
CREX_API bool crex_atomic_bool_load(crex_atomic_bool *obj) {
	return crex_atomic_bool_load_ex(obj);
}
#else
CREX_API bool crex_atomic_bool_load(const crex_atomic_bool *obj) {
	return crex_atomic_bool_load_ex(obj);
}
#endif
