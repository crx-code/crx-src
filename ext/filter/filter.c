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
  | Authors: Rasmus Lerdorf <rasmus@crx.net>                             |
  |          Derick Rethans <derick@crx.net>                             |
  |          Pierre-A. Joye <pierre@crx.net>                             |
  |          Ilia Alshanetsky <iliaa@crx.net>                            |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx_filter.h"

CREX_DECLARE_MODULE_GLOBALS(filter)

#include "filter_private.h"
#include "filter_arginfo.h"

typedef struct filter_list_entry {
	const char *name;
	int    id;
	void (*function)(CRX_INPUT_FILTER_PARAM_DECL);
} filter_list_entry;

/* {{{ filter_list */
static const filter_list_entry filter_list[] = {
	{ "int",             FILTER_VALIDATE_INT,           crx_filter_int             },
	{ "boolean",         FILTER_VALIDATE_BOOL,          crx_filter_boolean         },
	{ "float",           FILTER_VALIDATE_FLOAT,         crx_filter_float           },

	{ "validate_regexp", FILTER_VALIDATE_REGEXP,        crx_filter_validate_regexp },
	{ "validate_domain", FILTER_VALIDATE_DOMAIN,        crx_filter_validate_domain },
	{ "validate_url",    FILTER_VALIDATE_URL,           crx_filter_validate_url    },
	{ "validate_email",  FILTER_VALIDATE_EMAIL,         crx_filter_validate_email  },
	{ "validate_ip",     FILTER_VALIDATE_IP,            crx_filter_validate_ip     },
	{ "validate_mac",    FILTER_VALIDATE_MAC,           crx_filter_validate_mac    },

	{ "string",          FILTER_SANITIZE_STRING,        crx_filter_string          },
	{ "stripped",        FILTER_SANITIZE_STRING,        crx_filter_string          },
	{ "encoded",         FILTER_SANITIZE_ENCODED,       crx_filter_encoded         },
	{ "special_chars",   FILTER_SANITIZE_SPECIAL_CHARS, crx_filter_special_chars   },
	{ "full_special_chars",   FILTER_SANITIZE_FULL_SPECIAL_CHARS, crx_filter_full_special_chars   },
	{ "unsafe_raw",      FILTER_UNSAFE_RAW,             crx_filter_unsafe_raw      },
	{ "email",           FILTER_SANITIZE_EMAIL,         crx_filter_email           },
	{ "url",             FILTER_SANITIZE_URL,           crx_filter_url             },
	{ "number_int",      FILTER_SANITIZE_NUMBER_INT,    crx_filter_number_int      },
	{ "number_float",    FILTER_SANITIZE_NUMBER_FLOAT,  crx_filter_number_float    },
	{ "add_slashes",     FILTER_SANITIZE_ADD_SLASHES,   crx_filter_add_slashes     },

	{ "callback",        FILTER_CALLBACK,               crx_filter_callback        },
};
/* }}} */

#ifndef PARSE_ENV
#define PARSE_ENV 4
#endif

#ifndef PARSE_SERVER
#define PARSE_SERVER 5
#endif

#ifndef PARSE_SESSION
#define PARSE_SESSION 6
#endif

static unsigned int crx_sapi_filter(int arg, const char *var, char **val, size_t val_len, size_t *new_val_len);
static unsigned int crx_sapi_filter_init(void);

/* {{{ filter_module_entry */
crex_module_entry filter_module_entry = {
	STANDARD_MODULE_HEADER,
	"filter",
	ext_functions,
	CRX_MINIT(filter),
	CRX_MSHUTDOWN(filter),
	NULL,
	CRX_RSHUTDOWN(filter),
	CRX_MINFO(filter),
	CRX_FILTER_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_FILTER
#ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
#endif
CREX_GET_MODULE(filter)
#endif

static CRX_INI_MH(UpdateDefaultFilter) /* {{{ */
{
	int i, size = sizeof(filter_list) / sizeof(filter_list_entry);

	for (i = 0; i < size; ++i) {
		if ((strcasecmp(ZSTR_VAL(new_value), filter_list[i].name) == 0)) {
			IF_G(default_filter) = filter_list[i].id;
			if (IF_G(default_filter) != FILTER_DEFAULT) {
				crex_error(E_DEPRECATED, "The filter.default ini setting is deprecated");
			}
			return SUCCESS;
		}
	}
	/* Fallback to the default filter */
	IF_G(default_filter) = FILTER_DEFAULT;
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_INI */
static CRX_INI_MH(OnUpdateFlags)
{
	if (!new_value) {
		IF_G(default_filter_flags) = FILTER_FLAG_NO_ENCODE_QUOTES;
	} else {
		IF_G(default_filter_flags) = atoi(ZSTR_VAL(new_value));
	}
	return SUCCESS;
}

CRX_INI_BEGIN()
	STD_CRX_INI_ENTRY("filter.default",   "unsafe_raw", CRX_INI_SYSTEM|CRX_INI_PERDIR, UpdateDefaultFilter, default_filter, crex_filter_globals, filter_globals)
	CRX_INI_ENTRY("filter.default_flags", NULL,     CRX_INI_SYSTEM|CRX_INI_PERDIR, OnUpdateFlags)
CRX_INI_END()
/* }}} */

static void crx_filter_init_globals(crex_filter_globals *filter_globals) /* {{{ */
{
#if defined(COMPILE_DL_FILTER) && defined(ZTS)
CREX_TSRMLS_CACHE_UPDATE();
#endif
	ZVAL_UNDEF(&filter_globals->post_array);
	ZVAL_UNDEF(&filter_globals->get_array);
	ZVAL_UNDEF(&filter_globals->cookie_array);
	ZVAL_UNDEF(&filter_globals->env_array);
	ZVAL_UNDEF(&filter_globals->server_array);
#if 0
	ZVAL_UNDEF(&filter_globals->session_array);
#endif
	filter_globals->default_filter = FILTER_DEFAULT;
}
/* }}} */

#define PARSE_REQUEST 99

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(filter)
{
	CREX_INIT_MODULE_GLOBALS(filter, crx_filter_init_globals, NULL);

	REGISTER_INI_ENTRIES();

	register_filter_symbols(module_number);

	sapi_register_input_filter(crx_sapi_filter, crx_sapi_filter_init);

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(filter)
{
	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_RSHUTDOWN_FUNCTION */
#define VAR_ARRAY_COPY_DTOR(a)   \
	if (!C_ISUNDEF(IF_G(a))) {   \
		zval_ptr_dtor(&IF_G(a)); \
		ZVAL_UNDEF(&IF_G(a));    \
	}

CRX_RSHUTDOWN_FUNCTION(filter)
{
	VAR_ARRAY_COPY_DTOR(get_array)
	VAR_ARRAY_COPY_DTOR(post_array)
	VAR_ARRAY_COPY_DTOR(cookie_array)
	VAR_ARRAY_COPY_DTOR(server_array)
	VAR_ARRAY_COPY_DTOR(env_array)
#if 0
	VAR_ARRAY_COPY_DTOR(session_array)
#endif
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(filter)
{
	crx_info_print_table_start();
	crx_info_print_table_row( 2, "Input Validation and Filtering", "enabled" );
	crx_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

static filter_list_entry crx_find_filter(crex_long id) /* {{{ */
{
	int i, size = sizeof(filter_list) / sizeof(filter_list_entry);

	for (i = 0; i < size; ++i) {
		if (filter_list[i].id == id) {
			return filter_list[i];
		}
	}
	/* Fallback to "string" filter */
	for (i = 0; i < size; ++i) {
		if (filter_list[i].id == FILTER_DEFAULT) {
			return filter_list[i];
		}
	}
	/* To shut up GCC */
	return filter_list[0];
}
/* }}} */

static unsigned int crx_sapi_filter_init(void)
{
	ZVAL_UNDEF(&IF_G(get_array));
	ZVAL_UNDEF(&IF_G(post_array));
	ZVAL_UNDEF(&IF_G(cookie_array));
	ZVAL_UNDEF(&IF_G(server_array));
	ZVAL_UNDEF(&IF_G(env_array));
#if 0
	ZVAL_UNDEF(&IF_G(session_array));
#endif
	return SUCCESS;
}

static void crx_zval_filter(zval *value, crex_long filter, crex_long flags, zval *options, char* charset, bool copy) /* {{{ */
{
	filter_list_entry  filter_func;

	filter_func = crx_find_filter(filter);

	if (!filter_func.id) {
		/* Find default filter */
		filter_func = crx_find_filter(FILTER_DEFAULT);
	}

	/* #49274, fatal error with object without a toString method
	  Fails nicely instead of getting a recovarable fatal error. */
	if (C_TYPE_P(value) == IS_OBJECT) {
		crex_class_entry *ce;

		ce = C_OBJCE_P(value);
		if (!ce->__tostring) {
			zval_ptr_dtor(value);
			/* #67167: doesn't return null on failure for objects */
			if (flags & FILTER_NULL_ON_FAILURE) {
				ZVAL_NULL(value);
			} else {
				ZVAL_FALSE(value);
			}
			goto handle_default;
		}
	}

	/* Here be strings */
	convert_to_string(value);

	filter_func.function(value, flags, options, charset);

handle_default:
	if (options && C_TYPE_P(options) == IS_ARRAY &&
		((flags & FILTER_NULL_ON_FAILURE && C_TYPE_P(value) == IS_NULL) ||
		(!(flags & FILTER_NULL_ON_FAILURE) && C_TYPE_P(value) == IS_FALSE))) {
		zval *tmp;
		if ((tmp = crex_hash_str_find(C_ARRVAL_P(options), "default", sizeof("default") - 1)) != NULL) {
			ZVAL_COPY(value, tmp);
		}
	}
}
/* }}} */

static unsigned int crx_sapi_filter(int arg, const char *var, char **val, size_t val_len, size_t *new_val_len) /* {{{ */
{
	zval  new_var, raw_var;
	zval *array_ptr = NULL, *orig_array_ptr = NULL;
	int retval = 0;

	assert(*val != NULL);

#define PARSE_CASE(s,a,t)                     		\
		case s:                               		\
			if (C_ISUNDEF(IF_G(a))) {         		\
				array_init(&IF_G(a)); 				\
			}										\
			array_ptr = &IF_G(a);          			\
			orig_array_ptr = &PG(http_globals)[t]; 	\
			break;

	switch (arg) {
		PARSE_CASE(PARSE_POST,    post_array,    TRACK_VARS_POST)
		PARSE_CASE(PARSE_GET,     get_array,     TRACK_VARS_GET)
		PARSE_CASE(PARSE_COOKIE,  cookie_array,  TRACK_VARS_COOKIE)
		PARSE_CASE(PARSE_SERVER,  server_array,  TRACK_VARS_SERVER)
		PARSE_CASE(PARSE_ENV,     env_array,     TRACK_VARS_ENV)

		case PARSE_STRING: /* PARSE_STRING is used by parse_str() function */
			retval = 1;
			break;
	}

	/*
	 * According to rfc2965, more specific paths are listed above the less specific ones.
	 * If we encounter a duplicate cookie name, we should skip it, since it is not possible
	 * to have the same (plain text) cookie name for the same path and we should not overwrite
	 * more specific cookies with the less specific ones.
	*/
	if (arg == PARSE_COOKIE && orig_array_ptr &&
			crex_symtable_str_exists(C_ARRVAL_P(orig_array_ptr), var, strlen(var))) {
		return 0;
	}

	if (array_ptr) {
		/* Store the RAW variable internally */
		ZVAL_STRINGL(&raw_var, *val, val_len);
		crx_register_variable_ex(var, &raw_var, array_ptr);
	}

	if (val_len) {
		/* Register mangled variable */
		if (IF_G(default_filter) != FILTER_UNSAFE_RAW) {
			ZVAL_STRINGL(&new_var, *val, val_len);
			crx_zval_filter(&new_var, IF_G(default_filter), IF_G(default_filter_flags), NULL, NULL, 0);
		} else {
			ZVAL_STRINGL(&new_var, *val, val_len);
		}
	} else { /* empty string */
		ZVAL_EMPTY_STRING(&new_var);
	}

	if (orig_array_ptr) {
		crx_register_variable_ex(var, &new_var, orig_array_ptr);
	}

	if (retval) {
		if (new_val_len) {
			*new_val_len = C_STRLEN(new_var);
		}
		efree(*val);
		if (C_STRLEN(new_var)) {
			*val = estrndup(C_STRVAL(new_var), C_STRLEN(new_var));
		} else {
			*val = estrdup("");
		}
		zval_ptr_dtor(&new_var);
	}

	return retval;
}
/* }}} */

static void crx_zval_filter_recursive(zval *value, crex_long filter, crex_long flags, zval *options, char *charset, bool copy) /* {{{ */
{
	if (C_TYPE_P(value) == IS_ARRAY) {
		zval *element;

		if (C_IS_RECURSIVE_P(value)) {
			return;
		}
		C_PROTECT_RECURSION_P(value);

		CREX_HASH_FOREACH_VAL(C_ARRVAL_P(value), element) {
			ZVAL_DEREF(element);
			if (C_TYPE_P(element) == IS_ARRAY) {
				SEPARATE_ARRAY(element);
				crx_zval_filter_recursive(element, filter, flags, options, charset, copy);
			} else {
				crx_zval_filter(element, filter, flags, options, charset, copy);
			}
		} CREX_HASH_FOREACH_END();
		C_UNPROTECT_RECURSION_P(value);
	} else {
		crx_zval_filter(value, filter, flags, options, charset, copy);
	}
}
/* }}} */

static zval *crx_filter_get_storage(crex_long arg)/* {{{ */

{
	zval *array_ptr = NULL;

	switch (arg) {
		case PARSE_GET:
			array_ptr = &IF_G(get_array);
			break;
		case PARSE_POST:
			array_ptr = &IF_G(post_array);
			break;
		case PARSE_COOKIE:
			array_ptr = &IF_G(cookie_array);
			break;
		case PARSE_SERVER:
			if (PG(auto_globals_jit)) {
				crex_is_auto_global(ZSTR_KNOWN(CREX_STR_AUTOGLOBAL_SERVER));
			}
			array_ptr = &IF_G(server_array);
			break;
		case PARSE_ENV:
			if (PG(auto_globals_jit)) {
				crex_is_auto_global(ZSTR_KNOWN(CREX_STR_AUTOGLOBAL_ENV));
			}
			array_ptr = !C_ISUNDEF(IF_G(env_array)) ? &IF_G(env_array) : &PG(http_globals)[TRACK_VARS_ENV];
			break;
		default:
			crex_argument_value_error(1, "must be an INPUT_* constant");
			return NULL;
	}

	if (array_ptr && C_TYPE_P(array_ptr) != IS_ARRAY) {
		/* Storage not initialized */
		return NULL;
	}

	return array_ptr;
}
/* }}} */

/* {{{ Returns true if the variable with the name 'name' exists in source. */
CRX_FUNCTION(filter_has_var)
{
	crex_long         arg;
	crex_string *var;
	zval        *array_ptr = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "lS", &arg, &var) == FAILURE) {
		RETURN_THROWS();
	}

	array_ptr = crx_filter_get_storage(arg);
	if (EG(exception)) {
		RETURN_THROWS();
	}

	if (array_ptr && crex_hash_exists(C_ARRVAL_P(array_ptr), var)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}
/* }}} */

static void crx_filter_call(
	zval *filtered, crex_long filter, HashTable *filter_args_ht, crex_long filter_args_long,
	const int copy, crex_long filter_flags
) /* {{{ */ {
	zval *options = NULL;
	zval *option;
	char *charset = NULL;

	if (!filter_args_ht) {
		if (filter != -1) { /* handler for array apply */
			/* filter_args is the filter_flags */
			filter_flags = filter_args_long;

			if (!(filter_flags & FILTER_REQUIRE_ARRAY ||  filter_flags & FILTER_FORCE_ARRAY)) {
				filter_flags |= FILTER_REQUIRE_SCALAR;
			}
		} else {
			filter = filter_args_long;
		}
	} else {
		if ((option = crex_hash_str_find(filter_args_ht, "filter", sizeof("filter") - 1)) != NULL) {
			filter = zval_get_long(option);
		}

		if ((option = crex_hash_str_find_deref(filter_args_ht, "options", sizeof("options") - 1)) != NULL) {
			if (filter != FILTER_CALLBACK) {
				if (C_TYPE_P(option) == IS_ARRAY) {
					options = option;
				}
			} else {
				options = option;
				filter_flags = 0;
			}
		}

		if ((option = crex_hash_str_find(filter_args_ht, "flags", sizeof("flags") - 1)) != NULL) {
			filter_flags = zval_get_long(option);

			if (!(filter_flags & FILTER_REQUIRE_ARRAY ||  filter_flags & FILTER_FORCE_ARRAY)) {
				filter_flags |= FILTER_REQUIRE_SCALAR;
			}
		}
	}

	if (C_TYPE_P(filtered) == IS_ARRAY) {
		if (filter_flags & FILTER_REQUIRE_SCALAR) {
			zval_ptr_dtor(filtered);
			if (filter_flags & FILTER_NULL_ON_FAILURE) {
				ZVAL_NULL(filtered);
			} else {
				ZVAL_FALSE(filtered);
			}
			return;
		}
		crx_zval_filter_recursive(filtered, filter, filter_flags, options, charset, copy);
		return;
	}
	if (filter_flags & FILTER_REQUIRE_ARRAY) {
		zval_ptr_dtor(filtered);
		if (filter_flags & FILTER_NULL_ON_FAILURE) {
			ZVAL_NULL(filtered);
		} else {
			ZVAL_FALSE(filtered);
		}
		return;
	}

	crx_zval_filter(filtered, filter, filter_flags, options, charset, copy);
	if (filter_flags & FILTER_FORCE_ARRAY) {
		zval tmp;
		ZVAL_COPY_VALUE(&tmp, filtered);
		array_init(filtered);
		add_next_index_zval(filtered, &tmp);
	}
}
/* }}} */

static void crx_filter_array_handler(zval *input, HashTable *op_ht, crex_long op_long,
	zval *return_value, bool add_empty
) /* {{{ */ {
	crex_string *arg_key;
	zval *tmp, *arg_elm;

	if (!op_ht) {
		ZVAL_DUP(return_value, input);
		crx_filter_call(return_value, -1, NULL, op_long, 0, FILTER_REQUIRE_ARRAY);
	} else {
		array_init(return_value);

		CREX_HASH_FOREACH_STR_KEY_VAL(op_ht, arg_key, arg_elm) {
			if (arg_key == NULL) {
				crex_argument_type_error(2, "must contain only string keys");
				RETURN_THROWS();
	 		}
			if (ZSTR_LEN(arg_key) == 0) {
				crex_argument_value_error(2, "cannot contain empty keys");
				RETURN_THROWS();
			}
			if ((tmp = crex_hash_find(C_ARRVAL_P(input), arg_key)) == NULL) {
				if (add_empty) {
					add_assoc_null_ex(return_value, ZSTR_VAL(arg_key), ZSTR_LEN(arg_key));
				}
			} else {
				zval nval;
				ZVAL_DEREF(tmp);
				ZVAL_DUP(&nval, tmp);
				crx_filter_call(&nval, -1,
					C_TYPE_P(arg_elm) == IS_ARRAY ? C_ARRVAL_P(arg_elm) : NULL,
					C_TYPE_P(arg_elm) == IS_ARRAY ? 0 : zval_get_long(arg_elm),
					0, FILTER_REQUIRE_SCALAR
				);
				crex_hash_update(C_ARRVAL_P(return_value), arg_key, &nval);
			}
		} CREX_HASH_FOREACH_END();
	}
}
/* }}} */

/* {{{ Returns the filtered variable 'name'* from source `type`. */
CRX_FUNCTION(filter_input)
{
	crex_long fetch_from, filter = FILTER_DEFAULT;
	zval *input = NULL, *tmp;
	crex_string *var;
	HashTable *filter_args_ht = NULL;
	crex_long filter_args_long = 0;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_LONG(fetch_from)
		C_PARAM_STR(var)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(filter)
		C_PARAM_ARRAY_HT_OR_LONG(filter_args_ht, filter_args_long)
	CREX_PARSE_PARAMETERS_END();

	if (!CRX_FILTER_ID_EXISTS(filter)) {
		crx_error_docref(NULL, E_WARNING, "Unknown filter with ID " CREX_LONG_FMT, filter);
		RETURN_FALSE;
	}

	input = crx_filter_get_storage(fetch_from);
	if (EG(exception)) {
		RETURN_THROWS();
	}

	if (!input || (tmp = crex_hash_find(C_ARRVAL_P(input), var)) == NULL) {
		crex_long filter_flags = 0;
		zval *option, *opt, *def;
		if (!filter_args_ht) {
			filter_flags = filter_args_long;
		} else {
			if ((option = crex_hash_str_find(filter_args_ht, "flags", sizeof("flags") - 1)) != NULL) {
				filter_flags = zval_get_long(option);
			}

			if ((opt = crex_hash_str_find_deref(filter_args_ht, "options", sizeof("options") - 1)) != NULL &&
				C_TYPE_P(opt) == IS_ARRAY &&
				(def = crex_hash_str_find_deref(C_ARRVAL_P(opt), "default", sizeof("default") - 1)) != NULL
			) {
				ZVAL_COPY(return_value, def);
				return;
			}
		}

		/* The FILTER_NULL_ON_FAILURE flag inverts the usual return values of
		 * the function: normally when validation fails false is returned, and
		 * when the input value doesn't exist NULL is returned. With the flag
		 * set, NULL and false should be returned, respectively. Ergo, although
		 * the code below looks incorrect, it's actually right. */
		if (filter_flags & FILTER_NULL_ON_FAILURE) {
			RETURN_FALSE;
		} else {
			RETURN_NULL();
		}
	}

	ZVAL_DUP(return_value, tmp);

	crx_filter_call(return_value, filter, filter_args_ht, filter_args_long, 1, FILTER_REQUIRE_SCALAR);
}
/* }}} */

/* {{{ Returns the filtered version of the variable. */
CRX_FUNCTION(filter_var)
{
	crex_long filter = FILTER_DEFAULT;
	zval *data;
	HashTable *filter_args_ht = NULL;
	crex_long filter_args_long = 0;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_ZVAL(data)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(filter)
		C_PARAM_ARRAY_HT_OR_LONG(filter_args_ht, filter_args_long)
	CREX_PARSE_PARAMETERS_END();

	if (!CRX_FILTER_ID_EXISTS(filter)) {
		crx_error_docref(NULL, E_WARNING, "Unknown filter with ID " CREX_LONG_FMT, filter);
		RETURN_FALSE;
	}

	ZVAL_DUP(return_value, data);

	crx_filter_call(return_value, filter, filter_args_ht, filter_args_long, 1, FILTER_REQUIRE_SCALAR);
}
/* }}} */

/* {{{ Returns an array with all arguments defined in 'definition'. */
CRX_FUNCTION(filter_input_array)
{
	crex_long    fetch_from;
	zval   *array_input = NULL;
	bool add_empty = 1;
	HashTable *op_ht = NULL;
	crex_long op_long = FILTER_DEFAULT;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_LONG(fetch_from)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_LONG(op_ht, op_long)
		C_PARAM_BOOL(add_empty)
	CREX_PARSE_PARAMETERS_END();

	if (!op_ht && !CRX_FILTER_ID_EXISTS(op_long)) {
		crx_error_docref(NULL, E_WARNING, "Unknown filter with ID " CREX_LONG_FMT, op_long);
		RETURN_FALSE;
	}

	array_input = crx_filter_get_storage(fetch_from);
	if (EG(exception)) {
		RETURN_THROWS();
	}

	if (!array_input) {
		crex_long filter_flags = 0;
		zval *option;
		if (op_long) {
			filter_flags = op_long;
		} else if (op_ht && (option = crex_hash_str_find(op_ht, "flags", sizeof("flags") - 1)) != NULL) {
			filter_flags = zval_get_long(option);
		}

		/* The FILTER_NULL_ON_FAILURE flag inverts the usual return values of
		 * the function: normally when validation fails false is returned, and
		 * when the input value doesn't exist NULL is returned. With the flag
		 * set, NULL and false should be returned, respectively. Ergo, although
		 * the code below looks incorrect, it's actually right. */
		if (filter_flags & FILTER_NULL_ON_FAILURE) {
			RETURN_FALSE;
		} else {
			RETURN_NULL();
		}
	}

	crx_filter_array_handler(array_input, op_ht, op_long, return_value, add_empty);
}
/* }}} */

/* {{{ Returns an array with all arguments defined in 'definition'. */
CRX_FUNCTION(filter_var_array)
{
	zval *array_input = NULL;
	bool add_empty = 1;
	HashTable *op_ht = NULL;
	crex_long op_long = FILTER_DEFAULT;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_ARRAY(array_input)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT_OR_LONG(op_ht, op_long)
		C_PARAM_BOOL(add_empty)
	CREX_PARSE_PARAMETERS_END();

	if (!op_ht && !CRX_FILTER_ID_EXISTS(op_long)) {
		crx_error_docref(NULL, E_WARNING, "Unknown filter with ID " CREX_LONG_FMT, op_long);
		RETURN_FALSE;
	}

	crx_filter_array_handler(array_input, op_ht, op_long, return_value, add_empty);
}
/* }}} */

/* {{{ Returns a list of all supported filters */
CRX_FUNCTION(filter_list)
{
	int i, size = sizeof(filter_list) / sizeof(filter_list_entry);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);
	for (i = 0; i < size; ++i) {
		add_next_index_string(return_value, (char *)filter_list[i].name);
	}
}
/* }}} */

/* {{{ Returns the filter ID belonging to a named filter */
CRX_FUNCTION(filter_id)
{
	int i;
	size_t filter_len;
	int size = sizeof(filter_list) / sizeof(filter_list_entry);
	char *filter;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &filter, &filter_len) == FAILURE) {
		RETURN_THROWS();
	}

	for (i = 0; i < size; ++i) {
		if (strcmp(filter_list[i].name, filter) == 0) {
			RETURN_LONG(filter_list[i].id);
		}
	}

	RETURN_FALSE;
}
/* }}} */
