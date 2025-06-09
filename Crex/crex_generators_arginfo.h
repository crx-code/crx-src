/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 0af5e8985dd4645bf23490b8cec312f8fd1fee2e */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Generator_rewind, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Generator_valid, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Generator_current, 0, 0, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Generator_key arginfo_class_Generator_current

#define arginfo_class_Generator_next arginfo_class_Generator_rewind

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Generator_send, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Generator_throw, 0, 1, IS_MIXED, 0)
	CREX_ARG_OBJ_INFO(0, exception, Throwable, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Generator_getReturn arginfo_class_Generator_current


CREX_METHOD(Generator, rewind);
CREX_METHOD(Generator, valid);
CREX_METHOD(Generator, current);
CREX_METHOD(Generator, key);
CREX_METHOD(Generator, next);
CREX_METHOD(Generator, send);
CREX_METHOD(Generator, throw);
CREX_METHOD(Generator, getReturn);


static const crex_function_entry class_Generator_methods[] = {
	CREX_ME(Generator, rewind, arginfo_class_Generator_rewind, CREX_ACC_PUBLIC)
	CREX_ME(Generator, valid, arginfo_class_Generator_valid, CREX_ACC_PUBLIC)
	CREX_ME(Generator, current, arginfo_class_Generator_current, CREX_ACC_PUBLIC)
	CREX_ME(Generator, key, arginfo_class_Generator_key, CREX_ACC_PUBLIC)
	CREX_ME(Generator, next, arginfo_class_Generator_next, CREX_ACC_PUBLIC)
	CREX_ME(Generator, send, arginfo_class_Generator_send, CREX_ACC_PUBLIC)
	CREX_ME(Generator, throw, arginfo_class_Generator_throw, CREX_ACC_PUBLIC)
	CREX_ME(Generator, getReturn, arginfo_class_Generator_getReturn, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_ClosedGeneratorException_methods[] = {
	CREX_FE_END
};

static crex_class_entry *register_class_Generator(crex_class_entry *class_entry_Iterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Generator", class_Generator_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;
	crex_class_implements(class_entry, 1, class_entry_Iterator);

	return class_entry;
}

static crex_class_entry *register_class_ClosedGeneratorException(crex_class_entry *class_entry_Exception)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ClosedGeneratorException", class_ClosedGeneratorException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Exception);

	return class_entry;
}
