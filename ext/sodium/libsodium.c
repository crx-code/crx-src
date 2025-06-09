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
   | Authors: Frank Denis <jedisct1@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "crx_libsodium.h"
#include "crex_attributes.h"
#include "crex_exceptions.h"

#include <sodium.h>
#include <stdint.h>
#include <string.h>

#define CRX_SODIUM_ZSTR_TRUNCATE(zs, len) do { ZSTR_LEN(zs) = (len); } while(0)

static crex_class_entry *sodium_exception_ce;

#if (defined(__amd64) || defined(__amd64__) || defined(__x86_64__) || defined(__i386__) || \
	 defined(_M_AMD64) || defined(_M_IX86))
# define HAVE_AESGCM 1
#endif

static crex_always_inline crex_string *crex_string_checked_alloc(size_t len, int persistent)
{
	crex_string *zs;

	if (CREX_MM_ALIGNED_SIZE(_ZSTR_STRUCT_SIZE(len)) < len) {
		crex_error_noreturn(E_ERROR, "Memory allocation too large (%zu bytes)", len);
	}
	zs = crex_string_alloc(len, persistent);
	ZSTR_VAL(zs)[len] = 0;

	return zs;
}

#ifndef crypto_kdf_BYTES_MIN
# define crypto_kdf_BYTES_MIN 16
# define crypto_kdf_BYTES_MAX 64
# define crypto_kdf_CONTEXTBYTES 8
# define crypto_kdf_KEYBYTES 32
#endif

#ifndef crypto_kx_SEEDBYTES
# define crypto_kx_SEEDBYTES 32
# define crypto_kx_SESSIONKEYBYTES 32
# define crypto_kx_PUBLICKEYBYTES 32
# define crypto_kx_SECRETKEYBYTES 32
#endif

#include "libsodium_arginfo.h"

#ifndef crypto_aead_chacha20poly1305_IETF_KEYBYTES
# define crypto_aead_chacha20poly1305_IETF_KEYBYTES crypto_aead_chacha20poly1305_KEYBYTES
#endif
#ifndef crypto_aead_chacha20poly1305_IETF_NSECBYTES
# define crypto_aead_chacha20poly1305_IETF_NSECBYTES crypto_aead_chacha20poly1305_NSECBYTES
#endif
#ifndef crypto_aead_chacha20poly1305_IETF_ABYTES
# define crypto_aead_chacha20poly1305_IETF_ABYTES crypto_aead_chacha20poly1305_ABYTES
#endif

#if defined(crypto_secretstream_xchacha20poly1305_ABYTES) && SODIUM_LIBRARY_VERSION_MAJOR < 10
# undef crypto_secretstream_xchacha20poly1305_ABYTES
#endif

#ifndef crypto_pwhash_OPSLIMIT_MIN
# define crypto_pwhash_OPSLIMIT_MIN crypto_pwhash_OPSLIMIT_INTERACTIVE
#endif
#ifndef crypto_pwhash_MEMLIMIT_MIN
# define crypto_pwhash_MEMLIMIT_MIN crypto_pwhash_MEMLIMIT_INTERACTIVE
#endif
#ifndef crypto_pwhash_scryptsalsa208sha256_OPSLIMIT_MIN
# define crypto_pwhash_scryptsalsa208sha256_OPSLIMIT_MIN crypto_pwhash_scryptsalsa208sha256_OPSLIMIT_INTERACTIVE
#endif
#ifndef crypto_pwhash_scryptsalsa208sha256_MEMLIMIT_MIN
# define crypto_pwhash_scryptsalsa208sha256_MEMLIMIT_MIN crypto_pwhash_scryptsalsa208sha256_MEMLIMIT_INTERACTIVE
#endif

/* Load after the "standard" module in order to give it
 * priority in registering argon2i/argon2id password hashers.
 */
static const crex_module_dep sodium_deps[] = {
	CREX_MOD_REQUIRED("standard")
	CREX_MOD_END
};

crex_module_entry sodium_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	sodium_deps,
	"sodium",
	ext_functions,
	CRX_MINIT(sodium),
	CRX_MSHUTDOWN(sodium),
	NULL,
	NULL,
	CRX_MINFO(sodium),
	CRX_SODIUM_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SODIUM
CREX_GET_MODULE(sodium)
#endif

/* Remove argument information from backtrace to prevent information leaks */
static void sodium_remove_param_values_from_backtrace(crex_object *obj) {
	zval rv;
	zval *trace = crex_read_property_ex(crex_get_exception_base(obj), obj, ZSTR_KNOWN(CREX_STR_TRACE), /* silent */ false, &rv);
	if (trace && C_TYPE_P(trace) == IS_ARRAY) {
		zval *frame;
		CREX_HASH_FOREACH_VAL(C_ARRVAL_P(trace), frame) {
			if (C_TYPE_P(frame) == IS_ARRAY) {
				zval *args = crex_hash_find(C_ARRVAL_P(frame), ZSTR_KNOWN(CREX_STR_ARGS));
				if (args) {
					zval_ptr_dtor(args);
					ZVAL_EMPTY_ARRAY(args);
				}
			}
		} CREX_HASH_FOREACH_END();
	}
}

static crex_object *sodium_exception_create_object(crex_class_entry *ce) {
	crex_object *obj = crex_ce_exception->create_object(ce);
	sodium_remove_param_values_from_backtrace(obj);
	return obj;
}

static void sodium_separate_string(zval *zv) {
	CREX_ASSERT(C_TYPE_P(zv) == IS_STRING);
	if (!C_REFCOUNTED_P(zv) || C_REFCOUNT_P(zv) > 1) {
		crex_string *copy = crex_string_init(C_STRVAL_P(zv), C_STRLEN_P(zv), 0);
		C_TRY_DELREF_P(zv);
		ZVAL_STR(zv, copy);
	}
}

CRX_MINIT_FUNCTION(sodium)
{
	if (sodium_init() < 0) {
		crex_error(E_ERROR, "sodium_init()");
	}

	sodium_exception_ce = register_class_SodiumException(crex_ce_exception);
	sodium_exception_ce->create_object = sodium_exception_create_object;

#if SODIUM_LIBRARY_VERSION_MAJOR > 9 || (SODIUM_LIBRARY_VERSION_MAJOR == 9 && SODIUM_LIBRARY_VERSION_MINOR >= 6)
	if (FAILURE == CRX_MINIT(sodium_password_hash)(INIT_FUNC_ARGS_PASSTHRU)) {
		return FAILURE;
	}
#endif

	register_libsodium_symbols(module_number);

	return SUCCESS;
}

CRX_MSHUTDOWN_FUNCTION(sodium)
{
	randombytes_close();
	return SUCCESS;
}

CRX_MINFO_FUNCTION(sodium)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "sodium support", "enabled");
	crx_info_print_table_row(2, "libsodium headers version", SODIUM_VERSION_STRING);
	crx_info_print_table_row(2, "libsodium library version", sodium_version_string());
	crx_info_print_table_end();
}

CRX_FUNCTION(sodium_memzero)
{
	zval      *buf_zv;

	if (crex_parse_parameters(CREX_NUM_ARGS(),
									"z", &buf_zv) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	ZVAL_DEREF(buf_zv);
	if (C_TYPE_P(buf_zv) != IS_STRING) {
		crex_throw_exception(sodium_exception_ce, "a CRX string is required", 0);
		RETURN_THROWS();
	}
	if (C_REFCOUNTED_P(buf_zv) && C_REFCOUNT_P(buf_zv) == 1) {
		char *buf = C_STRVAL(*buf_zv);
		size_t buf_len = C_STRLEN(*buf_zv);
		if (buf_len > 0) {
			sodium_memzero(buf, (size_t) buf_len);
		}
	}
	convert_to_null(buf_zv);
}

CRX_FUNCTION(sodium_increment)
{
	zval          *val_zv;
	unsigned char *val;
	size_t         val_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(),
									"z", &val_zv) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	ZVAL_DEREF(val_zv);
	if (C_TYPE_P(val_zv) != IS_STRING) {
		crex_throw_exception(sodium_exception_ce, "a CRX string is required", 0);
		RETURN_THROWS();
	}

	sodium_separate_string(val_zv);
	val = (unsigned char *) C_STRVAL(*val_zv);
	val_len = C_STRLEN(*val_zv);
	sodium_increment(val, val_len);
}

CRX_FUNCTION(sodium_add)
{
	zval          *val_zv;
	unsigned char *val;
	unsigned char *addv;
	size_t         val_len;
	size_t         addv_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(),
									"zs", &val_zv, &addv, &addv_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	ZVAL_DEREF(val_zv);
	if (C_TYPE_P(val_zv) != IS_STRING) {
		crex_throw_exception(sodium_exception_ce, "CRX strings are required", 0);
		RETURN_THROWS();
	}

	sodium_separate_string(val_zv);
	val = (unsigned char *) C_STRVAL(*val_zv);
	val_len = C_STRLEN(*val_zv);
	if (val_len != addv_len) {
		crex_argument_error(sodium_exception_ce, 1, "and argument #2 ($string_2) must have the same length");
		RETURN_THROWS();
	}
	sodium_add(val, addv, val_len);
}

CRX_FUNCTION(sodium_memcmp)
{
	char      *buf1;
	char      *buf2;
	size_t     len1;
	size_t     len2;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&buf1, &len1,
									&buf2, &len2) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (len1 != len2) {
		crex_argument_error(sodium_exception_ce, 1, "and argument #2 ($string_2) must have the same length");
		RETURN_THROWS();
	}
	RETURN_LONG(sodium_memcmp(buf1, buf2, len1));
}

CRX_FUNCTION(sodium_crypto_shorthash)
{
	crex_string   *hash;
	unsigned char *key;
	unsigned char *msg;
	size_t         key_len;
	size_t         msg_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&msg, &msg_len,
									&key, &key_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (key_len != crypto_shorthash_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_SHORTHASH_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	hash = crex_string_alloc(crypto_shorthash_BYTES, 0);
	if (crypto_shorthash((unsigned char *) ZSTR_VAL(hash), msg,
						 (unsigned long long) msg_len, key) != 0) {
		crex_string_efree(hash);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(hash)[crypto_shorthash_BYTES] = 0;

	RETURN_NEW_STR(hash);
}

CRX_FUNCTION(sodium_crypto_secretbox)
{
	crex_string   *ciphertext;
	unsigned char *key;
	unsigned char *msg;
	unsigned char *nonce;
	size_t         key_len;
	size_t         msg_len;
	size_t         nonce_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sss",
									&msg, &msg_len,
									&nonce, &nonce_len,
									&key, &key_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (nonce_len != crypto_secretbox_NONCEBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_SECRETBOX_NONCEBYTES bytes long");
		RETURN_THROWS();
	}
	if (key_len != crypto_secretbox_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_SECRETBOX_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (SIZE_MAX - msg_len <= crypto_secretbox_MACBYTES) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	ciphertext = crex_string_alloc((size_t) msg_len + crypto_secretbox_MACBYTES, 0);
	if (crypto_secretbox_easy((unsigned char *) ZSTR_VAL(ciphertext),
							  msg, (unsigned long long) msg_len,
							  nonce, key) != 0) {
		crex_string_efree(ciphertext);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(ciphertext)[msg_len + crypto_secretbox_MACBYTES] = 0;

	RETURN_NEW_STR(ciphertext);
}

CRX_FUNCTION(sodium_crypto_secretbox_open)
{
	crex_string   *msg;
	unsigned char *key;
	unsigned char *ciphertext;
	unsigned char *nonce;
	size_t         key_len;
	size_t         ciphertext_len;
	size_t         nonce_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sss",
									&ciphertext, &ciphertext_len,
									&nonce, &nonce_len,
									&key, &key_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (nonce_len != crypto_secretbox_NONCEBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_SECRETBOX_NONCEBYTES bytes long");
		RETURN_THROWS();
	}
	if (key_len != crypto_secretbox_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_SECRETBOX_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (ciphertext_len < crypto_secretbox_MACBYTES) {
		RETURN_FALSE;
	}
	msg = crex_string_alloc
		((size_t) ciphertext_len - crypto_secretbox_MACBYTES, 0);
	if (crypto_secretbox_open_easy((unsigned char *) ZSTR_VAL(msg), ciphertext,
								   (unsigned long long) ciphertext_len,
								   nonce, key) != 0) {
		crex_string_efree(msg);
		RETURN_FALSE;
	} else {
		ZSTR_VAL(msg)[ciphertext_len - crypto_secretbox_MACBYTES] = 0;
		RETURN_NEW_STR(msg);
	}
}

CRX_FUNCTION(sodium_crypto_generichash)
{
	crex_string   *hash;
	unsigned char *key = NULL;
	unsigned char *msg;
	crex_long      hash_len = crypto_generichash_BYTES;
	size_t         key_len = 0;
	size_t         msg_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s|sl",
									&msg, &msg_len,
									&key, &key_len,
									&hash_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (hash_len < crypto_generichash_BYTES_MIN ||
		hash_len > crypto_generichash_BYTES_MAX) {
		crex_throw_exception(sodium_exception_ce, "unsupported output length", 0);
		RETURN_THROWS();
	}
	if (key_len != 0 &&
		(key_len < crypto_generichash_KEYBYTES_MIN ||
		 key_len > crypto_generichash_KEYBYTES_MAX)) {
		crex_throw_exception(sodium_exception_ce, "unsupported key length", 0);
		RETURN_THROWS();
	}
	hash = crex_string_alloc(hash_len, 0);
	if (crypto_generichash((unsigned char *) ZSTR_VAL(hash), (size_t) hash_len,
						   msg, (unsigned long long) msg_len,
						   key, (size_t) key_len) != 0) {
		crex_string_efree(hash);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(hash)[hash_len] = 0;

	RETURN_NEW_STR(hash);
}

CRX_FUNCTION(sodium_crypto_generichash_init)
{
	crypto_generichash_state  state_tmp;
	crex_string              *state;
	unsigned char            *key = NULL;
	size_t                    state_len = sizeof (crypto_generichash_state);
	crex_long                 hash_len = crypto_generichash_BYTES;
	size_t                    key_len = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|sl",
									&key, &key_len,
									&hash_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (hash_len < crypto_generichash_BYTES_MIN ||
		hash_len > crypto_generichash_BYTES_MAX) {
		crex_throw_exception(sodium_exception_ce, "unsupported output length", 0);
		RETURN_THROWS();
	}
	if (key_len != 0 &&
		(key_len < crypto_generichash_KEYBYTES_MIN ||
		 key_len > crypto_generichash_KEYBYTES_MAX)) {
		crex_throw_exception(sodium_exception_ce, "unsupported key length", 0);
		RETURN_THROWS();
	}
	memset(&state_tmp, 0, sizeof state_tmp);
	if (crypto_generichash_init((void *) &state_tmp, key, (size_t) key_len,
								(size_t) hash_len) != 0) {
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	state = crex_string_alloc(state_len, 0);
	memcpy(ZSTR_VAL(state), &state_tmp, state_len);
	sodium_memzero(&state_tmp, sizeof state_tmp);
	ZSTR_VAL(state)[state_len] = 0;

	RETURN_STR(state);
}

CRX_FUNCTION(sodium_crypto_generichash_update)
{
	crypto_generichash_state  state_tmp;
	zval                     *state_zv;
	unsigned char            *msg;
	unsigned char            *state;
	size_t                    msg_len;
	size_t                    state_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "zs",
									&state_zv, &msg, &msg_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	ZVAL_DEREF(state_zv);
	if (C_TYPE_P(state_zv) != IS_STRING) {
		crex_argument_error(sodium_exception_ce, 1, "must be a reference to a state");
		RETURN_THROWS();
	}
	sodium_separate_string(state_zv);
	state = (unsigned char *) C_STRVAL(*state_zv);
	state_len = C_STRLEN(*state_zv);
	if (state_len != sizeof (crypto_generichash_state)) {
		crex_throw_exception(sodium_exception_ce, "incorrect state length", 0);
		RETURN_THROWS();
	}
	memcpy(&state_tmp, state, sizeof state_tmp);
	if (crypto_generichash_update((void *) &state_tmp, msg,
								  (unsigned long long) msg_len) != 0) {
		sodium_memzero(&state_tmp, sizeof state_tmp);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	memcpy(state, &state_tmp, state_len);
	sodium_memzero(&state_tmp, sizeof state_tmp);

	RETURN_TRUE;
}

CRX_FUNCTION(sodium_crypto_generichash_final)
{
	crypto_generichash_state  state_tmp;
	crex_string              *hash;
	zval                     *state_zv;
	unsigned char            *state;
	size_t                    state_len;
	crex_long                 hash_len = crypto_generichash_BYTES;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "z|l",
									&state_zv, &hash_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	ZVAL_DEREF(state_zv);
	if (C_TYPE_P(state_zv) != IS_STRING) {
		crex_argument_error(sodium_exception_ce, 1, "must be a reference to a state");
		RETURN_THROWS();
	}
	sodium_separate_string(state_zv);
	state = (unsigned char *) C_STRVAL(*state_zv);
	state_len = C_STRLEN(*state_zv);
	if (state_len != sizeof (crypto_generichash_state)) {
		crex_throw_exception(sodium_exception_ce, "incorrect state length", 0);
		RETURN_THROWS();
	}
	if (hash_len < crypto_generichash_BYTES_MIN ||
		hash_len > crypto_generichash_BYTES_MAX) {
		crex_throw_exception(sodium_exception_ce, "unsupported output length", 0);
		RETURN_THROWS();
	}
	hash = crex_string_alloc(hash_len, 0);
	memcpy(&state_tmp, state, sizeof state_tmp);
	if (crypto_generichash_final((void *) &state_tmp,
								 (unsigned char *) ZSTR_VAL(hash),
								 (size_t) hash_len) != 0) {
		sodium_memzero(&state_tmp, sizeof state_tmp);
		crex_string_efree(hash);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	sodium_memzero(&state_tmp, sizeof state_tmp);
	sodium_memzero(state, state_len);
	convert_to_null(state_zv);
	ZSTR_VAL(hash)[hash_len] = 0;

	RETURN_NEW_STR(hash);
}

CRX_FUNCTION(sodium_crypto_box_keypair)
{
	crex_string *keypair;
	size_t       keypair_len;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	keypair_len = crypto_box_SECRETKEYBYTES + crypto_box_PUBLICKEYBYTES;
	keypair = crex_string_alloc(keypair_len, 0);
	if (crypto_box_keypair((unsigned char *) ZSTR_VAL(keypair) +
						   crypto_box_SECRETKEYBYTES,
						   (unsigned char *) ZSTR_VAL(keypair)) != 0) {
		crex_string_efree(keypair);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(keypair)[keypair_len] = 0;

	RETURN_NEW_STR(keypair);
}

CRX_FUNCTION(sodium_crypto_box_seed_keypair)
{
	crex_string   *keypair;
	unsigned char *seed;
	size_t         keypair_len;
	size_t         seed_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
									&seed, &seed_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (seed_len != crypto_box_SEEDBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_BOX_SEEDBYTES bytes long");
		RETURN_THROWS();
	}
	keypair_len = crypto_box_SECRETKEYBYTES + crypto_box_PUBLICKEYBYTES;
	keypair = crex_string_alloc(keypair_len, 0);
	if (crypto_box_seed_keypair((unsigned char *) ZSTR_VAL(keypair) +
								 crypto_box_SECRETKEYBYTES,
								 (unsigned char *) ZSTR_VAL(keypair),
								 seed) != 0) {
		crex_string_efree(keypair);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(keypair)[keypair_len] = 0;

	RETURN_NEW_STR(keypair);
}

CRX_FUNCTION(sodium_crypto_box_keypair_from_secretkey_and_publickey)
{
	crex_string *keypair;
	char        *publickey;
	char        *secretkey;
	size_t       keypair_len;
	size_t       publickey_len;
	size_t       secretkey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&secretkey, &secretkey_len,
									&publickey, &publickey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (secretkey_len != crypto_box_SECRETKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_BOX_SECRETKEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (publickey_len != crypto_box_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_BOX_PUBLICKEYBYTES bytes long");
		RETURN_THROWS();
	}
	keypair_len = crypto_box_SECRETKEYBYTES + crypto_box_PUBLICKEYBYTES;
	keypair = crex_string_alloc(keypair_len, 0);
	memcpy(ZSTR_VAL(keypair), secretkey, crypto_box_SECRETKEYBYTES);
	memcpy(ZSTR_VAL(keypair) + crypto_box_SECRETKEYBYTES, publickey,
		   crypto_box_PUBLICKEYBYTES);
	ZSTR_VAL(keypair)[keypair_len] = 0;

	RETURN_STR(keypair);
}

CRX_FUNCTION(sodium_crypto_box_secretkey)
{
	crex_string   *secretkey;
	unsigned char *keypair;
	size_t         keypair_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
									&keypair, &keypair_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (keypair_len !=
		crypto_box_SECRETKEYBYTES + crypto_box_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_BOX_KEYPAIRBYTES bytes long");
		RETURN_THROWS();
	}
	secretkey = crex_string_alloc(crypto_box_SECRETKEYBYTES, 0);
	memcpy(ZSTR_VAL(secretkey), keypair, crypto_box_SECRETKEYBYTES);
	ZSTR_VAL(secretkey)[crypto_box_SECRETKEYBYTES] = 0;

	RETURN_STR(secretkey);
}

CRX_FUNCTION(sodium_crypto_box_publickey)
{
	crex_string   *publickey;
	unsigned char *keypair;
	size_t         keypair_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
									&keypair, &keypair_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (keypair_len !=
		crypto_box_SECRETKEYBYTES + crypto_box_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_BOX_KEYPAIRBYTES bytes long");
		RETURN_THROWS();
	}
	publickey = crex_string_alloc(crypto_box_PUBLICKEYBYTES, 0);
	memcpy(ZSTR_VAL(publickey), keypair + crypto_box_SECRETKEYBYTES,
		   crypto_box_PUBLICKEYBYTES);
	ZSTR_VAL(publickey)[crypto_box_PUBLICKEYBYTES] = 0;

	RETURN_STR(publickey);
}

CRX_FUNCTION(sodium_crypto_box_publickey_from_secretkey)
{
	crex_string   *publickey;
	unsigned char *secretkey;
	size_t         secretkey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
									&secretkey, &secretkey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (secretkey_len != crypto_box_SECRETKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_BOX_SECRETKEYBYTES bytes long");
		RETURN_THROWS();
	}
	publickey = crex_string_alloc(crypto_box_PUBLICKEYBYTES, 0);
	(void) sizeof(int[crypto_scalarmult_BYTES ==
					  crypto_box_PUBLICKEYBYTES ? 1 : -1]);
	(void) sizeof(int[crypto_scalarmult_SCALARBYTES ==
					  crypto_box_SECRETKEYBYTES ? 1 : -1]);
	crypto_scalarmult_base((unsigned char *) ZSTR_VAL(publickey), secretkey);
	ZSTR_VAL(publickey)[crypto_box_PUBLICKEYBYTES] = 0;

	RETURN_STR(publickey);
}

CRX_FUNCTION(sodium_crypto_box)
{
	crex_string   *ciphertext;
	unsigned char *keypair;
	unsigned char *msg;
	unsigned char *nonce;
	unsigned char *publickey;
	unsigned char *secretkey;
	size_t         keypair_len;
	size_t         msg_len;
	size_t         nonce_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sss",
									&msg, &msg_len,
									&nonce, &nonce_len,
									&keypair, &keypair_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (nonce_len != crypto_box_NONCEBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_BOX_NONCEBYTES bytes long");
		RETURN_THROWS();
	}
	if (keypair_len != crypto_box_SECRETKEYBYTES + crypto_box_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_BOX_KEYPAIRBYTES bytes long");
		RETURN_THROWS();
	}
	secretkey = keypair;
	publickey = keypair + crypto_box_SECRETKEYBYTES;
	if (SIZE_MAX - msg_len <= crypto_box_MACBYTES) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	ciphertext = crex_string_alloc((size_t) msg_len + crypto_box_MACBYTES, 0);
	if (crypto_box_easy((unsigned char *) ZSTR_VAL(ciphertext), msg,
						(unsigned long long) msg_len,
						nonce, publickey, secretkey) != 0) {
		crex_string_efree(ciphertext);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(ciphertext)[msg_len + crypto_box_MACBYTES] = 0;

	RETURN_NEW_STR(ciphertext);
}

CRX_FUNCTION(sodium_crypto_box_open)
{
	crex_string   *msg;
	unsigned char *ciphertext;
	unsigned char *keypair;
	unsigned char *nonce;
	unsigned char *publickey;
	unsigned char *secretkey;
	size_t         ciphertext_len;
	size_t         keypair_len;
	size_t         nonce_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sss",
									&ciphertext, &ciphertext_len,
									&nonce, &nonce_len,
									&keypair, &keypair_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (nonce_len != crypto_box_NONCEBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_BOX_NONCEBYTES bytes long");
		RETURN_THROWS();
	}
	if (keypair_len != crypto_box_SECRETKEYBYTES + crypto_box_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_BOX_KEYPAIRBYTES bytes long");
		RETURN_THROWS();
	}
	secretkey = keypair;
	publickey = keypair + crypto_box_SECRETKEYBYTES;
	if (ciphertext_len < crypto_box_MACBYTES) {
		RETURN_FALSE;
	}
	msg = crex_string_alloc((size_t) ciphertext_len - crypto_box_MACBYTES, 0);
	if (crypto_box_open_easy((unsigned char *) ZSTR_VAL(msg), ciphertext,
							 (unsigned long long) ciphertext_len,
							 nonce, publickey, secretkey) != 0) {
		crex_string_efree(msg);
		RETURN_FALSE;
	} else {
		ZSTR_VAL(msg)[ciphertext_len - crypto_box_MACBYTES] = 0;
		RETURN_NEW_STR(msg);
	}
}

CRX_FUNCTION(sodium_crypto_box_seal)
{
	crex_string   *ciphertext;
	unsigned char *msg;
	unsigned char *publickey;
	size_t         msg_len;
	size_t         publickey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&msg, &msg_len,
									&publickey, &publickey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (publickey_len != crypto_box_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_BOX_PUBLICKEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (SIZE_MAX - msg_len <= crypto_box_SEALBYTES) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	ciphertext = crex_string_alloc((size_t) msg_len + crypto_box_SEALBYTES, 0);
	if (crypto_box_seal((unsigned char *) ZSTR_VAL(ciphertext), msg,
						(unsigned long long) msg_len, publickey) != 0) {
		crex_string_efree(ciphertext);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(ciphertext)[msg_len + crypto_box_SEALBYTES] = 0;

	RETURN_NEW_STR(ciphertext);
}

CRX_FUNCTION(sodium_crypto_box_seal_open)
{
	crex_string   *msg;
	unsigned char *ciphertext;
	unsigned char *keypair;
	unsigned char *publickey;
	unsigned char *secretkey;
	size_t         ciphertext_len;
	size_t         keypair_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&ciphertext, &ciphertext_len,
									&keypair, &keypair_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (keypair_len != crypto_box_SECRETKEYBYTES + crypto_box_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_BOX_KEYPAIRBYTES bytes long");
		RETURN_THROWS();
	}
	secretkey = keypair;
	publickey = keypair + crypto_box_SECRETKEYBYTES;
	if (ciphertext_len < crypto_box_SEALBYTES) {
		RETURN_FALSE;
	}
	msg = crex_string_alloc((size_t) ciphertext_len - crypto_box_SEALBYTES, 0);
	if (crypto_box_seal_open((unsigned char *) ZSTR_VAL(msg), ciphertext,
							 (unsigned long long) ciphertext_len,
							 publickey, secretkey) != 0) {
		crex_string_efree(msg);
		RETURN_FALSE;
	} else {
		ZSTR_VAL(msg)[ciphertext_len - crypto_box_SEALBYTES] = 0;
		RETURN_NEW_STR(msg);
	}
}

CRX_FUNCTION(sodium_crypto_sign_keypair)
{
	crex_string *keypair;
	size_t       keypair_len;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	keypair_len = crypto_sign_SECRETKEYBYTES + crypto_sign_PUBLICKEYBYTES;
	keypair = crex_string_alloc(keypair_len, 0);
	if (crypto_sign_keypair((unsigned char *) ZSTR_VAL(keypair) +
							crypto_sign_SECRETKEYBYTES,
							(unsigned char *) ZSTR_VAL(keypair)) != 0) {
		crex_string_efree(keypair);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(keypair)[keypair_len] = 0;

	RETURN_NEW_STR(keypair);
}

CRX_FUNCTION(sodium_crypto_sign_seed_keypair)
{
	crex_string   *keypair;
	unsigned char *seed;
	size_t         keypair_len;
	size_t         seed_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
									&seed, &seed_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (seed_len != crypto_sign_SEEDBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_SIGN_SEEDBYTES bytes long");
		RETURN_THROWS();
	}
	keypair_len = crypto_sign_SECRETKEYBYTES + crypto_sign_PUBLICKEYBYTES;
	keypair = crex_string_alloc(keypair_len, 0);
	if (crypto_sign_seed_keypair((unsigned char *) ZSTR_VAL(keypair) +
								 crypto_sign_SECRETKEYBYTES,
								 (unsigned char *) ZSTR_VAL(keypair),
								 seed) != 0) {
		crex_string_efree(keypair);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(keypair)[keypair_len] = 0;

	RETURN_NEW_STR(keypair);
}

CRX_FUNCTION(sodium_crypto_sign_keypair_from_secretkey_and_publickey)
{
	crex_string *keypair;
	char        *publickey;
	char        *secretkey;
	size_t       keypair_len;
	size_t       publickey_len;
	size_t       secretkey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&secretkey, &secretkey_len,
									&publickey, &publickey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (secretkey_len != crypto_sign_SECRETKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_SIGN_SECRETKEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (publickey_len != crypto_sign_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_SIGN_PUBLICKEYBYTES bytes long");
		RETURN_THROWS();
	}
	keypair_len = crypto_sign_SECRETKEYBYTES + crypto_sign_PUBLICKEYBYTES;
	keypair = crex_string_alloc(keypair_len, 0);
	memcpy(ZSTR_VAL(keypair), secretkey, crypto_sign_SECRETKEYBYTES);
	memcpy(ZSTR_VAL(keypair) + crypto_sign_SECRETKEYBYTES, publickey,
		   crypto_sign_PUBLICKEYBYTES);
	ZSTR_VAL(keypair)[keypair_len] = 0;

	RETURN_STR(keypair);
}

CRX_FUNCTION(sodium_crypto_sign_publickey_from_secretkey)
{
	crex_string *publickey;
	char        *secretkey;
	size_t       secretkey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
									&secretkey, &secretkey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (secretkey_len != crypto_sign_SECRETKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_SIGN_SECRETKEYBYTES bytes long");
		RETURN_THROWS();
	}
	publickey = crex_string_alloc(crypto_sign_PUBLICKEYBYTES, 0);

	if (crypto_sign_ed25519_sk_to_pk((unsigned char *) ZSTR_VAL(publickey),
									 (const unsigned char *) secretkey) != 0) {
		crex_throw_exception(sodium_exception_ce,
				   "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(publickey)[crypto_sign_PUBLICKEYBYTES] = 0;

	RETURN_STR(publickey);
}

CRX_FUNCTION(sodium_crypto_sign_secretkey)
{
	crex_string   *secretkey;
	unsigned char *keypair;
	size_t         keypair_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
									&keypair, &keypair_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (keypair_len !=
		crypto_sign_SECRETKEYBYTES + crypto_sign_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_SIGN_KEYPAIRBYTES bytes long");
		RETURN_THROWS();
	}
	secretkey = crex_string_alloc(crypto_sign_SECRETKEYBYTES, 0);
	memcpy(ZSTR_VAL(secretkey), keypair, crypto_sign_SECRETKEYBYTES);
	ZSTR_VAL(secretkey)[crypto_sign_SECRETKEYBYTES] = 0;

	RETURN_STR(secretkey);
}

CRX_FUNCTION(sodium_crypto_sign_publickey)
{
	crex_string   *publickey;
	unsigned char *keypair;
	size_t         keypair_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
									&keypair, &keypair_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (keypair_len !=
		crypto_sign_SECRETKEYBYTES + crypto_sign_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_SIGN_KEYPAIRBYTES bytes long");
		RETURN_THROWS();
	}
	publickey = crex_string_alloc(crypto_sign_PUBLICKEYBYTES, 0);
	memcpy(ZSTR_VAL(publickey), keypair + crypto_sign_SECRETKEYBYTES,
		   crypto_sign_PUBLICKEYBYTES);
	ZSTR_VAL(publickey)[crypto_sign_PUBLICKEYBYTES] = 0;

	RETURN_STR(publickey);
}

CRX_FUNCTION(sodium_crypto_sign)
{
	crex_string        *msg_signed;
	unsigned char      *msg;
	unsigned char      *secretkey;
	unsigned long long  msg_signed_real_len;
	size_t              msg_len;
	size_t              msg_signed_len;
	size_t              secretkey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&msg, &msg_len,
									&secretkey, &secretkey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (secretkey_len != crypto_sign_SECRETKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_SIGN_SECRETKEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (SIZE_MAX - msg_len <= crypto_sign_BYTES) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	msg_signed_len = msg_len + crypto_sign_BYTES;
	msg_signed = crex_string_alloc((size_t) msg_signed_len, 0);
	if (crypto_sign((unsigned char *) ZSTR_VAL(msg_signed),
					&msg_signed_real_len, msg,
					(unsigned long long) msg_len, secretkey) != 0) {
		crex_string_efree(msg_signed);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	if (msg_signed_real_len >= SIZE_MAX || msg_signed_real_len > msg_signed_len) {
		crex_string_efree(msg_signed);
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	CRX_SODIUM_ZSTR_TRUNCATE(msg_signed, (size_t) msg_signed_real_len);
	ZSTR_VAL(msg_signed)[msg_signed_real_len] = 0;

	RETURN_NEW_STR(msg_signed);
}

CRX_FUNCTION(sodium_crypto_sign_open)
{
	crex_string        *msg;
	unsigned char      *msg_signed;
	unsigned char      *publickey;
	unsigned long long  msg_real_len;
	size_t              msg_len;
	size_t              msg_signed_len;
	size_t              publickey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&msg_signed, &msg_signed_len,
									&publickey, &publickey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (publickey_len != crypto_sign_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_SIGN_PUBLICKEYBYTES bytes long");
		RETURN_THROWS();
	}
	msg_len = msg_signed_len;
	if (msg_len >= SIZE_MAX) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	msg = crex_string_alloc((size_t) msg_len, 0);
	if (crypto_sign_open((unsigned char *) ZSTR_VAL(msg), &msg_real_len,
						 msg_signed, (unsigned long long) msg_signed_len,
						 publickey) != 0) {
		crex_string_efree(msg);
		RETURN_FALSE;
	}
	if (msg_real_len >= SIZE_MAX || msg_real_len > msg_signed_len) {
		crex_string_efree(msg);
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	CRX_SODIUM_ZSTR_TRUNCATE(msg, (size_t) msg_real_len);
	ZSTR_VAL(msg)[msg_real_len] = 0;

	RETURN_NEW_STR(msg);
}

CRX_FUNCTION(sodium_crypto_sign_detached)
{
	crex_string        *signature;
	unsigned char      *msg;
	unsigned char      *secretkey;
	unsigned long long  signature_real_len;
	size_t              msg_len;
	size_t              secretkey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&msg, &msg_len,
									&secretkey, &secretkey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (secretkey_len != crypto_sign_SECRETKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_SIGN_SECRETKEYBYTES bytes long");
		RETURN_THROWS();
	}
	signature = crex_string_alloc((size_t) crypto_sign_BYTES, 0);
	memset(ZSTR_VAL(signature), 0, (size_t) crypto_sign_BYTES);
	if (crypto_sign_detached((unsigned char *) ZSTR_VAL(signature),
							 &signature_real_len, msg,
							 (unsigned long long) msg_len, secretkey) != 0) {
		crex_string_efree(signature);
		crex_throw_exception(sodium_exception_ce, "signature creation failed", 0);
		RETURN_THROWS();
	}
	if (signature_real_len <= 0U || signature_real_len > crypto_sign_BYTES) {
		crex_string_efree(signature);
		crex_throw_exception(sodium_exception_ce, "signature has a bogus size", 0);
		RETURN_THROWS();
	}
	CRX_SODIUM_ZSTR_TRUNCATE(signature, (size_t) signature_real_len);
	ZSTR_VAL(signature)[signature_real_len] = 0;

	RETURN_NEW_STR(signature);
}

CRX_FUNCTION(sodium_crypto_sign_verify_detached)
{
	unsigned char *msg;
	unsigned char *publickey;
	unsigned char *signature;
	size_t         msg_len;
	size_t         publickey_len;
	size_t         signature_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sss",
									&signature, &signature_len,
									&msg, &msg_len,
									&publickey, &publickey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (signature_len != crypto_sign_BYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_SIGN_BYTES bytes long");
		RETURN_THROWS();
	}
	if (publickey_len != crypto_sign_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_SIGN_PUBLICKEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (crypto_sign_verify_detached(signature,
									msg, (unsigned long long) msg_len,
									publickey) != 0) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

CRX_FUNCTION(sodium_crypto_stream)
{
	crex_string   *ciphertext;
	unsigned char *key;
	unsigned char *nonce;
	crex_long      ciphertext_len;
	size_t         key_len;
	size_t         nonce_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "lss",
									&ciphertext_len,
									&nonce, &nonce_len,
									&key, &key_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (ciphertext_len <= 0 || ciphertext_len >= SIZE_MAX) {
		crex_argument_error(sodium_exception_ce, 1, "must be greater than 0");
		RETURN_THROWS();
	}
	if (nonce_len != crypto_stream_NONCEBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_STREAM_NONCEBYTES bytes long");
		RETURN_THROWS();
	}
	if (key_len != crypto_stream_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_STREAM_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	ciphertext = crex_string_alloc((size_t) ciphertext_len, 0);
	if (crypto_stream((unsigned char *) ZSTR_VAL(ciphertext),
					  (unsigned long long) ciphertext_len, nonce, key) != 0) {
		crex_string_efree(ciphertext);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(ciphertext)[ciphertext_len] = 0;

	RETURN_NEW_STR(ciphertext);
}

CRX_FUNCTION(sodium_crypto_stream_xor)
{
	crex_string   *ciphertext;
	unsigned char *key;
	unsigned char *msg;
	unsigned char *nonce;
	size_t         ciphertext_len;
	size_t         key_len;
	size_t         msg_len;
	size_t         nonce_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sss",
									&msg, &msg_len,
									&nonce, &nonce_len,
									&key, &key_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (nonce_len != crypto_stream_NONCEBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_STREAM_NONCEBYTES bytes long");
		RETURN_THROWS();
	}
	if (key_len != crypto_stream_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_STREAM_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	ciphertext_len = msg_len;
	ciphertext = crex_string_alloc((size_t) ciphertext_len, 0);
	if (crypto_stream_xor((unsigned char *) ZSTR_VAL(ciphertext), msg,
						  (unsigned long long) msg_len, nonce, key) != 0) {
		crex_string_efree(ciphertext);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(ciphertext)[ciphertext_len] = 0;

	RETURN_NEW_STR(ciphertext);
}

#ifdef crypto_stream_xchacha20_KEYBYTES
CRX_FUNCTION(sodium_crypto_stream_xchacha20)
{
	crex_string   *ciphertext;
	unsigned char *key;
	unsigned char *nonce;
	crex_long      ciphertext_len;
	size_t         key_len;
	size_t         nonce_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "lss",
									&ciphertext_len,
									&nonce, &nonce_len,
									&key, &key_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (ciphertext_len <= 0 || ciphertext_len >= SIZE_MAX) {
		crex_argument_error(sodium_exception_ce, 1, "must be greater than 0");
		RETURN_THROWS();
	}
	if (nonce_len != crypto_stream_xchacha20_NONCEBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_STREAM_XCHACHA20_NONCEBYTES bytes long");
		RETURN_THROWS();
	}
	if (key_len != crypto_stream_xchacha20_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_STREAM_XCHACHA20_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	ciphertext = crex_string_checked_alloc((size_t) ciphertext_len, 0);
	if (crypto_stream_xchacha20((unsigned char *) ZSTR_VAL(ciphertext),
								(unsigned long long) ciphertext_len, nonce, key) != 0) {
		crex_string_free(ciphertext);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(ciphertext)[ciphertext_len] = 0;

	RETURN_NEW_STR(ciphertext);
}

CRX_FUNCTION(sodium_crypto_stream_xchacha20_xor)
{
	crex_string   *ciphertext;
	unsigned char *key;
	unsigned char *msg;
	unsigned char *nonce;
	size_t         ciphertext_len;
	size_t         key_len;
	size_t         msg_len;
	size_t         nonce_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sss",
									&msg, &msg_len,
									&nonce, &nonce_len,
									&key, &key_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (nonce_len != crypto_stream_xchacha20_NONCEBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_STREAM_XCHACHA20_NONCEBYTES bytes long");
		RETURN_THROWS();
	}
	if (key_len != crypto_stream_xchacha20_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_STREAM_XCHACHA20_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	ciphertext_len = msg_len;
	ciphertext = crex_string_checked_alloc((size_t) ciphertext_len, 0);
	if (crypto_stream_xchacha20_xor((unsigned char *) ZSTR_VAL(ciphertext), msg,
									(unsigned long long) msg_len, nonce, key) != 0) {
		crex_string_free(ciphertext);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(ciphertext)[ciphertext_len] = 0;

	RETURN_NEW_STR(ciphertext);
}

CRX_FUNCTION(sodium_crypto_stream_xchacha20_xor_ic)
{
	crex_string   *ciphertext;
	unsigned char *key;
	unsigned char *msg;
	unsigned char *nonce;
	crex_long      ic;

	size_t         ciphertext_len;
	size_t         key_len;
	size_t         msg_len;
	size_t         nonce_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ssls",
									&msg, &msg_len,
									&nonce, &nonce_len,
									&ic,
									&key, &key_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (nonce_len != crypto_stream_xchacha20_NONCEBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_STREAM_XCHACHA20_NONCEBYTES bytes long");
		RETURN_THROWS();
	}
	if (key_len != crypto_stream_xchacha20_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_STREAM_XCHACHA20_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	ciphertext_len = msg_len;
	ciphertext = crex_string_checked_alloc((size_t) ciphertext_len, 0);
	if (crypto_stream_xchacha20_xor_ic((unsigned char *) ZSTR_VAL(ciphertext), msg,
									   (unsigned long long) msg_len, nonce,
									   (uint64_t) ic, key) != 0) {
		crex_string_free(ciphertext);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(ciphertext)[ciphertext_len] = 0;

	RETURN_NEW_STR(ciphertext);
}
#endif

#ifdef crypto_pwhash_SALTBYTES
CRX_FUNCTION(sodium_crypto_pwhash)
{
	crex_string   *hash;
	unsigned char *salt;
	char          *passwd;
	crex_long      hash_len;
	crex_long      memlimit;
	crex_long      opslimit;
	crex_long      alg;
	size_t         passwd_len;
	size_t         salt_len;
	int            ret;

	alg = (crex_long) crypto_pwhash_ALG_DEFAULT;
	if (crex_parse_parameters(CREX_NUM_ARGS(), "lssll|l",
									&hash_len,
									&passwd, &passwd_len,
									&salt, &salt_len,
									&opslimit, &memlimit, &alg) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (hash_len <= 0) {
		crex_argument_error(sodium_exception_ce, 1, "must be greater than 0");
		RETURN_THROWS();
	}
	if (hash_len >= 0xffffffff) {
		crex_argument_error(sodium_exception_ce, 1, "is too large");
		RETURN_THROWS();
	}
	if (passwd_len >= 0xffffffff) {
		crex_argument_error(sodium_exception_ce, 2, "is too long");
		RETURN_THROWS();
	}
	if (opslimit <= 0) {
		crex_argument_error(sodium_exception_ce, 4, "must be greater than 0");
		RETURN_THROWS();
	}
	if (memlimit <= 0 || memlimit > SIZE_MAX) {
		crex_argument_error(sodium_exception_ce, 5, "must be greater than 0");
		RETURN_THROWS();
	}
	if (alg != crypto_pwhash_ALG_ARGON2I13
# ifdef crypto_pwhash_ALG_ARGON2ID13
		&& alg != crypto_pwhash_ALG_ARGON2ID13
# endif
		&& alg != crypto_pwhash_ALG_DEFAULT) {
		crex_throw_exception(sodium_exception_ce, "unsupported password hashing algorithm", 0);
		RETURN_THROWS();
	}
	if (passwd_len <= 0) {
		crex_error(E_WARNING, "empty password");
	}
	if (salt_len != crypto_pwhash_SALTBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_PWHASH_SALTBYTES bytes long");
		RETURN_THROWS();
	}
	if (opslimit < crypto_pwhash_OPSLIMIT_MIN) {
		crex_argument_error(sodium_exception_ce, 4, "must be greater than or equal to %d", crypto_pwhash_OPSLIMIT_MIN);
		RETURN_THROWS();
	}
	if (memlimit < crypto_pwhash_MEMLIMIT_MIN) {
		crex_argument_error(sodium_exception_ce, 5, "must be greater than or equal to %d", crypto_pwhash_MEMLIMIT_MIN);
	}
	hash = crex_string_alloc((size_t) hash_len, 0);
	ret = -1;
# ifdef crypto_pwhash_ALG_ARGON2ID13
	if (alg == crypto_pwhash_ALG_ARGON2ID13) {
		ret = crypto_pwhash_argon2id
			((unsigned char *) ZSTR_VAL(hash), (unsigned long long) hash_len,
			  passwd, (unsigned long long) passwd_len, salt,
			  (unsigned long long) opslimit, (size_t) memlimit, (int) alg);
	}
# endif
	if (ret == -1) {
		ret = crypto_pwhash
			((unsigned char *) ZSTR_VAL(hash), (unsigned long long) hash_len,
			  passwd, (unsigned long long) passwd_len, salt,
			  (unsigned long long) opslimit, (size_t) memlimit, (int) alg);
	}
	if (ret != 0) {
		crex_string_efree(hash);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(hash)[hash_len] = 0;

	RETURN_NEW_STR(hash);
}

CRX_FUNCTION(sodium_crypto_pwhash_str)
{
	crex_string *hash_str;
	char        *passwd;
	crex_long    memlimit;
	crex_long    opslimit;
	size_t       passwd_len;
	size_t       len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sll",
									&passwd, &passwd_len,
									&opslimit, &memlimit) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (opslimit <= 0) {
		crex_argument_error(sodium_exception_ce, 2, "must be greater than 0");
		RETURN_THROWS();
	}
	if (memlimit <= 0 || memlimit > SIZE_MAX) {
		crex_argument_error(sodium_exception_ce, 3, "must be greater than 0");
		RETURN_THROWS();
	}
	if (passwd_len >= 0xffffffff) {
		crex_argument_error(sodium_exception_ce, 1, "is too long");
		RETURN_THROWS();
	}
	if (passwd_len <= 0) {
		crex_error(E_WARNING, "empty password");
	}
	if (opslimit < crypto_pwhash_OPSLIMIT_MIN) {
		crex_argument_error(sodium_exception_ce, 2, "must be greater than or equal to %d", crypto_pwhash_OPSLIMIT_MIN);
	}
	if (memlimit < crypto_pwhash_MEMLIMIT_MIN) {
		crex_argument_error(sodium_exception_ce, 3, "must be greater than or equal to %d", crypto_pwhash_MEMLIMIT_MIN);
	}
	hash_str = crex_string_alloc(crypto_pwhash_STRBYTES - 1, 0);
	if (crypto_pwhash_str
		(ZSTR_VAL(hash_str), passwd, (unsigned long long) passwd_len,
		 (unsigned long long) opslimit, (size_t) memlimit) != 0) {
		crex_string_efree(hash_str);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(hash_str)[crypto_pwhash_STRBYTES - 1] = 0;

	len = strlen(ZSTR_VAL(hash_str));
	CRX_SODIUM_ZSTR_TRUNCATE(hash_str, len);

	RETURN_NEW_STR(hash_str);
}

#if SODIUM_LIBRARY_VERSION_MAJOR > 9 || (SODIUM_LIBRARY_VERSION_MAJOR == 9 && SODIUM_LIBRARY_VERSION_MINOR >= 6)
CRX_FUNCTION(sodium_crypto_pwhash_str_needs_rehash)
{
	char      *hash_str;
	crex_long  memlimit;
	crex_long  opslimit;
	size_t     hash_str_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sll",
									&hash_str, &hash_str_len, &opslimit, &memlimit) == FAILURE) {
		RETURN_THROWS();
	}
	if (crypto_pwhash_str_needs_rehash(hash_str, opslimit, memlimit) == 0) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
#endif

CRX_FUNCTION(sodium_crypto_pwhash_str_verify)
{
	char      *hash_str;
	char      *passwd;
	size_t     hash_str_len;
	size_t     passwd_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&hash_str, &hash_str_len,
									&passwd, &passwd_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (passwd_len >= 0xffffffff) {
		crex_argument_error(sodium_exception_ce, 2, "is too long");
		RETURN_THROWS();
	}
	if (passwd_len <= 0) {
		crex_error(E_WARNING, "empty password");
	}
	if (crypto_pwhash_str_verify
		(hash_str, passwd, (unsigned long long) passwd_len) == 0) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
#endif

#ifdef crypto_pwhash_scryptsalsa208sha256_SALTBYTES
CRX_FUNCTION(sodium_crypto_pwhash_scryptsalsa208sha256)
{
	crex_string   *hash;
	unsigned char *salt;
	char          *passwd;
	crex_long      hash_len;
	crex_long      memlimit;
	crex_long      opslimit;
	size_t         passwd_len;
	size_t         salt_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "lssll",
									&hash_len,
									&passwd, &passwd_len,
									&salt, &salt_len,
									&opslimit, &memlimit) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (hash_len <= 0 || hash_len >= SIZE_MAX || hash_len > 0x1fffffffe0ULL) {
		crex_argument_error(sodium_exception_ce, 1, "must be greater than 0");
		RETURN_THROWS();
	}
	if (opslimit <= 0) {
		crex_argument_error(sodium_exception_ce, 4, "must be greater than 0");
		RETURN_THROWS();
	}
	if (memlimit <= 0 || memlimit > SIZE_MAX) {
		crex_argument_error(sodium_exception_ce, 5, "must be greater than 0");
		RETURN_THROWS();
	}
	if (passwd_len <= 0) {
		crex_error(E_WARNING, "empty password");
	}
	if (salt_len != crypto_pwhash_scryptsalsa208sha256_SALTBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_PWHASH_SCRYPTSALSA208SHA256_SALTBYTES bytes long");
		RETURN_THROWS();
	}
	if (opslimit < crypto_pwhash_scryptsalsa208sha256_OPSLIMIT_INTERACTIVE) {
		crex_argument_error(sodium_exception_ce, 4, "must be greater than or equal to %d", crypto_pwhash_scryptsalsa208sha256_OPSLIMIT_INTERACTIVE);
	}
	if (memlimit < crypto_pwhash_scryptsalsa208sha256_MEMLIMIT_INTERACTIVE) {
		crex_argument_error(sodium_exception_ce, 5, "must be greater than or equal to %d", crypto_pwhash_scryptsalsa208sha256_MEMLIMIT_INTERACTIVE);
	}
	hash = crex_string_alloc((size_t) hash_len, 0);
	if (crypto_pwhash_scryptsalsa208sha256
		((unsigned char *) ZSTR_VAL(hash), (unsigned long long) hash_len,
		 passwd, (unsigned long long) passwd_len, salt,
		 (unsigned long long) opslimit, (size_t) memlimit) != 0) {
		crex_string_efree(hash);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(hash)[hash_len] = 0;

	RETURN_NEW_STR(hash);
}

CRX_FUNCTION(sodium_crypto_pwhash_scryptsalsa208sha256_str)
{
	crex_string *hash_str;
	char        *passwd;
	crex_long    memlimit;
	crex_long    opslimit;
	size_t       passwd_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sll",
									&passwd, &passwd_len,
									&opslimit, &memlimit) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (opslimit <= 0) {
		crex_argument_error(sodium_exception_ce, 2, "must be greater than 0");
		RETURN_THROWS();
	}
	if (memlimit <= 0 || memlimit > SIZE_MAX) {
		crex_argument_error(sodium_exception_ce, 3, "must be greater than 0");
		RETURN_THROWS();
	}
	if (passwd_len <= 0) {
		crex_error(E_WARNING, "empty password");
	}
	if (opslimit < crypto_pwhash_scryptsalsa208sha256_OPSLIMIT_INTERACTIVE) {
		crex_argument_error(sodium_exception_ce, 2, "must be greater than or equal to %d", crypto_pwhash_scryptsalsa208sha256_OPSLIMIT_INTERACTIVE);
	}
	if (memlimit < crypto_pwhash_scryptsalsa208sha256_MEMLIMIT_INTERACTIVE) {
		crex_argument_error(sodium_exception_ce, 3, "must be greater than or equal to %d", crypto_pwhash_scryptsalsa208sha256_MEMLIMIT_INTERACTIVE);
	}
	hash_str = crex_string_alloc
		(crypto_pwhash_scryptsalsa208sha256_STRBYTES - 1, 0);
	if (crypto_pwhash_scryptsalsa208sha256_str
		(ZSTR_VAL(hash_str), passwd, (unsigned long long) passwd_len,
		 (unsigned long long) opslimit, (size_t) memlimit) != 0) {
		crex_string_efree(hash_str);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(hash_str)[crypto_pwhash_scryptsalsa208sha256_STRBYTES - 1] = 0;

	RETURN_NEW_STR(hash_str);
}

CRX_FUNCTION(sodium_crypto_pwhash_scryptsalsa208sha256_str_verify)
{
	char      *hash_str;
	char      *passwd;
	size_t     hash_str_len;
	size_t     passwd_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&hash_str, &hash_str_len,
									&passwd, &passwd_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (passwd_len <= 0) {
		crex_error(E_WARNING, "empty password");
	}
	if (hash_str_len != crypto_pwhash_scryptsalsa208sha256_STRBYTES - 1) {
		crex_error(E_WARNING, "wrong size for the hashed password");
		RETURN_FALSE;
	}
	if (crypto_pwhash_scryptsalsa208sha256_str_verify
		(hash_str, passwd, (unsigned long long) passwd_len) == 0) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
#endif

CRX_FUNCTION(sodium_crypto_aead_aes256gcm_is_available)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
#ifdef HAVE_AESGCM
	RETURN_BOOL(crypto_aead_aes256gcm_is_available());
#else
	RETURN_FALSE;
#endif
}

#ifdef HAVE_AESGCM
CRX_FUNCTION(sodium_crypto_aead_aes256gcm_encrypt)
{
	crex_string        *ciphertext;
	unsigned char      *ad;
	unsigned char      *msg;
	unsigned char      *npub;
	unsigned char      *secretkey;
	unsigned long long  ciphertext_real_len;
	size_t              ad_len;
	size_t              ciphertext_len;
	size_t              msg_len;
	size_t              npub_len;
	size_t              secretkey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ssss",
									&msg, &msg_len,
									&ad, &ad_len,
									&npub, &npub_len,
									&secretkey, &secretkey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (npub_len != crypto_aead_aes256gcm_NPUBBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_AEAD_AES256GCM_NPUBBYTES bytes long");
		RETURN_THROWS();
	}
	if (secretkey_len != crypto_aead_aes256gcm_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 4, "must be SODIUM_CRYPTO_AEAD_AES256GCM_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (SIZE_MAX - msg_len <= crypto_aead_aes256gcm_ABYTES) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	if ((unsigned long long) msg_len > (16ULL * ((1ULL << 32) - 2ULL)) - crypto_aead_aes256gcm_ABYTES) {
		crex_throw_exception(sodium_exception_ce, "message too long for a single key", 0);
		RETURN_THROWS();
	}
	ciphertext_len = msg_len + crypto_aead_aes256gcm_ABYTES;
	ciphertext = crex_string_alloc((size_t) ciphertext_len, 0);
	if (crypto_aead_aes256gcm_encrypt
		((unsigned char *) ZSTR_VAL(ciphertext), &ciphertext_real_len, msg,
		 (unsigned long long) msg_len,
		 ad, (unsigned long long) ad_len, NULL, npub, secretkey) != 0) {
		crex_string_efree(ciphertext);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	if (ciphertext_real_len <= 0U || ciphertext_real_len >= SIZE_MAX ||
		ciphertext_real_len > ciphertext_len) {
		crex_string_efree(ciphertext);
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	CRX_SODIUM_ZSTR_TRUNCATE(ciphertext, (size_t) ciphertext_real_len);
	ZSTR_VAL(ciphertext)[ciphertext_real_len] = 0;

	RETURN_NEW_STR(ciphertext);
}

CRX_FUNCTION(sodium_crypto_aead_aes256gcm_decrypt)
{
	crex_string        *msg;
	unsigned char      *ad;
	unsigned char      *ciphertext;
	unsigned char      *npub;
	unsigned char      *secretkey;
	unsigned long long  msg_real_len;
	size_t              ad_len;
	size_t              ciphertext_len;
	size_t              msg_len;
	size_t              npub_len;
	size_t              secretkey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ssss",
									&ciphertext, &ciphertext_len,
									&ad, &ad_len,
									&npub, &npub_len,
									&secretkey, &secretkey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (npub_len != crypto_aead_aes256gcm_NPUBBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_AEAD_AES256GCM_NPUBBYTES bytes long");
		RETURN_THROWS();
	}
	if (secretkey_len != crypto_aead_aes256gcm_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 4, "must be SODIUM_CRYPTO_AEAD_AES256GCM_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (ciphertext_len < crypto_aead_aes256gcm_ABYTES) {
		RETURN_FALSE;
	}
	if (ciphertext_len - crypto_aead_aes256gcm_ABYTES > 16ULL * ((1ULL << 32) - 2ULL)) {
		crex_argument_error(sodium_exception_ce, 1, "is too long");
		RETURN_THROWS();
	}
	msg_len = ciphertext_len;
	if (msg_len >= SIZE_MAX) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	msg = crex_string_alloc((size_t) msg_len, 0);
	if (crypto_aead_aes256gcm_decrypt
		((unsigned char *) ZSTR_VAL(msg), &msg_real_len, NULL,
		 ciphertext, (unsigned long long) ciphertext_len,
		 ad, (unsigned long long) ad_len, npub, secretkey) != 0) {
		crex_string_efree(msg);
		RETURN_FALSE;
	}
	if (msg_real_len >= SIZE_MAX || msg_real_len > msg_len) {
		crex_string_efree(msg);
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	CRX_SODIUM_ZSTR_TRUNCATE(msg, (size_t) msg_real_len);
	ZSTR_VAL(msg)[msg_real_len] = 0;

	RETURN_NEW_STR(msg);
}
#endif

CRX_FUNCTION(sodium_crypto_aead_chacha20poly1305_encrypt)
{
	crex_string        *ciphertext;
	unsigned char      *ad;
	unsigned char      *msg;
	unsigned char      *npub;
	unsigned char      *secretkey;
	unsigned long long  ciphertext_real_len;
	size_t              ad_len;
	size_t              ciphertext_len;
	size_t              msg_len;
	size_t              npub_len;
	size_t              secretkey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ssss",
									&msg, &msg_len,
									&ad, &ad_len,
									&npub, &npub_len,
									&secretkey, &secretkey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (npub_len != crypto_aead_chacha20poly1305_NPUBBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_NPUBBYTES bytes long");
		RETURN_THROWS();
	}
	if (secretkey_len != crypto_aead_chacha20poly1305_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 4, "must be SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (SIZE_MAX - msg_len <= crypto_aead_chacha20poly1305_ABYTES) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	ciphertext_len = msg_len + crypto_aead_chacha20poly1305_ABYTES;
	ciphertext = crex_string_alloc((size_t) ciphertext_len, 0);
	if (crypto_aead_chacha20poly1305_encrypt
		((unsigned char *) ZSTR_VAL(ciphertext), &ciphertext_real_len, msg,
		 (unsigned long long) msg_len,
		 ad, (unsigned long long) ad_len, NULL, npub, secretkey) != 0) {
		crex_string_efree(ciphertext);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	if (ciphertext_real_len <= 0U || ciphertext_real_len >= SIZE_MAX ||
		ciphertext_real_len > ciphertext_len) {
		crex_string_efree(ciphertext);
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	CRX_SODIUM_ZSTR_TRUNCATE(ciphertext, (size_t) ciphertext_real_len);
	ZSTR_VAL(ciphertext)[ciphertext_real_len] = 0;

	RETURN_NEW_STR(ciphertext);
}

CRX_FUNCTION(sodium_crypto_aead_chacha20poly1305_decrypt)
{
	crex_string        *msg;
	unsigned char      *ad;
	unsigned char      *ciphertext;
	unsigned char      *npub;
	unsigned char      *secretkey;
	unsigned long long  msg_real_len;
	size_t              ad_len;
	size_t              ciphertext_len;
	size_t              msg_len;
	size_t              npub_len;
	size_t              secretkey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ssss",
									&ciphertext, &ciphertext_len,
									&ad, &ad_len,
									&npub, &npub_len,
									&secretkey, &secretkey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (npub_len != crypto_aead_chacha20poly1305_NPUBBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_NPUBBYTES bytes long");
		RETURN_THROWS();
	}
	if (secretkey_len != crypto_aead_chacha20poly1305_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 4, "must be SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (ciphertext_len < crypto_aead_chacha20poly1305_ABYTES) {
		RETURN_FALSE;
	}
	msg_len = ciphertext_len;
	if (msg_len >= SIZE_MAX) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	msg = crex_string_alloc((size_t) msg_len, 0);
	if (crypto_aead_chacha20poly1305_decrypt
		((unsigned char *) ZSTR_VAL(msg), &msg_real_len, NULL,
		 ciphertext, (unsigned long long) ciphertext_len,
		 ad, (unsigned long long) ad_len, npub, secretkey) != 0) {
		crex_string_efree(msg);
		RETURN_FALSE;
	}
	if (msg_real_len >= SIZE_MAX || msg_real_len > msg_len) {
		crex_string_efree(msg);
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	CRX_SODIUM_ZSTR_TRUNCATE(msg, (size_t) msg_real_len);
	ZSTR_VAL(msg)[msg_real_len] = 0;

	RETURN_NEW_STR(msg);
}

CRX_FUNCTION(sodium_crypto_aead_chacha20poly1305_ietf_encrypt)
{
	crex_string        *ciphertext;
	unsigned char      *ad;
	unsigned char      *msg;
	unsigned char      *npub;
	unsigned char      *secretkey;
	unsigned long long  ciphertext_real_len;
	size_t              ad_len;
	size_t              ciphertext_len;
	size_t              msg_len;
	size_t              npub_len;
	size_t              secretkey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ssss",
									&msg, &msg_len,
									&ad, &ad_len,
									&npub, &npub_len,
									&secretkey, &secretkey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (npub_len != crypto_aead_chacha20poly1305_IETF_NPUBBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_IETF_NPUBBYTES bytes long");
		RETURN_THROWS();
	}
	if (secretkey_len != crypto_aead_chacha20poly1305_IETF_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 4, "must be SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_IETF_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (SIZE_MAX - msg_len <= crypto_aead_chacha20poly1305_IETF_ABYTES) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	if ((unsigned long long) msg_len > 64ULL * (1ULL << 32) - 64ULL) {
		crex_throw_exception(sodium_exception_ce, "message too long for a single key", 0);
		RETURN_THROWS();
	}
	ciphertext_len = msg_len + crypto_aead_chacha20poly1305_IETF_ABYTES;
	ciphertext = crex_string_alloc((size_t) ciphertext_len, 0);
	if (crypto_aead_chacha20poly1305_ietf_encrypt
		((unsigned char *) ZSTR_VAL(ciphertext), &ciphertext_real_len, msg,
		 (unsigned long long) msg_len,
		 ad, (unsigned long long) ad_len, NULL, npub, secretkey) != 0) {
		crex_string_efree(ciphertext);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	if (ciphertext_real_len <= 0U || ciphertext_real_len >= SIZE_MAX ||
		ciphertext_real_len > ciphertext_len) {
		crex_string_efree(ciphertext);
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	CRX_SODIUM_ZSTR_TRUNCATE(ciphertext, (size_t) ciphertext_real_len);
	ZSTR_VAL(ciphertext)[ciphertext_real_len] = 0;

	RETURN_NEW_STR(ciphertext);
}

CRX_FUNCTION(sodium_crypto_aead_chacha20poly1305_ietf_decrypt)
{
	crex_string        *msg;
	unsigned char      *ad;
	unsigned char      *ciphertext;
	unsigned char      *npub;
	unsigned char      *secretkey;
	unsigned long long  msg_real_len;
	size_t              ad_len;
	size_t              ciphertext_len;
	size_t              msg_len;
	size_t              npub_len;
	size_t              secretkey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ssss",
									&ciphertext, &ciphertext_len,
									&ad, &ad_len,
									&npub, &npub_len,
									&secretkey, &secretkey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (npub_len != crypto_aead_chacha20poly1305_IETF_NPUBBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_IETF_NPUBBYTES bytes long");
		RETURN_THROWS();
	}
	if (secretkey_len != crypto_aead_chacha20poly1305_IETF_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 4, "must be SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_IETF_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	msg_len = ciphertext_len;
	if (msg_len >= SIZE_MAX) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	if (ciphertext_len < crypto_aead_chacha20poly1305_IETF_ABYTES) {
		RETURN_FALSE;
	}
	if ((unsigned long long) ciphertext_len -
		crypto_aead_chacha20poly1305_IETF_ABYTES > 64ULL * (1ULL << 32) - 64ULL) {
		crex_throw_exception(sodium_exception_ce, "message too long for a single key", 0);
		RETURN_THROWS();
	}
	msg = crex_string_alloc((size_t) msg_len, 0);
	if (crypto_aead_chacha20poly1305_ietf_decrypt
		((unsigned char *) ZSTR_VAL(msg), &msg_real_len, NULL,
		 ciphertext, (unsigned long long) ciphertext_len,
		 ad, (unsigned long long) ad_len, npub, secretkey) != 0) {
		crex_string_efree(msg);
		RETURN_FALSE;
	}
	if (msg_real_len >= SIZE_MAX || msg_real_len > msg_len) {
		crex_string_efree(msg);
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	CRX_SODIUM_ZSTR_TRUNCATE(msg, (size_t) msg_real_len);
	ZSTR_VAL(msg)[msg_real_len] = 0;

	RETURN_NEW_STR(msg);
}

#ifdef crypto_aead_xchacha20poly1305_IETF_NPUBBYTES
CRX_FUNCTION(sodium_crypto_aead_xchacha20poly1305_ietf_encrypt)
{
	crex_string        *ciphertext;
	unsigned char      *ad;
	unsigned char      *msg;
	unsigned char      *npub;
	unsigned char      *secretkey;
	unsigned long long  ciphertext_real_len;
	size_t              ad_len;
	size_t              ciphertext_len;
	size_t              msg_len;
	size_t              npub_len;
	size_t              secretkey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ssss",
									&msg, &msg_len,
									&ad, &ad_len,
									&npub, &npub_len,
									&secretkey, &secretkey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (npub_len != crypto_aead_xchacha20poly1305_IETF_NPUBBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_AEAD_XCHACHA20POLY1305_IETF_NPUBBYTES bytes long");
		RETURN_THROWS();
	}
	if (secretkey_len != crypto_aead_xchacha20poly1305_IETF_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 4, "must be SODIUM_CRYPTO_AEAD_XCHACHA20POLY1305_IETF_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (SIZE_MAX - msg_len <= crypto_aead_xchacha20poly1305_IETF_ABYTES) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	ciphertext_len = msg_len + crypto_aead_xchacha20poly1305_IETF_ABYTES;
	ciphertext = crex_string_alloc((size_t) ciphertext_len, 0);
	if (crypto_aead_xchacha20poly1305_ietf_encrypt
		((unsigned char *) ZSTR_VAL(ciphertext), &ciphertext_real_len, msg,
		 (unsigned long long) msg_len,
		 ad, (unsigned long long) ad_len, NULL, npub, secretkey) != 0) {
		crex_string_efree(ciphertext);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	if (ciphertext_real_len <= 0U || ciphertext_real_len >= SIZE_MAX ||
		ciphertext_real_len > ciphertext_len) {
		crex_string_efree(ciphertext);
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	CRX_SODIUM_ZSTR_TRUNCATE(ciphertext, (size_t) ciphertext_real_len);
	ZSTR_VAL(ciphertext)[ciphertext_real_len] = 0;

	RETURN_NEW_STR(ciphertext);
}

CRX_FUNCTION(sodium_crypto_aead_xchacha20poly1305_ietf_decrypt)
{
	crex_string        *msg;
	unsigned char      *ad;
	unsigned char      *ciphertext;
	unsigned char      *npub;
	unsigned char      *secretkey;
	unsigned long long  msg_real_len;
	size_t              ad_len;
	size_t              ciphertext_len;
	size_t              msg_len;
	size_t              npub_len;
	size_t              secretkey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ssss",
									&ciphertext, &ciphertext_len,
									&ad, &ad_len,
									&npub, &npub_len,
									&secretkey, &secretkey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (npub_len != crypto_aead_xchacha20poly1305_IETF_NPUBBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_AEAD_XCHACHA20POLY1305_IETF_NPUBBYTES bytes long");
		RETURN_THROWS();
	}
	if (secretkey_len != crypto_aead_xchacha20poly1305_IETF_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 4, "must be SODIUM_CRYPTO_AEAD_XCHACHA20POLY1305_IETF_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (ciphertext_len < crypto_aead_xchacha20poly1305_IETF_ABYTES) {
		RETURN_FALSE;
	}
	msg_len = ciphertext_len;
	if (msg_len - crypto_aead_xchacha20poly1305_IETF_ABYTES >= SIZE_MAX) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	if ((unsigned long long) ciphertext_len -
		crypto_aead_xchacha20poly1305_IETF_ABYTES > 64ULL * (1ULL << 32) - 64ULL) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	msg = crex_string_alloc((size_t) msg_len, 0);
	if (crypto_aead_xchacha20poly1305_ietf_decrypt
		((unsigned char *) ZSTR_VAL(msg), &msg_real_len, NULL,
		 ciphertext, (unsigned long long) ciphertext_len,
		 ad, (unsigned long long) ad_len, npub, secretkey) != 0) {
		crex_string_efree(msg);
		RETURN_FALSE;
	}
	if (msg_real_len >= SIZE_MAX || msg_real_len > msg_len) {
		crex_string_efree(msg);
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	CRX_SODIUM_ZSTR_TRUNCATE(msg, (size_t) msg_real_len);
	ZSTR_VAL(msg)[msg_real_len] = 0;

	RETURN_NEW_STR(msg);
}
#endif

CRX_FUNCTION(sodium_bin2hex)
{
	crex_string   *hex;
	unsigned char *bin;
	size_t         bin_len;
	size_t         hex_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
									&bin, &bin_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (bin_len >= SIZE_MAX / 2U) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	hex_len = bin_len * 2U;
	hex = crex_string_alloc((size_t) hex_len, 0);
	sodium_bin2hex(ZSTR_VAL(hex), hex_len + 1U, bin, bin_len);
	ZSTR_VAL(hex)[hex_len] = 0;

	RETURN_STR(hex);
}

CRX_FUNCTION(sodium_hex2bin)
{
	crex_string   *bin;
	const char    *end;
	char          *hex;
	char          *ignore = NULL;
	size_t         bin_real_len;
	size_t         bin_len;
	size_t         hex_len;
	size_t         ignore_len = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s|s",
									&hex, &hex_len,
									&ignore, &ignore_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	bin_len = hex_len / 2;
	bin = crex_string_alloc(bin_len, 0);
	if (sodium_hex2bin((unsigned char *) ZSTR_VAL(bin), bin_len, hex, hex_len,
					   ignore, &bin_real_len, &end) != 0 ||
		end != hex + hex_len) {
		crex_string_efree(bin);
		crex_argument_error(sodium_exception_ce, 1, "must be a valid hexadecimal string");
		RETURN_THROWS();
	}
	if (bin_real_len >= SIZE_MAX || bin_real_len > bin_len) {
		crex_string_efree(bin);
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	CRX_SODIUM_ZSTR_TRUNCATE(bin, (size_t) bin_real_len);
	ZSTR_VAL(bin)[bin_real_len] = 0;

	RETURN_NEW_STR(bin);
}

#ifdef sodium_base64_VARIANT_ORIGINAL
CRX_FUNCTION(sodium_bin2base64)
{
	crex_string   *b64;
	unsigned char *bin;
	crex_long      variant;
	size_t         bin_len;
	size_t         b64_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sl",
									&bin, &bin_len, &variant) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if ((((unsigned int) variant) & ~ 0x6U) != 0x1U) {
		crex_argument_error(sodium_exception_ce, 2, "must be a valid base64 variant identifier");
		RETURN_THROWS();
	}
	if (bin_len >= SIZE_MAX / 4U * 3U - 3U - 1U) {
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	b64_len = sodium_base64_ENCODED_LEN(bin_len, variant);
	b64 = crex_string_alloc((size_t) b64_len - 1U, 0);
	sodium_bin2base64(ZSTR_VAL(b64), b64_len, bin, bin_len, (int) variant);

	RETURN_STR(b64);
}

CRX_FUNCTION(sodium_base642bin)
{
	crex_string   *bin;
	char          *b64;
	const char    *end;
	char          *ignore = NULL;
	crex_long      variant;
	size_t         bin_real_len;
	size_t         bin_len;
	size_t         b64_len;
	size_t         ignore_len = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sl|s",
									&b64, &b64_len, &variant,
									&ignore, &ignore_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if ((((unsigned int) variant) & ~ 0x6U) != 0x1U) {
		crex_argument_error(sodium_exception_ce, 2, "must be a valid base64 variant identifier");
		RETURN_THROWS();
	}
	bin_len = b64_len / 4U * 3U + 2U;
	bin = crex_string_alloc(bin_len, 0);
	if (sodium_base642bin((unsigned char *) ZSTR_VAL(bin), bin_len,
						  b64, b64_len,
						  ignore, &bin_real_len, &end, (int) variant) != 0 ||
		end != b64 + b64_len) {
		crex_string_efree(bin);
		crex_argument_error(sodium_exception_ce, 1, "must be a valid base64 string");
		RETURN_THROWS();
	}
	if (bin_real_len >= SIZE_MAX || bin_real_len > bin_len) {
		crex_string_efree(bin);
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	CRX_SODIUM_ZSTR_TRUNCATE(bin, (size_t) bin_real_len);
	ZSTR_VAL(bin)[bin_real_len] = 0;

	RETURN_NEW_STR(bin);
}
#endif

CRX_FUNCTION(sodium_crypto_scalarmult)
{
	crex_string   *q;
	unsigned char *n;
	unsigned char *p;
	size_t         n_len;
	size_t         p_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&n, &n_len, &p, &p_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (n_len != crypto_scalarmult_SCALARBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_SCALARMULT_SCALARBYTES bytes long");
		RETURN_THROWS();
	}
	if (p_len != crypto_scalarmult_BYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_SCALARMULT_SCALARBYTES bytes long");
		RETURN_THROWS();
	}
	q = crex_string_alloc(crypto_scalarmult_BYTES, 0);
	if (crypto_scalarmult((unsigned char *) ZSTR_VAL(q), n, p) != 0) {
		crex_string_efree(q);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(q)[crypto_scalarmult_BYTES] = 0;

	RETURN_NEW_STR(q);
}

#ifdef crypto_core_ristretto255_HASHBYTES
CRX_FUNCTION(sodium_crypto_scalarmult_ristretto255)
{
	crex_string   *q;
	unsigned char *n;
	unsigned char *p;
	size_t         n_len;
	size_t         p_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&n, &n_len, &p, &p_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (n_len != crypto_scalarmult_ristretto255_SCALARBYTES) {
		crex_argument_error(sodium_exception_ce, 1,
							"must be SODIUM_CRYPTO_SCALARMULT_RISTRETTO255_SCALARBYTES bytes long");
		RETURN_THROWS();
	}
	if (p_len != crypto_scalarmult_ristretto255_BYTES) {
		crex_argument_error(sodium_exception_ce, 2,
							"must be SODIUM_CRYPTO_SCALARMULT_RISTRETTO255_BYTES bytes long");
		RETURN_THROWS();
	}
	q = crex_string_alloc(crypto_scalarmult_ristretto255_BYTES, 0);
	if (crypto_scalarmult_ristretto255((unsigned char *) ZSTR_VAL(q), n, p) != 0) {
		crex_string_efree(q);
		crex_throw_exception(sodium_exception_ce, "Result is identity element", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(q)[crypto_scalarmult_ristretto255_BYTES] = 0;

	RETURN_NEW_STR(q);
}

CRX_FUNCTION(sodium_crypto_scalarmult_ristretto255_base)
{
	crex_string   *q;
	unsigned char *n;
	size_t         n_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
							  &n, &n_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (n_len != crypto_scalarmult_ristretto255_SCALARBYTES) {
		crex_argument_error(sodium_exception_ce, 1,
							"must be SODIUM_CRYPTO_SCALARMULT_RISTRETTO255_SCALARBYTES bytes long");
		RETURN_THROWS();
	}
	q = crex_string_alloc(crypto_scalarmult_ristretto255_BYTES, 0);
	if (crypto_scalarmult_ristretto255_base((unsigned char *) ZSTR_VAL(q), n) != 0) {
		crex_string_efree(q);
		crex_argument_error(sodium_exception_ce, 1, "must not be zero", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(q)[crypto_scalarmult_BYTES] = 0;

	RETURN_NEW_STR(q);
}
#endif

CRX_FUNCTION(sodium_crypto_kx_seed_keypair)
{
	unsigned char *sk;
	unsigned char *pk;
	unsigned char *seed;
	size_t         seed_len;
	crex_string   *keypair;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
									&seed, &seed_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (seed_len != crypto_kx_SEEDBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_KX_SEEDBYTES bytes long");
		RETURN_THROWS();
	}
	(void) sizeof(int[crypto_scalarmult_SCALARBYTES == crypto_kx_PUBLICKEYBYTES ? 1 : -1]);
	(void) sizeof(int[crypto_scalarmult_SCALARBYTES == crypto_kx_SECRETKEYBYTES ? 1 : -1]);
	keypair = crex_string_alloc(crypto_kx_SECRETKEYBYTES + crypto_kx_PUBLICKEYBYTES, 0);
	sk = (unsigned char *) ZSTR_VAL(keypair);
	pk = sk + crypto_kx_SECRETKEYBYTES;
	crypto_generichash(sk, crypto_kx_SECRETKEYBYTES,
					   seed, crypto_kx_SEEDBYTES, NULL, 0);
	if (crypto_scalarmult_base(pk, sk) != 0) {
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(keypair)[crypto_kx_SECRETKEYBYTES + crypto_kx_PUBLICKEYBYTES] = 0;
	RETURN_STR(keypair);
}

CRX_FUNCTION(sodium_crypto_kx_keypair)
{
	unsigned char *sk;
	unsigned char *pk;
	crex_string   *keypair;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	keypair = crex_string_alloc(crypto_kx_SECRETKEYBYTES + crypto_kx_PUBLICKEYBYTES, 0);
	sk = (unsigned char *) ZSTR_VAL(keypair);
	pk = sk + crypto_kx_SECRETKEYBYTES;
	randombytes_buf(sk, crypto_kx_SECRETKEYBYTES);
	if (crypto_scalarmult_base(pk, sk) != 0) {
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(keypair)[crypto_kx_SECRETKEYBYTES + crypto_kx_PUBLICKEYBYTES] = 0;
	RETURN_STR(keypair);
}

CRX_FUNCTION(sodium_crypto_kx_secretkey)
{
	crex_string   *secretkey;
	unsigned char *keypair;
	size_t         keypair_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
									&keypair, &keypair_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (keypair_len !=
		crypto_kx_SECRETKEYBYTES + crypto_kx_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_KX_KEYPAIRBYTES bytes long");
		RETURN_THROWS();
	}
	secretkey = crex_string_alloc(crypto_kx_SECRETKEYBYTES, 0);
	memcpy(ZSTR_VAL(secretkey), keypair, crypto_kx_SECRETKEYBYTES);
	ZSTR_VAL(secretkey)[crypto_kx_SECRETKEYBYTES] = 0;

	RETURN_STR(secretkey);
}

CRX_FUNCTION(sodium_crypto_kx_publickey)
{
	crex_string   *publickey;
	unsigned char *keypair;
	size_t         keypair_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
									&keypair, &keypair_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (keypair_len !=
		crypto_kx_SECRETKEYBYTES + crypto_kx_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_KX_KEYPAIRBYTES bytes long");
		RETURN_THROWS();
	}
	publickey = crex_string_alloc(crypto_kx_PUBLICKEYBYTES, 0);
	memcpy(ZSTR_VAL(publickey), keypair + crypto_kx_SECRETKEYBYTES,
		   crypto_kx_PUBLICKEYBYTES);
	ZSTR_VAL(publickey)[crypto_kx_PUBLICKEYBYTES] = 0;

	RETURN_STR(publickey);
}

CRX_FUNCTION(sodium_crypto_kx_client_session_keys)
{
	crypto_generichash_state h;
	unsigned char  q[crypto_scalarmult_BYTES];
	unsigned char *keypair;
	unsigned char *client_sk;
	unsigned char *client_pk;
	unsigned char *server_pk;
	unsigned char  session_keys[2 * crypto_kx_SESSIONKEYBYTES];
	size_t         keypair_len;
	size_t         server_pk_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&keypair, &keypair_len,
									&server_pk, &server_pk_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (keypair_len != crypto_kx_SECRETKEYBYTES + crypto_kx_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_KX_KEYPAIRBYTES bytes long");
		RETURN_THROWS();
	}
	if (server_pk_len != crypto_kx_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_KX_PUBLICKEYBYTES bytes long");
		RETURN_THROWS();
	}
	client_sk = &keypair[0];
	client_pk = &keypair[crypto_kx_SECRETKEYBYTES];
	(void) sizeof(int[crypto_scalarmult_SCALARBYTES == crypto_kx_PUBLICKEYBYTES ? 1 : -1]);
	(void) sizeof(int[crypto_scalarmult_SCALARBYTES == crypto_kx_SECRETKEYBYTES ? 1 : -1]);
	if (crypto_scalarmult(q, client_sk, server_pk) != 0) {
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	crypto_generichash_init(&h, NULL, 0U, 2 * crypto_kx_SESSIONKEYBYTES);
	crypto_generichash_update(&h, q, sizeof q);
	sodium_memzero(q, sizeof q);
	crypto_generichash_update(&h, client_pk, crypto_kx_PUBLICKEYBYTES);
	crypto_generichash_update(&h, server_pk, crypto_kx_PUBLICKEYBYTES);
	crypto_generichash_final(&h, session_keys, 2 * crypto_kx_SESSIONKEYBYTES);
	sodium_memzero(&h, sizeof h);
	array_init(return_value);
	add_next_index_stringl(return_value,
						   (const char *) session_keys,
						   crypto_kx_SESSIONKEYBYTES);
	add_next_index_stringl(return_value,
						   (const char *) session_keys + crypto_kx_SESSIONKEYBYTES,
						   crypto_kx_SESSIONKEYBYTES);
}

CRX_FUNCTION(sodium_crypto_kx_server_session_keys)
{
	crypto_generichash_state h;
	unsigned char  q[crypto_scalarmult_BYTES];
	unsigned char *keypair;
	unsigned char *server_sk;
	unsigned char *server_pk;
	unsigned char *client_pk;
	unsigned char  session_keys[2 * crypto_kx_SESSIONKEYBYTES];
	size_t         keypair_len;
	size_t         client_pk_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&keypair, &keypair_len,
									&client_pk, &client_pk_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (keypair_len != crypto_kx_SECRETKEYBYTES + crypto_kx_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_KX_KEYPAIRBYTES bytes long");
		RETURN_THROWS();
	}
	if (client_pk_len != crypto_kx_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_KX_PUBLICKEYBYTES bytes long");
		RETURN_THROWS();
	}
	server_sk = &keypair[0];
	server_pk = &keypair[crypto_kx_SECRETKEYBYTES];
	(void) sizeof(int[crypto_scalarmult_SCALARBYTES == crypto_kx_PUBLICKEYBYTES ? 1 : -1]);
	(void) sizeof(int[crypto_scalarmult_SCALARBYTES == crypto_kx_SECRETKEYBYTES ? 1 : -1]);
	if (crypto_scalarmult(q, server_sk, client_pk) != 0) {
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	crypto_generichash_init(&h, NULL, 0U, 2 * crypto_kx_SESSIONKEYBYTES);
	crypto_generichash_update(&h, q, sizeof q);
	sodium_memzero(q, sizeof q);
	crypto_generichash_update(&h, client_pk, crypto_kx_PUBLICKEYBYTES);
	crypto_generichash_update(&h, server_pk, crypto_kx_PUBLICKEYBYTES);
	crypto_generichash_final(&h, session_keys, 2 * crypto_kx_SESSIONKEYBYTES);
	sodium_memzero(&h, sizeof h);
	array_init(return_value);
	add_next_index_stringl(return_value,
						   (const char *) session_keys + crypto_kx_SESSIONKEYBYTES,
						   crypto_kx_SESSIONKEYBYTES);
	add_next_index_stringl(return_value,
						   (const char *) session_keys,
						   crypto_kx_SESSIONKEYBYTES);
}

CRX_FUNCTION(sodium_crypto_auth)
{
	crex_string *mac;
	char        *key;
	char        *msg;
	size_t       msg_len;
	size_t       key_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&msg, &msg_len,
									&key, &key_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (key_len != crypto_auth_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_AUTH_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	mac = crex_string_alloc(crypto_auth_BYTES, 0);
	if (crypto_auth((unsigned char *) ZSTR_VAL(mac),
					(const unsigned char *) msg, msg_len,
					(const unsigned char *) key) != 0) {
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(mac)[crypto_auth_BYTES] = 0;

	RETURN_STR(mac);
}

CRX_FUNCTION(sodium_crypto_auth_verify)
{
	char      *mac;
	char      *key;
	char      *msg;
	size_t     mac_len;
	size_t     msg_len;
	size_t     key_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sss",
									&mac, &mac_len,
									&msg, &msg_len,
									&key, &key_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (key_len != crypto_auth_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_AUTH_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (mac_len != crypto_auth_BYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_AUTH_BYTES bytes long");
		RETURN_THROWS();
	}
	if (crypto_auth_verify((const unsigned char *) mac,
						   (const unsigned char *) msg, msg_len,
						   (const unsigned char *) key) != 0) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

CRX_FUNCTION(sodium_crypto_sign_ed25519_sk_to_curve25519)
{
	crex_string *ecdhkey;
	char        *eddsakey;
	size_t       eddsakey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
									&eddsakey, &eddsakey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (eddsakey_len != crypto_sign_SECRETKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_SIGN_SECRETKEYBYTES bytes long");
		RETURN_THROWS();
	}
	ecdhkey = crex_string_alloc(crypto_box_SECRETKEYBYTES, 0);

	if (crypto_sign_ed25519_sk_to_curve25519((unsigned char *) ZSTR_VAL(ecdhkey),
											 (const unsigned char *) eddsakey) != 0) {
		crex_throw_exception(sodium_exception_ce, "conversion failed", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(ecdhkey)[crypto_box_SECRETKEYBYTES] = 0;

	RETURN_STR(ecdhkey);
}

CRX_FUNCTION(sodium_crypto_sign_ed25519_pk_to_curve25519)
{
	crex_string *ecdhkey;
	char        *eddsakey;
	size_t       eddsakey_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
									&eddsakey, &eddsakey_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (eddsakey_len != crypto_sign_PUBLICKEYBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_SIGN_PUBLICKEYBYTES bytes long");
		RETURN_THROWS();
	}
	ecdhkey = crex_string_alloc(crypto_sign_PUBLICKEYBYTES, 0);

	if (crypto_sign_ed25519_pk_to_curve25519((unsigned char *) ZSTR_VAL(ecdhkey),
											 (const unsigned char *) eddsakey) != 0) {
		crex_throw_exception(sodium_exception_ce, "conversion failed", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(ecdhkey)[crypto_box_PUBLICKEYBYTES] = 0;

	RETURN_STR(ecdhkey);
}

CRX_FUNCTION(sodium_compare)
{
	char      *buf1;
	char      *buf2;
	size_t     len1;
	size_t     len2;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&buf1, &len1,
									&buf2, &len2) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (len1 != len2) {
		crex_argument_error(sodium_exception_ce, 1, "and argument #2 ($string_2) must have the same length");
		RETURN_THROWS();
	} else {
		RETURN_LONG(sodium_compare((const unsigned char *) buf1,
								   (const unsigned char *) buf2, (size_t) len1));
	}
}

#ifdef HAVE_AESGCM
CRX_FUNCTION(sodium_crypto_aead_aes256gcm_keygen)
{
	unsigned char key[crypto_aead_aes256gcm_KEYBYTES];

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	randombytes_buf(key, sizeof key);
	RETURN_STRINGL((const char *) key, sizeof key);
}
#endif

CRX_FUNCTION(sodium_crypto_aead_chacha20poly1305_keygen)
{
	unsigned char key[crypto_aead_chacha20poly1305_KEYBYTES];

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	randombytes_buf(key, sizeof key);
	RETURN_STRINGL((const char *) key, sizeof key);
}

CRX_FUNCTION(sodium_crypto_aead_chacha20poly1305_ietf_keygen)
{
	unsigned char key[crypto_aead_chacha20poly1305_IETF_KEYBYTES];

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	randombytes_buf(key, sizeof key);
	RETURN_STRINGL((const char *) key, sizeof key);
}

#ifdef crypto_aead_xchacha20poly1305_IETF_NPUBBYTES
CRX_FUNCTION(sodium_crypto_aead_xchacha20poly1305_ietf_keygen)
{
	unsigned char key[crypto_aead_xchacha20poly1305_IETF_KEYBYTES];

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	randombytes_buf(key, sizeof key);
	RETURN_STRINGL((const char *) key, sizeof key);
}
#endif

CRX_FUNCTION(sodium_crypto_auth_keygen)
{
	unsigned char key[crypto_auth_KEYBYTES];

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	randombytes_buf(key, sizeof key);
	RETURN_STRINGL((const char *) key, sizeof key);
}

CRX_FUNCTION(sodium_crypto_generichash_keygen)
{
	unsigned char key[crypto_generichash_KEYBYTES];

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	randombytes_buf(key, sizeof key);
	RETURN_STRINGL((const char *) key, sizeof key);
}

CRX_FUNCTION(sodium_crypto_kdf_keygen)
{
	unsigned char key[crypto_kdf_KEYBYTES];

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	randombytes_buf(key, sizeof key);
	RETURN_STRINGL((const char *) key, sizeof key);
}

CRX_FUNCTION(sodium_crypto_secretbox_keygen)
{
	unsigned char key[crypto_secretbox_KEYBYTES];

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	randombytes_buf(key, sizeof key);
	RETURN_STRINGL((const char *) key, sizeof key);
}

CRX_FUNCTION(sodium_crypto_shorthash_keygen)
{
	unsigned char key[crypto_shorthash_KEYBYTES];

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	randombytes_buf(key, sizeof key);
	RETURN_STRINGL((const char *) key, sizeof key);
}

CRX_FUNCTION(sodium_crypto_stream_keygen)
{
	unsigned char key[crypto_stream_KEYBYTES];

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	randombytes_buf(key, sizeof key);
	RETURN_STRINGL((const char *) key, sizeof key);
}
#ifdef crypto_stream_xchacha20_KEYBYTES
CRX_FUNCTION(sodium_crypto_stream_xchacha20_keygen)
{
	unsigned char key[crypto_stream_xchacha20_KEYBYTES];

	if (crex_parse_parameters_none() == FAILURE) {
		return;
	}
	randombytes_buf(key, sizeof key);
	RETURN_STRINGL((const char *) key, sizeof key);
}
#endif

CRX_FUNCTION(sodium_crypto_kdf_derive_from_key)
{
	unsigned char  ctx_padded[crypto_generichash_blake2b_PERSONALBYTES];
#ifndef crypto_kdf_PRIMITIVE
	unsigned char  salt[crypto_generichash_blake2b_SALTBYTES];
#endif
	char          *ctx;
	char          *key;
	crex_string   *subkey;
	crex_long      subkey_id;
	crex_long      subkey_len;
	size_t         ctx_len;
	size_t         key_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "llss",
									&subkey_len,
									&subkey_id,
									&ctx, &ctx_len,
									&key, &key_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (subkey_len < crypto_kdf_BYTES_MIN) {
		crex_argument_error(sodium_exception_ce, 1, "must be greater than or equal to SODIUM_CRYPTO_KDF_BYTES_MIN");
		RETURN_THROWS();
	}
	if (subkey_len > crypto_kdf_BYTES_MAX || subkey_len > SIZE_MAX) {
		crex_argument_error(sodium_exception_ce, 1, "must be less than or equal to SODIUM_CRYPTO_KDF_BYTES_MAX");
		RETURN_THROWS();
	}
	if (subkey_id < 0) {
		crex_argument_error(sodium_exception_ce, 2, "must be greater than or equal to 0");
		RETURN_THROWS();
	}
	if (ctx_len != crypto_kdf_CONTEXTBYTES) {
		crex_argument_error(sodium_exception_ce, 3, "must be SODIUM_CRYPTO_KDF_CONTEXTBYTES bytes long");
		RETURN_THROWS();
	}
	if (key_len != crypto_kdf_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 4, "must be SODIUM_CRYPTO_KDF_BYTES_MIN bytes long");
		RETURN_THROWS();
	}
	memcpy(ctx_padded, ctx, crypto_kdf_CONTEXTBYTES);
	memset(ctx_padded + crypto_kdf_CONTEXTBYTES, 0, sizeof ctx_padded - crypto_kdf_CONTEXTBYTES);
	subkey = crex_string_alloc((size_t) subkey_len, 0);
#ifdef crypto_kdf_PRIMITIVE
	crypto_kdf_derive_from_key((unsigned char *) ZSTR_VAL(subkey),
							   (size_t) subkey_len, (uint64_t) subkey_id,
							   ctx, (const unsigned char *) key);
#else
	salt[0] = (unsigned char) (((uint64_t) subkey_id)      );
	salt[1] = (unsigned char) (((uint64_t) subkey_id) >>  8);
	salt[2] = (unsigned char) (((uint64_t) subkey_id) >> 16);
	salt[3] = (unsigned char) (((uint64_t) subkey_id) >> 24);
	salt[4] = (unsigned char) (((uint64_t) subkey_id) >> 32);
	salt[5] = (unsigned char) (((uint64_t) subkey_id) >> 40);
	salt[6] = (unsigned char) (((uint64_t) subkey_id) >> 48);
	salt[7] = (unsigned char) (((uint64_t) subkey_id) >> 56);
	memset(salt + 8, 0, (sizeof salt) - 8);
	crypto_generichash_blake2b_salt_personal((unsigned char *) ZSTR_VAL(subkey),
											 (size_t) subkey_len,
											 NULL, 0,
											 (const unsigned char *) key,
											 crypto_kdf_KEYBYTES,
											 salt, ctx_padded);
#endif
	ZSTR_VAL(subkey)[subkey_len] = 0;

	RETURN_STR(subkey);
}

CRX_FUNCTION(sodium_pad)
{
	crex_string    *padded;
	char           *unpadded;
	crex_long       blocksize;
	volatile size_t st;
	size_t          i, j, k;
	size_t          unpadded_len;
	size_t          xpadlen;
	size_t          xpadded_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sl",
									&unpadded, &unpadded_len, &blocksize) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (blocksize <= 0) {
		crex_argument_error(sodium_exception_ce, 2, "must be greater than 0");
		RETURN_THROWS();
	}
	if (blocksize > SIZE_MAX) {
		crex_argument_error(sodium_exception_ce, 2, "is too large");
		RETURN_THROWS();
	}
	xpadlen = blocksize - 1U;
	if ((blocksize & (blocksize - 1U)) == 0U) {
		xpadlen -= unpadded_len & ((size_t) blocksize - 1U);
	} else {
		xpadlen -= unpadded_len % (size_t) blocksize;
	}
	if ((size_t) SIZE_MAX - unpadded_len <= xpadlen) {
		crex_throw_exception(sodium_exception_ce, "input is too large", 0);
		RETURN_THROWS();
	}
	xpadded_len = unpadded_len + xpadlen;
	padded = crex_string_alloc(xpadded_len + 1U, 0);
	if (unpadded_len > 0) {
		st = 1U;
		i = 0U;
		k = unpadded_len;
		for (j = 0U; j <= xpadded_len; j++) {
			ZSTR_VAL(padded)[j] = unpadded[i];
			k -= st;
			st = (size_t) (~(((( (((uint64_t) k) >> 48) | (((uint64_t) k) >> 32) |
								 (k >> 16) | k) & 0xffff) - 1U) >> 16)) & 1U;
			i += st;
		}
	}
#if SODIUM_LIBRARY_VERSION_MAJOR > 9 || (SODIUM_LIBRARY_VERSION_MAJOR == 9 && SODIUM_LIBRARY_VERSION_MINOR >= 6)
	if (sodium_pad(NULL, (unsigned char *) ZSTR_VAL(padded), unpadded_len,
				   (size_t) blocksize, xpadded_len + 1U) != 0) {
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
#else
	{
		char                   *tail;
		volatile unsigned char  mask;
		unsigned char           barrier_mask;

		tail = &ZSTR_VAL(padded)[xpadded_len];
		mask = 0U;
		for (i = 0; i < blocksize; i++) {
			barrier_mask = (unsigned char)
				(((i ^ xpadlen) - 1U) >> ((sizeof(size_t) - 1U) * CHAR_BIT));
			tail[-i] = (tail[-i] & mask) | (0x80 & barrier_mask);
			mask |= barrier_mask;
		}
	}
#endif
	ZSTR_VAL(padded)[xpadded_len + 1U] = 0;

	RETURN_STR(padded);
}

CRX_FUNCTION(sodium_unpad)
{
	crex_string *unpadded;
	char        *padded;
	size_t       padded_len;
	size_t       unpadded_len;
	crex_long    blocksize;
	int          ret;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sl",
									&padded, &padded_len, &blocksize) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (blocksize <= 0) {
		crex_argument_error(sodium_exception_ce, 2, "must be greater than 0");
		RETURN_THROWS();
	}
	if (blocksize > SIZE_MAX) {
		crex_argument_error(sodium_exception_ce, 2, "is too large");
		RETURN_THROWS();
	}
	if (padded_len < blocksize) {
		crex_argument_error(sodium_exception_ce, 1, "must be at least as long as the block size");
		RETURN_THROWS();
	}

#if SODIUM_LIBRARY_VERSION_MAJOR > 9 || (SODIUM_LIBRARY_VERSION_MAJOR == 9 && SODIUM_LIBRARY_VERSION_MINOR >= 6)
	ret = sodium_unpad(&unpadded_len, (const unsigned char *) padded,
					   padded_len, (size_t) blocksize);
#else
	{
		const char      *tail;
		unsigned char    acc = 0U;
		unsigned char    c;
		unsigned char    valid = 0U;
		volatile size_t  pad_len = 0U;
		size_t           i;
		size_t           is_barrier;

		tail = &padded[padded_len - 1U];

		for (i = 0U; i < (size_t) blocksize; i++) {
			c = tail[-i];
			is_barrier =
				(( (acc - 1U) & (pad_len - 1U) & ((c ^ 0x80) - 1U) ) >> 8) & 1U;
			acc |= c;
			pad_len |= i & (1U + ~is_barrier);
			valid |= (unsigned char) is_barrier;
		}
		unpadded_len = padded_len - 1U - pad_len;
		ret = (int) (valid - 1U);
	}
#endif
	if (ret != 0 || unpadded_len > LONG_MAX) {
		crex_throw_exception(sodium_exception_ce, "invalid padding", 0);
		RETURN_THROWS();
	}
	unpadded = crex_string_init(padded, padded_len, 0);
	CRX_SODIUM_ZSTR_TRUNCATE(unpadded, unpadded_len);
	ZSTR_VAL(unpadded)[unpadded_len] = 0;
	RETURN_STR(unpadded);
}

#ifdef crypto_secretstream_xchacha20poly1305_ABYTES
CRX_FUNCTION(sodium_crypto_secretstream_xchacha20poly1305_keygen)
{
	unsigned char key[crypto_secretstream_xchacha20poly1305_KEYBYTES];

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	randombytes_buf(key, sizeof key);
	RETURN_STRINGL((const char *) key, sizeof key);
}

CRX_FUNCTION(sodium_crypto_secretstream_xchacha20poly1305_init_push)
{
	crypto_secretstream_xchacha20poly1305_state  state;
	unsigned char                                header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
	unsigned char                               *key;
	size_t                                       key_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
									&key, &key_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (key_len != crypto_secretstream_xchacha20poly1305_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (crypto_secretstream_xchacha20poly1305_init_push(&state,
														header, key) != 0) {
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	array_init(return_value);
	add_next_index_stringl(return_value, (const char *) &state, sizeof state);
	add_next_index_stringl(return_value, (const char *) header, sizeof header);
}

CRX_FUNCTION(sodium_crypto_secretstream_xchacha20poly1305_push)
{
	zval               *state_zv;
	crex_string        *c;
	unsigned char      *ad = NULL;
	unsigned char      *msg;
	unsigned char      *state;
	unsigned long long  c_real_len;
	crex_long           tag = crypto_secretstream_xchacha20poly1305_TAG_MESSAGE;
	size_t              ad_len = (size_t) 0U;
	size_t              c_len;
	size_t              msg_len;
	size_t              state_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "zs|sl",
									&state_zv,
									&msg, &msg_len, &ad, &ad_len, &tag) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	ZVAL_DEREF(state_zv);
	if (C_TYPE_P(state_zv) != IS_STRING) {
		crex_argument_error(sodium_exception_ce, 1, "must be a reference to a state");
		RETURN_THROWS();
	}
	sodium_separate_string(state_zv);
	state = (unsigned char *) C_STRVAL(*state_zv);
	state_len = C_STRLEN(*state_zv);
	if (state_len != sizeof (crypto_secretstream_xchacha20poly1305_state)) {
		crex_argument_error(sodium_exception_ce, 1, "must have a correct length");
		RETURN_THROWS();
	}
	if (msg_len > crypto_secretstream_xchacha20poly1305_MESSAGEBYTES_MAX ||
		msg_len > SIZE_MAX - crypto_secretstream_xchacha20poly1305_ABYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be at most SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_MESSAGEBYTES_MAX bytes long");
		RETURN_THROWS();
	}
	if (tag < 0 || tag > 255) {
		crex_argument_error(sodium_exception_ce, 4, "must be in the range of 0-255");
		RETURN_THROWS();
	}
	c_len = msg_len + crypto_secretstream_xchacha20poly1305_ABYTES;
	c = crex_string_alloc((size_t) c_len, 0);
	if (crypto_secretstream_xchacha20poly1305_push
		((void *) state, (unsigned char *) ZSTR_VAL(c), &c_real_len,
		 msg, (unsigned long long) msg_len, ad, (unsigned long long) ad_len,
		 (unsigned char) tag) != 0) {
		crex_string_efree(c);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	if (c_real_len <= 0U || c_real_len >= SIZE_MAX || c_real_len > c_len) {
		crex_string_efree(c);
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	CRX_SODIUM_ZSTR_TRUNCATE(c, (size_t) c_real_len);
	ZSTR_VAL(c)[c_real_len] = 0;

	RETURN_NEW_STR(c);
}

CRX_FUNCTION(sodium_crypto_secretstream_xchacha20poly1305_init_pull)
{
	crypto_secretstream_xchacha20poly1305_state  state;
	unsigned char                               *header;
	unsigned char                               *key;
	size_t                                       header_len;
	size_t                                       key_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&header, &header_len,
									&key, &key_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (header_len != crypto_secretstream_xchacha20poly1305_HEADERBYTES) {
		crex_argument_error(sodium_exception_ce, 1, "must be SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_HEADERBYTES bytes long");
		RETURN_THROWS();
	}
	if (key_len != crypto_secretstream_xchacha20poly1305_KEYBYTES) {
		crex_argument_error(sodium_exception_ce, 2, "must be SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_KEYBYTES bytes long");
		RETURN_THROWS();
	}
	if (crypto_secretstream_xchacha20poly1305_init_pull(&state,
														header, key) != 0) {
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	RETURN_STRINGL((const char *) &state, sizeof state);
}

CRX_FUNCTION(sodium_crypto_secretstream_xchacha20poly1305_pull)
{
	zval               *state_zv;
	crex_string        *msg;
	unsigned char      *ad = NULL;
	unsigned char      *c;
	unsigned char      *state;
	unsigned long long  msg_real_len;
	size_t              ad_len = (size_t) 0U;
	size_t              msg_len;
	size_t              c_len;
	size_t              state_len;
	unsigned char       tag;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "zs|s",
									&state_zv,
									&c, &c_len, &ad, &ad_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	ZVAL_DEREF(state_zv);
	if (C_TYPE_P(state_zv) != IS_STRING) {
		crex_argument_error(sodium_exception_ce, 1, "must be a reference to a state");
		RETURN_THROWS();
	}
	sodium_separate_string(state_zv);
	state = (unsigned char *) C_STRVAL(*state_zv);
	state_len = C_STRLEN(*state_zv);
	if (state_len != sizeof (crypto_secretstream_xchacha20poly1305_state)) {
		crex_throw_exception(sodium_exception_ce, "incorrect state length", 0);
		RETURN_THROWS();
	}
	if (c_len < crypto_secretstream_xchacha20poly1305_ABYTES) {
		RETURN_FALSE;
	}
	msg_len = c_len - crypto_secretstream_xchacha20poly1305_ABYTES;
	msg = crex_string_alloc((size_t) msg_len, 0);
	if (crypto_secretstream_xchacha20poly1305_pull
		((void *) state, (unsigned char *) ZSTR_VAL(msg), &msg_real_len, &tag,
		 c, (unsigned long long) c_len, ad, (unsigned long long) ad_len) != 0) {
		crex_string_efree(msg);
		RETURN_FALSE;
	}
	if (msg_real_len >= SIZE_MAX || msg_real_len > msg_len) {
		crex_string_efree(msg);
		crex_throw_exception(sodium_exception_ce, "arithmetic overflow", 0);
		RETURN_THROWS();
	}
	CRX_SODIUM_ZSTR_TRUNCATE(msg, (size_t) msg_real_len);
	ZSTR_VAL(msg)[msg_real_len] = 0;
	array_init(return_value);
	add_next_index_str(return_value, msg);
	add_next_index_long(return_value, (long) tag);
}

CRX_FUNCTION(sodium_crypto_secretstream_xchacha20poly1305_rekey)
{
	zval          *state_zv;
	unsigned char *state;
	size_t         state_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "z", &state_zv) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	ZVAL_DEREF(state_zv);
	if (C_TYPE_P(state_zv) != IS_STRING) {
		crex_argument_error(sodium_exception_ce, 1, "must be a reference to a state");
		RETURN_THROWS();
	}
	sodium_separate_string(state_zv);
	state = (unsigned char *) C_STRVAL(*state_zv);
	state_len = C_STRLEN(*state_zv);
	if (state_len != sizeof (crypto_secretstream_xchacha20poly1305_state)) {
		crex_throw_exception(sodium_exception_ce, "incorrect state length", 0);
		RETURN_THROWS();
	}
	crypto_secretstream_xchacha20poly1305_rekey((void *) state);
}
#endif

#ifdef crypto_core_ristretto255_HASHBYTES
CRX_FUNCTION(sodium_crypto_core_ristretto255_add)
{
	crex_string   *r;
	unsigned char *p;
	unsigned char *q;
	size_t         p_len;
	size_t         q_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&p, &p_len, &q, &q_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (p_len != crypto_core_ristretto255_BYTES) {
		crex_argument_error(sodium_exception_ce, 1,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_BYTES bytes long");
		RETURN_THROWS();
	}
	if (q_len != crypto_core_ristretto255_BYTES) {
		crex_argument_error(sodium_exception_ce, 2,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_BYTES bytes long");
		RETURN_THROWS();
	}
	r = crex_string_alloc(crypto_core_ristretto255_BYTES, 0);
	if (crypto_core_ristretto255_add((unsigned char *) ZSTR_VAL(r), p, q) != 0) {
		crex_string_efree(r);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(r)[crypto_core_ristretto255_BYTES] = 0;
	RETURN_NEW_STR(r);
}

CRX_FUNCTION(sodium_crypto_core_ristretto255_from_hash)
{
	crex_string   *r;
	unsigned char *s;
	size_t         s_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
							  &s, &s_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (s_len != crypto_core_ristretto255_HASHBYTES) {
		crex_argument_error(sodium_exception_ce, 1,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_HASHBYTES bytes long");
		RETURN_THROWS();
	}
	r = crex_string_alloc(crypto_core_ristretto255_SCALARBYTES, 0);
	if (crypto_core_ristretto255_from_hash((unsigned char *) ZSTR_VAL(r), s) != 0) {
		crex_string_efree(r);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(r)[crypto_core_ristretto255_SCALARBYTES] = 0;
	RETURN_NEW_STR(r);
}

CRX_FUNCTION(sodium_crypto_core_ristretto255_is_valid_point)
{
	unsigned char *s;
	size_t         s_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
							  &s, &s_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (s_len != crypto_core_ristretto255_BYTES) {
		crex_argument_error(sodium_exception_ce, 1,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_BYTES bytes long");
		RETURN_THROWS();
	}
	RETURN_BOOL(crypto_core_ristretto255_is_valid_point(s));
}

CRX_FUNCTION(sodium_crypto_core_ristretto255_random)
{
	crex_string   *r;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	r = crex_string_alloc(crypto_core_ristretto255_BYTES, 0);
	crypto_core_ristretto255_random((unsigned char *) ZSTR_VAL(r));
	ZSTR_VAL(r)[crypto_core_ristretto255_BYTES] = 0;
	RETURN_NEW_STR(r);
}

CRX_FUNCTION(sodium_crypto_core_ristretto255_scalar_add)
{
	crex_string   *r;
	unsigned char *p;
	unsigned char *q;
	size_t         p_len;
	size_t         q_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&p, &p_len, &q, &q_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (p_len != crypto_core_ristretto255_SCALARBYTES) {
		crex_argument_error(sodium_exception_ce, 1,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_SCALARBYTES bytes long");
		RETURN_THROWS();
	}
	if (q_len != crypto_core_ristretto255_SCALARBYTES) {
		crex_argument_error(sodium_exception_ce, 2,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_SCALARBYTES bytes long");
		RETURN_THROWS();
	}
	r = crex_string_alloc(crypto_core_ristretto255_BYTES, 0);
	crypto_core_ristretto255_scalar_add((unsigned char *) ZSTR_VAL(r), p, q);
	ZSTR_VAL(r)[crypto_core_ristretto255_SCALARBYTES] = 0;
	RETURN_NEW_STR(r);
}

CRX_FUNCTION(sodium_crypto_core_ristretto255_scalar_complement)
{
	crex_string   *r;
	unsigned char *s;
	size_t         s_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
							  &s, &s_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (s_len != crypto_core_ristretto255_SCALARBYTES) {
		crex_argument_error(sodium_exception_ce, 1,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_SCALARBYTES bytes long");
		RETURN_THROWS();
	}
	r = crex_string_alloc(crypto_core_ristretto255_SCALARBYTES, 0);
	crypto_core_ristretto255_scalar_complement((unsigned char *) ZSTR_VAL(r), s);
	ZSTR_VAL(r)[crypto_core_ristretto255_SCALARBYTES] = 0;
	RETURN_NEW_STR(r);
}

CRX_FUNCTION(sodium_crypto_core_ristretto255_scalar_invert)
{
	crex_string   *r;
	unsigned char *s;
	size_t         s_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
							  &s, &s_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (s_len != crypto_core_ristretto255_SCALARBYTES) {
		crex_argument_error(sodium_exception_ce, 1,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_SCALARBYTES bytes long");
		RETURN_THROWS();
	}
	r = crex_string_alloc(crypto_core_ristretto255_SCALARBYTES, 0);
	if (crypto_core_ristretto255_scalar_invert((unsigned char *) ZSTR_VAL(r), s) != 0) {
		crex_string_efree(r);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(r)[crypto_core_ristretto255_SCALARBYTES] = 0;
	RETURN_NEW_STR(r);
}

CRX_FUNCTION(sodium_crypto_core_ristretto255_scalar_mul)
{
	crex_string   *r;
	unsigned char *x;
	unsigned char *y;
	size_t         x_len;
	size_t         y_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&x, &x_len, &y, &y_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (x_len != crypto_core_ristretto255_SCALARBYTES) {
		crex_argument_error(sodium_exception_ce, 1,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_SCALARBYTES bytes long");
		RETURN_THROWS();
	}
	if (y_len != crypto_core_ristretto255_SCALARBYTES) {
		crex_argument_error(sodium_exception_ce, 2,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_SCALARBYTES bytes long");
		RETURN_THROWS();
	}
	r = crex_string_alloc(crypto_core_ristretto255_BYTES, 0);
	crypto_core_ristretto255_scalar_mul((unsigned char *) ZSTR_VAL(r), x, y);
	ZSTR_VAL(r)[crypto_core_ristretto255_SCALARBYTES] = 0;
	RETURN_NEW_STR(r);
}

CRX_FUNCTION(sodium_crypto_core_ristretto255_scalar_negate)
{
	crex_string   *r;
	unsigned char *s;
	size_t         s_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
							  &s, &s_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (s_len != crypto_core_ristretto255_SCALARBYTES) {
		crex_argument_error(sodium_exception_ce, 1,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_SCALARBYTES bytes long");
		RETURN_THROWS();
	}
	r = crex_string_alloc(crypto_core_ristretto255_SCALARBYTES, 0);
	crypto_core_ristretto255_scalar_negate((unsigned char *) ZSTR_VAL(r), s);
	ZSTR_VAL(r)[crypto_core_ristretto255_SCALARBYTES] = 0;
	RETURN_NEW_STR(r);
}

CRX_FUNCTION(sodium_crypto_core_ristretto255_scalar_random)
{
	crex_string   *r;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	};
	r = crex_string_alloc(crypto_core_ristretto255_SCALARBYTES, 0);
	crypto_core_ristretto255_scalar_random((unsigned char *) ZSTR_VAL(r));
	ZSTR_VAL(r)[crypto_core_ristretto255_SCALARBYTES] = 0;
	RETURN_NEW_STR(r);
}

CRX_FUNCTION(sodium_crypto_core_ristretto255_scalar_reduce)
{
	crex_string   *r;
	unsigned char *s;
	size_t         s_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s",
							  &s, &s_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (s_len != crypto_core_ristretto255_NONREDUCEDSCALARBYTES) {
		crex_argument_error(sodium_exception_ce, 1,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_NONREDUCEDSCALARBYTES bytes long");
		RETURN_THROWS();
	}
	r = crex_string_alloc(crypto_core_ristretto255_SCALARBYTES, 0);
	crypto_core_ristretto255_scalar_reduce((unsigned char *) ZSTR_VAL(r), s);
	ZSTR_VAL(r)[crypto_core_ristretto255_SCALARBYTES] = 0;
	RETURN_NEW_STR(r);
}

CRX_FUNCTION(sodium_crypto_core_ristretto255_scalar_sub)
{
	crex_string   *r;
	unsigned char *p;
	unsigned char *q;
	size_t         p_len;
	size_t         q_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&p, &p_len, &q, &q_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (p_len != crypto_core_ristretto255_SCALARBYTES) {
		crex_argument_error(sodium_exception_ce, 1,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_SCALARBYTES bytes long");
		RETURN_THROWS();
	}
	if (q_len != crypto_core_ristretto255_SCALARBYTES) {
		crex_argument_error(sodium_exception_ce, 2,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_SCALARBYTES bytes long");
		RETURN_THROWS();
	}
	r = crex_string_alloc(crypto_core_ristretto255_BYTES, 0);
	crypto_core_ristretto255_scalar_sub((unsigned char *) ZSTR_VAL(r), p, q);
	ZSTR_VAL(r)[crypto_core_ristretto255_SCALARBYTES] = 0;
	RETURN_NEW_STR(r);
}

CRX_FUNCTION(sodium_crypto_core_ristretto255_sub)
{
	crex_string   *r;
	unsigned char *p;
	unsigned char *q;
	size_t         p_len;
	size_t         q_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss",
									&p, &p_len, &q, &q_len) == FAILURE) {
		sodium_remove_param_values_from_backtrace(EG(exception));
		RETURN_THROWS();
	}
	if (p_len != crypto_core_ristretto255_BYTES) {
		crex_argument_error(sodium_exception_ce, 1,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_BYTES bytes long");
		RETURN_THROWS();
	}
	if (q_len != crypto_core_ristretto255_BYTES) {
		crex_argument_error(sodium_exception_ce, 2,
							"must be SODIUM_CRYPTO_CORE_RISTRETTO255_BYTES bytes long");
		RETURN_THROWS();
	}
	r = crex_string_alloc(crypto_core_ristretto255_BYTES, 0);
	if (crypto_core_ristretto255_sub((unsigned char *) ZSTR_VAL(r), p, q) != 0) {
		crex_string_efree(r);
		crex_throw_exception(sodium_exception_ce, "internal error", 0);
		RETURN_THROWS();
	}
	ZSTR_VAL(r)[crypto_core_ristretto255_BYTES] = 0;
	RETURN_NEW_STR(r);
}
#endif
