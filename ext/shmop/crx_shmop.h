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
   | Authors: Slava Poliakov <hackie@prohost.org>                         |
   |          Ilia Alshanetsky <ilia@prohost.org>                         |
   +----------------------------------------------------------------------+
 */
#ifndef CRX_SHMOP_H
#define CRX_SHMOP_H

#ifdef HAVE_SHMOP

extern crex_module_entry shmop_module_entry;
#define crxext_shmop_ptr &shmop_module_entry

#include "crx_version.h"
#define CRX_SHMOP_VERSION CRX_VERSION

CRX_MINIT_FUNCTION(shmop);
CRX_MINFO_FUNCTION(shmop);

#ifdef CRX_WIN32
# include "win32/ipc.h"
#endif

#else

#define crxext_shmop_ptr NULL

#endif

#endif	/* CRX_SHMOP_H */
