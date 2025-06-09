/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 8cd7626122b050585503ccebe370a61781ff83f2 */

CREX_BEGIN_ARG_INFO_EX(arginfo_class__CrexTestFiber___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class__CrexTestFiber_start, 0, 0, IS_MIXED, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class__CrexTestFiber_resume, 0, 0, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_MIXED, 0, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class__CrexTestFiber_pipeTo, 0, 1, _CrexTestFiber, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()

#define arginfo_class__CrexTestFiber_suspend arginfo_class__CrexTestFiber_resume


static CREX_METHOD(_CrexTestFiber, __main);
static CREX_METHOD(_CrexTestFiber, start);
static CREX_METHOD(_CrexTestFiber, resume);
static CREX_METHOD(_CrexTestFiber, pipeTo);
static CREX_METHOD(_CrexTestFiber, suspend);


static const crex_function_entry class__CrexTestFiber_methods[] = {
	CREX_ME(_CrexTestFiber, __main, arginfo_class__CrexTestFiber___main, CREX_ACC_PUBLIC)
	CREX_ME(_CrexTestFiber, start, arginfo_class__CrexTestFiber_start, CREX_ACC_PUBLIC)
	CREX_ME(_CrexTestFiber, resume, arginfo_class__CrexTestFiber_resume, CREX_ACC_PUBLIC)
	CREX_ME(_CrexTestFiber, pipeTo, arginfo_class__CrexTestFiber_pipeTo, CREX_ACC_PUBLIC)
	CREX_ME(_CrexTestFiber, suspend, arginfo_class__CrexTestFiber_suspend, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_FE_END
};

static crex_class_entry *register_class__CrexTestFiber(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "_CrexTestFiber", class__CrexTestFiber_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	return class_entry;
}
