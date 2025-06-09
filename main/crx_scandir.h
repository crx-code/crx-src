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
   | Authors: Shane Caraveo <shane@caraveo.com>                           |
   |          Ilia Alshanetsky  <ilia@prohost.org>                        |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_SCANDIR_H
#define CRX_SCANDIR_H

#include <sys/types.h>

#ifdef CRX_WIN32
#include "config.w32.h"
#include "win32/readdir.h"
#else
#include <crx_config.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef HAVE_SCANDIR
#define crx_scandir		scandir
#else
CRXAPI int crx_scandir(const char *dirname, struct dirent **namelist[], int (*selector) (const struct dirent *entry), int (*compare) (const struct dirent **a, const struct dirent **b));
#endif

#ifdef HAVE_ALPHASORT
#define crx_alphasort	alphasort
#else
CRXAPI int crx_alphasort(const struct dirent **a, const struct dirent **b);
#endif

#endif /* CRX_SCANDIR_H */
