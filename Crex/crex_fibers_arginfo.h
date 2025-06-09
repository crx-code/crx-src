/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: e82bbc8e81fe98873a9a5697a4b38e63a24379da */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Fiber___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Fiber_start, 0, 0, IS_MIXED, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Fiber_resume, 0, 0, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_MIXED, 0, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Fiber_throw, 0, 1, IS_MIXED, 0)
	CREX_ARG_OBJ_INFO(0, exception, Throwable, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Fiber_isStarted, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Fiber_isSuspended arginfo_class_Fiber_isStarted

#define arginfo_class_Fiber_isRunning arginfo_class_Fiber_isStarted

#define arginfo_class_Fiber_isTerminated arginfo_class_Fiber_isStarted

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Fiber_getReturn, 0, 0, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Fiber_getCurrent, 0, 0, Fiber, 1)
CREX_END_ARG_INFO()

#define arginfo_class_Fiber_suspend arginfo_class_Fiber_resume

CREX_BEGIN_ARG_INFO_EX(arginfo_class_FiberError___main, 0, 0, 0)
CREX_END_ARG_INFO()


CREX_METHOD(Fiber, __main);
CREX_METHOD(Fiber, start);
CREX_METHOD(Fiber, resume);
CREX_METHOD(Fiber, throw);
CREX_METHOD(Fiber, isStarted);
CREX_METHOD(Fiber, isSuspended);
CREX_METHOD(Fiber, isRunning);
CREX_METHOD(Fiber, isTerminated);
CREX_METHOD(Fiber, getReturn);
CREX_METHOD(Fiber, getCurrent);
CREX_METHOD(Fiber, suspend);
CREX_METHOD(FiberError, __main);


static const crex_function_entry class_Fiber_methods[] = {
	CREX_ME(Fiber, __main, arginfo_class_Fiber___main, CREX_ACC_PUBLIC)
	CREX_ME(Fiber, start, arginfo_class_Fiber_start, CREX_ACC_PUBLIC)
	CREX_ME(Fiber, resume, arginfo_class_Fiber_resume, CREX_ACC_PUBLIC)
	CREX_ME(Fiber, throw, arginfo_class_Fiber_throw, CREX_ACC_PUBLIC)
	CREX_ME(Fiber, isStarted, arginfo_class_Fiber_isStarted, CREX_ACC_PUBLIC)
	CREX_ME(Fiber, isSuspended, arginfo_class_Fiber_isSuspended, CREX_ACC_PUBLIC)
	CREX_ME(Fiber, isRunning, arginfo_class_Fiber_isRunning, CREX_ACC_PUBLIC)
	CREX_ME(Fiber, isTerminated, arginfo_class_Fiber_isTerminated, CREX_ACC_PUBLIC)
	CREX_ME(Fiber, getReturn, arginfo_class_Fiber_getReturn, CREX_ACC_PUBLIC)
	CREX_ME(Fiber, getCurrent, arginfo_class_Fiber_getCurrent, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_ME(Fiber, suspend, arginfo_class_Fiber_suspend, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
	CREX_FE_END
};


static const crex_function_entry class_FiberError_methods[] = {
	CREX_ME(FiberError, __main, arginfo_class_FiberError___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_Fiber(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Fiber", class_Fiber_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}

static crex_class_entry *register_class_FiberError(crex_class_entry *class_entry_Error)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "FiberError", class_FiberError_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Error);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	return class_entry;
}
