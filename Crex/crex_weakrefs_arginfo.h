/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: d91889851d9732d41e43fffddb6235d033c67534 */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_WeakReference___main, 0, 0, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_WeakReference_create, 0, 1, WeakReference, 0)
	CREX_ARG_TYPE_INFO(0, object, IS_OBJECT, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_WeakReference_get, 0, 0, IS_OBJECT, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_WeakMap_offsetGet, 0, 1, IS_MIXED, 0)
	CREX_ARG_INFO(0, object)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_WeakMap_offsetSet, 0, 2, IS_VOID, 0)
	CREX_ARG_INFO(0, object)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_WeakMap_offsetExists, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, object)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_WeakMap_offsetUnset, 0, 1, IS_VOID, 0)
	CREX_ARG_INFO(0, object)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_WeakMap_count, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_WeakMap_getIterator, 0, 0, Iterator, 0)
CREX_END_ARG_INFO()


CREX_METHOD(WeakReference, __main);
CREX_METHOD(WeakReference, create);
CREX_METHOD(WeakReference, get);
CREX_METHOD(WeakMap, offsetGet);
CREX_METHOD(WeakMap, offsetSet);
CREX_METHOD(WeakMap, offsetExists);
CREX_METHOD(WeakMap, offsetUnset);
CREX_METHOD(WeakMap, count);
CREX_METHOD(WeakMap, getIterator);


static const crex_function_entry class_WeakReference_methods[] = {
	CREX_ME(WeakReference, __main, arginfo_class_WeakReference___main, CREX_ACC_PUBLIC)
	CREX_ME(WeakReference, create, arginfo_class_WeakReference_create, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(WeakReference, get, arginfo_class_WeakReference_get, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_WeakMap_methods[] = {
	CREX_ME(WeakMap, offsetGet, arginfo_class_WeakMap_offsetGet, CREX_ACC_PUBLIC)
	CREX_ME(WeakMap, offsetSet, arginfo_class_WeakMap_offsetSet, CREX_ACC_PUBLIC)
	CREX_ME(WeakMap, offsetExists, arginfo_class_WeakMap_offsetExists, CREX_ACC_PUBLIC)
	CREX_ME(WeakMap, offsetUnset, arginfo_class_WeakMap_offsetUnset, CREX_ACC_PUBLIC)
	CREX_ME(WeakMap, count, arginfo_class_WeakMap_count, CREX_ACC_PUBLIC)
	CREX_ME(WeakMap, getIterator, arginfo_class_WeakMap_getIterator, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_WeakReference(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "WeakReference", class_WeakReference_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}

static crex_class_entry *register_class_WeakMap(crex_class_entry *class_entry_ArrayAccess, crex_class_entry *class_entry_Countable, crex_class_entry *class_entry_IteratorAggregate)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "WeakMap", class_WeakMap_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;
	crex_class_implements(class_entry, 3, class_entry_ArrayAccess, class_entry_Countable, class_entry_IteratorAggregate);

	return class_entry;
}
