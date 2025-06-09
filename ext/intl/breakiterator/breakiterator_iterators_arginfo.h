/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 2b201b4c2f4fb706484085c9fff483d66a7285ea */

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_IntlPartsIterator_getBreakIterator, 0, 0, IntlBreakIterator, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_IntlPartsIterator_getRuleStatus, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()


CREX_METHOD(IntlPartsIterator, getBreakIterator);
CREX_METHOD(IntlPartsIterator, getRuleStatus);


static const crex_function_entry class_IntlPartsIterator_methods[] = {
	CREX_ME(IntlPartsIterator, getBreakIterator, arginfo_class_IntlPartsIterator_getBreakIterator, CREX_ACC_PUBLIC)
	CREX_ME(IntlPartsIterator, getRuleStatus, arginfo_class_IntlPartsIterator_getRuleStatus, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_IntlPartsIterator(crex_class_entry *class_entry_IntlIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "IntlPartsIterator", class_IntlPartsIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_IntlIterator);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;

	zval const_KEY_SEQUENTIAL_value;
	ZVAL_LONG(&const_KEY_SEQUENTIAL_value, PARTS_ITERATOR_KEY_SEQUENTIAL);
	crex_string *const_KEY_SEQUENTIAL_name = crex_string_init_interned("KEY_SEQUENTIAL", sizeof("KEY_SEQUENTIAL") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_KEY_SEQUENTIAL_name, &const_KEY_SEQUENTIAL_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_KEY_SEQUENTIAL_name);

	zval const_KEY_LEFT_value;
	ZVAL_LONG(&const_KEY_LEFT_value, PARTS_ITERATOR_KEY_LEFT);
	crex_string *const_KEY_LEFT_name = crex_string_init_interned("KEY_LEFT", sizeof("KEY_LEFT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_KEY_LEFT_name, &const_KEY_LEFT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_KEY_LEFT_name);

	zval const_KEY_RIGHT_value;
	ZVAL_LONG(&const_KEY_RIGHT_value, PARTS_ITERATOR_KEY_RIGHT);
	crex_string *const_KEY_RIGHT_name = crex_string_init_interned("KEY_RIGHT", sizeof("KEY_RIGHT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_KEY_RIGHT_name, &const_KEY_RIGHT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_KEY_RIGHT_name);

	return class_entry;
}
