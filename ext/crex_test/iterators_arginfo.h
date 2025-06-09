/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: f9558686a7393ddd4ba3302e811f70d4496317ee */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_CrexTest_Iterators_TraversableTest___main, 0, 0, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_CrexTest_Iterators_TraversableTest_getIterator, 0, 0, Iterator, 0)
CREX_END_ARG_INFO()


static CREX_METHOD(CrexTest_Iterators_TraversableTest, __main);
static CREX_METHOD(CrexTest_Iterators_TraversableTest, getIterator);


static const crex_function_entry class_CrexTest_Iterators_TraversableTest_methods[] = {
	CREX_ME(CrexTest_Iterators_TraversableTest, __main, arginfo_class_CrexTest_Iterators_TraversableTest___main, CREX_ACC_PUBLIC)
	CREX_ME(CrexTest_Iterators_TraversableTest, getIterator, arginfo_class_CrexTest_Iterators_TraversableTest_getIterator, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_CrexTest_Iterators_TraversableTest(crex_class_entry *class_entry_IteratorAggregate)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "CrexTest\\Iterators", "TraversableTest", class_CrexTest_Iterators_TraversableTest_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;
	crex_class_implements(class_entry, 1, class_entry_IteratorAggregate);

	return class_entry;
}
