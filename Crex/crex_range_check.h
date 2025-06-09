/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Anatol Belski <ab@crx.net>                                  |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_RANGE_CHECK_H
#define CREX_RANGE_CHECK_H

#include "crex_long.h"

/* Flag macros for basic range recognition. Notable is that
   always sizeof(signed) == sizeof(unsigned), so no need to
   overcomplicate things. */
#if SIZEOF_INT < SIZEOF_CREX_LONG
# define CREX_LONG_CAN_OVFL_INT 1
# define CREX_LONG_CAN_OVFL_UINT 1
#endif

#if SIZEOF_INT < SIZEOF_SIZE_T
/* size_t can always overflow signed int on the same platform.
   Furthermore, by the current design, size_t can always
   overflow crex_long. */
# define CREX_SIZE_T_CAN_OVFL_UINT 1
#endif


/* crex_long vs. (unsigned) int checks. */
#ifdef CREX_LONG_CAN_OVFL_INT
# define CREX_LONG_INT_OVFL(zlong) UNEXPECTED((zlong) > (crex_long)INT_MAX)
# define CREX_LONG_INT_UDFL(zlong) UNEXPECTED((zlong) < (crex_long)INT_MIN)
# define CREX_LONG_EXCEEDS_INT(zlong) UNEXPECTED(CREX_LONG_INT_OVFL(zlong) || CREX_LONG_INT_UDFL(zlong))
# define CREX_LONG_UINT_OVFL(zlong) UNEXPECTED((zlong) < 0 || (zlong) > (crex_long)UINT_MAX)
#else
# define CREX_LONG_INT_OVFL(zl) (0)
# define CREX_LONG_INT_UDFL(zl) (0)
# define CREX_LONG_EXCEEDS_INT(zlong) (0)
# define CREX_LONG_UINT_OVFL(zl) (0)
#endif

/* size_t vs (unsigned) int checks. */
#define CREX_SIZE_T_INT_OVFL(size) 	UNEXPECTED((size) > (size_t)INT_MAX)
#ifdef CREX_SIZE_T_CAN_OVFL_UINT
# define CREX_SIZE_T_UINT_OVFL(size) UNEXPECTED((size) > (size_t)UINT_MAX)
#else
# define CREX_SIZE_T_UINT_OVFL(size) (0)
#endif

/* Comparison crex_long vs size_t */
#define CREX_SIZE_T_GT_CREX_LONG(size, zlong) ((zlong) < 0 || (size) > (size_t)(zlong))
#define CREX_SIZE_T_GTE_CREX_LONG(size, zlong) ((zlong) < 0 || (size) >= (size_t)(zlong))
#define CREX_SIZE_T_LT_CREX_LONG(size, zlong) ((zlong) >= 0 && (size) < (size_t)(zlong))
#define CREX_SIZE_T_LTE_CREX_LONG(size, zlong) ((zlong) >= 0 && (size) <= (size_t)(zlong))

#endif /* CREX_RANGE_CHECK_H */
