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
   | Author: Thies C. Arntzen <thies@thieso.net>                          |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_READLINE_H
#define CRX_READLINE_H

#if HAVE_LIBEDIT
#define READLINE_LIB "libedit"
#else
#define READLINE_LIB "readline"
#endif

#if HAVE_LIBREADLINE || HAVE_LIBEDIT

extern crex_module_entry readline_module_entry;
#define crxext_readline_ptr &readline_module_entry

#include "crx_version.h"
#define CRX_READLINE_VERSION CRX_VERSION

#else

#define crxext_readline_ptr NULL

#endif /* HAVE_LIBREADLINE */

#endif /* CRX_READLINE_H */
