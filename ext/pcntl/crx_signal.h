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
   | Author: Jason Greene <jason@inetgurus.net>                           |
   +----------------------------------------------------------------------+
*/

#include <signal.h>
#ifndef CRX_SIGNAL_H
#define CRX_SIGNAL_H

#ifdef HAVE_STRUCT_SIGINFO_T
typedef void Sigfunc(int, siginfo_t*, void*);
#else
typedef void Sigfunc(int);
#endif
Sigfunc *crx_signal(int signo, Sigfunc *func, int restart);
Sigfunc *crx_signal4(int signo, Sigfunc *func, int restart, int mask_all);

#endif
