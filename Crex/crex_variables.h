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
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_VARIABLES_H
#define CREX_VARIABLES_H

#include "crex_types.h"
#include "crex_gc.h"

BEGIN_EXTERN_C()

CREX_API void CREX_FASTCALL rc_dtor_func(crex_refcounted *p);
CREX_API void CREX_FASTCALL zval_copy_ctor_func(zval *zvalue);

static crex_always_inline void zval_ptr_dtor_nogc(zval *zval_ptr)
{
	if (C_REFCOUNTED_P(zval_ptr) && !C_DELREF_P(zval_ptr)) {
		rc_dtor_func(C_COUNTED_P(zval_ptr));
	}
}

static crex_always_inline void i_zval_ptr_dtor(zval *zval_ptr)
{
	if (C_REFCOUNTED_P(zval_ptr)) {
		crex_refcounted *ref = C_COUNTED_P(zval_ptr);
		if (!GC_DELREF(ref)) {
			rc_dtor_func(ref);
		} else {
			gc_check_possible_root(ref);
		}
	}
}

static crex_always_inline void zval_copy_ctor(zval *zvalue)
{
	if (C_TYPE_P(zvalue) == IS_ARRAY) {
		ZVAL_ARR(zvalue, crex_array_dup(C_ARR_P(zvalue)));
	} else if (C_REFCOUNTED_P(zvalue)) {
		C_ADDREF_P(zvalue);
	}
}

static crex_always_inline void zval_opt_copy_ctor(zval *zvalue)
{
	if (C_OPT_TYPE_P(zvalue) == IS_ARRAY) {
		ZVAL_ARR(zvalue, crex_array_dup(C_ARR_P(zvalue)));
	} else if (C_OPT_REFCOUNTED_P(zvalue)) {
		C_ADDREF_P(zvalue);
	}
}

static crex_always_inline void zval_ptr_dtor_str(zval *zval_ptr)
{
	if (C_REFCOUNTED_P(zval_ptr) && !C_DELREF_P(zval_ptr)) {
		CREX_ASSERT(C_TYPE_P(zval_ptr) == IS_STRING);
		CREX_ASSERT(!ZSTR_IS_INTERNED(C_STR_P(zval_ptr)));
		CREX_ASSERT(!(GC_FLAGS(C_STR_P(zval_ptr)) & IS_STR_PERSISTENT));
		efree(C_STR_P(zval_ptr));
	}
}

CREX_API void zval_ptr_dtor(zval *zval_ptr);
CREX_API void zval_internal_ptr_dtor(zval *zvalue);

/* Kept for compatibility */
#define zval_dtor(zvalue) zval_ptr_dtor_nogc(zvalue)

CREX_API void zval_add_ref(zval *p);

END_EXTERN_C()

#define ZVAL_PTR_DTOR zval_ptr_dtor
#define ZVAL_INTERNAL_PTR_DTOR zval_internal_ptr_dtor

#endif
