/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 21ec2dcca99c85c90afcd319da76016a9f678dc2 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_implements, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_INFO(0, object_or_class)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, autoload, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

#define arginfo_class_parents arginfo_class_implements

#define arginfo_class_uses arginfo_class_implements

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_spl_autoload, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, class, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, file_extensions, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_spl_autoload_call, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, class, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_spl_autoload_extensions, 0, 0, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, file_extensions, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_spl_autoload_functions, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_spl_autoload_register, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_CALLABLE, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, throw, _IS_BOOL, 0, "true")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, prepend, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_spl_autoload_unregister, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()

#define arginfo_spl_classes arginfo_spl_autoload_functions

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_spl_object_hash, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, object, IS_OBJECT, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_spl_object_id, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, object, IS_OBJECT, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_iterator_apply, 0, 2, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, iterator, Traversable, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, args, IS_ARRAY, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_iterator_count, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, iterator, Traversable, MAY_BE_ARRAY, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_iterator_to_array, 0, 1, IS_ARRAY, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, iterator, Traversable, MAY_BE_ARRAY, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, preserve_keys, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()


CREX_FUNCTION(class_implements);
CREX_FUNCTION(class_parents);
CREX_FUNCTION(class_uses);
CREX_FUNCTION(spl_autoload);
CREX_FUNCTION(spl_autoload_call);
CREX_FUNCTION(spl_autoload_extensions);
CREX_FUNCTION(spl_autoload_functions);
CREX_FUNCTION(spl_autoload_register);
CREX_FUNCTION(spl_autoload_unregister);
CREX_FUNCTION(spl_classes);
CREX_FUNCTION(spl_object_hash);
CREX_FUNCTION(spl_object_id);
CREX_FUNCTION(iterator_apply);
CREX_FUNCTION(iterator_count);
CREX_FUNCTION(iterator_to_array);


static const crex_function_entry ext_functions[] = {
	CREX_FE(class_implements, arginfo_class_implements)
	CREX_FE(class_parents, arginfo_class_parents)
	CREX_FE(class_uses, arginfo_class_uses)
	CREX_FE(spl_autoload, arginfo_spl_autoload)
	CREX_FE(spl_autoload_call, arginfo_spl_autoload_call)
	CREX_FE(spl_autoload_extensions, arginfo_spl_autoload_extensions)
	CREX_FE(spl_autoload_functions, arginfo_spl_autoload_functions)
	CREX_FE(spl_autoload_register, arginfo_spl_autoload_register)
	CREX_FE(spl_autoload_unregister, arginfo_spl_autoload_unregister)
	CREX_FE(spl_classes, arginfo_spl_classes)
	CREX_FE(spl_object_hash, arginfo_spl_object_hash)
	CREX_FE(spl_object_id, arginfo_spl_object_id)
	CREX_FE(iterator_apply, arginfo_iterator_apply)
	CREX_FE(iterator_count, arginfo_iterator_count)
	CREX_FE(iterator_to_array, arginfo_iterator_to_array)
	CREX_FE_END
};
