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
   | Authors: Stig Venaas <venaas@crx.net>                                |
   |          Wez Furlong <wez@thebrainroom.com                           |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_OPENSSL_H
#define CRX_OPENSSL_H
/* HAVE_OPENSSL would include SSL MySQL stuff */
#ifdef HAVE_OPENSSL_EXT
extern crex_module_entry openssl_module_entry;
#define crxext_openssl_ptr &openssl_module_entry

#include "crx_version.h"
#define CRX_OPENSSL_VERSION CRX_VERSION

#include <openssl/opensslv.h>
#if defined(LIBRESSL_VERSION_NUMBER)
/* LibreSSL version check */
#if LIBRESSL_VERSION_NUMBER < 0x20700000L
#define CRX_OPENSSL_API_VERSION 0x10001
#else
#define CRX_OPENSSL_API_VERSION 0x10100
#endif
#else
/* OpenSSL version check */
#if OPENSSL_VERSION_NUMBER < 0x10100000L
#define CRX_OPENSSL_API_VERSION 0x10002
#elif OPENSSL_VERSION_NUMBER < 0x30000000L
#define CRX_OPENSSL_API_VERSION 0x10100
#else
#define CRX_OPENSSL_API_VERSION 0x30000
#endif
#endif

#define OPENSSL_RAW_DATA 1
#define OPENSSL_ZERO_PADDING 2
#define OPENSSL_DONT_ZERO_PAD_KEY 4

#define OPENSSL_ERROR_X509_PRIVATE_KEY_VALUES_MISMATCH 0x0B080074

/* Used for client-initiated handshake renegotiation DoS protection*/
#define OPENSSL_DEFAULT_RENEG_LIMIT 2
#define OPENSSL_DEFAULT_RENEG_WINDOW 300
#define OPENSSL_DEFAULT_STREAM_VERIFY_DEPTH 9
#define OPENSSL_DEFAULT_STREAM_CIPHERS "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:" \
	"ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:" \
	"DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:" \
	"ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:" \
	"ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:" \
	"DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:" \
	"AES256-GCM-SHA384:AES128:AES256:HIGH:!SSLv2:!aNULL:!eNULL:!EXPORT:!DES:!MD5:!RC4:!ADH"

#include <openssl/err.h>

#ifdef CRX_WIN32
#	define CRX_OPENSSL_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define CRX_OPENSSL_API __attribute__((visibility("default")))
#else
#	define CRX_OPENSSL_API
#endif

struct crx_openssl_errors {
	int buffer[ERR_NUM_ERRORS];
	int top;
	int bottom;
};

CREX_BEGIN_MODULE_GLOBALS(openssl)
	struct crx_openssl_errors *errors;
	struct crx_openssl_errors *errors_mark;
CREX_END_MODULE_GLOBALS(openssl)

#define OPENSSL_G(v) CREX_MODULE_GLOBALS_ACCESSOR(openssl, v)

#if defined(ZTS) && defined(COMPILE_DL_OPENSSL)
CREX_TSRMLS_CACHE_EXTERN();
#endif

crx_stream_transport_factory_func crx_openssl_ssl_socket_factory;

void crx_openssl_store_errors(void);

/* openssl file path extra */
bool crx_openssl_check_path_ex(
		const char *file_path, size_t file_path_len, char *real_path, uint32_t arg_num,
		bool contains_file_protocol, bool is_from_array, const char *option_name);

/* openssl file path check */
static inline bool crx_openssl_check_path(
		const char *file_path, size_t file_path_len, char *real_path, uint32_t arg_num)
{
	return crx_openssl_check_path_ex(
			file_path, file_path_len, real_path, arg_num, false, false, NULL);
}

/* openssl file path extra check with crex string */
static inline bool crx_openssl_check_path_str_ex(
		crex_string *file_path, char *real_path, uint32_t arg_num,
		bool contains_file_protocol, bool is_from_array, const char *option_name)
{
	return crx_openssl_check_path_ex(
			ZSTR_VAL(file_path), ZSTR_LEN(file_path), real_path, arg_num, contains_file_protocol,
			is_from_array, option_name);
}

/* openssl file path check with crex string */
static inline bool crx_openssl_check_path_str(
		crex_string *file_path, char *real_path, uint32_t arg_num)
{
	return crx_openssl_check_path_str_ex(file_path, real_path, arg_num, true, false, NULL);
}

CRX_OPENSSL_API crex_long crx_openssl_cipher_iv_length(const char *method);
CRX_OPENSSL_API crex_long crx_openssl_cipher_key_length(const char *method);
CRX_OPENSSL_API crex_string* crx_openssl_random_pseudo_bytes(crex_long length);
CRX_OPENSSL_API crex_string* crx_openssl_encrypt(
	const char *data, size_t data_len,
	const char *method, size_t method_len,
	const char *password, size_t password_len,
	crex_long options,
	const char *iv, size_t iv_len,
	zval *tag, crex_long tag_len,
	const char *aad, size_t aad_len);
CRX_OPENSSL_API crex_string* crx_openssl_decrypt(
	const char *data, size_t data_len,
	const char *method, size_t method_len,
	const char *password, size_t password_len,
	crex_long options,
	const char *iv, size_t iv_len,
	const char *tag, crex_long tag_len,
	const char *aad, size_t aad_len);

/* OpenSSLCertificate class */

typedef struct _crx_openssl_certificate_object {
	X509 *x509;
	crex_object std;
} crx_openssl_certificate_object;

extern crex_class_entry *crx_openssl_certificate_ce;

static inline crx_openssl_certificate_object *crx_openssl_certificate_from_obj(crex_object *obj) {
	return (crx_openssl_certificate_object *)((char *)(obj) - XtOffsetOf(crx_openssl_certificate_object, std));
}

#define C_OPENSSL_CERTIFICATE_P(zv) crx_openssl_certificate_from_obj(C_OBJ_P(zv))

CRX_MINIT_FUNCTION(openssl);
CRX_MSHUTDOWN_FUNCTION(openssl);
CRX_MINFO_FUNCTION(openssl);
CRX_GINIT_FUNCTION(openssl);
CRX_GSHUTDOWN_FUNCTION(openssl);

#ifdef CRX_WIN32
#define CRX_OPENSSL_BIO_MODE_R(flags) (((flags) & PKCS7_BINARY) ? "rb" : "r")
#define CRX_OPENSSL_BIO_MODE_W(flags) (((flags) & PKCS7_BINARY) ? "wb" : "w")
#else
#define CRX_OPENSSL_BIO_MODE_R(flags) "r"
#define CRX_OPENSSL_BIO_MODE_W(flags) "w"
#endif

#else

#define crxext_openssl_ptr NULL

#endif


#endif
