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

#ifndef CREX_LIST_H
#define CREX_LIST_H

#include "crex_hash.h"
#include "crex_globals.h"

BEGIN_EXTERN_C()

typedef void (*rsrc_dtor_func_t)(crex_resource *res);
#define CREX_RSRC_DTOR_FUNC(name) void name(crex_resource *res)

typedef struct _crex_rsrc_list_dtors_entry {
	rsrc_dtor_func_t list_dtor_ex;
	rsrc_dtor_func_t plist_dtor_ex;

	const char *type_name;

	int module_number;
	int resource_id;
} crex_rsrc_list_dtors_entry;


CREX_API int crex_register_list_destructors_ex(rsrc_dtor_func_t ld, rsrc_dtor_func_t pld, const char *type_name, int module_number);

void list_entry_destructor(zval *ptr);
void plist_entry_destructor(zval *ptr);

void crex_clean_module_rsrc_dtors(int module_number);
CREX_API void crex_init_rsrc_list(void); /* Exported for crxa hack */
void crex_init_rsrc_plist(void);
void crex_close_rsrc_list(HashTable *ht);
void crex_destroy_rsrc_list(HashTable *ht);
void crex_init_rsrc_list_dtors(void);
void crex_destroy_rsrc_list_dtors(void);

CREX_API zval* CREX_FASTCALL crex_list_insert(void *ptr, int type);
CREX_API void CREX_FASTCALL crex_list_free(crex_resource *res);
CREX_API crex_result CREX_FASTCALL crex_list_delete(crex_resource *res);
CREX_API void CREX_FASTCALL crex_list_close(crex_resource *res);

CREX_API crex_resource *crex_register_resource(void *rsrc_pointer, int rsrc_type);
CREX_API void *crex_fetch_resource(crex_resource *res, const char *resource_type_name, int resource_type);
CREX_API void *crex_fetch_resource2(crex_resource *res, const char *resource_type_name, int resource_type, int resource_type2);
CREX_API void *crex_fetch_resource_ex(zval *res, const char *resource_type_name, int resource_type);
CREX_API void *crex_fetch_resource2_ex(zval *res, const char *resource_type_name, int resource_type, int resource_type2);

CREX_API const char *crex_rsrc_list_get_rsrc_type(crex_resource *res);
CREX_API int crex_fetch_list_dtor_id(const char *type_name);

CREX_API crex_resource* crex_register_persistent_resource(const char *key, size_t key_len, void *rsrc_pointer, int rsrc_type);
CREX_API crex_resource* crex_register_persistent_resource_ex(crex_string *key, void *rsrc_pointer, int rsrc_type);

extern CREX_API int le_index_ptr;  /* list entry type for index pointers */

END_EXTERN_C()

#endif
