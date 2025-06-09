/*
  +----------------------------------------------------------------------+
  | Copyright (c) The CRX Group                                          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the CRX license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | https://www.crx.net/license/3_01.txt                                 |
  | If you did not receive a copy of the CRX license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@crx.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Brad Lafountain <rodif_bl@yahoo.com>                        |
  |          Shane Caraveo <shane@caraveo.com>                           |
  |          Dmitry Stogov <dmitry@crx.net>                              |
  +----------------------------------------------------------------------+
*/

#ifndef CRX_SOAP_H
#define CRX_SOAP_H

#include "crx.h"
#include "crx_globals.h"
#include "ext/standard/info.h"
#include "ext/standard/crx_standard.h"
#if defined(HAVE_CRX_SESSION) && !defined(COMPILE_DL_SESSION)
#include "ext/session/crx_session.h"
#endif
#include "crex_smart_str.h"
#include "crx_ini.h"
#include "SAPI.h"
#include <libxml/parser.h>
#include <libxml/xpath.h>

#define CRX_SOAP_VERSION CRX_VERSION

#ifndef CRX_WIN32
# define TRUE 1
# define FALSE 0
# define stricmp strcasecmp
#endif

extern int le_url;

typedef struct _encodeType encodeType, *encodeTypePtr;
typedef struct _encode encode, *encodePtr;

typedef struct _sdl sdl, *sdlPtr;
typedef struct _sdlRestrictionInt sdlRestrictionInt, *sdlRestrictionIntPtr;
typedef struct _sdlRestrictionChar sdlRestrictionChar, *sdlRestrictionCharPtr;
typedef struct _sdlRestrictions sdlRestrictions, *sdlRestrictionsPtr;
typedef struct _sdlType sdlType, *sdlTypePtr;
typedef struct _sdlParam sdlParam, *sdlParamPtr;
typedef struct _sdlFunction sdlFunction, *sdlFunctionPtr;
typedef struct _sdlAttribute sdlAttribute, *sdlAttributePtr;
typedef struct _sdlBinding sdlBinding, *sdlBindingPtr;
typedef struct _sdlSoapBinding sdlSoapBinding, *sdlSoapBindingPtr;
typedef struct _sdlSoapBindingFunction sdlSoapBindingFunction, *sdlSoapBindingFunctionPtr;
typedef struct _sdlSoapBindingFunctionBody sdlSoapBindingFunctionBody, *sdlSoapBindingFunctionBodyPtr;

typedef struct _soapMapping soapMapping, *soapMappingPtr;
typedef struct _soapService soapService, *soapServicePtr;

#include "crx_xml.h"
#include "crx_encoding.h"
#include "crx_sdl.h"
#include "crx_schema.h"
#include "crx_http.h"
#include "crx_packet_soap.h"

struct _soapMapping {
	zval to_xml;
	zval to_zval;
};

struct _soapHeader;

struct _soapService {
	sdlPtr sdl;

	struct _soap_functions {
		HashTable *ft;
		int functions_all;
	} soap_functions;

	struct _soap_class {
		crex_class_entry *ce;
		zval *argv;
		int argc;
		int persistence;
	} soap_class;

	zval soap_object;

	HashTable *typemap;
	int        version;
	int        type;
	char      *actor;
	char      *uri;
	xmlCharEncodingHandlerPtr encoding;
	HashTable *class_map;
	int        features;
	struct _soapHeader **soap_headers_ptr;
	int send_errors;
};

#define SOAP_CLASS 1
#define SOAP_FUNCTIONS 2
#define SOAP_OBJECT 3
#define SOAP_FUNCTIONS_ALL 999

#define SOAP_MAP_FUNCTION 1
#define SOAP_MAP_CLASS 2

#define SOAP_PERSISTENCE_SESSION 1
#define SOAP_PERSISTENCE_REQUEST 2

#define SOAP_1_1 1
#define SOAP_1_2 2

#define SOAP_ACTOR_NEXT             1
#define SOAP_ACTOR_NONE             2
#define SOAP_ACTOR_UNLIMATERECEIVER 3

#define SOAP_1_1_ACTOR_NEXT             "http://schemas.xmlsoap.org/soap/actor/next"

#define SOAP_1_2_ACTOR_NEXT             "http://www.w3.org/2003/05/soap-envelope/role/next"
#define SOAP_1_2_ACTOR_NONE             "http://www.w3.org/2003/05/soap-envelope/role/none"
#define SOAP_1_2_ACTOR_UNLIMATERECEIVER "http://www.w3.org/2003/05/soap-envelope/role/ultimateReceiver"

#define SOAP_COMPRESSION_ACCEPT  0x20
#define SOAP_COMPRESSION_GZIP    0x00
#define SOAP_COMPRESSION_DEFLATE 0x10

#define SOAP_AUTHENTICATION_BASIC   0
#define SOAP_AUTHENTICATION_DIGEST  1

#define SOAP_SINGLE_ELEMENT_ARRAYS  (1<<0)
#define SOAP_WAIT_ONE_WAY_CALLS     (1<<1)
#define SOAP_USE_XSI_ARRAY_TYPE     (1<<2)

#define WSDL_CACHE_NONE     0x0
#define WSDL_CACHE_DISK     0x1
#define WSDL_CACHE_MEMORY   0x2
#define WSDL_CACHE_BOTH     0x3

/* New SOAP SSL Method Constants */
#define SOAP_SSL_METHOD_TLS     0
#define SOAP_SSL_METHOD_SSLv2   1
#define SOAP_SSL_METHOD_SSLv3   2
#define SOAP_SSL_METHOD_SSLv23  3


CREX_BEGIN_MODULE_GLOBALS(soap)
	HashTable  defEncNs;     /* mapping of default namespaces to prefixes */
	HashTable  defEnc;
	HashTable  defEncIndex;
	HashTable *typemap;
	int        cur_uniq_ns;
	int        soap_version;
	sdlPtr     sdl;
	bool  use_soap_error_handler;
	char*      error_code;
	zval       error_object;
	char       cache;
	char       cache_mode;
	char       cache_enabled;
	char*      cache_dir;
	crex_long       cache_ttl;
	crex_long       cache_limit;
	HashTable *mem_cache;
	xmlCharEncodingHandlerPtr encoding;
	HashTable *class_map;
	int        features;
	HashTable  wsdl_cache;
	int        cur_uniq_ref;
	HashTable *ref_map;
CREX_END_MODULE_GLOBALS(soap)

#ifdef ZTS
#include "TSRM.h"
#endif

extern crex_module_entry soap_module_entry;
#define soap_module_ptr &soap_module_entry
#define crxext_soap_ptr soap_module_ptr

CREX_EXTERN_MODULE_GLOBALS(soap)
#define SOAP_GLOBAL(v) CREX_MODULE_GLOBALS_ACCESSOR(soap, v)

#if defined(ZTS) && defined(COMPILE_DL_SOAP)
CREX_TSRMLS_CACHE_EXTERN()
#endif

extern crex_class_entry* soap_class_entry;
extern crex_class_entry* soap_var_class_entry;

void add_soap_fault(zval *obj, char *fault_code, char *fault_string, char *fault_actor, zval *fault_detail);

#define soap_error0(severity, format) \
	crx_error(severity, "SOAP-ERROR: " format)

#define soap_error1(severity, format, param1) \
	crx_error(severity, "SOAP-ERROR: " format, param1)

#define soap_error2(severity, format, param1, param2) \
	crx_error(severity, "SOAP-ERROR: " format, param1, param2)

#define soap_error3(severity, format, param1, param2, param3) \
	crx_error(severity, "SOAP-ERROR: " format, param1, param2, param3)

static crex_always_inline zval *crx_soap_deref(zval *zv) {
	if (UNEXPECTED(C_TYPE_P(zv) == IS_REFERENCE)) {
		return C_REFVAL_P(zv);
	}
	return zv;
}

#define C_CLIENT_URI_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 0))
#define C_CLIENT_STYLE_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 1))
#define C_CLIENT_USE_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 2))
#define C_CLIENT_LOCATION_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 3))
#define C_CLIENT_TRACE_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 4))
#define C_CLIENT_COMPRESSION_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 5))
#define C_CLIENT_SDL_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 6))
#define C_CLIENT_TYPEMAP_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 7))
#define C_CLIENT_HTTPSOCKET_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 8))
#define C_CLIENT_HTTPURL_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 9))
#define C_CLIENT_LOGIN_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 10))
#define C_CLIENT_PASSWORD_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 11))
#define C_CLIENT_USE_DIGEST_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 12))
#define C_CLIENT_DIGEST_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 13))
#define C_CLIENT_PROXY_HOST_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 14))
#define C_CLIENT_PROXY_PORT_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 15))
#define C_CLIENT_PROXY_LOGIN_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 16))
#define C_CLIENT_PROXY_PASSWORD_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 17))
#define C_CLIENT_EXCEPTIONS_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 18))
#define C_CLIENT_ENCODING_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 19))
#define C_CLIENT_CLASSMAP_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 20))
#define C_CLIENT_FEATURES_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 21))
#define C_CLIENT_CONNECTION_TIMEOUT_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 22))
#define C_CLIENT_STREAM_CONTEXT_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 23))
#define C_CLIENT_USER_AGENT_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 24))
#define C_CLIENT_KEEP_ALIVE_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 25))
#define C_CLIENT_SSL_METHOD_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 26))
#define C_CLIENT_SOAP_VERSION_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 27))
#define C_CLIENT_USE_PROXY_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 28))
#define C_CLIENT_COOKIES_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 29))
#define C_CLIENT_DEFAULT_HEADERS_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 30))
#define C_CLIENT_SOAP_FAULT_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 31))
#define C_CLIENT_LAST_REQUEST_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 32))
#define C_CLIENT_LAST_RESPONSE_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 33))
#define C_CLIENT_LAST_REQUEST_HEADERS_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 34))
#define C_CLIENT_LAST_RESPONSE_HEADERS_P(zv) crx_soap_deref(OBJ_PROP_NUM(C_OBJ_P(zv), 35))

#endif
