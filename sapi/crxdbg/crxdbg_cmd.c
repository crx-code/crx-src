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

#include "crxdbg.h"
#include "crxdbg_cmd.h"
#include "crxdbg_utils.h"
#include "crxdbg_set.h"
#include "crxdbg_prompt.h"
#include "crxdbg_io.h"

CREX_EXTERN_MODULE_GLOBALS(crxdbg)

static inline const char *crxdbg_command_name(const crxdbg_command_t *command, char *buffer) {
	size_t pos = 0;

	if (command->parent) {
		memcpy(&buffer[pos], command->parent->name, command->parent->name_len);
		pos += command->parent->name_len;
		memcpy(&buffer[pos], " ", sizeof(" ")-1);
		pos += (sizeof(" ")-1);
	}

	memcpy(&buffer[pos], command->name, command->name_len);
	pos += command->name_len;
	buffer[pos] = 0;

	return buffer;
}

CRXDBG_API const char *crxdbg_get_param_type(const crxdbg_param_t *param) /* {{{ */
{
	switch (param->type) {
		case STACK_PARAM:
			return "stack";
		case EMPTY_PARAM:
			return "empty";
		case ADDR_PARAM:
			return "address";
		case NUMERIC_PARAM:
			return "numeric";
		case METHOD_PARAM:
			return "method";
		case NUMERIC_FUNCTION_PARAM:
			return "function opline";
		case NUMERIC_METHOD_PARAM:
			return "method opline";
		case FILE_PARAM:
			return "file or file opline";
		case STR_PARAM:
			return "string";
		default: /* this is bad */
			return "unknown";
	}
}

CRXDBG_API void crxdbg_clear_param(crxdbg_param_t *param) /* {{{ */
{
	if (param) {
		switch (param->type) {
			case FILE_PARAM:
				efree(param->file.name);
				break;
			case METHOD_PARAM:
				efree(param->method.class);
				efree(param->method.name);
				break;
			case STR_PARAM:
				efree(param->str);
				break;
			default:
				break;
		}
	}

} /* }}} */

CRXDBG_API char* crxdbg_param_tostring(const crxdbg_param_t *param, char **pointer) /* {{{ */
{
	switch (param->type) {
		case STR_PARAM:
			CREX_IGNORE_VALUE(asprintf(pointer, "%s", param->str));
		break;

		case ADDR_PARAM:
			CREX_IGNORE_VALUE(asprintf(pointer, CREX_ULONG_FMT, param->addr));
		break;

		case NUMERIC_PARAM:
			CREX_IGNORE_VALUE(asprintf(pointer, CREX_LONG_FMT, param->num));
		break;

		case METHOD_PARAM:
			CREX_IGNORE_VALUE(asprintf(pointer, "%s::%s", param->method.class, param->method.name));
		break;

		case FILE_PARAM:
			if (param->num) {
				CREX_IGNORE_VALUE(asprintf(pointer, "%s:"CREX_ULONG_FMT"#"CREX_ULONG_FMT, param->file.name, param->file.line, param->num));
			} else {
				CREX_IGNORE_VALUE(asprintf(pointer, "%s:"CREX_ULONG_FMT, param->file.name, param->file.line));
			}
		break;

		case NUMERIC_FUNCTION_PARAM:
			CREX_IGNORE_VALUE(asprintf(pointer, "%s#"CREX_ULONG_FMT, param->str, param->num));
		break;

		case NUMERIC_METHOD_PARAM:
			CREX_IGNORE_VALUE(asprintf(pointer, "%s::%s#"CREX_ULONG_FMT, param->method.class, param->method.name, param->num));
		break;

		default:
			*pointer = strdup("unknown");
	}

	return *pointer;
} /* }}} */

CRXDBG_API void crxdbg_copy_param(const crxdbg_param_t* src, crxdbg_param_t* dest) /* {{{ */
{
	switch ((dest->type = src->type)) {
		case STACK_PARAM:
			/* nope */
		break;

		case STR_PARAM:
			dest->str = estrndup(src->str, src->len);
			dest->len = src->len;
		break;

		case OP_PARAM:
			dest->str = estrndup(src->str, src->len);
			dest->len = src->len;
		break;

		case ADDR_PARAM:
			dest->addr = src->addr;
		break;

		case NUMERIC_PARAM:
			dest->num = src->num;
		break;

		case METHOD_PARAM:
			dest->method.class = estrdup(src->method.class);
			dest->method.name = estrdup(src->method.name);
		break;

		case NUMERIC_FILE_PARAM:
		case FILE_PARAM:
			dest->file.name = estrdup(src->file.name);
			dest->file.line = src->file.line;
			if (src->num)
				dest->num   = src->num;
		break;

		case NUMERIC_FUNCTION_PARAM:
			dest->str = estrndup(src->str, src->len);
			dest->num = src->num;
			dest->len = src->len;
		break;

		case NUMERIC_METHOD_PARAM:
			dest->method.class = estrdup(src->method.class);
			dest->method.name = estrdup(src->method.name);
			dest->num = src->num;
		break;

		case EMPTY_PARAM: { /* do nothing */ } break;

		default: {
			/* not yet */
		}
	}
} /* }}} */

CRXDBG_API crex_ulong crxdbg_hash_param(const crxdbg_param_t *param) /* {{{ */
{
	crex_ulong hash = param->type;

	switch (param->type) {
		case STACK_PARAM:
			/* nope */
		break;

		case STR_PARAM:
			hash += crex_hash_func(param->str, param->len);
		break;

		case METHOD_PARAM:
			hash += crex_hash_func(param->method.class, strlen(param->method.class));
			hash += crex_hash_func(param->method.name, strlen(param->method.name));
		break;

		case FILE_PARAM:
			hash += crex_hash_func(param->file.name, strlen(param->file.name));
			hash += param->file.line;
			if (param->num)
				hash += param->num;
		break;

		case ADDR_PARAM:
			hash += param->addr;
		break;

		case NUMERIC_PARAM:
			hash += param->num;
		break;

		case NUMERIC_FUNCTION_PARAM:
			hash += crex_hash_func(param->str, param->len);
			hash += param->num;
		break;

		case NUMERIC_METHOD_PARAM:
			hash += crex_hash_func(param->method.class, strlen(param->method.class));
			hash += crex_hash_func(param->method.name, strlen(param->method.name));
			if (param->num)
				hash+= param->num;
		break;

		case EMPTY_PARAM: { /* do nothing */ } break;

		default: {
			/* not yet */
		}
	}

	return hash;
} /* }}} */

CRXDBG_API bool crxdbg_match_param(const crxdbg_param_t *l, const crxdbg_param_t *r) /* {{{ */
{
	if (l && r) {
		if (l->type == r->type) {
			switch (l->type) {
				case STACK_PARAM:
					/* nope, or yep */
					return 1;
				break;

				case NUMERIC_FUNCTION_PARAM:
					if (l->num != r->num) {
						break;
					}
				CREX_FALLTHROUGH;

				case STR_PARAM:
					return (l->len == r->len) &&
							(memcmp(l->str, r->str, l->len) == SUCCESS);

				case NUMERIC_PARAM:
					return (l->num == r->num);

				case ADDR_PARAM:
					return (l->addr == r->addr);

				case FILE_PARAM: {
					if (l->file.line == r->file.line) {
						size_t lengths[2] = {
							strlen(l->file.name), strlen(r->file.name)};

						if (lengths[0] == lengths[1]) {
							if ((!l->num && !r->num) || (l->num == r->num)) {
								return (memcmp(
									l->file.name, r->file.name, lengths[0]) == SUCCESS);
							}
						}
					}
				} break;

				case NUMERIC_METHOD_PARAM:
					if (l->num != r->num) {
						break;
					}
				CREX_FALLTHROUGH;

				case METHOD_PARAM: {
					size_t lengths[2] = {
						strlen(l->method.class), strlen(r->method.class)};
					if (lengths[0] == lengths[1]) {
						if (memcmp(l->method.class, r->method.class, lengths[0]) == SUCCESS) {
							lengths[0] = strlen(l->method.name);
							lengths[1] = strlen(r->method.name);

							if (lengths[0] == lengths[1]) {
								return (memcmp(
									l->method.name, r->method.name, lengths[0]) == SUCCESS);
							}
						}
					}
				} break;

				case EMPTY_PARAM:
					return 1;

				default: {
					/* not yet */
				}
			}
		}
	}
	return 0;
} /* }}} */

/* {{{ */
CRXDBG_API void crxdbg_param_debug(const crxdbg_param_t *param, const char *msg) {
	if (param && param->type) {
		switch (param->type) {
			case STR_PARAM:
				fprintf(stderr, "%s STR_PARAM(%s=%zu)\n", msg, param->str, param->len);
			break;

			case ADDR_PARAM:
				fprintf(stderr, "%s ADDR_PARAM(" CREX_ULONG_FMT ")\n", msg, param->addr);
			break;

			case NUMERIC_FILE_PARAM:
				fprintf(stderr, "%s NUMERIC_FILE_PARAM(%s:#"CREX_ULONG_FMT")\n", msg, param->file.name, param->file.line);
			break;

			case FILE_PARAM:
				fprintf(stderr, "%s FILE_PARAM(%s:"CREX_ULONG_FMT")\n", msg, param->file.name, param->file.line);
			break;

			case METHOD_PARAM:
				fprintf(stderr, "%s METHOD_PARAM(%s::%s)\n", msg, param->method.class, param->method.name);
			break;

			case NUMERIC_METHOD_PARAM:
				fprintf(stderr, "%s NUMERIC_METHOD_PARAM(%s::%s)\n", msg, param->method.class, param->method.name);
			break;

			case NUMERIC_FUNCTION_PARAM:
				fprintf(stderr, "%s NUMERIC_FUNCTION_PARAM(%s::"CREX_LONG_FMT")\n", msg, param->str, param->num);
			break;

			case NUMERIC_PARAM:
				fprintf(stderr, "%s NUMERIC_PARAM("CREX_LONG_FMT")\n", msg, param->num);
			break;

			case COND_PARAM:
				fprintf(stderr, "%s COND_PARAM(%s=%zu)\n", msg, param->str, param->len);
			break;

			case OP_PARAM:
				fprintf(stderr, "%s OP_PARAM(%s=%zu)\n", msg, param->str, param->len);
			break;

			default: {
				/* not yet */
			}
		}
	}
} /* }}} */

/* {{{ */
CRXDBG_API void crxdbg_stack_free(crxdbg_param_t *stack) {
	CREX_ASSERT(stack != NULL);

	if (stack->next) {
		crxdbg_param_t *remove = stack->next;

		while (remove) {
			crxdbg_param_t *next = NULL;

			if (remove->next)
				next = remove->next;

			switch (remove->type) {
				case NUMERIC_METHOD_PARAM:
				case METHOD_PARAM:
					if (remove->method.class) {
						efree(remove->method.class);
					}
					if (remove->method.name) {
						efree(remove->method.name);
					}
				break;

				case NUMERIC_FUNCTION_PARAM:
				case STR_PARAM:
				case OP_PARAM:
				case EVAL_PARAM:
				case SHELL_PARAM:
				case COND_PARAM:
				case RUN_PARAM:
					if (remove->str) {
						efree(remove->str);
					}
				break;

				case NUMERIC_FILE_PARAM:
				case FILE_PARAM:
					if (remove->file.name) {
						efree(remove->file.name);
					}
				break;

				default: {
					/* nothing */
				}
			}

			free(remove);
			remove = NULL;

			if (next)
				remove = next;
			else break;
		}

		stack->next = NULL;
	}
} /* }}} */

/* {{{ */
CRXDBG_API void crxdbg_stack_push(crxdbg_param_t *stack, crxdbg_param_t *param) {
	crxdbg_param_t *next = calloc(1, sizeof(crxdbg_param_t));

	if (!next) {
		return;
	}

	*(next) = *(param);

	next->next = NULL;

	if (stack->top == NULL) {
		stack->top = next;
		next->top = NULL;
		stack->next = next;
	} else {
		stack->top->next = next;
		next->top = stack->top;
		stack->top = next;
	}

	stack->len++;
} /* }}} */

/* {{{ */
CRXDBG_API void crxdbg_stack_separate(crxdbg_param_t *param) {
	crxdbg_param_t *stack = calloc(1, sizeof(crxdbg_param_t));

	stack->type = STACK_PARAM;
	stack->next = param->next;
	param->next = stack;
	stack->top = param->top;
} /* }}} */

CRXDBG_API int crxdbg_stack_verify(const crxdbg_command_t *command, crxdbg_param_t **stack) {
	if (command) {
		char buffer[128] = {0,};
		const crxdbg_param_t *top = (stack != NULL) ? *stack : NULL;
		const char *arg = command->args;
		crex_ulong least = 0L,
		           received = 0L,
		           current = 0L;
		bool optional = 0;

		/* check for arg spec */
		if (!(arg) || !(*arg)) {
			if (!top || top->type == STACK_PARAM) {
				return SUCCESS;
			}

			crxdbg_error("The command \"%s\" expected no arguments",
				crxdbg_command_name(command, buffer));
			return FAILURE;
		}

		least = 0L;

		/* count least amount of arguments */
		while (arg && *arg) {
			if (arg[0] == '|') {
				break;
			}
			least++;
			arg++;
		}

		arg = command->args;

#define verify_arg(e, a, t) if (!(a)) { \
	if (!optional) { \
		crxdbg_error("The command \"%s\" expected %s and got nothing at parameter "CREX_ULONG_FMT, \
			crxdbg_command_name(command, buffer), \
			(e), \
			current); \
		return FAILURE;\
	} \
} else if ((a)->type != (t)) { \
	crxdbg_error("The command \"%s\" expected %s and got %s at parameter "CREX_ULONG_FMT, \
		crxdbg_command_name(command, buffer), \
		(e),\
		crxdbg_get_param_type((a)), \
		current); \
	return FAILURE; \
}

		while (arg && *arg) {
			if (top && top->type == STACK_PARAM) {
				break;
			}

			current++;

			switch (*arg) {
				case '|': {
					current--;
					optional = 1;
					arg++;
				} continue;

				case 'i': verify_arg("raw input", top, STR_PARAM); break;
				case 's': verify_arg("string", top, STR_PARAM); break;
				case 'n': verify_arg("number", top, NUMERIC_PARAM); break;
				case 'm': verify_arg("method", top, METHOD_PARAM); break;
				case 'a': verify_arg("address", top, ADDR_PARAM); break;
				case 'f': verify_arg("file:line", top, FILE_PARAM); break;
				case 'c': verify_arg("condition", top, COND_PARAM); break;
				case 'o': verify_arg("opcode", top, OP_PARAM); break;
				case 'b': verify_arg("boolean", top, NUMERIC_PARAM); break;

				case '*': { /* do nothing */ } break;
			}

			if (top) {
				top = top->next;
			} else {
				break;
			}

			received++;
			arg++;
		}

#undef verify_arg

		if ((received < least)) {
			crxdbg_error("The command \"%s\" expected at least "CREX_ULONG_FMT" arguments (%s) and received "CREX_ULONG_FMT,
				crxdbg_command_name(command, buffer),
				least,
				command->args,
				received);
			return FAILURE;
		}
	}

	return SUCCESS;
}

/* {{{ */
CRXDBG_API const crxdbg_command_t *crxdbg_stack_resolve(const crxdbg_command_t *commands, const crxdbg_command_t *parent, crxdbg_param_t **top) {
	const crxdbg_command_t *command = commands;
	crxdbg_param_t *name = *top;
	const crxdbg_command_t *matched[3] = {NULL, NULL, NULL};
	crex_ulong matches = 0L;

	while (command && command->name && command->handler) {
		if (name->len == 1 || command->name_len >= name->len) {
			/* match single letter alias */
			if (command->alias && (name->len == 1)) {
				if (command->alias == (*name->str)) {
					matched[matches] = command;
					matches++;
				}
			} else {
				/* match full, case insensitive, command name */
				if (strncasecmp(command->name, name->str, name->len) == SUCCESS) {
					if (matches < 3) {
						/* only allow abbreviating commands that can be aliased */
						if ((name->len != command->name_len && command->alias) || name->len == command->name_len) {
							matched[matches] = command;
							matches++;
						}

						/* exact match */
						if (name->len == command->name_len) {
							break;
						}
					} else {
						break;
					}
				}
			}
		}

		command++;
	}

	switch (matches) {
		case 0:
			if (parent) {
				crxdbg_error("The command \"%s %s\" could not be found", parent->name, name->str);
			} else {
				crxdbg_error("The command \"%s\" could not be found", name->str);
			}
			return parent;

		case 1:
			(*top) = (*top)->next;

			command = matched[0];
			break;

		default: {
			char *list = NULL;
			uint32_t it = 0;
			size_t pos = 0;

			while (it < matches) {
				if (!list) {
					list = emalloc(matched[it]->name_len + 1 + (it + 1 < matches ? sizeof(", ") - 1 : 0));
				} else {
					list = erealloc(list, (pos + matched[it]->name_len) + 1 + (it + 1 < matches ? sizeof(", ") - 1 : 0));
				}
				memcpy(&list[pos], matched[it]->name, matched[it]->name_len);
				pos += matched[it]->name_len;
				if ((it + 1) < matches) {
					memcpy(&list[pos], ", ", sizeof(", ") - 1);
					pos += (sizeof(", ") - 1);
				}

				list[pos] = 0;
				it++;
			}

			/* ", " separated matches */
			crxdbg_error("The command \"%s\" is ambiguous, matching "CREX_ULONG_FMT" commands (%s)", name->str, matches, list);
			efree(list);

			return NULL;
		}
	}

	if (command->subs && (*top) && ((*top)->type == STR_PARAM)) {
		return crxdbg_stack_resolve(command->subs, command, top);
	} else {
		return command;
	}

	return NULL;
} /* }}} */

static int crxdbg_internal_stack_execute(crxdbg_param_t *stack, bool allow_async_unsafe) {
	const crxdbg_command_t *handler = NULL;
	crxdbg_param_t *top = (crxdbg_param_t *) stack->next;

	switch (top->type) {
		case EVAL_PARAM:
			crxdbg_activate_err_buf(0);
			crxdbg_free_err_buf();
			return CRXDBG_COMMAND_HANDLER(ev)(top);

		case RUN_PARAM:
			if (!allow_async_unsafe) {
				crxdbg_error("run command is disallowed during hard interrupt");
			}
			crxdbg_activate_err_buf(0);
			crxdbg_free_err_buf();
			return CRXDBG_COMMAND_HANDLER(run)(top);

		case SHELL_PARAM:
			if (!allow_async_unsafe) {
				crxdbg_error("sh command is disallowed during hard interrupt");
				return FAILURE;
			}
			crxdbg_activate_err_buf(0);
			crxdbg_free_err_buf();
			return CRXDBG_COMMAND_HANDLER(sh)(top);

		case STR_PARAM: {
			handler = crxdbg_stack_resolve(crxdbg_prompt_commands, NULL, &top);

			if (handler) {
				if (!allow_async_unsafe && !(handler->flags & CRXDBG_ASYNC_SAFE)) {
					crxdbg_error("%s command is disallowed during hard interrupt", handler->name);
					return FAILURE;
				}

				if (crxdbg_stack_verify(handler, &top) == SUCCESS) {
					crxdbg_activate_err_buf(0);
					crxdbg_free_err_buf();
					return handler->handler(top);
				}
			}
		} return FAILURE;

		default:
			crxdbg_error("The first parameter makes no sense !");
			return FAILURE;
	}

	return SUCCESS;
} /* }}} */

/* {{{ */
CRXDBG_API int crxdbg_stack_execute(crxdbg_param_t *stack, bool allow_async_unsafe) {
	crxdbg_param_t *top = stack;

	if (stack->type != STACK_PARAM) {
		crxdbg_error("The passed argument was not a stack !");
		return FAILURE;
	}

	if (!stack->len) {
		crxdbg_error("The stack contains nothing !");
		return FAILURE;
	}

	do {
		if (top->type == STACK_PARAM) {
			int result;
			if ((result = crxdbg_internal_stack_execute(top, allow_async_unsafe)) != SUCCESS) {
				return result;
			}
		}
	} while ((top = top->next));

	return SUCCESS;
} /* }}} */

CRXDBG_API char *crxdbg_read_input(const char *buffered) /* {{{ */
{
	char buf[CRXDBG_MAX_CMD];
	char *buffer = NULL;

	if ((CRXDBG_G(flags) & (CRXDBG_IS_STOPPING | CRXDBG_IS_RUNNING)) != CRXDBG_IS_STOPPING) {
		if (buffered == NULL) {
#ifdef HAVE_CRXDBG_READLINE
			char *cmd = readline(crxdbg_get_prompt());
			CRXDBG_G(last_was_newline) = 1;

			if (!cmd) {
				CRXDBG_G(flags) |= CRXDBG_IS_QUITTING;
				crex_bailout();
			}

			add_history(cmd);
			buffer = estrdup(cmd);
			free(cmd);
#else
			crxdbg_write("%s", crxdbg_get_prompt());
			crxdbg_consume_stdin_line(buf);
			buffer = estrdup(buf);
#endif
		} else {
			buffer = estrdup(buffered);
		}
	}

	if (buffer && isspace(*buffer)) {
		char *trimmed = buffer;
		while (isspace(*trimmed))
			trimmed++;

		trimmed = estrdup(trimmed);
		efree(buffer);
		buffer = trimmed;
	}

	if (buffer && strlen(buffer)) {
		if (CRXDBG_G(buffer)) {
			free(CRXDBG_G(buffer));
		}
		CRXDBG_G(buffer) = strdup(buffer);
	} else if (CRXDBG_G(buffer)) {
		if (buffer) {
			efree(buffer);
		}
		buffer = estrdup(CRXDBG_G(buffer));
	}

	return buffer;
} /* }}} */

CRXDBG_API void crxdbg_destroy_input(char **input) /*{{{ */
{
	efree(*input);
} /* }}} */

CRXDBG_API int crxdbg_ask_user_permission(const char *question) {
	char buf[CRXDBG_MAX_CMD];
	crxdbg_out("%s", question);
	crxdbg_out(" (type y or n): ");

	while (1) {
		crxdbg_consume_stdin_line(buf);
		if ((buf[1] == '\n' || (buf[1] == '\r' && buf[2] == '\n')) && (buf[0] == 'y' || buf[0] == 'n')) {
			if (buf[0] == 'y') {
				return SUCCESS;
			}
			return FAILURE;
		}
		crxdbg_out("Please enter either y (yes) or n (no): ");
	}

	return SUCCESS;
}
