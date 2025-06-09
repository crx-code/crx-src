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

#ifndef CRXDBG_LIST_H
#define CRXDBG_LIST_H

#include "TSRM.h"
#include "crxdbg_cmd.h"

#define CRXDBG_LIST(name)         CRXDBG_COMMAND(list_##name)
#define CRXDBG_LIST_HANDLER(name) CRXDBG_COMMAND_HANDLER(list_##name)

CRXDBG_LIST(lines);
CRXDBG_LIST(class);
CRXDBG_LIST(method);
CRXDBG_LIST(func);

void crxdbg_list_function_byname(const char *, size_t);
void crxdbg_list_function(const crex_function *);
void crxdbg_list_file(crex_string *, uint32_t, int, uint32_t);

extern const crxdbg_command_t crxdbg_list_commands[];

void crxdbg_init_list(void);
void crxdbg_list_update(void);

typedef struct {
	char *buf;
	size_t len;
	crex_op_array op_array;
	uint32_t lines;
	uint32_t line[1];
} crxdbg_file_source;

#endif /* CRXDBG_LIST_H */
