/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 6bbbdc8c4a33d1ff9984b3d81e4f5c9b76efcb14 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_session_name, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, name, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_session_module_name, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, module, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_session_save_path, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, path, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_session_id, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, id, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_session_create_id, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, prefix, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_session_regenerate_id, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, delete_old_session, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_session_decode, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_session_encode, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_session_destroy, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_session_unset arginfo_session_destroy

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_session_gc, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_session_get_cookie_params, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_session_write_close arginfo_session_destroy

#define arginfo_session_abort arginfo_session_destroy

#define arginfo_session_reset arginfo_session_destroy

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_session_status, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_session_register_shutdown, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

#define arginfo_session_commit arginfo_session_destroy

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_session_set_save_handler, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, open)
	CREX_ARG_INFO(0, close)
	CREX_ARG_TYPE_INFO(0, read, IS_CALLABLE, 0)
	CREX_ARG_TYPE_INFO(0, write, IS_CALLABLE, 0)
	CREX_ARG_TYPE_INFO(0, destroy, IS_CALLABLE, 0)
	CREX_ARG_TYPE_INFO(0, gc, IS_CALLABLE, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, create_sid, IS_CALLABLE, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, validate_sid, IS_CALLABLE, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, update_timestamp, IS_CALLABLE, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_session_cache_limiter, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_session_cache_expire, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_session_set_cookie_params, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_MASK(0, lifetime_or_options, MAY_BE_ARRAY|MAY_BE_LONG, NULL)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, path, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, domain, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, secure, _IS_BOOL, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, httponly, _IS_BOOL, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_session_start, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SessionHandlerInterface_open, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SessionHandlerInterface_close, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_SessionHandlerInterface_read, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, id, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SessionHandlerInterface_write, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, id, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SessionHandlerInterface_destroy, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, id, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_SessionHandlerInterface_gc, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, max_lifetime, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SessionIdInterface_create_sid, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SessionUpdateTimestampHandlerInterface_validateId arginfo_class_SessionHandlerInterface_destroy

#define arginfo_class_SessionUpdateTimestampHandlerInterface_updateTimestamp arginfo_class_SessionHandlerInterface_write

#define arginfo_class_SessionHandler_open arginfo_class_SessionHandlerInterface_open

#define arginfo_class_SessionHandler_close arginfo_class_SessionHandlerInterface_close

#define arginfo_class_SessionHandler_read arginfo_class_SessionHandlerInterface_read

#define arginfo_class_SessionHandler_write arginfo_class_SessionHandlerInterface_write

#define arginfo_class_SessionHandler_destroy arginfo_class_SessionHandlerInterface_destroy

#define arginfo_class_SessionHandler_gc arginfo_class_SessionHandlerInterface_gc

#define arginfo_class_SessionHandler_create_sid arginfo_class_SessionIdInterface_create_sid


CREX_FUNCTION(session_name);
CREX_FUNCTION(session_module_name);
CREX_FUNCTION(session_save_path);
CREX_FUNCTION(session_id);
CREX_FUNCTION(session_create_id);
CREX_FUNCTION(session_regenerate_id);
CREX_FUNCTION(session_decode);
CREX_FUNCTION(session_encode);
CREX_FUNCTION(session_destroy);
CREX_FUNCTION(session_unset);
CREX_FUNCTION(session_gc);
CREX_FUNCTION(session_get_cookie_params);
CREX_FUNCTION(session_write_close);
CREX_FUNCTION(session_abort);
CREX_FUNCTION(session_reset);
CREX_FUNCTION(session_status);
CREX_FUNCTION(session_register_shutdown);
CREX_FUNCTION(session_set_save_handler);
CREX_FUNCTION(session_cache_limiter);
CREX_FUNCTION(session_cache_expire);
CREX_FUNCTION(session_set_cookie_params);
CREX_FUNCTION(session_start);
CREX_METHOD(SessionHandler, open);
CREX_METHOD(SessionHandler, close);
CREX_METHOD(SessionHandler, read);
CREX_METHOD(SessionHandler, write);
CREX_METHOD(SessionHandler, destroy);
CREX_METHOD(SessionHandler, gc);
CREX_METHOD(SessionHandler, create_sid);


static const crex_function_entry ext_functions[] = {
	CREX_FE(session_name, arginfo_session_name)
	CREX_FE(session_module_name, arginfo_session_module_name)
	CREX_FE(session_save_path, arginfo_session_save_path)
	CREX_FE(session_id, arginfo_session_id)
	CREX_FE(session_create_id, arginfo_session_create_id)
	CREX_FE(session_regenerate_id, arginfo_session_regenerate_id)
	CREX_FE(session_decode, arginfo_session_decode)
	CREX_FE(session_encode, arginfo_session_encode)
	CREX_FE(session_destroy, arginfo_session_destroy)
	CREX_FE(session_unset, arginfo_session_unset)
	CREX_FE(session_gc, arginfo_session_gc)
	CREX_FE(session_get_cookie_params, arginfo_session_get_cookie_params)
	CREX_FE(session_write_close, arginfo_session_write_close)
	CREX_FE(session_abort, arginfo_session_abort)
	CREX_FE(session_reset, arginfo_session_reset)
	CREX_FE(session_status, arginfo_session_status)
	CREX_FE(session_register_shutdown, arginfo_session_register_shutdown)
	CREX_FALIAS(session_commit, session_write_close, arginfo_session_commit)
	CREX_FE(session_set_save_handler, arginfo_session_set_save_handler)
	CREX_FE(session_cache_limiter, arginfo_session_cache_limiter)
	CREX_FE(session_cache_expire, arginfo_session_cache_expire)
	CREX_FE(session_set_cookie_params, arginfo_session_set_cookie_params)
	CREX_FE(session_start, arginfo_session_start)
	CREX_FE_END
};


static const crex_function_entry class_SessionHandlerInterface_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(SessionHandlerInterface, open, arginfo_class_SessionHandlerInterface_open, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(SessionHandlerInterface, close, arginfo_class_SessionHandlerInterface_close, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(SessionHandlerInterface, read, arginfo_class_SessionHandlerInterface_read, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(SessionHandlerInterface, write, arginfo_class_SessionHandlerInterface_write, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(SessionHandlerInterface, destroy, arginfo_class_SessionHandlerInterface_destroy, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(SessionHandlerInterface, gc, arginfo_class_SessionHandlerInterface_gc, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_SessionIdInterface_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(SessionIdInterface, create_sid, arginfo_class_SessionIdInterface_create_sid, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_SessionUpdateTimestampHandlerInterface_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(SessionUpdateTimestampHandlerInterface, validateId, arginfo_class_SessionUpdateTimestampHandlerInterface_validateId, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(SessionUpdateTimestampHandlerInterface, updateTimestamp, arginfo_class_SessionUpdateTimestampHandlerInterface_updateTimestamp, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_SessionHandler_methods[] = {
	CREX_ME(SessionHandler, open, arginfo_class_SessionHandler_open, CREX_ACC_PUBLIC)
	CREX_ME(SessionHandler, close, arginfo_class_SessionHandler_close, CREX_ACC_PUBLIC)
	CREX_ME(SessionHandler, read, arginfo_class_SessionHandler_read, CREX_ACC_PUBLIC)
	CREX_ME(SessionHandler, write, arginfo_class_SessionHandler_write, CREX_ACC_PUBLIC)
	CREX_ME(SessionHandler, destroy, arginfo_class_SessionHandler_destroy, CREX_ACC_PUBLIC)
	CREX_ME(SessionHandler, gc, arginfo_class_SessionHandler_gc, CREX_ACC_PUBLIC)
	CREX_ME(SessionHandler, create_sid, arginfo_class_SessionHandler_create_sid, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static void register_session_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("CRX_SESSION_DISABLED", crx_session_disabled, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_SESSION_NONE", crx_session_none, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("CRX_SESSION_ACTIVE", crx_session_active, CONST_PERSISTENT);
}

static crex_class_entry *register_class_SessionHandlerInterface(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SessionHandlerInterface", class_SessionHandlerInterface_methods);
	class_entry = crex_register_internal_interface(&ce);

	return class_entry;
}

static crex_class_entry *register_class_SessionIdInterface(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SessionIdInterface", class_SessionIdInterface_methods);
	class_entry = crex_register_internal_interface(&ce);

	return class_entry;
}

static crex_class_entry *register_class_SessionUpdateTimestampHandlerInterface(void)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SessionUpdateTimestampHandlerInterface", class_SessionUpdateTimestampHandlerInterface_methods);
	class_entry = crex_register_internal_interface(&ce);

	return class_entry;
}

static crex_class_entry *register_class_SessionHandler(crex_class_entry *class_entry_SessionHandlerInterface, crex_class_entry *class_entry_SessionIdInterface)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SessionHandler", class_SessionHandler_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 2, class_entry_SessionHandlerInterface, class_entry_SessionIdInterface);

	return class_entry;
}
