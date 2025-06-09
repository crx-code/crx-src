/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 12899073f3791c5da31aa555c0e612ee1faadf55 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_version, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_func_num_args, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_func_get_arg, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, position, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_func_get_args, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_strlen, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_strcmp, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, string1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string2, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_strncmp, 0, 3, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, string1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, length, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_strcasecmp arginfo_strcmp

#define arginfo_strncasecmp arginfo_strncmp

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_error_reporting, 0, 0, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, error_level, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_define, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, constant_name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, case_insensitive, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_defined, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, constant_name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_get_class, 0, 0, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, object, IS_OBJECT, 0)
CREX_END_ARG_INFO()

#define arginfo_get_called_class arginfo_crex_version

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_get_parent_class, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_MASK(0, object_or_class, MAY_BE_OBJECT|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_is_subclass_of, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, object_or_class, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, class, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, allow_string, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_is_a, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, object_or_class, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, class, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, allow_string, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_get_class_vars, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, class, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_get_object_vars, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, object, IS_OBJECT, 0)
CREX_END_ARG_INFO()

#define arginfo_get_mangled_object_vars arginfo_get_object_vars

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_get_class_methods, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_MASK(0, object_or_class, MAY_BE_OBJECT|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_method_exists, 0, 2, _IS_BOOL, 0)
	CREX_ARG_INFO(0, object_or_class)
	CREX_ARG_TYPE_INFO(0, method, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_property_exists, 0, 2, _IS_BOOL, 0)
	CREX_ARG_INFO(0, object_or_class)
	CREX_ARG_TYPE_INFO(0, property, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_exists, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, class, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, autoload, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_interface_exists, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, interface, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, autoload, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_trait_exists, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, trait, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, autoload, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_enum_exists, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, enum, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, autoload, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_function_exists, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, function, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_alias, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, class, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, alias, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, autoload, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

#define arginfo_get_included_files arginfo_func_get_args

#define arginfo_get_required_files arginfo_func_get_args

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_trigger_error, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, message, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, error_level, IS_LONG, 0, "E_USER_NOTICE")
CREX_END_ARG_INFO()

#define arginfo_user_error arginfo_trigger_error

CREX_BEGIN_ARG_INFO_EX(arginfo_set_error_handler, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, error_levels, IS_LONG, 0, "E_ALL")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_restore_error_handler, 0, 0, IS_TRUE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_set_exception_handler, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 1)
CREX_END_ARG_INFO()

#define arginfo_restore_exception_handler arginfo_restore_error_handler

#define arginfo_get_declared_classes arginfo_func_get_args

#define arginfo_get_declared_traits arginfo_func_get_args

#define arginfo_get_declared_interfaces arginfo_func_get_args

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_get_defined_functions, 0, 0, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, exclude_disabled, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

#define arginfo_get_defined_vars arginfo_func_get_args

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_get_resource_type, 0, 1, IS_STRING, 0)
	CREX_ARG_INFO(0, resource)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_get_resource_id, 0, 1, IS_LONG, 0)
	CREX_ARG_INFO(0, resource)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_get_resources, 0, 0, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_get_loaded_extensions, 0, 0, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, crex_extensions, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_get_defined_constants, 0, 0, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, categorize, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_debug_backtrace, 0, 0, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "DEBUG_BACKTRACE_PROVIDE_OBJECT")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, limit, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_debug_print_backtrace, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, limit, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_extension_loaded, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, extension, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_get_extension_funcs, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, extension, IS_STRING, 0)
CREX_END_ARG_INFO()

#if CREX_DEBUG && defined(ZTS)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crex_thread_id, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

#define arginfo_gc_mem_caches arginfo_func_num_args

#define arginfo_gc_collect_cycles arginfo_func_num_args

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gc_enabled, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_gc_enable, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

#define arginfo_gc_disable arginfo_gc_enable

#define arginfo_gc_status arginfo_func_get_args


CREX_FUNCTION(crex_version);
CREX_FUNCTION(func_num_args);
CREX_FUNCTION(func_get_arg);
CREX_FUNCTION(func_get_args);
CREX_FUNCTION(strlen);
CREX_FUNCTION(strcmp);
CREX_FUNCTION(strncmp);
CREX_FUNCTION(strcasecmp);
CREX_FUNCTION(strncasecmp);
CREX_FUNCTION(error_reporting);
CREX_FUNCTION(define);
CREX_FUNCTION(defined);
CREX_FUNCTION(get_class);
CREX_FUNCTION(get_called_class);
CREX_FUNCTION(get_parent_class);
CREX_FUNCTION(is_subclass_of);
CREX_FUNCTION(is_a);
CREX_FUNCTION(get_class_vars);
CREX_FUNCTION(get_object_vars);
CREX_FUNCTION(get_mangled_object_vars);
CREX_FUNCTION(get_class_methods);
CREX_FUNCTION(method_exists);
CREX_FUNCTION(property_exists);
CREX_FUNCTION(class_exists);
CREX_FUNCTION(interface_exists);
CREX_FUNCTION(trait_exists);
CREX_FUNCTION(enum_exists);
CREX_FUNCTION(function_exists);
CREX_FUNCTION(class_alias);
CREX_FUNCTION(get_included_files);
CREX_FUNCTION(trigger_error);
CREX_FUNCTION(set_error_handler);
CREX_FUNCTION(restore_error_handler);
CREX_FUNCTION(set_exception_handler);
CREX_FUNCTION(restore_exception_handler);
CREX_FUNCTION(get_declared_classes);
CREX_FUNCTION(get_declared_traits);
CREX_FUNCTION(get_declared_interfaces);
CREX_FUNCTION(get_defined_functions);
CREX_FUNCTION(get_defined_vars);
CREX_FUNCTION(get_resource_type);
CREX_FUNCTION(get_resource_id);
CREX_FUNCTION(get_resources);
CREX_FUNCTION(get_loaded_extensions);
CREX_FUNCTION(get_defined_constants);
CREX_FUNCTION(debug_backtrace);
CREX_FUNCTION(debug_print_backtrace);
CREX_FUNCTION(extension_loaded);
CREX_FUNCTION(get_extension_funcs);
#if CREX_DEBUG && defined(ZTS)
CREX_FUNCTION(crex_thread_id);
#endif
CREX_FUNCTION(gc_mem_caches);
CREX_FUNCTION(gc_collect_cycles);
CREX_FUNCTION(gc_enabled);
CREX_FUNCTION(gc_enable);
CREX_FUNCTION(gc_disable);
CREX_FUNCTION(gc_status);


static const crex_function_entry ext_functions[] = {
	CREX_FE(crex_version, arginfo_crex_version)
	CREX_FE(func_num_args, arginfo_func_num_args)
	CREX_FE(func_get_arg, arginfo_func_get_arg)
	CREX_FE(func_get_args, arginfo_func_get_args)
	CREX_FE(strlen, arginfo_strlen)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strcmp, arginfo_strcmp)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strncmp, arginfo_strncmp)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strcasecmp, arginfo_strcasecmp)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(strncasecmp, arginfo_strncasecmp)
	CREX_FE(error_reporting, arginfo_error_reporting)
	CREX_FE(define, arginfo_define)
	CREX_FE(defined, arginfo_defined)
	CREX_FE(get_class, arginfo_get_class)
	CREX_FE(get_called_class, arginfo_get_called_class)
	CREX_FE(get_parent_class, arginfo_get_parent_class)
	CREX_FE(is_subclass_of, arginfo_is_subclass_of)
	CREX_FE(is_a, arginfo_is_a)
	CREX_FE(get_class_vars, arginfo_get_class_vars)
	CREX_FE(get_object_vars, arginfo_get_object_vars)
	CREX_FE(get_mangled_object_vars, arginfo_get_mangled_object_vars)
	CREX_FE(get_class_methods, arginfo_get_class_methods)
	CREX_FE(method_exists, arginfo_method_exists)
	CREX_FE(property_exists, arginfo_property_exists)
	CREX_FE(class_exists, arginfo_class_exists)
	CREX_FE(interface_exists, arginfo_interface_exists)
	CREX_FE(trait_exists, arginfo_trait_exists)
	CREX_FE(enum_exists, arginfo_enum_exists)
	CREX_FE(function_exists, arginfo_function_exists)
	CREX_FE(class_alias, arginfo_class_alias)
	CREX_FE(get_included_files, arginfo_get_included_files)
	CREX_FALIAS(get_required_files, get_included_files, arginfo_get_required_files)
	CREX_FE(trigger_error, arginfo_trigger_error)
	CREX_FALIAS(user_error, trigger_error, arginfo_user_error)
	CREX_FE(set_error_handler, arginfo_set_error_handler)
	CREX_FE(restore_error_handler, arginfo_restore_error_handler)
	CREX_FE(set_exception_handler, arginfo_set_exception_handler)
	CREX_FE(restore_exception_handler, arginfo_restore_exception_handler)
	CREX_FE(get_declared_classes, arginfo_get_declared_classes)
	CREX_FE(get_declared_traits, arginfo_get_declared_traits)
	CREX_FE(get_declared_interfaces, arginfo_get_declared_interfaces)
	CREX_FE(get_defined_functions, arginfo_get_defined_functions)
	CREX_FE(get_defined_vars, arginfo_get_defined_vars)
	CREX_FE(get_resource_type, arginfo_get_resource_type)
	CREX_FE(get_resource_id, arginfo_get_resource_id)
	CREX_FE(get_resources, arginfo_get_resources)
	CREX_FE(get_loaded_extensions, arginfo_get_loaded_extensions)
	CREX_FE(get_defined_constants, arginfo_get_defined_constants)
	CREX_FE(debug_backtrace, arginfo_debug_backtrace)
	CREX_FE(debug_print_backtrace, arginfo_debug_print_backtrace)
	CREX_FE(extension_loaded, arginfo_extension_loaded)
	CREX_FE(get_extension_funcs, arginfo_get_extension_funcs)
#if CREX_DEBUG && defined(ZTS)
	CREX_FE(crex_thread_id, arginfo_crex_thread_id)
#endif
	CREX_FE(gc_mem_caches, arginfo_gc_mem_caches)
	CREX_FE(gc_collect_cycles, arginfo_gc_collect_cycles)
	CREX_FE(gc_enabled, arginfo_gc_enabled)
	CREX_FE(gc_enable, arginfo_gc_enable)
	CREX_FE(gc_disable, arginfo_gc_disable)
	CREX_FE(gc_status, arginfo_gc_status)
	CREX_FE_END
};


static const crex_function_entry class_stdClass_methods[] = {
	CREX_FE_END
};

static crex_class_entry *register_class_stdClass(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "stdClass", class_stdClass_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_ALLOW_DYNAMIC_PROPERTIES;

	crex_string *attribute_name_AllowDynamicProperties_class_stdClass_0 = crex_string_init_interned("AllowDynamicProperties", sizeof("AllowDynamicProperties") - 1, 1);
	crex_add_class_attribute(class_entry, attribute_name_AllowDynamicProperties_class_stdClass_0, 0);
	crex_string_release(attribute_name_AllowDynamicProperties_class_stdClass_0);

	return class_entry;
}
