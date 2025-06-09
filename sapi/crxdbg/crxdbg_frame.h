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

#ifndef CRXDBG_FRAME_H
#define CRXDBG_FRAME_H

#include "TSRM.h"

crex_string *crxdbg_compile_stackframe(crex_execute_data *);
void crxdbg_restore_frame(void);
void crxdbg_switch_frame(int);
void crxdbg_dump_backtrace(size_t);
void crxdbg_open_generator_frame(crex_generator *);

#endif /* CRXDBG_FRAME_H */
