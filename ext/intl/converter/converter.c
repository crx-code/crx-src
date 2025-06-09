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
   | Authors: Sara Golemon <pollita@crx.net>                              |
   +----------------------------------------------------------------------+
 */

#include "converter.h"
#include "crex_exceptions.h"

#include <unicode/utypes.h>
#include <unicode/utf8.h>
#include <unicode/utf16.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>

#include "../intl_error.h"
#include "../intl_common.h"
#include "converter_arginfo.h"

typedef struct _crx_converter_object {
	UConverter *src, *dest;
	crex_fcall_info to_cb, from_cb;
	crex_fcall_info_cache to_cache, from_cache;
	intl_error error;
	crex_object obj;
} crx_converter_object;


static inline crx_converter_object *crx_converter_fetch_object(crex_object *obj) {
	return (crx_converter_object *)((char*)(obj) - XtOffsetOf(crx_converter_object, obj));
}
#define C_INTL_CONVERTER_P(zv) crx_converter_fetch_object(C_OBJ_P(zv))

static crex_class_entry     *crx_converter_ce;
static crex_object_handlers  crx_converter_object_handlers;

#define CONV_GET(pzv)  (C_INTL_CONVERTER_P((pzv)))
#define THROW_UFAILURE(obj, fname, error) crx_converter_throw_failure(obj, error, \
                                          fname "() returned error " CREX_LONG_FMT ": %s", (crex_long)error, u_errorName(error))

/* {{{ crx_converter_throw_failure */
static inline void crx_converter_throw_failure(crx_converter_object *objval, UErrorCode error, const char *format, ...) {
	intl_error *err = objval ? &(objval->error) : NULL;
	char message[1024];
	va_list vargs;

	va_start(vargs, format);
	vsnprintf(message, sizeof(message), format, vargs);
	va_end(vargs);

	intl_errors_set(err, error, message, 1);
}
/* }}} */

/* {{{ crx_converter_default_callback */
static void crx_converter_default_callback(zval *return_value, zval *zobj, crex_long reason, zval *error) {
	/* Basic functionality so children can call parent::toUCallback() */
	switch (reason) {
		case UCNV_UNASSIGNED:
		case UCNV_ILLEGAL:
		case UCNV_IRREGULAR:
		{
			crx_converter_object *objval = (crx_converter_object*)CONV_GET(zobj);
			char chars[127];
			int8_t chars_len = sizeof(chars);
			UErrorCode uerror = U_ZERO_ERROR;
			if(!objval->src) {
				crx_converter_throw_failure(objval, U_INVALID_STATE_ERROR, "Source Converter has not been initialized yet");
				chars[0] = 0x1A;
				chars[1] = 0;
				chars_len = 1;
				CREX_TRY_ASSIGN_REF_LONG(error, U_INVALID_STATE_ERROR);
				RETVAL_STRINGL(chars, chars_len);
				return;
			}

			/* Yes, this is fairly wasteful at first glance,
			 * but considering that the alternative is to store
			 * what's sent into setSubstChars() and the fact
			 * that this is an extremely unlikely codepath
			 * I'd rather take the CPU hit here, than waste time
			 * storing a value I'm unlikely to use.
			 */
			ucnv_getSubstChars(objval->src, chars, &chars_len, &uerror);
			if (U_FAILURE(uerror)) {
				THROW_UFAILURE(objval, "ucnv_getSubstChars", uerror);
				chars[0] = 0x1A;
				chars[1] = 0;
				chars_len = 1;
			}
			CREX_TRY_ASSIGN_REF_LONG(error, uerror);
			RETVAL_STRINGL(chars, chars_len);
		}
	}
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, toUCallback) {
	crex_long reason;
	crex_string *source, *codeUnits;
	zval *error;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "lSSz",
		&reason, &source, &codeUnits, &error) == FAILURE) {
		RETURN_THROWS();
	}

	crx_converter_default_callback(return_value, CREX_THIS, reason, error);
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, fromUCallback) {
	crex_long reason;
	zval *source, *error;
	crex_long codePoint;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "lalz",
		&reason, &source, &codePoint, &error) == FAILURE) {
		RETURN_THROWS();
	}

	crx_converter_default_callback(return_value, CREX_THIS, reason, error);
}
/* }}} */

/* {{{ crx_converter_check_limits */
static inline bool crx_converter_check_limits(crx_converter_object *objval, crex_long available, crex_long needed) {
	if (available < needed) {
		crx_converter_throw_failure(objval, U_BUFFER_OVERFLOW_ERROR, "Buffer overrun " CREX_LONG_FMT " bytes needed, " CREX_LONG_FMT " available", needed, available);
		return 0;
	}
	return 1;
}
/* }}} */

#define TARGET_CHECK(cnvargs, needed) crx_converter_check_limits(objval, cnvargs->targetLimit - cnvargs->target, needed)

/* {{{ crx_converter_append_toUnicode_target */
static void crx_converter_append_toUnicode_target(zval *val, UConverterToUnicodeArgs *args, crx_converter_object *objval) {
	switch (C_TYPE_P(val)) {
		case IS_NULL:
			/* Code unit is being skipped */
			return;
		case IS_LONG:
		{
			crex_long lval = C_LVAL_P(val);
			if ((lval < 0) || (lval > 0x10FFFF)) {
				crx_converter_throw_failure(objval, U_ILLEGAL_ARGUMENT_ERROR, "Invalid codepoint U+%04lx", lval);
				return;
			}
			if (lval > 0xFFFF) {
				/* Supplemental planes U+010000 - U+10FFFF */
				if (TARGET_CHECK(args, 2)) {
					/* TODO: Find the ICU call which does this properly */
					*(args->target++) = (UChar)(((lval - 0x10000) >> 10)   | 0xD800);
					*(args->target++) = (UChar)(((lval - 0x10000) & 0x3FF) | 0xDC00);
				}
				return;
			}
			/* Non-suggogate BMP codepoint */
			if (TARGET_CHECK(args, 1)) {
				*(args->target++) = (UChar)lval;
			}
			return;
		}
		case IS_STRING:
		{
			const char *strval = C_STRVAL_P(val);
			int i = 0, strlen = C_STRLEN_P(val);

			while((i != strlen) && TARGET_CHECK(args, 1)) {
				UChar c;
				U8_NEXT(strval, i, strlen, c);
				*(args->target++) = c;
			}
			return;
		}
		case IS_ARRAY:
		{
			HashTable *ht = C_ARRVAL_P(val);
			zval *tmpzval;

			CREX_HASH_FOREACH_VAL(ht, tmpzval) {
				crx_converter_append_toUnicode_target(tmpzval, args, objval);
			} CREX_HASH_FOREACH_END();
			return;
		}
		default:
			crx_converter_throw_failure(objval, U_ILLEGAL_ARGUMENT_ERROR,
			                            "toUCallback() specified illegal type for substitution character");
	}
}
/* }}} */

/* {{{ crx_converter_to_u_callback */
static void crx_converter_to_u_callback(const void *context,
                                        UConverterToUnicodeArgs *args,
                                        const char *codeUnits, int32_t length,
                                        UConverterCallbackReason reason,
                                        UErrorCode *pErrorCode) {
	crx_converter_object *objval = (crx_converter_object*)context;
	zval retval;
	zval zargs[4];

	ZVAL_LONG(&zargs[0], reason);
	if (args->source) {
		ZVAL_STRINGL(&zargs[1], args->source, args->sourceLimit - args->source);
	} else {
		ZVAL_EMPTY_STRING(&zargs[1]);
	}
	if (codeUnits) {
		ZVAL_STRINGL(&zargs[2], codeUnits, length);
	} else {
		ZVAL_EMPTY_STRING(&zargs[2]);
	}
	ZVAL_LONG(&zargs[3], *pErrorCode);
	ZVAL_MAKE_REF(&zargs[3]);

	objval->to_cb.param_count    = 4;
	objval->to_cb.params = zargs;
	objval->to_cb.retval = &retval;
	if (crex_call_function(&(objval->to_cb), &(objval->to_cache)) == FAILURE) {
		/* Unlikely */
		crx_converter_throw_failure(objval, U_INTERNAL_PROGRAM_ERROR, "Unexpected failure calling toUCallback()");
	} else if (!C_ISUNDEF(retval)) {
		crx_converter_append_toUnicode_target(&retval, args, objval);
		zval_ptr_dtor(&retval);
	}

	if (C_TYPE(zargs[3]) == IS_LONG) {
		*pErrorCode = C_LVAL(zargs[3]);
	} else if (C_ISREF(zargs[3]) && C_TYPE_P(C_REFVAL(zargs[3])) == IS_LONG) {
		*pErrorCode = C_LVAL_P(C_REFVAL(zargs[3]));
	}

	zval_ptr_dtor(&zargs[0]);
	zval_ptr_dtor(&zargs[1]);
	zval_ptr_dtor(&zargs[2]);
	zval_ptr_dtor(&zargs[3]);
}
/* }}} */

/* {{{ crx_converter_append_fromUnicode_target */
static void crx_converter_append_fromUnicode_target(zval *val, UConverterFromUnicodeArgs *args, crx_converter_object *objval) {
	switch (C_TYPE_P(val)) {
		case IS_NULL:
			/* Ignore */
			return;
		case IS_LONG:
			if (TARGET_CHECK(args, 1)) {
				*(args->target++) = C_LVAL_P(val);
			}
			return;
		case IS_STRING:
		{
			size_t vallen = C_STRLEN_P(val);
			if (TARGET_CHECK(args, vallen)) {
				memcpy(args->target, C_STRVAL_P(val), vallen);
				args->target += vallen;
			}
			return;
		}
		case IS_ARRAY:
		{
			HashTable *ht = C_ARRVAL_P(val);
			zval *tmpzval;
			CREX_HASH_FOREACH_VAL(ht, tmpzval) {
				crx_converter_append_fromUnicode_target(tmpzval, args, objval);
			} CREX_HASH_FOREACH_END();
			return;
		}
		default:
			crx_converter_throw_failure(objval, U_ILLEGAL_ARGUMENT_ERROR, "fromUCallback() specified illegal type for substitution character");
	}
}
/* }}} */

/* {{{ crx_converter_from_u_callback */
static void crx_converter_from_u_callback(const void *context,
                                          UConverterFromUnicodeArgs *args,
                                          const UChar *codeUnits, int32_t length, UChar32 codePoint,
                                          UConverterCallbackReason reason,
                                          UErrorCode *pErrorCode) {
	crx_converter_object *objval = (crx_converter_object*)context;
	zval retval;
	zval zargs[4];
	int i;

	ZVAL_LONG(&zargs[0], reason);
	array_init(&zargs[1]);
	i = 0;
	while (i < length) {
		UChar32 c;
		U16_NEXT(codeUnits, i, length, c);
		add_next_index_long(&zargs[1], c);
	}
	ZVAL_LONG(&zargs[2], codePoint);
	ZVAL_LONG(&zargs[3], *pErrorCode);
	ZVAL_MAKE_REF(&zargs[3]);

	objval->from_cb.param_count = 4;
	objval->from_cb.params = zargs;
	objval->from_cb.retval = &retval;
	if (crex_call_function(&(objval->from_cb), &(objval->from_cache)) == FAILURE) {
		/* Unlikely */
		crx_converter_throw_failure(objval, U_INTERNAL_PROGRAM_ERROR, "Unexpected failure calling fromUCallback()");
	} else if (!C_ISUNDEF(retval)) {
		crx_converter_append_fromUnicode_target(&retval, args, objval);
		zval_ptr_dtor(&retval);
	}

	if (C_TYPE(zargs[3]) == IS_LONG) {
		*pErrorCode = C_LVAL(zargs[3]);
	} else if (C_ISREF(zargs[3]) && C_TYPE_P(C_REFVAL(zargs[3])) == IS_LONG) {
		*pErrorCode = C_LVAL_P(C_REFVAL(zargs[3]));
	}

	zval_ptr_dtor(&zargs[0]);
	zval_ptr_dtor(&zargs[1]);
	zval_ptr_dtor(&zargs[2]);
	zval_ptr_dtor(&zargs[3]);
}
/* }}} */

/* {{{ crx_converter_set_callbacks */
static inline bool crx_converter_set_callbacks(crx_converter_object *objval, UConverter *cnv) {
	bool ret = 1;
	UErrorCode error = U_ZERO_ERROR;

	if (objval->obj.ce == crx_converter_ce) {
		/* Short-circuit having to go through method calls and data marshalling
		 * when we're using default behavior
		 */
		return 1;
	}

	ucnv_setToUCallBack(cnv, (UConverterToUCallback)crx_converter_to_u_callback, (const void*)objval,
	                    NULL, NULL, &error);
	if (U_FAILURE(error)) {
		THROW_UFAILURE(objval, "ucnv_setToUCallBack", error);
		ret = 0;
	}

	error = U_ZERO_ERROR;
	ucnv_setFromUCallBack(cnv, (UConverterFromUCallback)crx_converter_from_u_callback, (const void*)objval,
	                      NULL, NULL, &error);
	if (U_FAILURE(error)) {
		THROW_UFAILURE(objval, "ucnv_setFromUCallBack", error);
		ret = 0;
	}
	return ret;
}
/* }}} */

/* {{{ crx_converter_set_encoding */
static bool crx_converter_set_encoding(crx_converter_object *objval,
                                       UConverter **pcnv,
                                       const char *enc, size_t enc_len) {
	UErrorCode error = U_ZERO_ERROR;
	UConverter *cnv = ucnv_open(enc, &error);

	if (error == U_AMBIGUOUS_ALIAS_WARNING) {
		UErrorCode getname_error = U_ZERO_ERROR;
		const char *actual_encoding = ucnv_getName(cnv, &getname_error);
		if (U_FAILURE(getname_error)) {
			/* Should never happen */
			actual_encoding = "(unknown)";
		}
		crx_error_docref(NULL, E_WARNING, "Ambiguous encoding specified, using %s", actual_encoding);
	} else if (U_FAILURE(error)) {
		if (objval) {
			THROW_UFAILURE(objval, "ucnv_open", error);
		} else {
			crx_error_docref(NULL, E_WARNING, "Error setting encoding: %d - %s", (int)error, u_errorName(error));
		}
		return 0;
	}

	if (objval && !crx_converter_set_callbacks(objval, cnv)) {
		return 0;
	}

	if (*pcnv) {
		ucnv_close(*pcnv);
	}
	*pcnv = cnv;
	return 1;
}
/* }}} */

/* {{{ crx_converter_do_set_encoding */
static void crx_converter_do_set_encoding(UConverter **pcnv, INTERNAL_FUNCTION_PARAMETERS) {
	crx_converter_object *objval = CONV_GET(CREX_THIS);
	char *enc;
	size_t enc_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &enc, &enc_len) == FAILURE) {
		RETURN_THROWS();
	}
	intl_errors_reset(&objval->error);

	RETURN_BOOL(crx_converter_set_encoding(objval, pcnv, enc, enc_len));
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, setSourceEncoding) {
	crx_converter_object *objval = CONV_GET(CREX_THIS);
	crx_converter_do_set_encoding(&(objval->src), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, setDestinationEncoding) {
	crx_converter_object *objval = CONV_GET(CREX_THIS);
	crx_converter_do_set_encoding(&(objval->dest), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ crx_converter_do_get_encoding */
static void crx_converter_do_get_encoding(crx_converter_object *objval, UConverter *cnv, INTERNAL_FUNCTION_PARAMETERS) {
	const char *name;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	intl_errors_reset(&objval->error);

	if (!cnv) {
		RETURN_NULL();
	}

	name = ucnv_getName(cnv, &objval->error.code);
	if (U_FAILURE(objval->error.code)) {
		THROW_UFAILURE(objval, "ucnv_getName()", objval->error.code);
		RETURN_FALSE;
	}

	RETURN_STRING(name);
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, getSourceEncoding) {
	crx_converter_object *objval = CONV_GET(CREX_THIS);
	crx_converter_do_get_encoding(objval, objval->src, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, getDestinationEncoding) {
	crx_converter_object *objval = CONV_GET(CREX_THIS);
	crx_converter_do_get_encoding(objval, objval->dest, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ crx_converter_do_get_type */
static void crx_converter_do_get_type(crx_converter_object *objval, UConverter *cnv, INTERNAL_FUNCTION_PARAMETERS) {
	UConverterType t;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	intl_errors_reset(&objval->error);

	if (!cnv) {
		RETURN_NULL();
	}

	t = ucnv_getType(cnv);
	if (U_FAILURE(objval->error.code)) {
		THROW_UFAILURE(objval, "ucnv_getType", objval->error.code);
		RETURN_FALSE;
	}

	RETURN_LONG(t);
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, getSourceType) {
	crx_converter_object *objval = CONV_GET(CREX_THIS);
	crx_converter_do_get_type(objval, objval->src, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, getDestinationType) {
	crx_converter_object *objval = CONV_GET(CREX_THIS);
	crx_converter_do_get_type(objval, objval->dest, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ crx_converter_resolve_callback */
static void crx_converter_resolve_callback(zval *zobj,
                                           crx_converter_object *objval,
                                           const char *callback_name,
                                           crex_fcall_info *finfo,
                                           crex_fcall_info_cache *fcache) {
	char *errstr = NULL;
	zval caller;

	array_init(&caller);
	C_ADDREF_P(zobj);
	add_index_zval(&caller, 0, zobj);
	add_index_string(&caller, 1, callback_name);
	if (crex_fcall_info_init(&caller, 0, finfo, fcache, NULL, &errstr) == FAILURE) {
		crx_converter_throw_failure(objval, U_INTERNAL_PROGRAM_ERROR, "Error setting converter callback: %s", errstr);
	}
	crex_array_destroy(C_ARR(caller));
	ZVAL_UNDEF(&finfo->function_name);
	if (errstr) {
		efree(errstr);
	}
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, __main) {
	crx_converter_object *objval = CONV_GET(CREX_THIS);
	char *src = "utf-8";
	size_t src_len = sizeof("utf-8") - 1;
	char *dest = src;
	size_t dest_len = src_len;

	intl_error_reset(NULL);

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|s!s!", &dest, &dest_len, &src, &src_len) == FAILURE) {
		RETURN_THROWS();
	}

	crx_converter_set_encoding(objval, &(objval->src),  src,  src_len );
	crx_converter_set_encoding(objval, &(objval->dest), dest, dest_len);
	crx_converter_resolve_callback(CREX_THIS, objval, "toUCallback",   &(objval->to_cb),   &(objval->to_cache));
	crx_converter_resolve_callback(CREX_THIS, objval, "fromUCallback", &(objval->from_cb), &(objval->from_cache));
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, setSubstChars) {
	crx_converter_object *objval = CONV_GET(CREX_THIS);
	char *chars;
	size_t chars_len;
	int ret = 1;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &chars, &chars_len) == FAILURE) {
		RETURN_THROWS();
	}
	intl_errors_reset(&objval->error);

	if (objval->src) {
		UErrorCode error = U_ZERO_ERROR;
		ucnv_setSubstChars(objval->src, chars, chars_len, &error);
		if (U_FAILURE(error)) {
			THROW_UFAILURE(objval, "ucnv_setSubstChars", error);
			ret = 0;
		}
	} else {
		crx_converter_throw_failure(objval, U_INVALID_STATE_ERROR, "Source Converter has not been initialized yet");
		ret = 0;
	}

	if (objval->dest) {
		UErrorCode error = U_ZERO_ERROR;
		ucnv_setSubstChars(objval->dest, chars, chars_len, &error);
		if (U_FAILURE(error)) {
			THROW_UFAILURE(objval, "ucnv_setSubstChars", error);
			ret = 0;
		}
	} else {
		crx_converter_throw_failure(objval, U_INVALID_STATE_ERROR, "Destination Converter has not been initialized yet");
		ret = 0;
	}

	RETURN_BOOL(ret);
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, getSubstChars) {
	crx_converter_object *objval = CONV_GET(CREX_THIS);
	char chars[127];
	int8_t chars_len = sizeof(chars);
	UErrorCode error = U_ZERO_ERROR;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	intl_errors_reset(&objval->error);

	if (!objval->src) {
		RETURN_NULL();
	}

	/* src and dest get the same subst chars set,
	 * so it doesn't really matter which one we read from
	 */
	ucnv_getSubstChars(objval->src, chars, &chars_len, &error);
	if (U_FAILURE(error)) {
		THROW_UFAILURE(objval, "ucnv_getSubstChars", error);
		RETURN_FALSE;
	}

	RETURN_STRINGL(chars, chars_len);
}
/* }}} */

/* {{{ crx_converter_do_convert */
static crex_string* crx_converter_do_convert(UConverter *dest_cnv,
                                             UConverter *src_cnv,  const char *src, int32_t src_len,
                                             crx_converter_object *objval
                                            ) {
	UErrorCode	error = U_ZERO_ERROR;
	int32_t		temp_len, ret_len;
	crex_string	*ret;
	UChar		*temp;

	if (!src_cnv || !dest_cnv) {
		crx_converter_throw_failure(objval, U_INVALID_STATE_ERROR,
		                            "Internal converters not initialized");
		return NULL;
	}

	/* Get necessary buffer size first */
	temp_len = 1 + ucnv_toUChars(src_cnv, NULL, 0, src, src_len, &error);
	if (U_FAILURE(error) && error != U_BUFFER_OVERFLOW_ERROR) {
		THROW_UFAILURE(objval, "ucnv_toUChars", error);
		return NULL;
	}
	temp = safe_emalloc(sizeof(UChar), temp_len, sizeof(UChar));

	/* Convert to intermediate UChar* array */
	error = U_ZERO_ERROR;
	temp_len = ucnv_toUChars(src_cnv, temp, temp_len, src, src_len, &error);
	if (U_FAILURE(error)) {
		THROW_UFAILURE(objval, "ucnv_toUChars", error);
		efree(temp);
		return NULL;
	}
	temp[temp_len] = 0;

	/* Get necessary output buffer size */
	ret_len = ucnv_fromUChars(dest_cnv, NULL, 0, temp, temp_len, &error);
	if (U_FAILURE(error) && error != U_BUFFER_OVERFLOW_ERROR) {
		THROW_UFAILURE(objval, "ucnv_fromUChars", error);
		efree(temp);
		return NULL;
	}

	ret = crex_string_alloc(ret_len, 0);

	/* Convert to final encoding */
	error = U_ZERO_ERROR;
	ZSTR_LEN(ret) = ucnv_fromUChars(dest_cnv, ZSTR_VAL(ret), ret_len+1, temp, temp_len, &error);
	efree(temp);
	if (U_FAILURE(error)) {
		THROW_UFAILURE(objval, "ucnv_fromUChars", error);
		crex_string_efree(ret);
		return NULL;
	}

	return ret;
}
/* }}} */

/* {{{ */
#define UCNV_REASON_CASE(v) case (UCNV_ ## v) : RETURN_STRINGL( "REASON_" #v , sizeof( "REASON_" #v ) - 1);
CRX_METHOD(UConverter, reasonText) {
	crex_long reason;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &reason) == FAILURE) {
		RETURN_THROWS();
	}
	intl_error_reset(NULL);

	switch (reason) {
		UCNV_REASON_CASE(UNASSIGNED)
		UCNV_REASON_CASE(ILLEGAL)
		UCNV_REASON_CASE(IRREGULAR)
		UCNV_REASON_CASE(RESET)
		UCNV_REASON_CASE(CLOSE)
		UCNV_REASON_CASE(CLONE)
		default:
			crex_argument_value_error(1, "must be a UConverter::REASON_* constant");
			RETURN_THROWS();
	}
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, convert) {
	crx_converter_object *objval = CONV_GET(CREX_THIS);
	char *str;
	size_t str_len;
	crex_string *ret;
	bool reverse = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s|b",
	                          &str, &str_len, &reverse) == FAILURE) {
		RETURN_THROWS();
	}
	intl_errors_reset(&objval->error);

	ret = crx_converter_do_convert(reverse ? objval->src : objval->dest,
	                               reverse ? objval->dest : objval->src,
	                               str,   str_len,
	                               objval);
	if (ret) {
		RETURN_NEW_STR(ret);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, transcode) {
	char *str, *src, *dest;
	size_t str_len, src_len, dest_len;
	zval *options = NULL;
	UConverter *src_cnv = NULL, *dest_cnv = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sss|a!",
			&str, &str_len, &dest, &dest_len, &src, &src_len, &options) == FAILURE) {
		RETURN_THROWS();
	}
	intl_error_reset(NULL);

	if (crx_converter_set_encoding(NULL, &src_cnv,  src,  src_len) &&
	    crx_converter_set_encoding(NULL, &dest_cnv, dest, dest_len)) {
	    crex_string *ret;
		UErrorCode error = U_ZERO_ERROR;

		if (options && crex_hash_num_elements(C_ARRVAL_P(options))) {
			zval *tmpzval;

			if (U_SUCCESS(error) &&
				(tmpzval = crex_hash_str_find(C_ARRVAL_P(options), "from_subst", sizeof("from_subst") - 1)) != NULL &&
				C_TYPE_P(tmpzval) == IS_STRING) {
				error = U_ZERO_ERROR;
				ucnv_setSubstChars(src_cnv, C_STRVAL_P(tmpzval), C_STRLEN_P(tmpzval) & 0x7F, &error);
			}
			if (U_SUCCESS(error) &&
				(tmpzval = crex_hash_str_find(C_ARRVAL_P(options), "to_subst", sizeof("to_subst") - 1)) != NULL &&
				C_TYPE_P(tmpzval) == IS_STRING) {
				error = U_ZERO_ERROR;
				ucnv_setSubstChars(dest_cnv, C_STRVAL_P(tmpzval), C_STRLEN_P(tmpzval) & 0x7F, &error);
			}
		}

		if (U_SUCCESS(error) &&
			(ret = crx_converter_do_convert(dest_cnv, src_cnv, str, str_len, NULL)) != NULL) {
			RETVAL_NEW_STR(ret);
		}

		if (U_FAILURE(error)) {
			THROW_UFAILURE(NULL, "transcode", error);
			RETVAL_FALSE;
		}
	} else {
		RETVAL_FALSE;
	}

	if (src_cnv) {
		ucnv_close(src_cnv);
	}
	if (dest_cnv) {
		ucnv_close(dest_cnv);
	}
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, getErrorCode) {
	crx_converter_object *objval = CONV_GET(CREX_THIS);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	RETURN_LONG(intl_error_get_code(&(objval->error)));
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, getErrorMessage) {
	crx_converter_object *objval = CONV_GET(CREX_THIS);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	crex_string *message = intl_error_get_message(&(objval->error));
	if (message) {
		RETURN_STR(message);
	} else {
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, getAvailable) {
	int32_t i,
			count = ucnv_countAvailable();

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	intl_error_reset(NULL);

	array_init(return_value);
	for(i = 0; i < count; i++) {
		const char *name = ucnv_getAvailableName(i);
		add_next_index_string(return_value, name);
	}
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, getAliases) {
	char *name;
	size_t name_len;
	UErrorCode error = U_ZERO_ERROR;
	uint16_t i, count;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &name, &name_len) == FAILURE) {
		RETURN_THROWS();
	}
	intl_error_reset(NULL);

	count = ucnv_countAliases(name, &error);
	if (U_FAILURE(error)) {
		THROW_UFAILURE(NULL, "ucnv_countAliases", error);
		RETURN_FALSE;
	}

	array_init(return_value);
	for(i = 0; i < count; i++) {
		const char *alias;

		error = U_ZERO_ERROR;
		alias = ucnv_getAlias(name, i, &error);
		if (U_FAILURE(error)) {
			THROW_UFAILURE(NULL, "ucnv_getAlias", error);
			crex_array_destroy(C_ARR_P(return_value));
			RETURN_NULL();
		}
		add_next_index_string(return_value, alias);
	}
}
/* }}} */

/* {{{ */
CRX_METHOD(UConverter, getStandards) {
	uint16_t i, count;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	intl_error_reset(NULL);

	array_init(return_value);
	count = ucnv_countStandards();
	for(i = 0; i < count; i++) {
		UErrorCode error = U_ZERO_ERROR;
		const char *name = ucnv_getStandard(i, &error);
		if (U_FAILURE(error)) {
			THROW_UFAILURE(NULL, "ucnv_getStandard", error);
			crex_array_destroy(C_ARR_P(return_value));
			RETURN_NULL();
		}
		add_next_index_string(return_value, name);
	}
}
/* }}} */

/* {{{ Converter create/clone/destroy */
static void crx_converter_free_object(crex_object *obj) {
	crx_converter_object *objval = crx_converter_fetch_object(obj);

	if (objval->src) {
		ucnv_close(objval->src);
	}

	if (objval->dest) {
		ucnv_close(objval->dest);
	}

	intl_error_reset(&objval->error);
	crex_object_std_dtor(obj);
}

static crex_object *crx_converter_object_ctor(crex_class_entry *ce, crx_converter_object **pobjval) {
	crx_converter_object *objval;

	objval = crex_object_alloc(sizeof(crx_converter_object), ce);

	crex_object_std_init(&objval->obj, ce);
	object_properties_init(&objval->obj, ce);
	intl_error_init(&(objval->error));

	*pobjval = objval;

	return &objval->obj;
}

static crex_object *crx_converter_create_object(crex_class_entry *ce) {
	crx_converter_object *objval = NULL;
	crex_object *retval = crx_converter_object_ctor(ce, &objval);

	object_properties_init(&(objval->obj), ce);

	return retval;
}

static crex_object *crx_converter_clone_object(crex_object *object) {
	crx_converter_object *objval, *oldobj = crx_converter_fetch_object(object);
	crex_object *retval = crx_converter_object_ctor(object->ce, &objval);
	UErrorCode error = U_ZERO_ERROR;

	intl_errors_reset(&oldobj->error);

#if U_ICU_VERSION_MAJOR_NUM > 70
	objval->src = ucnv_clone(oldobj->src, &error);
#else
	objval->src = ucnv_safeClone(oldobj->src, NULL, NULL, &error);
#endif
	if (U_SUCCESS(error)) {
		error = U_ZERO_ERROR;
#if U_ICU_VERSION_MAJOR_NUM > 70
		objval->dest = ucnv_clone(oldobj->dest, &error);
#else
		objval->dest = ucnv_safeClone(oldobj->dest, NULL, NULL, &error);
#endif
	}
	if (U_FAILURE(error)) {
		crex_string *err_msg;
		THROW_UFAILURE(oldobj, "ucnv_safeClone", error);

		err_msg = intl_error_get_message(&oldobj->error);
		crex_throw_exception(NULL, ZSTR_VAL(err_msg), 0);
		crex_string_release_ex(err_msg, 0);

		return retval;
	}

	/* Update contexts for converter error handlers */
	crx_converter_set_callbacks(objval, objval->src );
	crx_converter_set_callbacks(objval, objval->dest);

	crex_objects_clone_members(&(objval->obj), &(oldobj->obj));

	/* Newly cloned object deliberately does not inherit error state from original object */

	return retval;
}
/* }}} */

/* {{{ crx_converter_minit */
int crx_converter_minit(INIT_FUNC_ARGS) {
	crx_converter_ce = register_class_UConverter();
	crx_converter_ce->create_object = crx_converter_create_object;
	crx_converter_ce->default_object_handlers = &crx_converter_object_handlers;
	memcpy(&crx_converter_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	crx_converter_object_handlers.offset = XtOffsetOf(crx_converter_object, obj);
	crx_converter_object_handlers.clone_obj = crx_converter_clone_object;
	crx_converter_object_handlers.free_obj = crx_converter_free_object;

	return SUCCESS;
}
/* }}} */
