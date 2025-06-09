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

#ifndef CRXDBG_CMD_H
#define CRXDBG_CMD_H

#include "TSRM.h"
#include "crex_generators.h"

/* {{{ Command and Parameter */
enum {
	NO_ARG = 0,
	REQUIRED_ARG,
	OPTIONAL_ARG
};

typedef enum {
	EMPTY_PARAM = 0,
	ADDR_PARAM,
	FILE_PARAM,
	NUMERIC_FILE_PARAM,
	METHOD_PARAM,
	STR_PARAM,
	NUMERIC_PARAM,
	NUMERIC_FUNCTION_PARAM,
	NUMERIC_METHOD_PARAM,
	STACK_PARAM,
	EVAL_PARAM,
	SHELL_PARAM,
	COND_PARAM,
	OP_PARAM,
	ORIG_PARAM,
	RUN_PARAM
} crxdbg_param_type;

typedef struct _crxdbg_param crxdbg_param_t;
struct _crxdbg_param {
	crxdbg_param_type type;
	crex_long num;
	crex_ulong addr;
	struct {
		char *name;
		crex_ulong line;
	} file;
	struct {
		char *class;
		char *name;
	} method;
	char *str;
	size_t len;
	crxdbg_param_t *next;
	crxdbg_param_t *top;
};

#define crxdbg_init_param(v, t) do{ \
	(v)->type = (t); \
	(v)->addr = 0; \
	(v)->num = 0; \
	(v)->file.name = NULL; \
	(v)->file.line = 0; \
	(v)->method.class = NULL; \
	(v)->method.name = NULL; \
	(v)->str = NULL; \
	(v)->len = 0; \
	(v)->next = NULL; \
	(v)->top = NULL; \
} while(0)

#define CRXDBG_ASYNC_SAFE 1

typedef int (*crxdbg_command_handler_t)(const crxdbg_param_t*);

typedef struct _crxdbg_command_t crxdbg_command_t;
struct _crxdbg_command_t {
	const char *name;                   /* Command name */
	size_t name_len;                    /* Command name length */
	const char *tip;                    /* Menu tip */
	size_t tip_len;                     /* Menu tip length */
	char alias;                         /* Alias */
	crxdbg_command_handler_t handler;   /* Command handler */
	const crxdbg_command_t *subs;       /* Sub Commands */
	char *args;                         /* Argument Spec */
	const crxdbg_command_t *parent;     /* Parent Command */
	bool flags;                    /* General flags */
};
/* }}} */

/* {{{ misc */
#define CRXDBG_STRL(s) s, sizeof(s)-1
#define CRXDBG_MAX_CMD 500
#define CRXDBG_FRAME(v) (CRXDBG_G(frame).v)
#define CRXDBG_EX(v) (EG(current_execute_data)->v)

typedef struct {
	int num;
	crex_generator *generator;
	crex_execute_data *execute_data;
} crxdbg_frame_t;
/* }}} */

/*
* Workflow:
* 1) the lexer/parser creates a stack of commands and arguments from input
* 2) the commands at the top of the stack are resolved sensibly using aliases, abbreviations and case insensitive matching
* 3) the remaining arguments in the stack are verified (optionally) against the handlers declared argument specification
* 4) the handler is called passing the top of the stack as the only parameter
* 5) the stack is destroyed upon return from the handler
*/

/*
* Input Management
*/
CRXDBG_API char *crxdbg_read_input(const char *buffered);
CRXDBG_API void crxdbg_destroy_input(char **input);
CRXDBG_API int crxdbg_ask_user_permission(const char *question);

/**
 * Stack Management
 */
CRXDBG_API void crxdbg_stack_push(crxdbg_param_t *stack, crxdbg_param_t *param);
CRXDBG_API void crxdbg_stack_separate(crxdbg_param_t *param);
CRXDBG_API const crxdbg_command_t *crxdbg_stack_resolve(const crxdbg_command_t *commands, const crxdbg_command_t *parent, crxdbg_param_t **top);
CRXDBG_API int crxdbg_stack_verify(const crxdbg_command_t *command, crxdbg_param_t **stack);
CRXDBG_API int crxdbg_stack_execute(crxdbg_param_t *stack, bool allow_async_unsafe);
CRXDBG_API void crxdbg_stack_free(crxdbg_param_t *stack);

/*
* Parameter Management
*/
CRXDBG_API void crxdbg_clear_param(crxdbg_param_t*);
CRXDBG_API void crxdbg_copy_param(const crxdbg_param_t*, crxdbg_param_t*);
CRXDBG_API bool crxdbg_match_param(const crxdbg_param_t *, const crxdbg_param_t *);
CRXDBG_API crex_ulong crxdbg_hash_param(const crxdbg_param_t *);
CRXDBG_API const char* crxdbg_get_param_type(const crxdbg_param_t*);
CRXDBG_API char* crxdbg_param_tostring(const crxdbg_param_t *param, char **pointer);
CRXDBG_API void crxdbg_param_debug(const crxdbg_param_t *param, const char *msg);

/**
 * Command Declarators
 */
#define CRXDBG_COMMAND_HANDLER(name) crxdbg_do_##name

#define CRXDBG_COMMAND_D_EXP(name, tip, alias, handler, children, args, parent, flags) \
	{CRXDBG_STRL(#name), tip, sizeof(tip)-1, alias, crxdbg_do_##handler, children, args, parent, flags}

#define CRXDBG_COMMAND_D_EX(name, tip, alias, handler, children, args, flags) \
	{CRXDBG_STRL(#name), tip, sizeof(tip)-1, alias, crxdbg_do_##handler, children, args, NULL, flags}

#define CRXDBG_COMMAND_D(name, tip, alias, children, args, flags) \
	{CRXDBG_STRL(#name), tip, sizeof(tip)-1, alias, crxdbg_do_##name, children, args, NULL, flags}

#define CRXDBG_COMMAND(name) int crxdbg_do_##name(const crxdbg_param_t *param)

#define CRXDBG_COMMAND_ARGS param

#define CRXDBG_END_COMMAND {NULL, 0, NULL, 0, '\0', NULL, NULL, NULL, NULL, 0}

/*
* Default Switch Case
*/
#define crxdbg_default_switch_case() \
	default: \
		crxdbg_error("Unsupported parameter type (%s) for command", crxdbg_get_param_type(param)); \
	break

#endif /* CRXDBG_CMD_H */
