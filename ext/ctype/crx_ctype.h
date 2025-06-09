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
   | Author: Hartmut Holzgraefe <hholzgra@crx.net>                        |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_CTYPE_H
#define CRX_CTYPE_H

#include "crx_version.h"
#define CRX_CTYPE_VERSION CRX_VERSION

#ifdef HAVE_CTYPE

extern crex_module_entry ctype_module_entry;
#define crxext_ctype_ptr &ctype_module_entry

#else

#define crxext_ctype_ptr NULL

#endif

#endif	/* CRX_CTYPE_H */
