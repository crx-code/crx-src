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
   | Author: Michael Wallner <mike@crx.net>                               |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_OUTPUT_H
#define CRX_OUTPUT_H

#define CRX_OUTPUT_NEWAPI 1

/* handler ops */
#define CRX_OUTPUT_HANDLER_WRITE	0x00	/* standard passthru */
#define CRX_OUTPUT_HANDLER_START	0x01	/* start */
#define CRX_OUTPUT_HANDLER_CLEAN	0x02	/* restart */
#define CRX_OUTPUT_HANDLER_FLUSH	0x04	/* pass along as much as possible */
#define CRX_OUTPUT_HANDLER_FINAL	0x08	/* finalize */
#define CRX_OUTPUT_HANDLER_CONT		CRX_OUTPUT_HANDLER_WRITE
#define CRX_OUTPUT_HANDLER_END		CRX_OUTPUT_HANDLER_FINAL

/* handler types */
#define CRX_OUTPUT_HANDLER_INTERNAL		0x0000
#define CRX_OUTPUT_HANDLER_USER			0x0001

/* handler ability flags */
#define CRX_OUTPUT_HANDLER_CLEANABLE	0x0010
#define CRX_OUTPUT_HANDLER_FLUSHABLE	0x0020
#define CRX_OUTPUT_HANDLER_REMOVABLE	0x0040
#define CRX_OUTPUT_HANDLER_STDFLAGS		0x0070

/* handler status flags */
#define CRX_OUTPUT_HANDLER_STARTED		0x1000
#define CRX_OUTPUT_HANDLER_DISABLED		0x2000
#define CRX_OUTPUT_HANDLER_PROCESSED	0x4000

/* handler op return values */
typedef enum _crx_output_handler_status_t {
	CRX_OUTPUT_HANDLER_FAILURE,
	CRX_OUTPUT_HANDLER_SUCCESS,
	CRX_OUTPUT_HANDLER_NO_DATA
} crx_output_handler_status_t;

/* crx_output_stack_pop() flags */
#define CRX_OUTPUT_POP_TRY			0x000
#define CRX_OUTPUT_POP_FORCE		0x001
#define CRX_OUTPUT_POP_DISCARD		0x010
#define CRX_OUTPUT_POP_SILENT		0x100

/* real global flags */
#define CRX_OUTPUT_IMPLICITFLUSH		0x01
#define CRX_OUTPUT_DISABLED				0x02
#define CRX_OUTPUT_WRITTEN				0x04
#define CRX_OUTPUT_SENT					0x08
/* supplementary flags for crx_output_get_status() */
#define CRX_OUTPUT_ACTIVE				0x10
#define CRX_OUTPUT_LOCKED				0x20
/* output layer is ready to use */
#define CRX_OUTPUT_ACTIVATED		0x100000

/* handler hooks */
typedef enum _crx_output_handler_hook_t {
	CRX_OUTPUT_HANDLER_HOOK_GET_OPAQ,
	CRX_OUTPUT_HANDLER_HOOK_GET_FLAGS,
	CRX_OUTPUT_HANDLER_HOOK_GET_LEVEL,
	CRX_OUTPUT_HANDLER_HOOK_IMMUTABLE,
	CRX_OUTPUT_HANDLER_HOOK_DISABLE,
	/* unused */
	CRX_OUTPUT_HANDLER_HOOK_LAST
} crx_output_handler_hook_t;

#define CRX_OUTPUT_HANDLER_INITBUF_SIZE(s) \
( ((s) > 1) ? \
	(s) + CRX_OUTPUT_HANDLER_ALIGNTO_SIZE - ((s) % (CRX_OUTPUT_HANDLER_ALIGNTO_SIZE)) : \
	CRX_OUTPUT_HANDLER_DEFAULT_SIZE \
)
#define CRX_OUTPUT_HANDLER_ALIGNTO_SIZE		0x1000
#define CRX_OUTPUT_HANDLER_DEFAULT_SIZE		0x4000

typedef struct _crx_output_buffer {
	char *data;
	size_t size;
	size_t used;
	uint32_t free:1;
	uint32_t _reserved:31;
} crx_output_buffer;

typedef struct _crx_output_context {
	int op;
	crx_output_buffer in;
	crx_output_buffer out;
} crx_output_context;

/* old-style, stateless callback */
typedef void (*crx_output_handler_func_t)(char *output, size_t output_len, char **handled_output, size_t *handled_output_len, int mode);
/* new-style, opaque context callback */
typedef int (*crx_output_handler_context_func_t)(void **handler_context, crx_output_context *output_context);
/* output handler context dtor */
typedef void (*crx_output_handler_context_dtor_t)(void *opaq);
/* conflict check callback */
typedef int (*crx_output_handler_conflict_check_t)(const char *handler_name, size_t handler_name_len);
/* ctor for aliases */
typedef struct _crx_output_handler *(*crx_output_handler_alias_ctor_t)(const char *handler_name, size_t handler_name_len, size_t chunk_size, int flags);

typedef struct _crx_output_handler_user_func_t {
	crex_fcall_info fci;
	crex_fcall_info_cache fcc;
	zval zoh;
} crx_output_handler_user_func_t;

typedef struct _crx_output_handler {
	crex_string *name;
	int flags;
	int level;
	size_t size;
	crx_output_buffer buffer;

	void *opaq;
	void (*dtor)(void *opaq);

	union {
		crx_output_handler_user_func_t *user;
		crx_output_handler_context_func_t internal;
	} func;
} crx_output_handler;

CREX_BEGIN_MODULE_GLOBALS(output)
	crex_stack handlers;
	crx_output_handler *active;
	crx_output_handler *running;
	crex_string *output_start_filename;
	int output_start_lineno;
	int flags;
CREX_END_MODULE_GLOBALS(output)

CRXAPI CREX_EXTERN_MODULE_GLOBALS(output)

/* there should not be a need to use OG() from outside of output.c */
#ifdef ZTS
# define OG(v) CREX_TSRMG(output_globals_id, crex_output_globals *, v)
#else
# define OG(v) (output_globals.v)
#endif

/* convenience macros */
#define CRXWRITE(str, str_len)		crx_output_write((str), (str_len))
#define CRXWRITE_H(str, str_len)	crx_output_write_unbuffered((str), (str_len))

#define PUTC(c)						crx_output_write((const char *) &(c), 1)
#define PUTC_H(c)					crx_output_write_unbuffered((const char *) &(c), 1)

#define PUTS(str)					do {				\
	const char *__str = (str);							\
	crx_output_write(__str, strlen(__str));	\
} while (0)
#define PUTS_H(str)					do {							\
	const char *__str = (str);										\
	crx_output_write_unbuffered(__str, strlen(__str));	\
} while (0)


BEGIN_EXTERN_C()

extern const char crx_output_default_handler_name[sizeof("default output handler")];
extern const char crx_output_devnull_handler_name[sizeof("null output handler")];

#define crx_output_tearup() \
	crx_output_startup(); \
	crx_output_activate()
#define crx_output_teardown() \
	crx_output_end_all(); \
	crx_output_deactivate(); \
	crx_output_shutdown()

/* MINIT */
CRXAPI void crx_output_startup(void);
/* MSHUTDOWN */
CRXAPI void crx_output_shutdown(void);

/* RINIT */
CRXAPI int crx_output_activate(void);
/* RSHUTDOWN */
CRXAPI void crx_output_deactivate(void);

CRXAPI void crx_output_set_status(int status);
CRXAPI int crx_output_get_status(void);
CRXAPI void crx_output_set_implicit_flush(int flush);
CRXAPI const char *crx_output_get_start_filename(void);
CRXAPI int crx_output_get_start_lineno(void);

CRXAPI size_t crx_output_write_unbuffered(const char *str, size_t len);
CRXAPI size_t crx_output_write(const char *str, size_t len);

CRXAPI int crx_output_flush(void);
CRXAPI void crx_output_flush_all(void);
CRXAPI int crx_output_clean(void);
CRXAPI void crx_output_clean_all(void);
CRXAPI int crx_output_end(void);
CRXAPI void crx_output_end_all(void);
CRXAPI int crx_output_discard(void);
CRXAPI void crx_output_discard_all(void);

CRXAPI int crx_output_get_contents(zval *p);
CRXAPI int crx_output_get_length(zval *p);
CRXAPI int crx_output_get_level(void);
CRXAPI crx_output_handler* crx_output_get_active_handler(void);

CRXAPI int crx_output_start_default(void);
CRXAPI int crx_output_start_devnull(void);

CRXAPI int crx_output_start_user(zval *output_handler, size_t chunk_size, int flags);
CRXAPI int crx_output_start_internal(const char *name, size_t name_len, crx_output_handler_func_t output_handler, size_t chunk_size, int flags);

CRXAPI crx_output_handler *crx_output_handler_create_user(zval *handler, size_t chunk_size, int flags);
CRXAPI crx_output_handler *crx_output_handler_create_internal(const char *name, size_t name_len, crx_output_handler_context_func_t handler, size_t chunk_size, int flags);

CRXAPI void crx_output_handler_set_context(crx_output_handler *handler, void *opaq, void (*dtor)(void*));
CRXAPI int crx_output_handler_start(crx_output_handler *handler);
CRXAPI int crx_output_handler_started(const char *name, size_t name_len);
CRXAPI int crx_output_handler_hook(crx_output_handler_hook_t type, void *arg);
CRXAPI void crx_output_handler_dtor(crx_output_handler *handler);
CRXAPI void crx_output_handler_free(crx_output_handler **handler);

CRXAPI int crx_output_handler_conflict(const char *handler_new, size_t handler_new_len, const char *handler_set, size_t handler_set_len);
CRXAPI int crx_output_handler_conflict_register(const char *handler_name, size_t handler_name_len, crx_output_handler_conflict_check_t check_func);
CRXAPI int crx_output_handler_reverse_conflict_register(const char *handler_name, size_t handler_name_len, crx_output_handler_conflict_check_t check_func);

CRXAPI crx_output_handler_alias_ctor_t crx_output_handler_alias(const char *handler_name, size_t handler_name_len);
CRXAPI int crx_output_handler_alias_register(const char *handler_name, size_t handler_name_len, crx_output_handler_alias_ctor_t func);

END_EXTERN_C()

#endif
