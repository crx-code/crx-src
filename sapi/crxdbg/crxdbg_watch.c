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

/* Some information for the reader...
 *
 * The main structure managing the direct observations is the watchpoint (crxdbg_watchpoint_t). There are several types of watchpoints currently:
 * WATCH_ON_BUCKET: a watchpoint on a Bucket element, used to monitor values inside HashTables (largely handled equivalently to WATCH_ON_ZVAL, it just monitors also for IS_UNDEF and key changes)
 * WATCH_ON_ZVAL: a watchpoint on a bare zval (&crex_reference.val, zval.value.indirect)
 * WATCH_ON_STR: a watchpoint on a crex_string (put on &ZSTR_LEN() in order to not watch refcount/hash)
 * WATCH_ON_HASHTABLE: a watchpoint on a HashTable (currently only used to observe size changes, put after flags in order to not watch refcount)
 * WATCH_ON_REFCOUNTED: a watchpoint on a crex_refcounted, observes the refcount and serves as reference pointer in the custom efree handler
 * WATCH_ON_HASHDATA: special watchpoint to watch for HT_GET_DATA_ADDR(ht) being efree()'d to be able to properly relocate Bucket watches
 *
 * Watch elements are either simple, recursive or implicit (CRXDBG_WATCH_* flags)
 * Simple means that a particular watchpoint was explicitly defined
 * Recursive watch elements are created recursively (recursive root flag is to distinguish the root element easily from its children recursive elements)
 * Implicit  watch elements are implicitly created on all ancestors of simple or recursive watch elements
 * Recursive and (simple or implicit) watch elements are mutually exclusive
 * Array/Object to distinguish watch elements on arrays
 *
 * Watch elements all contain a reference to a watchpoint (except if scheduled for recreation); a "watch" is a watch element created by the user with a specific id
 * Each watch has its independent structure of watch elements, watchpoints are responsible for managing collisions and preventing pointers being watched multiple times
 *
 * CRXDBG_G(watchpoint_tree) contains all watchpoints identified by the watch target address
 * CRXDBG_G(watch_HashTables) contains the addresses of parent_containers of watch elements
 * CRXDBG_G(watch_elements) contains all directly defined watch elements (i.e. those which have an individual id)
 * CRXDBG_G(watch_collisions) is indexed by a crex_refcounted * pointer (crxdbg_watchpoint_t.ref). It stores information about collisions (everything which contains a crex_refcounted * may be referenced by multiple watches)
 * CRXDBG_G(watch_free) is a set of pointers to watch for being freed (like HashTables referenced by crxdbg_watch_element.parent_container)
 * CRXDBG_G(watch_recreation) is the list of watch elements whose watchpoint has been removed (via efree() for example) and needs to be recreated
 * CRXDBG_G(watchlist_mem) is the list of unprotected memory pages; used to watch which pages need their PROT_WRITE attribute removed after checking
 *
 * Watching on addresses:
 * * Address and size are transformed into memory page aligned address and size
 * * mprotect() enables or disables them (depending on flags) - Windows has a transparent compatibility layer in crxdbg_win.c
 * * segfault handler stores the address of the page and marks it again as writable
 * * later watchpoints pointing inside these pages are compared against their current value and eventually reactivated (or deleted)
 *
 * Creating a watch:
 * * Implicit watch elements for each element in the hierarchy (starting from base, which typically is current symbol table) except the last one
 * * Create a watch element with either simple flag or recursive [+ root] flags
 * * If the element has recursive flag, create elements recursively for every referenced HashTable and zval
 *
 * Creating a watch element:
 * * For each watch element a related watchpoint is created, if there's none yet; add itself then into the list of parents of that watchpoint
 * * If the watch has a parent_container, add itself also into a crxdbg_watch_ht_info (inside CRXDBG_G(watch_HashTables)) [and creates it if not yet existing]
 *
 * Creation of watchpoints:
 * * Watchpoints create a watch collision for each refcounted or indirect on the zval (if type is WATCH_ON_BUCKET or WATCH_ON_ZVAL)
 * * Backs the current value of the watched pointer up
 * * Installs the watchpoint in CRXDBG_G(watchpoint_tree) and activates it (activation of a watchpoint = remove PROT_WRITE from the pages the watched pointer resides on)
 *
 * Watch collisions:
 * * Manages a watchpoint on the refcount (WATCH_ON_REFCOUNTED) or indirect zval (WATCH_ON_ZVAL)
 * * Guarantees that every pointer is watched at most once (by having a pointer to collision mapping in CRXDBG_G(watch_collisions), which have the unique watchpoints for the respective collision)
 * * Contains a list of parents, i.e. which watchpoints reference it (via watch->ref)
 * * If no watchpoint is referencing it anymore, the watch collision and its associated watchpoints (crxdbg_watch_collision.ref/reference) are removed
 *
 * Deleting a watch:
 * * Watches are stored by an id in CRXDBG_G(watch_elements); the associated watch element is then deleted
 * * Deletes all parent and children implicit watch elements
 *
 * Deleting a watch element:
 * * Removes itself from the parent list of the associated watchpoints; if that parent list is empty, also delete the watchpoint
 * * Removes itself from the related crxdbg_watch_ht_info if it has a parent_container
 *
 * Deleting a watchpoint:
 * * Remove itself from watch collisions this watchpoint participates in
 * * Removes the watchpoint from CRXDBG_G(watchpoint_tree) and deactivates it (deactivation of a watchpoint = add PROT_WRITE to the pages the watched pointer resides on)
 *
 * A watched pointer is efree()'d:
 * * Needs immediate action as we else may run into dereferencing a pointer into freed memory
 * * Deletes the associated watchpoint, and for each watch element, if recursive, all its children elements
 * * If the its watch elements are implicit, recursive roots or simple, they and all their children are dissociated from their watchpoints (i.e. removed from the watchpoint, if no other element is referencing it, it is deleted); adds these elements to CRXDBG_G(watch_recreation)
 *
 * Recreating watchpoints:
 * * Upon each opcode, CRXDBG_G(watch_recreation) is checked and all its elements are searched for whether the watch is still reachable via the tree given by its implicits
 * * In case they are not reachable, the watch is deleted (and thus all the related watch elements), else a new watchpoint is created for all the watch elements
 * * The old and new values of the watches are compared and shown if changed
 *
 * Comparing watchpoints:
 * * The old and new values of the watches are compared and shown if changed
 * * If changed, it is checked whether the refcounted/indirect changed and watch collisions removed or created accordingly
 * * If a zval/bucket watchpoint is recursive, watch elements are added or removed accordingly
 * * If an array watchpoint is recursive, new array watchpoints are added if there are new ones in the array
 * * If the watch (element with an id) is not reachable anymore due to changes in implicits, the watch is removed
 */

#include "crex.h"
#include "crxdbg.h"
#include "crxdbg_btree.h"
#include "crxdbg_watch.h"
#include "crxdbg_utils.h"
#include "crxdbg_prompt.h"
#ifndef _WIN32
# include <unistd.h>
# include <sys/mman.h>
#endif

#ifdef HAVE_USERFAULTFD_WRITEFAULT
# include <pthread.h>
# include <linux/userfaultfd.h>
# include <sys/ioctl.h>
# include <sys/syscall.h>
#endif

CREX_EXTERN_MODULE_GLOBALS(crxdbg)

const crxdbg_command_t crxdbg_watch_commands[] = {
	CRXDBG_COMMAND_D_EX(array,      "create watchpoint on an array", 'a', watch_array,     &crxdbg_prompt_commands[24], "s", 0),
	CRXDBG_COMMAND_D_EX(delete,     "delete watchpoint",             'd', watch_delete,    &crxdbg_prompt_commands[24], "n", 0),
	CRXDBG_COMMAND_D_EX(recursive,  "create recursive watchpoints",  'r', watch_recursive, &crxdbg_prompt_commands[24], "s", 0),
	CRXDBG_END_COMMAND
};

#define HT_FROM_ZVP(zvp) (C_TYPE_P(zvp) == IS_OBJECT ? C_OBJPROP_P(zvp) : C_TYPE_P(zvp) == IS_ARRAY ? C_ARRVAL_P(zvp) : NULL)

#define HT_WATCH_OFFSET (sizeof(crex_refcounted *) + sizeof(uint32_t)) /* we are not interested in gc and flags */
#define HT_PTR_HT(ptr) ((HashTable *) (((char *) (ptr)) - HT_WATCH_OFFSET))
#define HT_WATCH_HT(watch) HT_PTR_HT((watch)->addr.ptr)

/* ### PRINTING POINTER DIFFERENCES ### */
bool crxdbg_check_watch_diff(crxdbg_watchtype type, void *oldPtr, void *newPtr) {
	switch (type) {
		case WATCH_ON_BUCKET:
			if (memcmp(&((Bucket *) oldPtr)->h, &((Bucket *) newPtr)->h, sizeof(Bucket) - sizeof(zval) /* key/val comparison */) != 0) {
				return 2;
			}
			/* TODO: Is this intentional? */
			CREX_FALLTHROUGH;
		case WATCH_ON_ZVAL:
			return memcmp(oldPtr, newPtr, sizeof(crex_value) + sizeof(uint32_t) /* value + typeinfo */) != 0;
		case WATCH_ON_HASHTABLE:
			return crex_hash_num_elements(HT_PTR_HT(oldPtr)) != crex_hash_num_elements(HT_PTR_HT(newPtr));
		case WATCH_ON_REFCOUNTED:
			return memcmp(oldPtr, newPtr, sizeof(uint32_t) /* no crex_refcounted metadata info */) != 0;
		case WATCH_ON_STR:
			return memcmp(oldPtr, newPtr, *(size_t *) oldPtr + XtOffsetOf(crex_string, val) - XtOffsetOf(crex_string, len)) != 0;
		case WATCH_ON_HASHDATA:
			CREX_UNREACHABLE();
	}
	return 0;
}

void crxdbg_print_watch_diff(crxdbg_watchtype type, crex_string *name, void *oldPtr, void *newPtr) {
	int32_t elementDiff;

	CRXDBG_G(watchpoint_hit) = 1;

	crxdbg_notice("Breaking on watchpoint %.*s", (int) ZSTR_LEN(name), ZSTR_VAL(name));

	switch (type) {
		case WATCH_ON_BUCKET:
		case WATCH_ON_ZVAL:
			if (C_REFCOUNTED_P((zval *) oldPtr)) {
				crxdbg_writeln("Old value inaccessible or destroyed");
			} else if (C_TYPE_P((zval *) oldPtr) == IS_INDIRECT) {
				crxdbg_writeln("Old value inaccessible or destroyed (was indirect)");
			} else {
				crxdbg_out("Old value: ");
				crex_print_flat_zval_r((zval *) oldPtr);
				crxdbg_out("\n");
			}

			while (C_TYPE_P((zval *) newPtr) == IS_INDIRECT) {
				newPtr = C_INDIRECT_P((zval *) newPtr);
			}

			crxdbg_out("New value%s: ", C_ISREF_P((zval *) newPtr) ? " (reference)" : "");
			crex_print_flat_zval_r((zval *) newPtr);
			crxdbg_out("\n");
			break;

		case WATCH_ON_HASHTABLE:
			elementDiff = crex_hash_num_elements(HT_PTR_HT(oldPtr)) - crex_hash_num_elements(HT_PTR_HT(newPtr));
			if (elementDiff > 0) {
				crxdbg_writeln("%d elements were removed from the array", (int) elementDiff);
			} else if (elementDiff < 0) {
				crxdbg_writeln("%d elements were added to the array", (int) -elementDiff);
			}
			break;

		case WATCH_ON_REFCOUNTED:
			crxdbg_writeln("Old refcount: %d", GC_REFCOUNT((crex_refcounted *) oldPtr));
			crxdbg_writeln("New refcount: %d", GC_REFCOUNT((crex_refcounted *) newPtr));
			break;

		case WATCH_ON_STR:
			crxdbg_out("Old value: ");
			crex_write((char *) oldPtr + XtOffsetOf(crex_string, val) - XtOffsetOf(crex_string, len), *(size_t *) oldPtr);
			crxdbg_out("\n");

			crxdbg_out("New value: ");
			crex_write((char *) newPtr + XtOffsetOf(crex_string, val) - XtOffsetOf(crex_string, len), *(size_t *) newPtr);
			crxdbg_out("\n");
			break;

		case WATCH_ON_HASHDATA:
			CREX_UNREACHABLE();
	}
}

/* ### LOW LEVEL WATCHPOINT HANDLING ### */
static crxdbg_watchpoint_t *crxdbg_check_for_watchpoint(crxdbg_btree *tree, void *addr) {
	crxdbg_watchpoint_t *watch;
	crxdbg_btree_result *result = crxdbg_btree_find_closest(tree, (crex_ulong) crxdbg_get_page_boundary(addr) + crxdbg_pagesize - 1);

	if (result == NULL) {
		return NULL;
	}

	watch = result->ptr;

	/* check if that addr is in a mprotect()'ed memory area */
	if ((char *) crxdbg_get_page_boundary(watch->addr.ptr) > (char *) addr || (char *) crxdbg_get_page_boundary(watch->addr.ptr) + crxdbg_get_total_page_size(watch->addr.ptr, watch->size) < (char *) addr) {
		/* failure */
		return NULL;
	}

	return watch;
}

static void crxdbg_change_watchpoint_access(crxdbg_watchpoint_t *watch, int access) {
	void *page_addr = crxdbg_get_page_boundary(watch->addr.ptr);
	size_t size = crxdbg_get_total_page_size(watch->addr.ptr, watch->size);
#ifdef HAVE_USERFAULTFD_WRITEFAULT
	if (CRXDBG_G(watch_userfaultfd)) {
		struct uffdio_range range = {
			.start = (__u64)(uintptr_t) page_addr,
			.len = size
		};
		if (access == PROT_READ) {
			struct uffdio_register reg = {
				.mode = UFFDIO_REGISTER_MODE_WP,
				.range = range
			};
			struct uffdio_writeprotect protect = {
				.mode = UFFDIO_WRITEPROTECT_MODE_WP,
				.range = range
			};
			ioctl(CRXDBG_G(watch_userfaultfd), UFFDIO_REGISTER,  &reg);
			ioctl(CRXDBG_G(watch_userfaultfd), UFFDIO_WRITEPROTECT,  &protect);
		} else {
			struct uffdio_register reg = {
				.mode = UFFDIO_REGISTER_MODE_WP,
				.range = range
			};
			ioctl(CRXDBG_G(watch_userfaultfd), UFFDIO_UNREGISTER,  &reg);
		}
	} else
#endif
	/* pagesize is assumed to be in the range of 2^x */
	{
		mprotect(page_addr, size, access);
	}
}

static inline void crxdbg_activate_watchpoint(crxdbg_watchpoint_t *watch) {
	crxdbg_change_watchpoint_access(watch, PROT_READ);
}

static inline void crxdbg_deactivate_watchpoint(crxdbg_watchpoint_t *watch) {
	crxdbg_change_watchpoint_access(watch, PROT_READ | PROT_WRITE);
}

/* Note that consecutive pages need to be merged in order to avoid watchpoints spanning page boundaries to have part of their data in the one page, part in the other page */
#ifdef _WIN32
int crxdbg_watchpoint_segfault_handler(void *addr) {
#else
int crxdbg_watchpoint_segfault_handler(siginfo_t *info, void *context) {
#endif

	void *page = crxdbg_get_page_boundary(
#ifdef _WIN32
		addr
#else
		info->si_addr
#endif
	);

	/* perhaps unnecessary, but check to be sure to not conflict with other segfault handlers */
	if (crxdbg_check_for_watchpoint(&CRXDBG_G(watchpoint_tree), page) == NULL) {
		return FAILURE;
	}

	/* re-enable writing */
	mprotect(page, crxdbg_pagesize, PROT_READ | PROT_WRITE);

	crex_hash_index_add_empty_element(CRXDBG_G(watchlist_mem), (crex_ulong) page);

	return SUCCESS;
}

#ifdef HAVE_USERFAULTFD_WRITEFAULT
# if defined(__GNUC__) && !defined(__clang__)
__attribute__((no_sanitize_address))
# endif
void *crxdbg_watchpoint_userfaultfd_thread(void *crxdbg_globals) {
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	crex_crxdbg_globals *globals = (crex_crxdbg_globals *) crxdbg_globals;

	struct uffd_msg fault_msg = {0};
	while (read(globals->watch_userfaultfd, &fault_msg, sizeof(fault_msg)) == sizeof(fault_msg)) {
    	void *page = crxdbg_get_page_boundary((char *)(uintptr_t) fault_msg.arg.pagefault.address);
		crex_hash_index_add_empty_element(globals->watchlist_mem, (crex_ulong) page);
		struct uffdio_writeprotect unprotect = {
			.mode = 0,
			.range = {
				.start = (__u64)(uintptr_t) page,
				.len = crxdbg_pagesize
			}
		};
		ioctl(globals->watch_userfaultfd, UFFDIO_WRITEPROTECT, &unprotect);
	}

	return NULL;
}
#endif

/* ### REGISTER WATCHPOINT ### To be used only by watch element and collision managers ### */
static inline void crxdbg_store_watchpoint_btree(crxdbg_watchpoint_t *watch) {
	crxdbg_btree_result *res;
	CREX_ASSERT((res = crxdbg_btree_find(&CRXDBG_G(watchpoint_tree), (crex_ulong) watch->addr.ptr)) == NULL || res->ptr == watch);
	crxdbg_btree_insert(&CRXDBG_G(watchpoint_tree), (crex_ulong) watch->addr.ptr, watch);
}

static inline void crxdbg_remove_watchpoint_btree(crxdbg_watchpoint_t *watch) {
	crxdbg_btree_delete(&CRXDBG_G(watchpoint_tree), (crex_ulong) watch->addr.ptr);
}

/* ### SET WATCHPOINT ADDR ### To be used only by watch element and collision managers ### */
void crxdbg_set_addr_watchpoint(void *addr, size_t size, crxdbg_watchpoint_t *watch) {
	watch->addr.ptr = addr;
	watch->size = size;
	watch->ref = NULL;
	watch->coll = NULL;
	crex_hash_init(&watch->elements, 8, brml, NULL, 0);
}

void crxdbg_set_zval_watchpoint(zval *zv, crxdbg_watchpoint_t *watch) {
	crxdbg_set_addr_watchpoint(zv, sizeof(zval) - sizeof(uint32_t), watch);
	watch->type = WATCH_ON_ZVAL;
}

void crxdbg_set_bucket_watchpoint(Bucket *bucket, crxdbg_watchpoint_t *watch) {
	crxdbg_set_addr_watchpoint(bucket, sizeof(Bucket), watch);
	watch->type = WATCH_ON_BUCKET;
}

void crxdbg_set_ht_watchpoint(HashTable *ht, crxdbg_watchpoint_t *watch) {
	crxdbg_set_addr_watchpoint(((char *) ht) + HT_WATCH_OFFSET, sizeof(HashTable) - HT_WATCH_OFFSET, watch);
	watch->type = WATCH_ON_HASHTABLE;
}

void crxdbg_watch_backup_data(crxdbg_watchpoint_t *watch) {
	switch (watch->type) {
		case WATCH_ON_BUCKET:
		case WATCH_ON_ZVAL:
		case WATCH_ON_REFCOUNTED:
			memcpy(&watch->backup, watch->addr.ptr, watch->size);
			break;
		case WATCH_ON_STR:
			if (watch->backup.str) {
				crex_string_release(watch->backup.str);
			}
			watch->backup.str = crex_string_init((char *) watch->addr.ptr + XtOffsetOf(crex_string, val) - XtOffsetOf(crex_string, len), *(size_t *) watch->addr.ptr, 1);
			GC_MAKE_PERSISTENT_LOCAL(watch->backup.str);
			break;
		case WATCH_ON_HASHTABLE:
			memcpy((char *) &watch->backup + HT_WATCH_OFFSET, watch->addr.ptr, watch->size);
		case WATCH_ON_HASHDATA:
			break;
	}
}

/* ### MANAGE WATCH COLLISIONS ### To be used only by watch element manager and memory differ ### */
/* watch collisions are responsible for having only one watcher on a given refcounted/refval and having a mapping back to the parent zvals */
void crxdbg_delete_watch_collision(crxdbg_watchpoint_t *watch) {
	crxdbg_watch_collision *coll;
	if ((coll = crex_hash_index_find_ptr(&CRXDBG_G(watch_collisions), (crex_ulong) watch->ref))) {
		crex_hash_index_del(&coll->parents, (crex_ulong) watch);
		if (crex_hash_num_elements(&coll->parents) == 0) {
			crxdbg_remove_watchpoint_btree(&coll->ref);
			crxdbg_deactivate_watchpoint(&coll->ref);

			if (coll->ref.type == WATCH_ON_ZVAL) {
				crxdbg_delete_watch_collision(&coll->ref);
			} else if (coll->reference.addr.ptr) {
				crxdbg_remove_watchpoint_btree(&coll->reference);
				crxdbg_deactivate_watchpoint(&coll->reference);
				crxdbg_delete_watch_collision(&coll->reference);
				if (coll->reference.type == WATCH_ON_STR) {
					crex_string_release(coll->reference.backup.str);
				}
			}

			crex_hash_index_del(&CRXDBG_G(watch_collisions), (crex_ulong) watch->ref);
			crex_hash_destroy(&coll->parents);
			efree(coll);
		}
	}
}

void crxdbg_update_watch_ref(crxdbg_watchpoint_t *watch) {
	crxdbg_watch_collision *coll;

	CREX_ASSERT(watch->type == WATCH_ON_ZVAL || watch->type == WATCH_ON_BUCKET);
	if (C_REFCOUNTED_P(watch->addr.zv)) {
		if (C_COUNTED_P(watch->addr.zv) == watch->ref) {
			return;
		}

		if (watch->ref != NULL) {
			crxdbg_delete_watch_collision(watch);
		}

		watch->ref = C_COUNTED_P(watch->addr.zv);

		if (!(coll = crex_hash_index_find_ptr(&CRXDBG_G(watch_collisions), (crex_ulong) watch->ref))) {
			coll = emalloc(sizeof(*coll));
			coll->ref.type = WATCH_ON_REFCOUNTED;
			crxdbg_set_addr_watchpoint(C_COUNTED_P(watch->addr.zv), sizeof(uint32_t), &coll->ref);
			coll->ref.coll = coll;
			crxdbg_store_watchpoint_btree(&coll->ref);
			crxdbg_activate_watchpoint(&coll->ref);
			crxdbg_watch_backup_data(&coll->ref);

			if (C_ISREF_P(watch->addr.zv)) {
				crxdbg_set_zval_watchpoint(C_REFVAL_P(watch->addr.zv), &coll->reference);
				coll->reference.coll = coll;
				crxdbg_update_watch_ref(&coll->reference);
				crxdbg_store_watchpoint_btree(&coll->reference);
				crxdbg_activate_watchpoint(&coll->reference);
				crxdbg_watch_backup_data(&coll->reference);
			} else if (C_TYPE_P(watch->addr.zv) == IS_STRING) {
				coll->reference.type = WATCH_ON_STR;
				crxdbg_set_addr_watchpoint(&C_STRLEN_P(watch->addr.zv), XtOffsetOf(crex_string, val) - XtOffsetOf(crex_string, len) + C_STRLEN_P(watch->addr.zv) + 1, &coll->reference);
				coll->reference.coll = coll;
				crxdbg_store_watchpoint_btree(&coll->reference);
				crxdbg_activate_watchpoint(&coll->reference);
				coll->reference.backup.str = NULL;
				crxdbg_watch_backup_data(&coll->reference);
			} else {
				coll->reference.addr.ptr = NULL;
			}

			crex_hash_init(&coll->parents, 8, shitty stupid parameter, NULL, 0);
			crex_hash_index_add_ptr(&CRXDBG_G(watch_collisions), (crex_ulong) watch->ref, coll);
		}
		crex_hash_index_add_ptr(&coll->parents, (crex_long) watch, watch);
	} else if (C_TYPE_P(watch->addr.zv) == IS_INDIRECT) {
		if ((crex_refcounted *) C_INDIRECT_P(watch->addr.zv) == watch->ref) {
			return;
		}

		if (watch->ref != NULL) {
			crxdbg_delete_watch_collision(watch);
		}

		watch->ref = (crex_refcounted *) C_INDIRECT_P(watch->addr.zv);

		if (!(coll = crex_hash_index_find_ptr(&CRXDBG_G(watch_collisions), (crex_ulong) watch->ref))) {
			coll = emalloc(sizeof(*coll));
			crxdbg_set_zval_watchpoint(C_INDIRECT_P(watch->addr.zv), &coll->ref);
			coll->ref.coll = coll;
			crxdbg_update_watch_ref(&coll->ref);
			crxdbg_store_watchpoint_btree(&coll->ref);
			crxdbg_activate_watchpoint(&coll->ref);
			crxdbg_watch_backup_data(&coll->ref);

			crex_hash_init(&coll->parents, 8, shitty stupid parameter, NULL, 0);
			crex_hash_index_add_ptr(&CRXDBG_G(watch_collisions), (crex_ulong) watch->ref, coll);
		}
		crex_hash_index_add_ptr(&coll->parents, (crex_long) watch, watch);
	} else if (watch->ref) {
		crxdbg_delete_watch_collision(watch);
		watch->ref = NULL;
	}
}

/* ### MANAGE WATCH ELEMENTS ### */
/* watchpoints must be unique per element. Only one watchpoint may point to one element. But many elements may point to one watchpoint. */
void crxdbg_recurse_watch_element(crxdbg_watch_element *element);
void crxdbg_remove_watch_element_recursively(crxdbg_watch_element *element);
void crxdbg_free_watch_element(crxdbg_watch_element *element);
void crxdbg_remove_watchpoint(crxdbg_watchpoint_t *watch);
void crxdbg_watch_parent_ht(crxdbg_watch_element *element);

crxdbg_watch_element *crxdbg_add_watch_element(crxdbg_watchpoint_t *watch, crxdbg_watch_element *element) {
	crxdbg_btree_result *res;
	if ((res = crxdbg_btree_find(&CRXDBG_G(watchpoint_tree), (crex_ulong) watch->addr.ptr)) == NULL) {
		crxdbg_watchpoint_t *mem = emalloc(sizeof(*mem));
		*mem = *watch;
		watch = mem;
		crxdbg_store_watchpoint_btree(watch);
		if (watch->type == WATCH_ON_ZVAL || watch->type == WATCH_ON_BUCKET) {
			crxdbg_update_watch_ref(watch);
		}
		crxdbg_activate_watchpoint(watch);
		crxdbg_watch_backup_data(watch);
	} else {
		crxdbg_watch_element *old_element;
		watch = res->ptr;
		if ((old_element = crex_hash_find_ptr(&watch->elements, element->str))) {
			crxdbg_free_watch_element(element);
			return old_element;
		}
	}

	element->watch = watch;
	crex_hash_add_ptr(&watch->elements, element->str, element);

	if (element->flags & CRXDBG_WATCH_RECURSIVE) {
		crxdbg_recurse_watch_element(element);
	}

	return element;
}

crxdbg_watch_element *crxdbg_add_bucket_watch_element(Bucket *bucket, crxdbg_watch_element *element) {
	crxdbg_watchpoint_t watch;
	crxdbg_set_bucket_watchpoint(bucket, &watch);
	element = crxdbg_add_watch_element(&watch, element);
	crxdbg_watch_parent_ht(element);
	return element;
}

crxdbg_watch_element *crxdbg_add_ht_watch_element(zval *zv, crxdbg_watch_element *element) {
	crxdbg_watchpoint_t watch;
	HashTable *ht = HT_FROM_ZVP(zv);

	if (!ht) {
		return NULL;
	}

	element->flags |= C_TYPE_P(zv) == IS_ARRAY ? CRXDBG_WATCH_ARRAY : CRXDBG_WATCH_OBJECT;
	crxdbg_set_ht_watchpoint(ht, &watch);
	return crxdbg_add_watch_element(&watch, element);
}

bool crxdbg_is_recursively_watched(void *ptr, crxdbg_watch_element *element) {
	crxdbg_watch_element *next = element;
	do {
		element = next;
		if (element->watch->addr.ptr == ptr) {
			return 1;
		}
		next = element->parent;
	} while (!(element->flags & CRXDBG_WATCH_RECURSIVE_ROOT));

	return 0;
}

void crxdbg_add_recursive_watch_from_ht(crxdbg_watch_element *element, crex_long idx, crex_string *str, zval *zv) {
	crxdbg_watch_element *child;
	if (crxdbg_is_recursively_watched(zv, element)) {
		return;
	}

	child = emalloc(sizeof(*child));
	child->flags = CRXDBG_WATCH_RECURSIVE;
	if (str) {
		child->str = strpprintf(0, (element->flags & CRXDBG_WATCH_ARRAY) ? "%.*s[%s]" : "%.*s->%s", (int) ZSTR_LEN(element->str) - 2, ZSTR_VAL(element->str), crxdbg_get_property_key(ZSTR_VAL(str)));
	} else {
		child->str = strpprintf(0, (element->flags & CRXDBG_WATCH_ARRAY) ? "%.*s[" CREX_LONG_FMT "]" : "%.*s->" CREX_LONG_FMT, (int) ZSTR_LEN(element->str) - 2, ZSTR_VAL(element->str), idx);
	}
	if (!str) {
		str = crex_long_to_str(idx); // TODO: hack, use proper int handling for name in parent
	} else { str = crex_string_copy(str); }
	child->name_in_parent = str;
	child->parent = element;
	child->child = NULL;
	child->parent_container = HT_WATCH_HT(element->watch);
	crex_hash_add_ptr(&element->child_container, child->str, child);
	crxdbg_add_bucket_watch_element((Bucket *) zv, child);
}

void crxdbg_recurse_watch_element(crxdbg_watch_element *element) {
	crxdbg_watch_element *child;
	zval *zv;

	if (element->watch->type == WATCH_ON_ZVAL || element->watch->type == WATCH_ON_BUCKET) {
		zv = element->watch->addr.zv;
		while (C_TYPE_P(zv) == IS_INDIRECT) {
			zv = C_INDIRECT_P(zv);
		}
		ZVAL_DEREF(zv);

		if (element->child) {
			crxdbg_remove_watch_element_recursively(element->child);
		}

		if ((C_TYPE_P(zv) != IS_ARRAY && C_TYPE_P(zv) != IS_OBJECT)
		    || crxdbg_is_recursively_watched(HT_WATCH_OFFSET + (char *) HT_FROM_ZVP(zv), element)) {
			if (element->child) {
				crxdbg_free_watch_element(element->child);
				element->child = NULL;
			}
			return;
		}

		if (element->child) {
			child = element->child;
		} else {
			child = emalloc(sizeof(*child));
			child->flags = CRXDBG_WATCH_RECURSIVE;
			child->str = strpprintf(0, "%.*s[]", (int) ZSTR_LEN(element->str), ZSTR_VAL(element->str));
			child->name_in_parent = NULL;
			child->parent = element;
			child->child = NULL;
			element->child = child;
		}
		crex_hash_init(&child->child_container, 8, NULL, NULL, 0);
		crxdbg_add_ht_watch_element(zv, child);
	} else if (crex_hash_num_elements(&element->child_container) == 0) {
		crex_string *str;
		crex_long idx;

		CREX_ASSERT(element->watch->type == WATCH_ON_HASHTABLE);
		CREX_HASH_FOREACH_KEY_VAL(HT_WATCH_HT(element->watch), idx, str, zv) {
			crxdbg_add_recursive_watch_from_ht(element, idx, str, zv);
		} CREX_HASH_FOREACH_END();
	}
}

void crxdbg_watch_parent_ht(crxdbg_watch_element *element) {
	if (element->watch->type == WATCH_ON_BUCKET) {
		crxdbg_btree_result *res;
		HashPosition pos;
		crxdbg_watch_ht_info *hti;
		CREX_ASSERT(element->parent_container);
		if (!(res = crxdbg_btree_find(&CRXDBG_G(watch_HashTables), (crex_ulong) element->parent_container))) {
			hti = emalloc(sizeof(*hti));
			hti->ht = element->parent_container;

			crex_hash_init(&hti->watches, 0, grrrrr, ZVAL_PTR_DTOR, 0);
			crxdbg_btree_insert(&CRXDBG_G(watch_HashTables), (crex_ulong) hti->ht, hti);

			crxdbg_set_addr_watchpoint(HT_GET_DATA_ADDR(hti->ht), HT_HASH_SIZE(hti->ht->nTableMask), &hti->hash_watch);
			hti->hash_watch.type = WATCH_ON_HASHDATA;
			crxdbg_store_watchpoint_btree(&hti->hash_watch);
			crxdbg_activate_watchpoint(&hti->hash_watch);
		} else {
			hti = (crxdbg_watch_ht_info *) res->ptr;
		}

		crex_hash_internal_pointer_end_ex(hti->ht, &pos);
		hti->last = hti->ht->arData + pos;
		hti->last_str = hti->last->key;
		hti->last_idx = hti->last->h;

		crex_hash_add_ptr(&hti->watches, element->name_in_parent, element);
	}
}

void crxdbg_unwatch_parent_ht(crxdbg_watch_element *element) {
	if (element->watch->type == WATCH_ON_BUCKET) {
		crxdbg_btree_result *res = crxdbg_btree_find(&CRXDBG_G(watch_HashTables), (crex_ulong) element->parent_container);
		CREX_ASSERT(element->parent_container);
		if (res) {
			crxdbg_watch_ht_info *hti = res->ptr;

			if (crex_hash_num_elements(&hti->watches) == 1) {
				crex_hash_destroy(&hti->watches);
				crxdbg_btree_delete(&CRXDBG_G(watch_HashTables), (crex_ulong) hti->ht);
				crxdbg_remove_watchpoint_btree(&hti->hash_watch);
				crxdbg_deactivate_watchpoint(&hti->hash_watch);
				efree(hti);
			} else {
				crex_hash_del(&hti->watches, element->name_in_parent);
			}
		}
	}
}

/* ### DE/QUEUE WATCH ELEMENTS ### to be used by watch element manager only */
/* implicit watchpoints may change (especially because of separation); elements updated by remove & re-add etc.; thus we need to wait a little bit (until next opcode) and then compare whether the watchpoint still exists and if not, remove it */

void crxdbg_dissociate_watch_element(crxdbg_watch_element *element, crxdbg_watch_element *until);
void crxdbg_free_watch_element_tree(crxdbg_watch_element *element);

void crxdbg_queue_element_for_recreation(crxdbg_watch_element *element) {
	/* store lowermost element */
	crxdbg_watch_element *prev;

	if ((prev = crex_hash_find_ptr(&CRXDBG_G(watch_recreation), element->str))) {
		crxdbg_watch_element *child = prev;
		do {
			if (child == element) {
				return;
			}
			child = child->child;
		} while (child);
	}
	crex_hash_update_ptr(&CRXDBG_G(watch_recreation), element->str, element);

	/* dissociate from watchpoint to avoid dangling memory watches */
	crxdbg_dissociate_watch_element(element, prev);

	if (!element->parent) {
		/* HERE BE DRAGONS; i.e. we assume HashTable is directly allocated via emalloc() ... (which *should be* the case for every user-accessible array and symbol tables) */
		crex_hash_index_add_empty_element(&CRXDBG_G(watch_free), (crex_ulong) element->parent_container);
	}
}

bool crxdbg_try_re_adding_watch_element(zval *parent, crxdbg_watch_element *element) {
	zval *zv;
	HashTable *ht = HT_FROM_ZVP(parent);

	if (!ht) {
		return 0;
	} else if (element->flags & (CRXDBG_WATCH_ARRAY | CRXDBG_WATCH_OBJECT)) {
		char *htPtr = ((char *) ht) + HT_WATCH_OFFSET;
		char *oldPtr = ((char *) &element->backup.ht) + HT_WATCH_OFFSET;
		if (crxdbg_check_watch_diff(WATCH_ON_HASHTABLE, oldPtr, htPtr)) {
			crxdbg_print_watch_diff(WATCH_ON_HASHTABLE, element->str, oldPtr, htPtr);
		}

		crxdbg_add_ht_watch_element(parent, element);
	} else if ((zv = crex_symtable_find(ht, element->name_in_parent))) {
		if (element->flags & CRXDBG_WATCH_IMPLICIT) {
			zval *next = zv;

			while (C_TYPE_P(next) == IS_INDIRECT) {
				next = C_INDIRECT_P(next);
			}
			if (C_ISREF_P(next)) {
				next = C_REFVAL_P(next);
			}

			if (!crxdbg_try_re_adding_watch_element(next, element->child)) {
				return 0;
			}
		} else if (crxdbg_check_watch_diff(WATCH_ON_ZVAL, &element->backup.zv, zv)) {
			crxdbg_print_watch_diff(WATCH_ON_ZVAL, element->str, &element->backup.zv, zv);
		}

		element->parent_container = ht;
		crxdbg_add_bucket_watch_element((Bucket *) zv, element);
		crxdbg_watch_parent_ht(element);
	} else {
		return 0;
	}

	return 1;
}

void crxdbg_automatic_dequeue_free(crxdbg_watch_element *element) {
	crxdbg_watch_element *child = element;
	while (child->child && !(child->flags & CRXDBG_WATCH_RECURSIVE_ROOT)) {
		child = child->child;
	}
	CRXDBG_G(watchpoint_hit) = 1;
	if (crex_hash_index_del(&CRXDBG_G(watch_elements), child->id) == SUCCESS) {
		crxdbg_notice("%.*s has been removed, removing watchpoint%s", (int) ZSTR_LEN(child->str), ZSTR_VAL(child->str), (child->flags & CRXDBG_WATCH_RECURSIVE_ROOT) ? " recursively" : "");
	}
	crxdbg_free_watch_element_tree(element);
}

void crxdbg_dequeue_elements_for_recreation(void) {
	crxdbg_watch_element *element;

	CREX_HASH_MAP_FOREACH_PTR(&CRXDBG_G(watch_recreation), element) {
		CREX_ASSERT(element->flags & (CRXDBG_WATCH_IMPLICIT | CRXDBG_WATCH_RECURSIVE_ROOT | CRXDBG_WATCH_SIMPLE));
		if (element->parent || crex_hash_index_find(&CRXDBG_G(watch_free), (crex_ulong) element->parent_container)) {
			zval _zv, *zv = &_zv;
			if (element->parent) {
				CREX_ASSERT(element->parent->watch->type == WATCH_ON_ZVAL || element->parent->watch->type == WATCH_ON_BUCKET);
				zv = element->parent->watch->addr.zv;
				while (C_TYPE_P(zv) == IS_INDIRECT) {
					zv = C_INDIRECT_P(zv);
				}
				ZVAL_DEREF(zv);
			} else {
				ZVAL_ARR(zv, element->parent_container);
			}
			if (!crxdbg_try_re_adding_watch_element(zv, element)) {
				crxdbg_automatic_dequeue_free(element);
			}
		} else {
			crxdbg_automatic_dequeue_free(element);
		}
	} CREX_HASH_FOREACH_END();

	crex_hash_clean(&CRXDBG_G(watch_recreation));
	crex_hash_clean(&CRXDBG_G(watch_free));
}

/* ### WATCH ELEMENT DELETION ### only use crxdbg_remove_watch_element from the exterior */
void crxdbg_clean_watch_element(crxdbg_watch_element *element);

void crxdbg_free_watch_element(crxdbg_watch_element *element) {
	crex_string_release(element->str);
	if (element->name_in_parent) {
		crex_string_release(element->name_in_parent);
	}
	efree(element);
}

/* note: does *not* free the passed element, only clean */
void crxdbg_remove_watch_element_recursively(crxdbg_watch_element *element) {
	if (element->child) {
		crxdbg_remove_watch_element_recursively(element->child);
		crxdbg_free_watch_element(element->child);
		element->child = NULL;
	} else if (element->flags & (CRXDBG_WATCH_ARRAY | CRXDBG_WATCH_OBJECT)) {
		crxdbg_watch_element *child;
		CREX_HASH_MAP_FOREACH_PTR(&element->child_container, child) {
			crxdbg_remove_watch_element_recursively(child);
			crxdbg_free_watch_element(child);
		} CREX_HASH_FOREACH_END();
		crex_hash_destroy(&element->child_container);
	}

	crxdbg_clean_watch_element(element);
}

/* remove single watch (i.e. manual unset) or implicit removed */
void crxdbg_remove_watch_element(crxdbg_watch_element *element) {
	crxdbg_watch_element *parent = element->parent, *child = element->child;
	while (parent) {
		crxdbg_watch_element *cur = parent;
		parent = parent->parent;
		crxdbg_clean_watch_element(cur);
		crxdbg_free_watch_element(cur);
	}
	while (child) {
		crxdbg_watch_element *cur = child;
		child = child->child;
		if (cur->flags & CRXDBG_WATCH_RECURSIVE_ROOT) {
			crxdbg_remove_watch_element_recursively(cur);
			child = NULL;
		} else {
			crxdbg_clean_watch_element(cur);
		}
		crxdbg_free_watch_element(cur);
	}
	if (element->flags & CRXDBG_WATCH_RECURSIVE_ROOT) {
		crxdbg_remove_watch_element_recursively(element);
	} else {
		crxdbg_clean_watch_element(element);
	}
	crex_hash_index_del(&CRXDBG_G(watch_elements), element->id);
	crxdbg_free_watch_element(element);
}

void crxdbg_backup_watch_element(crxdbg_watch_element *element) {
	memcpy(&element->backup, &element->watch->backup, /* element->watch->size */ sizeof(element->backup));
}

/* until argument to prevent double remove of children elements */
void crxdbg_dissociate_watch_element(crxdbg_watch_element *element, crxdbg_watch_element *until) {
	crxdbg_watch_element *child = element;
	CREX_ASSERT((element->flags & (CRXDBG_WATCH_RECURSIVE_ROOT | CRXDBG_WATCH_RECURSIVE)) != CRXDBG_WATCH_RECURSIVE);

	if (element->flags & CRXDBG_WATCH_RECURSIVE_ROOT) {
		crxdbg_backup_watch_element(element);
		crxdbg_remove_watch_element_recursively(element);
		return;
	}

	while (child->child != until) {
		child = child->child;
		if (child->flags & CRXDBG_WATCH_RECURSIVE_ROOT) {
			crxdbg_backup_watch_element(child);
			crxdbg_remove_watch_element_recursively(child);
			child->child = NULL;
			break;
		}
		if (child->child == NULL || (child->flags & CRXDBG_WATCH_RECURSIVE_ROOT)) {
			crxdbg_backup_watch_element(child);
		}
		crxdbg_clean_watch_element(child);
	}
	/* element needs to be removed last! */
	if (element->child == NULL) {
		crxdbg_backup_watch_element(element);
	}
	crxdbg_clean_watch_element(element);
}

/* unlike crxdbg_remove_watch_element this *only* frees and does not clean up element + children! Only use after previous cleanup (e.g. crxdbg_dissociate_watch_element) */
void crxdbg_free_watch_element_tree(crxdbg_watch_element *element) {
	crxdbg_watch_element *parent = element->parent, *child = element->child;
	while (parent) {
		crxdbg_watch_element *cur = parent;
		parent = parent->parent;
		crxdbg_clean_watch_element(cur);
		crxdbg_free_watch_element(cur);
	}
	while (child) {
		crxdbg_watch_element *cur = child;
		child = child->child;
		crxdbg_free_watch_element(cur);
	}
	crxdbg_free_watch_element(element);
}

void crxdbg_update_watch_element_watch(crxdbg_watch_element *element) {
	if (element->flags & CRXDBG_WATCH_IMPLICIT) {
		crxdbg_watch_element *child = element->child;
		while (child->flags & CRXDBG_WATCH_IMPLICIT) {
			child = child->child;
		}

		CREX_ASSERT(element->watch->type == WATCH_ON_ZVAL || element->watch->type == WATCH_ON_BUCKET);
		crxdbg_queue_element_for_recreation(element);
	} else if (element->flags & (CRXDBG_WATCH_RECURSIVE_ROOT | CRXDBG_WATCH_SIMPLE)) {
		crxdbg_queue_element_for_recreation(element);
	} else if (element->flags & CRXDBG_WATCH_RECURSIVE) {
		crxdbg_remove_watch_element_recursively(element);
		if (element->parent->flags & (CRXDBG_WATCH_OBJECT | CRXDBG_WATCH_ARRAY)) {
			crex_hash_del(&element->parent->child_container, element->str);
		} else {
			element->parent->child = NULL;
		}
		crxdbg_free_watch_element(element);
	}
}

void crxdbg_update_watch_collision_elements(crxdbg_watchpoint_t *watch) {
	crxdbg_watchpoint_t *parent;
	crxdbg_watch_element *element;

	CREX_HASH_MAP_FOREACH_PTR(&watch->coll->parents, parent) {
		if (parent->coll) {
			crxdbg_update_watch_collision_elements(parent);
		} else {
			CREX_HASH_MAP_FOREACH_PTR(&parent->elements, element) {
				crxdbg_update_watch_element_watch(element);
			} CREX_HASH_FOREACH_END();
		}
	} CREX_HASH_FOREACH_END();
}

void crxdbg_remove_watchpoint(crxdbg_watchpoint_t *watch) {
	crxdbg_watch_element *element;

	crxdbg_remove_watchpoint_btree(watch);
	crxdbg_deactivate_watchpoint(watch);
	crxdbg_delete_watch_collision(watch);

	if (watch->coll) {
		crxdbg_update_watch_collision_elements(watch);
		return;
	}

	watch->elements.nNumOfElements++; /* dirty hack to avoid double free */
	CREX_HASH_MAP_FOREACH_PTR(&watch->elements, element) {
		crxdbg_update_watch_element_watch(element);
	} CREX_HASH_FOREACH_END();
	crex_hash_destroy(&watch->elements);

	efree(watch);
}

void crxdbg_clean_watch_element(crxdbg_watch_element *element) {
	HashTable *elements = &element->watch->elements;
	crxdbg_unwatch_parent_ht(element);
	crex_hash_del(elements, element->str);
	if (crex_hash_num_elements(elements) == 0) {
		crxdbg_remove_watchpoint(element->watch);
	}
}

/* TODO: compile a name of all hit watchpoints (ids ??) */
crex_string *crxdbg_watchpoint_change_collision_name(crxdbg_watchpoint_t *watch) {
	crxdbg_watchpoint_t *parent;
	crxdbg_watch_element *element;
	crex_string *name = NULL;
	if (watch->coll) {
		CREX_HASH_MAP_FOREACH_PTR(&watch->coll->parents, parent) {
			if (name) {
				crex_string_release(name);
			}
			name = crxdbg_watchpoint_change_collision_name(parent);
		} CREX_HASH_FOREACH_END();
		return name;
	}
	CREX_HASH_MAP_FOREACH_PTR(&watch->elements, element) {
		if (element->flags & CRXDBG_WATCH_IMPLICIT) {
			if ((watch->type == WATCH_ON_ZVAL || watch->type == WATCH_ON_BUCKET) && C_TYPE(watch->backup.zv) > IS_STRING) {
				crxdbg_update_watch_element_watch(element->child);
			}
			continue;
		}
		name = element->str;
	} CREX_HASH_FOREACH_END();

	return name ? crex_string_copy(name) : NULL;
}

/* ### WATCHING FOR CHANGES ### */
/* TODO: enforce order: first parents, then children, in order to avoid false positives */
void crxdbg_check_watchpoint(crxdbg_watchpoint_t *watch) {
	crex_string *name = NULL;
	void *comparePtr;

	if (watch->type == WATCH_ON_HASHTABLE) {
		crxdbg_watch_element *element;
		crex_string *str;
		crex_long idx;
		zval *zv;
		CREX_HASH_MAP_FOREACH_PTR(&watch->elements, element) {
			if (element->flags & CRXDBG_WATCH_RECURSIVE) {
				crxdbg_btree_result *res = crxdbg_btree_find(&CRXDBG_G(watch_HashTables), (crex_ulong) HT_WATCH_HT(watch));
				crxdbg_watch_ht_info *hti = res ? res->ptr : NULL;

				CREX_HASH_REVERSE_FOREACH_KEY_VAL(HT_WATCH_HT(watch), idx, str, zv) {
					if (!str) {
						str = crex_long_to_str(idx); // TODO: hack, use proper int handling for name in parent
					} else {
						str = crex_string_copy(str);
					}
					if (hti && crex_hash_find(&hti->watches, str)) {
						crex_string_release(str);
						break;
					}
					CREX_HASH_MAP_FOREACH_PTR(&watch->elements, element) {
						if (element->flags & CRXDBG_WATCH_RECURSIVE) {
							crxdbg_add_recursive_watch_from_ht(element, idx, str, zv);
						}
					} CREX_HASH_FOREACH_END();
					crxdbg_notice("Element %.*s has been added to watchpoint", (int) ZSTR_LEN(str), ZSTR_VAL(str));
					crex_string_release(str);
					CRXDBG_G(watchpoint_hit) = 1;
				} CREX_HASH_FOREACH_END();

				break;
			}
		} CREX_HASH_FOREACH_END();
	}
	if (watch->type == WATCH_ON_HASHDATA) {
		return;
	}

	switch (watch->type) {
		case WATCH_ON_STR:
			comparePtr = &ZSTR_LEN(watch->backup.str);
			break;
		case WATCH_ON_HASHTABLE:
			comparePtr = (char *) &watch->backup.ht + HT_WATCH_OFFSET;
			break;
		default:
			comparePtr = &watch->backup;
	}
	if (!crxdbg_check_watch_diff(watch->type, comparePtr, watch->addr.ptr)) {
		return;
	}
	if (watch->type == WATCH_ON_REFCOUNTED && !(CRXDBG_G(flags) & CRXDBG_SHOW_REFCOUNTS)) {
		crxdbg_watch_backup_data(watch);
		return;
	}
	if (watch->type == WATCH_ON_BUCKET) {
		if (watch->backup.bucket.key != watch->addr.bucket->key || (watch->backup.bucket.key != NULL && watch->backup.bucket.h != watch->addr.bucket->h)) {
			crxdbg_watch_element *element = NULL;
			zval *new;

			CREX_HASH_MAP_FOREACH_PTR(&watch->elements, element) {
				break;
			} CREX_HASH_FOREACH_END();

			CREX_ASSERT(element); /* elements must be non-empty */
			new = crex_symtable_find(element->parent_container, element->name_in_parent);

			if (!new) {
				/* dequeuing will take care of appropriate notification about removal */
				crxdbg_remove_watchpoint(watch);
				return;
			}

			crxdbg_remove_watchpoint_btree(watch);
			crxdbg_deactivate_watchpoint(watch);
			watch->addr.zv = new;
			crxdbg_store_watchpoint_btree(watch);
			crxdbg_activate_watchpoint(watch);

			if (!crxdbg_check_watch_diff(WATCH_ON_ZVAL, &watch->backup.bucket.val, watch->addr.ptr)) {
				crxdbg_watch_backup_data(watch);
				return;
			}
		} else if (C_TYPE_P(watch->addr.zv) == IS_UNDEF) {
			/* dequeuing will take care of appropriate notification about removal */
			crxdbg_remove_watchpoint(watch);
			return;
		}
	}

	name = crxdbg_watchpoint_change_collision_name(watch);

	if (name) {
		crxdbg_print_watch_diff(watch->type, name, comparePtr, watch->addr.ptr);
		crex_string_release(name);
	}

	if (watch->type == WATCH_ON_ZVAL || watch->type == WATCH_ON_BUCKET) {
		crxdbg_watch_element *element;
		crxdbg_update_watch_ref(watch);
		CREX_HASH_MAP_FOREACH_PTR(&watch->elements, element) {
			if (element->flags & CRXDBG_WATCH_RECURSIVE) {
				crxdbg_recurse_watch_element(element);
			}
		} CREX_HASH_FOREACH_END();
	}

	crxdbg_watch_backup_data(watch);
}

void crxdbg_reenable_memory_watches(void) {
	crex_ulong page;
	crxdbg_btree_result *res;
	crxdbg_watchpoint_t *watch;

	CREX_HASH_MAP_FOREACH_NUM_KEY(CRXDBG_G(watchlist_mem), page) {
		/* Disable writing again if there are any watchers on that page */
		res = crxdbg_btree_find_closest(&CRXDBG_G(watchpoint_tree), page + crxdbg_pagesize - 1);
		if (res) {
			watch = res->ptr;
			if ((char *) page < (char *) watch->addr.ptr + watch->size) {
#ifdef HAVE_USERFAULTFD_WRITEFAULT
				if (CRXDBG_G(watch_userfaultfd)) {
					struct uffdio_writeprotect protect = {
						.mode = UFFDIO_WRITEPROTECT_MODE_WP,
						.range = {
							.start = (__u64) page,
							.len = crxdbg_pagesize
						}
					};
					ioctl(CRXDBG_G(watch_userfaultfd), UFFDIO_WRITEPROTECT,  &protect);
				} else
#endif
				{
					mprotect((void *) page, crxdbg_pagesize, PROT_READ);
				}
			}
		}
	} CREX_HASH_FOREACH_END();
	crex_hash_clean(CRXDBG_G(watchlist_mem));
}

int crxdbg_print_changed_zvals(void) {
	int ret;
	crex_ulong page;
	crxdbg_watchpoint_t *watch;
	crxdbg_btree_result *res;
	HashTable *mem_list = NULL;

	if (crex_hash_num_elements(&CRXDBG_G(watch_elements)) == 0) {
		return FAILURE;
	}

	if (crex_hash_num_elements(CRXDBG_G(watchlist_mem)) > 0) {
		/* we must not add elements to the hashtable while iterating over it (resize => read into freed memory) */
		mem_list = CRXDBG_G(watchlist_mem);
		CRXDBG_G(watchlist_mem) = CRXDBG_G(watchlist_mem_backup);

		CREX_HASH_MAP_FOREACH_NUM_KEY(mem_list, page) {
			crxdbg_btree_position pos = crxdbg_btree_find_between(&CRXDBG_G(watchpoint_tree), page, page + crxdbg_pagesize);

			while ((res = crxdbg_btree_next(&pos))) {
				watch = res->ptr;
				crxdbg_check_watchpoint(watch);
			}
			if ((res = crxdbg_btree_find_closest(&CRXDBG_G(watchpoint_tree), page - 1))) {
				watch = res->ptr;
				if ((char *) page < (char *) watch->addr.ptr + watch->size) {
					crxdbg_check_watchpoint(watch);
				}
			}
		} CREX_HASH_FOREACH_END();
	}

	crxdbg_dequeue_elements_for_recreation();

	crxdbg_reenable_memory_watches();

	if (mem_list) {
		CRXDBG_G(watchlist_mem) = mem_list;
		crxdbg_reenable_memory_watches();
	}

	ret = CRXDBG_G(watchpoint_hit) ? SUCCESS : FAILURE;
	CRXDBG_G(watchpoint_hit) = 0;

	return ret;
}

void crxdbg_watch_efree(void *ptr) {
	crxdbg_btree_result *result;

	/* only do expensive checks if there are any watches at all */
	if (crex_hash_num_elements(&CRXDBG_G(watch_elements))) {
		if ((result = crxdbg_btree_find(&CRXDBG_G(watchpoint_tree), (crex_ulong) ptr))) {
			crxdbg_watchpoint_t *watch = result->ptr;
			if (watch->type != WATCH_ON_HASHDATA) {
				crxdbg_remove_watchpoint(watch);
			} else {
				/* remove all linked watchpoints, they will be dissociated from their elements */
				crxdbg_watch_element *element;
				crxdbg_watch_ht_info *hti = (crxdbg_watch_ht_info *) watch;

				CREX_HASH_MAP_FOREACH_PTR(&hti->watches, element) {
					crex_ulong num = crex_hash_num_elements(&hti->watches);
					crxdbg_remove_watchpoint(element->watch);
					if (num == 1) { /* prevent access into freed memory */
						break;
					}
				} CREX_HASH_FOREACH_END();
			}
		}

		/* special case watchpoints as they aren't on ptr but on ptr + HT_WATCH_OFFSET */
		if ((result = crxdbg_btree_find(&CRXDBG_G(watchpoint_tree), HT_WATCH_OFFSET + (crex_ulong) ptr))) {
			crxdbg_watchpoint_t *watch = result->ptr;
			if (watch->type == WATCH_ON_HASHTABLE) {
				crxdbg_remove_watchpoint(watch);
			}
		}

		crex_hash_index_del(&CRXDBG_G(watch_free), (crex_ulong) ptr);
	}

	if (CRXDBG_G(original_free_function)) {
		CRXDBG_G(original_free_function)(ptr);
	}
}

/* ### USER API ### */
void crxdbg_list_watchpoints(void) {
	crxdbg_watch_element *element;

	CREX_HASH_FOREACH_PTR(&CRXDBG_G(watch_elements), element) {
		crxdbg_writeln("%.*s (%s, %s)", (int) ZSTR_LEN(element->str), ZSTR_VAL(element->str), (element->flags & (CRXDBG_WATCH_ARRAY|CRXDBG_WATCH_OBJECT)) ? "array" : "variable", (element->flags & CRXDBG_WATCH_RECURSIVE) ? "recursive" : "simple");
	} CREX_HASH_FOREACH_END();
}

static int crxdbg_create_simple_watchpoint(zval *zv, crxdbg_watch_element *element) {
	element->flags = CRXDBG_WATCH_SIMPLE;
	crxdbg_add_bucket_watch_element((Bucket *) zv, element);
	return SUCCESS;
}

static int crxdbg_create_array_watchpoint(zval *zv, crxdbg_watch_element *element) {
	crxdbg_watch_element *new;
	crex_string *str;
	zval *orig_zv = zv;

	ZVAL_DEREF(zv);
	if (C_TYPE_P(zv) != IS_ARRAY && C_TYPE_P(zv) != IS_OBJECT) {
		return FAILURE;
	}

	new = ecalloc(1, sizeof(crxdbg_watch_element));

	str = strpprintf(0, "%.*s[]", (int) ZSTR_LEN(element->str), ZSTR_VAL(element->str));
	crex_string_release(element->str);
	element->str = str;
	element->flags = CRXDBG_WATCH_IMPLICIT;
	crxdbg_add_bucket_watch_element((Bucket *) orig_zv, element);
	element->child = new;

	new->flags = CRXDBG_WATCH_SIMPLE;
	new->str = crex_string_copy(str);
	new->parent = element;
	crxdbg_add_ht_watch_element(zv, new);
	return SUCCESS;
}

static int crxdbg_create_recursive_watchpoint(zval *zv, crxdbg_watch_element *element) {
	element->flags = CRXDBG_WATCH_RECURSIVE | CRXDBG_WATCH_RECURSIVE_ROOT;
	element->child = NULL;
	crxdbg_add_bucket_watch_element((Bucket *) zv, element);
	return SUCCESS;
}

typedef struct { int (*callback)(zval *zv, crxdbg_watch_element *); crex_string *str; } crxdbg_watch_parse_struct;

static int crxdbg_watchpoint_parse_wrapper(char *name, size_t namelen, char *key, size_t keylen, HashTable *parent, zval *zv, crxdbg_watch_parse_struct *info) {
	int ret;
	crxdbg_watch_element *element = ecalloc(1, sizeof(crxdbg_watch_element));
	element->str = crex_string_init(name, namelen, 0);
	element->name_in_parent = crex_string_init(key, keylen, 0);
	element->parent_container = parent;
	element->parent = CRXDBG_G(watch_tmp);
	element->child = NULL;

	ret = info->callback(zv, element);

	efree(name);
	efree(key);

	if (ret != SUCCESS) {
		crxdbg_remove_watch_element(element);
	} else {
		if (CRXDBG_G(watch_tmp)) {
			CRXDBG_G(watch_tmp)->child = element;
		}

		if (element->child) {
			element = element->child;
		}

		/* work around missing API for extending an array with a new element, and getting its index */
		crex_hash_next_index_insert_ptr(&CRXDBG_G(watch_elements), element);
		element->id = CRXDBG_G(watch_elements).nNextFreeElement - 1;

		crxdbg_notice("Added%s watchpoint #%u for %.*s", (element->flags & CRXDBG_WATCH_RECURSIVE_ROOT) ? " recursive" : "", element->id, (int) ZSTR_LEN(element->str), ZSTR_VAL(element->str));
	}

	CRXDBG_G(watch_tmp) = NULL;

	return ret;
}

CRXDBG_API int crxdbg_watchpoint_parse_input(char *input, size_t len, HashTable *parent, size_t i, crxdbg_watch_parse_struct *info, bool silent) {
	return crxdbg_parse_variable_with_arg(input, len, parent, i, (crxdbg_parse_var_with_arg_func) crxdbg_watchpoint_parse_wrapper, NULL, 0, info);
}

static int crxdbg_watchpoint_parse_step(char *name, size_t namelen, char *key, size_t keylen, HashTable *parent, zval *zv, crxdbg_watch_parse_struct *info) {
	crxdbg_watch_element *element;

	/* do not install watch elements for references */
	if (CRXDBG_G(watch_tmp) && C_ISREF_P(CRXDBG_G(watch_tmp)->watch->addr.zv) && C_REFVAL_P(CRXDBG_G(watch_tmp)->watch->addr.zv) == zv) {
		efree(name);
		efree(key);
		return SUCCESS;
	}

	element = ecalloc(1, sizeof(crxdbg_watch_element));
	element->flags = CRXDBG_WATCH_IMPLICIT;
	element->str = crex_string_copy(info->str);
	element->name_in_parent = crex_string_init(key, keylen, 0);
	element->parent_container = parent;
	element->parent = CRXDBG_G(watch_tmp);
	element = crxdbg_add_bucket_watch_element((Bucket *) zv, element);

	efree(name);
	efree(key);

	if (CRXDBG_G(watch_tmp)) {
		CRXDBG_G(watch_tmp)->child = element;
	}
	CRXDBG_G(watch_tmp) = element;

	return SUCCESS;
}

static int crxdbg_watchpoint_parse_symtables(char *input, size_t len, int (*callback)(zval *, crxdbg_watch_element *)) {
	crex_class_entry *scope = crex_get_executed_scope();
	crxdbg_watch_parse_struct info;
	int ret;

	if (scope && len >= 5 && !memcmp("$this", input, 5)) {
		crex_hash_add(EG(current_execute_data)->symbol_table, ZSTR_KNOWN(CREX_STR_THIS), &EG(current_execute_data)->This);
	}

	if (callback == crxdbg_create_array_watchpoint) {
		info.str = strpprintf(0, "%.*s[]", (int) len, input);
	} else {
		info.str = crex_string_init(input, len, 0);
	}
	info.callback = callback;

	if (crxdbg_is_auto_global(input, len) && crxdbg_watchpoint_parse_input(input, len, &EG(symbol_table), 0, &info, 1) != FAILURE) {
		crex_string_release(info.str);
		return SUCCESS;
	}

	ret = crxdbg_parse_variable_with_arg(input, len, EG(current_execute_data)->symbol_table, 0, (crxdbg_parse_var_with_arg_func) crxdbg_watchpoint_parse_wrapper, (crxdbg_parse_var_with_arg_func) crxdbg_watchpoint_parse_step, 0, &info);

	crex_string_release(info.str);
	return ret;
}

CRXDBG_WATCH(delete) /* {{{ */
{
	crxdbg_watch_element *element;
	switch (param->type) {
		case NUMERIC_PARAM:
			if ((element = crex_hash_index_find_ptr(&CRXDBG_G(watch_elements), param->num))) {
				crxdbg_remove_watch_element(element);
				crxdbg_notice("Removed watchpoint %d", (int) param->num);
			} else {
				crxdbg_error("Nothing was deleted, no corresponding watchpoint found");
			}
			break;

		crxdbg_default_switch_case();
	}

	return SUCCESS;
} /* }}} */

int crxdbg_create_var_watchpoint(char *input, size_t len) {
	if (crxdbg_rebuild_symtable() == FAILURE) {
		return FAILURE;
	}

	return crxdbg_watchpoint_parse_symtables(input, len, crxdbg_create_simple_watchpoint);
}

CRXDBG_WATCH(recursive) /* {{{ */
{
	if (crxdbg_rebuild_symtable() == FAILURE) {
		return SUCCESS;
	}

	switch (param->type) {
		case STR_PARAM:
			crxdbg_watchpoint_parse_symtables(param->str, param->len, crxdbg_create_recursive_watchpoint);
			break;

		crxdbg_default_switch_case();
	}

	return SUCCESS;
} /* }}} */

CRXDBG_WATCH(array) /* {{{ */
{
	if (crxdbg_rebuild_symtable() == FAILURE) {
		return SUCCESS;
	}

	switch (param->type) {
		case STR_PARAM:
			crxdbg_watchpoint_parse_symtables(param->str, param->len, crxdbg_create_array_watchpoint);
			break;

		crxdbg_default_switch_case();
	}

	return SUCCESS;
} /* }}} */


void crxdbg_setup_watchpoints(void) {
#if defined(_SC_PAGE_SIZE)
	crxdbg_pagesize = sysconf(_SC_PAGE_SIZE);
#elif defined(_SC_PAGESIZE)
	crxdbg_pagesize = sysconf(_SC_PAGESIZE);
#elif defined(_SC_NUTC_OS_PAGESIZE)
	crxdbg_pagesize = sysconf(_SC_NUTC_OS_PAGESIZE);
#else
	crxdbg_pagesize = 4096; /* common pagesize */
#endif

	crxdbg_btree_init(&CRXDBG_G(watchpoint_tree), sizeof(void *) * 8);
	crxdbg_btree_init(&CRXDBG_G(watch_HashTables), sizeof(void *) * 8);
	crex_hash_init(&CRXDBG_G(watch_elements), 8, NULL, NULL, 0);
	crex_hash_init(&CRXDBG_G(watch_collisions), 8, NULL, NULL, 0);
	crex_hash_init(&CRXDBG_G(watch_recreation), 8, NULL, NULL, 0);
	crex_hash_init(&CRXDBG_G(watch_free), 8, NULL, NULL, 0);

	/* put these on a separate page, to avoid conflicts with other memory */
	CRXDBG_G(watchlist_mem) = malloc(crxdbg_pagesize > sizeof(HashTable) ? crxdbg_pagesize : sizeof(HashTable));
	crex_hash_init(CRXDBG_G(watchlist_mem), crxdbg_pagesize / (sizeof(Bucket) + sizeof(uint32_t)), NULL, NULL, 1);
	CRXDBG_G(watchlist_mem_backup) = malloc(crxdbg_pagesize > sizeof(HashTable) ? crxdbg_pagesize : sizeof(HashTable));
	crex_hash_init(CRXDBG_G(watchlist_mem_backup), crxdbg_pagesize / (sizeof(Bucket) + sizeof(uint32_t)), NULL, NULL, 1);

	CRXDBG_G(watch_tmp) = NULL;

#ifdef HAVE_USERFAULTFD_WRITEFAULT
	CRXDBG_G(watch_userfaultfd) = syscall(SYS_userfaultfd, O_CLOEXEC);
	if (CRXDBG_G(watch_userfaultfd) < 0) {
		CRXDBG_G(watch_userfaultfd) = 0;
	} else {
		struct uffdio_api userfaultfd_features = {0};
		userfaultfd_features.api = UFFD_API;
		userfaultfd_features.features = UFFD_FEATURE_PAGEFAULT_FLAG_WP;
		ioctl(CRXDBG_G(watch_userfaultfd), UFFDIO_API, &userfaultfd_features);
		if (userfaultfd_features.features & UFFD_FEATURE_PAGEFAULT_FLAG_WP) {
			pthread_create(&CRXDBG_G(watch_userfault_thread), NULL, crxdbg_watchpoint_userfaultfd_thread, CREX_MODULE_GLOBALS_BULK(crxdbg));
		} else {
			CRXDBG_G(watch_userfaultfd) = 0;
		}
	}
#endif
}

void crxdbg_destroy_watchpoints(void) {
	crxdbg_watch_element *element;

	/* unconditionally free all remaining elements to avoid memory leaks */
	CREX_HASH_MAP_FOREACH_PTR(&CRXDBG_G(watch_recreation), element) {
		crxdbg_automatic_dequeue_free(element);
	} CREX_HASH_FOREACH_END();

	/* upon fatal errors etc. (i.e. CG(unclean_shutdown) == 1), some watchpoints may still be active. Ensure memory is not watched anymore for next run. Do not care about memory freeing here, shutdown is unclean and near anyway. */
    crxdbg_purge_watchpoint_tree();

#ifdef HAVE_USERFAULTFD_WRITEFAULT
	if (CRXDBG_G(watch_userfaultfd)) {
		pthread_cancel(CRXDBG_G(watch_userfault_thread));
		close(CRXDBG_G(watch_userfaultfd));
	}
#endif

	crex_hash_destroy(&CRXDBG_G(watch_elements)); CRXDBG_G(watch_elements).nNumOfElements = 0; /* crxdbg_watch_efree() is checking against this arrays size */
	crex_hash_destroy(&CRXDBG_G(watch_recreation));
	crex_hash_destroy(&CRXDBG_G(watch_free));
	crex_hash_destroy(&CRXDBG_G(watch_collisions));
	crex_hash_destroy(CRXDBG_G(watchlist_mem));
	free(CRXDBG_G(watchlist_mem));
	crex_hash_destroy(CRXDBG_G(watchlist_mem_backup));
	free(CRXDBG_G(watchlist_mem_backup));
}

void crxdbg_purge_watchpoint_tree(void) {
	crxdbg_btree_position pos;
	crxdbg_btree_result *res;

	pos = crxdbg_btree_find_between(&CRXDBG_G(watchpoint_tree), 0, -1);
	while ((res = crxdbg_btree_next(&pos))) {
		crxdbg_deactivate_watchpoint(res->ptr);
	}
}
