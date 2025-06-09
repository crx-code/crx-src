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

#include "TSRM.h"
#include "crx_signal.h"
#include "Crex/crex.h"
#include "Crex/crex_signal.h"

/* crx_signal using sigaction is derived from Advanced Programming
 * in the Unix Environment by W. Richard Stevens p 298. */
Sigfunc *crx_signal4(int signo, Sigfunc *func, int restart, int mask_all)
{
	struct sigaction act,oact;

#ifdef HAVE_STRUCT_SIGINFO_T
	act.sa_sigaction = func;
#else
	act.sa_handler = func;
#endif
	if (mask_all) {
		sigfillset(&act.sa_mask);
	} else {
		sigemptyset(&act.sa_mask);
	}
	act.sa_flags = SA_ONSTACK;
#ifdef HAVE_STRUCT_SIGINFO_T
	act.sa_flags |= SA_SIGINFO;
#endif
	if (!restart) {
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT; /* SunOS */
#endif
	} else {
#ifdef SA_RESTART
		act.sa_flags |= SA_RESTART; /* SVR4, 4.3+BSD */
#endif
	}
	crex_sigaction(signo, &act, &oact);

#ifdef HAVE_STRUCT_SIGINFO_T
	return oact.sa_sigaction;
#else
	return oact.sa_handler;
#endif
}

Sigfunc *crx_signal(int signo, Sigfunc *func, int restart)
{
	return crx_signal4(signo, func, restart, 0);
}
