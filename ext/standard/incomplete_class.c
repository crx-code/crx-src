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
   | Author:  Sascha Schumann <sascha@schumann.cx>                        |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "basic_functions.h"
#include "crx_incomplete_class.h"

#define INCOMPLETE_CLASS_MSG \
		"The script tried to %s on an incomplete object. " \
		"Please ensure that the class definition \"%s\" of the object " \
		"you are trying to operate on was loaded _before_ " \
		"unserialize() gets called or provide an autoloader " \
		"to load the class definition"

CRXAPI crex_class_entry *crx_ce_incomplete_class;
static crex_object_handlers crx_incomplete_object_handlers;

static void incomplete_class_message(crex_object *object)
{
	crex_string *class_name = crx_lookup_class_name(object);
	crx_error_docref(NULL, E_WARNING, INCOMPLETE_CLASS_MSG,
		"access a property", class_name ? ZSTR_VAL(class_name) : "unknown");
	if (class_name) {
		crex_string_release_ex(class_name, 0);
	}
}

static void throw_incomplete_class_error(crex_object *object, const char *what)
{
	crex_string *class_name = crx_lookup_class_name(object);
	crex_throw_error(NULL, INCOMPLETE_CLASS_MSG,
		what, class_name ? ZSTR_VAL(class_name) : "unknown");
	if (class_name) {
		crex_string_release_ex(class_name, 0);
	}
}

static zval *incomplete_class_get_property(crex_object *object, crex_string *member, int type, void **cache_slot, zval *rv) /* {{{ */
{
	incomplete_class_message(object);

	if (type == BP_VAR_W || type == BP_VAR_RW) {
		ZVAL_ERROR(rv);
		return rv;
	} else {
		return &EG(uninitialized_zval);
	}
}
/* }}} */

static zval *incomplete_class_write_property(crex_object *object, crex_string *member, zval *value, void **cache_slot) /* {{{ */
{
	throw_incomplete_class_error(object, "modify a property");
	return value;
}
/* }}} */

static zval *incomplete_class_get_property_ptr_ptr(crex_object *object, crex_string *member, int type, void **cache_slot) /* {{{ */
{
	throw_incomplete_class_error(object, "modify a property");
	return &EG(error_zval);
}
/* }}} */

static void incomplete_class_unset_property(crex_object *object, crex_string *member, void **cache_slot) /* {{{ */
{
	throw_incomplete_class_error(object, "modify a property");
}
/* }}} */

static int incomplete_class_has_property(crex_object *object, crex_string *member, int check_empty, void **cache_slot) /* {{{ */
{
	incomplete_class_message(object);
	return 0;
}
/* }}} */

static crex_function *incomplete_class_get_method(crex_object **object, crex_string *method, const zval *key) /* {{{ */
{
	throw_incomplete_class_error(*object, "call a method");
	return NULL;
}
/* }}} */

/* {{{ crx_create_incomplete_class */
static crex_object *crx_create_incomplete_object(crex_class_entry *class_type)
{
	crex_object *object;

	object = crex_objects_new( class_type);
	object->handlers = &crx_incomplete_object_handlers;

	object_properties_init(object, class_type);

	return object;
}

CRXAPI void crx_register_incomplete_class_handlers(void)
{
	memcpy(&crx_incomplete_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	crx_incomplete_object_handlers.read_property = incomplete_class_get_property;
	crx_incomplete_object_handlers.has_property = incomplete_class_has_property;
	crx_incomplete_object_handlers.unset_property = incomplete_class_unset_property;
	crx_incomplete_object_handlers.write_property = incomplete_class_write_property;
	crx_incomplete_object_handlers.get_property_ptr_ptr = incomplete_class_get_property_ptr_ptr;
	crx_incomplete_object_handlers.get_method = incomplete_class_get_method;

	crx_ce_incomplete_class->create_object = crx_create_incomplete_object;
}
/* }}} */

/* {{{ crx_lookup_class_name */
CRXAPI crex_string *crx_lookup_class_name(crex_object *object)
{
	if (object->properties) {
		zval *val = crex_hash_str_find(object->properties, MAGIC_MEMBER, sizeof(MAGIC_MEMBER)-1);

		if (val != NULL && C_TYPE_P(val) == IS_STRING) {
			return crex_string_copy(C_STR_P(val));
		}
	}

	return NULL;
}
/* }}} */

/* {{{ crx_store_class_name */
CRXAPI void crx_store_class_name(zval *object, crex_string *name)
{
	zval val;

	ZVAL_STR_COPY(&val, name);
	crex_hash_str_update(C_OBJPROP_P(object), MAGIC_MEMBER, sizeof(MAGIC_MEMBER)-1, &val);
}
/* }}} */
