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
   +----------------------------------------------------------------------+
*/

#ifndef CREX_HASH_H
#define CREX_HASH_H

#include "crex.h"
#include "crex_sort.h"

#define HASH_KEY_IS_STRING 1
#define HASH_KEY_IS_LONG 2
#define HASH_KEY_NON_EXISTENT 3

#define HASH_UPDATE 			(1<<0)
#define HASH_ADD				(1<<1)
#define HASH_UPDATE_INDIRECT	(1<<2)
#define HASH_ADD_NEW			(1<<3)
#define HASH_ADD_NEXT			(1<<4)
#define HASH_LOOKUP				(1<<5)

#define HASH_FLAG_CONSISTENCY      ((1<<0) | (1<<1))
#define HASH_FLAG_PACKED           (1<<2)
#define HASH_FLAG_UNINITIALIZED    (1<<3)
#define HASH_FLAG_STATIC_KEYS      (1<<4) /* long and interned strings */
#define HASH_FLAG_HAS_EMPTY_IND    (1<<5)
#define HASH_FLAG_ALLOW_COW_VIOLATION (1<<6)

/* Only the low byte are real flags */
#define HASH_FLAG_MASK 0xff

#define HT_FLAGS(ht) (ht)->u.flags

#define HT_INVALIDATE(ht) do { \
		HT_FLAGS(ht) = HASH_FLAG_UNINITIALIZED; \
	} while (0)

#define HT_IS_INITIALIZED(ht) \
	((HT_FLAGS(ht) & HASH_FLAG_UNINITIALIZED) == 0)

#define HT_IS_PACKED(ht) \
	((HT_FLAGS(ht) & HASH_FLAG_PACKED) != 0)

#define HT_IS_WITHOUT_HOLES(ht) \
	((ht)->nNumUsed == (ht)->nNumOfElements)

#define HT_HAS_STATIC_KEYS_ONLY(ht) \
	((HT_FLAGS(ht) & (HASH_FLAG_PACKED|HASH_FLAG_STATIC_KEYS)) != 0)

#if CREX_DEBUG
# define HT_ALLOW_COW_VIOLATION(ht) HT_FLAGS(ht) |= HASH_FLAG_ALLOW_COW_VIOLATION
#else
# define HT_ALLOW_COW_VIOLATION(ht)
#endif

#define HT_ITERATORS_COUNT(ht) (ht)->u.v.nIteratorsCount
#define HT_ITERATORS_OVERFLOW(ht) (HT_ITERATORS_COUNT(ht) == 0xff)
#define HT_HAS_ITERATORS(ht) (HT_ITERATORS_COUNT(ht) != 0)

#define HT_SET_ITERATORS_COUNT(ht, iters) \
	do { HT_ITERATORS_COUNT(ht) = (iters); } while (0)
#define HT_INC_ITERATORS_COUNT(ht) \
	HT_SET_ITERATORS_COUNT(ht, HT_ITERATORS_COUNT(ht) + 1)
#define HT_DEC_ITERATORS_COUNT(ht) \
	HT_SET_ITERATORS_COUNT(ht, HT_ITERATORS_COUNT(ht) - 1)

extern CREX_API const HashTable crex_empty_array;

#define ZVAL_EMPTY_ARRAY(z) do {						\
		zval *__z = (z);								\
		C_ARR_P(__z) = (crex_array*)&crex_empty_array;	\
		C_TYPE_INFO_P(__z) = IS_ARRAY; \
	} while (0)


typedef struct _crex_hash_key {
	crex_ulong h;
	crex_string *key;
} crex_hash_key;

typedef bool (*merge_checker_func_t)(HashTable *target_ht, zval *source_data, crex_hash_key *hash_key, void *pParam);

BEGIN_EXTERN_C()

/* startup/shutdown */
CREX_API void CREX_FASTCALL _crex_hash_init(HashTable *ht, uint32_t nSize, dtor_func_t pDestructor, bool persistent);
CREX_API void CREX_FASTCALL crex_hash_destroy(HashTable *ht);
CREX_API void CREX_FASTCALL crex_hash_clean(HashTable *ht);

#define crex_hash_init(ht, nSize, pHashFunction, pDestructor, persistent) \
	_crex_hash_init((ht), (nSize), (pDestructor), (persistent))

CREX_API void CREX_FASTCALL crex_hash_real_init(HashTable *ht, bool packed);
CREX_API void CREX_FASTCALL crex_hash_real_init_packed(HashTable *ht);
CREX_API void CREX_FASTCALL crex_hash_real_init_mixed(HashTable *ht);
CREX_API void CREX_FASTCALL crex_hash_packed_to_hash(HashTable *ht);
CREX_API void CREX_FASTCALL crex_hash_to_packed(HashTable *ht);
CREX_API void CREX_FASTCALL crex_hash_extend(HashTable *ht, uint32_t nSize, bool packed);
CREX_API void CREX_FASTCALL crex_hash_discard(HashTable *ht, uint32_t nNumUsed);
CREX_API void CREX_FASTCALL crex_hash_packed_grow(HashTable *ht);

/* additions/updates/changes */
CREX_API zval* CREX_FASTCALL crex_hash_add_or_update(HashTable *ht, crex_string *key, zval *pData, uint32_t flag);
CREX_API zval* CREX_FASTCALL crex_hash_update(HashTable *ht, crex_string *key,zval *pData);
CREX_API zval* CREX_FASTCALL crex_hash_update_ind(HashTable *ht, crex_string *key,zval *pData);
CREX_API zval* CREX_FASTCALL crex_hash_add(HashTable *ht, crex_string *key,zval *pData);
CREX_API zval* CREX_FASTCALL crex_hash_add_new(HashTable *ht, crex_string *key,zval *pData);

CREX_API zval* CREX_FASTCALL crex_hash_str_add_or_update(HashTable *ht, const char *key, size_t len, zval *pData, uint32_t flag);
CREX_API zval* CREX_FASTCALL crex_hash_str_update(HashTable *ht, const char *key, size_t len, zval *pData);
CREX_API zval* CREX_FASTCALL crex_hash_str_update_ind(HashTable *ht, const char *key, size_t len, zval *pData);
CREX_API zval* CREX_FASTCALL crex_hash_str_add(HashTable *ht, const char *key, size_t len, zval *pData);
CREX_API zval* CREX_FASTCALL crex_hash_str_add_new(HashTable *ht, const char *key, size_t len, zval *pData);

CREX_API zval* CREX_FASTCALL crex_hash_index_add_or_update(HashTable *ht, crex_ulong h, zval *pData, uint32_t flag);
CREX_API zval* CREX_FASTCALL crex_hash_index_add(HashTable *ht, crex_ulong h, zval *pData);
CREX_API zval* CREX_FASTCALL crex_hash_index_add_new(HashTable *ht, crex_ulong h, zval *pData);
CREX_API zval* CREX_FASTCALL crex_hash_index_update(HashTable *ht, crex_ulong h, zval *pData);
CREX_API zval* CREX_FASTCALL crex_hash_next_index_insert(HashTable *ht, zval *pData);
CREX_API zval* CREX_FASTCALL crex_hash_next_index_insert_new(HashTable *ht, zval *pData);

CREX_API zval* CREX_FASTCALL crex_hash_index_add_empty_element(HashTable *ht, crex_ulong h);
CREX_API zval* CREX_FASTCALL crex_hash_add_empty_element(HashTable *ht, crex_string *key);
CREX_API zval* CREX_FASTCALL crex_hash_str_add_empty_element(HashTable *ht, const char *key, size_t len);

CREX_API zval* CREX_FASTCALL crex_hash_set_bucket_key(HashTable *ht, Bucket *p, crex_string *key);

#define CREX_HASH_APPLY_KEEP				0
#define CREX_HASH_APPLY_REMOVE				1<<0
#define CREX_HASH_APPLY_STOP				1<<1

typedef int (*apply_func_t)(zval *pDest);
typedef int (*apply_func_arg_t)(zval *pDest, void *argument);
typedef int (*apply_func_args_t)(zval *pDest, int num_args, va_list args, crex_hash_key *hash_key);

CREX_API void CREX_FASTCALL crex_hash_graceful_destroy(HashTable *ht);
CREX_API void CREX_FASTCALL crex_hash_graceful_reverse_destroy(HashTable *ht);
CREX_API void CREX_FASTCALL crex_hash_apply(HashTable *ht, apply_func_t apply_func);
CREX_API void CREX_FASTCALL crex_hash_apply_with_argument(HashTable *ht, apply_func_arg_t apply_func, void *);
CREX_API void crex_hash_apply_with_arguments(HashTable *ht, apply_func_args_t apply_func, int, ...);

/* This function should be used with special care (in other words,
 * it should usually not be used).  When used with the CREX_HASH_APPLY_STOP
 * return value, it assumes things about the order of the elements in the hash.
 * Also, it does not provide the same kind of reentrancy protection that
 * the standard apply functions do.
 */
CREX_API void CREX_FASTCALL crex_hash_reverse_apply(HashTable *ht, apply_func_t apply_func);


/* Deletes */
CREX_API crex_result CREX_FASTCALL crex_hash_del(HashTable *ht, crex_string *key);
CREX_API crex_result CREX_FASTCALL crex_hash_del_ind(HashTable *ht, crex_string *key);
CREX_API crex_result CREX_FASTCALL crex_hash_str_del(HashTable *ht, const char *key, size_t len);
CREX_API crex_result CREX_FASTCALL crex_hash_str_del_ind(HashTable *ht, const char *key, size_t len);
CREX_API crex_result CREX_FASTCALL crex_hash_index_del(HashTable *ht, crex_ulong h);
CREX_API void CREX_FASTCALL crex_hash_del_bucket(HashTable *ht, Bucket *p);
CREX_API void CREX_FASTCALL crex_hash_packed_del_val(HashTable *ht, zval *zv);

/* Data retrieval */
CREX_API zval* CREX_FASTCALL crex_hash_find(const HashTable *ht, crex_string *key);
CREX_API zval* CREX_FASTCALL crex_hash_str_find(const HashTable *ht, const char *key, size_t len);
CREX_API zval* CREX_FASTCALL crex_hash_index_find(const HashTable *ht, crex_ulong h);
CREX_API zval* CREX_FASTCALL _crex_hash_index_find(const HashTable *ht, crex_ulong h);

/* The same as crex_hash_find(), but hash value of the key must be already calculated. */
CREX_API zval* CREX_FASTCALL crex_hash_find_known_hash(const HashTable *ht, const crex_string *key);

static crex_always_inline zval *crex_hash_find_ex(const HashTable *ht, crex_string *key, bool known_hash)
{
	if (known_hash) {
		return crex_hash_find_known_hash(ht, key);
	} else {
		return crex_hash_find(ht, key);
	}
}

#define CREX_HASH_INDEX_FIND(_ht, _h, _ret, _not_found) do { \
		if (EXPECTED(HT_IS_PACKED(_ht))) { \
			if (EXPECTED((crex_ulong)(_h) < (crex_ulong)(_ht)->nNumUsed)) { \
				_ret = &_ht->arPacked[_h]; \
				if (UNEXPECTED(C_TYPE_P(_ret) == IS_UNDEF)) { \
					goto _not_found; \
				} \
			} else { \
				goto _not_found; \
			} \
		} else { \
			_ret = _crex_hash_index_find(_ht, _h); \
			if (UNEXPECTED(_ret == NULL)) { \
				goto _not_found; \
			} \
		} \
	} while (0)


/* Find or add NULL, if doesn't exist */
CREX_API zval* CREX_FASTCALL crex_hash_lookup(HashTable *ht, crex_string *key);
CREX_API zval* CREX_FASTCALL crex_hash_index_lookup(HashTable *ht, crex_ulong h);

#define CREX_HASH_INDEX_LOOKUP(_ht, _h, _ret) do { \
		if (EXPECTED(HT_IS_PACKED(_ht))) { \
			if (EXPECTED((crex_ulong)(_h) < (crex_ulong)(_ht)->nNumUsed)) { \
				_ret = &_ht->arPacked[_h]; \
				if (EXPECTED(C_TYPE_P(_ret) != IS_UNDEF)) { \
					break; \
				} \
			} \
		} \
		_ret = crex_hash_index_lookup(_ht, _h); \
	} while (0)

/* Misc */
static crex_always_inline bool crex_hash_exists(const HashTable *ht, crex_string *key)
{
	return crex_hash_find(ht, key) != NULL;
}

static crex_always_inline bool crex_hash_str_exists(const HashTable *ht, const char *str, size_t len)
{
	return crex_hash_str_find(ht, str, len) != NULL;
}

static crex_always_inline bool crex_hash_index_exists(const HashTable *ht, crex_ulong h)
{
	return crex_hash_index_find(ht, h) != NULL;
}

/* traversing */
CREX_API HashPosition CREX_FASTCALL crex_hash_get_current_pos(const HashTable *ht);

CREX_API crex_result   CREX_FASTCALL crex_hash_move_forward_ex(HashTable *ht, HashPosition *pos);
CREX_API crex_result   CREX_FASTCALL crex_hash_move_backwards_ex(HashTable *ht, HashPosition *pos);
CREX_API int   CREX_FASTCALL crex_hash_get_current_key_ex(const HashTable *ht, crex_string **str_index, crex_ulong *num_index, const HashPosition *pos);
CREX_API void  CREX_FASTCALL crex_hash_get_current_key_zval_ex(const HashTable *ht, zval *key, const HashPosition *pos);
CREX_API int   CREX_FASTCALL crex_hash_get_current_key_type_ex(HashTable *ht, HashPosition *pos);
CREX_API zval* CREX_FASTCALL crex_hash_get_current_data_ex(HashTable *ht, HashPosition *pos);
CREX_API void  CREX_FASTCALL crex_hash_internal_pointer_reset_ex(HashTable *ht, HashPosition *pos);
CREX_API void  CREX_FASTCALL crex_hash_internal_pointer_end_ex(HashTable *ht, HashPosition *pos);

static crex_always_inline crex_result crex_hash_has_more_elements_ex(HashTable *ht, HashPosition *pos) {
	return (crex_hash_get_current_key_type_ex(ht, pos) == HASH_KEY_NON_EXISTENT ? FAILURE : SUCCESS);
}
static crex_always_inline crex_result crex_hash_has_more_elements(HashTable *ht) {
	return crex_hash_has_more_elements_ex(ht, &ht->nInternalPointer);
}
static crex_always_inline crex_result crex_hash_move_forward(HashTable *ht) {
	return crex_hash_move_forward_ex(ht, &ht->nInternalPointer);
}
static crex_always_inline crex_result crex_hash_move_backwards(HashTable *ht) {
	return crex_hash_move_backwards_ex(ht, &ht->nInternalPointer);
}
static crex_always_inline int crex_hash_get_current_key(const HashTable *ht, crex_string **str_index, crex_ulong *num_index) {
	return crex_hash_get_current_key_ex(ht, str_index, num_index, &ht->nInternalPointer);
}
static crex_always_inline void crex_hash_get_current_key_zval(const HashTable *ht, zval *key) {
	crex_hash_get_current_key_zval_ex(ht, key, &ht->nInternalPointer);
}
static crex_always_inline int crex_hash_get_current_key_type(HashTable *ht) {
	return crex_hash_get_current_key_type_ex(ht, &ht->nInternalPointer);
}
static crex_always_inline zval* crex_hash_get_current_data(HashTable *ht) {
	return crex_hash_get_current_data_ex(ht, &ht->nInternalPointer);
}
static crex_always_inline void crex_hash_internal_pointer_reset(HashTable *ht) {
	crex_hash_internal_pointer_reset_ex(ht, &ht->nInternalPointer);
}
static crex_always_inline void crex_hash_internal_pointer_end(HashTable *ht) {
	crex_hash_internal_pointer_end_ex(ht, &ht->nInternalPointer);
}

/* Copying, merging and sorting */
CREX_API void  CREX_FASTCALL crex_hash_copy(HashTable *target, HashTable *source, copy_ctor_func_t pCopyConstructor);
CREX_API void  CREX_FASTCALL crex_hash_merge(HashTable *target, HashTable *source, copy_ctor_func_t pCopyConstructor, bool overwrite);
CREX_API void  CREX_FASTCALL crex_hash_merge_ex(HashTable *target, HashTable *source, copy_ctor_func_t pCopyConstructor, merge_checker_func_t pMergeSource, void *pParam);
CREX_API void  crex_hash_bucket_swap(Bucket *p, Bucket *q);
CREX_API void  crex_hash_bucket_renum_swap(Bucket *p, Bucket *q);
CREX_API void  crex_hash_bucket_packed_swap(Bucket *p, Bucket *q);

typedef int (*bucket_compare_func_t)(Bucket *a, Bucket *b);
CREX_API int   crex_hash_compare(HashTable *ht1, HashTable *ht2, compare_func_t compar, bool ordered);
CREX_API void  CREX_FASTCALL crex_hash_sort_ex(HashTable *ht, sort_func_t sort_func, bucket_compare_func_t compare_func, bool renumber);
CREX_API zval* CREX_FASTCALL crex_hash_minmax(const HashTable *ht, compare_func_t compar, uint32_t flag);

static crex_always_inline void CREX_FASTCALL crex_hash_sort(HashTable *ht, bucket_compare_func_t compare_func, bool renumber) {
	crex_hash_sort_ex(ht, crex_sort, compare_func, renumber);
}

static crex_always_inline uint32_t crex_hash_num_elements(const HashTable *ht) {
	return ht->nNumOfElements;
}

static crex_always_inline crex_long crex_hash_next_free_element(const HashTable *ht) {
	return ht->nNextFreeElement;
}

CREX_API void CREX_FASTCALL crex_hash_rehash(HashTable *ht);

#if !CREX_DEBUG && defined(HAVE_BUILTIN_CONSTANT_P)
# define crex_new_array(size) \
	(__builtin_constant_p(size) ? \
		((((uint32_t)(size)) <= HT_MIN_SIZE) ? \
			_crex_new_array_0() \
		: \
			_crex_new_array((size)) \
		) \
	: \
		_crex_new_array((size)) \
	)
#else
# define crex_new_array(size) \
	_crex_new_array(size)
#endif

CREX_API HashTable* CREX_FASTCALL _crex_new_array_0(void);
CREX_API HashTable* CREX_FASTCALL _crex_new_array(uint32_t size);
CREX_API HashTable* CREX_FASTCALL crex_new_pair(zval *val1, zval *val2);
CREX_API uint32_t crex_array_count(HashTable *ht);
CREX_API HashTable* CREX_FASTCALL crex_array_dup(HashTable *source);
CREX_API void CREX_FASTCALL crex_array_destroy(HashTable *ht);
CREX_API HashTable* crex_array_to_list(HashTable *source);
CREX_API void CREX_FASTCALL crex_symtable_clean(HashTable *ht);
CREX_API HashTable* CREX_FASTCALL crex_symtable_to_proptable(HashTable *ht);
CREX_API HashTable* CREX_FASTCALL crex_proptable_to_symtable(HashTable *ht, bool always_duplicate);

CREX_API bool CREX_FASTCALL _crex_handle_numeric_str_ex(const char *key, size_t length, crex_ulong *idx);

CREX_API uint32_t     CREX_FASTCALL crex_hash_iterator_add(HashTable *ht, HashPosition pos);
CREX_API HashPosition CREX_FASTCALL crex_hash_iterator_pos(uint32_t idx, HashTable *ht);
CREX_API HashPosition CREX_FASTCALL crex_hash_iterator_pos_ex(uint32_t idx, zval *array);
CREX_API void         CREX_FASTCALL crex_hash_iterator_del(uint32_t idx);
CREX_API HashPosition CREX_FASTCALL crex_hash_iterators_lower_pos(HashTable *ht, HashPosition start);
CREX_API void         CREX_FASTCALL _crex_hash_iterators_update(HashTable *ht, HashPosition from, HashPosition to);
CREX_API void         CREX_FASTCALL crex_hash_iterators_advance(HashTable *ht, HashPosition step);

static crex_always_inline void crex_hash_iterators_update(HashTable *ht, HashPosition from, HashPosition to)
{
	if (UNEXPECTED(HT_HAS_ITERATORS(ht))) {
		_crex_hash_iterators_update(ht, from, to);
	}
}

/* For regular arrays (non-persistent, storing zvals). */
static crex_always_inline void crex_array_release(crex_array *array)
{
	if (!(GC_FLAGS(array) & IS_ARRAY_IMMUTABLE)) {
		if (GC_DELREF(array) == 0) {
			crex_array_destroy(array);
		}
	}
}

/* For general hashes (possibly persistent, storing any kind of value). */
static crex_always_inline void crex_hash_release(crex_array *array)
{
	if (!(GC_FLAGS(array) & IS_ARRAY_IMMUTABLE)) {
		if (GC_DELREF(array) == 0) {
			crex_hash_destroy(array);
			pefree(array, GC_FLAGS(array) & IS_ARRAY_PERSISTENT);
		}
	}
}

END_EXTERN_C()

#define CREX_INIT_SYMTABLE(ht)								\
	CREX_INIT_SYMTABLE_EX(ht, 8, 0)

#define CREX_INIT_SYMTABLE_EX(ht, n, persistent)			\
	crex_hash_init(ht, n, NULL, ZVAL_PTR_DTOR, persistent)

static crex_always_inline bool _crex_handle_numeric_str(const char *key, size_t length, crex_ulong *idx)
{
	const char *tmp = key;

	if (EXPECTED(*tmp > '9')) {
		return 0;
	} else if (*tmp < '0') {
		if (*tmp != '-') {
			return 0;
		}
		tmp++;
		if (*tmp > '9' || *tmp < '0') {
			return 0;
		}
	}
	return _crex_handle_numeric_str_ex(key, length, idx);
}

#define CREX_HANDLE_NUMERIC_STR(key, length, idx) \
	_crex_handle_numeric_str(key, length, &idx)

#define CREX_HANDLE_NUMERIC(key, idx) \
	CREX_HANDLE_NUMERIC_STR(ZSTR_VAL(key), ZSTR_LEN(key), idx)


static crex_always_inline zval *crex_hash_find_ind(const HashTable *ht, crex_string *key)
{
	zval *zv;

	zv = crex_hash_find(ht, key);
	return (zv && C_TYPE_P(zv) == IS_INDIRECT) ?
		((C_TYPE_P(C_INDIRECT_P(zv)) != IS_UNDEF) ? C_INDIRECT_P(zv) : NULL) : zv;
}


static crex_always_inline zval *crex_hash_find_ex_ind(const HashTable *ht, crex_string *key, bool known_hash)
{
	zval *zv;

	zv = crex_hash_find_ex(ht, key, known_hash);
	return (zv && C_TYPE_P(zv) == IS_INDIRECT) ?
		((C_TYPE_P(C_INDIRECT_P(zv)) != IS_UNDEF) ? C_INDIRECT_P(zv) : NULL) : zv;
}


static crex_always_inline bool crex_hash_exists_ind(const HashTable *ht, crex_string *key)
{
	zval *zv;

	zv = crex_hash_find(ht, key);
	return zv && (C_TYPE_P(zv) != IS_INDIRECT ||
			C_TYPE_P(C_INDIRECT_P(zv)) != IS_UNDEF);
}


static crex_always_inline zval *crex_hash_str_find_ind(const HashTable *ht, const char *str, size_t len)
{
	zval *zv;

	zv = crex_hash_str_find(ht, str, len);
	return (zv && C_TYPE_P(zv) == IS_INDIRECT) ?
		((C_TYPE_P(C_INDIRECT_P(zv)) != IS_UNDEF) ? C_INDIRECT_P(zv) : NULL) : zv;
}


static crex_always_inline bool crex_hash_str_exists_ind(const HashTable *ht, const char *str, size_t len)
{
	zval *zv;

	zv = crex_hash_str_find(ht, str, len);
	return zv && (C_TYPE_P(zv) != IS_INDIRECT ||
			C_TYPE_P(C_INDIRECT_P(zv)) != IS_UNDEF);
}

static crex_always_inline zval *crex_symtable_add_new(HashTable *ht, crex_string *key, zval *pData)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC(key, idx)) {
		return crex_hash_index_add_new(ht, idx, pData);
	} else {
		return crex_hash_add_new(ht, key, pData);
	}
}

static crex_always_inline zval *crex_symtable_update(HashTable *ht, crex_string *key, zval *pData)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC(key, idx)) {
		return crex_hash_index_update(ht, idx, pData);
	} else {
		return crex_hash_update(ht, key, pData);
	}
}


static crex_always_inline zval *crex_symtable_update_ind(HashTable *ht, crex_string *key, zval *pData)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC(key, idx)) {
		return crex_hash_index_update(ht, idx, pData);
	} else {
		return crex_hash_update_ind(ht, key, pData);
	}
}


static crex_always_inline crex_result crex_symtable_del(HashTable *ht, crex_string *key)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC(key, idx)) {
		return crex_hash_index_del(ht, idx);
	} else {
		return crex_hash_del(ht, key);
	}
}


static crex_always_inline crex_result crex_symtable_del_ind(HashTable *ht, crex_string *key)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC(key, idx)) {
		return crex_hash_index_del(ht, idx);
	} else {
		return crex_hash_del_ind(ht, key);
	}
}


static crex_always_inline zval *crex_symtable_find(const HashTable *ht, crex_string *key)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC(key, idx)) {
		return crex_hash_index_find(ht, idx);
	} else {
		return crex_hash_find(ht, key);
	}
}


static crex_always_inline zval *crex_symtable_find_ind(const HashTable *ht, crex_string *key)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC(key, idx)) {
		return crex_hash_index_find(ht, idx);
	} else {
		return crex_hash_find_ind(ht, key);
	}
}


static crex_always_inline bool crex_symtable_exists(HashTable *ht, crex_string *key)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC(key, idx)) {
		return crex_hash_index_exists(ht, idx);
	} else {
		return crex_hash_exists(ht, key);
	}
}


static crex_always_inline bool crex_symtable_exists_ind(HashTable *ht, crex_string *key)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC(key, idx)) {
		return crex_hash_index_exists(ht, idx);
	} else {
		return crex_hash_exists_ind(ht, key);
	}
}


static crex_always_inline zval *crex_symtable_str_update(HashTable *ht, const char *str, size_t len, zval *pData)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC_STR(str, len, idx)) {
		return crex_hash_index_update(ht, idx, pData);
	} else {
		return crex_hash_str_update(ht, str, len, pData);
	}
}


static crex_always_inline zval *crex_symtable_str_update_ind(HashTable *ht, const char *str, size_t len, zval *pData)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC_STR(str, len, idx)) {
		return crex_hash_index_update(ht, idx, pData);
	} else {
		return crex_hash_str_update_ind(ht, str, len, pData);
	}
}


static crex_always_inline crex_result crex_symtable_str_del(HashTable *ht, const char *str, size_t len)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC_STR(str, len, idx)) {
		return crex_hash_index_del(ht, idx);
	} else {
		return crex_hash_str_del(ht, str, len);
	}
}


static crex_always_inline crex_result crex_symtable_str_del_ind(HashTable *ht, const char *str, size_t len)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC_STR(str, len, idx)) {
		return crex_hash_index_del(ht, idx);
	} else {
		return crex_hash_str_del_ind(ht, str, len);
	}
}


static crex_always_inline zval *crex_symtable_str_find(HashTable *ht, const char *str, size_t len)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC_STR(str, len, idx)) {
		return crex_hash_index_find(ht, idx);
	} else {
		return crex_hash_str_find(ht, str, len);
	}
}


static crex_always_inline bool crex_symtable_str_exists(HashTable *ht, const char *str, size_t len)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC_STR(str, len, idx)) {
		return crex_hash_index_exists(ht, idx);
	} else {
		return crex_hash_str_exists(ht, str, len);
	}
}

static crex_always_inline void *crex_hash_add_ptr(HashTable *ht, crex_string *key, void *pData)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, pData);
	zv = crex_hash_add(ht, key, &tmp);
	if (zv) {
		CREX_ASSUME(C_PTR_P(zv));
		return C_PTR_P(zv);
	} else {
		return NULL;
	}
}

static crex_always_inline void *crex_hash_add_new_ptr(HashTable *ht, crex_string *key, void *pData)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, pData);
	zv = crex_hash_add_new(ht, key, &tmp);
	if (zv) {
		CREX_ASSUME(C_PTR_P(zv));
		return C_PTR_P(zv);
	} else {
		return NULL;
	}
}

static crex_always_inline void *crex_hash_str_add_ptr(HashTable *ht, const char *str, size_t len, void *pData)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, pData);
	zv = crex_hash_str_add(ht, str, len, &tmp);
	if (zv) {
		CREX_ASSUME(C_PTR_P(zv));
		return C_PTR_P(zv);
	} else {
		return NULL;
	}
}

static crex_always_inline void *crex_hash_str_add_new_ptr(HashTable *ht, const char *str, size_t len, void *pData)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, pData);
	zv = crex_hash_str_add_new(ht, str, len, &tmp);
	if (zv) {
		CREX_ASSUME(C_PTR_P(zv));
		return C_PTR_P(zv);
	} else {
		return NULL;
	}
}

static crex_always_inline void *crex_hash_update_ptr(HashTable *ht, crex_string *key, void *pData)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, pData);
	zv = crex_hash_update(ht, key, &tmp);
	CREX_ASSUME(C_PTR_P(zv));
	return C_PTR_P(zv);
}

static crex_always_inline void *crex_hash_str_update_ptr(HashTable *ht, const char *str, size_t len, void *pData)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, pData);
	zv = crex_hash_str_update(ht, str, len, &tmp);
	CREX_ASSUME(C_PTR_P(zv));
	return C_PTR_P(zv);
}

static crex_always_inline void *crex_hash_add_mem(HashTable *ht, crex_string *key, void *pData, size_t size)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, NULL);
	if ((zv = crex_hash_add(ht, key, &tmp))) {
		C_PTR_P(zv) = pemalloc(size, GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
		memcpy(C_PTR_P(zv), pData, size);
		return C_PTR_P(zv);
	}
	return NULL;
}

static crex_always_inline void *crex_hash_add_new_mem(HashTable *ht, crex_string *key, void *pData, size_t size)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, NULL);
	if ((zv = crex_hash_add_new(ht, key, &tmp))) {
		C_PTR_P(zv) = pemalloc(size, GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
		memcpy(C_PTR_P(zv), pData, size);
		return C_PTR_P(zv);
	}
	return NULL;
}

static crex_always_inline void *crex_hash_str_add_mem(HashTable *ht, const char *str, size_t len, void *pData, size_t size)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, NULL);
	if ((zv = crex_hash_str_add(ht, str, len, &tmp))) {
		C_PTR_P(zv) = pemalloc(size, GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
		memcpy(C_PTR_P(zv), pData, size);
		return C_PTR_P(zv);
	}
	return NULL;
}

static crex_always_inline void *crex_hash_str_add_new_mem(HashTable *ht, const char *str, size_t len, void *pData, size_t size)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, NULL);
	if ((zv = crex_hash_str_add_new(ht, str, len, &tmp))) {
		C_PTR_P(zv) = pemalloc(size, GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
		memcpy(C_PTR_P(zv), pData, size);
		return C_PTR_P(zv);
	}
	return NULL;
}

static crex_always_inline void *crex_hash_update_mem(HashTable *ht, crex_string *key, void *pData, size_t size)
{
	void *p;

	p = pemalloc(size, GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
	memcpy(p, pData, size);
	return crex_hash_update_ptr(ht, key, p);
}

static crex_always_inline void *crex_hash_str_update_mem(HashTable *ht, const char *str, size_t len, void *pData, size_t size)
{
	void *p;

	p = pemalloc(size, GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
	memcpy(p, pData, size);
	return crex_hash_str_update_ptr(ht, str, len, p);
}

static crex_always_inline void *crex_hash_index_add_ptr(HashTable *ht, crex_ulong h, void *pData)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, pData);
	zv = crex_hash_index_add(ht, h, &tmp);
	return zv ? C_PTR_P(zv) : NULL;
}

static crex_always_inline void *crex_hash_index_add_new_ptr(HashTable *ht, crex_ulong h, void *pData)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, pData);
	zv = crex_hash_index_add_new(ht, h, &tmp);
	return zv ? C_PTR_P(zv) : NULL;
}

static crex_always_inline void *crex_hash_index_update_ptr(HashTable *ht, crex_ulong h, void *pData)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, pData);
	zv = crex_hash_index_update(ht, h, &tmp);
	CREX_ASSUME(C_PTR_P(zv));
	return C_PTR_P(zv);
}

static crex_always_inline void *crex_hash_index_add_mem(HashTable *ht, crex_ulong h, void *pData, size_t size)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, NULL);
	if ((zv = crex_hash_index_add(ht, h, &tmp))) {
		C_PTR_P(zv) = pemalloc(size, GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
		memcpy(C_PTR_P(zv), pData, size);
		return C_PTR_P(zv);
	}
	return NULL;
}

static crex_always_inline void *crex_hash_next_index_insert_ptr(HashTable *ht, void *pData)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, pData);
	zv = crex_hash_next_index_insert(ht, &tmp);
	if (zv) {
		CREX_ASSUME(C_PTR_P(zv));
		return C_PTR_P(zv);
	} else {
		return NULL;
	}
}

static crex_always_inline void *crex_hash_index_update_mem(HashTable *ht, crex_ulong h, void *pData, size_t size)
{
	void *p;

	p = pemalloc(size, GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
	memcpy(p, pData, size);
	return crex_hash_index_update_ptr(ht, h, p);
}

static crex_always_inline void *crex_hash_next_index_insert_mem(HashTable *ht, void *pData, size_t size)
{
	zval tmp, *zv;

	ZVAL_PTR(&tmp, NULL);
	if ((zv = crex_hash_next_index_insert(ht, &tmp))) {
		C_PTR_P(zv) = pemalloc(size, GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
		memcpy(C_PTR_P(zv), pData, size);
		return C_PTR_P(zv);
	}
	return NULL;
}

static crex_always_inline void *crex_hash_find_ptr(const HashTable *ht, crex_string *key)
{
	zval *zv;

	zv = crex_hash_find(ht, key);
	if (zv) {
		CREX_ASSUME(C_PTR_P(zv));
		return C_PTR_P(zv);
	} else {
		return NULL;
	}
}

static crex_always_inline void *crex_hash_find_ex_ptr(const HashTable *ht, crex_string *key, bool known_hash)
{
	zval *zv;

	zv = crex_hash_find_ex(ht, key, known_hash);
	if (zv) {
		CREX_ASSUME(C_PTR_P(zv));
		return C_PTR_P(zv);
	} else {
		return NULL;
	}
}

static crex_always_inline void *crex_hash_str_find_ptr(const HashTable *ht, const char *str, size_t len)
{
	zval *zv;

	zv = crex_hash_str_find(ht, str, len);
	if (zv) {
		CREX_ASSUME(C_PTR_P(zv));
		return C_PTR_P(zv);
	} else {
		return NULL;
	}
}

/* Will lowercase the str; use only if you don't need the lowercased string for
 * anything else. If you have a lowered string, use crex_hash_str_find_ptr. */
CREX_API void *crex_hash_str_find_ptr_lc(const HashTable *ht, const char *str, size_t len);

/* Will lowercase the str; use only if you don't need the lowercased string for
 * anything else. If you have a lowered string, use crex_hash_find_ptr. */
CREX_API void *crex_hash_find_ptr_lc(const HashTable *ht, crex_string *key);

static crex_always_inline void *crex_hash_index_find_ptr(const HashTable *ht, crex_ulong h)
{
	zval *zv;

	zv = crex_hash_index_find(ht, h);
	if (zv) {
		CREX_ASSUME(C_PTR_P(zv));
		return C_PTR_P(zv);
	} else {
		return NULL;
	}
}

static crex_always_inline zval *crex_hash_index_find_deref(HashTable *ht, crex_ulong h)
{
	zval *zv = crex_hash_index_find(ht, h);
	if (zv) {
		ZVAL_DEREF(zv);
	}
	return zv;
}

static crex_always_inline zval *crex_hash_find_deref(HashTable *ht, crex_string *str)
{
	zval *zv = crex_hash_find(ht, str);
	if (zv) {
		ZVAL_DEREF(zv);
	}
	return zv;
}

static crex_always_inline zval *crex_hash_str_find_deref(HashTable *ht, const char *str, size_t len)
{
	zval *zv = crex_hash_str_find(ht, str, len);
	if (zv) {
		ZVAL_DEREF(zv);
	}
	return zv;
}

static crex_always_inline void *crex_symtable_str_find_ptr(HashTable *ht, const char *str, size_t len)
{
	crex_ulong idx;

	if (CREX_HANDLE_NUMERIC_STR(str, len, idx)) {
		return crex_hash_index_find_ptr(ht, idx);
	} else {
		return crex_hash_str_find_ptr(ht, str, len);
	}
}

static crex_always_inline void *crex_hash_get_current_data_ptr_ex(HashTable *ht, HashPosition *pos)
{
	zval *zv;

	zv = crex_hash_get_current_data_ex(ht, pos);
	if (zv) {
		CREX_ASSUME(C_PTR_P(zv));
		return C_PTR_P(zv);
	} else {
		return NULL;
	}
}

#define crex_hash_get_current_data_ptr(ht) \
	crex_hash_get_current_data_ptr_ex(ht, &(ht)->nInternalPointer)

/* Common hash/packed array iterators */
#if 0
# define CREX_HASH_ELEMENT_SIZE(__ht) \
	(HT_IS_PACKED(__ht) ? sizeof(zval) : sizeof(Bucket))
#else /* optimized version */
# define CREX_HASH_ELEMENT_SIZE(__ht) \
	(sizeof(zval) + (~HT_FLAGS(__ht) & HASH_FLAG_PACKED) * ((sizeof(Bucket)-sizeof(zval))/HASH_FLAG_PACKED))
#endif

#define CREX_HASH_ELEMENT_EX(__ht, _idx, _size) \
	((zval*)(((char*)(__ht)->arPacked) + ((_idx) * (_size))))

#define CREX_HASH_ELEMENT(__ht, _idx) \
	CREX_HASH_ELEMENT_EX(__ht, _idx, CREX_HASH_ELEMENT_SIZE(__ht))

#define CREX_HASH_NEXT_ELEMENT(_el, _size) \
	((zval*)(((char*)(_el)) + (_size)))

#define CREX_HASH_PREV_ELEMENT(_el, _size) \
	((zval*)(((char*)(_el)) - (_size)))

#define _CREX_HASH_FOREACH_VAL(_ht) do { \
		HashTable *__ht = (_ht); \
		uint32_t _count = __ht->nNumUsed; \
		size_t _size = CREX_HASH_ELEMENT_SIZE(__ht); \
		zval *_z = __ht->arPacked; \
		for (; _count > 0; _z = CREX_HASH_NEXT_ELEMENT(_z, _size), _count--) { \
			if (UNEXPECTED(C_TYPE_P(_z) == IS_UNDEF)) continue;

#define _CREX_HASH_REVERSE_FOREACH_VAL(_ht) do { \
		HashTable *__ht = (_ht); \
		uint32_t _idx = __ht->nNumUsed; \
		size_t _size = CREX_HASH_ELEMENT_SIZE(__ht); \
		zval *_z = CREX_HASH_ELEMENT_EX(__ht, _idx, _size); \
		for (;_idx > 0; _idx--) { \
			_z = CREX_HASH_PREV_ELEMENT(_z, _size); \
			if (UNEXPECTED(C_TYPE_P(_z) == IS_UNDEF)) continue;

#define CREX_HASH_FOREACH_FROM(_ht, indirect, _from) do { \
		HashTable *__ht = (_ht); \
		crex_ulong __h; \
		crex_string *__key = NULL; \
		uint32_t _idx = (_from); \
		size_t _size = CREX_HASH_ELEMENT_SIZE(__ht); \
		zval *__z = CREX_HASH_ELEMENT_EX(__ht, _idx, _size); \
		uint32_t _count = __ht->nNumUsed - _idx; \
		for (;_count > 0; _count--) { \
			zval *_z = __z; \
			if (HT_IS_PACKED(__ht)) { \
				__z++; \
				__h = _idx; \
				_idx++; \
			} else { \
				Bucket *_p = (Bucket*)__z; \
				__z = &(_p + 1)->val; \
				__h = _p->h; \
				__key = _p->key; \
				if (indirect && C_TYPE_P(_z) == IS_INDIRECT) { \
					_z = C_INDIRECT_P(_z); \
				} \
			} \
			(void) __h; (void) __key; (void) _idx; \
			if (UNEXPECTED(C_TYPE_P(_z) == IS_UNDEF)) continue;

#define CREX_HASH_FOREACH(_ht, indirect) CREX_HASH_FOREACH_FROM(_ht, indirect, 0)

#define CREX_HASH_REVERSE_FOREACH(_ht, indirect) do { \
		HashTable *__ht = (_ht); \
		uint32_t _idx = __ht->nNumUsed; \
		zval *_z; \
		crex_ulong __h; \
		crex_string *__key = NULL; \
		size_t _size = CREX_HASH_ELEMENT_SIZE(__ht); \
		zval *__z = CREX_HASH_ELEMENT_EX(__ht, _idx, _size); \
		for (;_idx > 0; _idx--) { \
			if (HT_IS_PACKED(__ht)) { \
				__z--; \
				_z = __z; \
				__h = _idx - 1; \
			} else { \
				Bucket *_p = (Bucket*)__z; \
				_p--; \
				__z = &_p->val; \
				_z = __z; \
				__h = _p->h; \
				__key = _p->key; \
				if (indirect && C_TYPE_P(_z) == IS_INDIRECT) { \
					_z = C_INDIRECT_P(_z); \
				} \
			} \
			(void) __h; (void) __key; (void) __z; \
			if (UNEXPECTED(C_TYPE_P(_z) == IS_UNDEF)) continue;

#define CREX_HASH_FOREACH_END() \
		} \
	} while (0)

#define CREX_HASH_FOREACH_END_DEL() \
	CREX_HASH_MAP_FOREACH_END_DEL()

#define CREX_HASH_FOREACH_BUCKET(ht, _bucket) \
	CREX_HASH_MAP_FOREACH_BUCKET(ht, _bucket)

#define CREX_HASH_FOREACH_BUCKET_FROM(ht, _bucket, _from) \
	CREX_HASH_MAP_FOREACH_BUCKET_FROM(ht, _bucket, _from)

#define CREX_HASH_REVERSE_FOREACH_BUCKET(ht, _bucket) \
	CREX_HASH_MAP_REVERSE_FOREACH_BUCKET(ht, _bucket)

#define CREX_HASH_FOREACH_VAL(ht, _val) \
	_CREX_HASH_FOREACH_VAL(ht); \
	_val = _z;

#define CREX_HASH_REVERSE_FOREACH_VAL(ht, _val) \
	_CREX_HASH_REVERSE_FOREACH_VAL(ht); \
	_val = _z;

#define CREX_HASH_FOREACH_VAL_IND(ht, _val) \
	CREX_HASH_FOREACH(ht, 1); \
	_val = _z;

#define CREX_HASH_REVERSE_FOREACH_VAL_IND(ht, _val) \
	CREX_HASH_REVERSE_FOREACH(ht, 1); \
	_val = _z;

#define CREX_HASH_FOREACH_PTR(ht, _ptr) \
	_CREX_HASH_FOREACH_VAL(ht); \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_FOREACH_PTR_FROM(ht, _ptr, _from) \
	CREX_HASH_FOREACH_FROM(ht, 0, _from); \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_REVERSE_FOREACH_PTR(ht, _ptr) \
	_CREX_HASH_REVERSE_FOREACH_VAL(ht); \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_FOREACH_NUM_KEY(ht, _h) \
	CREX_HASH_FOREACH(ht, 0); \
	_h = __h;

#define CREX_HASH_REVERSE_FOREACH_NUM_KEY(ht, _h) \
	CREX_HASH_REVERSE_FOREACH(ht, 0); \
	_h = __h;

#define CREX_HASH_FOREACH_STR_KEY(ht, _key) \
	CREX_HASH_FOREACH(ht, 0); \
	_key = __key;

#define CREX_HASH_REVERSE_FOREACH_STR_KEY(ht, _key) \
	CREX_HASH_REVERSE_FOREACH(ht, 0); \
	_key = __key;

#define CREX_HASH_FOREACH_KEY(ht, _h, _key) \
	CREX_HASH_FOREACH(ht, 0); \
	_h = __h; \
	_key = __key;

#define CREX_HASH_REVERSE_FOREACH_KEY(ht, _h, _key) \
	CREX_HASH_REVERSE_FOREACH(ht, 0); \
	_h = __h; \
	_key = __key;

#define CREX_HASH_FOREACH_NUM_KEY_VAL(ht, _h, _val) \
	CREX_HASH_FOREACH(ht, 0); \
	_h = __h; \
	_val = _z;

#define CREX_HASH_REVERSE_FOREACH_NUM_KEY_VAL(ht, _h, _val) \
	CREX_HASH_REVERSE_FOREACH(ht, 0); \
	_h = __h; \
	_val = _z;

#define CREX_HASH_FOREACH_STR_KEY_VAL(ht, _key, _val) \
	CREX_HASH_FOREACH(ht, 0); \
	_key = __key; \
	_val = _z;

#define CREX_HASH_FOREACH_STR_KEY_VAL_FROM(ht, _key, _val, _from) \
	CREX_HASH_FOREACH_FROM(ht, 0, _from); \
	_key = __key; \
	_val = _z;

#define CREX_HASH_REVERSE_FOREACH_STR_KEY_VAL(ht, _key, _val) \
	CREX_HASH_REVERSE_FOREACH(ht, 0); \
	_key = __key; \
	_val = _z;

#define CREX_HASH_FOREACH_KEY_VAL(ht, _h, _key, _val) \
	CREX_HASH_FOREACH(ht, 0); \
	_h = __h; \
	_key = __key; \
	_val = _z;

#define CREX_HASH_REVERSE_FOREACH_KEY_VAL(ht, _h, _key, _val) \
	CREX_HASH_REVERSE_FOREACH(ht, 0); \
	_h = __h; \
	_key = __key; \
	_val = _z;

#define CREX_HASH_FOREACH_STR_KEY_VAL_IND(ht, _key, _val) \
	CREX_HASH_FOREACH(ht, 1); \
	_key = __key; \
	_val = _z;

#define CREX_HASH_REVERSE_FOREACH_STR_KEY_VAL_IND(ht, _key, _val) \
	CREX_HASH_REVERSE_FOREACH(ht, 1); \
	_key = __key; \
	_val = _z;

#define CREX_HASH_FOREACH_KEY_VAL_IND(ht, _h, _key, _val) \
	CREX_HASH_FOREACH(ht, 1); \
	_h = __h; \
	_key = __key; \
	_val = _z;

#define CREX_HASH_REVERSE_FOREACH_KEY_VAL_IND(ht, _h, _key, _val) \
	CREX_HASH_REVERSE_FOREACH(ht, 1); \
	_h = __h; \
	_key = __key; \
	_val = _z;

#define CREX_HASH_FOREACH_NUM_KEY_PTR(ht, _h, _ptr) \
	CREX_HASH_FOREACH(ht, 0); \
	_h = __h; \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_REVERSE_FOREACH_NUM_KEY_PTR(ht, _h, _ptr) \
	CREX_HASH_REVERSE_FOREACH(ht, 0); \
	_h = __h; \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_FOREACH_STR_KEY_PTR(ht, _key, _ptr) \
	CREX_HASH_FOREACH(ht, 0); \
	_key = __key; \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_REVERSE_FOREACH_STR_KEY_PTR(ht, _key, _ptr) \
	CREX_HASH_REVERSE_FOREACH(ht, 0); \
	_key = __key; \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_FOREACH_KEY_PTR(ht, _h, _key, _ptr) \
	CREX_HASH_FOREACH(ht, 0); \
	_h = __h; \
	_key = __key; \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_REVERSE_FOREACH_KEY_PTR(ht, _h, _key, _ptr) \
	CREX_HASH_REVERSE_FOREACH(ht, 0); \
	_h = __h; \
	_key = __key; \
	_ptr = C_PTR_P(_z);

/* Hash array iterators */
#define CREX_HASH_MAP_FOREACH_FROM(_ht, indirect, _from) do { \
		HashTable *__ht = (_ht); \
		Bucket *_p = __ht->arData + (_from); \
		Bucket *_end = __ht->arData + __ht->nNumUsed; \
		CREX_ASSERT(!HT_IS_PACKED(__ht)); \
		for (; _p != _end; _p++) { \
			zval *_z = &_p->val; \
			if (indirect && C_TYPE_P(_z) == IS_INDIRECT) { \
				_z = C_INDIRECT_P(_z); \
			} \
			if (UNEXPECTED(C_TYPE_P(_z) == IS_UNDEF)) continue;

#define CREX_HASH_MAP_FOREACH(_ht, indirect) CREX_HASH_MAP_FOREACH_FROM(_ht, indirect, 0)

#define CREX_HASH_MAP_REVERSE_FOREACH(_ht, indirect) do { \
		HashTable *__ht = (_ht); \
		uint32_t _idx = __ht->nNumUsed; \
		Bucket *_p = __ht->arData + _idx; \
		zval *_z; \
		CREX_ASSERT(!HT_IS_PACKED(__ht)); \
		for (_idx = __ht->nNumUsed; _idx > 0; _idx--) { \
			_p--; \
			_z = &_p->val; \
			if (indirect && C_TYPE_P(_z) == IS_INDIRECT) { \
				_z = C_INDIRECT_P(_z); \
			} \
			if (UNEXPECTED(C_TYPE_P(_z) == IS_UNDEF)) continue;

#define CREX_HASH_MAP_FOREACH_END_DEL() \
			CREX_ASSERT(!HT_IS_PACKED(__ht)); \
			__ht->nNumOfElements--; \
			do { \
				uint32_t j = HT_IDX_TO_HASH(_idx - 1); \
				uint32_t nIndex = _p->h | __ht->nTableMask; \
				uint32_t i = HT_HASH(__ht, nIndex); \
				if (UNEXPECTED(j != i)) { \
					Bucket *prev = HT_HASH_TO_BUCKET(__ht, i); \
					while (C_NEXT(prev->val) != j) { \
						i = C_NEXT(prev->val); \
						prev = HT_HASH_TO_BUCKET(__ht, i); \
					} \
					C_NEXT(prev->val) = C_NEXT(_p->val); \
				} else { \
					HT_HASH(__ht, nIndex) = C_NEXT(_p->val); \
				} \
			} while (0); \
		} \
		__ht->nNumUsed = _idx; \
	} while (0)

#define CREX_HASH_MAP_FOREACH_BUCKET(ht, _bucket) \
	CREX_HASH_MAP_FOREACH(ht, 0); \
	_bucket = _p;

#define CREX_HASH_MAP_FOREACH_BUCKET_FROM(ht, _bucket, _from) \
	CREX_HASH_MAP_FOREACH_FROM(ht, 0, _from); \
	_bucket = _p;

#define CREX_HASH_MAP_REVERSE_FOREACH_BUCKET(ht, _bucket) \
	CREX_HASH_MAP_REVERSE_FOREACH(ht, 0); \
	_bucket = _p;

#define CREX_HASH_MAP_FOREACH_VAL(ht, _val) \
	CREX_HASH_MAP_FOREACH(ht, 0); \
	_val = _z;

#define CREX_HASH_MAP_REVERSE_FOREACH_VAL(ht, _val) \
	CREX_HASH_MAP_REVERSE_FOREACH(ht, 0); \
	_val = _z;

#define CREX_HASH_MAP_FOREACH_VAL_IND(ht, _val) \
	CREX_HASH_MAP_FOREACH(ht, 1); \
	_val = _z;

#define CREX_HASH_MAP_REVERSE_FOREACH_VAL_IND(ht, _val) \
	CREX_HASH_MAP_REVERSE_FOREACH(ht, 1); \
	_val = _z;

#define CREX_HASH_MAP_FOREACH_PTR(ht, _ptr) \
	CREX_HASH_MAP_FOREACH(ht, 0); \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_MAP_FOREACH_PTR_FROM(ht, _ptr, _from) \
	CREX_HASH_MAP_FOREACH_FROM(ht, 0, _from); \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_MAP_REVERSE_FOREACH_PTR(ht, _ptr) \
	CREX_HASH_MAP_REVERSE_FOREACH(ht, 0); \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_MAP_FOREACH_NUM_KEY(ht, _h) \
	CREX_HASH_MAP_FOREACH(ht, 0); \
	_h = _p->h;

#define CREX_HASH_MAP_REVERSE_FOREACH_NUM_KEY(ht, _h) \
	CREX_HASH_MAP_REVERSE_FOREACH(ht, 0); \
	_h = _p->h;

#define CREX_HASH_MAP_FOREACH_STR_KEY(ht, _key) \
	CREX_HASH_MAP_FOREACH(ht, 0); \
	_key = _p->key;

#define CREX_HASH_MAP_REVERSE_FOREACH_STR_KEY(ht, _key) \
	CREX_HASH_MAP_REVERSE_FOREACH(ht, 0); \
	_key = _p->key;

#define CREX_HASH_MAP_FOREACH_KEY(ht, _h, _key) \
	CREX_HASH_MAP_FOREACH(ht, 0); \
	_h = _p->h; \
	_key = _p->key;

#define CREX_HASH_MAP_REVERSE_FOREACH_KEY(ht, _h, _key) \
	CREX_HASH_MAP_REVERSE_FOREACH(ht, 0); \
	_h = _p->h; \
	_key = _p->key;

#define CREX_HASH_MAP_FOREACH_NUM_KEY_VAL(ht, _h, _val) \
	CREX_HASH_MAP_FOREACH(ht, 0); \
	_h = _p->h; \
	_val = _z;

#define CREX_HASH_MAP_REVERSE_FOREACH_NUM_KEY_VAL(ht, _h, _val) \
	CREX_HASH_MAP_REVERSE_FOREACH(ht, 0); \
	_h = _p->h; \
	_val = _z;

#define CREX_HASH_MAP_FOREACH_STR_KEY_VAL(ht, _key, _val) \
	CREX_HASH_MAP_FOREACH(ht, 0); \
	_key = _p->key; \
	_val = _z;

#define CREX_HASH_MAP_FOREACH_STR_KEY_VAL_FROM(ht, _key, _val, _from) \
	CREX_HASH_MAP_FOREACH_FROM(ht, 0, _from); \
	_key = _p->key; \
	_val = _z;

#define CREX_HASH_MAP_REVERSE_FOREACH_STR_KEY_VAL(ht, _key, _val) \
	CREX_HASH_MAP_REVERSE_FOREACH(ht, 0); \
	_key = _p->key; \
	_val = _z;

#define CREX_HASH_MAP_FOREACH_KEY_VAL(ht, _h, _key, _val) \
	CREX_HASH_MAP_FOREACH(ht, 0); \
	_h = _p->h; \
	_key = _p->key; \
	_val = _z;

#define CREX_HASH_MAP_REVERSE_FOREACH_KEY_VAL(ht, _h, _key, _val) \
	CREX_HASH_MAP_REVERSE_FOREACH(ht, 0); \
	_h = _p->h; \
	_key = _p->key; \
	_val = _z;

#define CREX_HASH_MAP_FOREACH_STR_KEY_VAL_IND(ht, _key, _val) \
	CREX_HASH_MAP_FOREACH(ht, 1); \
	_key = _p->key; \
	_val = _z;

#define CREX_HASH_MAP_REVERSE_FOREACH_STR_KEY_VAL_IND(ht, _key, _val) \
	CREX_HASH_MAP_REVERSE_FOREACH(ht, 1); \
	_key = _p->key; \
	_val = _z;

#define CREX_HASH_MAP_FOREACH_KEY_VAL_IND(ht, _h, _key, _val) \
	CREX_HASH_MAP_FOREACH(ht, 1); \
	_h = _p->h; \
	_key = _p->key; \
	_val = _z;

#define CREX_HASH_MAP_REVERSE_FOREACH_KEY_VAL_IND(ht, _h, _key, _val) \
	CREX_HASH_MAP_REVERSE_FOREACH(ht, 1); \
	_h = _p->h; \
	_key = _p->key; \
	_val = _z;

#define CREX_HASH_MAP_FOREACH_NUM_KEY_PTR(ht, _h, _ptr) \
	CREX_HASH_MAP_FOREACH(ht, 0); \
	_h = _p->h; \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_MAP_REVERSE_FOREACH_NUM_KEY_PTR(ht, _h, _ptr) \
	CREX_HASH_MAP_REVERSE_FOREACH(ht, 0); \
	_h = _p->h; \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_MAP_FOREACH_STR_KEY_PTR(ht, _key, _ptr) \
	CREX_HASH_MAP_FOREACH(ht, 0); \
	_key = _p->key; \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_MAP_REVERSE_FOREACH_STR_KEY_PTR(ht, _key, _ptr) \
	CREX_HASH_MAP_REVERSE_FOREACH(ht, 0); \
	_key = _p->key; \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_MAP_FOREACH_KEY_PTR(ht, _h, _key, _ptr) \
	CREX_HASH_MAP_FOREACH(ht, 0); \
	_h = _p->h; \
	_key = _p->key; \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_MAP_REVERSE_FOREACH_KEY_PTR(ht, _h, _key, _ptr) \
	CREX_HASH_MAP_REVERSE_FOREACH(ht, 0); \
	_h = _p->h; \
	_key = _p->key; \
	_ptr = C_PTR_P(_z);

/* Packed array iterators */
#define CREX_HASH_PACKED_FOREACH_FROM(_ht, _from) do { \
		HashTable *__ht = (_ht); \
		crex_ulong _idx = (_from); \
		zval *_z = __ht->arPacked + (_from); \
		zval *_end = __ht->arPacked + __ht->nNumUsed; \
		CREX_ASSERT(HT_IS_PACKED(__ht)); \
		for (;_z != _end; _z++, _idx++) { \
			(void) _idx; \
			if (UNEXPECTED(C_TYPE_P(_z) == IS_UNDEF)) continue;

#define CREX_HASH_PACKED_FOREACH(_ht) CREX_HASH_PACKED_FOREACH_FROM(_ht, 0)

#define CREX_HASH_PACKED_REVERSE_FOREACH(_ht) do { \
		HashTable *__ht = (_ht); \
		crex_ulong _idx = __ht->nNumUsed; \
		zval *_z = __ht->arPacked + _idx; \
		CREX_ASSERT(HT_IS_PACKED(__ht)); \
		while (_idx > 0) { \
			_z--; \
			_idx--; \
			(void) _idx; \
			if (UNEXPECTED(C_TYPE_P(_z) == IS_UNDEF)) continue;

#define CREX_HASH_PACKED_FOREACH_VAL(ht, _val) \
	CREX_HASH_PACKED_FOREACH(ht); \
	_val = _z;

#define CREX_HASH_PACKED_REVERSE_FOREACH_VAL(ht, _val) \
	CREX_HASH_PACKED_REVERSE_FOREACH(ht); \
	_val = _z;

#define CREX_HASH_PACKED_FOREACH_PTR(ht, _ptr) \
	CREX_HASH_PACKED_FOREACH(ht); \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_PACKED_REVERSE_FOREACH_PTR(ht, _ptr) \
	CREX_HASH_PACKED_REVERSE_FOREACH(ht); \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_PACKED_FOREACH_KEY(ht, _h) \
	CREX_HASH_PACKED_FOREACH(ht); \
	_h = _idx;

#define CREX_HASH_PACKED_REVERSE_FOREACH_KEY(ht, _h) \
	CREX_HASH_PACKED_REVERSE_FOREACH(ht); \
	_h = _idx;

#define CREX_HASH_PACKED_FOREACH_KEY_VAL(ht, _h, _val) \
	CREX_HASH_PACKED_FOREACH(ht); \
	_h = _idx; \
	_val = _z;

#define CREX_HASH_PACKED_REVERSE_FOREACH_KEY_VAL(ht, _h, _val) \
	CREX_HASH_PACKED_REVERSE_FOREACH(ht); \
	_h = _idx; \
	_val = _z;

#define CREX_HASH_PACKED_FOREACH_KEY_PTR(ht, _h, _ptr) \
	CREX_HASH_PACKED_FOREACH(ht); \
	_h = _idx; \
	_ptr = C_PTR_P(_z);

#define CREX_HASH_PACKED_REVERSE_FOREACH_KEY_PTR(ht, _h, _ptr) \
	CREX_HASH_PACKED_REVERSE_FOREACH(ht); \
	_h = _idx; \
	_ptr = C_PTR_P(_z);

/* The following macros are useful to insert a sequence of new elements
 * of packed array. They may be used instead of series of
 * crex_hash_next_index_insert_new()
 * (HashTable must have enough free buckets).
 */
#define CREX_HASH_FILL_PACKED(ht) do { \
		HashTable *__fill_ht = (ht); \
		zval *__fill_val = __fill_ht->arPacked + __fill_ht->nNumUsed; \
		uint32_t __fill_idx = __fill_ht->nNumUsed; \
		CREX_ASSERT(HT_IS_PACKED(__fill_ht));

#define CREX_HASH_FILL_GROW() do { \
		if (UNEXPECTED(__fill_idx >= __fill_ht->nTableSize)) { \
			__fill_ht->nNumOfElements += __fill_idx - __fill_ht->nNumUsed; \
			__fill_ht->nNumUsed = __fill_idx; \
			__fill_ht->nNextFreeElement = __fill_idx; \
			crex_hash_packed_grow(__fill_ht); \
			__fill_val = __fill_ht->arPacked + __fill_idx; \
		} \
	} while (0);

#define CREX_HASH_FILL_SET(_val) \
		ZVAL_COPY_VALUE(__fill_val, _val)

#define CREX_HASH_FILL_SET_NULL() \
		ZVAL_NULL(__fill_val)

#define CREX_HASH_FILL_SET_LONG(_val) \
		ZVAL_LONG(__fill_val, _val)

#define CREX_HASH_FILL_SET_DOUBLE(_val) \
		ZVAL_DOUBLE(__fill_val, _val)

#define CREX_HASH_FILL_SET_STR(_val) \
		ZVAL_STR(__fill_val, _val)

#define CREX_HASH_FILL_SET_STR_COPY(_val) \
		ZVAL_STR_COPY(__fill_val, _val)

#define CREX_HASH_FILL_SET_INTERNED_STR(_val) \
		ZVAL_INTERNED_STR(__fill_val, _val)

#define CREX_HASH_FILL_NEXT() do {\
		__fill_val++; \
		__fill_idx++; \
	} while (0)

#define CREX_HASH_FILL_ADD(_val) do { \
		CREX_HASH_FILL_SET(_val); \
		CREX_HASH_FILL_NEXT(); \
	} while (0)

#define CREX_HASH_FILL_FINISH() do { \
		__fill_ht->nNumOfElements += __fill_idx - __fill_ht->nNumUsed; \
		__fill_ht->nNumUsed = __fill_idx; \
		__fill_ht->nNextFreeElement = __fill_idx; \
		__fill_ht->nInternalPointer = 0; \
	} while (0)

#define CREX_HASH_FILL_END() \
		CREX_HASH_FILL_FINISH(); \
	} while (0)

/* Check if an array is a list */
static crex_always_inline bool crex_array_is_list(crex_array *array)
{
	crex_ulong expected_idx = 0;
	crex_ulong num_idx;
	crex_string* str_idx;
	/* Empty arrays are lists */
	if (crex_hash_num_elements(array) == 0) {
		return 1;
	}

	/* Packed arrays are lists */
	if (HT_IS_PACKED(array)) {
		if (HT_IS_WITHOUT_HOLES(array)) {
			return 1;
		}
		/* Check if the list could theoretically be repacked */
		CREX_HASH_PACKED_FOREACH_KEY(array, num_idx) {
			if (num_idx != expected_idx++) {
				return 0;
			}
		} CREX_HASH_FOREACH_END();
	} else {
		/* Check if the list could theoretically be repacked */
		CREX_HASH_MAP_FOREACH_KEY(array, num_idx, str_idx) {
			if (str_idx != NULL || num_idx != expected_idx++) {
				return 0;
			}
		} CREX_HASH_FOREACH_END();
	}

	return 1;
}


static crex_always_inline zval *_crex_hash_append_ex(HashTable *ht, crex_string *key, zval *zv, bool interned)
{
	uint32_t idx = ht->nNumUsed++;
	uint32_t nIndex;
	Bucket *p = ht->arData + idx;

	ZVAL_COPY_VALUE(&p->val, zv);
	if (!interned && !ZSTR_IS_INTERNED(key)) {
		HT_FLAGS(ht) &= ~HASH_FLAG_STATIC_KEYS;
		crex_string_addref(key);
		crex_string_hash_val(key);
	}
	p->key = key;
	p->h = ZSTR_H(key);
	nIndex = (uint32_t)p->h | ht->nTableMask;
	C_NEXT(p->val) = HT_HASH(ht, nIndex);
	HT_HASH(ht, nIndex) = HT_IDX_TO_HASH(idx);
	ht->nNumOfElements++;
	return &p->val;
}

static crex_always_inline zval *_crex_hash_append(HashTable *ht, crex_string *key, zval *zv)
{
	return _crex_hash_append_ex(ht, key, zv, 0);
}

static crex_always_inline zval *_crex_hash_append_ptr_ex(HashTable *ht, crex_string *key, void *ptr, bool interned)
{
	uint32_t idx = ht->nNumUsed++;
	uint32_t nIndex;
	Bucket *p = ht->arData + idx;

	ZVAL_PTR(&p->val, ptr);
	if (!interned && !ZSTR_IS_INTERNED(key)) {
		HT_FLAGS(ht) &= ~HASH_FLAG_STATIC_KEYS;
		crex_string_addref(key);
		crex_string_hash_val(key);
	}
	p->key = key;
	p->h = ZSTR_H(key);
	nIndex = (uint32_t)p->h | ht->nTableMask;
	C_NEXT(p->val) = HT_HASH(ht, nIndex);
	HT_HASH(ht, nIndex) = HT_IDX_TO_HASH(idx);
	ht->nNumOfElements++;
	return &p->val;
}

static crex_always_inline zval *_crex_hash_append_ptr(HashTable *ht, crex_string *key, void *ptr)
{
	return _crex_hash_append_ptr_ex(ht, key, ptr, 0);
}

static crex_always_inline void _crex_hash_append_ind(HashTable *ht, crex_string *key, zval *ptr)
{
	uint32_t idx = ht->nNumUsed++;
	uint32_t nIndex;
	Bucket *p = ht->arData + idx;

	ZVAL_INDIRECT(&p->val, ptr);
	if (!ZSTR_IS_INTERNED(key)) {
		HT_FLAGS(ht) &= ~HASH_FLAG_STATIC_KEYS;
		crex_string_addref(key);
		crex_string_hash_val(key);
	}
	p->key = key;
	p->h = ZSTR_H(key);
	nIndex = (uint32_t)p->h | ht->nTableMask;
	C_NEXT(p->val) = HT_HASH(ht, nIndex);
	HT_HASH(ht, nIndex) = HT_IDX_TO_HASH(idx);
	ht->nNumOfElements++;
}

#endif							/* CREX_HASH_H */
