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

#include <stdio.h>
#include "crex.h"
#include "crex_API.h"
#include "crex_ast.h"
#include "crex_globals.h"
#include "crex_constants.h"
#include "crex_list.h"

#if CREX_DEBUG
static void CREX_FASTCALL crex_string_destroy(crex_string *str);
#else
# define crex_string_destroy _efree
#endif
static void CREX_FASTCALL crex_reference_destroy(crex_reference *ref);
static void CREX_FASTCALL crex_empty_destroy(crex_reference *ref);

typedef void (CREX_FASTCALL *crex_rc_dtor_func_t)(crex_refcounted *p);

static const crex_rc_dtor_func_t crex_rc_dtor_func[] = {
	[IS_UNDEF] =        (crex_rc_dtor_func_t)crex_empty_destroy,
	[IS_NULL] =         (crex_rc_dtor_func_t)crex_empty_destroy,
	[IS_FALSE] =        (crex_rc_dtor_func_t)crex_empty_destroy,
	[IS_TRUE] =         (crex_rc_dtor_func_t)crex_empty_destroy,
	[IS_LONG] =         (crex_rc_dtor_func_t)crex_empty_destroy,
	[IS_DOUBLE] =       (crex_rc_dtor_func_t)crex_empty_destroy,
	[IS_STRING] =       (crex_rc_dtor_func_t)crex_string_destroy,
	[IS_ARRAY] =        (crex_rc_dtor_func_t)crex_array_destroy,
	[IS_OBJECT] =       (crex_rc_dtor_func_t)crex_objects_store_del,
	[IS_RESOURCE] =     (crex_rc_dtor_func_t)crex_list_free,
	[IS_REFERENCE] =    (crex_rc_dtor_func_t)crex_reference_destroy,
	[IS_CONSTANT_AST] = (crex_rc_dtor_func_t)crex_ast_ref_destroy
};

CREX_API void CREX_FASTCALL rc_dtor_func(crex_refcounted *p)
{
	CREX_ASSERT(GC_TYPE(p) <= IS_CONSTANT_AST);
	crex_rc_dtor_func[GC_TYPE(p)](p);
}

#if CREX_DEBUG
static void CREX_FASTCALL crex_string_destroy(crex_string *str)
{
	CHECK_ZVAL_STRING(str);
	CREX_ASSERT(!ZSTR_IS_INTERNED(str));
	CREX_ASSERT(GC_REFCOUNT(str) == 0);
	CREX_ASSERT(!(GC_FLAGS(str) & IS_STR_PERSISTENT));
	efree(str);
}
#endif

static void CREX_FASTCALL crex_reference_destroy(crex_reference *ref)
{
	CREX_ASSERT(!CREX_REF_HAS_TYPE_SOURCES(ref));
	i_zval_ptr_dtor(&ref->val);
	efree_size(ref, sizeof(crex_reference));
}

static void CREX_FASTCALL crex_empty_destroy(crex_reference *ref)
{
}

CREX_API void zval_ptr_dtor(zval *zval_ptr) /* {{{ */
{
	i_zval_ptr_dtor(zval_ptr);
}
/* }}} */

CREX_API void zval_internal_ptr_dtor(zval *zval_ptr) /* {{{ */
{
	if (C_REFCOUNTED_P(zval_ptr)) {
		crex_refcounted *ref = C_COUNTED_P(zval_ptr);

		if (GC_DELREF(ref) == 0) {
			if (C_TYPE_P(zval_ptr) == IS_STRING) {
				crex_string *str = (crex_string*)ref;

				CHECK_ZVAL_STRING(str);
				CREX_ASSERT(!ZSTR_IS_INTERNED(str));
				CREX_ASSERT((GC_FLAGS(str) & IS_STR_PERSISTENT));
				free(str);
			} else {
				crex_error_noreturn(E_CORE_ERROR, "Internal zval's can't be arrays, objects, resources or reference");
			}
		}
	}
}
/* }}} */

/* This function should only be used as a copy constructor, i.e. it
 * should only be called AFTER a zval has been copied to another
 * location using ZVAL_COPY_VALUE. Do not call it before copying,
 * otherwise a reference may be leaked. */
CREX_API void zval_add_ref(zval *p)
{
	if (C_REFCOUNTED_P(p)) {
		if (C_ISREF_P(p) && C_REFCOUNT_P(p) == 1) {
			ZVAL_COPY(p, C_REFVAL_P(p));
		} else {
			C_ADDREF_P(p);
		}
	}
}

CREX_API void CREX_FASTCALL zval_copy_ctor_func(zval *zvalue)
{
	if (EXPECTED(C_TYPE_P(zvalue) == IS_ARRAY)) {
		ZVAL_ARR(zvalue, crex_array_dup(C_ARRVAL_P(zvalue)));
	} else if (EXPECTED(C_TYPE_P(zvalue) == IS_STRING)) {
		CREX_ASSERT(!ZSTR_IS_INTERNED(C_STR_P(zvalue)));
		CHECK_ZVAL_STRING(C_STR_P(zvalue));
		ZVAL_NEW_STR(zvalue, crex_string_dup(C_STR_P(zvalue), 0));
	} else {
		CREX_UNREACHABLE();
	}
}
