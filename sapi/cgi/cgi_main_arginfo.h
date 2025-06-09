/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: df963adc6bc610cdd31861036889141fa9464ded */

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_apache_child_terminate, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_apache_request_headers, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_getallheaders arginfo_apache_request_headers

#define arginfo_apache_response_headers arginfo_apache_request_headers


CREX_FUNCTION(apache_child_terminate);
CREX_FUNCTION(apache_request_headers);
CREX_FUNCTION(apache_response_headers);


static const crex_function_entry ext_functions[] = {
	CREX_FE(apache_child_terminate, arginfo_apache_child_terminate)
	CREX_FE(apache_request_headers, arginfo_apache_request_headers)
	CREX_FALIAS(getallheaders, apache_request_headers, arginfo_getallheaders)
	CREX_FE(apache_response_headers, arginfo_apache_response_headers)
	CREX_FE_END
};
