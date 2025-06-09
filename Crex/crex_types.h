/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   |          Xinchen Hui <laruence@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_TYPES_H
#define CREX_TYPES_H

#include "crex_portability.h"
#include "crex_long.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __SSE2__
# include <mmintrin.h>
# include <emmintrin.h>
#endif
#if defined(__AVX2__)
# include <immintrin.h>
#endif
#if defined(__aarch64__) || defined(_M_ARM64)
# include <arm_neon.h>
#endif

#ifdef WORDS_BIGENDIAN
# define CREX_ENDIAN_LOHI(lo, hi)          hi; lo;
# define CREX_ENDIAN_LOHI_3(lo, mi, hi)    hi; mi; lo;
# define CREX_ENDIAN_LOHI_4(a, b, c, d)    d; c; b; a;
# define CREX_ENDIAN_LOHI_C(lo, hi)        hi, lo
# define CREX_ENDIAN_LOHI_C_3(lo, mi, hi)  hi, mi, lo,
# define CREX_ENDIAN_LOHI_C_4(a, b, c, d)  d, c, b, a
#else
# define CREX_ENDIAN_LOHI(lo, hi)          lo; hi;
# define CREX_ENDIAN_LOHI_3(lo, mi, hi)    lo; mi; hi;
# define CREX_ENDIAN_LOHI_4(a, b, c, d)    a; b; c; d;
# define CREX_ENDIAN_LOHI_C(lo, hi)        lo, hi
# define CREX_ENDIAN_LOHI_C_3(lo, mi, hi)  lo, mi, hi,
# define CREX_ENDIAN_LOHI_C_4(a, b, c, d)  a, b, c, d
#endif

typedef unsigned char crex_uchar;

typedef enum {
  SUCCESS =  0,
  FAILURE = -1,		/* this MUST stay a negative number, or it may affect functions! */
} CREX_RESULT_CODE;

typedef CREX_RESULT_CODE crex_result;

#ifdef CREX_ENABLE_ZVAL_LONG64
# ifdef CREX_WIN32
#  define CREX_SIZE_MAX  _UI64_MAX
# else
#  define CREX_SIZE_MAX  SIZE_MAX
# endif
#else
# if defined(CREX_WIN32)
#  define CREX_SIZE_MAX  _UI32_MAX
# else
#  define CREX_SIZE_MAX SIZE_MAX
# endif
#endif

#ifdef ZTS
#define CREX_TLS static TSRM_TLS
#define CREX_EXT_TLS TSRM_TLS
#else
#define CREX_TLS static
#define CREX_EXT_TLS
#endif

typedef struct _crex_object_handlers crex_object_handlers;
typedef struct _crex_class_entry     crex_class_entry;
typedef union  _crex_function        crex_function;
typedef struct _crex_execute_data    crex_execute_data;

typedef struct _zval_struct     zval;

typedef struct _crex_refcounted crex_refcounted;
typedef struct _crex_string     crex_string;
typedef struct _crex_array      crex_array;
typedef struct _crex_object     crex_object;
typedef struct _crex_resource   crex_resource;
typedef struct _crex_reference  crex_reference;
typedef struct _crex_ast_ref    crex_ast_ref;
typedef struct _crex_ast        crex_ast;

typedef int  (*compare_func_t)(const void *, const void *);
typedef void (*swap_func_t)(void *, void *);
typedef void (*sort_func_t)(void *, size_t, size_t, compare_func_t, swap_func_t);
typedef void (*dtor_func_t)(zval *pDest);
typedef void (*copy_ctor_func_t)(zval *pElement);

/*
 * crex_type - is an abstraction layer to represent information about type hint.
 * It shouldn't be used directly. Only through CREX_TYPE_* macros.
 *
 * CREX_TYPE_IS_SET()        - checks if there is a type-hint
 * CREX_TYPE_IS_ONLY_MASK()  - checks if type-hint refer to standard type only
 * CREX_TYPE_IS_COMPLEX()    - checks if type is a type_list, or contains a class either as a CE or as a name
 * CREX_TYPE_HAS_NAME()      - checks if type-hint contains some class as crex_string *
 * CREX_TYPE_HAS_LITERAL_NAME()	- checks if type-hint contains some class as const char *
 * CREX_TYPE_IS_INTERSECTION() - checks if the type_list represents an intersection type list
 * CREX_TYPE_IS_UNION()      - checks if the type_list represents a union type list
 *
 * CREX_TYPE_NAME()       - returns referenced class name
 * CREX_TYPE_PURE_MASK()  - returns MAY_BE_* type mask
 * CREX_TYPE_FULL_MASK()  - returns MAY_BE_* type mask together with other flags
 *
 * CREX_TYPE_ALLOW_NULL() - checks if NULL is allowed
 *
 * CREX_TYPE_INIT_*() should be used for construction.
 */

typedef struct {
	/* Not using a union here, because there's no good way to initialize them
	 * in a way that is supported in both C and C++ (designated initializers
	 * are only supported since C++20). */
	void *ptr;
	uint32_t type_mask;
	/* TODO: We could use the extra 32-bit of padding on 64-bit systems. */
} crex_type;

typedef struct {
	uint32_t num_types;
	crex_type types[1];
} crex_type_list;

#define _CREX_TYPE_EXTRA_FLAGS_SHIFT 25
#define _CREX_TYPE_MASK ((1u << 25) - 1)
/* Only one of these bits may be set. */
#define _CREX_TYPE_NAME_BIT (1u << 24)
// Used to signify that type.ptr is not a `crex_string*` but a `const char*`,
#define _CREX_TYPE_LITERAL_NAME_BIT (1u << 23)
#define _CREX_TYPE_LIST_BIT (1u << 22)
#define _CREX_TYPE_KIND_MASK (_CREX_TYPE_LIST_BIT|_CREX_TYPE_NAME_BIT|_CREX_TYPE_LITERAL_NAME_BIT)
/* For BC behaviour with iterable type */
#define _CREX_TYPE_ITERABLE_BIT (1u << 21)
/* Whether the type list is arena allocated */
#define _CREX_TYPE_ARENA_BIT (1u << 20)
/* Whether the type list is an intersection type */
#define _CREX_TYPE_INTERSECTION_BIT (1u << 19)
/* Whether the type is a union type */
#define _CREX_TYPE_UNION_BIT (1u << 18)
/* Type mask excluding the flags above. */
#define _CREX_TYPE_MAY_BE_MASK ((1u << 18) - 1)
/* Must have same value as MAY_BE_NULL */
#define _CREX_TYPE_NULLABLE_BIT 0x2u

#define CREX_TYPE_IS_SET(t) \
	(((t).type_mask & _CREX_TYPE_MASK) != 0)

/* If a type is complex it means it's either a list with a union or intersection,
 * or the void pointer is a class name */
#define CREX_TYPE_IS_COMPLEX(t) \
	((((t).type_mask) & _CREX_TYPE_KIND_MASK) != 0)

#define CREX_TYPE_HAS_NAME(t) \
	((((t).type_mask) & _CREX_TYPE_NAME_BIT) != 0)

#define CREX_TYPE_HAS_LITERAL_NAME(t) \
	((((t).type_mask) & _CREX_TYPE_LITERAL_NAME_BIT) != 0)

#define CREX_TYPE_HAS_LIST(t) \
	((((t).type_mask) & _CREX_TYPE_LIST_BIT) != 0)

#define CREX_TYPE_IS_ITERABLE_FALLBACK(t) \
	((((t).type_mask) & _CREX_TYPE_ITERABLE_BIT) != 0)

#define CREX_TYPE_IS_INTERSECTION(t) \
	((((t).type_mask) & _CREX_TYPE_INTERSECTION_BIT) != 0)

#define CREX_TYPE_IS_UNION(t) \
	((((t).type_mask) & _CREX_TYPE_UNION_BIT) != 0)

#define CREX_TYPE_USES_ARENA(t) \
	((((t).type_mask) & _CREX_TYPE_ARENA_BIT) != 0)

#define CREX_TYPE_IS_ONLY_MASK(t) \
	(CREX_TYPE_IS_SET(t) && (t).ptr == NULL)

#define CREX_TYPE_NAME(t) \
	((crex_string *) (t).ptr)

#define CREX_TYPE_LITERAL_NAME(t) \
	((const char *) (t).ptr)

#define CREX_TYPE_LIST(t) \
	((crex_type_list *) (t).ptr)

#define CREX_TYPE_LIST_SIZE(num_types) \
	(sizeof(crex_type_list) + ((num_types) - 1) * sizeof(crex_type))

/* This iterates over a crex_type_list. */
#define CREX_TYPE_LIST_FOREACH(list, type_ptr) do { \
	crex_type *_list = (list)->types; \
	crex_type *_end = _list + (list)->num_types; \
	for (; _list < _end; _list++) { \
		type_ptr = _list;

#define CREX_TYPE_LIST_FOREACH_END() \
	} \
} while (0)

/* This iterates over any crex_type. If it's a type list, all list elements will
 * be visited. If it's a single type, only the single type is visited. */
#define CREX_TYPE_FOREACH(type, type_ptr) do { \
	crex_type *_cur, *_end; \
	if (CREX_TYPE_HAS_LIST(type)) { \
		crex_type_list *_list = CREX_TYPE_LIST(type); \
		_cur = _list->types; \
		_end = _cur + _list->num_types; \
	} else { \
		_cur = &(type); \
		_end = _cur + 1; \
	} \
	do { \
		type_ptr = _cur;

#define CREX_TYPE_FOREACH_END() \
	} while (++_cur < _end); \
} while (0)

#define CREX_TYPE_SET_PTR(t, _ptr) \
	((t).ptr = (_ptr))

#define CREX_TYPE_SET_PTR_AND_KIND(t, _ptr, kind_bit) do { \
	(t).ptr = (_ptr); \
	(t).type_mask &= ~_CREX_TYPE_KIND_MASK; \
	(t).type_mask |= (kind_bit); \
} while (0)

#define CREX_TYPE_SET_LIST(t, list) \
	CREX_TYPE_SET_PTR_AND_KIND(t, list, _CREX_TYPE_LIST_BIT)

/* FULL_MASK() includes the MAY_BE_* type mask, as well as additional metadata bits.
 * The PURE_MASK() only includes the MAY_BE_* type mask. */
#define CREX_TYPE_FULL_MASK(t) \
	((t).type_mask)

#define CREX_TYPE_PURE_MASK(t) \
	((t).type_mask & _CREX_TYPE_MAY_BE_MASK)

#define CREX_TYPE_FULL_MASK_WITHOUT_NULL(t) \
	((t).type_mask & ~_CREX_TYPE_NULLABLE_BIT)

#define CREX_TYPE_PURE_MASK_WITHOUT_NULL(t) \
	((t).type_mask & _CREX_TYPE_MAY_BE_MASK & ~_CREX_TYPE_NULLABLE_BIT)

#define CREX_TYPE_CONTAINS_CODE(t, code) \
	(((t).type_mask & (1u << (code))) != 0)

#define CREX_TYPE_ALLOW_NULL(t) \
	(((t).type_mask & _CREX_TYPE_NULLABLE_BIT) != 0)

#if defined(__cplusplus) && defined(_MSC_VER)
# define _CREX_TYPE_PREFIX crex_type
#else
/* FIXME: We could add (crex_type) here at some point but this breaks in MSVC because
 * (crex_type)(crex_type){} is no longer considered constant. */
# define _CREX_TYPE_PREFIX
#endif

#define CREX_TYPE_INIT_NONE(extra_flags) \
	_CREX_TYPE_PREFIX { NULL, (extra_flags) }

#define CREX_TYPE_INIT_MASK(_type_mask) \
	_CREX_TYPE_PREFIX { NULL, (_type_mask) }

#define CREX_TYPE_INIT_CODE(code, allow_null, extra_flags) \
	CREX_TYPE_INIT_MASK(((code) == _IS_BOOL ? MAY_BE_BOOL : ( (code) == IS_ITERABLE ? _CREX_TYPE_ITERABLE_BIT : ((code) == IS_MIXED ? MAY_BE_ANY : (1 << (code))))) \
		| ((allow_null) ? _CREX_TYPE_NULLABLE_BIT : 0) | (extra_flags))

#define CREX_TYPE_INIT_PTR(ptr, type_kind, allow_null, extra_flags) \
	_CREX_TYPE_PREFIX { (void *) (ptr), \
		(type_kind) | ((allow_null) ? _CREX_TYPE_NULLABLE_BIT : 0) | (extra_flags) }

#define CREX_TYPE_INIT_PTR_MASK(ptr, type_mask) \
	_CREX_TYPE_PREFIX { (void *) (ptr), (type_mask) }

#define CREX_TYPE_INIT_UNION(ptr, extra_flags) \
	_CREX_TYPE_PREFIX { (void *) (ptr), (_CREX_TYPE_LIST_BIT|_CREX_TYPE_UNION_BIT) | (extra_flags) }

#define CREX_TYPE_INIT_INTERSECTION(ptr, extra_flags) \
	_CREX_TYPE_PREFIX { (void *) (ptr), (_CREX_TYPE_LIST_BIT|_CREX_TYPE_INTERSECTION_BIT) | (extra_flags) }

#define CREX_TYPE_INIT_CLASS(class_name, allow_null, extra_flags) \
	CREX_TYPE_INIT_PTR(class_name, _CREX_TYPE_NAME_BIT, allow_null, extra_flags)

#define CREX_TYPE_INIT_CLASS_MASK(class_name, type_mask) \
	CREX_TYPE_INIT_PTR_MASK(class_name, _CREX_TYPE_NAME_BIT | (type_mask))

#define CREX_TYPE_INIT_CLASS_CONST(class_name, allow_null, extra_flags) \
	CREX_TYPE_INIT_PTR(class_name, _CREX_TYPE_LITERAL_NAME_BIT, allow_null, extra_flags)

#define CREX_TYPE_INIT_CLASS_CONST_MASK(class_name, type_mask) \
	CREX_TYPE_INIT_PTR_MASK(class_name, (_CREX_TYPE_LITERAL_NAME_BIT | (type_mask)))

typedef union _crex_value {
	crex_long         lval;				/* long value */
	double            dval;				/* double value */
	crex_refcounted  *counted;
	crex_string      *str;
	crex_array       *arr;
	crex_object      *obj;
	crex_resource    *res;
	crex_reference   *ref;
	crex_ast_ref     *ast;
	zval             *zv;
	void             *ptr;
	crex_class_entry *ce;
	crex_function    *func;
	struct {
		uint32_t w1;
		uint32_t w2;
	} ww;
} crex_value;

struct _zval_struct {
	crex_value        value;			/* value */
	union {
		uint32_t type_info;
		struct {
			CREX_ENDIAN_LOHI_3(
				uint8_t    type,			/* active type */
				uint8_t    type_flags,
				union {
					uint16_t  extra;        /* not further specified */
				} u)
		} v;
	} u1;
	union {
		uint32_t     next;                 /* hash collision chain */
		uint32_t     cache_slot;           /* cache slot (for RECV_INIT) */
		uint32_t     opline_num;           /* opline number (for FAST_CALL) */
		uint32_t     lineno;               /* line number (for ast nodes) */
		uint32_t     num_args;             /* arguments number for EX(This) */
		uint32_t     fe_pos;               /* foreach position */
		uint32_t     fe_iter_idx;          /* foreach iterator index */
		uint32_t     guard;                /* recursion and single property guard */
		uint32_t     constant_flags;       /* constant flags */
		uint32_t     extra;                /* not further specified */
	} u2;
};

typedef struct _crex_refcounted_h {
	uint32_t         refcount;			/* reference counter 32-bit */
	union {
		uint32_t type_info;
	} u;
} crex_refcounted_h;

struct _crex_refcounted {
	crex_refcounted_h gc;
};

struct _crex_string {
	crex_refcounted_h gc;
	crex_ulong        h;                /* hash value */
	size_t            len;
	char              val[1];
};

typedef struct _Bucket {
	zval              val;
	crex_ulong        h;                /* hash value (or numeric index)   */
	crex_string      *key;              /* string key or NULL for numerics */
} Bucket;

typedef struct _crex_array HashTable;

struct _crex_array {
	crex_refcounted_h gc;
	union {
		struct {
			CREX_ENDIAN_LOHI_4(
				uint8_t    flags,
				uint8_t    _unused,
				uint8_t    nIteratorsCount,
				uint8_t    _unused2)
		} v;
		uint32_t flags;
	} u;
	uint32_t          nTableMask;
	union {
		uint32_t     *arHash;   /* hash table (allocated above this pointer) */
		Bucket       *arData;   /* array of hash buckets */
		zval         *arPacked; /* packed array of zvals */
	};
	uint32_t          nNumUsed;
	uint32_t          nNumOfElements;
	uint32_t          nTableSize;
	uint32_t          nInternalPointer;
	crex_long         nNextFreeElement;
	dtor_func_t       pDestructor;
};

/*
 * HashTable Data Layout
 * =====================
 *
 *                 +=============================+
 *                 | HT_HASH(ht, ht->nTableMask) |                   +=============================+
 *                 | ...                         |                   | HT_INVALID_IDX              |
 *                 | HT_HASH(ht, -1)             |                   | HT_INVALID_IDX              |
 *                 +-----------------------------+                   +-----------------------------+
 * ht->arData ---> | Bucket[0]                   | ht->arPacked ---> | ZVAL[0]                     |
 *                 | ...                         |                   | ...                         |
 *                 | Bucket[ht->nTableSize-1]    |                   | ZVAL[ht->nTableSize-1]      |
 *                 +=============================+                   +=============================+
 */

#define HT_INVALID_IDX ((uint32_t) -1)

#define HT_MIN_MASK ((uint32_t) -2)
#define HT_MIN_SIZE 8

/* HT_MAX_SIZE is chosen to satisfy the following constraints:
 * - HT_SIZE_TO_MASK(HT_MAX_SIZE) != 0
 * - HT_SIZE_EX(HT_MAX_SIZE, HT_SIZE_TO_MASK(HT_MAX_SIZE)) does not overflow or
 *   wrapparound, and is <= the addressable space size
 * - HT_MAX_SIZE must be a power of two:
 *   (nTableSize<HT_MAX_SIZE ? nTableSize+nTableSize : nTableSize) <= HT_MAX_SIZE
 */
#if SIZEOF_SIZE_T == 4
# define HT_MAX_SIZE 0x02000000
# define HT_HASH_TO_BUCKET_EX(data, idx) \
	((Bucket*)((char*)(data) + (idx)))
# define HT_IDX_TO_HASH(idx) \
	((idx) * sizeof(Bucket))
# define HT_HASH_TO_IDX(idx) \
	((idx) / sizeof(Bucket))
#elif SIZEOF_SIZE_T == 8
# define HT_MAX_SIZE 0x40000000
# define HT_HASH_TO_BUCKET_EX(data, idx) \
	((data) + (idx))
# define HT_IDX_TO_HASH(idx) \
	(idx)
# define HT_HASH_TO_IDX(idx) \
	(idx)
#else
# error "Unknown SIZEOF_SIZE_T"
#endif

#define HT_HASH_EX(data, idx) \
	((uint32_t*)(data))[(int32_t)(idx)]
#define HT_HASH(ht, idx) \
	HT_HASH_EX((ht)->arHash, idx)

#define HT_SIZE_TO_MASK(nTableSize) \
	((uint32_t)(-((nTableSize) + (nTableSize))))
#define HT_HASH_SIZE(nTableMask) \
	(((size_t)-(uint32_t)(nTableMask)) * sizeof(uint32_t))
#define HT_DATA_SIZE(nTableSize) \
	((size_t)(nTableSize) * sizeof(Bucket))
#define HT_SIZE_EX(nTableSize, nTableMask) \
	(HT_DATA_SIZE((nTableSize)) + HT_HASH_SIZE((nTableMask)))
#define HT_SIZE(ht) \
	HT_SIZE_EX((ht)->nTableSize, (ht)->nTableMask)
#define HT_USED_SIZE(ht) \
	(HT_HASH_SIZE((ht)->nTableMask) + ((size_t)(ht)->nNumUsed * sizeof(Bucket)))
#define HT_PACKED_DATA_SIZE(nTableSize) \
	((size_t)(nTableSize) * sizeof(zval))
#define HT_PACKED_SIZE_EX(nTableSize, nTableMask) \
	(HT_PACKED_DATA_SIZE((nTableSize)) + HT_HASH_SIZE((nTableMask)))
#define HT_PACKED_SIZE(ht) \
	HT_PACKED_SIZE_EX((ht)->nTableSize, (ht)->nTableMask)
#define HT_PACKED_USED_SIZE(ht) \
	(HT_HASH_SIZE((ht)->nTableMask) + ((size_t)(ht)->nNumUsed * sizeof(zval)))
#if defined(__AVX2__)
# define HT_HASH_RESET(ht) do { \
		char *p = (char*)&HT_HASH(ht, (ht)->nTableMask); \
		size_t size = HT_HASH_SIZE((ht)->nTableMask); \
		__m256i ymm0 = _mm256_setzero_si256(); \
		ymm0 = _mm256_cmpeq_epi64(ymm0, ymm0); \
		CREX_ASSERT(size >= 64 && ((size & 0x3f) == 0)); \
		do { \
			_mm256_storeu_si256((__m256i*)p, ymm0); \
			_mm256_storeu_si256((__m256i*)(p+32), ymm0); \
			p += 64; \
			size -= 64; \
		} while (size != 0); \
	} while (0)
#elif defined(__SSE2__)
# define HT_HASH_RESET(ht) do { \
		char *p = (char*)&HT_HASH(ht, (ht)->nTableMask); \
		size_t size = HT_HASH_SIZE((ht)->nTableMask); \
		__m128i xmm0 = _mm_setzero_si128(); \
		xmm0 = _mm_cmpeq_epi8(xmm0, xmm0); \
		CREX_ASSERT(size >= 64 && ((size & 0x3f) == 0)); \
		do { \
			_mm_storeu_si128((__m128i*)p, xmm0); \
			_mm_storeu_si128((__m128i*)(p+16), xmm0); \
			_mm_storeu_si128((__m128i*)(p+32), xmm0); \
			_mm_storeu_si128((__m128i*)(p+48), xmm0); \
			p += 64; \
			size -= 64; \
		} while (size != 0); \
	} while (0)
#elif defined(__aarch64__) || defined(_M_ARM64)
# define HT_HASH_RESET(ht) do { \
		char *p = (char*)&HT_HASH(ht, (ht)->nTableMask); \
		size_t size = HT_HASH_SIZE((ht)->nTableMask); \
		int32x4_t t = vdupq_n_s32(-1); \
		CREX_ASSERT(size >= 64 && ((size & 0x3f) == 0)); \
		do { \
			vst1q_s32((int32_t*)p, t); \
			vst1q_s32((int32_t*)(p+16), t); \
			vst1q_s32((int32_t*)(p+32), t); \
			vst1q_s32((int32_t*)(p+48), t); \
			p += 64; \
			size -= 64; \
		} while (size != 0); \
	} while (0)
#else
# define HT_HASH_RESET(ht) \
	memset(&HT_HASH(ht, (ht)->nTableMask), HT_INVALID_IDX, HT_HASH_SIZE((ht)->nTableMask))
#endif
#define HT_HASH_RESET_PACKED(ht) do { \
		HT_HASH(ht, -2) = HT_INVALID_IDX; \
		HT_HASH(ht, -1) = HT_INVALID_IDX; \
	} while (0)
#define HT_HASH_TO_BUCKET(ht, idx) \
	HT_HASH_TO_BUCKET_EX((ht)->arData, idx)

#define HT_SET_DATA_ADDR(ht, ptr) do { \
		(ht)->arData = (Bucket*)(((char*)(ptr)) + HT_HASH_SIZE((ht)->nTableMask)); \
	} while (0)
#define HT_GET_DATA_ADDR(ht) \
	((char*)((ht)->arData) - HT_HASH_SIZE((ht)->nTableMask))

typedef uint32_t HashPosition;

typedef struct _HashTableIterator {
	HashTable    *ht;
	HashPosition  pos;
	uint32_t      next_copy; // circular linked list via index into EG(ht_iterators)
} HashTableIterator;

struct _crex_object {
	crex_refcounted_h gc;
	uint32_t          handle; // TODO: may be removed ???
	crex_class_entry *ce;
	const crex_object_handlers *handlers;
	HashTable        *properties;
	zval              properties_table[1];
};

struct _crex_resource {
	crex_refcounted_h gc;
	crex_long         handle; // TODO: may be removed ???
	int               type;
	void             *ptr;
};

typedef struct {
	size_t num;
	size_t num_allocated;
	struct _crex_property_info *ptr[1];
} crex_property_info_list;

typedef union {
	struct _crex_property_info *ptr;
	uintptr_t list;
} crex_property_info_source_list;

#define CREX_PROPERTY_INFO_SOURCE_FROM_LIST(list) (0x1 | (uintptr_t) (list))
#define CREX_PROPERTY_INFO_SOURCE_TO_LIST(list) ((crex_property_info_list *) ((list) & ~0x1))
#define CREX_PROPERTY_INFO_SOURCE_IS_LIST(list) ((list) & 0x1)

struct _crex_reference {
	crex_refcounted_h              gc;
	zval                           val;
	crex_property_info_source_list sources;
};

struct _crex_ast_ref {
	crex_refcounted_h gc;
	/*crex_ast        ast; crex_ast follows the crex_ast_ref structure */
};

/* Regular data types: Must be in sync with crex_variables.c. */
#define IS_UNDEF					0
#define IS_NULL						1
#define IS_FALSE					2
#define IS_TRUE						3
#define IS_LONG						4
#define IS_DOUBLE					5
#define IS_STRING					6
#define IS_ARRAY					7
#define IS_OBJECT					8
#define IS_RESOURCE					9
#define IS_REFERENCE				10
#define IS_CONSTANT_AST				11 /* Constant expressions */

/* Fake types used only for type hinting.
 * These are allowed to overlap with the types below. */
#define IS_CALLABLE					12
#define IS_ITERABLE					13
#define IS_VOID						14
#define IS_STATIC					15
#define IS_MIXED					16
#define IS_NEVER					17

/* internal types */
#define IS_INDIRECT             	12
#define IS_PTR						13
#define IS_ALIAS_PTR				14
#define _IS_ERROR					15

/* used for casts */
#define _IS_BOOL					18
#define _IS_NUMBER					19

/* guard flags */
#define CREX_GUARD_PROPERTY_GET		(1<<0)
#define CREX_GUARD_PROPERTY_SET		(1<<1)
#define CREX_GUARD_PROPERTY_UNSET	(1<<2)
#define CREX_GUARD_PROPERTY_ISSET	(1<<3)
#define CREX_GUARD_PROPERTY_MASK	15
#define CREX_GUARD_RECURSION_DEBUG	(1<<4)
#define CREX_GUARD_RECURSION_EXPORT	(1<<5)
#define CREX_GUARD_RECURSION_JSON	(1<<6)

#define CREX_GUARD_RECURSION_TYPE(t) CREX_GUARD_RECURSION_ ## t

#define CREX_GUARD_IS_RECURSIVE(pg, t)			((*pg & CREX_GUARD_RECURSION_TYPE(t)) != 0)
#define CREX_GUARD_PROTECT_RECURSION(pg, t)		*pg |= CREX_GUARD_RECURSION_TYPE(t)
#define CREX_GUARD_UNPROTECT_RECURSION(pg, t)	*pg &= ~CREX_GUARD_RECURSION_TYPE(t)

static crex_always_inline uint8_t zval_get_type(const zval* pz) {
	return pz->u1.v.type;
}

#define CREX_SAME_FAKE_TYPE(faketype, realtype) ( \
	(faketype) == (realtype) \
	|| ((faketype) == _IS_BOOL && ((realtype) == IS_TRUE || (realtype) == IS_FALSE)) \
)

/* we should never set just C_TYPE, we should set C_TYPE_INFO */
#define C_TYPE(zval)				zval_get_type(&(zval))
#define C_TYPE_P(zval_p)			C_TYPE(*(zval_p))

#define C_TYPE_FLAGS(zval)			(zval).u1.v.type_flags
#define C_TYPE_FLAGS_P(zval_p)		C_TYPE_FLAGS(*(zval_p))

#define C_TYPE_EXTRA(zval)			(zval).u1.v.u.extra
#define C_TYPE_EXTRA_P(zval_p)		C_TYPE_EXTRA(*(zval_p))

#define C_TYPE_INFO(zval)			(zval).u1.type_info
#define C_TYPE_INFO_P(zval_p)		C_TYPE_INFO(*(zval_p))

#define C_NEXT(zval)				(zval).u2.next
#define C_NEXT_P(zval_p)			C_NEXT(*(zval_p))

#define C_CACHE_SLOT(zval)			(zval).u2.cache_slot
#define C_CACHE_SLOT_P(zval_p)		C_CACHE_SLOT(*(zval_p))

#define C_LINENO(zval)				(zval).u2.lineno
#define C_LINENO_P(zval_p)			C_LINENO(*(zval_p))

#define C_OPLINE_NUM(zval)			(zval).u2.opline_num
#define C_OPLINE_NUM_P(zval_p)		C_OPLINE_NUM(*(zval_p))

#define C_FE_POS(zval)				(zval).u2.fe_pos
#define C_FE_POS_P(zval_p)			C_FE_POS(*(zval_p))

#define C_FE_ITER(zval)				(zval).u2.fe_iter_idx
#define C_FE_ITER_P(zval_p)			C_FE_ITER(*(zval_p))

#define C_GUARD(zval)				(zval).u2.guard
#define C_GUARD_P(zval_p)			C_GUARD(*(zval_p))

#define C_CONSTANT_FLAGS(zval)		(zval).u2.constant_flags
#define C_CONSTANT_FLAGS_P(zval_p)	C_CONSTANT_FLAGS(*(zval_p))

#define C_EXTRA(zval)				(zval).u2.extra
#define C_EXTRA_P(zval_p)			C_EXTRA(*(zval_p))

#define C_COUNTED(zval)				(zval).value.counted
#define C_COUNTED_P(zval_p)			C_COUNTED(*(zval_p))

#define C_TYPE_MASK					0xff
#define C_TYPE_FLAGS_MASK			0xff00

#define C_TYPE_FLAGS_SHIFT			8
#define C_TYPE_INFO_EXTRA_SHIFT		16

#define GC_REFCOUNT(p)				crex_gc_refcount(&(p)->gc)
#define GC_SET_REFCOUNT(p, rc)		crex_gc_set_refcount(&(p)->gc, rc)
#define GC_ADDREF(p)				crex_gc_addref(&(p)->gc)
#define GC_DELREF(p)				crex_gc_delref(&(p)->gc)
#define GC_ADDREF_EX(p, rc)			crex_gc_addref_ex(&(p)->gc, rc)
#define GC_DELREF_EX(p, rc)			crex_gc_delref_ex(&(p)->gc, rc)
#define GC_TRY_ADDREF(p)			crex_gc_try_addref(&(p)->gc)
#define GC_TRY_DELREF(p)			crex_gc_try_delref(&(p)->gc)

#define GC_DTOR(p) \
	do { \
		crex_refcounted_h *_p = &(p)->gc; \
		if (crex_gc_delref(_p) == 0) { \
			rc_dtor_func((crex_refcounted *)_p); \
		} else { \
			gc_check_possible_root((crex_refcounted *)_p); \
		} \
	} while (0)

#define GC_DTOR_NO_REF(p) \
	do { \
		crex_refcounted_h *_p = &(p)->gc; \
		if (crex_gc_delref(_p) == 0) { \
			rc_dtor_func((crex_refcounted *)_p); \
		} else { \
			gc_check_possible_root_no_ref((crex_refcounted *)_p); \
		} \
	} while (0)

#define GC_TYPE_MASK				0x0000000f
#define GC_FLAGS_MASK				0x000003f0
#define GC_INFO_MASK				0xfffffc00
#define GC_FLAGS_SHIFT				0
#define GC_INFO_SHIFT				10

static crex_always_inline uint8_t zval_gc_type(uint32_t gc_type_info) {
	return (gc_type_info & GC_TYPE_MASK);
}

static crex_always_inline uint32_t zval_gc_flags(uint32_t gc_type_info) {
	return (gc_type_info >> GC_FLAGS_SHIFT) & (GC_FLAGS_MASK >> GC_FLAGS_SHIFT);
}

static crex_always_inline uint32_t zval_gc_info(uint32_t gc_type_info) {
	return (gc_type_info >> GC_INFO_SHIFT);
}

#define GC_TYPE_INFO(p)				(p)->gc.u.type_info
#define GC_TYPE(p)					zval_gc_type(GC_TYPE_INFO(p))
#define GC_FLAGS(p)					zval_gc_flags(GC_TYPE_INFO(p))
#define GC_INFO(p)					zval_gc_info(GC_TYPE_INFO(p))

#define GC_ADD_FLAGS(p, flags) do { \
		GC_TYPE_INFO(p) |= (flags) << GC_FLAGS_SHIFT; \
	} while (0)
#define GC_DEL_FLAGS(p, flags) do { \
		GC_TYPE_INFO(p) &= ~((flags) << GC_FLAGS_SHIFT); \
	} while (0)

#define C_GC_TYPE(zval)				GC_TYPE(C_COUNTED(zval))
#define C_GC_TYPE_P(zval_p)			C_GC_TYPE(*(zval_p))

#define C_GC_FLAGS(zval)			GC_FLAGS(C_COUNTED(zval))
#define C_GC_FLAGS_P(zval_p)		C_GC_FLAGS(*(zval_p))

#define C_GC_INFO(zval)				GC_INFO(C_COUNTED(zval))
#define C_GC_INFO_P(zval_p)			C_GC_INFO(*(zval_p))
#define C_GC_TYPE_INFO(zval)		GC_TYPE_INFO(C_COUNTED(zval))
#define C_GC_TYPE_INFO_P(zval_p)	C_GC_TYPE_INFO(*(zval_p))

/* zval_gc_flags(zval.value->gc.u.type_info) (common flags) */
#define GC_NOT_COLLECTABLE			(1<<4)
#define GC_PROTECTED                (1<<5) /* used for recursion detection */
#define GC_IMMUTABLE                (1<<6) /* can't be changed in place */
#define GC_PERSISTENT               (1<<7) /* allocated using malloc */
#define GC_PERSISTENT_LOCAL         (1<<8) /* persistent, but thread-local */

#define GC_NULL						(IS_NULL         | (GC_NOT_COLLECTABLE << GC_FLAGS_SHIFT))
#define GC_STRING					(IS_STRING       | (GC_NOT_COLLECTABLE << GC_FLAGS_SHIFT))
#define GC_ARRAY					IS_ARRAY
#define GC_OBJECT					IS_OBJECT
#define GC_RESOURCE					(IS_RESOURCE     | (GC_NOT_COLLECTABLE << GC_FLAGS_SHIFT))
#define GC_REFERENCE				(IS_REFERENCE    | (GC_NOT_COLLECTABLE << GC_FLAGS_SHIFT))
#define GC_CONSTANT_AST				(IS_CONSTANT_AST | (GC_NOT_COLLECTABLE << GC_FLAGS_SHIFT))

/* zval.u1.v.type_flags */
#define IS_TYPE_REFCOUNTED			(1<<0)
#define IS_TYPE_COLLECTABLE			(1<<1)
/* Used for static variables to check if they have been initialized. We can't use IS_UNDEF because
 * we can't store IS_UNDEF zvals in the static_variables HashTable. This needs to live in type_info
 * so that the CREX_ASSIGN overrides it but is moved to extra to avoid breaking the C_REFCOUNTED()
 * optimization that only checks for C_TYPE_FLAGS() without `& (IS_TYPE_COLLECTABLE|IS_TYPE_REFCOUNTED)`. */
#define IS_STATIC_VAR_UNINITIALIZED		(1<<0)

#if 1
/* This optimized version assumes that we have a single "type_flag" */
/* IS_TYPE_COLLECTABLE may be used only with IS_TYPE_REFCOUNTED */
# define C_TYPE_INFO_REFCOUNTED(t)	(((t) & C_TYPE_FLAGS_MASK) != 0)
#else
# define C_TYPE_INFO_REFCOUNTED(t)	(((t) & (IS_TYPE_REFCOUNTED << C_TYPE_FLAGS_SHIFT)) != 0)
#endif

/* extended types */
#define IS_INTERNED_STRING_EX		IS_STRING

#define IS_STRING_EX				(IS_STRING         | (IS_TYPE_REFCOUNTED << C_TYPE_FLAGS_SHIFT))
#define IS_ARRAY_EX					(IS_ARRAY          | (IS_TYPE_REFCOUNTED << C_TYPE_FLAGS_SHIFT) | (IS_TYPE_COLLECTABLE << C_TYPE_FLAGS_SHIFT))
#define IS_OBJECT_EX				(IS_OBJECT         | (IS_TYPE_REFCOUNTED << C_TYPE_FLAGS_SHIFT) | (IS_TYPE_COLLECTABLE << C_TYPE_FLAGS_SHIFT))
#define IS_RESOURCE_EX				(IS_RESOURCE       | (IS_TYPE_REFCOUNTED << C_TYPE_FLAGS_SHIFT))
#define IS_REFERENCE_EX				(IS_REFERENCE      | (IS_TYPE_REFCOUNTED << C_TYPE_FLAGS_SHIFT))

#define IS_CONSTANT_AST_EX			(IS_CONSTANT_AST   | (IS_TYPE_REFCOUNTED << C_TYPE_FLAGS_SHIFT))

/* string flags (zval.value->gc.u.flags) */
#define IS_STR_CLASS_NAME_MAP_PTR   GC_PROTECTED  /* refcount is a map_ptr offset of class_entry */
#define IS_STR_INTERNED				GC_IMMUTABLE  /* interned string */
#define IS_STR_PERSISTENT			GC_PERSISTENT /* allocated using malloc */
#define IS_STR_PERMANENT        	(1<<8)        /* relives request boundary */
#define IS_STR_VALID_UTF8           (1<<9)        /* valid UTF-8 according to PCRE */

/* array flags */
#define IS_ARRAY_IMMUTABLE			GC_IMMUTABLE
#define IS_ARRAY_PERSISTENT			GC_PERSISTENT

/* object flags (zval.value->gc.u.flags) */
#define IS_OBJ_WEAKLY_REFERENCED	GC_PERSISTENT
#define IS_OBJ_DESTRUCTOR_CALLED	(1<<8)
#define IS_OBJ_FREE_CALLED			(1<<9)

#define OBJ_FLAGS(obj)              GC_FLAGS(obj)

/* Fast class cache */
#define ZSTR_HAS_CE_CACHE(s)		(GC_FLAGS(s) & IS_STR_CLASS_NAME_MAP_PTR)
#define ZSTR_GET_CE_CACHE(s)		ZSTR_GET_CE_CACHE_EX(s, 1)
#define ZSTR_SET_CE_CACHE(s, ce)	ZSTR_SET_CE_CACHE_EX(s, ce, 1)

#define ZSTR_VALID_CE_CACHE(s)		EXPECTED((GC_REFCOUNT(s)-1)/sizeof(void *) < CG(map_ptr_last))

#define ZSTR_GET_CE_CACHE_EX(s, validate) \
	((!(validate) || ZSTR_VALID_CE_CACHE(s)) ? GET_CE_CACHE(GC_REFCOUNT(s)) : NULL)

#define ZSTR_SET_CE_CACHE_EX(s, ce, validate) do { \
		if (!(validate) || ZSTR_VALID_CE_CACHE(s)) { \
			CREX_ASSERT((validate) || ZSTR_VALID_CE_CACHE(s)); \
			SET_CE_CACHE(GC_REFCOUNT(s), ce); \
		} \
	} while (0)

#define GET_CE_CACHE(ce_cache) \
	(*(crex_class_entry **)CREX_MAP_PTR_OFFSET2PTR(ce_cache))

#define SET_CE_CACHE(ce_cache, ce) do { \
		*((crex_class_entry **)CREX_MAP_PTR_OFFSET2PTR(ce_cache)) = ce; \
	} while (0)

/* Recursion protection macros must be used only for arrays and objects */
#define GC_IS_RECURSIVE(p) \
	(GC_FLAGS(p) & GC_PROTECTED)

#define GC_PROTECT_RECURSION(p) do { \
		GC_ADD_FLAGS(p, GC_PROTECTED); \
	} while (0)

#define GC_UNPROTECT_RECURSION(p) do { \
		GC_DEL_FLAGS(p, GC_PROTECTED); \
	} while (0)

#define GC_TRY_PROTECT_RECURSION(p) do { \
		if (!(GC_FLAGS(p) & GC_IMMUTABLE)) GC_PROTECT_RECURSION(p); \
	} while (0)

#define GC_TRY_UNPROTECT_RECURSION(p) do { \
		if (!(GC_FLAGS(p) & GC_IMMUTABLE)) GC_UNPROTECT_RECURSION(p); \
	} while (0)

#define C_IS_RECURSIVE(zval)        GC_IS_RECURSIVE(C_COUNTED(zval))
#define C_PROTECT_RECURSION(zval)   GC_PROTECT_RECURSION(C_COUNTED(zval))
#define C_UNPROTECT_RECURSION(zval) GC_UNPROTECT_RECURSION(C_COUNTED(zval))
#define C_IS_RECURSIVE_P(zv)        C_IS_RECURSIVE(*(zv))
#define C_PROTECT_RECURSION_P(zv)   C_PROTECT_RECURSION(*(zv))
#define C_UNPROTECT_RECURSION_P(zv) C_UNPROTECT_RECURSION(*(zv))

#define CREX_GUARD_OR_GC_IS_RECURSIVE(pg, t, zobj) \
	(pg ? CREX_GUARD_IS_RECURSIVE(pg, t) : GC_IS_RECURSIVE(zobj))

#define CREX_GUARD_OR_GC_PROTECT_RECURSION(pg, t, zobj) do { \
		if (pg) { \
			CREX_GUARD_PROTECT_RECURSION(pg, t); \
		} else { \
			GC_PROTECT_RECURSION(zobj); \
		} \
	} while(0)

#define CREX_GUARD_OR_GC_UNPROTECT_RECURSION(pg, t, zobj) do { \
		if (pg) { \
			CREX_GUARD_UNPROTECT_RECURSION(pg, t); \
		} else { \
			GC_UNPROTECT_RECURSION(zobj); \
		} \
	} while(0)

/* All data types < IS_STRING have their constructor/destructors skipped */
#define C_CONSTANT(zval)			(C_TYPE(zval) == IS_CONSTANT_AST)
#define C_CONSTANT_P(zval_p)		C_CONSTANT(*(zval_p))

#if 1
/* This optimized version assumes that we have a single "type_flag" */
/* IS_TYPE_COLLECTABLE may be used only with IS_TYPE_REFCOUNTED */
#define C_REFCOUNTED(zval)			(C_TYPE_FLAGS(zval) != 0)
#else
#define C_REFCOUNTED(zval)			((C_TYPE_FLAGS(zval) & IS_TYPE_REFCOUNTED) != 0)
#endif
#define C_REFCOUNTED_P(zval_p)		C_REFCOUNTED(*(zval_p))

#define C_COLLECTABLE(zval)			((C_TYPE_FLAGS(zval) & IS_TYPE_COLLECTABLE) != 0)
#define C_COLLECTABLE_P(zval_p)		C_COLLECTABLE(*(zval_p))

/* deprecated: (COPYABLE is the same as IS_ARRAY) */
#define C_COPYABLE(zval)			(C_TYPE(zval) == IS_ARRAY)
#define C_COPYABLE_P(zval_p)		C_COPYABLE(*(zval_p))

/* deprecated: (IMMUTABLE is the same as IS_ARRAY && !REFCOUNTED) */
#define C_IMMUTABLE(zval)			(C_TYPE_INFO(zval) == IS_ARRAY)
#define C_IMMUTABLE_P(zval_p)		C_IMMUTABLE(*(zval_p))
#define C_OPT_IMMUTABLE(zval)		C_IMMUTABLE(zval_p)
#define C_OPT_IMMUTABLE_P(zval_p)	C_IMMUTABLE(*(zval_p))

/* the following C_OPT_* macros make better code when C_TYPE_INFO accessed before */
#define C_OPT_TYPE(zval)			(C_TYPE_INFO(zval) & C_TYPE_MASK)
#define C_OPT_TYPE_P(zval_p)		C_OPT_TYPE(*(zval_p))

#define C_OPT_CONSTANT(zval)		(C_OPT_TYPE(zval) == IS_CONSTANT_AST)
#define C_OPT_CONSTANT_P(zval_p)	C_OPT_CONSTANT(*(zval_p))

#define C_OPT_REFCOUNTED(zval)		C_TYPE_INFO_REFCOUNTED(C_TYPE_INFO(zval))
#define C_OPT_REFCOUNTED_P(zval_p)	C_OPT_REFCOUNTED(*(zval_p))

/* deprecated: (COPYABLE is the same as IS_ARRAY) */
#define C_OPT_COPYABLE(zval)		(C_OPT_TYPE(zval) == IS_ARRAY)
#define C_OPT_COPYABLE_P(zval_p)	C_OPT_COPYABLE(*(zval_p))

#define C_OPT_ISREF(zval)			(C_OPT_TYPE(zval) == IS_REFERENCE)
#define C_OPT_ISREF_P(zval_p)		C_OPT_ISREF(*(zval_p))

#define C_ISREF(zval)				(C_TYPE(zval) == IS_REFERENCE)
#define C_ISREF_P(zval_p)			C_ISREF(*(zval_p))

#define C_ISUNDEF(zval)				(C_TYPE(zval) == IS_UNDEF)
#define C_ISUNDEF_P(zval_p)			C_ISUNDEF(*(zval_p))

#define C_ISNULL(zval)				(C_TYPE(zval) == IS_NULL)
#define C_ISNULL_P(zval_p)			C_ISNULL(*(zval_p))

#define C_ISERROR(zval)				(C_TYPE(zval) == _IS_ERROR)
#define C_ISERROR_P(zval_p)			C_ISERROR(*(zval_p))

#define C_LVAL(zval)				(zval).value.lval
#define C_LVAL_P(zval_p)			C_LVAL(*(zval_p))

#define C_DVAL(zval)				(zval).value.dval
#define C_DVAL_P(zval_p)			C_DVAL(*(zval_p))

#define C_STR(zval)					(zval).value.str
#define C_STR_P(zval_p)				C_STR(*(zval_p))

#define C_STRVAL(zval)				ZSTR_VAL(C_STR(zval))
#define C_STRVAL_P(zval_p)			C_STRVAL(*(zval_p))

#define C_STRLEN(zval)				ZSTR_LEN(C_STR(zval))
#define C_STRLEN_P(zval_p)			C_STRLEN(*(zval_p))

#define C_STRHASH(zval)				ZSTR_HASH(C_STR(zval))
#define C_STRHASH_P(zval_p)			C_STRHASH(*(zval_p))

#define C_ARR(zval)					(zval).value.arr
#define C_ARR_P(zval_p)				C_ARR(*(zval_p))

#define C_ARRVAL(zval)				C_ARR(zval)
#define C_ARRVAL_P(zval_p)			C_ARRVAL(*(zval_p))

#define C_OBJ(zval)					(zval).value.obj
#define C_OBJ_P(zval_p)				C_OBJ(*(zval_p))

#define C_OBJ_HT(zval)				C_OBJ(zval)->handlers
#define C_OBJ_HT_P(zval_p)			C_OBJ_HT(*(zval_p))

#define C_OBJ_HANDLER(zval, hf)		C_OBJ_HT((zval))->hf
#define C_OBJ_HANDLER_P(zv_p, hf)	C_OBJ_HANDLER(*(zv_p), hf)

#define C_OBJ_HANDLE(zval)          (C_OBJ((zval)))->handle
#define C_OBJ_HANDLE_P(zval_p)      C_OBJ_HANDLE(*(zval_p))

#define C_OBJCE(zval)				(C_OBJ(zval)->ce)
#define C_OBJCE_P(zval_p)			C_OBJCE(*(zval_p))

#define C_OBJPROP(zval)				C_OBJ_HT((zval))->get_properties(C_OBJ(zval))
#define C_OBJPROP_P(zval_p)			C_OBJPROP(*(zval_p))

#define C_RES(zval)					(zval).value.res
#define C_RES_P(zval_p)				C_RES(*zval_p)

#define C_RES_HANDLE(zval)			C_RES(zval)->handle
#define C_RES_HANDLE_P(zval_p)		C_RES_HANDLE(*zval_p)

#define C_RES_TYPE(zval)			C_RES(zval)->type
#define C_RES_TYPE_P(zval_p)		C_RES_TYPE(*zval_p)

#define C_RES_VAL(zval)				C_RES(zval)->ptr
#define C_RES_VAL_P(zval_p)			C_RES_VAL(*zval_p)

#define C_REF(zval)					(zval).value.ref
#define C_REF_P(zval_p)				C_REF(*(zval_p))

#define C_REFVAL(zval)				&C_REF(zval)->val
#define C_REFVAL_P(zval_p)			C_REFVAL(*(zval_p))

#define C_AST(zval)					(zval).value.ast
#define C_AST_P(zval_p)				C_AST(*(zval_p))

#define GC_AST(p)					((crex_ast*)(((char*)p) + sizeof(crex_ast_ref)))

#define C_ASTVAL(zval)				GC_AST(C_AST(zval))
#define C_ASTVAL_P(zval_p)			C_ASTVAL(*(zval_p))

#define C_INDIRECT(zval)			(zval).value.zv
#define C_INDIRECT_P(zval_p)		C_INDIRECT(*(zval_p))

#define C_CE(zval)					(zval).value.ce
#define C_CE_P(zval_p)				C_CE(*(zval_p))

#define C_FUNC(zval)				(zval).value.func
#define C_FUNC_P(zval_p)			C_FUNC(*(zval_p))

#define C_PTR(zval)					(zval).value.ptr
#define C_PTR_P(zval_p)				C_PTR(*(zval_p))

#define ZVAL_UNDEF(z) do {				\
		C_TYPE_INFO_P(z) = IS_UNDEF;	\
	} while (0)

#define ZVAL_NULL(z) do {				\
		C_TYPE_INFO_P(z) = IS_NULL;		\
	} while (0)

#define ZVAL_FALSE(z) do {				\
		C_TYPE_INFO_P(z) = IS_FALSE;	\
	} while (0)

#define ZVAL_TRUE(z) do {				\
		C_TYPE_INFO_P(z) = IS_TRUE;		\
	} while (0)

#define ZVAL_BOOL(z, b) do {			\
		C_TYPE_INFO_P(z) =				\
			(b) ? IS_TRUE : IS_FALSE;	\
	} while (0)

#define ZVAL_LONG(z, l) do {			\
		zval *__z = (z);				\
		C_LVAL_P(__z) = l;				\
		C_TYPE_INFO_P(__z) = IS_LONG;	\
	} while (0)

#define ZVAL_DOUBLE(z, d) do {			\
		zval *__z = (z);				\
		C_DVAL_P(__z) = d;				\
		C_TYPE_INFO_P(__z) = IS_DOUBLE;	\
	} while (0)

#define ZVAL_STR(z, s) do {						\
		zval *__z = (z);						\
		crex_string *__s = (s);					\
		C_STR_P(__z) = __s;						\
		/* interned strings support */			\
		C_TYPE_INFO_P(__z) = ZSTR_IS_INTERNED(__s) ? \
			IS_INTERNED_STRING_EX : 			\
			IS_STRING_EX;						\
	} while (0)

#define ZVAL_INTERNED_STR(z, s) do {				\
		zval *__z = (z);							\
		crex_string *__s = (s);						\
		C_STR_P(__z) = __s;							\
		C_TYPE_INFO_P(__z) = IS_INTERNED_STRING_EX;	\
	} while (0)

#define ZVAL_NEW_STR(z, s) do {					\
		zval *__z = (z);						\
		crex_string *__s = (s);					\
		C_STR_P(__z) = __s;						\
		C_TYPE_INFO_P(__z) = IS_STRING_EX;		\
	} while (0)

#define ZVAL_STR_COPY(z, s) do {						\
		zval *__z = (z);								\
		crex_string *__s = (s);							\
		C_STR_P(__z) = __s;								\
		/* interned strings support */					\
		if (ZSTR_IS_INTERNED(__s)) {					\
			C_TYPE_INFO_P(__z) = IS_INTERNED_STRING_EX;	\
		} else {										\
			GC_ADDREF(__s);								\
			C_TYPE_INFO_P(__z) = IS_STRING_EX;			\
		}												\
	} while (0)

#define ZVAL_ARR(z, a) do {						\
		crex_array *__arr = (a);				\
		zval *__z = (z);						\
		C_ARR_P(__z) = __arr;					\
		C_TYPE_INFO_P(__z) = IS_ARRAY_EX;		\
	} while (0)

#define ZVAL_NEW_PERSISTENT_ARR(z) do {							\
		zval *__z = (z);										\
		crex_array *_arr =										\
		(crex_array *) malloc(sizeof(crex_array));				\
		C_ARR_P(__z) = _arr;									\
		C_TYPE_INFO_P(__z) = IS_ARRAY_EX;						\
	} while (0)

#define ZVAL_OBJ(z, o) do {						\
		zval *__z = (z);						\
		C_OBJ_P(__z) = (o);						\
		C_TYPE_INFO_P(__z) = IS_OBJECT_EX;		\
	} while (0)

#define ZVAL_OBJ_COPY(z, o) do {				\
		zval *__z = (z);						\
		crex_object *__o = (o);					\
		GC_ADDREF(__o);							\
		C_OBJ_P(__z) = __o;						\
		C_TYPE_INFO_P(__z) = IS_OBJECT_EX;		\
	} while (0)

#define ZVAL_RES(z, r) do {						\
		zval *__z = (z);						\
		C_RES_P(__z) = (r);						\
		C_TYPE_INFO_P(__z) = IS_RESOURCE_EX;	\
	} while (0)

#define ZVAL_NEW_RES(z, h, p, t) do {							\
		crex_resource *_res =									\
		(crex_resource *) emalloc(sizeof(crex_resource));		\
		zval *__z;												\
		GC_SET_REFCOUNT(_res, 1);								\
		GC_TYPE_INFO(_res) = GC_RESOURCE;						\
		_res->handle = (h);										\
		_res->type = (t);										\
		_res->ptr = (p);										\
		__z = (z);												\
		C_RES_P(__z) = _res;									\
		C_TYPE_INFO_P(__z) = IS_RESOURCE_EX;					\
	} while (0)

#define ZVAL_NEW_PERSISTENT_RES(z, h, p, t) do {				\
		crex_resource *_res =									\
		(crex_resource *) malloc(sizeof(crex_resource));		\
		zval *__z;												\
		GC_SET_REFCOUNT(_res, 1);								\
		GC_TYPE_INFO(_res) = GC_RESOURCE |						\
			(GC_PERSISTENT << GC_FLAGS_SHIFT);					\
		_res->handle = (h);										\
		_res->type = (t);										\
		_res->ptr = (p);										\
		__z = (z);												\
		C_RES_P(__z) = _res;									\
		C_TYPE_INFO_P(__z) = IS_RESOURCE_EX;					\
	} while (0)

#define ZVAL_REF(z, r) do {										\
		zval *__z = (z);										\
		C_REF_P(__z) = (r);										\
		C_TYPE_INFO_P(__z) = IS_REFERENCE_EX;					\
	} while (0)

#define ZVAL_NEW_EMPTY_REF(z) do {								\
		crex_reference *_ref =									\
		(crex_reference *) emalloc(sizeof(crex_reference));		\
		GC_SET_REFCOUNT(_ref, 1);								\
		GC_TYPE_INFO(_ref) = GC_REFERENCE;						\
		_ref->sources.ptr = NULL;									\
		C_REF_P(z) = _ref;										\
		C_TYPE_INFO_P(z) = IS_REFERENCE_EX;						\
	} while (0)

#define ZVAL_NEW_REF(z, r) do {									\
		crex_reference *_ref =									\
		(crex_reference *) emalloc(sizeof(crex_reference));		\
		GC_SET_REFCOUNT(_ref, 1);								\
		GC_TYPE_INFO(_ref) = GC_REFERENCE;						\
		ZVAL_COPY_VALUE(&_ref->val, r);							\
		_ref->sources.ptr = NULL;									\
		C_REF_P(z) = _ref;										\
		C_TYPE_INFO_P(z) = IS_REFERENCE_EX;						\
	} while (0)

#define ZVAL_MAKE_REF_EX(z, refcount) do {						\
		zval *_z = (z);											\
		crex_reference *_ref =									\
			(crex_reference *) emalloc(sizeof(crex_reference));	\
		GC_SET_REFCOUNT(_ref, (refcount));						\
		GC_TYPE_INFO(_ref) = GC_REFERENCE;						\
		ZVAL_COPY_VALUE(&_ref->val, _z);						\
		_ref->sources.ptr = NULL;									\
		C_REF_P(_z) = _ref;										\
		C_TYPE_INFO_P(_z) = IS_REFERENCE_EX;					\
	} while (0)

#define ZVAL_NEW_PERSISTENT_REF(z, r) do {						\
		crex_reference *_ref =									\
		(crex_reference *) malloc(sizeof(crex_reference));		\
		GC_SET_REFCOUNT(_ref, 1);								\
		GC_TYPE_INFO(_ref) = GC_REFERENCE |						\
			(GC_PERSISTENT << GC_FLAGS_SHIFT);					\
		ZVAL_COPY_VALUE(&_ref->val, r);							\
		_ref->sources.ptr = NULL;									\
		C_REF_P(z) = _ref;										\
		C_TYPE_INFO_P(z) = IS_REFERENCE_EX;						\
	} while (0)

#define ZVAL_AST(z, ast) do {									\
		zval *__z = (z);										\
		C_AST_P(__z) = ast;										\
		C_TYPE_INFO_P(__z) = IS_CONSTANT_AST_EX;				\
	} while (0)

#define ZVAL_INDIRECT(z, v) do {								\
		C_INDIRECT_P(z) = (v);									\
		C_TYPE_INFO_P(z) = IS_INDIRECT;							\
	} while (0)

#define ZVAL_PTR(z, p) do {										\
		C_PTR_P(z) = (p);										\
		C_TYPE_INFO_P(z) = IS_PTR;								\
	} while (0)

#define ZVAL_FUNC(z, f) do {									\
		C_FUNC_P(z) = (f);										\
		C_TYPE_INFO_P(z) = IS_PTR;								\
	} while (0)

#define ZVAL_CE(z, c) do {										\
		C_CE_P(z) = (c);										\
		C_TYPE_INFO_P(z) = IS_PTR;								\
	} while (0)

#define ZVAL_ALIAS_PTR(z, p) do {								\
		C_PTR_P(z) = (p);										\
		C_TYPE_INFO_P(z) = IS_ALIAS_PTR;						\
	} while (0)

#define ZVAL_ERROR(z) do {				\
		C_TYPE_INFO_P(z) = _IS_ERROR;	\
	} while (0)

#define C_REFCOUNT_P(pz)			zval_refcount_p(pz)
#define C_SET_REFCOUNT_P(pz, rc)	zval_set_refcount_p(pz, rc)
#define C_ADDREF_P(pz)				zval_addref_p(pz)
#define C_DELREF_P(pz)				zval_delref_p(pz)

#define C_REFCOUNT(z)				C_REFCOUNT_P(&(z))
#define C_SET_REFCOUNT(z, rc)		C_SET_REFCOUNT_P(&(z), rc)
#define C_ADDREF(z)					C_ADDREF_P(&(z))
#define C_DELREF(z)					C_DELREF_P(&(z))

#define C_TRY_ADDREF_P(pz) do {		\
	if (C_REFCOUNTED_P((pz))) {		\
		C_ADDREF_P((pz));			\
	}								\
} while (0)

#define C_TRY_DELREF_P(pz) do {		\
	if (C_REFCOUNTED_P((pz))) {		\
		C_DELREF_P((pz));			\
	}								\
} while (0)

#define C_TRY_ADDREF(z)				C_TRY_ADDREF_P(&(z))
#define C_TRY_DELREF(z)				C_TRY_DELREF_P(&(z))

#ifndef CREX_RC_DEBUG
# define CREX_RC_DEBUG 0
#endif

#if CREX_RC_DEBUG
extern CREX_API bool crex_rc_debug;
/* The GC_PERSISTENT flag is reused for IS_OBJ_WEAKLY_REFERENCED on objects.
 * Skip checks for OBJECT/NULL type to avoid interpreting the flag incorrectly. */
# define CREX_RC_MOD_CHECK(p) do { \
		if (crex_rc_debug) { \
			uint8_t type = zval_gc_type((p)->u.type_info); \
			if (type != IS_OBJECT && type != IS_NULL) { \
				CREX_ASSERT(!(zval_gc_flags((p)->u.type_info) & GC_IMMUTABLE)); \
				CREX_ASSERT((zval_gc_flags((p)->u.type_info) & (GC_PERSISTENT|GC_PERSISTENT_LOCAL)) != GC_PERSISTENT); \
			} \
		} \
	} while (0)
# define GC_MAKE_PERSISTENT_LOCAL(p) do { \
		GC_ADD_FLAGS(p, GC_PERSISTENT_LOCAL); \
	} while (0)
#else
# define CREX_RC_MOD_CHECK(p) \
	do { } while (0)
# define GC_MAKE_PERSISTENT_LOCAL(p) \
	do { } while (0)
#endif

static crex_always_inline uint32_t crex_gc_refcount(const crex_refcounted_h *p) {
	return p->refcount;
}

static crex_always_inline uint32_t crex_gc_set_refcount(crex_refcounted_h *p, uint32_t rc) {
	p->refcount = rc;
	return p->refcount;
}

static crex_always_inline uint32_t crex_gc_addref(crex_refcounted_h *p) {
	CREX_RC_MOD_CHECK(p);
	return ++(p->refcount);
}

static crex_always_inline void crex_gc_try_addref(crex_refcounted_h *p) {
	if (!(p->u.type_info & GC_IMMUTABLE)) {
		CREX_RC_MOD_CHECK(p);
		++p->refcount;
	}
}

static crex_always_inline void crex_gc_try_delref(crex_refcounted_h *p) {
	if (!(p->u.type_info & GC_IMMUTABLE)) {
		CREX_RC_MOD_CHECK(p);
		--p->refcount;
	}
}

static crex_always_inline uint32_t crex_gc_delref(crex_refcounted_h *p) {
	CREX_ASSERT(p->refcount > 0);
	CREX_RC_MOD_CHECK(p);
	return --(p->refcount);
}

static crex_always_inline uint32_t crex_gc_addref_ex(crex_refcounted_h *p, uint32_t rc) {
	CREX_RC_MOD_CHECK(p);
	p->refcount += rc;
	return p->refcount;
}

static crex_always_inline uint32_t crex_gc_delref_ex(crex_refcounted_h *p, uint32_t rc) {
	CREX_RC_MOD_CHECK(p);
	p->refcount -= rc;
	return p->refcount;
}

static crex_always_inline uint32_t zval_refcount_p(const zval* pz) {
#if CREX_DEBUG
	CREX_ASSERT(C_REFCOUNTED_P(pz) || C_TYPE_P(pz) == IS_ARRAY);
#endif
	return GC_REFCOUNT(C_COUNTED_P(pz));
}

static crex_always_inline uint32_t zval_set_refcount_p(zval* pz, uint32_t rc) {
	CREX_ASSERT(C_REFCOUNTED_P(pz));
	return GC_SET_REFCOUNT(C_COUNTED_P(pz), rc);
}

static crex_always_inline uint32_t zval_addref_p(zval* pz) {
	CREX_ASSERT(C_REFCOUNTED_P(pz));
	return GC_ADDREF(C_COUNTED_P(pz));
}

static crex_always_inline uint32_t zval_delref_p(zval* pz) {
	CREX_ASSERT(C_REFCOUNTED_P(pz));
	return GC_DELREF(C_COUNTED_P(pz));
}

#if SIZEOF_SIZE_T == 4
# define ZVAL_COPY_VALUE_EX(z, v, gc, t)				\
	do {												\
		uint32_t _w2 = v->value.ww.w2;					\
		C_COUNTED_P(z) = gc;							\
		z->value.ww.w2 = _w2;							\
		C_TYPE_INFO_P(z) = t;							\
	} while (0)
#elif SIZEOF_SIZE_T == 8
# define ZVAL_COPY_VALUE_EX(z, v, gc, t)				\
	do {												\
		C_COUNTED_P(z) = gc;							\
		C_TYPE_INFO_P(z) = t;							\
	} while (0)
#else
# error "Unknown SIZEOF_SIZE_T"
#endif

#define ZVAL_COPY_VALUE(z, v)							\
	do {												\
		zval *_z1 = (z);								\
		const zval *_z2 = (v);							\
		crex_refcounted *_gc = C_COUNTED_P(_z2);		\
		uint32_t _t = C_TYPE_INFO_P(_z2);				\
		ZVAL_COPY_VALUE_EX(_z1, _z2, _gc, _t);			\
	} while (0)

#define ZVAL_COPY(z, v)									\
	do {												\
		zval *_z1 = (z);								\
		const zval *_z2 = (v);							\
		crex_refcounted *_gc = C_COUNTED_P(_z2);		\
		uint32_t _t = C_TYPE_INFO_P(_z2);				\
		ZVAL_COPY_VALUE_EX(_z1, _z2, _gc, _t);			\
		if (C_TYPE_INFO_REFCOUNTED(_t)) {				\
			GC_ADDREF(_gc);								\
		}												\
	} while (0)

#define ZVAL_DUP(z, v)									\
	do {												\
		zval *_z1 = (z);								\
		const zval *_z2 = (v);							\
		crex_refcounted *_gc = C_COUNTED_P(_z2);		\
		uint32_t _t = C_TYPE_INFO_P(_z2);				\
		if ((_t & C_TYPE_MASK) == IS_ARRAY) {			\
			ZVAL_ARR(_z1, crex_array_dup((crex_array*)_gc));\
		} else {										\
			if (C_TYPE_INFO_REFCOUNTED(_t)) {			\
				GC_ADDREF(_gc);							\
			}											\
			ZVAL_COPY_VALUE_EX(_z1, _z2, _gc, _t);		\
		}												\
	} while (0)


/* ZVAL_COPY_OR_DUP() should be used instead of ZVAL_COPY() and ZVAL_DUP()
 * in all places where the source may be a persistent zval.
 */
#define ZVAL_COPY_OR_DUP(z, v)											\
	do {																\
		zval *_z1 = (z);												\
		const zval *_z2 = (v);											\
		crex_refcounted *_gc = C_COUNTED_P(_z2);						\
		uint32_t _t = C_TYPE_INFO_P(_z2);								\
		ZVAL_COPY_VALUE_EX(_z1, _z2, _gc, _t);							\
		if (C_TYPE_INFO_REFCOUNTED(_t)) {								\
			/* Objects reuse PERSISTENT as WEAKLY_REFERENCED */			\
			if (EXPECTED(!(GC_FLAGS(_gc) & GC_PERSISTENT)				\
					|| GC_TYPE(_gc) == IS_OBJECT)) {					\
				GC_ADDREF(_gc);											\
			} else {													\
				zval_copy_ctor_func(_z1);								\
			}															\
		}																\
	} while (0)

#define ZVAL_DEREF(z) do {								\
		if (UNEXPECTED(C_ISREF_P(z))) {					\
			(z) = C_REFVAL_P(z);						\
		}												\
	} while (0)

#define ZVAL_DEINDIRECT(z) do {							\
		if (C_TYPE_P(z) == IS_INDIRECT) {				\
			(z) = C_INDIRECT_P(z);						\
		}												\
	} while (0)

#define ZVAL_OPT_DEREF(z) do {							\
		if (UNEXPECTED(C_OPT_ISREF_P(z))) {				\
			(z) = C_REFVAL_P(z);						\
		}												\
	} while (0)

#define ZVAL_MAKE_REF(zv) do {							\
		zval *__zv = (zv);								\
		if (!C_ISREF_P(__zv)) {							\
			ZVAL_NEW_REF(__zv, __zv);					\
		}												\
	} while (0)

#define ZVAL_UNREF(z) do {								\
		zval *_z = (z);									\
		crex_reference *ref;							\
		CREX_ASSERT(C_ISREF_P(_z));						\
		ref = C_REF_P(_z);								\
		ZVAL_COPY_VALUE(_z, &ref->val);					\
		efree_size(ref, sizeof(crex_reference));		\
	} while (0)

#define ZVAL_COPY_DEREF(z, v) do {						\
		zval *_z3 = (v);								\
		if (C_OPT_REFCOUNTED_P(_z3)) {					\
			if (UNEXPECTED(C_OPT_ISREF_P(_z3))) {		\
				_z3 = C_REFVAL_P(_z3);					\
				if (C_OPT_REFCOUNTED_P(_z3)) {			\
					C_ADDREF_P(_z3);					\
				}										\
			} else {									\
				C_ADDREF_P(_z3);						\
			}											\
		}												\
		ZVAL_COPY_VALUE(z, _z3);						\
	} while (0)


#define SEPARATE_STRING(zv) do {						\
		zval *_zv = (zv);								\
		if (C_REFCOUNT_P(_zv) > 1) {					\
			crex_string *_str = C_STR_P(_zv);			\
			CREX_ASSERT(C_REFCOUNTED_P(_zv));			\
			CREX_ASSERT(!ZSTR_IS_INTERNED(_str));		\
			ZVAL_NEW_STR(_zv, crex_string_init(			\
				ZSTR_VAL(_str),	ZSTR_LEN(_str), 0));	\
			GC_DELREF(_str);							\
		}												\
	} while (0)

#define SEPARATE_ARRAY(zv) do {							\
		zval *__zv = (zv);								\
		crex_array *_arr = C_ARR_P(__zv);				\
		if (UNEXPECTED(GC_REFCOUNT(_arr) > 1)) {		\
			ZVAL_ARR(__zv, crex_array_dup(_arr));		\
			GC_TRY_DELREF(_arr);						\
		}												\
	} while (0)

#define SEPARATE_ZVAL_NOREF(zv) do {					\
		zval *_zv = (zv);								\
		CREX_ASSERT(C_TYPE_P(_zv) != IS_REFERENCE);		\
		if (C_TYPE_P(_zv) == IS_ARRAY) {				\
			SEPARATE_ARRAY(_zv);						\
		}												\
	} while (0)

#define SEPARATE_ZVAL(zv) do {							\
		zval *_zv = (zv);								\
		if (C_ISREF_P(_zv)) {							\
			crex_reference *_r = C_REF_P(_zv);			\
			ZVAL_COPY_VALUE(_zv, &_r->val);				\
			if (GC_DELREF(_r) == 0) {					\
				efree_size(_r, sizeof(crex_reference));	\
			} else if (C_OPT_TYPE_P(_zv) == IS_ARRAY) {	\
				ZVAL_ARR(_zv, crex_array_dup(C_ARR_P(_zv)));\
				break;									\
			} else if (C_OPT_REFCOUNTED_P(_zv)) {		\
				C_ADDREF_P(_zv);						\
				break;									\
			}											\
		}												\
		if (C_TYPE_P(_zv) == IS_ARRAY) {				\
			SEPARATE_ARRAY(_zv);						\
		}												\
	} while (0)

/* Properties store a flag distinguishing unset and uninitialized properties
 * (both use IS_UNDEF type) in the C_EXTRA space. As such we also need to copy
 * the C_EXTRA space when copying property default values etc. We define separate
 * macros for this purpose, so this workaround is easier to remove in the future. */
#define IS_PROP_UNINIT (1<<0)
#define IS_PROP_REINITABLE (1<<1)  /* It has impact only on readonly properties */
#define C_PROP_FLAG_P(z) C_EXTRA_P(z)
#define ZVAL_COPY_VALUE_PROP(z, v) \
	do { *(z) = *(v); } while (0)
#define ZVAL_COPY_PROP(z, v) \
	do { ZVAL_COPY(z, v); C_PROP_FLAG_P(z) = C_PROP_FLAG_P(v); } while (0)
#define ZVAL_COPY_OR_DUP_PROP(z, v) \
	do { ZVAL_COPY_OR_DUP(z, v); C_PROP_FLAG_P(z) = C_PROP_FLAG_P(v); } while (0)


static crex_always_inline bool crex_may_modify_arg_in_place(const zval *arg)
{
	return C_REFCOUNTED_P(arg) && !(GC_FLAGS(C_COUNTED_P(arg)) & (GC_IMMUTABLE | GC_PERSISTENT)) && C_REFCOUNT_P(arg) == 1;
}

#endif /* CREX_TYPES_H */
