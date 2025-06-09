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

#ifndef CREX_ALLOC_H
#define CREX_ALLOC_H

#include <stdio.h>

#include "../TSRM/TSRM.h"
#include "crex.h"

#ifndef CREX_MM_ALIGNMENT
# error "CREX_MM_ALIGNMENT was not defined during configure"
#endif

#define CREX_MM_ALIGNMENT_MASK ~(CREX_MM_ALIGNMENT - 1)

#define CREX_MM_ALIGNED_SIZE(size)	(((size) + CREX_MM_ALIGNMENT - 1) & CREX_MM_ALIGNMENT_MASK)

#define CREX_MM_ALIGNED_SIZE_EX(size, alignment) \
	(((size) + ((alignment) - 1)) & ~((alignment) - 1))

typedef struct _crex_leak_info {
	void *addr;
	size_t size;
	const char *filename;
	const char *orig_filename;
	uint32_t lineno;
	uint32_t orig_lineno;
} crex_leak_info;

#if CREX_DEBUG
typedef struct _crex_mm_debug_info {
	size_t             size;
	const char        *filename;
	const char        *orig_filename;
	uint32_t               lineno;
	uint32_t               orig_lineno;
} crex_mm_debug_info;

# define CREX_MM_OVERHEAD CREX_MM_ALIGNED_SIZE(sizeof(crex_mm_debug_info))
#else
# define CREX_MM_OVERHEAD 0
#endif

BEGIN_EXTERN_C()

CREX_API CREX_ATTRIBUTE_MALLOC char*  CREX_FASTCALL crex_strndup(const char *s, size_t length);

CREX_API CREX_ATTRIBUTE_MALLOC void*  CREX_FASTCALL _emalloc(size_t size CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC) CREX_ATTRIBUTE_ALLOC_SIZE(1);
CREX_API CREX_ATTRIBUTE_MALLOC void*  CREX_FASTCALL _safe_emalloc(size_t nmemb, size_t size, size_t offset CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC);
CREX_API CREX_ATTRIBUTE_MALLOC void*  CREX_FASTCALL _safe_malloc(size_t nmemb, size_t size, size_t offset);
CREX_API void   CREX_FASTCALL _efree(void *ptr CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC);
CREX_API CREX_ATTRIBUTE_MALLOC void*  CREX_FASTCALL _ecalloc(size_t nmemb, size_t size CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC) CREX_ATTRIBUTE_ALLOC_SIZE2(1,2);
CREX_API void*  CREX_FASTCALL _erealloc(void *ptr, size_t size CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC) CREX_ATTRIBUTE_ALLOC_SIZE(2);
CREX_API void*  CREX_FASTCALL _erealloc2(void *ptr, size_t size, size_t copy_size CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC) CREX_ATTRIBUTE_ALLOC_SIZE(2);
CREX_API void*  CREX_FASTCALL _safe_erealloc(void *ptr, size_t nmemb, size_t size, size_t offset CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC);
CREX_API void*  CREX_FASTCALL _safe_realloc(void *ptr, size_t nmemb, size_t size, size_t offset);
CREX_API CREX_ATTRIBUTE_MALLOC char*  CREX_FASTCALL _estrdup(const char *s CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC);
CREX_API CREX_ATTRIBUTE_MALLOC char*  CREX_FASTCALL _estrndup(const char *s, size_t length CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC);
CREX_API size_t CREX_FASTCALL _crex_mem_block_size(void *ptr CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC);

#include "crex_alloc_sizes.h"

/* _emalloc() & _efree() specialization */
#if !CREX_DEBUG && defined(HAVE_BUILTIN_CONSTANT_P)

# define _CREX_BIN_ALLOCATOR_DEF(_num, _size, _elements, _pages, x, y) \
	CREX_API CREX_ATTRIBUTE_MALLOC void* CREX_FASTCALL _emalloc_  ## _size(void);

CREX_MM_BINS_INFO(_CREX_BIN_ALLOCATOR_DEF, x, y)

CREX_API CREX_ATTRIBUTE_MALLOC void* CREX_FASTCALL _emalloc_large(size_t size) CREX_ATTRIBUTE_ALLOC_SIZE(1);
CREX_API CREX_ATTRIBUTE_MALLOC void* CREX_FASTCALL _emalloc_huge(size_t size) CREX_ATTRIBUTE_ALLOC_SIZE(1);

# define _CREX_BIN_ALLOCATOR_SELECTOR_START(_num, _size, _elements, _pages, size, y) \
	((size <= _size) ? _emalloc_ ## _size() :
# define _CREX_BIN_ALLOCATOR_SELECTOR_END(_num, _size, _elements, _pages, size, y) \
	)

# define CREX_ALLOCATOR(size) \
	CREX_MM_BINS_INFO(_CREX_BIN_ALLOCATOR_SELECTOR_START, size, y) \
	((size <= CREX_MM_MAX_LARGE_SIZE) ? _emalloc_large(size) : _emalloc_huge(size)) \
	CREX_MM_BINS_INFO(_CREX_BIN_ALLOCATOR_SELECTOR_END, size, y)

# define _emalloc(size) \
	(__builtin_constant_p(size) ? \
		CREX_ALLOCATOR(size) \
	: \
		_emalloc(size) \
	)

# define _CREX_BIN_DEALLOCATOR_DEF(_num, _size, _elements, _pages, x, y) \
	CREX_API void CREX_FASTCALL _efree_ ## _size(void *);

CREX_MM_BINS_INFO(_CREX_BIN_DEALLOCATOR_DEF, x, y)

CREX_API void CREX_FASTCALL _efree_large(void *, size_t size);
CREX_API void CREX_FASTCALL _efree_huge(void *, size_t size);

# define _CREX_BIN_DEALLOCATOR_SELECTOR_START(_num, _size, _elements, _pages, ptr, size) \
	if (size <= _size) { _efree_ ## _size(ptr); } else

# define CREX_DEALLOCATOR(ptr, size) \
	CREX_MM_BINS_INFO(_CREX_BIN_DEALLOCATOR_SELECTOR_START, ptr, size) \
	if (size <= CREX_MM_MAX_LARGE_SIZE) { _efree_large(ptr, size); } \
	else { _efree_huge(ptr, size); }

# define efree_size(ptr, size) do { \
		if (__builtin_constant_p(size)) { \
			CREX_DEALLOCATOR(ptr, size) \
		} else { \
			_efree(ptr); \
		} \
	} while (0)
# define efree_size_rel(ptr, size) \
	efree_size(ptr, size)

#else

# define efree_size(ptr, size) \
	efree(ptr)
# define efree_size_rel(ptr, size) \
	efree_rel(ptr)

#define _emalloc_large _emalloc
#define _emalloc_huge  _emalloc
#define _efree_large   _efree
#define _efree_huge    _efree

#endif

/* Standard wrapper macros */
#define emalloc(size)						_emalloc((size) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define emalloc_large(size)					_emalloc_large((size) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define emalloc_huge(size)					_emalloc_huge((size) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define safe_emalloc(nmemb, size, offset)	_safe_emalloc((nmemb), (size), (offset) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define efree(ptr)							_efree((ptr) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define efree_large(ptr)					_efree_large((ptr) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define efree_huge(ptr)						_efree_huge((ptr) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define ecalloc(nmemb, size)				_ecalloc((nmemb), (size) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define erealloc(ptr, size)					_erealloc((ptr), (size) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define erealloc2(ptr, size, copy_size)		_erealloc2((ptr), (size), (copy_size) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define safe_erealloc(ptr, nmemb, size, offset)	_safe_erealloc((ptr), (nmemb), (size), (offset) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define erealloc_recoverable(ptr, size)		_erealloc((ptr), (size) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define erealloc2_recoverable(ptr, size, copy_size) _erealloc2((ptr), (size), (copy_size) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define estrdup(s)							_estrdup((s) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define estrndup(s, length)					_estrndup((s), (length) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define crex_mem_block_size(ptr)			_crex_mem_block_size((ptr) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)

/* Relay wrapper macros */
#define emalloc_rel(size)						_emalloc((size) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)
#define safe_emalloc_rel(nmemb, size, offset)	_safe_emalloc((nmemb), (size), (offset) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)
#define efree_rel(ptr)							_efree((ptr) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)
#define ecalloc_rel(nmemb, size)				_ecalloc((nmemb), (size) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)
#define erealloc_rel(ptr, size)					_erealloc((ptr), (size) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)
#define erealloc2_rel(ptr, size, copy_size)		_erealloc2((ptr), (size), (copy_size) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)
#define erealloc_recoverable_rel(ptr, size)		_erealloc((ptr), (size) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)
#define erealloc2_recoverable_rel(ptr, size, copy_size) _erealloc2((ptr), (size), (copy_size) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)
#define safe_erealloc_rel(ptr, nmemb, size, offset)	_safe_erealloc((ptr), (nmemb), (size), (offset) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)
#define estrdup_rel(s)							_estrdup((s) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)
#define estrndup_rel(s, length)					_estrndup((s), (length) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)
#define crex_mem_block_size_rel(ptr)			_crex_mem_block_size((ptr) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)

CREX_API CREX_ATTRIBUTE_MALLOC void * __crex_malloc(size_t len) CREX_ATTRIBUTE_ALLOC_SIZE(1);
CREX_API CREX_ATTRIBUTE_MALLOC void * __crex_calloc(size_t nmemb, size_t len) CREX_ATTRIBUTE_ALLOC_SIZE2(1,2);
CREX_API void * __crex_realloc(void *p, size_t len) CREX_ATTRIBUTE_ALLOC_SIZE(2);
CREX_API CREX_ATTRIBUTE_MALLOC char * __crex_strdup(const char *s);

/* Selective persistent/non persistent allocation macros */
#define pemalloc(size, persistent) ((persistent)?__crex_malloc(size):emalloc(size))
#define safe_pemalloc(nmemb, size, offset, persistent)	((persistent)?_safe_malloc(nmemb, size, offset):safe_emalloc(nmemb, size, offset))
#define pefree(ptr, persistent)  ((persistent)?free(ptr):efree(ptr))
#define pefree_size(ptr, size, persistent)  do { \
		if (persistent) { \
			free(ptr); \
		} else { \
			efree_size(ptr, size);\
		} \
	} while (0)

#define pecalloc(nmemb, size, persistent) ((persistent)?__crex_calloc((nmemb), (size)):ecalloc((nmemb), (size)))
#define perealloc(ptr, size, persistent) ((persistent)?__crex_realloc((ptr), (size)):erealloc((ptr), (size)))
#define perealloc2(ptr, size, copy_size, persistent) ((persistent)?__crex_realloc((ptr), (size)):erealloc2((ptr), (size), (copy_size)))
#define safe_perealloc(ptr, nmemb, size, offset, persistent)	((persistent)?_safe_realloc((ptr), (nmemb), (size), (offset)):safe_erealloc((ptr), (nmemb), (size), (offset)))
#define perealloc_recoverable(ptr, size, persistent) ((persistent)?realloc((ptr), (size)):erealloc_recoverable((ptr), (size)))
#define perealloc2_recoverable(ptr, size, persistent) ((persistent)?realloc((ptr), (size)):erealloc2_recoverable((ptr), (size), (copy_size)))
#define pestrdup(s, persistent) ((persistent)?__crex_strdup(s):estrdup(s))
#define pestrndup(s, length, persistent) ((persistent)?crex_strndup((s),(length)):estrndup((s),(length)))

#define pemalloc_rel(size, persistent) ((persistent)?__crex_malloc(size):emalloc_rel(size))
#define pefree_rel(ptr, persistent)	((persistent)?free(ptr):efree_rel(ptr))
#define pecalloc_rel(nmemb, size, persistent) ((persistent)?__crex_calloc((nmemb), (size)):ecalloc_rel((nmemb), (size)))
#define perealloc_rel(ptr, size, persistent) ((persistent)?__crex_realloc((ptr), (size)):erealloc_rel((ptr), (size)))
#define perealloc2_rel(ptr, size, copy_size, persistent) ((persistent)?__crex_realloc((ptr), (size)):erealloc2_rel((ptr), (size), (copy_size)))
#define perealloc_recoverable_rel(ptr, size, persistent) ((persistent)?realloc((ptr), (size)):erealloc_recoverable_rel((ptr), (size)))
#define perealloc2_recoverable_rel(ptr, size, copy_size, persistent) ((persistent)?realloc((ptr), (size)):erealloc2_recoverable_rel((ptr), (size), (copy_size)))
#define pestrdup_rel(s, persistent) ((persistent)?strdup(s):estrdup_rel(s))

CREX_API crex_result crex_set_memory_limit(size_t memory_limit);
CREX_API bool crex_alloc_in_memory_limit_error_reporting(void);

CREX_API void start_memory_manager(void);
CREX_API void shutdown_memory_manager(bool silent, bool full_shutdown);
CREX_API bool is_crex_mm(void);
CREX_API bool is_crex_ptr(const void *ptr);

CREX_API size_t crex_memory_usage(bool real_usage);
CREX_API size_t crex_memory_peak_usage(bool real_usage);
CREX_API void crex_memory_reset_peak_usage(void);

/* fast cache for HashTables */
#define ALLOC_HASHTABLE(ht)	\
	(ht) = (HashTable *) emalloc(sizeof(HashTable))

#define FREE_HASHTABLE(ht)	\
	efree_size(ht, sizeof(HashTable))

#define ALLOC_HASHTABLE_REL(ht)	\
	(ht) = (HashTable *) emalloc_rel(sizeof(HashTable))

#define FREE_HASHTABLE_REL(ht)	\
	efree_size_rel(ht, sizeof(HashTable))

/* Heap functions */
typedef struct _crex_mm_heap crex_mm_heap;

CREX_API crex_mm_heap *crex_mm_startup(void);
CREX_API void crex_mm_shutdown(crex_mm_heap *heap, bool full_shutdown, bool silent);
CREX_API CREX_ATTRIBUTE_MALLOC void*  CREX_FASTCALL _crex_mm_alloc(crex_mm_heap *heap, size_t size CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC) CREX_ATTRIBUTE_ALLOC_SIZE(2);
CREX_API void   CREX_FASTCALL _crex_mm_free(crex_mm_heap *heap, void *p CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC);
CREX_API void*  CREX_FASTCALL _crex_mm_realloc(crex_mm_heap *heap, void *p, size_t size CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC);
CREX_API void*  CREX_FASTCALL _crex_mm_realloc2(crex_mm_heap *heap, void *p, size_t size, size_t copy_size CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC);
CREX_API size_t CREX_FASTCALL _crex_mm_block_size(crex_mm_heap *heap, void *p CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC);

#define crex_mm_alloc(heap, size)			_crex_mm_alloc((heap), (size) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define crex_mm_free(heap, p)				_crex_mm_free((heap), (p) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define crex_mm_realloc(heap, p, size)		_crex_mm_realloc((heap), (p), (size) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define crex_mm_realloc2(heap, p, size, copy_size) _crex_mm_realloc2((heap), (p), (size), (copy_size) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)
#define crex_mm_block_size(heap, p)			_crex_mm_block_size((heap), (p) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)

#define crex_mm_alloc_rel(heap, size)		_crex_mm_alloc((heap), (size) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)
#define crex_mm_free_rel(heap, p)			_crex_mm_free((heap), (p) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)
#define crex_mm_realloc_rel(heap, p, size)	_crex_mm_realloc((heap), (p), (size) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)
#define crex_mm_realloc2_rel(heap, p, size, copy_size) _crex_mm_realloc2((heap), (p), (size), (copy_size) CREX_FILE_LINE_RELAY_CC CREX_FILE_LINE_CC)
#define crex_mm_block_size_rel(heap, p)		_crex_mm_block_size((heap), (p) CREX_FILE_LINE_CC CREX_FILE_LINE_EMPTY_CC)

CREX_API crex_mm_heap *crex_mm_set_heap(crex_mm_heap *new_heap);
CREX_API crex_mm_heap *crex_mm_get_heap(void);

CREX_API size_t crex_mm_gc(crex_mm_heap *heap);

#define CREX_MM_CUSTOM_HEAP_NONE  0
#define CREX_MM_CUSTOM_HEAP_STD   1
#define CREX_MM_CUSTOM_HEAP_DEBUG 2

CREX_API bool crex_mm_is_custom_heap(crex_mm_heap *new_heap);
CREX_API void crex_mm_set_custom_handlers(crex_mm_heap *heap,
                                          void* (*_malloc)(size_t),
                                          void  (*_free)(void*),
                                          void* (*_realloc)(void*, size_t));
CREX_API void crex_mm_get_custom_handlers(crex_mm_heap *heap,
                                          void* (**_malloc)(size_t),
                                          void  (**_free)(void*),
                                          void* (**_realloc)(void*, size_t));

#if CREX_DEBUG
CREX_API void crex_mm_set_custom_debug_handlers(crex_mm_heap *heap,
                                          void* (*_malloc)(size_t CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC),
                                          void  (*_free)(void* CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC),
                                          void* (*_realloc)(void*, size_t CREX_FILE_LINE_DC CREX_FILE_LINE_ORIG_DC));
#endif

typedef struct _crex_mm_storage crex_mm_storage;

typedef	void* (*crex_mm_chunk_alloc_t)(crex_mm_storage *storage, size_t size, size_t alignment);
typedef void  (*crex_mm_chunk_free_t)(crex_mm_storage *storage, void *chunk, size_t size);
typedef bool   (*crex_mm_chunk_truncate_t)(crex_mm_storage *storage, void *chunk, size_t old_size, size_t new_size);
typedef bool   (*crex_mm_chunk_extend_t)(crex_mm_storage *storage, void *chunk, size_t old_size, size_t new_size);

typedef struct _crex_mm_handlers {
	crex_mm_chunk_alloc_t       chunk_alloc;
	crex_mm_chunk_free_t        chunk_free;
	crex_mm_chunk_truncate_t    chunk_truncate;
	crex_mm_chunk_extend_t      chunk_extend;
} crex_mm_handlers;

struct _crex_mm_storage {
	const crex_mm_handlers handlers;
	void *data;
};

CREX_API crex_mm_storage *crex_mm_get_storage(crex_mm_heap *heap);
CREX_API crex_mm_heap *crex_mm_startup_ex(const crex_mm_handlers *handlers, void *data, size_t data_size);

/*

// The following example shows how to use crex_mm_heap API with custom storage

static crex_mm_heap *apc_heap = NULL;
static HashTable    *apc_ht = NULL;

typedef struct _apc_data {
	void     *mem;
	uint32_t  free_pages;
} apc_data;

static void *apc_chunk_alloc(crex_mm_storage *storage, size_t size, size_t alignment)
{
	apc_data *data = (apc_data*)(storage->data);
	size_t real_size = ((size + (CREX_MM_CHUNK_SIZE-1)) & ~(CREX_MM_CHUNK_SIZE-1));
	uint32_t count = real_size / CREX_MM_CHUNK_SIZE;
	uint32_t first, last, i;

	CREX_ASSERT(alignment == CREX_MM_CHUNK_SIZE);

	for (first = 0; first < 32; first++) {
		if (!(data->free_pages & (1 << first))) {
			last = first;
			do {
				if (last - first == count - 1) {
					for (i = first; i <= last; i++) {
						data->free_pages |= (1 << i);
					}
					return (void *)(((char*)(data->mem)) + CREX_MM_CHUNK_SIZE * (1 << first));
				}
				last++;
			} while (last < 32 && !(data->free_pages & (1 << last)));
			first = last;
		}
	}
	return NULL;
}

static void apc_chunk_free(crex_mm_storage *storage, void *chunk, size_t size)
{
	apc_data *data = (apc_data*)(storage->data);
	uint32_t i;

	CREX_ASSERT(((uintptr_t)chunk & (CREX_MM_CHUNK_SIZE - 1)) == 0);

	i = ((uintptr_t)chunk - (uintptr_t)(data->mem)) / CREX_MM_CHUNK_SIZE;
	while (1) {
		data->free_pages &= ~(1 << i);
		if (size <= CREX_MM_CHUNK_SIZE) {
			break;
		}
		size -= CREX_MM_CHUNK_SIZE;
	}
}

static void apc_init_heap(void)
{
	crex_mm_handlers apc_handlers = {
		apc_chunk_alloc,
		apc_chunk_free,
		NULL,
		NULL,
	};
	apc_data tmp_data;
	crex_mm_heap *old_heap;

	// Preallocate properly aligned SHM chunks (64MB)
	tmp_data.mem = shm_memalign(CREX_MM_CHUNK_SIZE, CREX_MM_CHUNK_SIZE * 32);

	// Initialize temporary storage data
	tmp_data.free_pages = 0;

	// Create heap
	apc_heap = crex_mm_startup_ex(&apc_handlers, &tmp_data, sizeof(tmp_data));

	// Allocate some data in the heap
	old_heap = crex_mm_set_heap(apc_heap);
	ALLOC_HASHTABLE(apc_ht);
	crex_hash_init(apc_ht, 64, NULL, ZVAL_PTR_DTOR, 0);
	crex_mm_set_heap(old_heap);
}

*/

#ifdef ZTS
size_t crex_mm_globals_size(void);
#endif

END_EXTERN_C()

#endif
