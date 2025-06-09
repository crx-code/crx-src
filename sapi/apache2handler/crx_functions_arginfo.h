/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 130666f6f971fe7b43a450d922e4b3d092e78667 */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_apache_lookup_uri, 0, 1, MAY_BE_OBJECT|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_virtual, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, uri, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_apache_request_headers, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_getallheaders arginfo_apache_request_headers

#define arginfo_apache_response_headers arginfo_apache_request_headers

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_apache_note, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, note_name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, note_value, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_apache_setenv, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, variable, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, walk_to_top, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_apache_getenv, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, variable, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, walk_to_top, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_apache_get_version, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

#define arginfo_apache_get_modules arginfo_apache_request_headers


CREX_FUNCTION(apache_lookup_uri);
CREX_FUNCTION(virtual);
CREX_FUNCTION(apache_request_headers);
CREX_FUNCTION(apache_response_headers);
CREX_FUNCTION(apache_note);
CREX_FUNCTION(apache_setenv);
CREX_FUNCTION(apache_getenv);
CREX_FUNCTION(apache_get_version);
CREX_FUNCTION(apache_get_modules);


static const crex_function_entry ext_functions[] = {
	CREX_FE(apache_lookup_uri, arginfo_apache_lookup_uri)
	CREX_FE(virtual, arginfo_virtual)
	CREX_FE(apache_request_headers, arginfo_apache_request_headers)
	CREX_FALIAS(getallheaders, apache_request_headers, arginfo_getallheaders)
	CREX_FE(apache_response_headers, arginfo_apache_response_headers)
	CREX_FE(apache_note, arginfo_apache_note)
	CREX_FE(apache_setenv, arginfo_apache_setenv)
	CREX_FE(apache_getenv, arginfo_apache_getenv)
	CREX_FE(apache_get_version, arginfo_apache_get_version)
	CREX_FE(apache_get_modules, arginfo_apache_get_modules)
	CREX_FE_END
};
