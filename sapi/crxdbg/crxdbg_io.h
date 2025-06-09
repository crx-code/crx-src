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
   | Authors: Anatol Belski <ab@crx.net>                                  |
   +----------------------------------------------------------------------+
*/

#ifndef CRXDBG_IO_H
#define CRXDBG_IO_H

#include "crxdbg.h"

CRXDBG_API int crxdbg_consume_stdin_line(char *buf);

CRXDBG_API int crxdbg_mixed_read(int fd, char *ptr, int len, int tmo);
CRXDBG_API int crxdbg_mixed_write(int fd, const char *ptr, int len);

#endif /* CRXDBG_IO_H */
