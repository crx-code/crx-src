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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unicode/brkiter.h>
#include "codepointiterator_internal.h"

#include "breakiterator_iterators.h"

extern "C" {
#include "../crx_intl.h"
#define USE_BREAKITERATOR_POINTER 1
#include "breakiterator_class.h"
#include "../locale/locale.h"
#include <crex_exceptions.h>
#include <crex_interfaces.h>
}

using CRX::CodePointBreakIterator;
using icu::BreakIterator;
using icu::Locale;

U_CFUNC CRX_METHOD(IntlBreakIterator, __main)
{
	crex_throw_exception( NULL,
		"An object of this type cannot be created with the new operator",
		0 );
}

static void _breakiter_factory(const char *func_name,
							   BreakIterator *(*func)(const Locale&, UErrorCode&),
							   INTERNAL_FUNCTION_PARAMETERS)
{
	BreakIterator	*biter;
	const char		*locale_str = NULL;
	size_t				dummy;
	char			*msg;
	UErrorCode		status = UErrorCode();
	intl_error_reset(NULL);

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|s!",
			&locale_str, &dummy) == FAILURE) {
		RETURN_THROWS();
	}

	if (locale_str == NULL) {
		locale_str = intl_locale_get_default();
	}

	biter = func(Locale::createFromName(locale_str), status);
	intl_error_set_code(NULL, status);
	if (U_FAILURE(status)) {
		spprintf(&msg, 0, "%s: error creating BreakIterator",
				func_name);
		intl_error_set_custom_msg(NULL, msg, 1);
		efree(msg);
		RETURN_NULL();
	}

	breakiterator_object_create(return_value, biter, 1);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, createWordInstance)
{
	_breakiter_factory("breakiter_create_word_instance",
			&BreakIterator::createWordInstance,
			INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, createLineInstance)
{
	_breakiter_factory("breakiter_create_line_instance",
			&BreakIterator::createLineInstance,
			INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, createCharacterInstance)
{
	_breakiter_factory("breakiter_create_character_instance",
			&BreakIterator::createCharacterInstance,
			INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, createSentenceInstance)
{
	_breakiter_factory("breakiter_create_sentence_instance",
			&BreakIterator::createSentenceInstance,
			INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, createTitleInstance)
{
	_breakiter_factory("breakiter_create_title_instance",
			&BreakIterator::createTitleInstance,
			INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, createCodePointInstance)
{
	intl_error_reset(NULL);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	CodePointBreakIterator *cpbi = new CodePointBreakIterator();
	breakiterator_object_create(return_value, cpbi, 1);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, getText)
{
	BREAKITER_METHOD_INIT_VARS;
	object = CREX_THIS;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	BREAKITER_METHOD_FETCH_OBJECT;

	if (C_ISUNDEF(bio->text)) {
		RETURN_NULL();
	} else {
		ZVAL_COPY(return_value, &bio->text);
	}
}

U_CFUNC CRX_METHOD(IntlBreakIterator, setText)
{
	UText	*ut = NULL;
	crex_string	*text;
	BREAKITER_METHOD_INIT_VARS;
	object = CREX_THIS;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S", &text) == FAILURE) {
		RETURN_THROWS();
	}

	BREAKITER_METHOD_FETCH_OBJECT;

	ut = utext_openUTF8(ut, ZSTR_VAL(text), ZSTR_LEN(text), BREAKITER_ERROR_CODE_P(bio));
	INTL_METHOD_CHECK_STATUS(bio, "breakiter_set_text: error opening UText");

	bio->biter->setText(ut, BREAKITER_ERROR_CODE(bio));
	utext_close(ut); /* ICU shallow clones the UText */
	INTL_METHOD_CHECK_STATUS(bio, "breakiter_set_text: error calling "
		"BreakIterator::setText()");

	/* When ICU clones the UText, it does not copy the buffer, so we have to
	 * keep the string buffer around by holding a reference to its zval. This
	 * also allows a faste implementation of getText() */
	zval_ptr_dtor(&bio->text);
	ZVAL_STR_COPY(&bio->text, text);

	RETURN_TRUE;
}

static void _breakiter_no_args_ret_int32(
		int32_t (BreakIterator::*func)(),
		INTERNAL_FUNCTION_PARAMETERS)
{
	BREAKITER_METHOD_INIT_VARS;
	object = CREX_THIS;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	BREAKITER_METHOD_FETCH_OBJECT;

	int32_t res = (bio->biter->*func)();

	RETURN_LONG((crex_long)res);
}

static void _breakiter_int32_ret_int32(
		int32_t (BreakIterator::*func)(int32_t),
		INTERNAL_FUNCTION_PARAMETERS)
{
	crex_long	arg;
	BREAKITER_METHOD_INIT_VARS;
	object = CREX_THIS;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &arg) == FAILURE) {
		RETURN_THROWS();
	}

	BREAKITER_METHOD_FETCH_OBJECT;

	if (UNEXPECTED(arg < INT32_MIN || arg > INT32_MAX)) {
		crex_argument_value_error(1, "must be between %d and %d", INT32_MIN, INT32_MAX);
		RETURN_THROWS();
	}

	int32_t res = (bio->biter->*func)((int32_t)arg);

	RETURN_LONG((crex_long)res);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, first)
{
	_breakiter_no_args_ret_int32(&BreakIterator::first,
			INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, last)
{
	_breakiter_no_args_ret_int32(&BreakIterator::last,
			INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, previous)
{
	_breakiter_no_args_ret_int32(&BreakIterator::previous,
			INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, next)
{
	zval *arg = NULL;

	if (CREX_NUM_ARGS() == 0) {
		goto no_arg_version;
	}
	if (crex_parse_parameters(CREX_NUM_ARGS(), "z!", &arg) == FAILURE) {
		RETURN_THROWS();
	}

	if (arg == NULL) {
		CREX_NUM_ARGS() = 0; /* pretend we don't have any argument */
		no_arg_version:
		_breakiter_no_args_ret_int32(&BreakIterator::next,
				INTERNAL_FUNCTION_PARAM_PASSTHRU);
	} else {
		_breakiter_int32_ret_int32(&BreakIterator::next,
				INTERNAL_FUNCTION_PARAM_PASSTHRU);
	}
}

U_CFUNC CRX_METHOD(IntlBreakIterator, current)
{
	BREAKITER_METHOD_INIT_VARS;
	object = CREX_THIS;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	BREAKITER_METHOD_FETCH_OBJECT;

	int32_t res = bio->biter->current();

	RETURN_LONG((crex_long)res);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, following)
{
	_breakiter_int32_ret_int32(
			&BreakIterator::following,
			INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, preceding)
{
	_breakiter_int32_ret_int32(
			&BreakIterator::preceding,
			INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, isBoundary)
{
	crex_long offset;
	BREAKITER_METHOD_INIT_VARS;
	object = CREX_THIS;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l",
			&offset) == FAILURE) {
		RETURN_THROWS();
	}

	if (UNEXPECTED(offset < INT32_MIN || offset > INT32_MAX)) {
		crex_argument_value_error(1, "must be between %d and %d", INT32_MIN, INT32_MAX);
		RETURN_THROWS();
	}

	BREAKITER_METHOD_FETCH_OBJECT;

	UBool res = bio->biter->isBoundary((int32_t)offset);

	RETURN_BOOL((crex_long)res);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, getLocale)
{
	crex_long	locale_type;
	BREAKITER_METHOD_INIT_VARS;
	object = CREX_THIS;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &locale_type) == FAILURE) {
		RETURN_THROWS();
	}

	/* Change to ValueError? */
	if (locale_type != ULOC_ACTUAL_LOCALE && locale_type != ULOC_VALID_LOCALE) {
		intl_error_set(NULL, U_ILLEGAL_ARGUMENT_ERROR,
			"breakiter_get_locale: invalid locale type", 0);
		RETURN_FALSE;
	}

	BREAKITER_METHOD_FETCH_OBJECT;

	Locale locale = bio->biter->getLocale((ULocDataLocaleType)locale_type,
		BREAKITER_ERROR_CODE(bio));
	INTL_METHOD_CHECK_STATUS(bio,
		"breakiter_get_locale: Call to ICU method has failed");

	RETURN_STRING(locale.getName());
}

U_CFUNC CRX_METHOD(IntlBreakIterator, getPartsIterator)
{
	crex_long key_type = 0;
	BREAKITER_METHOD_INIT_VARS;
	object = CREX_THIS;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|l", &key_type) == FAILURE) {
		RETURN_THROWS();
	}

	if (key_type != PARTS_ITERATOR_KEY_SEQUENTIAL
			&& key_type != PARTS_ITERATOR_KEY_LEFT
			&& key_type != PARTS_ITERATOR_KEY_RIGHT) {
		crex_argument_value_error(1, "must be one of IntlPartsIterator::KEY_SEQUENTIAL, "
			"IntlPartsIterator::KEY_LEFT, or IntlPartsIterator::KEY_RIGHT");
		RETURN_THROWS();
	}

	BREAKITER_METHOD_FETCH_OBJECT;

	IntlIterator_from_BreakIterator_parts(
		object, return_value, (parts_iter_key_type)key_type);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, getErrorCode)
{
	BREAKITER_METHOD_INIT_VARS;
	object = CREX_THIS;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	/* Fetch the object (without resetting its last error code ). */
	bio = C_INTL_BREAKITERATOR_P(object);
	RETURN_LONG((crex_long)BREAKITER_ERROR_CODE(bio));
}

U_CFUNC CRX_METHOD(IntlBreakIterator, getErrorMessage)
{
	crex_string* message = NULL;
	BREAKITER_METHOD_INIT_VARS;
	object = CREX_THIS;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}


	/* Fetch the object (without resetting its last error code ). */
	bio = C_INTL_BREAKITERATOR_P(object);

	/* Return last error message. */
	message = intl_error_get_message(BREAKITER_ERROR_P(bio));
	RETURN_STR(message);
}

U_CFUNC CRX_METHOD(IntlBreakIterator, getIterator)
{
	if (crex_parse_parameters_none() == FAILURE) {
		return;
	}

	crex_create_internal_iterator_zval(return_value, CREX_THIS);
}
