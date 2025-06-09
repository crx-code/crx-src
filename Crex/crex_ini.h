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
   | Author: Zeev Suraski <zeev@crx.net>                                  |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_INI_H
#define CREX_INI_H

#define CREX_INI_USER	(1<<0)
#define CREX_INI_PERDIR	(1<<1)
#define CREX_INI_SYSTEM	(1<<2)

#define CREX_INI_ALL (CREX_INI_USER|CREX_INI_PERDIR|CREX_INI_SYSTEM)

#define CREX_INI_MH(name) int name(crex_ini_entry *entry, crex_string *new_value, void *mh_arg1, void *mh_arg2, void *mh_arg3, int stage)
#define CREX_INI_DISP(name) CREX_COLD void name(crex_ini_entry *ini_entry, int type)

typedef struct _crex_ini_entry_def {
	const char *name;
	CREX_INI_MH((*on_modify));
	void *mh_arg1;
	void *mh_arg2;
	void *mh_arg3;
	const char *value;
	void (*displayer)(crex_ini_entry *ini_entry, int type);

	uint32_t value_length;
	uint16_t name_length;
	uint8_t modifiable;
} crex_ini_entry_def;

struct _crex_ini_entry {
	crex_string *name;
	CREX_INI_MH((*on_modify));
	void *mh_arg1;
	void *mh_arg2;
	void *mh_arg3;
	crex_string *value;
	crex_string *orig_value;
	void (*displayer)(crex_ini_entry *ini_entry, int type);

	int module_number;

	uint8_t modifiable;
	uint8_t orig_modifiable;
	uint8_t modified;

};

BEGIN_EXTERN_C()
CREX_API void crex_ini_startup(void);
CREX_API void crex_ini_shutdown(void);
CREX_API void crex_ini_global_shutdown(void);
CREX_API void crex_ini_deactivate(void);
CREX_API void crex_ini_dtor(HashTable *ini_directives);

CREX_API void crex_copy_ini_directives(void);

CREX_API void crex_ini_sort_entries(void);

CREX_API crex_result crex_register_ini_entries(const crex_ini_entry_def *ini_entry, int module_number);
CREX_API crex_result crex_register_ini_entries_ex(const crex_ini_entry_def *ini_entry, int module_number, int module_type);
CREX_API void crex_unregister_ini_entries(int module_number);
CREX_API void crex_unregister_ini_entries_ex(int module_number, int module_type);
CREX_API void crex_ini_refresh_caches(int stage);
CREX_API crex_result crex_alter_ini_entry(crex_string *name, crex_string *new_value, int modify_type, int stage);
CREX_API crex_result crex_alter_ini_entry_ex(crex_string *name, crex_string *new_value, int modify_type, int stage, bool force_change);
CREX_API crex_result crex_alter_ini_entry_chars(crex_string *name, const char *value, size_t value_length, int modify_type, int stage);
CREX_API crex_result crex_alter_ini_entry_chars_ex(crex_string *name, const char *value, size_t value_length, int modify_type, int stage, int force_change);
CREX_API crex_result crex_restore_ini_entry(crex_string *name, int stage);
CREX_API void display_ini_entries(crex_module_entry *module);

CREX_API crex_long crex_ini_long(const char *name, size_t name_length, int orig);
CREX_API double crex_ini_double(const char *name, size_t name_length, int orig);
CREX_API char *crex_ini_string(const char *name, size_t name_length, int orig);
CREX_API char *crex_ini_string_ex(const char *name, size_t name_length, int orig, bool *exists);
CREX_API crex_string *crex_ini_str(const char *name, size_t name_length, bool orig);
CREX_API crex_string *crex_ini_str_ex(const char *name, size_t name_length, bool orig, bool *exists);
CREX_API crex_string *crex_ini_get_value(crex_string *name);
CREX_API bool crex_ini_parse_bool(crex_string *str);

/**
 * Parses an ini quantity
 *
 * The value parameter must be a string in the form
 *
 *     sign? digits ws* multiplier?
 *
 * with
 *
 *     sign: [+-]
 *     digit: [0-9]
 *     digits: digit+
 *     ws: [ \t\n\r\v\f]
 *     multiplier: [KMG]
 *
 * Leading and trailing whitespaces are ignored.
 *
 * If the string is empty or consists only of only whitespaces, 0 is returned.
 *
 * Digits is parsed as decimal unless the first digit is '0', in which case
 * digits is parsed as octal.
 *
 * The multiplier is case-insensitive. K, M, and G multiply the quantity by
 * 2**10, 2**20, and 2**30, respectively.
 *
 * For backwards compatibility, ill-formatted values are handled as follows:
 * - No leading digits: value is treated as '0'
 * - Invalid multiplier: multiplier is ignored
 * - Invalid characters between digits and multiplier: invalid characters are
 *   ignored
 * - Integer overflow: The result of the overflow is returned
 *
 * In any of these cases an error string is stored in *errstr (caller must
 * release it), otherwise *errstr is set to NULL.
 */
CREX_API crex_long crex_ini_parse_quantity(crex_string *value, crex_string **errstr);

/**
 * Unsigned variant of crex_ini_parse_quantity
 */
CREX_API crex_ulong crex_ini_parse_uquantity(crex_string *value, crex_string **errstr);

CREX_API crex_long crex_ini_parse_quantity_warn(crex_string *value, crex_string *setting);

CREX_API crex_ulong crex_ini_parse_uquantity_warn(crex_string *value, crex_string *setting);

CREX_API crex_result crex_ini_register_displayer(const char *name, uint32_t name_length, void (*displayer)(crex_ini_entry *ini_entry, int type));

CREX_API CREX_INI_DISP(crex_ini_boolean_displayer_cb);
CREX_API CREX_INI_DISP(crex_ini_color_displayer_cb);
CREX_API CREX_INI_DISP(display_link_numbers);
END_EXTERN_C()

#define CREX_INI_BEGIN()		static const crex_ini_entry_def ini_entries[] = {
#define CREX_INI_END()		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0} };

#define CREX_INI_ENTRY3_EX(name, default_value, modifiable, on_modify, arg1, arg2, arg3, displayer) \
	{ name, on_modify, arg1, arg2, arg3, default_value, displayer, sizeof(default_value)-1, sizeof(name)-1, modifiable },

#define CREX_INI_ENTRY3(name, default_value, modifiable, on_modify, arg1, arg2, arg3) \
	CREX_INI_ENTRY3_EX(name, default_value, modifiable, on_modify, arg1, arg2, arg3, NULL)

#define CREX_INI_ENTRY2_EX(name, default_value, modifiable, on_modify, arg1, arg2, displayer) \
	CREX_INI_ENTRY3_EX(name, default_value, modifiable, on_modify, arg1, arg2, NULL, displayer)

#define CREX_INI_ENTRY2(name, default_value, modifiable, on_modify, arg1, arg2) \
	CREX_INI_ENTRY2_EX(name, default_value, modifiable, on_modify, arg1, arg2, NULL)

#define CREX_INI_ENTRY1_EX(name, default_value, modifiable, on_modify, arg1, displayer) \
	CREX_INI_ENTRY3_EX(name, default_value, modifiable, on_modify, arg1, NULL, NULL, displayer)

#define CREX_INI_ENTRY1(name, default_value, modifiable, on_modify, arg1) \
	CREX_INI_ENTRY1_EX(name, default_value, modifiable, on_modify, arg1, NULL)

#define CREX_INI_ENTRY_EX(name, default_value, modifiable, on_modify, displayer) \
	CREX_INI_ENTRY3_EX(name, default_value, modifiable, on_modify, NULL, NULL, NULL, displayer)

#define CREX_INI_ENTRY(name, default_value, modifiable, on_modify) \
	CREX_INI_ENTRY_EX(name, default_value, modifiable, on_modify, NULL)

#ifdef ZTS
#define STD_CREX_INI_ENTRY(name, default_value, modifiable, on_modify, property_name, struct_type, struct_ptr) \
	CREX_INI_ENTRY2(name, default_value, modifiable, on_modify, (void *) XtOffsetOf(struct_type, property_name), (void *) &struct_ptr##_id)
#define STD_CREX_INI_ENTRY_EX(name, default_value, modifiable, on_modify, property_name, struct_type, struct_ptr, displayer) \
	CREX_INI_ENTRY2_EX(name, default_value, modifiable, on_modify, (void *) XtOffsetOf(struct_type, property_name), (void *) &struct_ptr##_id, displayer)
#define STD_CREX_INI_BOOLEAN(name, default_value, modifiable, on_modify, property_name, struct_type, struct_ptr) \
	CREX_INI_ENTRY3_EX(name, default_value, modifiable, on_modify, (void *) XtOffsetOf(struct_type, property_name), (void *) &struct_ptr##_id, NULL, crex_ini_boolean_displayer_cb)
#else
#define STD_CREX_INI_ENTRY(name, default_value, modifiable, on_modify, property_name, struct_type, struct_ptr) \
	CREX_INI_ENTRY2(name, default_value, modifiable, on_modify, (void *) XtOffsetOf(struct_type, property_name), (void *) &struct_ptr)
#define STD_CREX_INI_ENTRY_EX(name, default_value, modifiable, on_modify, property_name, struct_type, struct_ptr, displayer) \
	CREX_INI_ENTRY2_EX(name, default_value, modifiable, on_modify, (void *) XtOffsetOf(struct_type, property_name), (void *) &struct_ptr, displayer)
#define STD_CREX_INI_BOOLEAN(name, default_value, modifiable, on_modify, property_name, struct_type, struct_ptr) \
	CREX_INI_ENTRY3_EX(name, default_value, modifiable, on_modify, (void *) XtOffsetOf(struct_type, property_name), (void *) &struct_ptr, NULL, crex_ini_boolean_displayer_cb)
#endif

#define INI_INT(name) crex_ini_long((name), strlen(name), 0)
#define INI_FLT(name) crex_ini_double((name), strlen(name), 0)
#define INI_STR(name) crex_ini_string_ex((name), strlen(name), 0, NULL)
#define INI_BOOL(name) ((bool) INI_INT(name))

#define INI_ORIG_INT(name)	crex_ini_long((name), strlen(name), 1)
#define INI_ORIG_FLT(name)	crex_ini_double((name), strlen(name), 1)
#define INI_ORIG_STR(name)	crex_ini_string((name), strlen(name), 1)
#define INI_ORIG_BOOL(name) ((bool) INI_ORIG_INT(name))

#define REGISTER_INI_ENTRIES() crex_register_ini_entries_ex(ini_entries, module_number, type)
#define UNREGISTER_INI_ENTRIES() crex_unregister_ini_entries_ex(module_number, type)
#define DISPLAY_INI_ENTRIES() display_ini_entries(crex_module)

#define REGISTER_INI_DISPLAYER(name, displayer) crex_ini_register_displayer((name), strlen(name), displayer)
#define REGISTER_INI_BOOLEAN(name) REGISTER_INI_DISPLAYER(name, crex_ini_boolean_displayer_cb)

/* Standard message handlers */
BEGIN_EXTERN_C()
CREX_API CREX_INI_MH(OnUpdateBool);
CREX_API CREX_INI_MH(OnUpdateLong);
CREX_API CREX_INI_MH(OnUpdateLongGEZero);
CREX_API CREX_INI_MH(OnUpdateReal);
/* char* versions */
CREX_API CREX_INI_MH(OnUpdateString);
CREX_API CREX_INI_MH(OnUpdateStringUnempty);
/* crex_string* versions */
CREX_API CREX_INI_MH(OnUpdateStr);
CREX_API CREX_INI_MH(OnUpdateStrNotEmpty);
END_EXTERN_C()

#define CREX_INI_DISPLAY_ORIG	1
#define CREX_INI_DISPLAY_ACTIVE	2

#define CREX_INI_STAGE_STARTUP		(1<<0)
#define CREX_INI_STAGE_SHUTDOWN		(1<<1)
#define CREX_INI_STAGE_ACTIVATE		(1<<2)
#define CREX_INI_STAGE_DEACTIVATE	(1<<3)
#define CREX_INI_STAGE_RUNTIME		(1<<4)
#define CREX_INI_STAGE_HTACCESS		(1<<5)

#define CREX_INI_STAGE_IN_REQUEST   (CREX_INI_STAGE_ACTIVATE|CREX_INI_STAGE_DEACTIVATE|CREX_INI_STAGE_RUNTIME|CREX_INI_STAGE_HTACCESS)

/* INI parsing engine */
typedef void (*crex_ini_parser_cb_t)(zval *arg1, zval *arg2, zval *arg3, int callback_type, void *arg);
BEGIN_EXTERN_C()
CREX_API crex_result crex_parse_ini_file(crex_file_handle *fh, bool unbuffered_errors, int scanner_mode, crex_ini_parser_cb_t ini_parser_cb, void *arg);
CREX_API crex_result crex_parse_ini_string(const char *str, bool unbuffered_errors, int scanner_mode, crex_ini_parser_cb_t ini_parser_cb, void *arg);
END_EXTERN_C()

/* INI entries */
#define CREX_INI_PARSER_ENTRY     1 /* Normal entry: foo = bar */
#define CREX_INI_PARSER_SECTION	  2 /* Section: [foobar] */
#define CREX_INI_PARSER_POP_ENTRY 3 /* Offset entry: foo[] = bar */

typedef struct _crex_ini_parser_param {
	crex_ini_parser_cb_t ini_parser_cb;
	void *arg;
} crex_ini_parser_param;

#ifndef ZTS
# define CREX_INI_GET_BASE() ((char *) mh_arg2)
#else
# define CREX_INI_GET_BASE() ((char *) ts_resource(*((int *) mh_arg2)))
#endif

#define CREX_INI_GET_ADDR() (CREX_INI_GET_BASE() + (size_t) mh_arg1)

#endif /* CREX_INI_H */
