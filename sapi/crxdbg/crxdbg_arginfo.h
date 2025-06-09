/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 08e29f02953f23bfce6ce04f435227b4e5e61545 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crxdbg_break_next, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crxdbg_break_file, 0, 2, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, file, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, line, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crxdbg_break_method, 0, 2, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, class, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, method, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crxdbg_break_function, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, function, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crxdbg_color, 0, 2, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, element, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, color, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crxdbg_prompt, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, string, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_crxdbg_exec, 0, 1, MAY_BE_STRING|MAY_BE_BOOL)
	CREX_ARG_TYPE_INFO(0, context, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_crxdbg_clear arginfo_crxdbg_break_next

#define arginfo_crxdbg_start_oplog arginfo_crxdbg_break_next

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crxdbg_end_oplog, 0, 0, IS_ARRAY, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_crxdbg_get_executable, 0, 0, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()


CREX_FUNCTION(crxdbg_break_next);
CREX_FUNCTION(crxdbg_break_file);
CREX_FUNCTION(crxdbg_break_method);
CREX_FUNCTION(crxdbg_break_function);
CREX_FUNCTION(crxdbg_color);
CREX_FUNCTION(crxdbg_prompt);
CREX_FUNCTION(crxdbg_exec);
CREX_FUNCTION(crxdbg_clear);
CREX_FUNCTION(crxdbg_start_oplog);
CREX_FUNCTION(crxdbg_end_oplog);
CREX_FUNCTION(crxdbg_get_executable);


static const crex_function_entry ext_functions[] = {
	CREX_FE(crxdbg_break_next, arginfo_crxdbg_break_next)
	CREX_FE(crxdbg_break_file, arginfo_crxdbg_break_file)
	CREX_FE(crxdbg_break_method, arginfo_crxdbg_break_method)
	CREX_FE(crxdbg_break_function, arginfo_crxdbg_break_function)
	CREX_FE(crxdbg_color, arginfo_crxdbg_color)
	CREX_FE(crxdbg_prompt, arginfo_crxdbg_prompt)
	CREX_FE(crxdbg_exec, arginfo_crxdbg_exec)
	CREX_FE(crxdbg_clear, arginfo_crxdbg_clear)
	CREX_FE(crxdbg_start_oplog, arginfo_crxdbg_start_oplog)
	CREX_FE(crxdbg_end_oplog, arginfo_crxdbg_end_oplog)
	CREX_FE(crxdbg_get_executable, arginfo_crxdbg_get_executable)
	CREX_FE_END
};

static void register_crxdbg_symbols(int module_number)
{
	REGISTER_STRING_CONSTANT("CRXDBG_VERSION", CRXDBG_VERSION, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRXDBG_COLOR_PROMPT", CRXDBG_COLOR_PROMPT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRXDBG_COLOR_NOTICE", CRXDBG_COLOR_NOTICE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRXDBG_COLOR_ERROR", CRXDBG_COLOR_ERROR, CONST_PERSISTENT);
}
