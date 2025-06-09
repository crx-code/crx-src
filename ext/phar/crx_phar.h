/*
  +----------------------------------------------------------------------+
  | crxa crx single-file executable CRX extension                        |
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
  | Authors: Gregory Beaver <cellog@crx.net>                             |
  |          Marcus Boerger <helly@crx.net>                              |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_CRXA_H
#define CRX_CRXA_H

#define CRX_CRXA_VERSION CRX_VERSION

#include "ext/standard/basic_functions.h"
extern crex_module_entry crxa_module_entry;
#define crxext_crxa_ptr &crxa_module_entry

#ifdef CRX_WIN32
#define CRX_CRXA_API __declspec(dllexport)
#else
#define CRX_CRXA_API CRXAPI
#endif

CRX_CRXA_API int crxa_resolve_alias(char *alias, size_t alias_len, char **filename, size_t *filename_len);

#endif /* CRX_CRXA_H */
