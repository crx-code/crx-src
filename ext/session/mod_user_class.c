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
   | Author: Arpad Ray <arpad@crx.net>                                    |
   +----------------------------------------------------------------------+
 */

#include "crx.h"
#include "crx_session.h"

#define PS_SANITY_CHECK						\
	if (PS(session_status) != crx_session_active) { \
		crex_throw_error(NULL, "Session is not active"); \
		RETURN_THROWS(); \
	} \
	if (PS(default_mod) == NULL) { \
		crex_throw_error(NULL, "Cannot call default session handler"); \
		RETURN_THROWS(); \
	}

#define PS_SANITY_CHECK_IS_OPEN				\
	PS_SANITY_CHECK; \
	if (!PS(mod_user_is_open)) {			\
		crx_error_docref(NULL, E_WARNING, "Parent session handler is not open");	\
		RETURN_FALSE;						\
	}

/* {{{ Wraps the old open handler */
CRX_METHOD(SessionHandler, open)
{
	char *save_path = NULL, *session_name = NULL;
	size_t save_path_len, session_name_len;
	crex_result ret;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss", &save_path, &save_path_len, &session_name, &session_name_len) == FAILURE) {
		RETURN_THROWS();
	}

	PS_SANITY_CHECK;

	PS(mod_user_is_open) = 1;

	crex_try {
		ret = PS(default_mod)->s_open(&PS(mod_data), save_path, session_name);
	} crex_catch {
		PS(session_status) = crx_session_none;
		crex_bailout();
	} crex_end_try();

	RETURN_BOOL(SUCCESS == ret);
}
/* }}} */

/* {{{ Wraps the old close handler */
CRX_METHOD(SessionHandler, close)
{
	crex_result ret;

	// don't return on failure, since not closing the default handler
	// could result in memory leaks or other nasties
	crex_parse_parameters_none();

	PS_SANITY_CHECK_IS_OPEN;

	PS(mod_user_is_open) = 0;

	crex_try {
		ret = PS(default_mod)->s_close(&PS(mod_data));
	} crex_catch {
		PS(session_status) = crx_session_none;
		crex_bailout();
	} crex_end_try();

	RETURN_BOOL(SUCCESS == ret);
}
/* }}} */

/* {{{ Wraps the old read handler */
CRX_METHOD(SessionHandler, read)
{
	crex_string *val;
	crex_string *key;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S", &key) == FAILURE) {
		RETURN_THROWS();
	}

	PS_SANITY_CHECK_IS_OPEN;

	if (PS(default_mod)->s_read(&PS(mod_data), key, &val, PS(gc_maxlifetime)) == FAILURE) {
		RETURN_FALSE;
	}

	RETURN_STR(val);
}
/* }}} */

/* {{{ Wraps the old write handler */
CRX_METHOD(SessionHandler, write)
{
	crex_string *key, *val;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "SS", &key, &val) == FAILURE) {
		RETURN_THROWS();
	}

	PS_SANITY_CHECK_IS_OPEN;

	RETURN_BOOL(SUCCESS == PS(default_mod)->s_write(&PS(mod_data), key, val, PS(gc_maxlifetime)));
}
/* }}} */

/* {{{ Wraps the old destroy handler */
CRX_METHOD(SessionHandler, destroy)
{
	crex_string *key;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S", &key) == FAILURE) {
		RETURN_THROWS();
	}

	PS_SANITY_CHECK_IS_OPEN;

	RETURN_BOOL(SUCCESS == PS(default_mod)->s_destroy(&PS(mod_data), key));
}
/* }}} */

/* {{{ Wraps the old gc handler */
CRX_METHOD(SessionHandler, gc)
{
	crex_long maxlifetime;
	crex_long nrdels = -1;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &maxlifetime) == FAILURE) {
		RETURN_THROWS();
	}

	PS_SANITY_CHECK_IS_OPEN;

	if (PS(default_mod)->s_gc(&PS(mod_data), maxlifetime, &nrdels) == FAILURE) {
		RETURN_FALSE;
	}
	RETURN_LONG(nrdels);
}
/* }}} */

/* {{{ Wraps the old create_sid handler */
CRX_METHOD(SessionHandler, create_sid)
{
	crex_string *id;

	if (crex_parse_parameters_none() == FAILURE) {
	    RETURN_THROWS();
	}

	PS_SANITY_CHECK;

	id = PS(default_mod)->s_create_sid(&PS(mod_data));

	RETURN_STR(id);
}
/* }}} */
