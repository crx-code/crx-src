/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: d0ce4612e25d2b8a765544c835fa2347ae9b23f2 */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_ArrayObject___main, 0, 0, 0)
	CREX_ARG_TYPE_MASK(0, array, MAY_BE_ARRAY|MAY_BE_OBJECT, "[]")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, iteratorClass, IS_STRING, 0, "ArrayIterator::class")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject_offsetExists, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, key, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject_offsetGet, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, key, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject_offsetSet, 0, 2, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, key, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject_offsetUnset, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, key, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject_append, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject_getArrayCopy, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject_count, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_ArrayObject_getFlags arginfo_class_ArrayObject_count

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject_setFlags, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject_asort, 0, 0, IS_TRUE, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "SORT_REGULAR")
CREX_END_ARG_INFO()

#define arginfo_class_ArrayObject_ksort arginfo_class_ArrayObject_asort

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject_uasort, 0, 1, IS_TRUE, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()

#define arginfo_class_ArrayObject_uksort arginfo_class_ArrayObject_uasort

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject_natsort, 0, 0, IS_TRUE, 0)
CREX_END_ARG_INFO()

#define arginfo_class_ArrayObject_natcasesort arginfo_class_ArrayObject_natsort

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject_unserialize, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject_serialize, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_ArrayObject___serialize arginfo_class_ArrayObject_getArrayCopy

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject___unserialize, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_ArrayObject_getIterator, 0, 0, Iterator, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject_exchangeArray, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_MASK(0, array, MAY_BE_ARRAY|MAY_BE_OBJECT, NULL)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayObject_setIteratorClass, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, iteratorClass, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_ArrayObject_getIteratorClass arginfo_class_ArrayObject_serialize

#define arginfo_class_ArrayObject___debugInfo arginfo_class_ArrayObject_getArrayCopy

CREX_BEGIN_ARG_INFO_EX(arginfo_class_ArrayIterator___main, 0, 0, 0)
	CREX_ARG_TYPE_MASK(0, array, MAY_BE_ARRAY|MAY_BE_OBJECT, "[]")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

#define arginfo_class_ArrayIterator_offsetExists arginfo_class_ArrayObject_offsetExists

#define arginfo_class_ArrayIterator_offsetGet arginfo_class_ArrayObject_offsetGet

#define arginfo_class_ArrayIterator_offsetSet arginfo_class_ArrayObject_offsetSet

#define arginfo_class_ArrayIterator_offsetUnset arginfo_class_ArrayObject_offsetUnset

#define arginfo_class_ArrayIterator_append arginfo_class_ArrayObject_append

#define arginfo_class_ArrayIterator_getArrayCopy arginfo_class_ArrayObject_getArrayCopy

#define arginfo_class_ArrayIterator_count arginfo_class_ArrayObject_count

#define arginfo_class_ArrayIterator_getFlags arginfo_class_ArrayObject_count

#define arginfo_class_ArrayIterator_setFlags arginfo_class_ArrayObject_setFlags

#define arginfo_class_ArrayIterator_asort arginfo_class_ArrayObject_asort

#define arginfo_class_ArrayIterator_ksort arginfo_class_ArrayObject_asort

#define arginfo_class_ArrayIterator_uasort arginfo_class_ArrayObject_uasort

#define arginfo_class_ArrayIterator_uksort arginfo_class_ArrayObject_uasort

#define arginfo_class_ArrayIterator_natsort arginfo_class_ArrayObject_natsort

#define arginfo_class_ArrayIterator_natcasesort arginfo_class_ArrayObject_natsort

#define arginfo_class_ArrayIterator_unserialize arginfo_class_ArrayObject_unserialize

#define arginfo_class_ArrayIterator_serialize arginfo_class_ArrayObject_serialize

#define arginfo_class_ArrayIterator___serialize arginfo_class_ArrayObject_getArrayCopy

#define arginfo_class_ArrayIterator___unserialize arginfo_class_ArrayObject___unserialize

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayIterator_rewind, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayIterator_current, 0, 0, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_ArrayIterator_key, 0, 0, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_NULL)
CREX_END_ARG_INFO()

#define arginfo_class_ArrayIterator_next arginfo_class_ArrayIterator_rewind

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayIterator_valid, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayIterator_seek, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_ArrayIterator___debugInfo arginfo_class_ArrayObject_getArrayCopy

#define arginfo_class_RecursiveArrayIterator_hasChildren arginfo_class_ArrayIterator_valid

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_RecursiveArrayIterator_getChildren, 0, 0, RecursiveArrayIterator, 1)
CREX_END_ARG_INFO()


CREX_METHOD(ArrayObject, __main);
CREX_METHOD(ArrayObject, offsetExists);
CREX_METHOD(ArrayObject, offsetGet);
CREX_METHOD(ArrayObject, offsetSet);
CREX_METHOD(ArrayObject, offsetUnset);
CREX_METHOD(ArrayObject, append);
CREX_METHOD(ArrayObject, getArrayCopy);
CREX_METHOD(ArrayObject, count);
CREX_METHOD(ArrayObject, getFlags);
CREX_METHOD(ArrayObject, setFlags);
CREX_METHOD(ArrayObject, asort);
CREX_METHOD(ArrayObject, ksort);
CREX_METHOD(ArrayObject, uasort);
CREX_METHOD(ArrayObject, uksort);
CREX_METHOD(ArrayObject, natsort);
CREX_METHOD(ArrayObject, natcasesort);
CREX_METHOD(ArrayObject, unserialize);
CREX_METHOD(ArrayObject, serialize);
CREX_METHOD(ArrayObject, __serialize);
CREX_METHOD(ArrayObject, __unserialize);
CREX_METHOD(ArrayObject, getIterator);
CREX_METHOD(ArrayObject, exchangeArray);
CREX_METHOD(ArrayObject, setIteratorClass);
CREX_METHOD(ArrayObject, getIteratorClass);
CREX_METHOD(ArrayObject, __debugInfo);
CREX_METHOD(ArrayIterator, __main);
CREX_METHOD(ArrayIterator, rewind);
CREX_METHOD(ArrayIterator, current);
CREX_METHOD(ArrayIterator, key);
CREX_METHOD(ArrayIterator, next);
CREX_METHOD(ArrayIterator, valid);
CREX_METHOD(ArrayIterator, seek);
CREX_METHOD(RecursiveArrayIterator, hasChildren);
CREX_METHOD(RecursiveArrayIterator, getChildren);


static const crex_function_entry class_ArrayObject_methods[] = {
	CREX_ME(ArrayObject, __main, arginfo_class_ArrayObject___main, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, offsetExists, arginfo_class_ArrayObject_offsetExists, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, offsetGet, arginfo_class_ArrayObject_offsetGet, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, offsetSet, arginfo_class_ArrayObject_offsetSet, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, offsetUnset, arginfo_class_ArrayObject_offsetUnset, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, append, arginfo_class_ArrayObject_append, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, getArrayCopy, arginfo_class_ArrayObject_getArrayCopy, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, count, arginfo_class_ArrayObject_count, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, getFlags, arginfo_class_ArrayObject_getFlags, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, setFlags, arginfo_class_ArrayObject_setFlags, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, asort, arginfo_class_ArrayObject_asort, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, ksort, arginfo_class_ArrayObject_ksort, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, uasort, arginfo_class_ArrayObject_uasort, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, uksort, arginfo_class_ArrayObject_uksort, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, natsort, arginfo_class_ArrayObject_natsort, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, natcasesort, arginfo_class_ArrayObject_natcasesort, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, unserialize, arginfo_class_ArrayObject_unserialize, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, serialize, arginfo_class_ArrayObject_serialize, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, __serialize, arginfo_class_ArrayObject___serialize, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, __unserialize, arginfo_class_ArrayObject___unserialize, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, getIterator, arginfo_class_ArrayObject_getIterator, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, exchangeArray, arginfo_class_ArrayObject_exchangeArray, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, setIteratorClass, arginfo_class_ArrayObject_setIteratorClass, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, getIteratorClass, arginfo_class_ArrayObject_getIteratorClass, CREX_ACC_PUBLIC)
	CREX_ME(ArrayObject, __debugInfo, arginfo_class_ArrayObject___debugInfo, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_ArrayIterator_methods[] = {
	CREX_ME(ArrayIterator, __main, arginfo_class_ArrayIterator___main, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, offsetExists, offsetExists, arginfo_class_ArrayIterator_offsetExists, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, offsetGet, offsetGet, arginfo_class_ArrayIterator_offsetGet, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, offsetSet, offsetSet, arginfo_class_ArrayIterator_offsetSet, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, offsetUnset, offsetUnset, arginfo_class_ArrayIterator_offsetUnset, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, append, append, arginfo_class_ArrayIterator_append, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, getArrayCopy, getArrayCopy, arginfo_class_ArrayIterator_getArrayCopy, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, count, count, arginfo_class_ArrayIterator_count, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, getFlags, getFlags, arginfo_class_ArrayIterator_getFlags, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, setFlags, setFlags, arginfo_class_ArrayIterator_setFlags, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, asort, asort, arginfo_class_ArrayIterator_asort, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, ksort, ksort, arginfo_class_ArrayIterator_ksort, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, uasort, uasort, arginfo_class_ArrayIterator_uasort, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, uksort, uksort, arginfo_class_ArrayIterator_uksort, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, natsort, natsort, arginfo_class_ArrayIterator_natsort, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, natcasesort, natcasesort, arginfo_class_ArrayIterator_natcasesort, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, unserialize, unserialize, arginfo_class_ArrayIterator_unserialize, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, serialize, serialize, arginfo_class_ArrayIterator_serialize, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, __serialize, __serialize, arginfo_class_ArrayIterator___serialize, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, __unserialize, __unserialize, arginfo_class_ArrayIterator___unserialize, CREX_ACC_PUBLIC)
	CREX_ME(ArrayIterator, rewind, arginfo_class_ArrayIterator_rewind, CREX_ACC_PUBLIC)
	CREX_ME(ArrayIterator, current, arginfo_class_ArrayIterator_current, CREX_ACC_PUBLIC)
	CREX_ME(ArrayIterator, key, arginfo_class_ArrayIterator_key, CREX_ACC_PUBLIC)
	CREX_ME(ArrayIterator, next, arginfo_class_ArrayIterator_next, CREX_ACC_PUBLIC)
	CREX_ME(ArrayIterator, valid, arginfo_class_ArrayIterator_valid, CREX_ACC_PUBLIC)
	CREX_ME(ArrayIterator, seek, arginfo_class_ArrayIterator_seek, CREX_ACC_PUBLIC)
	CREX_MALIAS(ArrayObject, __debugInfo, __debugInfo, arginfo_class_ArrayIterator___debugInfo, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_RecursiveArrayIterator_methods[] = {
	CREX_ME(RecursiveArrayIterator, hasChildren, arginfo_class_RecursiveArrayIterator_hasChildren, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveArrayIterator, getChildren, arginfo_class_RecursiveArrayIterator_getChildren, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_ArrayObject(crex_class_entry *class_entry_IteratorAggregate, crex_class_entry *class_entry_ArrayAccess, crex_class_entry *class_entry_Serializable, crex_class_entry *class_entry_Countable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ArrayObject", class_ArrayObject_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 4, class_entry_IteratorAggregate, class_entry_ArrayAccess, class_entry_Serializable, class_entry_Countable);

	zval const_STD_PROP_LIST_value;
	ZVAL_LONG(&const_STD_PROP_LIST_value, SPL_ARRAY_STD_PROP_LIST);
	crex_string *const_STD_PROP_LIST_name = crex_string_init_interned("STD_PROP_LIST", sizeof("STD_PROP_LIST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_STD_PROP_LIST_name, &const_STD_PROP_LIST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_STD_PROP_LIST_name);

	zval const_ARRAY_AS_PROPS_value;
	ZVAL_LONG(&const_ARRAY_AS_PROPS_value, SPL_ARRAY_ARRAY_AS_PROPS);
	crex_string *const_ARRAY_AS_PROPS_name = crex_string_init_interned("ARRAY_AS_PROPS", sizeof("ARRAY_AS_PROPS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ARRAY_AS_PROPS_name, &const_ARRAY_AS_PROPS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ARRAY_AS_PROPS_name);

	return class_entry;
}

static crex_class_entry *register_class_ArrayIterator(crex_class_entry *class_entry_SeekableIterator, crex_class_entry *class_entry_ArrayAccess, crex_class_entry *class_entry_Serializable, crex_class_entry *class_entry_Countable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ArrayIterator", class_ArrayIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 4, class_entry_SeekableIterator, class_entry_ArrayAccess, class_entry_Serializable, class_entry_Countable);

	zval const_STD_PROP_LIST_value;
	ZVAL_LONG(&const_STD_PROP_LIST_value, SPL_ARRAY_STD_PROP_LIST);
	crex_string *const_STD_PROP_LIST_name = crex_string_init_interned("STD_PROP_LIST", sizeof("STD_PROP_LIST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_STD_PROP_LIST_name, &const_STD_PROP_LIST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_STD_PROP_LIST_name);

	zval const_ARRAY_AS_PROPS_value;
	ZVAL_LONG(&const_ARRAY_AS_PROPS_value, SPL_ARRAY_ARRAY_AS_PROPS);
	crex_string *const_ARRAY_AS_PROPS_name = crex_string_init_interned("ARRAY_AS_PROPS", sizeof("ARRAY_AS_PROPS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ARRAY_AS_PROPS_name, &const_ARRAY_AS_PROPS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ARRAY_AS_PROPS_name);

	return class_entry;
}

static crex_class_entry *register_class_RecursiveArrayIterator(crex_class_entry *class_entry_ArrayIterator, crex_class_entry *class_entry_RecursiveIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "RecursiveArrayIterator", class_RecursiveArrayIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_ArrayIterator);
	crex_class_implements(class_entry, 1, class_entry_RecursiveIterator);

	zval const_CHILD_ARRAYS_ONLY_value;
	ZVAL_LONG(&const_CHILD_ARRAYS_ONLY_value, SPL_ARRAY_CHILD_ARRAYS_ONLY);
	crex_string *const_CHILD_ARRAYS_ONLY_name = crex_string_init_interned("CHILD_ARRAYS_ONLY", sizeof("CHILD_ARRAYS_ONLY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHILD_ARRAYS_ONLY_name, &const_CHILD_ARRAYS_ONLY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHILD_ARRAYS_ONLY_name);

	return class_entry;
}
