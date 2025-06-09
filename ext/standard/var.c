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
   | Authors: Jani Lehtimäki <jkl@njet.net>                               |
   |          Thies C. Arntzen <thies@thieso.net>                         |
   |          Sascha Schumann <sascha@schumann.cx>                        |
   +----------------------------------------------------------------------+
*/

/* {{{ includes */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "crx.h"
#include "crx_string.h"
#include "crx_var.h"
#include "crex_smart_str.h"
#include "basic_functions.h"
#include "crx_incomplete_class.h"
#include "crex_enum.h"
#include "crex_exceptions.h"
/* }}} */

struct crx_serialize_data {
	HashTable ht;
	uint32_t n;
};

#define COMMON (is_ref ? "&" : "")

static void crx_array_element_dump(zval *zv, crex_ulong index, crex_string *key, int level) /* {{{ */
{
	if (key == NULL) { /* numeric key */
		crx_printf("%*c[" CREX_LONG_FMT "]=>\n", level + 1, ' ', index);
	} else { /* string key */
		crx_printf("%*c[\"", level + 1, ' ');
		CRXWRITE(ZSTR_VAL(key), ZSTR_LEN(key));
		crx_printf("\"]=>\n");
	}
	crx_var_dump(zv, level + 2);
}
/* }}} */

static void crx_object_property_dump(crex_property_info *prop_info, zval *zv, crex_ulong index, crex_string *key, int level) /* {{{ */
{
	const char *prop_name, *class_name;

	if (key == NULL) { /* numeric key */
		crx_printf("%*c[" CREX_LONG_FMT "]=>\n", level + 1, ' ', index);
	} else { /* string key */
		int unmangle = crex_unmangle_property_name(key, &class_name, &prop_name);
		crx_printf("%*c[", level + 1, ' ');

		if (class_name && unmangle == SUCCESS) {
			if (class_name[0] == '*') {
				crx_printf("\"%s\":protected", prop_name);
			} else {
				crx_printf("\"%s\":\"%s\":private", prop_name, class_name);
			}
		} else {
			crx_printf("\"");
			CRXWRITE(ZSTR_VAL(key), ZSTR_LEN(key));
			crx_printf("\"");
		}
		CREX_PUTS("]=>\n");
	}

	if (C_TYPE_P(zv) == IS_UNDEF) {
		CREX_ASSERT(CREX_TYPE_IS_SET(prop_info->type));
		crex_string *type_str = crex_type_to_string(prop_info->type);
		crx_printf("%*cuninitialized(%s)\n",
			level + 1, ' ', ZSTR_VAL(type_str));
		crex_string_release(type_str);
	} else {
		crx_var_dump(zv, level + 2);
	}
}
/* }}} */

CRXAPI void crx_var_dump(zval *struc, int level) /* {{{ */
{
	HashTable *myht;
	crex_string *class_name;
	int is_ref = 0;
	crex_ulong num;
	crex_string *key;
	zval *val;
	uint32_t count;

	if (level > 1) {
		crx_printf("%*c", level - 1, ' ');
	}

again:
	switch (C_TYPE_P(struc)) {
		case IS_FALSE:
			crx_printf("%sbool(false)\n", COMMON);
			break;
		case IS_TRUE:
			crx_printf("%sbool(true)\n", COMMON);
			break;
		case IS_NULL:
			crx_printf("%sNULL\n", COMMON);
			break;
		case IS_LONG:
			crx_printf("%sint(" CREX_LONG_FMT ")\n", COMMON, C_LVAL_P(struc));
			break;
		case IS_DOUBLE:
			crx_printf_unchecked("%sfloat(%.*H)\n", COMMON, (int) PG(serialize_precision), C_DVAL_P(struc));
			break;
		case IS_STRING:
			crx_printf("%sstring(%zd) \"", COMMON, C_STRLEN_P(struc));
			CRXWRITE(C_STRVAL_P(struc), C_STRLEN_P(struc));
			PUTS("\"\n");
			break;
		case IS_ARRAY:
			myht = C_ARRVAL_P(struc);
			if (!(GC_FLAGS(myht) & GC_IMMUTABLE)) {
				if (GC_IS_RECURSIVE(myht)) {
					PUTS("*RECURSION*\n");
					return;
				}
				GC_ADDREF(myht);
				GC_PROTECT_RECURSION(myht);
			}
			count = crex_hash_num_elements(myht);
			crx_printf("%sarray(%d) {\n", COMMON, count);
			CREX_HASH_FOREACH_KEY_VAL(myht, num, key, val) {
				crx_array_element_dump(val, num, key, level);
			} CREX_HASH_FOREACH_END();
			if (!(GC_FLAGS(myht) & GC_IMMUTABLE)) {
				GC_UNPROTECT_RECURSION(myht);
				GC_DELREF(myht);
			}
			if (level > 1) {
				crx_printf("%*c", level-1, ' ');
			}
			PUTS("}\n");
			break;
		case IS_OBJECT: {
			crex_class_entry *ce = C_OBJCE_P(struc);
			if (ce->ce_flags & CREX_ACC_ENUM) {
				zval *case_name_zval = crex_enum_fetch_case_name(C_OBJ_P(struc));
				crx_printf("%senum(%s::%s)\n", COMMON, ZSTR_VAL(ce->name), C_STRVAL_P(case_name_zval));
				return;
			}
			crex_object *zobj = C_OBJ_P(struc);
			uint32_t *guard = crex_get_recursion_guard(zobj);
			if (CREX_GUARD_OR_GC_IS_RECURSIVE(guard, DEBUG, zobj)) {
				PUTS("*RECURSION*\n");
				return;
			}
			CREX_GUARD_OR_GC_PROTECT_RECURSION(guard, DEBUG, zobj);

			myht = crex_get_properties_for(struc, CREX_PROP_PURPOSE_DEBUG);
			class_name = C_OBJ_HANDLER_P(struc, get_class_name)(C_OBJ_P(struc));
			crx_printf("%sobject(%s)#%d (%d) {\n", COMMON, ZSTR_VAL(class_name), C_OBJ_HANDLE_P(struc), myht ? crex_array_count(myht) : 0);
			crex_string_release_ex(class_name, 0);

			if (myht) {
				crex_ulong num;
				crex_string *key;
				zval *val;

				CREX_HASH_FOREACH_KEY_VAL(myht, num, key, val) {
					crex_property_info *prop_info = NULL;

					if (C_TYPE_P(val) == IS_INDIRECT) {
						val = C_INDIRECT_P(val);
						if (key) {
							prop_info = crex_get_typed_property_info_for_slot(C_OBJ_P(struc), val);
						}
					}

					if (!C_ISUNDEF_P(val) || prop_info) {
						crx_object_property_dump(prop_info, val, num, key, level);
					}
				} CREX_HASH_FOREACH_END();
				crex_release_properties(myht);
			}
			if (level > 1) {
				crx_printf("%*c", level-1, ' ');
			}
			PUTS("}\n");
			CREX_GUARD_OR_GC_UNPROTECT_RECURSION(guard, DEBUG, zobj);
			break;
		}
		case IS_RESOURCE: {
			const char *type_name = crex_rsrc_list_get_rsrc_type(C_RES_P(struc));
			crx_printf("%sresource(" CREX_LONG_FMT ") of type (%s)\n", COMMON, C_RES_P(struc)->handle, type_name ? type_name : "Unknown");
			break;
		}
		case IS_REFERENCE:
			//??? hide references with refcount==1 (for compatibility)
			if (C_REFCOUNT_P(struc) > 1) {
				is_ref = 1;
			}
			struc = C_REFVAL_P(struc);
			goto again;
			break;
		default:
			crx_printf("%sUNKNOWN:0\n", COMMON);
			break;
	}
}
/* }}} */

/* {{{ Dumps a string representation of variable to output */
CRX_FUNCTION(var_dump)
{
	zval *args;
	int argc;
	int	i;

	CREX_PARSE_PARAMETERS_START(1, -1)
		C_PARAM_VARIADIC('+', args, argc)
	CREX_PARSE_PARAMETERS_END();

	for (i = 0; i < argc; i++) {
		crx_var_dump(&args[i], 1);
	}
}
/* }}} */

static void zval_array_element_dump(zval *zv, crex_ulong index, crex_string *key, int level) /* {{{ */
{
	if (key == NULL) { /* numeric key */
		crx_printf("%*c[" CREX_LONG_FMT "]=>\n", level + 1, ' ', index);
	} else { /* string key */
		crx_printf("%*c[\"", level + 1, ' ');
		CRXWRITE(ZSTR_VAL(key), ZSTR_LEN(key));
		crx_printf("\"]=>\n");
	}
	crx_debug_zval_dump(zv, level + 2);
}
/* }}} */

static void zval_object_property_dump(crex_property_info *prop_info, zval *zv, crex_ulong index, crex_string *key, int level) /* {{{ */
{
	const char *prop_name, *class_name;

	if (key == NULL) { /* numeric key */
		crx_printf("%*c[" CREX_LONG_FMT "]=>\n", level + 1, ' ', index);
	} else { /* string key */
		crex_unmangle_property_name(key, &class_name, &prop_name);
		crx_printf("%*c[", level + 1, ' ');

		if (class_name) {
			if (class_name[0] == '*') {
				crx_printf("\"%s\":protected", prop_name);
			} else {
				crx_printf("\"%s\":\"%s\":private", prop_name, class_name);
			}
		} else {
			crx_printf("\"%s\"", prop_name);
		}
		CREX_PUTS("]=>\n");
	}
	if (prop_info && C_TYPE_P(zv) == IS_UNDEF) {
		crex_string *type_str = crex_type_to_string(prop_info->type);
		crx_printf("%*cuninitialized(%s)\n",
			level + 1, ' ', ZSTR_VAL(type_str));
		crex_string_release(type_str);
	} else {
		crx_debug_zval_dump(zv, level + 2);
	}
}
/* }}} */

CRXAPI void crx_debug_zval_dump(zval *struc, int level) /* {{{ */
{
	HashTable *myht = NULL;
	crex_string *class_name;
	crex_ulong index;
	crex_string *key;
	zval *val;
	uint32_t count;

	if (level > 1) {
		crx_printf("%*c", level - 1, ' ');
	}

	switch (C_TYPE_P(struc)) {
	case IS_FALSE:
		PUTS("bool(false)\n");
		break;
	case IS_TRUE:
		PUTS("bool(true)\n");
		break;
	case IS_NULL:
		PUTS("NULL\n");
		break;
	case IS_LONG:
		crx_printf("int(" CREX_LONG_FMT ")\n", C_LVAL_P(struc));
		break;
	case IS_DOUBLE:
		crx_printf_unchecked("float(%.*H)\n", (int) PG(serialize_precision), C_DVAL_P(struc));
		break;
	case IS_STRING:
		crx_printf("string(%zd) \"", C_STRLEN_P(struc));
		CRXWRITE(C_STRVAL_P(struc), C_STRLEN_P(struc));
		if (C_REFCOUNTED_P(struc)) {
			crx_printf("\" refcount(%u)\n", C_REFCOUNT_P(struc));
		} else {
			PUTS("\" interned\n");
		}
		break;
	case IS_ARRAY:
		myht = C_ARRVAL_P(struc);
		if (!(GC_FLAGS(myht) & GC_IMMUTABLE)) {
			if (GC_IS_RECURSIVE(myht)) {
				PUTS("*RECURSION*\n");
				return;
			}
			GC_ADDREF(myht);
			GC_PROTECT_RECURSION(myht);
		}
		count = crex_hash_num_elements(myht);
		if (C_REFCOUNTED_P(struc)) {
			/* -1 because of ADDREF above. */
			crx_printf("array(%d) refcount(%u){\n", count, C_REFCOUNT_P(struc) - 1);
		} else {
			crx_printf("array(%d) interned {\n", count);
		}
		CREX_HASH_FOREACH_KEY_VAL(myht, index, key, val) {
			zval_array_element_dump(val, index, key, level);
		} CREX_HASH_FOREACH_END();
		if (!(GC_FLAGS(myht) & GC_IMMUTABLE)) {
			GC_UNPROTECT_RECURSION(myht);
			GC_DELREF(myht);
		}
		if (level > 1) {
			crx_printf("%*c", level - 1, ' ');
		}
		PUTS("}\n");
		break;
	case IS_OBJECT: {
		/* Check if this is already recursing on the object before calling crex_get_properties_for,
		 * to allow infinite recursion detection to work even if classes return temporary arrays,
		 * and to avoid the need to update the properties table in place to reflect the state
		 * if the result won't be used. (https://github.com/crx/crx-src/issues/8044) */
		crex_object *zobj = C_OBJ_P(struc);
		uint32_t *guard = crex_get_recursion_guard(zobj);
		if (CREX_GUARD_OR_GC_IS_RECURSIVE(guard, DEBUG, zobj)) {
			PUTS("*RECURSION*\n");
			return;
		}
		CREX_GUARD_OR_GC_PROTECT_RECURSION(guard, DEBUG, zobj);

		myht = crex_get_properties_for(struc, CREX_PROP_PURPOSE_DEBUG);
		class_name = C_OBJ_HANDLER_P(struc, get_class_name)(C_OBJ_P(struc));
		crx_printf("object(%s)#%d (%d) refcount(%u){\n", ZSTR_VAL(class_name), C_OBJ_HANDLE_P(struc), myht ? crex_array_count(myht) : 0, C_REFCOUNT_P(struc));
		crex_string_release_ex(class_name, 0);
		if (myht) {
			CREX_HASH_FOREACH_KEY_VAL(myht, index, key, val) {
				crex_property_info *prop_info = NULL;

				if (C_TYPE_P(val) == IS_INDIRECT) {
					val = C_INDIRECT_P(val);
					if (key) {
						prop_info = crex_get_typed_property_info_for_slot(C_OBJ_P(struc), val);
					}
				}

				if (!C_ISUNDEF_P(val) || prop_info) {
					zval_object_property_dump(prop_info, val, index, key, level);
				}
			} CREX_HASH_FOREACH_END();
			crex_release_properties(myht);
		}
		if (level > 1) {
			crx_printf("%*c", level - 1, ' ');
		}
		PUTS("}\n");
		CREX_GUARD_OR_GC_UNPROTECT_RECURSION(guard, DEBUG, zobj);
		break;
	}
	case IS_RESOURCE: {
		const char *type_name = crex_rsrc_list_get_rsrc_type(C_RES_P(struc));
		crx_printf("resource(" CREX_LONG_FMT ") of type (%s) refcount(%u)\n", C_RES_P(struc)->handle, type_name ? type_name : "Unknown", C_REFCOUNT_P(struc));
		break;
	}
	case IS_REFERENCE:
		crx_printf("reference refcount(%u) {\n", C_REFCOUNT_P(struc));
		crx_debug_zval_dump(C_REFVAL_P(struc), level + 2);
		if (level > 1) {
			crx_printf("%*c", level - 1, ' ');
		}
		PUTS("}\n");
		break;
	default:
		PUTS("UNKNOWN:0\n");
		break;
	}
}
/* }}} */

/* {{{ Dumps a string representation of an internal crex value to output. */
CRX_FUNCTION(debug_zval_dump)
{
	zval *args;
	int argc;
	int	i;

	CREX_PARSE_PARAMETERS_START(1, -1)
		C_PARAM_VARIADIC('+', args, argc)
	CREX_PARSE_PARAMETERS_END();

	for (i = 0; i < argc; i++) {
		crx_debug_zval_dump(&args[i], 1);
	}
}
/* }}} */

#define buffer_append_spaces(buf, num_spaces) \
	do { \
		char *tmp_spaces; \
		size_t tmp_spaces_len; \
		tmp_spaces_len = spprintf(&tmp_spaces, 0,"%*c", num_spaces, ' '); \
		smart_str_appendl(buf, tmp_spaces, tmp_spaces_len); \
		efree(tmp_spaces); \
	} while(0);

static void crx_array_element_export(zval *zv, crex_ulong index, crex_string *key, int level, smart_str *buf) /* {{{ */
{
	if (key == NULL) { /* numeric key */
		buffer_append_spaces(buf, level+1);
		smart_str_append_long(buf, (crex_long) index);
		smart_str_appendl(buf, " => ", 4);

	} else { /* string key */
		crex_string *tmp_str;
		crex_string *ckey = crx_addcslashes(key, "'\\", 2);
		tmp_str = crx_str_to_str(ZSTR_VAL(ckey), ZSTR_LEN(ckey), "\0", 1, "' . \"\\0\" . '", 12);

		buffer_append_spaces(buf, level + 1);

		smart_str_appendc(buf, '\'');
		smart_str_append(buf, tmp_str);
		smart_str_appendl(buf, "' => ", 5);

		crex_string_free(ckey);
		crex_string_free(tmp_str);
	}
	crx_var_export_ex(zv, level + 2, buf);

	smart_str_appendc(buf, ',');
	smart_str_appendc(buf, '\n');
}
/* }}} */

static void crx_object_element_export(zval *zv, crex_ulong index, crex_string *key, int level, smart_str *buf) /* {{{ */
{
	buffer_append_spaces(buf, level + 2);
	if (key != NULL) {
		const char *class_name, *prop_name;
		size_t prop_name_len;
		crex_string *pname_esc;

		crex_unmangle_property_name_ex(key, &class_name, &prop_name, &prop_name_len);
		pname_esc = crx_addcslashes_str(prop_name, prop_name_len, "'\\", 2);

		smart_str_appendc(buf, '\'');
		smart_str_append(buf, pname_esc);
		smart_str_appendc(buf, '\'');
		crex_string_release_ex(pname_esc, 0);
	} else {
		smart_str_append_long(buf, (crex_long) index);
	}
	smart_str_appendl(buf, " => ", 4);
	crx_var_export_ex(zv, level + 2, buf);
	smart_str_appendc(buf, ',');
	smart_str_appendc(buf, '\n');
}
/* }}} */

CRXAPI void crx_var_export_ex(zval *struc, int level, smart_str *buf) /* {{{ */
{
	HashTable *myht;
	crex_string *ztmp, *ztmp2;
	crex_ulong index;
	crex_string *key;
	zval *val;

again:
	switch (C_TYPE_P(struc)) {
		case IS_FALSE:
			smart_str_appendl(buf, "false", 5);
			break;
		case IS_TRUE:
			smart_str_appendl(buf, "true", 4);
			break;
		case IS_NULL:
			smart_str_appendl(buf, "NULL", 4);
			break;
		case IS_LONG:
			/* INT_MIN as a literal will be parsed as a float. Emit something like
			 * -9223372036854775807-1 to avoid this. */
			if (C_LVAL_P(struc) == CREX_LONG_MIN) {
				smart_str_append_long(buf, CREX_LONG_MIN+1);
				smart_str_appends(buf, "-1");
				break;
			}
			smart_str_append_long(buf, C_LVAL_P(struc));
			break;
		case IS_DOUBLE:
			smart_str_append_double(
				buf, C_DVAL_P(struc), (int) PG(serialize_precision), /* zero_fraction */ true);
			break;
		case IS_STRING:
			ztmp = crx_addcslashes(C_STR_P(struc), "'\\", 2);
			ztmp2 = crx_str_to_str(ZSTR_VAL(ztmp), ZSTR_LEN(ztmp), "\0", 1, "' . \"\\0\" . '", 12);

			smart_str_appendc(buf, '\'');
			smart_str_append(buf, ztmp2);
			smart_str_appendc(buf, '\'');

			crex_string_free(ztmp);
			crex_string_free(ztmp2);
			break;
		case IS_ARRAY:
			myht = C_ARRVAL_P(struc);
			if (!(GC_FLAGS(myht) & GC_IMMUTABLE)) {
				if (GC_IS_RECURSIVE(myht)) {
					smart_str_appendl(buf, "NULL", 4);
					crex_error(E_WARNING, "var_export does not handle circular references");
					return;
				}
				GC_ADDREF(myht);
				GC_PROTECT_RECURSION(myht);
			}
			if (level > 1) {
				smart_str_appendc(buf, '\n');
				buffer_append_spaces(buf, level - 1);
			}
			smart_str_appendl(buf, "array (\n", 8);
			CREX_HASH_FOREACH_KEY_VAL(myht, index, key, val) {
				crx_array_element_export(val, index, key, level, buf);
			} CREX_HASH_FOREACH_END();
			if (!(GC_FLAGS(myht) & GC_IMMUTABLE)) {
				GC_UNPROTECT_RECURSION(myht);
				GC_DELREF(myht);
			}
			if (level > 1) {
				buffer_append_spaces(buf, level - 1);
			}
			smart_str_appendc(buf, ')');

			break;

		case IS_OBJECT: {
			/* Check if this is already recursing on the object before calling crex_get_properties_for,
			 * to allow infinite recursion detection to work even if classes return temporary arrays,
			 * and to avoid the need to update the properties table in place to reflect the state
			 * if the result won't be used. (https://github.com/crx/crx-src/issues/8044) */
			crex_object *zobj = C_OBJ_P(struc);
			uint32_t *guard = crex_get_recursion_guard(zobj);
			if (CREX_GUARD_OR_GC_IS_RECURSIVE(guard, EXPORT, zobj)) {
				smart_str_appendl(buf, "NULL", 4);
				crex_error(E_WARNING, "var_export does not handle circular references");
				return;
			}
			CREX_GUARD_OR_GC_PROTECT_RECURSION(guard, EXPORT, zobj);
			myht = crex_get_properties_for(struc, CREX_PROP_PURPOSE_VAR_EXPORT);
			if (level > 1) {
				smart_str_appendc(buf, '\n');
				buffer_append_spaces(buf, level - 1);
			}

			crex_class_entry *ce = C_OBJCE_P(struc);
			bool is_enum = ce->ce_flags & CREX_ACC_ENUM;

			/* stdClass has no __set_state method, but can be casted to */
			if (ce == crex_standard_class_def) {
				smart_str_appendl(buf, "(object) array(\n", 16);
			} else {
				smart_str_appendc(buf, '\\');
				smart_str_append(buf, ce->name);
				if (is_enum) {
					crex_object *zobj = C_OBJ_P(struc);
					zval *case_name_zval = crex_enum_fetch_case_name(zobj);
					smart_str_appendl(buf, "::", 2);
					smart_str_append(buf, C_STR_P(case_name_zval));
				} else {
					smart_str_appendl(buf, "::__set_state(array(\n", 21);
				}
			}

			if (myht) {
				if (!is_enum) {
					CREX_HASH_FOREACH_KEY_VAL_IND(myht, index, key, val) {
						crx_object_element_export(val, index, key, level, buf);
					} CREX_HASH_FOREACH_END();
				}
				crex_release_properties(myht);
			}
			CREX_GUARD_OR_GC_UNPROTECT_RECURSION(guard, EXPORT, zobj);
			if (level > 1 && !is_enum) {
				buffer_append_spaces(buf, level - 1);
			}
			if (ce == crex_standard_class_def) {
				smart_str_appendc(buf, ')');
			} else if (!is_enum) {
				smart_str_appendl(buf, "))", 2);
			}

			break;
		}
		case IS_REFERENCE:
			struc = C_REFVAL_P(struc);
			goto again;
			break;
		default:
			smart_str_appendl(buf, "NULL", 4);
			break;
	}
}
/* }}} */

/* FOR BC reasons, this will always perform and then print */
CRXAPI void crx_var_export(zval *struc, int level) /* {{{ */
{
	smart_str buf = {0};
	crx_var_export_ex(struc, level, &buf);
	smart_str_0(&buf);
	CRXWRITE(ZSTR_VAL(buf.s), ZSTR_LEN(buf.s));
	smart_str_free(&buf);
}
/* }}} */

/* {{{ Outputs or returns a string representation of a variable */
CRX_FUNCTION(var_export)
{
	zval *var;
	bool return_output = 0;
	smart_str buf = {0};

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_ZVAL(var)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(return_output)
	CREX_PARSE_PARAMETERS_END();

	crx_var_export_ex(var, 1, &buf);
	smart_str_0 (&buf);

	if (return_output) {
		RETURN_STR(smart_str_extract(&buf));
	} else {
		CRXWRITE(ZSTR_VAL(buf.s), ZSTR_LEN(buf.s));
		smart_str_free(&buf);
	}
}
/* }}} */

static void crx_var_serialize_intern(smart_str *buf, zval *struc, crx_serialize_data_t var_hash, bool in_rcn_array, bool is_root);

/**
 * @param bool in_rcn_array Whether the element appears in a potentially nested array with RC > 1.
 */
static inline crex_long crx_add_var_hash(crx_serialize_data_t data, zval *var, bool in_rcn_array) /* {{{ */
{
	zval *zv;
	crex_ulong key;
	bool is_ref = C_ISREF_P(var);

	data->n += 1;

	if (is_ref) {
		/* pass */
	} else if (C_TYPE_P(var) != IS_OBJECT) {
		return 0;
	} else if (!in_rcn_array
	 && C_REFCOUNT_P(var) == 1
	 && (C_OBJ_P(var)->properties == NULL || GC_REFCOUNT(C_OBJ_P(var)->properties) == 1)) {
		return 0;
	}

	/* References to objects are treated as if the reference didn't exist */
	if (is_ref && C_TYPE_P(C_REFVAL_P(var)) == IS_OBJECT) {
		var = C_REFVAL_P(var);
	}

	/* Index for the variable is stored using the numeric value of the pointer to
	 * the crex_refcounted struct */
	key = (crex_ulong) (uintptr_t) C_COUNTED_P(var);
	zv = crex_hash_index_find(&data->ht, key);

	if (zv) {
		/* References are only counted once, undo the data->n increment above */
		if (is_ref && C_LVAL_P(zv) != -1) {
			data->n -= 1;
		}

		return C_LVAL_P(zv);
	} else {
		zval zv_n;
		ZVAL_LONG(&zv_n, data->n);
		crex_hash_index_add_new(&data->ht, key, &zv_n);

		/* Additionally to the index, we also store the variable, to ensure that it is
		 * not destroyed during serialization and its pointer reused. The variable is
		 * stored at the numeric value of the pointer + 1, which cannot be the location
		 * of another crex_refcounted structure. */
		crex_hash_index_add_new(&data->ht, key + 1, var);
		C_ADDREF_P(var);

		return 0;
	}
}
/* }}} */

static inline void crx_var_serialize_long(smart_str *buf, crex_long val) /* {{{ */
{
	char b[32];
	char *s = crex_print_long_to_buf(b + sizeof(b) - 1, val);
	size_t l = b + sizeof(b) - 1 - s;
	char *res = smart_str_extend(buf, 2 + l + 1);
	memcpy(res, "i:", 2);
	res += 2;
	memcpy(res, s, l);
	res[l] = ';';
}
/* }}} */

static inline void crx_var_serialize_string(smart_str *buf, char *str, size_t len) /* {{{ */
{
	char b[32];
	char *s = crex_print_long_to_buf(b + sizeof(b) - 1, len);
	size_t l = b + sizeof(b) - 1 - s;
	char *res = smart_str_extend(buf, 2 + l + 2 + len + 2);
	memcpy(res, "s:", 2);
	res += 2;
	memcpy(res, s, l);
	res += l;
	memcpy(res, ":\"", 2);
	res += 2;
	memcpy(res, str, len);
	res += len;
	memcpy(res, "\";", 2);
}
/* }}} */

static inline bool crx_var_serialize_class_name(smart_str *buf, zval *struc) /* {{{ */
{
	char b[32];
	CRX_CLASS_ATTRIBUTES;

	CRX_SET_CLASS_ATTRIBUTES(struc);
	size_t class_name_len = ZSTR_LEN(class_name);
	char *s = crex_print_long_to_buf(b + sizeof(b) - 1, class_name_len);
	size_t l = b + sizeof(b) - 1 - s;
	char *res = smart_str_extend(buf, 2 + l + 2 + class_name_len + 2);
	memcpy(res, "O:", 2);
	res += 2;
	memcpy(res, s, l);
	res += l;
	memcpy(res, ":\"", 2);
	res += 2;
	memcpy(res, ZSTR_VAL(class_name), class_name_len);
	res += class_name_len;
	memcpy(res, "\":", 2);
	CRX_CLEANUP_CLASS_ATTRIBUTES();
	return incomplete_class;
}
/* }}} */

static HashTable* crx_var_serialize_call_sleep(crex_object *obj, crex_function *fn) /* {{{ */
{
	zval retval;

	BG(serialize_lock)++;
	crex_call_known_instance_method(fn, obj, &retval, /* param_count */ 0, /* params */ NULL);
	BG(serialize_lock)--;

	if (C_ISUNDEF(retval) || EG(exception)) {
		zval_ptr_dtor(&retval);
		return NULL;
	}

	if (C_TYPE(retval) != IS_ARRAY) {
		zval_ptr_dtor(&retval);
		crx_error_docref(NULL, E_WARNING, "%s::__sleep() should return an array only containing the names of instance-variables to serialize", ZSTR_VAL(obj->ce->name));
		return NULL;
	}

	return C_ARRVAL(retval);
}
/* }}} */

static int crx_var_serialize_call_magic_serialize(zval *retval, zval *obj) /* {{{ */
{
	BG(serialize_lock)++;
	crex_call_known_instance_method_with_0_params(
		C_OBJCE_P(obj)->__serialize, C_OBJ_P(obj), retval);
	BG(serialize_lock)--;

	if (EG(exception)) {
		zval_ptr_dtor(retval);
		return FAILURE;
	}

	if (C_TYPE_P(retval) != IS_ARRAY) {
		zval_ptr_dtor(retval);
		crex_type_error("%s::__serialize() must return an array", ZSTR_VAL(C_OBJCE_P(obj)->name));
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

static int crx_var_serialize_try_add_sleep_prop(
		HashTable *ht, HashTable *props, crex_string *name, crex_string *error_name, zval *struc) /* {{{ */
{
	zval *val = crex_hash_find(props, name);
	if (val == NULL) {
		return FAILURE;
	}

	if (C_TYPE_P(val) == IS_INDIRECT) {
		val = C_INDIRECT_P(val);
		if (C_TYPE_P(val) == IS_UNDEF) {
			crex_property_info *info = crex_get_typed_property_info_for_slot(C_OBJ_P(struc), val);
			if (info) {
				return SUCCESS;
			}
			return FAILURE;
		}
	}

	if (!crex_hash_add(ht, name, val)) {
		crx_error_docref(NULL, E_WARNING,
			"\"%s\" is returned from __sleep() multiple times", ZSTR_VAL(error_name));
		return SUCCESS;
	}

	C_TRY_ADDREF_P(val);
	return SUCCESS;
}
/* }}} */

static int crx_var_serialize_get_sleep_props(
		HashTable *ht, zval *struc, HashTable *sleep_retval) /* {{{ */
{
	crex_class_entry *ce = C_OBJCE_P(struc);
	HashTable *props = crex_get_properties_for(struc, CREX_PROP_PURPOSE_SERIALIZE);
	zval *name_val;
	int retval = SUCCESS;

	crex_hash_init(ht, crex_hash_num_elements(sleep_retval), NULL, ZVAL_PTR_DTOR, 0);
	/* TODO: Rewrite this by fetching the property info instead of trying out different
	 * name manglings? */
	CREX_HASH_FOREACH_VAL_IND(sleep_retval, name_val) {
		crex_string *name, *tmp_name, *priv_name, *prot_name;

		ZVAL_DEREF(name_val);
		if (C_TYPE_P(name_val) != IS_STRING) {
			crx_error_docref(NULL, E_WARNING,
					"%s::__sleep() should return an array only containing the names of instance-variables to serialize",
					ZSTR_VAL(ce->name));
		}

		name = zval_get_tmp_string(name_val, &tmp_name);
		if (crx_var_serialize_try_add_sleep_prop(ht, props, name, name, struc) == SUCCESS) {
			crex_tmp_string_release(tmp_name);
			continue;
		}

		if (EG(exception)) {
			crex_tmp_string_release(tmp_name);
			retval = FAILURE;
			break;
		}

		priv_name = crex_mangle_property_name(
			ZSTR_VAL(ce->name), ZSTR_LEN(ce->name),
			ZSTR_VAL(name), ZSTR_LEN(name), ce->type & CREX_INTERNAL_CLASS);
		if (crx_var_serialize_try_add_sleep_prop(ht, props, priv_name, name, struc) == SUCCESS) {
			crex_tmp_string_release(tmp_name);
			crex_string_release(priv_name);
			continue;
		}
		crex_string_release(priv_name);

		if (EG(exception)) {
			crex_tmp_string_release(tmp_name);
			retval = FAILURE;
			break;
		}

		prot_name = crex_mangle_property_name(
			"*", 1, ZSTR_VAL(name), ZSTR_LEN(name), ce->type & CREX_INTERNAL_CLASS);
		if (crx_var_serialize_try_add_sleep_prop(ht, props, prot_name, name, struc) == SUCCESS) {
			crex_tmp_string_release(tmp_name);
			crex_string_release(prot_name);
			continue;
		}
		crex_string_release(prot_name);

		if (EG(exception)) {
			crex_tmp_string_release(tmp_name);
			retval = FAILURE;
			break;
		}

		crx_error_docref(NULL, E_WARNING,
			"\"%s\" returned as member variable from __sleep() but does not exist", ZSTR_VAL(name));
		crex_tmp_string_release(tmp_name);
	} CREX_HASH_FOREACH_END();

	crex_release_properties(props);
	return retval;
}
/* }}} */

static void crx_var_serialize_nested_data(smart_str *buf, zval *struc, HashTable *ht, uint32_t count, bool incomplete_class, crx_serialize_data_t var_hash, bool in_rcn_array) /* {{{ */
{
	smart_str_append_unsigned(buf, count);
	smart_str_appendl(buf, ":{", 2);
	if (count > 0) {
		crex_string *key;
		zval *data;
		crex_ulong index;

		CREX_HASH_FOREACH_KEY_VAL_IND(ht, index, key, data) {
			if (incomplete_class && crex_string_equals_literal(key, MAGIC_MEMBER)) {
				incomplete_class = 0;
				continue;
			}

			if (!key) {
				crx_var_serialize_long(buf, index);
			} else {
				crx_var_serialize_string(buf, ZSTR_VAL(key), ZSTR_LEN(key));
			}

			if (C_ISREF_P(data) && C_REFCOUNT_P(data) == 1) {
				data = C_REFVAL_P(data);
			}

			/* we should still add element even if it's not OK,
			 * since we already wrote the length of the array before */
			if (C_TYPE_P(data) == IS_ARRAY) {
				if (UNEXPECTED(C_IS_RECURSIVE_P(data))
					|| UNEXPECTED(C_TYPE_P(struc) == IS_ARRAY && C_ARR_P(data) == C_ARR_P(struc))) {
					crx_add_var_hash(var_hash, struc, in_rcn_array);
					smart_str_appendl(buf, "N;", 2);
				} else {
					if (C_REFCOUNTED_P(data)) {
						C_PROTECT_RECURSION_P(data);
					}
					crx_var_serialize_intern(buf, data, var_hash, in_rcn_array, false);
					if (C_REFCOUNTED_P(data)) {
						C_UNPROTECT_RECURSION_P(data);
					}
				}
			} else {
				crx_var_serialize_intern(buf, data, var_hash, in_rcn_array, false);
			}
		} CREX_HASH_FOREACH_END();
	}
	smart_str_appendc(buf, '}');
}
/* }}} */

static void crx_var_serialize_class(smart_str *buf, zval *struc, HashTable *ht, crx_serialize_data_t var_hash) /* {{{ */
{
	HashTable props;

	if (crx_var_serialize_get_sleep_props(&props, struc, ht) == SUCCESS) {
		crx_var_serialize_class_name(buf, struc);
		crx_var_serialize_nested_data(
			buf, struc, &props, crex_hash_num_elements(&props), /* incomplete_class */ 0, var_hash, GC_REFCOUNT(&props) > 1);
	}
	crex_hash_destroy(&props);
}
/* }}} */

static void crx_var_serialize_intern(smart_str *buf, zval *struc, crx_serialize_data_t var_hash, bool in_rcn_array, bool is_root) /* {{{ */
{
	crex_long var_already;
	HashTable *myht;

	if (EG(exception)) {
		return;
	}

	if (var_hash && (var_already = crx_add_var_hash(var_hash, struc, in_rcn_array))) {
		if (var_already == -1) {
			/* Reference to an object that failed to serialize, replace with null. */
			smart_str_appendl(buf, "N;", 2);
			return;
		} else if (C_ISREF_P(struc)) {
			smart_str_appendl(buf, "R:", 2);
			smart_str_append_long(buf, var_already);
			smart_str_appendc(buf, ';');
			return;
		} else if (C_TYPE_P(struc) == IS_OBJECT) {
			smart_str_appendl(buf, "r:", 2);
			smart_str_append_long(buf, var_already);
			smart_str_appendc(buf, ';');
			return;
		}
	}

again:
	switch (C_TYPE_P(struc)) {
		case IS_FALSE:
			smart_str_appendl(buf, "b:0;", 4);
			return;

		case IS_TRUE:
			smart_str_appendl(buf, "b:1;", 4);
			return;

		case IS_NULL:
			smart_str_appendl(buf, "N;", 2);
			return;

		case IS_LONG:
			crx_var_serialize_long(buf, C_LVAL_P(struc));
			return;

		case IS_DOUBLE: {
			char tmp_str[CREX_DOUBLE_MAX_LENGTH];
			crex_gcvt(C_DVAL_P(struc), (int)PG(serialize_precision), '.', 'E', tmp_str);

			size_t len = strlen(tmp_str);
			char *res = smart_str_extend(buf, 2 + len + 1);
			memcpy(res, "d:", 2);
			res += 2;
			memcpy(res, tmp_str, len);
			res[len] = ';';
			return;
		}

		case IS_STRING:
			crx_var_serialize_string(buf, C_STRVAL_P(struc), C_STRLEN_P(struc));
			return;

		case IS_OBJECT: {
				crex_class_entry *ce = C_OBJCE_P(struc);
				bool incomplete_class;
				uint32_t count;

				if (ce->ce_flags & CREX_ACC_NOT_SERIALIZABLE) {
					crex_throw_exception_ex(NULL, 0, "Serialization of '%s' is not allowed",
						ZSTR_VAL(ce->name));
					return;
				}

				if (ce->ce_flags & CREX_ACC_ENUM) {
					CRX_CLASS_ATTRIBUTES;

					zval *case_name_zval = crex_enum_fetch_case_name(C_OBJ_P(struc));

					CRX_SET_CLASS_ATTRIBUTES(struc);
					smart_str_appendl(buf, "E:", 2);
					smart_str_append_unsigned(buf, ZSTR_LEN(class_name) + strlen(":") + C_STRLEN_P(case_name_zval));
					smart_str_appendl(buf, ":\"", 2);
					smart_str_append(buf, class_name);
					smart_str_appendc(buf, ':');
					smart_str_append(buf, C_STR_P(case_name_zval));
					smart_str_appendl(buf, "\";", 2);
					CRX_CLEANUP_CLASS_ATTRIBUTES();
					return;
				}

				if (ce->__serialize) {
					zval retval, obj;
					crex_string *key;
					zval *data;
					crex_ulong index;

					ZVAL_OBJ_COPY(&obj, C_OBJ_P(struc));
					if (crx_var_serialize_call_magic_serialize(&retval, &obj) == FAILURE) {
						if (!EG(exception)) {
							smart_str_appendl(buf, "N;", 2);
						}
						zval_ptr_dtor(&obj);
						return;
					}

					crx_var_serialize_class_name(buf, &obj);
					smart_str_append_unsigned(buf, crex_hash_num_elements(C_ARRVAL(retval)));
					smart_str_appendl(buf, ":{", 2);
					CREX_HASH_FOREACH_KEY_VAL(C_ARRVAL(retval), index, key, data) {
						if (!key) {
							crx_var_serialize_long(buf, index);
						} else {
							crx_var_serialize_string(buf, ZSTR_VAL(key), ZSTR_LEN(key));
						}

						if (C_ISREF_P(data) && C_REFCOUNT_P(data) == 1) {
							data = C_REFVAL_P(data);
						}
						crx_var_serialize_intern(buf, data, var_hash, C_REFCOUNT(retval) > 1, false);
					} CREX_HASH_FOREACH_END();
					smart_str_appendc(buf, '}');

					zval_ptr_dtor(&obj);
					zval_ptr_dtor(&retval);
					return;
				}

				if (ce->serialize != NULL) {
					/* has custom handler */
					unsigned char *serialized_data = NULL;
					size_t serialized_length;

					if (ce->serialize(struc, &serialized_data, &serialized_length, (crex_serialize_data *)var_hash) == SUCCESS) {
						char b1[32], b2[32];
						char *s1 = crex_print_long_to_buf(b1 + sizeof(b1) - 1, ZSTR_LEN(C_OBJCE_P(struc)->name));
						size_t l1 = b1 + sizeof(b1) - 1 - s1;
						char *s2 = crex_print_long_to_buf(b2 + sizeof(b2) - 1, serialized_length);
						size_t l2 = b2 + sizeof(b2) - 1 - s2;
						char *res = smart_str_extend(buf, 2 + l1 + 2 + ZSTR_LEN(C_OBJCE_P(struc)->name) + 2 + l2 + 2 + serialized_length + 1);
						memcpy(res, "C:", 2);
						res += 2;
						memcpy(res, s1, l1);
						res += l1;
						memcpy(res, ":\"", 2);
						res += 2;
						memcpy(res, ZSTR_VAL(C_OBJCE_P(struc)->name), ZSTR_LEN(C_OBJCE_P(struc)->name));
						res += ZSTR_LEN(C_OBJCE_P(struc)->name);
						memcpy(res, "\":", 2);
						res += 2;

						memcpy(res, s2, l2);
						res += l2;
						memcpy(res, ":{", 2);
						res += 2;
						memcpy(res, (char *) serialized_data, serialized_length);
						res[serialized_length] = '}';
					} else {
						/* Mark this value in the var_hash, to avoid creating references to it. */
						zval *var_idx = crex_hash_index_find(&var_hash->ht,
							(crex_ulong) (uintptr_t) C_COUNTED_P(struc));
						if (var_idx) {
							ZVAL_LONG(var_idx, -1);
						}
						smart_str_appendl(buf, "N;", 2);
					}
					if (serialized_data) {
						efree(serialized_data);
					}
					return;
				}

				if (ce != CRX_IC_ENTRY) {
					zval *zv = crex_hash_find_known_hash(&ce->function_table, ZSTR_KNOWN(CREX_STR_SLEEP));

					if (zv) {
						HashTable *ht;
						zval tmp;

						ZVAL_OBJ_COPY(&tmp, C_OBJ_P(struc));
						if (!(ht = crx_var_serialize_call_sleep(C_OBJ(tmp), C_FUNC_P(zv)))) {
							if (!EG(exception)) {
								/* we should still add element even if it's not OK,
								 * since we already wrote the length of the array before */
								smart_str_appendl(buf, "N;", 2);
							}
							OBJ_RELEASE(C_OBJ(tmp));
							return;
						}

						crx_var_serialize_class(buf, &tmp, ht, var_hash);
						crex_array_release(ht);
						OBJ_RELEASE(C_OBJ(tmp));
						return;
					}
				}

				incomplete_class = crx_var_serialize_class_name(buf, struc);

				if (C_OBJ_P(struc)->properties == NULL
				 && C_OBJ_HT_P(struc)->get_properties_for == NULL
				 && C_OBJ_HT_P(struc)->get_properties == crex_std_get_properties) {
					/* Optimized version without rebulding properties HashTable */
					crex_object *obj = C_OBJ_P(struc);
					crex_class_entry *ce = obj->ce;
					crex_property_info *prop_info;
					zval *prop;
					int i;

					count = ce->default_properties_count;
					for (i = 0; i < ce->default_properties_count; i++) {
						prop_info = ce->properties_info_table[i];
						if (!prop_info) {
							count--;
							continue;
						}
						prop = OBJ_PROP(obj, prop_info->offset);
						if (C_TYPE_P(prop) == IS_UNDEF) {
							count--;
							continue;
						}
					}
					if (count) {
						smart_str_append_unsigned(buf, count);
						smart_str_appendl(buf, ":{", 2);
						for (i = 0; i < ce->default_properties_count; i++) {
							prop_info = ce->properties_info_table[i];
							if (!prop_info) {
								continue;
							}
							prop = OBJ_PROP(obj, prop_info->offset);
							if (C_TYPE_P(prop) == IS_UNDEF) {
								continue;
							}

							crx_var_serialize_string(buf, ZSTR_VAL(prop_info->name), ZSTR_LEN(prop_info->name));

							if (C_ISREF_P(prop) && C_REFCOUNT_P(prop) == 1) {
								prop = C_REFVAL_P(prop);
							}

							crx_var_serialize_intern(buf, prop, var_hash, false, false);
						}
						smart_str_appendc(buf, '}');
					} else {
						smart_str_appendl(buf, "0:{}", 4);
					}
					return;
				}
				myht = crex_get_properties_for(struc, CREX_PROP_PURPOSE_SERIALIZE);
				/* count after serializing name, since crx_var_serialize_class_name
				 * changes the count if the variable is incomplete class */
				count = crex_array_count(myht);
				if (count > 0 && incomplete_class) {
					--count;
				}
				crx_var_serialize_nested_data(buf, struc, myht, count, incomplete_class, var_hash, GC_REFCOUNT(myht) > 1);
				crex_release_properties(myht);
				return;
			}
		case IS_ARRAY:
			smart_str_appendl(buf, "a:", 2);
			myht = C_ARRVAL_P(struc);
			crx_var_serialize_nested_data(
				buf, struc, myht, crex_array_count(myht), /* incomplete_class */ 0, var_hash,
					!is_root && (in_rcn_array || GC_REFCOUNT(myht) > 1));
			return;
		case IS_REFERENCE:
			struc = C_REFVAL_P(struc);
			goto again;
		default:
			smart_str_appendl(buf, "i:0;", 4);
			return;
	}
}
/* }}} */

CRXAPI void crx_var_serialize(smart_str *buf, zval *struc, crx_serialize_data_t *data) /* {{{ */
{
	crx_var_serialize_intern(buf, struc, *data, false, true);
	smart_str_0(buf);
}
/* }}} */

CRXAPI crx_serialize_data_t crx_var_serialize_init(void) {
	struct crx_serialize_data *d;
	/* fprintf(stderr, "SERIALIZE_INIT      == lock: %u, level: %u\n", BG(serialize_lock), BG(serialize).level); */
	if (BG(serialize_lock) || !BG(serialize).level) {
		d = emalloc(sizeof(struct crx_serialize_data));
		crex_hash_init(&d->ht, 16, NULL, ZVAL_PTR_DTOR, 0);
		d->n = 0;
		if (!BG(serialize_lock)) {
			BG(serialize).data = d;
			BG(serialize).level = 1;
		}
	} else {
		d = BG(serialize).data;
		++BG(serialize).level;
	}
	return d;
}

CRXAPI void crx_var_serialize_destroy(crx_serialize_data_t d) {
	/* fprintf(stderr, "SERIALIZE_DESTROY   == lock: %u, level: %u\n", BG(serialize_lock), BG(serialize).level); */
	if (BG(serialize_lock) || BG(serialize).level == 1) {
		crex_hash_destroy(&d->ht);
		efree(d);
	}
	if (!BG(serialize_lock) && !--BG(serialize).level) {
		BG(serialize).data = NULL;
	}
}

/* {{{ Returns a string representation of variable (which can later be unserialized) */
CRX_FUNCTION(serialize)
{
	zval *struc;
	crx_serialize_data_t var_hash;
	smart_str buf = {0};

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(struc)
	CREX_PARSE_PARAMETERS_END();

	CRX_VAR_SERIALIZE_INIT(var_hash);
	crx_var_serialize(&buf, struc, &var_hash);
	CRX_VAR_SERIALIZE_DESTROY(var_hash);

	if (EG(exception)) {
		smart_str_free(&buf);
		RETURN_THROWS();
	}

	RETURN_STR(smart_str_extract(&buf));
}
/* }}} */

/* {{{ Takes a string representation of variable and recreates it, subject to the optional unserialize options HashTable */
CRXAPI void crx_unserialize_with_options(zval *return_value, const char *buf, const size_t buf_len, HashTable *options, const char* function_name)
{
	const unsigned char *p;
	crx_unserialize_data_t var_hash;
	zval *retval;
	HashTable *class_hash = NULL, *prev_class_hash;
	crex_long prev_max_depth, prev_cur_depth;

	if (buf_len == 0) {
		RETURN_FALSE;
	}

	p = (const unsigned char*) buf;
	CRX_VAR_UNSERIALIZE_INIT(var_hash);

	prev_class_hash = crx_var_unserialize_get_allowed_classes(var_hash);
	prev_max_depth = crx_var_unserialize_get_max_depth(var_hash);
	prev_cur_depth = crx_var_unserialize_get_cur_depth(var_hash);
	if (options != NULL) {
		zval *classes, *max_depth;

		classes = crex_hash_str_find_deref(options, "allowed_classes", sizeof("allowed_classes")-1);
		if (classes && C_TYPE_P(classes) != IS_ARRAY && C_TYPE_P(classes) != IS_TRUE && C_TYPE_P(classes) != IS_FALSE) {
			crex_type_error("%s(): Option \"allowed_classes\" must be of type array|bool, %s given", function_name, crex_zval_value_name(classes));
			goto cleanup;
		}

		if(classes && (C_TYPE_P(classes) == IS_ARRAY || !crex_is_true(classes))) {
			ALLOC_HASHTABLE(class_hash);
			crex_hash_init(class_hash, (C_TYPE_P(classes) == IS_ARRAY)?crex_hash_num_elements(C_ARRVAL_P(classes)):0, NULL, NULL, 0);
		}
		if(class_hash && C_TYPE_P(classes) == IS_ARRAY) {
			zval *entry;
			crex_string *lcname;

			CREX_HASH_FOREACH_VAL(C_ARRVAL_P(classes), entry) {
				convert_to_string(entry);
				lcname = crex_string_tolower(C_STR_P(entry));
				crex_hash_add_empty_element(class_hash, lcname);
		        crex_string_release_ex(lcname, 0);
			} CREX_HASH_FOREACH_END();

			/* Exception during string conversion. */
			if (EG(exception)) {
				goto cleanup;
			}
		}
		crx_var_unserialize_set_allowed_classes(var_hash, class_hash);

		max_depth = crex_hash_str_find_deref(options, "max_depth", sizeof("max_depth") - 1);
		if (max_depth) {
			if (C_TYPE_P(max_depth) != IS_LONG) {
				crex_type_error("%s(): Option \"max_depth\" must be of type int, %s given", function_name, crex_zval_value_name(max_depth));
				goto cleanup;
			}
			if (C_LVAL_P(max_depth) < 0) {
				crex_value_error("%s(): Option \"max_depth\" must be greater than or equal to 0", function_name);
				goto cleanup;
			}

			crx_var_unserialize_set_max_depth(var_hash, C_LVAL_P(max_depth));
			/* If the max_depth for a nested unserialize() call has been overridden,
			 * start counting from zero again (for the nested call only). */
			crx_var_unserialize_set_cur_depth(var_hash, 0);
		}
	}

	if (BG(unserialize).level > 1) {
		retval = var_tmp_var(&var_hash);
	} else {
		retval = return_value;
	}
	if (!crx_var_unserialize(retval, &p, p + buf_len, &var_hash)) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Error at offset " CREX_LONG_FMT " of %zd bytes",
				(crex_long)((char*)p - buf), buf_len);
		}
		if (BG(unserialize).level <= 1) {
			zval_ptr_dtor(return_value);
		}
		RETVAL_FALSE;
	} else {
		if ((char*)p < buf + buf_len) {
			if (!EG(exception)) {
				crx_error_docref(NULL, E_WARNING, "Extra data starting at offset " CREX_LONG_FMT " of %zd bytes",
					(crex_long)((char*)p - buf), buf_len);
			}
		}
		if (BG(unserialize).level > 1) {
			ZVAL_COPY(return_value, retval);
		} else if (C_REFCOUNTED_P(return_value)) {
			crex_refcounted *ref = C_COUNTED_P(return_value);
			gc_check_possible_root(ref);
		}
	}

cleanup:
	if (class_hash) {
		crex_hash_destroy(class_hash);
		FREE_HASHTABLE(class_hash);
	}

	/* Reset to previous options in case this is a nested call */
	crx_var_unserialize_set_allowed_classes(var_hash, prev_class_hash);
	crx_var_unserialize_set_max_depth(var_hash, prev_max_depth);
	crx_var_unserialize_set_cur_depth(var_hash, prev_cur_depth);
	CRX_VAR_UNSERIALIZE_DESTROY(var_hash);

	/* Per calling convention we must not return a reference here, so unwrap. We're doing this at
	 * the very end, because __wakeup() calls performed during UNSERIALIZE_DESTROY might affect
	 * the value we unwrap here. This is compatible with behavior in CRX <=7.0. */
	if (C_ISREF_P(return_value)) {
		crex_unwrap_reference(return_value);
	}
}
/* }}} */

/* {{{ Takes a string representation of variable and recreates it */
CRX_FUNCTION(unserialize)
{
	char *buf = NULL;
	size_t buf_len;
	HashTable *options = NULL;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STRING(buf, buf_len)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_HT(options)
	CREX_PARSE_PARAMETERS_END();

	crx_unserialize_with_options(return_value, buf, buf_len, options, "unserialize");
}
/* }}} */

/* {{{ Returns the allocated by CRX memory */
CRX_FUNCTION(memory_get_usage) {
	bool real_usage = 0;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(real_usage)
	CREX_PARSE_PARAMETERS_END();

	RETURN_LONG(crex_memory_usage(real_usage));
}
/* }}} */

/* {{{ Returns the peak allocated by CRX memory */
CRX_FUNCTION(memory_get_peak_usage) {
	bool real_usage = 0;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(real_usage)
	CREX_PARSE_PARAMETERS_END();

	RETURN_LONG(crex_memory_peak_usage(real_usage));
}
/* }}} */

/* {{{ Resets the peak CRX memory usage */
CRX_FUNCTION(memory_reset_peak_usage) {
	CREX_PARSE_PARAMETERS_NONE();

	crex_memory_reset_peak_usage();
}
/* }}} */

CRX_INI_BEGIN()
	STD_CRX_INI_ENTRY("unserialize_max_depth", "4096", CRX_INI_ALL, OnUpdateLong, unserialize_max_depth, crx_basic_globals, basic_globals)
CRX_INI_END()

CRX_MINIT_FUNCTION(var)
{
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
