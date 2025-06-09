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
   | Authors: Gustavo Lopes <cataphract@crx.net>                          |
   +----------------------------------------------------------------------+
 */

#ifndef BREAKITERATOR_CLASS_H
#define BREAKITERATOR_CLASS_H

//redefinition of inline in CRX headers causes problems, so include this before
#include <math.h>

#include <crx.h>
#include "../intl_error.h"
#include "../intl_data.h"

#ifndef USE_BREAKITERATOR_POINTER
typedef void BreakIterator;
#else
using icu::BreakIterator;
#endif

typedef struct {
	// 	error handling
	intl_error  err;

	// ICU break iterator
	BreakIterator*	biter;

	// current text
	zval text;

	crex_object	zo;
} BreakIterator_object;

static inline BreakIterator_object *crx_intl_breakiterator_fetch_object(crex_object *obj) {
	return (BreakIterator_object *)((char*)(obj) - XtOffsetOf(BreakIterator_object, zo));
}
#define C_INTL_BREAKITERATOR_P(zv) crx_intl_breakiterator_fetch_object(C_OBJ_P(zv))

#define BREAKITER_ERROR(bio)		(bio)->err
#define BREAKITER_ERROR_P(bio)		&(BREAKITER_ERROR(bio))

#define BREAKITER_ERROR_CODE(bio)	INTL_ERROR_CODE(BREAKITER_ERROR(bio))
#define BREAKITER_ERROR_CODE_P(bio)	&(INTL_ERROR_CODE(BREAKITER_ERROR(bio)))

#define BREAKITER_METHOD_INIT_VARS		        INTL_METHOD_INIT_VARS(BreakIterator, bio)
#define BREAKITER_METHOD_FETCH_OBJECT_NO_CHECK	INTL_METHOD_FETCH_OBJECT(INTL_BREAKITERATOR, bio)
#define BREAKITER_METHOD_FETCH_OBJECT \
	BREAKITER_METHOD_FETCH_OBJECT_NO_CHECK; \
	if (bio->biter == NULL) \
	{ \
		crex_throw_error(NULL, "Found unconstructed BreakIterator"); \
		RETURN_THROWS(); \
	}

void breakiterator_object_create(zval *object, BreakIterator *break_iter, int brand_new);

void breakiterator_object_construct(zval *object, BreakIterator *break_iter);

void breakiterator_register_BreakIterator_class(void);

extern crex_class_entry *BreakIterator_ce_ptr,
						*RuleBasedBreakIterator_ce_ptr;

extern crex_object_handlers BreakIterator_handlers;

#endif /* #ifndef BREAKITERATOR_CLASS_H */
