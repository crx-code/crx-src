/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 54b0ffc3af871b189435266df516f7575c1b9675 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_test1, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_test2, 0, 0, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, str, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()


CREX_FUNCTION(test1);
CREX_FUNCTION(test2);


static const crex_function_entry ext_functions[] = {
	CREX_FE(test1, arginfo_test1)
	CREX_FE(test2, arginfo_test2)
	CREX_FE_END
};
