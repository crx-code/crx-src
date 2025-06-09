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

#ifndef CRXDBG_PRINT_H
#define CRXDBG_PRINT_H

#include "crxdbg_cmd.h"

#define CRXDBG_PRINT(name) CRXDBG_COMMAND(print_##name)

/**
 * Printer Forward Declarations
 */
CRXDBG_PRINT(exec);
CRXDBG_PRINT(opline);
CRXDBG_PRINT(class);
CRXDBG_PRINT(method);
CRXDBG_PRINT(func);
CRXDBG_PRINT(stack);

extern const crxdbg_command_t crxdbg_print_commands[];

void crxdbg_print_opcodes(const char *function);

void crxdbg_print_opline(crex_execute_data *execute_data, bool ignore_flags);
#endif /* CRXDBG_PRINT_H */
