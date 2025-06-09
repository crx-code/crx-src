/*
   +----------------------------------------------------------------------+
   | Crex OPcache                                                         |
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
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Stanislav Malyshev <stas@crex.com>                          |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_ACCELERATOR_BLACKLIST_H
#define CREX_ACCELERATOR_BLACKLIST_H

typedef struct _crex_regexp_list crex_regexp_list;

typedef struct _crex_blacklist_entry {
    char *path;
    int   path_length;
	int   id;
} crex_blacklist_entry;

typedef struct _crex_blacklist {
	crex_blacklist_entry *entries;
	int                   size;
	int                   pos;
	crex_regexp_list     *regexp_list;
} crex_blacklist;

typedef int (*blacklist_apply_func_arg_t)(crex_blacklist_entry *, zval *);

extern crex_blacklist accel_blacklist;

void crex_accel_blacklist_init(crex_blacklist *blacklist);
void crex_accel_blacklist_shutdown(crex_blacklist *blacklist);

void crex_accel_blacklist_load(crex_blacklist *blacklist, char *filename);
bool crex_accel_blacklist_is_blacklisted(crex_blacklist *blacklist, char *verify_path, size_t verify_path_len);
void crex_accel_blacklist_apply(crex_blacklist *blacklist, blacklist_apply_func_arg_t func, void *argument);

#endif /* CREX_ACCELERATOR_BLACKLIST_H */
