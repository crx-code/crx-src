/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Dmitry Stogov <dmitry@crex.com>                             |
   |          Xinchen Hui <laruence@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_GDB
#define CREX_GDB

CREX_API bool crex_gdb_register_code(const void *object, size_t size);
CREX_API void crex_gdb_unregister_all(void);
CREX_API bool crex_gdb_present(void);

#endif
