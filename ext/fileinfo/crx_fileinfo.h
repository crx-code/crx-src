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
  | Author: Ilia Alshanetsky <ilia@crx.net>                              |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_FILEINFO_H
#define CRX_FILEINFO_H

extern crex_module_entry fileinfo_module_entry;
#define crxext_fileinfo_ptr &fileinfo_module_entry

#define CRX_FILEINFO_VERSION CRX_VERSION

#ifdef CRX_WIN32
#define CRX_FILEINFO_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define CRX_FILEINFO_API __attribute__((visibility("default")))
#else
#define CRX_FILEINFO_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

CRX_MINFO_FUNCTION(fileinfo);

#endif	/* CRX_FILEINFO_H */
