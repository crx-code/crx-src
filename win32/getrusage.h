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
   | Authors: Kalle Sommer Nielsen <kalle@crx.net>                        |
   +----------------------------------------------------------------------+
 */

#ifndef HAVE_GETRUSAGE_H
# define HAVE_GETRUSAGE_H

/*
 * Note
 *
 * RUSAGE_CHILDREN is not implemented, and the RUSAGE_THREAD will
 * therefore instead be used instead to emulate the behavior.
 */

# define RUSAGE_SELF		0
# define RUSAGE_CHILDREN	1

# define RUSAGE_THREAD		RUSAGE_CHILDREN

/*
 * Implementation support
 *
 *  RUSAGE_SELF
 *		ru_utime
 *		ru_stime
 *		ru_majflt
 *		ru_maxrss
 *
 *  RUSAGE_THREAD
 *		ru_utime
 *		ru_stime
 *
 * Not implemented:
 *		ru_ixrss		(unused)
 *		ru_idrss		(unused)
 *		ru_isrss		(unused)
 *		ru_minflt
 *		ru_nswap		(unused)
 *		ru_inblock
 *		ru_oublock
 *		ru_msgsnd		(unused)
 *		ru_msgrcv		(unused)
 *		ru_nsignals		(unused)
 *		ru_nvcsw
 *		ru_nivcsw
 */

struct rusage
{
	/* User time used */
	struct timeval ru_utime;

	/* System time used */
	struct timeval ru_stime;

	/* Integral max resident set size */
	crex_long ru_maxrss;

	/* Page faults */
	crex_long ru_majflt;
#if 0
	/* Integral shared text memory size */
	crex_long ru_ixrss;

	/* Integral unshared data size */
	crex_long ru_idrss;

	/* Integral unshared stack size */
	crex_long ru_isrss;

	/* Page reclaims */
	crex_long ru_minflt;

	/* Swaps */
	crex_long ru_nswap;

	/* Block input operations */
	crex_long ru_inblock;

	/* Block output operations */
	crex_long ru_oublock;

	/* Messages sent */
	crex_long ru_msgsnd;

	/* Messages received */
	crex_long ru_msgrcv;

	/* Signals received */
	crex_long ru_nsignals;

	/* Voluntary context switches */
	crex_long ru_nvcsw;

	/* Involuntary context switches */
	crex_long ru_nivcsw;
#endif
};

CRXAPI int getrusage(int who, struct rusage *usage);

#endif
