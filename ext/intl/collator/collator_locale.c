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
   | Authors: Vadim Savchuk <vsavchuk@productengine.com>                  |
   |          Dmitry Lakhtyuk <dlakhtyuk@productengine.com>               |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx_intl.h"
#include "collator_class.h"
#include "intl_convert.h"

#include <crex_API.h>

/* {{{ Gets the locale name of the collator. */
CRX_FUNCTION( collator_get_locale )
{
	crex_long   type        = 0;
	char*  locale_name = NULL;

	COLLATOR_METHOD_INIT_VARS

	/* Parse parameters. */
	if( crex_parse_method_parameters( CREX_NUM_ARGS(), getThis(), "Ol",
		&object, Collator_ce_ptr, &type ) == FAILURE )
	{
		RETURN_THROWS();
	}

	/* Fetch the object. */
	COLLATOR_METHOD_FETCH_OBJECT;

	if (!co || !co->ucoll) {
		intl_error_set_code( NULL, COLLATOR_ERROR_CODE( co ) );
		intl_errors_set_custom_msg( COLLATOR_ERROR_P( co ),
			"Object not initialized", 0 );
		crex_throw_error(NULL, "Object not initialized");

		RETURN_THROWS();
	}

	/* Get locale by specified type. */
	locale_name = (char*) ucol_getLocaleByType(
		co->ucoll, type, COLLATOR_ERROR_CODE_P( co ) );
	COLLATOR_CHECK_STATUS( co, "Error getting locale by type" );

	/* Return it. */
	RETVAL_STRINGL( locale_name, strlen(locale_name) );
}
/* }}} */
