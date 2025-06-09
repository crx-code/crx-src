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

#ifndef CRXDBG_HELP_H
#define CRXDBG_HELP_H

#include "TSRM.h"
#include "crxdbg.h"
#include "crxdbg_cmd.h"

#define CRXDBG_HELP(name) CRXDBG_COMMAND(help_##name)

/**
 * Helper Forward Declarations
 */
CRXDBG_HELP(aliases);

extern const crxdbg_command_t crxdbg_help_commands[];

#define crxdbg_help_header() \
	crxdbg_notice("version", "version=\"%s\"", "Welcome to crxdbg, the interactive CRX debugger, v%s", CRXDBG_VERSION);
#define crxdbg_help_footer() \
	crxdbg_notice("issues", "url=\"%s\"", "Please report bugs to <%s>", CRXDBG_ISSUES);

typedef struct _crxdbg_help_text_t {
	char *key;
	char *text;
} crxdbg_help_text_t;

extern const crxdbg_help_text_t crxdbg_help_text[];

extern void crxdbg_do_help_cmd(const char *type);
#endif /* CRXDBG_HELP_H */
