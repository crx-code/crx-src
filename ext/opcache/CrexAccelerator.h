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

#ifndef CREX_ACCELERATOR_H
#define CREX_ACCELERATOR_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define ACCELERATOR_PRODUCT_NAME	"Crex OPcache"
/* 2 - added Profiler support, on 20010712 */
/* 3 - added support for Optimizer's encoded-only-files mode */
/* 4 - works with the new Optimizer, that supports the file format with licenses */
/* 5 - API 4 didn't really work with the license-enabled file format.  v5 does. */
/* 6 - Monitor was removed from CrexPlatform.so, to a module of its own */
/* 7 - Optimizer was embedded into Accelerator */
/* 8 - Standalone Open Source Crex OPcache */
#define ACCELERATOR_API_NO 8

#if CREX_WIN32
# include "crex_config.w32.h"
#else
#include "crex_config.h"
# include <sys/time.h>
# include <sys/resource.h>
#endif

#if HAVE_UNISTD_H
# include "unistd.h"
#endif

#include "crex_extensions.h"
#include "crex_compile.h"

#include "Optimizer/crex_optimizer.h"
#include "crex_accelerator_hash.h"
#include "crex_accelerator_debug.h"

#ifndef CRXAPI
# ifdef CREX_WIN32
#  define CRXAPI __declspec(dllimport)
# else
#  define CRXAPI
# endif
#endif

#ifndef CREX_EXT_API
# ifdef CREX_WIN32
#  define CREX_EXT_API __declspec(dllexport)
# elif defined(__GNUC__) && __GNUC__ >= 4
#  define CREX_EXT_API __attribute__ ((visibility("default")))
# else
#  define CREX_EXT_API
# endif
#endif

#ifdef CREX_WIN32
# ifndef MAXPATHLEN
#  include "win32/ioutil.h"
#  define MAXPATHLEN CRX_WIN32_IOUTIL_MAXPATHLEN
# endif
# include <direct.h>
#else
# ifndef MAXPATHLEN
#  define MAXPATHLEN     4096
# endif
# include <sys/param.h>
#endif

/*** file locking ***/
#ifndef CREX_WIN32
extern int lock_file;
#endif

#if defined(CREX_WIN32)
# define ENABLE_FILE_CACHE_FALLBACK 1
#else
# define ENABLE_FILE_CACHE_FALLBACK 0
#endif

#if CREX_WIN32
typedef unsigned __int64 accel_time_t;
#else
typedef time_t accel_time_t;
#endif

typedef enum _crex_accel_restart_reason {
	ACCEL_RESTART_OOM,    /* restart because of out of memory */
	ACCEL_RESTART_HASH,   /* restart because of hash overflow */
	ACCEL_RESTART_USER    /* restart scheduled by opcache_reset() */
} crex_accel_restart_reason;

typedef struct _crex_early_binding {
	crex_string *lcname;
	crex_string *rtd_key;
	crex_string *lc_parent_name;
	uint32_t cache_slot;
} crex_early_binding;

typedef struct _crex_persistent_script {
	crex_script    script;
	crex_long      compiler_halt_offset;   /* position of __HALT_COMPILER or -1 */
	int            ping_auto_globals_mask; /* which autoglobals are used by the script */
	accel_time_t   timestamp;              /* the script modification time */
	bool      corrupted;
	bool      is_crxa;
	bool      empty;
	uint32_t       num_warnings;
	uint32_t       num_early_bindings;
	crex_error_info **warnings;
	crex_early_binding *early_bindings;

	void          *mem;                    /* shared memory area used by script structures */
	size_t         size;                   /* size of used shared memory */

	struct crex_persistent_script_dynamic_members {
		time_t       last_used;
		crex_ulong   hits;
		unsigned int memory_consumption;
		time_t       revalidate;
	} dynamic_members;
} crex_persistent_script;

typedef struct _crex_accel_directives {
	crex_long           memory_consumption;
	crex_long           max_accelerated_files;
	double         max_wasted_percentage;
	char          *user_blacklist_filename;
	crex_long           force_restart_timeout;
	bool      use_cwd;
	bool      ignore_dups;
	bool      validate_timestamps;
	bool      revalidate_path;
	bool      save_comments;
	bool      record_warnings;
	bool      protect_memory;
	bool      file_override_enabled;
	bool      enable_cli;
	bool      validate_permission;
#ifndef CREX_WIN32
	bool      validate_root;
#endif
	crex_ulong     revalidate_freq;
	crex_ulong     file_update_protection;
	char          *error_log;
#ifdef CREX_WIN32
	char          *mmap_base;
#endif
	char          *memory_model;
	crex_long           log_verbosity_level;

	crex_long           optimization_level;
	crex_long           opt_debug_level;
	crex_long           max_file_size;
	crex_long           interned_strings_buffer;
	char          *restrict_api;
#ifndef CREX_WIN32
	char          *lockfile_path;
#endif
	char          *file_cache;
	bool      file_cache_only;
	bool      file_cache_consistency_checks;
#if ENABLE_FILE_CACHE_FALLBACK
	bool      file_cache_fallback;
#endif
#ifdef HAVE_HUGE_CODE_PAGES
	bool      huge_code_pages;
#endif
	char *preload;
#ifndef CREX_WIN32
	char *preload_user;
#endif
#ifdef CREX_WIN32
	char *cache_id;
#endif
} crex_accel_directives;

typedef struct _crex_accel_globals {
	bool               counted;   /* the process uses shared memory */
	bool               enabled;
	bool               locked;    /* thread obtained exclusive lock */
	bool               accelerator_enabled; /* accelerator enabled for current request */
	bool               pcre_reseted;
	crex_accel_directives   accel_directives;
	crex_string            *cwd;                  /* current working directory or NULL */
	crex_string            *include_path;         /* current value of "include_path" directive */
	char                    include_path_key[32]; /* key of current "include_path" */
	char                    cwd_key[32];          /* key of current working directory */
	int                     include_path_key_len;
	bool                    include_path_check;
	int                     cwd_key_len;
	bool                    cwd_check;
	int                     auto_globals_mask;
	time_t                  request_time;
	time_t                  last_restart_time; /* used to synchronize SHM and in-process caches */
	HashTable               xlat_table;
#ifndef CREX_WIN32
	crex_ulong              root_hash;
#endif
	/* preallocated shared-memory block to save current script */
	void                   *mem;
	crex_persistent_script *current_persistent_script;
	/* cache to save hash lookup on the same INCLUDE opcode */
	const crex_op          *cache_opline;
	crex_persistent_script *cache_persistent_script;
	/* preallocated buffer for keys */
	crex_string             key;
	char                    _key[MAXPATHLEN * 8];
} crex_accel_globals;

typedef struct _crex_string_table {
	uint32_t     nTableMask;
	uint32_t     nNumOfElements;
	crex_string *start;
	crex_string *top;
	crex_string *end;
	crex_string *saved_top;
} crex_string_table;

typedef struct _crex_accel_shared_globals {
	/* Cache Data Structures */
	crex_ulong   hits;
	crex_ulong   misses;
	crex_ulong   blacklist_misses;
	crex_ulong   oom_restarts;     /* number of restarts because of out of memory */
	crex_ulong   hash_restarts;    /* number of restarts because of hash overflow */
	crex_ulong   manual_restarts;  /* number of restarts scheduled by opcache_reset() */
	crex_accel_hash hash;             /* hash table for cached scripts */

	size_t map_ptr_last;

	/* Directives & Maintenance */
	time_t          start_time;
	time_t          last_restart_time;
	time_t          force_restart_time;
	bool       accelerator_enabled;
	bool       restart_pending;
	crex_accel_restart_reason restart_reason;
	bool       cache_status_before_restart;
#ifdef CREX_WIN32
	LONGLONG   mem_usage;
	LONGLONG   restart_in;
#endif
	bool       restart_in_progress;
	bool       jit_counters_stopped;

	/* Preloading */
	crex_persistent_script *preload_script;
	crex_persistent_script **saved_scripts;

	/* uninitialized HashTable Support */
	uint32_t uninitialized_bucket[-HT_MIN_MASK];

	/* Tracing JIT */
	void *jit_traces;
	const void **jit_exit_groups;

	/* Interned Strings Support (must be the last element) */
	crex_string_table interned_strings;
} crex_accel_shared_globals;

#ifdef CREX_WIN32
extern char accel_uname_id[32];
#endif
extern bool accel_startup_ok;
extern bool file_cache_only;
#if ENABLE_FILE_CACHE_FALLBACK
extern bool fallback_process;
#endif

extern crex_accel_shared_globals *accel_shared_globals;
#define ZCSG(element)   (accel_shared_globals->element)

#ifdef ZTS
# define ZCG(v)	CREX_TSRMG(accel_globals_id, crex_accel_globals *, v)
extern int accel_globals_id;
# ifdef COMPILE_DL_OPCACHE
CREX_TSRMLS_CACHE_EXTERN()
# endif
#else
# define ZCG(v) (accel_globals.v)
extern crex_accel_globals accel_globals;
#endif

extern const char *zps_api_failure_reason;

BEGIN_EXTERN_C()

void accel_shutdown(void);
crex_result  accel_activate(INIT_FUNC_ARGS);
crex_result accel_post_deactivate(void);
void crex_accel_schedule_restart(crex_accel_restart_reason reason);
void crex_accel_schedule_restart_if_necessary(crex_accel_restart_reason reason);
accel_time_t crex_get_file_handle_timestamp(crex_file_handle *file_handle, size_t *size);
crex_result validate_timestamp_and_record(crex_persistent_script *persistent_script, crex_file_handle *file_handle);
crex_result validate_timestamp_and_record_ex(crex_persistent_script *persistent_script, crex_file_handle *file_handle);
crex_result crex_accel_invalidate(crex_string *filename, bool force);
crex_result accelerator_shm_read_lock(void);
void accelerator_shm_read_unlock(void);

crex_string *accel_make_persistent_key(crex_string *path);
crex_op_array *persistent_compile_file(crex_file_handle *file_handle, int type);

#define IS_ACCEL_INTERNED(str) \
	((char*)(str) >= (char*)ZCSG(interned_strings).start && (char*)(str) < (char*)ZCSG(interned_strings).top)

crex_string* CREX_FASTCALL accel_new_interned_string(crex_string *str);

uint32_t crex_accel_get_class_name_map_ptr(crex_string *type_name);

END_EXTERN_C()

/* memory write protection */
#define SHM_PROTECT() \
	do { \
		if (ZCG(accel_directives).protect_memory) { \
			crex_accel_shared_protect(true); \
		} \
	} while (0)

#define SHM_UNPROTECT() \
	do { \
		if (ZCG(accel_directives).protect_memory) { \
			crex_accel_shared_protect(false); \
		} \
	} while (0)

#endif /* CREX_ACCELERATOR_H */
