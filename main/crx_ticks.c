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
   | Author: Stig Bakken <ssb@crx.net>                                    |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "crx_ticks.h"

struct st_tick_function
{
	void (*func)(int, void *);
	void *arg;
};

int crx_startup_ticks(void)
{
	crex_llist_init(&PG(tick_functions), sizeof(struct st_tick_function), NULL, 1);
	return SUCCESS;
}

void crx_deactivate_ticks(void)
{
	crex_llist_clean(&PG(tick_functions));
}

void crx_shutdown_ticks(crx_core_globals *core_globals)
{
	crex_llist_destroy(&core_globals->tick_functions);
}

static int crx_compare_tick_functions(void *elem1, void *elem2)
{
	struct st_tick_function *e1 = (struct st_tick_function *)elem1;
	struct st_tick_function *e2 = (struct st_tick_function *)elem2;
	return e1->func == e2->func && e1->arg == e2->arg;
}

CRXAPI void crx_add_tick_function(void (*func)(int, void*), void * arg)
{
	struct st_tick_function tmp = {func, arg};
	crex_llist_add_element(&PG(tick_functions), (void *)&tmp);
}

CRXAPI void crx_remove_tick_function(void (*func)(int, void *), void * arg)
{
	struct st_tick_function tmp = {func, arg};
	crex_llist_del_element(&PG(tick_functions), (void *)&tmp, (int(*)(void*, void*))crx_compare_tick_functions);
}

static void crx_tick_iterator(void *d, void *arg)
{
	struct st_tick_function *data = (struct st_tick_function *)d;
	data->func(*((int *)arg), data->arg);
}

void crx_run_ticks(int count)
{
	crex_llist_apply_with_argument(&PG(tick_functions), (llist_apply_with_arg_func_t) crx_tick_iterator, &count);
}
