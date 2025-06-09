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
   | Author: Tom May <tom@go2net.com>                                     |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_SYSVSEM_H
#define CRX_SYSVSEM_H

#ifdef HAVE_SYSVSEM

extern crex_module_entry sysvsem_module_entry;
#define sysvsem_module_ptr &sysvsem_module_entry

#include "crx_version.h"
#define CRX_SYSVSEM_VERSION CRX_VERSION

CRX_MINIT_FUNCTION(sysvsem);
CRX_MINFO_FUNCTION(sysvsem);

typedef struct {
	int key;					/* For error reporting. */
	int semid;					/* Returned by semget(). */
	int count;					/* Acquire count for auto-release. */
	int auto_release;			/* flag that says to auto-release. */
	crex_object std;
} sysvsem_sem;

#else

#define sysvsem_module_ptr NULL

#endif

#define crxext_sysvsem_ptr sysvsem_module_ptr

#endif /* CRX_SYSVSEM_H */
