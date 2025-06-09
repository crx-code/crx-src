/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: b91206482b5119ce6d7c899e9599acfa2e06ec2a */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_variant_set, 0, 2, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, variant, variant, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_variant_add, 0, 2, variant, 0)
	CREX_ARG_TYPE_INFO(0, left, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, right, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_variant_cat arginfo_variant_add

#define arginfo_variant_sub arginfo_variant_add

#define arginfo_variant_mul arginfo_variant_add

#define arginfo_variant_and arginfo_variant_add

#define arginfo_variant_div arginfo_variant_add

#define arginfo_variant_eqv arginfo_variant_add

#define arginfo_variant_idiv arginfo_variant_add

#define arginfo_variant_imp arginfo_variant_add

#define arginfo_variant_mod arginfo_variant_add

#define arginfo_variant_or arginfo_variant_add

#define arginfo_variant_pow arginfo_variant_add

#define arginfo_variant_xor arginfo_variant_add

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_variant_abs, 0, 1, variant, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_variant_fix arginfo_variant_abs

#define arginfo_variant_int arginfo_variant_abs

#define arginfo_variant_neg arginfo_variant_abs

#define arginfo_variant_not arginfo_variant_abs

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_variant_round, 0, 2, variant, 1)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, decimals, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_variant_cmp, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, left, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, right, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, locale_id, IS_LONG, 0, "LOCALE_SYSTEM_DEFAULT")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_variant_date_to_timestamp, 0, 1, IS_LONG, 1)
	CREX_ARG_OBJ_INFO(0, variant, variant, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_variant_date_from_timestamp, 0, 1, variant, 0)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_variant_get_type, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, variant, variant, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_variant_set_type, 0, 2, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, variant, variant, 0)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_variant_cast, 0, 2, variant, 0)
	CREX_ARG_OBJ_INFO(0, variant, variant, 0)
	CREX_ARG_TYPE_INFO(0, type, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_com_get_active_object, 0, 1, variant, 0)
	CREX_ARG_TYPE_INFO(0, prog_id, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, codepage, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_com_create_guid, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_com_event_sink, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, variant, variant, 0)
	CREX_ARG_TYPE_INFO(0, sink_object, IS_OBJECT, 0)
	CREX_ARG_TYPE_MASK(0, sink_interface, MAY_BE_ARRAY|MAY_BE_STRING|MAY_BE_NULL, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_com_print_typeinfo, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_TYPE_MASK(0, variant, variant, MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dispatch_interface, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, display_sink, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_com_message_pump, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout_milliseconds, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_com_load_typelib, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, typelib, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, case_insensitive, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_variant___main, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_MIXED, 0, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "VT_EMPTY")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, codepage, IS_LONG, 0, "CP_ACP")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_com___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, module_name, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, server_name, MAY_BE_ARRAY|MAY_BE_STRING|MAY_BE_NULL, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, codepage, IS_LONG, 0, "CP_ACP")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, typelib, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

#if HAVE_MSCOREE_H
CREX_BEGIN_ARG_INFO_EX(arginfo_class_dotnet___main, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, assembly_name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, datatype_name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, codepage, IS_LONG, 0, "CP_ACP")
CREX_END_ARG_INFO()
#endif


CREX_FUNCTION(variant_set);
CREX_FUNCTION(variant_add);
CREX_FUNCTION(variant_cat);
CREX_FUNCTION(variant_sub);
CREX_FUNCTION(variant_mul);
CREX_FUNCTION(variant_and);
CREX_FUNCTION(variant_div);
CREX_FUNCTION(variant_eqv);
CREX_FUNCTION(variant_idiv);
CREX_FUNCTION(variant_imp);
CREX_FUNCTION(variant_mod);
CREX_FUNCTION(variant_or);
CREX_FUNCTION(variant_pow);
CREX_FUNCTION(variant_xor);
CREX_FUNCTION(variant_abs);
CREX_FUNCTION(variant_fix);
CREX_FUNCTION(variant_int);
CREX_FUNCTION(variant_neg);
CREX_FUNCTION(variant_not);
CREX_FUNCTION(variant_round);
CREX_FUNCTION(variant_cmp);
CREX_FUNCTION(variant_date_to_timestamp);
CREX_FUNCTION(variant_date_from_timestamp);
CREX_FUNCTION(variant_get_type);
CREX_FUNCTION(variant_set_type);
CREX_FUNCTION(variant_cast);
CREX_FUNCTION(com_get_active_object);
CREX_FUNCTION(com_create_guid);
CREX_FUNCTION(com_event_sink);
CREX_FUNCTION(com_print_typeinfo);
CREX_FUNCTION(com_message_pump);
CREX_FUNCTION(com_load_typelib);
CREX_METHOD(variant, __main);
CREX_METHOD(com, __main);
#if HAVE_MSCOREE_H
CREX_METHOD(dotnet, __main);
#endif


static const crex_function_entry ext_functions[] = {
	CREX_FE(variant_set, arginfo_variant_set)
	CREX_FE(variant_add, arginfo_variant_add)
	CREX_FE(variant_cat, arginfo_variant_cat)
	CREX_FE(variant_sub, arginfo_variant_sub)
	CREX_FE(variant_mul, arginfo_variant_mul)
	CREX_FE(variant_and, arginfo_variant_and)
	CREX_FE(variant_div, arginfo_variant_div)
	CREX_FE(variant_eqv, arginfo_variant_eqv)
	CREX_FE(variant_idiv, arginfo_variant_idiv)
	CREX_FE(variant_imp, arginfo_variant_imp)
	CREX_FE(variant_mod, arginfo_variant_mod)
	CREX_FE(variant_or, arginfo_variant_or)
	CREX_FE(variant_pow, arginfo_variant_pow)
	CREX_FE(variant_xor, arginfo_variant_xor)
	CREX_FE(variant_abs, arginfo_variant_abs)
	CREX_FE(variant_fix, arginfo_variant_fix)
	CREX_FE(variant_int, arginfo_variant_int)
	CREX_FE(variant_neg, arginfo_variant_neg)
	CREX_FE(variant_not, arginfo_variant_not)
	CREX_FE(variant_round, arginfo_variant_round)
	CREX_FE(variant_cmp, arginfo_variant_cmp)
	CREX_FE(variant_date_to_timestamp, arginfo_variant_date_to_timestamp)
	CREX_FE(variant_date_from_timestamp, arginfo_variant_date_from_timestamp)
	CREX_FE(variant_get_type, arginfo_variant_get_type)
	CREX_FE(variant_set_type, arginfo_variant_set_type)
	CREX_FE(variant_cast, arginfo_variant_cast)
	CREX_FE(com_get_active_object, arginfo_com_get_active_object)
	CREX_FE(com_create_guid, arginfo_com_create_guid)
	CREX_FE(com_event_sink, arginfo_com_event_sink)
	CREX_FE(com_print_typeinfo, arginfo_com_print_typeinfo)
	CREX_FE(com_message_pump, arginfo_com_message_pump)
	CREX_FE(com_load_typelib, arginfo_com_load_typelib)
	CREX_FE_END
};


static const crex_function_entry class_variant_methods[] = {
	CREX_ME(variant, __main, arginfo_class_variant___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_com_methods[] = {
	CREX_ME(com, __main, arginfo_class_com___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};


#if HAVE_MSCOREE_H
static const crex_function_entry class_dotnet_methods[] = {
	CREX_ME(dotnet, __main, arginfo_class_dotnet___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};
#endif


static const crex_function_entry class_com_safearray_proxy_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_com_exception_methods[] = {
	CREX_FE_END
};

static void register_com_extension_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("CLSCTX_INPROC_SERVER", CLSCTX_INPROC_SERVER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CLSCTX_INPROC_HANDLER", CLSCTX_INPROC_HANDLER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CLSCTX_LOCAL_SERVER", CLSCTX_LOCAL_SERVER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CLSCTX_REMOTE_SERVER", CLSCTX_REMOTE_SERVER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CLSCTX_SERVER", CLSCTX_SERVER, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CLSCTX_ALL", CLSCTX_ALL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_NULL", VT_NULL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_EMPTY", VT_EMPTY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_UI1", VT_UI1, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_I1", VT_I1, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_UI2", VT_UI2, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_I2", VT_I2, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_UI4", VT_UI4, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_I4", VT_I4, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_R4", VT_R4, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_R8", VT_R8, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_BOOL", VT_BOOL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_ERROR", VT_ERROR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_CY", VT_CY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_DATE", VT_DATE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_BSTR", VT_BSTR, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_DECIMAL", VT_DECIMAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_UNKNOWN", VT_UNKNOWN, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_DISPATCH", VT_DISPATCH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_VARIANT", VT_VARIANT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_INT", VT_INT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_UINT", VT_UINT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_ARRAY", VT_ARRAY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VT_BYREF", VT_BYREF, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CP_ACP", CP_ACP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CP_MACCP", CP_MACCP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CP_OEMCP", CP_OEMCP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CP_UTF7", CP_UTF7, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CP_UTF8", CP_UTF8, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CP_SYMBOL", CP_SYMBOL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CP_THREAD_ACP", CP_THREAD_ACP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VARCMP_LT", VARCMP_LT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VARCMP_EQ", VARCMP_EQ, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VARCMP_GT", VARCMP_GT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VARCMP_NULL", VARCMP_NULL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOCALE_SYSTEM_DEFAULT", LOCALE_SYSTEM_DEFAULT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOCALE_NEUTRAL", LOCALE_NEUTRAL, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NORM_IGNORECASE", NORM_IGNORECASE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NORM_IGNORENONSPACE", NORM_IGNORENONSPACE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NORM_IGNORESYMBOLS", NORM_IGNORESYMBOLS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NORM_IGNOREWIDTH", NORM_IGNOREWIDTH, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NORM_IGNOREKANATYPE", NORM_IGNOREKANATYPE, CONST_PERSISTENT);
#if defined(NORM_IGNOREKASHIDA)
	REGISTER_LONG_CONSTANT("NORM_IGNOREKASHIDA", NORM_IGNOREKASHIDA, CONST_PERSISTENT);
#endif
	REGISTER_LONG_CONSTANT("DISP_E_DIVBYZERO", CRX_DISP_E_DIVBYZERO, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DISP_E_OVERFLOW", CRX_DISP_E_OVERFLOW, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DISP_E_BADINDEX", CRX_DISP_E_BADINDEX, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DISP_E_PARAMNOTFOUND", CRX_DISP_E_PARAMNOTFOUND, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MK_E_UNAVAILABLE", CRX_MK_E_UNAVAILABLE, CONST_PERSISTENT);
#if SIZEOF_CREX_LONG == 8
	REGISTER_LONG_CONSTANT("VT_UI8", VT_UI8, CONST_PERSISTENT);
#endif
#if SIZEOF_CREX_LONG == 8
	REGISTER_LONG_CONSTANT("VT_I8", VT_I8, CONST_PERSISTENT);
#endif
}

static crex_class_entry *register_class_variant(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "variant", class_variant_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}

static crex_class_entry *register_class_com(crex_class_entry *class_entry_variant)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "com", class_com_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_variant);

	return class_entry;
}

#if HAVE_MSCOREE_H
static crex_class_entry *register_class_dotnet(crex_class_entry *class_entry_variant)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "dotnet", class_dotnet_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_variant);

	return class_entry;
}
#endif

static crex_class_entry *register_class_com_safearray_proxy(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "com_safearray_proxy", class_com_safearray_proxy_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	return class_entry;
}

static crex_class_entry *register_class_com_exception(crex_class_entry *class_entry_Exception)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "com_exception", class_com_exception_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Exception);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	return class_entry;
}
