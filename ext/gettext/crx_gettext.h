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
   | Author: Alex Plotnick <alex@wgate.com>                               |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_GETTEXT_H
#define CRX_GETTEXT_H

#ifdef HAVE_LIBINTL

extern crex_module_entry crx_gettext_module_entry;
#define gettext_module_ptr &crx_gettext_module_entry

#include "crx_version.h"
#define CRX_GETTEXT_VERSION CRX_VERSION

CRX_MINFO_FUNCTION(crx_gettext);

#else
#define gettext_module_ptr NULL
#endif /* HAVE_LIBINTL */

#define crxext_gettext_ptr gettext_module_ptr

#endif /* CRX_GETTEXT_H */
