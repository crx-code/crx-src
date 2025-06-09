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
  +----------------------------------------------------------------------+
*/

#ifndef CRX_HASH_H
#define CRX_HASH_H

#include "crx.h"

#define CRX_HASH_EXTNAME	"hash"
#define CRX_HASH_VERSION	CRX_VERSION
#define CRX_MHASH_VERSION	CRX_VERSION

#define CRX_HASH_HMAC		0x0001

#define CRX_HASH_SERIALIZE_MAGIC_SPEC          2

#define L64 INT64_C

typedef struct _crx_hashcontext_object crx_hashcontext_object;

typedef void (*crx_hash_init_func_t)(void *context, HashTable *args);
typedef void (*crx_hash_update_func_t)(void *context, const unsigned char *buf, size_t count);
typedef void (*crx_hash_final_func_t)(unsigned char *digest, void *context);
typedef int  (*crx_hash_copy_func_t)(const void *ops, void *orig_context, void *dest_context);
typedef int  (*crx_hash_serialize_func_t)(const crx_hashcontext_object *hash, crex_long *magic, zval *zv);
typedef int  (*crx_hash_unserialize_func_t)(crx_hashcontext_object *hash, crex_long magic, const zval *zv);

typedef struct _crx_hash_ops {
	const char *algo;
	crx_hash_init_func_t hash_init;
	crx_hash_update_func_t hash_update;
	crx_hash_final_func_t hash_final;
	crx_hash_copy_func_t hash_copy;
	crx_hash_serialize_func_t hash_serialize;
	crx_hash_unserialize_func_t hash_unserialize;
	const char *serialize_spec;

	size_t digest_size;
	size_t block_size;
	size_t context_size;
	unsigned is_crypto: 1;
} crx_hash_ops;

struct _crx_hashcontext_object {
	const crx_hash_ops *ops;
	void *context;

	crex_long options;
	unsigned char *key;

	crex_object std;
};

static inline crx_hashcontext_object *crx_hashcontext_from_object(crex_object *obj) {
	return ((crx_hashcontext_object*)(obj + 1)) - 1;
}

extern const crx_hash_ops crx_hash_md2_ops;
extern const crx_hash_ops crx_hash_md4_ops;
extern const crx_hash_ops crx_hash_md5_ops;
extern const crx_hash_ops crx_hash_sha1_ops;
extern const crx_hash_ops crx_hash_sha224_ops;
extern const crx_hash_ops crx_hash_sha256_ops;
extern const crx_hash_ops crx_hash_sha384_ops;
extern const crx_hash_ops crx_hash_sha512_ops;
extern const crx_hash_ops crx_hash_sha512_256_ops;
extern const crx_hash_ops crx_hash_sha512_224_ops;
extern const crx_hash_ops crx_hash_sha3_224_ops;
extern const crx_hash_ops crx_hash_sha3_256_ops;
extern const crx_hash_ops crx_hash_sha3_384_ops;
extern const crx_hash_ops crx_hash_sha3_512_ops;
extern const crx_hash_ops crx_hash_ripemd128_ops;
extern const crx_hash_ops crx_hash_ripemd160_ops;
extern const crx_hash_ops crx_hash_ripemd256_ops;
extern const crx_hash_ops crx_hash_ripemd320_ops;
extern const crx_hash_ops crx_hash_whirlpool_ops;
extern const crx_hash_ops crx_hash_3tiger128_ops;
extern const crx_hash_ops crx_hash_3tiger160_ops;
extern const crx_hash_ops crx_hash_3tiger192_ops;
extern const crx_hash_ops crx_hash_4tiger128_ops;
extern const crx_hash_ops crx_hash_4tiger160_ops;
extern const crx_hash_ops crx_hash_4tiger192_ops;
extern const crx_hash_ops crx_hash_snefru_ops;
extern const crx_hash_ops crx_hash_gost_ops;
extern const crx_hash_ops crx_hash_gost_crypto_ops;
extern const crx_hash_ops crx_hash_adler32_ops;
extern const crx_hash_ops crx_hash_crc32_ops;
extern const crx_hash_ops crx_hash_crc32b_ops;
extern const crx_hash_ops crx_hash_crc32c_ops;
extern const crx_hash_ops crx_hash_fnv132_ops;
extern const crx_hash_ops crx_hash_fnv1a32_ops;
extern const crx_hash_ops crx_hash_fnv164_ops;
extern const crx_hash_ops crx_hash_fnv1a64_ops;
extern const crx_hash_ops crx_hash_joaat_ops;
extern const crx_hash_ops crx_hash_murmur3a_ops;
extern const crx_hash_ops crx_hash_murmur3c_ops;
extern const crx_hash_ops crx_hash_murmur3f_ops;
extern const crx_hash_ops crx_hash_xxh32_ops;
extern const crx_hash_ops crx_hash_xxh64_ops;
extern const crx_hash_ops crx_hash_xxh3_64_ops;
extern const crx_hash_ops crx_hash_xxh3_128_ops;

#define CRX_HASH_HAVAL_OPS(p,b)	extern const crx_hash_ops crx_hash_##p##haval##b##_ops;

CRX_HASH_HAVAL_OPS(3,128)
CRX_HASH_HAVAL_OPS(3,160)
CRX_HASH_HAVAL_OPS(3,192)
CRX_HASH_HAVAL_OPS(3,224)
CRX_HASH_HAVAL_OPS(3,256)

CRX_HASH_HAVAL_OPS(4,128)
CRX_HASH_HAVAL_OPS(4,160)
CRX_HASH_HAVAL_OPS(4,192)
CRX_HASH_HAVAL_OPS(4,224)
CRX_HASH_HAVAL_OPS(4,256)

CRX_HASH_HAVAL_OPS(5,128)
CRX_HASH_HAVAL_OPS(5,160)
CRX_HASH_HAVAL_OPS(5,192)
CRX_HASH_HAVAL_OPS(5,224)
CRX_HASH_HAVAL_OPS(5,256)

extern crex_module_entry hash_module_entry;
#define crxext_hash_ptr &hash_module_entry

#ifdef CRX_WIN32
#	define CRX_HASH_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define CRX_HASH_API __attribute__ ((visibility("default")))
#else
#	define CRX_HASH_API
#endif

extern CRX_HASH_API crex_class_entry *crx_hashcontext_ce;
CRX_HASH_API const crx_hash_ops *crx_hash_fetch_ops(crex_string *algo);
CRX_HASH_API void crx_hash_register_algo(const char *algo, const crx_hash_ops *ops);
CRX_HASH_API int crx_hash_copy(const void *ops, void *orig_context, void *dest_context);
CRX_HASH_API int crx_hash_serialize(const crx_hashcontext_object *context, crex_long *magic, zval *zv);
CRX_HASH_API int crx_hash_unserialize(crx_hashcontext_object *context, crex_long magic, const zval *zv);
CRX_HASH_API int crx_hash_serialize_spec(const crx_hashcontext_object *context, zval *zv, const char *spec);
CRX_HASH_API int crx_hash_unserialize_spec(crx_hashcontext_object *hash, const zval *zv, const char *spec);

static inline void *crx_hash_alloc_context(const crx_hash_ops *ops) {
	/* Zero out context memory so serialization doesn't expose internals */
	return ecalloc(1, ops->context_size);
}

static inline void crx_hash_bin2hex(char *out, const unsigned char *in, size_t in_len)
{
	static const char hexits[17] = "0123456789abcdef";
	size_t i;

	for(i = 0; i < in_len; i++) {
		out[i * 2]       = hexits[in[i] >> 4];
		out[(i * 2) + 1] = hexits[in[i] &  0x0F];
	}
}

#endif	/* CRX_HASH_H */
