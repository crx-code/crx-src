/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: a89f03303f8a7d254509ae2bc46a36bb79a3c900 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_token_get_all, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, code, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_token_name, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, id, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_CrxToken_tokenize arginfo_token_get_all

CREX_BEGIN_ARG_INFO_EX(arginfo_class_CrxToken___main, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, id, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, text, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, line, IS_LONG, 0, "-1")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pos, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_CrxToken_is, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, kind)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_CrxToken_isIgnorable, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_CrxToken_getTokenName, 0, 0, IS_STRING, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_CrxToken___toString, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(token_get_all);
CREX_FUNCTION(token_name);
CREX_METHOD(CrxToken, tokenize);
CREX_METHOD(CrxToken, __main);
CREX_METHOD(CrxToken, is);
CREX_METHOD(CrxToken, isIgnorable);
CREX_METHOD(CrxToken, getTokenName);
CREX_METHOD(CrxToken, __toString);


static const crex_function_entry ext_functions[] = {
	CREX_FE(token_get_all, arginfo_token_get_all)
	CREX_FE(token_name, arginfo_token_name)
	CREX_FE_END
};


static const crex_function_entry class_CrxToken_methods[] = {
	CREX_ME(CrxToken, tokenize, arginfo_class_CrxToken_tokenize, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(CrxToken, __main, arginfo_class_CrxToken___main, CREX_ACC_PUBLIC|CREX_ACC_FINAL)
	CREX_ME(CrxToken, is, arginfo_class_CrxToken_is, CREX_ACC_PUBLIC)
	CREX_ME(CrxToken, isIgnorable, arginfo_class_CrxToken_isIgnorable, CREX_ACC_PUBLIC)
	CREX_ME(CrxToken, getTokenName, arginfo_class_CrxToken_getTokenName, CREX_ACC_PUBLIC)
	CREX_ME(CrxToken, __toString, arginfo_class_CrxToken___toString, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static void register_tokenizer_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("TOKEN_PARSE", TOKEN_PARSE, CONST_PERSISTENT);
}

static crex_class_entry *register_class_CrxToken(crex_class_entry *class_entry_Stringable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CrxToken", class_CrxToken_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 1, class_entry_Stringable);

	zval property_id_default_value;
	ZVAL_UNDEF(&property_id_default_value);
	crex_string *property_id_name = crex_string_init("id", sizeof("id") - 1, 1);
	crex_declare_typed_property(class_entry, property_id_name, &property_id_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_id_name);

	zval property_text_default_value;
	ZVAL_UNDEF(&property_text_default_value);
	crex_string *property_text_name = crex_string_init("text", sizeof("text") - 1, 1);
	crex_declare_typed_property(class_entry, property_text_name, &property_text_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_text_name);

	zval property_line_default_value;
	ZVAL_UNDEF(&property_line_default_value);
	crex_string *property_line_name = crex_string_init("line", sizeof("line") - 1, 1);
	crex_declare_typed_property(class_entry, property_line_name, &property_line_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_line_name);

	zval property_pos_default_value;
	ZVAL_UNDEF(&property_pos_default_value);
	crex_string *property_pos_name = crex_string_init("pos", sizeof("pos") - 1, 1);
	crex_declare_typed_property(class_entry, property_pos_name, &property_pos_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_pos_name);

	return class_entry;
}
