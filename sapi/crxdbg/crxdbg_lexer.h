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

#ifndef CRXDBG_LEXER_H
#define CRXDBG_LEXER_H

#include "crxdbg_cmd.h"

typedef struct {
        unsigned int len;
        unsigned char *text;
        unsigned char *cursor;
        unsigned char *marker;
        unsigned char *ctxmarker;
        int state;
} crxdbg_lexer_data;

#define yyparse crxdbg_parse
#define yylex crxdbg_lex

void crxdbg_init_lexer (crxdbg_param_t *stack, char *input);

int crxdbg_lex (crxdbg_param_t* yylval);

#endif
