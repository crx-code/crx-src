/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: d595f5c582996ebb96ab39df8cb56c4cf6c8dfcf */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_MessageFormatter___main, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_MessageFormatter_create, 0, 2, MessageFormatter, 1)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_MessageFormatter_format, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, values, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_MessageFormatter_formatMessage, 0, 3, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, values, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_MessageFormatter_parse, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_MessageFormatter_parseMessage, 0, 3, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, locale, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, message, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_MessageFormatter_setPattern, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_MessageFormatter_getPattern, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_MessageFormatter_getLocale, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_MessageFormatter_getErrorCode, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_MessageFormatter_getErrorMessage arginfo_class_MessageFormatter_getLocale


CREX_METHOD(MessageFormatter, __main);
CREX_FUNCTION(msgfmt_create);
CREX_FUNCTION(msgfmt_format);
CREX_FUNCTION(msgfmt_format_message);
CREX_FUNCTION(msgfmt_parse);
CREX_FUNCTION(msgfmt_parse_message);
CREX_FUNCTION(msgfmt_set_pattern);
CREX_FUNCTION(msgfmt_get_pattern);
CREX_FUNCTION(msgfmt_get_locale);
CREX_FUNCTION(msgfmt_get_error_code);
CREX_FUNCTION(msgfmt_get_error_message);


static const crex_function_entry class_MessageFormatter_methods[] = {
	CREX_ME(MessageFormatter, __main, arginfo_class_MessageFormatter___main, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(create, msgfmt_create, arginfo_class_MessageFormatter_create, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(format, msgfmt_format, arginfo_class_MessageFormatter_format, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(formatMessage, msgfmt_format_message, arginfo_class_MessageFormatter_formatMessage, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(parse, msgfmt_parse, arginfo_class_MessageFormatter_parse, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(parseMessage, msgfmt_parse_message, arginfo_class_MessageFormatter_parseMessage, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME_MAPPING(setPattern, msgfmt_set_pattern, arginfo_class_MessageFormatter_setPattern, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getPattern, msgfmt_get_pattern, arginfo_class_MessageFormatter_getPattern, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getLocale, msgfmt_get_locale, arginfo_class_MessageFormatter_getLocale, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getErrorCode, msgfmt_get_error_code, arginfo_class_MessageFormatter_getErrorCode, CREX_ACC_PUBLIC)
	CREX_ME_MAPPING(getErrorMessage, msgfmt_get_error_message, arginfo_class_MessageFormatter_getErrorMessage, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_MessageFormatter(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "MessageFormatter", class_MessageFormatter_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}
