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
   | Author: Marcus Boerger <helly@crx.net>                               |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_GETOPT_H
#define CRX_GETOPT_H

#include "crx.h"

/* Define structure for one recognized option (both single char and long name).
 * If short_open is '-' this is the last option. */
typedef struct _opt_struct {
	char opt_char;
	int  need_param;
	char * opt_name;
} opt_struct;

BEGIN_EXTERN_C()
/* holds the index of the latest fetched element from the opts array */
extern CRXAPI int crx_optidx;
CRXAPI int crx_getopt(int argc, char* const *argv, const opt_struct opts[], char **optarg, int *optind, int show_err, int arg_start);
END_EXTERN_C()

/* crx_getopt will return this value if there is an error in arguments */
#define CRX_GETOPT_INVALID_ARG (-2)

#endif
