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

#ifndef CREX_ACCELERATOR_HASH_H
#define CREX_ACCELERATOR_HASH_H

#include "crex.h"

/*
	crex_accel_hash - is a hash table allocated in shared memory and
	distributed across simultaneously running processes. The hash tables have
	fixed sizen selected during construction by crex_accel_hash_init(). All the
	hash entries are preallocated in the 'hash_entries' array. 'num_entries' is
	initialized by zero and grows when new data is added.
	crex_accel_hash_update() just takes the next entry from 'hash_entries'
	array and puts it into appropriate place of 'hash_table'.
	Hash collisions are resolved by separate chaining with linked lists,
	however, entries are still taken from the same 'hash_entries' array.
	'key' and 'data' passed to crex_accel_hash_update() must be already
	allocated in shared memory. Few keys may be resolved to the same data.
	using 'indirect' entries, that point to other entries ('data' is actually
	a pointer to another crex_accel_hash_entry).
	crex_accel_hash_update() requires exclusive lock, however,
	crex_accel_hash_find() does not.
*/

typedef struct _crex_accel_hash_entry crex_accel_hash_entry;

struct _crex_accel_hash_entry {
	crex_ulong             hash_value;
	crex_string           *key;
	crex_accel_hash_entry *next;
	void                  *data;
	bool                   indirect;
};

typedef struct _crex_accel_hash {
	crex_accel_hash_entry **hash_table;
	crex_accel_hash_entry  *hash_entries;
	uint32_t               num_entries;
	uint32_t               max_num_entries;
	uint32_t               num_direct_entries;
} crex_accel_hash;

BEGIN_EXTERN_C()

void crex_accel_hash_init(crex_accel_hash *accel_hash, uint32_t hash_size);
void crex_accel_hash_clean(crex_accel_hash *accel_hash);

crex_accel_hash_entry* crex_accel_hash_update(
		crex_accel_hash        *accel_hash,
		crex_string            *key,
		bool                   indirect,
		void                   *data);

void* crex_accel_hash_find(
		crex_accel_hash        *accel_hash,
		crex_string            *key);

crex_accel_hash_entry* crex_accel_hash_find_entry(
		crex_accel_hash        *accel_hash,
		crex_string            *key);

int crex_accel_hash_unlink(
		crex_accel_hash        *accel_hash,
		crex_string            *key);

static inline bool crex_accel_hash_is_full(crex_accel_hash *accel_hash)
{
	if (accel_hash->num_entries == accel_hash->max_num_entries) {
		return 1;
	} else {
		return 0;
	}
}

END_EXTERN_C()

#endif /* CREX_ACCELERATOR_HASH_H */
