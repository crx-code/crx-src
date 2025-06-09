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
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   +----------------------------------------------------------------------+
*/

/* resource lists */

#include "crex.h"
#include "crex_list.h"
#include "crex_API.h"
#include "crex_globals.h"

CREX_API int le_index_ptr;

/* true global */
static HashTable list_destructors;

CREX_API zval* CREX_FASTCALL crex_list_insert(void *ptr, int type)
{
	zval zv;

	crex_long index = crex_hash_next_free_element(&EG(regular_list));
	if (index == 0) {
		index = 1;
	} else if (index == CREX_LONG_MAX) {
		crex_error_noreturn(E_ERROR, "Resource ID space overflow");
	}
	ZVAL_NEW_RES(&zv, index, ptr, type);
	return crex_hash_index_add_new(&EG(regular_list), index, &zv);
}

CREX_API crex_result CREX_FASTCALL crex_list_delete(crex_resource *res)
{
	if (GC_DELREF(res) <= 0) {
		return crex_hash_index_del(&EG(regular_list), res->handle);
	} else {
		return SUCCESS;
	}
}

CREX_API void CREX_FASTCALL crex_list_free(crex_resource *res)
{
	CREX_ASSERT(GC_REFCOUNT(res) == 0);
	crex_hash_index_del(&EG(regular_list), res->handle);
}

static void crex_resource_dtor(crex_resource *res)
{
	crex_rsrc_list_dtors_entry *ld;
	crex_resource r = *res;

	res->type = -1;
	res->ptr = NULL;

	ld = crex_hash_index_find_ptr(&list_destructors, r.type);
	CREX_ASSERT(ld && "Unknown list entry type");

	if (ld->list_dtor_ex) {
		ld->list_dtor_ex(&r);
	}
}


CREX_API void CREX_FASTCALL crex_list_close(crex_resource *res)
{
	if (GC_REFCOUNT(res) <= 0) {
		crex_list_free(res);
	} else if (res->type >= 0) {
		crex_resource_dtor(res);
	}
}

CREX_API crex_resource* crex_register_resource(void *rsrc_pointer, int rsrc_type)
{
	zval *zv;

	zv = crex_list_insert(rsrc_pointer, rsrc_type);

	return C_RES_P(zv);
}

CREX_API void *crex_fetch_resource2(crex_resource *res, const char *resource_type_name, int resource_type1, int resource_type2)
{
	if (res) {
		if (resource_type1 == res->type) {
			return res->ptr;
		}

		if (resource_type2 == res->type) {
			return res->ptr;
		}
	}

	if (resource_type_name) {
		const char *space;
		const char *class_name = get_active_class_name(&space);
		crex_type_error("%s%s%s(): supplied resource is not a valid %s resource", class_name, space, get_active_function_name(), resource_type_name);
	}

	return NULL;
}

CREX_API void *crex_fetch_resource(crex_resource *res, const char *resource_type_name, int resource_type)
{
	if (resource_type == res->type) {
		return res->ptr;
	}

	if (resource_type_name) {
		const char *space;
		const char *class_name = get_active_class_name(&space);
		crex_type_error("%s%s%s(): supplied resource is not a valid %s resource", class_name, space, get_active_function_name(), resource_type_name);
	}

	return NULL;
}

CREX_API void *crex_fetch_resource_ex(zval *res, const char *resource_type_name, int resource_type)
{
	const char *space, *class_name;
	if (res == NULL) {
		if (resource_type_name) {
			class_name = get_active_class_name(&space);
			crex_type_error("%s%s%s(): no %s resource supplied", class_name, space, get_active_function_name(), resource_type_name);
		}
		return NULL;
	}
	if (C_TYPE_P(res) != IS_RESOURCE) {
		if (resource_type_name) {
			class_name = get_active_class_name(&space);
			crex_type_error("%s%s%s(): supplied argument is not a valid %s resource", class_name, space, get_active_function_name(), resource_type_name);
		}
		return NULL;
	}

	return crex_fetch_resource(C_RES_P(res), resource_type_name, resource_type);
}

CREX_API void *crex_fetch_resource2_ex(zval *res, const char *resource_type_name, int resource_type1, int resource_type2)
{
	const char *space, *class_name;
	if (res == NULL) {
		if (resource_type_name) {
			class_name = get_active_class_name(&space);
			crex_type_error("%s%s%s(): no %s resource supplied", class_name, space, get_active_function_name(), resource_type_name);
		}
		return NULL;
	}
	if (C_TYPE_P(res) != IS_RESOURCE) {
		if (resource_type_name) {
			class_name = get_active_class_name(&space);
			crex_type_error("%s%s%s(): supplied argument is not a valid %s resource", class_name, space, get_active_function_name(), resource_type_name);
		}
		return NULL;
	}

	return crex_fetch_resource2(C_RES_P(res), resource_type_name, resource_type1, resource_type2);
}

void list_entry_destructor(zval *zv)
{
	crex_resource *res = C_RES_P(zv);

	ZVAL_UNDEF(zv);
	if (res->type >= 0) {
		crex_resource_dtor(res);
	}
	efree_size(res, sizeof(crex_resource));
}

void plist_entry_destructor(zval *zv)
{
	crex_resource *res = C_RES_P(zv);

	if (res->type >= 0) {
		crex_rsrc_list_dtors_entry *ld;

		ld = crex_hash_index_find_ptr(&list_destructors, res->type);
		CREX_ASSERT(ld && "Unknown list entry type");

		if (ld->plist_dtor_ex) {
			ld->plist_dtor_ex(res);
		}
	}
	free(res);
}

CREX_API void crex_init_rsrc_list(void)
{
	crex_hash_init(&EG(regular_list), 8, NULL, list_entry_destructor, 0);
	EG(regular_list).nNextFreeElement = 0;
}


void crex_init_rsrc_plist(void)
{
	crex_hash_init(&EG(persistent_list), 8, NULL, plist_entry_destructor, 1);
}


void crex_close_rsrc_list(HashTable *ht)
{
	/* Reload ht->arData on each iteration, as it may be reallocated. */
	uint32_t i = ht->nNumUsed;

	while (i-- > 0) {
		zval *p = CREX_HASH_ELEMENT(ht, i);
		if (C_TYPE_P(p) != IS_UNDEF) {
			crex_resource *res = C_PTR_P(p);
			if (res->type >= 0) {
				crex_resource_dtor(res);
			}
		}
	}
}


void crex_destroy_rsrc_list(HashTable *ht)
{
	crex_hash_graceful_reverse_destroy(ht);
}

/* int return due to HashTable API */
static int clean_module_resource(zval *zv, void *arg)
{
	int resource_id = *(int *)arg;

	return C_RES_TYPE_P(zv) == resource_id;
}

/* int return due to HashTable API */
static int crex_clean_module_rsrc_dtors_cb(zval *zv, void *arg)
{
	crex_rsrc_list_dtors_entry *ld = (crex_rsrc_list_dtors_entry *)C_PTR_P(zv);
	int module_number = *(int *)arg;
	if (ld->module_number == module_number) {
		crex_hash_apply_with_argument(&EG(persistent_list), clean_module_resource, (void *) &(ld->resource_id));
		return 1;
	} else {
		return 0;
	}
}


void crex_clean_module_rsrc_dtors(int module_number)
{
	crex_hash_apply_with_argument(&list_destructors, crex_clean_module_rsrc_dtors_cb, (void *) &module_number);
}


CREX_API int crex_register_list_destructors_ex(rsrc_dtor_func_t ld, rsrc_dtor_func_t pld, const char *type_name, int module_number)
{
	crex_rsrc_list_dtors_entry *lde;
	zval zv;

	lde = malloc(sizeof(crex_rsrc_list_dtors_entry));
	lde->list_dtor_ex = ld;
	lde->plist_dtor_ex = pld;
	lde->module_number = module_number;
	lde->resource_id = list_destructors.nNextFreeElement;
	lde->type_name = type_name;
	ZVAL_PTR(&zv, lde);

	if (crex_hash_next_index_insert(&list_destructors, &zv) == NULL) {
		free(lde);
		return FAILURE;
	}
	return list_destructors.nNextFreeElement-1;
}

CREX_API int crex_fetch_list_dtor_id(const char *type_name)
{
	crex_rsrc_list_dtors_entry *lde;

	CREX_HASH_PACKED_FOREACH_PTR(&list_destructors, lde) {
		if (lde->type_name && (strcmp(type_name, lde->type_name) == 0)) {
			return lde->resource_id;
		}
	} CREX_HASH_FOREACH_END();

	return 0;
}

static void list_destructors_dtor(zval *zv)
{
	free(C_PTR_P(zv));
}

void crex_init_rsrc_list_dtors(void)
{
	crex_hash_init(&list_destructors, 64, NULL, list_destructors_dtor, 1);
	list_destructors.nNextFreeElement=1;	/* we don't want resource type 0 */
}


void crex_destroy_rsrc_list_dtors(void)
{
	crex_hash_destroy(&list_destructors);
}


const char *crex_rsrc_list_get_rsrc_type(crex_resource *res)
{
	crex_rsrc_list_dtors_entry *lde;

	lde = crex_hash_index_find_ptr(&list_destructors, res->type);
	if (lde) {
		return lde->type_name;
	} else {
		return NULL;
	}
}

CREX_API crex_resource* crex_register_persistent_resource_ex(crex_string *key, void *rsrc_pointer, int rsrc_type)
{
	zval *zv;
	zval tmp;

	ZVAL_NEW_PERSISTENT_RES(&tmp, -1, rsrc_pointer, rsrc_type);
	GC_MAKE_PERSISTENT_LOCAL(C_COUNTED(tmp));
	GC_MAKE_PERSISTENT_LOCAL(key);

	zv = crex_hash_update(&EG(persistent_list), key, &tmp);

	return C_RES_P(zv);
}

CREX_API crex_resource* crex_register_persistent_resource(const char *key, size_t key_len, void *rsrc_pointer, int rsrc_type)
{
	crex_string *str = crex_string_init(key, key_len, 1);
	crex_resource *ret  = crex_register_persistent_resource_ex(str, rsrc_pointer, rsrc_type);

	crex_string_release_ex(str, 1);
	return ret;
}
