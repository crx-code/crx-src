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
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "crx_test.h"
#include "observer.h"
#include "fiber.h"
#include "iterators.h"
#include "object_handlers.h"
#include "crex_attributes.h"
#include "crex_enum.h"
#include "crex_interfaces.h"
#include "crex_weakrefs.h"
#include "Crex/Optimizer/crex_optimizer.h"
#include "test_arginfo.h"
#include "crex_call_stack.h"
#include "crex_exceptions.h"

#if defined(HAVE_LIBXML) && !defined(CRX_WIN32)
# include <libxml/globals.h>
# include <libxml/parser.h>
#endif

CREX_DECLARE_MODULE_GLOBALS(crex_test)

static crex_class_entry *crex_test_interface;
static crex_class_entry *crex_test_class;
static crex_class_entry *crex_test_child_class;
static crex_class_entry *crex_attribute_test_class;
static crex_class_entry *crex_test_trait;
static crex_class_entry *crex_test_attribute;
static crex_class_entry *crex_test_repeatable_attribute;
static crex_class_entry *crex_test_parameter_attribute;
static crex_class_entry *crex_test_property_attribute;
static crex_class_entry *crex_test_class_with_method_with_parameter_attribute;
static crex_class_entry *crex_test_child_class_with_method_with_parameter_attribute;
static crex_class_entry *crex_test_class_with_property_attribute;
static crex_class_entry *crex_test_forbid_dynamic_call;
static crex_class_entry *crex_test_ns_foo_class;
static crex_class_entry *crex_test_ns_unlikely_compile_error_class;
static crex_class_entry *crex_test_ns_not_unlikely_compile_error_class;
static crex_class_entry *crex_test_ns2_foo_class;
static crex_class_entry *crex_test_ns2_ns_foo_class;
static crex_class_entry *crex_test_unit_enum;
static crex_class_entry *crex_test_string_enum;
static crex_class_entry *crex_test_int_enum;
static crex_object_handlers crex_test_class_handlers;

static int le_throwing_resource;

static CREX_FUNCTION(crex_test_func)
{
	RETVAL_STR_COPY(EX(func)->common.function_name);

	/* Cleanup trampoline */
	CREX_ASSERT(EX(func)->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE);
	crex_string_release(EX(func)->common.function_name);
	crex_free_trampoline(EX(func));
	EX(func) = NULL;
}

static CREX_FUNCTION(crex_test_array_return)
{
	CREX_PARSE_PARAMETERS_NONE();
}

static CREX_FUNCTION(crex_test_nullable_array_return)
{
	CREX_PARSE_PARAMETERS_NONE();
}

static CREX_FUNCTION(crex_test_void_return)
{
	/* dummy */
	CREX_PARSE_PARAMETERS_NONE();
}

static void pass1(crex_script *script, void *context)
{
	crx_printf("pass1\n");
}

static void pass2(crex_script *script, void *context)
{
	crx_printf("pass2\n");
}

static CREX_FUNCTION(crex_test_deprecated)
{
	zval *arg1;

	crex_parse_parameters(CREX_NUM_ARGS(), "|z", &arg1);
}

/* Create a string without terminating null byte. Must be terminated with
 * crex_terminate_string() before destruction, otherwise a warning is issued
 * in debug builds. */
static CREX_FUNCTION(crex_create_unterminated_string)
{
	crex_string *str, *res;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S", &str) == FAILURE) {
		RETURN_THROWS();
	}

	res = crex_string_alloc(ZSTR_LEN(str), 0);
	memcpy(ZSTR_VAL(res), ZSTR_VAL(str), ZSTR_LEN(str));
	/* No trailing null byte */

	RETURN_STR(res);
}

/* Enforce terminate null byte on string. This avoids a warning in debug builds. */
static CREX_FUNCTION(crex_terminate_string)
{
	crex_string *str;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S", &str) == FAILURE) {
		RETURN_THROWS();
	}

	ZSTR_VAL(str)[ZSTR_LEN(str)] = '\0';
}

/* Cause an intentional memory leak, for testing/debugging purposes */
static CREX_FUNCTION(crex_leak_bytes)
{
	crex_long leakbytes = 3;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|l", &leakbytes) == FAILURE) {
		RETURN_THROWS();
	}

	emalloc(leakbytes);
}

/* Leak a refcounted variable */
static CREX_FUNCTION(crex_leak_variable)
{
	zval *zv;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "z", &zv) == FAILURE) {
		RETURN_THROWS();
	}

	if (!C_REFCOUNTED_P(zv)) {
		crex_error(E_WARNING, "Cannot leak variable that is not refcounted");
		return;
	}

	C_ADDREF_P(zv);
}

/* Tests C_PARAM_OBJ_OR_STR */
static CREX_FUNCTION(crex_string_or_object)
{
	crex_string *str;
	crex_object *object;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_OBJ_OR_STR(object, str)
	CREX_PARSE_PARAMETERS_END();

	if (str) {
		RETURN_STR_COPY(str);
	} else {
		RETURN_OBJ_COPY(object);
	}
}

/* Tests C_PARAM_OBJ_OR_STR_OR_NULL */
static CREX_FUNCTION(crex_string_or_object_or_null)
{
	crex_string *str;
	crex_object *object;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_OBJ_OR_STR_OR_NULL(object, str)
	CREX_PARSE_PARAMETERS_END();

	if (str) {
		RETURN_STR_COPY(str);
	} else if (object) {
		RETURN_OBJ_COPY(object);
	} else {
		RETURN_NULL();
	}
}

/* Tests C_PARAM_OBJ_OF_CLASS_OR_STR */
static CREX_FUNCTION(crex_string_or_stdclass)
{
	crex_string *str;
	crex_object *object;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_OBJ_OF_CLASS_OR_STR(object, crex_standard_class_def, str)
	CREX_PARSE_PARAMETERS_END();

	if (str) {
		RETURN_STR_COPY(str);
	} else {
		RETURN_OBJ_COPY(object);
	}
}

static CREX_FUNCTION(crex_test_compile_string)
{
	crex_string *source_string = NULL;
	crex_string *filename = NULL;
	crex_long position = CREX_COMPILE_POSITION_AT_OPEN_TAG;

	CREX_PARSE_PARAMETERS_START(3, 3)
		C_PARAM_STR(source_string)
		C_PARAM_STR(filename)
		C_PARAM_LONG(position)
	CREX_PARSE_PARAMETERS_END();

	crex_op_array *op_array = NULL;

	op_array = compile_string(source_string, ZSTR_VAL(filename), position);

	if (op_array) {
		zval retval;

		crex_try {
			ZVAL_UNDEF(&retval);
			crex_execute(op_array, &retval);
		} crex_catch {
			destroy_op_array(op_array);
			efree_size(op_array, sizeof(crex_op_array));
			crex_bailout();
		} crex_end_try();

		destroy_op_array(op_array);
		efree_size(op_array, sizeof(crex_op_array));
	}

	return;
}

/* Tests C_PARAM_OBJ_OF_CLASS_OR_STR_OR_NULL */
static CREX_FUNCTION(crex_string_or_stdclass_or_null)
{
	crex_string *str;
	crex_object *object;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_OBJ_OF_CLASS_OR_STR_OR_NULL(object, crex_standard_class_def, str)
	CREX_PARSE_PARAMETERS_END();

	if (str) {
		RETURN_STR_COPY(str);
	} else if (object) {
		RETURN_OBJ_COPY(object);
	} else {
		RETURN_NULL();
	}
}

/* Tests C_PARAM_NUMBER_OR_STR */
static CREX_FUNCTION(crex_number_or_string)
{
	zval *input;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_NUMBER_OR_STR(input)
	CREX_PARSE_PARAMETERS_END();

	switch (C_TYPE_P(input)) {
		case IS_LONG:
			RETURN_LONG(C_LVAL_P(input));
		case IS_DOUBLE:
			RETURN_DOUBLE(C_DVAL_P(input));
		case IS_STRING:
			RETURN_STR_COPY(C_STR_P(input));
		EMPTY_SWITCH_DEFAULT_CASE();
	}
}

/* Tests C_PARAM_NUMBER_OR_STR_OR_NULL */
static CREX_FUNCTION(crex_number_or_string_or_null)
{
	zval *input;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_NUMBER_OR_STR_OR_NULL(input)
	CREX_PARSE_PARAMETERS_END();

	if (!input) {
		RETURN_NULL();
	}

	switch (C_TYPE_P(input)) {
		case IS_LONG:
			RETURN_LONG(C_LVAL_P(input));
		case IS_DOUBLE:
			RETURN_DOUBLE(C_DVAL_P(input));
		case IS_STRING:
			RETURN_STR_COPY(C_STR_P(input));
		EMPTY_SWITCH_DEFAULT_CASE();
	}
}

static CREX_FUNCTION(crex_weakmap_attach)
{
	zval *value;
	crex_object *obj;

	CREX_PARSE_PARAMETERS_START(2, 2)
			C_PARAM_OBJ(obj)
			C_PARAM_ZVAL(value)
	CREX_PARSE_PARAMETERS_END();

	if (crex_weakrefs_hash_add(&ZT_G(global_weakmap), obj, value)) {
		C_TRY_ADDREF_P(value);
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

static CREX_FUNCTION(crex_weakmap_remove)
{
	crex_object *obj;

	CREX_PARSE_PARAMETERS_START(1, 1)
			C_PARAM_OBJ(obj)
	CREX_PARSE_PARAMETERS_END();

	RETURN_BOOL(crex_weakrefs_hash_del(&ZT_G(global_weakmap), obj) == SUCCESS);
}

static CREX_FUNCTION(crex_weakmap_dump)
{
	CREX_PARSE_PARAMETERS_NONE();
	RETURN_ARR(crex_array_dup(&ZT_G(global_weakmap)));
}

static CREX_FUNCTION(crex_get_current_func_name)
{
    CREX_PARSE_PARAMETERS_NONE();

    crex_string *function_name = get_function_or_method_name(EG(current_execute_data)->prev_execute_data->func);

    RETURN_STR(function_name);
}

#if defined(HAVE_LIBXML) && !defined(CRX_WIN32)
static CREX_FUNCTION(crex_test_override_libxml_global_state)
{
	CREX_PARSE_PARAMETERS_NONE();

	xmlLoadExtDtdDefaultValue = 1;
	xmlDoValidityCheckingDefaultValue = 1;
	(void) xmlPedanticParserDefault(1);
	(void) xmlSubstituteEntitiesDefault(1);
	(void) xmlLineNumbersDefault(1);
	(void) xmlKeepBlanksDefault(0);
}
#endif

/* TESTS C_PARAM_ITERABLE and C_PARAM_ITERABLE_OR_NULL */
static CREX_FUNCTION(crex_iterable)
{
	zval *arg1, *arg2;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_ITERABLE(arg1)
		C_PARAM_OPTIONAL
		C_PARAM_ITERABLE_OR_NULL(arg2)
	CREX_PARSE_PARAMETERS_END();
}

static CREX_FUNCTION(crex_iterable_legacy)
{
	zval *arg1, *arg2;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_ITERABLE(arg1)
		C_PARAM_OPTIONAL
		C_PARAM_ITERABLE_OR_NULL(arg2)
	CREX_PARSE_PARAMETERS_END();

	RETURN_COPY(arg1);
}

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_iterable_legacy, 0, 1, IS_ITERABLE, 0)
	CREX_ARG_TYPE_INFO(0, arg1, IS_ITERABLE, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg2, IS_ITERABLE, 1, "null")
CREX_END_ARG_INFO()

static const crex_function_entry ext_function_legacy[] = {
	CREX_FE(crex_iterable_legacy, arginfo_crex_iterable_legacy)
	CREX_FE_END
};

/* Call a method on a class or object using crex_call_method() */
static CREX_FUNCTION(crex_call_method)
{
	crex_string *method_name;
	zval *class_or_object, *arg1 = NULL, *arg2 = NULL;
	crex_object *obj = NULL;
	crex_class_entry *ce = NULL;
	int argc = CREX_NUM_ARGS();

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_ZVAL(class_or_object)
		C_PARAM_STR(method_name)
		C_PARAM_OPTIONAL
		C_PARAM_ZVAL(arg1)
		C_PARAM_ZVAL(arg2)
	CREX_PARSE_PARAMETERS_END();

	if (C_TYPE_P(class_or_object) == IS_OBJECT) {
		obj = C_OBJ_P(class_or_object);
		ce = obj->ce;
	} else if (C_TYPE_P(class_or_object) == IS_STRING) {
		ce = crex_lookup_class(C_STR_P(class_or_object));
		if (!ce) {
			crex_error(E_ERROR, "Unknown class '%s'", C_STRVAL_P(class_or_object));
			return;
		}
	} else {
		crex_argument_type_error(1, "must be of type object|string, %s given", crex_zval_value_name(class_or_object));
		return;
	}

	CREX_ASSERT((argc >= 2) && (argc <= 4));
	crex_call_method(obj, ce, NULL, ZSTR_VAL(method_name), ZSTR_LEN(method_name), return_value, argc - 2, arg1, arg2);
}

static CREX_FUNCTION(crex_get_unit_enum)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_OBJ_COPY(crex_enum_get_case_cstr(crex_test_unit_enum, "Foo"));
}

static CREX_FUNCTION(crex_test_crex_ini_parse_quantity)
{
	crex_string *str;
	crex_string *errstr;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_LONG(crex_ini_parse_quantity(str, &errstr));

	if (errstr) {
		crex_error(E_WARNING, "%s", ZSTR_VAL(errstr));
		crex_string_release(errstr);
	}
}

static CREX_FUNCTION(crex_test_crex_ini_parse_uquantity)
{
	crex_string *str;
	crex_string *errstr;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_LONG((crex_long)crex_ini_parse_uquantity(str, &errstr));

	if (errstr) {
		crex_error(E_WARNING, "%s", ZSTR_VAL(errstr));
		crex_string_release(errstr);
	}
}

static CREX_FUNCTION(crex_test_crex_ini_str)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_STR(ZT_G(str_test));
}

static CREX_FUNCTION(crex_test_is_string_marked_as_valid_utf8)
{
	crex_string *str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(str)
	CREX_PARSE_PARAMETERS_END();

	RETURN_BOOL(ZSTR_IS_VALID_UTF8(str));
}

static CREX_FUNCTION(CrexTestNS2_namespaced_func)
{
	CREX_PARSE_PARAMETERS_NONE();
	RETURN_TRUE;
}

static CREX_FUNCTION(CrexTestNS2_namespaced_deprecated_func)
{
	CREX_PARSE_PARAMETERS_NONE();
}

static CREX_FUNCTION(CrexTestNS2_CrexSubNS_namespaced_func)
{
	CREX_PARSE_PARAMETERS_NONE();
	RETURN_TRUE;
}

static CREX_FUNCTION(CrexTestNS2_CrexSubNS_namespaced_deprecated_func)
{
	CREX_PARSE_PARAMETERS_NONE();
}

static CREX_FUNCTION(crex_test_parameter_with_attribute)
{
	crex_string *parameter;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(parameter)
	CREX_PARSE_PARAMETERS_END();

	RETURN_LONG(1);
}

#ifdef CREX_CHECK_STACK_LIMIT
static CREX_FUNCTION(crex_test_crex_call_stack_get)
{
	crex_call_stack stack;

	CREX_PARSE_PARAMETERS_NONE();

	if (crex_call_stack_get(&stack)) {
		crex_string *str;

		array_init(return_value);

		str = strpprintf(0, "%p", stack.base);
		add_assoc_str(return_value, "base", str);

		str = strpprintf(0, "0x%zx", stack.max_size);
		add_assoc_str(return_value, "max_size", str);

		str = strpprintf(0, "%p", crex_call_stack_position());
		add_assoc_str(return_value, "position", str);

		str = strpprintf(0, "%p", EG(stack_limit));
		add_assoc_str(return_value, "EG(stack_limit)", str);

		return;
	}

	RETURN_NULL();
}

crex_long (*volatile crex_call_stack_use_all_fun)(void *limit);

static crex_long crex_call_stack_use_all(void *limit)
{
	if (crex_call_stack_overflowed(limit)) {
		return 1;
	}

	return 1 + crex_call_stack_use_all_fun(limit);
}

static CREX_FUNCTION(crex_test_crex_call_stack_use_all)
{
	crex_call_stack stack;

	CREX_PARSE_PARAMETERS_NONE();

	if (!crex_call_stack_get(&stack)) {
		return;
	}

	crex_call_stack_use_all_fun = crex_call_stack_use_all;

	void *limit = crex_call_stack_limit(stack.base, stack.max_size, 4096);

	RETURN_LONG(crex_call_stack_use_all(limit));
}
#endif /* CREX_CHECK_STACK_LIMIT */

static CREX_FUNCTION(crex_get_map_ptr_last)
{
	CREX_PARSE_PARAMETERS_NONE();
	RETURN_LONG(CG(map_ptr_last));
}

static CREX_FUNCTION(crex_test_crash)
{
	crex_string *message = NULL;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_NULL(message)
	CREX_PARSE_PARAMETERS_END();

	if (message) {
		crx_printf("%s", ZSTR_VAL(message));
	}

	char *invalid = (char *) 1;
	crx_printf("%s", invalid);
}

static CREX_FUNCTION(crex_test_fill_packed_array)
{
	HashTable *parameter;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ARRAY_HT_EX(parameter, 0, 1)
	CREX_PARSE_PARAMETERS_END();

	if (!HT_IS_PACKED(parameter)) {
		crex_argument_value_error(1, "must be a packed array");
		RETURN_THROWS();
	}

	crex_hash_extend(parameter, parameter->nNumUsed + 10, true);
	CREX_HASH_FILL_PACKED(parameter) {
		for (int i = 0; i < 10; i++) {
			zval value;
			ZVAL_LONG(&value, i);
			CREX_HASH_FILL_ADD(&value);
		}
	} CREX_HASH_FILL_END();
}

static CREX_FUNCTION(get_open_basedir)
{
	CREX_PARSE_PARAMETERS_NONE();
	if (PG(open_basedir)) {
		RETURN_STRING(PG(open_basedir));
	} else {
		RETURN_NULL();
	}
}

static CREX_FUNCTION(crex_test_is_pcre_bundled)
{
	CREX_PARSE_PARAMETERS_NONE();
#if HAVE_BUNDLED_PCRE
	RETURN_TRUE;
#else
	RETURN_FALSE;
#endif
}

static crex_object *crex_test_class_new(crex_class_entry *class_type)
{
	crex_object *obj = crex_objects_new(class_type);
	object_properties_init(obj, class_type);
	obj->handlers = &crex_test_class_handlers;
	return obj;
}

static crex_function *crex_test_class_method_get(crex_object **object, crex_string *name, const zval *key)
{
	if (crex_string_equals_literal_ci(name, "test")) {
	    crex_internal_function *fptr;

	    if (EXPECTED(EG(trampoline).common.function_name == NULL)) {
		    fptr = (crex_internal_function *) &EG(trampoline);
	    } else {
		    fptr = emalloc(sizeof(crex_internal_function));
	    }
	    memset(fptr, 0, sizeof(crex_internal_function));
	    fptr->type = CREX_INTERNAL_FUNCTION;
	    fptr->num_args = 1;
	    fptr->scope = (*object)->ce;
	    fptr->fn_flags = CREX_ACC_CALL_VIA_HANDLER;
	    fptr->function_name = crex_string_copy(name);
	    fptr->handler = CREX_FN(crex_test_func);

	    return (crex_function*)fptr;
	}
	return crex_std_get_method(object, name, key);
}

static crex_function *crex_test_class_static_method_get(crex_class_entry *ce, crex_string *name)
{
	if (crex_string_equals_literal_ci(name, "test")) {
		crex_internal_function *fptr;

		if (EXPECTED(EG(trampoline).common.function_name == NULL)) {
			fptr = (crex_internal_function *) &EG(trampoline);
		} else {
			fptr = emalloc(sizeof(crex_internal_function));
		}
		memset(fptr, 0, sizeof(crex_internal_function));
		fptr->type = CREX_INTERNAL_FUNCTION;
		fptr->num_args = 1;
		fptr->scope = ce;
		fptr->fn_flags = CREX_ACC_CALL_VIA_HANDLER|CREX_ACC_STATIC;
		fptr->function_name = crex_string_copy(name);
		fptr->handler = CREX_FN(crex_test_func);

		return (crex_function*)fptr;
	}
	return crex_std_get_static_method(ce, name, NULL);
}

void crex_attribute_validate_crextestattribute(crex_attribute *attr, uint32_t target, crex_class_entry *scope)
{
	if (target != CREX_ATTRIBUTE_TARGET_CLASS) {
		crex_error(E_COMPILE_ERROR, "Only classes can be marked with #[CrexTestAttribute]");
	}
}

static CREX_METHOD(_CrexTestClass, __toString)
{
	CREX_PARSE_PARAMETERS_NONE();
	RETURN_EMPTY_STRING();
}

/* Internal function returns bool, we return int. */
static CREX_METHOD(_CrexTestClass, is_object)
{
	CREX_PARSE_PARAMETERS_NONE();
	RETURN_LONG(42);
}

static CREX_METHOD(_CrexTestClass, returnsStatic) {
	CREX_PARSE_PARAMETERS_NONE();
	object_init_ex(return_value, crex_get_called_scope(execute_data));
}

static CREX_METHOD(_CrexTestClass, returnsThrowable)
{
	CREX_PARSE_PARAMETERS_NONE();
	crex_throw_error(NULL, "Dummy");
}

static CREX_METHOD(_CrexTestClass, variadicTest) {
	int      argc, i;
	zval    *args = NULL;

	CREX_PARSE_PARAMETERS_START(0, -1)
		C_PARAM_VARIADIC('*', args, argc)
	CREX_PARSE_PARAMETERS_END();

	for (i = 0; i < argc; i++) {
		zval *arg = args + i;

		if (C_TYPE_P(arg) == IS_STRING) {
			continue;
		}
		if (C_TYPE_P(arg) == IS_OBJECT && instanceof_function(C_OBJ_P(arg)->ce, crex_ce_iterator)) {
			continue;
		}

		crex_argument_type_error(i + 1, "must be of class Iterator or a string, %s given", crex_zval_type_name(arg));
		RETURN_THROWS();
	}

	object_init_ex(return_value, crex_get_called_scope(execute_data));
}

static CREX_METHOD(_CrexTestChildClass, returnsThrowable)
{
	CREX_PARSE_PARAMETERS_NONE();
	crex_throw_error(NULL, "Dummy");
}

static CREX_METHOD(CrexAttributeTest, testMethod)
{
	CREX_PARSE_PARAMETERS_NONE();
	RETURN_TRUE;
}

static CREX_METHOD(_CrexTestTrait, testMethod)
{
	CREX_PARSE_PARAMETERS_NONE();
	RETURN_TRUE;
}

static CREX_METHOD(CrexTestNS_Foo, method)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_LONG(0);
}

static CREX_METHOD(CrexTestNS_UnlikelyCompileError, method)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_NULL();
}

static CREX_METHOD(CrexTestNS_NotUnlikelyCompileError, method)
{
	CREX_PARSE_PARAMETERS_NONE();

	RETURN_NULL();
}

static CREX_METHOD(CrexTestNS2_Foo, method)
{
	CREX_PARSE_PARAMETERS_NONE();
}

static CREX_METHOD(CrexTestNS2_CrexSubNS_Foo, method)
{
	CREX_PARSE_PARAMETERS_NONE();
}

static CREX_METHOD(CrexTestParameterAttribute, __main)
{
	crex_string *parameter;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(parameter)
	CREX_PARSE_PARAMETERS_END();

	ZVAL_STR_COPY(OBJ_PROP_NUM(C_OBJ_P(CREX_THIS), 0), parameter);
}

static CREX_METHOD(CrexTestPropertyAttribute, __main)
{
	crex_string *parameter;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(parameter)
	CREX_PARSE_PARAMETERS_END();

	ZVAL_STR_COPY(OBJ_PROP_NUM(C_OBJ_P(CREX_THIS), 0), parameter);
}

static CREX_METHOD(CrexTestClassWithMethodWithParameterAttribute, no_override)
{
	crex_string *parameter;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(parameter)
	CREX_PARSE_PARAMETERS_END();

	RETURN_LONG(2);
}

static CREX_METHOD(CrexTestClassWithMethodWithParameterAttribute, override)
{
	crex_string *parameter;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(parameter)
	CREX_PARSE_PARAMETERS_END();

	RETURN_LONG(3);
}

static CREX_METHOD(CrexTestChildClassWithMethodWithParameterAttribute, override)
{
	crex_string *parameter;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STR(parameter)
	CREX_PARSE_PARAMETERS_END();

	RETURN_LONG(4);
}

static CREX_METHOD(CrexTestForbidDynamicCall, call)
{
	CREX_PARSE_PARAMETERS_NONE();

	crex_forbid_dynamic_call();
}

static CREX_METHOD(CrexTestForbidDynamicCall, callStatic)
{
	CREX_PARSE_PARAMETERS_NONE();

	crex_forbid_dynamic_call();
}

CRX_INI_BEGIN()
	STD_CRX_INI_BOOLEAN("crex_test.replace_crex_execute_ex", "0", CRX_INI_SYSTEM, OnUpdateBool, replace_crex_execute_ex, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_BOOLEAN("crex_test.register_passes", "0", CRX_INI_SYSTEM, OnUpdateBool, register_passes, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_BOOLEAN("crex_test.print_stderr_mshutdown", "0", CRX_INI_SYSTEM, OnUpdateBool, print_stderr_mshutdown, crex_crex_test_globals, crex_test_globals)
#ifdef HAVE_COPY_FILE_RANGE
	STD_CRX_INI_ENTRY("crex_test.limit_copy_file_range", "-1", CRX_INI_ALL, OnUpdateLong, limit_copy_file_range, crex_crex_test_globals, crex_test_globals)
#endif
	STD_CRX_INI_ENTRY("crex_test.quantity_value", "0", CRX_INI_ALL, OnUpdateLong, quantity_value, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_ENTRY("crex_test.str_test", "", CRX_INI_ALL, OnUpdateStr, str_test, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_ENTRY("crex_test.not_empty_str_test", "val", CRX_INI_ALL, OnUpdateStrNotEmpty, not_empty_str_test, crex_crex_test_globals, crex_test_globals)
CRX_INI_END()

void (*old_crex_execute_ex)(crex_execute_data *execute_data);
static void custom_crex_execute_ex(crex_execute_data *execute_data)
{
	old_crex_execute_ex(execute_data);
}

static void le_throwing_resource_dtor(crex_resource *rsrc)
{
	crex_throw_exception(NULL, "Throwing resource destructor called", 0);
}

static CREX_METHOD(_CrexTestClass, takesUnionType)
{
	crex_object *obj;
	CREX_PARSE_PARAMETERS_START(1, 1);
		C_PARAM_OBJ(obj)
	CREX_PARSE_PARAMETERS_END();
	// we have to perform type-checking to avoid arginfo/zpp mismatch error
	bool type_matches = (
		instanceof_function(obj->ce, crex_standard_class_def)
		||
		instanceof_function(obj->ce, crex_ce_iterator)
	);
	if (!type_matches) {
		crex_string *ty = crex_type_to_string(execute_data->func->internal_function.arg_info->type);
		crex_argument_type_error(1, "must be of type %s, %s given", ty->val, obj->ce->name->val);
		crex_string_release(ty);
		RETURN_THROWS();
	}

	RETURN_NULL();
}

// Returns a newly allocated DNF type `Iterator|(Traversable&Countable)`.
//
// We need to generate it "manually" because gen_stubs.crx does not support codegen for DNF types ATM.
static crex_type create_test_dnf_type(void) {
	crex_string *class_Iterator = crex_string_init_interned("Iterator", sizeof("Iterator") - 1, true);
	crex_alloc_ce_cache(class_Iterator);
	crex_string *class_Traversable = ZSTR_KNOWN(CREX_STR_TRAVERSABLE);
	crex_string *class_Countable = crex_string_init_interned("Countable", sizeof("Countable") - 1, true);
	crex_alloc_ce_cache(class_Countable);
	//
	crex_type_list *intersection_list = malloc(CREX_TYPE_LIST_SIZE(2));
	intersection_list->num_types = 2;
	intersection_list->types[0] = (crex_type) CREX_TYPE_INIT_CLASS(class_Traversable, 0, 0);
	intersection_list->types[1] = (crex_type) CREX_TYPE_INIT_CLASS(class_Countable, 0, 0);
	crex_type_list *union_list = malloc(CREX_TYPE_LIST_SIZE(2));
	union_list->num_types = 2;
	union_list->types[0] = (crex_type) CREX_TYPE_INIT_CLASS(class_Iterator, 0, 0);
	union_list->types[1] = (crex_type) CREX_TYPE_INIT_INTERSECTION(intersection_list, 0);
	return (crex_type) CREX_TYPE_INIT_UNION(union_list, 0);
}

static void register_CrexTestClass_dnf_property(crex_class_entry *ce) {
	crex_string *prop_name = crex_string_init_interned("dnfProperty", sizeof("dnfProperty") - 1, true);
	zval default_value;
	ZVAL_UNDEF(&default_value);
	crex_type type = create_test_dnf_type();
	crex_declare_typed_property(ce, prop_name, &default_value, CREX_ACC_PUBLIC, NULL, type);
}

// arg_info for `crex_test_internal_dnf_arguments`
// The types are upgraded to DNF types in `register_dynamic_function_entries()`
static crex_internal_arg_info arginfo_crex_test_internal_dnf_arguments[] = {
	// first entry is a crex_internal_function_info (see crex_compile.h): {argument_count, return_type, unused}
	{(const char*)(uintptr_t)(1), {0}, NULL},
	{"arg", {0}, NULL}
};

static CREX_NAMED_FUNCTION(crex_test_internal_dnf_arguments)
{
	crex_object *obj;
	CREX_PARSE_PARAMETERS_START(1, 1);
		C_PARAM_OBJ(obj)
	CREX_PARSE_PARAMETERS_END();
	// we have to perform type-checking to avoid arginfo/zpp mismatch error
	bool type_matches = (
		instanceof_function(obj->ce, crex_ce_iterator)
		|| (
			instanceof_function(obj->ce, crex_ce_traversable)
			&& instanceof_function(obj->ce, crex_ce_countable)
		)
	);
	if (!type_matches) {
		crex_string *ty = crex_type_to_string(arginfo_crex_test_internal_dnf_arguments[1].type);
		crex_argument_type_error(1, "must be of type %s, %s given", ty->val, obj->ce->name->val);
		crex_string_release(ty);
		RETURN_THROWS();
	}

	RETURN_OBJ_COPY(obj);
}

static const crex_function_entry dynamic_function_entries[] = {
	{
		.fname = "crex_test_internal_dnf_arguments",
		.handler = crex_test_internal_dnf_arguments,
		.arg_info = arginfo_crex_test_internal_dnf_arguments,
		.num_args = 1,
		.flags = 0,
	},
	CREX_FE_END,
};

static void register_dynamic_function_entries(int module_type) {
	// return-type is at index 0
	arginfo_crex_test_internal_dnf_arguments[0].type = create_test_dnf_type();
	arginfo_crex_test_internal_dnf_arguments[1].type = create_test_dnf_type();
	//
	crex_register_functions(NULL, dynamic_function_entries, NULL, module_type);
}

CRX_MINIT_FUNCTION(crex_test)
{
	register_dynamic_function_entries(type);

	crex_test_interface = register_class__CrexTestInterface();

	crex_test_class = register_class__CrexTestClass(crex_test_interface);
	register_CrexTestClass_dnf_property(crex_test_class);
	crex_test_class->create_object = crex_test_class_new;
	crex_test_class->get_static_method = crex_test_class_static_method_get;

	crex_test_child_class = register_class__CrexTestChildClass(crex_test_class);

	memcpy(&crex_test_class_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	crex_test_class_handlers.get_method = crex_test_class_method_get;

	crex_attribute_test_class = register_class_CrexAttributeTest();

	crex_test_trait = register_class__CrexTestTrait();

	register_test_symbols(module_number);

	crex_test_attribute = register_class_CrexTestAttribute();
	{
		crex_internal_attribute *attr = crex_mark_internal_attribute(crex_test_attribute);
		attr->validator = crex_attribute_validate_crextestattribute;
	}

	crex_test_repeatable_attribute = register_class_CrexTestRepeatableAttribute();
	crex_mark_internal_attribute(crex_test_repeatable_attribute);

	crex_test_parameter_attribute = register_class_CrexTestParameterAttribute();
	crex_mark_internal_attribute(crex_test_parameter_attribute);

	crex_test_property_attribute = register_class_CrexTestPropertyAttribute();
	crex_mark_internal_attribute(crex_test_property_attribute);

	crex_test_class_with_method_with_parameter_attribute = register_class_CrexTestClassWithMethodWithParameterAttribute();
	crex_test_child_class_with_method_with_parameter_attribute = register_class_CrexTestChildClassWithMethodWithParameterAttribute(crex_test_class_with_method_with_parameter_attribute);

	crex_test_class_with_property_attribute = register_class_CrexTestClassWithPropertyAttribute();
	{
		crex_property_info *prop_info = crex_hash_str_find_ptr(&crex_test_class_with_property_attribute->properties_info, "attributed", sizeof("attributed") - 1);
		crex_add_property_attribute(crex_test_class_with_property_attribute, prop_info, crex_test_attribute->name, 0);
	}

	crex_test_forbid_dynamic_call = register_class_CrexTestForbidDynamicCall();

	crex_test_ns_foo_class = register_class_CrexTestNS_Foo();
	crex_test_ns_unlikely_compile_error_class = register_class_CrexTestNS_UnlikelyCompileError();
	crex_test_ns_not_unlikely_compile_error_class = register_class_CrexTestNS_NotUnlikelyCompileError();
	crex_test_ns2_foo_class = register_class_CrexTestNS2_Foo();
	crex_test_ns2_ns_foo_class = register_class_CrexTestNS2_CrexSubNS_Foo();

	crex_test_unit_enum = register_class_CrexTestUnitEnum();
	crex_test_string_enum = register_class_CrexTestStringEnum();
	crex_test_int_enum = register_class_CrexTestIntEnum();

	crex_register_functions(NULL, ext_function_legacy, NULL, EG(current_module)->type);

	// Loading via dl() not supported with the observer API
	if (type != MODULE_TEMPORARY) {
		REGISTER_INI_ENTRIES();
	} else {
		(void)ini_entries;
	}

	if (ZT_G(replace_crex_execute_ex)) {
		old_crex_execute_ex = crex_execute_ex;
		crex_execute_ex = custom_crex_execute_ex;
	}

	if (ZT_G(register_passes)) {
		crex_optimizer_register_pass(pass1);
		crex_optimizer_register_pass(pass2);
	}

	crex_test_observer_init(INIT_FUNC_ARGS_PASSTHRU);
	crex_test_fiber_init();
	crex_test_iterators_init();
	crex_test_object_handlers_init();

	le_throwing_resource = crex_register_list_destructors_ex(le_throwing_resource_dtor, NULL, "throwing resource", module_number);

	return SUCCESS;
}

CRX_MSHUTDOWN_FUNCTION(crex_test)
{
	if (type != MODULE_TEMPORARY) {
		UNREGISTER_INI_ENTRIES();
	}

	crex_test_observer_shutdown(SHUTDOWN_FUNC_ARGS_PASSTHRU);

	if (ZT_G(print_stderr_mshutdown)) {
		fprintf(stderr, "[crex-test] MSHUTDOWN\n");
	}

	return SUCCESS;
}

CRX_RINIT_FUNCTION(crex_test)
{
	crex_hash_init(&ZT_G(global_weakmap), 8, NULL, ZVAL_PTR_DTOR, 0);
	ZT_G(observer_nesting_depth) = 0;
	return SUCCESS;
}

CRX_RSHUTDOWN_FUNCTION(crex_test)
{
	crex_ulong obj_key;
	CREX_HASH_FOREACH_NUM_KEY(&ZT_G(global_weakmap), obj_key) {
		crex_weakrefs_hash_del(&ZT_G(global_weakmap), crex_weakref_key_to_object(obj_key));
	} CREX_HASH_FOREACH_END();
	crex_hash_destroy(&ZT_G(global_weakmap));
	return SUCCESS;
}

static CRX_GINIT_FUNCTION(crex_test)
{
#if defined(COMPILE_DL_CREX_TEST) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	memset(crex_test_globals, 0, sizeof(*crex_test_globals));

	crex_test_observer_ginit(crex_test_globals);
}

static CRX_GSHUTDOWN_FUNCTION(crex_test)
{
	crex_test_observer_gshutdown(crex_test_globals);
}

CRX_MINFO_FUNCTION(crex_test)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "crex_test extension", "enabled");
	crx_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}

crex_module_entry crex_test_module_entry = {
	STANDARD_MODULE_HEADER,
	"crex_test",
	ext_functions,
	CRX_MINIT(crex_test),
	CRX_MSHUTDOWN(crex_test),
	CRX_RINIT(crex_test),
	CRX_RSHUTDOWN(crex_test),
	CRX_MINFO(crex_test),
	CRX_CREX_TEST_VERSION,
	CRX_MODULE_GLOBALS(crex_test),
	CRX_GINIT(crex_test),
	CRX_GSHUTDOWN(crex_test),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_CREX_TEST
# ifdef ZTS
CREX_TSRMLS_CACHE_DEFINE()
# endif
CREX_GET_MODULE(crex_test)
#endif

/* The important part here is the CREX_FASTCALL. */
CRX_CREX_TEST_API int CREX_FASTCALL bug78270(const char *str, size_t str_len)
{
	char * copy = crex_strndup(str, str_len);
	int r = (int) CREX_ATOL(copy);
	free(copy);
	return r;
}

CRX_CREX_TEST_API struct bug79096 bug79096(void)
{
	struct bug79096 b;

	b.a = 1;
	b.b = 1;
	return b;
}

CRX_CREX_TEST_API void bug79532(off_t *array, size_t elems)
{
	int i;
	for (i = 0; i < elems; i++) {
		array[i] = i;
	}
}

CRX_CREX_TEST_API int *(*bug79177_cb)(void);
void bug79177(void)
{
	bug79177_cb();
}

typedef struct bug80847_01 {
	uint64_t b;
	double c;
} bug80847_01;
typedef struct bug80847_02 {
	bug80847_01 a;
} bug80847_02;

CRX_CREX_TEST_API bug80847_02 ffi_bug80847(bug80847_02 s) {
	s.a.b += 10;
	s.a.c -= 10.0;
	return s;
}

CRX_CREX_TEST_API void (*bug_gh9090_void_none_ptr)(void) = NULL;
CRX_CREX_TEST_API void (*bug_gh9090_void_int_char_ptr)(int, char *) = NULL;
CRX_CREX_TEST_API void (*bug_gh9090_void_int_char_var_ptr)(int, char *, ...) = NULL;
CRX_CREX_TEST_API void (*bug_gh9090_void_char_int_ptr)(char *, int) = NULL;
CRX_CREX_TEST_API int (*bug_gh9090_int_int_char_ptr)(int, char *) = NULL;

CRX_CREX_TEST_API void bug_gh9090_void_none(void) {
    crx_printf("bug_gh9090_none\n");
}

CRX_CREX_TEST_API void bug_gh9090_void_int_char(int i, char *s) {
    crx_printf("bug_gh9090_int_char %d %s\n", i, s);
}

CRX_CREX_TEST_API void bug_gh9090_void_int_char_var(int i, char *fmt, ...) {
    va_list args;
    char *buffer;

    va_start(args, fmt);

    crex_vspprintf(&buffer, 0, fmt, args);
    crx_printf("bug_gh9090_void_int_char_var %s\n", buffer);
    efree(buffer);

    va_end(args);
}

CRX_CREX_TEST_API int gh11934b_ffi_var_test_cdata;

#ifdef HAVE_COPY_FILE_RANGE
/**
 * This function allows us to simulate early return of copy_file_range by setting the limit_copy_file_range ini setting.
 */
CRX_CREX_TEST_API ssize_t copy_file_range(int fd_in, off64_t *off_in, int fd_out, off64_t *off_out, size_t len, unsigned int flags)
{
	ssize_t (*original_copy_file_range)(int, off64_t *, int, off64_t *, size_t, unsigned int) = dlsym(RTLD_NEXT, "copy_file_range");
	if (ZT_G(limit_copy_file_range) >= C_L(0)) {
		len = ZT_G(limit_copy_file_range);
	}
	return original_copy_file_range(fd_in, off_in, fd_out, off_out, len, flags);
}
#endif


static CRX_FUNCTION(crex_test_create_throwing_resource)
{
	CREX_PARSE_PARAMETERS_NONE();
	crex_resource *res = crex_register_resource(NULL, le_throwing_resource);
	ZVAL_RES(return_value, res);
}
