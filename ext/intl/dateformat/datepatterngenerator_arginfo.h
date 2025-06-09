/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 4456b13f7ed59847bbf129cd45b0d1f63ce70108 */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_IntlDatePatternGenerator___main, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, locale, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_IntlDatePatternGenerator_create, 0, 0, IntlDatePatternGenerator, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, locale, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_IntlDatePatternGenerator_getBestPattern, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, skeleton, IS_STRING, 0)
CREX_END_ARG_INFO()


CREX_METHOD(IntlDatePatternGenerator, __main);
CREX_METHOD(IntlDatePatternGenerator, create);
CREX_METHOD(IntlDatePatternGenerator, getBestPattern);


static const crex_function_entry class_IntlDatePatternGenerator_methods[] = {
	CREX_ME(IntlDatePatternGenerator, __main, arginfo_class_IntlDatePatternGenerator___main, CREX_ACC_PUBLIC)
	CREX_ME(IntlDatePatternGenerator, create, arginfo_class_IntlDatePatternGenerator_create, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(IntlDatePatternGenerator, getBestPattern, arginfo_class_IntlDatePatternGenerator_getBestPattern, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_IntlDatePatternGenerator(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "IntlDatePatternGenerator", class_IntlDatePatternGenerator_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}
