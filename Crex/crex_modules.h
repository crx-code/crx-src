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

#ifndef MODULES_H
#define MODULES_H

#include "crex.h"
#include "crex_compile.h"
#include "crex_build.h"

#define INIT_FUNC_ARGS		int type, int module_number
#define INIT_FUNC_ARGS_PASSTHRU	type, module_number
#define SHUTDOWN_FUNC_ARGS	int type, int module_number
#define SHUTDOWN_FUNC_ARGS_PASSTHRU type, module_number
#define CREX_MODULE_INFO_FUNC_ARGS crex_module_entry *crex_module
#define CREX_MODULE_INFO_FUNC_ARGS_PASSTHRU crex_module

#define CREX_MODULE_API_NO 20230831
#ifdef ZTS
#define USING_ZTS 1
#else
#define USING_ZTS 0
#endif

#define STANDARD_MODULE_HEADER_EX sizeof(crex_module_entry), CREX_MODULE_API_NO, CREX_DEBUG, USING_ZTS
#define STANDARD_MODULE_HEADER \
	STANDARD_MODULE_HEADER_EX, NULL, NULL
#define ZE2_STANDARD_MODULE_HEADER \
	STANDARD_MODULE_HEADER_EX, ini_entries, NULL

#define CREX_MODULE_BUILD_ID "API" CREX_TOSTR(CREX_MODULE_API_NO) CREX_BUILD_TS CREX_BUILD_DEBUG CREX_BUILD_SYSTEM CREX_BUILD_EXTRA

#define STANDARD_MODULE_PROPERTIES_EX 0, 0, NULL, 0, CREX_MODULE_BUILD_ID

#define NO_MODULE_GLOBALS 0, NULL, NULL, NULL

#ifdef ZTS
# define CREX_MODULE_GLOBALS(module_name) sizeof(crex_##module_name##_globals), &module_name##_globals_id
#else
# define CREX_MODULE_GLOBALS(module_name) sizeof(crex_##module_name##_globals), &module_name##_globals
#endif

#define STANDARD_MODULE_PROPERTIES \
	NO_MODULE_GLOBALS, NULL, STANDARD_MODULE_PROPERTIES_EX

#define NO_VERSION_YET NULL

#define MODULE_PERSISTENT 1
#define MODULE_TEMPORARY 2

struct _crex_ini_entry;
typedef struct _crex_module_entry crex_module_entry;
typedef struct _crex_module_dep crex_module_dep;

struct _crex_module_entry {
	unsigned short size;
	unsigned int crex_api;
	unsigned char crex_debug;
	unsigned char zts;
	const struct _crex_ini_entry *ini_entry;
	const struct _crex_module_dep *deps;
	const char *name;
	const struct _crex_function_entry *functions;
	crex_result (*module_startup_func)(INIT_FUNC_ARGS);
	crex_result (*module_shutdown_func)(SHUTDOWN_FUNC_ARGS);
	crex_result (*request_startup_func)(INIT_FUNC_ARGS);
	crex_result (*request_shutdown_func)(SHUTDOWN_FUNC_ARGS);
	void (*info_func)(CREX_MODULE_INFO_FUNC_ARGS);
	const char *version;
	size_t globals_size;
#ifdef ZTS
	ts_rsrc_id* globals_id_ptr;
#else
	void* globals_ptr;
#endif
	void (*globals_ctor)(void *global);
	void (*globals_dtor)(void *global);
	crex_result (*post_deactivate_func)(void);
	int module_started;
	unsigned char type;
	void *handle;
	int module_number;
	const char *build_id;
};

#define MODULE_DEP_REQUIRED		1
#define MODULE_DEP_CONFLICTS	2
#define MODULE_DEP_OPTIONAL		3

#define CREX_MOD_REQUIRED_EX(name, rel, ver)	{ name, rel, ver, MODULE_DEP_REQUIRED  },
#define CREX_MOD_CONFLICTS_EX(name, rel, ver)	{ name, rel, ver, MODULE_DEP_CONFLICTS },
#define CREX_MOD_OPTIONAL_EX(name, rel, ver)	{ name, rel, ver, MODULE_DEP_OPTIONAL  },

#define CREX_MOD_REQUIRED(name)		CREX_MOD_REQUIRED_EX(name, NULL, NULL)
#define CREX_MOD_CONFLICTS(name)	CREX_MOD_CONFLICTS_EX(name, NULL, NULL)
#define CREX_MOD_OPTIONAL(name)		CREX_MOD_OPTIONAL_EX(name, NULL, NULL)

#define CREX_MOD_END { NULL, NULL, NULL, 0 }

struct _crex_module_dep {
	const char *name;		/* module name */
	const char *rel;		/* version relationship: NULL (exists), lt|le|eq|ge|gt (to given version) */
	const char *version;	/* version */
	unsigned char type;		/* dependency type */
};

BEGIN_EXTERN_C()
extern CREX_API HashTable module_registry;

void module_destructor(crex_module_entry *module);
int module_registry_request_startup(crex_module_entry *module);
int module_registry_unload_temp(const crex_module_entry *module);
END_EXTERN_C()

#endif
