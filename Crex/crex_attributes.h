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

#ifndef CREX_ATTRIBUTES_H
#define CREX_ATTRIBUTES_H

#define CREX_ATTRIBUTE_TARGET_CLASS			(1<<0)
#define CREX_ATTRIBUTE_TARGET_FUNCTION		(1<<1)
#define CREX_ATTRIBUTE_TARGET_METHOD		(1<<2)
#define CREX_ATTRIBUTE_TARGET_PROPERTY		(1<<3)
#define CREX_ATTRIBUTE_TARGET_CLASS_CONST	(1<<4)
#define CREX_ATTRIBUTE_TARGET_PARAMETER		(1<<5)
#define CREX_ATTRIBUTE_TARGET_ALL			((1<<6) - 1)
#define CREX_ATTRIBUTE_IS_REPEATABLE		(1<<6)
#define CREX_ATTRIBUTE_FLAGS				((1<<7) - 1)

/* Flags for crex_attribute.flags */
#define CREX_ATTRIBUTE_PERSISTENT   (1<<0)
#define CREX_ATTRIBUTE_STRICT_TYPES (1<<1)

#define CREX_ATTRIBUTE_SIZE(argc) \
	(sizeof(crex_attribute) + sizeof(crex_attribute_arg) * (argc) - sizeof(crex_attribute_arg))

BEGIN_EXTERN_C()

extern CREX_API crex_class_entry *crex_ce_attribute;
extern CREX_API crex_class_entry *crex_ce_allow_dynamic_properties;
extern CREX_API crex_class_entry *crex_ce_sensitive_parameter;
extern CREX_API crex_class_entry *crex_ce_sensitive_parameter_value;
extern CREX_API crex_class_entry *crex_ce_override;

typedef struct {
	crex_string *name;
	zval value;
} crex_attribute_arg;

typedef struct _crex_attribute {
	crex_string *name;
	crex_string *lcname;
	uint32_t flags;
	uint32_t lineno;
	/* Parameter offsets start at 1, everything else uses 0. */
	uint32_t offset;
	uint32_t argc;
	crex_attribute_arg args[1];
} crex_attribute;

typedef struct _crex_internal_attribute {
	crex_class_entry *ce;
	uint32_t flags;
	void (*validator)(crex_attribute *attr, uint32_t target, crex_class_entry *scope);
} crex_internal_attribute;

CREX_API crex_attribute *crex_get_attribute(HashTable *attributes, crex_string *lcname);
CREX_API crex_attribute *crex_get_attribute_str(HashTable *attributes, const char *str, size_t len);

CREX_API crex_attribute *crex_get_parameter_attribute(HashTable *attributes, crex_string *lcname, uint32_t offset);
CREX_API crex_attribute *crex_get_parameter_attribute_str(HashTable *attributes, const char *str, size_t len, uint32_t offset);

CREX_API crex_result crex_get_attribute_value(zval *ret, crex_attribute *attr, uint32_t i, crex_class_entry *scope);

CREX_API crex_string *crex_get_attribute_target_names(uint32_t targets);
CREX_API bool crex_is_attribute_repeated(HashTable *attributes, crex_attribute *attr);

CREX_API crex_internal_attribute *crex_mark_internal_attribute(crex_class_entry *ce);
CREX_API crex_internal_attribute *crex_internal_attribute_register(crex_class_entry *ce, uint32_t flags);
CREX_API crex_internal_attribute *crex_internal_attribute_get(crex_string *lcname);

CREX_API crex_attribute *crex_add_attribute(
		HashTable **attributes, crex_string *name, uint32_t argc,
		uint32_t flags, uint32_t offset, uint32_t lineno);

END_EXTERN_C()

static crex_always_inline crex_attribute *crex_add_class_attribute(crex_class_entry *ce, crex_string *name, uint32_t argc)
{
	uint32_t flags = ce->type != CREX_USER_CLASS ? CREX_ATTRIBUTE_PERSISTENT : 0;
	return crex_add_attribute(&ce->attributes, name, argc, flags, 0, 0);
}

static crex_always_inline crex_attribute *crex_add_function_attribute(crex_function *func, crex_string *name, uint32_t argc)
{
	uint32_t flags = func->common.type != CREX_USER_FUNCTION ? CREX_ATTRIBUTE_PERSISTENT : 0;
	return crex_add_attribute(&func->common.attributes, name, argc, flags, 0, 0);
}

static crex_always_inline crex_attribute *crex_add_parameter_attribute(crex_function *func, uint32_t offset, crex_string *name, uint32_t argc)
{
	uint32_t flags = func->common.type != CREX_USER_FUNCTION ? CREX_ATTRIBUTE_PERSISTENT : 0;
	return crex_add_attribute(&func->common.attributes, name, argc, flags, offset + 1, 0);
}

static crex_always_inline crex_attribute *crex_add_property_attribute(crex_class_entry *ce, crex_property_info *info, crex_string *name, uint32_t argc)
{
	uint32_t flags = ce->type != CREX_USER_CLASS ? CREX_ATTRIBUTE_PERSISTENT : 0;
	return crex_add_attribute(&info->attributes, name, argc, flags, 0, 0);
}

static crex_always_inline crex_attribute *crex_add_class_constant_attribute(crex_class_entry *ce, crex_class_constant *c, crex_string *name, uint32_t argc)
{
	uint32_t flags = ce->type != CREX_USER_CLASS ? CREX_ATTRIBUTE_PERSISTENT : 0;
	return crex_add_attribute(&c->attributes, name, argc, flags, 0, 0);
}

void crex_register_attribute_ce(void);
void crex_attributes_shutdown(void);

#endif
