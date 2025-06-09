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

#ifndef CREX_SCANNER_H
#define CREX_SCANNER_H

typedef struct _crex_lex_state {
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

	crex_file_handle *in;
	uint32_t lineno;
	crex_string *filename;

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

	/* hooks */
	void (*on_event)(
		crex_crx_scanner_event event, int token, int line,
		const char *text, size_t length, void *context);
	void *on_event_context;

	crex_ast *ast;
	crex_arena *ast_arena;
} crex_lex_state;

typedef struct _crex_heredoc_label {
	char *label;
	int length;
	int indentation;
	bool indentation_uses_spaces;
} crex_heredoc_label;

/* Track locations of unclosed {, [, (, etc. for better syntax error reporting */
typedef struct _crex_nest_location {
	char text;
	int  lineno;
} crex_nest_location;

BEGIN_EXTERN_C()
CREX_API void crex_save_lexical_state(crex_lex_state *lex_state);
CREX_API void crex_restore_lexical_state(crex_lex_state *lex_state);
CREX_API void crex_prepare_string_for_scanning(zval *str, crex_string *filename);
CREX_API void crex_multibyte_yyinput_again(crex_encoding_filter old_input_filter, const crex_encoding *old_encoding);
CREX_API crex_result crex_multibyte_set_filter(const crex_encoding *onetime_encoding);
CREX_API crex_result crex_lex_tstring(zval *zv, unsigned char *ident);

END_EXTERN_C()

#endif
