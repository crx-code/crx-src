/*
   +----------------------------------------------------------------------+
   | Crex JIT                                                             |
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
   | Authors: Dmitry Stogov <dmitry@crx.net>                              |
   |          Xinchen Hui <laruence@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef HAVE_CREX_JIT_GDB_H
#define HAVE_CREX_JIT_GDB_H

#include "crex_compile.h"

#define HAVE_GDB

int crex_jit_gdb_register(const char    *name,
                          const crex_op_array *op_array,
                          const void    *start,
                          size_t         size,
                          uint32_t       sp_offset,
                          uint32_t       sp_adjustment);

int crex_jit_gdb_unregister(void);
void crex_jit_gdb_init(void);

#endif /* HAVE_CREX_JIT_GDB_H */
