/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: e2d66473e8e2436652b78abd9c237e1a50c3d0dc */

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObserver_update, 0, 1, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, subject, SplSubject, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplSubject_attach, 0, 1, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, observer, SplObserver, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplSubject_detach arginfo_class_SplSubject_attach

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplSubject_notify, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_attach, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, object, IS_OBJECT, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, info, IS_MIXED, 0, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_detach, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, object, IS_OBJECT, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_contains, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, object, IS_OBJECT, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_addAll, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, storage, SplObjectStorage, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplObjectStorage_removeAll arginfo_class_SplObjectStorage_addAll

#define arginfo_class_SplObjectStorage_removeAllExcept arginfo_class_SplObjectStorage_addAll

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_getInfo, 0, 0, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_setInfo, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, info, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_count, 0, 0, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "COUNT_NORMAL")
CREX_END_ARG_INFO()

#define arginfo_class_SplObjectStorage_rewind arginfo_class_SplSubject_notify

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_valid, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_key, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_current, 0, 0, IS_OBJECT, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplObjectStorage_next arginfo_class_SplSubject_notify

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_unserialize, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_serialize, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_offsetExists, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, object)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_offsetGet, 0, 1, IS_MIXED, 0)
	CREX_ARG_INFO(0, object)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_offsetSet, 0, 1, IS_VOID, 0)
	CREX_ARG_INFO(0, object)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, info, IS_MIXED, 0, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_offsetUnset, 0, 1, IS_VOID, 0)
	CREX_ARG_INFO(0, object)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage_getHash, 0, 1, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, object, IS_OBJECT, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage___serialize, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplObjectStorage___unserialize, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplObjectStorage___debugInfo arginfo_class_SplObjectStorage___serialize

CREX_BEGIN_ARG_INFO_EX(arginfo_class_MultipleIterator___main, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "MultipleIterator::MIT_NEED_ALL | MultipleIterator::MIT_KEYS_NUMERIC")
CREX_END_ARG_INFO()

#define arginfo_class_MultipleIterator_getFlags arginfo_class_SplObjectStorage_key

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_MultipleIterator_setFlags, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_MultipleIterator_attachIterator, 0, 1, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, iterator, Iterator, 0)
	CREX_ARG_TYPE_MASK(0, info, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_NULL, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_MultipleIterator_detachIterator, 0, 1, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, iterator, Iterator, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_MultipleIterator_containsIterator, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, iterator, Iterator, 0)
CREX_END_ARG_INFO()

#define arginfo_class_MultipleIterator_countIterators arginfo_class_SplObjectStorage_key

#define arginfo_class_MultipleIterator_rewind arginfo_class_SplSubject_notify

#define arginfo_class_MultipleIterator_valid arginfo_class_SplObjectStorage_valid

#define arginfo_class_MultipleIterator_key arginfo_class_SplObjectStorage___serialize

#define arginfo_class_MultipleIterator_current arginfo_class_SplObjectStorage___serialize

#define arginfo_class_MultipleIterator_next arginfo_class_SplSubject_notify

#define arginfo_class_MultipleIterator___debugInfo arginfo_class_SplObjectStorage___serialize


CREX_METHOD(SplObjectStorage, attach);
CREX_METHOD(SplObjectStorage, detach);
CREX_METHOD(SplObjectStorage, contains);
CREX_METHOD(SplObjectStorage, addAll);
CREX_METHOD(SplObjectStorage, removeAll);
CREX_METHOD(SplObjectStorage, removeAllExcept);
CREX_METHOD(SplObjectStorage, getInfo);
CREX_METHOD(SplObjectStorage, setInfo);
CREX_METHOD(SplObjectStorage, count);
CREX_METHOD(SplObjectStorage, rewind);
CREX_METHOD(SplObjectStorage, valid);
CREX_METHOD(SplObjectStorage, key);
CREX_METHOD(SplObjectStorage, current);
CREX_METHOD(SplObjectStorage, next);
CREX_METHOD(SplObjectStorage, unserialize);
CREX_METHOD(SplObjectStorage, serialize);
CREX_METHOD(SplObjectStorage, offsetGet);
CREX_METHOD(SplObjectStorage, getHash);
CREX_METHOD(SplObjectStorage, __serialize);
CREX_METHOD(SplObjectStorage, __unserialize);
CREX_METHOD(SplObjectStorage, __debugInfo);
CREX_METHOD(MultipleIterator, __main);
CREX_METHOD(MultipleIterator, getFlags);
CREX_METHOD(MultipleIterator, setFlags);
CREX_METHOD(MultipleIterator, attachIterator);
CREX_METHOD(MultipleIterator, detachIterator);
CREX_METHOD(MultipleIterator, containsIterator);
CREX_METHOD(MultipleIterator, countIterators);
CREX_METHOD(MultipleIterator, rewind);
CREX_METHOD(MultipleIterator, valid);
CREX_METHOD(MultipleIterator, key);
CREX_METHOD(MultipleIterator, current);
CREX_METHOD(MultipleIterator, next);


static const crex_function_entry class_SplObserver_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(SplObserver, update, arginfo_class_SplObserver_update, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_SplSubject_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(SplSubject, attach, arginfo_class_SplSubject_attach, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(SplSubject, detach, arginfo_class_SplSubject_detach, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(SplSubject, notify, arginfo_class_SplSubject_notify, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_SplObjectStorage_methods[] = {
	CREX_ME(SplObjectStorage, attach, arginfo_class_SplObjectStorage_attach, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, detach, arginfo_class_SplObjectStorage_detach, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, contains, arginfo_class_SplObjectStorage_contains, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, addAll, arginfo_class_SplObjectStorage_addAll, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, removeAll, arginfo_class_SplObjectStorage_removeAll, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, removeAllExcept, arginfo_class_SplObjectStorage_removeAllExcept, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, getInfo, arginfo_class_SplObjectStorage_getInfo, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, setInfo, arginfo_class_SplObjectStorage_setInfo, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, count, arginfo_class_SplObjectStorage_count, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, rewind, arginfo_class_SplObjectStorage_rewind, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, valid, arginfo_class_SplObjectStorage_valid, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, key, arginfo_class_SplObjectStorage_key, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, current, arginfo_class_SplObjectStorage_current, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, next, arginfo_class_SplObjectStorage_next, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, unserialize, arginfo_class_SplObjectStorage_unserialize, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, serialize, arginfo_class_SplObjectStorage_serialize, CREX_ACC_PUBLIC)
	CREX_MALIAS(SplObjectStorage, offsetExists, contains, arginfo_class_SplObjectStorage_offsetExists, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, offsetGet, arginfo_class_SplObjectStorage_offsetGet, CREX_ACC_PUBLIC)
	CREX_MALIAS(SplObjectStorage, offsetSet, attach, arginfo_class_SplObjectStorage_offsetSet, CREX_ACC_PUBLIC)
	CREX_MALIAS(SplObjectStorage, offsetUnset, detach, arginfo_class_SplObjectStorage_offsetUnset, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, getHash, arginfo_class_SplObjectStorage_getHash, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, __serialize, arginfo_class_SplObjectStorage___serialize, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, __unserialize, arginfo_class_SplObjectStorage___unserialize, CREX_ACC_PUBLIC)
	CREX_ME(SplObjectStorage, __debugInfo, arginfo_class_SplObjectStorage___debugInfo, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_MultipleIterator_methods[] = {
	CREX_ME(MultipleIterator, __main, arginfo_class_MultipleIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(MultipleIterator, getFlags, arginfo_class_MultipleIterator_getFlags, CREX_ACC_PUBLIC)
	CREX_ME(MultipleIterator, setFlags, arginfo_class_MultipleIterator_setFlags, CREX_ACC_PUBLIC)
	CREX_ME(MultipleIterator, attachIterator, arginfo_class_MultipleIterator_attachIterator, CREX_ACC_PUBLIC)
	CREX_ME(MultipleIterator, detachIterator, arginfo_class_MultipleIterator_detachIterator, CREX_ACC_PUBLIC)
	CREX_ME(MultipleIterator, containsIterator, arginfo_class_MultipleIterator_containsIterator, CREX_ACC_PUBLIC)
	CREX_ME(MultipleIterator, countIterators, arginfo_class_MultipleIterator_countIterators, CREX_ACC_PUBLIC)
	CREX_ME(MultipleIterator, rewind, arginfo_class_MultipleIterator_rewind, CREX_ACC_PUBLIC)
	CREX_ME(MultipleIterator, valid, arginfo_class_MultipleIterator_valid, CREX_ACC_PUBLIC)
	CREX_ME(MultipleIterator, key, arginfo_class_MultipleIterator_key, CREX_ACC_PUBLIC)
	CREX_ME(MultipleIterator, current, arginfo_class_MultipleIterator_current, CREX_ACC_PUBLIC)
	CREX_ME(MultipleIterator, next, arginfo_class_MultipleIterator_next, CREX_ACC_PUBLIC)
	CREX_MALIAS(SplObjectStorage, __debugInfo, __debugInfo, arginfo_class_MultipleIterator___debugInfo, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_SplObserver(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SplObserver", class_SplObserver_methods);
	class_entry = crex_register_internal_interface(&ce);

	return class_entry;
}

static crex_class_entry *register_class_SplSubject(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SplSubject", class_SplSubject_methods);
	class_entry = crex_register_internal_interface(&ce);

	return class_entry;
}

static crex_class_entry *register_class_SplObjectStorage(crex_class_entry *class_entry_Countable, crex_class_entry *class_entry_Iterator, crex_class_entry *class_entry_Serializable, crex_class_entry *class_entry_ArrayAccess)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SplObjectStorage", class_SplObjectStorage_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 4, class_entry_Countable, class_entry_Iterator, class_entry_Serializable, class_entry_ArrayAccess);

	return class_entry;
}

static crex_class_entry *register_class_MultipleIterator(crex_class_entry *class_entry_Iterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "MultipleIterator", class_MultipleIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 1, class_entry_Iterator);

	zval const_MIT_NEED_ANY_value;
	ZVAL_LONG(&const_MIT_NEED_ANY_value, MIT_NEED_ANY);
	crex_string *const_MIT_NEED_ANY_name = crex_string_init_interned("MIT_NEED_ANY", sizeof("MIT_NEED_ANY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MIT_NEED_ANY_name, &const_MIT_NEED_ANY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MIT_NEED_ANY_name);

	zval const_MIT_NEED_ALL_value;
	ZVAL_LONG(&const_MIT_NEED_ALL_value, MIT_NEED_ALL);
	crex_string *const_MIT_NEED_ALL_name = crex_string_init_interned("MIT_NEED_ALL", sizeof("MIT_NEED_ALL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MIT_NEED_ALL_name, &const_MIT_NEED_ALL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MIT_NEED_ALL_name);

	zval const_MIT_KEYS_NUMERIC_value;
	ZVAL_LONG(&const_MIT_KEYS_NUMERIC_value, MIT_KEYS_NUMERIC);
	crex_string *const_MIT_KEYS_NUMERIC_name = crex_string_init_interned("MIT_KEYS_NUMERIC", sizeof("MIT_KEYS_NUMERIC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MIT_KEYS_NUMERIC_name, &const_MIT_KEYS_NUMERIC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MIT_KEYS_NUMERIC_name);

	zval const_MIT_KEYS_ASSOC_value;
	ZVAL_LONG(&const_MIT_KEYS_ASSOC_value, MIT_KEYS_ASSOC);
	crex_string *const_MIT_KEYS_ASSOC_name = crex_string_init_interned("MIT_KEYS_ASSOC", sizeof("MIT_KEYS_ASSOC") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MIT_KEYS_ASSOC_name, &const_MIT_KEYS_ASSOC_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MIT_KEYS_ASSOC_name);

	return class_entry;
}
