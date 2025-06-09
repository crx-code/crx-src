/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 0b508ad6499b70c92bf25960b30fefa913532a3c */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_SplFixedArray___main, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, size, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFixedArray___wakeup, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SplFixedArray___serialize, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SplFixedArray___unserialize, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFixedArray_count, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFixedArray_toArray, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_SplFixedArray_fromArray, 0, 1, SplFixedArray, 0)
	CREX_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, preserveKeys, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

#define arginfo_class_SplFixedArray_getSize arginfo_class_SplFixedArray_count

CREX_BEGIN_ARG_INFO_EX(arginfo_class_SplFixedArray_setSize, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, size, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFixedArray_offsetExists, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, index)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFixedArray_offsetGet, 0, 1, IS_MIXED, 0)
	CREX_ARG_INFO(0, index)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFixedArray_offsetSet, 0, 2, IS_VOID, 0)
	CREX_ARG_INFO(0, index)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFixedArray_offsetUnset, 0, 1, IS_VOID, 0)
	CREX_ARG_INFO(0, index)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_SplFixedArray_getIterator, 0, 0, Iterator, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplFixedArray_jsonSerialize arginfo_class_SplFixedArray___serialize


CREX_METHOD(SplFixedArray, __main);
CREX_METHOD(SplFixedArray, __wakeup);
CREX_METHOD(SplFixedArray, __serialize);
CREX_METHOD(SplFixedArray, __unserialize);
CREX_METHOD(SplFixedArray, count);
CREX_METHOD(SplFixedArray, toArray);
CREX_METHOD(SplFixedArray, fromArray);
CREX_METHOD(SplFixedArray, getSize);
CREX_METHOD(SplFixedArray, setSize);
CREX_METHOD(SplFixedArray, offsetExists);
CREX_METHOD(SplFixedArray, offsetGet);
CREX_METHOD(SplFixedArray, offsetSet);
CREX_METHOD(SplFixedArray, offsetUnset);
CREX_METHOD(SplFixedArray, getIterator);
CREX_METHOD(SplFixedArray, jsonSerialize);


static const crex_function_entry class_SplFixedArray_methods[] = {
	CREX_ME(SplFixedArray, __main, arginfo_class_SplFixedArray___main, CREX_ACC_PUBLIC)
	CREX_ME(SplFixedArray, __wakeup, arginfo_class_SplFixedArray___wakeup, CREX_ACC_PUBLIC)
	CREX_ME(SplFixedArray, __serialize, arginfo_class_SplFixedArray___serialize, CREX_ACC_PUBLIC)
	CREX_ME(SplFixedArray, __unserialize, arginfo_class_SplFixedArray___unserialize, CREX_ACC_PUBLIC)
	CREX_ME(SplFixedArray, count, arginfo_class_SplFixedArray_count, CREX_ACC_PUBLIC)
	CREX_ME(SplFixedArray, toArray, arginfo_class_SplFixedArray_toArray, CREX_ACC_PUBLIC)
	CREX_ME(SplFixedArray, fromArray, arginfo_class_SplFixedArray_fromArray, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(SplFixedArray, getSize, arginfo_class_SplFixedArray_getSize, CREX_ACC_PUBLIC)
	CREX_ME(SplFixedArray, setSize, arginfo_class_SplFixedArray_setSize, CREX_ACC_PUBLIC)
	CREX_ME(SplFixedArray, offsetExists, arginfo_class_SplFixedArray_offsetExists, CREX_ACC_PUBLIC)
	CREX_ME(SplFixedArray, offsetGet, arginfo_class_SplFixedArray_offsetGet, CREX_ACC_PUBLIC)
	CREX_ME(SplFixedArray, offsetSet, arginfo_class_SplFixedArray_offsetSet, CREX_ACC_PUBLIC)
	CREX_ME(SplFixedArray, offsetUnset, arginfo_class_SplFixedArray_offsetUnset, CREX_ACC_PUBLIC)
	CREX_ME(SplFixedArray, getIterator, arginfo_class_SplFixedArray_getIterator, CREX_ACC_PUBLIC)
	CREX_ME(SplFixedArray, jsonSerialize, arginfo_class_SplFixedArray_jsonSerialize, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_SplFixedArray(crex_class_entry *class_entry_IteratorAggregate, crex_class_entry *class_entry_ArrayAccess, crex_class_entry *class_entry_Countable, crex_class_entry *class_entry_JsonSerializable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SplFixedArray", class_SplFixedArray_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 4, class_entry_IteratorAggregate, class_entry_ArrayAccess, class_entry_Countable, class_entry_JsonSerializable);

	return class_entry;
}
