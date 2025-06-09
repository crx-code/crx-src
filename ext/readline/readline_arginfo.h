/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 64d630be9ea75d584a4a999dd4d4c6bc769f5aca */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_readline, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, prompt, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_readline_info, 0, 0, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, var_name, IS_STRING, 1, "null")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, value, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_readline_add_history, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, prompt, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_readline_clear_history, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#if defined(HAVE_HISTORY_LIST)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_readline_list_history, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_readline_read_history, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, filename, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_readline_write_history arginfo_readline_read_history

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_readline_completion_function, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()

#if HAVE_RL_CALLBACK_READ_CHAR
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_readline_callback_handler_install, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, prompt, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()
#endif

#if HAVE_RL_CALLBACK_READ_CHAR
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_readline_callback_read_char, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()
#endif

#if HAVE_RL_CALLBACK_READ_CHAR
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_readline_callback_handler_remove, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()
#endif

#if HAVE_RL_CALLBACK_READ_CHAR
#define arginfo_readline_redisplay arginfo_readline_callback_read_char
#endif

#if HAVE_RL_CALLBACK_READ_CHAR && HAVE_RL_ON_NEW_LINE
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_readline_on_new_line, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()
#endif


CREX_FUNCTION(readline);
CREX_FUNCTION(readline_info);
CREX_FUNCTION(readline_add_history);
CREX_FUNCTION(readline_clear_history);
#if defined(HAVE_HISTORY_LIST)
CREX_FUNCTION(readline_list_history);
#endif
CREX_FUNCTION(readline_read_history);
CREX_FUNCTION(readline_write_history);
CREX_FUNCTION(readline_completion_function);
#if HAVE_RL_CALLBACK_READ_CHAR
CREX_FUNCTION(readline_callback_handler_install);
#endif
#if HAVE_RL_CALLBACK_READ_CHAR
CREX_FUNCTION(readline_callback_read_char);
#endif
#if HAVE_RL_CALLBACK_READ_CHAR
CREX_FUNCTION(readline_callback_handler_remove);
#endif
#if HAVE_RL_CALLBACK_READ_CHAR
CREX_FUNCTION(readline_redisplay);
#endif
#if HAVE_RL_CALLBACK_READ_CHAR && HAVE_RL_ON_NEW_LINE
CREX_FUNCTION(readline_on_new_line);
#endif


static const crex_function_entry ext_functions[] = {
	CREX_FE(readline, arginfo_readline)
	CREX_FE(readline_info, arginfo_readline_info)
	CREX_FE(readline_add_history, arginfo_readline_add_history)
	CREX_FE(readline_clear_history, arginfo_readline_clear_history)
#if defined(HAVE_HISTORY_LIST)
	CREX_FE(readline_list_history, arginfo_readline_list_history)
#endif
	CREX_FE(readline_read_history, arginfo_readline_read_history)
	CREX_FE(readline_write_history, arginfo_readline_write_history)
	CREX_FE(readline_completion_function, arginfo_readline_completion_function)
#if HAVE_RL_CALLBACK_READ_CHAR
	CREX_FE(readline_callback_handler_install, arginfo_readline_callback_handler_install)
#endif
#if HAVE_RL_CALLBACK_READ_CHAR
	CREX_FE(readline_callback_read_char, arginfo_readline_callback_read_char)
#endif
#if HAVE_RL_CALLBACK_READ_CHAR
	CREX_FE(readline_callback_handler_remove, arginfo_readline_callback_handler_remove)
#endif
#if HAVE_RL_CALLBACK_READ_CHAR
	CREX_FE(readline_redisplay, arginfo_readline_redisplay)
#endif
#if HAVE_RL_CALLBACK_READ_CHAR && HAVE_RL_ON_NEW_LINE
	CREX_FE(readline_on_new_line, arginfo_readline_on_new_line)
#endif
	CREX_FE_END
};

static void register_readline_symbols(int module_number)
{
	REGISTER_STRING_CONSTANT("READLINE_LIB", READLINE_LIB, CONST_PERSISTENT);
}
