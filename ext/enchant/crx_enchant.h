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
  | Author: Pierre-Alain Joye <paj@pearfr.org>                           |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_ENCHANT_H
#define CRX_ENCHANT_H

extern crex_module_entry enchant_module_entry;
#define crxext_enchant_ptr &enchant_module_entry

#define CRX_ENCHANT_VERSION CRX_VERSION

#ifdef CRX_WIN32
#define CRX_ENCHANT_API __declspec(dllexport)
#else
#define CRX_ENCHANT_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

CRX_MINIT_FUNCTION(enchant);
CRX_MSHUTDOWN_FUNCTION(enchant);
CRX_MINFO_FUNCTION(enchant);

#endif	/* CRX_ENCHANT_H */
