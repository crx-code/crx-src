/*
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Max Kellermann <max.kellermann@ionos.com>                   |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_MMAP_H
#define CREX_MMAP_H

#include "crex_portability.h"

#ifdef HAVE_PRCTL
# include <sys/prctl.h>

/* fallback definitions if our libc is older than the kernel */
# ifndef PR_SET_VMA
#  define PR_SET_VMA 0x53564d41
# endif
# ifndef PR_SET_VMA_ANON_NAME
#  define PR_SET_VMA_ANON_NAME 0
# endif
#endif // HAVE_PRCTL

/**
 * Set a name for the specified memory area.
 *
 * This feature requires Linux 5.17.
 */
static crex_always_inline void crex_mmap_set_name(const void *start, size_t len, const char *name)
{
#ifdef HAVE_PRCTL
	prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, (unsigned long)start, len, (unsigned long)name);
#endif
}

#endif /* CREX_MMAP_H */
