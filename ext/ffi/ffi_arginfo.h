/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 81892d30ea498304dfa4105fc430a3d43f0ad54f */

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_FFI_cdef, 0, 0, FFI, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, code, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, lib, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_FFI_load, 0, 1, FFI, 1)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_FFI_scope, 0, 1, FFI, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_FFI_new, 0, 1, FFI\\CData, 1)
	CREX_ARG_OBJ_TYPE_MASK(0, type, FFI\\CType, MAY_BE_STRING, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, owned, _IS_BOOL, 0, "true")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, persistent, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_FFI_free, 0, 1, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(CREX_SEND_PREFER_REF, ptr, FFI\\CData, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_FFI_cast, 0, 2, FFI\\CData, 1)
	CREX_ARG_OBJ_TYPE_MASK(0, type, FFI\\CType, MAY_BE_STRING, NULL)
	CREX_ARG_INFO(CREX_SEND_PREFER_REF, ptr)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_FFI_type, 0, 1, FFI\\CType, 1)
	CREX_ARG_TYPE_INFO(0, type, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_FFI_typeof, 0, 1, FFI\\CType, 0)
	CREX_ARG_OBJ_INFO(CREX_SEND_PREFER_REF, ptr, FFI\\CData, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_FFI_arrayType, 0, 2, FFI\\CType, 0)
	CREX_ARG_OBJ_INFO(0, type, FFI\\CType, 0)
	CREX_ARG_TYPE_INFO(0, dimensions, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_FFI_addr, 0, 1, FFI\\CData, 0)
	CREX_ARG_OBJ_INFO(CREX_SEND_PREFER_REF, ptr, FFI\\CData, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_FFI_sizeof, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_TYPE_MASK(CREX_SEND_PREFER_REF, ptr, FFI\\CData|FFI\\CType, 0, NULL)
CREX_END_ARG_INFO()

#define arginfo_class_FFI_alignof arginfo_class_FFI_sizeof

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_FFI_memcpy, 0, 3, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(CREX_SEND_PREFER_REF, to, FFI\\CData, 0)
	CREX_ARG_INFO(CREX_SEND_PREFER_REF, from)
	CREX_ARG_TYPE_INFO(0, size, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_FFI_memcmp, 0, 3, IS_LONG, 0)
	CREX_ARG_INFO(CREX_SEND_PREFER_REF, ptr1)
	CREX_ARG_INFO(CREX_SEND_PREFER_REF, ptr2)
	CREX_ARG_TYPE_INFO(0, size, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_FFI_memset, 0, 3, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(CREX_SEND_PREFER_REF, ptr, FFI\\CData, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, size, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_FFI_string, 0, 1, IS_STRING, 0)
	CREX_ARG_OBJ_INFO(CREX_SEND_PREFER_REF, ptr, FFI\\CData, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, size, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_FFI_isNull, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(CREX_SEND_PREFER_REF, ptr, FFI\\CData, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_FFI_CType_getName, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_FFI_CType_getKind, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_FFI_CType_getSize arginfo_class_FFI_CType_getKind

#define arginfo_class_FFI_CType_getAlignment arginfo_class_FFI_CType_getKind

#define arginfo_class_FFI_CType_getAttributes arginfo_class_FFI_CType_getKind

#define arginfo_class_FFI_CType_getEnumKind arginfo_class_FFI_CType_getKind

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_FFI_CType_getArrayElementType, 0, 0, FFI\\CType, 0)
CREX_END_ARG_INFO()

#define arginfo_class_FFI_CType_getArrayLength arginfo_class_FFI_CType_getKind

#define arginfo_class_FFI_CType_getPointerType arginfo_class_FFI_CType_getArrayElementType

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_FFI_CType_getStructFieldNames, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_FFI_CType_getStructFieldOffset, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_FFI_CType_getStructFieldType, 0, 1, FFI\\CType, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_FFI_CType_getFuncABI arginfo_class_FFI_CType_getKind

#define arginfo_class_FFI_CType_getFuncReturnType arginfo_class_FFI_CType_getArrayElementType

#define arginfo_class_FFI_CType_getFuncParameterCount arginfo_class_FFI_CType_getKind

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_FFI_CType_getFuncParameterType, 0, 1, FFI\\CType, 0)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
CREX_END_ARG_INFO()


CREX_METHOD(FFI, cdef);
CREX_METHOD(FFI, load);
CREX_METHOD(FFI, scope);
CREX_METHOD(FFI, new);
CREX_METHOD(FFI, free);
CREX_METHOD(FFI, cast);
CREX_METHOD(FFI, type);
CREX_METHOD(FFI, typeof);
CREX_METHOD(FFI, arrayType);
CREX_METHOD(FFI, addr);
CREX_METHOD(FFI, sizeof);
CREX_METHOD(FFI, alignof);
CREX_METHOD(FFI, memcpy);
CREX_METHOD(FFI, memcmp);
CREX_METHOD(FFI, memset);
CREX_METHOD(FFI, string);
CREX_METHOD(FFI, isNull);
CREX_METHOD(FFI_CType, getName);
CREX_METHOD(FFI_CType, getKind);
CREX_METHOD(FFI_CType, getSize);
CREX_METHOD(FFI_CType, getAlignment);
CREX_METHOD(FFI_CType, getAttributes);
CREX_METHOD(FFI_CType, getEnumKind);
CREX_METHOD(FFI_CType, getArrayElementType);
CREX_METHOD(FFI_CType, getArrayLength);
CREX_METHOD(FFI_CType, getPointerType);
CREX_METHOD(FFI_CType, getStructFieldNames);
CREX_METHOD(FFI_CType, getStructFieldOffset);
CREX_METHOD(FFI_CType, getStructFieldType);
CREX_METHOD(FFI_CType, getFuncABI);
CREX_METHOD(FFI_CType, getFuncReturnType);
CREX_METHOD(FFI_CType, getFuncParameterCount);
CREX_METHOD(FFI_CType, getFuncParameterType);


static const crex_function_entry class_FFI_methods[] = {
	CREX_ME(FFI, cdef, arginfo_class_FFI_cdef, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, load, arginfo_class_FFI_load, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, scope, arginfo_class_FFI_scope, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, new, arginfo_class_FFI_new, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, free, arginfo_class_FFI_free, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, cast, arginfo_class_FFI_cast, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, type, arginfo_class_FFI_type, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, typeof, arginfo_class_FFI_typeof, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, arrayType, arginfo_class_FFI_arrayType, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, addr, arginfo_class_FFI_addr, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, sizeof, arginfo_class_FFI_sizeof, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, alignof, arginfo_class_FFI_alignof, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, memcpy, arginfo_class_FFI_memcpy, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, memcmp, arginfo_class_FFI_memcmp, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, memset, arginfo_class_FFI_memset, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, string, arginfo_class_FFI_string, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(FFI, isNull, arginfo_class_FFI_isNull, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_FE_END
};


static const crex_function_entry class_FFI_CData_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_FFI_CType_methods[] = {
	CREX_ME(FFI_CType, getName, arginfo_class_FFI_CType_getName, CREX_ACC_PUBLIC)
	CREX_ME(FFI_CType, getKind, arginfo_class_FFI_CType_getKind, CREX_ACC_PUBLIC)
	CREX_ME(FFI_CType, getSize, arginfo_class_FFI_CType_getSize, CREX_ACC_PUBLIC)
	CREX_ME(FFI_CType, getAlignment, arginfo_class_FFI_CType_getAlignment, CREX_ACC_PUBLIC)
	CREX_ME(FFI_CType, getAttributes, arginfo_class_FFI_CType_getAttributes, CREX_ACC_PUBLIC)
	CREX_ME(FFI_CType, getEnumKind, arginfo_class_FFI_CType_getEnumKind, CREX_ACC_PUBLIC)
	CREX_ME(FFI_CType, getArrayElementType, arginfo_class_FFI_CType_getArrayElementType, CREX_ACC_PUBLIC)
	CREX_ME(FFI_CType, getArrayLength, arginfo_class_FFI_CType_getArrayLength, CREX_ACC_PUBLIC)
	CREX_ME(FFI_CType, getPointerType, arginfo_class_FFI_CType_getPointerType, CREX_ACC_PUBLIC)
	CREX_ME(FFI_CType, getStructFieldNames, arginfo_class_FFI_CType_getStructFieldNames, CREX_ACC_PUBLIC)
	CREX_ME(FFI_CType, getStructFieldOffset, arginfo_class_FFI_CType_getStructFieldOffset, CREX_ACC_PUBLIC)
	CREX_ME(FFI_CType, getStructFieldType, arginfo_class_FFI_CType_getStructFieldType, CREX_ACC_PUBLIC)
	CREX_ME(FFI_CType, getFuncABI, arginfo_class_FFI_CType_getFuncABI, CREX_ACC_PUBLIC)
	CREX_ME(FFI_CType, getFuncReturnType, arginfo_class_FFI_CType_getFuncReturnType, CREX_ACC_PUBLIC)
	CREX_ME(FFI_CType, getFuncParameterCount, arginfo_class_FFI_CType_getFuncParameterCount, CREX_ACC_PUBLIC)
	CREX_ME(FFI_CType, getFuncParameterType, arginfo_class_FFI_CType_getFuncParameterType, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_FFI_Exception_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_FFI_ParserException_methods[] = {
	CREX_FE_END
};

static crex_class_entry *register_class_FFI(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "FFI", class_FFI_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NOT_SERIALIZABLE;

	zval const___BIGGEST_ALIGNMENT___value;
	ZVAL_LONG(&const___BIGGEST_ALIGNMENT___value, __BIGGEST_ALIGNMENT__);
	crex_string *const___BIGGEST_ALIGNMENT___name = crex_string_init_interned("__BIGGEST_ALIGNMENT__", sizeof("__BIGGEST_ALIGNMENT__") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const___BIGGEST_ALIGNMENT___name, &const___BIGGEST_ALIGNMENT___value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const___BIGGEST_ALIGNMENT___name);

	return class_entry;
}

static crex_class_entry *register_class_FFI_CData(void)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "FFI", "CData", class_FFI_CData_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}

static crex_class_entry *register_class_FFI_CType(void)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "FFI", "CType", class_FFI_CType_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NOT_SERIALIZABLE;

	zval const_TYPE_VOID_value;
	ZVAL_LONG(&const_TYPE_VOID_value, CREX_FFI_TYPE_VOID);
	crex_string *const_TYPE_VOID_name = crex_string_init_interned("TYPE_VOID", sizeof("TYPE_VOID") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_VOID_name, &const_TYPE_VOID_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_VOID_name);

	zval const_TYPE_FLOAT_value;
	ZVAL_LONG(&const_TYPE_FLOAT_value, CREX_FFI_TYPE_FLOAT);
	crex_string *const_TYPE_FLOAT_name = crex_string_init_interned("TYPE_FLOAT", sizeof("TYPE_FLOAT") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_FLOAT_name, &const_TYPE_FLOAT_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_FLOAT_name);

	zval const_TYPE_DOUBLE_value;
	ZVAL_LONG(&const_TYPE_DOUBLE_value, CREX_FFI_TYPE_DOUBLE);
	crex_string *const_TYPE_DOUBLE_name = crex_string_init_interned("TYPE_DOUBLE", sizeof("TYPE_DOUBLE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_DOUBLE_name, &const_TYPE_DOUBLE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_DOUBLE_name);
#if defined(HAVE_LONG_DOUBLE)

	zval const_TYPE_LONGDOUBLE_value;
	ZVAL_LONG(&const_TYPE_LONGDOUBLE_value, CREX_FFI_TYPE_LONGDOUBLE);
	crex_string *const_TYPE_LONGDOUBLE_name = crex_string_init_interned("TYPE_LONGDOUBLE", sizeof("TYPE_LONGDOUBLE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_LONGDOUBLE_name, &const_TYPE_LONGDOUBLE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_LONGDOUBLE_name);
#endif

	zval const_TYPE_UINT8_value;
	ZVAL_LONG(&const_TYPE_UINT8_value, CREX_FFI_TYPE_UINT8);
	crex_string *const_TYPE_UINT8_name = crex_string_init_interned("TYPE_UINT8", sizeof("TYPE_UINT8") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_UINT8_name, &const_TYPE_UINT8_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_UINT8_name);

	zval const_TYPE_SINT8_value;
	ZVAL_LONG(&const_TYPE_SINT8_value, CREX_FFI_TYPE_SINT8);
	crex_string *const_TYPE_SINT8_name = crex_string_init_interned("TYPE_SINT8", sizeof("TYPE_SINT8") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_SINT8_name, &const_TYPE_SINT8_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_SINT8_name);

	zval const_TYPE_UINT16_value;
	ZVAL_LONG(&const_TYPE_UINT16_value, CREX_FFI_TYPE_UINT16);
	crex_string *const_TYPE_UINT16_name = crex_string_init_interned("TYPE_UINT16", sizeof("TYPE_UINT16") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_UINT16_name, &const_TYPE_UINT16_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_UINT16_name);

	zval const_TYPE_SINT16_value;
	ZVAL_LONG(&const_TYPE_SINT16_value, CREX_FFI_TYPE_SINT16);
	crex_string *const_TYPE_SINT16_name = crex_string_init_interned("TYPE_SINT16", sizeof("TYPE_SINT16") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_SINT16_name, &const_TYPE_SINT16_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_SINT16_name);

	zval const_TYPE_UINT32_value;
	ZVAL_LONG(&const_TYPE_UINT32_value, CREX_FFI_TYPE_UINT32);
	crex_string *const_TYPE_UINT32_name = crex_string_init_interned("TYPE_UINT32", sizeof("TYPE_UINT32") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_UINT32_name, &const_TYPE_UINT32_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_UINT32_name);

	zval const_TYPE_SINT32_value;
	ZVAL_LONG(&const_TYPE_SINT32_value, CREX_FFI_TYPE_SINT32);
	crex_string *const_TYPE_SINT32_name = crex_string_init_interned("TYPE_SINT32", sizeof("TYPE_SINT32") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_SINT32_name, &const_TYPE_SINT32_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_SINT32_name);

	zval const_TYPE_UINT64_value;
	ZVAL_LONG(&const_TYPE_UINT64_value, CREX_FFI_TYPE_UINT64);
	crex_string *const_TYPE_UINT64_name = crex_string_init_interned("TYPE_UINT64", sizeof("TYPE_UINT64") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_UINT64_name, &const_TYPE_UINT64_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_UINT64_name);

	zval const_TYPE_SINT64_value;
	ZVAL_LONG(&const_TYPE_SINT64_value, CREX_FFI_TYPE_SINT64);
	crex_string *const_TYPE_SINT64_name = crex_string_init_interned("TYPE_SINT64", sizeof("TYPE_SINT64") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_SINT64_name, &const_TYPE_SINT64_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_SINT64_name);

	zval const_TYPE_ENUM_value;
	ZVAL_LONG(&const_TYPE_ENUM_value, CREX_FFI_TYPE_ENUM);
	crex_string *const_TYPE_ENUM_name = crex_string_init_interned("TYPE_ENUM", sizeof("TYPE_ENUM") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_ENUM_name, &const_TYPE_ENUM_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_ENUM_name);

	zval const_TYPE_BOOL_value;
	ZVAL_LONG(&const_TYPE_BOOL_value, CREX_FFI_TYPE_BOOL);
	crex_string *const_TYPE_BOOL_name = crex_string_init_interned("TYPE_BOOL", sizeof("TYPE_BOOL") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_BOOL_name, &const_TYPE_BOOL_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_BOOL_name);

	zval const_TYPE_CHAR_value;
	ZVAL_LONG(&const_TYPE_CHAR_value, CREX_FFI_TYPE_CHAR);
	crex_string *const_TYPE_CHAR_name = crex_string_init_interned("TYPE_CHAR", sizeof("TYPE_CHAR") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_CHAR_name, &const_TYPE_CHAR_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_CHAR_name);

	zval const_TYPE_POINTER_value;
	ZVAL_LONG(&const_TYPE_POINTER_value, CREX_FFI_TYPE_POINTER);
	crex_string *const_TYPE_POINTER_name = crex_string_init_interned("TYPE_POINTER", sizeof("TYPE_POINTER") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_POINTER_name, &const_TYPE_POINTER_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_POINTER_name);

	zval const_TYPE_FUNC_value;
	ZVAL_LONG(&const_TYPE_FUNC_value, CREX_FFI_TYPE_FUNC);
	crex_string *const_TYPE_FUNC_name = crex_string_init_interned("TYPE_FUNC", sizeof("TYPE_FUNC") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_FUNC_name, &const_TYPE_FUNC_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_FUNC_name);

	zval const_TYPE_ARRAY_value;
	ZVAL_LONG(&const_TYPE_ARRAY_value, CREX_FFI_TYPE_ARRAY);
	crex_string *const_TYPE_ARRAY_name = crex_string_init_interned("TYPE_ARRAY", sizeof("TYPE_ARRAY") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_ARRAY_name, &const_TYPE_ARRAY_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_ARRAY_name);

	zval const_TYPE_STRUCT_value;
	ZVAL_LONG(&const_TYPE_STRUCT_value, CREX_FFI_TYPE_STRUCT);
	crex_string *const_TYPE_STRUCT_name = crex_string_init_interned("TYPE_STRUCT", sizeof("TYPE_STRUCT") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TYPE_STRUCT_name, &const_TYPE_STRUCT_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TYPE_STRUCT_name);

	zval const_ATTR_CONST_value;
	ZVAL_LONG(&const_ATTR_CONST_value, CREX_FFI_ATTR_CONST);
	crex_string *const_ATTR_CONST_name = crex_string_init_interned("ATTR_CONST", sizeof("ATTR_CONST") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ATTR_CONST_name, &const_ATTR_CONST_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ATTR_CONST_name);

	zval const_ATTR_INCOMPLETE_TAG_value;
	ZVAL_LONG(&const_ATTR_INCOMPLETE_TAG_value, CREX_FFI_ATTR_INCOMPLETE_TAG);
	crex_string *const_ATTR_INCOMPLETE_TAG_name = crex_string_init_interned("ATTR_INCOMPLETE_TAG", sizeof("ATTR_INCOMPLETE_TAG") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ATTR_INCOMPLETE_TAG_name, &const_ATTR_INCOMPLETE_TAG_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ATTR_INCOMPLETE_TAG_name);

	zval const_ATTR_VARIADIC_value;
	ZVAL_LONG(&const_ATTR_VARIADIC_value, CREX_FFI_ATTR_VARIADIC);
	crex_string *const_ATTR_VARIADIC_name = crex_string_init_interned("ATTR_VARIADIC", sizeof("ATTR_VARIADIC") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ATTR_VARIADIC_name, &const_ATTR_VARIADIC_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ATTR_VARIADIC_name);

	zval const_ATTR_INCOMPLETE_ARRAY_value;
	ZVAL_LONG(&const_ATTR_INCOMPLETE_ARRAY_value, CREX_FFI_ATTR_INCOMPLETE_ARRAY);
	crex_string *const_ATTR_INCOMPLETE_ARRAY_name = crex_string_init_interned("ATTR_INCOMPLETE_ARRAY", sizeof("ATTR_INCOMPLETE_ARRAY") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ATTR_INCOMPLETE_ARRAY_name, &const_ATTR_INCOMPLETE_ARRAY_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ATTR_INCOMPLETE_ARRAY_name);

	zval const_ATTR_VLA_value;
	ZVAL_LONG(&const_ATTR_VLA_value, CREX_FFI_ATTR_VLA);
	crex_string *const_ATTR_VLA_name = crex_string_init_interned("ATTR_VLA", sizeof("ATTR_VLA") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ATTR_VLA_name, &const_ATTR_VLA_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ATTR_VLA_name);

	zval const_ATTR_UNION_value;
	ZVAL_LONG(&const_ATTR_UNION_value, CREX_FFI_ATTR_UNION);
	crex_string *const_ATTR_UNION_name = crex_string_init_interned("ATTR_UNION", sizeof("ATTR_UNION") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ATTR_UNION_name, &const_ATTR_UNION_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ATTR_UNION_name);

	zval const_ATTR_PACKED_value;
	ZVAL_LONG(&const_ATTR_PACKED_value, CREX_FFI_ATTR_PACKED);
	crex_string *const_ATTR_PACKED_name = crex_string_init_interned("ATTR_PACKED", sizeof("ATTR_PACKED") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ATTR_PACKED_name, &const_ATTR_PACKED_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ATTR_PACKED_name);

	zval const_ATTR_MS_STRUCT_value;
	ZVAL_LONG(&const_ATTR_MS_STRUCT_value, CREX_FFI_ATTR_MS_STRUCT);
	crex_string *const_ATTR_MS_STRUCT_name = crex_string_init_interned("ATTR_MS_STRUCT", sizeof("ATTR_MS_STRUCT") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ATTR_MS_STRUCT_name, &const_ATTR_MS_STRUCT_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ATTR_MS_STRUCT_name);

	zval const_ATTR_GCC_STRUCT_value;
	ZVAL_LONG(&const_ATTR_GCC_STRUCT_value, CREX_FFI_ATTR_GCC_STRUCT);
	crex_string *const_ATTR_GCC_STRUCT_name = crex_string_init_interned("ATTR_GCC_STRUCT", sizeof("ATTR_GCC_STRUCT") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ATTR_GCC_STRUCT_name, &const_ATTR_GCC_STRUCT_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ATTR_GCC_STRUCT_name);

	zval const_ABI_DEFAULT_value;
	ZVAL_LONG(&const_ABI_DEFAULT_value, CREX_FFI_ABI_DEFAULT);
	crex_string *const_ABI_DEFAULT_name = crex_string_init_interned("ABI_DEFAULT", sizeof("ABI_DEFAULT") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ABI_DEFAULT_name, &const_ABI_DEFAULT_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ABI_DEFAULT_name);

	zval const_ABI_CDECL_value;
	ZVAL_LONG(&const_ABI_CDECL_value, CREX_FFI_ABI_CDECL);
	crex_string *const_ABI_CDECL_name = crex_string_init_interned("ABI_CDECL", sizeof("ABI_CDECL") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ABI_CDECL_name, &const_ABI_CDECL_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ABI_CDECL_name);

	zval const_ABI_FASTCALL_value;
	ZVAL_LONG(&const_ABI_FASTCALL_value, CREX_FFI_ABI_FASTCALL);
	crex_string *const_ABI_FASTCALL_name = crex_string_init_interned("ABI_FASTCALL", sizeof("ABI_FASTCALL") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ABI_FASTCALL_name, &const_ABI_FASTCALL_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ABI_FASTCALL_name);

	zval const_ABI_THISCALL_value;
	ZVAL_LONG(&const_ABI_THISCALL_value, CREX_FFI_ABI_THISCALL);
	crex_string *const_ABI_THISCALL_name = crex_string_init_interned("ABI_THISCALL", sizeof("ABI_THISCALL") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ABI_THISCALL_name, &const_ABI_THISCALL_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ABI_THISCALL_name);

	zval const_ABI_STDCALL_value;
	ZVAL_LONG(&const_ABI_STDCALL_value, CREX_FFI_ABI_STDCALL);
	crex_string *const_ABI_STDCALL_name = crex_string_init_interned("ABI_STDCALL", sizeof("ABI_STDCALL") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ABI_STDCALL_name, &const_ABI_STDCALL_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ABI_STDCALL_name);

	zval const_ABI_PASCAL_value;
	ZVAL_LONG(&const_ABI_PASCAL_value, CREX_FFI_ABI_PASCAL);
	crex_string *const_ABI_PASCAL_name = crex_string_init_interned("ABI_PASCAL", sizeof("ABI_PASCAL") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ABI_PASCAL_name, &const_ABI_PASCAL_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ABI_PASCAL_name);

	zval const_ABI_REGISTER_value;
	ZVAL_LONG(&const_ABI_REGISTER_value, CREX_FFI_ABI_REGISTER);
	crex_string *const_ABI_REGISTER_name = crex_string_init_interned("ABI_REGISTER", sizeof("ABI_REGISTER") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ABI_REGISTER_name, &const_ABI_REGISTER_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ABI_REGISTER_name);

	zval const_ABI_MS_value;
	ZVAL_LONG(&const_ABI_MS_value, CREX_FFI_ABI_MS);
	crex_string *const_ABI_MS_name = crex_string_init_interned("ABI_MS", sizeof("ABI_MS") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ABI_MS_name, &const_ABI_MS_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ABI_MS_name);

	zval const_ABI_SYSV_value;
	ZVAL_LONG(&const_ABI_SYSV_value, CREX_FFI_ABI_SYSV);
	crex_string *const_ABI_SYSV_name = crex_string_init_interned("ABI_SYSV", sizeof("ABI_SYSV") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ABI_SYSV_name, &const_ABI_SYSV_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ABI_SYSV_name);

	zval const_ABI_VECTORCALL_value;
	ZVAL_LONG(&const_ABI_VECTORCALL_value, CREX_FFI_ABI_VECTORCALL);
	crex_string *const_ABI_VECTORCALL_name = crex_string_init_interned("ABI_VECTORCALL", sizeof("ABI_VECTORCALL") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ABI_VECTORCALL_name, &const_ABI_VECTORCALL_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ABI_VECTORCALL_name);

	return class_entry;
}

static crex_class_entry *register_class_FFI_Exception(crex_class_entry *class_entry_Error)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "FFI", "Exception", class_FFI_Exception_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Error);

	return class_entry;
}

static crex_class_entry *register_class_FFI_ParserException(crex_class_entry *class_entry_FFI_Exception)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "FFI", "ParserException", class_FFI_ParserException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_FFI_Exception);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	return class_entry;
}
