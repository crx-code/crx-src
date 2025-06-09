/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 93677b78d9aaa4d6dbb5d1dcf3e79a8418add5c0 */

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_shm_attach, 0, 1, SysvSharedMemory, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, key, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, size, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, permissions, IS_LONG, 0, "0666")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_shm_detach, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, shm, SysvSharedMemory, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_shm_has_var, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, shm, SysvSharedMemory, 0)
	CREX_ARG_TYPE_INFO(0, key, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_shm_remove arginfo_shm_detach

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_shm_put_var, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, shm, SysvSharedMemory, 0)
	CREX_ARG_TYPE_INFO(0, key, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_shm_get_var, 0, 2, IS_MIXED, 0)
	CREX_ARG_OBJ_INFO(0, shm, SysvSharedMemory, 0)
	CREX_ARG_TYPE_INFO(0, key, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_shm_remove_var arginfo_shm_has_var


CREX_FUNCTION(shm_attach);
CREX_FUNCTION(shm_detach);
CREX_FUNCTION(shm_has_var);
CREX_FUNCTION(shm_remove);
CREX_FUNCTION(shm_put_var);
CREX_FUNCTION(shm_get_var);
CREX_FUNCTION(shm_remove_var);


static const crex_function_entry ext_functions[] = {
	CREX_FE(shm_attach, arginfo_shm_attach)
	CREX_FE(shm_detach, arginfo_shm_detach)
	CREX_FE(shm_has_var, arginfo_shm_has_var)
	CREX_FE(shm_remove, arginfo_shm_remove)
	CREX_FE(shm_put_var, arginfo_shm_put_var)
	CREX_FE(shm_get_var, arginfo_shm_get_var)
	CREX_FE(shm_remove_var, arginfo_shm_remove_var)
	CREX_FE_END
};


static const crex_function_entry class_SysvSharedMemory_methods[] = {
	CREX_FE_END
};

static crex_class_entry *register_class_SysvSharedMemory(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SysvSharedMemory", class_SysvSharedMemory_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}
