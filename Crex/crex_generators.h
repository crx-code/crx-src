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
   | Authors: Nikita Popov <nikic@crx.net>                                |
   |          Bob Weinand <bobwei9@hotmail.com>                           |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_GENERATORS_H
#define CREX_GENERATORS_H

#include <stdint.h>

BEGIN_EXTERN_C()

extern CREX_API crex_class_entry *crex_ce_generator;
extern CREX_API crex_class_entry *crex_ce_ClosedGeneratorException;

typedef struct _crex_generator_node crex_generator_node;
typedef struct _crex_generator crex_generator;

/* The concept of `yield from` exposes problems when accessed at different levels of the chain of delegated generators. We need to be able to reference the currently executed Generator in all cases and still being able to access the return values of finished Generators.
 * The solution to this problem is a doubly-linked tree, which all Generators referenced in maintain a reference to. It should be impossible to avoid walking the tree in all cases. This way, we only need tree walks from leaf to root in case where some part of the `yield from` chain is passed to another `yield from`. (Update of leaf node pointer and list of multi-children nodes needed when leaf gets a child in direct path from leaf to root node.) But only in that case, which should be a fairly rare case (which is then possible, but not totally cheap).
 * The root of the tree is then the currently executed Generator. The subnodes of the tree (all except the root node) are all Generators which do `yield from`. Each node of the tree knows a pointer to one leaf descendant node. Each node with multiple children needs a list of all leaf descendant nodes paired with pointers to their respective child node. (The stack is determined by leaf node pointers) Nodes with only one child just don't need a list, there it is enough to just have a pointer to the child node. Further, leaf nodes store a pointer to the root node.
 * That way, when we advance any generator, we just need to look up a leaf node (which all have a reference to a root node). Then we can see at the root node whether current Generator is finished. If it isn't, all is fine and we can just continue. If the Generator finished, there will be two cases. Either it is a simple node with just one child, then go down to child node. Or it has multiple children and we now will remove the current leaf node from the list of nodes (unnecessary, is microoptimization) and go down to the child node whose reference was paired with current leaf node. Child node is then removed its parent reference and becomes new top node. Or the current node references the Generator we're currently executing, then we can continue from the YIELD_FROM opcode. When a node referenced as root node in a leaf node has a parent, then we go the way up until we find a root node without parent.
 * In case we go into a new `yield from` level, a node is created on top of current root and becomes the new root. Leaf node needs to be updated with new root node then.
 * When a Generator referenced by a node of the tree is added to `yield from`, that node now gets a list of children (we need to walk the descendants of that node and nodes of the tree of the other Generator down to the first multi-children node and copy all the leaf node pointers from there). In case there was no multi-children node (linear tree), we just add a pair (pointer to leaf node, pointer to child node), with the child node being in a direct path from leaf to this node.
 */

struct _crex_generator_node {
	crex_generator *parent; /* NULL for root */
	uint32_t children;
	union {
		HashTable *ht; /* if multiple children */
		crex_generator *single; /* if one child */
	} child;
	/* One generator can cache a direct pointer to the current root.
	 * The leaf member points back to the generator using the root cache. */
	union {
		crex_generator *leaf; /* if parent != NULL */
		crex_generator *root; /* if parent == NULL */
	} ptr;
};

struct _crex_generator {
	crex_object std;

	/* The suspended execution context. */
	crex_execute_data *execute_data;

	/* Frozen call stack for "yield" used in context of other calls */
	crex_execute_data *frozen_call_stack;

	/* Current value */
	zval value;
	/* Current key */
	zval key;
	/* Return value */
	zval retval;
	/* Variable to put sent value into */
	zval *send_target;
	/* Largest used integer key for auto-incrementing keys */
	crex_long largest_used_integer_key;

	/* Values specified by "yield from" to yield from this generator.
	 * This is only used for arrays or non-generator Traversables.
	 * This zval also uses the u2 structure in the same way as
	 * by-value foreach. */
	zval values;

	/* Node of waiting generators when multiple "yield from" expressions
	 * are nested. */
	crex_generator_node node;

	/* Fake execute_data for stacktraces */
	crex_execute_data execute_fake;

	/* CREX_GENERATOR_* flags */
	uint8_t flags;
};

static const uint8_t CREX_GENERATOR_CURRENTLY_RUNNING = 0x1;
static const uint8_t CREX_GENERATOR_FORCED_CLOSE      = 0x2;
static const uint8_t CREX_GENERATOR_AT_FIRST_YIELD    = 0x4;
static const uint8_t CREX_GENERATOR_DO_INIT           = 0x8;
static const uint8_t CREX_GENERATOR_IN_FIBER          = 0x10;

void crex_register_generator_ce(void);
CREX_API void crex_generator_close(crex_generator *generator, bool finished_execution);
CREX_API void crex_generator_resume(crex_generator *generator);

CREX_API void crex_generator_restore_call_stack(crex_generator *generator);
CREX_API crex_execute_data* crex_generator_freeze_call_stack(crex_execute_data *execute_data);

void crex_generator_yield_from(crex_generator *generator, crex_generator *from);
CREX_API crex_execute_data *crex_generator_check_placeholder_frame(crex_execute_data *ptr);

CREX_API crex_generator *crex_generator_update_current(crex_generator *generator);
CREX_API crex_generator *crex_generator_update_root(crex_generator *generator);
static crex_always_inline crex_generator *crex_generator_get_current(crex_generator *generator)
{
	if (EXPECTED(generator->node.parent == NULL)) {
		/* we're not in yield from mode */
		return generator;
	}

	crex_generator *root = generator->node.ptr.root;
	if (!root) {
		root = crex_generator_update_root(generator);
	}

	if (EXPECTED(root->execute_data)) {
		/* generator still running */
		return root;
	}

	return crex_generator_update_current(generator);
}

END_EXTERN_C()

#endif
