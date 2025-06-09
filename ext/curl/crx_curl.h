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
   | Author: Sterling Hughes <sterling@crx.net>                           |
   |         Wez Furlong <wez@thebrainroom.com>                           |
   +----------------------------------------------------------------------+
*/

#ifndef _CRX_CURL_H
#define _CRX_CURL_H

#include "crx.h"

#ifdef CRX_WIN32
# ifdef CRX_CURL_EXPORTS
#  define CRX_CURL_API __declspec(dllexport)
# else
#  define CRX_CURL_API __declspec(dllimport)
# endif
#elif defined(__GNUC__) && __GNUC__ >= 4
# define CRX_CURL_API __attribute__ ((visibility("default")))
#else
# define CRX_CURL_API
#endif

extern crex_module_entry curl_module_entry;
#define crxext_curl_ptr &curl_module_entry

CRX_CURL_API extern crex_class_entry *curl_ce;
CRX_CURL_API extern crex_class_entry *curl_share_ce;
CRX_CURL_API extern crex_class_entry *curl_multi_ce;
CRX_CURL_API extern crex_class_entry *curl_CURLFile_class;
CRX_CURL_API extern crex_class_entry *curl_CURLStringFile_class;

#endif  /* _CRX_CURL_H */
