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
#include "crxdbg_cmd.h"
#include "crxdbg_set.h"
#include "crxdbg_utils.h"
#include "crxdbg_bp.h"
#include "crxdbg_prompt.h"

CREX_EXTERN_MODULE_GLOBALS(crxdbg)

#define CRXDBG_SET_COMMAND_D(f, h, a, m, l, s, flags) \
	CRXDBG_COMMAND_D_EXP(f, h, a, m, l, s, &crxdbg_prompt_commands[17], flags)

const crxdbg_command_t crxdbg_set_commands[] = {
	CRXDBG_SET_COMMAND_D(prompt,       "usage: set prompt [<string>]",            'p', set_prompt,       NULL, "|s", 0),
	CRXDBG_SET_COMMAND_D(pagination,   "usage: set pagination [<on|off>]",        'P', set_pagination,   NULL, "|b", CRXDBG_ASYNC_SAFE),
#ifndef _WIN32
	CRXDBG_SET_COMMAND_D(color,        "usage: set color  <element> <color>",     'c', set_color,        NULL, "ss", CRXDBG_ASYNC_SAFE),
	CRXDBG_SET_COMMAND_D(colors,       "usage: set colors [<on|off>]",            'C', set_colors,       NULL, "|b", CRXDBG_ASYNC_SAFE),
#endif
	CRXDBG_SET_COMMAND_D(break,        "usage: set break id [<on|off>]",          'b', set_break,        NULL, "l|b", CRXDBG_ASYNC_SAFE),
	CRXDBG_SET_COMMAND_D(breaks,       "usage: set breaks [<on|off>]",            'B', set_breaks,       NULL, "|b", CRXDBG_ASYNC_SAFE),
	CRXDBG_SET_COMMAND_D(quiet,        "usage: set quiet [<on|off>]",             'q', set_quiet,        NULL, "|b", CRXDBG_ASYNC_SAFE),
	CRXDBG_SET_COMMAND_D(stepping,     "usage: set stepping [<line|op>]",         's', set_stepping,     NULL, "|s", CRXDBG_ASYNC_SAFE),
	CRXDBG_SET_COMMAND_D(refcount,     "usage: set refcount [<on|off>]",          'r', set_refcount,     NULL, "|b", CRXDBG_ASYNC_SAFE),
	CRXDBG_SET_COMMAND_D(lines,        "usage: set lines [<number>]",             'l', set_lines,        NULL, "|l", CRXDBG_ASYNC_SAFE),
	CRXDBG_END_COMMAND
};

CRXDBG_SET(prompt) /* {{{ */
{
	if (!param || param->type == EMPTY_PARAM) {
		crxdbg_writeln("Current prompt: %s", crxdbg_get_prompt());
	} else {
		crxdbg_set_prompt(param->str);
	}

	return SUCCESS;
} /* }}} */

CRXDBG_SET(pagination) /* {{{ */
{
	if (!param || param->type == EMPTY_PARAM) {
		crxdbg_writeln("Pagination %s", CRXDBG_G(flags) & CRXDBG_HAS_PAGINATION ? "on" : "off");
	} else switch (param->type) {
		case NUMERIC_PARAM: {
			if (param->num) {
				CRXDBG_G(flags) |= CRXDBG_HAS_PAGINATION;
			} else {
				CRXDBG_G(flags) &= ~CRXDBG_HAS_PAGINATION;
			}
		} break;

		default:
			crxdbg_error("set pagination used incorrectly: set pagination <on|off>");
	}

	return SUCCESS;
} /* }}} */

CRXDBG_SET(lines) /* {{{ */
{
	if (!param || param->type == EMPTY_PARAM) {
		crxdbg_writeln("Lines "CREX_ULONG_FMT, CRXDBG_G(lines));
	} else switch (param->type) {
		case NUMERIC_PARAM: {
			CRXDBG_G(lines) = param->num;
		} break;

		default:
			crxdbg_error("set lines used incorrectly: set lines <number>");
	}

	return SUCCESS;
} /* }}} */

CRXDBG_SET(break) /* {{{ */
{
	switch (param->type) {
		case NUMERIC_PARAM: {
			if (param->next) {
				if (param->next->num) {
					crxdbg_enable_breakpoint(param->num);
				} else {
					crxdbg_disable_breakpoint(param->num);
				}
			} else {
				crxdbg_breakbase_t *brake = crxdbg_find_breakbase(param->num);
				if (brake) {
					crxdbg_writeln("Breakpoint #"CREX_LONG_FMT" %s", param->num, brake->disabled ? "off" : "on");
				} else {
					crxdbg_error("Failed to find breakpoint #"CREX_LONG_FMT, param->num);
				}
			}
		} break;

		default:
			crxdbg_error("set break used incorrectly: set break [id] <on|off>");
	}

	return SUCCESS;
} /* }}} */

CRXDBG_SET(breaks) /* {{{ */
{
	if (!param || param->type == EMPTY_PARAM) {
		crxdbg_writeln("Breakpoints %s",CRXDBG_G(flags) & CRXDBG_IS_BP_ENABLED ? "on" : "off");
	} else switch (param->type) {
		case NUMERIC_PARAM: {
			if (param->num) {
				crxdbg_enable_breakpoints();
			} else {
				crxdbg_disable_breakpoints();
			}
		} break;

		default:
			crxdbg_error("set breaks used incorrectly: set breaks <on|off>");
	}

	return SUCCESS;
} /* }}} */

#ifndef _WIN32
CRXDBG_SET(color) /* {{{ */
{
	const crxdbg_color_t *color = crxdbg_get_color(param->next->str, param->next->len);

	if (!color) {
		crxdbg_error("Failed to find the requested color (%s)", param->next->str);
		return SUCCESS;
	}

	switch (crxdbg_get_element(param->str, param->len)) {
		case CRXDBG_COLOR_PROMPT:
			crxdbg_notice("setting prompt color to %s (%s)", color->name, color->code);
			if (CRXDBG_G(prompt)[1]) {
				free(CRXDBG_G(prompt)[1]);
				CRXDBG_G(prompt)[1]=NULL;
			}
			crxdbg_set_color(CRXDBG_COLOR_PROMPT, color);
		break;

		case CRXDBG_COLOR_ERROR:
			crxdbg_notice("setting error color to %s (%s)", color->name, color->code);
			crxdbg_set_color(CRXDBG_COLOR_ERROR, color);
		break;

		case CRXDBG_COLOR_NOTICE:
			crxdbg_notice("setting notice color to %s (%s)", color->name, color->code);
			crxdbg_set_color(CRXDBG_COLOR_NOTICE, color);
		break;

		default:
			crxdbg_error("Failed to find the requested element (%s)", param->str);
	}

	return SUCCESS;
} /* }}} */

CRXDBG_SET(colors) /* {{{ */
{
	if (!param || param->type == EMPTY_PARAM) {
		crxdbg_writeln("Colors %s", CRXDBG_G(flags) & CRXDBG_IS_COLOURED ? "on" : "off");
	} else switch (param->type) {
		case NUMERIC_PARAM: {
			if (param->num) {
				CRXDBG_G(flags) |= CRXDBG_IS_COLOURED;
			} else {
				CRXDBG_G(flags) &= ~CRXDBG_IS_COLOURED;
			}
		} break;

		default:
			crxdbg_error("set colors used incorrectly: set colors <on|off>");
	}

	return SUCCESS;
} /* }}} */
#endif

CRXDBG_SET(quiet) /* {{{ */
{
	if (!param || param->type == EMPTY_PARAM) {
		crxdbg_writeln("Quietness %s", CRXDBG_G(flags) & CRXDBG_IS_QUIET ? "on" : "off");
	} else switch (param->type) {
		case NUMERIC_PARAM: {
			if (param->num) {
				CRXDBG_G(flags) |= CRXDBG_IS_QUIET;
			} else {
				CRXDBG_G(flags) &= ~CRXDBG_IS_QUIET;
			}
		} break;

		crxdbg_default_switch_case();
	}

	return SUCCESS;
} /* }}} */

CRXDBG_SET(stepping) /* {{{ */
{
	if (!param || param->type == EMPTY_PARAM) {
		crxdbg_writeln("Stepping %s", CRXDBG_G(flags) & CRXDBG_STEP_OPCODE ? "opcode" : "line");
	} else switch (param->type) {
		case STR_PARAM: {
			if (param->len == sizeof("opcode") - 1 && !memcmp(param->str, "opcode", sizeof("opcode"))) {
				CRXDBG_G(flags) |= CRXDBG_STEP_OPCODE;
			} else if (param->len == sizeof("line") - 1 && !memcmp(param->str, "line", sizeof("line"))) {
				CRXDBG_G(flags) &= ~CRXDBG_STEP_OPCODE;
			} else {
				crxdbg_error("usage set stepping [<opcode|line>]");
			}
		} break;

		crxdbg_default_switch_case();
	}

	return SUCCESS;
} /* }}} */

CRXDBG_SET(refcount) /* {{{ */
{
	if (!param || param->type == EMPTY_PARAM) {
		crxdbg_writeln("Showing refcounts %s", CRXDBG_G(flags) & CRXDBG_IS_QUIET ? "on" : "off");
	} else switch (param->type) {
		case NUMERIC_PARAM: {
			if (param->num) {
				CRXDBG_G(flags) |= CRXDBG_SHOW_REFCOUNTS;
			} else {
				CRXDBG_G(flags) &= ~CRXDBG_SHOW_REFCOUNTS;
			}
		} break;

		crxdbg_default_switch_case();
	}

	return SUCCESS;
} /* }}} */
