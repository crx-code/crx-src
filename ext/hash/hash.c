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
  | Author: Sara Golemon <pollita@crx.net>                               |
  |         Scott MacVicar <scottmac@crx.net>                            |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include "crx_hash.h"
#include "ext/standard/info.h"
#include "ext/standard/file.h"
#include "ext/standard/crx_var.h"
#include "ext/spl/spl_exceptions.h"

#include "crex_attributes.h"
#include "crex_exceptions.h"
#include "crex_interfaces.h"
#include "crex_smart_str.h"

#include "hash_arginfo.h"

#ifdef CRX_WIN32
# define __alignof__ __alignof
#else
# ifndef HAVE_ALIGNOF
#  include <stddef.h>
#  define __alignof__(type) offsetof (struct { char c; type member;}, member)
# endif
#endif

HashTable crx_hash_hashtable;
crex_class_entry *crx_hashcontext_ce;
static crex_object_handlers crx_hashcontext_handlers;

#ifdef CRX_MHASH_BC
struct mhash_bc_entry {
	char *mhash_name;
	char *hash_name;
	int value;
};

#define MHASH_NUM_ALGOS 42

static struct mhash_bc_entry mhash_to_hash[MHASH_NUM_ALGOS] = {
	{"CRC32", "crc32", 0}, /* used by bzip */
	{"MD5", "md5", 1},
	{"SHA1", "sha1", 2},
	{"HAVAL256", "haval256,3", 3},
	{NULL, NULL, 4},
	{"RIPEMD160", "ripemd160", 5},
	{NULL, NULL, 6},
	{"TIGER", "tiger192,3", 7},
	{"GOST", "gost", 8},
	{"CRC32B", "crc32b", 9}, /* used by ethernet (IEEE 802.3), gzip, zip, png, etc */
	{"HAVAL224", "haval224,3", 10},
	{"HAVAL192", "haval192,3", 11},
	{"HAVAL160", "haval160,3", 12},
	{"HAVAL128", "haval128,3", 13},
	{"TIGER128", "tiger128,3", 14},
	{"TIGER160", "tiger160,3", 15},
	{"MD4", "md4", 16},
	{"SHA256", "sha256", 17},
	{"ADLER32", "adler32", 18},
	{"SHA224", "sha224", 19},
	{"SHA512", "sha512", 20},
	{"SHA384", "sha384", 21},
	{"WHIRLPOOL", "whirlpool", 22},
	{"RIPEMD128", "ripemd128", 23},
	{"RIPEMD256", "ripemd256", 24},
	{"RIPEMD320", "ripemd320", 25},
	{NULL, NULL, 26}, /* support needs to be added for snefru 128 */
	{"SNEFRU256", "snefru256", 27},
	{"MD2", "md2", 28},
	{"FNV132", "fnv132", 29},
	{"FNV1A32", "fnv1a32", 30},
	{"FNV164", "fnv164", 31},
	{"FNV1A64", "fnv1a64", 32},
	{"JOAAT", "joaat", 33},
	{"CRC32C", "crc32c", 34}, /* Castagnoli's CRC, used by iSCSI, SCTP, Btrfs, ext4, etc */
	{"MURMUR3A", "murmur3a", 35},
	{"MURMUR3C", "murmur3c", 36},
	{"MURMUR3F", "murmur3f", 37},
	{"XXH32", "xxh32", 38},
	{"XXH64", "xxh64", 39},
	{"XXH3", "xxh3", 40},
	{"XXH128", "xxh128", 41},
};
#endif

/* Hash Registry Access */

CRX_HASH_API const crx_hash_ops *crx_hash_fetch_ops(crex_string *algo) /* {{{ */
{
	crex_string *lower = crex_string_tolower(algo);
	crx_hash_ops *ops = crex_hash_find_ptr(&crx_hash_hashtable, lower);
	crex_string_release(lower);

	return ops;
}
/* }}} */

CRX_HASH_API void crx_hash_register_algo(const char *algo, const crx_hash_ops *ops) /* {{{ */
{
	size_t algo_len = strlen(algo);
	char *lower = crex_str_tolower_dup(algo, algo_len);
	crex_hash_add_ptr(&crx_hash_hashtable, crex_string_init_interned(lower, algo_len, 1), (void *) ops);
	efree(lower);
}
/* }}} */

CRX_HASH_API int crx_hash_copy(const void *ops, void *orig_context, void *dest_context) /* {{{ */
{
	crx_hash_ops *hash_ops = (crx_hash_ops *)ops;

	memcpy(dest_context, orig_context, hash_ops->context_size);
	return SUCCESS;
}
/* }}} */


static inline size_t align_to(size_t pos, size_t alignment) {
	size_t offset = pos & (alignment - 1);
	return pos + (offset ? alignment - offset : 0);
}

static size_t parse_serialize_spec(
		const char **specp, size_t *pos, size_t *sz, size_t *max_alignment) {
	size_t count, alignment;
	const char *spec = *specp;
	/* parse size */
	if (*spec == 's' || *spec == 'S') {
		*sz = 2;
		alignment = __alignof__(uint16_t); /* usually 2 */
	} else if (*spec == 'l' || *spec == 'L') {
		*sz = 4;
		alignment = __alignof__(uint32_t); /* usually 4 */
	} else if (*spec == 'q' || *spec == 'Q') {
		*sz = 8;
		alignment = __alignof__(uint64_t); /* usually 8 */
	} else if (*spec == 'i' || *spec == 'I') {
		*sz = sizeof(int);
		alignment = __alignof__(int);      /* usually 4 */
	} else {
		CREX_ASSERT(*spec == 'b' || *spec == 'B');
		*sz = 1;
		alignment = 1;
	}
	/* process alignment */
	*pos = align_to(*pos, alignment);
	*max_alignment = *max_alignment < alignment ? alignment : *max_alignment;
	/* parse count */
	++spec;
	if (isdigit((unsigned char) *spec)) {
		count = 0;
		while (isdigit((unsigned char) *spec)) {
			count = 10 * count + *spec - '0';
			++spec;
		}
	} else {
		count = 1;
	}
	*specp = spec;
	return count;
}

static uint64_t one_from_buffer(size_t sz, const unsigned char *buf) {
	if (sz == 2) {
		const uint16_t *x = (const uint16_t *) buf;
		return *x;
	} else if (sz == 4) {
		const uint32_t *x = (const uint32_t *) buf;
		return *x;
	} else if (sz == 8) {
		const uint64_t *x = (const uint64_t *) buf;
		return *x;
	} else {
		CREX_ASSERT(sz == 1);
		return *buf;
	}
}

static void one_to_buffer(size_t sz, unsigned char *buf, uint64_t val) {
	if (sz == 2) {
		uint16_t *x = (uint16_t *) buf;
		*x = val;
	} else if (sz == 4) {
		uint32_t *x = (uint32_t *) buf;
		*x = val;
	} else if (sz == 8) {
		uint64_t *x = (uint64_t *) buf;
		*x = val;
	} else {
		CREX_ASSERT(sz == 1);
		*buf = val;
	}
}

/* Serialize a hash context according to a `spec` string.
   Spec contents:
   b[COUNT] -- serialize COUNT bytes
   s[COUNT] -- serialize COUNT 16-bit integers
   l[COUNT] -- serialize COUNT 32-bit integers
   q[COUNT] -- serialize COUNT 64-bit integers
   i[COUNT] -- serialize COUNT `int`s
   B[COUNT] -- skip COUNT bytes
   S[COUNT], L[COUNT], etc. -- uppercase versions skip instead of read
   . (must be last character) -- assert that the hash context has exactly
       this size
   Example: "llllllb64l16." is the spec for an MD5 context: 6 32-bit
   integers, followed by 64 bytes, then 16 32-bit integers, and that's
   exactly the size of the context.

   The serialization result is an array. Each integer is serialized as a
   32-bit integer, except that a run of 2 or more bytes is encoded as a
   string, and each 64-bit integer is serialized as two 32-bit integers, least
   significant bits first. This allows 32-bit and 64-bit architectures to
   interchange serialized HashContexts. */

CRX_HASH_API int crx_hash_serialize_spec(const crx_hashcontext_object *hash, zval *zv, const char *spec) /* {{{ */
{
	size_t pos = 0, max_alignment = 1;
	unsigned char *buf = (unsigned char *) hash->context;
	zval tmp;
	if (buf == NULL) {
		return FAILURE;
	}
	array_init(zv);
	while (*spec != '\0' && *spec != '.') {
		char spec_ch = *spec;
		size_t sz, count = parse_serialize_spec(&spec, &pos, &sz, &max_alignment);
		if (pos + count * sz > hash->ops->context_size) {
			return FAILURE;
		}
		if (isupper((unsigned char) spec_ch)) {
			pos += count * sz;
		} else if (sz == 1 && count > 1) {
			ZVAL_STRINGL(&tmp, (char *) buf + pos, count);
			crex_hash_next_index_insert(C_ARRVAL_P(zv), &tmp);
			pos += count;
		} else {
			while (count > 0) {
				uint64_t val = one_from_buffer(sz, buf + pos);
				pos += sz;
				ZVAL_LONG(&tmp, (int32_t) val);
				crex_hash_next_index_insert(C_ARRVAL_P(zv), &tmp);
				if (sz == 8) {
					ZVAL_LONG(&tmp, (int32_t) (val >> 32));
					crex_hash_next_index_insert(C_ARRVAL_P(zv), &tmp);
				}
				--count;
			}
		}
	}
	if (*spec == '.' && align_to(pos, max_alignment) != hash->ops->context_size) {
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

/* Unserialize a hash context serialized by `crx_hash_serialize_spec` with `spec`.
   Returns SUCCESS on success and a negative error code on failure.
   Codes: FAILURE (-1) == generic failure
   -999 == spec wrong size for context
   -1000 - POS == problem at byte offset POS */

CRX_HASH_API int crx_hash_unserialize_spec(crx_hashcontext_object *hash, const zval *zv, const char *spec) /* {{{ */
{
	size_t pos = 0, max_alignment = 1, j = 0;
	unsigned char *buf = (unsigned char *) hash->context;
	zval *elt;
	if (C_TYPE_P(zv) != IS_ARRAY) {
		return FAILURE;
	}
	while (*spec != '\0' && *spec != '.') {
		char spec_ch = *spec;
		size_t sz, count = parse_serialize_spec(&spec, &pos, &sz, &max_alignment);
		if (pos + count * sz > hash->ops->context_size) {
			return -999;
		}
		if (isupper((unsigned char) spec_ch)) {
			pos += count * sz;
		} else if (sz == 1 && count > 1) {
			elt = crex_hash_index_find(C_ARRVAL_P(zv), j);
			if (!elt || C_TYPE_P(elt) != IS_STRING || C_STRLEN_P(elt) != count) {
				return -1000 - pos;
			}
			++j;
			memcpy(buf + pos, C_STRVAL_P(elt), count);
			pos += count;
		} else {
			while (count > 0) {
				uint64_t val;
				elt = crex_hash_index_find(C_ARRVAL_P(zv), j);
				if (!elt || C_TYPE_P(elt) != IS_LONG) {
					return -1000 - pos;
				}
				++j;
				val = (uint32_t) C_LVAL_P(elt);
				if (sz == 8) {
					elt = crex_hash_index_find(C_ARRVAL_P(zv), j);
					if (!elt || C_TYPE_P(elt) != IS_LONG) {
						return -1000 - pos;
					}
					++j;
					val += ((uint64_t) C_LVAL_P(elt)) << 32;
				}
				one_to_buffer(sz, buf + pos, val);
				pos += sz;
				--count;
			}
		}
	}
	if (*spec == '.' && align_to(pos, max_alignment) != hash->ops->context_size) {
		return -999;
	}
	return SUCCESS;
}
/* }}} */

CRX_HASH_API int crx_hash_serialize(const crx_hashcontext_object *hash, crex_long *magic, zval *zv) /* {{{ */
{
	if (hash->ops->serialize_spec) {
		*magic = CRX_HASH_SERIALIZE_MAGIC_SPEC;
		return crx_hash_serialize_spec(hash, zv, hash->ops->serialize_spec);
	} else {
		return FAILURE;
	}
}
/* }}} */

CRX_HASH_API int crx_hash_unserialize(crx_hashcontext_object *hash, crex_long magic, const zval *zv) /* {{{ */
{
	if (hash->ops->serialize_spec
		&& magic == CRX_HASH_SERIALIZE_MAGIC_SPEC) {
		return crx_hash_unserialize_spec(hash, zv, hash->ops->serialize_spec);
	} else {
		return FAILURE;
	}
}
/* }}} */

/* Userspace */

static void crx_hash_do_hash(
	zval *return_value, crex_string *algo, char *data, size_t data_len, bool raw_output, bool isfilename, HashTable *args
) /* {{{ */ {
	crex_string *digest;
	const crx_hash_ops *ops;
	void *context;
	crx_stream *stream = NULL;

	ops = crx_hash_fetch_ops(algo);
	if (!ops) {
		crex_argument_value_error(1, "must be a valid hashing algorithm");
		RETURN_THROWS();
	}
	if (isfilename) {
		if (CHECK_NULL_PATH(data, data_len)) {
			crex_argument_value_error(1, "must not contain any null bytes");
			RETURN_THROWS();
		}
		stream = crx_stream_open_wrapper_ex(data, "rb", REPORT_ERRORS, NULL, FG(default_context));
		if (!stream) {
			/* Stream will report errors opening file */
			RETURN_FALSE;
		}
	}

	context = crx_hash_alloc_context(ops);
	ops->hash_init(context, args);

	if (isfilename) {
		char buf[1024];
		ssize_t n;

		while ((n = crx_stream_read(stream, buf, sizeof(buf))) > 0) {
			ops->hash_update(context, (unsigned char *) buf, n);
		}
		crx_stream_close(stream);
		if (n < 0) {
			efree(context);
			RETURN_FALSE;
		}
	} else {
		ops->hash_update(context, (unsigned char *) data, data_len);
	}

	digest = crex_string_alloc(ops->digest_size, 0);
	ops->hash_final((unsigned char *) ZSTR_VAL(digest), context);
	efree(context);

	if (raw_output) {
		ZSTR_VAL(digest)[ops->digest_size] = 0;
		RETURN_NEW_STR(digest);
	} else {
		crex_string *hex_digest = crex_string_safe_alloc(ops->digest_size, 2, 0, 0);

		crx_hash_bin2hex(ZSTR_VAL(hex_digest), (unsigned char *) ZSTR_VAL(digest), ops->digest_size);
		ZSTR_VAL(hex_digest)[2 * ops->digest_size] = 0;
		crex_string_release_ex(digest, 0);
		RETURN_NEW_STR(hex_digest);
	}
}
/* }}} */

/* {{{ Generate a hash of a given input string
Returns lowercase hexits by default */
CRX_FUNCTION(hash)
{
	crex_string *algo;
	char *data;
	size_t data_len;
	bool raw_output = 0;
	HashTable *args = NULL;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_STR(algo)
		C_PARAM_STRING(data, data_len)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(raw_output)
		C_PARAM_ARRAY_HT(args)
	CREX_PARSE_PARAMETERS_END();

	crx_hash_do_hash(return_value, algo, data, data_len, raw_output, 0, args);
}
/* }}} */

/* {{{ Generate a hash of a given file
Returns lowercase hexits by default */
CRX_FUNCTION(hash_file)
{
	crex_string *algo;
	char *data;
	size_t data_len;
	bool raw_output = 0;
	HashTable *args = NULL;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_STR(algo)
		C_PARAM_STRING(data, data_len)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(raw_output)
		C_PARAM_ARRAY_HT(args)
	CREX_PARSE_PARAMETERS_END();

	crx_hash_do_hash(return_value, algo, data, data_len, raw_output, 1, args);
}
/* }}} */

static inline void crx_hash_string_xor_char(unsigned char *out, const unsigned char *in, const unsigned char xor_with, const size_t length) {
	size_t i;
	for (i=0; i < length; i++) {
		out[i] = in[i] ^ xor_with;
	}
}

static inline void crx_hash_string_xor(unsigned char *out, const unsigned char *in, const unsigned char *xor_with, const size_t length) {
	size_t i;
	for (i=0; i < length; i++) {
		out[i] = in[i] ^ xor_with[i];
	}
}

static inline void crx_hash_hmac_prep_key(unsigned char *K, const crx_hash_ops *ops, void *context, const unsigned char *key, const size_t key_len) {
	memset(K, 0, ops->block_size);
	if (key_len > ops->block_size) {
		/* Reduce the key first */
		ops->hash_init(context, NULL);
		ops->hash_update(context, key, key_len);
		ops->hash_final(K, context);
	} else {
		memcpy(K, key, key_len);
	}
	/* XOR the key with 0x36 to get the ipad) */
	crx_hash_string_xor_char(K, K, 0x36, ops->block_size);
}

static inline void crx_hash_hmac_round(unsigned char *final, const crx_hash_ops *ops, void *context, const unsigned char *key, const unsigned char *data, const crex_long data_size) {
	ops->hash_init(context, NULL);
	ops->hash_update(context, key, ops->block_size);
	ops->hash_update(context, data, data_size);
	ops->hash_final(final, context);
}

static void crx_hash_do_hash_hmac(
	zval *return_value, crex_string *algo, char *data, size_t data_len, char *key, size_t key_len, bool raw_output, bool isfilename
) /* {{{ */ {
	crex_string *digest;
	unsigned char *K;
	const crx_hash_ops *ops;
	void *context;
	crx_stream *stream = NULL;

	ops = crx_hash_fetch_ops(algo);
	if (!ops || !ops->is_crypto) {
		crex_argument_value_error(1, "must be a valid cryptographic hashing algorithm");
		RETURN_THROWS();
	}

	if (isfilename) {
		if (CHECK_NULL_PATH(data, data_len)) {
			crex_argument_value_error(2, "must not contain any null bytes");
			RETURN_THROWS();
		}
		stream = crx_stream_open_wrapper_ex(data, "rb", REPORT_ERRORS, NULL, FG(default_context));
		if (!stream) {
			/* Stream will report errors opening file */
			RETURN_FALSE;
		}
	}

	context = crx_hash_alloc_context(ops);

	K = emalloc(ops->block_size);
	digest = crex_string_alloc(ops->digest_size, 0);

	crx_hash_hmac_prep_key(K, ops, context, (unsigned char *) key, key_len);

	if (isfilename) {
		char buf[1024];
		ssize_t n;
		ops->hash_init(context, NULL);
		ops->hash_update(context, K, ops->block_size);
		while ((n = crx_stream_read(stream, buf, sizeof(buf))) > 0) {
			ops->hash_update(context, (unsigned char *) buf, n);
		}
		crx_stream_close(stream);
		if (n < 0) {
			efree(context);
			efree(K);
			crex_string_release(digest);
			RETURN_FALSE;
		}

		ops->hash_final((unsigned char *) ZSTR_VAL(digest), context);
	} else {
		crx_hash_hmac_round((unsigned char *) ZSTR_VAL(digest), ops, context, K, (unsigned char *) data, data_len);
	}

	crx_hash_string_xor_char(K, K, 0x6A, ops->block_size);

	crx_hash_hmac_round((unsigned char *) ZSTR_VAL(digest), ops, context, K, (unsigned char *) ZSTR_VAL(digest), ops->digest_size);

	/* Zero the key */
	CREX_SECURE_ZERO(K, ops->block_size);
	efree(K);
	efree(context);

	if (raw_output) {
		ZSTR_VAL(digest)[ops->digest_size] = 0;
		RETURN_NEW_STR(digest);
	} else {
		crex_string *hex_digest = crex_string_safe_alloc(ops->digest_size, 2, 0, 0);

		crx_hash_bin2hex(ZSTR_VAL(hex_digest), (unsigned char *) ZSTR_VAL(digest), ops->digest_size);
		ZSTR_VAL(hex_digest)[2 * ops->digest_size] = 0;
		crex_string_release_ex(digest, 0);
		RETURN_NEW_STR(hex_digest);
	}
}
/* }}} */

/* {{{ Generate a hash of a given input string with a key using HMAC
Returns lowercase hexits by default */
CRX_FUNCTION(hash_hmac)
{
	crex_string *algo;
	char *data, *key;
	size_t data_len, key_len;
	bool raw_output = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Sss|b", &algo, &data, &data_len, &key, &key_len, &raw_output) == FAILURE) {
		RETURN_THROWS();
	}

	crx_hash_do_hash_hmac(return_value, algo, data, data_len, key, key_len, raw_output, 0);
}
/* }}} */

/* {{{ Generate a hash of a given file with a key using HMAC
Returns lowercase hexits by default */
CRX_FUNCTION(hash_hmac_file)
{
	crex_string *algo;
	char *data, *key;
	size_t data_len, key_len;
	bool raw_output = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Sss|b", &algo, &data, &data_len, &key, &key_len, &raw_output) == FAILURE) {
		RETURN_THROWS();
	}

	crx_hash_do_hash_hmac(return_value, algo, data, data_len, key, key_len, raw_output, 1);
}
/* }}} */

/* {{{ Initialize a hashing context */
CRX_FUNCTION(hash_init)
{
	crex_string *algo, *key = NULL;
	crex_long options = 0;
	void *context;
	const crx_hash_ops *ops;
	crx_hashcontext_object *hash;
	HashTable *args = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S|lSh", &algo, &options, &key, &args) == FAILURE) {
		RETURN_THROWS();
	}

	ops = crx_hash_fetch_ops(algo);
	if (!ops) {
		crex_argument_value_error(1, "must be a valid hashing algorithm");
		RETURN_THROWS();
	}

	if (options & CRX_HASH_HMAC) {
		if (!ops->is_crypto) {
			crex_argument_value_error(1, "must be a cryptographic hashing algorithm if HMAC is requested");
			RETURN_THROWS();
		}
		if (!key || (ZSTR_LEN(key) == 0)) {
			/* Note: a zero length key is no key at all */
			crex_argument_value_error(3, "cannot be empty when HMAC is requested");
			RETURN_THROWS();
		}
	}

	object_init_ex(return_value, crx_hashcontext_ce);
	hash = crx_hashcontext_from_object(C_OBJ_P(return_value));

	context = crx_hash_alloc_context(ops);
	ops->hash_init(context, args);

	hash->ops = ops;
	hash->context = context;
	hash->options = options;
	hash->key = NULL;

	if (options & CRX_HASH_HMAC) {
		char *K = emalloc(ops->block_size);
		size_t i, block_size;

		memset(K, 0, ops->block_size);

		if (ZSTR_LEN(key) > ops->block_size) {
			/* Reduce the key first */
			ops->hash_update(context, (unsigned char *) ZSTR_VAL(key), ZSTR_LEN(key));
			ops->hash_final((unsigned char *) K, context);
			/* Make the context ready to start over */
			ops->hash_init(context, args);
		} else {
			memcpy(K, ZSTR_VAL(key), ZSTR_LEN(key));
		}

		/* XOR ipad */
		block_size = ops->block_size;
		for(i = 0; i < block_size; i++) {
			K[i] ^= 0x36;
		}
		ops->hash_update(context, (unsigned char *) K, ops->block_size);
		hash->key = (unsigned char *) K;
	}
}
/* }}} */

#define CRX_HASHCONTEXT_VERIFY(hash) { \
	if (!hash->context) { \
		crex_argument_type_error(1, "must be a valid, non-finalized HashContext"); \
		RETURN_THROWS(); \
	} \
}

/* {{{ Pump data into the hashing algorithm */
CRX_FUNCTION(hash_update)
{
	zval *zhash;
	crx_hashcontext_object *hash;
	crex_string *data;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OS", &zhash, crx_hashcontext_ce, &data) == FAILURE) {
		RETURN_THROWS();
	}

	hash = crx_hashcontext_from_object(C_OBJ_P(zhash));
	CRX_HASHCONTEXT_VERIFY(hash);
	hash->ops->hash_update(hash->context, (unsigned char *) ZSTR_VAL(data), ZSTR_LEN(data));

	RETURN_TRUE;
}
/* }}} */

/* {{{ Pump data into the hashing algorithm from an open stream */
CRX_FUNCTION(hash_update_stream)
{
	zval *zhash, *zstream;
	crx_hashcontext_object *hash;
	crx_stream *stream = NULL;
	crex_long length = -1, didread = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Or|l", &zhash, crx_hashcontext_ce, &zstream, &length) == FAILURE) {
		RETURN_THROWS();
	}

	hash = crx_hashcontext_from_object(C_OBJ_P(zhash));
	CRX_HASHCONTEXT_VERIFY(hash);
	crx_stream_from_zval(stream, zstream);

	while (length) {
		char buf[1024];
		crex_long toread = 1024;
		ssize_t n;

		if (length > 0 && toread > length) {
			toread = length;
		}

		if ((n = crx_stream_read(stream, buf, toread)) <= 0) {
			RETURN_LONG(didread);
		}
		hash->ops->hash_update(hash->context, (unsigned char *) buf, n);
		length -= n;
		didread += n;
	}

	RETURN_LONG(didread);
}
/* }}} */

/* {{{ Pump data into the hashing algorithm from a file */
CRX_FUNCTION(hash_update_file)
{
	zval *zhash, *zcontext = NULL;
	crx_hashcontext_object *hash;
	crx_stream_context *context = NULL;
	crx_stream *stream;
	crex_string *filename;
	char buf[1024];
	ssize_t n;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OP|r!", &zhash, crx_hashcontext_ce, &filename, &zcontext) == FAILURE) {
		RETURN_THROWS();
	}

	hash = crx_hashcontext_from_object(C_OBJ_P(zhash));
	CRX_HASHCONTEXT_VERIFY(hash);
	context = crx_stream_context_from_zval(zcontext, 0);

	stream = crx_stream_open_wrapper_ex(ZSTR_VAL(filename), "rb", REPORT_ERRORS, NULL, context);
	if (!stream) {
		/* Stream will report errors opening file */
		RETURN_FALSE;
	}

	while ((n = crx_stream_read(stream, buf, sizeof(buf))) > 0) {
		hash->ops->hash_update(hash->context, (unsigned char *) buf, n);
	}
	crx_stream_close(stream);

	RETURN_BOOL(n >= 0);
}
/* }}} */

/* {{{ Output resulting digest */
CRX_FUNCTION(hash_final)
{
	zval *zhash;
	crx_hashcontext_object *hash;
	bool raw_output = 0;
	crex_string *digest;
	size_t digest_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O|b", &zhash, crx_hashcontext_ce, &raw_output) == FAILURE) {
		RETURN_THROWS();
	}

	hash = crx_hashcontext_from_object(C_OBJ_P(zhash));
	CRX_HASHCONTEXT_VERIFY(hash);

	digest_len = hash->ops->digest_size;
	digest = crex_string_alloc(digest_len, 0);
	hash->ops->hash_final((unsigned char *) ZSTR_VAL(digest), hash->context);
	if (hash->options & CRX_HASH_HMAC) {
		size_t i, block_size;

		/* Convert K to opad -- 0x6A = 0x36 ^ 0x5C */
		block_size = hash->ops->block_size;
		for(i = 0; i < block_size; i++) {
			hash->key[i] ^= 0x6A;
		}

		/* Feed this result into the outer hash */
		hash->ops->hash_init(hash->context, NULL);
		hash->ops->hash_update(hash->context, hash->key, hash->ops->block_size);
		hash->ops->hash_update(hash->context, (unsigned char *) ZSTR_VAL(digest), hash->ops->digest_size);
		hash->ops->hash_final((unsigned char *) ZSTR_VAL(digest), hash->context);

		/* Zero the key */
		CREX_SECURE_ZERO(hash->key, hash->ops->block_size);
		efree(hash->key);
		hash->key = NULL;
	}
	ZSTR_VAL(digest)[digest_len] = 0;

	/* Invalidate the object from further use */
	efree(hash->context);
	hash->context = NULL;

	if (raw_output) {
		RETURN_NEW_STR(digest);
	} else {
		crex_string *hex_digest = crex_string_safe_alloc(digest_len, 2, 0, 0);

		crx_hash_bin2hex(ZSTR_VAL(hex_digest), (unsigned char *) ZSTR_VAL(digest), digest_len);
		ZSTR_VAL(hex_digest)[2 * digest_len] = 0;
		crex_string_release_ex(digest, 0);
		RETURN_NEW_STR(hex_digest);
	}
}
/* }}} */

/* {{{ Copy hash object */
CRX_FUNCTION(hash_copy)
{
	zval *zhash;
	crx_hashcontext_object *context;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &zhash, crx_hashcontext_ce) == FAILURE) {
		RETURN_THROWS();
	}

	context = crx_hashcontext_from_object(C_OBJ_P(zhash));
	CRX_HASHCONTEXT_VERIFY(context);

	RETVAL_OBJ(C_OBJ_HANDLER_P(zhash, clone_obj)(C_OBJ_P(zhash)));

	if (crx_hashcontext_from_object(C_OBJ_P(return_value))->context == NULL) {
		zval_ptr_dtor(return_value);

		crex_throw_error(NULL, "Cannot copy hash");
		RETURN_THROWS();
	}
}
/* }}} */

/* {{{ Return a list of registered hashing algorithms */
CRX_FUNCTION(hash_algos)
{
	crex_string *str;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);
	CREX_HASH_MAP_FOREACH_STR_KEY(&crx_hash_hashtable, str) {
		add_next_index_str(return_value, crex_string_copy(str));
	} CREX_HASH_FOREACH_END();
}
/* }}} */

/* {{{ Return a list of registered hashing algorithms suitable for hash_hmac() */
CRX_FUNCTION(hash_hmac_algos)
{
	crex_string *str;
	const crx_hash_ops *ops;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);
	CREX_HASH_MAP_FOREACH_STR_KEY_PTR(&crx_hash_hashtable, str, ops) {
		if (ops->is_crypto) {
			add_next_index_str(return_value, crex_string_copy(str));
		}
	} CREX_HASH_FOREACH_END();
}
/* }}} */

/* {{{ RFC5869 HMAC-based key derivation function */
CRX_FUNCTION(hash_hkdf)
{
	crex_string *returnval, *ikm, *algo, *info = NULL, *salt = NULL;
	crex_long length = 0;
	unsigned char *prk, *digest, *K;
	size_t i;
	size_t rounds;
	const crx_hash_ops *ops;
	void *context;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "SS|lSS", &algo, &ikm, &length, &info, &salt) == FAILURE) {
		RETURN_THROWS();
	}

	ops = crx_hash_fetch_ops(algo);
	if (!ops || !ops->is_crypto) {
		crex_argument_value_error(1, "must be a valid cryptographic hashing algorithm");
		RETURN_THROWS();
	}

	if (ZSTR_LEN(ikm) == 0) {
		crex_argument_value_error(2, "cannot be empty");
		RETURN_THROWS();
	}

	if (length < 0) {
		crex_argument_value_error(3, "must be greater than or equal to 0");
		RETURN_THROWS();
	} else if (length == 0) {
		length = ops->digest_size;
	} else if (length > (crex_long) (ops->digest_size * 255)) {
		crex_argument_value_error(3, "must be less than or equal to %zd", ops->digest_size * 255);
		RETURN_THROWS();
	}

	context = crx_hash_alloc_context(ops);

	// Extract
	ops->hash_init(context, NULL);
	K = emalloc(ops->block_size);
	crx_hash_hmac_prep_key(K, ops, context,
		(unsigned char *) (salt ? ZSTR_VAL(salt) : ""), salt ? ZSTR_LEN(salt) : 0);

	prk = emalloc(ops->digest_size);
	crx_hash_hmac_round(prk, ops, context, K, (unsigned char *) ZSTR_VAL(ikm), ZSTR_LEN(ikm));
	crx_hash_string_xor_char(K, K, 0x6A, ops->block_size);
	crx_hash_hmac_round(prk, ops, context, K, prk, ops->digest_size);
	CREX_SECURE_ZERO(K, ops->block_size);

	// Expand
	returnval = crex_string_alloc(length, 0);
	digest = emalloc(ops->digest_size);
	for (i = 1, rounds = (length - 1) / ops->digest_size + 1; i <= rounds; i++) {
		// chr(i)
		unsigned char c[1];
		c[0] = (i & 0xFF);

		crx_hash_hmac_prep_key(K, ops, context, prk, ops->digest_size);
		ops->hash_init(context, NULL);
		ops->hash_update(context, K, ops->block_size);

		if (i > 1) {
			ops->hash_update(context, digest, ops->digest_size);
		}

		if (info != NULL && ZSTR_LEN(info) > 0) {
			ops->hash_update(context, (unsigned char *) ZSTR_VAL(info), ZSTR_LEN(info));
		}

		ops->hash_update(context, c, 1);
		ops->hash_final(digest, context);
		crx_hash_string_xor_char(K, K, 0x6A, ops->block_size);
		crx_hash_hmac_round(digest, ops, context, K, digest, ops->digest_size);
		memcpy(
			ZSTR_VAL(returnval) + ((i - 1) * ops->digest_size),
			digest,
			(i == rounds ? length - ((i - 1) * ops->digest_size) : ops->digest_size)
		);
	}

	CREX_SECURE_ZERO(K, ops->block_size);
	CREX_SECURE_ZERO(digest, ops->digest_size);
	CREX_SECURE_ZERO(prk, ops->digest_size);
	efree(K);
	efree(context);
	efree(prk);
	efree(digest);
	ZSTR_VAL(returnval)[length] = 0;
	RETURN_STR(returnval);
}

/* {{{ Generate a PBKDF2 hash of the given password and salt
Returns lowercase hexits by default */
CRX_FUNCTION(hash_pbkdf2)
{
	crex_string *returnval, *algo;
	char *salt, *pass = NULL;
	unsigned char *computed_salt, *digest, *temp, *result, *K1, *K2 = NULL;
	crex_long loops, i, j, iterations, digest_length = 0, length = 0;
	size_t pass_len, salt_len = 0;
	bool raw_output = 0;
	const crx_hash_ops *ops;
	void *context;
	HashTable *args = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Sssl|lbh", &algo, &pass, &pass_len, &salt, &salt_len, &iterations, &length, &raw_output, &args) == FAILURE) {
		RETURN_THROWS();
	}

	ops = crx_hash_fetch_ops(algo);
	if (!ops || !ops->is_crypto) {
		crex_argument_value_error(1, "must be a valid cryptographic hashing algorithm");
		RETURN_THROWS();
	}

	if (salt_len > INT_MAX - 4) {
		crex_argument_value_error(3, "must be less than or equal to INT_MAX - 4 bytes");
		RETURN_THROWS();
	}

	if (iterations <= 0) {
		crex_argument_value_error(4, "must be greater than 0");
		RETURN_THROWS();
	}

	if (length < 0) {
		crex_argument_value_error(5, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	context = crx_hash_alloc_context(ops);
	ops->hash_init(context, args);

	K1 = emalloc(ops->block_size);
	K2 = emalloc(ops->block_size);
	digest = emalloc(ops->digest_size);
	temp = emalloc(ops->digest_size);

	/* Setup Keys that will be used for all hmac rounds */
	crx_hash_hmac_prep_key(K1, ops, context, (unsigned char *) pass, pass_len);
	/* Convert K1 to opad -- 0x6A = 0x36 ^ 0x5C */
	crx_hash_string_xor_char(K2, K1, 0x6A, ops->block_size);

	/* Setup Main Loop to build a long enough result */
	if (length == 0) {
		length = ops->digest_size;
		if (!raw_output) {
			length = length * 2;
		}
	}
	digest_length = length;
	if (!raw_output) {
		digest_length = (crex_long) ceil((float) length / 2.0);
	}

	loops = (crex_long) ceil((float) digest_length / (float) ops->digest_size);

	result = safe_emalloc(loops, ops->digest_size, 0);

	computed_salt = safe_emalloc(salt_len, 1, 4);
	memcpy(computed_salt, (unsigned char *) salt, salt_len);

	for (i = 1; i <= loops; i++) {
		/* digest = hash_hmac(salt + pack('N', i), password) { */

		/* pack("N", i) */
		computed_salt[salt_len] = (unsigned char) (i >> 24);
		computed_salt[salt_len + 1] = (unsigned char) ((i & 0xFF0000) >> 16);
		computed_salt[salt_len + 2] = (unsigned char) ((i & 0xFF00) >> 8);
		computed_salt[salt_len + 3] = (unsigned char) (i & 0xFF);

		crx_hash_hmac_round(digest, ops, context, K1, computed_salt, (crex_long) salt_len + 4);
		crx_hash_hmac_round(digest, ops, context, K2, digest, ops->digest_size);
		/* } */

		/* temp = digest */
		memcpy(temp, digest, ops->digest_size);

		/*
		 * Note that the loop starting at 1 is intentional, since we've already done
		 * the first round of the algorithm.
		 */
		for (j = 1; j < iterations; j++) {
			/* digest = hash_hmac(digest, password) { */
			crx_hash_hmac_round(digest, ops, context, K1, digest, ops->digest_size);
			crx_hash_hmac_round(digest, ops, context, K2, digest, ops->digest_size);
			/* } */
			/* temp ^= digest */
			crx_hash_string_xor(temp, temp, digest, ops->digest_size);
		}
		/* result += temp */
		memcpy(result + ((i - 1) * ops->digest_size), temp, ops->digest_size);
	}
	/* Zero potentially sensitive variables */
	CREX_SECURE_ZERO(K1, ops->block_size);
	CREX_SECURE_ZERO(K2, ops->block_size);
	CREX_SECURE_ZERO(computed_salt, salt_len + 4);
	efree(K1);
	efree(K2);
	efree(computed_salt);
	efree(context);
	efree(digest);
	efree(temp);

	returnval = crex_string_alloc(length, 0);
	if (raw_output) {
		memcpy(ZSTR_VAL(returnval), result, length);
	} else {
		crx_hash_bin2hex(ZSTR_VAL(returnval), result, digest_length);
	}
	ZSTR_VAL(returnval)[length] = 0;
	efree(result);
	RETURN_NEW_STR(returnval);
}
/* }}} */

/* {{{ Compares two strings using the same time whether they're equal or not.
   A difference in length will leak */
CRX_FUNCTION(hash_equals)
{
	zval *known_zval, *user_zval;
	int result = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "zz", &known_zval, &user_zval) == FAILURE) {
		RETURN_THROWS();
	}

	/* We only allow comparing string to prevent unexpected results. */
	if (C_TYPE_P(known_zval) != IS_STRING) {
		crex_argument_type_error(1, "must be of type string, %s given", crex_zval_value_name(known_zval));
		RETURN_THROWS();
	}

	if (C_TYPE_P(user_zval) != IS_STRING) {
		crex_argument_type_error(2, "must be of type string, %s given", crex_zval_value_name(user_zval));
		RETURN_THROWS();
	}

	/* This is security sensitive code. Do not optimize this for speed. */
	result = crx_safe_bcmp(C_STR_P(known_zval), C_STR_P(user_zval));

	RETURN_BOOL(0 == result);
}
/* }}} */

/* {{{ */
CRX_METHOD(HashContext, __main) {
	/* Normally unreachable as private/final */
	crex_throw_exception(crex_ce_error, "Illegal call to private/final constructor", 0);
}
/* }}} */

/* Module Housekeeping */

#define CRX_HASH_HAVAL_REGISTER(p,b)	crx_hash_register_algo("haval" #b "," #p , &crx_hash_##p##haval##b##_ops);

#ifdef CRX_MHASH_BC

#if 0
/* See #69823, we should not insert module into module_registry while doing startup */

CRX_MINFO_FUNCTION(mhash)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "MHASH support", "Enabled");
	crx_info_print_table_row(2, "MHASH API Version", "Emulated Support");
	crx_info_print_table_end();
}

crex_module_entry mhash_module_entry = {
	STANDARD_MODULE_HEADER,
	"mhash",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	CRX_MINFO(mhash),
	CRX_MHASH_VERSION,
	STANDARD_MODULE_PROPERTIES,
};
#endif

static void mhash_init(INIT_FUNC_ARGS)
{
	char buf[128];
	int len;
	int algo_number = 0;

	for (algo_number = 0; algo_number < MHASH_NUM_ALGOS; algo_number++) {
		struct mhash_bc_entry algorithm = mhash_to_hash[algo_number];
		if (algorithm.mhash_name == NULL) {
			continue;
		}

		len = slprintf(buf, 127, "MHASH_%s", algorithm.mhash_name);
		crex_register_long_constant(buf, len, algorithm.value, CONST_PERSISTENT, module_number);
	}

	/* TODO: this cause #69823 crex_register_internal_module(&mhash_module_entry); */
}

/* {{{ Hash data with hash */
CRX_FUNCTION(mhash)
{
	crex_long algorithm;
	crex_string *algo = NULL;
	char *data, *key = NULL;
	size_t data_len, key_len = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ls|s!", &algorithm, &data, &data_len, &key, &key_len) == FAILURE) {
		RETURN_THROWS();
	}

	/* need to convert the first parameter from int constant to string algorithm name */
	if (algorithm >= 0 && algorithm < MHASH_NUM_ALGOS) {
		struct mhash_bc_entry algorithm_lookup = mhash_to_hash[algorithm];
		if (algorithm_lookup.hash_name) {
			algo = crex_string_init(algorithm_lookup.hash_name, strlen(algorithm_lookup.hash_name), 0);
		}
	}

	if (key) {
		crx_hash_do_hash_hmac(return_value, algo, data, data_len, key, key_len, 1, 0);
	} else {
		crx_hash_do_hash(return_value, algo, data, data_len, 1, 0, NULL);
	}

	if (algo) {
		crex_string_release(algo);
	}
}
/* }}} */

/* {{{ Gets the name of hash */
CRX_FUNCTION(mhash_get_hash_name)
{
	crex_long algorithm;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &algorithm) == FAILURE) {
		RETURN_THROWS();
	}

	if (algorithm >= 0 && algorithm  < MHASH_NUM_ALGOS) {
		struct mhash_bc_entry algorithm_lookup = mhash_to_hash[algorithm];
		if (algorithm_lookup.mhash_name) {
			RETURN_STRING(algorithm_lookup.mhash_name);
		}
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ Gets the number of available hashes */
CRX_FUNCTION(mhash_count)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	RETURN_LONG(MHASH_NUM_ALGOS - 1);
}
/* }}} */

/* {{{ Gets the block size of hash */
CRX_FUNCTION(mhash_get_block_size)
{
	crex_long algorithm;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &algorithm) == FAILURE) {
		RETURN_THROWS();
	}
	RETVAL_FALSE;

	if (algorithm >= 0 && algorithm  < MHASH_NUM_ALGOS) {
		struct mhash_bc_entry algorithm_lookup = mhash_to_hash[algorithm];
		if (algorithm_lookup.mhash_name) {
			const crx_hash_ops *ops = crex_hash_str_find_ptr(&crx_hash_hashtable, algorithm_lookup.hash_name, strlen(algorithm_lookup.hash_name));
			if (ops) {
				RETVAL_LONG(ops->digest_size);
			}
		}
	}
}
/* }}} */

#define SALT_SIZE 8

/* {{{ Generates a key using hash functions */
CRX_FUNCTION(mhash_keygen_s2k)
{
	crex_long algorithm, l_bytes;
	int bytes;
	char *password, *salt;
	size_t password_len, salt_len;
	char padded_salt[SALT_SIZE];

	if (crex_parse_parameters(CREX_NUM_ARGS(), "lssl", &algorithm, &password, &password_len, &salt, &salt_len, &l_bytes) == FAILURE) {
		RETURN_THROWS();
	}

	bytes = (int)l_bytes;
	if (bytes <= 0){
		crex_argument_value_error(4, "must be a greater than 0");
		RETURN_THROWS();
	}

	salt_len = MIN(salt_len, SALT_SIZE);

	memcpy(padded_salt, salt, salt_len);
	if (salt_len < SALT_SIZE) {
		memset(padded_salt + salt_len, 0, SALT_SIZE - salt_len);
	}
	salt_len = SALT_SIZE;

	RETVAL_FALSE;
	if (algorithm >= 0 && algorithm < MHASH_NUM_ALGOS) {
		struct mhash_bc_entry algorithm_lookup = mhash_to_hash[algorithm];
		if (algorithm_lookup.mhash_name) {
			const crx_hash_ops *ops = crex_hash_str_find_ptr(&crx_hash_hashtable, algorithm_lookup.hash_name, strlen(algorithm_lookup.hash_name));
			if (ops) {
				unsigned char null = '\0';
				void *context;
				char *key, *digest;
				int i = 0, j = 0;
				size_t block_size = ops->digest_size;
				size_t times = bytes / block_size;

				if ((bytes % block_size) != 0) {
					times++;
				}

				context = crx_hash_alloc_context(ops);
				ops->hash_init(context, NULL);

				key = ecalloc(1, times * block_size);
				digest = emalloc(ops->digest_size + 1);

				for (i = 0; i < times; i++) {
					ops->hash_init(context, NULL);

					for (j=0;j<i;j++) {
						ops->hash_update(context, &null, 1);
					}
					ops->hash_update(context, (unsigned char *)padded_salt, salt_len);
					ops->hash_update(context, (unsigned char *)password, password_len);
					ops->hash_final((unsigned char *)digest, context);
					memcpy( &key[i*block_size], digest, block_size);
				}

				RETVAL_STRINGL(key, bytes);
				CREX_SECURE_ZERO(key, bytes);
				efree(digest);
				efree(context);
				efree(key);
			}
		}
	}
}
/* }}} */

#endif

/* ----------------------------------------------------------------------- */

/* {{{ crx_hashcontext_create */
static crex_object* crx_hashcontext_create(crex_class_entry *ce) {
	crx_hashcontext_object *objval = crex_object_alloc(sizeof(crx_hashcontext_object), ce);
	crex_object *zobj = &objval->std;

	crex_object_std_init(zobj, ce);
	object_properties_init(zobj, ce);
	zobj->handlers = &crx_hashcontext_handlers;

	return zobj;
}
/* }}} */

/* {{{ crx_hashcontext_dtor */
static void crx_hashcontext_dtor(crex_object *obj) {
	crx_hashcontext_object *hash = crx_hashcontext_from_object(obj);

	if (hash->context) {
		efree(hash->context);
		hash->context = NULL;
	}

	if (hash->key) {
		CREX_SECURE_ZERO(hash->key, hash->ops->block_size);
		efree(hash->key);
		hash->key = NULL;
	}
}
/* }}} */

static void crx_hashcontext_free(crex_object *obj) {
	crx_hashcontext_dtor(obj);
	crex_object_std_dtor(obj);
}

/* {{{ crx_hashcontext_clone */
static crex_object *crx_hashcontext_clone(crex_object *zobj) {
	crx_hashcontext_object *oldobj = crx_hashcontext_from_object(zobj);
	crex_object *znew = crx_hashcontext_create(zobj->ce);
	crx_hashcontext_object *newobj = crx_hashcontext_from_object(znew);

	if (!oldobj->context) {
		crex_throw_exception(crex_ce_value_error, "Cannot clone a finalized HashContext", 0);
		return znew;
	}

	crex_objects_clone_members(znew, zobj);

	newobj->ops = oldobj->ops;
	newobj->options = oldobj->options;
	newobj->context = crx_hash_alloc_context(newobj->ops);
	newobj->ops->hash_init(newobj->context, NULL);

	if (SUCCESS != newobj->ops->hash_copy(newobj->ops, oldobj->context, newobj->context)) {
		efree(newobj->context);
		newobj->context = NULL;
		return znew;
	}

	newobj->key = ecalloc(1, newobj->ops->block_size);
	if (oldobj->key) {
		memcpy(newobj->key, oldobj->key, newobj->ops->block_size);
	}

	return znew;
}
/* }}} */

/* Serialization format: 5-element array
   Index 0: hash algorithm (string)
   Index 1: options (long, 0)
   Index 2: hash-determined serialization of context state (usually array)
   Index 3: magic number defining layout of context state (long, usually 2)
   Index 4: properties (array)

   HashContext serializations are not necessarily portable between architectures or
   CRX versions. If the format of a serialized hash context changes, that should
   be reflected in either a different value of `magic` or a different format of
   the serialized context state. Most context states are unparsed and parsed using
   a spec string, such as "llb128.", using the format defined by
   `crx_hash_serialize_spec`/`crx_hash_unserialize_spec`. Some hash algorithms must
   also check the unserialized state for validity, to ensure that using an
   unserialized context is safe from memory errors.

   Currently HASH_HMAC contexts cannot be serialized, because serializing them
   would require serializing the HMAC key in plaintext. */

/* {{{ Serialize the object */
CRX_METHOD(HashContext, __serialize)
{
	zval *object = CREX_THIS;
	crx_hashcontext_object *hash = crx_hashcontext_from_object(C_OBJ_P(object));
	crex_long magic = 0;
	zval tmp;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);

	if (!hash->ops->hash_serialize) {
		goto serialize_failure;
	} else if (hash->options & CRX_HASH_HMAC) {
		crex_throw_exception(NULL, "HashContext with HASH_HMAC option cannot be serialized", 0);
		RETURN_THROWS();
	}

	ZVAL_STRING(&tmp, hash->ops->algo);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &tmp);

	ZVAL_LONG(&tmp, hash->options);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &tmp);

	if (hash->ops->hash_serialize(hash, &magic, &tmp) != SUCCESS) {
		goto serialize_failure;
	}
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &tmp);

	ZVAL_LONG(&tmp, magic);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &tmp);

	/* members */
	ZVAL_ARR(&tmp, crex_std_get_properties(&hash->std));
	C_TRY_ADDREF(tmp);
	crex_hash_next_index_insert(C_ARRVAL_P(return_value), &tmp);

	return;

serialize_failure:
	crex_throw_exception_ex(NULL, 0, "HashContext for algorithm \"%s\" cannot be serialized", hash->ops->algo);
	RETURN_THROWS();
}
/* }}} */

/* {{{ unserialize the object */
CRX_METHOD(HashContext, __unserialize)
{
	zval *object = CREX_THIS;
	crx_hashcontext_object *hash = crx_hashcontext_from_object(C_OBJ_P(object));
	HashTable *data;
	zval *algo_zv, *magic_zv, *options_zv, *hash_zv, *members_zv;
	crex_long magic, options;
	int unserialize_result;
	const crx_hash_ops *ops;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "h", &data) == FAILURE) {
		RETURN_THROWS();
	}

	if (hash->context) {
		crex_throw_exception(NULL, "HashContext::__unserialize called on initialized object", 0);
		RETURN_THROWS();
	}

	algo_zv = crex_hash_index_find(data, 0);
	options_zv = crex_hash_index_find(data, 1);
	hash_zv = crex_hash_index_find(data, 2);
	magic_zv = crex_hash_index_find(data, 3);
	members_zv = crex_hash_index_find(data, 4);

	if (!algo_zv || C_TYPE_P(algo_zv) != IS_STRING
		|| !magic_zv || C_TYPE_P(magic_zv) != IS_LONG
		|| !options_zv || C_TYPE_P(options_zv) != IS_LONG
		|| !hash_zv
		|| !members_zv || C_TYPE_P(members_zv) != IS_ARRAY) {
		crex_throw_exception(NULL, "Incomplete or ill-formed serialization data", 0);
		RETURN_THROWS();
	}

	magic = C_LVAL_P(magic_zv);
	options = C_LVAL_P(options_zv);
	if (options & CRX_HASH_HMAC) {
		crex_throw_exception(NULL, "HashContext with HASH_HMAC option cannot be serialized", 0);
		RETURN_THROWS();
	}

	ops = crx_hash_fetch_ops(C_STR_P(algo_zv));
	if (!ops) {
		crex_throw_exception(NULL, "Unknown hash algorithm", 0);
		RETURN_THROWS();
	} else if (!ops->hash_unserialize) {
		crex_throw_exception_ex(NULL, 0, "Hash algorithm \"%s\" cannot be unserialized", ops->algo);
		RETURN_THROWS();
	}

	hash->ops = ops;
	hash->context = crx_hash_alloc_context(ops);
	hash->options = options;
	ops->hash_init(hash->context, NULL);

	unserialize_result = ops->hash_unserialize(hash, magic, hash_zv);
	if (unserialize_result != SUCCESS) {
		crex_throw_exception_ex(NULL, 0, "Incomplete or ill-formed serialization data (\"%s\" code %d)", ops->algo, unserialize_result);
		/* free context */
		crx_hashcontext_dtor(C_OBJ_P(object));
		RETURN_THROWS();
	}

	object_properties_load(&hash->std, C_ARRVAL_P(members_zv));
}
/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(hash)
{
	crex_hash_init(&crx_hash_hashtable, 35, NULL, NULL, 1);

	crx_hash_register_algo("md2",			&crx_hash_md2_ops);
	crx_hash_register_algo("md4",			&crx_hash_md4_ops);
	crx_hash_register_algo("md5",			&crx_hash_md5_ops);
	crx_hash_register_algo("sha1",			&crx_hash_sha1_ops);
	crx_hash_register_algo("sha224",		&crx_hash_sha224_ops);
	crx_hash_register_algo("sha256",		&crx_hash_sha256_ops);
	crx_hash_register_algo("sha384",		&crx_hash_sha384_ops);
	crx_hash_register_algo("sha512/224",            &crx_hash_sha512_224_ops);
	crx_hash_register_algo("sha512/256",            &crx_hash_sha512_256_ops);
	crx_hash_register_algo("sha512",		&crx_hash_sha512_ops);
	crx_hash_register_algo("sha3-224",		&crx_hash_sha3_224_ops);
	crx_hash_register_algo("sha3-256",		&crx_hash_sha3_256_ops);
	crx_hash_register_algo("sha3-384",		&crx_hash_sha3_384_ops);
	crx_hash_register_algo("sha3-512",		&crx_hash_sha3_512_ops);
	crx_hash_register_algo("ripemd128",		&crx_hash_ripemd128_ops);
	crx_hash_register_algo("ripemd160",		&crx_hash_ripemd160_ops);
	crx_hash_register_algo("ripemd256",		&crx_hash_ripemd256_ops);
	crx_hash_register_algo("ripemd320",		&crx_hash_ripemd320_ops);
	crx_hash_register_algo("whirlpool",		&crx_hash_whirlpool_ops);
	crx_hash_register_algo("tiger128,3",	&crx_hash_3tiger128_ops);
	crx_hash_register_algo("tiger160,3",	&crx_hash_3tiger160_ops);
	crx_hash_register_algo("tiger192,3",	&crx_hash_3tiger192_ops);
	crx_hash_register_algo("tiger128,4",	&crx_hash_4tiger128_ops);
	crx_hash_register_algo("tiger160,4",	&crx_hash_4tiger160_ops);
	crx_hash_register_algo("tiger192,4",	&crx_hash_4tiger192_ops);
	crx_hash_register_algo("snefru",		&crx_hash_snefru_ops);
	crx_hash_register_algo("snefru256",		&crx_hash_snefru_ops);
	crx_hash_register_algo("gost",			&crx_hash_gost_ops);
	crx_hash_register_algo("gost-crypto",		&crx_hash_gost_crypto_ops);
	crx_hash_register_algo("adler32",		&crx_hash_adler32_ops);
	crx_hash_register_algo("crc32",			&crx_hash_crc32_ops);
	crx_hash_register_algo("crc32b",		&crx_hash_crc32b_ops);
	crx_hash_register_algo("crc32c",		&crx_hash_crc32c_ops);
	crx_hash_register_algo("fnv132",		&crx_hash_fnv132_ops);
	crx_hash_register_algo("fnv1a32",		&crx_hash_fnv1a32_ops);
	crx_hash_register_algo("fnv164",		&crx_hash_fnv164_ops);
	crx_hash_register_algo("fnv1a64",		&crx_hash_fnv1a64_ops);
	crx_hash_register_algo("joaat",			&crx_hash_joaat_ops);
	crx_hash_register_algo("murmur3a",		&crx_hash_murmur3a_ops);
	crx_hash_register_algo("murmur3c",		&crx_hash_murmur3c_ops);
	crx_hash_register_algo("murmur3f",		&crx_hash_murmur3f_ops);
	crx_hash_register_algo("xxh32",		&crx_hash_xxh32_ops);
	crx_hash_register_algo("xxh64",		&crx_hash_xxh64_ops);
	crx_hash_register_algo("xxh3",		&crx_hash_xxh3_64_ops);
	crx_hash_register_algo("xxh128",		&crx_hash_xxh3_128_ops);

	CRX_HASH_HAVAL_REGISTER(3,128);
	CRX_HASH_HAVAL_REGISTER(3,160);
	CRX_HASH_HAVAL_REGISTER(3,192);
	CRX_HASH_HAVAL_REGISTER(3,224);
	CRX_HASH_HAVAL_REGISTER(3,256);

	CRX_HASH_HAVAL_REGISTER(4,128);
	CRX_HASH_HAVAL_REGISTER(4,160);
	CRX_HASH_HAVAL_REGISTER(4,192);
	CRX_HASH_HAVAL_REGISTER(4,224);
	CRX_HASH_HAVAL_REGISTER(4,256);

	CRX_HASH_HAVAL_REGISTER(5,128);
	CRX_HASH_HAVAL_REGISTER(5,160);
	CRX_HASH_HAVAL_REGISTER(5,192);
	CRX_HASH_HAVAL_REGISTER(5,224);
	CRX_HASH_HAVAL_REGISTER(5,256);

	register_hash_symbols(module_number);

	crx_hashcontext_ce = register_class_HashContext();
	crx_hashcontext_ce->create_object = crx_hashcontext_create;

	memcpy(&crx_hashcontext_handlers, &std_object_handlers,
	       sizeof(crex_object_handlers));
	crx_hashcontext_handlers.offset = XtOffsetOf(crx_hashcontext_object, std);
	crx_hashcontext_handlers.free_obj = crx_hashcontext_free;
	crx_hashcontext_handlers.clone_obj = crx_hashcontext_clone;

#ifdef CRX_MHASH_BC
	mhash_init(INIT_FUNC_ARGS_PASSTHRU);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(hash)
{
	crex_hash_destroy(&crx_hash_hashtable);

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(hash)
{
	char buffer[2048];
	crex_string *str;
	char *s = buffer, *e = s + sizeof(buffer);

	CREX_HASH_MAP_FOREACH_STR_KEY(&crx_hash_hashtable, str) {
		s += slprintf(s, e - s, "%s ", ZSTR_VAL(str));
	} CREX_HASH_FOREACH_END();
	*s = 0;

	crx_info_print_table_start();
	crx_info_print_table_row(2, "hash support", "enabled");
	crx_info_print_table_row(2, "Hashing Engines", buffer);
	crx_info_print_table_end();

#ifdef CRX_MHASH_BC
	crx_info_print_table_start();
	crx_info_print_table_row(2, "MHASH support", "Enabled");
	crx_info_print_table_row(2, "MHASH API Version", "Emulated Support");
	crx_info_print_table_end();
#endif

}
/* }}} */

/* {{{ hash_module_entry */
crex_module_entry hash_module_entry = {
	STANDARD_MODULE_HEADER,
	CRX_HASH_EXTNAME,
	ext_functions,
	CRX_MINIT(hash),
	CRX_MSHUTDOWN(hash),
	NULL, /* RINIT */
	NULL, /* RSHUTDOWN */
	CRX_MINFO(hash),
	CRX_HASH_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */
