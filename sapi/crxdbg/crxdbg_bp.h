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

#ifndef CRXDBG_BP_H
#define CRXDBG_BP_H

/* {{{ defines */
#define CRXDBG_BREAK_FILE            0
#define CRXDBG_BREAK_FILE_PENDING    1
#define CRXDBG_BREAK_SYM             2
#define CRXDBG_BREAK_OPLINE          3
#define CRXDBG_BREAK_METHOD          4
#define CRXDBG_BREAK_COND            5
#define CRXDBG_BREAK_OPCODE          6
#define CRXDBG_BREAK_FUNCTION_OPLINE 7
#define CRXDBG_BREAK_METHOD_OPLINE   8
#define CRXDBG_BREAK_FILE_OPLINE     9
#define CRXDBG_BREAK_MAP             10
#define CRXDBG_BREAK_TABLES          11 /* }}} */

/* {{{ */
typedef struct _crex_op *crxdbg_opline_ptr_t; /* }}} */

/* {{{ breakpoint base structure */
#define crxdbg_breakbase(name) \
	int         id; \
	uint8_t  type; \
	crex_ulong  hits; \
	bool   disabled; \
	const char *name /* }}} */

/* {{{ breakpoint base */
typedef struct _crxdbg_breakbase_t {
	crxdbg_breakbase(name);
} crxdbg_breakbase_t; /* }}} */

/**
 * Breakpoint file-based representation
 */
typedef struct _crxdbg_breakfile_t {
	crxdbg_breakbase(filename);
	crex_ulong line;
} crxdbg_breakfile_t;

/**
 * Breakpoint symbol-based representation
 */
typedef struct _crxdbg_breaksymbol_t {
	crxdbg_breakbase(symbol);
} crxdbg_breaksymbol_t;

/**
 * Breakpoint method based representation
 */
typedef struct _crxdbg_breakmethod_t {
	crxdbg_breakbase(class_name);
	size_t      class_len;
	const char *func_name;
	size_t      func_len;
} crxdbg_breakmethod_t;

/**
 * Breakpoint opline num based representation
 */
typedef struct _crxdbg_breakopline_t {
	crxdbg_breakbase(func_name);
	size_t      func_len;
	const char *class_name;
	size_t      class_len;
	crex_ulong  opline_num;
	crex_ulong  opline;
} crxdbg_breakopline_t;

/**
 * Breakpoint opline based representation
 */
typedef struct _crxdbg_breakline_t {
	crxdbg_breakbase(name);
	crex_ulong opline;
	crxdbg_breakopline_t *base;
} crxdbg_breakline_t;

/**
 * Breakpoint opcode based representation
 */
typedef struct _crxdbg_breakop_t {
	crxdbg_breakbase(name);
	crex_ulong hash;
} crxdbg_breakop_t;

/**
 * Breakpoint condition based representation
 */
typedef struct _crxdbg_breakcond_t {
	crxdbg_breakbase(code);
	size_t          code_len;
	bool       paramed;
	crxdbg_param_t  param;
	crex_ulong      hash;
	crex_op_array  *ops;
} crxdbg_breakcond_t;

/* {{{ Resolving breaks API */
CRXDBG_API void crxdbg_resolve_op_array_breaks(crex_op_array *op_array);
CRXDBG_API int crxdbg_resolve_op_array_break(crxdbg_breakopline_t *brake, crex_op_array *op_array);
CRXDBG_API int crxdbg_resolve_opline_break(crxdbg_breakopline_t *new_break);
CRXDBG_API HashTable *crxdbg_resolve_pending_file_break_ex(const char *file, uint32_t filelen, crex_string *cur, HashTable *fileht);
CRXDBG_API void crxdbg_resolve_pending_file_break(const char *file); /* }}} */

/* {{{ Breakpoint Creation API */
CRXDBG_API void crxdbg_set_breakpoint_file(const char* filename, size_t path_len, crex_ulong lineno);
CRXDBG_API void crxdbg_set_breakpoint_symbol(const char* func_name, size_t func_name_len);
CRXDBG_API void crxdbg_set_breakpoint_method(const char* class_name, const char* func_name);
CRXDBG_API void crxdbg_set_breakpoint_opcode(const char* opname, size_t opname_len);
CRXDBG_API void crxdbg_set_breakpoint_opline(crex_ulong opline);
CRXDBG_API void crxdbg_set_breakpoint_opline_ex(crxdbg_opline_ptr_t opline);
CRXDBG_API void crxdbg_set_breakpoint_method_opline(const char *class, const char *method, crex_ulong opline);
CRXDBG_API void crxdbg_set_breakpoint_function_opline(const char *function, crex_ulong opline);
CRXDBG_API void crxdbg_set_breakpoint_file_opline(const char *file, crex_ulong opline);
CRXDBG_API void crxdbg_set_breakpoint_expression(const char* expression, size_t expression_len);
CRXDBG_API void crxdbg_set_breakpoint_at(const crxdbg_param_t *param); /* }}} */

/* {{{ Breakpoint Detection API */
CRXDBG_API crxdbg_breakbase_t* crxdbg_find_breakpoint(crex_execute_data*); /* }}} */

/* {{{ Misc Breakpoint API */
CRXDBG_API void crxdbg_hit_breakpoint(crxdbg_breakbase_t* brake, bool output);
CRXDBG_API void crxdbg_print_breakpoints(crex_ulong type);
CRXDBG_API void crxdbg_print_breakpoint(crxdbg_breakbase_t* brake);
CRXDBG_API void crxdbg_reset_breakpoints(void);
CRXDBG_API void crxdbg_clear_breakpoints(void);
CRXDBG_API void crxdbg_delete_breakpoint(crex_ulong num);
CRXDBG_API void crxdbg_enable_breakpoints(void);
CRXDBG_API void crxdbg_enable_breakpoint(crex_ulong id);
CRXDBG_API void crxdbg_disable_breakpoint(crex_ulong id);
CRXDBG_API void crxdbg_disable_breakpoints(void); /* }}} */

/* {{{ Breakbase API */
CRXDBG_API crxdbg_breakbase_t *crxdbg_find_breakbase(crex_ulong id);
CRXDBG_API crxdbg_breakbase_t *crxdbg_find_breakbase_ex(crex_ulong id, HashTable **table, crex_ulong *numkey, crex_string **strkey); /* }}} */

/* {{{ Breakpoint Exportation API */
CRXDBG_API void crxdbg_export_breakpoints(FILE *handle);
CRXDBG_API void crxdbg_export_breakpoints_to_string(char **str); /* }}} */

#endif /* CRXDBG_BP_H */
