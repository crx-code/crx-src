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
   | Author: Sascha Schumann <sascha@schumann.cx>                         |
   +----------------------------------------------------------------------+
 */

#include "crx.h"
#include "crx_session.h"
#include "mod_user.h"

const ps_module ps_mod_user = {
	PS_MOD_UPDATE_TIMESTAMP(user)
};


static void ps_call_handler(zval *func, int argc, zval *argv, zval *retval)
{
	int i;
	if (PS(in_save_handler)) {
		PS(in_save_handler) = 0;
		ZVAL_UNDEF(retval);
		crx_error_docref(NULL, E_WARNING, "Cannot call session save handler in a recursive manner");
		return;
	}
	PS(in_save_handler) = 1;
	if (call_user_function(NULL, NULL, func, retval, argc, argv) == FAILURE) {
		zval_ptr_dtor(retval);
		ZVAL_UNDEF(retval);
	} else if (C_ISUNDEF_P(retval)) {
		ZVAL_NULL(retval);
	}
	PS(in_save_handler) = 0;
	for (i = 0; i < argc; i++) {
		zval_ptr_dtor(&argv[i]);
	}
}

#define PSF(a) PS(mod_user_names).ps_##a

static crex_result verify_bool_return_type_userland_calls(const zval* value)
{
	/* Exit or exception in userland call */
	if (C_TYPE_P(value) == IS_UNDEF) {
		return FAILURE;
	}
	if (C_TYPE_P(value) == IS_TRUE) {
		return SUCCESS;
	}
	if (C_TYPE_P(value) == IS_FALSE) {
		return FAILURE;
	}
	if ((C_TYPE_P(value) == IS_LONG) && (C_LVAL_P(value) == -1)) {
		/* TODO Why are exception cheked? */
		if (!EG(exception)) {
			crx_error_docref(NULL, E_DEPRECATED, "Session callback must have a return value of type bool, %s returned", crex_zval_value_name(value));
		}
		return FAILURE;
	}
	if ((C_TYPE_P(value) == IS_LONG) && (C_LVAL_P(value) == 0)) {
		/* TODO Why are exception cheked? */
		if (!EG(exception)) {
			crx_error_docref(NULL, E_DEPRECATED, "Session callback must have a return value of type bool, %s returned", crex_zval_value_name(value));
		}
		return SUCCESS;
	}
	if (!EG(exception)) {
		crex_type_error("Session callback must have a return value of type bool, %s returned", crex_zval_value_name(value)); \
    }
    return FAILURE;
}

PS_OPEN_FUNC(user)
{
	zval args[2];
	zval retval;
	crex_result ret = FAILURE;

	CREX_ASSERT(!C_ISUNDEF(PSF(open)));

	ZVAL_STRING(&args[0], (char*)save_path);
	ZVAL_STRING(&args[1], (char*)session_name);

	crex_try {
		ps_call_handler(&PSF(open), 2, args, &retval);
	} crex_catch {
		PS(session_status) = crx_session_none;
		if (!C_ISUNDEF(retval)) {
			zval_ptr_dtor(&retval);
		}
		crex_bailout();
	} crex_end_try();

	PS(mod_user_implemented) = 1;

	ret = verify_bool_return_type_userland_calls(&retval);
	zval_ptr_dtor(&retval);
	return ret;
}

PS_CLOSE_FUNC(user)
{
	bool bailout = 0;
	zval retval;
	crex_result ret = FAILURE;

	CREX_ASSERT(!C_ISUNDEF(PSF(close)));

	if (!PS(mod_user_implemented)) {
		/* already closed */
		return SUCCESS;
	}

	crex_try {
		ps_call_handler(&PSF(close), 0, NULL, &retval);
	} crex_catch {
		bailout = 1;
	} crex_end_try();

	PS(mod_user_implemented) = 0;

	if (bailout) {
		if (!C_ISUNDEF(retval)) {
			zval_ptr_dtor(&retval);
		}
		crex_bailout();
	}

	ret = verify_bool_return_type_userland_calls(&retval);
	zval_ptr_dtor(&retval);
	return ret;
}

PS_READ_FUNC(user)
{
	zval args[1];
	zval retval;
	crex_result ret = FAILURE;

	CREX_ASSERT(!C_ISUNDEF(PSF(read)));

	ZVAL_STR_COPY(&args[0], key);

	ps_call_handler(&PSF(read), 1, args, &retval);

	if (!C_ISUNDEF(retval)) {
		if (C_TYPE(retval) == IS_STRING) {
			*val = crex_string_copy(C_STR(retval));
			ret = SUCCESS;
		}
		zval_ptr_dtor(&retval);
	}

	return ret;
}

PS_WRITE_FUNC(user)
{
	zval args[2];
	zval retval;
	crex_result ret = FAILURE;

	CREX_ASSERT(!C_ISUNDEF(PSF(write)));

	ZVAL_STR_COPY(&args[0], key);
	ZVAL_STR_COPY(&args[1], val);

	ps_call_handler(&PSF(write), 2, args, &retval);

	ret = verify_bool_return_type_userland_calls(&retval);
	zval_ptr_dtor(&retval);
	return ret;
}

PS_DESTROY_FUNC(user)
{
	zval args[1];
	zval retval;
	crex_result ret = FAILURE;

	CREX_ASSERT(!C_ISUNDEF(PSF(destroy)));

	ZVAL_STR_COPY(&args[0], key);

	ps_call_handler(&PSF(destroy), 1, args, &retval);

	ret = verify_bool_return_type_userland_calls(&retval);
	zval_ptr_dtor(&retval);
	return ret;
}

PS_GC_FUNC(user)
{
	zval args[1];
	zval retval;

	CREX_ASSERT(!C_ISUNDEF(PSF(gc)));

	ZVAL_LONG(&args[0], maxlifetime);

	ps_call_handler(&PSF(gc), 1, args, &retval);

	if (C_TYPE(retval) == IS_LONG) {
		*nrdels = C_LVAL(retval);
	} else if (C_TYPE(retval) == IS_TRUE) {
		/* This is for older API compatibility */
		*nrdels = 1;
	} else {
		/* Anything else is some kind of error */
		*nrdels = -1; // Error
	}
	return *nrdels;
}

PS_CREATE_SID_FUNC(user)
{
	/* maintain backwards compatibility */
	if (!C_ISUNDEF(PSF(create_sid))) {
		crex_string *id = NULL;
		zval retval;

		ps_call_handler(&PSF(create_sid), 0, NULL, &retval);

		if (!C_ISUNDEF(retval)) {
			if (C_TYPE(retval) == IS_STRING) {
				id = crex_string_copy(C_STR(retval));
			}
			zval_ptr_dtor(&retval);
		} else {
			crex_throw_error(NULL, "No session id returned by function");
			return NULL;
		}

		if (!id) {
			crex_throw_error(NULL, "Session id must be a string");
			return NULL;
		}

		return id;
	}

	/* function as defined by PS_MOD */
	return crx_session_create_id(mod_data);
}

PS_VALIDATE_SID_FUNC(user)
{
	/* maintain backwards compatibility */
	if (!C_ISUNDEF(PSF(validate_sid))) {
		zval args[1];
		zval retval;
		crex_result ret = FAILURE;

		ZVAL_STR_COPY(&args[0], key);

		ps_call_handler(&PSF(validate_sid), 1, args, &retval);

		ret = verify_bool_return_type_userland_calls(&retval);
		zval_ptr_dtor(&retval);
		return ret;
	}

	/* dummy function defined by PS_MOD */
	return crx_session_validate_sid(mod_data, key);
}

PS_UPDATE_TIMESTAMP_FUNC(user)
{
	zval args[2];
	zval retval;
	crex_result ret = FAILURE;

	ZVAL_STR_COPY(&args[0], key);
	ZVAL_STR_COPY(&args[1], val);

	/* maintain backwards compatibility */
	if (!C_ISUNDEF(PSF(update_timestamp))) {
		ps_call_handler(&PSF(update_timestamp), 2, args, &retval);
	} else {
		ps_call_handler(&PSF(write), 2, args, &retval);
	}

	ret = verify_bool_return_type_userland_calls(&retval);
	zval_ptr_dtor(&retval);
	return ret;
}
