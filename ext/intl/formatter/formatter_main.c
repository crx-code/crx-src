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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unicode/ustring.h>

#include "crx_intl.h"
#include "formatter_class.h"
#include "intl_convert.h"

/* {{{ */
static int numfmt_ctor(INTERNAL_FUNCTION_PARAMETERS, crex_error_handling *error_handling, bool *error_handling_replaced)
{
	const char* locale;
	char*       pattern = NULL;
	size_t      locale_len = 0, pattern_len = 0;
	crex_long   style;
	UChar*      spattern     = NULL;
	int32_t     spattern_len = 0;
	FORMATTER_METHOD_INIT_VARS;

	/* Parse parameters. */
	if( crex_parse_parameters( CREX_NUM_ARGS(), "sl|s!",
		&locale, &locale_len, &style, &pattern, &pattern_len ) == FAILURE )
	{
		return FAILURE;
	}

	if (error_handling != NULL) {
		crex_replace_error_handling(EH_THROW, IntlException_ce_ptr, error_handling);
		*error_handling_replaced = 1;
	}

	INTL_CHECK_LOCALE_LEN_OR_FAILURE(locale_len);
	object = return_value;
	FORMATTER_METHOD_FETCH_OBJECT_NO_CHECK;
	if (FORMATTER_OBJECT(nfo)) {
		crex_throw_error(NULL, "NumberFormatter object is already constructed");
		return FAILURE;
	}

	/* Convert pattern (if specified) to UTF-16. */
	if(pattern && pattern_len) {
		intl_convert_utf8_to_utf16(&spattern, &spattern_len, pattern, pattern_len, &INTL_DATA_ERROR_CODE(nfo));
		INTL_CTOR_CHECK_STATUS(nfo, "numfmt_create: error converting pattern to UTF-16");
	}

	if(locale_len == 0) {
		locale = intl_locale_get_default();
	}

	/* Create an ICU number formatter. */
	FORMATTER_OBJECT(nfo) = unum_open(style, spattern, spattern_len, locale, NULL, &INTL_DATA_ERROR_CODE(nfo));

	if(spattern) {
		efree(spattern);
	}

	INTL_CTOR_CHECK_STATUS(nfo, "numfmt_create: number formatter creation failed");
	return SUCCESS;
}
/* }}} */

/* {{{ Create number formatter. */
CRX_FUNCTION( numfmt_create )
{
	object_init_ex( return_value, NumberFormatter_ce_ptr );
	if (numfmt_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, NULL, NULL) == FAILURE) {
		zval_ptr_dtor(return_value);
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ NumberFormatter object constructor. */
CRX_METHOD( NumberFormatter, __main )
{
	crex_error_handling error_handling;
	bool error_handling_replaced = 0;

	return_value = CREX_THIS;
	if (numfmt_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, &error_handling, &error_handling_replaced) == FAILURE) {
		if (!EG(exception)) {
			crex_throw_exception(IntlException_ce_ptr, "Constructor failed", 0);
		}
	}
	if (error_handling_replaced) {
		crex_restore_error_handling(&error_handling);
	}
}
/* }}} */

/* {{{ Get formatter's last error code. */
CRX_FUNCTION( numfmt_get_error_code )
{
	FORMATTER_METHOD_INIT_VARS

	/* Parse parameters. */
	if( crex_parse_method_parameters( CREX_NUM_ARGS(), getThis(), "O",
		&object, NumberFormatter_ce_ptr ) == FAILURE )
	{
		RETURN_THROWS();
	}

	nfo = C_INTL_NUMBERFORMATTER_P(object);

	/* Return formatter's last error code. */
	RETURN_LONG( INTL_DATA_ERROR_CODE(nfo) );
}
/* }}} */

/* {{{ Get text description for formatter's last error code. */
CRX_FUNCTION( numfmt_get_error_message )
{
	crex_string *message = NULL;
	FORMATTER_METHOD_INIT_VARS

	/* Parse parameters. */
	if( crex_parse_method_parameters( CREX_NUM_ARGS(), getThis(), "O",
		&object, NumberFormatter_ce_ptr ) == FAILURE )
	{
		RETURN_THROWS();
	}

	nfo = C_INTL_NUMBERFORMATTER_P(object);

	/* Return last error message. */
	message = intl_error_get_message( INTL_DATA_ERROR_P(nfo) );
	RETURN_STR(message);
}
/* }}} */
