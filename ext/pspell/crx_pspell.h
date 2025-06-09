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
   | Author: Vlad Krupin <crxdevel@echospace.com>                         |
   +----------------------------------------------------------------------+
*/

#ifndef _PSPELL_H
#define _PSPELL_H
#ifdef HAVE_PSPELL
extern crex_module_entry pspell_module_entry;
#define pspell_module_ptr &pspell_module_entry

#include "crx_version.h"
#define CRX_PSPELL_VERSION CRX_VERSION

#else
#define pspell_module_ptr NULL
#endif

#define crxext_pspell_ptr pspell_module_ptr

#endif /* _PSPELL_H */
