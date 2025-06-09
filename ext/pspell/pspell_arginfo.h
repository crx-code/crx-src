/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 8d35f61a0b48c5422b31e78f587d9258fd3e8e37 */

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_pspell_new, 0, 1, PSpell\\Dictionary, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, language, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, spelling, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, jargon, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_pspell_new_personal, 0, 2, PSpell\\Dictionary, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, language, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, spelling, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, jargon, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_pspell_new_config, 0, 1, PSpell\\Dictionary, MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, config, PSpell\\Config, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pspell_check, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, dictionary, PSpell\\Dictionary, 0)
	CREX_ARG_TYPE_INFO(0, word, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_pspell_suggest, 0, 2, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, dictionary, PSpell\\Dictionary, 0)
	CREX_ARG_TYPE_INFO(0, word, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pspell_store_replacement, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, dictionary, PSpell\\Dictionary, 0)
	CREX_ARG_TYPE_INFO(0, misspelled, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, correct, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_pspell_add_to_personal arginfo_pspell_check

#define arginfo_pspell_add_to_session arginfo_pspell_check

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pspell_clear_session, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, dictionary, PSpell\\Dictionary, 0)
CREX_END_ARG_INFO()

#define arginfo_pspell_save_wordlist arginfo_pspell_clear_session

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_pspell_config_create, 0, 1, PSpell\\Config, 0)
	CREX_ARG_TYPE_INFO(0, language, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, spelling, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, jargon, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, encoding, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pspell_config_runtogether, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, config, PSpell\\Config, 0)
	CREX_ARG_TYPE_INFO(0, allow, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pspell_config_mode, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, config, PSpell\\Config, 0)
	CREX_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pspell_config_ignore, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, config, PSpell\\Config, 0)
	CREX_ARG_TYPE_INFO(0, min_length, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pspell_config_personal, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, config, PSpell\\Config, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pspell_config_dict_dir, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, config, PSpell\\Config, 0)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_pspell_config_data_dir arginfo_pspell_config_dict_dir

#define arginfo_pspell_config_repl arginfo_pspell_config_personal

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pspell_config_save_repl, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, config, PSpell\\Config, 0)
	CREX_ARG_TYPE_INFO(0, save, _IS_BOOL, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(pspell_new);
CREX_FUNCTION(pspell_new_personal);
CREX_FUNCTION(pspell_new_config);
CREX_FUNCTION(pspell_check);
CREX_FUNCTION(pspell_suggest);
CREX_FUNCTION(pspell_store_replacement);
CREX_FUNCTION(pspell_add_to_personal);
CREX_FUNCTION(pspell_add_to_session);
CREX_FUNCTION(pspell_clear_session);
CREX_FUNCTION(pspell_save_wordlist);
CREX_FUNCTION(pspell_config_create);
CREX_FUNCTION(pspell_config_runtogether);
CREX_FUNCTION(pspell_config_mode);
CREX_FUNCTION(pspell_config_ignore);
CREX_FUNCTION(pspell_config_personal);
CREX_FUNCTION(pspell_config_dict_dir);
CREX_FUNCTION(pspell_config_data_dir);
CREX_FUNCTION(pspell_config_repl);
CREX_FUNCTION(pspell_config_save_repl);


static const crex_function_entry ext_functions[] = {
	CREX_FE(pspell_new, arginfo_pspell_new)
	CREX_FE(pspell_new_personal, arginfo_pspell_new_personal)
	CREX_FE(pspell_new_config, arginfo_pspell_new_config)
	CREX_FE(pspell_check, arginfo_pspell_check)
	CREX_FE(pspell_suggest, arginfo_pspell_suggest)
	CREX_FE(pspell_store_replacement, arginfo_pspell_store_replacement)
	CREX_FE(pspell_add_to_personal, arginfo_pspell_add_to_personal)
	CREX_FE(pspell_add_to_session, arginfo_pspell_add_to_session)
	CREX_FE(pspell_clear_session, arginfo_pspell_clear_session)
	CREX_FE(pspell_save_wordlist, arginfo_pspell_save_wordlist)
	CREX_FE(pspell_config_create, arginfo_pspell_config_create)
	CREX_FE(pspell_config_runtogether, arginfo_pspell_config_runtogether)
	CREX_FE(pspell_config_mode, arginfo_pspell_config_mode)
	CREX_FE(pspell_config_ignore, arginfo_pspell_config_ignore)
	CREX_FE(pspell_config_personal, arginfo_pspell_config_personal)
	CREX_FE(pspell_config_dict_dir, arginfo_pspell_config_dict_dir)
	CREX_FE(pspell_config_data_dir, arginfo_pspell_config_data_dir)
	CREX_FE(pspell_config_repl, arginfo_pspell_config_repl)
	CREX_FE(pspell_config_save_repl, arginfo_pspell_config_save_repl)
	CREX_FE_END
};


static const crex_function_entry class_PSpell_Dictionary_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_PSpell_Config_methods[] = {
	CREX_FE_END
};

static void register_pspell_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("PSPELL_FAST", PSPELL_FAST, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PSPELL_NORMAL", PSPELL_NORMAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PSPELL_BAD_SPELLERS", PSPELL_BAD_SPELLERS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PSPELL_RUN_TOGETHER", PSPELL_RUN_TOGETHER, CONST_PERSISTENT);
}

static crex_class_entry *register_class_PSpell_Dictionary(void)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "PSpell", "Dictionary", class_PSpell_Dictionary_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}

static crex_class_entry *register_class_PSpell_Config(void)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "PSpell", "Config", class_PSpell_Config_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}
