/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 547ddbc21e9aa853b491cb17e902bbbb9cc2df00 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dl_test_test1, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_dl_test_test2, 0, 0, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, str, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()


CREX_FUNCTION(dl_test_test1);
CREX_FUNCTION(dl_test_test2);


static const crex_function_entry ext_functions[] = {
	CREX_FE(dl_test_test1, arginfo_dl_test_test1)
	CREX_FE(dl_test_test2, arginfo_dl_test_test2)
	CREX_FE_END
};
