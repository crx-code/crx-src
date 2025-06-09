/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: b4ac4c0f1d91c354293e21185a2e6d9f99cc9fcc */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_fastcgi_finish_request, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_apache_request_headers, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_getallheaders arginfo_apache_request_headers

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_fpm_get_status, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
CREX_END_ARG_INFO()


CREX_FUNCTION(fastcgi_finish_request);
CREX_FUNCTION(apache_request_headers);
CREX_FUNCTION(fpm_get_status);


static const crex_function_entry ext_functions[] = {
	CREX_FE(fastcgi_finish_request, arginfo_fastcgi_finish_request)
	CREX_FE(apache_request_headers, arginfo_apache_request_headers)
	CREX_FALIAS(getallheaders, apache_request_headers, arginfo_getallheaders)
	CREX_FE(fpm_get_status, arginfo_fpm_get_status)
	CREX_FE_END
};
