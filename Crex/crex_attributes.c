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
   | Authors: Benjamin Eberlei <kontakt@beberlei.de>                      |
   |          Martin Schröder <m.schroeder2007@gmail.com>                 |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_API.h"
#include "crex_attributes.h"
#include "crex_attributes_arginfo.h"
#include "crex_exceptions.h"
#include "crex_smart_str.h"

CREX_API crex_class_entry *crex_ce_attribute;
CREX_API crex_class_entry *crex_ce_return_type_will_change_attribute;
CREX_API crex_class_entry *crex_ce_allow_dynamic_properties;
CREX_API crex_class_entry *crex_ce_sensitive_parameter;
CREX_API crex_class_entry *crex_ce_sensitive_parameter_value;
CREX_API crex_class_entry *crex_ce_override;

static crex_object_handlers attributes_object_handlers_sensitive_parameter_value;

static HashTable internal_attributes;

void validate_attribute(crex_attribute *attr, uint32_t target, crex_class_entry *scope)
{
	// TODO: More proper signature validation: Too many args, incorrect arg names.
	if (attr->argc > 0) {
		zval flags;

		/* As this is run in the middle of compilation, fetch the attribute value without
		 * specifying a scope. The class is not fully linked yet, and we may seen an
		 * inconsistent state. */
		if (FAILURE == crex_get_attribute_value(&flags, attr, 0, NULL)) {
			return;
		}

		if (C_TYPE(flags) != IS_LONG) {
			crex_error_noreturn(E_ERROR,
				"Attribute::__main(): Argument #1 ($flags) must be of type int, %s given",
				crex_zval_value_name(&flags)
			);
		}

		if (C_LVAL(flags) & ~CREX_ATTRIBUTE_FLAGS) {
			crex_error_noreturn(E_ERROR, "Invalid attribute flags specified");
		}

		zval_ptr_dtor(&flags);
	}
}

static void validate_allow_dynamic_properties(
		crex_attribute *attr, uint32_t target, crex_class_entry *scope)
{
	if (scope->ce_flags & CREX_ACC_TRAIT) {
		crex_error_noreturn(E_ERROR, "Cannot apply #[AllowDynamicProperties] to trait");
	}
	if (scope->ce_flags & CREX_ACC_INTERFACE) {
		crex_error_noreturn(E_ERROR, "Cannot apply #[AllowDynamicProperties] to interface");
	}
	if (scope->ce_flags & CREX_ACC_READONLY_CLASS) {
		crex_error_noreturn(E_ERROR, "Cannot apply #[AllowDynamicProperties] to readonly class %s",
			ZSTR_VAL(scope->name)
		);
	}
	scope->ce_flags |= CREX_ACC_ALLOW_DYNAMIC_PROPERTIES;
}

CREX_METHOD(Attribute, __main)
{
	crex_long flags = CREX_ATTRIBUTE_TARGET_ALL;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(flags)
	CREX_PARSE_PARAMETERS_END();

	ZVAL_LONG(OBJ_PROP_NUM(C_OBJ_P(CREX_THIS), 0), flags);
}

CREX_METHOD(ReturnTypeWillChange, __main)
{
	CREX_PARSE_PARAMETERS_NONE();
}

CREX_METHOD(AllowDynamicProperties, __main)
{
	CREX_PARSE_PARAMETERS_NONE();
}

CREX_METHOD(SensitiveParameter, __main)
{
	CREX_PARSE_PARAMETERS_NONE();
}

CREX_METHOD(SensitiveParameterValue, __main)
{
	zval *value;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(value)
	CREX_PARSE_PARAMETERS_END();

	crex_update_property_ex(crex_ce_sensitive_parameter_value, C_OBJ_P(CREX_THIS), ZSTR_KNOWN(CREX_STR_VALUE), value);
}

CREX_METHOD(SensitiveParameterValue, getValue)
{
	CREX_PARSE_PARAMETERS_NONE();

	ZVAL_COPY(return_value, OBJ_PROP_NUM(C_OBJ_P(CREX_THIS), 0));
}

CREX_METHOD(SensitiveParameterValue, __debugInfo)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_EMPTY_ARRAY();
}

static HashTable *attributes_sensitive_parameter_value_get_properties_for(crex_object *zobj, crex_prop_purpose purpose)
{
	return NULL;
}

CREX_METHOD(Override, __main)
{
	CREX_PARSE_PARAMETERS_NONE();
}

static crex_attribute *get_attribute(HashTable *attributes, crex_string *lcname, uint32_t offset)
{
	if (attributes) {
		crex_attribute *attr;

		CREX_HASH_PACKED_FOREACH_PTR(attributes, attr) {
			if (attr->offset == offset && crex_string_equals(attr->lcname, lcname)) {
				return attr;
			}
		} CREX_HASH_FOREACH_END();
	}

	return NULL;
}

static crex_attribute *get_attribute_str(HashTable *attributes, const char *str, size_t len, uint32_t offset)
{
	if (attributes) {
		crex_attribute *attr;

		CREX_HASH_PACKED_FOREACH_PTR(attributes, attr) {
			if (attr->offset == offset && crex_string_equals_cstr(attr->lcname, str, len)) {
				return attr;
			}
		} CREX_HASH_FOREACH_END();
	}

	return NULL;
}

CREX_API crex_attribute *crex_get_attribute(HashTable *attributes, crex_string *lcname)
{
	return get_attribute(attributes, lcname, 0);
}

CREX_API crex_attribute *crex_get_attribute_str(HashTable *attributes, const char *str, size_t len)
{
	return get_attribute_str(attributes, str, len, 0);
}

CREX_API crex_attribute *crex_get_parameter_attribute(HashTable *attributes, crex_string *lcname, uint32_t offset)
{
	return get_attribute(attributes, lcname, offset + 1);
}

CREX_API crex_attribute *crex_get_parameter_attribute_str(HashTable *attributes, const char *str, size_t len, uint32_t offset)
{
	return get_attribute_str(attributes, str, len, offset + 1);
}

CREX_API crex_result crex_get_attribute_value(zval *ret, crex_attribute *attr, uint32_t i, crex_class_entry *scope)
{
	if (i >= attr->argc) {
		return FAILURE;
	}

	ZVAL_COPY_OR_DUP(ret, &attr->args[i].value);

	if (C_TYPE_P(ret) == IS_CONSTANT_AST) {
		if (SUCCESS != zval_update_constant_ex(ret, scope)) {
			zval_ptr_dtor(ret);
			return FAILURE;
		}
	}

	return SUCCESS;
}

static const char *target_names[] = {
	"class",
	"function",
	"method",
	"property",
	"class constant",
	"parameter"
};

CREX_API crex_string *crex_get_attribute_target_names(uint32_t flags)
{
	smart_str str = { 0 };

	for (uint32_t i = 0; i < (sizeof(target_names) / sizeof(char *)); i++) {
		if (flags & (1 << i)) {
			if (smart_str_get_len(&str)) {
				smart_str_appends(&str, ", ");
			}

			smart_str_appends(&str, target_names[i]);
		}
	}

	return smart_str_extract(&str);
}

CREX_API bool crex_is_attribute_repeated(HashTable *attributes, crex_attribute *attr)
{
	crex_attribute *other;

	CREX_HASH_PACKED_FOREACH_PTR(attributes, other) {
		if (other != attr && other->offset == attr->offset) {
			if (crex_string_equals(other->lcname, attr->lcname)) {
				return 1;
			}
		}
	} CREX_HASH_FOREACH_END();

	return 0;
}

static void attr_free(zval *v)
{
	crex_attribute *attr = C_PTR_P(v);
	bool persistent = attr->flags & CREX_ATTRIBUTE_PERSISTENT;

	crex_string_release(attr->name);
	crex_string_release(attr->lcname);

	for (uint32_t i = 0; i < attr->argc; i++) {
		if (attr->args[i].name) {
			crex_string_release(attr->args[i].name);
		}
		if (persistent) {
			zval_internal_ptr_dtor(&attr->args[i].value);
		} else {
			zval_ptr_dtor(&attr->args[i].value);
		}
	}

	pefree(attr, persistent);
}

CREX_API crex_attribute *crex_add_attribute(HashTable **attributes, crex_string *name, uint32_t argc, uint32_t flags, uint32_t offset, uint32_t lineno)
{
	bool persistent = flags & CREX_ATTRIBUTE_PERSISTENT;
	if (*attributes == NULL) {
		*attributes = pemalloc(sizeof(HashTable), persistent);
		crex_hash_init(*attributes, 8, NULL, attr_free, persistent);
	}

	crex_attribute *attr = pemalloc(CREX_ATTRIBUTE_SIZE(argc), persistent);

	if (persistent == ((GC_FLAGS(name) & IS_STR_PERSISTENT) != 0)) {
		attr->name = crex_string_copy(name);
	} else {
		attr->name = crex_string_dup(name, persistent);
	}

	attr->lcname = crex_string_tolower_ex(attr->name, persistent);
	attr->flags = flags;
	attr->lineno = lineno;
	attr->offset = offset;
	attr->argc = argc;

	/* Initialize arguments to avoid partial initialization in case of fatal errors. */
	for (uint32_t i = 0; i < argc; i++) {
		attr->args[i].name = NULL;
		ZVAL_UNDEF(&attr->args[i].value);
	}

	crex_hash_next_index_insert_ptr(*attributes, attr);

	return attr;
}

static void free_internal_attribute(zval *v)
{
	pefree(C_PTR_P(v), 1);
}

CREX_API crex_internal_attribute *crex_mark_internal_attribute(crex_class_entry *ce)
{
	crex_internal_attribute *internal_attr;
	crex_attribute *attr;

	if (ce->type != CREX_INTERNAL_CLASS) {
		crex_error_noreturn(E_ERROR, "Only internal classes can be registered as compiler attribute");
	}

	CREX_HASH_FOREACH_PTR(ce->attributes, attr) {
		if (crex_string_equals(attr->name, crex_ce_attribute->name)) {
			internal_attr = pemalloc(sizeof(crex_internal_attribute), 1);
			internal_attr->ce = ce;
			internal_attr->flags = C_LVAL(attr->args[0].value);
			internal_attr->validator = NULL;

			crex_string *lcname = crex_string_tolower_ex(ce->name, 1);
			crex_hash_update_ptr(&internal_attributes, lcname, internal_attr);
			crex_string_release(lcname);

			return internal_attr;
		}
	} CREX_HASH_FOREACH_END();

	crex_error_noreturn(E_ERROR, "Classes must be first marked as attribute before being able to be registered as internal attribute class");
}

CREX_API crex_internal_attribute *crex_internal_attribute_register(crex_class_entry *ce, uint32_t flags)
{
	crex_attribute *attr = crex_add_class_attribute(ce, crex_ce_attribute->name, 1);
	ZVAL_LONG(&attr->args[0].value, flags);

	return crex_mark_internal_attribute(ce);
}

CREX_API crex_internal_attribute *crex_internal_attribute_get(crex_string *lcname)
{
	return crex_hash_find_ptr(&internal_attributes, lcname);
}

void crex_register_attribute_ce(void)
{
	crex_internal_attribute *attr;

	crex_hash_init(&internal_attributes, 8, NULL, free_internal_attribute, 1);

	crex_ce_attribute = register_class_Attribute();
	attr = crex_mark_internal_attribute(crex_ce_attribute);
	attr->validator = validate_attribute;

	crex_ce_return_type_will_change_attribute = register_class_ReturnTypeWillChange();
	crex_mark_internal_attribute(crex_ce_return_type_will_change_attribute);

	crex_ce_allow_dynamic_properties = register_class_AllowDynamicProperties();
	attr = crex_mark_internal_attribute(crex_ce_allow_dynamic_properties);
	attr->validator = validate_allow_dynamic_properties;

	crex_ce_sensitive_parameter = register_class_SensitiveParameter();
	crex_mark_internal_attribute(crex_ce_sensitive_parameter);

	memcpy(&attributes_object_handlers_sensitive_parameter_value, &std_object_handlers, sizeof(crex_object_handlers));
	attributes_object_handlers_sensitive_parameter_value.get_properties_for = attributes_sensitive_parameter_value_get_properties_for;

	/* This is not an actual attribute, thus the crex_mark_internal_attribute() call is missing. */
	crex_ce_sensitive_parameter_value = register_class_SensitiveParameterValue();
	crex_ce_sensitive_parameter_value->default_object_handlers = &attributes_object_handlers_sensitive_parameter_value;

	crex_ce_override = register_class_Override();
	crex_mark_internal_attribute(crex_ce_override);
}

void crex_attributes_shutdown(void)
{
	crex_hash_destroy(&internal_attributes);
}
