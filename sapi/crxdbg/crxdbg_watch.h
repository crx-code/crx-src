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
   | Authors: Felipe Pena <felipe@crx.net>                                |
   | Authors: Joe Watkins <joe.watkins@live.co.uk>                        |
   | Authors: Bob Weinand <bwoebi@crx.net>                                |
   +----------------------------------------------------------------------+
*/

#ifndef CRXDBG_WATCH_H
#define CRXDBG_WATCH_H

#include "crxdbg_cmd.h"

#ifdef _WIN32
#	include "crxdbg_win.h"
#endif

#define CRXDBG_WATCH(name) CRXDBG_COMMAND(watch_##name)

/**
 * Printer Forward Declarations
 */
CRXDBG_WATCH(array);
CRXDBG_WATCH(delete);
CRXDBG_WATCH(recursive);

extern const crxdbg_command_t crxdbg_watch_commands[];

/* Watchpoint functions/typedefs */

/* References are managed through their parent zval *, being a simple WATCH_ON_ZVAL and eventually WATCH_ON_REFCOUNTED */
typedef enum {
	WATCH_ON_ZVAL,
	WATCH_ON_HASHTABLE,
	WATCH_ON_REFCOUNTED,
	WATCH_ON_STR,
	WATCH_ON_HASHDATA,
	WATCH_ON_BUCKET,
} crxdbg_watchtype;


#define CRXDBG_WATCH_SIMPLE     0x01
#define CRXDBG_WATCH_RECURSIVE  0x02
#define CRXDBG_WATCH_ARRAY      0x04
#define CRXDBG_WATCH_OBJECT     0x08
#define CRXDBG_WATCH_NORMAL     (CRXDBG_WATCH_SIMPLE | CRXDBG_WATCH_RECURSIVE)
#define CRXDBG_WATCH_IMPLICIT   0x10
#define CRXDBG_WATCH_RECURSIVE_ROOT 0x20

typedef struct _crxdbg_watch_collision crxdbg_watch_collision;

typedef struct _crxdbg_watchpoint_t {
	union {
		zval *zv;
		crex_refcounted *ref;
		Bucket *bucket;
		void *ptr;
	} addr;
	size_t size;
	crxdbg_watchtype type;
	crex_refcounted *ref; /* key to fetch the collision on parents */
	HashTable elements;
	crxdbg_watch_collision *coll; /* only present on *children* */
	union {
		zval zv;
		Bucket bucket;
		crex_refcounted ref;
		HashTable ht;
		crex_string *str;
	} backup;
} crxdbg_watchpoint_t;

struct _crxdbg_watch_collision {
	crxdbg_watchpoint_t ref;
	crxdbg_watchpoint_t reference;
	HashTable parents;
};

typedef struct _crxdbg_watch_element {
	uint32_t id;
	crxdbg_watchpoint_t *watch;
	char flags;
	struct _crxdbg_watch_element *child; /* always set for implicit watches */
	struct _crxdbg_watch_element *parent;
	HashTable child_container; /* children of this watch element for recursive array elements */
	HashTable *parent_container; /* container of the value */
	crex_string *name_in_parent;
	crex_string *str;
	union {
		zval zv;
		crex_refcounted ref;
		HashTable ht;
	} backup; /* backup for when watchpoint gets dissociated */
} crxdbg_watch_element;

typedef struct {
	/* to watch rehashes (yes, this is not *perfect*, but good enough for everything in CRX...) */
	crxdbg_watchpoint_t hash_watch; /* must be first element */
	Bucket *last;
	crex_string *last_str;
	crex_ulong last_idx;

	HashTable *ht;
	size_t data_size;
	HashTable watches; /* contains crxdbg_watch_element */
} crxdbg_watch_ht_info;

void crxdbg_setup_watchpoints(void);
void crxdbg_destroy_watchpoints(void);
void crxdbg_purge_watchpoint_tree(void);

#ifndef _WIN32
int crxdbg_watchpoint_segfault_handler(siginfo_t *info, void *context);
#else
int crxdbg_watchpoint_segfault_handler(void *addr);
#endif

void crxdbg_create_addr_watchpoint(void *addr, size_t size, crxdbg_watchpoint_t *watch);
void crxdbg_create_zval_watchpoint(zval *zv, crxdbg_watchpoint_t *watch);

int crxdbg_delete_var_watchpoint(char *input, size_t len);
int crxdbg_create_var_watchpoint(char *input, size_t len);

int crxdbg_print_changed_zvals(void);

void crxdbg_list_watchpoints(void);

void crxdbg_watch_efree(void *ptr);


static long crxdbg_pagesize;

static crex_always_inline void *crxdbg_get_page_boundary(void *addr) {
	return (void *) ((size_t) addr & ~(crxdbg_pagesize - 1));
}

static crex_always_inline size_t crxdbg_get_total_page_size(void *addr, size_t size) {
	return (size_t) crxdbg_get_page_boundary((void *) ((size_t) addr + size - 1)) - (size_t) crxdbg_get_page_boundary(addr) + crxdbg_pagesize;
}

#endif
