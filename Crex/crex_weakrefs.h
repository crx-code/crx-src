/*
   +----------------------------------------------------------------------+
   | Copyright (c) The CRX Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: krakjoe@crx.net                                             |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_WEAKREFS_H
#define CREX_WEAKREFS_H

#include "crex_alloc.h"

BEGIN_EXTERN_C()

extern CREX_API crex_class_entry *crex_ce_weakref;

void crex_register_weakref_ce(void);

void crex_weakrefs_init(void);
void crex_weakrefs_shutdown(void);

CREX_API void crex_weakrefs_notify(crex_object *object);

CREX_API zval *crex_weakrefs_hash_add(HashTable *ht, crex_object *key, zval *pData);
CREX_API crex_result crex_weakrefs_hash_del(HashTable *ht, crex_object *key);
static crex_always_inline void *crex_weakrefs_hash_add_ptr(HashTable *ht, crex_object *key, void *ptr) {
	zval tmp, *zv;
	ZVAL_PTR(&tmp, ptr);
	if ((zv = crex_weakrefs_hash_add(ht, key, &tmp))) {
		return C_PTR_P(zv);
	} else {
		return NULL;
	}
}

/* Because crx uses the raw numbers as a hash function, raw pointers will lead to hash collisions.
 * We have a guarantee that the lowest CREX_MM_ALIGNED_OFFSET_LOG2 bits of a pointer are zero.
 *
 * E.g. On most 64-bit platforms, pointers are aligned to 8 bytes, so the least significant 3 bits are always 0 and can be discarded.
 *
 * NOTE: This function is only used for EG(weakrefs) and crex_weakmap->ht.
 * It is not used for the HashTable instances associated with CREX_WEAKREF_TAG_HT tags (created in crex_weakref_register, which uses CREX_WEAKREF_ENCODE instead).
 * The CREX_WEAKREF_TAG_HT instances are used to disambiguate between multiple weak references to the same crex_object.
 */
static crex_always_inline crex_ulong crex_object_to_weakref_key(const crex_object *object)
{
	CREX_ASSERT(((uintptr_t)object) % CREX_MM_ALIGNMENT == 0);
	return ((uintptr_t) object) >> CREX_MM_ALIGNMENT_LOG2;
}

static crex_always_inline crex_object *crex_weakref_key_to_object(crex_ulong key)
{
	return (crex_object *) (((uintptr_t) key) << CREX_MM_ALIGNMENT_LOG2);
}

HashTable *crex_weakmap_get_gc(crex_object *object, zval **table, int *n);
HashTable *crex_weakmap_get_key_entry_gc(crex_object *object, zval **table, int *n);
HashTable *crex_weakmap_get_entry_gc(crex_object *object, zval **table, int *n);
HashTable *crex_weakmap_get_object_key_entry_gc(crex_object *object, zval **table, int *n);
HashTable *crex_weakmap_get_object_entry_gc(crex_object *object, zval **table, int *n);

END_EXTERN_C()

#endif

