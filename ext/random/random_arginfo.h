/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 35cb16abb3392bd257a43cc675cad4f5af5549c1 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_lcg_value, 0, 0, IS_DOUBLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mt_srand, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, seed, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "MT_RAND_MT19937")
CREX_END_ARG_INFO()

#define arginfo_srand arginfo_mt_srand

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_rand, 0, 0, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, min, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, max, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_mt_rand arginfo_rand

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mt_getrandmax, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_getrandmax arginfo_mt_getrandmax

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_random_bytes, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, length, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_random_int, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, min, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, max, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Random_Engine_Mt19937___main, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, seed, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "MT_RAND_MT19937")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Random_Engine_Mt19937_generate, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Random_Engine_Mt19937___serialize, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Random_Engine_Mt19937___unserialize, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Random_Engine_Mt19937___debugInfo arginfo_class_Random_Engine_Mt19937___serialize

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Random_Engine_PcgOneseq128XslRr64___main, 0, 0, 0)
	CREX_ARG_TYPE_MASK(0, seed, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_NULL, "null")
CREX_END_ARG_INFO()

#define arginfo_class_Random_Engine_PcgOneseq128XslRr64_generate arginfo_class_Random_Engine_Mt19937_generate

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Random_Engine_PcgOneseq128XslRr64_jump, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, advance, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Random_Engine_PcgOneseq128XslRr64___serialize arginfo_class_Random_Engine_Mt19937___serialize

#define arginfo_class_Random_Engine_PcgOneseq128XslRr64___unserialize arginfo_class_Random_Engine_Mt19937___unserialize

#define arginfo_class_Random_Engine_PcgOneseq128XslRr64___debugInfo arginfo_class_Random_Engine_Mt19937___serialize

#define arginfo_class_Random_Engine_Xoshiro256StarStar___main arginfo_class_Random_Engine_PcgOneseq128XslRr64___main

#define arginfo_class_Random_Engine_Xoshiro256StarStar_generate arginfo_class_Random_Engine_Mt19937_generate

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Random_Engine_Xoshiro256StarStar_jump, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Random_Engine_Xoshiro256StarStar_jumpLong arginfo_class_Random_Engine_Xoshiro256StarStar_jump

#define arginfo_class_Random_Engine_Xoshiro256StarStar___serialize arginfo_class_Random_Engine_Mt19937___serialize

#define arginfo_class_Random_Engine_Xoshiro256StarStar___unserialize arginfo_class_Random_Engine_Mt19937___unserialize

#define arginfo_class_Random_Engine_Xoshiro256StarStar___debugInfo arginfo_class_Random_Engine_Mt19937___serialize

#define arginfo_class_Random_Engine_Secure_generate arginfo_class_Random_Engine_Mt19937_generate

#define arginfo_class_Random_Engine_generate arginfo_class_Random_Engine_Mt19937_generate

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Random_Randomizer___main, 0, 0, 0)
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, engine, Random\\Engine, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_Random_Randomizer_nextInt arginfo_mt_getrandmax

#define arginfo_class_Random_Randomizer_nextFloat arginfo_lcg_value

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Random_Randomizer_getFloat, 0, 2, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, min, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, max, IS_DOUBLE, 0)
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, boundary, Random\\IntervalBoundary, 0, "Random\\IntervalBoundary::ClosedOpen")
CREX_END_ARG_INFO()

#define arginfo_class_Random_Randomizer_getInt arginfo_random_int

#define arginfo_class_Random_Randomizer_getBytes arginfo_random_bytes

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Random_Randomizer_getBytesFromString, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, length, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Random_Randomizer_shuffleArray, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Random_Randomizer_shuffleBytes, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, bytes, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Random_Randomizer_pickArrayKeys, 0, 2, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, num, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Random_Randomizer___serialize arginfo_class_Random_Engine_Mt19937___serialize

#define arginfo_class_Random_Randomizer___unserialize arginfo_class_Random_Engine_Mt19937___unserialize


CREX_FUNCTION(lcg_value);
CREX_FUNCTION(mt_srand);
CREX_FUNCTION(rand);
CREX_FUNCTION(mt_rand);
CREX_FUNCTION(mt_getrandmax);
CREX_FUNCTION(random_bytes);
CREX_FUNCTION(random_int);
CREX_METHOD(Random_Engine_Mt19937, __main);
CREX_METHOD(Random_Engine_Mt19937, generate);
CREX_METHOD(Random_Engine_Mt19937, __serialize);
CREX_METHOD(Random_Engine_Mt19937, __unserialize);
CREX_METHOD(Random_Engine_Mt19937, __debugInfo);
CREX_METHOD(Random_Engine_PcgOneseq128XslRr64, __main);
CREX_METHOD(Random_Engine_PcgOneseq128XslRr64, jump);
CREX_METHOD(Random_Engine_Xoshiro256StarStar, __main);
CREX_METHOD(Random_Engine_Xoshiro256StarStar, jump);
CREX_METHOD(Random_Engine_Xoshiro256StarStar, jumpLong);
CREX_METHOD(Random_Randomizer, __main);
CREX_METHOD(Random_Randomizer, nextInt);
CREX_METHOD(Random_Randomizer, nextFloat);
CREX_METHOD(Random_Randomizer, getFloat);
CREX_METHOD(Random_Randomizer, getInt);
CREX_METHOD(Random_Randomizer, getBytes);
CREX_METHOD(Random_Randomizer, getBytesFromString);
CREX_METHOD(Random_Randomizer, shuffleArray);
CREX_METHOD(Random_Randomizer, shuffleBytes);
CREX_METHOD(Random_Randomizer, pickArrayKeys);
CREX_METHOD(Random_Randomizer, __serialize);
CREX_METHOD(Random_Randomizer, __unserialize);


static const crex_function_entry ext_functions[] = {
	CREX_FE(lcg_value, arginfo_lcg_value)
	CREX_FE(mt_srand, arginfo_mt_srand)
	CREX_FALIAS(srand, mt_srand, arginfo_srand)
	CREX_FE(rand, arginfo_rand)
	CREX_FE(mt_rand, arginfo_mt_rand)
	CREX_FE(mt_getrandmax, arginfo_mt_getrandmax)
	CREX_FALIAS(getrandmax, mt_getrandmax, arginfo_getrandmax)
	CREX_FE(random_bytes, arginfo_random_bytes)
	CREX_FE(random_int, arginfo_random_int)
	CREX_FE_END
};


static const crex_function_entry class_Random_Engine_Mt19937_methods[] = {
	CREX_ME(Random_Engine_Mt19937, __main, arginfo_class_Random_Engine_Mt19937___main, CREX_ACC_PUBLIC)
	CREX_ME(Random_Engine_Mt19937, generate, arginfo_class_Random_Engine_Mt19937_generate, CREX_ACC_PUBLIC)
	CREX_ME(Random_Engine_Mt19937, __serialize, arginfo_class_Random_Engine_Mt19937___serialize, CREX_ACC_PUBLIC)
	CREX_ME(Random_Engine_Mt19937, __unserialize, arginfo_class_Random_Engine_Mt19937___unserialize, CREX_ACC_PUBLIC)
	CREX_ME(Random_Engine_Mt19937, __debugInfo, arginfo_class_Random_Engine_Mt19937___debugInfo, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_Random_Engine_PcgOneseq128XslRr64_methods[] = {
	CREX_ME(Random_Engine_PcgOneseq128XslRr64, __main, arginfo_class_Random_Engine_PcgOneseq128XslRr64___main, CREX_ACC_PUBLIC)
	CREX_MALIAS(Random_Engine_Mt19937, generate, generate, arginfo_class_Random_Engine_PcgOneseq128XslRr64_generate, CREX_ACC_PUBLIC)
	CREX_ME(Random_Engine_PcgOneseq128XslRr64, jump, arginfo_class_Random_Engine_PcgOneseq128XslRr64_jump, CREX_ACC_PUBLIC)
	CREX_MALIAS(Random_Engine_Mt19937, __serialize, __serialize, arginfo_class_Random_Engine_PcgOneseq128XslRr64___serialize, CREX_ACC_PUBLIC)
	CREX_MALIAS(Random_Engine_Mt19937, __unserialize, __unserialize, arginfo_class_Random_Engine_PcgOneseq128XslRr64___unserialize, CREX_ACC_PUBLIC)
	CREX_MALIAS(Random_Engine_Mt19937, __debugInfo, __debugInfo, arginfo_class_Random_Engine_PcgOneseq128XslRr64___debugInfo, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_Random_Engine_Xoshiro256StarStar_methods[] = {
	CREX_ME(Random_Engine_Xoshiro256StarStar, __main, arginfo_class_Random_Engine_Xoshiro256StarStar___main, CREX_ACC_PUBLIC)
	CREX_MALIAS(Random_Engine_Mt19937, generate, generate, arginfo_class_Random_Engine_Xoshiro256StarStar_generate, CREX_ACC_PUBLIC)
	CREX_ME(Random_Engine_Xoshiro256StarStar, jump, arginfo_class_Random_Engine_Xoshiro256StarStar_jump, CREX_ACC_PUBLIC)
	CREX_ME(Random_Engine_Xoshiro256StarStar, jumpLong, arginfo_class_Random_Engine_Xoshiro256StarStar_jumpLong, CREX_ACC_PUBLIC)
	CREX_MALIAS(Random_Engine_Mt19937, __serialize, __serialize, arginfo_class_Random_Engine_Xoshiro256StarStar___serialize, CREX_ACC_PUBLIC)
	CREX_MALIAS(Random_Engine_Mt19937, __unserialize, __unserialize, arginfo_class_Random_Engine_Xoshiro256StarStar___unserialize, CREX_ACC_PUBLIC)
	CREX_MALIAS(Random_Engine_Mt19937, __debugInfo, __debugInfo, arginfo_class_Random_Engine_Xoshiro256StarStar___debugInfo, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_Random_Engine_Secure_methods[] = {
	CREX_MALIAS(Random_Engine_Mt19937, generate, generate, arginfo_class_Random_Engine_Secure_generate, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_Random_Engine_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(Random_Engine, generate, arginfo_class_Random_Engine_generate, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_Random_CryptoSafeEngine_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_Random_Randomizer_methods[] = {
	CREX_ME(Random_Randomizer, __main, arginfo_class_Random_Randomizer___main, CREX_ACC_PUBLIC)
	CREX_ME(Random_Randomizer, nextInt, arginfo_class_Random_Randomizer_nextInt, CREX_ACC_PUBLIC)
	CREX_ME(Random_Randomizer, nextFloat, arginfo_class_Random_Randomizer_nextFloat, CREX_ACC_PUBLIC)
	CREX_ME(Random_Randomizer, getFloat, arginfo_class_Random_Randomizer_getFloat, CREX_ACC_PUBLIC)
	CREX_ME(Random_Randomizer, getInt, arginfo_class_Random_Randomizer_getInt, CREX_ACC_PUBLIC)
	CREX_ME(Random_Randomizer, getBytes, arginfo_class_Random_Randomizer_getBytes, CREX_ACC_PUBLIC)
	CREX_ME(Random_Randomizer, getBytesFromString, arginfo_class_Random_Randomizer_getBytesFromString, CREX_ACC_PUBLIC)
	CREX_ME(Random_Randomizer, shuffleArray, arginfo_class_Random_Randomizer_shuffleArray, CREX_ACC_PUBLIC)
	CREX_ME(Random_Randomizer, shuffleBytes, arginfo_class_Random_Randomizer_shuffleBytes, CREX_ACC_PUBLIC)
	CREX_ME(Random_Randomizer, pickArrayKeys, arginfo_class_Random_Randomizer_pickArrayKeys, CREX_ACC_PUBLIC)
	CREX_ME(Random_Randomizer, __serialize, arginfo_class_Random_Randomizer___serialize, CREX_ACC_PUBLIC)
	CREX_ME(Random_Randomizer, __unserialize, arginfo_class_Random_Randomizer___unserialize, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_Random_IntervalBoundary_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_Random_RandomError_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_Random_BrokenRandomEngineError_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_Random_RandomException_methods[] = {
	CREX_FE_END
};

static void register_random_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("MT_RAND_MT19937", MT_RAND_MT19937, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MT_RAND_CRX", MT_RAND_CRX, CONST_PERSISTENT | CONST_DEPRECATED);
}

static crex_class_entry *register_class_Random_Engine_Mt19937(crex_class_entry *class_entry_Random_Engine)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Random\\Engine", "Mt19937", class_Random_Engine_Mt19937_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES;
	crex_class_implements(class_entry, 1, class_entry_Random_Engine);

	return class_entry;
}

static crex_class_entry *register_class_Random_Engine_PcgOneseq128XslRr64(crex_class_entry *class_entry_Random_Engine)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Random\\Engine", "PcgOneseq128XslRr64", class_Random_Engine_PcgOneseq128XslRr64_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES;
	crex_class_implements(class_entry, 1, class_entry_Random_Engine);

	return class_entry;
}

static crex_class_entry *register_class_Random_Engine_Xoshiro256StarStar(crex_class_entry *class_entry_Random_Engine)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Random\\Engine", "Xoshiro256StarStar", class_Random_Engine_Xoshiro256StarStar_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES;
	crex_class_implements(class_entry, 1, class_entry_Random_Engine);

	return class_entry;
}

static crex_class_entry *register_class_Random_Engine_Secure(crex_class_entry *class_entry_Random_CryptoSafeEngine)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Random\\Engine", "Secure", class_Random_Engine_Secure_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;
	crex_class_implements(class_entry, 1, class_entry_Random_CryptoSafeEngine);

	return class_entry;
}

static crex_class_entry *register_class_Random_Engine(void)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Random", "Engine", class_Random_Engine_methods);
	class_entry = crex_register_internal_interface(&ce);

	return class_entry;
}

static crex_class_entry *register_class_Random_CryptoSafeEngine(crex_class_entry *class_entry_Random_Engine)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Random", "CryptoSafeEngine", class_Random_CryptoSafeEngine_methods);
	class_entry = crex_register_internal_interface(&ce);
	crex_class_implements(class_entry, 1, class_entry_Random_Engine);

	return class_entry;
}

static crex_class_entry *register_class_Random_Randomizer(void)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Random", "Randomizer", class_Random_Randomizer_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES;

	zval property_engine_default_value;
	ZVAL_UNDEF(&property_engine_default_value);
	crex_string *property_engine_name = crex_string_init("engine", sizeof("engine") - 1, 1);
	crex_string *property_engine_class_Random_Engine = crex_string_init("Random\\Engine", sizeof("Random\\Engine")-1, 1);
	crex_declare_typed_property(class_entry, property_engine_name, &property_engine_default_value, CREX_ACC_PUBLIC|CREX_ACC_READONLY, NULL, (crex_type) CREX_TYPE_INIT_CLASS(property_engine_class_Random_Engine, 0, 0));
	crex_string_release(property_engine_name);

	return class_entry;
}

static crex_class_entry *register_class_Random_IntervalBoundary(void)
{
	crex_class_entry *class_entry = crex_register_internal_enum("Random\\IntervalBoundary", IS_UNDEF, class_Random_IntervalBoundary_methods);

	crex_enum_add_case_cstr(class_entry, "ClosedOpen", NULL);

	crex_enum_add_case_cstr(class_entry, "ClosedClosed", NULL);

	crex_enum_add_case_cstr(class_entry, "OpenClosed", NULL);

	crex_enum_add_case_cstr(class_entry, "OpenOpen", NULL);

	return class_entry;
}

static crex_class_entry *register_class_Random_RandomError(crex_class_entry *class_entry_Error)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Random", "RandomError", class_Random_RandomError_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Error);
	class_entry->ce_flags |= CREX_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}

static crex_class_entry *register_class_Random_BrokenRandomEngineError(crex_class_entry *class_entry_Random_RandomError)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Random", "BrokenRandomEngineError", class_Random_BrokenRandomEngineError_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Random_RandomError);
	class_entry->ce_flags |= CREX_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}

static crex_class_entry *register_class_Random_RandomException(crex_class_entry *class_entry_Exception)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Random", "RandomException", class_Random_RandomException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Exception);
	class_entry->ce_flags |= CREX_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}
