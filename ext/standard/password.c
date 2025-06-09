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
   | Authors: Anthony Ferrara <ircmaxell@crx.net>                         |
   |          Charles R. Portwood II <charlesportwoodii@erianna.com>      |
   +----------------------------------------------------------------------+
*/

#include <stdlib.h>

#include "crx.h"

#include "fcntl.h"
#include "crx_password.h"
#include "crx_crypt.h"
#include "base64.h"
#include "crex_interfaces.h"
#include "info.h"
#include "ext/random/crx_random.h"
#ifdef HAVE_ARGON2LIB
#include "argon2.h"
#endif

#ifdef CRX_WIN32
#include "win32/winutil.h"
#endif

static crex_array crx_password_algos;

int crx_password_algo_register(const char *ident, const crx_password_algo *algo) {
	crex_string *key = crex_string_init_interned(ident, strlen(ident), 1);
	return crex_hash_add_ptr(&crx_password_algos, key, (void *) algo) ? SUCCESS : FAILURE;
}

void crx_password_algo_unregister(const char *ident) {
	crex_hash_str_del(&crx_password_algos, ident, strlen(ident));
}

static int crx_password_salt_to64(const char *str, const size_t str_len, const size_t out_len, char *ret) /* {{{ */
{
	size_t pos = 0;
	crex_string *buffer;
	if ((int) str_len < 0) {
		return FAILURE;
	}
	buffer = crx_base64_encode((unsigned char*) str, str_len);
	if (ZSTR_LEN(buffer) < out_len) {
		/* Too short of an encoded string generated */
		crex_string_release_ex(buffer, 0);
		return FAILURE;
	}
	for (pos = 0; pos < out_len; pos++) {
		if (ZSTR_VAL(buffer)[pos] == '+') {
			ret[pos] = '.';
		} else if (ZSTR_VAL(buffer)[pos] == '=') {
			crex_string_free(buffer);
			return FAILURE;
		} else {
			ret[pos] = ZSTR_VAL(buffer)[pos];
		}
	}
	crex_string_free(buffer);
	return SUCCESS;
}
/* }}} */

static crex_string* crx_password_make_salt(size_t length) /* {{{ */
{
	crex_string *ret, *buffer;

	if (length > (INT_MAX / 3)) {
		crex_value_error("Length is too large to safely generate");
		return NULL;
	}

	buffer = crex_string_alloc(length * 3 / 4 + 1, 0);
	if (FAILURE == crx_random_bytes_throw(ZSTR_VAL(buffer), ZSTR_LEN(buffer))) {
		crex_value_error("Unable to generate salt");
		crex_string_release_ex(buffer, 0);
		return NULL;
	}

	ret = crex_string_alloc(length, 0);
	if (crx_password_salt_to64(ZSTR_VAL(buffer), ZSTR_LEN(buffer), length, ZSTR_VAL(ret)) == FAILURE) {
		crex_value_error("Generated salt too short");
		crex_string_release_ex(buffer, 0);
		crex_string_release_ex(ret, 0);
		return NULL;
	}
	crex_string_release_ex(buffer, 0);
	ZSTR_VAL(ret)[length] = 0;
	return ret;
}
/* }}} */

static crex_string* crx_password_get_salt(zval *unused_, size_t required_salt_len, HashTable *options) {
	if (options && crex_hash_str_exists(options, "salt", sizeof("salt") - 1)) {
		crx_error_docref(NULL, E_WARNING, "The \"salt\" option has been ignored, since providing a custom salt is no longer supported");
	}

	return crx_password_make_salt(required_salt_len);
}

/* bcrypt implementation */

static bool crx_password_bcrypt_valid(const crex_string *hash) {
	const char *h = ZSTR_VAL(hash);
	return (ZSTR_LEN(hash) == 60) &&
		(h[0] == '$') && (h[1] == '2') && (h[2] == 'y');
}

static int crx_password_bcrypt_get_info(zval *return_value, const crex_string *hash) {
	crex_long cost = CRX_PASSWORD_BCRYPT_COST;

	if (!crx_password_bcrypt_valid(hash)) {
		/* Should never get called this way. */
		return FAILURE;
	}

	sscanf(ZSTR_VAL(hash), "$2y$" CREX_LONG_FMT "$", &cost);
	add_assoc_long(return_value, "cost", cost);

	return SUCCESS;
}

static bool crx_password_bcrypt_needs_rehash(const crex_string *hash, crex_array *options) {
	zval *znew_cost;
	crex_long old_cost = CRX_PASSWORD_BCRYPT_COST;
	crex_long new_cost = CRX_PASSWORD_BCRYPT_COST;

	if (!crx_password_bcrypt_valid(hash)) {
		/* Should never get called this way. */
		return 1;
	}

	sscanf(ZSTR_VAL(hash), "$2y$" CREX_LONG_FMT "$", &old_cost);
	if (options && (znew_cost = crex_hash_str_find(options, "cost", sizeof("cost")-1)) != NULL) {
		new_cost = zval_get_long(znew_cost);
	}

	return old_cost != new_cost;
}

static bool crx_password_bcrypt_verify(const crex_string *password, const crex_string *hash) {
	int status = 0;
	crex_string *ret = crx_crypt(ZSTR_VAL(password), (int)ZSTR_LEN(password), ZSTR_VAL(hash), (int)ZSTR_LEN(hash), 1);

	if (!ret) {
		return 0;
	}

	if (ZSTR_LEN(hash) < 13) {
		crex_string_free(ret);
		return 0;
	}

	/* We're using this method instead of == in order to provide
	 * resistance towards timing attacks. This is a constant time
	 * equality check that will always check every byte of both
	 * values. */
	status = crx_safe_bcmp(ret, hash);

	crex_string_free(ret);
	return status == 0;
}

static crex_string* crx_password_bcrypt_hash(const crex_string *password, crex_array *options) {
	char hash_format[10];
	size_t hash_format_len;
	crex_string *result, *hash, *salt;
	zval *zcost;
	crex_long cost = CRX_PASSWORD_BCRYPT_COST;

	if (options && (zcost = crex_hash_str_find(options, "cost", sizeof("cost")-1)) != NULL) {
		cost = zval_get_long(zcost);
	}

	if (cost < 4 || cost > 31) {
		crex_value_error("Invalid bcrypt cost parameter specified: " CREX_LONG_FMT, cost);
		return NULL;
	}

	hash_format_len = snprintf(hash_format, sizeof(hash_format), "$2y$%02" CREX_LONG_FMT_SPEC "$", cost);
	if (!(salt = crx_password_get_salt(NULL, C_UL(22), options))) {
		return NULL;
	}
	ZSTR_VAL(salt)[ZSTR_LEN(salt)] = 0;

	hash = crex_string_alloc(ZSTR_LEN(salt) + hash_format_len, 0);
	sprintf(ZSTR_VAL(hash), "%s%s", hash_format, ZSTR_VAL(salt));
	ZSTR_VAL(hash)[hash_format_len + ZSTR_LEN(salt)] = 0;

	crex_string_release_ex(salt, 0);

	/* This cast is safe, since both values are defined here in code and cannot overflow */
	result = crx_crypt(ZSTR_VAL(password), (int)ZSTR_LEN(password), ZSTR_VAL(hash), (int)ZSTR_LEN(hash), 1);
	crex_string_release_ex(hash, 0);

	if (!result) {
		return NULL;
	}

	if (ZSTR_LEN(result) < 13) {
		crex_string_free(result);
		return NULL;
	}

	return result;
}

const crx_password_algo crx_password_algo_bcrypt = {
	"bcrypt",
	crx_password_bcrypt_hash,
	crx_password_bcrypt_verify,
	crx_password_bcrypt_needs_rehash,
	crx_password_bcrypt_get_info,
	crx_password_bcrypt_valid,
};


#ifdef HAVE_ARGON2LIB
/* argon2i/argon2id shared implementation */

static int extract_argon2_parameters(const crex_string *hash,
									  crex_long *v, crex_long *memory_cost,
									  crex_long *time_cost, crex_long *threads) /* {{{ */
{
	const char *p = ZSTR_VAL(hash);
	if (!hash || (ZSTR_LEN(hash) < sizeof("$argon2id$"))) {
		return FAILURE;
	}
	if (!memcmp(p, "$argon2i$", sizeof("$argon2i$") - 1)) {
		p += sizeof("$argon2i$") - 1;
	} else if (!memcmp(p, "$argon2id$", sizeof("$argon2id$") - 1)) {
		p += sizeof("$argon2id$") - 1;
	} else {
		return FAILURE;
	}

	sscanf(p, "v=" CREX_LONG_FMT "$m=" CREX_LONG_FMT ",t=" CREX_LONG_FMT ",p=" CREX_LONG_FMT,
	       v, memory_cost, time_cost, threads);

	return SUCCESS;
}
/* }}} */

static int crx_password_argon2_get_info(zval *return_value, const crex_string *hash) {
	crex_long v = 0;
	crex_long memory_cost = CRX_PASSWORD_ARGON2_MEMORY_COST;
	crex_long time_cost = CRX_PASSWORD_ARGON2_TIME_COST;
	crex_long threads = CRX_PASSWORD_ARGON2_THREADS;

	extract_argon2_parameters(hash, &v, &memory_cost, &time_cost, &threads);

	add_assoc_long(return_value, "memory_cost", memory_cost);
	add_assoc_long(return_value, "time_cost", time_cost);
	add_assoc_long(return_value, "threads", threads);

	return SUCCESS;
}

static bool crx_password_argon2_needs_rehash(const crex_string *hash, crex_array *options) {
	crex_long v = 0;
	crex_long new_memory_cost = CRX_PASSWORD_ARGON2_MEMORY_COST, memory_cost = 0;
	crex_long new_time_cost = CRX_PASSWORD_ARGON2_TIME_COST, time_cost = 0;
	crex_long new_threads = CRX_PASSWORD_ARGON2_THREADS, threads = 0;
	zval *option_buffer;

	if (options && (option_buffer = crex_hash_str_find(options, "memory_cost", sizeof("memory_cost")-1)) != NULL) {
		new_memory_cost = zval_get_long(option_buffer);
	}

	if (options && (option_buffer = crex_hash_str_find(options, "time_cost", sizeof("time_cost")-1)) != NULL) {
		new_time_cost = zval_get_long(option_buffer);
	}

	if (options && (option_buffer = crex_hash_str_find(options, "threads", sizeof("threads")-1)) != NULL) {
		new_threads = zval_get_long(option_buffer);
	}

	extract_argon2_parameters(hash, &v, &memory_cost, &time_cost, &threads);

	return (new_time_cost != time_cost) ||
			(new_memory_cost != memory_cost) ||
			(new_threads != threads);
}

static crex_string *crx_password_argon2_hash(const crex_string *password, crex_array *options, argon2_type type) {
	zval *option_buffer;
	crex_string *salt, *out, *encoded;
	size_t time_cost = CRX_PASSWORD_ARGON2_TIME_COST;
	size_t memory_cost = CRX_PASSWORD_ARGON2_MEMORY_COST;
	size_t threads = CRX_PASSWORD_ARGON2_THREADS;
	size_t encoded_len;
	int status = 0;

	if (options && (option_buffer = crex_hash_str_find(options, "memory_cost", sizeof("memory_cost")-1)) != NULL) {
		memory_cost = zval_get_long(option_buffer);
	}

	if (memory_cost > ARGON2_MAX_MEMORY || memory_cost < ARGON2_MIN_MEMORY) {
		crex_value_error("Memory cost is outside of allowed memory range");
		return NULL;
	}

	if (options && (option_buffer = crex_hash_str_find(options, "time_cost", sizeof("time_cost")-1)) != NULL) {
		time_cost = zval_get_long(option_buffer);
	}

	if (time_cost > ARGON2_MAX_TIME || time_cost < ARGON2_MIN_TIME) {
		crex_value_error("Time cost is outside of allowed time range");
		return NULL;
	}

	if (options && (option_buffer = crex_hash_str_find(options, "threads", sizeof("threads")-1)) != NULL) {
		threads = zval_get_long(option_buffer);
	}

	if (threads > ARGON2_MAX_LANES || threads == 0) {
		crex_value_error("Invalid number of threads");
		return NULL;
	}

	if (!(salt = crx_password_get_salt(NULL, C_UL(16), options))) {
		return NULL;
	}

	out = crex_string_alloc(32, 0);
	encoded_len = argon2_encodedlen(
		time_cost,
		memory_cost,
		threads,
		(uint32_t)ZSTR_LEN(salt),
		ZSTR_LEN(out),
		type
	);

	encoded = crex_string_alloc(encoded_len - 1, 0);
	status = argon2_hash(
		time_cost,
		memory_cost,
		threads,
		ZSTR_VAL(password),
		ZSTR_LEN(password),
		ZSTR_VAL(salt),
		ZSTR_LEN(salt),
		ZSTR_VAL(out),
		ZSTR_LEN(out),
		ZSTR_VAL(encoded),
		encoded_len,
		type,
		ARGON2_VERSION_NUMBER
	);

	crex_string_release_ex(out, 0);
	crex_string_release_ex(salt, 0);

	if (status != ARGON2_OK) {
		crex_string_efree(encoded);
		crex_value_error("%s", argon2_error_message(status));
		return NULL;
	}

	ZSTR_VAL(encoded)[ZSTR_LEN(encoded)] = 0;
	return encoded;
}

/* argon2i specific methods */

static bool crx_password_argon2i_verify(const crex_string *password, const crex_string *hash) {
	return ARGON2_OK == argon2_verify(ZSTR_VAL(hash), ZSTR_VAL(password), ZSTR_LEN(password), Argon2_i);
}

static crex_string *crx_password_argon2i_hash(const crex_string *password, crex_array *options) {
	return crx_password_argon2_hash(password, options, Argon2_i);
}

const crx_password_algo crx_password_algo_argon2i = {
	"argon2i",
	crx_password_argon2i_hash,
	crx_password_argon2i_verify,
	crx_password_argon2_needs_rehash,
	crx_password_argon2_get_info,
	NULL,
};

/* argon2id specific methods */

static bool crx_password_argon2id_verify(const crex_string *password, const crex_string *hash) {
	return ARGON2_OK == argon2_verify(ZSTR_VAL(hash), ZSTR_VAL(password), ZSTR_LEN(password), Argon2_id);
}

static crex_string *crx_password_argon2id_hash(const crex_string *password, crex_array *options) {
	return crx_password_argon2_hash(password, options, Argon2_id);
}

const crx_password_algo crx_password_algo_argon2id = {
	"argon2id",
	crx_password_argon2id_hash,
	crx_password_argon2id_verify,
	crx_password_argon2_needs_rehash,
	crx_password_argon2_get_info,
	NULL,
};
#endif

CRX_MINIT_FUNCTION(password) /* {{{ */
{
	crex_hash_init(&crx_password_algos, 4, NULL, ZVAL_PTR_DTOR, 1);
	REGISTER_STRING_CONSTANT("PASSWORD_DEFAULT", "2y", CONST_PERSISTENT);

	if (FAILURE == crx_password_algo_register("2y", &crx_password_algo_bcrypt)) {
		return FAILURE;
	}
	REGISTER_STRING_CONSTANT("PASSWORD_BCRYPT", "2y", CONST_PERSISTENT);

#ifdef HAVE_ARGON2LIB
	if (FAILURE == crx_password_algo_register("argon2i", &crx_password_algo_argon2i)) {
		return FAILURE;
	}
	REGISTER_STRING_CONSTANT("PASSWORD_ARGON2I", "argon2i", CONST_PERSISTENT);

	if (FAILURE == crx_password_algo_register("argon2id", &crx_password_algo_argon2id)) {
		return FAILURE;
	}
	REGISTER_STRING_CONSTANT("PASSWORD_ARGON2ID", "argon2id", CONST_PERSISTENT);
#endif

	REGISTER_LONG_CONSTANT("PASSWORD_BCRYPT_DEFAULT_COST", CRX_PASSWORD_BCRYPT_COST, CONST_PERSISTENT);
#ifdef HAVE_ARGON2LIB
	REGISTER_LONG_CONSTANT("PASSWORD_ARGON2_DEFAULT_MEMORY_COST", CRX_PASSWORD_ARGON2_MEMORY_COST, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PASSWORD_ARGON2_DEFAULT_TIME_COST", CRX_PASSWORD_ARGON2_TIME_COST, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PASSWORD_ARGON2_DEFAULT_THREADS", CRX_PASSWORD_ARGON2_THREADS, CONST_PERSISTENT);

	REGISTER_STRING_CONSTANT("PASSWORD_ARGON2_PROVIDER", "standard", CONST_PERSISTENT);
#endif

	return SUCCESS;
}
/* }}} */

CRX_MSHUTDOWN_FUNCTION(password) /* {{{ */
{
#ifdef ZTS
	if (!tsrm_is_main_thread()) {
		return SUCCESS;
	}
#endif
	crex_hash_destroy(&crx_password_algos);
	return SUCCESS;
}
/* }}} */

const crx_password_algo* crx_password_algo_default(void) {
	return &crx_password_algo_bcrypt;
}

const crx_password_algo* crx_password_algo_find(const crex_string *ident) {
	zval *tmp;

	if (!ident) {
		return NULL;
	}

	tmp = crex_hash_find(&crx_password_algos, (crex_string*)ident);
	if (!tmp || (C_TYPE_P(tmp) != IS_PTR)) {
		return NULL;
	}

	return C_PTR_P(tmp);
}

static const crx_password_algo* crx_password_algo_find_zval(crex_string *arg_str, crex_long arg_long, bool arg_is_null) {
	if (arg_is_null) {
		return crx_password_algo_default();
	}

	if (arg_str) {
		return crx_password_algo_find(arg_str);
	}

	switch (arg_long) {
		case 0: return crx_password_algo_default();
		case 1: return &crx_password_algo_bcrypt;
#ifdef HAVE_ARGON2LIB
		case 2: return &crx_password_algo_argon2i;
		case 3: return &crx_password_algo_argon2id;
#else
		case 2:
			{
			crex_string *n = ZSTR_INIT_LITERAL("argon2i", 0);
			const crx_password_algo* ret = crx_password_algo_find(n);
			crex_string_release(n);
			return ret;
			}
		case 3:
			{
			crex_string *n = ZSTR_INIT_LITERAL("argon2id", 0);
			const crx_password_algo* ret = crx_password_algo_find(n);
			crex_string_release(n);
			return ret;
			}
#endif
	}

	return NULL;
}

crex_string *crx_password_algo_extract_ident(const crex_string* hash) {
	const char *ident, *ident_end;

	if (!hash || ZSTR_LEN(hash) < 3) {
		/* Minimum prefix: "$x$" */
		return NULL;
	}

	ident = ZSTR_VAL(hash) + 1;
	ident_end = strchr(ident, '$');
	if (!ident_end) {
		/* No terminating '$' */
		return NULL;
	}

	return crex_string_init(ident, ident_end - ident, 0);
}

const crx_password_algo* crx_password_algo_identify_ex(const crex_string* hash, const crx_password_algo *default_algo) {
	const crx_password_algo *algo;
	crex_string *ident = crx_password_algo_extract_ident(hash);

	if (!ident) {
		return default_algo;
	}

	algo = crx_password_algo_find(ident);
	crex_string_release(ident);
	return (!algo || (algo->valid && !algo->valid(hash))) ? default_algo : algo;
}

/* {{{ Retrieves information about a given hash */
CRX_FUNCTION(password_get_info)
{
	const crx_password_algo *algo;
	crex_string *hash, *ident;
	zval options;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(hash)
	CREX_PARSE_PARAMETERS_END();

	array_init(return_value);
	array_init(&options);

	ident = crx_password_algo_extract_ident(hash);
	algo = crx_password_algo_find(ident);
	if (!algo || (algo->valid && !algo->valid(hash))) {
		if (ident) {
			crex_string_release(ident);
		}
		add_assoc_null(return_value, "algo");
		add_assoc_string(return_value, "algoName", "unknown");
		add_assoc_zval(return_value, "options", &options);
		return;
	}

	add_assoc_str(return_value, "algo", crx_password_algo_extract_ident(hash));
	crex_string_release(ident);

	add_assoc_string(return_value, "algoName", algo->name);
	if (algo->get_info) {
		algo->get_info(&options, hash);
	}
	add_assoc_zval(return_value, "options", &options);
}
/** }}} */

/* {{{ Determines if a given hash requires re-hashing based upon parameters */
CRX_FUNCTION(password_needs_rehash)
{
	const crx_password_algo *old_algo, *new_algo;
	crex_string *hash;
	crex_string *new_algo_str;
	crex_long new_algo_long = 0;
	bool new_algo_is_null;
	crex_array *options = NULL;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(hash)
		C_PARAM_STR_OR_LONG_OR_NULL(new_algo_str, new_algo_long, new_algo_is_null)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT(options)
	CREX_PARSE_PARAMETERS_END();

	new_algo = crx_password_algo_find_zval(new_algo_str, new_algo_long, new_algo_is_null);
	if (!new_algo) {
		/* Unknown new algorithm, never prompt to rehash. */
		RETURN_FALSE;
	}

	old_algo = crx_password_algo_identify_ex(hash, NULL);
	if (old_algo != new_algo) {
		/* Different algorithm preferred, always rehash. */
		RETURN_TRUE;
	}

	RETURN_BOOL(old_algo->needs_rehash(hash, options));
}
/* }}} */

/* {{{ Verify a hash created using crypt() or password_hash() */
CRX_FUNCTION(password_verify)
{
	crex_string *password, *hash;
	const crx_password_algo *algo;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_STR(password)
		C_PARAM_STR(hash)
	CREX_PARSE_PARAMETERS_END();

	algo = crx_password_algo_identify(hash);
	RETURN_BOOL(algo && (!algo->verify || algo->verify(password, hash)));
}
/* }}} */

/* {{{ Hash a password */
CRX_FUNCTION(password_hash)
{
	crex_string *password, *digest = NULL;
	crex_string *algo_str;
	crex_long algo_long = 0;
	bool algo_is_null;
	const crx_password_algo *algo;
	crex_array *options = NULL;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_STR(password)
		C_PARAM_STR_OR_LONG_OR_NULL(algo_str, algo_long, algo_is_null)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT(options)
	CREX_PARSE_PARAMETERS_END();

	algo = crx_password_algo_find_zval(algo_str, algo_long, algo_is_null);
	if (!algo) {
		crex_argument_value_error(2, "must be a valid password hashing algorithm");
		RETURN_THROWS();
	}

	digest = algo->hash(password, options);
	if (!digest) {
		if (!EG(exception)) {
			crex_throw_error(NULL, "Password hashing failed for unknown reason");
		}
		RETURN_THROWS();
	}

	RETURN_NEW_STR(digest);
}
/* }}} */

/* {{{ */
CRX_FUNCTION(password_algos) {
	crex_string *algo;

	CREX_PARSE_PARAMETERS_NONE();

	array_init(return_value);
	CREX_HASH_MAP_FOREACH_STR_KEY(&crx_password_algos, algo) {
		add_next_index_str(return_value, crex_string_copy(algo));
	} CREX_HASH_FOREACH_END();
}
/* }}} */
