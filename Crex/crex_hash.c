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

#include "crex.h"
#include "crex_globals.h"
#include "crex_variables.h"

#if defined(__aarch64__) || defined(_M_ARM64)
# include <arm_neon.h>
#endif

/* Prefer to use AVX2 instructions for better latency and throughput */
#if defined(__AVX2__)
# include <immintrin.h>
#elif defined( __SSE2__)
# include <mmintrin.h>
# include <emmintrin.h>
#endif

#if CREX_DEBUG
# define HT_ASSERT(ht, expr) \
	CREX_ASSERT((expr) || (HT_FLAGS(ht) & HASH_FLAG_ALLOW_COW_VIOLATION))
#else
# define HT_ASSERT(ht, expr)
#endif

#define HT_ASSERT_RC1(ht) HT_ASSERT(ht, GC_REFCOUNT(ht) == 1)

#define HT_POISONED_PTR ((HashTable *) (intptr_t) -1)

#if CREX_DEBUG

#define HT_OK					0x00
#define HT_IS_DESTROYING		0x01
#define HT_DESTROYED			0x02
#define HT_CLEANING				0x03

static void _crex_is_inconsistent(const HashTable *ht, const char *file, int line)
{
	if ((HT_FLAGS(ht) & HASH_FLAG_CONSISTENCY) == HT_OK) {
		return;
	}
	switch (HT_FLAGS(ht) & HASH_FLAG_CONSISTENCY) {
		case HT_IS_DESTROYING:
			crex_output_debug_string(1, "%s(%d) : ht=%p is being destroyed", file, line, ht);
			break;
		case HT_DESTROYED:
			crex_output_debug_string(1, "%s(%d) : ht=%p is already destroyed", file, line, ht);
			break;
		case HT_CLEANING:
			crex_output_debug_string(1, "%s(%d) : ht=%p is being cleaned", file, line, ht);
			break;
		default:
			crex_output_debug_string(1, "%s(%d) : ht=%p is inconsistent", file, line, ht);
			break;
	}
	CREX_UNREACHABLE();
}
#define IS_CONSISTENT(a) _crex_is_inconsistent(a, __FILE__, __LINE__);
#define SET_INCONSISTENT(n) do { \
		HT_FLAGS(ht) = (HT_FLAGS(ht) & ~HASH_FLAG_CONSISTENCY) | (n); \
	} while (0)
#else
#define IS_CONSISTENT(a)
#define SET_INCONSISTENT(n)
#endif

#define CREX_HASH_IF_FULL_DO_RESIZE(ht)				\
	if ((ht)->nNumUsed >= (ht)->nTableSize) {		\
		crex_hash_do_resize(ht);					\
	}

CREX_API void *crex_hash_str_find_ptr_lc(const HashTable *ht, const char *str, size_t len) {
	void *result;
	char *lc_str;

	/* Stack allocate small strings to improve performance */
	ALLOCA_FLAG(use_heap)

	lc_str = crex_str_tolower_copy(do_alloca(len + 1, use_heap), str, len);
	result = crex_hash_str_find_ptr(ht, lc_str, len);
	free_alloca(lc_str, use_heap);

	return result;
}

CREX_API void *crex_hash_find_ptr_lc(const HashTable *ht, crex_string *key) {
	void *result;
	crex_string *lc_key = crex_string_tolower(key);
	result = crex_hash_find_ptr(ht, lc_key);
	crex_string_release(lc_key);
	return result;
}

static void CREX_FASTCALL crex_hash_do_resize(HashTable *ht);

static crex_always_inline uint32_t crex_hash_check_size(uint32_t nSize)
{
#ifdef CREX_WIN32
	unsigned long index;
#endif

	/* Use big enough power of 2 */
	/* size should be between HT_MIN_SIZE and HT_MAX_SIZE */
	if (nSize <= HT_MIN_SIZE) {
		return HT_MIN_SIZE;
	} else if (UNEXPECTED(nSize > HT_MAX_SIZE)) {
		crex_error_noreturn(E_ERROR, "Possible integer overflow in memory allocation (%u * %zu + %zu)", nSize, sizeof(Bucket), sizeof(Bucket));
	}

#ifdef CREX_WIN32
	if (BitScanReverse(&index, nSize - 1)) {
		return 0x2u << ((31 - index) ^ 0x1f);
	} else {
		/* nSize is ensured to be in the valid range, fall back to it
		   rather than using an undefined bis scan result. */
		return nSize;
	}
#elif (defined(__GNUC__) || __has_builtin(__builtin_clz))  && defined(CRX_HAVE_BUILTIN_CLZ)
	return 0x2u << (__builtin_clz(nSize - 1) ^ 0x1f);
#else
	nSize -= 1;
	nSize |= (nSize >> 1);
	nSize |= (nSize >> 2);
	nSize |= (nSize >> 4);
	nSize |= (nSize >> 8);
	nSize |= (nSize >> 16);
	return nSize + 1;
#endif
}

static crex_always_inline void crex_hash_real_init_packed_ex(HashTable *ht)
{
	void *data;

	if (UNEXPECTED(GC_FLAGS(ht) & IS_ARRAY_PERSISTENT)) {
		data = pemalloc(HT_PACKED_SIZE_EX(ht->nTableSize, HT_MIN_MASK), 1);
	} else if (EXPECTED(ht->nTableSize == HT_MIN_SIZE)) {
		/* Use specialized API with constant allocation amount for a particularly common case. */
		data = emalloc(HT_PACKED_SIZE_EX(HT_MIN_SIZE, HT_MIN_MASK));
	} else {
		data = emalloc(HT_PACKED_SIZE_EX(ht->nTableSize, HT_MIN_MASK));
	}
	HT_SET_DATA_ADDR(ht, data);
	/* Don't overwrite iterator count. */
	ht->u.v.flags = HASH_FLAG_PACKED | HASH_FLAG_STATIC_KEYS;
	HT_HASH_RESET_PACKED(ht);
}

static crex_always_inline void crex_hash_real_init_mixed_ex(HashTable *ht)
{
	void *data;
	uint32_t nSize = ht->nTableSize;

	CREX_ASSERT(HT_SIZE_TO_MASK(nSize));

	if (UNEXPECTED(GC_FLAGS(ht) & IS_ARRAY_PERSISTENT)) {
		data = pemalloc(HT_SIZE_EX(nSize, HT_SIZE_TO_MASK(nSize)), 1);
	} else if (EXPECTED(nSize == HT_MIN_SIZE)) {
		data = emalloc(HT_SIZE_EX(HT_MIN_SIZE, HT_SIZE_TO_MASK(HT_MIN_SIZE)));
		ht->nTableMask = HT_SIZE_TO_MASK(HT_MIN_SIZE);
		HT_SET_DATA_ADDR(ht, data);
		/* Don't overwrite iterator count. */
		ht->u.v.flags = HASH_FLAG_STATIC_KEYS;
#if defined(__AVX2__)
		do {
			__m256i ymm0 = _mm256_setzero_si256();
			ymm0 = _mm256_cmpeq_epi64(ymm0, ymm0);
			_mm256_storeu_si256((__m256i*)&HT_HASH_EX(data,  0), ymm0);
			_mm256_storeu_si256((__m256i*)&HT_HASH_EX(data,  8), ymm0);
		} while(0);
#elif defined (__SSE2__)
		do {
			__m128i xmm0 = _mm_setzero_si128();
			xmm0 = _mm_cmpeq_epi8(xmm0, xmm0);
			_mm_storeu_si128((__m128i*)&HT_HASH_EX(data,  0), xmm0);
			_mm_storeu_si128((__m128i*)&HT_HASH_EX(data,  4), xmm0);
			_mm_storeu_si128((__m128i*)&HT_HASH_EX(data,  8), xmm0);
			_mm_storeu_si128((__m128i*)&HT_HASH_EX(data, 12), xmm0);
		} while (0);
#elif defined(__aarch64__) || defined(_M_ARM64)
		do {
			int32x4_t t = vdupq_n_s32(-1);
			vst1q_s32((int32_t*)&HT_HASH_EX(data,  0), t);
			vst1q_s32((int32_t*)&HT_HASH_EX(data,  4), t);
			vst1q_s32((int32_t*)&HT_HASH_EX(data,  8), t);
			vst1q_s32((int32_t*)&HT_HASH_EX(data, 12), t);
		} while (0);
#else
		HT_HASH_EX(data,  0) = -1;
		HT_HASH_EX(data,  1) = -1;
		HT_HASH_EX(data,  2) = -1;
		HT_HASH_EX(data,  3) = -1;
		HT_HASH_EX(data,  4) = -1;
		HT_HASH_EX(data,  5) = -1;
		HT_HASH_EX(data,  6) = -1;
		HT_HASH_EX(data,  7) = -1;
		HT_HASH_EX(data,  8) = -1;
		HT_HASH_EX(data,  9) = -1;
		HT_HASH_EX(data, 10) = -1;
		HT_HASH_EX(data, 11) = -1;
		HT_HASH_EX(data, 12) = -1;
		HT_HASH_EX(data, 13) = -1;
		HT_HASH_EX(data, 14) = -1;
		HT_HASH_EX(data, 15) = -1;
#endif
		return;
	} else {
		data = emalloc(HT_SIZE_EX(nSize, HT_SIZE_TO_MASK(nSize)));
	}
	ht->nTableMask = HT_SIZE_TO_MASK(nSize);
	HT_SET_DATA_ADDR(ht, data);
	HT_FLAGS(ht) = HASH_FLAG_STATIC_KEYS;
	HT_HASH_RESET(ht);
}

static crex_always_inline void crex_hash_real_init_ex(HashTable *ht, bool packed)
{
	HT_ASSERT_RC1(ht);
	CREX_ASSERT(HT_FLAGS(ht) & HASH_FLAG_UNINITIALIZED);
	if (packed) {
		crex_hash_real_init_packed_ex(ht);
	} else {
		crex_hash_real_init_mixed_ex(ht);
	}
}

static const uint32_t uninitialized_bucket[-HT_MIN_MASK] =
	{HT_INVALID_IDX, HT_INVALID_IDX};

CREX_API const HashTable crex_empty_array = {
	.gc.refcount = 2,
	.gc.u.type_info = IS_ARRAY | (GC_IMMUTABLE << GC_FLAGS_SHIFT),
	.u.flags = HASH_FLAG_UNINITIALIZED,
	.nTableMask = HT_MIN_MASK,
	{.arData = (Bucket*)&uninitialized_bucket[2]},
	.nNumUsed = 0,
	.nNumOfElements = 0,
	.nTableSize = HT_MIN_SIZE,
	.nInternalPointer = 0,
	.nNextFreeElement = CREX_LONG_MIN,
	.pDestructor = ZVAL_PTR_DTOR
};

static crex_always_inline void _crex_hash_init_int(HashTable *ht, uint32_t nSize, dtor_func_t pDestructor, bool persistent)
{
	GC_SET_REFCOUNT(ht, 1);
	GC_TYPE_INFO(ht) = GC_ARRAY | (persistent ? ((GC_PERSISTENT|GC_NOT_COLLECTABLE) << GC_FLAGS_SHIFT) : 0);
	HT_FLAGS(ht) = HASH_FLAG_UNINITIALIZED;
	ht->nTableMask = HT_MIN_MASK;
	HT_SET_DATA_ADDR(ht, &uninitialized_bucket);
	ht->nNumUsed = 0;
	ht->nNumOfElements = 0;
	ht->nInternalPointer = 0;
	ht->nNextFreeElement = CREX_LONG_MIN;
	ht->pDestructor = pDestructor;
	ht->nTableSize = crex_hash_check_size(nSize);
}

CREX_API void CREX_FASTCALL _crex_hash_init(HashTable *ht, uint32_t nSize, dtor_func_t pDestructor, bool persistent)
{
	_crex_hash_init_int(ht, nSize, pDestructor, persistent);
}

CREX_API HashTable* CREX_FASTCALL _crex_new_array_0(void)
{
	HashTable *ht = emalloc(sizeof(HashTable));
	_crex_hash_init_int(ht, HT_MIN_SIZE, ZVAL_PTR_DTOR, 0);
	return ht;
}

CREX_API HashTable* CREX_FASTCALL _crex_new_array(uint32_t nSize)
{
	HashTable *ht = emalloc(sizeof(HashTable));
	_crex_hash_init_int(ht, nSize, ZVAL_PTR_DTOR, 0);
	return ht;
}

CREX_API HashTable* CREX_FASTCALL crex_new_pair(zval *val1, zval *val2)
{
	zval *zv;
	HashTable *ht = emalloc(sizeof(HashTable));
	_crex_hash_init_int(ht, HT_MIN_SIZE, ZVAL_PTR_DTOR, 0);
	ht->nNumUsed = ht->nNumOfElements = ht->nNextFreeElement = 2;
	crex_hash_real_init_packed_ex(ht);

	zv = ht->arPacked;
	ZVAL_COPY_VALUE(zv, val1);
	zv++;
	ZVAL_COPY_VALUE(zv, val2);
	return ht;
}

CREX_API void CREX_FASTCALL crex_hash_packed_grow(HashTable *ht)
{
	HT_ASSERT_RC1(ht);
	if (ht->nTableSize >= HT_MAX_SIZE) {
		crex_error_noreturn(E_ERROR, "Possible integer overflow in memory allocation (%u * %zu + %zu)", ht->nTableSize * 2, sizeof(Bucket), sizeof(Bucket));
	}
	uint32_t newTableSize = ht->nTableSize * 2;
	HT_SET_DATA_ADDR(ht, perealloc2(HT_GET_DATA_ADDR(ht), HT_PACKED_SIZE_EX(newTableSize, HT_MIN_MASK), HT_PACKED_USED_SIZE(ht), GC_FLAGS(ht) & IS_ARRAY_PERSISTENT));
	ht->nTableSize = newTableSize;
}

CREX_API void CREX_FASTCALL crex_hash_real_init(HashTable *ht, bool packed)
{
	IS_CONSISTENT(ht);

	HT_ASSERT_RC1(ht);
	crex_hash_real_init_ex(ht, packed);
}

CREX_API void CREX_FASTCALL crex_hash_real_init_packed(HashTable *ht)
{
	IS_CONSISTENT(ht);

	HT_ASSERT_RC1(ht);
	crex_hash_real_init_packed_ex(ht);
}

CREX_API void CREX_FASTCALL crex_hash_real_init_mixed(HashTable *ht)
{
	IS_CONSISTENT(ht);

	HT_ASSERT_RC1(ht);
	crex_hash_real_init_mixed_ex(ht);
}

CREX_API void CREX_FASTCALL crex_hash_packed_to_hash(HashTable *ht)
{
	void *new_data, *old_data = HT_GET_DATA_ADDR(ht);
	zval *src = ht->arPacked;
	Bucket *dst;
	uint32_t i;
	uint32_t nSize = ht->nTableSize;

	CREX_ASSERT(HT_SIZE_TO_MASK(nSize));

	HT_ASSERT_RC1(ht);
	// Alloc before assign to avoid inconsistencies on OOM
	new_data = pemalloc(HT_SIZE_EX(nSize, HT_SIZE_TO_MASK(nSize)), GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
	HT_FLAGS(ht) &= ~HASH_FLAG_PACKED;
	ht->nTableMask = HT_SIZE_TO_MASK(ht->nTableSize);
	HT_SET_DATA_ADDR(ht, new_data);
	dst = ht->arData;
	for (i = 0; i < ht->nNumUsed; i++) {
		ZVAL_COPY_VALUE(&dst->val, src);
		dst->h = i;
		dst->key = NULL;
		dst++;
		src++;
	}
	pefree(old_data, GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
	crex_hash_rehash(ht);
}

CREX_API void CREX_FASTCALL crex_hash_to_packed(HashTable *ht)
{
	void *new_data, *old_data = HT_GET_DATA_ADDR(ht);
	Bucket *src = ht->arData;
	zval *dst;
	uint32_t i;

	HT_ASSERT_RC1(ht);
	new_data = pemalloc(HT_PACKED_SIZE_EX(ht->nTableSize, HT_MIN_MASK), GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
	HT_FLAGS(ht) |= HASH_FLAG_PACKED | HASH_FLAG_STATIC_KEYS;
	ht->nTableMask = HT_MIN_MASK;
	HT_SET_DATA_ADDR(ht, new_data);
	HT_HASH_RESET_PACKED(ht);
	dst = ht->arPacked;
	for (i = 0; i < ht->nNumUsed; i++) {
		ZVAL_COPY_VALUE(dst, &src->val);
		dst++;
		src++;
	}
	pefree(old_data, GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
}

CREX_API void CREX_FASTCALL crex_hash_extend(HashTable *ht, uint32_t nSize, bool packed)
{
	HT_ASSERT_RC1(ht);

	if (nSize == 0) return;

	CREX_ASSERT(HT_SIZE_TO_MASK(nSize));

	if (UNEXPECTED(HT_FLAGS(ht) & HASH_FLAG_UNINITIALIZED)) {
		if (nSize > ht->nTableSize) {
			ht->nTableSize = crex_hash_check_size(nSize);
		}
		crex_hash_real_init(ht, packed);
	} else {
		if (packed) {
			CREX_ASSERT(HT_IS_PACKED(ht));
			if (nSize > ht->nTableSize) {
				uint32_t newTableSize = crex_hash_check_size(nSize);
				HT_SET_DATA_ADDR(ht, perealloc2(HT_GET_DATA_ADDR(ht), HT_PACKED_SIZE_EX(newTableSize, HT_MIN_MASK), HT_PACKED_USED_SIZE(ht), GC_FLAGS(ht) & IS_ARRAY_PERSISTENT));
				ht->nTableSize = newTableSize;
			}
		} else {
			CREX_ASSERT(!HT_IS_PACKED(ht));
			if (nSize > ht->nTableSize) {
				void *new_data, *old_data = HT_GET_DATA_ADDR(ht);
				Bucket *old_buckets = ht->arData;
				nSize = crex_hash_check_size(nSize);
				new_data = pemalloc(HT_SIZE_EX(nSize, HT_SIZE_TO_MASK(nSize)), GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
				ht->nTableSize = nSize;
				ht->nTableMask = HT_SIZE_TO_MASK(ht->nTableSize);
				HT_SET_DATA_ADDR(ht, new_data);
				memcpy(ht->arData, old_buckets, sizeof(Bucket) * ht->nNumUsed);
				pefree(old_data, GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
				crex_hash_rehash(ht);
			}
		}
	}
}

CREX_API void CREX_FASTCALL crex_hash_discard(HashTable *ht, uint32_t nNumUsed)
{
	Bucket *p, *end, *arData;
	uint32_t nIndex;

	CREX_ASSERT(!HT_IS_PACKED(ht));
	arData = ht->arData;
	p = arData + ht->nNumUsed;
	end = arData + nNumUsed;
	ht->nNumUsed = nNumUsed;
	while (p != end) {
		p--;
		if (UNEXPECTED(C_TYPE(p->val) == IS_UNDEF)) continue;
		ht->nNumOfElements--;
		/* Collision pointers always directed from higher to lower buckets */
#if 0
		if (!(C_NEXT(p->val) == HT_INVALID_IDX || HT_HASH_TO_BUCKET_EX(arData, C_NEXT(p->val)) < p)) {
			abort();
		}
#endif
		nIndex = p->h | ht->nTableMask;
		HT_HASH_EX(arData, nIndex) = C_NEXT(p->val);
	}
}

static uint32_t crex_array_recalc_elements(HashTable *ht)
{
	zval *val;
	uint32_t num = ht->nNumOfElements;

	CREX_HASH_MAP_FOREACH_VAL(ht, val) {
		if (C_TYPE_P(val) == IS_INDIRECT) {
			if (UNEXPECTED(C_TYPE_P(C_INDIRECT_P(val)) == IS_UNDEF)) {
				num--;
			}
		}
	} CREX_HASH_FOREACH_END();
	return num;
}
/* }}} */

CREX_API uint32_t crex_array_count(HashTable *ht)
{
	uint32_t num;
	if (UNEXPECTED(HT_FLAGS(ht) & HASH_FLAG_HAS_EMPTY_IND)) {
		num = crex_array_recalc_elements(ht);
		if (UNEXPECTED(ht->nNumOfElements == num)) {
			HT_FLAGS(ht) &= ~HASH_FLAG_HAS_EMPTY_IND;
		}
	} else if (UNEXPECTED(ht == &EG(symbol_table))) {
		num = crex_array_recalc_elements(ht);
	} else {
		num = crex_hash_num_elements(ht);
	}
	return num;
}
/* }}} */

static crex_always_inline HashPosition _crex_hash_get_valid_pos(const HashTable *ht, HashPosition pos)
{
	if (HT_IS_PACKED(ht)) {
		while (pos < ht->nNumUsed && C_ISUNDEF(ht->arPacked[pos])) {
			pos++;
		}
	} else {
		while (pos < ht->nNumUsed && C_ISUNDEF(ht->arData[pos].val)) {
			pos++;
		}
	}
	return pos;
}

static crex_always_inline HashPosition _crex_hash_get_current_pos(const HashTable *ht)
{
	return _crex_hash_get_valid_pos(ht, ht->nInternalPointer);
}

CREX_API HashPosition CREX_FASTCALL crex_hash_get_current_pos(const HashTable *ht)
{
	return _crex_hash_get_current_pos(ht);
}

static void crex_hash_remove_iterator_copies(uint32_t idx) {
	HashTableIterator *iterators = EG(ht_iterators);

	HashTableIterator *iter = iterators + idx;
	uint32_t next_idx = iter->next_copy;
	while (next_idx != idx) {
		uint32_t cur_idx = next_idx;
		HashTableIterator *cur_iter = iterators + cur_idx;
		next_idx = cur_iter->next_copy;
		cur_iter->next_copy = cur_idx; // avoid recursion in crex_hash_iterator_del
		crex_hash_iterator_del(cur_idx);
	}
	iter->next_copy = idx;
}

CREX_API uint32_t CREX_FASTCALL crex_hash_iterator_add(HashTable *ht, HashPosition pos)
{
	HashTableIterator *iter = EG(ht_iterators);
	HashTableIterator *end  = iter + EG(ht_iterators_count);
	uint32_t idx;

	if (EXPECTED(!HT_ITERATORS_OVERFLOW(ht))) {
		HT_INC_ITERATORS_COUNT(ht);
	}
	while (iter != end) {
		if (iter->ht == NULL) {
			iter->ht = ht;
			iter->pos = pos;
			idx = iter - EG(ht_iterators);
			iter->next_copy = idx;
			if (idx + 1 > EG(ht_iterators_used)) {
				EG(ht_iterators_used) = idx + 1;
			}
			return idx;
		}
		iter++;
	}
	if (EG(ht_iterators) == EG(ht_iterators_slots)) {
		EG(ht_iterators) = emalloc(sizeof(HashTableIterator) * (EG(ht_iterators_count) + 8));
		memcpy(EG(ht_iterators), EG(ht_iterators_slots), sizeof(HashTableIterator) * EG(ht_iterators_count));
	} else {
		EG(ht_iterators) = erealloc(EG(ht_iterators), sizeof(HashTableIterator) * (EG(ht_iterators_count) + 8));
	}
	iter = EG(ht_iterators) + EG(ht_iterators_count);
	EG(ht_iterators_count) += 8;
	iter->ht = ht;
	iter->pos = pos;
	memset(iter + 1, 0, sizeof(HashTableIterator) * 7);
	idx = iter - EG(ht_iterators);
	iter->next_copy = idx;
	EG(ht_iterators_used) = idx + 1;
	return idx;
}

// To avoid losing track of the HashTable when separating arrays, we track all copies at once.
static crex_always_inline bool crex_hash_iterator_find_copy_pos(uint32_t idx, HashTable *ht) {
	HashTableIterator *iter = EG(ht_iterators) + idx;

	uint32_t next_idx = iter->next_copy;
	if (EXPECTED(next_idx != idx)) {
		HashTableIterator *copy_iter;
		while (next_idx != idx) {
			copy_iter = EG(ht_iterators) + next_idx;
			if (copy_iter->ht == ht) {
				// We have found the hashtable we are actually iterating over
				// Now clean any intermittent copies and replace the original index by the found one
				if (EXPECTED(iter->ht) && EXPECTED(iter->ht != HT_POISONED_PTR)
					&& EXPECTED(!HT_ITERATORS_OVERFLOW(iter->ht))) {
					HT_DEC_ITERATORS_COUNT(iter->ht);
				}
				if (EXPECTED(!HT_ITERATORS_OVERFLOW(ht))) {
					HT_INC_ITERATORS_COUNT(ht);
				}
				iter->ht = copy_iter->ht;
				iter->pos = copy_iter->pos;
				crex_hash_remove_iterator_copies(idx);
				return true;
			}
			next_idx = copy_iter->next_copy;
		}
		crex_hash_remove_iterator_copies(idx);
	}

	return false;
}

CREX_API HashPosition CREX_FASTCALL crex_hash_iterator_pos(uint32_t idx, HashTable *ht)
{
	HashTableIterator *iter = EG(ht_iterators) + idx;

	CREX_ASSERT(idx != (uint32_t)-1);
	if (UNEXPECTED(iter->ht != ht) && !crex_hash_iterator_find_copy_pos(idx, ht)) {
		if (EXPECTED(iter->ht) && EXPECTED(iter->ht != HT_POISONED_PTR)
				&& EXPECTED(!HT_ITERATORS_OVERFLOW(iter->ht))) {
			HT_DEC_ITERATORS_COUNT(iter->ht);
		}
		if (EXPECTED(!HT_ITERATORS_OVERFLOW(ht))) {
			HT_INC_ITERATORS_COUNT(ht);
		}
		iter->ht = ht;
		iter->pos = _crex_hash_get_current_pos(ht);
	}
	return iter->pos;
}

CREX_API HashPosition CREX_FASTCALL crex_hash_iterator_pos_ex(uint32_t idx, zval *array)
{
	HashTable *ht = C_ARRVAL_P(array);
	HashTableIterator *iter = EG(ht_iterators) + idx;

	CREX_ASSERT(idx != (uint32_t)-1);
	if (UNEXPECTED(iter->ht != ht) && !crex_hash_iterator_find_copy_pos(idx, ht)) {
		if (EXPECTED(iter->ht) && EXPECTED(iter->ht != HT_POISONED_PTR)
				&& EXPECTED(!HT_ITERATORS_OVERFLOW(ht))) {
			HT_DEC_ITERATORS_COUNT(iter->ht);
		}
		SEPARATE_ARRAY(array);
		ht = C_ARRVAL_P(array);
		if (EXPECTED(!HT_ITERATORS_OVERFLOW(ht))) {
			HT_INC_ITERATORS_COUNT(ht);
		}
		iter->ht = ht;
		iter->pos = _crex_hash_get_current_pos(ht);
	}
	return iter->pos;
}

CREX_API void CREX_FASTCALL crex_hash_iterator_del(uint32_t idx)
{
	HashTableIterator *iter = EG(ht_iterators) + idx;

	CREX_ASSERT(idx != (uint32_t)-1);

	if (EXPECTED(iter->ht) && EXPECTED(iter->ht != HT_POISONED_PTR)
			&& EXPECTED(!HT_ITERATORS_OVERFLOW(iter->ht))) {
		CREX_ASSERT(HT_ITERATORS_COUNT(iter->ht) != 0);
		HT_DEC_ITERATORS_COUNT(iter->ht);
	}
	iter->ht = NULL;

	if (UNEXPECTED(iter->next_copy != idx)) {
		crex_hash_remove_iterator_copies(idx);
	}

	if (idx == EG(ht_iterators_used) - 1) {
		while (idx > 0 && EG(ht_iterators)[idx - 1].ht == NULL) {
			idx--;
		}
		EG(ht_iterators_used) = idx;
	}
}

static crex_never_inline void CREX_FASTCALL _crex_hash_iterators_remove(HashTable *ht)
{
	HashTableIterator *iter = EG(ht_iterators);
	HashTableIterator *end  = iter + EG(ht_iterators_used);

	while (iter != end) {
		if (iter->ht == ht) {
			iter->ht = HT_POISONED_PTR;
		}
		iter++;
	}
}

static crex_always_inline void crex_hash_iterators_remove(HashTable *ht)
{
	if (UNEXPECTED(HT_HAS_ITERATORS(ht))) {
		_crex_hash_iterators_remove(ht);
	}
}

CREX_API HashPosition CREX_FASTCALL crex_hash_iterators_lower_pos(HashTable *ht, HashPosition start)
{
	HashTableIterator *iter = EG(ht_iterators);
	HashTableIterator *end  = iter + EG(ht_iterators_used);
	HashPosition res = ht->nNumUsed;

	while (iter != end) {
		if (iter->ht == ht) {
			if (iter->pos >= start && iter->pos < res) {
				res = iter->pos;
			}
		}
		iter++;
	}
	return res;
}

CREX_API void CREX_FASTCALL _crex_hash_iterators_update(HashTable *ht, HashPosition from, HashPosition to)
{
	HashTableIterator *iter = EG(ht_iterators);
	HashTableIterator *end  = iter + EG(ht_iterators_used);

	while (iter != end) {
		if (iter->ht == ht && iter->pos == from) {
			iter->pos = to;
		}
		iter++;
	}
}

CREX_API void CREX_FASTCALL crex_hash_iterators_advance(HashTable *ht, HashPosition step)
{
	HashTableIterator *iter = EG(ht_iterators);
	HashTableIterator *end  = iter + EG(ht_iterators_used);

	while (iter != end) {
		if (iter->ht == ht) {
			iter->pos += step;
		}
		iter++;
	}
}

/* Hash must be known and precomputed before */
static crex_always_inline Bucket *crex_hash_find_bucket(const HashTable *ht, const crex_string *key)
{
	uint32_t nIndex;
	uint32_t idx;
	Bucket *p, *arData;

	CREX_ASSERT(ZSTR_H(key) != 0 && "Hash must be known");

	arData = ht->arData;
	nIndex = ZSTR_H(key) | ht->nTableMask;
	idx = HT_HASH_EX(arData, nIndex);

	if (UNEXPECTED(idx == HT_INVALID_IDX)) {
		return NULL;
	}
	p = HT_HASH_TO_BUCKET_EX(arData, idx);
	if (EXPECTED(p->key == key)) { /* check for the same interned string */
		return p;
	}

	while (1) {
		if (p->h == ZSTR_H(key) &&
		    EXPECTED(p->key) &&
		    crex_string_equal_content(p->key, key)) {
			return p;
		}
		idx = C_NEXT(p->val);
		if (idx == HT_INVALID_IDX) {
			return NULL;
		}
		p = HT_HASH_TO_BUCKET_EX(arData, idx);
		if (p->key == key) { /* check for the same interned string */
			return p;
		}
	}
}

static crex_always_inline Bucket *crex_hash_str_find_bucket(const HashTable *ht, const char *str, size_t len, crex_ulong h)
{
	uint32_t nIndex;
	uint32_t idx;
	Bucket *p, *arData;

	arData = ht->arData;
	nIndex = h | ht->nTableMask;
	idx = HT_HASH_EX(arData, nIndex);
	while (idx != HT_INVALID_IDX) {
		CREX_ASSERT(idx < HT_IDX_TO_HASH(ht->nTableSize));
		p = HT_HASH_TO_BUCKET_EX(arData, idx);
		if ((p->h == h)
			 && p->key
			 && crex_string_equals_cstr(p->key, str, len)) {
			return p;
		}
		idx = C_NEXT(p->val);
	}
	return NULL;
}

static crex_always_inline Bucket *crex_hash_index_find_bucket(const HashTable *ht, crex_ulong h)
{
	uint32_t nIndex;
	uint32_t idx;
	Bucket *p, *arData;

	arData = ht->arData;
	nIndex = h | ht->nTableMask;
	idx = HT_HASH_EX(arData, nIndex);
	while (idx != HT_INVALID_IDX) {
		CREX_ASSERT(idx < HT_IDX_TO_HASH(ht->nTableSize));
		p = HT_HASH_TO_BUCKET_EX(arData, idx);
		if (p->h == h && !p->key) {
			return p;
		}
		idx = C_NEXT(p->val);
	}
	return NULL;
}

static crex_always_inline zval *_crex_hash_add_or_update_i(HashTable *ht, crex_string *key, zval *pData, uint32_t flag)
{
	crex_ulong h;
	uint32_t nIndex;
	uint32_t idx;
	Bucket *p, *arData;

	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);
	crex_string_hash_val(key);

	if (UNEXPECTED(HT_FLAGS(ht) & (HASH_FLAG_UNINITIALIZED|HASH_FLAG_PACKED))) {
		if (EXPECTED(HT_FLAGS(ht) & HASH_FLAG_UNINITIALIZED)) {
			crex_hash_real_init_mixed(ht);
			goto add_to_hash;
		} else {
			crex_hash_packed_to_hash(ht);
		}
	} else if ((flag & HASH_ADD_NEW) == 0 || CREX_DEBUG) {
		p = crex_hash_find_bucket(ht, key);

		if (p) {
			zval *data;

			CREX_ASSERT((flag & HASH_ADD_NEW) == 0);
			if (flag & HASH_LOOKUP) {
				return &p->val;
			} else if (flag & HASH_ADD) {
				if (!(flag & HASH_UPDATE_INDIRECT)) {
					return NULL;
				}
				CREX_ASSERT(&p->val != pData);
				data = &p->val;
				if (C_TYPE_P(data) == IS_INDIRECT) {
					data = C_INDIRECT_P(data);
					if (C_TYPE_P(data) != IS_UNDEF) {
						return NULL;
					}
				} else {
					return NULL;
				}
			} else {
				CREX_ASSERT(&p->val != pData);
				data = &p->val;
				if ((flag & HASH_UPDATE_INDIRECT) && C_TYPE_P(data) == IS_INDIRECT) {
					data = C_INDIRECT_P(data);
				}
			}
			if (ht->pDestructor) {
				ht->pDestructor(data);
			}
			ZVAL_COPY_VALUE(data, pData);
			return data;
		}
	}

	CREX_HASH_IF_FULL_DO_RESIZE(ht);		/* If the Hash table is full, resize it */

add_to_hash:
	if (!ZSTR_IS_INTERNED(key)) {
		crex_string_addref(key);
		HT_FLAGS(ht) &= ~HASH_FLAG_STATIC_KEYS;
	}
	idx = ht->nNumUsed++;
	ht->nNumOfElements++;
	arData = ht->arData;
	p = arData + idx;
	p->key = key;
	p->h = h = ZSTR_H(key);
	nIndex = h | ht->nTableMask;
	C_NEXT(p->val) = HT_HASH_EX(arData, nIndex);
	HT_HASH_EX(arData, nIndex) = HT_IDX_TO_HASH(idx);
	if (flag & HASH_LOOKUP) {
		ZVAL_NULL(&p->val);
	} else {
		ZVAL_COPY_VALUE(&p->val, pData);
	}

	return &p->val;
}

static crex_always_inline zval *_crex_hash_str_add_or_update_i(HashTable *ht, const char *str, size_t len, crex_ulong h, zval *pData, uint32_t flag)
{
	crex_string *key;
	uint32_t nIndex;
	uint32_t idx;
	Bucket *p;

	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);

	if (UNEXPECTED(HT_FLAGS(ht) & (HASH_FLAG_UNINITIALIZED|HASH_FLAG_PACKED))) {
		if (EXPECTED(HT_FLAGS(ht) & HASH_FLAG_UNINITIALIZED)) {
			crex_hash_real_init_mixed(ht);
			goto add_to_hash;
		} else {
			crex_hash_packed_to_hash(ht);
		}
	} else if ((flag & HASH_ADD_NEW) == 0) {
		p = crex_hash_str_find_bucket(ht, str, len, h);

		if (p) {
			zval *data;

			if (flag & HASH_LOOKUP) {
				return &p->val;
			} else if (flag & HASH_ADD) {
				if (!(flag & HASH_UPDATE_INDIRECT)) {
					return NULL;
				}
				CREX_ASSERT(&p->val != pData);
				data = &p->val;
				if (C_TYPE_P(data) == IS_INDIRECT) {
					data = C_INDIRECT_P(data);
					if (C_TYPE_P(data) != IS_UNDEF) {
						return NULL;
					}
				} else {
					return NULL;
				}
			} else {
				CREX_ASSERT(&p->val != pData);
				data = &p->val;
				if ((flag & HASH_UPDATE_INDIRECT) && C_TYPE_P(data) == IS_INDIRECT) {
					data = C_INDIRECT_P(data);
				}
			}
			if (ht->pDestructor) {
				ht->pDestructor(data);
			}
			ZVAL_COPY_VALUE(data, pData);
			return data;
		}
	}

	CREX_HASH_IF_FULL_DO_RESIZE(ht);		/* If the Hash table is full, resize it */

add_to_hash:
	idx = ht->nNumUsed++;
	ht->nNumOfElements++;
	p = ht->arData + idx;
	p->key = key = crex_string_init(str, len, GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
#if CREX_RC_DEBUG
	if (GC_FLAGS(ht) & GC_PERSISTENT_LOCAL) {
		GC_MAKE_PERSISTENT_LOCAL(key);
	}
#endif
	p->h = ZSTR_H(key) = h;
	HT_FLAGS(ht) &= ~HASH_FLAG_STATIC_KEYS;
	if (flag & HASH_LOOKUP) {
		ZVAL_NULL(&p->val);
	} else {
		ZVAL_COPY_VALUE(&p->val, pData);
	}
	nIndex = h | ht->nTableMask;
	C_NEXT(p->val) = HT_HASH(ht, nIndex);
	HT_HASH(ht, nIndex) = HT_IDX_TO_HASH(idx);

	return &p->val;
}

CREX_API zval* CREX_FASTCALL crex_hash_add_or_update(HashTable *ht, crex_string *key, zval *pData, uint32_t flag)
{
	if (flag == HASH_ADD) {
		return crex_hash_add(ht, key, pData);
	} else if (flag == HASH_ADD_NEW) {
		return crex_hash_add_new(ht, key, pData);
	} else if (flag == HASH_UPDATE) {
		return crex_hash_update(ht, key, pData);
	} else {
		CREX_ASSERT(flag == (HASH_UPDATE|HASH_UPDATE_INDIRECT));
		return crex_hash_update_ind(ht, key, pData);
	}
}

CREX_API zval* CREX_FASTCALL crex_hash_add(HashTable *ht, crex_string *key, zval *pData)
{
	return _crex_hash_add_or_update_i(ht, key, pData, HASH_ADD);
}

CREX_API zval* CREX_FASTCALL crex_hash_update(HashTable *ht, crex_string *key, zval *pData)
{
	return _crex_hash_add_or_update_i(ht, key, pData, HASH_UPDATE);
}

CREX_API zval* CREX_FASTCALL crex_hash_update_ind(HashTable *ht, crex_string *key, zval *pData)
{
	return _crex_hash_add_or_update_i(ht, key, pData, HASH_UPDATE | HASH_UPDATE_INDIRECT);
}

CREX_API zval* CREX_FASTCALL crex_hash_add_new(HashTable *ht, crex_string *key, zval *pData)
{
	return _crex_hash_add_or_update_i(ht, key, pData, HASH_ADD_NEW);
}

CREX_API zval* CREX_FASTCALL crex_hash_lookup(HashTable *ht, crex_string *key)
{
	return _crex_hash_add_or_update_i(ht, key, NULL, HASH_LOOKUP);
}

CREX_API zval* CREX_FASTCALL crex_hash_str_add_or_update(HashTable *ht, const char *str, size_t len, zval *pData, uint32_t flag)
{
	if (flag == HASH_ADD) {
		return crex_hash_str_add(ht, str, len, pData);
	} else if (flag == HASH_ADD_NEW) {
		return crex_hash_str_add_new(ht, str, len, pData);
	} else if (flag == HASH_UPDATE) {
		return crex_hash_str_update(ht, str, len, pData);
	} else {
		CREX_ASSERT(flag == (HASH_UPDATE|HASH_UPDATE_INDIRECT));
		return crex_hash_str_update_ind(ht, str, len, pData);
	}
}

CREX_API zval* CREX_FASTCALL crex_hash_str_update(HashTable *ht, const char *str, size_t len, zval *pData)
{
	crex_ulong h = crex_hash_func(str, len);

	return _crex_hash_str_add_or_update_i(ht, str, len, h, pData, HASH_UPDATE);
}

CREX_API zval* CREX_FASTCALL crex_hash_str_update_ind(HashTable *ht, const char *str, size_t len, zval *pData)
{
	crex_ulong h = crex_hash_func(str, len);

	return _crex_hash_str_add_or_update_i(ht, str, len, h, pData, HASH_UPDATE | HASH_UPDATE_INDIRECT);
}

CREX_API zval* CREX_FASTCALL crex_hash_str_add(HashTable *ht, const char *str, size_t len, zval *pData)
{
	crex_ulong h = crex_hash_func(str, len);

	return _crex_hash_str_add_or_update_i(ht, str, len, h, pData, HASH_ADD);
}

CREX_API zval* CREX_FASTCALL crex_hash_str_add_new(HashTable *ht, const char *str, size_t len, zval *pData)
{
	crex_ulong h = crex_hash_func(str, len);

	return _crex_hash_str_add_or_update_i(ht, str, len, h, pData, HASH_ADD_NEW);
}

CREX_API zval* CREX_FASTCALL crex_hash_index_add_empty_element(HashTable *ht, crex_ulong h)
{
	zval dummy;

	ZVAL_NULL(&dummy);
	return crex_hash_index_add(ht, h, &dummy);
}

CREX_API zval* CREX_FASTCALL crex_hash_add_empty_element(HashTable *ht, crex_string *key)
{
	zval dummy;

	ZVAL_NULL(&dummy);
	return crex_hash_add(ht, key, &dummy);
}

CREX_API zval* CREX_FASTCALL crex_hash_str_add_empty_element(HashTable *ht, const char *str, size_t len)
{
	zval dummy;

	ZVAL_NULL(&dummy);
	return crex_hash_str_add(ht, str, len, &dummy);
}

static crex_always_inline zval *_crex_hash_index_add_or_update_i(HashTable *ht, crex_ulong h, zval *pData, uint32_t flag)
{
	uint32_t nIndex;
	uint32_t idx;
	Bucket *p;
	zval *zv;

	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);

	if ((flag & HASH_ADD_NEXT) && h == CREX_LONG_MIN) {
		h = 0;
	}

	if (HT_IS_PACKED(ht)) {
		if ((flag & (HASH_ADD_NEW|HASH_ADD_NEXT)) != (HASH_ADD_NEW|HASH_ADD_NEXT)
		 && h < ht->nNumUsed) {
			zv = ht->arPacked + h;
			if (C_TYPE_P(zv) != IS_UNDEF) {
				if (flag & HASH_LOOKUP) {
					return zv;
				}
replace:
				if (flag & HASH_ADD) {
					return NULL;
				}
				if (ht->pDestructor) {
					ht->pDestructor(zv);
				}
				ZVAL_COPY_VALUE(zv, pData);
				return zv;
			} else { /* we have to keep the order :( */
				goto convert_to_hash;
			}
		} else if (EXPECTED(h < ht->nTableSize)) {
add_to_packed:
			zv = ht->arPacked + h;
			/* incremental initialization of empty Buckets */
			if ((flag & (HASH_ADD_NEW|HASH_ADD_NEXT)) != (HASH_ADD_NEW|HASH_ADD_NEXT)) {
				if (h > ht->nNumUsed) {
					zval *q = ht->arPacked + ht->nNumUsed;
					while (q != zv) {
						ZVAL_UNDEF(q);
						q++;
					}
				}
			}
			ht->nNextFreeElement = ht->nNumUsed = h + 1;
			ht->nNumOfElements++;
			if (flag & HASH_LOOKUP) {
				ZVAL_NULL(zv);
			} else {
				ZVAL_COPY_VALUE(zv, pData);
			}

			return zv;
		} else if ((h >> 1) < ht->nTableSize &&
		           (ht->nTableSize >> 1) < ht->nNumOfElements) {
			crex_hash_packed_grow(ht);
			goto add_to_packed;
		} else {
			if (ht->nNumUsed >= ht->nTableSize) {
				ht->nTableSize += ht->nTableSize;
			}
convert_to_hash:
			crex_hash_packed_to_hash(ht);
		}
	} else if (HT_FLAGS(ht) & HASH_FLAG_UNINITIALIZED) {
		if (h < ht->nTableSize) {
			crex_hash_real_init_packed_ex(ht);
			goto add_to_packed;
		}
		crex_hash_real_init_mixed(ht);
	} else {
		if ((flag & HASH_ADD_NEW) == 0 || CREX_DEBUG) {
			p = crex_hash_index_find_bucket(ht, h);
			if (p) {
				if (flag & HASH_LOOKUP) {
					return &p->val;
				}
				CREX_ASSERT((flag & HASH_ADD_NEW) == 0);
				zv = &p->val;
				goto replace;
			}
		}
		CREX_HASH_IF_FULL_DO_RESIZE(ht);		/* If the Hash table is full, resize it */
	}

	idx = ht->nNumUsed++;
	nIndex = h | ht->nTableMask;
	p = ht->arData + idx;
	C_NEXT(p->val) = HT_HASH(ht, nIndex);
	HT_HASH(ht, nIndex) = HT_IDX_TO_HASH(idx);
	if ((crex_long)h >= ht->nNextFreeElement) {
		ht->nNextFreeElement = (crex_long)h < CREX_LONG_MAX ? h + 1 : CREX_LONG_MAX;
	}
	ht->nNumOfElements++;
	p->h = h;
	p->key = NULL;
	if (flag & HASH_LOOKUP) {
		ZVAL_NULL(&p->val);
	} else {
		ZVAL_COPY_VALUE(&p->val, pData);
	}

	return &p->val;
}

CREX_API zval* CREX_FASTCALL crex_hash_index_add_or_update(HashTable *ht, crex_ulong h, zval *pData, uint32_t flag)
{
	if (flag == HASH_ADD) {
		return crex_hash_index_add(ht, h, pData);
	} else if (flag == (HASH_ADD|HASH_ADD_NEW)) {
		return crex_hash_index_add_new(ht, h, pData);
	} else if (flag == (HASH_ADD|HASH_ADD_NEXT)) {
		CREX_ASSERT(h == ht->nNextFreeElement);
		return crex_hash_next_index_insert(ht, pData);
	} else if (flag == (HASH_ADD|HASH_ADD_NEW|HASH_ADD_NEXT)) {
		CREX_ASSERT(h == ht->nNextFreeElement);
		return crex_hash_next_index_insert_new(ht, pData);
	} else {
		CREX_ASSERT(flag == HASH_UPDATE);
		return crex_hash_index_update(ht, h, pData);
	}
}

CREX_API zval* CREX_FASTCALL crex_hash_index_add(HashTable *ht, crex_ulong h, zval *pData)
{
	return _crex_hash_index_add_or_update_i(ht, h, pData, HASH_ADD);
}

CREX_API zval* CREX_FASTCALL crex_hash_index_add_new(HashTable *ht, crex_ulong h, zval *pData)
{
	return _crex_hash_index_add_or_update_i(ht, h, pData, HASH_ADD | HASH_ADD_NEW);
}

CREX_API zval* CREX_FASTCALL crex_hash_index_update(HashTable *ht, crex_ulong h, zval *pData)
{
	return _crex_hash_index_add_or_update_i(ht, h, pData, HASH_UPDATE);
}

CREX_API zval* CREX_FASTCALL crex_hash_next_index_insert(HashTable *ht, zval *pData)
{
	return _crex_hash_index_add_or_update_i(ht, ht->nNextFreeElement, pData, HASH_ADD | HASH_ADD_NEXT);
}

CREX_API zval* CREX_FASTCALL crex_hash_next_index_insert_new(HashTable *ht, zval *pData)
{
	return _crex_hash_index_add_or_update_i(ht, ht->nNextFreeElement, pData, HASH_ADD | HASH_ADD_NEW | HASH_ADD_NEXT);
}

CREX_API zval* CREX_FASTCALL crex_hash_index_lookup(HashTable *ht, crex_ulong h)
{
	return _crex_hash_index_add_or_update_i(ht, h, NULL, HASH_LOOKUP);
}

CREX_API zval* CREX_FASTCALL crex_hash_set_bucket_key(HashTable *ht, Bucket *b, crex_string *key)
{
	uint32_t nIndex;
	uint32_t idx, i;
	Bucket *p, *arData;

	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);
	CREX_ASSERT(!HT_IS_PACKED(ht));

	(void)crex_string_hash_val(key);
	p = crex_hash_find_bucket(ht, key);
	if (UNEXPECTED(p)) {
		return (p == b) ? &p->val : NULL;
	}

	if (!ZSTR_IS_INTERNED(key)) {
		crex_string_addref(key);
		HT_FLAGS(ht) &= ~HASH_FLAG_STATIC_KEYS;
	}

	arData = ht->arData;

	/* del from hash */
	idx = HT_IDX_TO_HASH(b - arData);
	nIndex = b->h | ht->nTableMask;
	i = HT_HASH_EX(arData, nIndex);
	if (i == idx) {
		HT_HASH_EX(arData, nIndex) = C_NEXT(b->val);
	} else {
		p = HT_HASH_TO_BUCKET_EX(arData, i);
		while (C_NEXT(p->val) != idx) {
			i = C_NEXT(p->val);
			p = HT_HASH_TO_BUCKET_EX(arData, i);
		}
		C_NEXT(p->val) = C_NEXT(b->val);
	}
	crex_string_release(b->key);

	/* add to hash */
	idx = b - arData;
	b->key = key;
	b->h = ZSTR_H(key);
	nIndex = b->h | ht->nTableMask;
	idx = HT_IDX_TO_HASH(idx);
	i = HT_HASH_EX(arData, nIndex);
	if (i == HT_INVALID_IDX || i < idx) {
		C_NEXT(b->val) = i;
		HT_HASH_EX(arData, nIndex) = idx;
	} else {
		p = HT_HASH_TO_BUCKET_EX(arData, i);
		while (C_NEXT(p->val) != HT_INVALID_IDX && C_NEXT(p->val) > idx) {
			i = C_NEXT(p->val);
			p = HT_HASH_TO_BUCKET_EX(arData, i);
		}
		C_NEXT(b->val) = C_NEXT(p->val);
		C_NEXT(p->val) = idx;
	}
	return &b->val;
}

static void CREX_FASTCALL crex_hash_do_resize(HashTable *ht)
{

	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);

	CREX_ASSERT(!HT_IS_PACKED(ht));
	if (ht->nNumUsed > ht->nNumOfElements + (ht->nNumOfElements >> 5)) { /* additional term is there to amortize the cost of compaction */
		crex_hash_rehash(ht);
	} else if (ht->nTableSize < HT_MAX_SIZE) {	/* Let's double the table size */
		void *new_data, *old_data = HT_GET_DATA_ADDR(ht);
		uint32_t nSize = ht->nTableSize + ht->nTableSize;
		Bucket *old_buckets = ht->arData;

		CREX_ASSERT(HT_SIZE_TO_MASK(nSize));

		new_data = pemalloc(HT_SIZE_EX(nSize, HT_SIZE_TO_MASK(nSize)), GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
		ht->nTableSize = nSize;
		ht->nTableMask = HT_SIZE_TO_MASK(ht->nTableSize);
		HT_SET_DATA_ADDR(ht, new_data);
		memcpy(ht->arData, old_buckets, sizeof(Bucket) * ht->nNumUsed);
		pefree(old_data, GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
		crex_hash_rehash(ht);
	} else {
		crex_error_noreturn(E_ERROR, "Possible integer overflow in memory allocation (%u * %zu + %zu)", ht->nTableSize * 2, sizeof(Bucket) + sizeof(uint32_t), sizeof(Bucket));
	}
}

CREX_API void CREX_FASTCALL crex_hash_rehash(HashTable *ht)
{
	Bucket *p;
	uint32_t nIndex, i;

	IS_CONSISTENT(ht);

	if (UNEXPECTED(ht->nNumOfElements == 0)) {
		if (!(HT_FLAGS(ht) & HASH_FLAG_UNINITIALIZED)) {
			ht->nNumUsed = 0;
			HT_HASH_RESET(ht);
		}
		return;
	}

	HT_HASH_RESET(ht);
	i = 0;
	p = ht->arData;
	if (HT_IS_WITHOUT_HOLES(ht)) {
		do {
			nIndex = p->h | ht->nTableMask;
			C_NEXT(p->val) = HT_HASH(ht, nIndex);
			HT_HASH(ht, nIndex) = HT_IDX_TO_HASH(i);
			p++;
		} while (++i < ht->nNumUsed);
	} else {
		uint32_t old_num_used = ht->nNumUsed;
		do {
			if (UNEXPECTED(C_TYPE(p->val) == IS_UNDEF)) {
				uint32_t j = i;
				Bucket *q = p;

				if (EXPECTED(!HT_HAS_ITERATORS(ht))) {
					while (++i < ht->nNumUsed) {
						p++;
						if (EXPECTED(C_TYPE_INFO(p->val) != IS_UNDEF)) {
							ZVAL_COPY_VALUE(&q->val, &p->val);
							q->h = p->h;
							nIndex = q->h | ht->nTableMask;
							q->key = p->key;
							C_NEXT(q->val) = HT_HASH(ht, nIndex);
							HT_HASH(ht, nIndex) = HT_IDX_TO_HASH(j);
							if (UNEXPECTED(ht->nInternalPointer == i)) {
								ht->nInternalPointer = j;
							}
							q++;
							j++;
						}
					}
				} else {
					uint32_t iter_pos = crex_hash_iterators_lower_pos(ht, i + 1);

					while (++i < ht->nNumUsed) {
						p++;
						if (EXPECTED(C_TYPE_INFO(p->val) != IS_UNDEF)) {
							ZVAL_COPY_VALUE(&q->val, &p->val);
							q->h = p->h;
							nIndex = q->h | ht->nTableMask;
							q->key = p->key;
							C_NEXT(q->val) = HT_HASH(ht, nIndex);
							HT_HASH(ht, nIndex) = HT_IDX_TO_HASH(j);
							if (UNEXPECTED(ht->nInternalPointer == i)) {
								ht->nInternalPointer = j;
							}
							if (UNEXPECTED(i >= iter_pos)) {
								do {
									crex_hash_iterators_update(ht, iter_pos, j);
									iter_pos = crex_hash_iterators_lower_pos(ht, iter_pos + 1);
								} while (iter_pos < i);
							}
							q++;
							j++;
						}
					}
				}
				ht->nNumUsed = j;
				break;
			}
			nIndex = p->h | ht->nTableMask;
			C_NEXT(p->val) = HT_HASH(ht, nIndex);
			HT_HASH(ht, nIndex) = HT_IDX_TO_HASH(i);
			p++;
		} while (++i < ht->nNumUsed);

		/* Migrate pointer to one past the end of the array to the new one past the end, so that
		 * newly inserted elements are picked up correctly. */
		if (UNEXPECTED(HT_HAS_ITERATORS(ht))) {
			_crex_hash_iterators_update(ht, old_num_used, ht->nNumUsed);
		}
	}
}

static crex_always_inline void _crex_hash_packed_del_val(HashTable *ht, uint32_t idx, zval *zv)
{
	idx = HT_HASH_TO_IDX(idx);
	ht->nNumOfElements--;
	if (ht->nInternalPointer == idx || UNEXPECTED(HT_HAS_ITERATORS(ht))) {
		uint32_t new_idx;

		new_idx = idx;
		while (1) {
			new_idx++;
			if (new_idx >= ht->nNumUsed) {
				break;
			} else if (C_TYPE(ht->arPacked[new_idx]) != IS_UNDEF) {
				break;
			}
		}
		if (ht->nInternalPointer == idx) {
			ht->nInternalPointer = new_idx;
		}
		crex_hash_iterators_update(ht, idx, new_idx);
	}
	if (ht->nNumUsed - 1 == idx) {
		do {
			ht->nNumUsed--;
		} while (ht->nNumUsed > 0 && (UNEXPECTED(C_TYPE(ht->arPacked[ht->nNumUsed-1]) == IS_UNDEF)));
		ht->nInternalPointer = MIN(ht->nInternalPointer, ht->nNumUsed);
	}
	if (ht->pDestructor) {
		zval tmp;
		ZVAL_COPY_VALUE(&tmp, zv);
		ZVAL_UNDEF(zv);
		ht->pDestructor(&tmp);
	} else {
		ZVAL_UNDEF(zv);
	}
}

static crex_always_inline void _crex_hash_del_el_ex(HashTable *ht, uint32_t idx, Bucket *p, Bucket *prev)
{
	if (prev) {
		C_NEXT(prev->val) = C_NEXT(p->val);
	} else {
		HT_HASH(ht, p->h | ht->nTableMask) = C_NEXT(p->val);
	}
	idx = HT_HASH_TO_IDX(idx);
	ht->nNumOfElements--;
	if (ht->nInternalPointer == idx || UNEXPECTED(HT_HAS_ITERATORS(ht))) {
		uint32_t new_idx;

		new_idx = idx;
		while (1) {
			new_idx++;
			if (new_idx >= ht->nNumUsed) {
				break;
			} else if (C_TYPE(ht->arData[new_idx].val) != IS_UNDEF) {
				break;
			}
		}
		if (ht->nInternalPointer == idx) {
			ht->nInternalPointer = new_idx;
		}
		crex_hash_iterators_update(ht, idx, new_idx);
	}
	if (ht->nNumUsed - 1 == idx) {
		do {
			ht->nNumUsed--;
		} while (ht->nNumUsed > 0 && (UNEXPECTED(C_TYPE(ht->arData[ht->nNumUsed-1].val) == IS_UNDEF)));
		ht->nInternalPointer = MIN(ht->nInternalPointer, ht->nNumUsed);
	}
	if (ht->pDestructor) {
		zval tmp;
		ZVAL_COPY_VALUE(&tmp, &p->val);
		ZVAL_UNDEF(&p->val);
		ht->pDestructor(&tmp);
	} else {
		ZVAL_UNDEF(&p->val);
	}
}

static crex_always_inline void _crex_hash_del_el(HashTable *ht, uint32_t idx, Bucket *p)
{
	Bucket *prev = NULL;
	uint32_t nIndex;
	uint32_t i;

	nIndex = p->h | ht->nTableMask;
	i = HT_HASH(ht, nIndex);

	if (i != idx) {
		prev = HT_HASH_TO_BUCKET(ht, i);
		while (C_NEXT(prev->val) != idx) {
			i = C_NEXT(prev->val);
			prev = HT_HASH_TO_BUCKET(ht, i);
		}
	}

	if (p->key) {
		crex_string_release(p->key);
		p->key = NULL;
	}
	_crex_hash_del_el_ex(ht, idx, p, prev);
}

CREX_API void CREX_FASTCALL crex_hash_packed_del_val(HashTable *ht, zval *zv)
{
	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);
	CREX_ASSERT(HT_IS_PACKED(ht));
	_crex_hash_packed_del_val(ht, HT_IDX_TO_HASH(zv - ht->arPacked), zv);
}


CREX_API void CREX_FASTCALL crex_hash_del_bucket(HashTable *ht, Bucket *p)
{
	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);
	CREX_ASSERT(!HT_IS_PACKED(ht));
	_crex_hash_del_el(ht, HT_IDX_TO_HASH(p - ht->arData), p);
}

CREX_API crex_result CREX_FASTCALL crex_hash_del(HashTable *ht, crex_string *key)
{
	crex_ulong h;
	uint32_t nIndex;
	uint32_t idx;
	Bucket *p;
	Bucket *prev = NULL;

	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);

	h = crex_string_hash_val(key);
	nIndex = h | ht->nTableMask;

	idx = HT_HASH(ht, nIndex);
	while (idx != HT_INVALID_IDX) {
		p = HT_HASH_TO_BUCKET(ht, idx);
		if ((p->key == key) ||
			(p->h == h &&
		     p->key &&
		     crex_string_equal_content(p->key, key))) {
			crex_string_release(p->key);
			p->key = NULL;
			_crex_hash_del_el_ex(ht, idx, p, prev);
			return SUCCESS;
		}
		prev = p;
		idx = C_NEXT(p->val);
	}
	return FAILURE;
}

CREX_API crex_result CREX_FASTCALL crex_hash_del_ind(HashTable *ht, crex_string *key)
{
	crex_ulong h;
	uint32_t nIndex;
	uint32_t idx;
	Bucket *p;
	Bucket *prev = NULL;

	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);

	h = crex_string_hash_val(key);
	nIndex = h | ht->nTableMask;

	idx = HT_HASH(ht, nIndex);
	while (idx != HT_INVALID_IDX) {
		p = HT_HASH_TO_BUCKET(ht, idx);
		if ((p->key == key) ||
			(p->h == h &&
		     p->key &&
		     crex_string_equal_content(p->key, key))) {
			if (C_TYPE(p->val) == IS_INDIRECT) {
				zval *data = C_INDIRECT(p->val);

				if (UNEXPECTED(C_TYPE_P(data) == IS_UNDEF)) {
					return FAILURE;
				} else {
					if (ht->pDestructor) {
						zval tmp;
						ZVAL_COPY_VALUE(&tmp, data);
						ZVAL_UNDEF(data);
						ht->pDestructor(&tmp);
					} else {
						ZVAL_UNDEF(data);
					}
					HT_FLAGS(ht) |= HASH_FLAG_HAS_EMPTY_IND;
				}
			} else {
				crex_string_release(p->key);
				p->key = NULL;
				_crex_hash_del_el_ex(ht, idx, p, prev);
			}
			return SUCCESS;
		}
		prev = p;
		idx = C_NEXT(p->val);
	}
	return FAILURE;
}

CREX_API crex_result CREX_FASTCALL crex_hash_str_del_ind(HashTable *ht, const char *str, size_t len)
{
	crex_ulong h;
	uint32_t nIndex;
	uint32_t idx;
	Bucket *p;
	Bucket *prev = NULL;

	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);

	h = crex_inline_hash_func(str, len);
	nIndex = h | ht->nTableMask;

	idx = HT_HASH(ht, nIndex);
	while (idx != HT_INVALID_IDX) {
		p = HT_HASH_TO_BUCKET(ht, idx);
		if ((p->h == h)
			 && p->key
			 && crex_string_equals_cstr(p->key, str, len)) {
			if (C_TYPE(p->val) == IS_INDIRECT) {
				zval *data = C_INDIRECT(p->val);

				if (UNEXPECTED(C_TYPE_P(data) == IS_UNDEF)) {
					return FAILURE;
				} else {
					if (ht->pDestructor) {
						ht->pDestructor(data);
					}
					ZVAL_UNDEF(data);
					HT_FLAGS(ht) |= HASH_FLAG_HAS_EMPTY_IND;
				}
			} else {
				crex_string_release(p->key);
				p->key = NULL;
				_crex_hash_del_el_ex(ht, idx, p, prev);
			}
			return SUCCESS;
		}
		prev = p;
		idx = C_NEXT(p->val);
	}
	return FAILURE;
}

CREX_API crex_result CREX_FASTCALL crex_hash_str_del(HashTable *ht, const char *str, size_t len)
{
	crex_ulong h;
	uint32_t nIndex;
	uint32_t idx;
	Bucket *p;
	Bucket *prev = NULL;

	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);

	h = crex_inline_hash_func(str, len);
	nIndex = h | ht->nTableMask;

	idx = HT_HASH(ht, nIndex);
	while (idx != HT_INVALID_IDX) {
		p = HT_HASH_TO_BUCKET(ht, idx);
		if ((p->h == h)
			 && p->key
			 && crex_string_equals_cstr(p->key, str, len)) {
			crex_string_release(p->key);
			p->key = NULL;
			_crex_hash_del_el_ex(ht, idx, p, prev);
			return SUCCESS;
		}
		prev = p;
		idx = C_NEXT(p->val);
	}
	return FAILURE;
}

CREX_API crex_result CREX_FASTCALL crex_hash_index_del(HashTable *ht, crex_ulong h)
{
	uint32_t nIndex;
	uint32_t idx;
	Bucket *p;
	Bucket *prev = NULL;

	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);

	if (HT_IS_PACKED(ht)) {
		if (h < ht->nNumUsed) {
			zval *zv = ht->arPacked + h;
			if (C_TYPE_P(zv) != IS_UNDEF) {
				_crex_hash_packed_del_val(ht, HT_IDX_TO_HASH(h), zv);
				return SUCCESS;
			}
		}
		return FAILURE;
	}
	nIndex = h | ht->nTableMask;

	idx = HT_HASH(ht, nIndex);
	while (idx != HT_INVALID_IDX) {
		p = HT_HASH_TO_BUCKET(ht, idx);
		if ((p->h == h) && (p->key == NULL)) {
			_crex_hash_del_el_ex(ht, idx, p, prev);
			return SUCCESS;
		}
		prev = p;
		idx = C_NEXT(p->val);
	}
	return FAILURE;
}

CREX_API void CREX_FASTCALL crex_hash_destroy(HashTable *ht)
{
	IS_CONSISTENT(ht);
	HT_ASSERT(ht, GC_REFCOUNT(ht) <= 1);

	if (ht->nNumUsed) {
		if (HT_IS_PACKED(ht)) {
			if (ht->pDestructor) {
				zval *zv = ht->arPacked;
				zval *end = zv + ht->nNumUsed;

				SET_INCONSISTENT(HT_IS_DESTROYING);
				if (HT_IS_WITHOUT_HOLES(ht)) {
					do {
						ht->pDestructor(zv);
					} while (++zv != end);
				} else {
					do {
						if (EXPECTED(C_TYPE_P(zv) != IS_UNDEF)) {
							ht->pDestructor(zv);
						}
					} while (++zv != end);
				}
				SET_INCONSISTENT(HT_DESTROYED);
			}
			crex_hash_iterators_remove(ht);
		} else {
			Bucket *p = ht->arData;
			Bucket *end = p + ht->nNumUsed;

			if (ht->pDestructor) {
				SET_INCONSISTENT(HT_IS_DESTROYING);

				if (HT_HAS_STATIC_KEYS_ONLY(ht)) {
					if (HT_IS_WITHOUT_HOLES(ht)) {
						do {
							ht->pDestructor(&p->val);
						} while (++p != end);
					} else {
						do {
							if (EXPECTED(C_TYPE(p->val) != IS_UNDEF)) {
								ht->pDestructor(&p->val);
							}
						} while (++p != end);
					}
				} else if (HT_IS_WITHOUT_HOLES(ht)) {
					do {
						ht->pDestructor(&p->val);
						if (EXPECTED(p->key)) {
							crex_string_release(p->key);
						}
					} while (++p != end);
				} else {
					do {
						if (EXPECTED(C_TYPE(p->val) != IS_UNDEF)) {
							ht->pDestructor(&p->val);
							if (EXPECTED(p->key)) {
								crex_string_release(p->key);
							}
						}
					} while (++p != end);
				}

				SET_INCONSISTENT(HT_DESTROYED);
			} else {
				if (!HT_HAS_STATIC_KEYS_ONLY(ht)) {
					do {
						if (EXPECTED(p->key)) {
							crex_string_release(p->key);
						}
					} while (++p != end);
				}
			}
			crex_hash_iterators_remove(ht);
		}
	} else if (EXPECTED(HT_FLAGS(ht) & HASH_FLAG_UNINITIALIZED)) {
		return;
	}
	pefree(HT_GET_DATA_ADDR(ht), GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
}

CREX_API void CREX_FASTCALL crex_array_destroy(HashTable *ht)
{
	IS_CONSISTENT(ht);
	HT_ASSERT(ht, GC_REFCOUNT(ht) <= 1);

	/* break possible cycles */
	GC_REMOVE_FROM_BUFFER(ht);
	GC_TYPE_INFO(ht) = GC_NULL /*???| (GC_WHITE << 16)*/;

	if (ht->nNumUsed) {
		/* In some rare cases destructors of regular arrays may be changed */
		if (UNEXPECTED(ht->pDestructor != ZVAL_PTR_DTOR)) {
			crex_hash_destroy(ht);
			goto free_ht;
		}

		SET_INCONSISTENT(HT_IS_DESTROYING);

		if (HT_IS_PACKED(ht)) {
			zval *zv = ht->arPacked;
			zval *end = zv + ht->nNumUsed;

			do {
				i_zval_ptr_dtor(zv);
			} while (++zv != end);
		} else {
			Bucket *p = ht->arData;
			Bucket *end = p + ht->nNumUsed;

			if (HT_HAS_STATIC_KEYS_ONLY(ht)) {
				do {
					i_zval_ptr_dtor(&p->val);
				} while (++p != end);
			} else if (HT_IS_WITHOUT_HOLES(ht)) {
				do {
					i_zval_ptr_dtor(&p->val);
					if (EXPECTED(p->key)) {
						crex_string_release_ex(p->key, 0);
					}
				} while (++p != end);
			} else {
				do {
					if (EXPECTED(C_TYPE(p->val) != IS_UNDEF)) {
						i_zval_ptr_dtor(&p->val);
						if (EXPECTED(p->key)) {
							crex_string_release_ex(p->key, 0);
						}
					}
				} while (++p != end);
			}
		}
	} else if (EXPECTED(HT_FLAGS(ht) & HASH_FLAG_UNINITIALIZED)) {
		goto free_ht;
	}
	SET_INCONSISTENT(HT_DESTROYED);
	efree(HT_GET_DATA_ADDR(ht));
free_ht:
	crex_hash_iterators_remove(ht);
	FREE_HASHTABLE(ht);
}

CREX_API void CREX_FASTCALL crex_hash_clean(HashTable *ht)
{
	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);

	if (ht->nNumUsed) {
		if (HT_IS_PACKED(ht)) {
			zval *zv = ht->arPacked;
			zval *end = zv + ht->nNumUsed;

			if (ht->pDestructor) {
				if (HT_HAS_STATIC_KEYS_ONLY(ht)) {
					if (HT_IS_WITHOUT_HOLES(ht)) {
						do {
							ht->pDestructor(zv);
						} while (++zv != end);
					} else {
						do {
							if (EXPECTED(C_TYPE_P(zv) != IS_UNDEF)) {
								ht->pDestructor(zv);
							}
						} while (++zv != end);
					}
				}
			}
		} else {
			Bucket *p = ht->arData;
			Bucket *end = p + ht->nNumUsed;

			if (ht->pDestructor) {
				if (HT_HAS_STATIC_KEYS_ONLY(ht)) {
					if (HT_IS_WITHOUT_HOLES(ht)) {
						do {
							ht->pDestructor(&p->val);
						} while (++p != end);
					} else {
						do {
							if (EXPECTED(C_TYPE(p->val) != IS_UNDEF)) {
								ht->pDestructor(&p->val);
							}
						} while (++p != end);
					}
				} else if (HT_IS_WITHOUT_HOLES(ht)) {
					do {
						ht->pDestructor(&p->val);
						if (EXPECTED(p->key)) {
							crex_string_release(p->key);
						}
					} while (++p != end);
				} else {
					do {
						if (EXPECTED(C_TYPE(p->val) != IS_UNDEF)) {
							ht->pDestructor(&p->val);
							if (EXPECTED(p->key)) {
								crex_string_release(p->key);
							}
						}
					} while (++p != end);
				}
			} else {
				if (!HT_HAS_STATIC_KEYS_ONLY(ht)) {
					do {
						if (EXPECTED(p->key)) {
							crex_string_release(p->key);
						}
					} while (++p != end);
				}
			}
			HT_HASH_RESET(ht);
		}
	}
	ht->nNumUsed = 0;
	ht->nNumOfElements = 0;
	ht->nNextFreeElement = CREX_LONG_MIN;
	ht->nInternalPointer = 0;
}

CREX_API void CREX_FASTCALL crex_symtable_clean(HashTable *ht)
{
	Bucket *p, *end;

	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);

	if (ht->nNumUsed) {
		CREX_ASSERT(!HT_IS_PACKED(ht));
		p = ht->arData;
		end = p + ht->nNumUsed;
		if (HT_HAS_STATIC_KEYS_ONLY(ht)) {
			do {
				i_zval_ptr_dtor(&p->val);
			} while (++p != end);
		} else if (HT_IS_WITHOUT_HOLES(ht)) {
			do {
				i_zval_ptr_dtor(&p->val);
				if (EXPECTED(p->key)) {
					crex_string_release(p->key);
				}
			} while (++p != end);
		} else {
			do {
				if (EXPECTED(C_TYPE(p->val) != IS_UNDEF)) {
					i_zval_ptr_dtor(&p->val);
					if (EXPECTED(p->key)) {
						crex_string_release(p->key);
					}
				}
			} while (++p != end);
		}
		HT_HASH_RESET(ht);
	}
	ht->nNumUsed = 0;
	ht->nNumOfElements = 0;
	ht->nNextFreeElement = CREX_LONG_MIN;
	ht->nInternalPointer = 0;
}

CREX_API void CREX_FASTCALL crex_hash_graceful_destroy(HashTable *ht)
{
	uint32_t idx;

	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);

	if (HT_IS_PACKED(ht)) {
		zval *zv = ht->arPacked;

		for (idx = 0; idx < ht->nNumUsed; idx++, zv++) {
			if (UNEXPECTED(C_TYPE_P(zv) == IS_UNDEF)) continue;
			_crex_hash_packed_del_val(ht, HT_IDX_TO_HASH(idx), zv);
		}
	} else {
		Bucket *p = ht->arData;

		for (idx = 0; idx < ht->nNumUsed; idx++, p++) {
			if (UNEXPECTED(C_TYPE(p->val) == IS_UNDEF)) continue;
			_crex_hash_del_el(ht, HT_IDX_TO_HASH(idx), p);
		}
	}
	if (!(HT_FLAGS(ht) & HASH_FLAG_UNINITIALIZED)) {
		pefree(HT_GET_DATA_ADDR(ht), GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
	}

	SET_INCONSISTENT(HT_DESTROYED);
}

CREX_API void CREX_FASTCALL crex_hash_graceful_reverse_destroy(HashTable *ht)
{
	uint32_t idx;

	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);

	idx = ht->nNumUsed;
	if (HT_IS_PACKED(ht)) {
		zval *zv = ht->arPacked + ht->nNumUsed;

		while (idx > 0) {
			idx--;
			zv--;
			if (UNEXPECTED(C_TYPE_P(zv) == IS_UNDEF)) continue;
			_crex_hash_packed_del_val(ht, HT_IDX_TO_HASH(idx), zv);
		}
	} else {
		Bucket *p = ht->arData + ht->nNumUsed;

		while (idx > 0) {
			idx--;
			p--;
			if (UNEXPECTED(C_TYPE(p->val) == IS_UNDEF)) continue;
			_crex_hash_del_el(ht, HT_IDX_TO_HASH(idx), p);
		}
	}

	if (!(HT_FLAGS(ht) & HASH_FLAG_UNINITIALIZED)) {
		pefree(HT_GET_DATA_ADDR(ht), GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
	}

	SET_INCONSISTENT(HT_DESTROYED);
}

/* This is used to recurse elements and selectively delete certain entries
 * from a hashtable. apply_func() receives the data and decides if the entry
 * should be deleted or recursion should be stopped. The following three
 * return codes are possible:
 * CREX_HASH_APPLY_KEEP   - continue
 * CREX_HASH_APPLY_STOP   - stop iteration
 * CREX_HASH_APPLY_REMOVE - delete the element, combinable with the former
 */

CREX_API void CREX_FASTCALL crex_hash_apply(HashTable *ht, apply_func_t apply_func)
{
	uint32_t idx;
	int result;

	IS_CONSISTENT(ht);
	if (HT_IS_PACKED(ht)) {
		for (idx = 0; idx < ht->nNumUsed; idx++) {
			zval *zv = ht->arPacked + idx;

			if (UNEXPECTED(C_TYPE_P(zv) == IS_UNDEF)) continue;
			result = apply_func(zv);

			if (result & CREX_HASH_APPLY_REMOVE) {
				HT_ASSERT_RC1(ht);
				_crex_hash_packed_del_val(ht, HT_IDX_TO_HASH(idx), zv);
			}
			if (result & CREX_HASH_APPLY_STOP) {
				break;
			}
		}
	} else {
		for (idx = 0; idx < ht->nNumUsed; idx++) {
			Bucket *p = ht->arData + idx;

			if (UNEXPECTED(C_TYPE(p->val) == IS_UNDEF)) continue;
			result = apply_func(&p->val);

			if (result & CREX_HASH_APPLY_REMOVE) {
				HT_ASSERT_RC1(ht);
				_crex_hash_del_el(ht, HT_IDX_TO_HASH(idx), p);
			}
			if (result & CREX_HASH_APPLY_STOP) {
				break;
			}
		}
	}
}


CREX_API void CREX_FASTCALL crex_hash_apply_with_argument(HashTable *ht, apply_func_arg_t apply_func, void *argument)
{
	uint32_t idx;
	int result;

	IS_CONSISTENT(ht);
	if (HT_IS_PACKED(ht)) {
		for (idx = 0; idx < ht->nNumUsed; idx++) {
			zval *zv = ht->arPacked + idx;
			if (UNEXPECTED(C_TYPE_P(zv) == IS_UNDEF)) continue;
			result = apply_func(zv, argument);

			if (result & CREX_HASH_APPLY_REMOVE) {
				HT_ASSERT_RC1(ht);
				_crex_hash_packed_del_val(ht, HT_IDX_TO_HASH(idx), zv);
			}
			if (result & CREX_HASH_APPLY_STOP) {
				break;
			}
		}
	} else {
		for (idx = 0; idx < ht->nNumUsed; idx++) {
			Bucket *p = ht->arData + idx;
			if (UNEXPECTED(C_TYPE(p->val) == IS_UNDEF)) continue;
			result = apply_func(&p->val, argument);

			if (result & CREX_HASH_APPLY_REMOVE) {
				HT_ASSERT_RC1(ht);
				_crex_hash_del_el(ht, HT_IDX_TO_HASH(idx), p);
			}
			if (result & CREX_HASH_APPLY_STOP) {
				break;
			}
		}
	}
}


CREX_API void crex_hash_apply_with_arguments(HashTable *ht, apply_func_args_t apply_func, int num_args, ...)
{
	uint32_t idx;
	va_list args;
	crex_hash_key hash_key;
	int result;

	IS_CONSISTENT(ht);

	if (HT_IS_PACKED(ht)) {
		for (idx = 0; idx < ht->nNumUsed; idx++) {
			zval *zv = ht->arPacked + idx;

			if (UNEXPECTED(C_TYPE_P(zv) == IS_UNDEF)) continue;
			va_start(args, num_args);
			hash_key.h = idx;
			hash_key.key = NULL;

			result = apply_func(zv, num_args, args, &hash_key);

			if (result & CREX_HASH_APPLY_REMOVE) {
				HT_ASSERT_RC1(ht);
				_crex_hash_packed_del_val(ht, HT_IDX_TO_HASH(idx), zv);
			}
			if (result & CREX_HASH_APPLY_STOP) {
				va_end(args);
				break;
			}
			va_end(args);
		}
	} else {
		for (idx = 0; idx < ht->nNumUsed; idx++) {
			Bucket *p = ht->arData + idx;

			if (UNEXPECTED(C_TYPE(p->val) == IS_UNDEF)) continue;
			va_start(args, num_args);
			hash_key.h = p->h;
			hash_key.key = p->key;

			result = apply_func(&p->val, num_args, args, &hash_key);

			if (result & CREX_HASH_APPLY_REMOVE) {
				HT_ASSERT_RC1(ht);
				_crex_hash_del_el(ht, HT_IDX_TO_HASH(idx), p);
			}
			if (result & CREX_HASH_APPLY_STOP) {
				va_end(args);
				break;
			}
			va_end(args);
		}
	}
}


CREX_API void CREX_FASTCALL crex_hash_reverse_apply(HashTable *ht, apply_func_t apply_func)
{
	uint32_t idx;
	int result;

	IS_CONSISTENT(ht);

	idx = ht->nNumUsed;
	if (HT_IS_PACKED(ht)) {
		zval *zv;

		while (idx > 0) {
			idx--;
			zv = ht->arPacked + idx;
			if (UNEXPECTED(C_TYPE_P(zv) == IS_UNDEF)) continue;

			result = apply_func(zv);

			if (result & CREX_HASH_APPLY_REMOVE) {
				HT_ASSERT_RC1(ht);
				_crex_hash_packed_del_val(ht, HT_IDX_TO_HASH(idx), zv);
			}
			if (result & CREX_HASH_APPLY_STOP) {
				break;
			}
		}
	} else {
		Bucket *p;

		while (idx > 0) {
			idx--;
			p = ht->arData + idx;
			if (UNEXPECTED(C_TYPE(p->val) == IS_UNDEF)) continue;

			result = apply_func(&p->val);

			if (result & CREX_HASH_APPLY_REMOVE) {
				HT_ASSERT_RC1(ht);
				_crex_hash_del_el(ht, HT_IDX_TO_HASH(idx), p);
			}
			if (result & CREX_HASH_APPLY_STOP) {
				break;
			}
		}
	}
}


CREX_API void CREX_FASTCALL crex_hash_copy(HashTable *target, HashTable *source, copy_ctor_func_t pCopyConstructor)
{
	uint32_t idx;
	zval *new_entry, *data;

	IS_CONSISTENT(source);
	IS_CONSISTENT(target);
	HT_ASSERT_RC1(target);

	if (HT_IS_PACKED(source)) {
		for (idx = 0; idx < source->nNumUsed; idx++) {
			zval *zv = source->arPacked + idx;
			if (UNEXPECTED(C_TYPE_P(zv) == IS_UNDEF)) continue;

			new_entry = crex_hash_index_update(target, idx, zv);
			if (pCopyConstructor) {
				pCopyConstructor(new_entry);
			}
		}
		return;
	}

	for (idx = 0; idx < source->nNumUsed; idx++) {
		Bucket *p = source->arData + idx;

		if (UNEXPECTED(C_TYPE(p->val) == IS_UNDEF)) continue;

		/* INDIRECT element may point to UNDEF-ined slots */
		data = &p->val;
		if (C_TYPE_P(data) == IS_INDIRECT) {
			data = C_INDIRECT_P(data);
			if (UNEXPECTED(C_TYPE_P(data) == IS_UNDEF)) {
				continue;
			}
		}
		if (p->key) {
			new_entry = crex_hash_update(target, p->key, data);
		} else {
			new_entry = crex_hash_index_update(target, p->h, data);
		}
		if (pCopyConstructor) {
			pCopyConstructor(new_entry);
		}
	}
}


static crex_always_inline bool crex_array_dup_value(HashTable *source, HashTable *target, zval *data, zval *dest, bool packed, bool with_holes)
{
	if (with_holes) {
		if (!packed && C_TYPE_INFO_P(data) == IS_INDIRECT) {
			data = C_INDIRECT_P(data);
		}
		if (UNEXPECTED(C_TYPE_INFO_P(data) == IS_UNDEF)) {
			return 0;
		}
	} else if (!packed) {
		/* INDIRECT element may point to UNDEF-ined slots */
		if (C_TYPE_INFO_P(data) == IS_INDIRECT) {
			data = C_INDIRECT_P(data);
			if (UNEXPECTED(C_TYPE_INFO_P(data) == IS_UNDEF)) {
				return 0;
			}
		}
	}

	do {
		if (C_OPT_REFCOUNTED_P(data)) {
			if (C_ISREF_P(data) && C_REFCOUNT_P(data) == 1 &&
			    (C_TYPE_P(C_REFVAL_P(data)) != IS_ARRAY ||
			      C_ARRVAL_P(C_REFVAL_P(data)) != source)) {
				data = C_REFVAL_P(data);
				if (!C_OPT_REFCOUNTED_P(data)) {
					break;
				}
			}
			C_ADDREF_P(data);
		}
	} while (0);
	ZVAL_COPY_VALUE(dest, data);

	return 1;
}

static crex_always_inline bool crex_array_dup_element(HashTable *source, HashTable *target, uint32_t idx, Bucket *p, Bucket *q, bool packed, bool static_keys, bool with_holes)
{
	if (!crex_array_dup_value(source, target, &p->val, &q->val, packed, with_holes)) {
		return 0;
	}

	if (!packed) {
		uint32_t nIndex;

		q->h = p->h;
		q->key = p->key;
		if (!static_keys && q->key) {
			crex_string_addref(q->key);
		}

		nIndex = q->h | target->nTableMask;
		C_NEXT(q->val) = HT_HASH(target, nIndex);
		HT_HASH(target, nIndex) = HT_IDX_TO_HASH(idx);
	}
	return 1;
}

// We need to duplicate iterators to be able to search through all copy-on-write copies to find the actually iterated HashTable and position back
static void crex_array_dup_ht_iterators(HashTable *source, HashTable *target) {
	HashTableIterator *iter = EG(ht_iterators);
	HashTableIterator *end  = iter + EG(ht_iterators_used);

	while (iter != end) {
		if (iter->ht == source) {
			uint32_t copy_idx = crex_hash_iterator_add(target, iter->pos);
			HashTableIterator *copy_iter = EG(ht_iterators) + copy_idx;
			copy_iter->next_copy = iter->next_copy;
			iter->next_copy = copy_idx;
		}
		iter++;
	}
}

static crex_always_inline void crex_array_dup_packed_elements(HashTable *source, HashTable *target, bool with_holes)
{
	zval *p = source->arPacked;
	zval *q = target->arPacked;
	zval *end = p + source->nNumUsed;

	do {
		if (!crex_array_dup_value(source, target, p, q, 1, with_holes)) {
			if (with_holes) {
				ZVAL_UNDEF(q);
			}
		}
		p++; q++;
	} while (p != end);

	if (UNEXPECTED(HT_HAS_ITERATORS(source))) {
		crex_array_dup_ht_iterators(source, target);
	}
}

static crex_always_inline uint32_t crex_array_dup_elements(HashTable *source, HashTable *target, bool static_keys, bool with_holes)
{
	uint32_t idx = 0;
	Bucket *p = source->arData;
	Bucket *q = target->arData;
	Bucket *end = p + source->nNumUsed;

	if (UNEXPECTED(HT_HAS_ITERATORS(source))) {
		crex_array_dup_ht_iterators(source, target);
	}

	do {
		if (!crex_array_dup_element(source, target, idx, p, q, 0, static_keys, with_holes)) {
			uint32_t target_idx = idx;

			idx++; p++;
			if (EXPECTED(!HT_HAS_ITERATORS(target))) {
				while (p != end) {
					if (crex_array_dup_element(source, target, target_idx, p, q, 0, static_keys, with_holes)) {
						if (source->nInternalPointer == idx) {
							target->nInternalPointer = target_idx;
						}
						target_idx++; q++;
					}
					idx++; p++;
				}
			} else {
				target->nNumUsed = source->nNumOfElements;
				uint32_t iter_pos = crex_hash_iterators_lower_pos(target, idx);

				while (p != end) {
					if (crex_array_dup_element(source, target, target_idx, p, q, 0, static_keys, with_holes)) {
						if (source->nInternalPointer == idx) {
							target->nInternalPointer = target_idx;
						}
						if (UNEXPECTED(idx >= iter_pos)) {
							do {
								crex_hash_iterators_update(target, iter_pos, target_idx);
								iter_pos = crex_hash_iterators_lower_pos(target, iter_pos + 1);
							} while (iter_pos < idx);
						}
						target_idx++; q++;
					}
					idx++; p++;
				}
			}
			return target_idx;
		}
		idx++; p++; q++;
	} while (p != end);
	return idx;
}

CREX_API HashTable* CREX_FASTCALL crex_array_dup(HashTable *source)
{
	uint32_t idx;
	HashTable *target;

	IS_CONSISTENT(source);

	ALLOC_HASHTABLE(target);
	GC_SET_REFCOUNT(target, 1);
	GC_TYPE_INFO(target) = GC_ARRAY;

	target->pDestructor = ZVAL_PTR_DTOR;

	if (source->nNumOfElements == 0) {
		HT_FLAGS(target) = HASH_FLAG_UNINITIALIZED;
		target->nTableMask = HT_MIN_MASK;
		target->nNumUsed = 0;
		target->nNumOfElements = 0;
		target->nNextFreeElement = source->nNextFreeElement;
		target->nInternalPointer = 0;
		target->nTableSize = HT_MIN_SIZE;
		HT_SET_DATA_ADDR(target, &uninitialized_bucket);
	} else if (GC_FLAGS(source) & IS_ARRAY_IMMUTABLE) {
		HT_FLAGS(target) = HT_FLAGS(source) & HASH_FLAG_MASK;
		target->nTableMask = source->nTableMask;
		target->nNumUsed = source->nNumUsed;
		target->nNumOfElements = source->nNumOfElements;
		target->nNextFreeElement = source->nNextFreeElement;
		target->nTableSize = source->nTableSize;
		if (HT_IS_PACKED(source)) {
			HT_SET_DATA_ADDR(target, emalloc(HT_PACKED_SIZE(target)));
			target->nInternalPointer = source->nInternalPointer;
			memcpy(HT_GET_DATA_ADDR(target), HT_GET_DATA_ADDR(source), HT_PACKED_USED_SIZE(source));
		} else {
			HT_SET_DATA_ADDR(target, emalloc(HT_SIZE(target)));
			target->nInternalPointer = source->nInternalPointer;
			memcpy(HT_GET_DATA_ADDR(target), HT_GET_DATA_ADDR(source), HT_USED_SIZE(source));
		}
	} else if (HT_IS_PACKED(source)) {
		HT_FLAGS(target) = HT_FLAGS(source) & HASH_FLAG_MASK;
		target->nTableMask = HT_MIN_MASK;
		target->nNumUsed = source->nNumUsed;
		target->nNumOfElements = source->nNumOfElements;
		target->nNextFreeElement = source->nNextFreeElement;
		target->nTableSize = source->nTableSize;
		HT_SET_DATA_ADDR(target, emalloc(HT_PACKED_SIZE_EX(target->nTableSize, HT_MIN_MASK)));
		target->nInternalPointer =
			(source->nInternalPointer < source->nNumUsed) ?
				source->nInternalPointer : 0;

		HT_HASH_RESET_PACKED(target);

		if (HT_IS_WITHOUT_HOLES(target)) {
			crex_array_dup_packed_elements(source, target, 0);
		} else {
			crex_array_dup_packed_elements(source, target, 1);
		}
	} else {
		HT_FLAGS(target) = HT_FLAGS(source) & HASH_FLAG_MASK;
		target->nTableMask = source->nTableMask;
		target->nNextFreeElement = source->nNextFreeElement;
		target->nInternalPointer =
			(source->nInternalPointer < source->nNumUsed) ?
				source->nInternalPointer : 0;

		target->nTableSize = source->nTableSize;
		HT_SET_DATA_ADDR(target, emalloc(HT_SIZE(target)));
		HT_HASH_RESET(target);

		if (HT_HAS_STATIC_KEYS_ONLY(target)) {
			if (HT_IS_WITHOUT_HOLES(source)) {
				idx = crex_array_dup_elements(source, target, 1, 0);
			} else {
				idx = crex_array_dup_elements(source, target, 1, 1);
			}
		} else {
			if (HT_IS_WITHOUT_HOLES(source)) {
				idx = crex_array_dup_elements(source, target, 0, 0);
			} else {
				idx = crex_array_dup_elements(source, target, 0, 1);
			}
		}
		target->nNumUsed = idx;
		target->nNumOfElements = idx;
	}
	return target;
}

CREX_API HashTable* crex_array_to_list(HashTable *source)
{
	HashTable *result = _crex_new_array(crex_hash_num_elements(source));
	crex_hash_real_init_packed(result);

	CREX_HASH_FILL_PACKED(result) {
		zval *entry;

		CREX_HASH_FOREACH_VAL(source, entry) {
			if (UNEXPECTED(C_ISREF_P(entry) && C_REFCOUNT_P(entry) == 1)) {
				entry = C_REFVAL_P(entry);
			}
			C_TRY_ADDREF_P(entry);
			CREX_HASH_FILL_ADD(entry);
		} CREX_HASH_FOREACH_END();
	} CREX_HASH_FILL_END();

	return result;
}


CREX_API void CREX_FASTCALL crex_hash_merge(HashTable *target, HashTable *source, copy_ctor_func_t pCopyConstructor, bool overwrite)
{
	uint32_t idx;
	Bucket *p;
	zval *t, *s;

	IS_CONSISTENT(source);
	IS_CONSISTENT(target);
	HT_ASSERT_RC1(target);

	if (overwrite) {
		if (HT_IS_PACKED(source)) {
			for (idx = 0; idx < source->nNumUsed; idx++) {
				s = source->arPacked + idx;
				if (UNEXPECTED(C_TYPE_P(s) == IS_UNDEF)) {
					continue;
				}
				t = crex_hash_index_update(target, idx, s);
				if (pCopyConstructor) {
					pCopyConstructor(t);
				}
			}
			return;
		}

		for (idx = 0; idx < source->nNumUsed; idx++) {
			p = source->arData + idx;
			s = &p->val;
			if (UNEXPECTED(C_TYPE_P(s) == IS_INDIRECT)) {
				s = C_INDIRECT_P(s);
			}
			if (UNEXPECTED(C_TYPE_P(s) == IS_UNDEF)) {
				continue;
			}
			if (p->key) {
				t = _crex_hash_add_or_update_i(target, p->key, s, HASH_UPDATE | HASH_UPDATE_INDIRECT);
				if (pCopyConstructor) {
					pCopyConstructor(t);
				}
			} else {
				t = crex_hash_index_update(target, p->h, s);
				if (pCopyConstructor) {
					pCopyConstructor(t);
				}
			}
		}
	} else {
		if (HT_IS_PACKED(source)) {
			for (idx = 0; idx < source->nNumUsed; idx++) {
				s = source->arPacked + idx;
				if (UNEXPECTED(C_TYPE_P(s) == IS_UNDEF)) {
					continue;
				}
				t = crex_hash_index_add(target, idx, s);
				if (t && pCopyConstructor) {
					pCopyConstructor(t);
				}
			}
			return;
		}

		for (idx = 0; idx < source->nNumUsed; idx++) {
			p = source->arData + idx;
			s = &p->val;
			if (UNEXPECTED(C_TYPE_P(s) == IS_INDIRECT)) {
				s = C_INDIRECT_P(s);
			}
			if (UNEXPECTED(C_TYPE_P(s) == IS_UNDEF)) {
				continue;
			}
			if (p->key) {
				t = _crex_hash_add_or_update_i(target, p->key, s, HASH_ADD | HASH_UPDATE_INDIRECT);
				if (t && pCopyConstructor) {
					pCopyConstructor(t);
				}
			} else {
				t = crex_hash_index_add(target, p->h, s);
				if (t && pCopyConstructor) {
					pCopyConstructor(t);
				}
			}
		}
	}
}


static bool CREX_FASTCALL crex_hash_replace_checker_wrapper(HashTable *target, zval *source_data, crex_ulong h, crex_string *key, void *pParam, merge_checker_func_t merge_checker_func)
{
	crex_hash_key hash_key;

	hash_key.h = h;
	hash_key.key = key;
	return merge_checker_func(target, source_data, &hash_key, pParam);
}


CREX_API void CREX_FASTCALL crex_hash_merge_ex(HashTable *target, HashTable *source, copy_ctor_func_t pCopyConstructor, merge_checker_func_t pMergeSource, void *pParam)
{
	uint32_t idx;
	Bucket *p;
	zval *t;

	IS_CONSISTENT(source);
	IS_CONSISTENT(target);
	HT_ASSERT_RC1(target);

	CREX_ASSERT(!HT_IS_PACKED(source));
	for (idx = 0; idx < source->nNumUsed; idx++) {
		p = source->arData + idx;
		if (UNEXPECTED(C_TYPE(p->val) == IS_UNDEF)) continue;
		if (crex_hash_replace_checker_wrapper(target, &p->val, p->h, p->key, pParam, pMergeSource)) {
			t = crex_hash_update(target, p->key, &p->val);
			if (pCopyConstructor) {
				pCopyConstructor(t);
			}
		}
	}
}


/* Returns the hash table data if found and NULL if not. */
CREX_API zval* CREX_FASTCALL crex_hash_find(const HashTable *ht, crex_string *key)
{
	Bucket *p;

	IS_CONSISTENT(ht);

	(void)crex_string_hash_val(key);
	p = crex_hash_find_bucket(ht, key);
	return p ? &p->val : NULL;
}

CREX_API zval* CREX_FASTCALL crex_hash_find_known_hash(const HashTable *ht, const crex_string *key)
{
	Bucket *p;

	IS_CONSISTENT(ht);

	p = crex_hash_find_bucket(ht, key);
	return p ? &p->val : NULL;
}

CREX_API zval* CREX_FASTCALL crex_hash_str_find(const HashTable *ht, const char *str, size_t len)
{
	crex_ulong h;
	Bucket *p;

	IS_CONSISTENT(ht);

	h = crex_inline_hash_func(str, len);
	p = crex_hash_str_find_bucket(ht, str, len, h);
	return p ? &p->val : NULL;
}

CREX_API zval* CREX_FASTCALL crex_hash_index_find(const HashTable *ht, crex_ulong h)
{
	Bucket *p;

	IS_CONSISTENT(ht);

	if (HT_IS_PACKED(ht)) {
		if (h < ht->nNumUsed) {
			zval *zv = ht->arPacked + h;

			if (C_TYPE_P(zv) != IS_UNDEF) {
				return zv;
			}
		}
		return NULL;
	}

	p = crex_hash_index_find_bucket(ht, h);
	return p ? &p->val : NULL;
}

CREX_API zval* CREX_FASTCALL _crex_hash_index_find(const HashTable *ht, crex_ulong h)
{
	Bucket *p;

	IS_CONSISTENT(ht);
	CREX_ASSERT(!HT_IS_PACKED(ht));

	p = crex_hash_index_find_bucket(ht, h);
	return p ? &p->val : NULL;
}

CREX_API void CREX_FASTCALL crex_hash_internal_pointer_reset_ex(HashTable *ht, HashPosition *pos)
{
	IS_CONSISTENT(ht);
	HT_ASSERT(ht, &ht->nInternalPointer != pos || GC_REFCOUNT(ht) == 1);
	*pos = _crex_hash_get_valid_pos(ht, 0);
}


/* This function will be extremely optimized by remembering
 * the end of the list
 */
CREX_API void CREX_FASTCALL crex_hash_internal_pointer_end_ex(HashTable *ht, HashPosition *pos)
{
	uint32_t idx;

	IS_CONSISTENT(ht);
	HT_ASSERT(ht, &ht->nInternalPointer != pos || GC_REFCOUNT(ht) == 1);

	idx = ht->nNumUsed;
	if (HT_IS_PACKED(ht)) {
		while (idx > 0) {
			idx--;
			if (C_TYPE(ht->arPacked[idx]) != IS_UNDEF) {
				*pos = idx;
				return;
			}
		}
	} else {
		while (idx > 0) {
			idx--;
			if (C_TYPE(ht->arData[idx].val) != IS_UNDEF) {
				*pos = idx;
				return;
			}
		}
	}
	*pos = ht->nNumUsed;
}


CREX_API crex_result CREX_FASTCALL crex_hash_move_forward_ex(HashTable *ht, HashPosition *pos)
{
	uint32_t idx;

	IS_CONSISTENT(ht);
	HT_ASSERT(ht, &ht->nInternalPointer != pos || GC_REFCOUNT(ht) == 1);

	idx = _crex_hash_get_valid_pos(ht, *pos);
	if (idx < ht->nNumUsed) {
		if (HT_IS_PACKED(ht)) {
			while (1) {
				idx++;
				if (idx >= ht->nNumUsed) {
					*pos = ht->nNumUsed;
					return SUCCESS;
				}
				if (C_TYPE(ht->arPacked[idx]) != IS_UNDEF) {
					*pos = idx;
					return SUCCESS;
				}
			}
		} else {
			while (1) {
				idx++;
				if (idx >= ht->nNumUsed) {
					*pos = ht->nNumUsed;
					return SUCCESS;
				}
				if (C_TYPE(ht->arData[idx].val) != IS_UNDEF) {
					*pos = idx;
					return SUCCESS;
				}
			}
		}
	} else {
		return FAILURE;
	}
}

CREX_API crex_result CREX_FASTCALL crex_hash_move_backwards_ex(HashTable *ht, HashPosition *pos)
{
	uint32_t idx = *pos;

	IS_CONSISTENT(ht);
	HT_ASSERT(ht, &ht->nInternalPointer != pos || GC_REFCOUNT(ht) == 1);

	if (idx < ht->nNumUsed) {
		if (HT_IS_PACKED(ht)) {
			while (idx > 0) {
				idx--;
				if (C_TYPE(ht->arPacked[idx]) != IS_UNDEF) {
					*pos = idx;
					return SUCCESS;
				}
			}
		} else {
			while (idx > 0) {
				idx--;
				if (C_TYPE(ht->arData[idx].val) != IS_UNDEF) {
					*pos = idx;
					return SUCCESS;
				}
			}
		}
		*pos = ht->nNumUsed;
		return SUCCESS;
	} else {
		return FAILURE;
	}
}


/* This function should be made binary safe  */
CREX_API int CREX_FASTCALL crex_hash_get_current_key_ex(const HashTable *ht, crex_string **str_index, crex_ulong *num_index, const HashPosition *pos)
{
	uint32_t idx;
	Bucket *p;

	IS_CONSISTENT(ht);
	idx = _crex_hash_get_valid_pos(ht, *pos);
	if (idx < ht->nNumUsed) {
		if (HT_IS_PACKED(ht)) {
			*num_index = idx;
			return HASH_KEY_IS_LONG;
		}
		p = ht->arData + idx;
		if (p->key) {
			*str_index = p->key;
			return HASH_KEY_IS_STRING;
		} else {
			*num_index = p->h;
			return HASH_KEY_IS_LONG;
		}
	}
	return HASH_KEY_NON_EXISTENT;
}

CREX_API void CREX_FASTCALL crex_hash_get_current_key_zval_ex(const HashTable *ht, zval *key, const HashPosition *pos)
{
	uint32_t idx;
	Bucket *p;

	IS_CONSISTENT(ht);
	idx = _crex_hash_get_valid_pos(ht, *pos);
	if (idx >= ht->nNumUsed) {
		ZVAL_NULL(key);
	} else {
		if (HT_IS_PACKED(ht)) {
			ZVAL_LONG(key, idx);
			return;
		}
		p = ht->arData + idx;
		if (p->key) {
			ZVAL_STR_COPY(key, p->key);
		} else {
			ZVAL_LONG(key, p->h);
		}
	}
}

CREX_API int CREX_FASTCALL crex_hash_get_current_key_type_ex(HashTable *ht, HashPosition *pos)
{
	uint32_t idx;
	Bucket *p;

	IS_CONSISTENT(ht);
	idx = _crex_hash_get_valid_pos(ht, *pos);
	if (idx < ht->nNumUsed) {
		if (HT_IS_PACKED(ht)) {
			return HASH_KEY_IS_LONG;
		}
		p = ht->arData + idx;
		if (p->key) {
			return HASH_KEY_IS_STRING;
		} else {
			return HASH_KEY_IS_LONG;
		}
	}
	return HASH_KEY_NON_EXISTENT;
}


CREX_API zval* CREX_FASTCALL crex_hash_get_current_data_ex(HashTable *ht, HashPosition *pos)
{
	uint32_t idx;
	Bucket *p;

	IS_CONSISTENT(ht);
	idx = _crex_hash_get_valid_pos(ht, *pos);
	if (idx < ht->nNumUsed) {
		if (HT_IS_PACKED(ht)) {
			return &ht->arPacked[idx];
		}
		p = ht->arData + idx;
		return &p->val;
	} else {
		return NULL;
	}
}

CREX_API void crex_hash_bucket_swap(Bucket *p, Bucket *q)
{
	zval val;
	crex_ulong h;
	crex_string *key;

	val = p->val;
	h = p->h;
	key = p->key;

	p->val = q->val;
	p->h = q->h;
	p->key = q->key;

	q->val = val;
	q->h = h;
	q->key = key;
}

CREX_API void crex_hash_bucket_renum_swap(Bucket *p, Bucket *q)
{
	zval val;

	val = p->val;
	p->val = q->val;
	q->val = val;
}

CREX_API void crex_hash_bucket_packed_swap(Bucket *p, Bucket *q)
{
	zval val;
	crex_ulong h;

	val = p->val;
	h = p->h;

	p->val = q->val;
	p->h = q->h;

	q->val = val;
	q->h = h;
}

CREX_API void CREX_FASTCALL crex_hash_sort_ex(HashTable *ht, sort_func_t sort, bucket_compare_func_t compar, bool renumber)
{
	Bucket *p;
	uint32_t i, j;

	IS_CONSISTENT(ht);
	HT_ASSERT_RC1(ht);

	if (!(ht->nNumOfElements>1) && !(renumber && ht->nNumOfElements>0)) {
		/* Doesn't require sorting */
		return;
	}

	if (HT_IS_PACKED(ht)) {
		crex_hash_packed_to_hash(ht); // TODO: ???
	}

	if (HT_IS_WITHOUT_HOLES(ht)) {
		/* Store original order of elements in extra space to allow stable sorting. */
		for (i = 0; i < ht->nNumUsed; i++) {
			C_EXTRA(ht->arData[i].val) = i;
		}
	} else {
		/* Remove holes and store original order. */
		for (j = 0, i = 0; j < ht->nNumUsed; j++) {
			p = ht->arData + j;
			if (UNEXPECTED(C_TYPE(p->val) == IS_UNDEF)) continue;
			if (i != j) {
				ht->arData[i] = *p;
			}
			C_EXTRA(ht->arData[i].val) = i;
			i++;
		}
		ht->nNumUsed = i;
	}

	if (!HT_IS_PACKED(ht)) {
		/* We broke the hash collisions chains overriding C_NEXT() by C_EXTRA().
		 * Reset the hash headers table as well to avoid possible inconsistent
		 * access on recursive data structures.
	     *
	     * See Crex/tests/bug63882_2.crxt
		 */
		HT_HASH_RESET(ht);
	}

	sort((void *)ht->arData, ht->nNumUsed, sizeof(Bucket), (compare_func_t) compar,
			(swap_func_t)(renumber? crex_hash_bucket_renum_swap :
				(HT_IS_PACKED(ht) ? crex_hash_bucket_packed_swap : crex_hash_bucket_swap)));

	ht->nInternalPointer = 0;

	if (renumber) {
		for (j = 0; j < i; j++) {
			p = ht->arData + j;
			p->h = j;
			if (p->key) {
				crex_string_release(p->key);
				p->key = NULL;
			}
		}

		ht->nNextFreeElement = i;
	}
	if (HT_IS_PACKED(ht)) {
		if (!renumber) {
			crex_hash_packed_to_hash(ht);
		}
	} else {
		if (renumber) {
			void *new_data, *old_data = HT_GET_DATA_ADDR(ht);
			Bucket *old_buckets = ht->arData;
			zval *zv;

			new_data = pemalloc(HT_PACKED_SIZE_EX(ht->nTableSize, HT_MIN_MASK), (GC_FLAGS(ht) & IS_ARRAY_PERSISTENT));
			HT_FLAGS(ht) |= HASH_FLAG_PACKED | HASH_FLAG_STATIC_KEYS;
			ht->nTableMask = HT_MIN_MASK;
			HT_SET_DATA_ADDR(ht, new_data);
			p = old_buckets;
			zv = ht->arPacked;
			for (i = 0; i < ht->nTableSize; i++) {
				ZVAL_COPY_VALUE(zv, &p->val);
				zv++;
				p++;
			}
			pefree(old_data, GC_FLAGS(ht) & IS_ARRAY_PERSISTENT);
			HT_HASH_RESET_PACKED(ht);
		} else {
			crex_hash_rehash(ht);
		}
	}
}

static crex_always_inline int crex_hash_compare_impl(HashTable *ht1, HashTable *ht2, compare_func_t compar, bool ordered) {
	uint32_t idx1, idx2;
	crex_string *key1, *key2;
	crex_ulong h1, h2;
	zval *pData1, *pData2;;
	int result;

	if (ht1->nNumOfElements != ht2->nNumOfElements) {
		return ht1->nNumOfElements > ht2->nNumOfElements ? 1 : -1;
	}

	for (idx1 = 0, idx2 = 0; idx1 < ht1->nNumUsed; idx1++) {
		if (HT_IS_PACKED(ht1)) {
			pData1 = ht1->arPacked + idx1;
			h1 = idx1;
			key1 = NULL;
		} else {
			Bucket *p = ht1->arData + idx1;
			pData1 = &p->val;
			h1 = p->h;
			key1 = p->key;
		}

		if (C_TYPE_P(pData1) == IS_UNDEF) continue;
		if (ordered) {
			if (HT_IS_PACKED(ht2)) {
				while (1) {
					CREX_ASSERT(idx2 != ht2->nNumUsed);
					pData2 = ht2->arPacked + idx2;
					h2 = idx2;
					key2 = NULL;
					if (C_TYPE_P(pData2) != IS_UNDEF) break;
					idx2++;
				}
			} else {
				while (1) {
					Bucket *p;
					CREX_ASSERT(idx2 != ht2->nNumUsed);
					p = ht2->arData + idx2;
					pData2 = &p->val;
					h2 = p->h;
					key2 = p->key;
					if (C_TYPE_P(pData2) != IS_UNDEF) break;
					idx2++;
				}
			}
			if (key1 == NULL && key2 == NULL) { /* numeric indices */
				if (h1 != h2) {
					return h1 > h2 ? 1 : -1;
				}
			} else if (key1 != NULL && key2 != NULL) { /* string indices */
				if (ZSTR_LEN(key1) != ZSTR_LEN(key2)) {
					return ZSTR_LEN(key1) > ZSTR_LEN(key2) ? 1 : -1;
				}

				result = memcmp(ZSTR_VAL(key1), ZSTR_VAL(key2), ZSTR_LEN(key1));
				if (result != 0) {
					return result;
				}
			} else {
				/* Mixed key types: A string key is considered as larger */
				return key1 != NULL ? 1 : -1;
			}
			idx2++;
		} else {
			if (key1 == NULL) { /* numeric index */
				pData2 = crex_hash_index_find(ht2, h1);
				if (pData2 == NULL) {
					return 1;
				}
			} else { /* string index */
				pData2 = crex_hash_find(ht2, key1);
				if (pData2 == NULL) {
					return 1;
				}
			}
		}

		if (C_TYPE_P(pData1) == IS_INDIRECT) {
			pData1 = C_INDIRECT_P(pData1);
		}
		if (C_TYPE_P(pData2) == IS_INDIRECT) {
			pData2 = C_INDIRECT_P(pData2);
		}

		if (C_TYPE_P(pData1) == IS_UNDEF) {
			if (C_TYPE_P(pData2) != IS_UNDEF) {
				return -1;
			}
		} else if (C_TYPE_P(pData2) == IS_UNDEF) {
			return 1;
		} else {
			result = compar(pData1, pData2);
			if (result != 0) {
				return result;
			}
		}
	}

	return 0;
}

CREX_API int crex_hash_compare(HashTable *ht1, HashTable *ht2, compare_func_t compar, bool ordered)
{
	int result;
	IS_CONSISTENT(ht1);
	IS_CONSISTENT(ht2);

	if (ht1 == ht2) {
		return 0;
	}

	/* It's enough to protect only one of the arrays.
	 * The second one may be referenced from the first and this may cause
	 * false recursion detection.
	 */
	if (UNEXPECTED(GC_IS_RECURSIVE(ht1))) {
		crex_error_noreturn(E_ERROR, "Nesting level too deep - recursive dependency?");
	}

	GC_TRY_PROTECT_RECURSION(ht1);
	result = crex_hash_compare_impl(ht1, ht2, compar, ordered);
	GC_TRY_UNPROTECT_RECURSION(ht1);

	return result;
}


CREX_API zval* CREX_FASTCALL crex_hash_minmax(const HashTable *ht, compare_func_t compar, uint32_t flag)
{
	uint32_t idx;
	zval *res;

	IS_CONSISTENT(ht);

	if (ht->nNumOfElements == 0 ) {
		return NULL;
	}

	if (HT_IS_PACKED(ht)) {
		zval *zv;

		idx = 0;
		while (1) {
			if (idx == ht->nNumUsed) {
				return NULL;
			}
			if (C_TYPE(ht->arPacked[idx]) != IS_UNDEF) break;
			idx++;
		}
		res = ht->arPacked + idx;
		for (; idx < ht->nNumUsed; idx++) {
			zv = ht->arPacked + idx;
			if (UNEXPECTED(C_TYPE_P(zv) == IS_UNDEF)) continue;

			if (flag) {
				if (compar(res, zv) < 0) { /* max */
					res = zv;
				}
			} else {
				if (compar(res, zv) > 0) { /* min */
					res = zv;
				}
			}
		}
	} else {
		Bucket *p;

		idx = 0;
		while (1) {
			if (idx == ht->nNumUsed) {
				return NULL;
			}
			if (C_TYPE(ht->arData[idx].val) != IS_UNDEF) break;
			idx++;
		}
		res = &ht->arData[idx].val;
		for (; idx < ht->nNumUsed; idx++) {
			p = ht->arData + idx;
			if (UNEXPECTED(C_TYPE(p->val) == IS_UNDEF)) continue;

			if (flag) {
				if (compar(res, &p->val) < 0) { /* max */
					res = &p->val;
				}
			} else {
				if (compar(res, &p->val) > 0) { /* min */
					res = &p->val;
				}
			}
		}
	}
	return res;
}

CREX_API bool CREX_FASTCALL _crex_handle_numeric_str_ex(const char *key, size_t length, crex_ulong *idx)
{
	const char *tmp = key;

	const char *end = key + length;

	if (*tmp == '-') {
		tmp++;
	}

	if ((*tmp == '0' && length > 1) /* numbers with leading zeros */
	 || (end - tmp > MAX_LENGTH_OF_LONG - 1) /* number too long */
	 || (SIZEOF_CREX_LONG == 4 &&
	     end - tmp == MAX_LENGTH_OF_LONG - 1 &&
	     *tmp > '2')) { /* overflow */
		return 0;
	}
	*idx = (*tmp - '0');
	while (1) {
		++tmp;
		if (tmp == end) {
			if (*key == '-') {
				if (*idx-1 > CREX_LONG_MAX) { /* overflow */
					return 0;
				}
				*idx = 0 - *idx;
			} else if (*idx > CREX_LONG_MAX) { /* overflow */
				return 0;
			}
			return 1;
		}
		if (*tmp <= '9' && *tmp >= '0') {
			*idx = (*idx * 10) + (*tmp - '0');
		} else {
			return 0;
		}
	}
}

/* Takes a "symtable" hashtable (contains integer and non-numeric string keys)
 * and converts it to a "proptable" (contains only string keys).
 * If the symtable didn't need duplicating, its refcount is incremented.
 */
CREX_API HashTable* CREX_FASTCALL crex_symtable_to_proptable(HashTable *ht)
{
	crex_ulong num_key;
	crex_string *str_key;
	zval *zv;

	if (UNEXPECTED(HT_IS_PACKED(ht))) {
		goto convert;
	}

	CREX_HASH_MAP_FOREACH_STR_KEY(ht, str_key) {
		if (!str_key) {
			goto convert;
		}
	} CREX_HASH_FOREACH_END();

	if (!(GC_FLAGS(ht) & IS_ARRAY_IMMUTABLE)) {
		GC_ADDREF(ht);
	}

	return ht;

convert:
	{
		HashTable *new_ht = crex_new_array(crex_hash_num_elements(ht));

		CREX_HASH_FOREACH_KEY_VAL(ht, num_key, str_key, zv) {
			if (!str_key) {
				str_key = crex_long_to_str(num_key);
				crex_string_delref(str_key);
			}
			do {
				if (C_OPT_REFCOUNTED_P(zv)) {
					if (C_ISREF_P(zv) && C_REFCOUNT_P(zv) == 1) {
						zv = C_REFVAL_P(zv);
						if (!C_OPT_REFCOUNTED_P(zv)) {
							break;
						}
					}
					C_ADDREF_P(zv);
				}
			} while (0);
			crex_hash_update(new_ht, str_key, zv);
		} CREX_HASH_FOREACH_END();

		return new_ht;
	}
}

/* Takes a "proptable" hashtable (contains only string keys) and converts it to
 * a "symtable" (contains integer and non-numeric string keys).
 * If the proptable didn't need duplicating, its refcount is incremented.
 */
CREX_API HashTable* CREX_FASTCALL crex_proptable_to_symtable(HashTable *ht, bool always_duplicate)
{
	crex_ulong num_key;
	crex_string *str_key;
	zval *zv;

	if (!HT_IS_PACKED(ht)) {
		CREX_HASH_MAP_FOREACH_STR_KEY(ht, str_key) {
			/* The `str_key &&` here might seem redundant: property tables should
			 * only have string keys. Unfortunately, this isn't true, at the very
			 * least because of ArrayObject, which stores a symtable where the
			 * property table should be.
			 */
			if (str_key && CREX_HANDLE_NUMERIC(str_key, num_key)) {
				goto convert;
			}
		} CREX_HASH_FOREACH_END();
	}

	if (always_duplicate) {
		return crex_array_dup(ht);
	}

	if (EXPECTED(!(GC_FLAGS(ht) & IS_ARRAY_IMMUTABLE))) {
		GC_ADDREF(ht);
	}

	return ht;

convert:
	{
		HashTable *new_ht = crex_new_array(crex_hash_num_elements(ht));

		CREX_HASH_MAP_FOREACH_KEY_VAL_IND(ht, num_key, str_key, zv) {
			do {
				if (C_OPT_REFCOUNTED_P(zv)) {
					if (C_ISREF_P(zv) && C_REFCOUNT_P(zv) == 1) {
						zv = C_REFVAL_P(zv);
						if (!C_OPT_REFCOUNTED_P(zv)) {
							break;
						}
					}
					C_ADDREF_P(zv);
				}
			} while (0);
			/* Again, thank ArrayObject for `!str_key ||`. */
			if (!str_key || CREX_HANDLE_NUMERIC(str_key, num_key)) {
				crex_hash_index_update(new_ht, num_key, zv);
			} else {
				crex_hash_update(new_ht, str_key, zv);
			}
		} CREX_HASH_FOREACH_END();

		return new_ht;
	}
}
