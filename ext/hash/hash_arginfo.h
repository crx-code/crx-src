/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: f4531ebc6817042a2729c3dd1502631656c29f05 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_hash, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, algo, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, binary, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_hash_file, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, algo, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, binary, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_hash_hmac, 0, 3, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, algo, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, binary, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_hash_hmac_file, 0, 3, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, algo, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, binary, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_hash_init, 0, 1, HashContext, 0)
	CREX_ARG_TYPE_INFO(0, algo, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, key, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_hash_update, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, context, HashContext, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_hash_update_stream, 0, 2, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, context, HashContext, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_hash_update_file, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, context, HashContext, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, stream_context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_hash_final, 0, 1, IS_STRING, 0)
	CREX_ARG_OBJ_INFO(0, context, HashContext, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, binary, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_hash_copy, 0, 1, HashContext, 0)
	CREX_ARG_OBJ_INFO(0, context, HashContext, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_hash_algos, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_hash_hmac_algos arginfo_hash_algos

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_hash_pbkdf2, 0, 4, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, algo, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, password, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, salt, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, iterations, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, binary, _IS_BOOL, 0, "false")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_hash_equals, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, known_string, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, user_string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_hash_hkdf, 0, 2, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, algo, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, info, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, salt, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

#if defined(CRX_MHASH_BC)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mhash_get_block_size, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, algo, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

#if defined(CRX_MHASH_BC)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mhash_get_hash_name, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, algo, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

#if defined(CRX_MHASH_BC)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mhash_keygen_s2k, 0, 4, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, algo, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, password, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, salt, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, length, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

#if defined(CRX_MHASH_BC)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_mhash_count, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

#if defined(CRX_MHASH_BC)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_mhash, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, algo, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, key, IS_STRING, 1, "null")
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_INFO_EX(arginfo_class_HashContext___main, 0, 0, 0)
CREX_END_ARG_INFO()

#define arginfo_class_HashContext___serialize arginfo_hash_algos

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_HashContext___unserialize, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(hash);
CREX_FUNCTION(hash_file);
CREX_FUNCTION(hash_hmac);
CREX_FUNCTION(hash_hmac_file);
CREX_FUNCTION(hash_init);
CREX_FUNCTION(hash_update);
CREX_FUNCTION(hash_update_stream);
CREX_FUNCTION(hash_update_file);
CREX_FUNCTION(hash_final);
CREX_FUNCTION(hash_copy);
CREX_FUNCTION(hash_algos);
CREX_FUNCTION(hash_hmac_algos);
CREX_FUNCTION(hash_pbkdf2);
CREX_FUNCTION(hash_equals);
CREX_FUNCTION(hash_hkdf);
#if defined(CRX_MHASH_BC)
CREX_FUNCTION(mhash_get_block_size);
#endif
#if defined(CRX_MHASH_BC)
CREX_FUNCTION(mhash_get_hash_name);
#endif
#if defined(CRX_MHASH_BC)
CREX_FUNCTION(mhash_keygen_s2k);
#endif
#if defined(CRX_MHASH_BC)
CREX_FUNCTION(mhash_count);
#endif
#if defined(CRX_MHASH_BC)
CREX_FUNCTION(mhash);
#endif
CREX_METHOD(HashContext, __main);
CREX_METHOD(HashContext, __serialize);
CREX_METHOD(HashContext, __unserialize);


static const crex_function_entry ext_functions[] = {
	CREX_FE(hash, arginfo_hash)
	CREX_FE(hash_file, arginfo_hash_file)
	CREX_FE(hash_hmac, arginfo_hash_hmac)
	CREX_FE(hash_hmac_file, arginfo_hash_hmac_file)
	CREX_FE(hash_init, arginfo_hash_init)
	CREX_FE(hash_update, arginfo_hash_update)
	CREX_FE(hash_update_stream, arginfo_hash_update_stream)
	CREX_FE(hash_update_file, arginfo_hash_update_file)
	CREX_FE(hash_final, arginfo_hash_final)
	CREX_FE(hash_copy, arginfo_hash_copy)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(hash_algos, arginfo_hash_algos)
	CREX_SUPPORTS_COMPILE_TIME_EVAL_FE(hash_hmac_algos, arginfo_hash_hmac_algos)
	CREX_FE(hash_pbkdf2, arginfo_hash_pbkdf2)
	CREX_FE(hash_equals, arginfo_hash_equals)
	CREX_FE(hash_hkdf, arginfo_hash_hkdf)
#if defined(CRX_MHASH_BC)
	CREX_DEP_FE(mhash_get_block_size, arginfo_mhash_get_block_size)
#endif
#if defined(CRX_MHASH_BC)
	CREX_DEP_FE(mhash_get_hash_name, arginfo_mhash_get_hash_name)
#endif
#if defined(CRX_MHASH_BC)
	CREX_DEP_FE(mhash_keygen_s2k, arginfo_mhash_keygen_s2k)
#endif
#if defined(CRX_MHASH_BC)
	CREX_DEP_FE(mhash_count, arginfo_mhash_count)
#endif
#if defined(CRX_MHASH_BC)
	CREX_DEP_FE(mhash, arginfo_mhash)
#endif
	CREX_FE_END
};


static const crex_function_entry class_HashContext_methods[] = {
	CREX_ME(HashContext, __main, arginfo_class_HashContext___main, CREX_ACC_PRIVATE)
	CREX_ME(HashContext, __serialize, arginfo_class_HashContext___serialize, CREX_ACC_PUBLIC)
	CREX_ME(HashContext, __unserialize, arginfo_class_HashContext___unserialize, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static void register_hash_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("HASH_HMAC", CRX_HASH_HMAC, CONST_PERSISTENT);


	crex_add_parameter_attribute(crex_hash_str_find_ptr(CG(function_table), "hash_hmac", sizeof("hash_hmac") - 1), 2, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);

	crex_add_parameter_attribute(crex_hash_str_find_ptr(CG(function_table), "hash_hmac_file", sizeof("hash_hmac_file") - 1), 2, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);

	crex_add_parameter_attribute(crex_hash_str_find_ptr(CG(function_table), "hash_init", sizeof("hash_init") - 1), 2, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);

	crex_add_parameter_attribute(crex_hash_str_find_ptr(CG(function_table), "hash_pbkdf2", sizeof("hash_pbkdf2") - 1), 1, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);

	crex_add_parameter_attribute(crex_hash_str_find_ptr(CG(function_table), "hash_equals", sizeof("hash_equals") - 1), 0, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);

	crex_add_parameter_attribute(crex_hash_str_find_ptr(CG(function_table), "hash_equals", sizeof("hash_equals") - 1), 1, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);

	crex_add_parameter_attribute(crex_hash_str_find_ptr(CG(function_table), "hash_hkdf", sizeof("hash_hkdf") - 1), 1, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);
}

static crex_class_entry *register_class_HashContext(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "HashContext", class_HashContext_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	return class_entry;
}
