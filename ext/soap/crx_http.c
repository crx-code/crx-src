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

#include "crx_soap.h"
#include "ext/standard/base64.h"
#include "ext/standard/md5.h"
#include "ext/random/crx_random.h"
#include "ext/hash/crx_hash.h"

static char *get_http_header_value_nodup(char *headers, char *type, size_t *len);
static char *get_http_header_value(char *headers, char *type);
static crex_string *get_http_body(crx_stream *socketd, int close, char *headers);
static crex_string *get_http_headers(crx_stream *socketd);

#define smart_str_append_const(str, const) \
	smart_str_appendl(str,const,sizeof(const)-1)

/* Proxy HTTP Authentication */
int proxy_authentication(zval* this_ptr, smart_str* soap_headers)
{
	zval *login = C_CLIENT_PROXY_LOGIN_P(this_ptr);
	if (C_TYPE_P(login) == IS_STRING) {
		smart_str auth = {0};
		smart_str_append(&auth, C_STR_P(login));
		smart_str_appendc(&auth, ':');

		zval *password = C_CLIENT_PROXY_PASSWORD_P(this_ptr);
		if (C_TYPE_P(password) == IS_STRING) {
			smart_str_append(&auth, C_STR_P(password));
		}
		smart_str_0(&auth);
		crex_string *buf = crx_base64_encode((unsigned char*)ZSTR_VAL(auth.s), ZSTR_LEN(auth.s));
		smart_str_append_const(soap_headers, "Proxy-Authorization: Basic ");
		smart_str_append(soap_headers, buf);
		smart_str_append_const(soap_headers, "\r\n");
		crex_string_release_ex(buf, 0);
		smart_str_free(&auth);
		return 1;
	}
	return 0;
}

/* HTTP Authentication */
int basic_authentication(zval* this_ptr, smart_str* soap_headers)
{
	zval *login = C_CLIENT_LOGIN_P(this_ptr);
	zval *use_digest = C_CLIENT_USE_DIGEST_P(this_ptr);
	if (C_TYPE_P(login) == IS_STRING && C_TYPE_P(use_digest) != IS_TRUE) {
		smart_str auth = {0};
		smart_str_append(&auth, C_STR_P(login));
		smart_str_appendc(&auth, ':');

		zval *password = C_CLIENT_PASSWORD_P(this_ptr);
		if (C_TYPE_P(password) == IS_STRING) {
			smart_str_append(&auth, C_STR_P(password));
		}
		smart_str_0(&auth);
		crex_string *buf = crx_base64_encode((unsigned char*)ZSTR_VAL(auth.s), ZSTR_LEN(auth.s));
		smart_str_append_const(soap_headers, "Authorization: Basic ");
		smart_str_append(soap_headers, buf);
		smart_str_append_const(soap_headers, "\r\n");
		crex_string_release_ex(buf, 0);
		smart_str_free(&auth);
		return 1;
	}
	return 0;
}

/* Additional HTTP headers */
void http_context_headers(crx_stream_context* context,
                          bool has_authorization,
                          bool has_proxy_authorization,
                          bool has_cookies,
                          smart_str* soap_headers)
{
	zval *tmp;

	if (context &&
		(tmp = crx_stream_context_get_option(context, "http", "header")) != NULL &&
		C_TYPE_P(tmp) == IS_STRING && C_STRLEN_P(tmp)) {
		char *s = C_STRVAL_P(tmp);
		char *p;
		int name_len;

		while (*s) {
			/* skip leading newlines and spaces */
			while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') {
				s++;
			}
			/* extract header name */
			p = s;
			name_len = -1;
			while (*p) {
				if (*p == ':') {
					if (name_len < 0) name_len = p - s;
					break;
				} else if (*p == ' ' || *p == '\t') {
					if (name_len < 0) name_len = p - s;
				} else if (*p == '\r' || *p == '\n') {
					break;
				}
				p++;
			}
			if (*p == ':') {
				/* extract header value */
				while (*p && *p != '\r' && *p != '\n') {
					p++;
				}
				/* skip some predefined headers */
				if ((name_len != sizeof("host")-1 ||
				     strncasecmp(s, "host", sizeof("host")-1) != 0) &&
				    (name_len != sizeof("connection")-1 ||
				     strncasecmp(s, "connection", sizeof("connection")-1) != 0) &&
				    (name_len != sizeof("user-agent")-1 ||
				     strncasecmp(s, "user-agent", sizeof("user-agent")-1) != 0) &&
				    (name_len != sizeof("content-length")-1 ||
				     strncasecmp(s, "content-length", sizeof("content-length")-1) != 0) &&
				    (name_len != sizeof("content-type")-1 ||
				     strncasecmp(s, "content-type", sizeof("content-type")-1) != 0) &&
				    (!has_cookies ||
				     name_len != sizeof("cookie")-1 ||
				     strncasecmp(s, "cookie", sizeof("cookie")-1) != 0) &&
				    (!has_authorization ||
				     name_len != sizeof("authorization")-1 ||
				     strncasecmp(s, "authorization", sizeof("authorization")-1) != 0) &&
				    (!has_proxy_authorization ||
				     name_len != sizeof("proxy-authorization")-1 ||
				     strncasecmp(s, "proxy-authorization", sizeof("proxy-authorization")-1) != 0)) {
				    /* add header */
					smart_str_appendl(soap_headers, s, p-s);
					smart_str_append_const(soap_headers, "\r\n");
				}
			}
			s = (*p) ? (p + 1) : p;
		}
	}
}

static crx_stream* http_connect(zval* this_ptr, crx_url *crxurl, int use_ssl, crx_stream_context *context, int *use_proxy)
{
	crx_stream *stream;
	zval *tmp, ssl_proxy_peer_name;
	char *host;
	char *name;
	char *protocol;
	crex_long namelen;
	int port;
	int old_error_reporting;
	struct timeval tv;
	struct timeval *timeout = NULL;

	zval *proxy_host = C_CLIENT_PROXY_HOST_P(this_ptr);
	zval *proxy_port = C_CLIENT_PROXY_PORT_P(this_ptr);
	if (C_TYPE_P(proxy_host) == IS_STRING && C_TYPE_P(proxy_port) == IS_LONG) {
		host = C_STRVAL_P(proxy_host);
		port = C_LVAL_P(proxy_port);
		*use_proxy = 1;
	} else {
		host = ZSTR_VAL(crxurl->host);
		port = crxurl->port;
	}

	tmp = C_CLIENT_CONNECTION_TIMEOUT_P(this_ptr);
	if (C_TYPE_P(tmp) == IS_LONG && C_LVAL_P(tmp) > 0) {
		tv.tv_sec = C_LVAL_P(tmp);
		tv.tv_usec = 0;
		timeout = &tv;
	}

	old_error_reporting = EG(error_reporting);
	EG(error_reporting) &= ~(E_WARNING|E_NOTICE|E_USER_WARNING|E_USER_NOTICE);

	/* Changed ternary operator to an if/else so that additional comparisons can be done on the ssl_method property */
	if (use_ssl && !*use_proxy) {
		tmp = C_CLIENT_SSL_METHOD_P(this_ptr);
		if (C_TYPE_P(tmp) == IS_LONG) {
			/* uses constants declared in soap.c to determine ssl uri protocol */
			switch (C_LVAL_P(tmp)) {
				case SOAP_SSL_METHOD_TLS:
					protocol = "tls";
					break;

				case SOAP_SSL_METHOD_SSLv2:
					protocol = "sslv2";
					break;

				case SOAP_SSL_METHOD_SSLv3:
					protocol = "sslv3";
					break;

				case SOAP_SSL_METHOD_SSLv23:
					protocol = "ssl";
					break;

				default:
					protocol = "ssl";
					break;

			}
		} else {
			protocol = "ssl";
		}
	} else {
		protocol = "tcp";
	}

	namelen = spprintf(&name, 0, "%s://%s:%d", protocol, host, port);

	stream = crx_stream_xport_create(name, namelen,
		REPORT_ERRORS,
		STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT,
		NULL /*persistent_id*/,
		timeout,
		context,
		NULL, NULL);
	efree(name);

	/* SSL & proxy */
	if (stream && *use_proxy && use_ssl) {
		smart_str soap_headers = {0};

		/* Set peer_name or name verification will try to use the proxy server name */
		if (!context || (tmp = crx_stream_context_get_option(context, "ssl", "peer_name")) == NULL) {
			ZVAL_STR_COPY(&ssl_proxy_peer_name, crxurl->host);
			crx_stream_context_set_option(CRX_STREAM_CONTEXT(stream), "ssl", "peer_name", &ssl_proxy_peer_name);
			zval_ptr_dtor(&ssl_proxy_peer_name);
		}

		smart_str_append_const(&soap_headers, "CONNECT ");
		smart_str_appends(&soap_headers, ZSTR_VAL(crxurl->host));
		smart_str_appendc(&soap_headers, ':');
		smart_str_append_unsigned(&soap_headers, crxurl->port);
		smart_str_append_const(&soap_headers, " HTTP/1.1\r\n");
		smart_str_append_const(&soap_headers, "Host: ");
		smart_str_appends(&soap_headers, ZSTR_VAL(crxurl->host));
		if (crxurl->port != 80) {
			smart_str_appendc(&soap_headers, ':');
			smart_str_append_unsigned(&soap_headers, crxurl->port);
		}
		smart_str_append_const(&soap_headers, "\r\n");
		proxy_authentication(this_ptr, &soap_headers);
		smart_str_append_const(&soap_headers, "\r\n");
		if (crx_stream_write(stream, ZSTR_VAL(soap_headers.s), ZSTR_LEN(soap_headers.s)) != ZSTR_LEN(soap_headers.s)) {
			crx_stream_close(stream);
			stream = NULL;
		}
		smart_str_free(&soap_headers);

		if (stream) {
			crex_string *http_headers = get_http_headers(stream);
			if (http_headers) {
				crex_string_free(http_headers);
			} else {
				crx_stream_close(stream);
				stream = NULL;
			}
		}
		/* enable SSL transport layer */
		if (stream) {
			/* if a stream is created without encryption, check to see if SSL method parameter is specified and use
			   proper encrypyion method based on constants defined in soap.c */
			int crypto_method = STREAM_CRYPTO_METHOD_SSLv23_CLIENT;
			tmp = C_CLIENT_SSL_METHOD_P(this_ptr);
			if (C_TYPE_P(tmp) == IS_LONG) {
				switch (C_LVAL_P(tmp)) {
					case SOAP_SSL_METHOD_TLS:
						crypto_method = STREAM_CRYPTO_METHOD_TLS_CLIENT;
						break;

					case SOAP_SSL_METHOD_SSLv2:
						crypto_method = STREAM_CRYPTO_METHOD_SSLv2_CLIENT;
						break;

					case SOAP_SSL_METHOD_SSLv3:
						crypto_method = STREAM_CRYPTO_METHOD_SSLv3_CLIENT;
						break;

					case SOAP_SSL_METHOD_SSLv23:
						crypto_method = STREAM_CRYPTO_METHOD_SSLv23_CLIENT;
						break;

					default:
						crypto_method = STREAM_CRYPTO_METHOD_TLS_CLIENT;
						break;
				}
			}
			if (crx_stream_xport_crypto_setup(stream, crypto_method, NULL) < 0 ||
			    crx_stream_xport_crypto_enable(stream, 1) < 0) {
				crx_stream_close(stream);
				stream = NULL;
			}
		}
	}

	EG(error_reporting) = old_error_reporting;
	return stream;
}

static int in_domain(const char *host, const char *domain)
{
	if (domain[0] == '.') {
		int l1 = strlen(host);
		int l2 = strlen(domain);
		if (l1 > l2) {
			return strcmp(host+l1-l2,domain) == 0;
		} else {
			return 0;
		}
	} else {
		return strcmp(host,domain) == 0;
	}
}

int make_http_soap_request(zval        *this_ptr,
                           crex_string *buf,
                           char        *location,
                           char        *soapaction,
                           int          soap_version,
                           zval        *return_value)
{
	crex_string *request;
	smart_str soap_headers = {0};
	smart_str soap_headers_z = {0};
	size_t err;
	crx_url *crxurl = NULL;
	crx_stream *stream;
	zval *tmp;
	int use_proxy = 0;
	int use_ssl;
	crex_string *http_body;
	char *content_type, *http_version, *cookie_itt;
	size_t cookie_len;
	int http_close;
	crex_string *http_headers;
	char *connection;
	int http_1_1;
	int http_status;
	int content_type_xml = 0;
	crex_long redirect_max = 20;
	char *content_encoding;
	char *http_msg = NULL;
	bool old_allow_url_fopen;
	crx_stream_context *context = NULL;
	bool has_authorization = 0;
	bool has_proxy_authorization = 0;
	bool has_cookies = 0;

	if (this_ptr == NULL || C_TYPE_P(this_ptr) != IS_OBJECT) {
		return FALSE;
	}

	request = buf;
	/* Compress request */
	tmp = C_CLIENT_COMPRESSION_P(this_ptr);
	if (C_TYPE_P(tmp) == IS_LONG) {
		int level = C_LVAL_P(tmp) & 0x0f;
		int kind  = C_LVAL_P(tmp) & SOAP_COMPRESSION_DEFLATE;

		if (level > 9) {level = 9;}

	  if ((C_LVAL_P(tmp) & SOAP_COMPRESSION_ACCEPT) != 0) {
			smart_str_append_const(&soap_headers_z,"Accept-Encoding: gzip, deflate\r\n");
	  }
	  if (level > 0) {
			zval func;
			zval retval;
			zval params[3];
			int n;

			ZVAL_STR_COPY(&params[0], buf);
			ZVAL_LONG(&params[1], level);
			if (kind == SOAP_COMPRESSION_DEFLATE) {
				n = 2;
				ZVAL_STRING(&func, "gzcompress");
				smart_str_append_const(&soap_headers_z,"Content-Encoding: deflate\r\n");
			} else {
				n = 3;
				ZVAL_STRING(&func, "gzencode");
				smart_str_append_const(&soap_headers_z,"Content-Encoding: gzip\r\n");
				ZVAL_LONG(&params[2], 0x1f);
			}
			if (call_user_function(CG(function_table), (zval*)NULL, &func, &retval, n, params) == SUCCESS &&
			    C_TYPE(retval) == IS_STRING) {
				zval_ptr_dtor(&params[0]);
				zval_ptr_dtor(&func);
				request = C_STR(retval);
			} else {
				zval_ptr_dtor(&params[0]);
				zval_ptr_dtor(&func);
				if (request != buf) {
					crex_string_release_ex(request, 0);
				}
				smart_str_free(&soap_headers_z);
				return FALSE;
			}
	  }
	}

	tmp = C_CLIENT_HTTPSOCKET_P(this_ptr);
	if (C_TYPE_P(tmp) == IS_RESOURCE) {
		crx_stream_from_zval_no_verify(stream,tmp);
		tmp = C_CLIENT_USE_PROXY_P(this_ptr);
		if (C_TYPE_P(tmp) == IS_LONG) {
			use_proxy = C_LVAL_P(tmp);
		}
	} else {
		stream = NULL;
	}

	if (location != NULL && location[0] != '\000') {
		crxurl = crx_url_parse(location);
	}

	tmp = C_CLIENT_STREAM_CONTEXT_P(this_ptr);
	if (C_TYPE_P(tmp) == IS_RESOURCE) {
		context = crx_stream_context_from_zval(tmp, 0);
	}

	if (context &&
		(tmp = crx_stream_context_get_option(context, "http", "max_redirects")) != NULL) {
		if (C_TYPE_P(tmp) != IS_STRING || !is_numeric_string(C_STRVAL_P(tmp), C_STRLEN_P(tmp), &redirect_max, NULL, 1)) {
			if (C_TYPE_P(tmp) == IS_LONG)
				redirect_max = C_LVAL_P(tmp);
		}
	}

try_again:
	if (crxurl == NULL || crxurl->host == NULL) {
	  if (crxurl != NULL) {crx_url_free(crxurl);}
		if (request != buf) {
			crex_string_release_ex(request, 0);
		}
		add_soap_fault(this_ptr, "HTTP", "Unable to parse URL", NULL, NULL);
		smart_str_free(&soap_headers_z);
		return FALSE;
	}

	use_ssl = 0;
	if (crxurl->scheme != NULL && crex_string_equals_literal(crxurl->scheme, "https")) {
		use_ssl = 1;
	} else if (crxurl->scheme == NULL || !crex_string_equals_literal(crxurl->scheme, "http")) {
		crx_url_free(crxurl);
		if (request != buf) {
			crex_string_release_ex(request, 0);
		}
		add_soap_fault(this_ptr, "HTTP", "Unknown protocol. Only http and https are allowed.", NULL, NULL);
		smart_str_free(&soap_headers_z);
		return FALSE;
	}

	old_allow_url_fopen = PG(allow_url_fopen);
	PG(allow_url_fopen) = 1;
	if (use_ssl && crx_stream_locate_url_wrapper("https://", NULL, STREAM_LOCATE_WRAPPERS_ONLY) == NULL) {
		crx_url_free(crxurl);
		if (request != buf) {
			crex_string_release_ex(request, 0);
		}
		add_soap_fault(this_ptr, "HTTP", "SSL support is not available in this build", NULL, NULL);
		PG(allow_url_fopen) = old_allow_url_fopen;
		smart_str_free(&soap_headers_z);
		return FALSE;
	}

	if (crxurl->port == 0) {
		crxurl->port = use_ssl ? 443 : 80;
	}

	/* Check if request to the same host */
	if (stream != NULL) {
		crx_url *orig;
		tmp = C_CLIENT_HTTPURL_P(this_ptr);
		if (C_TYPE_P(tmp) == IS_RESOURCE &&
			(orig = (crx_url *) crex_fetch_resource_ex(tmp, "httpurl", le_url)) != NULL &&
		    ((use_proxy && !use_ssl) ||
		     (((use_ssl && orig->scheme != NULL && crex_string_equals_literal(orig->scheme, "https")) ||
		      (!use_ssl && orig->scheme == NULL) ||
		      (!use_ssl && !crex_string_equals_literal(orig->scheme, "https"))) &&
		     crex_string_equals(orig->host, crxurl->host) &&
		     orig->port == crxurl->port))) {
		} else {
			crx_stream_close(stream);
			convert_to_null(C_CLIENT_HTTPURL_P(this_ptr));
			convert_to_null(C_CLIENT_HTTPSOCKET_P(this_ptr));
			convert_to_null(C_CLIENT_USE_PROXY_P(this_ptr));
			stream = NULL;
			use_proxy = 0;
		}
	}

	/* Check if keep-alive connection is still opened */
	if (stream != NULL && crx_stream_eof(stream)) {
		crx_stream_close(stream);
		convert_to_null(C_CLIENT_HTTPURL_P(this_ptr));
		convert_to_null(C_CLIENT_HTTPSOCKET_P(this_ptr));
		convert_to_null(C_CLIENT_USE_PROXY_P(this_ptr));
		stream = NULL;
		use_proxy = 0;
	}

	if (!stream) {
		stream = http_connect(this_ptr, crxurl, use_ssl, context, &use_proxy);
		if (stream) {
			crx_stream_auto_cleanup(stream);
			ZVAL_RES(C_CLIENT_HTTPSOCKET_P(this_ptr), stream->res);
			GC_ADDREF(stream->res);
			ZVAL_LONG(C_CLIENT_USE_PROXY_P(this_ptr), use_proxy);
		} else {
			crx_url_free(crxurl);
			if (request != buf) {
				crex_string_release_ex(request, 0);
			}
			add_soap_fault(this_ptr, "HTTP", "Could not connect to host", NULL, NULL);
			PG(allow_url_fopen) = old_allow_url_fopen;
			smart_str_free(&soap_headers_z);
			return FALSE;
		}
	}
	PG(allow_url_fopen) = old_allow_url_fopen;

	if (stream) {
		zval *cookies, *login, *password;
		crex_resource *ret = crex_register_resource(crxurl, le_url);
		ZVAL_RES(C_CLIENT_HTTPURL_P(this_ptr), ret);
		GC_ADDREF(ret);

		if (context &&
		    (tmp = crx_stream_context_get_option(context, "http", "protocol_version")) != NULL &&
		    C_TYPE_P(tmp) == IS_DOUBLE &&
		    C_DVAL_P(tmp) == 1.0) {
			http_1_1 = 0;
		} else {
			http_1_1 = 1;
		}

		smart_str_append_const(&soap_headers, "POST ");
		if (use_proxy && !use_ssl) {
			smart_str_appends(&soap_headers, ZSTR_VAL(crxurl->scheme));
			smart_str_append_const(&soap_headers, "://");
			smart_str_appends(&soap_headers, ZSTR_VAL(crxurl->host));
			smart_str_appendc(&soap_headers, ':');
			smart_str_append_unsigned(&soap_headers, crxurl->port);
		}
		if (crxurl->path) {
			smart_str_appends(&soap_headers, ZSTR_VAL(crxurl->path));
		} else {
			smart_str_appendc(&soap_headers, '/');
		}
		if (crxurl->query) {
			smart_str_appendc(&soap_headers, '?');
			smart_str_appends(&soap_headers, ZSTR_VAL(crxurl->query));
		}
		if (crxurl->fragment) {
			smart_str_appendc(&soap_headers, '#');
			smart_str_appends(&soap_headers, ZSTR_VAL(crxurl->fragment));
		}
		if (http_1_1) {
			smart_str_append_const(&soap_headers, " HTTP/1.1\r\n");
		} else {
			smart_str_append_const(&soap_headers, " HTTP/1.0\r\n");
		}
		smart_str_append_const(&soap_headers, "Host: ");
		smart_str_appends(&soap_headers, ZSTR_VAL(crxurl->host));
		if (crxurl->port != (use_ssl?443:80)) {
			smart_str_appendc(&soap_headers, ':');
			smart_str_append_unsigned(&soap_headers, crxurl->port);
		}
		if (!http_1_1 || C_TYPE_P(C_CLIENT_KEEP_ALIVE_P(this_ptr)) == IS_FALSE) {
			smart_str_append_const(&soap_headers, "\r\n"
				"Connection: close\r\n");
		} else {
			smart_str_append_const(&soap_headers, "\r\n"
				"Connection: Keep-Alive\r\n");
		}
		tmp = C_CLIENT_USER_AGENT_P(this_ptr);
		if (C_TYPE_P(tmp) == IS_STRING) {
			if (C_STRLEN_P(tmp) > 0) {
				smart_str_append_const(&soap_headers, "User-Agent: ");
				smart_str_appendl(&soap_headers, C_STRVAL_P(tmp), C_STRLEN_P(tmp));
				smart_str_append_const(&soap_headers, "\r\n");
			}
		} else if (context &&
		           (tmp = crx_stream_context_get_option(context, "http", "user_agent")) != NULL &&
		           C_TYPE_P(tmp) == IS_STRING) {
			if (C_STRLEN_P(tmp) > 0) {
				smart_str_append_const(&soap_headers, "User-Agent: ");
				smart_str_appendl(&soap_headers, C_STRVAL_P(tmp), C_STRLEN_P(tmp));
				smart_str_append_const(&soap_headers, "\r\n");
			}
		} else if (FG(user_agent)) {
			smart_str_append_const(&soap_headers, "User-Agent: ");
			smart_str_appends(&soap_headers, FG(user_agent));
			smart_str_append_const(&soap_headers, "\r\n");
		} else {
			smart_str_append_const(&soap_headers, "User-Agent: CRX-SOAP/"CRX_VERSION"\r\n");
		}

		smart_str_append_smart_str(&soap_headers, &soap_headers_z);

		if (soap_version == SOAP_1_2) {
			if (context &&
				(tmp = crx_stream_context_get_option(context, "http", "content_type")) != NULL &&
				C_TYPE_P(tmp) == IS_STRING &&
				C_STRLEN_P(tmp) > 0
			) {
				smart_str_append_const(&soap_headers, "Content-Type: ");
				smart_str_appendl(&soap_headers, C_STRVAL_P(tmp), C_STRLEN_P(tmp));
			} else {
				smart_str_append_const(&soap_headers, "Content-Type: application/soap+xml; charset=utf-8");
			}
			if (soapaction) {
				smart_str_append_const(&soap_headers,"; action=\"");
				smart_str_appends(&soap_headers, soapaction);
				smart_str_append_const(&soap_headers,"\"");
			}
			smart_str_append_const(&soap_headers,"\r\n");
		} else {
			if (context &&
				(tmp = crx_stream_context_get_option(context, "http", "content_type")) != NULL &&
				C_TYPE_P(tmp) == IS_STRING &&
				C_STRLEN_P(tmp) > 0
			) {
				smart_str_append_const(&soap_headers, "Content-Type: ");
				smart_str_appendl(&soap_headers, C_STRVAL_P(tmp), C_STRLEN_P(tmp));
				smart_str_append_const(&soap_headers, "\r\n");
			} else {
				smart_str_append_const(&soap_headers, "Content-Type: text/xml; charset=utf-8\r\n");
			}
			if (soapaction) {
				smart_str_append_const(&soap_headers, "SOAPAction: \"");
				smart_str_appends(&soap_headers, soapaction);
				smart_str_append_const(&soap_headers, "\"\r\n");
			}
		}
		smart_str_append_const(&soap_headers,"Content-Length: ");
		smart_str_append_long(&soap_headers, request->len);
		smart_str_append_const(&soap_headers, "\r\n");

		/* HTTP Authentication */
		login = C_CLIENT_LOGIN_P(this_ptr);
		if (C_TYPE_P(login) == IS_STRING) {
			zval *digest = C_CLIENT_DIGEST_P(this_ptr);

			has_authorization = 1;
			if (C_TYPE_P(digest) == IS_ARRAY) {
				char          HA1[33], HA2[33], response[33], cnonce[33], nc[9];
				unsigned char nonce[16];
				CRX_MD5_CTX   md5ctx;
				unsigned char hash[16];

				if (UNEXPECTED(crx_random_bytes_throw(&nonce, sizeof(nonce)) != SUCCESS)) {
					CREX_ASSERT(EG(exception));
					crx_stream_close(stream);
					convert_to_null(C_CLIENT_HTTPURL_P(this_ptr));
					convert_to_null(C_CLIENT_HTTPSOCKET_P(this_ptr));
					convert_to_null(C_CLIENT_USE_PROXY_P(this_ptr));
					smart_str_free(&soap_headers_z);
					smart_str_free(&soap_headers);
					return FALSE;
				}

				crx_hash_bin2hex(cnonce, nonce, sizeof(nonce));
				cnonce[32] = 0;

				if ((tmp = crex_hash_str_find(C_ARRVAL_P(digest), "nc", sizeof("nc")-1)) != NULL &&
					C_TYPE_P(tmp) == IS_LONG) {
					C_LVAL_P(tmp)++;
					snprintf(nc, sizeof(nc), "%08" CREX_LONG_FMT_SPEC, C_LVAL_P(tmp));
				} else {
					add_assoc_long(digest, "nc", 1);
					strcpy(nc, "00000001");
				}

				CRX_MD5Init(&md5ctx);
				CRX_MD5Update(&md5ctx, (unsigned char*)C_STRVAL_P(login), C_STRLEN_P(login));
				CRX_MD5Update(&md5ctx, (unsigned char*)":", 1);
				if ((tmp = crex_hash_str_find(C_ARRVAL_P(digest), "realm", sizeof("realm")-1)) != NULL &&
					C_TYPE_P(tmp) == IS_STRING) {
					CRX_MD5Update(&md5ctx, (unsigned char*)C_STRVAL_P(tmp), C_STRLEN_P(tmp));
				}
				CRX_MD5Update(&md5ctx, (unsigned char*)":", 1);
				password = C_CLIENT_PASSWORD_P(this_ptr);
				if (C_TYPE_P(password) == IS_STRING) {
					CRX_MD5Update(&md5ctx, (unsigned char*)C_STRVAL_P(password), C_STRLEN_P(password));
				}
				CRX_MD5Final(hash, &md5ctx);
				make_digest(HA1, hash);
				if ((tmp = crex_hash_str_find(C_ARRVAL_P(digest), "algorithm", sizeof("algorithm")-1)) != NULL &&
					C_TYPE_P(tmp) == IS_STRING &&
					C_STRLEN_P(tmp) == sizeof("md5-sess")-1 &&
					stricmp(C_STRVAL_P(tmp), "md5-sess") == 0) {
					CRX_MD5Init(&md5ctx);
					CRX_MD5Update(&md5ctx, (unsigned char*)HA1, 32);
					CRX_MD5Update(&md5ctx, (unsigned char*)":", 1);
					if ((tmp = crex_hash_str_find(C_ARRVAL_P(digest), "nonce", sizeof("nonce")-1)) != NULL &&
						C_TYPE_P(tmp) == IS_STRING) {
						CRX_MD5Update(&md5ctx, (unsigned char*)C_STRVAL_P(tmp), C_STRLEN_P(tmp));
					}
					CRX_MD5Update(&md5ctx, (unsigned char*)":", 1);
					CRX_MD5Update(&md5ctx, (unsigned char*)cnonce, 8);
					CRX_MD5Final(hash, &md5ctx);
					make_digest(HA1, hash);
				}

				CRX_MD5Init(&md5ctx);
				CRX_MD5Update(&md5ctx, (unsigned char*)"POST:", sizeof("POST:")-1);
				if (crxurl->path) {
					CRX_MD5Update(&md5ctx, (unsigned char*)ZSTR_VAL(crxurl->path), ZSTR_LEN(crxurl->path));
				} else {
					CRX_MD5Update(&md5ctx, (unsigned char*)"/", 1);
				}
				if (crxurl->query) {
					CRX_MD5Update(&md5ctx, (unsigned char*)"?", 1);
					CRX_MD5Update(&md5ctx, (unsigned char*)ZSTR_VAL(crxurl->query), ZSTR_LEN(crxurl->query));
				}

				CRX_MD5Final(hash, &md5ctx);
				make_digest(HA2, hash);

				CRX_MD5Init(&md5ctx);
				CRX_MD5Update(&md5ctx, (unsigned char*)HA1, 32);
				CRX_MD5Update(&md5ctx, (unsigned char*)":", 1);
				if ((tmp = crex_hash_str_find(C_ARRVAL_P(digest), "nonce", sizeof("nonce")-1)) != NULL &&
					C_TYPE_P(tmp) == IS_STRING) {
					CRX_MD5Update(&md5ctx, (unsigned char*)C_STRVAL_P(tmp), C_STRLEN_P(tmp));
				}
				CRX_MD5Update(&md5ctx, (unsigned char*)":", 1);
				if ((tmp = crex_hash_str_find(C_ARRVAL_P(digest), "qop", sizeof("qop")-1)) != NULL &&
					C_TYPE_P(tmp) == IS_STRING) {
					CRX_MD5Update(&md5ctx, (unsigned char*)nc, 8);
					CRX_MD5Update(&md5ctx, (unsigned char*)":", 1);
					CRX_MD5Update(&md5ctx, (unsigned char*)cnonce, 8);
					CRX_MD5Update(&md5ctx, (unsigned char*)":", 1);
					/* TODO: Support for qop="auth-int" */
					CRX_MD5Update(&md5ctx, (unsigned char*)"auth", sizeof("auth")-1);
					CRX_MD5Update(&md5ctx, (unsigned char*)":", 1);
				}
				CRX_MD5Update(&md5ctx, (unsigned char*)HA2, 32);
				CRX_MD5Final(hash, &md5ctx);
				make_digest(response, hash);

				smart_str_append_const(&soap_headers, "Authorization: Digest username=\"");
				smart_str_appendl(&soap_headers, C_STRVAL_P(login), C_STRLEN_P(login));
				if ((tmp = crex_hash_str_find(C_ARRVAL_P(digest), "realm", sizeof("realm")-1)) != NULL &&
					C_TYPE_P(tmp) == IS_STRING) {
					smart_str_append_const(&soap_headers, "\", realm=\"");
					smart_str_appendl(&soap_headers, C_STRVAL_P(tmp), C_STRLEN_P(tmp));
				}
				if ((tmp = crex_hash_str_find(C_ARRVAL_P(digest), "nonce", sizeof("nonce")-1)) != NULL &&
					C_TYPE_P(tmp) == IS_STRING) {
					smart_str_append_const(&soap_headers, "\", nonce=\"");
					smart_str_appendl(&soap_headers, C_STRVAL_P(tmp), C_STRLEN_P(tmp));
				}
				smart_str_append_const(&soap_headers, "\", uri=\"");
				if (crxurl->path) {
					smart_str_appends(&soap_headers, ZSTR_VAL(crxurl->path));
				} else {
					smart_str_appendc(&soap_headers, '/');
				}
				if (crxurl->query) {
					smart_str_appendc(&soap_headers, '?');
					smart_str_appends(&soap_headers, ZSTR_VAL(crxurl->query));
				}
				if (crxurl->fragment) {
					smart_str_appendc(&soap_headers, '#');
					smart_str_appends(&soap_headers, ZSTR_VAL(crxurl->fragment));
				}
				if ((tmp = crex_hash_str_find(C_ARRVAL_P(digest), "qop", sizeof("qop")-1)) != NULL &&
					C_TYPE_P(tmp) == IS_STRING) {
				/* TODO: Support for qop="auth-int" */
					smart_str_append_const(&soap_headers, "\", qop=\"auth");
					smart_str_append_const(&soap_headers, "\", nc=\"");
					smart_str_appendl(&soap_headers, nc, 8);
					smart_str_append_const(&soap_headers, "\", cnonce=\"");
					smart_str_appendl(&soap_headers, cnonce, 8);
				}
				smart_str_append_const(&soap_headers, "\", response=\"");
				smart_str_appendl(&soap_headers, response, 32);
				if ((tmp = crex_hash_str_find(C_ARRVAL_P(digest), "opaque", sizeof("opaque")-1)) != NULL &&
					C_TYPE_P(tmp) == IS_STRING) {
					smart_str_append_const(&soap_headers, "\", opaque=\"");
					smart_str_appendl(&soap_headers, C_STRVAL_P(tmp), C_STRLEN_P(tmp));
				}
				if ((tmp = crex_hash_str_find(C_ARRVAL_P(digest), "algorithm", sizeof("algorithm")-1)) != NULL &&
					C_TYPE_P(tmp) == IS_STRING) {
					smart_str_append_const(&soap_headers, "\", algorithm=\"");
					smart_str_appendl(&soap_headers, C_STRVAL_P(tmp), C_STRLEN_P(tmp));
				}
				smart_str_append_const(&soap_headers, "\"\r\n");
			} else {
				crex_string *buf;

				smart_str auth = {0};
				smart_str_append(&auth, C_STR_P(login));
				smart_str_appendc(&auth, ':');
				password = C_CLIENT_PASSWORD_P(this_ptr);
				if (C_TYPE_P(password) == IS_STRING) {
					smart_str_append(&auth, C_STR_P(password));
				}
				smart_str_0(&auth);
				buf = crx_base64_encode((unsigned char*)ZSTR_VAL(auth.s), ZSTR_LEN(auth.s));
				smart_str_append_const(&soap_headers, "Authorization: Basic ");
				smart_str_append(&soap_headers, buf);
				smart_str_append_const(&soap_headers, "\r\n");
				crex_string_release_ex(buf, 0);
				smart_str_free(&auth);
			}
		}

		/* Proxy HTTP Authentication */
		if (use_proxy && !use_ssl) {
			has_proxy_authorization = proxy_authentication(this_ptr, &soap_headers);
		}

		/* Send cookies along with request */
		cookies = C_CLIENT_COOKIES_P(this_ptr);
		CREX_ASSERT(C_TYPE_P(cookies) == IS_ARRAY);
		if (crex_hash_num_elements(C_ARRVAL_P(cookies)) != 0 && !HT_IS_PACKED(C_ARRVAL_P(cookies))) {
			zval *data;
			crex_string *key;
			has_cookies = 1;
			smart_str_append_const(&soap_headers, "Cookie: ");
			CREX_HASH_MAP_FOREACH_STR_KEY_VAL(C_ARRVAL_P(cookies), key, data) {
				if (key && C_TYPE_P(data) == IS_ARRAY) {
					zval *value;

					if ((value = crex_hash_index_find(C_ARRVAL_P(data), 0)) != NULL &&
						C_TYPE_P(value) == IS_STRING) {
					  zval *tmp;
					  if (((tmp = crex_hash_index_find(C_ARRVAL_P(data), 1)) == NULL ||
						   C_TYPE_P(tmp) != IS_STRING ||
						   strncmp(crxurl->path?ZSTR_VAL(crxurl->path):"/",C_STRVAL_P(tmp),C_STRLEN_P(tmp)) == 0) &&
						  ((tmp = crex_hash_index_find(C_ARRVAL_P(data), 2)) == NULL ||
						   C_TYPE_P(tmp) != IS_STRING ||
						   in_domain(ZSTR_VAL(crxurl->host),C_STRVAL_P(tmp))) &&
						  (use_ssl || (tmp = crex_hash_index_find(C_ARRVAL_P(data), 3)) == NULL)) {
							smart_str_append(&soap_headers, key);
							smart_str_appendc(&soap_headers, '=');
							smart_str_append(&soap_headers, C_STR_P(value));
							smart_str_appendc(&soap_headers, ';');
						}
					}
				}
			} CREX_HASH_FOREACH_END();
			smart_str_append_const(&soap_headers, "\r\n");
		}

		http_context_headers(context, has_authorization, has_proxy_authorization, has_cookies, &soap_headers);

		smart_str_append_const(&soap_headers, "\r\n");
		smart_str_0(&soap_headers);
		if (C_TYPE_P(C_CLIENT_TRACE_P(this_ptr)) == IS_TRUE) {
			zval_ptr_dtor(C_CLIENT_LAST_REQUEST_HEADERS_P(this_ptr));
			/* Need to copy the string here, as we continue appending to soap_headers below. */
			ZVAL_STRINGL(C_CLIENT_LAST_REQUEST_HEADERS_P(this_ptr),
				ZSTR_VAL(soap_headers.s), ZSTR_LEN(soap_headers.s));
		}
		smart_str_appendl(&soap_headers, request->val, request->len);
		smart_str_0(&soap_headers);

		err = crx_stream_write(stream, ZSTR_VAL(soap_headers.s), ZSTR_LEN(soap_headers.s));
		if (err != ZSTR_LEN(soap_headers.s)) {
			if (request != buf) {
				crex_string_release_ex(request, 0);
			}
			crx_stream_close(stream);
			convert_to_null(C_CLIENT_HTTPURL_P(this_ptr));
			convert_to_null(C_CLIENT_HTTPSOCKET_P(this_ptr));
			convert_to_null(C_CLIENT_USE_PROXY_P(this_ptr));
			add_soap_fault(this_ptr, "HTTP", "Failed Sending HTTP SOAP request", NULL, NULL);
			smart_str_free(&soap_headers_z);
			return FALSE;
		}
		smart_str_free(&soap_headers);
	} else {
		add_soap_fault(this_ptr, "HTTP", "Failed to create stream??", NULL, NULL);
		smart_str_free(&soap_headers_z);
		return FALSE;
	}

	if (!return_value) {
		crx_stream_close(stream);
		convert_to_null(C_CLIENT_HTTPSOCKET_P(this_ptr));
		convert_to_null(C_CLIENT_USE_PROXY_P(this_ptr));
		smart_str_free(&soap_headers_z);
		return TRUE;
	}

	do {
		http_headers = get_http_headers(stream);
		if (!http_headers) {
			if (request != buf) {
				crex_string_release_ex(request, 0);
			}
			crx_stream_close(stream);
			convert_to_null(C_CLIENT_HTTPSOCKET_P(this_ptr));
			convert_to_null(C_CLIENT_USE_PROXY_P(this_ptr));
			add_soap_fault(this_ptr, "HTTP", "Error Fetching http headers", NULL, NULL);
			smart_str_free(&soap_headers_z);
			return FALSE;
		}

		if (C_TYPE_P(C_CLIENT_TRACE_P(this_ptr)) == IS_TRUE) {
			zval_ptr_dtor(C_CLIENT_LAST_RESPONSE_HEADERS_P(this_ptr));
			ZVAL_STR_COPY(C_CLIENT_LAST_RESPONSE_HEADERS_P(this_ptr), http_headers);
		}

		/* Check to see what HTTP status was sent */
		http_1_1 = 0;
		http_status = 0;
		http_version = get_http_header_value(ZSTR_VAL(http_headers), "HTTP/");
		if (http_version) {
			char *tmp;

			if (!strncmp(http_version,"1.1", 3)) {
				http_1_1 = 1;
			}

			tmp = strstr(http_version," ");
			if (tmp != NULL) {
				tmp++;
				http_status = atoi(tmp);
			}
			tmp = strstr(tmp," ");
			if (tmp != NULL) {
				tmp++;
				if (http_msg) {
					efree(http_msg);
				}
				http_msg = estrdup(tmp);
			}
			efree(http_version);

			/* Try and get headers again */
			if (http_status == 100) {
				crex_string_release_ex(http_headers, 0);
			}
		}
	} while (http_status == 100);

	/* Grab and send back every cookie */

	/* Not going to worry about Path: because
	   we shouldn't be changing urls so path doesn't
	   matter too much
	*/
	cookie_itt = ZSTR_VAL(http_headers);

	while ((cookie_itt = get_http_header_value_nodup(cookie_itt, "Set-Cookie: ", &cookie_len))) {
		zval *cookies = C_CLIENT_COOKIES_P(this_ptr);
		SEPARATE_ARRAY(cookies);

		char *cookie = estrndup(cookie_itt, cookie_len);
		char *eqpos = strstr(cookie, "=");
		char *sempos = strstr(cookie, ";");
		if (eqpos != NULL && (sempos == NULL || sempos > eqpos)) {
			smart_str name = {0};
			int cookie_len;
			zval zcookie;

			if (sempos != NULL) {
				cookie_len = sempos-(eqpos+1);
			} else {
				cookie_len = strlen(cookie)-(eqpos-cookie)-1;
			}

			smart_str_appendl(&name, cookie, eqpos - cookie);
			smart_str_0(&name);

			array_init(&zcookie);
			add_index_stringl(&zcookie, 0, eqpos + 1, cookie_len);

			if (sempos != NULL) {
				char *options = cookie + cookie_len+1;
				while (*options) {
					while (*options == ' ') {options++;}
					sempos = strstr(options, ";");
					if (strstr(options,"path=") == options) {
						eqpos = options + sizeof("path=")-1;
						add_index_stringl(&zcookie, 1, eqpos, sempos?(size_t)(sempos-eqpos):strlen(eqpos));
					} else if (strstr(options,"domain=") == options) {
						eqpos = options + sizeof("domain=")-1;
						add_index_stringl(&zcookie, 2, eqpos, sempos?(size_t)(sempos-eqpos):strlen(eqpos));
					} else if (strstr(options,"secure") == options) {
						add_index_bool(&zcookie, 3, 1);
					}
					if (sempos != NULL) {
						options = sempos+1;
					} else {
					  break;
					}
				}
			}
			if (!crex_hash_index_exists(C_ARRVAL(zcookie), 1)) {
				char *t = crxurl->path?ZSTR_VAL(crxurl->path):"/";
				char *c = strrchr(t, '/');
				if (c) {
					add_index_stringl(&zcookie, 1, t, c-t);
				}
			}
			if (!crex_hash_index_exists(C_ARRVAL(zcookie), 2)) {
				add_index_str(&zcookie, 2, crxurl->host);
				GC_ADDREF(crxurl->host);
			}

			crex_symtable_update(C_ARRVAL_P(cookies), name.s, &zcookie);
			smart_str_free(&name);
		}

		cookie_itt = cookie_itt + cookie_len;
		efree(cookie);
	}

	/* See if the server requested a close */
	if (http_1_1) {
		http_close = FALSE;
		if (use_proxy && !use_ssl) {
			connection = get_http_header_value(ZSTR_VAL(http_headers), "Proxy-Connection: ");
			if (connection) {
				if (strncasecmp(connection, "close", sizeof("close")-1) == 0) {
					http_close = TRUE;
				}
				efree(connection);
			}
		}
		if (http_close == FALSE) {
			connection = get_http_header_value(ZSTR_VAL(http_headers), "Connection: ");
			if (connection) {
				if (strncasecmp(connection, "close", sizeof("close")-1) == 0) {
					http_close = TRUE;
				}
				efree(connection);
			}
		}
	} else {
		http_close = TRUE;
		if (use_proxy && !use_ssl) {
			connection = get_http_header_value(ZSTR_VAL(http_headers), "Proxy-Connection: ");
			if (connection) {
				if (strncasecmp(connection, "Keep-Alive", sizeof("Keep-Alive")-1) == 0) {
					http_close = FALSE;
				}
				efree(connection);
			}
		}
		if (http_close == TRUE) {
			connection = get_http_header_value(ZSTR_VAL(http_headers), "Connection: ");
			if (connection) {
				if (strncasecmp(connection, "Keep-Alive", sizeof("Keep-Alive")-1) == 0) {
					http_close = FALSE;
				}
				efree(connection);
			}
		}
	}


	http_body = get_http_body(stream, http_close, ZSTR_VAL(http_headers));
	if (!http_body) {
		if (request != buf) {
			crex_string_release_ex(request, 0);
		}
		crx_stream_close(stream);
		crex_string_release_ex(http_headers, 0);
		convert_to_null(C_CLIENT_HTTPSOCKET_P(this_ptr));
		convert_to_null(C_CLIENT_USE_PROXY_P(this_ptr));
		add_soap_fault(this_ptr, "HTTP", "Error Fetching http body, No Content-Length, connection closed or chunked data", NULL, NULL);
		if (http_msg) {
			efree(http_msg);
		}
		smart_str_free(&soap_headers_z);
		return FALSE;
	}

	if (request != buf) {
		crex_string_release_ex(request, 0);
	}

	if (http_close) {
		crx_stream_close(stream);
		convert_to_null(C_CLIENT_HTTPSOCKET_P(this_ptr));
		convert_to_null(C_CLIENT_USE_PROXY_P(this_ptr));
		stream = NULL;
	}

	/* Process HTTP status codes */
	if (http_status >= 300 && http_status < 400) {
		char *loc;

		if ((loc = get_http_header_value(ZSTR_VAL(http_headers), "Location: ")) != NULL) {
			crx_url *new_url  = crx_url_parse(loc);

			if (new_url != NULL) {
				crex_string_release_ex(http_headers, 0);
				crex_string_release_ex(http_body, 0);
				efree(loc);
				if (new_url->scheme == NULL && new_url->path != NULL) {
					new_url->scheme = crxurl->scheme ? crex_string_copy(crxurl->scheme) : NULL;
					new_url->host = crxurl->host ? crex_string_copy(crxurl->host) : NULL;
					new_url->port = crxurl->port;
					if (new_url->path && ZSTR_VAL(new_url->path)[0] != '/') {
						if (crxurl->path) {
							char *t = ZSTR_VAL(crxurl->path);
							char *p = strrchr(t, '/');
							if (p) {
								crex_string *s = crex_string_alloc((p - t) + ZSTR_LEN(new_url->path) + 2, 0);
								strncpy(ZSTR_VAL(s), t, (p - t) + 1);
								ZSTR_VAL(s)[(p - t) + 1] = 0;
								strcat(ZSTR_VAL(s), ZSTR_VAL(new_url->path));
								crex_string_release_ex(new_url->path, 0);
								new_url->path = s;
							}
						} else {
							crex_string *s = crex_string_alloc(ZSTR_LEN(new_url->path) + 2, 0);
							ZSTR_VAL(s)[0] = '/';
							ZSTR_VAL(s)[1] = 0;
							strcat(ZSTR_VAL(s), ZSTR_VAL(new_url->path));
							crex_string_release_ex(new_url->path, 0);
							new_url->path = s;
						}
					}
				}
				crxurl = new_url;

				if (--redirect_max < 1) {
					add_soap_fault(this_ptr, "HTTP", "Redirection limit reached, aborting", NULL, NULL);
					smart_str_free(&soap_headers_z);
					return FALSE;
				}

				goto try_again;
			}
		}
	} else if (http_status == 401) {
		/* Digest authentication */
		zval *digest = C_CLIENT_DIGEST_P(this_ptr);
		zval *login = C_CLIENT_LOGIN_P(this_ptr);
		zval *password = C_CLIENT_PASSWORD_P(this_ptr);
		char *auth = get_http_header_value(ZSTR_VAL(http_headers), "WWW-Authenticate: ");
		if (auth && strstr(auth, "Digest") == auth && C_TYPE_P(digest) != IS_ARRAY
				&& C_TYPE_P(login) == IS_STRING && C_TYPE_P(password) == IS_STRING) {
			char *s;
			zval digest;

			ZVAL_UNDEF(&digest);
			s = auth + sizeof("Digest")-1;
			while (*s != '\0') {
				char *name, *val;
				while (*s == ' ') ++s;
				name = s;
				while (*s != '\0' && *s != '=') ++s;
				if (*s == '=') {
					*s = '\0';
					++s;
					if (*s == '"') {
						++s;
						val = s;
						while (*s != '\0' && *s != '"') ++s;
					} else {
						val = s;
						while (*s != '\0' && *s != ' ' && *s != ',') ++s;
					}
					if (*s != '\0') {
						if (*s != ',') {
							*s = '\0';
							++s;
							while (*s != '\0' && *s != ',') ++s;
							if (*s != '\0') ++s;
						} else {
							*s = '\0';
							++s;
						}
					}
					if (C_TYPE(digest) == IS_UNDEF) {
						array_init(&digest);
					}
					add_assoc_string(&digest, name, val);
				}
			}

			if (C_TYPE(digest) != IS_UNDEF) {
				crx_url *new_url = emalloc(sizeof(crx_url));

				zval_ptr_dtor(C_CLIENT_DIGEST_P(this_ptr));
				ZVAL_COPY_VALUE(C_CLIENT_DIGEST_P(this_ptr), &digest);

				*new_url = *crxurl;
				if (crxurl->scheme) crxurl->scheme = crex_string_copy(crxurl->scheme);
				if (crxurl->user) crxurl->user = crex_string_copy(crxurl->user);
				if (crxurl->pass) crxurl->pass = crex_string_copy(crxurl->pass);
				if (crxurl->host) crxurl->host = crex_string_copy(crxurl->host);
				if (crxurl->path) crxurl->path = crex_string_copy(crxurl->path);
				if (crxurl->query) crxurl->query = crex_string_copy(crxurl->query);
				if (crxurl->fragment) crxurl->fragment = crex_string_copy(crxurl->fragment);
				crxurl = new_url;

				efree(auth);
				crex_string_release_ex(http_headers, 0);
				crex_string_release_ex(http_body, 0);

				goto try_again;
			}
		}
		if (auth) efree(auth);
	}
	smart_str_free(&soap_headers_z);

	/* Check and see if the server even sent a xml document */
	content_type = get_http_header_value(ZSTR_VAL(http_headers), "Content-Type: ");
	if (content_type) {
		char *pos = NULL;
		int cmplen;
		pos = strstr(content_type,";");
		if (pos != NULL) {
			cmplen = pos - content_type;
		} else {
			cmplen = strlen(content_type);
		}
		if (strncmp(content_type, "text/xml", cmplen) == 0 ||
		    strncmp(content_type, "application/soap+xml", cmplen) == 0) {
			content_type_xml = 1;
/*
			if (strncmp(http_body, "<?xml", 5)) {
				zval *err;
				MAKE_STD_ZVAL(err);
				ZVAL_STRINGL(err, http_body, http_body_size, 1);
				add_soap_fault(this_ptr, "HTTP", "Didn't receive an xml document", NULL, err);
				efree(content_type);
				crex_string_release_ex(http_headers, 0);
				efree(http_body);
				return FALSE;
			}
*/
		}
		efree(content_type);
	}

	/* Decompress response */
	content_encoding = get_http_header_value(ZSTR_VAL(http_headers), "Content-Encoding: ");
	if (content_encoding) {
		zval func;
		zval retval;
		zval params[1];

		if ((strcmp(content_encoding,"gzip") == 0 ||
		     strcmp(content_encoding,"x-gzip") == 0) &&
		     crex_hash_str_exists(EG(function_table), "gzinflate", sizeof("gzinflate")-1)) {
			ZVAL_STRING(&func, "gzinflate");
			ZVAL_STRINGL(&params[0], http_body->val+10, http_body->len-10);
		} else if (strcmp(content_encoding,"deflate") == 0 &&
		           crex_hash_str_exists(EG(function_table), "gzuncompress", sizeof("gzuncompress")-1)) {
			ZVAL_STRING(&func, "gzuncompress");
			ZVAL_STR_COPY(&params[0], http_body);
		} else {
			efree(content_encoding);
			crex_string_release_ex(http_headers, 0);
			crex_string_release_ex(http_body, 0);
			if (http_msg) {
				efree(http_msg);
			}
			add_soap_fault(this_ptr, "HTTP", "Unknown Content-Encoding", NULL, NULL);
			return FALSE;
		}
		if (call_user_function(CG(function_table), (zval*)NULL, &func, &retval, 1, params) == SUCCESS &&
		    C_TYPE(retval) == IS_STRING) {
			zval_ptr_dtor(&params[0]);
			zval_ptr_dtor(&func);
			crex_string_release_ex(http_body, 0);
			ZVAL_COPY_VALUE(return_value, &retval);
		} else {
			zval_ptr_dtor(&params[0]);
			zval_ptr_dtor(&func);
			efree(content_encoding);
			crex_string_release_ex(http_headers, 0);
			crex_string_release_ex(http_body, 0);
			add_soap_fault(this_ptr, "HTTP", "Can't uncompress compressed response", NULL, NULL);
			if (http_msg) {
				efree(http_msg);
			}
			return FALSE;
		}
		efree(content_encoding);
	} else {
		ZVAL_STR(return_value, http_body);
	}

	crex_string_release_ex(http_headers, 0);

	if (http_status >= 400) {
		int error = 0;

		if (C_STRLEN_P(return_value) == 0) {
			error = 1;
		} else if (C_STRLEN_P(return_value) > 0) {
			if (!content_type_xml) {
				char *s = C_STRVAL_P(return_value);

				while (*s != '\0' && *s < ' ') {
					s++;
				}
				if (strncmp(s, "<?xml", 5)) {
					error = 1;
				}
			}
		}

		if (error) {
			zval_ptr_dtor(return_value);
			ZVAL_UNDEF(return_value);
			add_soap_fault(this_ptr, "HTTP", http_msg, NULL, NULL);
			efree(http_msg);
			return FALSE;
		}
	}

	if (http_msg) {
		efree(http_msg);
	}

	return TRUE;
}

static char *get_http_header_value_nodup(char *headers, char *type, size_t *len)
{
	char *pos, *tmp = NULL;
	int typelen, headerslen;

	typelen = strlen(type);
	headerslen = strlen(headers);

	/* header `titles' can be lower case, or any case combination, according
	 * to the various RFC's. */
	pos = headers;
	do {
		/* start of buffer or start of line */
		if (strncasecmp(pos, type, typelen) == 0) {
			char *eol;

			/* match */
			tmp = pos + typelen;

			/* strip leading whitespace */
			while (*tmp == ' ' || *tmp == '\t') {
				tmp++;
			}

			eol = strchr(tmp, '\n');
			if (eol == NULL) {
				eol = headers + headerslen;
			} else if (eol > tmp) {
				if (*(eol-1) == '\r') {
					eol--;
				}

				/* strip trailing whitespace */
				while (eol > tmp && (*(eol-1) == ' ' || *(eol-1) == '\t')) {
					eol--;
				}
			}

			*len = eol - tmp;
			return tmp;
		}

		/* find next line */
		pos = strchr(pos, '\n');
		if (pos) {
			pos++;
		}

	} while (pos);

	return NULL;
}

static char *get_http_header_value(char *headers, char *type)
{
	size_t len;
	char *value;

	value = get_http_header_value_nodup(headers, type, &len);

	if (value) {
		return estrndup(value, len);
	}

	return NULL;
}

static crex_string* get_http_body(crx_stream *stream, int close, char *headers)
{
	crex_string *http_buf = NULL;
	char *header;
	int header_close = close, header_chunked = 0, header_length = 0, http_buf_size = 0;

	if (!close) {
		header = get_http_header_value(headers, "Connection: ");
		if (header) {
			if(!strncasecmp(header, "close", sizeof("close")-1)) header_close = 1;
			efree(header);
		}
	}
	header = get_http_header_value(headers, "Transfer-Encoding: ");
	if (header) {
		if(!strncasecmp(header, "chunked", sizeof("chunked")-1)) header_chunked = 1;
		efree(header);
	}
	header = get_http_header_value(headers, "Content-Length: ");
	if (header) {
		header_length = atoi(header);
		efree(header);
		if (!header_length && !header_chunked) {
			/* Empty response */
			return ZSTR_EMPTY_ALLOC();
		}
	}

	if (header_chunked) {
		char ch, done, headerbuf[8192];

		done = FALSE;

		while (!done) {
			int buf_size = 0;

			crx_stream_gets(stream, headerbuf, sizeof(headerbuf));
			if (sscanf(headerbuf, "%x", &buf_size) > 0 ) {
				if (buf_size > 0) {
					size_t len_size = 0;

					if (http_buf_size + buf_size + 1 < 0) {
						if (http_buf) {
							crex_string_release_ex(http_buf, 0);
						}
						return NULL;
					}

					if (http_buf) {
						http_buf = crex_string_realloc(http_buf, http_buf_size + buf_size, 0);
					} else {
						http_buf = crex_string_alloc(buf_size, 0);
					}

					while (len_size < buf_size) {
						ssize_t len_read = crx_stream_read(stream, http_buf->val + http_buf_size, buf_size - len_size);
						if (len_read <= 0) {
							/* Error or EOF */
							done = TRUE;
						  break;
						}
						len_size += len_read;
	 					http_buf_size += len_read;
					}

					/* Eat up '\r' '\n' */
					ch = crx_stream_getc(stream);
					if (ch == '\r') {
						ch = crx_stream_getc(stream);
					}
					if (ch != '\n') {
						/* Something wrong in chunked encoding */
						if (http_buf) {
							crex_string_release_ex(http_buf, 0);
						}
						return NULL;
					}
				}
			} else {
				/* Something wrong in chunked encoding */
				if (http_buf) {
					crex_string_release_ex(http_buf, 0);
				}
				return NULL;
			}
			if (buf_size == 0) {
				done = TRUE;
			}
		}

		/* Ignore trailer headers */
		while (1) {
			if (!crx_stream_gets(stream, headerbuf, sizeof(headerbuf))) {
				break;
			}

			if ((headerbuf[0] == '\r' && headerbuf[1] == '\n') ||
			    (headerbuf[0] == '\n')) {
				/* empty line marks end of headers */
				break;
			}
		}

		if (http_buf == NULL) {
			return ZSTR_EMPTY_ALLOC();
		}

	} else if (header_length) {
		if (header_length < 0 || header_length >= INT_MAX) {
			return NULL;
		}
		http_buf = crex_string_alloc(header_length, 0);
		while (http_buf_size < header_length) {
			ssize_t len_read = crx_stream_read(stream, http_buf->val + http_buf_size, header_length - http_buf_size);
			if (len_read <= 0) {
				break;
			}
			http_buf_size += len_read;
		}
	} else if (header_close) {
		do {
			ssize_t len_read;
			if (http_buf) {
				http_buf = crex_string_realloc(http_buf, http_buf_size + 4096, 0);
			} else {
				http_buf = crex_string_alloc(4096, 0);
			}
			len_read = crx_stream_read(stream, http_buf->val + http_buf_size, 4096);
			if (len_read > 0) {
				http_buf_size += len_read;
			}
		} while(!crx_stream_eof(stream));
	} else {
		return NULL;
	}

	http_buf->val[http_buf_size] = '\0';
	http_buf->len = http_buf_size;
	return http_buf;
}

static crex_string *get_http_headers(crx_stream *stream)
{
	smart_str tmp_response = {0};
	char headerbuf[8192];

	while (crx_stream_gets(stream, headerbuf, sizeof(headerbuf))) {
		if ((headerbuf[0] == '\r' && headerbuf[1] == '\n') ||
		    (headerbuf[0] == '\n')) {
			/* empty line marks end of headers */
			smart_str_0(&tmp_response);
			return tmp_response.s;
		}

		/* add header to collection */
		smart_str_appends(&tmp_response, headerbuf);
	}

	smart_str_free(&tmp_response);
	return NULL;
}
