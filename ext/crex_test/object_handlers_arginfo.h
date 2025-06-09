/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 164cdd464289c8db351f4ec49979a66d44ba3e87 */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DoOperationNoCast___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, val, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_LongCastableNoOperations___main arginfo_class_DoOperationNoCast___main

CREX_BEGIN_ARG_INFO_EX(arginfo_class_FloatCastableNoOperations___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, val, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_NumericCastableNoOperations___main, 0, 0, 1)
	CREX_ARG_TYPE_MASK(0, val, MAY_BE_LONG|MAY_BE_DOUBLE, NULL)
CREX_END_ARG_INFO()


static CREX_METHOD(DoOperationNoCast, __main);
static CREX_METHOD(LongCastableNoOperations, __main);
static CREX_METHOD(FloatCastableNoOperations, __main);
static CREX_METHOD(NumericCastableNoOperations, __main);


static const crex_function_entry class_DoOperationNoCast_methods[] = {
	CREX_ME(DoOperationNoCast, __main, arginfo_class_DoOperationNoCast___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_LongCastableNoOperations_methods[] = {
	CREX_ME(LongCastableNoOperations, __main, arginfo_class_LongCastableNoOperations___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_FloatCastableNoOperations_methods[] = {
	CREX_ME(FloatCastableNoOperations, __main, arginfo_class_FloatCastableNoOperations___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_NumericCastableNoOperations_methods[] = {
	CREX_ME(NumericCastableNoOperations, __main, arginfo_class_NumericCastableNoOperations___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_DoOperationNoCast(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DoOperationNoCast", class_DoOperationNoCast_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	zval property_val_default_value;
	ZVAL_UNDEF(&property_val_default_value);
	crex_string *property_val_name = crex_string_init("val", sizeof("val") - 1, 1);
	crex_declare_typed_property(class_entry, property_val_name, &property_val_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_val_name);

	return class_entry;
}

static crex_class_entry *register_class_LongCastableNoOperations(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "LongCastableNoOperations", class_LongCastableNoOperations_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	zval property_val_default_value;
	ZVAL_UNDEF(&property_val_default_value);
	crex_string *property_val_name = crex_string_init("val", sizeof("val") - 1, 1);
	crex_declare_typed_property(class_entry, property_val_name, &property_val_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_val_name);

	return class_entry;
}

static crex_class_entry *register_class_FloatCastableNoOperations(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "FloatCastableNoOperations", class_FloatCastableNoOperations_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	zval property_val_default_value;
	ZVAL_UNDEF(&property_val_default_value);
	crex_string *property_val_name = crex_string_init("val", sizeof("val") - 1, 1);
	crex_declare_typed_property(class_entry, property_val_name, &property_val_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_DOUBLE));
	crex_string_release(property_val_name);

	return class_entry;
}

static crex_class_entry *register_class_NumericCastableNoOperations(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "NumericCastableNoOperations", class_NumericCastableNoOperations_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	zval property_val_default_value;
	ZVAL_UNDEF(&property_val_default_value);
	crex_string *property_val_name = crex_string_init("val", sizeof("val") - 1, 1);
	crex_declare_typed_property(class_entry, property_val_name, &property_val_default_value, CREX_ACC_PRIVATE, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG|MAY_BE_DOUBLE));
	crex_string_release(property_val_name);

	return class_entry;
}
