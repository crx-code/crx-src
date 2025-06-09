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

#include <unicode/unum.h>

#include "formatter_class.h"
#include "crx_intl.h"
#include "formatter_data.h"
#include "formatter_format.h"
#include "formatter_arginfo.h"

#include <crex_exceptions.h>
#include "Crex/crex_interfaces.h"

crex_class_entry *NumberFormatter_ce_ptr = NULL;
static crex_object_handlers NumberFormatter_handlers;

/*
 * Auxiliary functions needed by objects of 'NumberFormatter' class
 */

/* {{{ NumberFormatter_objects_free */
void NumberFormatter_object_free( crex_object *object )
{
	NumberFormatter_object* nfo = crx_intl_number_format_fetch_object(object);

	crex_object_std_dtor( &nfo->zo );

	formatter_data_free( &nfo->nf_data );
}
/* }}} */

/* {{{ NumberFormatter_object_create */
crex_object *NumberFormatter_object_create(crex_class_entry *ce)
{
	NumberFormatter_object*     intern;

	intern = crex_object_alloc(sizeof(NumberFormatter_object), ce);
	formatter_data_init( &intern->nf_data );
	crex_object_std_init( &intern->zo, ce );
	object_properties_init(&intern->zo, ce);

	return &intern->zo;
}
/* }}} */

/* {{{ NumberFormatter_object_clone */
crex_object *NumberFormatter_object_clone(crex_object *object)
{
	NumberFormatter_object *nfo, *new_nfo;
	crex_object *new_obj;

	nfo = crx_intl_number_format_fetch_object(object);
	intl_error_reset(INTL_DATA_ERROR_P(nfo));

	new_obj = NumberFormatter_ce_ptr->create_object(object->ce);
	new_nfo = crx_intl_number_format_fetch_object(new_obj);
	/* clone standard parts */
	crex_objects_clone_members(&new_nfo->zo, &nfo->zo);
	/* clone formatter object. It may fail, the destruction code must handle this case */
	if (FORMATTER_OBJECT(nfo) != NULL) {
		FORMATTER_OBJECT(new_nfo) = unum_clone(FORMATTER_OBJECT(nfo),
				&INTL_DATA_ERROR_CODE(nfo));
		if (U_FAILURE(INTL_DATA_ERROR_CODE(nfo))) {
			/* set up error in case error handler is interested */
			intl_errors_set(INTL_DATA_ERROR_P(nfo), INTL_DATA_ERROR_CODE(nfo),
					"Failed to clone NumberFormatter object", 0);
			crex_throw_exception(NULL, "Failed to clone NumberFormatter object", 0);
		}
	} else {
		crex_throw_exception(NULL, "Cannot clone unconstructed NumberFormatter", 0);
	}
	return new_obj;
}
/* }}} */

/*
 * 'NumberFormatter' class registration structures & functions
 */

/* {{{ formatter_register_class
 * Initialize 'NumberFormatter' class
 */
void formatter_register_class( void )
{
	/* Create and register 'NumberFormatter' class. */
	NumberFormatter_ce_ptr = register_class_NumberFormatter();
	NumberFormatter_ce_ptr->create_object = NumberFormatter_object_create;
	NumberFormatter_ce_ptr->default_object_handlers = &NumberFormatter_handlers;

	memcpy(&NumberFormatter_handlers, &std_object_handlers,
		sizeof(NumberFormatter_handlers));
	NumberFormatter_handlers.offset = XtOffsetOf(NumberFormatter_object, zo);
	NumberFormatter_handlers.clone_obj = NumberFormatter_object_clone;
	NumberFormatter_handlers.free_obj = NumberFormatter_object_free;
}
/* }}} */
