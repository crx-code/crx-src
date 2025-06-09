/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 81f337ea4ac5361ca4a0873fcd3b033beaf524c6 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_opcache_reset, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_opcache_get_status, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, include_scripts, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_opcache_compile_file, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_opcache_invalidate, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, force, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_opcache_get_configuration, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
CREX_END_ARG_INFO()

#define arginfo_opcache_is_script_cached arginfo_opcache_compile_file


CREX_FUNCTION(opcache_reset);
CREX_FUNCTION(opcache_get_status);
CREX_FUNCTION(opcache_compile_file);
CREX_FUNCTION(opcache_invalidate);
CREX_FUNCTION(opcache_get_configuration);
CREX_FUNCTION(opcache_is_script_cached);


static const crex_function_entry ext_functions[] = {
	CREX_FE(opcache_reset, arginfo_opcache_reset)
	CREX_FE(opcache_get_status, arginfo_opcache_get_status)
	CREX_FE(opcache_compile_file, arginfo_opcache_compile_file)
	CREX_FE(opcache_invalidate, arginfo_opcache_invalidate)
	CREX_FE(opcache_get_configuration, arginfo_opcache_get_configuration)
	CREX_FE(opcache_is_script_cached, arginfo_opcache_is_script_cached)
	CREX_FE_END
};
