/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 3419f4e77bd091e09e0cfc55d81f443d5a3396ff */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_litespeed_request_headers, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_getallheaders arginfo_litespeed_request_headers

#define arginfo_apache_request_headers arginfo_litespeed_request_headers

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_litespeed_response_headers, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
CREX_END_ARG_INFO()

#define arginfo_apache_response_headers arginfo_litespeed_response_headers

#define arginfo_apache_get_modules arginfo_litespeed_request_headers

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_litespeed_finish_request, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(litespeed_request_headers);
CREX_FUNCTION(litespeed_response_headers);
CREX_FUNCTION(apache_get_modules);
CREX_FUNCTION(litespeed_finish_request);


static const crex_function_entry ext_functions[] = {
	CREX_FE(litespeed_request_headers, arginfo_litespeed_request_headers)
	CREX_FALIAS(getallheaders, litespeed_request_headers, arginfo_getallheaders)
	CREX_FALIAS(apache_request_headers, litespeed_request_headers, arginfo_apache_request_headers)
	CREX_FE(litespeed_response_headers, arginfo_litespeed_response_headers)
	CREX_FALIAS(apache_response_headers, litespeed_response_headers, arginfo_apache_response_headers)
	CREX_FE(apache_get_modules, arginfo_apache_get_modules)
	CREX_FE(litespeed_finish_request, arginfo_litespeed_finish_request)
	CREX_FE_END
};
