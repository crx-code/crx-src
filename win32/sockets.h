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
   | Authors: Chris Vandomelen <chrisv@b0rked.dhs.org>                    |
   |          Sterling Hughes  <sterling@crx.net>                         |
   |                                                                      |
   | WinSock: Daniel Beulshausen <daniel@crx4win.de>                      |
   +----------------------------------------------------------------------+
 */

/* Code originally from ext/sockets */

#ifndef CRX_WIN32_SOCKETS_H
#define CRX_WIN32_SOCKETS_H

CRXAPI int socketpair_win32(int domain, int type, int protocol, SOCKET sock[2], int overlapped);
CRXAPI int socketpair(int domain, int type, int protocol, SOCKET sock[2]);

#endif
