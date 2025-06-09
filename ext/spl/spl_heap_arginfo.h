/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 4045035ec5bee0f951fa31df75c3f42c31bd8be2 */

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplPriorityQueue_compare, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, priority1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, priority2, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_SplPriorityQueue_insert, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, priority, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplPriorityQueue_setExtractFlags, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplPriorityQueue_top, 0, 0, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplPriorityQueue_extract arginfo_class_SplPriorityQueue_top

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplPriorityQueue_count, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplPriorityQueue_isEmpty, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplPriorityQueue_rewind, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplPriorityQueue_current arginfo_class_SplPriorityQueue_top

#define arginfo_class_SplPriorityQueue_key arginfo_class_SplPriorityQueue_count

#define arginfo_class_SplPriorityQueue_next arginfo_class_SplPriorityQueue_rewind

#define arginfo_class_SplPriorityQueue_valid arginfo_class_SplPriorityQueue_isEmpty

CREX_BEGIN_ARG_INFO_EX(arginfo_class_SplPriorityQueue_recoverFromCorruption, 0, 0, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplPriorityQueue_isCorrupted arginfo_class_SplPriorityQueue_isEmpty

#define arginfo_class_SplPriorityQueue_getExtractFlags arginfo_class_SplPriorityQueue_count

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplPriorityQueue___debugInfo, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplHeap_extract arginfo_class_SplPriorityQueue_top

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplHeap_insert, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplHeap_top arginfo_class_SplPriorityQueue_top

#define arginfo_class_SplHeap_count arginfo_class_SplPriorityQueue_count

#define arginfo_class_SplHeap_isEmpty arginfo_class_SplPriorityQueue_isEmpty

#define arginfo_class_SplHeap_rewind arginfo_class_SplPriorityQueue_rewind

#define arginfo_class_SplHeap_current arginfo_class_SplPriorityQueue_top

#define arginfo_class_SplHeap_key arginfo_class_SplPriorityQueue_count

#define arginfo_class_SplHeap_next arginfo_class_SplPriorityQueue_rewind

#define arginfo_class_SplHeap_valid arginfo_class_SplPriorityQueue_isEmpty

#define arginfo_class_SplHeap_recoverFromCorruption arginfo_class_SplPriorityQueue_isEmpty

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplHeap_compare, 0, 2, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, value2, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplHeap_isCorrupted arginfo_class_SplPriorityQueue_isEmpty

#define arginfo_class_SplHeap___debugInfo arginfo_class_SplPriorityQueue___debugInfo

#define arginfo_class_SplMinHeap_compare arginfo_class_SplHeap_compare

#define arginfo_class_SplMaxHeap_compare arginfo_class_SplHeap_compare


CREX_METHOD(SplPriorityQueue, compare);
CREX_METHOD(SplPriorityQueue, insert);
CREX_METHOD(SplPriorityQueue, setExtractFlags);
CREX_METHOD(SplPriorityQueue, top);
CREX_METHOD(SplPriorityQueue, extract);
CREX_METHOD(SplHeap, count);
CREX_METHOD(SplHeap, isEmpty);
CREX_METHOD(SplHeap, rewind);
CREX_METHOD(SplPriorityQueue, current);
CREX_METHOD(SplHeap, key);
CREX_METHOD(SplHeap, next);
CREX_METHOD(SplHeap, valid);
CREX_METHOD(SplHeap, recoverFromCorruption);
CREX_METHOD(SplHeap, isCorrupted);
CREX_METHOD(SplPriorityQueue, getExtractFlags);
CREX_METHOD(SplPriorityQueue, __debugInfo);
CREX_METHOD(SplHeap, extract);
CREX_METHOD(SplHeap, insert);
CREX_METHOD(SplHeap, top);
CREX_METHOD(SplHeap, current);
CREX_METHOD(SplHeap, __debugInfo);
CREX_METHOD(SplMinHeap, compare);
CREX_METHOD(SplMaxHeap, compare);


static const crex_function_entry class_SplPriorityQueue_methods[] = {
	CREX_ME(SplPriorityQueue, compare, arginfo_class_SplPriorityQueue_compare, CREX_ACC_PUBLIC)
	CREX_ME(SplPriorityQueue, insert, arginfo_class_SplPriorityQueue_insert, CREX_ACC_PUBLIC)
	CREX_ME(SplPriorityQueue, setExtractFlags, arginfo_class_SplPriorityQueue_setExtractFlags, CREX_ACC_PUBLIC)
	CREX_ME(SplPriorityQueue, top, arginfo_class_SplPriorityQueue_top, CREX_ACC_PUBLIC)
	CREX_ME(SplPriorityQueue, extract, arginfo_class_SplPriorityQueue_extract, CREX_ACC_PUBLIC)
	CREX_MALIAS(SplHeap, count, count, arginfo_class_SplPriorityQueue_count, CREX_ACC_PUBLIC)
	CREX_MALIAS(SplHeap, isEmpty, isEmpty, arginfo_class_SplPriorityQueue_isEmpty, CREX_ACC_PUBLIC)
	CREX_MALIAS(SplHeap, rewind, rewind, arginfo_class_SplPriorityQueue_rewind, CREX_ACC_PUBLIC)
	CREX_ME(SplPriorityQueue, current, arginfo_class_SplPriorityQueue_current, CREX_ACC_PUBLIC)
	CREX_MALIAS(SplHeap, key, key, arginfo_class_SplPriorityQueue_key, CREX_ACC_PUBLIC)
	CREX_MALIAS(SplHeap, next, next, arginfo_class_SplPriorityQueue_next, CREX_ACC_PUBLIC)
	CREX_MALIAS(SplHeap, valid, valid, arginfo_class_SplPriorityQueue_valid, CREX_ACC_PUBLIC)
	CREX_MALIAS(SplHeap, recoverFromCorruption, recoverFromCorruption, arginfo_class_SplPriorityQueue_recoverFromCorruption, CREX_ACC_PUBLIC)
	CREX_MALIAS(SplHeap, isCorrupted, isCorrupted, arginfo_class_SplPriorityQueue_isCorrupted, CREX_ACC_PUBLIC)
	CREX_ME(SplPriorityQueue, getExtractFlags, arginfo_class_SplPriorityQueue_getExtractFlags, CREX_ACC_PUBLIC)
	CREX_ME(SplPriorityQueue, __debugInfo, arginfo_class_SplPriorityQueue___debugInfo, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_SplHeap_methods[] = {
	CREX_ME(SplHeap, extract, arginfo_class_SplHeap_extract, CREX_ACC_PUBLIC)
	CREX_ME(SplHeap, insert, arginfo_class_SplHeap_insert, CREX_ACC_PUBLIC)
	CREX_ME(SplHeap, top, arginfo_class_SplHeap_top, CREX_ACC_PUBLIC)
	CREX_ME(SplHeap, count, arginfo_class_SplHeap_count, CREX_ACC_PUBLIC)
	CREX_ME(SplHeap, isEmpty, arginfo_class_SplHeap_isEmpty, CREX_ACC_PUBLIC)
	CREX_ME(SplHeap, rewind, arginfo_class_SplHeap_rewind, CREX_ACC_PUBLIC)
	CREX_ME(SplHeap, current, arginfo_class_SplHeap_current, CREX_ACC_PUBLIC)
	CREX_ME(SplHeap, key, arginfo_class_SplHeap_key, CREX_ACC_PUBLIC)
	CREX_ME(SplHeap, next, arginfo_class_SplHeap_next, CREX_ACC_PUBLIC)
	CREX_ME(SplHeap, valid, arginfo_class_SplHeap_valid, CREX_ACC_PUBLIC)
	CREX_ME(SplHeap, recoverFromCorruption, arginfo_class_SplHeap_recoverFromCorruption, CREX_ACC_PUBLIC)
	CREX_ABSTRACT_ME_WITH_FLAGS(SplHeap, compare, arginfo_class_SplHeap_compare, CREX_ACC_PROTECTED|CREX_ACC_ABSTRACT)
	CREX_ME(SplHeap, isCorrupted, arginfo_class_SplHeap_isCorrupted, CREX_ACC_PUBLIC)
	CREX_ME(SplHeap, __debugInfo, arginfo_class_SplHeap___debugInfo, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_SplMinHeap_methods[] = {
	CREX_ME(SplMinHeap, compare, arginfo_class_SplMinHeap_compare, CREX_ACC_PROTECTED)
	CREX_FE_END
};


static const crex_function_entry class_SplMaxHeap_methods[] = {
	CREX_ME(SplMaxHeap, compare, arginfo_class_SplMaxHeap_compare, CREX_ACC_PROTECTED)
	CREX_FE_END
};

static crex_class_entry *register_class_SplPriorityQueue(crex_class_entry *class_entry_Iterator, crex_class_entry *class_entry_Countable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SplPriorityQueue", class_SplPriorityQueue_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 2, class_entry_Iterator, class_entry_Countable);

	zval const_EXTR_BOTH_value;
	ZVAL_LONG(&const_EXTR_BOTH_value, SPL_PQUEUE_EXTR_BOTH);
	crex_string *const_EXTR_BOTH_name = crex_string_init_interned("EXTR_BOTH", sizeof("EXTR_BOTH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EXTR_BOTH_name, &const_EXTR_BOTH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EXTR_BOTH_name);

	zval const_EXTR_PRIORITY_value;
	ZVAL_LONG(&const_EXTR_PRIORITY_value, SPL_PQUEUE_EXTR_PRIORITY);
	crex_string *const_EXTR_PRIORITY_name = crex_string_init_interned("EXTR_PRIORITY", sizeof("EXTR_PRIORITY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EXTR_PRIORITY_name, &const_EXTR_PRIORITY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EXTR_PRIORITY_name);

	zval const_EXTR_DATA_value;
	ZVAL_LONG(&const_EXTR_DATA_value, SPL_PQUEUE_EXTR_DATA);
	crex_string *const_EXTR_DATA_name = crex_string_init_interned("EXTR_DATA", sizeof("EXTR_DATA") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_EXTR_DATA_name, &const_EXTR_DATA_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_EXTR_DATA_name);

	return class_entry;
}

static crex_class_entry *register_class_SplHeap(crex_class_entry *class_entry_Iterator, crex_class_entry *class_entry_Countable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SplHeap", class_SplHeap_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_ABSTRACT;
	crex_class_implements(class_entry, 2, class_entry_Iterator, class_entry_Countable);

	return class_entry;
}

static crex_class_entry *register_class_SplMinHeap(crex_class_entry *class_entry_SplHeap)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SplMinHeap", class_SplMinHeap_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_SplHeap);

	return class_entry;
}

static crex_class_entry *register_class_SplMaxHeap(crex_class_entry *class_entry_SplHeap)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SplMaxHeap", class_SplMaxHeap_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_SplHeap);

	return class_entry;
}
