/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 32f0458c20f04099e353a8300ffb19e40bc38f69 */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Attribute___main, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "Attribute::TARGET_ALL")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_ReturnTypeWillChange___main, 0, 0, 0)
CREX_END_ARG_INFO()

#define arginfo_class_AllowDynamicProperties___main arginfo_class_ReturnTypeWillChange___main

#define arginfo_class_SensitiveParameter___main arginfo_class_ReturnTypeWillChange___main

CREX_BEGIN_ARG_INFO_EX(arginfo_class_SensitiveParameterValue___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SensitiveParameterValue_getValue, 0, 0, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SensitiveParameterValue___debugInfo, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Override___main arginfo_class_ReturnTypeWillChange___main


CREX_METHOD(Attribute, __main);
CREX_METHOD(ReturnTypeWillChange, __main);
CREX_METHOD(AllowDynamicProperties, __main);
CREX_METHOD(SensitiveParameter, __main);
CREX_METHOD(SensitiveParameterValue, __main);
CREX_METHOD(SensitiveParameterValue, getValue);
CREX_METHOD(SensitiveParameterValue, __debugInfo);
CREX_METHOD(Override, __main);


static const crex_function_entry class_Attribute_methods[] = {
	CREX_ME(Attribute, __main, arginfo_class_Attribute___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_ReturnTypeWillChange_methods[] = {
	CREX_ME(ReturnTypeWillChange, __main, arginfo_class_ReturnTypeWillChange___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_AllowDynamicProperties_methods[] = {
	CREX_ME(AllowDynamicProperties, __main, arginfo_class_AllowDynamicProperties___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_SensitiveParameter_methods[] = {
	CREX_ME(SensitiveParameter, __main, arginfo_class_SensitiveParameter___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_SensitiveParameterValue_methods[] = {
	CREX_ME(SensitiveParameterValue, __main, arginfo_class_SensitiveParameterValue___main, CREX_ACC_PUBLIC)
	CREX_ME(SensitiveParameterValue, getValue, arginfo_class_SensitiveParameterValue_getValue, CREX_ACC_PUBLIC)
	CREX_ME(SensitiveParameterValue, __debugInfo, arginfo_class_SensitiveParameterValue___debugInfo, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_Override_methods[] = {
	CREX_ME(Override, __main, arginfo_class_Override___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_Attribute(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Attribute", class_Attribute_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	zval const_TARGET_CLASS_value;
	ZVAL_LONG(&const_TARGET_CLASS_value, CREX_ATTRIBUTE_TARGET_CLASS);
	crex_string *const_TARGET_CLASS_name = crex_string_init_interned("TARGET_CLASS", sizeof("TARGET_CLASS") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TARGET_CLASS_name, &const_TARGET_CLASS_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TARGET_CLASS_name);

	zval const_TARGET_FUNCTION_value;
	ZVAL_LONG(&const_TARGET_FUNCTION_value, CREX_ATTRIBUTE_TARGET_FUNCTION);
	crex_string *const_TARGET_FUNCTION_name = crex_string_init_interned("TARGET_FUNCTION", sizeof("TARGET_FUNCTION") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TARGET_FUNCTION_name, &const_TARGET_FUNCTION_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TARGET_FUNCTION_name);

	zval const_TARGET_METHOD_value;
	ZVAL_LONG(&const_TARGET_METHOD_value, CREX_ATTRIBUTE_TARGET_METHOD);
	crex_string *const_TARGET_METHOD_name = crex_string_init_interned("TARGET_METHOD", sizeof("TARGET_METHOD") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TARGET_METHOD_name, &const_TARGET_METHOD_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TARGET_METHOD_name);

	zval const_TARGET_PROPERTY_value;
	ZVAL_LONG(&const_TARGET_PROPERTY_value, CREX_ATTRIBUTE_TARGET_PROPERTY);
	crex_string *const_TARGET_PROPERTY_name = crex_string_init_interned("TARGET_PROPERTY", sizeof("TARGET_PROPERTY") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TARGET_PROPERTY_name, &const_TARGET_PROPERTY_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TARGET_PROPERTY_name);

	zval const_TARGET_CLASS_CONSTANT_value;
	ZVAL_LONG(&const_TARGET_CLASS_CONSTANT_value, CREX_ATTRIBUTE_TARGET_CLASS_CONST);
	crex_string *const_TARGET_CLASS_CONSTANT_name = crex_string_init_interned("TARGET_CLASS_CONSTANT", sizeof("TARGET_CLASS_CONSTANT") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TARGET_CLASS_CONSTANT_name, &const_TARGET_CLASS_CONSTANT_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TARGET_CLASS_CONSTANT_name);

	zval const_TARGET_PARAMETER_value;
	ZVAL_LONG(&const_TARGET_PARAMETER_value, CREX_ATTRIBUTE_TARGET_PARAMETER);
	crex_string *const_TARGET_PARAMETER_name = crex_string_init_interned("TARGET_PARAMETER", sizeof("TARGET_PARAMETER") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TARGET_PARAMETER_name, &const_TARGET_PARAMETER_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TARGET_PARAMETER_name);

	zval const_TARGET_ALL_value;
	ZVAL_LONG(&const_TARGET_ALL_value, CREX_ATTRIBUTE_TARGET_ALL);
	crex_string *const_TARGET_ALL_name = crex_string_init_interned("TARGET_ALL", sizeof("TARGET_ALL") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TARGET_ALL_name, &const_TARGET_ALL_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TARGET_ALL_name);

	zval const_IS_REPEATABLE_value;
	ZVAL_LONG(&const_IS_REPEATABLE_value, CREX_ATTRIBUTE_IS_REPEATABLE);
	crex_string *const_IS_REPEATABLE_name = crex_string_init_interned("IS_REPEATABLE", sizeof("IS_REPEATABLE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_IS_REPEATABLE_name, &const_IS_REPEATABLE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_IS_REPEATABLE_name);

	zval property_flags_default_value;
	ZVAL_UNDEF(&property_flags_default_value);
	crex_string *property_flags_name = crex_string_init("flags", sizeof("flags") - 1, 1);
	crex_declare_typed_property(class_entry, property_flags_name, &property_flags_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_flags_name);

	crex_string *attribute_name_Attribute_class_Attribute_0 = crex_string_init_interned("Attribute", sizeof("Attribute") - 1, 1);
	crex_attribute *attribute_Attribute_class_Attribute_0 = crex_add_class_attribute(class_entry, attribute_name_Attribute_class_Attribute_0, 1);
	crex_string_release(attribute_name_Attribute_class_Attribute_0);
	zval attribute_Attribute_class_Attribute_0_arg0;
	ZVAL_LONG(&attribute_Attribute_class_Attribute_0_arg0, CREX_ATTRIBUTE_TARGET_CLASS);
	ZVAL_COPY_VALUE(&attribute_Attribute_class_Attribute_0->args[0].value, &attribute_Attribute_class_Attribute_0_arg0);

	return class_entry;
}

static crex_class_entry *register_class_ReturnTypeWillChange(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ReturnTypeWillChange", class_ReturnTypeWillChange_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	crex_string *attribute_name_Attribute_class_ReturnTypeWillChange_0 = crex_string_init_interned("Attribute", sizeof("Attribute") - 1, 1);
	crex_attribute *attribute_Attribute_class_ReturnTypeWillChange_0 = crex_add_class_attribute(class_entry, attribute_name_Attribute_class_ReturnTypeWillChange_0, 1);
	crex_string_release(attribute_name_Attribute_class_ReturnTypeWillChange_0);
	zval attribute_Attribute_class_ReturnTypeWillChange_0_arg0;
	ZVAL_LONG(&attribute_Attribute_class_ReturnTypeWillChange_0_arg0, CREX_ATTRIBUTE_TARGET_METHOD);
	ZVAL_COPY_VALUE(&attribute_Attribute_class_ReturnTypeWillChange_0->args[0].value, &attribute_Attribute_class_ReturnTypeWillChange_0_arg0);

	return class_entry;
}

static crex_class_entry *register_class_AllowDynamicProperties(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "AllowDynamicProperties", class_AllowDynamicProperties_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	crex_string *attribute_name_Attribute_class_AllowDynamicProperties_0 = crex_string_init_interned("Attribute", sizeof("Attribute") - 1, 1);
	crex_attribute *attribute_Attribute_class_AllowDynamicProperties_0 = crex_add_class_attribute(class_entry, attribute_name_Attribute_class_AllowDynamicProperties_0, 1);
	crex_string_release(attribute_name_Attribute_class_AllowDynamicProperties_0);
	zval attribute_Attribute_class_AllowDynamicProperties_0_arg0;
	ZVAL_LONG(&attribute_Attribute_class_AllowDynamicProperties_0_arg0, CREX_ATTRIBUTE_TARGET_CLASS);
	ZVAL_COPY_VALUE(&attribute_Attribute_class_AllowDynamicProperties_0->args[0].value, &attribute_Attribute_class_AllowDynamicProperties_0_arg0);

	return class_entry;
}

static crex_class_entry *register_class_SensitiveParameter(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SensitiveParameter", class_SensitiveParameter_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES;

	crex_string *attribute_name_Attribute_class_SensitiveParameter_0 = crex_string_init_interned("Attribute", sizeof("Attribute") - 1, 1);
	crex_attribute *attribute_Attribute_class_SensitiveParameter_0 = crex_add_class_attribute(class_entry, attribute_name_Attribute_class_SensitiveParameter_0, 1);
	crex_string_release(attribute_name_Attribute_class_SensitiveParameter_0);
	zval attribute_Attribute_class_SensitiveParameter_0_arg0;
	ZVAL_LONG(&attribute_Attribute_class_SensitiveParameter_0_arg0, CREX_ATTRIBUTE_TARGET_PARAMETER);
	ZVAL_COPY_VALUE(&attribute_Attribute_class_SensitiveParameter_0->args[0].value, &attribute_Attribute_class_SensitiveParameter_0_arg0);

	return class_entry;
}

static crex_class_entry *register_class_SensitiveParameterValue(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SensitiveParameterValue", class_SensitiveParameterValue_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	zval property_value_default_value;
	ZVAL_UNDEF(&property_value_default_value);
	crex_string *property_value_name = crex_string_init("value", sizeof("value") - 1, 1);
	crex_declare_typed_property(class_entry, property_value_name, &property_value_default_value, CREX_ACC_PRIVATE|CREX_ACC_READONLY, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_ANY));
	crex_string_release(property_value_name);

	return class_entry;
}

static crex_class_entry *register_class_Override(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Override", class_Override_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES;

	crex_string *attribute_name_Attribute_class_Override_0 = crex_string_init_interned("Attribute", sizeof("Attribute") - 1, 1);
	crex_attribute *attribute_Attribute_class_Override_0 = crex_add_class_attribute(class_entry, attribute_name_Attribute_class_Override_0, 1);
	crex_string_release(attribute_name_Attribute_class_Override_0);
	zval attribute_Attribute_class_Override_0_arg0;
	ZVAL_LONG(&attribute_Attribute_class_Override_0_arg0, CREX_ATTRIBUTE_TARGET_METHOD);
	ZVAL_COPY_VALUE(&attribute_Attribute_class_Override_0->args[0].value, &attribute_Attribute_class_Override_0_arg0);

	return class_entry;
}
