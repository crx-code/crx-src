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
   | Authors: Felipe Pena <felipe@crx.net>                                |
   | Authors: Joe Watkins <joe.watkins@live.co.uk>                        |
   | Authors: Bob Weinand <bwoebi@crx.net>                                |
   +----------------------------------------------------------------------+
*/

#ifndef CRXDBG_H
#define CRXDBG_H

#ifdef CRX_WIN32
# define CRXDBG_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
# define CRXDBG_API __attribute__ ((visibility("default")))
#else
# define CRXDBG_API
#endif

#include <stdint.h>
#include <stddef.h>
#include "crx.h"
#include "crx_globals.h"
#include "crx_variables.h"
#include "crx_getopt.h"
#include "crex_builtin_functions.h"
#include "crex_extensions.h"
#include "crex_modules.h"
#include "crex_globals.h"
#include "crex_ini_scanner.h"
#include "crex_stream.h"
#include "crex_signal.h"
#if !defined(_WIN32) && !defined(CREX_SIGNALS)
#	include <signal.h>
#elif defined(CRX_WIN32)
#	include "win32/signal.h"
#endif
#include "SAPI.h"
#include <fcntl.h>
#include <sys/types.h>
#if defined(_WIN32) && !defined(__MINGW32__)
#	include <windows.h>
#	include "config.w32.h"
#	undef  strcasecmp
#	undef  strncasecmp
#	define strcasecmp _stricmp
#	define strncasecmp _strnicmp
#else
#	include "crx_config.h"
#endif
#ifndef O_BINARY
#	define O_BINARY 0
#endif
#include "crx_main.h"

#ifdef ZTS
# include "TSRM.h"
#endif

#undef crex_hash_str_add
#ifdef CRX_WIN32
#define crex_hash_str_add(...) \
	crex_hash_str_add(__VA_ARGS__)
#else
#define crex_hash_str_add_tmp(ht, key, len, pData) \
	crex_hash_str_add(ht, key, len, pData)
#define crex_hash_str_add(...) crex_hash_str_add_tmp(__VA_ARGS__)
#endif

#ifdef HAVE_CRXDBG_READLINE
# ifdef HAVE_LIBREADLINE
#	 include <readline/readline.h>
#	 include <readline/history.h>
# endif
# ifdef HAVE_LIBEDIT
#	 include <editline/readline.h>
# endif
#endif

/* {{{ strings */
#define CRXDBG_NAME "crxdbg"
#define CRXDBG_AUTHORS "Felipe Pena, Joe Watkins and Bob Weinand" /* Ordered by last name */
#define CRXDBG_ISSUES "http://bugs.crx.net/report.crx"
#define CRXDBG_VERSION CRX_VERSION
#define CRXDBG_INIT_FILENAME ".crxdbginit"
#define CRXDBG_DEFAULT_PROMPT "prompt>"
/* }}} */

#ifdef ZTS
# define CRXDBG_G(v) CREX_TSRMG(crxdbg_globals_id, crex_crxdbg_globals *, v)
#else
# define CRXDBG_G(v) (crxdbg_globals.v)
#endif

#include "crxdbg_sigsafe.h"
#include "crxdbg_out.h"
#include "crxdbg_lexer.h"
#include "crxdbg_cmd.h"
#include "crxdbg_utils.h"
#include "crxdbg_btree.h"
#include "crxdbg_watch.h"
#include "crxdbg_bp.h"

int crxdbg_do_parse(crxdbg_param_t *stack, char *input);

#define CRXDBG_NEXT   2
#define CRXDBG_UNTIL  3
#define CRXDBG_FINISH 4
#define CRXDBG_LEAVE  5

/*
 BEGIN: DO NOT CHANGE DO NOT CHANGE DO NOT CHANGE
*/

/* {{{ flags */
#define CRXDBG_HAS_FILE_BP            (1ULL<<1)
#define CRXDBG_HAS_PENDING_FILE_BP    (1ULL<<2)
#define CRXDBG_HAS_SYM_BP             (1ULL<<3)
#define CRXDBG_HAS_OPLINE_BP          (1ULL<<4)
#define CRXDBG_HAS_METHOD_BP          (1ULL<<5)
#define CRXDBG_HAS_COND_BP            (1ULL<<6)
#define CRXDBG_HAS_OPCODE_BP          (1ULL<<7)
#define CRXDBG_HAS_FUNCTION_OPLINE_BP (1ULL<<8)
#define CRXDBG_HAS_METHOD_OPLINE_BP   (1ULL<<9)
#define CRXDBG_HAS_FILE_OPLINE_BP     (1ULL<<10) /* }}} */

/*
 END: DO NOT CHANGE DO NOT CHANGE DO NOT CHANGE
*/

#define CRXDBG_IN_COND_BP             (1ULL<<11)
#define CRXDBG_IN_EVAL                (1ULL<<12)

#define CRXDBG_IS_STEPPING            (1ULL<<13)
#define CRXDBG_STEP_OPCODE            (1ULL<<14)
#define CRXDBG_IS_QUIET               (1ULL<<15)
#define CRXDBG_IS_QUITTING            (1ULL<<16)
#define CRXDBG_IS_COLOURED            (1ULL<<17)
#define CRXDBG_IS_CLEANING            (1ULL<<18)
#define CRXDBG_IS_RUNNING             (1ULL<<19)

#define CRXDBG_IN_UNTIL               (1ULL<<20)
#define CRXDBG_IN_FINISH              (1ULL<<21)
#define CRXDBG_IN_LEAVE               (1ULL<<22)

#define CRXDBG_IS_REGISTERED          (1ULL<<23)
#define CRXDBG_IS_STEPONEVAL          (1ULL<<24)
#define CRXDBG_IS_INITIALIZING        (1ULL<<25)
#define CRXDBG_IS_SIGNALED            (1ULL<<26)
#define CRXDBG_IS_INTERACTIVE         (1ULL<<27)
#define CRXDBG_PREVENT_INTERACTIVE    (1ULL<<28)
#define CRXDBG_IS_BP_ENABLED          (1ULL<<29)
#define CRXDBG_SHOW_REFCOUNTS         (1ULL<<30)
#define CRXDBG_IN_SIGNAL_HANDLER      (1ULL<<31)
#define CRXDBG_DISCARD_OUTPUT         (1ULL<<32)
#define CRXDBG_HAS_PAGINATION         (1ULL<<33)

#define CRXDBG_SEEK_MASK              (CRXDBG_IN_UNTIL | CRXDBG_IN_FINISH | CRXDBG_IN_LEAVE)
#define CRXDBG_BP_RESOLVE_MASK	      (CRXDBG_HAS_FUNCTION_OPLINE_BP | CRXDBG_HAS_METHOD_OPLINE_BP | CRXDBG_HAS_FILE_OPLINE_BP)
#define CRXDBG_BP_MASK                (CRXDBG_HAS_FILE_BP | CRXDBG_HAS_SYM_BP | CRXDBG_HAS_METHOD_BP | CRXDBG_HAS_OPLINE_BP | CRXDBG_HAS_COND_BP | CRXDBG_HAS_OPCODE_BP | CRXDBG_HAS_FUNCTION_OPLINE_BP | CRXDBG_HAS_METHOD_OPLINE_BP | CRXDBG_HAS_FILE_OPLINE_BP)
#define CRXDBG_IS_STOPPING            (CRXDBG_IS_QUITTING | CRXDBG_IS_CLEANING)

#define CRXDBG_PRESERVE_FLAGS_MASK    \
    (CRXDBG_SHOW_REFCOUNTS | \
     CRXDBG_IS_STEPONEVAL | \
     CRXDBG_IS_BP_ENABLED | \
     CRXDBG_STEP_OPCODE | \
     CRXDBG_IS_QUIET | \
     CRXDBG_IS_COLOURED | \
     CRXDBG_HAS_PAGINATION)

#ifndef _WIN32
#	define CRXDBG_DEFAULT_FLAGS (CRXDBG_IS_QUIET | CRXDBG_IS_COLOURED | CRXDBG_IS_BP_ENABLED | CRXDBG_HAS_PAGINATION)
#else
#	define CRXDBG_DEFAULT_FLAGS (CRXDBG_IS_QUIET | CRXDBG_IS_BP_ENABLED | CRXDBG_HAS_PAGINATION)
#endif /* }}} */

/* {{{ output descriptors */
#define CRXDBG_STDIN 			0
#define CRXDBG_STDOUT			1
#define CRXDBG_STDERR			2
#define CRXDBG_IO_FDS 			3 /* }}} */

#define crxdbg_try_access \
	{                                                            \
		JMP_BUF *__orig_bailout = CRXDBG_G(sigsegv_bailout); \
		JMP_BUF __bailout;                                   \
                                                                     \
		CRXDBG_G(sigsegv_bailout) = &__bailout;              \
		if (SETJMP(__bailout) == 0) {
#define crxdbg_catch_access \
		} else {                                             \
			CRXDBG_G(sigsegv_bailout) = __orig_bailout;
#define crxdbg_end_try_access() \
		}                                                    \
			CRXDBG_G(sigsegv_bailout) = __orig_bailout;  \
	}


void crxdbg_register_file_handles(void);

typedef struct _crxdbg_oplog_entry crxdbg_oplog_entry;
struct _crxdbg_oplog_entry {
	crxdbg_oplog_entry *next;
	crex_string *function_name;
	crex_class_entry *scope;
	crex_string *filename;
	crex_op *opcodes;
	crex_op *op;
};

typedef struct _crxdbg_oplog_list crxdbg_oplog_list;
struct _crxdbg_oplog_list {
	crxdbg_oplog_list *prev;
	crxdbg_oplog_entry start; /* Only "next" member used. */
};


/* {{{ structs */
CREX_BEGIN_MODULE_GLOBALS(crxdbg)
	HashTable bp[CRXDBG_BREAK_TABLES];           /* break points */
	HashTable registered;                        /* registered */
	HashTable seek;                              /* seek oplines */
	crex_execute_data *seek_ex;                  /* call frame of oplines to seek to */
	crex_object *handled_exception;              /* last handled exception (prevent multiple handling of same exception) */
	crxdbg_frame_t frame;                        /* frame */
	uint32_t last_line;                          /* last executed line */

	char *cur_command;                           /* current command */
	crxdbg_lexer_data lexer;                     /* lexer data */
	crxdbg_param_t *parser_stack;                /* param stack during lexer / parser phase */

#ifndef _WIN32
	struct sigaction old_sigsegv_signal;         /* segv signal handler */
#endif
#ifdef HAVE_USERFAULTFD_WRITEFAULT
    int watch_userfaultfd;                       /* userfaultfd(2) handler, 0 if unused */
    pthread_t watch_userfault_thread;            /* thread for watch fault handling */
#endif
	crxdbg_btree watchpoint_tree;                /* tree with watchpoints */
	crxdbg_btree watch_HashTables;               /* tree with original dtors of watchpoints */
	HashTable watch_elements;                    /* user defined watch elements */
	HashTable watch_collisions;                  /* collision table to check if multiple watches share the same recursive watchpoint */
	HashTable watch_recreation;                  /* watch elements pending recreation of their respective watchpoints */
	HashTable watch_free;                        /* pointers to watch for being freed */
	HashTable *watchlist_mem;                    /* triggered watchpoints */
	HashTable *watchlist_mem_backup;             /* triggered watchpoints backup table while iterating over it */
	bool watchpoint_hit;                    /* a watchpoint was hit */
	void (*original_free_function)(void *);      /* the original AG(mm_heap)->_free function */
	crxdbg_watch_element *watch_tmp;             /* temporary pointer for a watch element */

	char *exec;                                  /* file to execute */
	size_t exec_len;                             /* size of exec */
	crex_op_array *ops;                 	     /* op_array */
	zval retval;                                 /* return value */
	int bp_count;                                /* breakpoint count */
	int vmret;                                   /* return from last opcode handler execution */
	bool in_execution;                      /* in execution? */
	bool unclean_eval;                      /* do not check for memory leaks when we needed to bail out during eval */

	crex_op_array *(*compile_file)(crex_file_handle *file_handle, int type);
	crex_op_array *(*init_compile_file)(crex_file_handle *file_handle, int type);
	crex_op_array *(*compile_string)(crex_string *source_string, const char *filename, crex_compile_position position);
	HashTable file_sources;

	crex_arena *oplog_arena;                     /* arena for storing oplog */
	crxdbg_oplog_list *oplog_list;               /* list of oplog starts */
	crxdbg_oplog_entry *oplog_cur;               /* current oplog entry */

	struct {
		int fd;
	} io[CRXDBG_IO_FDS];                         /* io */
	ssize_t (*crx_stdiop_write)(crx_stream *, const char *, size_t);
	struct {
		bool active;
		int type;
		int fd;
		char *msg;
		int msglen;
	} err_buf;                                   /* error buffer */
	crex_ulong req_id;                           /* "request id" to keep track of commands */

	char *prompt[2];                             /* prompt */
	const crxdbg_color_t *colors[CRXDBG_COLORS]; /* colors */
	char *buffer;                                /* buffer */
	bool last_was_newline;                  /* check if we don't need to output a newline upon next crxdbg_error or crxdbg_notice */

	FILE *stdin_file;                            /* FILE pointer to stdin source file */
	const crx_stream_wrapper *orig_url_wrap_crx;

	char input_buffer[CRXDBG_MAX_CMD];           /* stdin input buffer */
	int input_buflen;                            /* length of stdin input buffer */
	crxdbg_signal_safe_mem sigsafe_mem;          /* memory to use in async safe environment (only once!) */

	JMP_BUF *sigsegv_bailout;                    /* bailout address for accessibility probing */

	uint64_t flags;                              /* crxdbg flags */

	char *sapi_name_ptr;                         /* store sapi name to free it if necessary to not leak memory */
	crex_ulong lines;                                  /* max number of lines to display */
CREX_END_MODULE_GLOBALS(crxdbg) /* }}} */

#endif /* CRXDBG_H */
