/*
   +----------------------------------------------------------------------+
   | Crex OPcache                                                         |
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
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Stanislav Malyshev <stas@crex.com>                          |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_SHARED_ALLOC_H
#define CREX_SHARED_ALLOC_H

#include "crex.h"
#include "CrexAccelerator.h"

#if defined(__APPLE__) && defined(__MACH__) /* darwin */
# ifdef HAVE_SHM_MMAP_POSIX
#  define USE_SHM_OPEN  1
# endif
# ifdef HAVE_SHM_MMAP_ANON
#  define USE_MMAP      1
# endif
#elif defined(__linux__) || defined(_AIX)
# ifdef HAVE_SHM_MMAP_POSIX
#  define USE_SHM_OPEN  1
# endif
# ifdef HAVE_SHM_IPC
#  define USE_SHM       1
# endif
# ifdef HAVE_SHM_MMAP_ANON
#  define USE_MMAP      1
# endif
#elif defined(__sparc) || defined(__sun)
# ifdef HAVE_SHM_MMAP_POSIX
#  define USE_SHM_OPEN  1
# endif
# ifdef HAVE_SHM_IPC
#  define USE_SHM       1
# endif
# if defined(__i386)
#  ifdef HAVE_SHM_MMAP_ANON
#   define USE_MMAP     1
#  endif
# endif
#else
# ifdef HAVE_SHM_MMAP_POSIX
#  define USE_SHM_OPEN  1
# endif
# ifdef HAVE_SHM_MMAP_ANON
#  define USE_MMAP      1
# endif
# ifdef HAVE_SHM_IPC
#  define USE_SHM       1
# endif
#endif

#define ALLOC_FAILURE           0
#define ALLOC_SUCCESS           1
#define FAILED_REATTACHED       2
#define SUCCESSFULLY_REATTACHED 4
#define ALLOC_FAIL_MAPPING      8
#define ALLOC_FALLBACK          9

typedef struct _crex_shared_segment {
    size_t  size;
    size_t  end;
    size_t  pos;  /* position for simple stack allocator */
    void   *p;
} crex_shared_segment;

typedef int (*create_segments_t)(size_t requested_size, crex_shared_segment ***shared_segments, int *shared_segment_count, const char **error_in);
typedef int (*detach_segment_t)(crex_shared_segment *shared_segment);

typedef struct {
	create_segments_t create_segments;
	detach_segment_t detach_segment;
	size_t (*segment_type_size)(void);
} crex_shared_memory_handlers;

typedef struct _handler_entry {
	const char                  *name;
	const crex_shared_memory_handlers *handler;
} crex_shared_memory_handler_entry;

typedef struct _crex_shared_memory_state {
	size_t *positions;  /* current positions for each segment */
	size_t shared_free; /* amount of free shared memory */
} crex_shared_memory_state;

typedef struct _crex_smm_shared_globals {
    /* Shared Memory Manager */
    crex_shared_segment      **shared_segments;
    /* Number of allocated shared segments */
    int                        shared_segments_count;
    /* Amount of free shared memory */
    size_t                     shared_free;
    /* Amount of shared memory allocated by garbage */
    size_t                     wasted_shared_memory;
    /* No more shared memory flag */
    bool                  memory_exhausted;
    /* Saved Shared Allocator State */
    crex_shared_memory_state   shared_memory_state;
	/* Pointer to the application's shared data structures */
	void                      *app_shared_globals;
	/* Reserved shared memory */
	void                      *reserved;
	size_t                     reserved_size;
} crex_smm_shared_globals;

extern crex_smm_shared_globals *smm_shared_globals;

#define ZSMMG(element)		(smm_shared_globals->element)

#define SHARED_ALLOC_REATTACHED		(SUCCESS+1)

BEGIN_EXTERN_C()

int crex_shared_alloc_startup(size_t requested_size, size_t reserved_size);
void crex_shared_alloc_shutdown(void);

/* allocate shared memory block */
void *crex_shared_alloc(size_t size);

/**
 * Wrapper for crex_shared_alloc() which aligns at 64-byte boundary if
 * AVX or SSE2 are used.
 */
static inline void *crex_shared_alloc_aligned(size_t size) {
#if defined(__AVX__) || defined(__SSE2__)
	/* Align to 64-byte boundary */
	void *p = crex_shared_alloc(size + 64);
	return (void *)(((uintptr_t)p + 63L) & ~63L);
#else
	return crex_shared_alloc(size);
#endif
}

/* copy into shared memory */
void *crex_shared_memdup_get_put_free(void *source, size_t size);
void *crex_shared_memdup_put_free(void *source, size_t size);
void *crex_shared_memdup_free(void *source, size_t size);
void *crex_shared_memdup_get_put(void *source, size_t size);
void *crex_shared_memdup_put(void *source, size_t size);
void *crex_shared_memdup(void *source, size_t size);

int  crex_shared_memdup_size(void *p, size_t size);

bool crex_accel_in_shm(void *ptr);

typedef union _align_test {
	void   *ptr;
	double  dbl;
	crex_long  lng;
} align_test;

#if CREX_GCC_VERSION >= 2000
# define PLATFORM_ALIGNMENT (__alignof__(align_test) < 8 ? 8 : __alignof__(align_test))
#else
# define PLATFORM_ALIGNMENT (sizeof(align_test))
#endif

#define CREX_ALIGNED_SIZE(size) \
	CREX_MM_ALIGNED_SIZE_EX(size, PLATFORM_ALIGNMENT)

/* exclusive locking */
void crex_shared_alloc_lock(void);
void crex_shared_alloc_unlock(void); /* returns the allocated size during lock..unlock */
void crex_shared_alloc_safe_unlock(void);

/* old/new mapping functions */
void crex_shared_alloc_init_xlat_table(void);
void crex_shared_alloc_destroy_xlat_table(void);
void crex_shared_alloc_clear_xlat_table(void);
uint32_t crex_shared_alloc_checkpoint_xlat_table(void);
void crex_shared_alloc_restore_xlat_table(uint32_t checkpoint);
void crex_shared_alloc_register_xlat_entry(const void *key, const void *value);
void *crex_shared_alloc_get_xlat_entry(const void *key);

size_t crex_shared_alloc_get_free_memory(void);
void crex_shared_alloc_save_state(void);
void crex_shared_alloc_restore_state(void);
const char *crex_accel_get_shared_model(void);

/**
 * Memory write protection
 *
 * @param protected true to protect shared memory (read-only), false
 * to unprotect shared memory (writable)
 */
void crex_accel_shared_protect(bool protected);

#ifdef USE_MMAP
extern const crex_shared_memory_handlers crex_alloc_mmap_handlers;
#endif

#ifdef USE_SHM
extern const crex_shared_memory_handlers crex_alloc_shm_handlers;
#endif

#ifdef USE_SHM_OPEN
extern const crex_shared_memory_handlers crex_alloc_posix_handlers;
#endif

#ifdef CREX_WIN32
extern const crex_shared_memory_handlers crex_alloc_win32_handlers;
void crex_shared_alloc_create_lock(void);
void crex_shared_alloc_lock_win32(void);
void crex_shared_alloc_unlock_win32(void);
#endif

END_EXTERN_C()

#endif /* CREX_SHARED_ALLOC_H */
