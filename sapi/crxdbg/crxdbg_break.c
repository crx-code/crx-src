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
#include "crxdbg_print.h"
#include "crxdbg_utils.h"
#include "crxdbg_break.h"
#include "crxdbg_bp.h"
#include "crxdbg_prompt.h"

CREX_EXTERN_MODULE_GLOBALS(crxdbg)

#define CRXDBG_BREAK_COMMAND_D(f, h, a, m, l, s, flags) \
	CRXDBG_COMMAND_D_EXP(f, h, a, m, l, s, &crxdbg_prompt_commands[9], flags)

/**
 * Commands
 */
const crxdbg_command_t crxdbg_break_commands[] = {
	CRXDBG_BREAK_COMMAND_D(at,         "specify breakpoint by location and condition",           '@', break_at,      NULL, "*c", 0),
	CRXDBG_BREAK_COMMAND_D(del,        "delete breakpoint by identifier number",                 '~', break_del,     NULL, "n",  0),
	CRXDBG_END_COMMAND
};

CRXDBG_BREAK(at) /* {{{ */
{
	crxdbg_set_breakpoint_at(param);

	return SUCCESS;
} /* }}} */

CRXDBG_BREAK(del) /* {{{ */
{
	crxdbg_delete_breakpoint(param->num);

	return SUCCESS;
} /* }}} */
