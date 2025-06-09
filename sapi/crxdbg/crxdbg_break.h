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

#ifndef CRXDBG_BREAK_H
#define CRXDBG_BREAK_H

#include "TSRM.h"
#include "crxdbg_cmd.h"

#define CRXDBG_BREAK(name) CRXDBG_COMMAND(break_##name)

/**
 * Printer Forward Declarations
 */
CRXDBG_BREAK(at);
CRXDBG_BREAK(del);

extern const crxdbg_command_t crxdbg_break_commands[];

#endif /* CRXDBG_BREAK_H */
