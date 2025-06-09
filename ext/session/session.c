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
   | Authors: Sascha Schumann <sascha@schumann.cx>                        |
   |          Andrei Zmievski <andrei@crx.net>                            |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"

#ifdef CRX_WIN32
# include "win32/winutil.h"
# include "win32/time.h"
#else
# include <sys/time.h>
#endif

#include <sys/stat.h>
#include <fcntl.h>

#include "crx_ini.h"
#include "SAPI.h"
#include "rfc1867.h"
#include "crx_variables.h"
#include "crx_session.h"
#include "session_arginfo.h"
#include "ext/standard/crx_var.h"
#include "ext/date/crx_date.h"
#include "ext/standard/url_scanner_ex.h"
#include "ext/standard/info.h"
#include "crex_smart_str.h"
#include "ext/standard/url.h"
#include "ext/standard/basic_functions.h"
#include "ext/standard/head.h"
#include "ext/random/crx_random.h"

#include "mod_files.h"
#include "mod_user.h"

#ifdef HAVE_LIBMM
#include "mod_mm.h"
#endif

CRXAPI CREX_DECLARE_MODULE_GLOBALS(ps)

static crex_result crx_session_rfc1867_callback(unsigned int event, void *event_data, void **extra);
static crex_result (*crx_session_rfc1867_orig_callback)(unsigned int event, void *event_data, void **extra);
static void crx_session_track_init(void);

/* SessionHandler class */
crex_class_entry *crx_session_class_entry;

/* SessionHandlerInterface */
crex_class_entry *crx_session_iface_entry;

/* SessionIdInterface */
crex_class_entry *crx_session_id_iface_entry;

/* SessionUpdateTimestampInterface */
crex_class_entry *crx_session_update_timestamp_iface_entry;

#define PS_MAX_SID_LENGTH 256

/* ***********
   * Helpers *
   *********** */

#define IF_SESSION_VARS() \
	if (C_ISREF_P(&PS(http_session_vars)) && C_TYPE_P(C_REFVAL(PS(http_session_vars))) == IS_ARRAY)

#define SESSION_CHECK_ACTIVE_STATE	\
	if (PS(session_status) == crx_session_active) {	\
		crx_error_docref(NULL, E_WARNING, "Session ini settings cannot be changed when a session is active");	\
		return FAILURE;	\
	}

#define SESSION_CHECK_OUTPUT_STATE										\
	if (SG(headers_sent) && stage != CREX_INI_STAGE_DEACTIVATE) {												\
		crx_error_docref(NULL, E_WARNING, "Session ini settings cannot be changed after headers have already been sent");	\
		return FAILURE;													\
	}

#define SESSION_FORBIDDEN_CHARS "=,;.[ \t\r\n\013\014"

#define APPLY_TRANS_SID (PS(use_trans_sid) && !PS(use_only_cookies))

static crex_result crx_session_send_cookie(void);
static crex_result crx_session_abort(void);

/* Initialized in MINIT, readonly otherwise. */
static int my_module_number = 0;

/* Dispatched by RINIT and by crx_session_destroy */
static inline void crx_rinit_session_globals(void) /* {{{ */
{
	/* Do NOT init PS(mod_user_names) here! */
	/* TODO: These could be moved to MINIT and removed. These should be initialized by crx_rshutdown_session_globals() always when execution is finished. */
	PS(id) = NULL;
	PS(session_status) = crx_session_none;
	PS(in_save_handler) = 0;
	PS(set_handler) = 0;
	PS(mod_data) = NULL;
	PS(mod_user_is_open) = 0;
	PS(define_sid) = 1;
	PS(session_vars) = NULL;
	PS(module_number) = my_module_number;
	ZVAL_UNDEF(&PS(http_session_vars));
}
/* }}} */

static inline void crx_session_cleanup_filename(void) /* {{{ */
{
	if (PS(session_started_filename)) {
		crex_string_release(PS(session_started_filename));
		PS(session_started_filename) = NULL;
		PS(session_started_lineno) = 0;
	}
}
/* }}} */

/* Dispatched by RSHUTDOWN and by crx_session_destroy */
static inline void crx_rshutdown_session_globals(void) /* {{{ */
{
	/* Do NOT destroy PS(mod_user_names) here! */
	if (!C_ISUNDEF(PS(http_session_vars))) {
		zval_ptr_dtor(&PS(http_session_vars));
		ZVAL_UNDEF(&PS(http_session_vars));
	}
	if (PS(mod_data) || PS(mod_user_implemented)) {
		crex_try {
			PS(mod)->s_close(&PS(mod_data));
		} crex_end_try();
	}
	if (PS(id)) {
		crex_string_release_ex(PS(id), 0);
		PS(id) = NULL;
	}

	if (PS(session_vars)) {
		crex_string_release_ex(PS(session_vars), 0);
		PS(session_vars) = NULL;
	}

	if (PS(mod_user_class_name)) {
		crex_string_release(PS(mod_user_class_name));
		PS(mod_user_class_name) = NULL;
	}

	crx_session_cleanup_filename();

	/* User save handlers may end up directly here by misuse, bugs in user script, etc. */
	/* Set session status to prevent error while restoring save handler INI value. */
	PS(session_status) = crx_session_none;
}
/* }}} */

CRXAPI crex_result crx_session_destroy(void) /* {{{ */
{
	crex_result retval = SUCCESS;

	if (PS(session_status) != crx_session_active) {
		crx_error_docref(NULL, E_WARNING, "Trying to destroy uninitialized session");
		return FAILURE;
	}

	if (PS(id) && PS(mod)->s_destroy(&PS(mod_data), PS(id)) == FAILURE) {
		retval = FAILURE;
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Session object destruction failed");
		}
	}

	crx_rshutdown_session_globals();
	crx_rinit_session_globals();

	return retval;
}
/* }}} */

CRXAPI void crx_add_session_var(crex_string *name) /* {{{ */
{
	IF_SESSION_VARS() {
		zval *sess_var = C_REFVAL(PS(http_session_vars));
		SEPARATE_ARRAY(sess_var);
		if (!crex_hash_exists(C_ARRVAL_P(sess_var), name)) {
			zval empty_var;
			ZVAL_NULL(&empty_var);
			crex_hash_update(C_ARRVAL_P(sess_var), name, &empty_var);
		}
	}
}
/* }}} */

CRXAPI zval* crx_set_session_var(crex_string *name, zval *state_val, crx_unserialize_data_t *var_hash) /* {{{ */
{
	IF_SESSION_VARS() {
		zval *sess_var = C_REFVAL(PS(http_session_vars));
		SEPARATE_ARRAY(sess_var);
		return crex_hash_update(C_ARRVAL_P(sess_var), name, state_val);
	}
	return NULL;
}
/* }}} */

CRXAPI zval* crx_get_session_var(crex_string *name) /* {{{ */
{
	IF_SESSION_VARS() {
		return crex_hash_find(C_ARRVAL_P(C_REFVAL(PS(http_session_vars))), name);
	}
	return NULL;
}
/* }}} */

static void crx_session_track_init(void) /* {{{ */
{
	zval session_vars;
	crex_string *var_name = ZSTR_INIT_LITERAL("_SESSION", 0);
	/* Unconditionally destroy existing array -- possible dirty data */
	crex_delete_global_variable(var_name);

	if (!C_ISUNDEF(PS(http_session_vars))) {
		zval_ptr_dtor(&PS(http_session_vars));
	}

	array_init(&session_vars);
	ZVAL_NEW_REF(&PS(http_session_vars), &session_vars);
	C_ADDREF_P(&PS(http_session_vars));
	crex_hash_update_ind(&EG(symbol_table), var_name, &PS(http_session_vars));
	crex_string_release_ex(var_name, 0);
}
/* }}} */

static crex_string *crx_session_encode(void) /* {{{ */
{
	IF_SESSION_VARS() {
		if (!PS(serializer)) {
			crx_error_docref(NULL, E_WARNING, "Unknown session.serialize_handler. Failed to encode session object");
			return NULL;
		}
		return PS(serializer)->encode();
	} else {
		crx_error_docref(NULL, E_WARNING, "Cannot encode non-existent session");
	}
	return NULL;
}
/* }}} */

static crex_result crx_session_decode(crex_string *data) /* {{{ */
{
	if (!PS(serializer)) {
		crx_error_docref(NULL, E_WARNING, "Unknown session.serialize_handler. Failed to decode session object");
		return FAILURE;
	}
	if (PS(serializer)->decode(ZSTR_VAL(data), ZSTR_LEN(data)) == FAILURE) {
		crx_session_destroy();
		crx_session_track_init();
		crx_error_docref(NULL, E_WARNING, "Failed to decode session object. Session has been destroyed");
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

/*
 * Note that we cannot use the BASE64 alphabet here, because
 * it contains "/" and "+": both are unacceptable for simple inclusion
 * into URLs.
 */

static const char hexconvtab[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,-";

static void bin_to_readable(unsigned char *in, size_t inlen, char *out, size_t outlen, char nbits) /* {{{ */
{
	unsigned char *p, *q;
	unsigned short w;
	int mask;
	int have;

	p = (unsigned char *)in;
	q = (unsigned char *)in + inlen;

	w = 0;
	have = 0;
	mask = (1 << nbits) - 1;

	while (outlen--) {
		if (have < nbits) {
			if (p < q) {
				w |= *p++ << have;
				have += 8;
			} else {
				/* Should never happen. Input must be large enough. */
				CREX_UNREACHABLE();
				break;
			}
		}

		/* consume nbits */
		*out++ = hexconvtab[w & mask];
		w >>= nbits;
		have -= nbits;
	}

	*out = '\0';
}
/* }}} */

CRXAPI crex_string *crx_session_create_id(PS_CREATE_SID_ARGS) /* {{{ */
{
	unsigned char rbuf[PS_MAX_SID_LENGTH];
	crex_string *outid;

	/* It would be enough to read ceil(sid_length * sid_bits_per_character / 8) bytes here.
	 * We read sid_length bytes instead for simplicity. */
	if (crx_random_bytes_throw(rbuf, PS(sid_length)) == FAILURE) {
		return NULL;
	}

	outid = crex_string_alloc(PS(sid_length), 0);
	bin_to_readable(
		rbuf, PS(sid_length),
		ZSTR_VAL(outid), ZSTR_LEN(outid),
		(char)PS(sid_bits_per_character));

	return outid;
}
/* }}} */

/* Default session id char validation function allowed by ps_modules.
 * If you change the logic here, please also update the error message in
 * ps_modules appropriately */
CRXAPI crex_result crx_session_valid_key(const char *key) /* {{{ */
{
	size_t len;
	const char *p;
	char c;
	crex_result ret = SUCCESS;

	for (p = key; (c = *p); p++) {
		/* valid characters are a..z,A..Z,0..9 */
		if (!((c >= 'a' && c <= 'z')
				|| (c >= 'A' && c <= 'Z')
				|| (c >= '0' && c <= '9')
				|| c == ','
				|| c == '-')) {
			ret = FAILURE;
			break;
		}
	}

	len = p - key;

	/* Somewhat arbitrary length limit here, but should be way more than
	   anyone needs and avoids file-level warnings later on if we exceed MAX_PATH */
	if (len == 0 || len > PS_MAX_SID_LENGTH) {
		ret = FAILURE;
	}

	return ret;
}
/* }}} */


static crex_long crx_session_gc(bool immediate) /* {{{ */
{
	int nrand;
	crex_long num = -1;

	/* GC must be done before reading session data. */
	if ((PS(mod_data) || PS(mod_user_implemented))) {
		if (immediate) {
			PS(mod)->s_gc(&PS(mod_data), PS(gc_maxlifetime), &num);
			return num;
		}
		nrand = (crex_long) ((float) PS(gc_divisor) * crx_combined_lcg());
		if (PS(gc_probability) > 0 && nrand < PS(gc_probability)) {
			PS(mod)->s_gc(&PS(mod_data), PS(gc_maxlifetime), &num);
		}
	}
	return num;
} /* }}} */

static crex_result crx_session_initialize(void) /* {{{ */
{
	crex_string *val = NULL;

	PS(session_status) = crx_session_active;

	if (!PS(mod)) {
		PS(session_status) = crx_session_disabled;
		crx_error_docref(NULL, E_WARNING, "No storage module chosen - failed to initialize session");
		return FAILURE;
	}

	/* Open session handler first */
	if (PS(mod)->s_open(&PS(mod_data), PS(save_path), PS(session_name)) == FAILURE
		/* || PS(mod_data) == NULL */ /* FIXME: open must set valid PS(mod_data) with success */
	) {
		crx_session_abort();
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Failed to initialize storage module: %s (path: %s)", PS(mod)->s_name, PS(save_path));
		}
		return FAILURE;
	}

	/* If there is no ID, use session module to create one */
	if (!PS(id) || !ZSTR_VAL(PS(id))[0]) {
		if (PS(id)) {
			crex_string_release_ex(PS(id), 0);
		}
		PS(id) = PS(mod)->s_create_sid(&PS(mod_data));
		if (!PS(id)) {
			crx_session_abort();
			if (!EG(exception)) {
				crex_throw_error(NULL, "Failed to create session ID: %s (path: %s)", PS(mod)->s_name, PS(save_path));
			}
			return FAILURE;
		}
		if (PS(use_cookies)) {
			PS(send_cookie) = 1;
		}
	} else if (PS(use_strict_mode) && PS(mod)->s_validate_sid &&
		PS(mod)->s_validate_sid(&PS(mod_data), PS(id)) == FAILURE
	) {
		if (PS(id)) {
			crex_string_release_ex(PS(id), 0);
		}
		PS(id) = PS(mod)->s_create_sid(&PS(mod_data));
		if (!PS(id)) {
			PS(id) = crx_session_create_id(NULL);
		}
		if (PS(use_cookies)) {
			PS(send_cookie) = 1;
		}
	}

	if (crx_session_reset_id() == FAILURE) {
		crx_session_abort();
		return FAILURE;
	}

	/* Read data */
	crx_session_track_init();
	if (PS(mod)->s_read(&PS(mod_data), PS(id), &val, PS(gc_maxlifetime)) == FAILURE) {
		crx_session_abort();
		/* FYI: Some broken save handlers return FAILURE for non-existent session ID, this is incorrect */
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Failed to read session data: %s (path: %s)", PS(mod)->s_name, PS(save_path));
		}
		return FAILURE;
	}

	/* GC must be done after read */
	crx_session_gc(0);

	if (PS(session_vars)) {
		crex_string_release_ex(PS(session_vars), 0);
		PS(session_vars) = NULL;
	}
	if (val) {
		if (PS(lazy_write)) {
			PS(session_vars) = crex_string_copy(val);
		}
		crx_session_decode(val);
		crex_string_release_ex(val, 0);
	}

	crx_session_cleanup_filename();
	crex_string *session_started_filename = crex_get_executed_filename_ex();
	if (session_started_filename != NULL) {
		PS(session_started_filename) = crex_string_copy(session_started_filename);
		PS(session_started_lineno) = crex_get_executed_lineno();
	}
	return SUCCESS;
}
/* }}} */

static void crx_session_save_current_state(int write) /* {{{ */
{
	crex_result ret = FAILURE;

	if (write) {
		IF_SESSION_VARS() {
			crex_string *handler_class_name = PS(mod_user_class_name);
			const char *handler_function_name;

			if (PS(mod_data) || PS(mod_user_implemented)) {
				crex_string *val;

				val = crx_session_encode();
				if (val) {
					if (PS(lazy_write) && PS(session_vars)
						&& PS(mod)->s_update_timestamp
						&& PS(mod)->s_update_timestamp != crx_session_update_timestamp
						&& crex_string_equals(val, PS(session_vars))
					) {
						ret = PS(mod)->s_update_timestamp(&PS(mod_data), PS(id), val, PS(gc_maxlifetime));
						handler_function_name = handler_class_name != NULL ? "updateTimestamp" : "update_timestamp";
					} else {
						ret = PS(mod)->s_write(&PS(mod_data), PS(id), val, PS(gc_maxlifetime));
						handler_function_name = "write";
					}
					crex_string_release_ex(val, 0);
				} else {
					ret = PS(mod)->s_write(&PS(mod_data), PS(id), ZSTR_EMPTY_ALLOC(), PS(gc_maxlifetime));
					handler_function_name = "write";
				}
			}

			if ((ret == FAILURE) && !EG(exception)) {
				if (!PS(mod_user_implemented)) {
					crx_error_docref(NULL, E_WARNING, "Failed to write session data (%s). Please "
									 "verify that the current setting of session.save_path "
									 "is correct (%s)",
									 PS(mod)->s_name,
									 PS(save_path));
				} else if (handler_class_name != NULL) {
					crx_error_docref(NULL, E_WARNING, "Failed to write session data using user "
									 "defined save handler. (session.save_path: %s, handler: %s::%s)", PS(save_path),
									 ZSTR_VAL(handler_class_name), handler_function_name);
				} else {
					crx_error_docref(NULL, E_WARNING, "Failed to write session data using user "
									 "defined save handler. (session.save_path: %s, handler: %s)", PS(save_path),
									 handler_function_name);
				}
			}
		}
	}

	if (PS(mod_data) || PS(mod_user_implemented)) {
		PS(mod)->s_close(&PS(mod_data));
	}
}
/* }}} */

static void crx_session_normalize_vars(void) /* {{{ */
{
	PS_ENCODE_VARS;

	IF_SESSION_VARS() {
		PS_ENCODE_LOOP(
			if (C_TYPE_P(struc) == IS_PTR) {
				zval *zv = (zval *)C_PTR_P(struc);
				ZVAL_COPY_VALUE(struc, zv);
				ZVAL_UNDEF(zv);
			}
		);
	}
}
/* }}} */

/* *************************
   * INI Settings/Handlers *
   ************************* */

static CRX_INI_MH(OnUpdateSaveHandler) /* {{{ */
{
	const ps_module *tmp;
	int err_type = E_ERROR;

	SESSION_CHECK_ACTIVE_STATE;
	SESSION_CHECK_OUTPUT_STATE;

	tmp = _crx_find_ps_module(ZSTR_VAL(new_value));

	if (stage == CREX_INI_STAGE_RUNTIME) {
		err_type = E_WARNING;
	}

	if (PG(modules_activated) && !tmp) {
		/* Do not output error when restoring ini options. */
		if (stage != CREX_INI_STAGE_DEACTIVATE) {
			crx_error_docref(NULL, err_type, "Session save handler \"%s\" cannot be found", ZSTR_VAL(new_value));
		}

		return FAILURE;
	}

	/* "user" save handler should not be set by user */
	if (!PS(set_handler) &&  tmp == ps_user_ptr) {
		crx_error_docref(NULL, err_type, "Session save handler \"user\" cannot be set by ini_set()");
		return FAILURE;
	}

	PS(default_mod) = PS(mod);
	PS(mod) = tmp;

	return SUCCESS;
}
/* }}} */

static CRX_INI_MH(OnUpdateSerializer) /* {{{ */
{
	const ps_serializer *tmp;

	SESSION_CHECK_ACTIVE_STATE;
	SESSION_CHECK_OUTPUT_STATE;

	tmp = _crx_find_ps_serializer(ZSTR_VAL(new_value));

	if (PG(modules_activated) && !tmp) {
		int err_type;

		if (stage == CREX_INI_STAGE_RUNTIME) {
			err_type = E_WARNING;
		} else {
			err_type = E_ERROR;
		}

		/* Do not output error when restoring ini options. */
		if (stage != CREX_INI_STAGE_DEACTIVATE) {
			crx_error_docref(NULL, err_type, "Serialization handler \"%s\" cannot be found", ZSTR_VAL(new_value));
		}
		return FAILURE;
	}
	PS(serializer) = tmp;

	return SUCCESS;
}
/* }}} */

static CRX_INI_MH(OnUpdateSaveDir) /* {{{ */
{
	SESSION_CHECK_ACTIVE_STATE;
	SESSION_CHECK_OUTPUT_STATE;

	/* Only do the safemode/open_basedir check at runtime */
	if (stage == CRX_INI_STAGE_RUNTIME || stage == CRX_INI_STAGE_HTACCESS) {
		char *p;

		if (memchr(ZSTR_VAL(new_value), '\0', ZSTR_LEN(new_value)) != NULL) {
			return FAILURE;
		}

		/* we do not use crex_memrchr() since path can contain ; itself */
		if ((p = strchr(ZSTR_VAL(new_value), ';'))) {
			char *p2;
			p++;
			if ((p2 = strchr(p, ';'))) {
				p = p2 + 1;
			}
		} else {
			p = ZSTR_VAL(new_value);
		}

		if (PG(open_basedir) && *p && crx_check_open_basedir(p)) {
			return FAILURE;
		}
	}

	return OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
}
/* }}} */


static CRX_INI_MH(OnUpdateName) /* {{{ */
{
	SESSION_CHECK_ACTIVE_STATE;
	SESSION_CHECK_OUTPUT_STATE;

	/* Numeric session.name won't work at all */
	if ((!ZSTR_LEN(new_value) || is_numeric_string(ZSTR_VAL(new_value), ZSTR_LEN(new_value), NULL, NULL, 0))) {
		int err_type;

		if (stage == CREX_INI_STAGE_RUNTIME || stage == CREX_INI_STAGE_ACTIVATE || stage == CREX_INI_STAGE_STARTUP) {
			err_type = E_WARNING;
		} else {
			err_type = E_ERROR;
		}

		/* Do not output error when restoring ini options. */
		if (stage != CREX_INI_STAGE_DEACTIVATE) {
			crx_error_docref(NULL, err_type, "session.name \"%s\" cannot be numeric or empty", ZSTR_VAL(new_value));
		}
		return FAILURE;
	}

	return OnUpdateStringUnempty(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
}
/* }}} */


static CRX_INI_MH(OnUpdateCookieLifetime) /* {{{ */
{
	SESSION_CHECK_ACTIVE_STATE;
	SESSION_CHECK_OUTPUT_STATE;
	if (atol(ZSTR_VAL(new_value)) < 0) {
		crx_error_docref(NULL, E_WARNING, "CookieLifetime cannot be negative");
		return FAILURE;
	}
	return OnUpdateLongGEZero(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
}
/* }}} */


static CRX_INI_MH(OnUpdateSessionLong) /* {{{ */
{
	SESSION_CHECK_ACTIVE_STATE;
	SESSION_CHECK_OUTPUT_STATE;
	return OnUpdateLong(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
}
/* }}} */


static CRX_INI_MH(OnUpdateSessionString) /* {{{ */
{
	SESSION_CHECK_ACTIVE_STATE;
	SESSION_CHECK_OUTPUT_STATE;
	return OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
}
/* }}} */


static CRX_INI_MH(OnUpdateSessionBool) /* {{{ */
{
	SESSION_CHECK_ACTIVE_STATE;
	SESSION_CHECK_OUTPUT_STATE;
	return OnUpdateBool(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
}
/* }}} */


static CRX_INI_MH(OnUpdateSidLength) /* {{{ */
{
	crex_long val;
	char *endptr = NULL;

	SESSION_CHECK_ACTIVE_STATE;
	SESSION_CHECK_OUTPUT_STATE;
	val = CREX_STRTOL(ZSTR_VAL(new_value), &endptr, 10);
	if (endptr && (*endptr == '\0')
		&& val >= 22 && val <= PS_MAX_SID_LENGTH) {
		/* Numeric value */
		PS(sid_length) = val;
		return SUCCESS;
	}

	crx_error_docref(NULL, E_WARNING, "session.configuration \"session.sid_length\" must be between 22 and 256");
	return FAILURE;
}
/* }}} */

static CRX_INI_MH(OnUpdateSidBits) /* {{{ */
{
	crex_long val;
	char *endptr = NULL;

	SESSION_CHECK_ACTIVE_STATE;
	SESSION_CHECK_OUTPUT_STATE;
	val = CREX_STRTOL(ZSTR_VAL(new_value), &endptr, 10);
	if (endptr && (*endptr == '\0')
		&& val >= 4 && val <=6) {
		/* Numeric value */
		PS(sid_bits_per_character) = val;
		return SUCCESS;
	}

	crx_error_docref(NULL, E_WARNING, "session.configuration \"session.sid_bits_per_character\" must be between 4 and 6");
	return FAILURE;
}
/* }}} */

static CRX_INI_MH(OnUpdateRfc1867Freq) /* {{{ */
{
	int tmp = CREX_ATOL(ZSTR_VAL(new_value));
	if(tmp < 0) {
		crx_error_docref(NULL, E_WARNING, "session.upload_progress.freq must be greater than or equal to 0");
		return FAILURE;
	}
	if(ZSTR_LEN(new_value) > 0 && ZSTR_VAL(new_value)[ZSTR_LEN(new_value)-1] == '%') {
		if(tmp > 100) {
			crx_error_docref(NULL, E_WARNING, "session.upload_progress.freq must be less than or equal to 100%%");
			return FAILURE;
		}
		PS(rfc1867_freq) = -tmp;
	} else {
		PS(rfc1867_freq) = tmp;
	}
	return SUCCESS;
} /* }}} */

/* {{{ CRX_INI */
CRX_INI_BEGIN()
	STD_CRX_INI_ENTRY("session.save_path",          "",          CRX_INI_ALL, OnUpdateSaveDir,       save_path,          crx_ps_globals,    ps_globals)
	STD_CRX_INI_ENTRY("session.name",               "CRXSESSID", CRX_INI_ALL, OnUpdateName,          session_name,       crx_ps_globals,    ps_globals)
	CRX_INI_ENTRY("session.save_handler",           "files",     CRX_INI_ALL, OnUpdateSaveHandler)
	STD_CRX_INI_BOOLEAN("session.auto_start",       "0",         CRX_INI_PERDIR, OnUpdateBool,       auto_start,         crx_ps_globals,    ps_globals)
	STD_CRX_INI_ENTRY("session.gc_probability",     "1",         CRX_INI_ALL, OnUpdateSessionLong,          gc_probability,     crx_ps_globals,    ps_globals)
	STD_CRX_INI_ENTRY("session.gc_divisor",         "100",       CRX_INI_ALL, OnUpdateSessionLong,          gc_divisor,         crx_ps_globals,    ps_globals)
	STD_CRX_INI_ENTRY("session.gc_maxlifetime",     "1440",      CRX_INI_ALL, OnUpdateSessionLong,          gc_maxlifetime,     crx_ps_globals,    ps_globals)
	CRX_INI_ENTRY("session.serialize_handler",      "crx",       CRX_INI_ALL, OnUpdateSerializer)
	STD_CRX_INI_ENTRY("session.cookie_lifetime",    "0",         CRX_INI_ALL, OnUpdateCookieLifetime,cookie_lifetime,    crx_ps_globals,    ps_globals)
	STD_CRX_INI_ENTRY("session.cookie_path",        "/",         CRX_INI_ALL, OnUpdateSessionString, cookie_path,        crx_ps_globals,    ps_globals)
	STD_CRX_INI_ENTRY("session.cookie_domain",      "",          CRX_INI_ALL, OnUpdateSessionString, cookie_domain,      crx_ps_globals,    ps_globals)
	STD_CRX_INI_BOOLEAN("session.cookie_secure",    "0",         CRX_INI_ALL, OnUpdateSessionBool,   cookie_secure,      crx_ps_globals,    ps_globals)
	STD_CRX_INI_BOOLEAN("session.cookie_httponly",  "0",         CRX_INI_ALL, OnUpdateSessionBool,   cookie_httponly,    crx_ps_globals,    ps_globals)
	STD_CRX_INI_ENTRY("session.cookie_samesite",    "",          CRX_INI_ALL, OnUpdateSessionString, cookie_samesite,    crx_ps_globals,    ps_globals)
	STD_CRX_INI_BOOLEAN("session.use_cookies",      "1",         CRX_INI_ALL, OnUpdateSessionBool,   use_cookies,        crx_ps_globals,    ps_globals)
	STD_CRX_INI_BOOLEAN("session.use_only_cookies", "1",         CRX_INI_ALL, OnUpdateSessionBool,   use_only_cookies,   crx_ps_globals,    ps_globals)
	STD_CRX_INI_BOOLEAN("session.use_strict_mode",  "0",         CRX_INI_ALL, OnUpdateSessionBool,   use_strict_mode,    crx_ps_globals,    ps_globals)
	STD_CRX_INI_ENTRY("session.referer_check",      "",          CRX_INI_ALL, OnUpdateSessionString, extern_referer_chk, crx_ps_globals,    ps_globals)
	STD_CRX_INI_ENTRY("session.cache_limiter",      "nocache",   CRX_INI_ALL, OnUpdateSessionString, cache_limiter,      crx_ps_globals,    ps_globals)
	STD_CRX_INI_ENTRY("session.cache_expire",       "180",       CRX_INI_ALL, OnUpdateSessionLong,   cache_expire,       crx_ps_globals,    ps_globals)
	STD_CRX_INI_BOOLEAN("session.use_trans_sid",    "0",         CRX_INI_ALL, OnUpdateSessionBool,   use_trans_sid,      crx_ps_globals,    ps_globals)
	CRX_INI_ENTRY("session.sid_length",             "32",        CRX_INI_ALL, OnUpdateSidLength)
	CRX_INI_ENTRY("session.sid_bits_per_character", "4",         CRX_INI_ALL, OnUpdateSidBits)
	STD_CRX_INI_BOOLEAN("session.lazy_write",       "1",         CRX_INI_ALL, OnUpdateSessionBool,    lazy_write,         crx_ps_globals,    ps_globals)

	/* Upload progress */
	STD_CRX_INI_BOOLEAN("session.upload_progress.enabled",
	                                                "1",     CREX_INI_PERDIR, OnUpdateBool,        rfc1867_enabled, crx_ps_globals, ps_globals)
	STD_CRX_INI_BOOLEAN("session.upload_progress.cleanup",
	                                                "1",     CREX_INI_PERDIR, OnUpdateBool,        rfc1867_cleanup, crx_ps_globals, ps_globals)
	STD_CRX_INI_ENTRY("session.upload_progress.prefix",
	                                     "upload_progress_", CREX_INI_PERDIR, OnUpdateString,      rfc1867_prefix,  crx_ps_globals, ps_globals)
	STD_CRX_INI_ENTRY("session.upload_progress.name",
	                          "CRX_SESSION_UPLOAD_PROGRESS", CREX_INI_PERDIR, OnUpdateString,      rfc1867_name,    crx_ps_globals, ps_globals)
	STD_CRX_INI_ENTRY("session.upload_progress.freq",  "1%", CREX_INI_PERDIR, OnUpdateRfc1867Freq, rfc1867_freq,    crx_ps_globals, ps_globals)
	STD_CRX_INI_ENTRY("session.upload_progress.min_freq",
	                                                   "1",  CREX_INI_PERDIR, OnUpdateReal,        rfc1867_min_freq,crx_ps_globals, ps_globals)

	/* Commented out until future discussion */
	/* CRX_INI_ENTRY("session.encode_sources", "globals,track", CRX_INI_ALL, NULL) */
CRX_INI_END()
/* }}} */

/* ***************
   * Serializers *
   *************** */
PS_SERIALIZER_ENCODE_FUNC(crx_serialize) /* {{{ */
{
	smart_str buf = {0};
	crx_serialize_data_t var_hash;

	IF_SESSION_VARS() {
		CRX_VAR_SERIALIZE_INIT(var_hash);
		crx_var_serialize(&buf, C_REFVAL(PS(http_session_vars)), &var_hash);
		CRX_VAR_SERIALIZE_DESTROY(var_hash);
	}
	return buf.s;
}
/* }}} */

PS_SERIALIZER_DECODE_FUNC(crx_serialize) /* {{{ */
{
	const char *endptr = val + vallen;
	zval session_vars;
	crx_unserialize_data_t var_hash;
	bool result;
	crex_string *var_name = ZSTR_INIT_LITERAL("_SESSION", 0);

	ZVAL_NULL(&session_vars);
	CRX_VAR_UNSERIALIZE_INIT(var_hash);
	result = crx_var_unserialize(
		&session_vars, (const unsigned char **)&val, (const unsigned char *)endptr, &var_hash);
	CRX_VAR_UNSERIALIZE_DESTROY(var_hash);
	if (!result) {
		zval_ptr_dtor(&session_vars);
		ZVAL_NULL(&session_vars);
	}

	if (!C_ISUNDEF(PS(http_session_vars))) {
		zval_ptr_dtor(&PS(http_session_vars));
	}
	if (C_TYPE(session_vars) == IS_NULL) {
		array_init(&session_vars);
	}
	ZVAL_NEW_REF(&PS(http_session_vars), &session_vars);
	C_ADDREF_P(&PS(http_session_vars));
	crex_hash_update_ind(&EG(symbol_table), var_name, &PS(http_session_vars));
	crex_string_release_ex(var_name, 0);
	return result || !vallen ? SUCCESS : FAILURE;
}
/* }}} */

#define PS_BIN_NR_OF_BITS 8
#define PS_BIN_UNDEF (1<<(PS_BIN_NR_OF_BITS-1))
#define PS_BIN_MAX (PS_BIN_UNDEF-1)

PS_SERIALIZER_ENCODE_FUNC(crx_binary) /* {{{ */
{
	smart_str buf = {0};
	crx_serialize_data_t var_hash;
	PS_ENCODE_VARS;

	CRX_VAR_SERIALIZE_INIT(var_hash);

	PS_ENCODE_LOOP(
			if (ZSTR_LEN(key) > PS_BIN_MAX) continue;
			smart_str_appendc(&buf, (unsigned char)ZSTR_LEN(key));
			smart_str_appendl(&buf, ZSTR_VAL(key), ZSTR_LEN(key));
			crx_var_serialize(&buf, struc, &var_hash);
	);

	smart_str_0(&buf);
	CRX_VAR_SERIALIZE_DESTROY(var_hash);

	return buf.s;
}
/* }}} */

PS_SERIALIZER_DECODE_FUNC(crx_binary) /* {{{ */
{
	const char *p;
	const char *endptr = val + vallen;
	crex_string *name;
	crx_unserialize_data_t var_hash;
	zval *current, rv;

	CRX_VAR_UNSERIALIZE_INIT(var_hash);

	for (p = val; p < endptr; ) {
		size_t namelen = ((unsigned char)(*p)) & (~PS_BIN_UNDEF);

		if (namelen > PS_BIN_MAX || (p + namelen) >= endptr) {
			CRX_VAR_UNSERIALIZE_DESTROY(var_hash);
			return FAILURE;
		}

		name = crex_string_init(p + 1, namelen, 0);
		p += namelen + 1;
		current = var_tmp_var(&var_hash);

		if (crx_var_unserialize(current, (const unsigned char **) &p, (const unsigned char *) endptr, &var_hash)) {
			ZVAL_PTR(&rv, current);
			crx_set_session_var(name, &rv, &var_hash);
		} else {
			crex_string_release_ex(name, 0);
			crx_session_normalize_vars();
			CRX_VAR_UNSERIALIZE_DESTROY(var_hash);
			return FAILURE;
		}
		crex_string_release_ex(name, 0);
	}

	crx_session_normalize_vars();
	CRX_VAR_UNSERIALIZE_DESTROY(var_hash);

	return SUCCESS;
}
/* }}} */

#define PS_DELIMITER '|'

PS_SERIALIZER_ENCODE_FUNC(crx) /* {{{ */
{
	smart_str buf = {0};
	crx_serialize_data_t var_hash;
	PS_ENCODE_VARS;

	CRX_VAR_SERIALIZE_INIT(var_hash);

	PS_ENCODE_LOOP(
		smart_str_appendl(&buf, ZSTR_VAL(key), ZSTR_LEN(key));
		if (memchr(ZSTR_VAL(key), PS_DELIMITER, ZSTR_LEN(key))) {
			CRX_VAR_SERIALIZE_DESTROY(var_hash);
			smart_str_free(&buf);
			return NULL;
		}
		smart_str_appendc(&buf, PS_DELIMITER);
		crx_var_serialize(&buf, struc, &var_hash);
	);

	smart_str_0(&buf);

	CRX_VAR_SERIALIZE_DESTROY(var_hash);
	return buf.s;
}
/* }}} */

PS_SERIALIZER_DECODE_FUNC(crx) /* {{{ */
{
	const char *p, *q;
	const char *endptr = val + vallen;
	ptrdiff_t namelen;
	crex_string *name;
	crex_result retval = SUCCESS;
	crx_unserialize_data_t var_hash;
	zval *current, rv;

	CRX_VAR_UNSERIALIZE_INIT(var_hash);

	p = val;

	while (p < endptr) {
		q = p;
		while (*q != PS_DELIMITER) {
			if (++q >= endptr) {
				retval = FAILURE;
				goto break_outer_loop;
			}
		}

		namelen = q - p;
		name = crex_string_init(p, namelen, 0);
		q++;

		current = var_tmp_var(&var_hash);
		if (crx_var_unserialize(current, (const unsigned char **)&q, (const unsigned char *)endptr, &var_hash)) {
			ZVAL_PTR(&rv, current);
			crx_set_session_var(name, &rv, &var_hash);
		} else {
			crex_string_release_ex(name, 0);
			retval = FAILURE;
			goto break_outer_loop;
		}
		crex_string_release_ex(name, 0);
		p = q;
	}

break_outer_loop:
	crx_session_normalize_vars();

	CRX_VAR_UNSERIALIZE_DESTROY(var_hash);

	return retval;
}
/* }}} */

#define MAX_SERIALIZERS 32
#define PREDEFINED_SERIALIZERS 3

static ps_serializer ps_serializers[MAX_SERIALIZERS + 1] = {
	PS_SERIALIZER_ENTRY(crx_serialize),
	PS_SERIALIZER_ENTRY(crx),
	PS_SERIALIZER_ENTRY(crx_binary)
};

CRXAPI crex_result crx_session_register_serializer(const char *name, crex_string *(*encode)(PS_SERIALIZER_ENCODE_ARGS), crex_result (*decode)(PS_SERIALIZER_DECODE_ARGS)) /* {{{ */
{
	crex_result ret = FAILURE;

	for (int i = 0; i < MAX_SERIALIZERS; i++) {
		if (ps_serializers[i].name == NULL) {
			ps_serializers[i].name = name;
			ps_serializers[i].encode = encode;
			ps_serializers[i].decode = decode;
			ps_serializers[i + 1].name = NULL;
			ret = SUCCESS;
			break;
		}
	}
	return ret;
}
/* }}} */

/* *******************
   * Storage Modules *
   ******************* */

#define MAX_MODULES 32
#define PREDEFINED_MODULES 2

static const ps_module *ps_modules[MAX_MODULES + 1] = {
	ps_files_ptr,
	ps_user_ptr
};

CRXAPI crex_result crx_session_register_module(const ps_module *ptr) /* {{{ */
{
	int ret = FAILURE;

	for (int i = 0; i < MAX_MODULES; i++) {
		if (!ps_modules[i]) {
			ps_modules[i] = ptr;
			ret = SUCCESS;
			break;
		}
	}
	return ret;
}
/* }}} */

/* Dummy PS module function */
/* We consider any ID valid (thus also implying that a session with such an ID exists),
	thus we always return SUCCESS */
CRXAPI crex_result crx_session_validate_sid(PS_VALIDATE_SID_ARGS) {
	return SUCCESS;
}

/* Dummy PS module function */
CRXAPI crex_result crx_session_update_timestamp(PS_UPDATE_TIMESTAMP_ARGS) {
	return SUCCESS;
}


/* ******************
   * Cache Limiters *
   ****************** */

typedef struct {
	char *name;
	void (*func)(void);
} crx_session_cache_limiter_t;

#define CACHE_LIMITER(name) _crx_cache_limiter_##name
#define CACHE_LIMITER_FUNC(name) static void CACHE_LIMITER(name)(void)
#define CACHE_LIMITER_ENTRY(name) { #name, CACHE_LIMITER(name) },
#define ADD_HEADER(a) sapi_add_header(a, strlen(a), 1);
#define MAX_STR 512

static const char *month_names[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static const char *week_days[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

static inline void strcpy_gmt(char *ubuf, time_t *when) /* {{{ */
{
	char buf[MAX_STR];
	struct tm tm, *res;
	int n;

	res = crx_gmtime_r(when, &tm);

	if (!res) {
		ubuf[0] = '\0';
		return;
	}

	n = slprintf(buf, sizeof(buf), "%s, %02d %s %d %02d:%02d:%02d GMT", /* SAFE */
				week_days[tm.tm_wday], tm.tm_mday,
				month_names[tm.tm_mon], tm.tm_year + 1900,
				tm.tm_hour, tm.tm_min,
				tm.tm_sec);
	memcpy(ubuf, buf, n);
	ubuf[n] = '\0';
}
/* }}} */

static inline void last_modified(void) /* {{{ */
{
	const char *path;
	crex_stat_t sb = {0};
	char buf[MAX_STR + 1];

	path = SG(request_info).path_translated;
	if (path) {
		if (VCWD_STAT(path, &sb) == -1) {
			return;
		}

#define LAST_MODIFIED "Last-Modified: "
		memcpy(buf, LAST_MODIFIED, sizeof(LAST_MODIFIED) - 1);
		strcpy_gmt(buf + sizeof(LAST_MODIFIED) - 1, &sb.st_mtime);
		ADD_HEADER(buf);
	}
}
/* }}} */

#define EXPIRES "Expires: "
CACHE_LIMITER_FUNC(public) /* {{{ */
{
	char buf[MAX_STR + 1];
	struct timeval tv;
	time_t now;

	gettimeofday(&tv, NULL);
	now = tv.tv_sec + PS(cache_expire) * 60;
	memcpy(buf, EXPIRES, sizeof(EXPIRES) - 1);
	strcpy_gmt(buf + sizeof(EXPIRES) - 1, &now);
	ADD_HEADER(buf);

	snprintf(buf, sizeof(buf) , "Cache-Control: public, max-age=" CREX_LONG_FMT, PS(cache_expire) * 60); /* SAFE */
	ADD_HEADER(buf);

	last_modified();
}
/* }}} */

CACHE_LIMITER_FUNC(private_no_expire) /* {{{ */
{
	char buf[MAX_STR + 1];

	snprintf(buf, sizeof(buf), "Cache-Control: private, max-age=" CREX_LONG_FMT, PS(cache_expire) * 60); /* SAFE */
	ADD_HEADER(buf);

	last_modified();
}
/* }}} */

CACHE_LIMITER_FUNC(private) /* {{{ */
{
	ADD_HEADER("Expires: Thu, 19 Nov 1981 08:52:00 GMT");
	CACHE_LIMITER(private_no_expire)();
}
/* }}} */

CACHE_LIMITER_FUNC(nocache) /* {{{ */
{
	ADD_HEADER("Expires: Thu, 19 Nov 1981 08:52:00 GMT");

	/* For HTTP/1.1 conforming clients */
	ADD_HEADER("Cache-Control: no-store, no-cache, must-revalidate");

	/* For HTTP/1.0 conforming clients */
	ADD_HEADER("Pragma: no-cache");
}
/* }}} */

static const crx_session_cache_limiter_t crx_session_cache_limiters[] = {
	CACHE_LIMITER_ENTRY(public)
	CACHE_LIMITER_ENTRY(private)
	CACHE_LIMITER_ENTRY(private_no_expire)
	CACHE_LIMITER_ENTRY(nocache)
	{0}
};

static int crx_session_cache_limiter(void) /* {{{ */
{
	const crx_session_cache_limiter_t *lim;

	if (PS(cache_limiter)[0] == '\0') return 0;
	if (PS(session_status) != crx_session_active) return -1;

	if (SG(headers_sent)) {
		const char *output_start_filename = crx_output_get_start_filename();
		int output_start_lineno = crx_output_get_start_lineno();

		crx_session_abort();
		if (output_start_filename) {
			crx_error_docref(NULL, E_WARNING, "Session cache limiter cannot be sent after headers have already been sent (output started at %s:%d)", output_start_filename, output_start_lineno);
		} else {
			crx_error_docref(NULL, E_WARNING, "Session cache limiter cannot be sent after headers have already been sent");
		}
		return -2;
	}

	for (lim = crx_session_cache_limiters; lim->name; lim++) {
		if (!strcasecmp(lim->name, PS(cache_limiter))) {
			lim->func();
			return 0;
		}
	}

	return -1;
}
/* }}} */

/* *********************
   * Cookie Management *
   ********************* */

/*
 * Remove already sent session ID cookie.
 * It must be directly removed from SG(sapi_header) because sapi_add_header_ex()
 * removes all of matching cookie. i.e. It deletes all of Set-Cookie headers.
 */
static void crx_session_remove_cookie(void) {
	sapi_header_struct *header;
	crex_llist *l = &SG(sapi_headers).headers;
	crex_llist_element *next;
	crex_llist_element *current;
	char *session_cookie;
	size_t session_cookie_len;
	size_t len = sizeof("Set-Cookie")-1;

	CREX_ASSERT(strpbrk(PS(session_name), SESSION_FORBIDDEN_CHARS) == NULL);
	spprintf(&session_cookie, 0, "Set-Cookie: %s=", PS(session_name));

	session_cookie_len = strlen(session_cookie);
	current = l->head;
	while (current) {
		header = (sapi_header_struct *)(current->data);
		next = current->next;
		if (header->header_len > len && header->header[len] == ':'
			&& !strncmp(header->header, session_cookie, session_cookie_len)) {
			if (current->prev) {
				current->prev->next = next;
			} else {
				l->head = next;
			}
			if (next) {
				next->prev = current->prev;
			} else {
				l->tail = current->prev;
			}
			sapi_free_header(header);
			efree(current);
			--l->count;
		}
		current = next;
	}
	efree(session_cookie);
}

static crex_result crx_session_send_cookie(void) /* {{{ */
{
	smart_str ncookie = {0};
	crex_string *date_fmt = NULL;
	crex_string *e_id;

	if (SG(headers_sent)) {
		const char *output_start_filename = crx_output_get_start_filename();
		int output_start_lineno = crx_output_get_start_lineno();

		if (output_start_filename) {
			crx_error_docref(NULL, E_WARNING, "Session cookie cannot be sent after headers have already been sent (output started at %s:%d)", output_start_filename, output_start_lineno);
		} else {
			crx_error_docref(NULL, E_WARNING, "Session cookie cannot be sent after headers have already been sent");
		}
		return FAILURE;
	}

	/* Prevent broken Set-Cookie header, because the session_name might be user supplied */
	if (strpbrk(PS(session_name), SESSION_FORBIDDEN_CHARS) != NULL) {   /* man isspace for \013 and \014 */
		crx_error_docref(NULL, E_WARNING, "session.name cannot contain any of the following '=,;.[ \\t\\r\\n\\013\\014'");
		return FAILURE;
	}

	/* URL encode id because it might be user supplied */
	e_id = crx_url_encode(ZSTR_VAL(PS(id)), ZSTR_LEN(PS(id)));

	smart_str_appendl(&ncookie, "Set-Cookie: ", sizeof("Set-Cookie: ")-1);
	smart_str_appendl(&ncookie, PS(session_name), strlen(PS(session_name)));
	smart_str_appendc(&ncookie, '=');
	smart_str_appendl(&ncookie, ZSTR_VAL(e_id), ZSTR_LEN(e_id));

	crex_string_release_ex(e_id, 0);

	if (PS(cookie_lifetime) > 0) {
		struct timeval tv;
		time_t t;

		gettimeofday(&tv, NULL);
		t = tv.tv_sec + PS(cookie_lifetime);

		if (t > 0) {
			date_fmt = crx_format_date("D, d M Y H:i:s \\G\\M\\T", sizeof("D, d M Y H:i:s \\G\\M\\T")-1, t, 0);
			smart_str_appends(&ncookie, COOKIE_EXPIRES);
			smart_str_appendl(&ncookie, ZSTR_VAL(date_fmt), ZSTR_LEN(date_fmt));
			crex_string_release_ex(date_fmt, 0);

			smart_str_appends(&ncookie, COOKIE_MAX_AGE);
			smart_str_append_long(&ncookie, PS(cookie_lifetime));
		}
	}

	if (PS(cookie_path)[0]) {
		smart_str_appends(&ncookie, COOKIE_PATH);
		smart_str_appends(&ncookie, PS(cookie_path));
	}

	if (PS(cookie_domain)[0]) {
		smart_str_appends(&ncookie, COOKIE_DOMAIN);
		smart_str_appends(&ncookie, PS(cookie_domain));
	}

	if (PS(cookie_secure)) {
		smart_str_appends(&ncookie, COOKIE_SECURE);
	}

	if (PS(cookie_httponly)) {
		smart_str_appends(&ncookie, COOKIE_HTTPONLY);
	}

	if (PS(cookie_samesite)[0]) {
		smart_str_appends(&ncookie, COOKIE_SAMESITE);
		smart_str_appends(&ncookie, PS(cookie_samesite));
	}

	smart_str_0(&ncookie);

	crx_session_remove_cookie(); /* remove already sent session ID cookie */
	/*	'replace' must be 0 here, else a previous Set-Cookie
		header, probably sent with setcookie() will be replaced! */
	sapi_add_header_ex(estrndup(ZSTR_VAL(ncookie.s), ZSTR_LEN(ncookie.s)), ZSTR_LEN(ncookie.s), 0, 0);
	smart_str_free(&ncookie);

	return SUCCESS;
}
/* }}} */

CRXAPI const ps_module *_crx_find_ps_module(const char *name) /* {{{ */
{
	const ps_module *ret = NULL;
	const ps_module **mod;
	int i;

	for (i = 0, mod = ps_modules; i < MAX_MODULES; i++, mod++) {
		if (*mod && !strcasecmp(name, (*mod)->s_name)) {
			ret = *mod;
			break;
		}
	}
	return ret;
}
/* }}} */

CRXAPI const ps_serializer *_crx_find_ps_serializer(const char *name) /* {{{ */
{
	const ps_serializer *ret = NULL;
	const ps_serializer *mod;

	for (mod = ps_serializers; mod->name; mod++) {
		if (!strcasecmp(name, mod->name)) {
			ret = mod;
			break;
		}
	}
	return ret;
}
/* }}} */

static void ppid2sid(zval *ppid) {
	ZVAL_DEREF(ppid);
	if (C_TYPE_P(ppid) == IS_STRING) {
		PS(id) = crex_string_init(C_STRVAL_P(ppid), C_STRLEN_P(ppid), 0);
		PS(send_cookie) = 0;
	} else {
		PS(id) = NULL;
		PS(send_cookie) = 1;
	}
}


CRXAPI crex_result crx_session_reset_id(void) /* {{{ */
{
	int module_number = PS(module_number);
	zval *sid, *data, *ppid;
	bool apply_trans_sid;

	if (!PS(id)) {
		crx_error_docref(NULL, E_WARNING, "Cannot set session ID - session ID is not initialized");
		return FAILURE;
	}

	if (PS(use_cookies) && PS(send_cookie)) {
		crx_session_send_cookie();
		PS(send_cookie) = 0;
	}

	/* If the SID constant exists, destroy it. */
	/* We must not delete any items in EG(crex_constants) */
	/* crex_hash_str_del(EG(crex_constants), "sid", sizeof("sid") - 1); */
	sid = crex_get_constant_str("SID", sizeof("SID") - 1);

	if (PS(define_sid)) {
		smart_str var = {0};

		smart_str_appends(&var, PS(session_name));
		smart_str_appendc(&var, '=');
		smart_str_appends(&var, ZSTR_VAL(PS(id)));
		smart_str_0(&var);
		if (sid) {
			zval_ptr_dtor_str(sid);
			ZVAL_STR(sid, smart_str_extract(&var));
		} else {
			REGISTER_STRINGL_CONSTANT("SID", ZSTR_VAL(var.s), ZSTR_LEN(var.s), 0);
			smart_str_free(&var);
		}
	} else {
		if (sid) {
			zval_ptr_dtor_str(sid);
			ZVAL_EMPTY_STRING(sid);
		} else {
			REGISTER_STRINGL_CONSTANT("SID", "", 0, 0);
		}
	}

	/* Apply trans sid if sid cookie is not set */
	apply_trans_sid = 0;
	if (APPLY_TRANS_SID) {
		apply_trans_sid = 1;
		if (PS(use_cookies) &&
			(data = crex_hash_str_find(&EG(symbol_table), "_COOKIE", sizeof("_COOKIE") - 1))) {
			ZVAL_DEREF(data);
			if (C_TYPE_P(data) == IS_ARRAY &&
				(ppid = crex_hash_str_find(C_ARRVAL_P(data), PS(session_name), strlen(PS(session_name))))) {
				ZVAL_DEREF(ppid);
				apply_trans_sid = 0;
			}
		}
	}
	if (apply_trans_sid) {
		crex_string *sname;
		sname = crex_string_init(PS(session_name), strlen(PS(session_name)), 0);
		crx_url_scanner_reset_session_var(sname, 1); /* This may fail when session name has changed */
		crex_string_release_ex(sname, 0);
		crx_url_scanner_add_session_var(PS(session_name), strlen(PS(session_name)), ZSTR_VAL(PS(id)), ZSTR_LEN(PS(id)), 1);
	}
	return SUCCESS;
}
/* }}} */


CRXAPI crex_result crx_session_start(void) /* {{{ */
{
	zval *ppid;
	zval *data;
	char *value;
	size_t lensess;

	switch (PS(session_status)) {
		case crx_session_active:
			if (PS(session_started_filename)) {
				crx_error(E_NOTICE, "Ignoring session_start() because a session has already been started (started from %s on line %"PRIu32")", ZSTR_VAL(PS(session_started_filename)), PS(session_started_lineno));
			} else if (PS(auto_start)) {
				/* This option can't be changed at runtime, so we can assume it's because of this */
				crx_error(E_NOTICE, "Ignoring session_start() because a session has already been started automatically");
			} else {
				crx_error(E_NOTICE, "Ignoring session_start() because a session has already been started");
			}
			return FAILURE;
			break;

		case crx_session_disabled:
			value = crex_ini_string("session.save_handler", sizeof("session.save_handler") - 1, 0);
			if (!PS(mod) && value) {
				PS(mod) = _crx_find_ps_module(value);
				if (!PS(mod)) {
					crx_error_docref(NULL, E_WARNING, "Cannot find session save handler \"%s\" - session startup failed", value);
					return FAILURE;
				}
			}
			value = crex_ini_string("session.serialize_handler", sizeof("session.serialize_handler") - 1, 0);
			if (!PS(serializer) && value) {
				PS(serializer) = _crx_find_ps_serializer(value);
				if (!PS(serializer)) {
					crx_error_docref(NULL, E_WARNING, "Cannot find session serialization handler \"%s\" - session startup failed", value);
					return FAILURE;
				}
			}
			PS(session_status) = crx_session_none;
			CREX_FALLTHROUGH;

		case crx_session_none:
		default:
			/* Setup internal flags */
			PS(define_sid) = !PS(use_only_cookies); /* SID constant is defined when non-cookie ID is used */
			PS(send_cookie) = PS(use_cookies) || PS(use_only_cookies);
	}

	lensess = strlen(PS(session_name));

	/*
	 * Cookies are preferred, because initially cookie and get
	 * variables will be available.
	 * URL/POST session ID may be used when use_only_cookies=Off.
	 * session.use_strice_mode=On prevents session adoption.
	 * Session based file upload progress uses non-cookie ID.
	 */

	if (!PS(id)) {
		if (PS(use_cookies) && (data = crex_hash_str_find(&EG(symbol_table), "_COOKIE", sizeof("_COOKIE") - 1))) {
			ZVAL_DEREF(data);
			if (C_TYPE_P(data) == IS_ARRAY && (ppid = crex_hash_str_find(C_ARRVAL_P(data), PS(session_name), lensess))) {
				ppid2sid(ppid);
				PS(send_cookie) = 0;
				PS(define_sid) = 0;
			}
		}
		/* Initialize session ID from non cookie values */
		if (!PS(use_only_cookies)) {
			if (!PS(id) && (data = crex_hash_str_find(&EG(symbol_table), "_GET", sizeof("_GET") - 1))) {
				ZVAL_DEREF(data);
				if (C_TYPE_P(data) == IS_ARRAY && (ppid = crex_hash_str_find(C_ARRVAL_P(data), PS(session_name), lensess))) {
					ppid2sid(ppid);
				}
			}
			if (!PS(id) && (data = crex_hash_str_find(&EG(symbol_table), "_POST", sizeof("_POST") - 1))) {
				ZVAL_DEREF(data);
				if (C_TYPE_P(data) == IS_ARRAY && (ppid = crex_hash_str_find(C_ARRVAL_P(data), PS(session_name), lensess))) {
					ppid2sid(ppid);
				}
			}
			/* Check whether the current request was referred to by
			 * an external site which invalidates the previously found id. */
			if (PS(id) && PS(extern_referer_chk)[0] != '\0' &&
				!C_ISUNDEF(PG(http_globals)[TRACK_VARS_SERVER]) &&
				(data = crex_hash_str_find(C_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_REFERER", sizeof("HTTP_REFERER") - 1)) &&
				C_TYPE_P(data) == IS_STRING &&
				C_STRLEN_P(data) != 0 &&
				strstr(C_STRVAL_P(data), PS(extern_referer_chk)) == NULL
			) {
				crex_string_release_ex(PS(id), 0);
				PS(id) = NULL;
			}
		}
	}

	/* Finally check session id for dangerous characters
	 * Security note: session id may be embedded in HTML pages.*/
	if (PS(id) && strpbrk(ZSTR_VAL(PS(id)), "\r\n\t <>'\"\\")) {
		crex_string_release_ex(PS(id), 0);
		PS(id) = NULL;
	}

	if (crx_session_initialize() == FAILURE
		|| crx_session_cache_limiter() == -2) {
		PS(session_status) = crx_session_none;
		if (PS(id)) {
			crex_string_release_ex(PS(id), 0);
			PS(id) = NULL;
		}
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

CRXAPI crex_result crx_session_flush(int write) /* {{{ */
{
	if (PS(session_status) == crx_session_active) {
		crx_session_save_current_state(write);
		PS(session_status) = crx_session_none;
		return SUCCESS;
	}
	return FAILURE;
}
/* }}} */

static crex_result crx_session_abort(void) /* {{{ */
{
	if (PS(session_status) == crx_session_active) {
		if (PS(mod_data) || PS(mod_user_implemented)) {
			PS(mod)->s_close(&PS(mod_data));
		}
		PS(session_status) = crx_session_none;
		return SUCCESS;
	}
	return FAILURE;
}
/* }}} */

static crex_result crx_session_reset(void) /* {{{ */
{
	if (PS(session_status) == crx_session_active
		&& crx_session_initialize() == SUCCESS) {
		return SUCCESS;
	}
	return FAILURE;
}
/* }}} */


/* This API is not used by any CRX modules including session currently.
   session_adapt_url() may be used to set Session ID to target url without
   starting "URL-Rewriter" output handler. */
CRXAPI void session_adapt_url(const char *url, size_t url_len, char **new_url, size_t *new_len) /* {{{ */
{
	if (APPLY_TRANS_SID && (PS(session_status) == crx_session_active)) {
		*new_url = crx_url_scanner_adapt_single_url(url, url_len, PS(session_name), ZSTR_VAL(PS(id)), new_len, 1);
	}
}
/* }}} */

/* ********************************
   * Userspace exported functions *
   ******************************** */

/* {{{ session_set_cookie_params(array options)
   Set session cookie parameters */
CRX_FUNCTION(session_set_cookie_params)
{
	HashTable *options_ht;
	crex_long lifetime_long;
	crex_string *lifetime = NULL, *path = NULL, *domain = NULL, *samesite = NULL;
	bool secure = 0, secure_null = 1;
	bool httponly = 0, httponly_null = 1;
	crex_string *ini_name;
	crex_result result;
	int found = 0;

	if (!PS(use_cookies)) {
		return;
	}

	CREX_PARSE_PARAMETERS_START(1, 5)
		C_PARAM_ARRAY_HT_OR_LONG(options_ht, lifetime_long)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(path)
		C_PARAM_STR_OR_NULL(domain)
		C_PARAM_BOOL_OR_NULL(secure, secure_null)
		C_PARAM_BOOL_OR_NULL(httponly, httponly_null)
	CREX_PARSE_PARAMETERS_END();

	if (PS(session_status) == crx_session_active) {
		crx_error_docref(NULL, E_WARNING, "Session cookie parameters cannot be changed when a session is active");
		RETURN_FALSE;
	}

	if (SG(headers_sent)) {
		crx_error_docref(NULL, E_WARNING, "Session cookie parameters cannot be changed after headers have already been sent");
		RETURN_FALSE;
	}

	if (options_ht) {
		crex_string *key;
		zval *value;

		if (path) {
			crex_argument_value_error(2, "must be null when argument #1 ($lifetime_or_options) is an array");
			RETURN_THROWS();
		}

		if (domain) {
			crex_argument_value_error(3, "must be null when argument #1 ($lifetime_or_options) is an array");
			RETURN_THROWS();
		}

		if (!secure_null) {
			crex_argument_value_error(4, "must be null when argument #1 ($lifetime_or_options) is an array");
			RETURN_THROWS();
		}

		if (!httponly_null) {
			crex_argument_value_error(5, "must be null when argument #1 ($lifetime_or_options) is an array");
			RETURN_THROWS();
		}
		CREX_HASH_FOREACH_STR_KEY_VAL(options_ht, key, value) {
			if (key) {
				ZVAL_DEREF(value);
				if (crex_string_equals_literal_ci(key, "lifetime")) {
					lifetime = zval_get_string(value);
					found++;
				} else if (crex_string_equals_literal_ci(key, "path")) {
					path = zval_get_string(value);
					found++;
				} else if (crex_string_equals_literal_ci(key, "domain")) {
					domain = zval_get_string(value);
					found++;
				} else if (crex_string_equals_literal_ci(key, "secure")) {
					secure = zval_is_true(value);
					secure_null = 0;
					found++;
				} else if (crex_string_equals_literal_ci(key, "httponly")) {
					httponly = zval_is_true(value);
					httponly_null = 0;
					found++;
				} else if (crex_string_equals_literal_ci(key, "samesite")) {
					samesite = zval_get_string(value);
					found++;
				} else {
					crx_error_docref(NULL, E_WARNING, "Argument #1 ($lifetime_or_options) contains an unrecognized key \"%s\"", ZSTR_VAL(key));
				}
			} else {
				crx_error_docref(NULL, E_WARNING, "Argument #1 ($lifetime_or_options) cannot contain numeric keys");
			}
		} CREX_HASH_FOREACH_END();

		if (found == 0) {
			crex_argument_value_error(1, "must contain at least 1 valid key");
			RETURN_THROWS();
		}
	} else {
		lifetime = crex_long_to_str(lifetime_long);
	}

	/* Exception during string conversion */
	if (EG(exception)) {
		goto cleanup;
	}

	if (lifetime) {
		ini_name = ZSTR_INIT_LITERAL("session.cookie_lifetime", 0);
		result = crex_alter_ini_entry(ini_name, lifetime, CRX_INI_USER, CRX_INI_STAGE_RUNTIME);
		crex_string_release_ex(ini_name, 0);
		if (result == FAILURE) {
			RETVAL_FALSE;
			goto cleanup;
		}
	}
	if (path) {
		ini_name = ZSTR_INIT_LITERAL("session.cookie_path", 0);
		result = crex_alter_ini_entry(ini_name, path, CRX_INI_USER, CRX_INI_STAGE_RUNTIME);
		crex_string_release_ex(ini_name, 0);
		if (result == FAILURE) {
			RETVAL_FALSE;
			goto cleanup;
		}
	}
	if (domain) {
		ini_name = ZSTR_INIT_LITERAL("session.cookie_domain", 0);
		result = crex_alter_ini_entry(ini_name, domain, CRX_INI_USER, CRX_INI_STAGE_RUNTIME);
		crex_string_release_ex(ini_name, 0);
		if (result == FAILURE) {
			RETVAL_FALSE;
			goto cleanup;
		}
	}
	if (!secure_null) {
		ini_name = ZSTR_INIT_LITERAL("session.cookie_secure", 0);
		result = crex_alter_ini_entry_chars(ini_name, secure ? "1" : "0", 1, CRX_INI_USER, CRX_INI_STAGE_RUNTIME);
		crex_string_release_ex(ini_name, 0);
		if (result == FAILURE) {
			RETVAL_FALSE;
			goto cleanup;
		}
	}
	if (!httponly_null) {
		ini_name = ZSTR_INIT_LITERAL("session.cookie_httponly", 0);
		result = crex_alter_ini_entry_chars(ini_name, httponly ? "1" : "0", 1, CRX_INI_USER, CRX_INI_STAGE_RUNTIME);
		crex_string_release_ex(ini_name, 0);
		if (result == FAILURE) {
			RETVAL_FALSE;
			goto cleanup;
		}
	}
	if (samesite) {
		ini_name = ZSTR_INIT_LITERAL("session.cookie_samesite", 0);
		result = crex_alter_ini_entry(ini_name, samesite, CRX_INI_USER, CRX_INI_STAGE_RUNTIME);
		crex_string_release_ex(ini_name, 0);
		if (result == FAILURE) {
			RETVAL_FALSE;
			goto cleanup;
		}
	}

	RETVAL_TRUE;

cleanup:
	if (lifetime) crex_string_release(lifetime);
	if (found > 0) {
		if (path) crex_string_release(path);
		if (domain) crex_string_release(domain);
		if (samesite) crex_string_release(samesite);
	}
}
/* }}} */

/* {{{ Return the session cookie parameters */
CRX_FUNCTION(session_get_cookie_params)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);

	add_assoc_long(return_value, "lifetime", PS(cookie_lifetime));
	add_assoc_string(return_value, "path", PS(cookie_path));
	add_assoc_string(return_value, "domain", PS(cookie_domain));
	add_assoc_bool(return_value, "secure", PS(cookie_secure));
	add_assoc_bool(return_value, "httponly", PS(cookie_httponly));
	add_assoc_string(return_value, "samesite", PS(cookie_samesite));
}
/* }}} */

/* {{{ Return the current session name. If newname is given, the session name is replaced with newname */
CRX_FUNCTION(session_name)
{
	crex_string *name = NULL;
	crex_string *ini_name;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|S!", &name) == FAILURE) {
		RETURN_THROWS();
	}

	if (name && PS(session_status) == crx_session_active) {
		crx_error_docref(NULL, E_WARNING, "Session name cannot be changed when a session is active");
		RETURN_FALSE;
	}

	if (name && SG(headers_sent)) {
		crx_error_docref(NULL, E_WARNING, "Session name cannot be changed after headers have already been sent");
		RETURN_FALSE;
	}

	RETVAL_STRING(PS(session_name));

	if (name) {
		ini_name = ZSTR_INIT_LITERAL("session.name", 0);
		crex_alter_ini_entry(ini_name, name, CRX_INI_USER, CRX_INI_STAGE_RUNTIME);
		crex_string_release_ex(ini_name, 0);
	}
}
/* }}} */

/* {{{ Return the current module name used for accessing session data. If newname is given, the module name is replaced with newname */
CRX_FUNCTION(session_module_name)
{
	crex_string *name = NULL;
	crex_string *ini_name;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|S!", &name) == FAILURE) {
		RETURN_THROWS();
	}

	if (name && PS(session_status) == crx_session_active) {
		crx_error_docref(NULL, E_WARNING, "Session save handler module cannot be changed when a session is active");
		RETURN_FALSE;
	}

	if (name && SG(headers_sent)) {
		crx_error_docref(NULL, E_WARNING, "Session save handler module cannot be changed after headers have already been sent");
		RETURN_FALSE;
	}

	/* Set return_value to current module name */
	if (PS(mod) && PS(mod)->s_name) {
		RETVAL_STRING(PS(mod)->s_name);
	} else {
		RETVAL_EMPTY_STRING();
	}

	if (name) {
		if (crex_string_equals_ci(name, ZSTR_KNOWN(CREX_STR_USER))) {
			crex_argument_value_error(1, "cannot be \"user\"");
			RETURN_THROWS();
		}
		if (!_crx_find_ps_module(ZSTR_VAL(name))) {
			crx_error_docref(NULL, E_WARNING, "Session handler module \"%s\" cannot be found", ZSTR_VAL(name));

			zval_ptr_dtor_str(return_value);
			RETURN_FALSE;
		}
		if (PS(mod_data) || PS(mod_user_implemented)) {
			PS(mod)->s_close(&PS(mod_data));
		}
		PS(mod_data) = NULL;

		ini_name = ZSTR_INIT_LITERAL("session.save_handler", 0);
		crex_alter_ini_entry(ini_name, name, CRX_INI_USER, CRX_INI_STAGE_RUNTIME);
		crex_string_release_ex(ini_name, 0);
	}
}
/* }}} */

static bool can_session_handler_be_changed(void) {
	if (PS(session_status) == crx_session_active) {
		crx_error_docref(NULL, E_WARNING, "Session save handler cannot be changed when a session is active");
		return false;
	}

	if (SG(headers_sent)) {
		crx_error_docref(NULL, E_WARNING, "Session save handler cannot be changed after headers have already been sent");
		return false;
	}

	return true;
}

static inline void set_user_save_handler_ini(void) {
	crex_string *ini_name, *ini_val;

	ini_name = ZSTR_INIT_LITERAL("session.save_handler", 0);
	ini_val = ZSTR_KNOWN(CREX_STR_USER);
	PS(set_handler) = 1;
	crex_alter_ini_entry(ini_name, ini_val, CRX_INI_USER, CRX_INI_STAGE_RUNTIME);
	PS(set_handler) = 0;
	crex_string_release_ex(ini_val, 0);
	crex_string_release_ex(ini_name, 0);
}

#define SESSION_RELEASE_USER_HANDLER_OO(struct_name) \
	if (!C_ISUNDEF(PS(mod_user_names).struct_name)) { \
		zval_ptr_dtor(&PS(mod_user_names).struct_name); \
		ZVAL_UNDEF(&PS(mod_user_names).struct_name); \
	}

#define SESSION_SET_USER_HANDLER_OO(struct_name, zstr_method_name) \
	array_init_size(&PS(mod_user_names).struct_name, 2); \
	C_ADDREF_P(obj); \
	add_next_index_zval(&PS(mod_user_names).struct_name, obj); \
	add_next_index_str(&PS(mod_user_names).struct_name, zstr_method_name);

#define SESSION_SET_USER_HANDLER_OO_MANDATORY(struct_name, method_name) \
	if (!C_ISUNDEF(PS(mod_user_names).struct_name)) { \
		zval_ptr_dtor(&PS(mod_user_names).struct_name); \
	} \
	array_init_size(&PS(mod_user_names).struct_name, 2); \
	C_ADDREF_P(obj); \
	add_next_index_zval(&PS(mod_user_names).struct_name, obj); \
	add_next_index_str(&PS(mod_user_names).struct_name, crex_string_init(method_name, strlen(method_name), false));

#define SESSION_SET_USER_HANDLER_PROCEDURAL(struct_name, fci) \
	if (!C_ISUNDEF(PS(mod_user_names).struct_name)) { \
		zval_ptr_dtor(&PS(mod_user_names).struct_name); \
	} \
	ZVAL_COPY(&PS(mod_user_names).struct_name, &fci.function_name);

#define SESSION_SET_USER_HANDLER_PROCEDURAL_OPTIONAL(struct_name, fci) \
	if (CREX_FCI_INITIALIZED(fci)) { \
		SESSION_SET_USER_HANDLER_PROCEDURAL(struct_name, fci); \
	}

/* {{{ Sets user-level functions */
CRX_FUNCTION(session_set_save_handler)
{
	/* OOP Version */
	if (CREX_NUM_ARGS() <= 2) {
		zval *obj = NULL;
		bool register_shutdown = 1;

		if (crex_parse_parameters(CREX_NUM_ARGS(), "O|b", &obj, crx_session_iface_entry, &register_shutdown) == FAILURE) {
			RETURN_THROWS();
		}

		if (!can_session_handler_be_changed()) {
			RETURN_FALSE;
		}

		if (PS(mod_user_class_name)) {
			crex_string_release(PS(mod_user_class_name));
		}
		PS(mod_user_class_name) = crex_string_copy(C_OBJCE_P(obj)->name);

		/* Define mandatory handlers */
		SESSION_SET_USER_HANDLER_OO_MANDATORY(ps_open, "open");
		SESSION_SET_USER_HANDLER_OO_MANDATORY(ps_close, "close");
		SESSION_SET_USER_HANDLER_OO_MANDATORY(ps_read, "read");
		SESSION_SET_USER_HANDLER_OO_MANDATORY(ps_write, "write");
		SESSION_SET_USER_HANDLER_OO_MANDATORY(ps_destroy, "destroy");
		SESSION_SET_USER_HANDLER_OO_MANDATORY(ps_gc, "gc");

		/* Elements of object_methods HashTable are crex_function *method */
		HashTable *object_methods = &C_OBJCE_P(obj)->function_table;

		/* Find implemented methods - SessionIdInterface (optional) */
		/* First release old handlers */
		SESSION_RELEASE_USER_HANDLER_OO(ps_create_sid);
		crex_string *create_sid_name = ZSTR_INIT_LITERAL("create_sid", false);
		if (instanceof_function(C_OBJCE_P(obj), crx_session_id_iface_entry)) {
			SESSION_SET_USER_HANDLER_OO(ps_create_sid, crex_string_copy(create_sid_name));
		} else if (crex_hash_find_ptr(object_methods, create_sid_name)) {
			/* For BC reasons we accept methods even if the class does not implement the interface */
			SESSION_SET_USER_HANDLER_OO(ps_create_sid, crex_string_copy(create_sid_name));
		}
		crex_string_release_ex(create_sid_name, false);

		/* Find implemented methods - SessionUpdateTimestampInterface (optional) */
		/* First release old handlers */
		SESSION_RELEASE_USER_HANDLER_OO(ps_validate_sid);
		SESSION_RELEASE_USER_HANDLER_OO(ps_update_timestamp);
		/* Method names need to be lowercase */
		crex_string *validate_sid_name = ZSTR_INIT_LITERAL("validateid", false);
		crex_string *update_timestamp_name = ZSTR_INIT_LITERAL("updatetimestamp", false);
		if (instanceof_function(C_OBJCE_P(obj), crx_session_update_timestamp_iface_entry)) {
			/* Validate ID handler */
			SESSION_SET_USER_HANDLER_OO(ps_validate_sid, crex_string_copy(validate_sid_name));
			/* Update Timestamp handler */
			SESSION_SET_USER_HANDLER_OO(ps_update_timestamp, crex_string_copy(update_timestamp_name));
		} else {
			/* For BC reasons we accept methods even if the class does not implement the interface */
			if (crex_hash_find_ptr(object_methods, validate_sid_name)) {
				/* For BC reasons we accept methods even if the class does not implement the interface */
				SESSION_SET_USER_HANDLER_OO(ps_validate_sid, crex_string_copy(validate_sid_name));
			}
			if (crex_hash_find_ptr(object_methods, update_timestamp_name)) {
				/* For BC reasons we accept methods even if the class does not implement the interface */
				SESSION_SET_USER_HANDLER_OO(ps_update_timestamp, crex_string_copy(update_timestamp_name));
			}
		}
		crex_string_release_ex(validate_sid_name, false);
		crex_string_release_ex(update_timestamp_name, false);

		if (register_shutdown) {
			/* create shutdown function */
			crx_shutdown_function_entry shutdown_function_entry;
			zval callable;
			crex_result result;

			ZVAL_STRING(&callable, "session_register_shutdown");
			result = crex_fcall_info_init(&callable, 0, &shutdown_function_entry.fci,
				&shutdown_function_entry.fci_cache, NULL, NULL);

			CREX_ASSERT(result == SUCCESS);

			/* add shutdown function, removing the old one if it exists */
			if (!register_user_shutdown_function("session_shutdown", strlen("session_shutdown"), &shutdown_function_entry)) {
				zval_ptr_dtor(&callable);
				crx_error_docref(NULL, E_WARNING, "Unable to register session shutdown function");
				RETURN_FALSE;
			}
		} else {
			/* remove shutdown function */
			remove_user_shutdown_function("session_shutdown", strlen("session_shutdown"));
		}

		if (PS(session_status) != crx_session_active && (!PS(mod) || PS(mod) != &ps_mod_user)) {
			set_user_save_handler_ini();
		}

		RETURN_TRUE;
	}

	/* Procedural version */
	crex_fcall_info open_fci = {0};
	crex_fcall_info_cache open_fcc;
	crex_fcall_info close_fci = {0};
	crex_fcall_info_cache close_fcc;
	crex_fcall_info read_fci = {0};
	crex_fcall_info_cache read_fcc;
	crex_fcall_info write_fci = {0};
	crex_fcall_info_cache write_fcc;
	crex_fcall_info destroy_fci = {0};
	crex_fcall_info_cache destroy_fcc;
	crex_fcall_info gc_fci = {0};
	crex_fcall_info_cache gc_fcc;
	crex_fcall_info create_id_fci = {0};
	crex_fcall_info_cache create_id_fcc;
	crex_fcall_info validate_id_fci = {0};
	crex_fcall_info_cache validate_id_fcc;
	crex_fcall_info update_timestamp_fci = {0};
	crex_fcall_info_cache update_timestamp_fcc;

	if (crex_parse_parameters(CREX_NUM_ARGS(),
		"ffffff|f!f!f!",
		&open_fci, &open_fcc,
		&close_fci, &close_fcc,
		&read_fci, &read_fcc,
		&write_fci, &write_fcc,
		&destroy_fci, &destroy_fcc,
		&gc_fci, &gc_fcc,
		&create_id_fci, &create_id_fcc,
		&validate_id_fci, &validate_id_fcc,
		&update_timestamp_fci, &update_timestamp_fcc) == FAILURE
	) {
		RETURN_THROWS();
	}
	if (!can_session_handler_be_changed()) {
		RETURN_FALSE;
	}

	/* If a custom session handler is already set, release relevant info */
	if (PS(mod_user_class_name)) {
		crex_string_release(PS(mod_user_class_name));
		PS(mod_user_class_name) = NULL;
	}

	/* remove shutdown function */
	remove_user_shutdown_function("session_shutdown", strlen("session_shutdown"));

	if (!PS(mod) || PS(mod) != &ps_mod_user) {
		set_user_save_handler_ini();
	}

	/* Define mandatory handlers */
	SESSION_SET_USER_HANDLER_PROCEDURAL(ps_open, open_fci);
	SESSION_SET_USER_HANDLER_PROCEDURAL(ps_close, close_fci);
	SESSION_SET_USER_HANDLER_PROCEDURAL(ps_read, read_fci);
	SESSION_SET_USER_HANDLER_PROCEDURAL(ps_write, write_fci);
	SESSION_SET_USER_HANDLER_PROCEDURAL(ps_destroy, destroy_fci);
	SESSION_SET_USER_HANDLER_PROCEDURAL(ps_gc, gc_fci);

	/* Check for optional handlers */
	SESSION_SET_USER_HANDLER_PROCEDURAL_OPTIONAL(ps_create_sid, create_id_fci);
	SESSION_SET_USER_HANDLER_PROCEDURAL_OPTIONAL(ps_validate_sid, validate_id_fci);
	SESSION_SET_USER_HANDLER_PROCEDURAL_OPTIONAL(ps_update_timestamp, update_timestamp_fci);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Return the current save path passed to module_name. If newname is given, the save path is replaced with newname */
CRX_FUNCTION(session_save_path)
{
	crex_string *name = NULL;
	crex_string *ini_name;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|P!", &name) == FAILURE) {
		RETURN_THROWS();
	}

	if (name && PS(session_status) == crx_session_active) {
		crx_error_docref(NULL, E_WARNING, "Session save path cannot be changed when a session is active");
		RETURN_FALSE;
	}

	if (name && SG(headers_sent)) {
		crx_error_docref(NULL, E_WARNING, "Session save path cannot be changed after headers have already been sent");
		RETURN_FALSE;
	}

	RETVAL_STRING(PS(save_path));

	if (name) {
		ini_name = ZSTR_INIT_LITERAL("session.save_path", 0);
		crex_alter_ini_entry(ini_name, name, CRX_INI_USER, CRX_INI_STAGE_RUNTIME);
		crex_string_release_ex(ini_name, 0);
	}
}
/* }}} */

/* {{{ Return the current session id. If newid is given, the session id is replaced with newid */
CRX_FUNCTION(session_id)
{
	crex_string *name = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|S!", &name) == FAILURE) {
		RETURN_THROWS();
	}

	if (name && PS(session_status) == crx_session_active) {
		crx_error_docref(NULL, E_WARNING, "Session ID cannot be changed when a session is active");
		RETURN_FALSE;
	}

	if (name && PS(use_cookies) && SG(headers_sent)) {
		crx_error_docref(NULL, E_WARNING, "Session ID cannot be changed after headers have already been sent");
		RETURN_FALSE;
	}

	if (PS(id)) {
		/* keep compatibility for "\0" characters ???
		 * see: ext/session/tests/session_id_error3.crxt */
		size_t len = strlen(ZSTR_VAL(PS(id)));
		if (UNEXPECTED(len != ZSTR_LEN(PS(id)))) {
			RETVAL_NEW_STR(crex_string_init(ZSTR_VAL(PS(id)), len, 0));
		} else {
			RETVAL_STR_COPY(PS(id));
		}
	} else {
		RETVAL_EMPTY_STRING();
	}

	if (name) {
		if (PS(id)) {
			crex_string_release_ex(PS(id), 0);
		}
		PS(id) = crex_string_copy(name);
	}
}
/* }}} */

/* {{{ Update the current session id with a newly generated one. If delete_old_session is set to true, remove the old session. */
CRX_FUNCTION(session_regenerate_id)
{
	bool del_ses = 0;
	crex_string *data;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|b", &del_ses) == FAILURE) {
		RETURN_THROWS();
	}

	if (PS(session_status) != crx_session_active) {
		crx_error_docref(NULL, E_WARNING, "Session ID cannot be regenerated when there is no active session");
		RETURN_FALSE;
	}

	if (SG(headers_sent)) {
		crx_error_docref(NULL, E_WARNING, "Session ID cannot be regenerated after headers have already been sent");
		RETURN_FALSE;
	}

	/* Process old session data */
	if (del_ses) {
		if (PS(mod)->s_destroy(&PS(mod_data), PS(id)) == FAILURE) {
			PS(mod)->s_close(&PS(mod_data));
			PS(session_status) = crx_session_none;
			if (!EG(exception)) {
				crx_error_docref(NULL, E_WARNING, "Session object destruction failed. ID: %s (path: %s)", PS(mod)->s_name, PS(save_path));
			}
			RETURN_FALSE;
		}
	} else {
		crex_result ret;
		data = crx_session_encode();
		if (data) {
			ret = PS(mod)->s_write(&PS(mod_data), PS(id), data, PS(gc_maxlifetime));
			crex_string_release_ex(data, 0);
		} else {
			ret = PS(mod)->s_write(&PS(mod_data), PS(id), ZSTR_EMPTY_ALLOC(), PS(gc_maxlifetime));
		}
		if (ret == FAILURE) {
			PS(mod)->s_close(&PS(mod_data));
			PS(session_status) = crx_session_none;
			crx_error_docref(NULL, E_WARNING, "Session write failed. ID: %s (path: %s)", PS(mod)->s_name, PS(save_path));
			RETURN_FALSE;
		}
	}
	PS(mod)->s_close(&PS(mod_data));

	/* New session data */
	if (PS(session_vars)) {
		crex_string_release_ex(PS(session_vars), 0);
		PS(session_vars) = NULL;
	}
	crex_string_release_ex(PS(id), 0);
	PS(id) = NULL;

	if (PS(mod)->s_open(&PS(mod_data), PS(save_path), PS(session_name)) == FAILURE) {
		PS(session_status) = crx_session_none;
		if (!EG(exception)) {
			crex_throw_error(NULL, "Failed to open session: %s (path: %s)", PS(mod)->s_name, PS(save_path));
		}
		RETURN_THROWS();
	}

	PS(id) = PS(mod)->s_create_sid(&PS(mod_data));
	if (!PS(id)) {
		PS(session_status) = crx_session_none;
		if (!EG(exception)) {
			crex_throw_error(NULL, "Failed to create new session ID: %s (path: %s)", PS(mod)->s_name, PS(save_path));
		}
		RETURN_THROWS();
	}
	if (PS(use_strict_mode)) {
		if ((!PS(mod_user_implemented) && PS(mod)->s_validate_sid) || !C_ISUNDEF(PS(mod_user_names).ps_validate_sid)) {
			int limit = 3;
			/* Try to generate non-existing ID */
			while (limit-- && PS(mod)->s_validate_sid(&PS(mod_data), PS(id)) == SUCCESS) {
				crex_string_release_ex(PS(id), 0);
				PS(id) = PS(mod)->s_create_sid(&PS(mod_data));
				if (!PS(id)) {
					PS(mod)->s_close(&PS(mod_data));
					PS(session_status) = crx_session_none;
					if (!EG(exception)) {
						crex_throw_error(NULL, "Failed to create session ID by collision: %s (path: %s)", PS(mod)->s_name, PS(save_path));
					}
					RETURN_THROWS();
				}
			}
		}
		// TODO warn that ID cannot be verified? else { }
	}
	/* Read is required to make new session data at this point. */
	if (PS(mod)->s_read(&PS(mod_data), PS(id), &data, PS(gc_maxlifetime)) == FAILURE) {
		PS(mod)->s_close(&PS(mod_data));
		PS(session_status) = crx_session_none;
		if (!EG(exception)) {
			crex_throw_error(NULL, "Failed to create(read) session ID: %s (path: %s)", PS(mod)->s_name, PS(save_path));
		}
		RETURN_THROWS();
	}
	if (data) {
		crex_string_release_ex(data, 0);
	}

	if (PS(use_cookies)) {
		PS(send_cookie) = 1;
	}
	if (crx_session_reset_id() == FAILURE) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Generate new session ID. Intended for user save handlers. */
CRX_FUNCTION(session_create_id)
{
	crex_string *prefix = NULL, *new_id;
	smart_str id = {0};

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|S", &prefix) == FAILURE) {
		RETURN_THROWS();
	}

	if (prefix && ZSTR_LEN(prefix)) {
		if (crx_session_valid_key(ZSTR_VAL(prefix)) == FAILURE) {
			/* E_ERROR raised for security reason. */
			crx_error_docref(NULL, E_WARNING, "Prefix cannot contain special characters. Only the A-Z, a-z, 0-9, \"-\", and \",\" characters are allowed");
			RETURN_FALSE;
		} else {
			smart_str_append(&id, prefix);
		}
	}

	if (!PS(in_save_handler) && PS(session_status) == crx_session_active) {
		int limit = 3;
		while (limit--) {
			new_id = PS(mod)->s_create_sid(&PS(mod_data));
			if (!PS(mod)->s_validate_sid || (PS(mod_user_implemented) && C_ISUNDEF(PS(mod_user_names).ps_validate_sid))) {
				break;
			} else {
				/* Detect collision and retry */
				if (PS(mod)->s_validate_sid(&PS(mod_data), new_id) == SUCCESS) {
					crex_string_release_ex(new_id, 0);
					new_id = NULL;
					continue;
				}
				break;
			}
		}
	} else {
		new_id = crx_session_create_id(NULL);
	}

	if (new_id) {
		smart_str_append(&id, new_id);
		crex_string_release_ex(new_id, 0);
	} else {
		smart_str_free(&id);
		crx_error_docref(NULL, E_WARNING, "Failed to create new ID");
		RETURN_FALSE;
	}
	RETVAL_STR(smart_str_extract(&id));
}
/* }}} */

/* {{{ Return the current cache limiter. If new_cache_limited is given, the current cache_limiter is replaced with new_cache_limiter */
CRX_FUNCTION(session_cache_limiter)
{
	crex_string *limiter = NULL;
	crex_string *ini_name;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|S!", &limiter) == FAILURE) {
		RETURN_THROWS();
	}

	if (limiter && PS(session_status) == crx_session_active) {
		crx_error_docref(NULL, E_WARNING, "Session cache limiter cannot be changed when a session is active");
		RETURN_FALSE;
	}

	if (limiter && SG(headers_sent)) {
		crx_error_docref(NULL, E_WARNING, "Session cache limiter cannot be changed after headers have already been sent");
		RETURN_FALSE;
	}

	RETVAL_STRING(PS(cache_limiter));

	if (limiter) {
		ini_name = ZSTR_INIT_LITERAL("session.cache_limiter", 0);
		crex_alter_ini_entry(ini_name, limiter, CRX_INI_USER, CRX_INI_STAGE_RUNTIME);
		crex_string_release_ex(ini_name, 0);
	}
}
/* }}} */

/* {{{ Return the current cache expire. If new_cache_expire is given, the current cache_expire is replaced with new_cache_expire */
CRX_FUNCTION(session_cache_expire)
{
	crex_long expires;
	bool expires_is_null = 1;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|l!", &expires, &expires_is_null) == FAILURE) {
		RETURN_THROWS();
	}

	if (!expires_is_null && PS(session_status) == crx_session_active) {
		crx_error_docref(NULL, E_WARNING, "Session cache expiration cannot be changed when a session is active");
		RETURN_LONG(PS(cache_expire));
	}

	if (!expires_is_null && SG(headers_sent)) {
		crx_error_docref(NULL, E_WARNING, "Session cache expiration cannot be changed after headers have already been sent");
		RETURN_FALSE;
	}

	RETVAL_LONG(PS(cache_expire));

	if (!expires_is_null) {
		crex_string *ini_name = ZSTR_INIT_LITERAL("session.cache_expire", 0);
		crex_string *ini_value = crex_long_to_str(expires);
		crex_alter_ini_entry(ini_name, ini_value, CREX_INI_USER, CREX_INI_STAGE_RUNTIME);
		crex_string_release_ex(ini_name, 0);
		crex_string_release_ex(ini_value, 0);
	}
}
/* }}} */

/* {{{ Serializes the current setup and returns the serialized representation */
CRX_FUNCTION(session_encode)
{
	crex_string *enc;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	enc = crx_session_encode();
	if (enc == NULL) {
		RETURN_FALSE;
	}

	RETURN_STR(enc);
}
/* }}} */

/* {{{ Deserializes data and reinitializes the variables */
CRX_FUNCTION(session_decode)
{
	crex_string *str = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S", &str) == FAILURE) {
		RETURN_THROWS();
	}

	if (PS(session_status) != crx_session_active) {
		crx_error_docref(NULL, E_WARNING, "Session data cannot be decoded when there is no active session");
		RETURN_FALSE;
	}

	if (crx_session_decode(str) == FAILURE) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

static crex_result crx_session_start_set_ini(crex_string *varname, crex_string *new_value) {
	crex_result ret;
	smart_str buf ={0};
	smart_str_appends(&buf, "session");
	smart_str_appendc(&buf, '.');
	smart_str_append(&buf, varname);
	smart_str_0(&buf);
	ret = crex_alter_ini_entry_ex(buf.s, new_value, CRX_INI_USER, CRX_INI_STAGE_RUNTIME, 0);
	smart_str_free(&buf);
	return ret;
}

/* {{{ Begin session */
CRX_FUNCTION(session_start)
{
	zval *options = NULL;
	zval *value;
	crex_ulong num_idx;
	crex_string *str_idx;
	crex_long read_and_close = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|a", &options) == FAILURE) {
		RETURN_THROWS();
	}

	if (PS(session_status) == crx_session_active) {
		if (PS(session_started_filename)) {
			crx_error_docref(NULL, E_NOTICE, "Ignoring session_start() because a session is already active (started from %s on line %"PRIu32")", ZSTR_VAL(PS(session_started_filename)), PS(session_started_lineno));
		} else if (PS(auto_start)) {
			/* This option can't be changed at runtime, so we can assume it's because of this */
			crx_error_docref(NULL, E_NOTICE, "Ignoring session_start() because a session is already automatically active");
		} else {
			crx_error_docref(NULL, E_NOTICE, "Ignoring session_start() because a session is already active");
		}
		RETURN_TRUE;
	}

	/*
	 * TODO: To prevent unusable session with trans sid, actual output started status is
	 * required. i.e. There shouldn't be any outputs in output buffer, otherwise session
	 * module is unable to rewrite output.
	 */
	if (PS(use_cookies) && SG(headers_sent)) {
		crx_error_docref(NULL, E_WARNING, "Session cannot be started after headers have already been sent");
		RETURN_FALSE;
	}

	/* set options */
	if (options) {
		CREX_HASH_FOREACH_KEY_VAL(C_ARRVAL_P(options), num_idx, str_idx, value) {
			if (str_idx) {
				switch(C_TYPE_P(value)) {
					case IS_STRING:
					case IS_TRUE:
					case IS_FALSE:
					case IS_LONG:
						if (crex_string_equals_literal(str_idx, "read_and_close")) {
							read_and_close = zval_get_long(value);
						} else {
							crex_string *tmp_val;
							crex_string *val = zval_get_tmp_string(value, &tmp_val);
							if (crx_session_start_set_ini(str_idx, val) == FAILURE) {
								crx_error_docref(NULL, E_WARNING, "Setting option \"%s\" failed", ZSTR_VAL(str_idx));
							}
							crex_tmp_string_release(tmp_val);
						}
						break;
					default:
						crex_type_error("%s(): Option \"%s\" must be of type string|int|bool, %s given",
							get_active_function_name(), ZSTR_VAL(str_idx), crex_zval_value_name(value)
						);
						RETURN_THROWS();
				}
			}
			(void) num_idx;
		} CREX_HASH_FOREACH_END();
	}

	crx_session_start();

	if (PS(session_status) != crx_session_active) {
		IF_SESSION_VARS() {
			zval *sess_var = C_REFVAL(PS(http_session_vars));
			SEPARATE_ARRAY(sess_var);
			/* Clean $_SESSION. */
			crex_hash_clean(C_ARRVAL_P(sess_var));
		}
		RETURN_FALSE;
	}

	if (read_and_close) {
		crx_session_flush(0);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Destroy the current session and all data associated with it */
CRX_FUNCTION(session_destroy)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	RETURN_BOOL(crx_session_destroy() == SUCCESS);
}
/* }}} */

/* {{{ Unset all registered variables */
CRX_FUNCTION(session_unset)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (PS(session_status) != crx_session_active) {
		RETURN_FALSE;
	}

	IF_SESSION_VARS() {
		zval *sess_var = C_REFVAL(PS(http_session_vars));
		SEPARATE_ARRAY(sess_var);

		/* Clean $_SESSION. */
		crex_hash_clean(C_ARRVAL_P(sess_var));
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Perform GC and return number of deleted sessions */
CRX_FUNCTION(session_gc)
{
	crex_long num;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (PS(session_status) != crx_session_active) {
		crx_error_docref(NULL, E_WARNING, "Session cannot be garbage collected when there is no active session");
		RETURN_FALSE;
	}

	num = crx_session_gc(1);
	if (num < 0) {
		RETURN_FALSE;
	}

	RETURN_LONG(num);
}
/* }}} */


/* {{{ Write session data and end session */
CRX_FUNCTION(session_write_close)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (PS(session_status) != crx_session_active) {
		RETURN_FALSE;
	}
	crx_session_flush(1);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Abort session and end session. Session data will not be written */
CRX_FUNCTION(session_abort)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (PS(session_status) != crx_session_active) {
		RETURN_FALSE;
	}
	crx_session_abort();
	RETURN_TRUE;
}
/* }}} */

/* {{{ Reset session data from saved session data */
CRX_FUNCTION(session_reset)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	if (PS(session_status) != crx_session_active) {
		RETURN_FALSE;
	}
	crx_session_reset();
	RETURN_TRUE;
}
/* }}} */

/* {{{ Returns the current session status */
CRX_FUNCTION(session_status)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	RETURN_LONG(PS(session_status));
}
/* }}} */

/* {{{ Registers session_write_close() as a shutdown function */
CRX_FUNCTION(session_register_shutdown)
{
	crx_shutdown_function_entry shutdown_function_entry;
	zval callable;
	crex_result result;

	CREX_PARSE_PARAMETERS_NONE();

	/* This function is registered itself as a shutdown function by
	 * session_set_save_handler($obj). The reason we now register another
	 * shutdown function is in case the user registered their own shutdown
	 * function after calling session_set_save_handler(), which expects
	 * the session still to be available.
	 */
	ZVAL_STRING(&callable, "session_write_close");
	result = crex_fcall_info_init(&callable, 0, &shutdown_function_entry.fci,
		&shutdown_function_entry.fci_cache, NULL, NULL);

	CREX_ASSERT(result == SUCCESS);

	if (!append_user_shutdown_function(&shutdown_function_entry)) {
		zval_ptr_dtor(&callable);

		/* Unable to register shutdown function, presumably because of lack
		 * of memory, so flush the session now. It would be done in rshutdown
		 * anyway but the handler will have had it's dtor called by then.
		 * If the user does have a later shutdown function which needs the
		 * session then tough luck.
		 */
		crx_session_flush(1);
		crx_error_docref(NULL, E_WARNING, "Session shutdown function cannot be registered");
	}
}
/* }}} */

/* ********************************
   * Module Setup and Destruction *
   ******************************** */

static crex_result crx_rinit_session(bool auto_start) /* {{{ */
{
	crx_rinit_session_globals();

	PS(mod) = NULL;
	{
		char *value;

		value = crex_ini_string("session.save_handler", sizeof("session.save_handler") - 1, 0);
		if (value) {
			PS(mod) = _crx_find_ps_module(value);
		}
	}

	if (PS(serializer) == NULL) {
		char *value;

		value = crex_ini_string("session.serialize_handler", sizeof("session.serialize_handler") - 1, 0);
		if (value) {
			PS(serializer) = _crx_find_ps_serializer(value);
		}
	}

	if (PS(mod) == NULL || PS(serializer) == NULL) {
		/* current status is unusable */
		PS(session_status) = crx_session_disabled;
		return SUCCESS;
	}

	if (auto_start) {
		crx_session_start();
	}

	return SUCCESS;
} /* }}} */

static CRX_RINIT_FUNCTION(session) /* {{{ */
{
	return crx_rinit_session(PS(auto_start));
}
/* }}} */

#define SESSION_FREE_USER_HANDLER(struct_name) \
	if (!C_ISUNDEF(PS(mod_user_names).struct_name)) { \
		zval_ptr_dtor(&PS(mod_user_names).struct_name); \
		ZVAL_UNDEF(&PS(mod_user_names).struct_name); \
	}


static CRX_RSHUTDOWN_FUNCTION(session) /* {{{ */
{
	if (PS(session_status) == crx_session_active) {
		crex_try {
			crx_session_flush(1);
		} crex_end_try();
	}
	crx_rshutdown_session_globals();

	/* this should NOT be done in crx_rshutdown_session_globals() */
	/* Free user defined handlers */
	SESSION_FREE_USER_HANDLER(ps_open);
	SESSION_FREE_USER_HANDLER(ps_close);
	SESSION_FREE_USER_HANDLER(ps_read);
	SESSION_FREE_USER_HANDLER(ps_write);
	SESSION_FREE_USER_HANDLER(ps_destroy);
	SESSION_FREE_USER_HANDLER(ps_gc);
	SESSION_FREE_USER_HANDLER(ps_create_sid);
	SESSION_FREE_USER_HANDLER(ps_validate_sid);
	SESSION_FREE_USER_HANDLER(ps_update_timestamp);

	return SUCCESS;
}
/* }}} */

static CRX_GINIT_FUNCTION(ps) /* {{{ */
{
#if defined(COMPILE_DL_SESSION) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif

	ps_globals->save_path = NULL;
	ps_globals->session_name = NULL;
	ps_globals->id = NULL;
	ps_globals->mod = NULL;
	ps_globals->serializer = NULL;
	ps_globals->mod_data = NULL;
	ps_globals->session_status = crx_session_none;
	ps_globals->default_mod = NULL;
	ps_globals->mod_user_implemented = 0;
	ps_globals->mod_user_class_name = NULL;
	ps_globals->mod_user_is_open = 0;
	ps_globals->session_vars = NULL;
	ps_globals->set_handler = 0;
	ps_globals->session_started_filename = NULL;
	ps_globals->session_started_lineno = 0;
	/* Unset user defined handlers */
	ZVAL_UNDEF(&ps_globals->mod_user_names.ps_open);
	ZVAL_UNDEF(&ps_globals->mod_user_names.ps_close);
	ZVAL_UNDEF(&ps_globals->mod_user_names.ps_read);
	ZVAL_UNDEF(&ps_globals->mod_user_names.ps_write);
	ZVAL_UNDEF(&ps_globals->mod_user_names.ps_destroy);
	ZVAL_UNDEF(&ps_globals->mod_user_names.ps_gc);
	ZVAL_UNDEF(&ps_globals->mod_user_names.ps_create_sid);
	ZVAL_UNDEF(&ps_globals->mod_user_names.ps_validate_sid);
	ZVAL_UNDEF(&ps_globals->mod_user_names.ps_update_timestamp);
	ZVAL_UNDEF(&ps_globals->http_session_vars);
}
/* }}} */

static CRX_MINIT_FUNCTION(session) /* {{{ */
{
	crex_register_auto_global(crex_string_init_interned("_SESSION", sizeof("_SESSION") - 1, 1), 0, NULL);

	my_module_number = module_number;
	PS(module_number) = module_number;

	PS(session_status) = crx_session_none;
	REGISTER_INI_ENTRIES();

#ifdef HAVE_LIBMM
	CRX_MINIT(ps_mm) (INIT_FUNC_ARGS_PASSTHRU);
#endif
	crx_session_rfc1867_orig_callback = crx_rfc1867_callback;
	crx_rfc1867_callback = crx_session_rfc1867_callback;

	/* Register interfaces */
	crx_session_iface_entry = register_class_SessionHandlerInterface();

	crx_session_id_iface_entry = register_class_SessionIdInterface();

	crx_session_update_timestamp_iface_entry = register_class_SessionUpdateTimestampHandlerInterface();

	/* Register base class */
	crx_session_class_entry = register_class_SessionHandler(crx_session_iface_entry, crx_session_id_iface_entry);

	register_session_symbols(module_number);

	return SUCCESS;
}
/* }}} */

static CRX_MSHUTDOWN_FUNCTION(session) /* {{{ */
{
	UNREGISTER_INI_ENTRIES();

#ifdef HAVE_LIBMM
	CRX_MSHUTDOWN(ps_mm) (SHUTDOWN_FUNC_ARGS_PASSTHRU);
#endif

	/* reset rfc1867 callbacks */
	crx_session_rfc1867_orig_callback = NULL;
	if (crx_rfc1867_callback == crx_session_rfc1867_callback) {
		crx_rfc1867_callback = NULL;
	}

	ps_serializers[PREDEFINED_SERIALIZERS].name = NULL;
	memset(CREX_VOIDP(&ps_modules[PREDEFINED_MODULES]), 0, (MAX_MODULES-PREDEFINED_MODULES)*sizeof(ps_module *));

	return SUCCESS;
}
/* }}} */

static CRX_MINFO_FUNCTION(session) /* {{{ */
{
	const ps_module **mod;
	ps_serializer *ser;
	smart_str save_handlers = {0};
	smart_str ser_handlers = {0};
	int i;

	/* Get save handlers */
	for (i = 0, mod = ps_modules; i < MAX_MODULES; i++, mod++) {
		if (*mod && (*mod)->s_name) {
			smart_str_appends(&save_handlers, (*mod)->s_name);
			smart_str_appendc(&save_handlers, ' ');
		}
	}

	/* Get serializer handlers */
	for (i = 0, ser = ps_serializers; i < MAX_SERIALIZERS; i++, ser++) {
		if (ser->name) {
			smart_str_appends(&ser_handlers, ser->name);
			smart_str_appendc(&ser_handlers, ' ');
		}
	}

	crx_info_print_table_start();
	crx_info_print_table_row(2, "Session Support", "enabled" );

	if (save_handlers.s) {
		smart_str_0(&save_handlers);
		crx_info_print_table_row(2, "Registered save handlers", ZSTR_VAL(save_handlers.s));
		smart_str_free(&save_handlers);
	} else {
		crx_info_print_table_row(2, "Registered save handlers", "none");
	}

	if (ser_handlers.s) {
		smart_str_0(&ser_handlers);
		crx_info_print_table_row(2, "Registered serializer handlers", ZSTR_VAL(ser_handlers.s));
		smart_str_free(&ser_handlers);
	} else {
		crx_info_print_table_row(2, "Registered serializer handlers", "none");
	}

	crx_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

static const crex_module_dep session_deps[] = { /* {{{ */
	CREX_MOD_OPTIONAL("hash")
	CREX_MOD_REQUIRED("spl")
	CREX_MOD_END
};
/* }}} */

/* ************************
   * Upload hook handling *
   ************************ */

static bool early_find_sid_in(zval *dest, int where, crx_session_rfc1867_progress *progress) /* {{{ */
{
	zval *ppid;

	if (C_ISUNDEF(PG(http_globals)[where])) {
		return 0;
	}

	if ((ppid = crex_hash_str_find(C_ARRVAL(PG(http_globals)[where]), PS(session_name), progress->sname_len))
			&& C_TYPE_P(ppid) == IS_STRING) {
		zval_ptr_dtor(dest);
		ZVAL_COPY_DEREF(dest, ppid);
		return 1;
	}

	return 0;
} /* }}} */

static void crx_session_rfc1867_early_find_sid(crx_session_rfc1867_progress *progress) /* {{{ */
{

	if (PS(use_cookies)) {
		sapi_module.treat_data(PARSE_COOKIE, NULL, NULL);
		if (early_find_sid_in(&progress->sid, TRACK_VARS_COOKIE, progress)) {
			progress->apply_trans_sid = 0;
			return;
		}
	}
	if (PS(use_only_cookies)) {
		return;
	}
	sapi_module.treat_data(PARSE_GET, NULL, NULL);
	early_find_sid_in(&progress->sid, TRACK_VARS_GET, progress);
} /* }}} */

static bool crx_check_cancel_upload(crx_session_rfc1867_progress *progress) /* {{{ */
{
	zval *progress_ary, *cancel_upload;

	if ((progress_ary = crex_symtable_find(C_ARRVAL_P(C_REFVAL(PS(http_session_vars))), progress->key.s)) == NULL) {
		return 0;
	}
	if (C_TYPE_P(progress_ary) != IS_ARRAY) {
		return 0;
	}
	if ((cancel_upload = crex_hash_str_find(C_ARRVAL_P(progress_ary), "cancel_upload", sizeof("cancel_upload") - 1)) == NULL) {
		return 0;
	}
	return C_TYPE_P(cancel_upload) == IS_TRUE;
} /* }}} */

static void crx_session_rfc1867_update(crx_session_rfc1867_progress *progress, int force_update) /* {{{ */
{
	if (!force_update) {
		if (C_LVAL_P(progress->post_bytes_processed) < progress->next_update) {
			return;
		}
#ifdef HAVE_GETTIMEOFDAY
		if (PS(rfc1867_min_freq) > 0.0) {
			struct timeval tv = {0};
			double dtv;
			gettimeofday(&tv, NULL);
			dtv = (double) tv.tv_sec + tv.tv_usec / 1000000.0;
			if (dtv < progress->next_update_time) {
				return;
			}
			progress->next_update_time = dtv + PS(rfc1867_min_freq);
		}
#endif
		progress->next_update = C_LVAL_P(progress->post_bytes_processed) + progress->update_step;
	}

	crx_session_initialize();
	PS(session_status) = crx_session_active;
	IF_SESSION_VARS() {
		zval *sess_var = C_REFVAL(PS(http_session_vars));
		SEPARATE_ARRAY(sess_var);

		progress->cancel_upload |= crx_check_cancel_upload(progress);
		C_TRY_ADDREF(progress->data);
		crex_hash_update(C_ARRVAL_P(sess_var), progress->key.s, &progress->data);
	}
	crx_session_flush(1);
} /* }}} */

static void crx_session_rfc1867_cleanup(crx_session_rfc1867_progress *progress) /* {{{ */
{
	crx_session_initialize();
	PS(session_status) = crx_session_active;
	IF_SESSION_VARS() {
		zval *sess_var = C_REFVAL(PS(http_session_vars));
		SEPARATE_ARRAY(sess_var);
		crex_hash_del(C_ARRVAL_P(sess_var), progress->key.s);
	}
	crx_session_flush(1);
} /* }}} */

static crex_result crx_session_rfc1867_callback(unsigned int event, void *event_data, void **extra) /* {{{ */
{
	crx_session_rfc1867_progress *progress;
	crex_result retval = SUCCESS;

	if (crx_session_rfc1867_orig_callback) {
		retval = crx_session_rfc1867_orig_callback(event, event_data, extra);
	}
	if (!PS(rfc1867_enabled)) {
		return retval;
	}

	progress = PS(rfc1867_progress);

	switch(event) {
		case MULTIPART_EVENT_START: {
			multipart_event_start *data = (multipart_event_start *) event_data;
			progress = ecalloc(1, sizeof(crx_session_rfc1867_progress));
			progress->content_length = data->content_length;
			progress->sname_len  = strlen(PS(session_name));
			PS(rfc1867_progress) = progress;
		}
		break;
		case MULTIPART_EVENT_FORMDATA: {
			multipart_event_formdata *data = (multipart_event_formdata *) event_data;
			size_t value_len;

			if (C_TYPE(progress->sid) && progress->key.s) {
				break;
			}

			/* orig callback may have modified *data->newlength */
			if (data->newlength) {
				value_len = *data->newlength;
			} else {
				value_len = data->length;
			}

			if (data->name && data->value && value_len) {
				size_t name_len = strlen(data->name);

				if (name_len == progress->sname_len && memcmp(data->name, PS(session_name), name_len) == 0) {
					zval_ptr_dtor(&progress->sid);
					ZVAL_STRINGL(&progress->sid, (*data->value), value_len);
				} else if (name_len == strlen(PS(rfc1867_name)) && memcmp(data->name, PS(rfc1867_name), name_len + 1) == 0) {
					smart_str_free(&progress->key);
					smart_str_appends(&progress->key, PS(rfc1867_prefix));
					smart_str_appendl(&progress->key, *data->value, value_len);
					smart_str_0(&progress->key);

					progress->apply_trans_sid = APPLY_TRANS_SID;
					crx_session_rfc1867_early_find_sid(progress);
				}
			}
		}
		break;
		case MULTIPART_EVENT_FILE_START: {
			multipart_event_file_start *data = (multipart_event_file_start *) event_data;

			/* Do nothing when $_POST["CRX_SESSION_UPLOAD_PROGRESS"] is not set
			 * or when we have no session id */
			if (!C_TYPE(progress->sid) || !progress->key.s) {
				break;
			}

			/* First FILE_START event, initializing data */
			if (C_ISUNDEF(progress->data)) {

				if (PS(rfc1867_freq) >= 0) {
					progress->update_step = PS(rfc1867_freq);
				} else if (PS(rfc1867_freq) < 0) { /* % of total size */
					progress->update_step = progress->content_length * -PS(rfc1867_freq) / 100;
				}
				progress->next_update = 0;
				progress->next_update_time = 0.0;

				array_init(&progress->data);
				array_init(&progress->files);

				add_assoc_long_ex(&progress->data, "start_time", sizeof("start_time") - 1, (crex_long)sapi_get_request_time());
				add_assoc_long_ex(&progress->data, "content_length",  sizeof("content_length") - 1, progress->content_length);
				add_assoc_long_ex(&progress->data, "bytes_processed", sizeof("bytes_processed") - 1, data->post_bytes_processed);
				add_assoc_bool_ex(&progress->data, "done", sizeof("done") - 1, 0);
				add_assoc_zval_ex(&progress->data, "files", sizeof("files") - 1, &progress->files);

				progress->post_bytes_processed = crex_hash_str_find(C_ARRVAL(progress->data), "bytes_processed", sizeof("bytes_processed") - 1);

				crx_rinit_session(0);
				PS(id) = crex_string_init(C_STRVAL(progress->sid), C_STRLEN(progress->sid), 0);
				if (progress->apply_trans_sid) {
					/* Enable trans sid by modifying flags */
					PS(use_trans_sid) = 1;
					PS(use_only_cookies) = 0;
				}
				PS(send_cookie) = 0;
			}

			array_init(&progress->current_file);

			/* Each uploaded file has its own array. Trying to make it close to $_FILES entries. */
			add_assoc_string_ex(&progress->current_file, "field_name", sizeof("field_name") - 1, data->name);
			add_assoc_string_ex(&progress->current_file, "name", sizeof("name") - 1, *data->filename);
			add_assoc_null_ex(&progress->current_file, "tmp_name", sizeof("tmp_name") - 1);
			add_assoc_long_ex(&progress->current_file, "error", sizeof("error") - 1, 0);

			add_assoc_bool_ex(&progress->current_file, "done", sizeof("done") - 1, 0);
			add_assoc_long_ex(&progress->current_file, "start_time", sizeof("start_time") - 1, (crex_long)time(NULL));
			add_assoc_long_ex(&progress->current_file, "bytes_processed", sizeof("bytes_processed") - 1, 0);

			add_next_index_zval(&progress->files, &progress->current_file);

			progress->current_file_bytes_processed = crex_hash_str_find(C_ARRVAL(progress->current_file), "bytes_processed", sizeof("bytes_processed") - 1);

			C_LVAL_P(progress->current_file_bytes_processed) =  data->post_bytes_processed;
			crx_session_rfc1867_update(progress, 0);
		}
		break;
		case MULTIPART_EVENT_FILE_DATA: {
			multipart_event_file_data *data = (multipart_event_file_data *) event_data;

			if (!C_TYPE(progress->sid) || !progress->key.s) {
				break;
			}

			C_LVAL_P(progress->current_file_bytes_processed) = data->offset + data->length;
			C_LVAL_P(progress->post_bytes_processed) = data->post_bytes_processed;

			crx_session_rfc1867_update(progress, 0);
		}
		break;
		case MULTIPART_EVENT_FILE_END: {
			multipart_event_file_end *data = (multipart_event_file_end *) event_data;

			if (!C_TYPE(progress->sid) || !progress->key.s) {
				break;
			}

			if (data->temp_filename) {
				add_assoc_string_ex(&progress->current_file, "tmp_name",  sizeof("tmp_name") - 1, data->temp_filename);
			}

			add_assoc_long_ex(&progress->current_file, "error", sizeof("error") - 1, data->cancel_upload);
			add_assoc_bool_ex(&progress->current_file, "done", sizeof("done") - 1,  1);

			C_LVAL_P(progress->post_bytes_processed) = data->post_bytes_processed;

			crx_session_rfc1867_update(progress, 0);
		}
		break;
		case MULTIPART_EVENT_END: {
			multipart_event_end *data = (multipart_event_end *) event_data;

			if (C_TYPE(progress->sid) && progress->key.s) {
				if (PS(rfc1867_cleanup)) {
					crx_session_rfc1867_cleanup(progress);
				} else {
					if (!C_ISUNDEF(progress->data)) {
						SEPARATE_ARRAY(&progress->data);
						add_assoc_bool_ex(&progress->data, "done", sizeof("done") - 1, 1);
						C_LVAL_P(progress->post_bytes_processed) = data->post_bytes_processed;
						crx_session_rfc1867_update(progress, 1);
					}
				}
				crx_rshutdown_session_globals();
			}

			if (!C_ISUNDEF(progress->data)) {
				zval_ptr_dtor(&progress->data);
			}
			zval_ptr_dtor(&progress->sid);
			smart_str_free(&progress->key);
			efree(progress);
			progress = NULL;
			PS(rfc1867_progress) = NULL;
		}
		break;
	}

	if (progress && progress->cancel_upload) {
		return FAILURE;
	}
	return retval;

} /* }}} */

crex_module_entry session_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	session_deps,
	"session",
	ext_functions,
	CRX_MINIT(session), CRX_MSHUTDOWN(session),
	CRX_RINIT(session), CRX_RSHUTDOWN(session),
	CRX_MINFO(session),
	CRX_SESSION_VERSION,
	CRX_MODULE_GLOBALS(ps),
	CRX_GINIT(ps),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_SESSION
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(session)
#endif
