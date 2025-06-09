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

#include "crxdbg_btree.h"
#include "crxdbg.h"

#define CHOOSE_BRANCH(n) \
	branch = branch->branches[!!(n)];

#ifdef _Win32
# undef pemalloc
# undef pefree
# define pemalloc(size, persistent) malloc(size)
# define pefree(ptr, persistent) free(ptr)
#endif

/* depth in bits */
void crxdbg_btree_init(crxdbg_btree *tree, crex_ulong depth) {
	tree->depth = depth;
	tree->branch = NULL;
	tree->persistent = 0;
	tree->count = 0;
}

crxdbg_btree_result *crxdbg_btree_find(crxdbg_btree *tree, crex_ulong idx) {
	crxdbg_btree_branch *branch = tree->branch;
	int i = tree->depth - 1;

	if (branch == NULL) {
		return NULL;
	}

	do {
		if ((idx >> i) % 2 == 1) {
		 	if (branch->branches[1]) {
				CHOOSE_BRANCH(1);
			} else {
				return NULL;
			}
		} else {
			if (branch->branches[0]) {
				CHOOSE_BRANCH(0);
			} else {
				return NULL;
			}
		}
	} while (i--);

	return &branch->result;
}

crxdbg_btree_result *crxdbg_btree_find_closest(crxdbg_btree *tree, crex_ulong idx) {
	crxdbg_btree_branch *branch = tree->branch;
	int i = tree->depth - 1, last_superior_i = -1;

	if (branch == NULL) {
		return NULL;
	}

	/* find nearest watchpoint */
	do {
		if ((idx >> i) % 2 == 0) {
			if (branch->branches[0]) {
				CHOOSE_BRANCH(0);
			/* an impossible branch was found if: */
			} else {
				/* there's no lower branch than idx */
				if (last_superior_i == -1) {
					/* failure */
					return NULL;
				}
				/* reset state */
				branch = tree->branch;
				i = tree->depth - 1;
				/* follow branch according to bits in idx until the last lower branch before the impossible branch */
				do {
					CHOOSE_BRANCH((idx >> i) % 2 == 1 && branch->branches[1]);
				} while (--i > last_superior_i);
				/* use now the lower branch of which we can be sure that it contains only branches lower than idx */
				CHOOSE_BRANCH(0);
				/* and choose the highest possible branch in the branch containing only branches lower than idx */
				while (i--) {
					CHOOSE_BRANCH(branch->branches[1]);
				}
				break;
			}
		/* follow branch according to bits in idx until having found an impossible branch */
		} else {
			if (branch->branches[1]) {
				if (branch->branches[0]) {
					last_superior_i = i;
				}
				CHOOSE_BRANCH(1);
			} else {
				CHOOSE_BRANCH(0);
				while (i--) {
					CHOOSE_BRANCH(branch->branches[1]);
				}
				break;
			}
		}
	} while (i--);

	return &branch->result;
}

crxdbg_btree_position crxdbg_btree_find_between(crxdbg_btree *tree, crex_ulong lower_idx, crex_ulong higher_idx) {
	crxdbg_btree_position pos;

	pos.tree = tree;
	pos.end = lower_idx;
	pos.cur = higher_idx;

	return pos;
}

crxdbg_btree_result *crxdbg_btree_next(crxdbg_btree_position *pos) {
	crxdbg_btree_result *result = crxdbg_btree_find_closest(pos->tree, pos->cur);

	if (result == NULL || result->idx < pos->end) {
		return NULL;
	}

	pos->cur = result->idx - 1;

	return result;
}

int crxdbg_btree_insert_or_update(crxdbg_btree *tree, crex_ulong idx, void *ptr, int flags) {
	int i = tree->depth - 1;
	crxdbg_btree_branch **branch = &tree->branch;

	do {
		if (*branch == NULL) {
			break;
		}
		branch = &(*branch)->branches[(idx >> i) % 2];
	} while (i--);

	if (*branch == NULL) {
		if (!(flags & CRXDBG_BTREE_INSERT)) {
			return FAILURE;
		}

		{
			crxdbg_btree_branch *memory = *branch = pemalloc((i + 2) * sizeof(crxdbg_btree_branch), tree->persistent);
			do {
				(*branch)->branches[!((idx >> i) % 2)] = NULL;
				branch = &(*branch)->branches[(idx >> i) % 2];
				*branch = ++memory;
			} while (i--);
			tree->count++;
		}
	} else if (!(flags & CRXDBG_BTREE_UPDATE)) {
		return FAILURE;
	}

	(*branch)->result.idx = idx;
	(*branch)->result.ptr = ptr;

	return SUCCESS;
}

int crxdbg_btree_delete(crxdbg_btree *tree, crex_ulong idx) {
	int i = tree->depth;
	crxdbg_btree_branch *branch = tree->branch;
	int i_last_dual_branch = -1, last_dual_branch_branch;
	crxdbg_btree_branch *last_dual_branch = NULL;

	goto check_branch_existence;
	do {
		if (branch->branches[0] && branch->branches[1]) {
			last_dual_branch = branch;
			i_last_dual_branch = i;
			last_dual_branch_branch = (idx >> i) % 2;
		}
		branch = branch->branches[(idx >> i) % 2];

check_branch_existence:
		if (branch == NULL) {
			return FAILURE;
		}
	} while (i--);

	tree->count--;

	if (i_last_dual_branch == -1) {
		pefree(tree->branch, tree->persistent);
		tree->branch = NULL;
	} else {
		if (last_dual_branch->branches[last_dual_branch_branch] == last_dual_branch + 1) {
			crxdbg_btree_branch *original_branch = last_dual_branch->branches[!last_dual_branch_branch];

			memcpy(last_dual_branch + 1, last_dual_branch->branches[!last_dual_branch_branch], (i_last_dual_branch + 1) * sizeof(crxdbg_btree_branch));
			pefree(last_dual_branch->branches[!last_dual_branch_branch], tree->persistent);
			last_dual_branch->branches[!last_dual_branch_branch] = last_dual_branch + 1;

			branch = last_dual_branch->branches[!last_dual_branch_branch];
			for (i = i_last_dual_branch; i--;) {
				branch = (branch->branches[branch->branches[1] == ++original_branch] = last_dual_branch + i_last_dual_branch - i + 1);
			}
		} else {
			pefree(last_dual_branch->branches[last_dual_branch_branch], tree->persistent);
		}

		last_dual_branch->branches[last_dual_branch_branch] = NULL;
	}

	return SUCCESS;
}

void crxdbg_btree_clean_recursive(crxdbg_btree_branch *branch, crex_ulong depth, bool persistent) {
	crxdbg_btree_branch *start = branch;
	while (depth--) {
		bool use_branch = branch + 1 == branch->branches[0];
		if (branch->branches[use_branch]) {
			crxdbg_btree_clean_recursive(branch->branches[use_branch], depth, persistent);
		}
	}

	pefree(start, persistent);
}

void crxdbg_btree_clean(crxdbg_btree *tree) {
	if (tree->branch) {
		crxdbg_btree_clean_recursive(tree->branch, tree->depth, tree->persistent);
		tree->branch = NULL;
		tree->count = 0;
	}
}

void crxdbg_btree_branch_dump(crxdbg_btree_branch *branch, crex_ulong depth) {
	if (branch) {
		if (depth--) {
			crxdbg_btree_branch_dump(branch->branches[0], depth);
			crxdbg_btree_branch_dump(branch->branches[1], depth);
		} else {
			fprintf(stderr, "%p: %p\n", (void *) branch->result.idx, branch->result.ptr);
		}
	}
}

void crxdbg_btree_dump(crxdbg_btree *tree) {
	crxdbg_btree_branch_dump(tree->branch, tree->depth);
}
