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
   | Authors: Sara Golemon <pollita@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crx_http.h"
#include "crx_ini.h"
#include "url.h"

static void crx_url_encode_scalar(zval *scalar, smart_str *form_str,
	int encoding_type, crex_ulong index_int,
	const char *index_string, size_t index_string_len,
	const char *num_prefix, size_t num_prefix_len,
	const crex_string *key_prefix,
	const crex_string *arg_sep)
{
	if (form_str->s) {
		smart_str_append(form_str, arg_sep);
	}
	/* Simple key=value */
	if (key_prefix) {
		smart_str_append(form_str, key_prefix);
	}
	if (index_string) {
		crex_string *encoded_key;
		if (encoding_type == CRX_QUERY_RFC3986) {
			encoded_key = crx_raw_url_encode(index_string, index_string_len);
		} else {
			encoded_key = crx_url_encode(index_string, index_string_len);
		}
		smart_str_append(form_str, encoded_key);
		crex_string_free(encoded_key);
	} else {
		/* Numeric key */
		if (num_prefix) {
			smart_str_appendl(form_str, num_prefix, num_prefix_len);
		}
		smart_str_append_long(form_str, index_int);
	}
	if (key_prefix) {
		smart_str_appendl(form_str, "%5D", strlen("%5D"));
	}
	smart_str_appendc(form_str, '=');

	switch (C_TYPE_P(scalar)) {
		case IS_STRING: {
			crex_string *encoded_data;
			if (encoding_type == CRX_QUERY_RFC3986) {
				encoded_data = crx_raw_url_encode(C_STRVAL_P(scalar), C_STRLEN_P(scalar));
			} else {
				encoded_data = crx_url_encode(C_STRVAL_P(scalar), C_STRLEN_P(scalar));
			}
			smart_str_append(form_str, encoded_data);
			crex_string_free(encoded_data);
			break;
		}
		case IS_LONG:
			smart_str_append_long(form_str, C_LVAL_P(scalar));
			break;
		case IS_DOUBLE: {
			crex_string *encoded_data;
			crex_string *tmp = crex_double_to_str(C_DVAL_P(scalar));
			if (encoding_type == CRX_QUERY_RFC3986) {
				encoded_data = crx_raw_url_encode(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
			} else {
				encoded_data = crx_url_encode(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
			}
			smart_str_append(form_str, encoded_data);
			crex_string_free(tmp);
			crex_string_free(encoded_data);
			break;
		}
		case IS_FALSE:
			smart_str_appendc(form_str, '0');
			break;
		case IS_TRUE:
			smart_str_appendc(form_str, '1');
			break;
		/* All possible types are either handled here or previously */
		EMPTY_SWITCH_DEFAULT_CASE();
	}
}

/* {{{ crx_url_encode_hash */
CRXAPI void crx_url_encode_hash_ex(HashTable *ht, smart_str *formstr,
				const char *num_prefix, size_t num_prefix_len,
				const crex_string *key_prefix,
				zval *type, const crex_string *arg_sep, int enc_type)
{
	crex_string *key = NULL;
	const char *prop_name;
	size_t prop_len;
	crex_ulong idx;
	zval *zdata = NULL;
	CREX_ASSERT(ht);

	if (GC_IS_RECURSIVE(ht)) {
		/* Prevent recursion */
		return;
	}

	if (!arg_sep) {
		arg_sep = crex_ini_str("arg_separator.output", strlen("arg_separator.output"), false);
		if (ZSTR_LEN(arg_sep) == 0) {
			arg_sep = ZSTR_CHAR('&');
		}
	}

	CREX_HASH_FOREACH_KEY_VAL(ht, idx, key, zdata) {
		bool is_dynamic = 1;
		if (C_TYPE_P(zdata) == IS_INDIRECT) {
			zdata = C_INDIRECT_P(zdata);
			if (C_ISUNDEF_P(zdata)) {
				continue;
			}

			is_dynamic = 0;
		}

		/* handling for private & protected object properties */
		if (key) {
			prop_name = ZSTR_VAL(key);
			prop_len = ZSTR_LEN(key);

			if (type != NULL && crex_check_property_access(C_OBJ_P(type), key, is_dynamic) != SUCCESS) {
				/* property not visible in this scope */
				continue;
			}

			if (ZSTR_VAL(key)[0] == '\0' && type != NULL) {
				const char *tmp;
				crex_unmangle_property_name_ex(key, &tmp, &prop_name, &prop_len);
			} else {
				prop_name = ZSTR_VAL(key);
				prop_len = ZSTR_LEN(key);
			}
		} else {
			prop_name = NULL;
			prop_len = 0;
		}

		ZVAL_DEREF(zdata);
		if (C_TYPE_P(zdata) == IS_ARRAY || C_TYPE_P(zdata) == IS_OBJECT) {
			crex_string *new_prefix;
			if (key) {
				crex_string *encoded_key;
				if (enc_type == CRX_QUERY_RFC3986) {
					encoded_key = crx_raw_url_encode(prop_name, prop_len);
				} else {
					encoded_key = crx_url_encode(prop_name, prop_len);
				}

				if (key_prefix) {
					new_prefix = crex_string_concat3(ZSTR_VAL(key_prefix), ZSTR_LEN(key_prefix), ZSTR_VAL(encoded_key), ZSTR_LEN(encoded_key), "%5D%5B", strlen("%5D%5B"));
				} else {
					new_prefix = crex_string_concat2(ZSTR_VAL(encoded_key), ZSTR_LEN(encoded_key), "%5B", strlen("%5B"));
				}
				crex_string_release_ex(encoded_key, false);
			} else { /* is integer index */
				char *index_int_as_str;
				size_t index_int_as_str_len;

				index_int_as_str_len = spprintf(&index_int_as_str, 0, CREX_LONG_FMT, idx);

				if (key_prefix && num_prefix) {
					/* crex_string_concat4() */
					size_t len = ZSTR_LEN(key_prefix) + num_prefix_len + index_int_as_str_len + strlen("%5D%5B");
					new_prefix = crex_string_alloc(len, 0);

					memcpy(ZSTR_VAL(new_prefix), ZSTR_VAL(key_prefix), ZSTR_LEN(key_prefix));
					memcpy(ZSTR_VAL(new_prefix) + ZSTR_LEN(key_prefix), num_prefix, num_prefix_len);
					memcpy(ZSTR_VAL(new_prefix) + ZSTR_LEN(key_prefix) + num_prefix_len, index_int_as_str, index_int_as_str_len);
					memcpy(ZSTR_VAL(new_prefix) + ZSTR_LEN(key_prefix) + num_prefix_len +index_int_as_str_len, "%5D%5B", strlen("%5D%5B"));
					ZSTR_VAL(new_prefix)[len] = '\0';
				} else if (key_prefix) {
					new_prefix = crex_string_concat3(ZSTR_VAL(key_prefix), ZSTR_LEN(key_prefix), index_int_as_str, index_int_as_str_len, "%5D%5B", strlen("%5D%5B"));
				} else if (num_prefix) {
					new_prefix = crex_string_concat3(num_prefix, num_prefix_len, index_int_as_str, index_int_as_str_len, "%5B", strlen("%5B"));
				} else {
					new_prefix = crex_string_concat2(index_int_as_str, index_int_as_str_len, "%5B", strlen("%5B"));
				}
				efree(index_int_as_str);
			}
			GC_TRY_PROTECT_RECURSION(ht);
			crx_url_encode_hash_ex(HASH_OF(zdata), formstr, NULL, 0, new_prefix, (C_TYPE_P(zdata) == IS_OBJECT ? zdata : NULL), arg_sep, enc_type);
			GC_TRY_UNPROTECT_RECURSION(ht);
			crex_string_release_ex(new_prefix, false);
		} else if (C_TYPE_P(zdata) == IS_NULL || C_TYPE_P(zdata) == IS_RESOURCE) {
			/* Skip these types */
			continue;
		} else {
			crx_url_encode_scalar(zdata, formstr,
				enc_type, idx,
				prop_name, prop_len,
				num_prefix, num_prefix_len,
				key_prefix,
				arg_sep);
		}
	} CREX_HASH_FOREACH_END();
}
/* }}} */

	/* If there is a prefix we need to close the key with an encoded ] ("%5D") */
/* {{{ Generates a form-encoded query string from an associative array or object. */
CRX_FUNCTION(http_build_query)
{
	zval *formdata;
	char *prefix = NULL;
	size_t prefix_len = 0;
	crex_string *arg_sep = NULL;
	smart_str formstr = {0};
	crex_long enc_type = CRX_QUERY_RFC1738;

	CREX_PARSE_PARAMETERS_START(1, 4)
		C_PARAM_ARRAY_OR_OBJECT(formdata)
		C_PARAM_OPTIONAL
		C_PARAM_STRING(prefix, prefix_len)
		C_PARAM_STR(arg_sep)
		C_PARAM_LONG(enc_type)
	CREX_PARSE_PARAMETERS_END();

	crx_url_encode_hash_ex(HASH_OF(formdata), &formstr, prefix, prefix_len, /* key_prefix */ NULL, (C_TYPE_P(formdata) == IS_OBJECT ? formdata : NULL), arg_sep, (int)enc_type);

	RETURN_STR(smart_str_extract(&formstr));
}
/* }}} */
