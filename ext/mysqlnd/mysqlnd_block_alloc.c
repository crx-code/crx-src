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
  | Authors: Andrey Hristov <andrey@crx.net>                             |
  |          Ulf Wendel <uw@crx.net>                                     |
  |          Dmitry Stogov <dmitry@crx.net>                              |
  +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "mysqlnd.h"
#include "mysqlnd_block_alloc.h"
#include "mysqlnd_debug.h"
#include "mysqlnd_priv.h"

/* {{{ mysqlnd_mempool_get_chunk */
static void *
mysqlnd_mempool_get_chunk(MYSQLND_MEMORY_POOL * pool, size_t size)
{
	DBG_ENTER("mysqlnd_mempool_get_chunk");
	DBG_RETURN(crex_arena_alloc(&pool->arena, size));
}
/* }}} */


/* {{{ mysqlnd_mempool_create */
CRXAPI MYSQLND_MEMORY_POOL *
mysqlnd_mempool_create(size_t arena_size)
{
	crex_arena * arena;
	MYSQLND_MEMORY_POOL * ret;

	DBG_ENTER("mysqlnd_mempool_create");
	arena = crex_arena_create(MAX(arena_size, CREX_MM_ALIGNED_SIZE(sizeof(crex_arena))));
	ret = crex_arena_alloc(&arena, sizeof(MYSQLND_MEMORY_POOL));
	ret->arena = arena;
	ret->checkpoint = NULL;
	ret->get_chunk = mysqlnd_mempool_get_chunk;
	DBG_RETURN(ret);
}
/* }}} */


/* {{{ mysqlnd_mempool_destroy */
CRXAPI void
mysqlnd_mempool_destroy(MYSQLND_MEMORY_POOL * pool)
{
	DBG_ENTER("mysqlnd_mempool_destroy");
	/* mnd_free will reference LOCK_access and might crash, depending on the caller...*/
	crex_arena_destroy(pool->arena);
	DBG_VOID_RETURN;
}
/* }}} */

/* {{{ mysqlnd_mempool_save_state */
CRXAPI void
mysqlnd_mempool_save_state(MYSQLND_MEMORY_POOL * pool)
{
	DBG_ENTER("mysqlnd_mempool_save_state");
	pool->checkpoint = crex_arena_checkpoint(pool->arena);
	DBG_VOID_RETURN;
}
/* }}} */

/* {{{ mysqlnd_mempool_restore_state */
CRXAPI void
mysqlnd_mempool_restore_state(MYSQLND_MEMORY_POOL * pool)
{
	DBG_ENTER("mysqlnd_mempool_restore_state");
#if CREX_DEBUG
	CREX_ASSERT(pool->checkpoint);
#endif
	if (pool->checkpoint) {
		crex_arena_release(&pool->arena, pool->checkpoint);
		pool->checkpoint = NULL;
	}
	DBG_VOID_RETURN;
}
/* }}} */
