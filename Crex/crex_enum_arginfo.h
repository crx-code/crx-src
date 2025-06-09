/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 7092f1d4ba651f077cff37050899f090f00abf22 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_UnitEnum_cases, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_BackedEnum_from, 0, 1, IS_STATIC, 0)
	CREX_ARG_TYPE_MASK(0, value, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_BackedEnum_tryFrom, 0, 1, IS_STATIC, 1)
	CREX_ARG_TYPE_MASK(0, value, MAY_BE_LONG|MAY_BE_STRING, NULL)
CREX_END_ARG_INFO()




static const crex_function_entry class_UnitEnum_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(UnitEnum, cases, arginfo_class_UnitEnum_cases, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_BackedEnum_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(BackedEnum, from, arginfo_class_BackedEnum_from, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(BackedEnum, tryFrom, arginfo_class_BackedEnum_tryFrom, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};

static crex_class_entry *register_class_UnitEnum(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "UnitEnum", class_UnitEnum_methods);
	class_entry = crex_register_internal_interface(&ce);

	return class_entry;
}

static crex_class_entry *register_class_BackedEnum(crex_class_entry *class_entry_UnitEnum)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "BackedEnum", class_BackedEnum_methods);
	class_entry = crex_register_internal_interface(&ce);
	crex_class_implements(class_entry, 1, class_entry_UnitEnum);

	return class_entry;
}
