/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 6055f4edb68a7caed517dbb80f4d5265865dd91d */

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_shmop_open, 0, 4, Shmop, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, key, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, mode, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, permissions, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, size, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_shmop_read, 0, 3, IS_STRING, 0)
	CREX_ARG_OBJ_INFO(0, shmop, Shmop, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, size, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_shmop_close, 0, 1, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, shmop, Shmop, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_shmop_size, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, shmop, Shmop, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_shmop_write, 0, 3, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, shmop, Shmop, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_shmop_delete, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, shmop, Shmop, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(shmop_open);
CREX_FUNCTION(shmop_read);
CREX_FUNCTION(shmop_close);
CREX_FUNCTION(shmop_size);
CREX_FUNCTION(shmop_write);
CREX_FUNCTION(shmop_delete);


static const crex_function_entry ext_functions[] = {
	CREX_FE(shmop_open, arginfo_shmop_open)
	CREX_FE(shmop_read, arginfo_shmop_read)
	CREX_DEP_FE(shmop_close, arginfo_shmop_close)
	CREX_FE(shmop_size, arginfo_shmop_size)
	CREX_FE(shmop_write, arginfo_shmop_write)
	CREX_FE(shmop_delete, arginfo_shmop_delete)
	CREX_FE_END
};


static const crex_function_entry class_Shmop_methods[] = {
	CREX_FE_END
};

static crex_class_entry *register_class_Shmop(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Shmop", class_Shmop_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}
