/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: b88d228873f2d8e0caa583fd80c70ce2bfc51188 */

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplDoublyLinkedList_add, 0, 2, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplDoublyLinkedList_pop, 0, 0, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplDoublyLinkedList_shift arginfo_class_SplDoublyLinkedList_pop

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplDoublyLinkedList_push, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplDoublyLinkedList_unshift arginfo_class_SplDoublyLinkedList_push

#define arginfo_class_SplDoublyLinkedList_top arginfo_class_SplDoublyLinkedList_pop

#define arginfo_class_SplDoublyLinkedList_bottom arginfo_class_SplDoublyLinkedList_pop

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplDoublyLinkedList___debugInfo, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplDoublyLinkedList_count, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplDoublyLinkedList_isEmpty, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplDoublyLinkedList_setIteratorMode, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplDoublyLinkedList_getIteratorMode arginfo_class_SplDoublyLinkedList_count

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplDoublyLinkedList_offsetExists, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, index)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplDoublyLinkedList_offsetGet, 0, 1, IS_MIXED, 0)
	CREX_ARG_INFO(0, index)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplDoublyLinkedList_offsetSet, 0, 2, IS_VOID, 0)
	CREX_ARG_INFO(0, index)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplDoublyLinkedList_offsetUnset, 0, 1, IS_VOID, 0)
	CREX_ARG_INFO(0, index)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplDoublyLinkedList_rewind, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplDoublyLinkedList_current arginfo_class_SplDoublyLinkedList_pop

#define arginfo_class_SplDoublyLinkedList_key arginfo_class_SplDoublyLinkedList_count

#define arginfo_class_SplDoublyLinkedList_prev arginfo_class_SplDoublyLinkedList_rewind

#define arginfo_class_SplDoublyLinkedList_next arginfo_class_SplDoublyLinkedList_rewind

#define arginfo_class_SplDoublyLinkedList_valid arginfo_class_SplDoublyLinkedList_isEmpty

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplDoublyLinkedList_unserialize, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplDoublyLinkedList_serialize, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplDoublyLinkedList___serialize arginfo_class_SplDoublyLinkedList___debugInfo

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplDoublyLinkedList___unserialize, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplQueue_enqueue arginfo_class_SplDoublyLinkedList_push

#define arginfo_class_SplQueue_dequeue arginfo_class_SplDoublyLinkedList_pop


CREX_METHOD(SplDoublyLinkedList, add);
CREX_METHOD(SplDoublyLinkedList, pop);
CREX_METHOD(SplDoublyLinkedList, shift);
CREX_METHOD(SplDoublyLinkedList, push);
CREX_METHOD(SplDoublyLinkedList, unshift);
CREX_METHOD(SplDoublyLinkedList, top);
CREX_METHOD(SplDoublyLinkedList, bottom);
CREX_METHOD(SplDoublyLinkedList, __debugInfo);
CREX_METHOD(SplDoublyLinkedList, count);
CREX_METHOD(SplDoublyLinkedList, isEmpty);
CREX_METHOD(SplDoublyLinkedList, setIteratorMode);
CREX_METHOD(SplDoublyLinkedList, getIteratorMode);
CREX_METHOD(SplDoublyLinkedList, offsetExists);
CREX_METHOD(SplDoublyLinkedList, offsetGet);
CREX_METHOD(SplDoublyLinkedList, offsetSet);
CREX_METHOD(SplDoublyLinkedList, offsetUnset);
CREX_METHOD(SplDoublyLinkedList, rewind);
CREX_METHOD(SplDoublyLinkedList, current);
CREX_METHOD(SplDoublyLinkedList, key);
CREX_METHOD(SplDoublyLinkedList, prev);
CREX_METHOD(SplDoublyLinkedList, next);
CREX_METHOD(SplDoublyLinkedList, valid);
CREX_METHOD(SplDoublyLinkedList, unserialize);
CREX_METHOD(SplDoublyLinkedList, serialize);
CREX_METHOD(SplDoublyLinkedList, __serialize);
CREX_METHOD(SplDoublyLinkedList, __unserialize);


static const crex_function_entry class_SplDoublyLinkedList_methods[] = {
	CREX_ME(SplDoublyLinkedList, add, arginfo_class_SplDoublyLinkedList_add, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, pop, arginfo_class_SplDoublyLinkedList_pop, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, shift, arginfo_class_SplDoublyLinkedList_shift, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, push, arginfo_class_SplDoublyLinkedList_push, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, unshift, arginfo_class_SplDoublyLinkedList_unshift, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, top, arginfo_class_SplDoublyLinkedList_top, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, bottom, arginfo_class_SplDoublyLinkedList_bottom, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, __debugInfo, arginfo_class_SplDoublyLinkedList___debugInfo, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, count, arginfo_class_SplDoublyLinkedList_count, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, isEmpty, arginfo_class_SplDoublyLinkedList_isEmpty, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, setIteratorMode, arginfo_class_SplDoublyLinkedList_setIteratorMode, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, getIteratorMode, arginfo_class_SplDoublyLinkedList_getIteratorMode, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, offsetExists, arginfo_class_SplDoublyLinkedList_offsetExists, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, offsetGet, arginfo_class_SplDoublyLinkedList_offsetGet, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, offsetSet, arginfo_class_SplDoublyLinkedList_offsetSet, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, offsetUnset, arginfo_class_SplDoublyLinkedList_offsetUnset, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, rewind, arginfo_class_SplDoublyLinkedList_rewind, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, current, arginfo_class_SplDoublyLinkedList_current, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, key, arginfo_class_SplDoublyLinkedList_key, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, prev, arginfo_class_SplDoublyLinkedList_prev, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, next, arginfo_class_SplDoublyLinkedList_next, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, valid, arginfo_class_SplDoublyLinkedList_valid, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, unserialize, arginfo_class_SplDoublyLinkedList_unserialize, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, serialize, arginfo_class_SplDoublyLinkedList_serialize, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, __serialize, arginfo_class_SplDoublyLinkedList___serialize, CREX_ACC_PUBLIC)
	CREX_ME(SplDoublyLinkedList, __unserialize, arginfo_class_SplDoublyLinkedList___unserialize, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_SplQueue_methods[] = {
	CREX_MALIAS(SplDoublyLinkedList, enqueue, push, arginfo_class_SplQueue_enqueue, CREX_ACC_PUBLIC)
	CREX_MALIAS(SplDoublyLinkedList, dequeue, shift, arginfo_class_SplQueue_dequeue, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_SplStack_methods[] = {
	CREX_FE_END
};

static crex_class_entry *register_class_SplDoublyLinkedList(crex_class_entry *class_entry_Iterator, crex_class_entry *class_entry_Countable, crex_class_entry *class_entry_ArrayAccess, crex_class_entry *class_entry_Serializable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SplDoublyLinkedList", class_SplDoublyLinkedList_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 4, class_entry_Iterator, class_entry_Countable, class_entry_ArrayAccess, class_entry_Serializable);

	zval const_IT_MODE_LIFO_value;
	ZVAL_LONG(&const_IT_MODE_LIFO_value, SPL_DLLIST_IT_LIFO);
	crex_string *const_IT_MODE_LIFO_name = crex_string_init_interned("IT_MODE_LIFO", sizeof("IT_MODE_LIFO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_IT_MODE_LIFO_name, &const_IT_MODE_LIFO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_IT_MODE_LIFO_name);

	zval const_IT_MODE_FIFO_value;
	ZVAL_LONG(&const_IT_MODE_FIFO_value, SPL_DLLIST_IT_FIFO);
	crex_string *const_IT_MODE_FIFO_name = crex_string_init_interned("IT_MODE_FIFO", sizeof("IT_MODE_FIFO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_IT_MODE_FIFO_name, &const_IT_MODE_FIFO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_IT_MODE_FIFO_name);

	zval const_IT_MODE_DELETE_value;
	ZVAL_LONG(&const_IT_MODE_DELETE_value, SPL_DLLIST_IT_DELETE);
	crex_string *const_IT_MODE_DELETE_name = crex_string_init_interned("IT_MODE_DELETE", sizeof("IT_MODE_DELETE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_IT_MODE_DELETE_name, &const_IT_MODE_DELETE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_IT_MODE_DELETE_name);

	zval const_IT_MODE_KEEP_value;
	ZVAL_LONG(&const_IT_MODE_KEEP_value, SPL_DLLIST_IT_KEEP);
	crex_string *const_IT_MODE_KEEP_name = crex_string_init_interned("IT_MODE_KEEP", sizeof("IT_MODE_KEEP") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_IT_MODE_KEEP_name, &const_IT_MODE_KEEP_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_IT_MODE_KEEP_name);

	return class_entry;
}

static crex_class_entry *register_class_SplQueue(crex_class_entry *class_entry_SplDoublyLinkedList)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SplQueue", class_SplQueue_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_SplDoublyLinkedList);

	return class_entry;
}

static crex_class_entry *register_class_SplStack(crex_class_entry *class_entry_SplDoublyLinkedList)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SplStack", class_SplStack_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_SplDoublyLinkedList);

	return class_entry;
}
