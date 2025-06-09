/*
   +----------------------------------------------------------------------+
   | Copyright (c) The CRX Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the CRX license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.crx.net/license/3_01.txt                                 |
   | If you did not receive a copy of the CRX license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crx.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Thies C. Arntzen <thies@thieso.net>                          |
   +----------------------------------------------------------------------+
*/

/* {{{ includes */
#include "crx.h"
#include "crx_assert.h"
#include "crx_ini.h"
#include "crex_exceptions.h"
/* }}} */

CREX_BEGIN_MODULE_GLOBALS(assert)
	zval callback;
	char *cb;
	bool active;
	bool bail;
	bool warning;
	bool exception;
CREX_END_MODULE_GLOBALS(assert)

CREX_DECLARE_MODULE_GLOBALS(assert)

#define ASSERTG(v) CREX_MODULE_GLOBALS_ACCESSOR(assert, v)

CRXAPI crex_class_entry *assertion_error_ce;

/* Hack to pass a custom stage for the our OnModify handler so that a deprecation warning does not get emitted
 * when an option is modified via assert_option() function */
#define CREX_INI_STAGE_ASSERT_OPTIONS (1<<6)

static inline bool crx_must_emit_ini_deprecation(int stage)
{
	return stage != CREX_INI_STAGE_DEACTIVATE && stage != CREX_INI_STAGE_SHUTDOWN && stage != CREX_INI_STAGE_ASSERT_OPTIONS;
}

static CRX_INI_MH(OnChangeCallback) /* {{{ */
{
	if (EG(current_execute_data)) {
		if (C_TYPE(ASSERTG(callback)) != IS_UNDEF) {
			zval_ptr_dtor(&ASSERTG(callback));
			ZVAL_UNDEF(&ASSERTG(callback));
		}
		if (new_value && (C_TYPE(ASSERTG(callback)) != IS_UNDEF || ZSTR_LEN(new_value))) {
			if (crx_must_emit_ini_deprecation(stage)) {
				crx_error_docref(NULL, E_DEPRECATED, "assert.callback INI setting is deprecated");
			}
			ZVAL_STR_COPY(&ASSERTG(callback), new_value);
		}
	} else {
		if (ASSERTG(cb)) {
			pefree(ASSERTG(cb), 1);
		}
		if (new_value && ZSTR_LEN(new_value)) {
			if (crx_must_emit_ini_deprecation(stage)) {
				crx_error_docref(NULL, E_DEPRECATED, "assert.callback INI setting is deprecated");
			}
			ASSERTG(cb) = pemalloc(ZSTR_LEN(new_value) + 1, 1);
			memcpy(ASSERTG(cb), ZSTR_VAL(new_value), ZSTR_LEN(new_value));
			ASSERTG(cb)[ZSTR_LEN(new_value)] = '\0';
		} else {
			ASSERTG(cb) = NULL;
		}
	}
	return SUCCESS;
}
/* }}} */

static CRX_INI_MH(OnUpdateActiveBool)
{
	bool *p = (bool *) CREX_INI_GET_ADDR();
	*p = crex_ini_parse_bool(new_value);
	if (crx_must_emit_ini_deprecation(stage) && !*p) {
		crx_error_docref(NULL, E_DEPRECATED, "assert.active INI setting is deprecated");
	}
	return SUCCESS;
}

static CRX_INI_MH(OnUpdateBailBool)
{
	bool *p = (bool *) CREX_INI_GET_ADDR();
	*p = crex_ini_parse_bool(new_value);
	if (crx_must_emit_ini_deprecation(stage) && *p) {
		crx_error_docref(NULL, E_DEPRECATED, "assert.bail INI setting is deprecated");
	}
	return SUCCESS;
}

static CRX_INI_MH(OnUpdateExceptionBool)
{
	bool *p = (bool *) CREX_INI_GET_ADDR();
	*p = crex_ini_parse_bool(new_value);
	if (crx_must_emit_ini_deprecation(stage) && !*p) {
		crx_error_docref(NULL, E_DEPRECATED, "assert.exception INI setting is deprecated");
	}
	return SUCCESS;
}


static CRX_INI_MH(OnUpdateWarningBool)
{
	bool *p = (bool *) CREX_INI_GET_ADDR();
	*p = crex_ini_parse_bool(new_value);
	if (crx_must_emit_ini_deprecation(stage) && !*p) {
		crx_error_docref(NULL, E_DEPRECATED, "assert.warning INI setting is deprecated");
	}
	return SUCCESS;
}


CRX_INI_BEGIN()
	 STD_CRX_INI_BOOLEAN("assert.active",    "1",  CRX_INI_ALL,	OnUpdateActiveBool,		active,	 			crex_assert_globals,		assert_globals)
	 STD_CRX_INI_BOOLEAN("assert.bail",      "0",  CRX_INI_ALL,	OnUpdateBailBool,		bail,	 			crex_assert_globals,		assert_globals)
	 STD_CRX_INI_BOOLEAN("assert.warning",   "1",  CRX_INI_ALL,	OnUpdateWarningBool,		warning, 			crex_assert_globals,		assert_globals)
	 CRX_INI_ENTRY("assert.callback",        NULL, CRX_INI_ALL,	OnChangeCallback)
	 STD_CRX_INI_BOOLEAN("assert.exception", "1",  CRX_INI_ALL,	OnUpdateExceptionBool,		exception, 			crex_assert_globals,		assert_globals)
CRX_INI_END()

static void crx_assert_init_globals(crex_assert_globals *assert_globals_p) /* {{{ */
{
	ZVAL_UNDEF(&assert_globals_p->callback);
	assert_globals_p->cb = NULL;
}
/* }}} */

CRX_MINIT_FUNCTION(assert) /* {{{ */
{
	CREX_INIT_MODULE_GLOBALS(assert, crx_assert_init_globals, NULL);

	REGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

CRX_MSHUTDOWN_FUNCTION(assert) /* {{{ */
{
	if (ASSERTG(cb)) {
		pefree(ASSERTG(cb), 1);
		ASSERTG(cb) = NULL;
	}
	return SUCCESS;
}
/* }}} */

CRX_RSHUTDOWN_FUNCTION(assert) /* {{{ */
{
	if (C_TYPE(ASSERTG(callback)) != IS_UNDEF) {
		zval_ptr_dtor(&ASSERTG(callback));
		ZVAL_UNDEF(&ASSERTG(callback));
	}

	return SUCCESS;
}
/* }}} */

CRX_MINFO_FUNCTION(assert) /* {{{ */
{
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ Checks if assertion is false */
CRX_FUNCTION(assert)
{
	zval *assertion;
	crex_string *description_str = NULL;
	crex_object *description_obj = NULL;

	if (!ASSERTG(active)) {
		RETURN_TRUE;
	}

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_ZVAL(assertion)
		C_PARAM_OPTIONAL
		C_PARAM_OBJ_OF_CLASS_OR_STR_OR_NULL(description_obj, crex_ce_throwable, description_str)
	CREX_PARSE_PARAMETERS_END();

	if (crex_is_true(assertion)) {
		RETURN_TRUE;
	}

	if (description_obj) {
		GC_ADDREF(description_obj);
		crex_throw_exception_internal(description_obj);
		RETURN_THROWS();
	}

	if (C_TYPE(ASSERTG(callback)) == IS_UNDEF && ASSERTG(cb)) {
		ZVAL_STRING(&ASSERTG(callback), ASSERTG(cb));
	}

	if (C_TYPE(ASSERTG(callback)) != IS_UNDEF) {
		zval args[4];
		zval retval;
		uint32_t lineno = crex_get_executed_lineno();
		crex_string *filename = crex_get_executed_filename_ex();
		if (UNEXPECTED(!filename)) {
			filename = ZSTR_KNOWN(CREX_STR_UNKNOWN_CAPITALIZED);
		}

		ZVAL_STR(&args[0], filename);
		ZVAL_LONG(&args[1], lineno);
		ZVAL_NULL(&args[2]);

		ZVAL_FALSE(&retval);

		if (description_str) {
			ZVAL_STR(&args[3], description_str);
			call_user_function(NULL, NULL, &ASSERTG(callback), &retval, 4, args);
		} else {
			call_user_function(NULL, NULL, &ASSERTG(callback), &retval, 3, args);
		}

		zval_ptr_dtor(&retval);
	}

	if (ASSERTG(exception)) {
		crex_throw_exception(assertion_error_ce, description_str ? ZSTR_VAL(description_str) : NULL, E_ERROR);
		if (ASSERTG(bail)) {
			/* When bail is turned on, the exception will not be caught. */
			crex_exception_error(EG(exception), E_ERROR);
		}
	} else if (ASSERTG(warning)) {
		crx_error_docref(NULL, E_WARNING, "%s failed", description_str ? ZSTR_VAL(description_str) : "Assertion");
	}

	if (ASSERTG(bail)) {
		crex_throw_unwind_exit();
		RETURN_THROWS();
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Set/get the various assert flags */
CRX_FUNCTION(assert_options)
{
	zval *value = NULL;
	crex_long what;
	bool oldint;
	uint32_t ac = CREX_NUM_ARGS();
	crex_string *key;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_LONG(what)
		C_PARAM_OPTIONAL
		C_PARAM_ZVAL(value)
	CREX_PARSE_PARAMETERS_END();

	switch (what) {
	case CRX_ASSERT_ACTIVE:
		oldint = ASSERTG(active);
		if (ac == 2) {
			crex_string *value_str = zval_try_get_string(value);
			if (UNEXPECTED(!value_str)) {
				RETURN_THROWS();
			}

			key = ZSTR_INIT_LITERAL("assert.active", 0);
			crex_alter_ini_entry_ex(key, value_str, CRX_INI_USER, CREX_INI_STAGE_ASSERT_OPTIONS, 0);
			crex_string_release_ex(key, 0);
			crex_string_release_ex(value_str, 0);
		}
		RETURN_LONG(oldint);
		break;

	case CRX_ASSERT_BAIL:
		oldint = ASSERTG(bail);
		if (ac == 2) {
			crex_string *value_str = zval_try_get_string(value);
			if (UNEXPECTED(!value_str)) {
				RETURN_THROWS();
			}

			key = ZSTR_INIT_LITERAL("assert.bail", 0);
			crex_alter_ini_entry_ex(key, value_str, CRX_INI_USER, CREX_INI_STAGE_ASSERT_OPTIONS, 0);
			crex_string_release_ex(key, 0);
			crex_string_release_ex(value_str, 0);
		}
		RETURN_LONG(oldint);
		break;

	case CRX_ASSERT_WARNING:
		oldint = ASSERTG(warning);
		if (ac == 2) {
			crex_string *value_str = zval_try_get_string(value);
			if (UNEXPECTED(!value_str)) {
				RETURN_THROWS();
			}

			key = ZSTR_INIT_LITERAL("assert.warning", 0);
			crex_alter_ini_entry_ex(key, value_str, CRX_INI_USER, CREX_INI_STAGE_ASSERT_OPTIONS, 0);
			crex_string_release_ex(key, 0);
			crex_string_release_ex(value_str, 0);
		}
		RETURN_LONG(oldint);
		break;

	case CRX_ASSERT_CALLBACK:
		if (C_TYPE(ASSERTG(callback)) != IS_UNDEF) {
			ZVAL_COPY(return_value, &ASSERTG(callback));
		} else if (ASSERTG(cb)) {
			RETVAL_STRING(ASSERTG(cb));
		} else {
			RETVAL_NULL();
		}

		if (ac == 2) {
			zval_ptr_dtor(&ASSERTG(callback));
			if (C_TYPE_P(value) == IS_NULL) {
				ZVAL_UNDEF(&ASSERTG(callback));
			} else {
				ZVAL_COPY(&ASSERTG(callback), value);
			}
		}
		return;

	case CRX_ASSERT_EXCEPTION:
		oldint = ASSERTG(exception);
		if (ac == 2) {
			crex_string *val = zval_try_get_string(value);
			if (UNEXPECTED(!val)) {
				RETURN_THROWS();
			}

			key = ZSTR_INIT_LITERAL("assert.exception", 0);
			crex_alter_ini_entry_ex(key, val, CRX_INI_USER, CREX_INI_STAGE_ASSERT_OPTIONS, 0);
			crex_string_release_ex(val, 0);
			crex_string_release_ex(key, 0);
		}
		RETURN_LONG(oldint);
		break;

	default:
		crex_argument_value_error(1, "must be an ASSERT_* constant");
		RETURN_THROWS();
	}
}
/* }}} */
