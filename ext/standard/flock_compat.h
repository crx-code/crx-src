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
   | Author: Sascha Schumann <sascha@schumann.cx>                         |
   +----------------------------------------------------------------------+
*/

#ifndef FLOCK_COMPAT_H
#define FLOCK_COMPAT_H

#ifdef HAVE_STRUCT_FLOCK
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#endif

#ifdef CRX_WIN32
#include <io.h>
#include "config.w32.h"
#endif

/* crx_flock internally uses fcntl whether or not flock is available
 * This way our crx_flock even works on NFS files.
 * More info: /usr/src/linux/Documentation
 */
CRXAPI int crx_flock(int fd, int operation);

#ifndef HAVE_FLOCK
#	define LOCK_SH 1
#	define LOCK_EX 2
#	define LOCK_NB 4
#	define LOCK_UN 8
CRXAPI int flock(int fd, int operation);
#endif

/* Userland LOCK_* constants */
#define CRX_LOCK_SH 1
#define CRX_LOCK_EX 2
#define CRX_LOCK_UN 3
#define CRX_LOCK_NB 4

#ifdef CRX_WIN32
# ifdef EWOULDBLOCK
#  undef EWOULDBLOCK
# endif
# define EWOULDBLOCK WSAEWOULDBLOCK
# define fsync _commit
# define ftruncate(a, b) chsize(a, b)
#endif /* defined(CRX_WIN32) */

#ifndef HAVE_INET_ATON
# ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
# endif
# ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
# endif
# ifndef CRX_WIN32
extern int inet_aton(const char *, struct in_addr *);
# endif
#endif

#endif	/* FLOCK_COMPAT_H */
