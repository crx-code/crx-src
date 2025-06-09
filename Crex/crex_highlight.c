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

#include "crex.h"
#include <crex_language_parser.h>
#include "crex_compile.h"
#include "crex_highlight.h"
#include "crex_ptr_stack.h"
#include "crex_globals.h"
#include "crex_exceptions.h"

CREX_API void crex_html_putc(char c)
{
	switch (c) {
		case '<':
			CREX_PUTS("&lt;");
			break;
		case '>':
			CREX_PUTS("&gt;");
			break;
		case '&':
			CREX_PUTS("&amp;");
			break;
		case '\t':
			CREX_PUTS("    ");
			break;
		default:
			CREX_PUTC(c);
			break;
	}
}


CREX_API void crex_html_puts(const char *s, size_t len)
{
	const unsigned char *ptr = (const unsigned char*)s, *end = ptr + len;
	unsigned char *filtered = NULL;
	size_t filtered_len;

	if (LANG_SCNG(output_filter)) {
		LANG_SCNG(output_filter)(&filtered, &filtered_len, ptr, len);
		ptr = filtered;
		end = filtered + filtered_len;
	}

	while (ptr<end) {
		if (*ptr==' ') {
			do {
				crex_html_putc(*ptr);
			} while ((++ptr < end) && (*ptr==' '));
		} else {
			crex_html_putc(*ptr++);
		}
	}

	if (LANG_SCNG(output_filter)) {
		efree(filtered);
	}
}


CREX_API void crex_highlight(crex_syntax_highlighter_ini *syntax_highlighter_ini)
{
	zval token;
	int token_type;
	char *last_color = syntax_highlighter_ini->highlight_html;
	char *next_color;

	crex_printf("<pre><code style=\"color: %s\">", last_color);
	/* highlight stuff coming back from crexlex() */
	while ((token_type=lex_scan(&token, NULL))) {
		switch (token_type) {
			case T_INLINE_HTML:
				next_color = syntax_highlighter_ini->highlight_html;
				break;
			case T_COMMENT:
			case T_DOC_COMMENT:
				next_color = syntax_highlighter_ini->highlight_comment;
				break;
			case T_OPEN_TAG:
			case T_OPEN_TAG_WITH_ECHO:
			case T_CLOSE_TAG:
			case T_LINE:
			case T_FILE:
			case T_DIR:
			case T_TRAIT_C:
			case T_METHOD_C:
			case T_FUNC_C:
			case T_NS_C:
			case T_CLASS_C:
				next_color = syntax_highlighter_ini->highlight_default;
				break;
			case '"':
			case T_ENCAPSED_AND_WHITESPACE:
			case T_CONSTANT_ENCAPSED_STRING:
				next_color = syntax_highlighter_ini->highlight_string;
				break;
			case T_WHITESPACE:
				crex_html_puts((char*)LANG_SCNG(yy_text), LANG_SCNG(yy_leng));  /* no color needed */
				ZVAL_UNDEF(&token);
				continue;
				break;
			default:
				if (C_TYPE(token) == IS_UNDEF) {
					next_color = syntax_highlighter_ini->highlight_keyword;
				} else {
					next_color = syntax_highlighter_ini->highlight_default;
				}
				break;
		}

		if (last_color != next_color) {
			if (last_color != syntax_highlighter_ini->highlight_html) {
				crex_printf("</span>");
			}
			last_color = next_color;
			if (last_color != syntax_highlighter_ini->highlight_html) {
				crex_printf("<span style=\"color: %s\">", last_color);
			}
		}

		crex_html_puts((char*)LANG_SCNG(yy_text), LANG_SCNG(yy_leng));

		if (C_TYPE(token) == IS_STRING) {
			switch (token_type) {
				case T_OPEN_TAG:
				case T_OPEN_TAG_WITH_ECHO:
				case T_CLOSE_TAG:
				case T_WHITESPACE:
				case T_COMMENT:
				case T_DOC_COMMENT:
					break;
				default:
					zval_ptr_dtor_str(&token);
					break;
			}
		}
		ZVAL_UNDEF(&token);
	}

	if (last_color != syntax_highlighter_ini->highlight_html) {
		crex_printf("</span>");
	}
	crex_printf("</code></pre>");

	/* Discard parse errors thrown during tokenization */
	crex_clear_exception();
}

CREX_API void crex_strip(void)
{
	zval token;
	int token_type;
	int prev_space = 0;

	while ((token_type=lex_scan(&token, NULL))) {
		switch (token_type) {
			case T_WHITESPACE:
				if (!prev_space) {
					crex_write(" ", sizeof(" ") - 1);
					prev_space = 1;
				}
				CREX_FALLTHROUGH;
			case T_COMMENT:
			case T_DOC_COMMENT:
				ZVAL_UNDEF(&token);
				continue;

			case T_END_HEREDOC:
				crex_write((char*)LANG_SCNG(yy_text), LANG_SCNG(yy_leng));
				/* read the following character, either newline or ; */
				if (lex_scan(&token, NULL) != T_WHITESPACE) {
					crex_write((char*)LANG_SCNG(yy_text), LANG_SCNG(yy_leng));
				}
				crex_write("\n", sizeof("\n") - 1);
				prev_space = 1;
				ZVAL_UNDEF(&token);
				continue;

			default:
				crex_write((char*)LANG_SCNG(yy_text), LANG_SCNG(yy_leng));
				break;
		}

		if (C_TYPE(token) == IS_STRING) {
			switch (token_type) {
				case T_OPEN_TAG:
				case T_OPEN_TAG_WITH_ECHO:
				case T_CLOSE_TAG:
				case T_WHITESPACE:
				case T_COMMENT:
				case T_DOC_COMMENT:
					break;

				default:
					zval_ptr_dtor_str(&token);
					break;
			}
		}
		prev_space = 0;
		ZVAL_UNDEF(&token);
	}

	/* Discard parse errors thrown during tokenization */
	crex_clear_exception();
}
