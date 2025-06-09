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
   | Authors: Stanislav Malyshev <stas@crex.com>                          |
   +----------------------------------------------------------------------+
 */

#ifndef MSG_FORMAT_CLASS_H
#define MSG_FORMAT_CLASS_H

#include <crx.h>

#include <unicode/uconfig.h>

#include "../intl_common.h"
#include "../intl_error.h"
#include "../intl_data.h"

#include "msgformat_data.h"

typedef struct {
	msgformat_data  mf_data;
	crex_object     zo;
} MessageFormatter_object;


static inline MessageFormatter_object *crx_intl_messageformatter_fetch_object(crex_object *obj) {
	return (MessageFormatter_object *)((char*)(obj) - XtOffsetOf(MessageFormatter_object, zo));
}
#define C_INTL_MESSAGEFORMATTER_P(zv) crx_intl_messageformatter_fetch_object(C_OBJ_P(zv))

void msgformat_register_class( void );
extern crex_class_entry *MessageFormatter_ce_ptr;

/* Auxiliary macros */

#define MSG_FORMAT_METHOD_INIT_VARS		INTL_METHOD_INIT_VARS(MessageFormatter, mfo)
#define MSG_FORMAT_METHOD_FETCH_OBJECT_NO_CHECK	INTL_METHOD_FETCH_OBJECT(INTL_MESSAGEFORMATTER, mfo)
#define MSG_FORMAT_METHOD_FETCH_OBJECT									\
	MSG_FORMAT_METHOD_FETCH_OBJECT_NO_CHECK;							\
	if (MSG_FORMAT_OBJECT(mfo) == NULL)	{								\
		crex_throw_error(NULL, "Found unconstructed MessageFormatter");	\
		RETURN_THROWS();												\
	}

#define MSG_FORMAT_OBJECT(mfo)			(mfo)->mf_data.umsgf

#endif // #ifndef MSG_FORMAT_CLASS_H
