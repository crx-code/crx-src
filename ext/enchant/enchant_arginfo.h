/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 42bb5a4488d254e87d763c75ccff62e283e63335 */

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_enchant_broker_init, 0, 0, EnchantBroker, MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_enchant_broker_free, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, broker, EnchantBroker, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_enchant_broker_get_error, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, broker, EnchantBroker, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_enchant_broker_set_dict_path, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, broker, EnchantBroker, 0)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, path, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_enchant_broker_get_dict_path, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, broker, EnchantBroker, 0)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_enchant_broker_list_dicts, 0, 1, IS_ARRAY, 0)
	CREX_ARG_OBJ_INFO(0, broker, EnchantBroker, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_enchant_broker_request_dict, 0, 2, EnchantDictionary, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, broker, EnchantBroker, 0)
	CREX_ARG_TYPE_INFO(0, tag, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_enchant_broker_request_pwl_dict, 0, 2, EnchantDictionary, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, broker, EnchantBroker, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_enchant_broker_free_dict, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, dictionary, EnchantDictionary, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_enchant_broker_dict_exists, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, broker, EnchantBroker, 0)
	CREX_ARG_TYPE_INFO(0, tag, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_enchant_broker_set_ordering, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, broker, EnchantBroker, 0)
	CREX_ARG_TYPE_INFO(0, tag, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, ordering, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_enchant_broker_describe arginfo_enchant_broker_list_dicts

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_enchant_dict_quick_check, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, dictionary, EnchantDictionary, 0)
	CREX_ARG_TYPE_INFO(0, word, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, suggestions, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_enchant_dict_check, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, dictionary, EnchantDictionary, 0)
	CREX_ARG_TYPE_INFO(0, word, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_enchant_dict_suggest, 0, 2, IS_ARRAY, 0)
	CREX_ARG_OBJ_INFO(0, dictionary, EnchantDictionary, 0)
	CREX_ARG_TYPE_INFO(0, word, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_enchant_dict_add, 0, 2, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, dictionary, EnchantDictionary, 0)
	CREX_ARG_TYPE_INFO(0, word, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_enchant_dict_add_to_personal arginfo_enchant_dict_add

#define arginfo_enchant_dict_add_to_session arginfo_enchant_dict_add

#define arginfo_enchant_dict_is_added arginfo_enchant_dict_check

#define arginfo_enchant_dict_is_in_session arginfo_enchant_dict_check

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_enchant_dict_store_replacement, 0, 3, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, dictionary, EnchantDictionary, 0)
	CREX_ARG_TYPE_INFO(0, misspelled, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, correct, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_enchant_dict_get_error, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, dictionary, EnchantDictionary, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_enchant_dict_describe, 0, 1, IS_ARRAY, 0)
	CREX_ARG_OBJ_INFO(0, dictionary, EnchantDictionary, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(enchant_broker_init);
CREX_FUNCTION(enchant_broker_free);
CREX_FUNCTION(enchant_broker_get_error);
CREX_FUNCTION(enchant_broker_set_dict_path);
CREX_FUNCTION(enchant_broker_get_dict_path);
CREX_FUNCTION(enchant_broker_list_dicts);
CREX_FUNCTION(enchant_broker_request_dict);
CREX_FUNCTION(enchant_broker_request_pwl_dict);
CREX_FUNCTION(enchant_broker_free_dict);
CREX_FUNCTION(enchant_broker_dict_exists);
CREX_FUNCTION(enchant_broker_set_ordering);
CREX_FUNCTION(enchant_broker_describe);
CREX_FUNCTION(enchant_dict_quick_check);
CREX_FUNCTION(enchant_dict_check);
CREX_FUNCTION(enchant_dict_suggest);
CREX_FUNCTION(enchant_dict_add);
CREX_FUNCTION(enchant_dict_add_to_session);
CREX_FUNCTION(enchant_dict_is_added);
CREX_FUNCTION(enchant_dict_store_replacement);
CREX_FUNCTION(enchant_dict_get_error);
CREX_FUNCTION(enchant_dict_describe);


static const crex_function_entry ext_functions[] = {
	CREX_FE(enchant_broker_init, arginfo_enchant_broker_init)
	CREX_DEP_FE(enchant_broker_free, arginfo_enchant_broker_free)
	CREX_FE(enchant_broker_get_error, arginfo_enchant_broker_get_error)
	CREX_DEP_FE(enchant_broker_set_dict_path, arginfo_enchant_broker_set_dict_path)
	CREX_DEP_FE(enchant_broker_get_dict_path, arginfo_enchant_broker_get_dict_path)
	CREX_FE(enchant_broker_list_dicts, arginfo_enchant_broker_list_dicts)
	CREX_FE(enchant_broker_request_dict, arginfo_enchant_broker_request_dict)
	CREX_FE(enchant_broker_request_pwl_dict, arginfo_enchant_broker_request_pwl_dict)
	CREX_DEP_FE(enchant_broker_free_dict, arginfo_enchant_broker_free_dict)
	CREX_FE(enchant_broker_dict_exists, arginfo_enchant_broker_dict_exists)
	CREX_FE(enchant_broker_set_ordering, arginfo_enchant_broker_set_ordering)
	CREX_FE(enchant_broker_describe, arginfo_enchant_broker_describe)
	CREX_FE(enchant_dict_quick_check, arginfo_enchant_dict_quick_check)
	CREX_FE(enchant_dict_check, arginfo_enchant_dict_check)
	CREX_FE(enchant_dict_suggest, arginfo_enchant_dict_suggest)
	CREX_FE(enchant_dict_add, arginfo_enchant_dict_add)
	CREX_DEP_FALIAS(enchant_dict_add_to_personal, enchant_dict_add, arginfo_enchant_dict_add_to_personal)
	CREX_FE(enchant_dict_add_to_session, arginfo_enchant_dict_add_to_session)
	CREX_FE(enchant_dict_is_added, arginfo_enchant_dict_is_added)
	CREX_DEP_FALIAS(enchant_dict_is_in_session, enchant_dict_is_added, arginfo_enchant_dict_is_in_session)
	CREX_FE(enchant_dict_store_replacement, arginfo_enchant_dict_store_replacement)
	CREX_FE(enchant_dict_get_error, arginfo_enchant_dict_get_error)
	CREX_FE(enchant_dict_describe, arginfo_enchant_dict_describe)
	CREX_FE_END
};


static const crex_function_entry class_EnchantBroker_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_EnchantDictionary_methods[] = {
	CREX_FE_END
};

static void register_enchant_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("ENCHANT_MYSPELL", CRX_ENCHANT_MYSPELL, CONST_PERSISTENT | CONST_DEPRECATED);
	REGISTER_LONG_CONSTANT("ENCHANT_ISPELL", CRX_ENCHANT_ISPELL, CONST_PERSISTENT | CONST_DEPRECATED);
#if defined(HAVE_ENCHANT_GET_VERSION)
	REGISTER_STRING_CONSTANT("LIBENCHANT_VERSION", CRX_ENCHANT_GET_VERSION, CONST_PERSISTENT);
#endif
}

static crex_class_entry *register_class_EnchantBroker(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "EnchantBroker", class_EnchantBroker_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}

static crex_class_entry *register_class_EnchantDictionary(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "EnchantDictionary", class_EnchantDictionary_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}
