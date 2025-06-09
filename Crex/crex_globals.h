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
   +----------------------------------------------------------------------+
*/

#ifndef CREX_GLOBALS_H
#define CREX_GLOBALS_H


#include <setjmp.h>
#include <stdint.h>
#include <sys/types.h>

#include "crex_globals_macros.h"

#include "crex_atomic.h"
#include "crex_stack.h"
#include "crex_ptr_stack.h"
#include "crex_hash.h"
#include "crex_llist.h"
#include "crex_objects.h"
#include "crex_objects_API.h"
#include "crex_modules.h"
#include "crex_float.h"
#include "crex_multibyte.h"
#include "crex_multiply.h"
#include "crex_arena.h"
#include "crex_call_stack.h"
#include "crex_max_execution_timer.h"

/* Define ZTS if you want a thread-safe Crex */
/*#undef ZTS*/

#ifdef ZTS

BEGIN_EXTERN_C()
CREX_API extern int compiler_globals_id;
CREX_API extern int executor_globals_id;
CREX_API extern size_t compiler_globals_offset;
CREX_API extern size_t executor_globals_offset;
END_EXTERN_C()

#endif

#define SYMTABLE_CACHE_SIZE 32

#ifdef CREX_CHECK_STACK_LIMIT
# define CREX_MAX_ALLOWED_STACK_SIZE_UNCHECKED -1
# define CREX_MAX_ALLOWED_STACK_SIZE_DETECT     0
#endif

#include "crex_compile.h"

/* excpt.h on Digital Unix 4.0 defines function_table */
#undef function_table

typedef struct _crex_vm_stack *crex_vm_stack;
typedef struct _crex_ini_entry crex_ini_entry;
typedef struct _crex_fiber_context crex_fiber_context;
typedef struct _crex_fiber crex_fiber;

typedef enum {
	CREX_MEMOIZE_NONE,
	CREX_MEMOIZE_COMPILE,
	CREX_MEMOIZE_FETCH,
} crex_memoize_mode;

struct _crex_compiler_globals {
	crex_stack loop_var_stack;

	crex_class_entry *active_class_entry;

	crex_string *compiled_filename;

	int crex_lineno;

	crex_op_array *active_op_array;

	HashTable *function_table;	/* function symbol table */
	HashTable *class_table;		/* class table */

	HashTable *auto_globals;

	/* Refer to crex_yytnamerr() in crex_language_parser.y for meaning of values */
	uint8_t parse_error;
	bool in_compilation;
	bool short_tags;

	bool unclean_shutdown;

	bool ini_parser_unbuffered_errors;

	crex_llist open_files;

	struct _crex_ini_parser_param *ini_parser_param;

	bool skip_shebang;
	bool increment_lineno;

	bool variable_width_locale;   /* UTF-8, Shift-JIS, Big5, ISO 2022, EUC, etc */
	bool ascii_compatible_locale; /* locale uses ASCII characters as singletons */
	                              /* and don't use them as lead/trail units     */

	crex_string *doc_comment;
	uint32_t extra_fn_flags;

	uint32_t compiler_options; /* set of CREX_COMPILE_* constants */

	crex_oparray_context context;
	crex_file_context file_context;

	crex_arena *arena;

	HashTable interned_strings;

	const crex_encoding **script_encoding_list;
	size_t script_encoding_list_size;
	bool multibyte;
	bool detect_unicode;
	bool encoding_declared;

	crex_ast *ast;
	crex_arena *ast_arena;

	crex_stack delayed_oplines_stack;
	HashTable *memoized_exprs;
	crex_memoize_mode memoize_mode;

	void   *map_ptr_real_base;
	void   *map_ptr_base;
	size_t  map_ptr_size;
	size_t  map_ptr_last;

	HashTable *delayed_variance_obligations;
	HashTable *delayed_autoloads;
	HashTable *unlinked_uses;
	crex_class_entry *current_linking_class;

	uint32_t rtd_key_counter;

	crex_stack short_circuiting_opnums;
#ifdef ZTS
	uint32_t copied_functions_count;
#endif
};


struct _crex_executor_globals {
	zval uninitialized_zval;
	zval error_zval;

	/* symbol table cache */
	crex_array *symtable_cache[SYMTABLE_CACHE_SIZE];
	/* Pointer to one past the end of the symtable_cache */
	crex_array **symtable_cache_limit;
	/* Pointer to first unused symtable_cache slot */
	crex_array **symtable_cache_ptr;

	crex_array symbol_table;		/* main symbol table */

	HashTable included_files;	/* files already included */

	JMP_BUF *bailout;

	int error_reporting;
	int exit_status;

	HashTable *function_table;	/* function symbol table */
	HashTable *class_table;		/* class table */
	HashTable *crex_constants;	/* constants table */

	zval          *vm_stack_top;
	zval          *vm_stack_end;
	crex_vm_stack  vm_stack;
	size_t         vm_stack_page_size;

	struct _crex_execute_data *current_execute_data;
	crex_class_entry *fake_scope; /* used to avoid checks accessing properties */

	uint32_t jit_trace_num; /* Used by tracing JIT to reference the currently running trace */

	int ticks_count;

	crex_long precision;

	uint32_t persistent_constants_count;
	uint32_t persistent_functions_count;
	uint32_t persistent_classes_count;

	/* for extended information support */
	bool no_extensions;

	bool full_tables_cleanup;

	crex_atomic_bool vm_interrupt;
	crex_atomic_bool timed_out;

	HashTable *in_autoload;

	crex_long hard_timeout;
	void *stack_base;
	void *stack_limit;

#ifdef CREX_WIN32
	OSVERSIONINFOEX windows_version_info;
#endif

	HashTable regular_list;
	HashTable persistent_list;

	int user_error_handler_error_reporting;
	bool exception_ignore_args;
	zval user_error_handler;
	zval user_exception_handler;
	crex_stack user_error_handlers_error_reporting;
	crex_stack user_error_handlers;
	crex_stack user_exception_handlers;

	crex_class_entry      *exception_class;
	crex_error_handling_t  error_handling;

	int capture_warnings_during_sccp;

	/* timeout support */
	crex_long timeout_seconds;

	HashTable *ini_directives;
	HashTable *modified_ini_directives;
	crex_ini_entry *error_reporting_ini_entry;

	crex_objects_store objects_store;
	crex_object *exception, *prev_exception;
	const crex_op *opline_before_exception;
	crex_op exception_op[3];

	struct _crex_module_entry *current_module;

	bool active;
	uint8_t flags;

	crex_long assertions;

	uint32_t           ht_iterators_count;     /* number of allocated slots */
	uint32_t           ht_iterators_used;      /* number of used slots */
	HashTableIterator *ht_iterators;
	HashTableIterator  ht_iterators_slots[16];

	void *saved_fpu_cw_ptr;
#if XPFPA_HAVE_CW
	XPFPA_CW_DATATYPE saved_fpu_cw;
#endif

	crex_function trampoline;
	crex_op       call_trampoline_op;

	HashTable weakrefs;

	crex_long exception_string_param_max_len;

	crex_get_gc_buffer get_gc_buffer;

	crex_fiber_context *main_fiber_context;
	crex_fiber_context *current_fiber_context;

	/* Active instance of Fiber. */
	crex_fiber *active_fiber;

	/* Default fiber C stack size. */
	size_t fiber_stack_size;

	/* If record_errors is enabled, all emitted diagnostics will be recorded,
	 * in addition to being processed as usual. */
	bool record_errors;
	uint32_t num_errors;
	crex_error_info **errors;

	/* Override filename or line number of thrown errors and exceptions */
	crex_string *filename_override;
	crex_long lineno_override;

#ifdef CREX_CHECK_STACK_LIMIT
	crex_call_stack call_stack;
	crex_long max_allowed_stack_size;
	crex_ulong reserved_stack_size;
#endif

#ifdef CREX_MAX_EXECUTION_TIMERS
	timer_t max_execution_timer_timer;
	pid_t pid;
	struct sigaction oldact;
#endif

	void *reserved[CREX_MAX_RESERVED_RESOURCES];
};

#define EG_FLAGS_INITIAL				(0)
#define EG_FLAGS_IN_SHUTDOWN			(1<<0)
#define EG_FLAGS_OBJECT_STORE_NO_REUSE	(1<<1)
#define EG_FLAGS_IN_RESOURCE_SHUTDOWN	(1<<2)

struct _crex_ini_scanner_globals {
	crex_file_handle *yy_in;
	crex_file_handle *yy_out;

	unsigned int yy_leng;
	const unsigned char *yy_start;
	const unsigned char *yy_text;
	const unsigned char *yy_cursor;
	const unsigned char *yy_marker;
	const unsigned char *yy_limit;
	int yy_state;
	crex_stack state_stack;

	crex_string *filename;
	int lineno;

	/* Modes are: CREX_INI_SCANNER_NORMAL, CREX_INI_SCANNER_RAW, CREX_INI_SCANNER_TYPED */
	int scanner_mode;
};

typedef enum {
	ON_TOKEN,
	ON_FEEDBACK,
	ON_STOP
} crex_crx_scanner_event;

struct _crex_crx_scanner_globals {
	crex_file_handle *yy_in;
	crex_file_handle *yy_out;

	unsigned int yy_leng;
	unsigned char *yy_start;
	unsigned char *yy_text;
	unsigned char *yy_cursor;
	unsigned char *yy_marker;
	unsigned char *yy_limit;
	int yy_state;
	crex_stack state_stack;
	crex_ptr_stack heredoc_label_stack;
	crex_stack nest_location_stack; /* for syntax error reporting */
	bool heredoc_scan_ahead;
	int heredoc_indentation;
	bool heredoc_indentation_uses_spaces;

	/* original (unfiltered) script */
	unsigned char *script_org;
	size_t script_org_size;

	/* filtered script */
	unsigned char *script_filtered;
	size_t script_filtered_size;

	/* input/output filters */
	crex_encoding_filter input_filter;
	crex_encoding_filter output_filter;
	const crex_encoding *script_encoding;

	/* initial string length after scanning to first variable */
	int scanned_string_len;

	/* hooks */
	void (*on_event)(
		crex_crx_scanner_event event, int token, int line,
		const char *text, size_t length, void *context);
	void *on_event_context;
};

#endif /* CREX_GLOBALS_H */
