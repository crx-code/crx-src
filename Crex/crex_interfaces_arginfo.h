/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: a9c915c11e5989d8c7cf2d704ada09ca765670c3 */

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_IteratorAggregate_getIterator, 0, 0, Traversable, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Iterator_current, 0, 0, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Iterator_next, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Iterator_key arginfo_class_Iterator_current

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Iterator_valid, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Iterator_rewind arginfo_class_Iterator_next

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayAccess_offsetExists, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayAccess_offsetGet, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayAccess_offsetSet, 0, 2, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ArrayAccess_offsetUnset, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Serializable_serialize, 0, 0, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Serializable_unserialize, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Countable_count, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Stringable___toString, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_InternalIterator___main arginfo_class_Serializable_serialize

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_InternalIterator_current, 0, 0, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_class_InternalIterator_key arginfo_class_InternalIterator_current

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_InternalIterator_next, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_InternalIterator_valid, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_class_InternalIterator_rewind arginfo_class_InternalIterator_next


CREX_METHOD(InternalIterator, __main);
CREX_METHOD(InternalIterator, current);
CREX_METHOD(InternalIterator, key);
CREX_METHOD(InternalIterator, next);
CREX_METHOD(InternalIterator, valid);
CREX_METHOD(InternalIterator, rewind);


static const crex_function_entry class_Traversable_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_IteratorAggregate_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(IteratorAggregate, getIterator, arginfo_class_IteratorAggregate_getIterator, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_Iterator_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(Iterator, current, arginfo_class_Iterator_current, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(Iterator, next, arginfo_class_Iterator_next, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(Iterator, key, arginfo_class_Iterator_key, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(Iterator, valid, arginfo_class_Iterator_valid, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(Iterator, rewind, arginfo_class_Iterator_rewind, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_ArrayAccess_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(ArrayAccess, offsetExists, arginfo_class_ArrayAccess_offsetExists, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(ArrayAccess, offsetGet, arginfo_class_ArrayAccess_offsetGet, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(ArrayAccess, offsetSet, arginfo_class_ArrayAccess_offsetSet, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(ArrayAccess, offsetUnset, arginfo_class_ArrayAccess_offsetUnset, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_Serializable_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(Serializable, serialize, arginfo_class_Serializable_serialize, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(Serializable, unserialize, arginfo_class_Serializable_unserialize, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_Countable_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(Countable, count, arginfo_class_Countable_count, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_Stringable_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(Stringable, __toString, arginfo_class_Stringable___toString, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_InternalIterator_methods[] = {
	CREX_ME(InternalIterator, __main, arginfo_class_InternalIterator___main, CREX_ACC_PRIVATE)
	CREX_ME(InternalIterator, current, arginfo_class_InternalIterator_current, CREX_ACC_PUBLIC)
	CREX_ME(InternalIterator, key, arginfo_class_InternalIterator_key, CREX_ACC_PUBLIC)
	CREX_ME(InternalIterator, next, arginfo_class_InternalIterator_next, CREX_ACC_PUBLIC)
	CREX_ME(InternalIterator, valid, arginfo_class_InternalIterator_valid, CREX_ACC_PUBLIC)
	CREX_ME(InternalIterator, rewind, arginfo_class_InternalIterator_rewind, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_Traversable(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Traversable", class_Traversable_methods);
	class_entry = crex_register_internal_interface(&ce);

	return class_entry;
}

static crex_class_entry *register_class_IteratorAggregate(crex_class_entry *class_entry_Traversable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "IteratorAggregate", class_IteratorAggregate_methods);
	class_entry = crex_register_internal_interface(&ce);
	crex_class_implements(class_entry, 1, class_entry_Traversable);

	return class_entry;
}

static crex_class_entry *register_class_Iterator(crex_class_entry *class_entry_Traversable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Iterator", class_Iterator_methods);
	class_entry = crex_register_internal_interface(&ce);
	crex_class_implements(class_entry, 1, class_entry_Traversable);

	return class_entry;
}

static crex_class_entry *register_class_ArrayAccess(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ArrayAccess", class_ArrayAccess_methods);
	class_entry = crex_register_internal_interface(&ce);

	return class_entry;
}

static crex_class_entry *register_class_Serializable(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Serializable", class_Serializable_methods);
	class_entry = crex_register_internal_interface(&ce);

	return class_entry;
}

static crex_class_entry *register_class_Countable(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Countable", class_Countable_methods);
	class_entry = crex_register_internal_interface(&ce);

	return class_entry;
}

static crex_class_entry *register_class_Stringable(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Stringable", class_Stringable_methods);
	class_entry = crex_register_internal_interface(&ce);

	return class_entry;
}

static crex_class_entry *register_class_InternalIterator(crex_class_entry *class_entry_Iterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "InternalIterator", class_InternalIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NOT_SERIALIZABLE;
	crex_class_implements(class_entry, 1, class_entry_Iterator);

	return class_entry;
}
