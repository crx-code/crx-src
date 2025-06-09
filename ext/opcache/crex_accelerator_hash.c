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

#include "CrexAccelerator.h"
#include "crex_accelerator_hash.h"
#include "crex_hash.h"
#include "crex_shared_alloc.h"

/* Generated on an Octa-ALPHA 300MHz CPU & 2.5GB RAM monster */
static const uint32_t prime_numbers[] =
	{5, 11, 19, 53, 107, 223, 463, 983, 1979, 3907, 7963, 16229, 32531, 65407, 130987, 262237, 524521, 1048793 };
static const uint32_t num_prime_numbers = sizeof(prime_numbers) / sizeof(uint32_t);

void crex_accel_hash_clean(crex_accel_hash *accel_hash)
{
	accel_hash->num_entries = 0;
	accel_hash->num_direct_entries = 0;
	memset(accel_hash->hash_table, 0, sizeof(crex_accel_hash_entry *)*accel_hash->max_num_entries);
}

void crex_accel_hash_init(crex_accel_hash *accel_hash, uint32_t hash_size)
{
	uint32_t i;

	for (i=0; i<num_prime_numbers; i++) {
		if (hash_size <= prime_numbers[i]) {
			hash_size = prime_numbers[i];
			break;
		}
	}

	accel_hash->num_entries = 0;
	accel_hash->num_direct_entries = 0;
	accel_hash->max_num_entries = hash_size;

	/* set up hash pointers table */
	accel_hash->hash_table = crex_shared_alloc(sizeof(crex_accel_hash_entry *)*accel_hash->max_num_entries);
	if (!accel_hash->hash_table) {
		crex_accel_error_noreturn(ACCEL_LOG_FATAL, "Insufficient shared memory!");
		return;
	}

	/* set up hash values table */
	accel_hash->hash_entries = crex_shared_alloc(sizeof(crex_accel_hash_entry)*accel_hash->max_num_entries);
	if (!accel_hash->hash_entries) {
		crex_accel_error_noreturn(ACCEL_LOG_FATAL, "Insufficient shared memory!");
		return;
	}
	memset(accel_hash->hash_table, 0, sizeof(crex_accel_hash_entry *)*accel_hash->max_num_entries);
}

/* Returns NULL if hash is full
 * Returns pointer the actual hash entry on success
 * key needs to be already allocated as it is not copied
 */
crex_accel_hash_entry* crex_accel_hash_update(crex_accel_hash *accel_hash, crex_string *key, bool indirect, void *data)
{
	crex_ulong hash_value;
	crex_ulong index;
	crex_accel_hash_entry *entry;
	crex_accel_hash_entry *indirect_bucket = NULL;

	if (indirect) {
		indirect_bucket = (crex_accel_hash_entry*)data;
		while (indirect_bucket->indirect) {
			indirect_bucket = (crex_accel_hash_entry*)indirect_bucket->data;
		}
	}

	hash_value = crex_string_hash_val(key);
#ifndef CREX_WIN32
	hash_value ^= ZCG(root_hash);
#endif
	index = hash_value % accel_hash->max_num_entries;

	/* try to see if the element already exists in the hash */
	entry = accel_hash->hash_table[index];
	while (entry) {
		if (entry->hash_value == hash_value
		 && crex_string_equals(entry->key, key)) {

			if (entry->indirect) {
				if (indirect_bucket) {
					entry->data = indirect_bucket;
				} else {
					((crex_accel_hash_entry*)entry->data)->data = data;
				}
			} else {
				if (indirect_bucket) {
					accel_hash->num_direct_entries--;
					entry->data = indirect_bucket;
					entry->indirect = 1;
				} else {
					entry->data = data;
				}
			}
			return entry;
		}
		entry = entry->next;
	}

	/* Does not exist, add a new entry */
	if (accel_hash->num_entries == accel_hash->max_num_entries) {
		return NULL;
	}

	entry = &accel_hash->hash_entries[accel_hash->num_entries++];
	if (indirect) {
		entry->data = indirect_bucket;
		entry->indirect = 1;
	} else {
		accel_hash->num_direct_entries++;
		entry->data = data;
		entry->indirect = 0;
	}
	entry->hash_value = hash_value;
	entry->key = key;
	entry->next = accel_hash->hash_table[index];
	accel_hash->hash_table[index] = entry;
	return entry;
}

static crex_always_inline void* crex_accel_hash_find_ex(crex_accel_hash *accel_hash, crex_string *key, int data)
{
	crex_ulong index;
	crex_accel_hash_entry *entry;
	crex_ulong hash_value;

	hash_value = crex_string_hash_val(key);
#ifndef CREX_WIN32
	hash_value ^= ZCG(root_hash);
#endif
	index = hash_value % accel_hash->max_num_entries;

	entry = accel_hash->hash_table[index];
	while (entry) {
		if (entry->hash_value == hash_value
		 && crex_string_equals(entry->key, key)) {
			if (entry->indirect) {
				if (data) {
					return ((crex_accel_hash_entry*)entry->data)->data;
				} else {
					return entry->data;
				}
			} else {
				if (data) {
					return entry->data;
				} else {
					return entry;
				}
			}
		}
		entry = entry->next;
	}
	return NULL;
}

/* Returns the data associated with key on success
 * Returns NULL if data doesn't exist
 */
void* crex_accel_hash_find(crex_accel_hash *accel_hash, crex_string *key)
{
	return crex_accel_hash_find_ex(accel_hash, key, 1);
}

/* Returns the hash entry associated with key on success
 * Returns NULL if it doesn't exist
 */
crex_accel_hash_entry* crex_accel_hash_find_entry(crex_accel_hash *accel_hash, crex_string *key)
{
	return (crex_accel_hash_entry *)crex_accel_hash_find_ex(accel_hash, key, 0);
}

int crex_accel_hash_unlink(crex_accel_hash *accel_hash, crex_string *key)
{
	crex_ulong hash_value;
	crex_ulong index;
	crex_accel_hash_entry *entry, *last_entry=NULL;

	hash_value = crex_string_hash_val(key);
#ifndef CREX_WIN32
	hash_value ^= ZCG(root_hash);
#endif
	index = hash_value % accel_hash->max_num_entries;

	entry = accel_hash->hash_table[index];
	while (entry) {
		if (entry->hash_value == hash_value
		 && crex_string_equals(entry->key, key)) {
			if (!entry->indirect) {
				accel_hash->num_direct_entries--;
			}
			if (last_entry) {
				last_entry->next = entry->next;
			} else {
				accel_hash->hash_table[index] = entry->next;
			}
			return SUCCESS;
		}
		last_entry = entry;
		entry = entry->next;
	}
	return FAILURE;
}
