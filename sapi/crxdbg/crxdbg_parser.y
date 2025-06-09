%require "3.0"
/*
 * crxdbg_parser.y
 * (from crx-src root)
 */

%code requires {
#include "crxdbg.h"
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif
}

%code {

#include "crxdbg_cmd.h"
#include "crxdbg_utils.h"
#include "crxdbg_cmd.h"
#include "crxdbg_prompt.h"

#include "crxdbg_parser.h"
#include "crxdbg_lexer.h"

#undef yyerror
static int yyerror(const char *msg);

CREX_EXTERN_MODULE_GLOBALS(crxdbg)

#ifdef _MSC_VER
#define YYMALLOC malloc
#define YYFREE free
#endif

}

%define api.prefix {crxdbg_}
%define api.pure full
%define api.value.type {crxdbg_param_t}
%define parse.error verbose

%token END 0 "end of command"
%token T_EVAL       "eval"
%token T_RUN        "run"
%token T_SHELL      "shell"
%token T_IF         "if (condition)"
%token T_TRUTHY     "truthy (true, on, yes or enabled)"
%token T_FALSY      "falsy (false, off, no or disabled)"
%token T_STRING     "string (some input, perhaps)"
%token T_COLON      ": (colon)"
%token T_DCOLON     ":: (double colon)"
%token T_POUND      "# (pound sign followed by digits)"
%token T_SEPARATOR  "# (pound sign)"
%token T_PROTO      "protocol (file://)"
%token T_DIGITS     "digits (numbers)"
%token T_LITERAL    "literal (string)"
%token T_ADDR       "address"
%token T_OPCODE     "opcode"
%token T_ID         "identifier (command or function name)"
%token T_INPUT      "input (input string or data)"
%token T_UNEXPECTED "input"
%token T_REQ_ID     "request id (-r %d)"

%% /* Rules */

input
	: command { $$ = $1; }
	| input T_SEPARATOR command { crxdbg_stack_separate($1.top); $$ = $3; }
	| %empty { (void) crxdbg_nerrs; }
	;

command
	: parameters { $$.top = CRXDBG_G(parser_stack)->top; }
	| full_expression { crxdbg_stack_push(CRXDBG_G(parser_stack), &$1); $$.top = CRXDBG_G(parser_stack)->top; }
	;

parameters
	: parameter { crxdbg_stack_push(CRXDBG_G(parser_stack), &$1); $$.top = CRXDBG_G(parser_stack)->top; }
	| parameters parameter { crxdbg_stack_push(CRXDBG_G(parser_stack), &$2); $$.top = CRXDBG_G(parser_stack)->top; }
	| parameters T_REQ_ID { $$ = $1; CRXDBG_G(req_id) = $2.num; }
	;

parameter
	: T_ID T_COLON T_DIGITS {
		$$.type = FILE_PARAM;
		$$.file.name = $2.str;
		$$.file.line = $3.num;
	}
	| T_ID T_COLON T_POUND T_DIGITS {
		$$.type = NUMERIC_FILE_PARAM;
		$$.file.name = $1.str;
		$$.file.line = $4.num;
	}
	| T_PROTO T_ID T_COLON T_DIGITS {
		$$.type = FILE_PARAM;
		$$.file.name = malloc($1.len + $2.len + 1);
		if ($$.file.name) {
			memcpy(&$$.file.name[0], $1.str, $1.len);
			memcpy(&$$.file.name[$1.len], $2.str, $2.len);
			$$.file.name[$1.len + $2.len] = '\0';
		}
		$$.file.line = $4.num;
	}
	| T_PROTO T_ID T_COLON T_POUND T_DIGITS {
		$$.type = NUMERIC_FILE_PARAM;
		$$.file.name = malloc($1.len + $2.len + 1);
		if ($$.file.name) {
			memcpy(&$$.file.name[0], $1.str, $1.len);
			memcpy(&$$.file.name[$1.len], $2.str, $2.len);
			$$.file.name[$1.len + $2.len] = '\0';
		}
		$$.file.line = $5.num;
	}
	| T_ID T_DCOLON T_ID {
		$$.type = METHOD_PARAM;
		$$.method.class = $1.str;
		$$.method.name = $3.str;
	}
	| T_ID T_DCOLON T_ID T_POUND T_DIGITS {
		$$.type = NUMERIC_METHOD_PARAM;
		$$.method.class = $1.str;
		$$.method.name = $3.str;
		$$.num = $5.num;
	}
	| T_ID T_POUND T_DIGITS {
		$$.type = NUMERIC_FUNCTION_PARAM;
		$$.str = $1.str;
		$$.len = $1.len;
		$$.num = $3.num;
	}
	| T_IF T_INPUT {
		$$.type = COND_PARAM;
		$$.str = $2.str;
		$$.len = $2.len;
	}
	| T_OPCODE { $$ = $1; }
	| T_ADDR { $$ = $1; }
	| T_LITERAL { $$ = $1; }
	| T_TRUTHY { $$ = $1; }
	| T_FALSY { $$ = $1; }
	| T_DIGITS { $$ = $1; }
	| T_ID { $$ = $1; }
	;

req_id
	: T_REQ_ID { CRXDBG_G(req_id) = $1.num; }
	| %empty
;

full_expression
	: T_EVAL req_id T_INPUT {
		$$.type = EVAL_PARAM;
		$$.str = $3.str;
		$$.len = $3.len;
	}
	| T_SHELL req_id T_INPUT {
		$$.type = SHELL_PARAM;
		$$.str = $3.str;
		$$.len = $3.len;
	}
	| T_RUN req_id {
		$$.type = RUN_PARAM;
		$$.len = 0;
	}
	| T_RUN req_id T_INPUT {
		$$.type = RUN_PARAM;
		$$.str = $3.str;
		$$.len = $3.len;
	}
	;

%%

static int yyerror(const char *msg) {
	crxdbg_error("Parse Error: %s", msg);

	{
		const crxdbg_param_t *top = CRXDBG_G(parser_stack);

		while (top) {
			crxdbg_param_debug(top, "--> ");
			top = top->next;
		}
	}
	return 0;
}

int crxdbg_do_parse(crxdbg_param_t *stack, char *input) {
	if (!*input) {
		return 0;
	}

	if (CRXDBG_G(cur_command)) {
		free(CRXDBG_G(cur_command));
	}
	CRXDBG_G(cur_command) = strdup(input);

	crxdbg_init_lexer(stack, input);

	return yyparse();
}
