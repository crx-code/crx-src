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
   | Author: Marcus Boerger <helly@crx.net>                               |
   |         Johannes Schlueter <johannes@crx.net>                        |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "crex_smart_str_public.h"

CREX_BEGIN_MODULE_GLOBALS(cli_readline)
	char *pager;
	char *prompt;
	smart_str *prompt_str;
CREX_END_MODULE_GLOBALS(cli_readline)

#ifdef ZTS
# define CLIR_G(v) TSRMG(cli_readline_globals_id, crex_cli_readline_globals *, v)
#else
# define CLIR_G(v) (cli_readline_globals.v)
#endif

extern CRX_MINIT_FUNCTION(cli_readline);
extern CRX_MSHUTDOWN_FUNCTION(cli_readline);
extern CRX_MINFO_FUNCTION(cli_readline);

char **crx_readline_completion_cb(const char *text, int start, int end);

CREX_EXTERN_MODULE_GLOBALS(cli_readline)
