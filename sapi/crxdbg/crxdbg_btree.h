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

#ifndef CRXDBG_BTREE_H
#define CRXDBG_BTREE_H

#include "crex.h"

typedef struct {
	crex_ulong idx;
	void *ptr;
} crxdbg_btree_result;

typedef union _crxdbg_btree_branch crxdbg_btree_branch;
union _crxdbg_btree_branch {
	crxdbg_btree_branch *branches[2];
	crxdbg_btree_result result;
};

typedef struct {
	crex_ulong count;
	crex_ulong depth;
	bool persistent;
	crxdbg_btree_branch *branch;
} crxdbg_btree;

typedef struct {
	crxdbg_btree *tree;
	crex_ulong cur;
	crex_ulong end;
} crxdbg_btree_position;

void crxdbg_btree_init(crxdbg_btree *tree, crex_ulong depth);
void crxdbg_btree_clean(crxdbg_btree *tree);
crxdbg_btree_result *crxdbg_btree_find(crxdbg_btree *tree, crex_ulong idx);
crxdbg_btree_result *crxdbg_btree_find_closest(crxdbg_btree *tree, crex_ulong idx);
crxdbg_btree_position crxdbg_btree_find_between(crxdbg_btree *tree, crex_ulong lower_idx, crex_ulong higher_idx);
crxdbg_btree_result *crxdbg_btree_next(crxdbg_btree_position *pos);
int crxdbg_btree_delete(crxdbg_btree *tree, crex_ulong idx);

#define CRXDBG_BTREE_INSERT 1
#define CRXDBG_BTREE_UPDATE 2
#define CRXDBG_BTREE_OVERWRITE (CRXDBG_BTREE_INSERT | CRXDBG_BTREE_UPDATE)

int crxdbg_btree_insert_or_update(crxdbg_btree *tree, crex_ulong idx, void *ptr, int flags);
#define crxdbg_btree_insert(tree, idx, ptr) crxdbg_btree_insert_or_update(tree, idx, ptr, CRXDBG_BTREE_INSERT)
#define crxdbg_btree_update(tree, idx, ptr) crxdbg_btree_insert_or_update(tree, idx, ptr, CRXDBG_BTREE_UPDATE)
#define crxdbg_btree_overwrite(tree, idx, ptr) crxdbg_btree_insert_or_update(tree, idx, ptr, CRXDBG_BTREE_OVERWRITE)


/* debugging functions */
void crxdbg_btree_branch_dump(crxdbg_btree_branch *branch, crex_ulong depth);
void crxdbg_btree_dump(crxdbg_btree *tree);

#endif
