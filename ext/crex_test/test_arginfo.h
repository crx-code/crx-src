/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 1ac529029c01af5d6698f06c7e5f74b7149ea749 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_test_array_return, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_test_nullable_array_return, 0, 0, IS_ARRAY, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_test_void_return, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_test_compile_string, 0, 3, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, source_string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, position, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_test_deprecated, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arg, IS_MIXED, 0, "null")
CREX_END_ARG_INFO()

#define arginfo_crex_test_aliased arginfo_crex_test_void_return

#define arginfo_crex_test_deprecated_aliased arginfo_crex_test_void_return

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_create_unterminated_string, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, str, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_terminate_string, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(1, str, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_leak_variable, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, variable, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_leak_bytes, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, bytes, IS_LONG, 0, "3")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_crex_string_or_object, 0, 1, MAY_BE_OBJECT|MAY_BE_STRING)
	CREX_ARG_TYPE_MASK(0, param, MAY_BE_OBJECT|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_crex_string_or_object_or_null, 0, 1, MAY_BE_OBJECT|MAY_BE_STRING|MAY_BE_NULL)
	CREX_ARG_TYPE_MASK(0, param, MAY_BE_OBJECT|MAY_BE_STRING|MAY_BE_NULL, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_crex_string_or_stdclass, 0, 1, stdClass, MAY_BE_STRING)
	CREX_ARG_INFO(0, param)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_crex_string_or_stdclass_or_null, 0, 1, stdClass, MAY_BE_STRING|MAY_BE_NULL)
	CREX_ARG_INFO(0, param)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_crex_number_or_string, 0, 1, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_DOUBLE)
	CREX_ARG_TYPE_MASK(0, param, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_DOUBLE, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_crex_number_or_string_or_null, 0, 1, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_NULL)
	CREX_ARG_TYPE_MASK(0, param, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_NULL, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_iterable, 0, 1, IS_VOID, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, arg1, Traversable, MAY_BE_ARRAY, NULL)
	CREX_ARG_OBJ_TYPE_MASK(0, arg2, Traversable, MAY_BE_ARRAY|MAY_BE_NULL, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_weakmap_attach, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, object, IS_OBJECT, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_weakmap_remove, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, object, IS_OBJECT, 0)
CREX_END_ARG_INFO()

#define arginfo_crex_weakmap_dump arginfo_crex_test_array_return

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_crex_get_unit_enum, 0, 0, CrexTestUnitEnum, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_test_parameter_with_attribute, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, parameter, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_get_current_func_name, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_call_method, 0, 2, IS_MIXED, 0)
	CREX_ARG_TYPE_MASK(0, obj_or_class, MAY_BE_OBJECT|MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO(0, method, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, arg1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, arg2, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_test_crex_ini_parse_quantity, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, str, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_crex_test_crex_ini_parse_uquantity arginfo_crex_test_crex_ini_parse_quantity

#define arginfo_crex_test_crex_ini_str arginfo_crex_get_current_func_name

#if defined(CREX_CHECK_STACK_LIMIT)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_test_crex_call_stack_get, 0, 0, IS_ARRAY, 1)
CREX_END_ARG_INFO()
#endif

#if defined(CREX_CHECK_STACK_LIMIT)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_test_crex_call_stack_use_all, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_test_is_string_marked_as_valid_utf8, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_get_map_ptr_last, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_test_crash, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, message, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_test_fill_packed_array, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(1, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_crex_test_create_throwing_resource, 0, 0, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_get_open_basedir, 0, 0, IS_STRING, 1)
CREX_END_ARG_INFO()

#if defined(HAVE_LIBXML) && !defined(CRX_WIN32)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_test_override_libxml_global_state, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_test_is_pcre_bundled, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_CrexTestNS2_namespaced_func arginfo_crex_test_is_pcre_bundled

#define arginfo_CrexTestNS2_namespaced_deprecated_func arginfo_crex_test_void_return

#define arginfo_CrexTestNS2_namespaced_aliased_func arginfo_crex_test_void_return

#define arginfo_CrexTestNS2_namespaced_deprecated_aliased_func arginfo_crex_test_void_return

#define arginfo_CrexTestNS2_CrexSubNS_namespaced_func arginfo_crex_test_is_pcre_bundled

#define arginfo_CrexTestNS2_CrexSubNS_namespaced_deprecated_func arginfo_crex_test_void_return

#define arginfo_CrexTestNS2_CrexSubNS_namespaced_aliased_func arginfo_crex_test_void_return

#define arginfo_CrexTestNS2_CrexSubNS_namespaced_deprecated_aliased_func arginfo_crex_test_void_return

#define arginfo_class__CrexTestClass_is_object arginfo_crex_get_map_ptr_last

#define arginfo_class__CrexTestClass___toString arginfo_crex_get_current_func_name

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class__CrexTestClass_returnsStatic, 0, 0, IS_STATIC, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class__CrexTestClass_returnsThrowable, 0, 0, Throwable, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class__CrexTestClass_variadicTest, 0, 0, IS_STATIC, 0)
	CREX_ARG_VARIADIC_OBJ_TYPE_MASK(0, elements, Iterator, MAY_BE_STRING)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class__CrexTestClass_takesUnionType, 0, 1, IS_VOID, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, arg, stdclass|Iterator, 0, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class__CrexTestChildClass_returnsThrowable, 0, 0, Exception, 0)
CREX_END_ARG_INFO()

#define arginfo_class_CrexAttributeTest_testMethod arginfo_crex_test_is_pcre_bundled

#define arginfo_class__CrexTestTrait_testMethod arginfo_crex_test_is_pcre_bundled

CREX_BEGIN_ARG_INFO_EX(arginfo_class_CrexTestParameterAttribute___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, parameter, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_CrexTestPropertyAttribute___main arginfo_class_CrexTestParameterAttribute___main

#define arginfo_class_CrexTestClassWithMethodWithParameterAttribute_no_override arginfo_crex_test_parameter_with_attribute

#define arginfo_class_CrexTestClassWithMethodWithParameterAttribute_override arginfo_crex_test_parameter_with_attribute

#define arginfo_class_CrexTestChildClassWithMethodWithParameterAttribute_override arginfo_crex_test_parameter_with_attribute

#define arginfo_class_CrexTestForbidDynamicCall_call arginfo_crex_test_void_return

#define arginfo_class_CrexTestForbidDynamicCall_callStatic arginfo_crex_test_void_return

#if (CRX_VERSION_ID >= 80100)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_CrexTestNS_Foo_method, 0, 0, IS_LONG, 0)
#else
CREX_BEGIN_ARG_INFO_EX(arginfo_class_CrexTestNS_Foo_method, 0, 0, 0)
#endif
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_CrexTestNS_UnlikelyCompileError_method, 0, 0, CrexTestNS\\\125nlikelyCompileError, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_CrexTestNS_NotUnlikelyCompileError_method, 0, 0, CrexTestNS\\\116otUnlikelyCompileError, 1)
CREX_END_ARG_INFO()

#define arginfo_class_CrexTestNS2_Foo_method arginfo_crex_test_void_return

#define arginfo_class_CrexTestNS2_CrexSubNS_Foo_method arginfo_crex_test_void_return


static CREX_FUNCTION(crex_test_array_return);
static CREX_FUNCTION(crex_test_nullable_array_return);
static CREX_FUNCTION(crex_test_void_return);
static CREX_FUNCTION(crex_test_compile_string);
static CREX_FUNCTION(crex_test_deprecated);
static CREX_FUNCTION(crex_create_unterminated_string);
static CREX_FUNCTION(crex_terminate_string);
static CREX_FUNCTION(crex_leak_variable);
static CREX_FUNCTION(crex_leak_bytes);
static CREX_FUNCTION(crex_string_or_object);
static CREX_FUNCTION(crex_string_or_object_or_null);
static CREX_FUNCTION(crex_string_or_stdclass);
static CREX_FUNCTION(crex_string_or_stdclass_or_null);
static CREX_FUNCTION(crex_number_or_string);
static CREX_FUNCTION(crex_number_or_string_or_null);
static CREX_FUNCTION(crex_iterable);
static CREX_FUNCTION(crex_weakmap_attach);
static CREX_FUNCTION(crex_weakmap_remove);
static CREX_FUNCTION(crex_weakmap_dump);
static CREX_FUNCTION(crex_get_unit_enum);
static CREX_FUNCTION(crex_test_parameter_with_attribute);
static CREX_FUNCTION(crex_get_current_func_name);
static CREX_FUNCTION(crex_call_method);
static CREX_FUNCTION(crex_test_crex_ini_parse_quantity);
static CREX_FUNCTION(crex_test_crex_ini_parse_uquantity);
static CREX_FUNCTION(crex_test_crex_ini_str);
#if defined(CREX_CHECK_STACK_LIMIT)
static CREX_FUNCTION(crex_test_crex_call_stack_get);
#endif
#if defined(CREX_CHECK_STACK_LIMIT)
static CREX_FUNCTION(crex_test_crex_call_stack_use_all);
#endif
static CREX_FUNCTION(crex_test_is_string_marked_as_valid_utf8);
static CREX_FUNCTION(crex_get_map_ptr_last);
static CREX_FUNCTION(crex_test_crash);
static CREX_FUNCTION(crex_test_fill_packed_array);
static CREX_FUNCTION(crex_test_create_throwing_resource);
static CREX_FUNCTION(get_open_basedir);
#if defined(HAVE_LIBXML) && !defined(CRX_WIN32)
static CREX_FUNCTION(crex_test_override_libxml_global_state);
#endif
static CREX_FUNCTION(crex_test_is_pcre_bundled);
static CREX_FUNCTION(CrexTestNS2_namespaced_func);
static CREX_FUNCTION(CrexTestNS2_namespaced_deprecated_func);
static CREX_FUNCTION(CrexTestNS2_CrexSubNS_namespaced_func);
static CREX_FUNCTION(CrexTestNS2_CrexSubNS_namespaced_deprecated_func);
static CREX_METHOD(_CrexTestClass, is_object);
static CREX_METHOD(_CrexTestClass, __toString);
static CREX_METHOD(_CrexTestClass, returnsStatic);
static CREX_METHOD(_CrexTestClass, returnsThrowable);
static CREX_METHOD(_CrexTestClass, variadicTest);
static CREX_METHOD(_CrexTestClass, takesUnionType);
static CREX_METHOD(_CrexTestChildClass, returnsThrowable);
static CREX_METHOD(CrexAttributeTest, testMethod);
static CREX_METHOD(_CrexTestTrait, testMethod);
static CREX_METHOD(CrexTestParameterAttribute, __main);
static CREX_METHOD(CrexTestPropertyAttribute, __main);
static CREX_METHOD(CrexTestClassWithMethodWithParameterAttribute, no_override);
static CREX_METHOD(CrexTestClassWithMethodWithParameterAttribute, override);
static CREX_METHOD(CrexTestChildClassWithMethodWithParameterAttribute, override);
static CREX_METHOD(CrexTestForbidDynamicCall, call);
static CREX_METHOD(CrexTestForbidDynamicCall, callStatic);
static CREX_METHOD(CrexTestNS_Foo, method);
static CREX_METHOD(CrexTestNS_UnlikelyCompileError, method);
static CREX_METHOD(CrexTestNS_NotUnlikelyCompileError, method);
static CREX_METHOD(CrexTestNS2_Foo, method);
static CREX_METHOD(CrexTestNS2_CrexSubNS_Foo, method);


static const crex_function_entry ext_functions[] = {
	CREX_FE(crex_test_array_return, arginfo_crex_test_array_return)
	CREX_FE(crex_test_nullable_array_return, arginfo_crex_test_nullable_array_return)
	CREX_FE(crex_test_void_return, arginfo_crex_test_void_return)
	CREX_FE(crex_test_compile_string, arginfo_crex_test_compile_string)
	CREX_DEP_FE(crex_test_deprecated, arginfo_crex_test_deprecated)
	CREX_FALIAS(crex_test_aliased, crex_test_void_return, arginfo_crex_test_aliased)
	CREX_DEP_FALIAS(crex_test_deprecated_aliased, crex_test_void_return, arginfo_crex_test_deprecated_aliased)
	CREX_FE(crex_create_unterminated_string, arginfo_crex_create_unterminated_string)
	CREX_FE(crex_terminate_string, arginfo_crex_terminate_string)
	CREX_FE(crex_leak_variable, arginfo_crex_leak_variable)
	CREX_FE(crex_leak_bytes, arginfo_crex_leak_bytes)
	CREX_FE(crex_string_or_object, arginfo_crex_string_or_object)
	CREX_FE(crex_string_or_object_or_null, arginfo_crex_string_or_object_or_null)
	CREX_FE(crex_string_or_stdclass, arginfo_crex_string_or_stdclass)
	CREX_FE(crex_string_or_stdclass_or_null, arginfo_crex_string_or_stdclass_or_null)
	CREX_FE(crex_number_or_string, arginfo_crex_number_or_string)
	CREX_FE(crex_number_or_string_or_null, arginfo_crex_number_or_string_or_null)
	CREX_FE(crex_iterable, arginfo_crex_iterable)
	CREX_FE(crex_weakmap_attach, arginfo_crex_weakmap_attach)
	CREX_FE(crex_weakmap_remove, arginfo_crex_weakmap_remove)
	CREX_FE(crex_weakmap_dump, arginfo_crex_weakmap_dump)
	CREX_FE(crex_get_unit_enum, arginfo_crex_get_unit_enum)
	CREX_FE(crex_test_parameter_with_attribute, arginfo_crex_test_parameter_with_attribute)
	CREX_FE(crex_get_current_func_name, arginfo_crex_get_current_func_name)
	CREX_FE(crex_call_method, arginfo_crex_call_method)
	CREX_FE(crex_test_crex_ini_parse_quantity, arginfo_crex_test_crex_ini_parse_quantity)
	CREX_FE(crex_test_crex_ini_parse_uquantity, arginfo_crex_test_crex_ini_parse_uquantity)
	CREX_FE(crex_test_crex_ini_str, arginfo_crex_test_crex_ini_str)
#if defined(CREX_CHECK_STACK_LIMIT)
	CREX_FE(crex_test_crex_call_stack_get, arginfo_crex_test_crex_call_stack_get)
#endif
#if defined(CREX_CHECK_STACK_LIMIT)
	CREX_FE(crex_test_crex_call_stack_use_all, arginfo_crex_test_crex_call_stack_use_all)
#endif
	CREX_FE(crex_test_is_string_marked_as_valid_utf8, arginfo_crex_test_is_string_marked_as_valid_utf8)
	CREX_FE(crex_get_map_ptr_last, arginfo_crex_get_map_ptr_last)
	CREX_FE(crex_test_crash, arginfo_crex_test_crash)
	CREX_FE(crex_test_fill_packed_array, arginfo_crex_test_fill_packed_array)
	CREX_FE(crex_test_create_throwing_resource, arginfo_crex_test_create_throwing_resource)
	CREX_FE(get_open_basedir, arginfo_get_open_basedir)
#if defined(HAVE_LIBXML) && !defined(CRX_WIN32)
	CREX_FE(crex_test_override_libxml_global_state, arginfo_crex_test_override_libxml_global_state)
#endif
	CREX_FE(crex_test_is_pcre_bundled, arginfo_crex_test_is_pcre_bundled)
	CREX_NS_FALIAS("CrexTestNS2", namespaced_func, CrexTestNS2_namespaced_func, arginfo_CrexTestNS2_namespaced_func)
	CREX_NS_DEP_FALIAS("CrexTestNS2", namespaced_deprecated_func, CrexTestNS2_namespaced_deprecated_func, arginfo_CrexTestNS2_namespaced_deprecated_func)
	CREX_NS_FALIAS("CrexTestNS2", namespaced_aliased_func, crex_test_void_return, arginfo_CrexTestNS2_namespaced_aliased_func)
	CREX_NS_DEP_FALIAS("CrexTestNS2", namespaced_deprecated_aliased_func, crex_test_void_return, arginfo_CrexTestNS2_namespaced_deprecated_aliased_func)
	CREX_NS_FALIAS("CrexTestNS2\\CrexSubNS", namespaced_func, CrexTestNS2_CrexSubNS_namespaced_func, arginfo_CrexTestNS2_CrexSubNS_namespaced_func)
	CREX_NS_DEP_FALIAS("CrexTestNS2\\CrexSubNS", namespaced_deprecated_func, CrexTestNS2_CrexSubNS_namespaced_deprecated_func, arginfo_CrexTestNS2_CrexSubNS_namespaced_deprecated_func)
	CREX_NS_FALIAS("CrexTestNS2\\CrexSubNS", namespaced_aliased_func, crex_test_void_return, arginfo_CrexTestNS2_CrexSubNS_namespaced_aliased_func)
	CREX_NS_DEP_FALIAS("CrexTestNS2\\CrexSubNS", namespaced_deprecated_aliased_func, crex_test_void_return, arginfo_CrexTestNS2_CrexSubNS_namespaced_deprecated_aliased_func)
	CREX_FE_END
};


static const crex_function_entry class__CrexTestInterface_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class__CrexTestClass_methods[] = {
	CREX_ME(_CrexTestClass, is_object, arginfo_class__CrexTestClass_is_object, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(_CrexTestClass, __toString, arginfo_class__CrexTestClass___toString, CREX_ACC_PUBLIC|CREX_ACC_DEPRECATED)
	CREX_ME(_CrexTestClass, returnsStatic, arginfo_class__CrexTestClass_returnsStatic, CREX_ACC_PUBLIC)
	CREX_ME(_CrexTestClass, returnsThrowable, arginfo_class__CrexTestClass_returnsThrowable, CREX_ACC_PUBLIC)
	CREX_ME(_CrexTestClass, variadicTest, arginfo_class__CrexTestClass_variadicTest, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(_CrexTestClass, takesUnionType, arginfo_class__CrexTestClass_takesUnionType, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class__CrexTestChildClass_methods[] = {
	CREX_ME(_CrexTestChildClass, returnsThrowable, arginfo_class__CrexTestChildClass_returnsThrowable, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_CrexAttributeTest_methods[] = {
	CREX_ME(CrexAttributeTest, testMethod, arginfo_class_CrexAttributeTest_testMethod, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class__CrexTestTrait_methods[] = {
	CREX_ME(_CrexTestTrait, testMethod, arginfo_class__CrexTestTrait_testMethod, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_CrexTestAttribute_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_CrexTestRepeatableAttribute_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_CrexTestParameterAttribute_methods[] = {
	CREX_ME(CrexTestParameterAttribute, __main, arginfo_class_CrexTestParameterAttribute___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_CrexTestPropertyAttribute_methods[] = {
	CREX_ME(CrexTestPropertyAttribute, __main, arginfo_class_CrexTestPropertyAttribute___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_CrexTestClassWithMethodWithParameterAttribute_methods[] = {
	CREX_ME(CrexTestClassWithMethodWithParameterAttribute, no_override, arginfo_class_CrexTestClassWithMethodWithParameterAttribute_no_override, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_ME(CrexTestClassWithMethodWithParameterAttribute, override, arginfo_class_CrexTestClassWithMethodWithParameterAttribute_override, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_CrexTestChildClassWithMethodWithParameterAttribute_methods[] = {
	CREX_ME(CrexTestChildClassWithMethodWithParameterAttribute, override, arginfo_class_CrexTestChildClassWithMethodWithParameterAttribute_override, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_CrexTestClassWithPropertyAttribute_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_CrexTestForbidDynamicCall_methods[] = {
	CREX_ME(CrexTestForbidDynamicCall, call, arginfo_class_CrexTestForbidDynamicCall_call, CREX_ACC_PUBLIC)
	CREX_ME(CrexTestForbidDynamicCall, callStatic, arginfo_class_CrexTestForbidDynamicCall_callStatic, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_FE_END
};


static const crex_function_entry class_CrexTestUnitEnum_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_CrexTestStringEnum_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_CrexTestIntEnum_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_CrexTestNS_Foo_methods[] = {
	CREX_ME(CrexTestNS_Foo, method, arginfo_class_CrexTestNS_Foo_method, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_CrexTestNS_UnlikelyCompileError_methods[] = {
	CREX_ME(CrexTestNS_UnlikelyCompileError, method, arginfo_class_CrexTestNS_UnlikelyCompileError_method, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_CrexTestNS_NotUnlikelyCompileError_methods[] = {
	CREX_ME(CrexTestNS_NotUnlikelyCompileError, method, arginfo_class_CrexTestNS_NotUnlikelyCompileError_method, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_CrexTestNS2_Foo_methods[] = {
	CREX_ME(CrexTestNS2_Foo, method, arginfo_class_CrexTestNS2_Foo_method, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_CrexTestNS2_CrexSubNS_Foo_methods[] = {
	CREX_ME(CrexTestNS2_CrexSubNS_Foo, method, arginfo_class_CrexTestNS2_CrexSubNS_Foo_method, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static void register_test_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("CREX_TEST_DEPRECATED", 42, CONST_PERSISTENT | CONST_DEPRECATED);
	REGISTER_STRING_CONSTANT("CREX_CONSTANT_A", "global", CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("CrexTestNS2\\CREX_CONSTANT_A", "namespaced", CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("CrexTestNS2\\CrexSubNS\\CREX_CONSTANT_A", "namespaced", CONST_PERSISTENT);


	crex_string *attribute_name_CrexTestParameterAttribute_func_crex_test_parameter_with_attribute_arg0_0 = crex_string_init_interned("CrexTestParameterAttribute", sizeof("CrexTestParameterAttribute") - 1, 1);
	crex_attribute *attribute_CrexTestParameterAttribute_func_crex_test_parameter_with_attribute_arg0_0 = crex_add_parameter_attribute(crex_hash_str_find_ptr(CG(function_table), "crex_test_parameter_with_attribute", sizeof("crex_test_parameter_with_attribute") - 1), 0, attribute_name_CrexTestParameterAttribute_func_crex_test_parameter_with_attribute_arg0_0, 1);
	crex_string_release(attribute_name_CrexTestParameterAttribute_func_crex_test_parameter_with_attribute_arg0_0);
	zval attribute_CrexTestParameterAttribute_func_crex_test_parameter_with_attribute_arg0_0_arg0;
	crex_string *attribute_CrexTestParameterAttribute_func_crex_test_parameter_with_attribute_arg0_0_arg0_str = crex_string_init("value1", strlen("value1"), 1);
	ZVAL_STR(&attribute_CrexTestParameterAttribute_func_crex_test_parameter_with_attribute_arg0_0_arg0, attribute_CrexTestParameterAttribute_func_crex_test_parameter_with_attribute_arg0_0_arg0_str);
	ZVAL_COPY_VALUE(&attribute_CrexTestParameterAttribute_func_crex_test_parameter_with_attribute_arg0_0->args[0].value, &attribute_CrexTestParameterAttribute_func_crex_test_parameter_with_attribute_arg0_0_arg0);
}

static crex_class_entry *register_class__CrexTestInterface(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "_CrexTestInterface", class__CrexTestInterface_methods);
	class_entry = crex_register_internal_interface(&ce);

	zval const_DUMMY_value;
	ZVAL_LONG(&const_DUMMY_value, 0);
	crex_string *const_DUMMY_name = crex_string_init_interned("DUMMY", sizeof("DUMMY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DUMMY_name, &const_DUMMY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DUMMY_name);

	return class_entry;
}

static crex_class_entry *register_class__CrexTestClass(crex_class_entry *class_entry__CrexTestInterface)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "_CrexTestClass", class__CrexTestClass_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 1, class_entry__CrexTestInterface);
	crex_register_class_alias("_CrexTestClassAlias", class_entry);

	zval const_TYPED_CLASS_CONST1_value;
	ZVAL_EMPTY_ARRAY(&const_TYPED_CLASS_CONST1_value);
	crex_string *const_TYPED_CLASS_CONST1_name = crex_string_init_interned("TYPED_CLASS_CONST1", sizeof("TYPED_CLASS_CONST1") - 1, 1);
#if (CRX_VERSION_ID >= 80300)
	crex_declare_typed_class_constant(class_entry, const_TYPED_CLASS_CONST1_name, &const_TYPED_CLASS_CONST1_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ANY));
#else
	crex_declare_class_constant_ex(class_entry, const_TYPED_CLASS_CONST1_name, &const_TYPED_CLASS_CONST1_value, CREX_ACC_PUBLIC, NULL);
#endif
	crex_string_release(const_TYPED_CLASS_CONST1_name);

	zval const_TYPED_CLASS_CONST2_value;
	ZVAL_LONG(&const_TYPED_CLASS_CONST2_value, 42);
	crex_string *const_TYPED_CLASS_CONST2_name = crex_string_init_interned("TYPED_CLASS_CONST2", sizeof("TYPED_CLASS_CONST2") - 1, 1);
#if (CRX_VERSION_ID >= 80300)
	crex_declare_typed_class_constant(class_entry, const_TYPED_CLASS_CONST2_name, &const_TYPED_CLASS_CONST2_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG|MAY_BE_ARRAY));
#else
	crex_declare_class_constant_ex(class_entry, const_TYPED_CLASS_CONST2_name, &const_TYPED_CLASS_CONST2_value, CREX_ACC_PUBLIC, NULL);
#endif
	crex_string_release(const_TYPED_CLASS_CONST2_name);

	zval const_TYPED_CLASS_CONST3_value;
	ZVAL_LONG(&const_TYPED_CLASS_CONST3_value, 1);
	crex_string *const_TYPED_CLASS_CONST3_name = crex_string_init_interned("TYPED_CLASS_CONST3", sizeof("TYPED_CLASS_CONST3") - 1, 1);
#if (CRX_VERSION_ID >= 80300)
	crex_declare_typed_class_constant(class_entry, const_TYPED_CLASS_CONST3_name, &const_TYPED_CLASS_CONST3_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG|MAY_BE_STRING));
#else
	crex_declare_class_constant_ex(class_entry, const_TYPED_CLASS_CONST3_name, &const_TYPED_CLASS_CONST3_value, CREX_ACC_PUBLIC, NULL);
#endif
	crex_string_release(const_TYPED_CLASS_CONST3_name);

	zval property__StaticProp_default_value;
	ZVAL_NULL(&property__StaticProp_default_value);
	crex_string *property__StaticProp_name = crex_string_init("_StaticProp", sizeof("_StaticProp") - 1, 1);
	crex_declare_typed_property(class_entry, property__StaticProp_name, &property__StaticProp_default_value, CREX_ACC_PUBLIC|CREX_ACC_STATIC, NULL, (crex_type) CREX_TYPE_INIT_NONE(0));
	crex_string_release(property__StaticProp_name);

	zval property_staticIntProp_default_value;
	ZVAL_LONG(&property_staticIntProp_default_value, 123);
	crex_string *property_staticIntProp_name = crex_string_init("staticIntProp", sizeof("staticIntProp") - 1, 1);
	crex_declare_typed_property(class_entry, property_staticIntProp_name, &property_staticIntProp_default_value, CREX_ACC_PUBLIC|CREX_ACC_STATIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_staticIntProp_name);

	zval property_intProp_default_value;
	ZVAL_LONG(&property_intProp_default_value, 123);
	crex_string *property_intProp_name = crex_string_init("intProp", sizeof("intProp") - 1, 1);
	crex_declare_typed_property(class_entry, property_intProp_name, &property_intProp_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_intProp_name);

	zval property_classProp_default_value;
	ZVAL_NULL(&property_classProp_default_value);
	crex_string *property_classProp_name = crex_string_init("classProp", sizeof("classProp") - 1, 1);
	crex_string *property_classProp_class_stdClass = crex_string_init("stdClass", sizeof("stdClass")-1, 1);
	crex_declare_typed_property(class_entry, property_classProp_name, &property_classProp_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_classProp_class_stdClass, 0, MAY_BE_NULL));
	crex_string_release(property_classProp_name);

	zval property_classUnionProp_default_value;
	ZVAL_NULL(&property_classUnionProp_default_value);
	crex_string *property_classUnionProp_name = crex_string_init("classUnionProp", sizeof("classUnionProp") - 1, 1);
	crex_string *property_classUnionProp_class_stdClass = crex_string_init("stdClass", sizeof("stdClass") - 1, 1);
	crex_string *property_classUnionProp_class_Iterator = crex_string_init("Iterator", sizeof("Iterator") - 1, 1);
	crex_type_list *property_classUnionProp_type_list = malloc(CREX_TYPE_LIST_SIZE(2));
	property_classUnionProp_type_list->num_types = 2;
	property_classUnionProp_type_list->types[0] = (crex_type) CREX_TYPE_INIT_CLASS(property_classUnionProp_class_stdClass, 0, 0);
	property_classUnionProp_type_list->types[1] = (crex_type) CREX_TYPE_INIT_CLASS(property_classUnionProp_class_Iterator, 0, 0);
	crex_type property_classUnionProp_type = CREX_TYPE_INIT_UNION(property_classUnionProp_type_list, MAY_BE_NULL);
	crex_declare_typed_property(class_entry, property_classUnionProp_name, &property_classUnionProp_default_value, CREX_ACC_PUBLIC, NULL, property_classUnionProp_type);
	crex_string_release(property_classUnionProp_name);

	zval property_classIntersectionProp_default_value;
	ZVAL_UNDEF(&property_classIntersectionProp_default_value);
	crex_string *property_classIntersectionProp_name = crex_string_init("classIntersectionProp", sizeof("classIntersectionProp") - 1, 1);
	crex_string *property_classIntersectionProp_class_Traversable = crex_string_init("Traversable", sizeof("Traversable") - 1, 1);
	crex_string *property_classIntersectionProp_class_Countable = crex_string_init("Countable", sizeof("Countable") - 1, 1);
	crex_type_list *property_classIntersectionProp_type_list = malloc(CREX_TYPE_LIST_SIZE(2));
	property_classIntersectionProp_type_list->num_types = 2;
	property_classIntersectionProp_type_list->types[0] = (crex_type) CREX_TYPE_INIT_CLASS(property_classIntersectionProp_class_Traversable, 0, 0);
	property_classIntersectionProp_type_list->types[1] = (crex_type) CREX_TYPE_INIT_CLASS(property_classIntersectionProp_class_Countable, 0, 0);
	crex_type property_classIntersectionProp_type = CREX_TYPE_INIT_INTERSECTION(property_classIntersectionProp_type_list, 0);
	crex_declare_typed_property(class_entry, property_classIntersectionProp_name, &property_classIntersectionProp_default_value, CREX_ACC_PUBLIC, NULL, property_classIntersectionProp_type);
	crex_string_release(property_classIntersectionProp_name);

	zval property_readonlyProp_default_value;
	ZVAL_UNDEF(&property_readonlyProp_default_value);
	crex_string *property_readonlyProp_name = crex_string_init("readonlyProp", sizeof("readonlyProp") - 1, 1);
#if (CRX_VERSION_ID >= 80100)
	crex_declare_typed_property(class_entry, property_readonlyProp_name, &property_readonlyProp_default_value, CREX_ACC_PUBLIC|CREX_ACC_READONLY, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
#elif (CRX_VERSION_ID >= 80000)
	crex_declare_typed_property(class_entry, property_readonlyProp_name, &property_readonlyProp_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
#endif
	crex_string_release(property_readonlyProp_name);

	return class_entry;
}

static crex_class_entry *register_class__CrexTestChildClass(crex_class_entry *class_entry__CrexTestClass)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "_CrexTestChildClass", class__CrexTestChildClass_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry__CrexTestClass);

	return class_entry;
}

static crex_class_entry *register_class_CrexAttributeTest(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CrexAttributeTest", class_CrexAttributeTest_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval const_TEST_CONST_value;
	ZVAL_LONG(&const_TEST_CONST_value, 1);
	crex_string *const_TEST_CONST_name = crex_string_init_interned("TEST_CONST", sizeof("TEST_CONST") - 1, 1);
	crex_class_constant *const_TEST_CONST = crex_declare_class_constant_ex(class_entry, const_TEST_CONST_name, &const_TEST_CONST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_TEST_CONST_name);

	zval property_testProp_default_value;
	ZVAL_NULL(&property_testProp_default_value);
	crex_string *property_testProp_name = crex_string_init("testProp", sizeof("testProp") - 1, 1);
	crex_property_info *property_testProp = crex_declare_typed_property(class_entry, property_testProp_name, &property_testProp_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_NONE(0));
	crex_string_release(property_testProp_name);


	crex_string *attribute_name_CrexTestRepeatableAttribute_const_TEST_CONST_0 = crex_string_init_interned("CrexTestRepeatableAttribute", sizeof("CrexTestRepeatableAttribute") - 1, 1);
	crex_add_class_constant_attribute(class_entry, const_TEST_CONST, attribute_name_CrexTestRepeatableAttribute_const_TEST_CONST_0, 0);
	crex_string_release(attribute_name_CrexTestRepeatableAttribute_const_TEST_CONST_0);

	crex_string *attribute_name_CrexTestRepeatableAttribute_const_TEST_CONST_1 = crex_string_init_interned("CrexTestRepeatableAttribute", sizeof("CrexTestRepeatableAttribute") - 1, 1);
	crex_add_class_constant_attribute(class_entry, const_TEST_CONST, attribute_name_CrexTestRepeatableAttribute_const_TEST_CONST_1, 0);
	crex_string_release(attribute_name_CrexTestRepeatableAttribute_const_TEST_CONST_1);


	crex_string *attribute_name_CrexTestRepeatableAttribute_property_testProp_0 = crex_string_init_interned("CrexTestRepeatableAttribute", sizeof("CrexTestRepeatableAttribute") - 1, 1);
	crex_add_property_attribute(class_entry, property_testProp, attribute_name_CrexTestRepeatableAttribute_property_testProp_0, 0);
	crex_string_release(attribute_name_CrexTestRepeatableAttribute_property_testProp_0);

	crex_string *attribute_name_CrexTestPropertyAttribute_property_testProp_1 = crex_string_init_interned("CrexTestPropertyAttribute", sizeof("CrexTestPropertyAttribute") - 1, 1);
	crex_attribute *attribute_CrexTestPropertyAttribute_property_testProp_1 = crex_add_property_attribute(class_entry, property_testProp, attribute_name_CrexTestPropertyAttribute_property_testProp_1, 1);
	crex_string_release(attribute_name_CrexTestPropertyAttribute_property_testProp_1);
	zval attribute_CrexTestPropertyAttribute_property_testProp_1_arg0;
	crex_string *attribute_CrexTestPropertyAttribute_property_testProp_1_arg0_str = crex_string_init("testProp", strlen("testProp"), 1);
	ZVAL_STR(&attribute_CrexTestPropertyAttribute_property_testProp_1_arg0, attribute_CrexTestPropertyAttribute_property_testProp_1_arg0_str);
	ZVAL_COPY_VALUE(&attribute_CrexTestPropertyAttribute_property_testProp_1->args[0].value, &attribute_CrexTestPropertyAttribute_property_testProp_1_arg0);


	crex_string *attribute_name_CrexTestAttribute_func_testmethod_0 = crex_string_init_interned("CrexTestAttribute", sizeof("CrexTestAttribute") - 1, 1);
	crex_add_function_attribute(crex_hash_str_find_ptr(&class_entry->function_table, "testmethod", sizeof("testmethod") - 1), attribute_name_CrexTestAttribute_func_testmethod_0, 0);
	crex_string_release(attribute_name_CrexTestAttribute_func_testmethod_0);

	return class_entry;
}

static crex_class_entry *register_class__CrexTestTrait(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "_CrexTestTrait", class__CrexTestTrait_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_TRAIT;

	zval property_testProp_default_value;
	ZVAL_NULL(&property_testProp_default_value);
	crex_string *property_testProp_name = crex_string_init("testProp", sizeof("testProp") - 1, 1);
	crex_declare_typed_property(class_entry, property_testProp_name, &property_testProp_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_NONE(0));
	crex_string_release(property_testProp_name);

	zval property_classUnionProp_default_value;
	ZVAL_UNDEF(&property_classUnionProp_default_value);
	crex_string *property_classUnionProp_name = crex_string_init("classUnionProp", sizeof("classUnionProp") - 1, 1);
	crex_string *property_classUnionProp_class_Traversable = crex_string_init("Traversable", sizeof("Traversable") - 1, 1);
	crex_string *property_classUnionProp_class_Countable = crex_string_init("Countable", sizeof("Countable") - 1, 1);
	crex_type_list *property_classUnionProp_type_list = malloc(CREX_TYPE_LIST_SIZE(2));
	property_classUnionProp_type_list->num_types = 2;
	property_classUnionProp_type_list->types[0] = (crex_type) CREX_TYPE_INIT_CLASS(property_classUnionProp_class_Traversable, 0, 0);
	property_classUnionProp_type_list->types[1] = (crex_type) CREX_TYPE_INIT_CLASS(property_classUnionProp_class_Countable, 0, 0);
	crex_type property_classUnionProp_type = CREX_TYPE_INIT_UNION(property_classUnionProp_type_list, 0);
	crex_declare_typed_property(class_entry, property_classUnionProp_name, &property_classUnionProp_default_value, CREX_ACC_PUBLIC, NULL, property_classUnionProp_type);
	crex_string_release(property_classUnionProp_name);

	return class_entry;
}

static crex_class_entry *register_class_CrexTestAttribute(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CrexTestAttribute", class_CrexTestAttribute_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	crex_string *attribute_name_Attribute_class_CrexTestAttribute_0 = crex_string_init_interned("Attribute", sizeof("Attribute") - 1, 1);
	crex_attribute *attribute_Attribute_class_CrexTestAttribute_0 = crex_add_class_attribute(class_entry, attribute_name_Attribute_class_CrexTestAttribute_0, 1);
	crex_string_release(attribute_name_Attribute_class_CrexTestAttribute_0);
	zval attribute_Attribute_class_CrexTestAttribute_0_arg0;
	ZVAL_LONG(&attribute_Attribute_class_CrexTestAttribute_0_arg0, CREX_ATTRIBUTE_TARGET_ALL);
	ZVAL_COPY_VALUE(&attribute_Attribute_class_CrexTestAttribute_0->args[0].value, &attribute_Attribute_class_CrexTestAttribute_0_arg0);

	return class_entry;
}

static crex_class_entry *register_class_CrexTestRepeatableAttribute(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CrexTestRepeatableAttribute", class_CrexTestRepeatableAttribute_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	crex_string *attribute_name_Attribute_class_CrexTestRepeatableAttribute_0 = crex_string_init_interned("Attribute", sizeof("Attribute") - 1, 1);
	crex_attribute *attribute_Attribute_class_CrexTestRepeatableAttribute_0 = crex_add_class_attribute(class_entry, attribute_name_Attribute_class_CrexTestRepeatableAttribute_0, 1);
	crex_string_release(attribute_name_Attribute_class_CrexTestRepeatableAttribute_0);
	zval attribute_Attribute_class_CrexTestRepeatableAttribute_0_arg0;
	ZVAL_LONG(&attribute_Attribute_class_CrexTestRepeatableAttribute_0_arg0, CREX_ATTRIBUTE_TARGET_ALL | CREX_ATTRIBUTE_IS_REPEATABLE);
	ZVAL_COPY_VALUE(&attribute_Attribute_class_CrexTestRepeatableAttribute_0->args[0].value, &attribute_Attribute_class_CrexTestRepeatableAttribute_0_arg0);

	return class_entry;
}

static crex_class_entry *register_class_CrexTestParameterAttribute(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CrexTestParameterAttribute", class_CrexTestParameterAttribute_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	zval property_parameter_default_value;
	ZVAL_UNDEF(&property_parameter_default_value);
	crex_string *property_parameter_name = crex_string_init("parameter", sizeof("parameter") - 1, 1);
	crex_declare_typed_property(class_entry, property_parameter_name, &property_parameter_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_parameter_name);

	crex_string *attribute_name_Attribute_class_CrexTestParameterAttribute_0 = crex_string_init_interned("Attribute", sizeof("Attribute") - 1, 1);
	crex_attribute *attribute_Attribute_class_CrexTestParameterAttribute_0 = crex_add_class_attribute(class_entry, attribute_name_Attribute_class_CrexTestParameterAttribute_0, 1);
	crex_string_release(attribute_name_Attribute_class_CrexTestParameterAttribute_0);
	zval attribute_Attribute_class_CrexTestParameterAttribute_0_arg0;
	ZVAL_LONG(&attribute_Attribute_class_CrexTestParameterAttribute_0_arg0, CREX_ATTRIBUTE_TARGET_PARAMETER);
	ZVAL_COPY_VALUE(&attribute_Attribute_class_CrexTestParameterAttribute_0->args[0].value, &attribute_Attribute_class_CrexTestParameterAttribute_0_arg0);

	return class_entry;
}

static crex_class_entry *register_class_CrexTestPropertyAttribute(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CrexTestPropertyAttribute", class_CrexTestPropertyAttribute_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	zval property_parameter_default_value;
	ZVAL_UNDEF(&property_parameter_default_value);
	crex_string *property_parameter_name = crex_string_init("parameter", sizeof("parameter") - 1, 1);
	crex_declare_typed_property(class_entry, property_parameter_name, &property_parameter_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_parameter_name);

	crex_string *attribute_name_Attribute_class_CrexTestPropertyAttribute_0 = crex_string_init_interned("Attribute", sizeof("Attribute") - 1, 1);
	crex_attribute *attribute_Attribute_class_CrexTestPropertyAttribute_0 = crex_add_class_attribute(class_entry, attribute_name_Attribute_class_CrexTestPropertyAttribute_0, 1);
	crex_string_release(attribute_name_Attribute_class_CrexTestPropertyAttribute_0);
	zval attribute_Attribute_class_CrexTestPropertyAttribute_0_arg0;
	ZVAL_LONG(&attribute_Attribute_class_CrexTestPropertyAttribute_0_arg0, CREX_ATTRIBUTE_TARGET_PROPERTY);
	ZVAL_COPY_VALUE(&attribute_Attribute_class_CrexTestPropertyAttribute_0->args[0].value, &attribute_Attribute_class_CrexTestPropertyAttribute_0_arg0);

	return class_entry;
}

static crex_class_entry *register_class_CrexTestClassWithMethodWithParameterAttribute(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CrexTestClassWithMethodWithParameterAttribute", class_CrexTestClassWithMethodWithParameterAttribute_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);


	crex_string *attribute_name_CrexTestParameterAttribute_func_no_override_arg0_0 = crex_string_init_interned("CrexTestParameterAttribute", sizeof("CrexTestParameterAttribute") - 1, 1);
	crex_attribute *attribute_CrexTestParameterAttribute_func_no_override_arg0_0 = crex_add_parameter_attribute(crex_hash_str_find_ptr(&class_entry->function_table, "no_override", sizeof("no_override") - 1), 0, attribute_name_CrexTestParameterAttribute_func_no_override_arg0_0, 1);
	crex_string_release(attribute_name_CrexTestParameterAttribute_func_no_override_arg0_0);
	zval attribute_CrexTestParameterAttribute_func_no_override_arg0_0_arg0;
	crex_string *attribute_CrexTestParameterAttribute_func_no_override_arg0_0_arg0_str = crex_string_init("value2", strlen("value2"), 1);
	ZVAL_STR(&attribute_CrexTestParameterAttribute_func_no_override_arg0_0_arg0, attribute_CrexTestParameterAttribute_func_no_override_arg0_0_arg0_str);
	ZVAL_COPY_VALUE(&attribute_CrexTestParameterAttribute_func_no_override_arg0_0->args[0].value, &attribute_CrexTestParameterAttribute_func_no_override_arg0_0_arg0);

	crex_string *attribute_name_CrexTestParameterAttribute_func_override_arg0_0 = crex_string_init_interned("CrexTestParameterAttribute", sizeof("CrexTestParameterAttribute") - 1, 1);
	crex_attribute *attribute_CrexTestParameterAttribute_func_override_arg0_0 = crex_add_parameter_attribute(crex_hash_str_find_ptr(&class_entry->function_table, "override", sizeof("override") - 1), 0, attribute_name_CrexTestParameterAttribute_func_override_arg0_0, 1);
	crex_string_release(attribute_name_CrexTestParameterAttribute_func_override_arg0_0);
	zval attribute_CrexTestParameterAttribute_func_override_arg0_0_arg0;
	crex_string *attribute_CrexTestParameterAttribute_func_override_arg0_0_arg0_str = crex_string_init("value3", strlen("value3"), 1);
	ZVAL_STR(&attribute_CrexTestParameterAttribute_func_override_arg0_0_arg0, attribute_CrexTestParameterAttribute_func_override_arg0_0_arg0_str);
	ZVAL_COPY_VALUE(&attribute_CrexTestParameterAttribute_func_override_arg0_0->args[0].value, &attribute_CrexTestParameterAttribute_func_override_arg0_0_arg0);

	return class_entry;
}

static crex_class_entry *register_class_CrexTestChildClassWithMethodWithParameterAttribute(crex_class_entry *class_entry_CrexTestClassWithMethodWithParameterAttribute)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CrexTestChildClassWithMethodWithParameterAttribute", class_CrexTestChildClassWithMethodWithParameterAttribute_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_CrexTestClassWithMethodWithParameterAttribute);


	crex_string *attribute_name_CrexTestParameterAttribute_func_override_arg0_0 = crex_string_init_interned("CrexTestParameterAttribute", sizeof("CrexTestParameterAttribute") - 1, 1);
	crex_attribute *attribute_CrexTestParameterAttribute_func_override_arg0_0 = crex_add_parameter_attribute(crex_hash_str_find_ptr(&class_entry->function_table, "override", sizeof("override") - 1), 0, attribute_name_CrexTestParameterAttribute_func_override_arg0_0, 1);
	crex_string_release(attribute_name_CrexTestParameterAttribute_func_override_arg0_0);
	zval attribute_CrexTestParameterAttribute_func_override_arg0_0_arg0;
	crex_string *attribute_CrexTestParameterAttribute_func_override_arg0_0_arg0_str = crex_string_init("value4", strlen("value4"), 1);
	ZVAL_STR(&attribute_CrexTestParameterAttribute_func_override_arg0_0_arg0, attribute_CrexTestParameterAttribute_func_override_arg0_0_arg0_str);
	ZVAL_COPY_VALUE(&attribute_CrexTestParameterAttribute_func_override_arg0_0->args[0].value, &attribute_CrexTestParameterAttribute_func_override_arg0_0_arg0);

	return class_entry;
}

static crex_class_entry *register_class_CrexTestClassWithPropertyAttribute(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CrexTestClassWithPropertyAttribute", class_CrexTestClassWithPropertyAttribute_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval property_attributed_default_value;
	ZVAL_UNDEF(&property_attributed_default_value);
	crex_string *property_attributed_name = crex_string_init("attributed", sizeof("attributed") - 1, 1);
	crex_property_info *property_attributed = crex_declare_typed_property(class_entry, property_attributed_name, &property_attributed_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_attributed_name);


	crex_string *attribute_name_CrexTestAttribute_property_attributed_0 = crex_string_init_interned("CrexTestAttribute", sizeof("CrexTestAttribute") - 1, 1);
	crex_add_property_attribute(class_entry, property_attributed, attribute_name_CrexTestAttribute_property_attributed_0, 0);
	crex_string_release(attribute_name_CrexTestAttribute_property_attributed_0);

	return class_entry;
}

static crex_class_entry *register_class_CrexTestForbidDynamicCall(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CrexTestForbidDynamicCall", class_CrexTestForbidDynamicCall_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	return class_entry;
}

#if (CRX_VERSION_ID >= 80100)
static crex_class_entry *register_class_CrexTestUnitEnum(void)
{
	crex_class_entry *class_entry = crex_register_internal_enum("CrexTestUnitEnum", IS_UNDEF, class_CrexTestUnitEnum_methods);

	crex_enum_add_case_cstr(class_entry, "Foo", NULL);

	crex_enum_add_case_cstr(class_entry, "Bar", NULL);

	return class_entry;
}
#endif

#if (CRX_VERSION_ID >= 80100)
static crex_class_entry *register_class_CrexTestStringEnum(void)
{
	crex_class_entry *class_entry = crex_register_internal_enum("CrexTestStringEnum", IS_STRING, class_CrexTestStringEnum_methods);

	zval enum_case_Foo_value;
	crex_string *enum_case_Foo_value_str = crex_string_init("Test1", strlen("Test1"), 1);
	ZVAL_STR(&enum_case_Foo_value, enum_case_Foo_value_str);
	crex_enum_add_case_cstr(class_entry, "Foo", &enum_case_Foo_value);

	zval enum_case_Bar_value;
	crex_string *enum_case_Bar_value_str = crex_string_init("Test2", strlen("Test2"), 1);
	ZVAL_STR(&enum_case_Bar_value, enum_case_Bar_value_str);
	crex_enum_add_case_cstr(class_entry, "Bar", &enum_case_Bar_value);

	zval enum_case_Baz_value;
	crex_string *enum_case_Baz_value_str = crex_string_init("Test2\\a", strlen("Test2\\a"), 1);
	ZVAL_STR(&enum_case_Baz_value, enum_case_Baz_value_str);
	crex_enum_add_case_cstr(class_entry, "Baz", &enum_case_Baz_value);

	zval enum_case_FortyTwo_value;
	crex_string *enum_case_FortyTwo_value_str = crex_string_init("42", strlen("42"), 1);
	ZVAL_STR(&enum_case_FortyTwo_value, enum_case_FortyTwo_value_str);
	crex_enum_add_case_cstr(class_entry, "FortyTwo", &enum_case_FortyTwo_value);

	return class_entry;
}
#endif

#if (CRX_VERSION_ID >= 80100)
static crex_class_entry *register_class_CrexTestIntEnum(void)
{
	crex_class_entry *class_entry = crex_register_internal_enum("CrexTestIntEnum", IS_LONG, class_CrexTestIntEnum_methods);

	zval enum_case_Foo_value;
	ZVAL_LONG(&enum_case_Foo_value, 1);
	crex_enum_add_case_cstr(class_entry, "Foo", &enum_case_Foo_value);

	zval enum_case_Bar_value;
	ZVAL_LONG(&enum_case_Bar_value, 3);
	crex_enum_add_case_cstr(class_entry, "Bar", &enum_case_Bar_value);

	zval enum_case_Baz_value;
	ZVAL_LONG(&enum_case_Baz_value, -1);
	crex_enum_add_case_cstr(class_entry, "Baz", &enum_case_Baz_value);

	return class_entry;
}
#endif

static crex_class_entry *register_class_CrexTestNS_Foo(void)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "CrexTestNS", "Foo", class_CrexTestNS_Foo_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	return class_entry;
}

static crex_class_entry *register_class_CrexTestNS_UnlikelyCompileError(void)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "CrexTestNS", "UnlikelyCompileError", class_CrexTestNS_UnlikelyCompileError_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	return class_entry;
}

static crex_class_entry *register_class_CrexTestNS_NotUnlikelyCompileError(void)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "CrexTestNS", "NotUnlikelyCompileError", class_CrexTestNS_NotUnlikelyCompileError_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	return class_entry;
}

static crex_class_entry *register_class_CrexTestNS2_Foo(void)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "CrexTestNS2", "Foo", class_CrexTestNS2_Foo_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	zval property_foo_default_value;
	ZVAL_UNDEF(&property_foo_default_value);
	crex_string *property_foo_name = crex_string_init("foo", sizeof("foo") - 1, 1);
	crex_string *property_foo_class_CrexTestNS2_CrexSubNS_Foo = crex_string_init("CrexTestNS2\\CrexSubNS\\Foo", sizeof("CrexTestNS2\\CrexSubNS\\Foo")-1, 1);
	crex_declare_typed_property(class_entry, property_foo_name, &property_foo_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_foo_class_CrexTestNS2_CrexSubNS_Foo, 0, 0));
	crex_string_release(property_foo_name);

	return class_entry;
}

static crex_class_entry *register_class_CrexTestNS2_CrexSubNS_Foo(void)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "CrexTestNS2\\CrexSubNS", "Foo", class_CrexTestNS2_CrexSubNS_Foo_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);

	return class_entry;
}
