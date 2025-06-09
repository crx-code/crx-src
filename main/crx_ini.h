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
   | Author: Zeev Suraski <zeev@crx.net>                                  |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_INI_H
#define CRX_INI_H

#include "crex_ini.h"

BEGIN_EXTERN_C()
CRXAPI void config_zval_dtor(zval *zvalue);
int crx_init_config(void);
int crx_shutdown_config(void);
void crx_ini_register_extensions(void);
CRXAPI zval *cfg_get_entry_ex(crex_string *name);
CRXAPI zval *cfg_get_entry(const char *name, size_t name_length);
CRXAPI int cfg_get_long(const char *varname, crex_long *result);
CRXAPI int cfg_get_double(const char *varname, double *result);
CRXAPI int cfg_get_string(const char *varname, char **result);
CRXAPI int crx_parse_user_ini_file(const char *dirname, const char *ini_filename, HashTable *target_hash);
CRXAPI void crx_ini_activate_config(HashTable *source_hash, int modify_type, int stage);
CRXAPI int crx_ini_has_per_dir_config(void);
CRXAPI int crx_ini_has_per_host_config(void);
CRXAPI void crx_ini_activate_per_dir_config(char *path, size_t path_len);
CRXAPI void crx_ini_activate_per_host_config(const char *host, size_t host_len);
CRXAPI HashTable* crx_ini_get_configuration_hash(void);
END_EXTERN_C()

#define CRX_INI_USER	CREX_INI_USER
#define CRX_INI_PERDIR	CREX_INI_PERDIR
#define CRX_INI_SYSTEM	CREX_INI_SYSTEM

#define CRX_INI_ALL 	CREX_INI_ALL

#define crx_ini_entry	crex_ini_entry

#define CRX_INI_MH		CREX_INI_MH
#define CRX_INI_DISP	CREX_INI_DISP

#define CRX_INI_BEGIN		CREX_INI_BEGIN
#define CRX_INI_END			CREX_INI_END

#define CRX_INI_ENTRY3_EX	CREX_INI_ENTRY3_EX
#define CRX_INI_ENTRY3		CREX_INI_ENTRY3
#define CRX_INI_ENTRY2_EX	CREX_INI_ENTRY2_EX
#define CRX_INI_ENTRY2		CREX_INI_ENTRY2
#define CRX_INI_ENTRY1_EX	CREX_INI_ENTRY1_EX
#define CRX_INI_ENTRY1		CREX_INI_ENTRY1
#define CRX_INI_ENTRY_EX	CREX_INI_ENTRY_EX
#define CRX_INI_ENTRY		CREX_INI_ENTRY

#define STD_CRX_INI_ENTRY		STD_CREX_INI_ENTRY
#define STD_CRX_INI_ENTRY_EX	STD_CREX_INI_ENTRY_EX
#define STD_CRX_INI_BOOLEAN		STD_CREX_INI_BOOLEAN

#define CRX_INI_DISPLAY_ORIG	CREX_INI_DISPLAY_ORIG
#define CRX_INI_DISPLAY_ACTIVE	CREX_INI_DISPLAY_ACTIVE

#define CRX_INI_STAGE_STARTUP		CREX_INI_STAGE_STARTUP
#define CRX_INI_STAGE_SHUTDOWN		CREX_INI_STAGE_SHUTDOWN
#define CRX_INI_STAGE_ACTIVATE		CREX_INI_STAGE_ACTIVATE
#define CRX_INI_STAGE_DEACTIVATE	CREX_INI_STAGE_DEACTIVATE
#define CRX_INI_STAGE_RUNTIME		CREX_INI_STAGE_RUNTIME
#define CRX_INI_STAGE_HTACCESS		CREX_INI_STAGE_HTACCESS

#define crx_ini_boolean_displayer_cb	crex_ini_boolean_displayer_cb
#define crx_ini_color_displayer_cb		crex_ini_color_displayer_cb

#define crx_alter_ini_entry		crex_alter_ini_entry

#define crx_ini_long	crex_ini_long
#define crx_ini_double	crex_ini_double
#define crx_ini_string	crex_ini_string

#endif /* CRX_INI_H */
