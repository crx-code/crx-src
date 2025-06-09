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

#ifndef CRXDBG_UTILS_H
#define CRXDBG_UTILS_H

/**
 * Input scan functions
 */
CRXDBG_API int crxdbg_is_numeric(const char*);
CRXDBG_API int crxdbg_is_empty(const char*);
CRXDBG_API int crxdbg_is_addr(const char*);
CRXDBG_API int crxdbg_is_class_method(const char*, size_t, char**, char**);
CRXDBG_API const char *crxdbg_current_file(void);
CRXDBG_API char *crxdbg_resolve_path(const char*);
CRXDBG_API char *crxdbg_trim(const char*, size_t, size_t*);
CRXDBG_API const crex_function *crxdbg_get_function(const char*, const char*);

/* {{{ Color Management */
#define CRXDBG_COLOR_LEN 12
#define CRXDBG_COLOR_D(color, code) \
	{color, sizeof(color)-1, code}
#define CRXDBG_COLOR_END \
	{NULL, 0L, {0}}
#define CRXDBG_ELEMENT_LEN 3
#define CRXDBG_ELEMENT_D(name, id) \
	{name, sizeof(name)-1, id}
#define CRXDBG_ELEMENT_END \
	{NULL, 0L, 0}

#define CRXDBG_COLOR_INVALID	-1
#define CRXDBG_COLOR_PROMPT 	 0
#define CRXDBG_COLOR_ERROR		 1
#define CRXDBG_COLOR_NOTICE		 2
#define CRXDBG_COLORS			 3

typedef struct _crxdbg_color_t {
	char       *name;
	size_t      name_length;
	const char  code[CRXDBG_COLOR_LEN];
} crxdbg_color_t;

typedef struct _crxdbg_element_t {
	char		*name;
	size_t		name_length;
	int			id;
} crxdbg_element_t;

CRXDBG_API const crxdbg_color_t *crxdbg_get_color(const char *name, size_t name_length);
CRXDBG_API void crxdbg_set_color(int element, const crxdbg_color_t *color);
CRXDBG_API void crxdbg_set_color_ex(int element, const char *name, size_t name_length);
CRXDBG_API const crxdbg_color_t *crxdbg_get_colors(void);
CRXDBG_API int crxdbg_get_element(const char *name, size_t len); /* }}} */

/* {{{ Prompt Management */
CRXDBG_API void crxdbg_set_prompt(const char*);
CRXDBG_API const char *crxdbg_get_prompt(void); /* }}} */

/* {{{ Console size */
CRXDBG_API uint32_t crxdbg_get_terminal_width(void);
CRXDBG_API uint32_t crxdbg_get_terminal_height(void); /* }}} */

CRXDBG_API void crxdbg_set_async_io(int fd);

int crxdbg_rebuild_symtable(void);

int crxdbg_safe_class_lookup(const char *name, int name_length, crex_class_entry **ce);

char *crxdbg_get_property_key(char *key);

typedef int (*crxdbg_parse_var_func)(char *name, size_t len, char *keyname, size_t keylen, HashTable *parent, zval *zv);
typedef int (*crxdbg_parse_var_with_arg_func)(char *name, size_t len, char *keyname, size_t keylen, HashTable *parent, zval *zv, void *arg);

CRXDBG_API int crxdbg_parse_variable(char *input, size_t len, HashTable *parent, size_t i, crxdbg_parse_var_func callback, bool silent);
CRXDBG_API int crxdbg_parse_variable_with_arg(char *input, size_t len, HashTable *parent, size_t i, crxdbg_parse_var_with_arg_func callback, crxdbg_parse_var_with_arg_func step_cb, bool silent, void *arg);

int crxdbg_is_auto_global(char *name, int len);

char *crxdbg_short_zval_print(zval *zv, int maxlen);

CRXDBG_API bool crxdbg_check_caught_ex(crex_execute_data *ex, crex_object *exception);

static crex_always_inline crex_execute_data *crxdbg_user_execute_data(crex_execute_data *ex) {
	while (!ex->func || !CREX_USER_CODE(ex->func->common.type)) {
		ex = ex->prev_execute_data;
		CREX_ASSERT(ex);
	}
	return ex;
}

#ifdef ZTS
#define CRXDBG_OUTPUT_BACKUP_DEFINES() \
	crex_output_globals *output_globals_ptr; \
	crex_output_globals original_output_globals; \
	output_globals_ptr = TSRMG_BULK_STATIC(output_globals_id, crex_output_globals *);
#else
#define CRXDBG_OUTPUT_BACKUP_DEFINES() \
	crex_output_globals *output_globals_ptr; \
	crex_output_globals original_output_globals; \
	output_globals_ptr = &output_globals;
#endif

#define CRXDBG_OUTPUT_BACKUP_SWAP() \
	original_output_globals = *output_globals_ptr; \
	memset(output_globals_ptr, 0, sizeof(crex_output_globals)); \
	crx_output_activate();

#define CRXDBG_OUTPUT_BACKUP() \
	CRXDBG_OUTPUT_BACKUP_DEFINES() \
	CRXDBG_OUTPUT_BACKUP_SWAP()

#define CRXDBG_OUTPUT_BACKUP_RESTORE() \
	crx_output_deactivate(); \
	*output_globals_ptr = original_output_globals;

#endif /* CRXDBG_UTILS_H */
