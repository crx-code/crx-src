/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: d14d30fb232f08da37ba0df0b9186eb8bac5e1a4 */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_COMPersistHelper___main, 0, 0, 0)
	CREX_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, variant, variant, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_COMPersistHelper_GetCurFileName, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_COMPersistHelper_SaveToFile, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, remember, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_COMPersistHelper_LoadFromFile, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_COMPersistHelper_GetMaxStreamSize, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_COMPersistHelper_InitNew, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_COMPersistHelper_LoadFromStream, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, stream)
CREX_END_ARG_INFO()

#define arginfo_class_COMPersistHelper_SaveToStream arginfo_class_COMPersistHelper_LoadFromStream


CREX_METHOD(COMPersistHelper, __main);
CREX_METHOD(COMPersistHelper, GetCurFileName);
CREX_METHOD(COMPersistHelper, SaveToFile);
CREX_METHOD(COMPersistHelper, LoadFromFile);
CREX_METHOD(COMPersistHelper, GetMaxStreamSize);
CREX_METHOD(COMPersistHelper, InitNew);
CREX_METHOD(COMPersistHelper, LoadFromStream);
CREX_METHOD(COMPersistHelper, SaveToStream);


static const crex_function_entry class_COMPersistHelper_methods[] = {
	CREX_ME(COMPersistHelper, __main, arginfo_class_COMPersistHelper___main, CREX_ACC_PUBLIC)
	CREX_ME(COMPersistHelper, GetCurFileName, arginfo_class_COMPersistHelper_GetCurFileName, CREX_ACC_PUBLIC)
	CREX_ME(COMPersistHelper, SaveToFile, arginfo_class_COMPersistHelper_SaveToFile, CREX_ACC_PUBLIC)
	CREX_ME(COMPersistHelper, LoadFromFile, arginfo_class_COMPersistHelper_LoadFromFile, CREX_ACC_PUBLIC)
	CREX_ME(COMPersistHelper, GetMaxStreamSize, arginfo_class_COMPersistHelper_GetMaxStreamSize, CREX_ACC_PUBLIC)
	CREX_ME(COMPersistHelper, InitNew, arginfo_class_COMPersistHelper_InitNew, CREX_ACC_PUBLIC)
	CREX_ME(COMPersistHelper, LoadFromStream, arginfo_class_COMPersistHelper_LoadFromStream, CREX_ACC_PUBLIC)
	CREX_ME(COMPersistHelper, SaveToStream, arginfo_class_COMPersistHelper_SaveToStream, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_COMPersistHelper(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "COMPersistHelper", class_COMPersistHelper_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL;

	return class_entry;
}
