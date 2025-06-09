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

#ifndef CRXDBG_PROMPT_H
#define CRXDBG_PROMPT_H

/* {{{ */
void crxdbg_string_init(char *buffer);
void crxdbg_init(char *init_file, size_t init_file_len, bool use_default);
void crxdbg_try_file_init(char *init_file, size_t init_file_len, bool free_init);
int crxdbg_interactive(bool allow_async_unsafe, char *input);
int crxdbg_compile(void);
int crxdbg_compile_stdin(crex_string *code);
void crxdbg_force_interruption(void);
/* }}} */

/* {{{ crxdbg command handlers */
CRXDBG_COMMAND(exec);
CRXDBG_COMMAND(stdin);
CRXDBG_COMMAND(step);
CRXDBG_COMMAND(continue);
CRXDBG_COMMAND(run);
CRXDBG_COMMAND(ev);
CRXDBG_COMMAND(until);
CRXDBG_COMMAND(finish);
CRXDBG_COMMAND(leave);
CRXDBG_COMMAND(frame);
CRXDBG_COMMAND(print);
CRXDBG_COMMAND(break);
CRXDBG_COMMAND(back);
CRXDBG_COMMAND(list);
CRXDBG_COMMAND(info);
CRXDBG_COMMAND(clean);
CRXDBG_COMMAND(clear);
CRXDBG_COMMAND(help);
CRXDBG_COMMAND(sh);
CRXDBG_COMMAND(dl);
CRXDBG_COMMAND(generator);
CRXDBG_COMMAND(set);
CRXDBG_COMMAND(source);
CRXDBG_COMMAND(export);
CRXDBG_COMMAND(register);
CRXDBG_COMMAND(quit);
CRXDBG_COMMAND(watch);
CRXDBG_COMMAND(next);
CRXDBG_COMMAND(eol); /* }}} */

/* {{{ prompt commands */
extern const crxdbg_command_t crxdbg_prompt_commands[]; /* }}} */

void crxdbg_execute_ex(crex_execute_data *execute_data);

#endif /* CRXDBG_PROMPT_H */
