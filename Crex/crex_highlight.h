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

#ifndef CREX_HIGHLIGHT_H
#define CREX_HIGHLIGHT_H

#define HL_COMMENT_COLOR     "#FF8000"    /* orange */
#define HL_DEFAULT_COLOR     "#0000BB"    /* blue */
#define HL_HTML_COLOR        "#000000"    /* black */
#define HL_STRING_COLOR      "#DD0000"    /* red */
#define HL_KEYWORD_COLOR     "#007700"    /* green */


typedef struct _crex_syntax_highlighter_ini {
	char *highlight_html;
	char *highlight_comment;
	char *highlight_default;
	char *highlight_string;
	char *highlight_keyword;
} crex_syntax_highlighter_ini;


BEGIN_EXTERN_C()
CREX_API void crex_highlight(crex_syntax_highlighter_ini *syntax_highlighter_ini);
CREX_API void crex_strip(void);
CREX_API crex_result highlight_file(const char *filename, crex_syntax_highlighter_ini *syntax_highlighter_ini);
CREX_API void highlight_string(crex_string *str, crex_syntax_highlighter_ini *syntax_highlighter_ini, const char *str_name);
CREX_API void crex_html_putc(char c);
CREX_API void crex_html_puts(const char *s, size_t len);
END_EXTERN_C()

extern crex_syntax_highlighter_ini syntax_highlighter_ini;

#endif
