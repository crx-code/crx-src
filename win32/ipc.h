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
   | Author: Anatol Belski <ab@crx.net>                                   |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_WIN32_IPC_H
#define CRX_WIN32_IPC_H 1

#ifdef CRX_EXPORTS
# define CRX_WIN32_IPC_API __declspec(dllexport)
#else
# define CRX_WIN32_IPC_API __declspec(dllimport)
#endif

typedef int key_t;

CRX_WIN32_IPC_API key_t ftok(const char *path, int id);


#endif /* CRX_WIN32_IPC_H */
