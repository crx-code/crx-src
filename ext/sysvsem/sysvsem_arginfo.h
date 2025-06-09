/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 946ea9d0d2156ced1bac460d7d5fc3420e1934bb */

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_sem_get, 0, 1, SysvSemaphore, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, key, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, max_acquire, IS_LONG, 0, "1")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, permissions, IS_LONG, 0, "0666")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, auto_release, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sem_acquire, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, semaphore, SysvSemaphore, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, non_blocking, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sem_release, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, semaphore, SysvSemaphore, 0)
CREX_END_ARG_INFO()

#define arginfo_sem_remove arginfo_sem_release


CREX_FUNCTION(sem_get);
CREX_FUNCTION(sem_acquire);
CREX_FUNCTION(sem_release);
CREX_FUNCTION(sem_remove);


static const crex_function_entry ext_functions[] = {
	CREX_FE(sem_get, arginfo_sem_get)
	CREX_FE(sem_acquire, arginfo_sem_acquire)
	CREX_FE(sem_release, arginfo_sem_release)
	CREX_FE(sem_remove, arginfo_sem_remove)
	CREX_FE_END
};


static const crex_function_entry class_SysvSemaphore_methods[] = {
	CREX_FE_END
};

static crex_class_entry *register_class_SysvSemaphore(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SysvSemaphore", class_SysvSemaphore_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}
