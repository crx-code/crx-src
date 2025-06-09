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
   | Authors: Scott MacVicar <scottmac@crx.net>                           |
   +----------------------------------------------------------------------+
 */

#ifndef SPOOFCHECKER_CLASS_H
#define SPOOFCHECKER_CLASS_H

#include <crx.h>

#include "intl_common.h"
#include "intl_error.h"
#include "intl_data.h"

#include <unicode/uspoof.h>

typedef struct {
	// error handling
	intl_error  err;

	// ICU Spoofchecker
	USpoofChecker*     uspoof;
#if U_ICU_VERSION_MAJOR_NUM >= 58
	USpoofCheckResult* uspoofres;
#endif

	crex_object     zo;
} Spoofchecker_object;

static inline Spoofchecker_object *crx_intl_spoofchecker_fetch_object(crex_object *obj) {
	    return (Spoofchecker_object *)((char*)(obj) - XtOffsetOf(Spoofchecker_object, zo));
}
#define C_INTL_SPOOFCHECKER_P(zv) crx_intl_spoofchecker_fetch_object((C_OBJ_P(zv)))

#define SPOOFCHECKER_ERROR(co) (co)->err
#define SPOOFCHECKER_ERROR_P(co) &(SPOOFCHECKER_ERROR(co))

#define SPOOFCHECKER_ERROR_CODE(co)   INTL_ERROR_CODE(SPOOFCHECKER_ERROR(co))
#define SPOOFCHECKER_ERROR_CODE_P(co) &(INTL_ERROR_CODE(SPOOFCHECKER_ERROR(co)))

void spoofchecker_register_Spoofchecker_class(void);

void spoofchecker_object_init(Spoofchecker_object* co);
void spoofchecker_object_destroy(Spoofchecker_object* co);

extern crex_class_entry *Spoofchecker_ce_ptr;

/* Auxiliary macros */

#define SPOOFCHECKER_METHOD_INIT_VARS       \
    zval*             object  = CREX_THIS;  \
    Spoofchecker_object*  co  = NULL;   \
    intl_error_reset(NULL); \

#define SPOOFCHECKER_METHOD_FETCH_OBJECT_NO_CHECK	INTL_METHOD_FETCH_OBJECT(INTL_SPOOFCHECKER, co)
#define SPOOFCHECKER_METHOD_FETCH_OBJECT							\
	SPOOFCHECKER_METHOD_FETCH_OBJECT_NO_CHECK;						\
	if (co->uspoof == NULL)	{										\
		crex_throw_error(NULL, "Found unconstructed Spoofchecker");	\
		RETURN_THROWS();											\
	}

// Macro to check return value of a ucol_* function call.
#define SPOOFCHECKER_CHECK_STATUS(co, msg)                                        \
    intl_error_set_code(NULL, SPOOFCHECKER_ERROR_CODE(co));           \
    if (U_FAILURE(SPOOFCHECKER_ERROR_CODE(co))) {                                  \
        intl_errors_set_custom_msg(SPOOFCHECKER_ERROR_P(co), msg, 0); \
        RETURN_FALSE;                                                           \
    }                                                                           \

#if U_ICU_VERSION_MAJOR_NUM >= 58
#define SPOOFCHECKER_DEFAULT_RESTRICTION_LEVEL USPOOF_HIGHLY_RESTRICTIVE
#endif

#endif // #ifndef SPOOFCHECKER_CLASS_H
