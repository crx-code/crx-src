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

#ifndef CREX_EXTENSIONS_H
#define CREX_EXTENSIONS_H

#include "crex_compile.h"
#include "crex_build.h"

/*
The constants below are derived from ext/opcache/CrexAccelerator.h

You can use the following macro to check the extension API version for compatibilities:

#define	CREX_EXTENSION_API_NO_5_0_X	220040412
#define	CREX_EXTENSION_API_NO_5_1_X	220051025
#define	CREX_EXTENSION_API_NO_5_2_X	220060519
#define	CREX_EXTENSION_API_NO_5_3_X	220090626
#define	CREX_EXTENSION_API_NO_5_4_X	220100525
#define	CREX_EXTENSION_API_NO_5_5_X	220121212
#define	CREX_EXTENSION_API_NO_5_6_X	220131226
#define	CREX_EXTENSION_API_NO_7_0_X	320151012

#if CREX_EXTENSION_API_NO < CREX_EXTENSION_API_NO_5_5_X
   // do something for crx versions lower than 5.5.x
#endif
*/

/* The first number is the engine version and the rest is the date (YYYYMMDD).
 * This way engine 2/3 API no. is always greater than engine 1 API no..  */
#define CREX_EXTENSION_API_NO	420230831

typedef struct _crex_extension_version_info {
	int crex_extension_api_no;
	const char *build_id;
} crex_extension_version_info;

#define CREX_EXTENSION_BUILD_ID "API" CREX_TOSTR(CREX_EXTENSION_API_NO) CREX_BUILD_TS CREX_BUILD_DEBUG CREX_BUILD_SYSTEM CREX_BUILD_EXTRA

typedef struct _crex_extension crex_extension;

/* Typedef's for crex_extension function pointers */
typedef int (*startup_func_t)(crex_extension *extension);
typedef void (*shutdown_func_t)(crex_extension *extension);
typedef void (*activate_func_t)(void);
typedef void (*deactivate_func_t)(void);

typedef void (*message_handler_func_t)(int message, void *arg);

typedef void (*op_array_handler_func_t)(crex_op_array *op_array);

typedef void (*statement_handler_func_t)(crex_execute_data *frame);
typedef void (*fcall_begin_handler_func_t)(crex_execute_data *frame);
typedef void (*fcall_end_handler_func_t)(crex_execute_data *frame);

typedef void (*op_array_ctor_func_t)(crex_op_array *op_array);
typedef void (*op_array_dtor_func_t)(crex_op_array *op_array);
typedef size_t (*op_array_persist_calc_func_t)(crex_op_array *op_array);
typedef size_t (*op_array_persist_func_t)(crex_op_array *op_array, void *mem);

struct _crex_extension {
	const char *name;
	const char *version;
	const char *author;
	const char *URL;
	const char *copyright;

	startup_func_t startup;
	shutdown_func_t shutdown;
	activate_func_t activate;
	deactivate_func_t deactivate;

	message_handler_func_t message_handler;

	op_array_handler_func_t op_array_handler;

	statement_handler_func_t statement_handler;
	fcall_begin_handler_func_t fcall_begin_handler;
	fcall_end_handler_func_t fcall_end_handler;

	op_array_ctor_func_t op_array_ctor;
	op_array_dtor_func_t op_array_dtor;

	int (*api_no_check)(int api_no);
	int (*build_id_check)(const char* build_id);
	op_array_persist_calc_func_t op_array_persist_calc;
	op_array_persist_func_t op_array_persist;
	void *reserved5;
	void *reserved6;
	void *reserved7;
	void *reserved8;

	DL_HANDLE handle;
	int resource_number;
};

BEGIN_EXTERN_C()
extern CREX_API int crex_op_array_extension_handles;

CREX_API int crex_get_resource_handle(const char *module_name);
CREX_API int crex_get_op_array_extension_handle(const char *module_name);
CREX_API int crex_get_op_array_extension_handles(const char *module_name, int handles);
CREX_API void crex_extension_dispatch_message(int message, void *arg);
END_EXTERN_C()

#define CREX_EXTMSG_NEW_EXTENSION		1


#define CREX_EXTENSION()	\
	CREX_EXT_API crex_extension_version_info extension_version_info = { CREX_EXTENSION_API_NO, CREX_EXTENSION_BUILD_ID }

#define STANDARD_CREX_EXTENSION_PROPERTIES       NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1
#define COMPAT_CREX_EXTENSION_PROPERTIES         NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1
#define BUILD_COMPAT_CREX_EXTENSION_PROPERTIES   NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1


CREX_API extern crex_llist crex_extensions;
CREX_API extern uint32_t crex_extension_flags;

#define CREX_EXTENSIONS_HAVE_OP_ARRAY_CTOR         (1<<0)
#define CREX_EXTENSIONS_HAVE_OP_ARRAY_DTOR         (1<<1)
#define CREX_EXTENSIONS_HAVE_OP_ARRAY_HANDLER      (1<<2)
#define CREX_EXTENSIONS_HAVE_OP_ARRAY_PERSIST_CALC (1<<3)
#define CREX_EXTENSIONS_HAVE_OP_ARRAY_PERSIST      (1<<4)

void crex_extension_dtor(crex_extension *extension);
CREX_API void crex_append_version_info(const crex_extension *extension);
void crex_startup_extensions_mechanism(void);
void crex_startup_extensions(void);
void crex_shutdown_extensions(void);

CREX_API size_t crex_internal_run_time_cache_reserved_size(void);
CREX_API void crex_init_internal_run_time_cache(void);

BEGIN_EXTERN_C()
CREX_API crex_result crex_load_extension(const char *path);
CREX_API crex_result crex_load_extension_handle(DL_HANDLE handle, const char *path);
CREX_API void crex_register_extension(crex_extension *new_extension, DL_HANDLE handle);
CREX_API crex_extension *crex_get_extension(const char *extension_name);
CREX_API size_t crex_extensions_op_array_persist_calc(crex_op_array *op_array);
CREX_API size_t crex_extensions_op_array_persist(crex_op_array *op_array, void *mem);
END_EXTERN_C()

#endif /* CREX_EXTENSIONS_H */
