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

#include "crex_extensions.h"
#include "crex_system_id.h"

CREX_API crex_llist crex_extensions;
CREX_API uint32_t crex_extension_flags = 0;
CREX_API int crex_op_array_extension_handles = 0;
static int last_resource_number;

crex_result crex_load_extension(const char *path)
{
#if CREX_EXTENSIONS_SUPPORT
	DL_HANDLE handle;

	handle = DL_LOAD(path);
	if (!handle) {
#ifndef CREX_WIN32
		fprintf(stderr, "Failed loading %s:  %s\n", path, DL_ERROR());
#else
		fprintf(stderr, "Failed loading %s\n", path);
		/* See http://support.microsoft.com/kb/190351 */
		fflush(stderr);
#endif
		return FAILURE;
	}
#ifdef CREX_WIN32
	char *err;
	if (!crx_win32_image_compatible(handle, &err)) {
		crex_error(E_CORE_WARNING, err);
		return FAILURE;
	}
#endif
	return crex_load_extension_handle(handle, path);
#else
	fprintf(stderr, "Extensions are not supported on this platform.\n");
/* See http://support.microsoft.com/kb/190351 */
#ifdef CREX_WIN32
	fflush(stderr);
#endif
	return FAILURE;
#endif
}

crex_result crex_load_extension_handle(DL_HANDLE handle, const char *path)
{
#if CREX_EXTENSIONS_SUPPORT
	crex_extension *new_extension;

	const crex_extension_version_info *extension_version_info = (const crex_extension_version_info *) DL_FETCH_SYMBOL(handle, "extension_version_info");
	if (!extension_version_info) {
		extension_version_info = (const crex_extension_version_info *) DL_FETCH_SYMBOL(handle, "_extension_version_info");
	}
	new_extension = (crex_extension *) DL_FETCH_SYMBOL(handle, "crex_extension_entry");
	if (!new_extension) {
		new_extension = (crex_extension *) DL_FETCH_SYMBOL(handle, "_crex_extension_entry");
	}
	if (!extension_version_info || !new_extension) {
		fprintf(stderr, "%s doesn't appear to be a valid Crex extension\n", path);
/* See http://support.microsoft.com/kb/190351 */
#ifdef CREX_WIN32
		fflush(stderr);
#endif
		DL_UNLOAD(handle);
		return FAILURE;
	}

	/* allow extension to proclaim compatibility with any Crex version */
	if (extension_version_info->crex_extension_api_no != CREX_EXTENSION_API_NO &&(!new_extension->api_no_check || new_extension->api_no_check(CREX_EXTENSION_API_NO) != SUCCESS)) {
		if (extension_version_info->crex_extension_api_no > CREX_EXTENSION_API_NO) {
			fprintf(stderr, "%s requires Crex Engine API version %d.\n"
					"The Crex Engine API version %d which is installed, is outdated.\n\n",
					new_extension->name,
					extension_version_info->crex_extension_api_no,
					CREX_EXTENSION_API_NO);
/* See http://support.microsoft.com/kb/190351 */
#ifdef CREX_WIN32
			fflush(stderr);
#endif
			DL_UNLOAD(handle);
			return FAILURE;
		} else if (extension_version_info->crex_extension_api_no < CREX_EXTENSION_API_NO) {
			fprintf(stderr, "%s requires Crex Engine API version %d.\n"
					"The Crex Engine API version %d which is installed, is newer.\n"
					"Contact %s at %s for a later version of %s.\n\n",
					new_extension->name,
					extension_version_info->crex_extension_api_no,
					CREX_EXTENSION_API_NO,
					new_extension->author,
					new_extension->URL,
					new_extension->name);
/* See http://support.microsoft.com/kb/190351 */
#ifdef CREX_WIN32
			fflush(stderr);
#endif
			DL_UNLOAD(handle);
			return FAILURE;
		}
	} else if (strcmp(CREX_EXTENSION_BUILD_ID, extension_version_info->build_id) &&
	           (!new_extension->build_id_check || new_extension->build_id_check(CREX_EXTENSION_BUILD_ID) != SUCCESS)) {
		fprintf(stderr, "Cannot load %s - it was built with configuration %s, whereas running engine is %s\n",
					new_extension->name, extension_version_info->build_id, CREX_EXTENSION_BUILD_ID);
/* See http://support.microsoft.com/kb/190351 */
#ifdef CREX_WIN32
		fflush(stderr);
#endif
		DL_UNLOAD(handle);
		return FAILURE;
	} else if (crex_get_extension(new_extension->name)) {
		fprintf(stderr, "Cannot load %s - it was already loaded\n", new_extension->name);
/* See http://support.microsoft.com/kb/190351 */
#ifdef CREX_WIN32
		fflush(stderr);
#endif
		DL_UNLOAD(handle);
		return FAILURE;
	}

	crex_register_extension(new_extension, handle);
	return SUCCESS;
#else
	fprintf(stderr, "Extensions are not supported on this platform.\n");
/* See http://support.microsoft.com/kb/190351 */
#ifdef CREX_WIN32
	fflush(stderr);
#endif
	return FAILURE;
#endif
}


void crex_register_extension(crex_extension *new_extension, DL_HANDLE handle)
{
#if CREX_EXTENSIONS_SUPPORT
	crex_extension extension;

	extension = *new_extension;
	extension.handle = handle;

	crex_extension_dispatch_message(CREX_EXTMSG_NEW_EXTENSION, &extension);

	crex_llist_add_element(&crex_extensions, &extension);

	if (extension.op_array_ctor) {
		crex_extension_flags |= CREX_EXTENSIONS_HAVE_OP_ARRAY_CTOR;
	}
	if (extension.op_array_dtor) {
		crex_extension_flags |= CREX_EXTENSIONS_HAVE_OP_ARRAY_DTOR;
	}
	if (extension.op_array_handler) {
		crex_extension_flags |= CREX_EXTENSIONS_HAVE_OP_ARRAY_HANDLER;
	}
	if (extension.op_array_persist_calc) {
		crex_extension_flags |= CREX_EXTENSIONS_HAVE_OP_ARRAY_PERSIST_CALC;
	}
	if (extension.op_array_persist) {
		crex_extension_flags |= CREX_EXTENSIONS_HAVE_OP_ARRAY_PERSIST;
	}
	/*fprintf(stderr, "Loaded %s, version %s\n", extension.name, extension.version);*/
#endif
}


static void crex_extension_shutdown(crex_extension *extension)
{
#if CREX_EXTENSIONS_SUPPORT
	if (extension->shutdown) {
		extension->shutdown(extension);
	}
#endif
}

/* int return due to crex linked list API */
static int crex_extension_startup(crex_extension *extension)
{
#if CREX_EXTENSIONS_SUPPORT
	if (extension->startup) {
		if (extension->startup(extension)!=SUCCESS) {
			return 1;
		}
		crex_append_version_info(extension);
	}
#endif
	return 0;
}


void crex_startup_extensions_mechanism(void)
{
	/* Startup extensions mechanism */
	crex_llist_init(&crex_extensions, sizeof(crex_extension), (void (*)(void *)) crex_extension_dtor, 1);
	crex_op_array_extension_handles = 0;
	last_resource_number = 0;
}


void crex_startup_extensions(void)
{
	crex_llist_apply_with_del(&crex_extensions, (int (*)(void *)) crex_extension_startup);
}


void crex_shutdown_extensions(void)
{
	crex_llist_apply(&crex_extensions, (llist_apply_func_t) crex_extension_shutdown);
	crex_llist_destroy(&crex_extensions);
}


void crex_extension_dtor(crex_extension *extension)
{
#if CREX_EXTENSIONS_SUPPORT && !CREX_DEBUG
	if (extension->handle && !getenv("CREX_DONT_UNLOAD_MODULES")) {
		DL_UNLOAD(extension->handle);
	}
#endif
}


static void crex_extension_message_dispatcher(const crex_extension *extension, int num_args, va_list args)
{
	int message;
	void *arg;

	if (!extension->message_handler || num_args!=2) {
		return;
	}
	message = va_arg(args, int);
	arg = va_arg(args, void *);
	extension->message_handler(message, arg);
}


CREX_API void crex_extension_dispatch_message(int message, void *arg)
{
	crex_llist_apply_with_arguments(&crex_extensions, (llist_apply_with_args_func_t) crex_extension_message_dispatcher, 2, message, arg);
}


CREX_API int crex_get_resource_handle(const char *module_name)
{
	if (last_resource_number<CREX_MAX_RESERVED_RESOURCES) {
		crex_add_system_entropy(module_name, "crex_get_resource_handle", &last_resource_number, sizeof(int));
		return last_resource_number++;
	} else {
		return -1;
	}
}

/**
 * The handle returned by this function can be used with
 * `CREX_OP_ARRAY_EXTENSION(op_array, handle)`.
 *
 * The extension slot has been available since CRX 7.4 on user functions and
 * has been available since CRX 8.2 on internal functions.
 * 
 * # Safety
 * The extension slot made available by calling this function is initialized on
 * the first call made to the function in that request. If you need to
 * initialize it before this point, call `crex_init_func_run_time_cache`.
 *
 * The function cache slots are not available if the function is a trampoline,
 * which can be checked with something like:
 * 
 *     if (fbc->type == CREX_USER_FUNCTION
 *         && !(fbc->op_array.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE)
 *     ) {
 *         // Use CREX_OP_ARRAY_EXTENSION somehow
 *     }
 */  
CREX_API int crex_get_op_array_extension_handle(const char *module_name)
{
	int handle = crex_op_array_extension_handles++;
	crex_add_system_entropy(module_name, "crex_get_op_array_extension_handle", &crex_op_array_extension_handles, sizeof(int));
	return handle;
}

/** See crex_get_op_array_extension_handle for important usage information. */
CREX_API int crex_get_op_array_extension_handles(const char *module_name, int handles)
{
	int handle = crex_op_array_extension_handles;
	crex_op_array_extension_handles += handles;
	crex_add_system_entropy(module_name, "crex_get_op_array_extension_handle", &crex_op_array_extension_handles, sizeof(int));
	return handle;
}

CREX_API size_t crex_internal_run_time_cache_reserved_size(void) {
	return crex_op_array_extension_handles * sizeof(void *);
}

CREX_API void crex_init_internal_run_time_cache(void) {
	size_t rt_size = crex_internal_run_time_cache_reserved_size();
	if (rt_size) {
		size_t functions = crex_hash_num_elements(CG(function_table));
		crex_class_entry *ce;
		CREX_HASH_MAP_FOREACH_PTR(CG(class_table), ce) {
			functions += crex_hash_num_elements(&ce->function_table);
		} CREX_HASH_FOREACH_END();

		char *ptr = crex_arena_calloc(&CG(arena), functions, rt_size);
		crex_internal_function *zif;
		CREX_HASH_MAP_FOREACH_PTR(CG(function_table), zif) {
			if (!CREX_USER_CODE(zif->type) && CREX_MAP_PTR_GET(zif->run_time_cache) == NULL)
			{
				CREX_MAP_PTR_SET(zif->run_time_cache, (void *)ptr);
				ptr += rt_size;
			}
		} CREX_HASH_FOREACH_END();
		CREX_HASH_MAP_FOREACH_PTR(CG(class_table), ce) {
			CREX_HASH_MAP_FOREACH_PTR(&ce->function_table, zif) {
				if (!CREX_USER_CODE(zif->type) && CREX_MAP_PTR_GET(zif->run_time_cache) == NULL)
				{
					CREX_MAP_PTR_SET(zif->run_time_cache, (void *)ptr);
					ptr += rt_size;
				}
			} CREX_HASH_FOREACH_END();
		} CREX_HASH_FOREACH_END();
	}
}

CREX_API crex_extension *crex_get_extension(const char *extension_name)
{
	crex_llist_element *element;

	for (element = crex_extensions.head; element; element = element->next) {
		crex_extension *extension = (crex_extension *) element->data;

		if (!strcmp(extension->name, extension_name)) {
			return extension;
		}
	}
	return NULL;
}

typedef struct _crex_extension_persist_data {
	crex_op_array *op_array;
	size_t         size;
	char          *mem;
} crex_extension_persist_data;

static void crex_extension_op_array_persist_calc_handler(crex_extension *extension, crex_extension_persist_data *data)
{
	if (extension->op_array_persist_calc) {
		data->size += extension->op_array_persist_calc(data->op_array);
	}
}

static void crex_extension_op_array_persist_handler(crex_extension *extension, crex_extension_persist_data *data)
{
	if (extension->op_array_persist) {
		size_t size = extension->op_array_persist(data->op_array, data->mem);
		if (size) {
			data->mem = (void*)((char*)data->mem + size);
			data->size += size;
		}
	}
}

CREX_API size_t crex_extensions_op_array_persist_calc(crex_op_array *op_array)
{
	if (crex_extension_flags & CREX_EXTENSIONS_HAVE_OP_ARRAY_PERSIST_CALC) {
		crex_extension_persist_data data;

		data.op_array = op_array;
		data.size = 0;
		data.mem  = NULL;
		crex_llist_apply_with_argument(&crex_extensions, (llist_apply_with_arg_func_t) crex_extension_op_array_persist_calc_handler, &data);
		return data.size;
	}
	return 0;
}

CREX_API size_t crex_extensions_op_array_persist(crex_op_array *op_array, void *mem)
{
	if (crex_extension_flags & CREX_EXTENSIONS_HAVE_OP_ARRAY_PERSIST) {
		crex_extension_persist_data data;

		data.op_array = op_array;
		data.size = 0;
		data.mem  = mem;
		crex_llist_apply_with_argument(&crex_extensions, (llist_apply_with_arg_func_t) crex_extension_op_array_persist_handler, &data);
		return data.size;
	}
	return 0;
}
