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

#include "spoofchecker_class.h"
#include "spoofchecker_arginfo.h"
#include "crx_intl.h"
#include "intl_error.h"

#include <unicode/uspoof.h>

crex_class_entry *Spoofchecker_ce_ptr = NULL;
static crex_object_handlers Spoofchecker_handlers;

/*
 * Auxiliary functions needed by objects of 'Spoofchecker' class
 */

/* {{{ Spoofchecker_objects_free */
void Spoofchecker_objects_free(crex_object *object)
{
	Spoofchecker_object* co = crx_intl_spoofchecker_fetch_object(object);

	crex_object_std_dtor(&co->zo);

	spoofchecker_object_destroy(co);
}
/* }}} */

/* {{{ Spoofchecker_object_create */
crex_object *Spoofchecker_object_create(crex_class_entry *ce)
{
	Spoofchecker_object*     intern;

	intern = crex_object_alloc(sizeof(Spoofchecker_object), ce);
	intl_error_init(SPOOFCHECKER_ERROR_P(intern));
	crex_object_std_init(&intern->zo, ce);
	object_properties_init(&intern->zo, ce);

	return &intern->zo;
}
/* }}} */

/*
 * 'Spoofchecker' class registration structures & functions
 */

/* {{{ Spoofchecker_class_functions
 * Every 'Spoofchecker' class method has an entry in this table
 */

static crex_object *spoofchecker_clone_obj(crex_object *object) /* {{{ */
{
	crex_object *new_obj_val;
	Spoofchecker_object *sfo, *new_sfo;

	sfo = crx_intl_spoofchecker_fetch_object(object);
	intl_error_reset(SPOOFCHECKER_ERROR_P(sfo));

	new_obj_val = Spoofchecker_ce_ptr->create_object(object->ce);
	new_sfo = crx_intl_spoofchecker_fetch_object(new_obj_val);
	/* clone standard parts */
	crex_objects_clone_members(&new_sfo->zo, &sfo->zo);
	/* clone internal object */
	new_sfo->uspoof = uspoof_clone(sfo->uspoof, SPOOFCHECKER_ERROR_CODE_P(new_sfo));
	if(U_FAILURE(SPOOFCHECKER_ERROR_CODE(new_sfo))) {
		/* set up error in case error handler is interested */
		intl_error_set( NULL, SPOOFCHECKER_ERROR_CODE(new_sfo), "Failed to clone SpoofChecker object", 0 );
		Spoofchecker_objects_free(&new_sfo->zo); /* free new object */
		crex_error(E_ERROR, "Failed to clone SpoofChecker object");
	}
	return new_obj_val;
}
/* }}} */

/* {{{ spoofchecker_register_Spoofchecker_class
 * Initialize 'Spoofchecker' class
 */
void spoofchecker_register_Spoofchecker_class(void)
{
	/* Create and register 'Spoofchecker' class. */
	Spoofchecker_ce_ptr = register_class_Spoofchecker();
	Spoofchecker_ce_ptr->create_object = Spoofchecker_object_create;
	Spoofchecker_ce_ptr->default_object_handlers = &Spoofchecker_handlers;

	memcpy(&Spoofchecker_handlers, &std_object_handlers,
		sizeof Spoofchecker_handlers);
	Spoofchecker_handlers.offset = XtOffsetOf(Spoofchecker_object, zo);
	Spoofchecker_handlers.clone_obj = spoofchecker_clone_obj;
	Spoofchecker_handlers.free_obj = Spoofchecker_objects_free;
}
/* }}} */

/* {{{ void spoofchecker_object_init( Spoofchecker_object* co )
 * Initialize internals of Spoofchecker_object.
 * Must be called before any other call to 'spoofchecker_object_...' functions.
 */
void spoofchecker_object_init(Spoofchecker_object* co)
{
	if (!co) {
		return;
	}

	intl_error_init(SPOOFCHECKER_ERROR_P(co));
}
/* }}} */

/* {{{ void spoofchecker_object_destroy( Spoofchecker_object* co )
 * Clean up mem allocted by internals of Spoofchecker_object
 */
void spoofchecker_object_destroy(Spoofchecker_object* co)
{
	if (!co) {
		return;
	}

	if (co->uspoof) {
		uspoof_close(co->uspoof);
		co->uspoof = NULL;
	}

#if U_ICU_VERSION_MAJOR_NUM >= 58
	if (co->uspoofres) {
		uspoof_closeCheckResult(co->uspoofres);
		co->uspoofres = NULL;
	}
#endif

	intl_error_reset(SPOOFCHECKER_ERROR_P(co));
}
/* }}} */
