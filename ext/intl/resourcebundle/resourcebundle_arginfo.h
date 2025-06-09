/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 7816536650d8513ef6998233096b0bf6a29d7af4 */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_ResourceBundle___main, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, bundle, IS_STRING, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, fallback, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_ResourceBundle_create, 0, 2, ResourceBundle, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 1)
	CREX_ARG_TYPE_INFO(0, bundle, IS_STRING, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, fallback, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ResourceBundle_get, 0, 1, IS_MIXED, 0)
	CREX_ARG_INFO(0, index)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, fallback, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ResourceBundle_count, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_ResourceBundle_getLocales, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, bundle, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_ResourceBundle_getErrorCode arginfo_class_ResourceBundle_count

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ResourceBundle_getErrorMessage, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_ResourceBundle_getIterator, 0, 0, Iterator, 0)
CREX_END_ARG_INFO()


CREX_METHOD(ResourceBundle, __main);
CREX_FUNCTION(resourcebundle_create);
CREX_FUNCTION(resourcebundle_get);
CREX_FUNCTION(resourcebundle_count);
CREX_FUNCTION(resourcebundle_locales);
CREX_FUNCTION(resourcebundle_get_error_code);
CREX_FUNCTION(resourcebundle_get_error_message);
CREX_METHOD(ResourceBundle, getIterator);


static const crex_function_entry class_ResourceBundle_methods[] = {
	CREX_ME(ResourceBundle, __main, arginfo_class_ResourceBundle___main, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(create, resourcebundle_create, arginfo_class_ResourceBundle_create, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(get, resourcebundle_get, arginfo_class_ResourceBundle_get, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(count, resourcebundle_count, arginfo_class_ResourceBundle_count, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getLocales, resourcebundle_locales, arginfo_class_ResourceBundle_getLocales, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(getErrorCode, resourcebundle_get_error_code, arginfo_class_ResourceBundle_getErrorCode, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getErrorMessage, resourcebundle_get_error_message, arginfo_class_ResourceBundle_getErrorMessage, CREX_ACC_PUBLIC)
	CREX_ME(ResourceBundle, getIterator, arginfo_class_ResourceBundle_getIterator, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_ResourceBundle(crex_class_entry *class_entry_IteratorAggregate, crex_class_entry *class_entry_Countable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ResourceBundle", class_ResourceBundle_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;
	crex_class_implements(class_entry, 2, class_entry_IteratorAggregate, class_entry_Countable);

	return class_entry;
}
