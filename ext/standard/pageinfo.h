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
   | Author: Jim Winstead <jimw@crx.net>                                  |
   +----------------------------------------------------------------------+
*/

#ifndef PAGEINFO_H
#define PAGEINFO_H

CRXAPI void crx_statpage(void);
CRXAPI time_t crx_getlastmod(void);
extern crex_long crx_getuid(void);
extern crex_long crx_getgid(void);

#endif
