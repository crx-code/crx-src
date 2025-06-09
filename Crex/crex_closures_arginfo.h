/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: e3b480674671a698814db282c5ea34d438fe519d */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Closure___main, 0, 0, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Closure_bind, 0, 2, Closure, 1)
	CREX_ARG_OBJ_INFO(0, closure, Closure, 0)
	CREX_ARG_TYPE_INFO(0, newThis, IS_OBJECT, 1)
	CREX_ARG_TYPE_MASK(0, newScope, MAY_BE_OBJECT|MAY_BE_STRING|MAY_BE_NULL, "\"static\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Closure_bindTo, 0, 1, Closure, 1)
	CREX_ARG_TYPE_INFO(0, newThis, IS_OBJECT, 1)
	CREX_ARG_TYPE_MASK(0, newScope, MAY_BE_OBJECT|MAY_BE_STRING|MAY_BE_NULL, "\"static\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Closure_call, 0, 1, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO(0, newThis, IS_OBJECT, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Closure_fromCallable, 0, 1, Closure, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()


CREX_METHOD(Closure, __main);
CREX_METHOD(Closure, bind);
CREX_METHOD(Closure, bindTo);
CREX_METHOD(Closure, call);
CREX_METHOD(Closure, fromCallable);


static const crex_function_entry class_Closure_methods[] = {
	CREX_ME(Closure, __main, arginfo_class_Closure___main, CREX_ACC_PRIVATE)
	CREX_ME(Closure, bind, arginfo_class_Closure_bind, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(Closure, bindTo, arginfo_class_Closure_bindTo, CREX_ACC_PUBLIC)
	CREX_ME(Closure, call, arginfo_class_Closure_call, CREX_ACC_PUBLIC)
	CREX_ME(Closure, fromCallable, arginfo_class_Closure_fromCallable, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_FE_END
};

static crex_class_entry *register_class_Closure(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Closure", class_Closure_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}
