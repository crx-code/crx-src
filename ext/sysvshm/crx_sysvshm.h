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
   | Author: Christian Cartus <cartus@atrior.de>                          |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_SYSVSHM_H
#define CRX_SYSVSHM_H

#ifdef HAVE_SYSVSHM

extern crex_module_entry sysvshm_module_entry;
#define sysvshm_module_ptr &sysvshm_module_entry

#include "crx_version.h"
#define CRX_SYSVSHM_VERSION CRX_VERSION

#include <sys/types.h>

#ifdef CRX_WIN32
# include <TSRM/tsrm_win32.h>
# include "win32/ipc.h"
#else
# include <sys/ipc.h>
# include <sys/shm.h>
#endif

typedef struct {
	crex_long init_mem;
} sysvshm_module;

typedef struct {
	crex_long key;
	crex_long length;
	crex_long next;
	char mem;
} sysvshm_chunk;

typedef struct {
	char magic[8];
	crex_long start;
	crex_long end;
	crex_long free;
	crex_long total;
} sysvshm_chunk_head;

typedef struct {
	key_t key;               /* key set by user */
	crex_long id;                 /* returned by shmget */
	sysvshm_chunk_head *ptr; /* memory address of shared memory */
	crex_object std;
} sysvshm_shm;

CRX_MINIT_FUNCTION(sysvshm);
CRX_MINFO_FUNCTION(sysvshm);

extern sysvshm_module crx_sysvshm;

#else

#define sysvshm_module_ptr NULL

#endif

#define crxext_sysvshm_ptr sysvshm_module_ptr

#endif /* CRX_SYSVSHM_H */
