/*
   +----------------------------------------------------------------------+
   | Crex JIT                                                             |
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
   | Authors: Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef HAVE_JIT_H
#define HAVE_JIT_H

#if defined(__x86_64__) || defined(i386) || defined(CREX_WIN32)
# define CREX_JIT_TARGET_X86   1
# define CREX_JIT_TARGET_ARM64 0
#elif defined (__aarch64__)
# define CREX_JIT_TARGET_X86   0
# define CREX_JIT_TARGET_ARM64 1
#else
# error "JIT not supported on this platform"
#endif

#define CREX_JIT_LEVEL_NONE        0     /* no JIT */
#define CREX_JIT_LEVEL_MINIMAL     1     /* minimal JIT (subroutine threading) */
#define CREX_JIT_LEVEL_INLINE      2     /* selective inline threading */
#define CREX_JIT_LEVEL_OPT_FUNC    3     /* optimized JIT based on Type-Inference */
#define CREX_JIT_LEVEL_OPT_FUNCS   4     /* optimized JIT based on Type-Inference and call-tree */
#define CREX_JIT_LEVEL_OPT_SCRIPT  5     /* optimized JIT based on Type-Inference and inner-procedure analysis */

#define CREX_JIT_ON_SCRIPT_LOAD    0
#define CREX_JIT_ON_FIRST_EXEC     1
#define CREX_JIT_ON_PROF_REQUEST   2     /* compile the most frequently caled on first request functions */
#define CREX_JIT_ON_HOT_COUNTERS   3     /* compile functions after N calls or loop iterations */
#define CREX_JIT_ON_DOC_COMMENT    4     /* compile functions with "@jit" tag in doc-comments */
#define CREX_JIT_ON_HOT_TRACE      5     /* trace functions after N calls or loop iterations */

#define CREX_JIT_REG_ALLOC_LOCAL  (1<<0) /* local linear scan register allocation */
#define CREX_JIT_REG_ALLOC_GLOBAL (1<<1) /* global linear scan register allocation */
#define CREX_JIT_CPU_AVX          (1<<2) /* use AVX instructions, if available */

#define CREX_JIT_DEFAULT_BUFFER_SIZE  "0"

#define CREX_JIT_COUNTER_INIT         32531

#define CREX_JIT_DEBUG_ASM       (1<<0)
#define CREX_JIT_DEBUG_SSA       (1<<1)
#define CREX_JIT_DEBUG_REG_ALLOC (1<<2)
#define CREX_JIT_DEBUG_ASM_STUBS (1<<3)

#define CREX_JIT_DEBUG_PERF      (1<<4)
#define CREX_JIT_DEBUG_PERF_DUMP (1<<5)
#define CREX_JIT_DEBUG_VTUNE     (1<<7)

#define CREX_JIT_DEBUG_GDB       (1<<8)
#define CREX_JIT_DEBUG_SIZE      (1<<9)
#define CREX_JIT_DEBUG_ASM_ADDR  (1<<10)

#define CREX_JIT_DEBUG_TRACE_START     (1<<12)
#define CREX_JIT_DEBUG_TRACE_STOP      (1<<13)
#define CREX_JIT_DEBUG_TRACE_COMPILED  (1<<14)
#define CREX_JIT_DEBUG_TRACE_EXIT      (1<<15)
#define CREX_JIT_DEBUG_TRACE_ABORT     (1<<16)
#define CREX_JIT_DEBUG_TRACE_BLACKLIST (1<<17)
#define CREX_JIT_DEBUG_TRACE_BYTECODE  (1<<18)
#define CREX_JIT_DEBUG_TRACE_TSSA      (1<<19)
#define CREX_JIT_DEBUG_TRACE_EXIT_INFO (1<<20)

#define CREX_JIT_DEBUG_PERSISTENT      0x1f0 /* profile and debugger flags can't be changed at run-time */

#define CREX_JIT_TRACE_MAX_LENGTH        1024 /* max length of single trace */
#define CREX_JIT_TRACE_MAX_EXITS          512 /* max number of side exits per trace */

#define CREX_JIT_TRACE_MAX_FUNCS           30 /* max number of different functions in a single trace */
#define CREX_JIT_TRACE_MAX_CALL_DEPTH      10 /* max depth of inlined calls */
#define CREX_JIT_TRACE_MAX_RET_DEPTH        4 /* max depth of inlined returns */
#define CREX_JIT_TRACE_MAX_LOOPS_UNROLL    10 /* max number of unrolled loops */

#define CREX_JIT_TRACE_BAD_ROOT_SLOTS      64 /* number of slots in bad root trace cache */

typedef struct _crex_jit_trace_rec crex_jit_trace_rec;
typedef struct _crex_jit_trace_stack_frame crex_jit_trace_stack_frame;
typedef struct _sym_node crex_sym_node;

typedef struct _crex_jit_globals {
	bool enabled;
	bool on;
	uint8_t   trigger;
	uint8_t   opt_level;
	uint32_t  opt_flags;

	const char *options;
	crex_long   buffer_size;
	crex_long   debug;
	crex_long   bisect_limit;
	double      prof_threshold;
	crex_long   max_root_traces;       /* max number of root traces */
	crex_long   max_side_traces;       /* max number of side traces (per root trace) */
	crex_long   max_exit_counters;     /* max total number of side exits for all traces */
	crex_long   hot_loop;
	crex_long   hot_func;
	crex_long   hot_return;
	crex_long   hot_side_exit;         /* number of exits before taking side trace */
	crex_long   blacklist_root_trace;  /* number of attempts to JIT a root trace before blacklist it */
	crex_long   blacklist_side_trace;  /* number of attempts to JIT a side trace before blacklist it */
	crex_long   max_loop_unrolls;      /* max number of unrolled loops */
	crex_long   max_recursive_calls;   /* max number of recursive inlined call unrolls */
	crex_long   max_recursive_returns; /* max number of recursive inlined return unrolls */
	crex_long   max_polymorphic_calls; /* max number of inlined polymorphic calls */
	crex_long   max_trace_length; 	   /* max length of a single trace */

	crex_sym_node *symbols;            /* symbols for disassembler */

	bool tracing;

	crex_jit_trace_rec *current_trace;
	crex_jit_trace_stack_frame *current_frame;

	const crex_op *bad_root_cache_opline[CREX_JIT_TRACE_BAD_ROOT_SLOTS];
	uint8_t bad_root_cache_count[CREX_JIT_TRACE_BAD_ROOT_SLOTS];
	uint8_t bad_root_cache_stop[CREX_JIT_TRACE_BAD_ROOT_SLOTS];
	uint32_t bad_root_slot;

	uint8_t  *exit_counters;
} crex_jit_globals;

#ifdef ZTS
# define JIT_G(v) CREX_TSRMG(jit_globals_id, crex_jit_globals *, v)
extern int jit_globals_id;
#else
# define JIT_G(v) (jit_globals.v)
extern crex_jit_globals jit_globals;
#endif

CREX_EXT_API int  crex_jit_op_array(crex_op_array *op_array, crex_script *script);
CREX_EXT_API int  crex_jit_script(crex_script *script);
CREX_EXT_API void crex_jit_unprotect(void);
CREX_EXT_API void crex_jit_protect(void);
CREX_EXT_API void crex_jit_init(void);
CREX_EXT_API int  crex_jit_config(crex_string *jit_options, int stage);
CREX_EXT_API int  crex_jit_debug_config(crex_long old_val, crex_long new_val, int stage);
CREX_EXT_API int  crex_jit_check_support(void);
CREX_EXT_API int  crex_jit_startup(void *jit_buffer, size_t size, bool reattached);
CREX_EXT_API void crex_jit_shutdown(void);
CREX_EXT_API void crex_jit_activate(void);
CREX_EXT_API void crex_jit_deactivate(void);
CREX_EXT_API void crex_jit_status(zval *ret);
CREX_EXT_API void crex_jit_restart(void);

typedef struct _crex_lifetime_interval crex_lifetime_interval;
typedef struct _crex_life_range crex_life_range;

struct _crex_life_range {
	uint32_t         start;
	uint32_t         end;
	crex_life_range *next;
};

#define ZREG_FLAGS_SHIFT    8

#define ZREG_STORE          (1<<0)
#define ZREG_LOAD           (1<<1)
#define ZREG_LAST_USE       (1<<2)
#define ZREG_SPLIT          (1<<3)

struct _crex_lifetime_interval {
	int                     ssa_var;
	union {
		struct {
		CREX_ENDIAN_LOHI_3(
			int8_t          reg,
			uint8_t         flags,
			uint16_t        reserved
		)};
		uint32_t            reg_flags;
	};
	crex_life_range         range;
	crex_lifetime_interval *hint;
	crex_lifetime_interval *used_as_hint;
	crex_lifetime_interval *list_next;
};

#endif /* HAVE_JIT_H */
