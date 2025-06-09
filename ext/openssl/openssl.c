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
   |          Wez Furlong <wez@thebrainroom.com>                          |
   |          Sascha Kettler <kettler@gmx.net>                            |
   |          Pierre-Alain Joye <pierre@crx.net>                          |
   |          Marc Delling <delling@silpion.de> (PKCS12 functions)        |
   |          Jakub Zelenka <bukka@crx.net>                               |
   |          Eliot Lear <lear@ofcourseimright.com>                       |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "crx_openssl.h"
#include "crex_attributes.h"
#include "crex_exceptions.h"

/* CRX Includes */
#include "ext/standard/file.h"
#include "ext/standard/info.h"
#include "ext/standard/crx_fopen_wrappers.h"
#include "ext/standard/md5.h"
#include "ext/standard/base64.h"
#ifdef CRX_WIN32
# include "win32/winutil.h"
#endif

/* OpenSSL includes */
#include <openssl/evp.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/dsa.h>
#include <openssl/dh.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/crypto.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/pkcs12.h>
#include <openssl/cms.h>
#if CRX_OPENSSL_API_VERSION >= 0x30000
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L && !defined(OPENSSL_NO_ENGINE)
#include <openssl/engine.h>
#endif

/* Common */
#include <time.h>

#if (defined(CRX_WIN32) && defined(_MSC_VER) && _MSC_VER >= 1900)
#define timezone _timezone	/* timezone is called _timezone in LibC */
#endif

#define MIN_KEY_LENGTH		384

/* constants used in ext/crxa/util.c, keep in sync */
#define OPENSSL_ALGO_SHA1 	1
#define OPENSSL_ALGO_MD5	2
#ifndef OPENSSL_NO_MD4
#define OPENSSL_ALGO_MD4	3
#endif
#ifndef OPENSSL_NO_MD2
#define OPENSSL_ALGO_MD2	4
#endif
#if CRX_OPENSSL_API_VERSION < 0x10100
#define OPENSSL_ALGO_DSS1	5
#endif
#define OPENSSL_ALGO_SHA224 6
#define OPENSSL_ALGO_SHA256 7
#define OPENSSL_ALGO_SHA384 8
#define OPENSSL_ALGO_SHA512 9
#ifndef OPENSSL_NO_RMD160
#define OPENSSL_ALGO_RMD160 10
#endif
#define DEBUG_SMIME	0

#if !defined(OPENSSL_NO_EC) && defined(EVP_PKEY_EC)
#define HAVE_EVP_PKEY_EC 1

/* the OPENSSL_EC_EXPLICIT_CURVE value was added
 * in OpenSSL 1.1.0; previous versions should 
 * use 0 instead.
 */
#ifndef OPENSSL_EC_EXPLICIT_CURVE
#define OPENSSL_EC_EXPLICIT_CURVE 0x000
#endif
#endif

CREX_DECLARE_MODULE_GLOBALS(openssl)

/* FIXME: Use the openssl constants instead of
 * enum. It is now impossible to match real values
 * against crx constants. Also sorry to break the
 * enum principles here, BC...
 */
enum crx_openssl_key_type {
	OPENSSL_KEYTYPE_RSA,
	OPENSSL_KEYTYPE_DSA,
	OPENSSL_KEYTYPE_DH,
	OPENSSL_KEYTYPE_DEFAULT = OPENSSL_KEYTYPE_RSA,
#ifdef HAVE_EVP_PKEY_EC
	OPENSSL_KEYTYPE_EC = OPENSSL_KEYTYPE_DH +1
#endif
};

enum crx_openssl_cipher_type {
	CRX_OPENSSL_CIPHER_RC2_40,
	CRX_OPENSSL_CIPHER_RC2_128,
	CRX_OPENSSL_CIPHER_RC2_64,
	CRX_OPENSSL_CIPHER_DES,
	CRX_OPENSSL_CIPHER_3DES,
	CRX_OPENSSL_CIPHER_AES_128_CBC,
	CRX_OPENSSL_CIPHER_AES_192_CBC,
	CRX_OPENSSL_CIPHER_AES_256_CBC,

	CRX_OPENSSL_CIPHER_DEFAULT = CRX_OPENSSL_CIPHER_AES_128_CBC
};

/* Add some encoding rules.  This is normally handled through filters
 * in the OpenSSL code, but we will do that part as if we were one
 * of the OpenSSL binaries along the lines of -outform {DER|CMS|PEM}
 */
enum crx_openssl_encoding {
	ENCODING_DER,
	ENCODING_SMIME,
	ENCODING_PEM,
};

#include "openssl_arginfo.h"

/* OpenSSLCertificate class */

crex_class_entry *crx_openssl_certificate_ce;

static crex_object_handlers crx_openssl_certificate_object_handlers;

static crex_object *crx_openssl_certificate_create_object(crex_class_entry *class_type) {
	crx_openssl_certificate_object *intern = crex_object_alloc(sizeof(crx_openssl_certificate_object), class_type);

	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	return &intern->std;
}

static crex_function *crx_openssl_certificate_get_constructor(crex_object *object) {
	crex_throw_error(NULL, "Cannot directly construct OpenSSLCertificate, use openssl_x509_read() instead");
	return NULL;
}

static void crx_openssl_certificate_free_obj(crex_object *object)
{
	crx_openssl_certificate_object *x509_object = crx_openssl_certificate_from_obj(object);

	X509_free(x509_object->x509);
	crex_object_std_dtor(&x509_object->std);
}

/* OpenSSLCertificateSigningRequest class */

typedef struct _crx_openssl_x509_request_object {
	X509_REQ *csr;
	crex_object std;
} crx_openssl_request_object;

crex_class_entry *crx_openssl_request_ce;

static inline crx_openssl_request_object *crx_openssl_request_from_obj(crex_object *obj) {
	return (crx_openssl_request_object *)((char *)(obj) - XtOffsetOf(crx_openssl_request_object, std));
}

#define C_OPENSSL_REQUEST_P(zv) crx_openssl_request_from_obj(C_OBJ_P(zv))

static crex_object_handlers crx_openssl_request_object_handlers;

static crex_object *crx_openssl_request_create_object(crex_class_entry *class_type) {
	crx_openssl_request_object *intern = crex_object_alloc(sizeof(crx_openssl_request_object), class_type);

	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	return &intern->std;
}

static crex_function *crx_openssl_request_get_constructor(crex_object *object) {
	crex_throw_error(NULL, "Cannot directly construct OpenSSLCertificateSigningRequest, use openssl_csr_new() instead");
	return NULL;
}

static void crx_openssl_request_free_obj(crex_object *object)
{
	crx_openssl_request_object *x509_request = crx_openssl_request_from_obj(object);

	X509_REQ_free(x509_request->csr);
	crex_object_std_dtor(&x509_request->std);
}

/* OpenSSLAsymmetricKey class */

typedef struct _crx_openssl_pkey_object {
	EVP_PKEY *pkey;
	bool is_private;
	crex_object std;
} crx_openssl_pkey_object;

crex_class_entry *crx_openssl_pkey_ce;

static inline crx_openssl_pkey_object *crx_openssl_pkey_from_obj(crex_object *obj) {
	return (crx_openssl_pkey_object *)((char *)(obj) - XtOffsetOf(crx_openssl_pkey_object, std));
}

#define C_OPENSSL_PKEY_P(zv) crx_openssl_pkey_from_obj(C_OBJ_P(zv))

static crex_object_handlers crx_openssl_pkey_object_handlers;

static crex_object *crx_openssl_pkey_create_object(crex_class_entry *class_type) {
	crx_openssl_pkey_object *intern = crex_object_alloc(sizeof(crx_openssl_pkey_object), class_type);

	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	return &intern->std;
}

static void crx_openssl_pkey_object_init(zval *zv, EVP_PKEY *pkey, bool is_private) {
	object_init_ex(zv, crx_openssl_pkey_ce);
	crx_openssl_pkey_object *obj = C_OPENSSL_PKEY_P(zv);
	obj->pkey = pkey;
	obj->is_private = is_private;
}

static crex_function *crx_openssl_pkey_get_constructor(crex_object *object) {
	crex_throw_error(NULL, "Cannot directly construct OpenSSLAsymmetricKey, use openssl_pkey_new() instead");
	return NULL;
}

static void crx_openssl_pkey_free_obj(crex_object *object)
{
	crx_openssl_pkey_object *key_object = crx_openssl_pkey_from_obj(object);

	EVP_PKEY_free(key_object->pkey);
	crex_object_std_dtor(&key_object->std);
}

/* {{{ openssl_module_entry */
crex_module_entry openssl_module_entry = {
	STANDARD_MODULE_HEADER,
	"openssl",
	ext_functions,
	CRX_MINIT(openssl),
	CRX_MSHUTDOWN(openssl),
	NULL,
	NULL,
	CRX_MINFO(openssl),
	CRX_OPENSSL_VERSION,
	CRX_MODULE_GLOBALS(openssl),
	CRX_GINIT(openssl),
	CRX_GSHUTDOWN(openssl),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_OPENSSL
CREX_GET_MODULE(openssl)
#endif

/* {{{ OpenSSL compatibility functions and macros */
#if CRX_OPENSSL_API_VERSION < 0x10100

#define EVP_PKEY_get0_RSA(_pkey) _pkey->pkey.rsa
#define EVP_PKEY_get0_DH(_pkey) _pkey->pkey.dh
#define EVP_PKEY_get0_DSA(_pkey) _pkey->pkey.dsa
#define EVP_PKEY_get0_EC_KEY(_pkey) _pkey->pkey.ec

static int RSA_set0_key(RSA *r, BIGNUM *n, BIGNUM *e, BIGNUM *d)
{
	r->n = n;
	r->e = e;
	r->d = d;

	return 1;
}

static int RSA_set0_factors(RSA *r, BIGNUM *p, BIGNUM *q)
{
	r->p = p;
	r->q = q;

	return 1;
}

static int RSA_set0_crt_params(RSA *r, BIGNUM *dmp1, BIGNUM *dmq1, BIGNUM *iqmp)
{
	r->dmp1 = dmp1;
	r->dmq1 = dmq1;
	r->iqmp = iqmp;

	return 1;
}

static void RSA_get0_key(const RSA *r, const BIGNUM **n, const BIGNUM **e, const BIGNUM **d)
{
	*n = r->n;
	*e = r->e;
	*d = r->d;
}

static void RSA_get0_factors(const RSA *r, const BIGNUM **p, const BIGNUM **q)
{
	*p = r->p;
	*q = r->q;
}

static void RSA_get0_crt_params(const RSA *r, const BIGNUM **dmp1, const BIGNUM **dmq1, const BIGNUM **iqmp)
{
	*dmp1 = r->dmp1;
	*dmq1 = r->dmq1;
	*iqmp = r->iqmp;
}

static void DH_get0_pqg(const DH *dh, const BIGNUM **p, const BIGNUM **q, const BIGNUM **g)
{
	*p = dh->p;
	*q = dh->q;
	*g = dh->g;
}

static int DH_set0_pqg(DH *dh, BIGNUM *p, BIGNUM *q, BIGNUM *g)
{
	dh->p = p;
	dh->q = q;
	dh->g = g;

	return 1;
}

static void DH_get0_key(const DH *dh, const BIGNUM **pub_key, const BIGNUM **priv_key)
{
	*pub_key = dh->pub_key;
	*priv_key = dh->priv_key;
}

static int DH_set0_key(DH *dh, BIGNUM *pub_key, BIGNUM *priv_key)
{
	dh->pub_key = pub_key;
	dh->priv_key = priv_key;

	return 1;
}

static void DSA_get0_pqg(const DSA *d, const BIGNUM **p, const BIGNUM **q, const BIGNUM **g)
{
	*p = d->p;
	*q = d->q;
	*g = d->g;
}

int DSA_set0_pqg(DSA *d, BIGNUM *p, BIGNUM *q, BIGNUM *g)
{
	d->p = p;
	d->q = q;
	d->g = g;

	return 1;
}

static void DSA_get0_key(const DSA *d, const BIGNUM **pub_key, const BIGNUM **priv_key)
{
	*pub_key = d->pub_key;
	*priv_key = d->priv_key;
}

int DSA_set0_key(DSA *d, BIGNUM *pub_key, BIGNUM *priv_key)
{
	d->pub_key = pub_key;
	d->priv_key = priv_key;

	return 1;
}

static const unsigned char *ASN1_STRING_get0_data(const ASN1_STRING *asn1)
{
	return M_ASN1_STRING_data(asn1);
}

static int EVP_PKEY_up_ref(EVP_PKEY *pkey)
{
	return CRYPTO_add(&pkey->references, 1, CRYPTO_LOCK_EVP_PKEY);
}

#if CRX_OPENSSL_API_VERSION < 0x10002

static int X509_get_signature_nid(const X509 *x)
{
	return OBJ_obj2nid(x->sig_alg->algorithm);
}

#endif

#define OpenSSL_version		SSLeay_version
#define OPENSSL_VERSION		SSLEAY_VERSION
#define X509_getm_notBefore	X509_get_notBefore
#define X509_getm_notAfter	X509_get_notAfter
#define EVP_CIPHER_CTX_reset	EVP_CIPHER_CTX_cleanup

#endif
/* }}} */

/* number conversion flags checks */
#define CRX_OPENSSL_CHECK_NUMBER_CONVERSION(_cond, _name, _arg_num) \
	do { \
		if (_cond) { \
			crex_argument_value_error((_arg_num), #_name" is too long"); \
			RETURN_THROWS(); \
		} \
	} while(0)
#define CRX_OPENSSL_CHECK_NUMBER_CONVERSION_NULL_RETURN(_cond, _name) \
	do { \
		if (_cond) { \
			crex_value_error(#_name" is too long"); \
			return NULL; \
		} \
	} while(0)
/* check if size_t can be safely casted to int */
#define CRX_OPENSSL_CHECK_SIZE_T_TO_INT(_var, _name, _arg_num) \
	CRX_OPENSSL_CHECK_NUMBER_CONVERSION(CREX_SIZE_T_INT_OVFL(_var), _name, _arg_num)
#define CRX_OPENSSL_CHECK_SIZE_T_TO_INT_NULL_RETURN(_var, _name) \
	CRX_OPENSSL_CHECK_NUMBER_CONVERSION_NULL_RETURN(CREX_SIZE_T_INT_OVFL(_var), _name)
/* check if size_t can be safely casted to unsigned int */
#define CRX_OPENSSL_CHECK_SIZE_T_TO_UINT(_var, _name, _arg_num) \
	CRX_OPENSSL_CHECK_NUMBER_CONVERSION(CREX_SIZE_T_UINT_OVFL(_var), _name, _arg_num)
/* check if long can be safely casted to int */
#define CRX_OPENSSL_CHECK_LONG_TO_INT(_var, _name, _arg_num) \
	CRX_OPENSSL_CHECK_NUMBER_CONVERSION(CREX_LONG_EXCEEDS_INT(_var), _name, _arg_num)
#define CRX_OPENSSL_CHECK_LONG_TO_INT_NULL_RETURN(_var, _name) \
	CRX_OPENSSL_CHECK_NUMBER_CONVERSION_NULL_RETURN(CREX_LONG_EXCEEDS_INT(_var), _name)

/* {{{ crx_openssl_store_errors */
void crx_openssl_store_errors(void)
{
	struct crx_openssl_errors *errors;
	int error_code = ERR_get_error();

	if (!error_code) {
		return;
	}

	if (!OPENSSL_G(errors)) {
		OPENSSL_G(errors) = pecalloc(1, sizeof(struct crx_openssl_errors), 1);
	}

	errors = OPENSSL_G(errors);

	do {
		errors->top = (errors->top + 1) % ERR_NUM_ERRORS;
		if (errors->top == errors->bottom) {
			errors->bottom = (errors->bottom + 1) % ERR_NUM_ERRORS;
		}
		errors->buffer[errors->top] = error_code;
	} while ((error_code = ERR_get_error()));

}
/* }}} */

/* {{{ crx_openssl_errors_set_mark */
void crx_openssl_errors_set_mark(void) {
	if (!OPENSSL_G(errors)) {
		return;
	}

	if (!OPENSSL_G(errors_mark)) {
		OPENSSL_G(errors_mark) = pecalloc(1, sizeof(struct crx_openssl_errors), 1);
	}

	memcpy(OPENSSL_G(errors_mark), OPENSSL_G(errors), sizeof(struct crx_openssl_errors));
}
/* }}} */

/* {{{ crx_openssl_errors_restore_mark */
void crx_openssl_errors_restore_mark(void) {
	if (!OPENSSL_G(errors)) {
		return;
	}

	struct crx_openssl_errors *errors = OPENSSL_G(errors);

	if (!OPENSSL_G(errors_mark)) {
		errors->top = 0;
		errors->bottom = 0;
	} else {
		memcpy(errors, OPENSSL_G(errors_mark), sizeof(struct crx_openssl_errors));
	}
}
/* }}} */

/* openssl file path check error function */
static void crx_openssl_check_path_error(uint32_t arg_num, int type, const char *format, ...)
{
	va_list va;
	const char *arg_name;

	va_start(va, format);

	if (type == E_ERROR) {
		crex_argument_error_variadic(crex_ce_value_error, arg_num, format, va);
	} else {
		arg_name = get_active_function_arg_name(arg_num);
		crx_verror(NULL, arg_name, type, format, va);
	}
	va_end(va);
}

/* openssl file path check extended */
bool crx_openssl_check_path_ex(
		const char *file_path, size_t file_path_len, char *real_path, uint32_t arg_num,
		bool contains_file_protocol, bool is_from_array, const char *option_name)
{
	const char *fs_file_path;
	size_t fs_file_path_len;
	const char *error_msg = NULL;
	int error_type = E_WARNING;

	if (file_path_len == 0) {
		real_path[0] = '\0';
		return true;
	}

	if (contains_file_protocol) {
		size_t path_prefix_len = sizeof("file://") - 1;
		if (file_path_len <= path_prefix_len) {
			return false;
		}
		fs_file_path = file_path + path_prefix_len;
		fs_file_path_len = file_path_len - path_prefix_len;
	} else {
		fs_file_path = file_path;
		fs_file_path_len = file_path_len;
	}

	if (CHECK_NULL_PATH(fs_file_path, fs_file_path_len)) {
		error_msg = "must not contain any null bytes";
		error_type = E_ERROR;
	} else if (expand_filepath(fs_file_path, real_path) == NULL) {
		error_msg = "must be a valid file path";
	}

	if (error_msg != NULL) {
		if (arg_num == 0) {
			const char *option_title = option_name ? option_name : "unknown";
			const char *option_label = is_from_array ? "array item" : "option";
			crx_error_docref(NULL, E_WARNING, "Path for %s %s %s",
					option_title, option_label, error_msg);
		} else if (is_from_array && option_name != NULL) {
			crx_openssl_check_path_error(
					arg_num, error_type, "option %s array item %s", option_name, error_msg);
		} else if (is_from_array) {
			crx_openssl_check_path_error(arg_num, error_type, "array item %s", error_msg);
		} else if (option_name != NULL) {
			crx_openssl_check_path_error(
					arg_num, error_type, "option %s %s", option_name, error_msg);
		} else {
			crx_openssl_check_path_error(arg_num, error_type, "%s", error_msg);
		}
	} else if (!crx_check_open_basedir(real_path)) {
		return true;
	}

	return false;
}

static int ssl_stream_data_index;

crx_stream* crx_openssl_get_stream_from_ssl_handle(const SSL *ssl)
{
	return (crx_stream*)SSL_get_ex_data(ssl, ssl_stream_data_index);
}

int crx_openssl_get_ssl_stream_data_index(void)
{
	return ssl_stream_data_index;
}

/* openssl -> CRX "bridging" */
/* true global; readonly after module startup */
static char default_ssl_conf_filename[MAXPATHLEN];

struct crx_x509_request { /* {{{ */
	CONF *global_config;	/* Global SSL config */
	CONF *req_config;		/* SSL config for this request */
	const EVP_MD * md_alg;
	const EVP_MD * digest;
	char	* section_name,
			* config_filename,
			* digest_name,
			* extensions_section,
			* request_extensions_section;
	int priv_key_bits;
	int priv_key_type;

	int priv_key_encrypt;

#ifdef HAVE_EVP_PKEY_EC
	int curve_name;
#endif

	EVP_PKEY * priv_key;

	const EVP_CIPHER * priv_key_encrypt_cipher;
};
/* }}} */

static X509 *crx_openssl_x509_from_param(
		crex_object *cert_obj, crex_string *cert_str, uint32_t arg_num);
static X509 *crx_openssl_x509_from_zval(
		zval *val, bool *free_cert, uint32_t arg_num, bool is_from_array, const char *option_name);
static X509_REQ *crx_openssl_csr_from_param(
		crex_object *csr_obj, crex_string *csr_str, uint32_t arg_num);
static EVP_PKEY *crx_openssl_pkey_from_zval(
		zval *val, int public_key, char *passphrase, size_t passphrase_len, uint32_t arg_num);

static X509_STORE * crx_openssl_setup_verify(zval * calist, uint32_t arg_num);
static STACK_OF(X509) * crx_openssl_load_all_certs_from_file(
		char *cert_file, size_t cert_file_len, uint32_t arg_num);
static EVP_PKEY * crx_openssl_generate_private_key(struct crx_x509_request * req);

static void crx_openssl_add_assoc_name_entry(zval * val, char * key, X509_NAME * name, int shortname) /* {{{ */
{
	zval *data;
	zval subitem, tmp;
	int i;
	char *sname;
	int nid;
	X509_NAME_ENTRY * ne;
	ASN1_STRING * str = NULL;
	ASN1_OBJECT * obj;

	if (key != NULL) {
		array_init(&subitem);
	} else {
		ZVAL_COPY_VALUE(&subitem, val);
	}

	for (i = 0; i < X509_NAME_entry_count(name); i++) {
		const unsigned char *to_add = NULL;
		int to_add_len = 0;
		unsigned char *to_add_buf = NULL;

		ne = X509_NAME_get_entry(name, i);
		obj = X509_NAME_ENTRY_get_object(ne);
		nid = OBJ_obj2nid(obj);

		if (shortname) {
			sname = (char *) OBJ_nid2sn(nid);
		} else {
			sname = (char *) OBJ_nid2ln(nid);
		}

		str = X509_NAME_ENTRY_get_data(ne);
		if (ASN1_STRING_type(str) != V_ASN1_UTF8STRING) {
			/* ASN1_STRING_to_UTF8(3): The converted data is copied into a newly allocated buffer */
			to_add_len = ASN1_STRING_to_UTF8(&to_add_buf, str);
			to_add = to_add_buf;
		} else {
			/* ASN1_STRING_get0_data(3): Since this is an internal pointer it should not be freed or modified in any way */
			to_add = ASN1_STRING_get0_data(str);
			to_add_len = ASN1_STRING_length(str);
		}

		if (to_add_len != -1) {
			if ((data = crex_hash_str_find(C_ARRVAL(subitem), sname, strlen(sname))) != NULL) {
				if (C_TYPE_P(data) == IS_ARRAY) {
					add_next_index_stringl(data, (const char *)to_add, to_add_len);
				} else if (C_TYPE_P(data) == IS_STRING) {
					array_init(&tmp);
					add_next_index_str(&tmp, crex_string_copy(C_STR_P(data)));
					add_next_index_stringl(&tmp, (const char *)to_add, to_add_len);
					crex_hash_str_update(C_ARRVAL(subitem), sname, strlen(sname), &tmp);
				}
			} else {
				/* it might be better to expand it and pass zval from ZVAL_STRING
				 * to crex_symtable_str_update so we do not silently drop const
				 * but we need a test to cover this part first */
				add_assoc_stringl(&subitem, sname, (char *)to_add, to_add_len);
			}
		} else {
			crx_openssl_store_errors();
		}

		if (to_add_buf != NULL) {
			OPENSSL_free(to_add_buf);
		}
	}

	if (key != NULL) {
		crex_hash_str_update(C_ARRVAL_P(val), key, strlen(key), &subitem);
	}
}
/* }}} */

static void crx_openssl_add_assoc_asn1_string(zval * val, char * key, ASN1_STRING * str) /* {{{ */
{
	add_assoc_stringl(val, key, (char *)str->data, str->length);
}
/* }}} */

static time_t crx_openssl_asn1_time_to_time_t(ASN1_UTCTIME * timestr) /* {{{ */
{
/*
	This is how the time string is formatted:

   snprintf(p, sizeof(p), "%02d%02d%02d%02d%02d%02dZ",ts->tm_year%100,
	  ts->tm_mon+1,ts->tm_mday,ts->tm_hour,ts->tm_min,ts->tm_sec);
*/

	time_t ret;
	struct tm thetime;
	char * strbuf;
	char * thestr;
	long gmadjust = 0;
	size_t timestr_len;

	if (ASN1_STRING_type(timestr) != V_ASN1_UTCTIME && ASN1_STRING_type(timestr) != V_ASN1_GENERALIZEDTIME) {
		crx_error_docref(NULL, E_WARNING, "Illegal ASN1 data type for timestamp");
		return (time_t)-1;
	}

	timestr_len = (size_t)ASN1_STRING_length(timestr);

	if (timestr_len != strlen((const char *)ASN1_STRING_get0_data(timestr))) {
		crx_error_docref(NULL, E_WARNING, "Illegal length in timestamp");
		return (time_t)-1;
	}

	if (timestr_len < 13 && timestr_len != 11) {
		crx_error_docref(NULL, E_WARNING, "Unable to parse time string %s correctly", timestr->data);
		return (time_t)-1;
	}

	if (ASN1_STRING_type(timestr) == V_ASN1_GENERALIZEDTIME && timestr_len < 15) {
		crx_error_docref(NULL, E_WARNING, "Unable to parse time string %s correctly", timestr->data);
		return (time_t)-1;
	}

	strbuf = estrdup((const char *)ASN1_STRING_get0_data(timestr));

	memset(&thetime, 0, sizeof(thetime));

	/* we work backwards so that we can use atoi more easily */

	thestr = strbuf + timestr_len - 3;

	if (timestr_len == 11) {
		thetime.tm_sec = 0;
	} else {
		thetime.tm_sec = atoi(thestr);
		*thestr = '\0';
		thestr -= 2;
	}
	thetime.tm_min = atoi(thestr);
	*thestr = '\0';
	thestr -= 2;
	thetime.tm_hour = atoi(thestr);
	*thestr = '\0';
	thestr -= 2;
	thetime.tm_mday = atoi(thestr);
	*thestr = '\0';
	thestr -= 2;
	thetime.tm_mon = atoi(thestr)-1;

	*thestr = '\0';
	if( ASN1_STRING_type(timestr) == V_ASN1_UTCTIME ) {
		thestr -= 2;
		thetime.tm_year = atoi(thestr);

		if (thetime.tm_year < 68) {
			thetime.tm_year += 100;
		}
	} else if( ASN1_STRING_type(timestr) == V_ASN1_GENERALIZEDTIME ) {
		thestr -= 4;
		thetime.tm_year = atoi(thestr) - 1900;
	}


	thetime.tm_isdst = -1;
	ret = mktime(&thetime);

#ifdef HAVE_STRUCT_TM_TM_GMTOFF
	gmadjust = thetime.tm_gmtoff;
#else
	/*
	** If correcting for daylight savings time, we set the adjustment to
	** the value of timezone - 3600 seconds. Otherwise, we need to overcorrect and
	** set the adjustment to the main timezone + 3600 seconds.
	*/
	gmadjust = -(thetime.tm_isdst ? (long)timezone - 3600 : (long)timezone);
#endif
	ret += gmadjust;

	efree(strbuf);

	return ret;
}
/* }}} */

static inline int crx_openssl_config_check_syntax(const char * section_label, const char * config_filename, const char * section, CONF *config) /* {{{ */
{
	X509V3_CTX ctx;

	X509V3_set_ctx_test(&ctx);
	X509V3_set_nconf(&ctx, config);
	if (!X509V3_EXT_add_nconf(config, &ctx, (char *)section, NULL)) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Error loading %s section %s of %s",
				section_label,
				section,
				config_filename);
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

static char *crx_openssl_conf_get_string(CONF *conf, const char *group, const char *name) {
	/* OpenSSL reports an error if a configuration value is not found.
	 * However, we don't want to generate errors for optional configuration. */
	ERR_set_mark();
	char *str = NCONF_get_string(conf, group, name);
	ERR_pop_to_mark();
	return str;
}

static long crx_openssl_conf_get_number(CONF *conf, const char *group, const char *name) {
	/* Same here, ignore errors. */
	long res = 0;
	ERR_set_mark();
	NCONF_get_number(conf, group, name, &res);
	ERR_pop_to_mark();
	return res;
}

static int crx_openssl_add_oid_section(struct crx_x509_request * req) /* {{{ */
{
	char * str;
	STACK_OF(CONF_VALUE) * sktmp;
	CONF_VALUE * cnf;
	int i;

	str = crx_openssl_conf_get_string(req->req_config, NULL, "oid_section");
	if (str == NULL) {
		return SUCCESS;
	}
	sktmp = NCONF_get_section(req->req_config, str);
	if (sktmp == NULL) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Problem loading oid section %s", str);
		return FAILURE;
	}
	for (i = 0; i < sk_CONF_VALUE_num(sktmp); i++) {
		cnf = sk_CONF_VALUE_value(sktmp, i);
		if (OBJ_sn2nid(cnf->name) == NID_undef && OBJ_ln2nid(cnf->name) == NID_undef &&
				OBJ_create(cnf->value, cnf->name, cnf->name) == NID_undef) {
			crx_openssl_store_errors();
			crx_error_docref(NULL, E_WARNING, "Problem creating object %s=%s", cnf->name, cnf->value);
			return FAILURE;
		}
	}
	return SUCCESS;
}
/* }}} */

#define CRX_SSL_REQ_INIT(req)		memset(req, 0, sizeof(*req))
#define CRX_SSL_REQ_DISPOSE(req)	crx_openssl_dispose_config(req)
#define CRX_SSL_REQ_PARSE(req, zval)	crx_openssl_parse_config(req, zval)

#define CRX_SSL_CONFIG_SYNTAX_CHECK(var) if (req->var && crx_openssl_config_check_syntax(#var, \
			req->config_filename, req->var, req->req_config) == FAILURE) return FAILURE

#define SET_OPTIONAL_STRING_ARG(key, varname, defval)	\
	do { \
		if (optional_args && (item = crex_hash_str_find(C_ARRVAL_P(optional_args), key, sizeof(key)-1)) != NULL && C_TYPE_P(item) == IS_STRING) { \
			varname = C_STRVAL_P(item); \
		} else { \
			varname = defval; \
			if (varname == NULL) { \
				crx_openssl_store_errors(); \
			} \
		} \
	} while(0)

#define SET_OPTIONAL_LONG_ARG(key, varname, defval)	\
	if (optional_args && (item = crex_hash_str_find(C_ARRVAL_P(optional_args), key, sizeof(key)-1)) != NULL && C_TYPE_P(item) == IS_LONG) \
		varname = (int)C_LVAL_P(item); \
	else \
		varname = defval

static const EVP_CIPHER * crx_openssl_get_evp_cipher_from_algo(crex_long algo);

/* {{{ strip line endings from spkac */
static int crx_openssl_spki_cleanup(const char *src, char *dest)
{
	int removed = 0;

	while (*src) {
		if (*src != '\n' && *src != '\r') {
			*dest++ = *src;
		} else {
			++removed;
		}
		++src;
	}
	*dest = 0;
	return removed;
}
/* }}} */

static int crx_openssl_parse_config(struct crx_x509_request * req, zval * optional_args) /* {{{ */
{
	char * str, path[MAXPATHLEN];
	zval * item;

	SET_OPTIONAL_STRING_ARG("config", req->config_filename, default_ssl_conf_filename);
	SET_OPTIONAL_STRING_ARG("config_section_name", req->section_name, "req");
	req->global_config = NCONF_new(NULL);
	if (!NCONF_load(req->global_config, default_ssl_conf_filename, NULL)) {
		crx_openssl_store_errors();
	}

	req->req_config = NCONF_new(NULL);
	if (!NCONF_load(req->req_config, req->config_filename, NULL)) {
		return FAILURE;
	}

	/* read in the oids */
	str = crx_openssl_conf_get_string(req->req_config, NULL, "oid_file");
	if (str != NULL && crx_openssl_check_path_ex(str, strlen(str), path, 0, false, false, "oid_file")) {
		BIO *oid_bio = BIO_new_file(path, CRX_OPENSSL_BIO_MODE_R(PKCS7_BINARY));
		if (oid_bio) {
			OBJ_create_objects(oid_bio);
			BIO_free(oid_bio);
			crx_openssl_store_errors();
		}
	}
	if (crx_openssl_add_oid_section(req) == FAILURE) {
		return FAILURE;
	}
	SET_OPTIONAL_STRING_ARG("digest_alg", req->digest_name,
		crx_openssl_conf_get_string(req->req_config, req->section_name, "default_md"));
	SET_OPTIONAL_STRING_ARG("x509_extensions", req->extensions_section,
		crx_openssl_conf_get_string(req->req_config, req->section_name, "x509_extensions"));
	SET_OPTIONAL_STRING_ARG("req_extensions", req->request_extensions_section,
		crx_openssl_conf_get_string(req->req_config, req->section_name, "req_extensions"));
	SET_OPTIONAL_LONG_ARG("private_key_bits", req->priv_key_bits,
		crx_openssl_conf_get_number(req->req_config, req->section_name, "default_bits"));
	SET_OPTIONAL_LONG_ARG("private_key_type", req->priv_key_type, OPENSSL_KEYTYPE_DEFAULT);

	if (optional_args && (item = crex_hash_str_find(C_ARRVAL_P(optional_args), "encrypt_key", sizeof("encrypt_key")-1)) != NULL) {
		req->priv_key_encrypt = C_TYPE_P(item) == IS_TRUE ? 1 : 0;
	} else {
		str = crx_openssl_conf_get_string(req->req_config, req->section_name, "encrypt_rsa_key");
		if (str == NULL) {
			str = crx_openssl_conf_get_string(req->req_config, req->section_name, "encrypt_key");
		}
		if (str != NULL && strcmp(str, "no") == 0) {
			req->priv_key_encrypt = 0;
		} else {
			req->priv_key_encrypt = 1;
		}
	}

	if (req->priv_key_encrypt &&
		optional_args &&
		(item = crex_hash_str_find(C_ARRVAL_P(optional_args), "encrypt_key_cipher", sizeof("encrypt_key_cipher")-1)) != NULL &&
		C_TYPE_P(item) == IS_LONG
	) {
		crex_long cipher_algo = C_LVAL_P(item);
		const EVP_CIPHER* cipher = crx_openssl_get_evp_cipher_from_algo(cipher_algo);
		if (cipher == NULL) {
			crx_error_docref(NULL, E_WARNING, "Unknown cipher algorithm for private key");
			return FAILURE;
		} else {
			req->priv_key_encrypt_cipher = cipher;
		}
	} else {
		req->priv_key_encrypt_cipher = NULL;
	}

	/* digest alg */
	if (req->digest_name == NULL) {
		req->digest_name = crx_openssl_conf_get_string(req->req_config, req->section_name, "default_md");
	}
	if (req->digest_name != NULL) {
		req->digest = req->md_alg = EVP_get_digestbyname(req->digest_name);
	}
	if (req->md_alg == NULL) {
		req->md_alg = req->digest = EVP_sha1();
		crx_openssl_store_errors();
	}

	CRX_SSL_CONFIG_SYNTAX_CHECK(extensions_section);
#ifdef HAVE_EVP_PKEY_EC
	/* set the ec group curve name */
	req->curve_name = NID_undef;
	if (optional_args && (item = crex_hash_str_find(C_ARRVAL_P(optional_args), "curve_name", sizeof("curve_name")-1)) != NULL
		&& C_TYPE_P(item) == IS_STRING) {
		req->curve_name = OBJ_sn2nid(C_STRVAL_P(item));
		if (req->curve_name == NID_undef) {
			crx_error_docref(NULL, E_WARNING, "Unknown elliptic curve (short) name %s", C_STRVAL_P(item));
			return FAILURE;
		}
	}
#endif

	/* set the string mask */
	str = crx_openssl_conf_get_string(req->req_config, req->section_name, "string_mask");
	if (str != NULL && !ASN1_STRING_set_default_mask_asc(str)) {
		crx_error_docref(NULL, E_WARNING, "Invalid global string mask setting %s", str);
		return FAILURE;
	}

	CRX_SSL_CONFIG_SYNTAX_CHECK(request_extensions_section);

	return SUCCESS;
}
/* }}} */

static void crx_openssl_dispose_config(struct crx_x509_request * req) /* {{{ */
{
	if (req->priv_key) {
		EVP_PKEY_free(req->priv_key);
		req->priv_key = NULL;
	}
	if (req->global_config) {
		NCONF_free(req->global_config);
		req->global_config = NULL;
	}
	if (req->req_config) {
		NCONF_free(req->req_config);
		req->req_config = NULL;
	}
}
/* }}} */

#if defined(CRX_WIN32) || CRX_OPENSSL_API_VERSION >= 0x10100
#define CRX_OPENSSL_RAND_ADD_TIME() ((void) 0)
#else
#define CRX_OPENSSL_RAND_ADD_TIME() crx_openssl_rand_add_timeval()

static inline void crx_openssl_rand_add_timeval(void)  /* {{{ */
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	RAND_add(&tv, sizeof(tv), 0.0);
}
/* }}} */

#endif

static int crx_openssl_load_rand_file(const char * file, int *egdsocket, int *seeded) /* {{{ */
{
	char buffer[MAXPATHLEN];

	*egdsocket = 0;
	*seeded = 0;

	if (file == NULL) {
		file = RAND_file_name(buffer, sizeof(buffer));
#ifdef HAVE_RAND_EGD
	} else if (RAND_egd(file) > 0) {
		/* if the given filename is an EGD socket, don't
		 * write anything back to it */
		*egdsocket = 1;
		return SUCCESS;
#endif
	}
	if (file == NULL || !RAND_load_file(file, -1)) {
		if (RAND_status() == 0) {
			crx_openssl_store_errors();
			crx_error_docref(NULL, E_WARNING, "Unable to load random state; not enough random data!");
			return FAILURE;
		}
		return FAILURE;
	}
	*seeded = 1;
	return SUCCESS;
}
/* }}} */

static int crx_openssl_write_rand_file(const char * file, int egdsocket, int seeded) /* {{{ */
{
	char buffer[MAXPATHLEN];


	if (egdsocket || !seeded) {
		/* if we did not manage to read the seed file, we should not write
		 * a low-entropy seed file back */
		return FAILURE;
	}
	if (file == NULL) {
		file = RAND_file_name(buffer, sizeof(buffer));
	}
	CRX_OPENSSL_RAND_ADD_TIME();
	if (file == NULL || !RAND_write_file(file)) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Unable to write random state");
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

static EVP_MD * crx_openssl_get_evp_md_from_algo(crex_long algo) { /* {{{ */
	EVP_MD *mdtype;

	switch (algo) {
		case OPENSSL_ALGO_SHA1:
			mdtype = (EVP_MD *) EVP_sha1();
			break;
		case OPENSSL_ALGO_MD5:
			mdtype = (EVP_MD *) EVP_md5();
			break;
#ifndef OPENSSL_NO_MD4
		case OPENSSL_ALGO_MD4:
			mdtype = (EVP_MD *) EVP_md4();
			break;
#endif
#ifndef OPENSSL_NO_MD2
		case OPENSSL_ALGO_MD2:
			mdtype = (EVP_MD *) EVP_md2();
			break;
#endif
#if CRX_OPENSSL_API_VERSION < 0x10100
		case OPENSSL_ALGO_DSS1:
			mdtype = (EVP_MD *) EVP_dss1();
			break;
#endif
		case OPENSSL_ALGO_SHA224:
			mdtype = (EVP_MD *) EVP_sha224();
			break;
		case OPENSSL_ALGO_SHA256:
			mdtype = (EVP_MD *) EVP_sha256();
			break;
		case OPENSSL_ALGO_SHA384:
			mdtype = (EVP_MD *) EVP_sha384();
			break;
		case OPENSSL_ALGO_SHA512:
			mdtype = (EVP_MD *) EVP_sha512();
			break;
#ifndef OPENSSL_NO_RMD160
		case OPENSSL_ALGO_RMD160:
			mdtype = (EVP_MD *) EVP_ripemd160();
			break;
#endif
		default:
			return NULL;
			break;
	}
	return mdtype;
}
/* }}} */

static const EVP_CIPHER * crx_openssl_get_evp_cipher_from_algo(crex_long algo) { /* {{{ */
	switch (algo) {
#ifndef OPENSSL_NO_RC2
		case CRX_OPENSSL_CIPHER_RC2_40:
			return EVP_rc2_40_cbc();
			break;
		case CRX_OPENSSL_CIPHER_RC2_64:
			return EVP_rc2_64_cbc();
			break;
		case CRX_OPENSSL_CIPHER_RC2_128:
			return EVP_rc2_cbc();
			break;
#endif

#ifndef OPENSSL_NO_DES
		case CRX_OPENSSL_CIPHER_DES:
			return EVP_des_cbc();
			break;
		case CRX_OPENSSL_CIPHER_3DES:
			return EVP_des_ede3_cbc();
			break;
#endif

#ifndef OPENSSL_NO_AES
		case CRX_OPENSSL_CIPHER_AES_128_CBC:
			return EVP_aes_128_cbc();
			break;
		case CRX_OPENSSL_CIPHER_AES_192_CBC:
			return EVP_aes_192_cbc();
			break;
		case CRX_OPENSSL_CIPHER_AES_256_CBC:
			return EVP_aes_256_cbc();
			break;
#endif


		default:
			return NULL;
			break;
	}
}
/* }}} */

/* {{{ INI Settings */
CRX_INI_BEGIN()
	CRX_INI_ENTRY("openssl.cafile", NULL, CRX_INI_PERDIR, NULL)
	CRX_INI_ENTRY("openssl.capath", NULL, CRX_INI_PERDIR, NULL)
CRX_INI_END()
/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(openssl)
{
	char * config_filename;

	crx_openssl_certificate_ce = register_class_OpenSSLCertificate();
	crx_openssl_certificate_ce->create_object = crx_openssl_certificate_create_object;
	crx_openssl_certificate_ce->default_object_handlers = &crx_openssl_certificate_object_handlers;

	memcpy(&crx_openssl_certificate_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	crx_openssl_certificate_object_handlers.offset = XtOffsetOf(crx_openssl_certificate_object, std);
	crx_openssl_certificate_object_handlers.free_obj = crx_openssl_certificate_free_obj;
	crx_openssl_certificate_object_handlers.get_constructor = crx_openssl_certificate_get_constructor;
	crx_openssl_certificate_object_handlers.clone_obj = NULL;
	crx_openssl_certificate_object_handlers.compare = crex_objects_not_comparable;

	crx_openssl_request_ce = register_class_OpenSSLCertificateSigningRequest();
	crx_openssl_request_ce->create_object = crx_openssl_request_create_object;
	crx_openssl_request_ce->default_object_handlers = &crx_openssl_request_object_handlers;

	memcpy(&crx_openssl_request_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	crx_openssl_request_object_handlers.offset = XtOffsetOf(crx_openssl_request_object, std);
	crx_openssl_request_object_handlers.free_obj = crx_openssl_request_free_obj;
	crx_openssl_request_object_handlers.get_constructor = crx_openssl_request_get_constructor;
	crx_openssl_request_object_handlers.clone_obj = NULL;
	crx_openssl_request_object_handlers.compare = crex_objects_not_comparable;

	crx_openssl_pkey_ce = register_class_OpenSSLAsymmetricKey();
	crx_openssl_pkey_ce->create_object = crx_openssl_pkey_create_object;
	crx_openssl_pkey_ce->default_object_handlers = &crx_openssl_pkey_object_handlers;

	memcpy(&crx_openssl_pkey_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	crx_openssl_pkey_object_handlers.offset = XtOffsetOf(crx_openssl_pkey_object, std);
	crx_openssl_pkey_object_handlers.free_obj = crx_openssl_pkey_free_obj;
	crx_openssl_pkey_object_handlers.get_constructor = crx_openssl_pkey_get_constructor;
	crx_openssl_pkey_object_handlers.clone_obj = NULL;
	crx_openssl_pkey_object_handlers.compare = crex_objects_not_comparable;

#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined (LIBRESSL_VERSION_NUMBER)
	OPENSSL_config(NULL);
	SSL_library_init();
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
#else
	OPENSSL_init_ssl(OPENSSL_INIT_LOAD_CONFIG, NULL);
#endif

	/* register a resource id number with OpenSSL so that we can map SSL -> stream structures in
	 * OpenSSL callbacks */
	ssl_stream_data_index = SSL_get_ex_new_index(0, "CRX stream index", NULL, NULL, NULL);

	register_openssl_symbols(module_number);

	/* Determine default SSL configuration file */
	config_filename = getenv("OPENSSL_CONF");
	if (config_filename == NULL) {
		config_filename = getenv("SSLEAY_CONF");
	}

	/* default to 'openssl.cnf' if no environment variable is set */
	if (config_filename == NULL) {
		snprintf(default_ssl_conf_filename, sizeof(default_ssl_conf_filename), "%s/%s",
				X509_get_default_cert_area(),
				"openssl.cnf");
	} else {
		strlcpy(default_ssl_conf_filename, config_filename, sizeof(default_ssl_conf_filename));
	}

	crx_stream_xport_register("ssl", crx_openssl_ssl_socket_factory);
#ifndef OPENSSL_NO_SSL3
	crx_stream_xport_register("sslv3", crx_openssl_ssl_socket_factory);
#endif
	crx_stream_xport_register("tls", crx_openssl_ssl_socket_factory);
	crx_stream_xport_register("tlsv1.0", crx_openssl_ssl_socket_factory);
	crx_stream_xport_register("tlsv1.1", crx_openssl_ssl_socket_factory);
	crx_stream_xport_register("tlsv1.2", crx_openssl_ssl_socket_factory);
#if OPENSSL_VERSION_NUMBER >= 0x10101000
	crx_stream_xport_register("tlsv1.3", crx_openssl_ssl_socket_factory);
#endif

	/* override the default tcp socket provider */
	crx_stream_xport_register("tcp", crx_openssl_ssl_socket_factory);

	crx_register_url_stream_wrapper("https", &crx_stream_http_wrapper);
	crx_register_url_stream_wrapper("ftps", &crx_stream_ftp_wrapper);

	REGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_GINIT_FUNCTION */
CRX_GINIT_FUNCTION(openssl)
{
#if defined(COMPILE_DL_OPENSSL) && defined(ZTS)
	CREX_TSRMLS_CACHE_UPDATE();
#endif
	openssl_globals->errors = NULL;
	openssl_globals->errors_mark = NULL;
}
/* }}} */

/* {{{ CRX_GSHUTDOWN_FUNCTION */
CRX_GSHUTDOWN_FUNCTION(openssl)
{
	if (openssl_globals->errors) {
		pefree(openssl_globals->errors, 1);
	}
	if (openssl_globals->errors_mark) {
		pefree(openssl_globals->errors_mark, 1);
	}
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(openssl)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "OpenSSL support", "enabled");
	crx_info_print_table_row(2, "OpenSSL Library Version", OpenSSL_version(OPENSSL_VERSION));
	crx_info_print_table_row(2, "OpenSSL Header Version", OPENSSL_VERSION_TEXT);
	crx_info_print_table_row(2, "Openssl default config", default_ssl_conf_filename);
	crx_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ CRX_MSHUTDOWN_FUNCTION */
CRX_MSHUTDOWN_FUNCTION(openssl)
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined (LIBRESSL_VERSION_NUMBER)
	EVP_cleanup();

	/* prevent accessing locking callback from unloaded extension */
	CRYPTO_set_locking_callback(NULL);

#ifndef OPENSSL_NO_ENGINE
	/* Free engine list initialized by OPENSSL_config */
	ENGINE_cleanup();
#endif

	/* free allocated error strings */
	ERR_free_strings();
	CONF_modules_free();
#endif

	crx_unregister_url_stream_wrapper("https");
	crx_unregister_url_stream_wrapper("ftps");

	crx_stream_xport_unregister("ssl");
#ifndef OPENSSL_NO_SSL3
	crx_stream_xport_unregister("sslv3");
#endif
	crx_stream_xport_unregister("tls");
	crx_stream_xport_unregister("tlsv1.0");
	crx_stream_xport_unregister("tlsv1.1");
	crx_stream_xport_unregister("tlsv1.2");
#if OPENSSL_VERSION_NUMBER >= 0x10101000
	crx_stream_xport_unregister("tlsv1.3");
#endif

	/* reinstate the default tcp handler */
	crx_stream_xport_register("tcp", crx_stream_generic_socket_factory);

	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* {{{ x509 cert functions */

/* {{{ Retrieve an array mapping available certificate locations */
CRX_FUNCTION(openssl_get_cert_locations)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);

	add_assoc_string(return_value, "default_cert_file", (char *) X509_get_default_cert_file());
	add_assoc_string(return_value, "default_cert_file_env", (char *) X509_get_default_cert_file_env());
	add_assoc_string(return_value, "default_cert_dir", (char *) X509_get_default_cert_dir());
	add_assoc_string(return_value, "default_cert_dir_env", (char *) X509_get_default_cert_dir_env());
	add_assoc_string(return_value, "default_private_dir", (char *) X509_get_default_private_dir());
	add_assoc_string(return_value, "default_default_cert_area", (char *) X509_get_default_cert_area());
	add_assoc_string(return_value, "ini_cafile",
		crex_ini_string("openssl.cafile", sizeof("openssl.cafile")-1, 0));
	add_assoc_string(return_value, "ini_capath",
		crex_ini_string("openssl.capath", sizeof("openssl.capath")-1, 0));
}
/* }}} */

static X509 *crx_openssl_x509_from_str(
		crex_string *cert_str, uint32_t arg_num, bool is_from_array, const char *option_name) {
	X509 *cert = NULL;
	char cert_path[MAXPATHLEN];
	BIO *in;

	if (ZSTR_LEN(cert_str) > 7 && memcmp(ZSTR_VAL(cert_str), "file://", sizeof("file://") - 1) == 0) {
		if (!crx_openssl_check_path_str_ex(cert_str, cert_path, arg_num, true, is_from_array, option_name)) {
			return NULL;
		}

		in = BIO_new_file(cert_path, CRX_OPENSSL_BIO_MODE_R(PKCS7_BINARY));
		if (in == NULL) {
			crx_openssl_store_errors();
			return NULL;
		}
		cert = PEM_read_bio_X509(in, NULL, NULL, NULL);
	} else {
		in = BIO_new_mem_buf(ZSTR_VAL(cert_str), (int) ZSTR_LEN(cert_str));
		if (in == NULL) {
			crx_openssl_store_errors();
			return NULL;
		}
#ifdef TYPEDEF_D2I_OF
		cert = (X509 *) PEM_ASN1_read_bio((d2i_of_void *)d2i_X509, PEM_STRING_X509, in, NULL, NULL, NULL);
#else
		cert = (X509 *) PEM_ASN1_read_bio((char *(*)())d2i_X509, PEM_STRING_X509, in, NULL, NULL, NULL);
#endif
	}

	if (!BIO_free(in)) {
		crx_openssl_store_errors();
	}

	if (cert == NULL) {
		crx_openssl_store_errors();
		return NULL;
	}

	return cert;
}

/* {{{ crx_openssl_x509_from_param
	Given a parameter, extract it into an X509 object.
	The parameter can be:
		. X509 object created using openssl_read_x509()
		. a path to that cert if it starts with file://
		. the cert data otherwise
*/
static X509 *crx_openssl_x509_from_param(
		crex_object *cert_obj, crex_string *cert_str, uint32_t arg_num) {
	if (cert_obj) {
		return crx_openssl_certificate_from_obj(cert_obj)->x509;
	}

	CREX_ASSERT(cert_str);

	return crx_openssl_x509_from_str(cert_str, arg_num, false, NULL);
}
/* }}} */

static X509 *crx_openssl_x509_from_zval(
		zval *val, bool *free_cert, uint32_t arg_num, bool is_from_array, const char *option_name)
{
	if (C_TYPE_P(val) == IS_OBJECT && C_OBJCE_P(val) == crx_openssl_certificate_ce) {
		*free_cert = 0;

		return crx_openssl_certificate_from_obj(C_OBJ_P(val))->x509;
	}

	*free_cert = 1;

	if (!try_convert_to_string(val)) {
		return NULL;
	}

	return crx_openssl_x509_from_str(C_STR_P(val), arg_num, is_from_array, option_name);
}
/* }}} */

/* {{{ Exports a CERT to file or a var */
CRX_FUNCTION(openssl_x509_export_to_file)
{
	X509 *cert;
	crex_object *cert_obj;
	crex_string *cert_str;

	bool notext = 1;
	BIO * bio_out;
	char * filename, file_path[MAXPATHLEN];
	size_t filename_len;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_OBJ_OF_CLASS_OR_STR(cert_obj, crx_openssl_certificate_ce, cert_str)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(notext)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_FALSE;

	cert = crx_openssl_x509_from_param(cert_obj, cert_str, 1);
	if (cert == NULL) {
		crx_error_docref(NULL, E_WARNING, "X.509 Certificate cannot be retrieved");
		return;
	}

	if (!crx_openssl_check_path(filename, filename_len, file_path, 2)) {
		return;
	}

	bio_out = BIO_new_file(file_path, CRX_OPENSSL_BIO_MODE_W(PKCS7_BINARY));
	if (bio_out) {
		if (!notext && !X509_print(bio_out, cert)) {
			crx_openssl_store_errors();
		}
		if (!PEM_write_bio_X509(bio_out, cert)) {
			crx_openssl_store_errors();
		}

		RETVAL_TRUE;
	} else {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Error opening file %s", file_path);
	}

	if (cert_str) {
		X509_free(cert);
	}

	if (!BIO_free(bio_out)) {
		crx_openssl_store_errors();
	}
}
/* }}} */

/* {{{ Creates new private key (or uses existing) and creates a new spki cert
   outputting results to var */
CRX_FUNCTION(openssl_spki_new)
{
	size_t challenge_len;
	char * challenge = NULL, *spkstr = NULL;
	crex_string * s = NULL;
	const char *spkac = "SPKAC=";
	crex_long algo = OPENSSL_ALGO_MD5;

	zval *zpkey = NULL;
	EVP_PKEY *pkey = NULL;
	NETSCAPE_SPKI *spki=NULL;
	const EVP_MD *mdtype;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Os|l", &zpkey, crx_openssl_pkey_ce, &challenge, &challenge_len, &algo) == FAILURE) {
		RETURN_THROWS();
	}
	RETVAL_FALSE;

	CRX_OPENSSL_CHECK_SIZE_T_TO_INT(challenge_len, challenge, 2);

	pkey = crx_openssl_pkey_from_zval(zpkey, 0, challenge, challenge_len, 1);
	if (pkey == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Unable to use supplied private key");
		}
		goto cleanup;
	}

	mdtype = crx_openssl_get_evp_md_from_algo(algo);

	if (!mdtype) {
		crx_error_docref(NULL, E_WARNING, "Unknown digest algorithm");
		goto cleanup;
	}

	if ((spki = NETSCAPE_SPKI_new()) == NULL) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Unable to create new SPKAC");
		goto cleanup;
	}

	if (challenge) {
		if (!ASN1_STRING_set(spki->spkac->challenge, challenge, (int)challenge_len)) {
			crx_openssl_store_errors();
			crx_error_docref(NULL, E_WARNING, "Unable to set challenge data");
			goto cleanup;
		}
	}

	if (!NETSCAPE_SPKI_set_pubkey(spki, pkey)) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Unable to embed public key");
		goto cleanup;
	}

	if (!NETSCAPE_SPKI_sign(spki, pkey, mdtype)) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Unable to sign with specified digest algorithm");
		goto cleanup;
	}

	spkstr = NETSCAPE_SPKI_b64_encode(spki);
	if (!spkstr){
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Unable to encode SPKAC");
		goto cleanup;
	}

	s = crex_string_alloc(strlen(spkac) + strlen(spkstr), 0);
	sprintf(ZSTR_VAL(s), "%s%s", spkac, spkstr);
	ZSTR_LEN(s) = strlen(ZSTR_VAL(s));
	OPENSSL_free(spkstr);

	RETVAL_STR(s);
	goto cleanup;

cleanup:
	EVP_PKEY_free(pkey);
	if (spki != NULL) {
		NETSCAPE_SPKI_free(spki);
	}

	if (s && ZSTR_LEN(s) <= 0) {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ Verifies spki returns boolean */
CRX_FUNCTION(openssl_spki_verify)
{
	size_t spkstr_len;
	int i = 0, spkstr_cleaned_len = 0;
	char *spkstr, * spkstr_cleaned = NULL;

	EVP_PKEY *pkey = NULL;
	NETSCAPE_SPKI *spki = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &spkstr, &spkstr_len) == FAILURE) {
		RETURN_THROWS();
	}
	RETVAL_FALSE;

	spkstr_cleaned = emalloc(spkstr_len + 1);
	spkstr_cleaned_len = (int)(spkstr_len - crx_openssl_spki_cleanup(spkstr, spkstr_cleaned));

	if (spkstr_cleaned_len == 0) {
		crx_error_docref(NULL, E_WARNING, "Invalid SPKAC");
		goto cleanup;
	}

	spki = NETSCAPE_SPKI_b64_decode(spkstr_cleaned, spkstr_cleaned_len);
	if (spki == NULL) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Unable to decode supplied SPKAC");
		goto cleanup;
	}

	pkey = X509_PUBKEY_get(spki->spkac->pubkey);
	if (pkey == NULL) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Unable to acquire signed public key");
		goto cleanup;
	}

	i = NETSCAPE_SPKI_verify(spki, pkey);
	goto cleanup;

cleanup:
	if (spki != NULL) {
		NETSCAPE_SPKI_free(spki);
	}
	EVP_PKEY_free(pkey);
	if (spkstr_cleaned != NULL) {
		efree(spkstr_cleaned);
	}

	if (i > 0) {
		RETVAL_TRUE;
	} else {
		crx_openssl_store_errors();
	}
}
/* }}} */

/* {{{ Exports public key from existing spki to var */
CRX_FUNCTION(openssl_spki_export)
{
	size_t spkstr_len;
	char *spkstr, * spkstr_cleaned = NULL;
	int spkstr_cleaned_len;

	EVP_PKEY *pkey = NULL;
	NETSCAPE_SPKI *spki = NULL;
	BIO *out = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &spkstr, &spkstr_len) == FAILURE) {
		RETURN_THROWS();
	}
	RETVAL_FALSE;

	spkstr_cleaned = emalloc(spkstr_len + 1);
	spkstr_cleaned_len = (int)(spkstr_len - crx_openssl_spki_cleanup(spkstr, spkstr_cleaned));

	if (spkstr_cleaned_len == 0) {
		crx_error_docref(NULL, E_WARNING, "Invalid SPKAC");
		goto cleanup;
	}

	spki = NETSCAPE_SPKI_b64_decode(spkstr_cleaned, spkstr_cleaned_len);
	if (spki == NULL) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Unable to decode supplied SPKAC");
		goto cleanup;
	}

	pkey = X509_PUBKEY_get(spki->spkac->pubkey);
	if (pkey == NULL) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Unable to acquire signed public key");
		goto cleanup;
	}

	out = BIO_new(BIO_s_mem());
	if (out && PEM_write_bio_PUBKEY(out, pkey)) {
		BUF_MEM *bio_buf;

		BIO_get_mem_ptr(out, &bio_buf);
		RETVAL_STRINGL((char *)bio_buf->data, bio_buf->length);
	} else {
		crx_openssl_store_errors();
	}
	goto cleanup;

cleanup:

	if (spki != NULL) {
		NETSCAPE_SPKI_free(spki);
	}
	BIO_free_all(out);
	EVP_PKEY_free(pkey);
	if (spkstr_cleaned != NULL) {
		efree(spkstr_cleaned);
	}
}
/* }}} */

/* {{{ Exports spkac challenge from existing spki to var */
CRX_FUNCTION(openssl_spki_export_challenge)
{
	size_t spkstr_len;
	char *spkstr, * spkstr_cleaned = NULL;
	int spkstr_cleaned_len;

	NETSCAPE_SPKI *spki = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s", &spkstr, &spkstr_len) == FAILURE) {
		RETURN_THROWS();
	}
	RETVAL_FALSE;

	spkstr_cleaned = emalloc(spkstr_len + 1);
	spkstr_cleaned_len = (int)(spkstr_len - crx_openssl_spki_cleanup(spkstr, spkstr_cleaned));

	if (spkstr_cleaned_len == 0) {
		crx_error_docref(NULL, E_WARNING, "Invalid SPKAC");
		goto cleanup;
	}

	spki = NETSCAPE_SPKI_b64_decode(spkstr_cleaned, spkstr_cleaned_len);
	if (spki == NULL) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Unable to decode SPKAC");
		goto cleanup;
	}

	RETVAL_STRING((const char *)ASN1_STRING_get0_data(spki->spkac->challenge));
	goto cleanup;

cleanup:
	if (spkstr_cleaned != NULL) {
		efree(spkstr_cleaned);
	}
	if (spki) {
		NETSCAPE_SPKI_free(spki);
	}
}
/* }}} */

/* {{{ Exports a CERT to file or a var */
CRX_FUNCTION(openssl_x509_export)
{
	X509 *cert;
	crex_object *cert_obj;
	crex_string *cert_str;
	zval *zout;
	bool notext = 1;
	BIO * bio_out;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_OBJ_OF_CLASS_OR_STR(cert_obj, crx_openssl_certificate_ce, cert_str)
		C_PARAM_ZVAL(zout)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(notext)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_FALSE;

	cert = crx_openssl_x509_from_param(cert_obj, cert_str, 1);
	if (cert == NULL) {
		crx_error_docref(NULL, E_WARNING, "X.509 Certificate cannot be retrieved");
		return;
	}

	bio_out = BIO_new(BIO_s_mem());
	if (!bio_out) {
		crx_openssl_store_errors();
		goto cleanup;
	}
	if (!notext && !X509_print(bio_out, cert)) {
		crx_openssl_store_errors();
	}
	if (PEM_write_bio_X509(bio_out, cert)) {
		BUF_MEM *bio_buf;

		BIO_get_mem_ptr(bio_out, &bio_buf);
		CREX_TRY_ASSIGN_REF_STRINGL(zout, bio_buf->data, bio_buf->length);

		RETVAL_TRUE;
	} else {
		crx_openssl_store_errors();
	}

	BIO_free(bio_out);

cleanup:
	if (cert_str) {
		X509_free(cert);
	}
}
/* }}} */

crex_string* crx_openssl_x509_fingerprint(X509 *peer, const char *method, bool raw)
{
	unsigned char md[EVP_MAX_MD_SIZE];
	const EVP_MD *mdtype;
	unsigned int n;
	crex_string *ret;

	if (!(mdtype = EVP_get_digestbyname(method))) {
		crx_error_docref(NULL, E_WARNING, "Unknown digest algorithm");
		return NULL;
	} else if (!X509_digest(peer, mdtype, md, &n)) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_ERROR, "Could not generate signature");
		return NULL;
	}

	if (raw) {
		ret = crex_string_init((char*)md, n, 0);
	} else {
		ret = crex_string_alloc(n * 2, 0);
		make_digest_ex(ZSTR_VAL(ret), md, n);
		ZSTR_VAL(ret)[n * 2] = '\0';
	}

	return ret;
}

CRX_FUNCTION(openssl_x509_fingerprint)
{
	X509 *cert;
	crex_object *cert_obj;
	crex_string *cert_str;
	bool raw_output = 0;
	char *method = "sha1";
	size_t method_len;
	crex_string *fingerprint;

	CREX_PARSE_PARAMETERS_START(1, 3)
		C_PARAM_OBJ_OF_CLASS_OR_STR(cert_obj, crx_openssl_certificate_ce, cert_str)
		C_PARAM_OPTIONAL
		C_PARAM_STRING(method, method_len)
		C_PARAM_BOOL(raw_output)
	CREX_PARSE_PARAMETERS_END();

	cert = crx_openssl_x509_from_param(cert_obj, cert_str, 1);
	if (cert == NULL) {
		crx_error_docref(NULL, E_WARNING, "X.509 Certificate cannot be retrieved");
		RETURN_FALSE;
	}

	fingerprint = crx_openssl_x509_fingerprint(cert, method, raw_output);
	if (fingerprint) {
		RETVAL_STR(fingerprint);
	} else {
		RETVAL_FALSE;
	}

	if (cert_str) {
		X509_free(cert);
	}
}

/* {{{ Checks if a private key corresponds to a CERT */
CRX_FUNCTION(openssl_x509_check_private_key)
{
	X509 *cert;
	crex_object *cert_obj;
	crex_string *cert_str;
	zval *zkey;
	EVP_PKEY * key = NULL;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_OBJ_OF_CLASS_OR_STR(cert_obj, crx_openssl_certificate_ce, cert_str)
		C_PARAM_ZVAL(zkey)
	CREX_PARSE_PARAMETERS_END();

	cert = crx_openssl_x509_from_param(cert_obj, cert_str, 1);
	if (cert == NULL) {
		RETURN_FALSE;
	}

	RETVAL_FALSE;

	key = crx_openssl_pkey_from_zval(zkey, 0, "", 0, 2);
	if (key) {
		RETVAL_BOOL(X509_check_private_key(cert, key));
		EVP_PKEY_free(key);
	}

	if (cert_str) {
		X509_free(cert);
	}
}
/* }}} */

/* {{{ Verifies the signature of certificate cert using public key key */
CRX_FUNCTION(openssl_x509_verify)
{
	X509 *cert;
	crex_object *cert_obj;
	crex_string *cert_str;
	zval *zkey;
	EVP_PKEY * key = NULL;
	int err = -1;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_OBJ_OF_CLASS_OR_STR(cert_obj, crx_openssl_certificate_ce, cert_str)
		C_PARAM_ZVAL(zkey)
	CREX_PARSE_PARAMETERS_END();

	cert = crx_openssl_x509_from_param(cert_obj, cert_str, 1);
	if (cert == NULL) {
		RETURN_LONG(err);
	}

	key = crx_openssl_pkey_from_zval(zkey, 1, NULL, 0, 2);
	if (key != NULL) {
		err = X509_verify(cert, key);
		if (err < 0) {
			crx_openssl_store_errors();
		}

		EVP_PKEY_free(key);
	}

	if (cert_str) {
		X509_free(cert);
	}

	RETURN_LONG(err);
}
/* }}} */

/* Special handling of subjectAltName, see CVE-2013-4073
 * Christian Heimes
 */

static int openssl_x509v3_subjectAltName(BIO *bio, X509_EXTENSION *extension)
{
	GENERAL_NAMES *names;
	const X509V3_EXT_METHOD *method = NULL;
	ASN1_OCTET_STRING *extension_data;
	long i, length, num;
	const unsigned char *p;

	method = X509V3_EXT_get(extension);
	if (method == NULL) {
		return -1;
	}

	extension_data = X509_EXTENSION_get_data(extension);
	p = extension_data->data;
	length = extension_data->length;
	if (method->it) {
		names = (GENERAL_NAMES*) (ASN1_item_d2i(NULL, &p, length,
			ASN1_ITEM_ptr(method->it)));
	} else {
		names = (GENERAL_NAMES*) (method->d2i(NULL, &p, length));
	}
	if (names == NULL) {
		crx_openssl_store_errors();
		return -1;
	}

	num = sk_GENERAL_NAME_num(names);
	for (i = 0; i < num; i++) {
		GENERAL_NAME *name;
		ASN1_STRING *as;
		name = sk_GENERAL_NAME_value(names, i);
		switch (name->type) {
			case GEN_EMAIL:
				BIO_puts(bio, "email:");
				as = name->d.rfc822Name;
				BIO_write(bio, ASN1_STRING_get0_data(as),
					ASN1_STRING_length(as));
				break;
			case GEN_DNS:
				BIO_puts(bio, "DNS:");
				as = name->d.dNSName;
				BIO_write(bio, ASN1_STRING_get0_data(as),
					ASN1_STRING_length(as));
				break;
			case GEN_URI:
				BIO_puts(bio, "URI:");
				as = name->d.uniformResourceIdentifier;
				BIO_write(bio, ASN1_STRING_get0_data(as),
					ASN1_STRING_length(as));
				break;
			default:
				/* use builtin print for GEN_OTHERNAME, GEN_X400,
				 * GEN_EDIPARTY, GEN_DIRNAME, GEN_IPADD and GEN_RID
				 */
				GENERAL_NAME_print(bio, name);
			}
			/* trailing ', ' except for last element */
			if (i < (num - 1)) {
				BIO_puts(bio, ", ");
			}
	}
	sk_GENERAL_NAME_pop_free(names, GENERAL_NAME_free);

	return 0;
}

/* {{{ Returns an array of the fields/values of the CERT */
CRX_FUNCTION(openssl_x509_parse)
{
	X509 *cert;
	crex_object *cert_obj;
	crex_string *cert_str;
	int i, sig_nid;
	bool useshortnames = 1;
	char * tmpstr;
	zval subitem;
	X509_EXTENSION *extension;
	X509_NAME *subject_name;
	char *cert_name;
	char *extname;
	BIO *bio_out;
	BUF_MEM *bio_buf;
	ASN1_INTEGER *asn1_serial;
	BIGNUM *bn_serial;
	char *str_serial;
	char *hex_serial;
	char buf[256];

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_OBJ_OF_CLASS_OR_STR(cert_obj, crx_openssl_certificate_ce, cert_str)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(useshortnames)
	CREX_PARSE_PARAMETERS_END();

	cert = crx_openssl_x509_from_param(cert_obj, cert_str, 1);
	if (cert == NULL) {
		// TODO Add Warning?
		RETURN_FALSE;
	}
	array_init(return_value);

	subject_name = X509_get_subject_name(cert);
	cert_name = X509_NAME_oneline(subject_name, NULL, 0);
	add_assoc_string(return_value, "name", cert_name);
	OPENSSL_free(cert_name);

	crx_openssl_add_assoc_name_entry(return_value, "subject", subject_name, useshortnames);
	/* hash as used in CA directories to lookup cert by subject name */
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%08lx", X509_subject_name_hash(cert));
		add_assoc_string(return_value, "hash", buf);
	}

	crx_openssl_add_assoc_name_entry(return_value, "issuer", X509_get_issuer_name(cert), useshortnames);
	add_assoc_long(return_value, "version", X509_get_version(cert));

	asn1_serial = X509_get_serialNumber(cert);

	bn_serial = ASN1_INTEGER_to_BN(asn1_serial, NULL);
	/* Can return NULL on error or memory allocation failure */
	if (!bn_serial) {
		crx_openssl_store_errors();
		RETURN_FALSE;
	}

	hex_serial = BN_bn2hex(bn_serial);
	BN_free(bn_serial);
	/* Can return NULL on error or memory allocation failure */
	if (!hex_serial) {
		crx_openssl_store_errors();
		RETURN_FALSE;
	}

	str_serial = i2s_ASN1_INTEGER(NULL, asn1_serial);
	add_assoc_string(return_value, "serialNumber", str_serial);
	OPENSSL_free(str_serial);

	/* Return the hex representation of the serial number, as defined by OpenSSL */
	add_assoc_string(return_value, "serialNumberHex", hex_serial);
	OPENSSL_free(hex_serial);

	crx_openssl_add_assoc_asn1_string(return_value, "validFrom", 	X509_getm_notBefore(cert));
	crx_openssl_add_assoc_asn1_string(return_value, "validTo", 		X509_getm_notAfter(cert));

	add_assoc_long(return_value, "validFrom_time_t", crx_openssl_asn1_time_to_time_t(X509_getm_notBefore(cert)));
	add_assoc_long(return_value, "validTo_time_t",  crx_openssl_asn1_time_to_time_t(X509_getm_notAfter(cert)));

	tmpstr = (char *)X509_alias_get0(cert, NULL);
	if (tmpstr) {
		add_assoc_string(return_value, "alias", tmpstr);
	}

	sig_nid = X509_get_signature_nid(cert);
	add_assoc_string(return_value, "signatureTypeSN", (char*)OBJ_nid2sn(sig_nid));
	add_assoc_string(return_value, "signatureTypeLN", (char*)OBJ_nid2ln(sig_nid));
	add_assoc_long(return_value, "signatureTypeNID", sig_nid);
	array_init(&subitem);

	/* NOTE: the purposes are added as integer keys - the keys match up to the X509_PURPOSE_SSL_XXX defines
	   in x509v3.h */
	for (i = 0; i < X509_PURPOSE_get_count(); i++) {
		int id, purpset;
		char * pname;
		X509_PURPOSE * purp;
		zval subsub;

		array_init(&subsub);

		purp = X509_PURPOSE_get0(i);
		id = X509_PURPOSE_get_id(purp);

		purpset = X509_check_purpose(cert, id, 0);
		add_index_bool(&subsub, 0, purpset);

		purpset = X509_check_purpose(cert, id, 1);
		add_index_bool(&subsub, 1, purpset);

		pname = useshortnames ? X509_PURPOSE_get0_sname(purp) : X509_PURPOSE_get0_name(purp);
		add_index_string(&subsub, 2, pname);

		/* NOTE: if purpset > 1 then it's a warning - we should mention it ? */

		add_index_zval(&subitem, id, &subsub);
	}
	add_assoc_zval(return_value, "purposes", &subitem);

	array_init(&subitem);


	for (i = 0; i < X509_get_ext_count(cert); i++) {
		int nid;
		extension = X509_get_ext(cert, i);
		nid = OBJ_obj2nid(X509_EXTENSION_get_object(extension));
		if (nid != NID_undef) {
			extname = (char *)OBJ_nid2sn(OBJ_obj2nid(X509_EXTENSION_get_object(extension)));
		} else {
			OBJ_obj2txt(buf, sizeof(buf)-1, X509_EXTENSION_get_object(extension), 1);
			extname = buf;
		}
		bio_out = BIO_new(BIO_s_mem());
		if (bio_out == NULL) {
			crx_openssl_store_errors();
			RETURN_FALSE;
		}
		if (nid == NID_subject_alt_name) {
			if (openssl_x509v3_subjectAltName(bio_out, extension) == 0) {
				BIO_get_mem_ptr(bio_out, &bio_buf);
				add_assoc_stringl(&subitem, extname, bio_buf->data, bio_buf->length);
			} else {
				crex_array_destroy(C_ARR_P(return_value));
				BIO_free(bio_out);
				if (cert_str) {
					X509_free(cert);
				}
				RETURN_FALSE;
			}
		}
		else if (X509V3_EXT_print(bio_out, extension, 0, 0)) {
			BIO_get_mem_ptr(bio_out, &bio_buf);
			add_assoc_stringl(&subitem, extname, bio_buf->data, bio_buf->length);
		} else {
			crx_openssl_add_assoc_asn1_string(&subitem, extname, X509_EXTENSION_get_data(extension));
		}
		BIO_free(bio_out);
	}
	add_assoc_zval(return_value, "extensions", &subitem);
	if (cert_str) {
		X509_free(cert);
	}
}
/* }}} */

/* {{{ crx_openssl_load_all_certs_from_file */
static STACK_OF(X509) *crx_openssl_load_all_certs_from_file(
		char *cert_file, size_t cert_file_len, uint32_t arg_num)
{
	STACK_OF(X509_INFO) *sk=NULL;
	STACK_OF(X509) *stack=NULL, *ret=NULL;
	BIO *in=NULL;
	X509_INFO *xi;
	char cert_path[MAXPATHLEN];

	if(!(stack = sk_X509_new_null())) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_ERROR, "Memory allocation failure");
		goto end;
	}

	if (!crx_openssl_check_path(cert_file, cert_file_len, cert_path, arg_num)) {
		sk_X509_free(stack);
		goto end;
	}

	if (!(in = BIO_new_file(cert_path, CRX_OPENSSL_BIO_MODE_R(PKCS7_BINARY)))) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Error opening the file, %s", cert_path);
		sk_X509_free(stack);
		goto end;
	}

	/* This loads from a file, a stack of x509/crl/pkey sets */
	if (!(sk = PEM_X509_INFO_read_bio(in, NULL, NULL, NULL))) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Error reading the file, %s", cert_path);
		sk_X509_free(stack);
		goto end;
	}

	/* scan over it and pull out the certs */
	while (sk_X509_INFO_num(sk)) {
		xi=sk_X509_INFO_shift(sk);
		if (xi->x509 != NULL) {
			sk_X509_push(stack,xi->x509);
			xi->x509=NULL;
		}
		X509_INFO_free(xi);
	}
	if (!sk_X509_num(stack)) {
		crx_error_docref(NULL, E_WARNING, "No certificates in file, %s", cert_path);
		sk_X509_free(stack);
		goto end;
	}
	ret = stack;
end:
	BIO_free(in);
	sk_X509_INFO_free(sk);

	return ret;
}
/* }}} */

/* {{{ check_cert */
static int check_cert(X509_STORE *ctx, X509 *x, STACK_OF(X509) *untrustedchain, int purpose)
{
	int ret=0;
	X509_STORE_CTX *csc;

	csc = X509_STORE_CTX_new();
	if (csc == NULL) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_ERROR, "Memory allocation failure");
		return 0;
	}
	if (!X509_STORE_CTX_init(csc, ctx, x, untrustedchain)) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Certificate store initialization failed");
		return 0;
	}
	if (purpose >= 0 && !X509_STORE_CTX_set_purpose(csc, purpose)) {
		crx_openssl_store_errors();
	}
	ret = X509_verify_cert(csc);
	if (ret < 0) {
		crx_openssl_store_errors();
	}
	X509_STORE_CTX_free(csc);

	return ret;
}
/* }}} */

/* {{{ Checks the CERT to see if it can be used for the purpose in purpose. cainfo holds information about trusted CAs */
CRX_FUNCTION(openssl_x509_checkpurpose)
{
	X509 *cert;
	crex_object *cert_obj;
	crex_string *cert_str;
	zval *zcainfo = NULL;
	X509_STORE *cainfo = NULL;
	STACK_OF(X509) *untrustedchain = NULL;
	crex_long purpose;
	char * untrusted = NULL;
	size_t untrusted_len = 0;
	int ret;

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_OBJ_OF_CLASS_OR_STR(cert_obj, crx_openssl_certificate_ce, cert_str)
		C_PARAM_LONG(purpose)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY(zcainfo)
		C_PARAM_STRING_OR_NULL(untrusted, untrusted_len)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_LONG(-1);

	if (untrusted) {
		untrustedchain = crx_openssl_load_all_certs_from_file(untrusted, untrusted_len, 4);
		if (untrustedchain == NULL) {
			goto clean_exit;
		}
	}

	cainfo = crx_openssl_setup_verify(zcainfo, 3);
	if (cainfo == NULL) {
		goto clean_exit;
	}
	cert = crx_openssl_x509_from_param(cert_obj, cert_str, 1);
	if (cert == NULL) {
		// TODO Add Warning?
		goto clean_exit;
	}

	ret = check_cert(cainfo, cert, untrustedchain, (int)purpose);
	if (ret != 0 && ret != 1) {
		RETVAL_LONG(ret);
	} else {
		RETVAL_BOOL(ret);
	}
	if (cert_str) {
		X509_free(cert);
	}
clean_exit:
	if (cainfo) {
		X509_STORE_free(cainfo);
	}
	if (untrustedchain) {
		sk_X509_pop_free(untrustedchain, X509_free);
	}
}
/* }}} */

/* {{{ crx_openssl_setup_verify
 * calist is an array containing file and directory names.  create a
 * certificate store and add those certs to it for use in verification.
*/
static X509_STORE *crx_openssl_setup_verify(zval *calist, uint32_t arg_num)
{
	X509_STORE *store;
	X509_LOOKUP * dir_lookup, * file_lookup;
	int ndirs = 0, nfiles = 0;
	zval * item;
	crex_stat_t sb = {0};
	char file_path[MAXPATHLEN];

	store = X509_STORE_new();

	if (store == NULL) {
		crx_openssl_store_errors();
		return NULL;
	}

	if (calist && (C_TYPE_P(calist) == IS_ARRAY)) {
		CREX_HASH_FOREACH_VAL(C_ARRVAL_P(calist), item) {
			crex_string *str = zval_try_get_string(item);
			if (UNEXPECTED(!str)) {
				return NULL;
			}

			if (!crx_openssl_check_path_str_ex(str, file_path, arg_num, false, true, NULL)) {
				crex_string_release(str);
				continue;
			}
			crex_string_release(str);

			if (VCWD_STAT(file_path, &sb) == -1) {
				crx_error_docref(NULL, E_WARNING, "Unable to stat %s", file_path);
				continue;
			}

			if ((sb.st_mode & S_IFREG) == S_IFREG) {
				file_lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());
				if (file_lookup == NULL || !X509_LOOKUP_load_file(file_lookup, file_path, X509_FILETYPE_PEM)) {
					crx_openssl_store_errors();
					crx_error_docref(NULL, E_WARNING, "Error loading file %s", file_path);
				} else {
					nfiles++;
				}
				file_lookup = NULL;
			} else {
				dir_lookup = X509_STORE_add_lookup(store, X509_LOOKUP_hash_dir());
				if (dir_lookup == NULL || !X509_LOOKUP_add_dir(dir_lookup, file_path, X509_FILETYPE_PEM)) {
					crx_openssl_store_errors();
					crx_error_docref(NULL, E_WARNING, "Error loading directory %s", file_path);
				} else {
					ndirs++;
				}
				dir_lookup = NULL;
			}
		} CREX_HASH_FOREACH_END();
	}
	if (nfiles == 0) {
		file_lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());
		if (file_lookup == NULL || !X509_LOOKUP_load_file(file_lookup, NULL, X509_FILETYPE_DEFAULT)) {
			crx_openssl_store_errors();
		}
	}
	if (ndirs == 0) {
		dir_lookup = X509_STORE_add_lookup(store, X509_LOOKUP_hash_dir());
		if (dir_lookup == NULL || !X509_LOOKUP_add_dir(dir_lookup, NULL, X509_FILETYPE_DEFAULT)) {
			crx_openssl_store_errors();
		}
	}
	return store;
}
/* }}} */

/* {{{ Reads X.509 certificates */
CRX_FUNCTION(openssl_x509_read)
{
	X509 *cert;
	crx_openssl_certificate_object *x509_cert_obj;
	crex_object *cert_obj;
	crex_string *cert_str;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_OBJ_OF_CLASS_OR_STR(cert_obj, crx_openssl_certificate_ce, cert_str)
	CREX_PARSE_PARAMETERS_END();

	cert = crx_openssl_x509_from_param(cert_obj, cert_str, 1);
	if (cert == NULL) {
		crx_error_docref(NULL, E_WARNING, "X.509 Certificate cannot be retrieved");
		RETURN_FALSE;
	}

	object_init_ex(return_value, crx_openssl_certificate_ce);
	x509_cert_obj = C_OPENSSL_CERTIFICATE_P(return_value);
	x509_cert_obj->x509 = cert_obj ? X509_dup(cert) : cert;
}
/* }}} */

/* {{{ Frees X.509 certificates */
CRX_FUNCTION(openssl_x509_free)
{
	zval *x509;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_OBJECT_OF_CLASS(x509, crx_openssl_certificate_ce)
	CREX_PARSE_PARAMETERS_END();
}
/* }}} */

/* }}} */

/* Pop all X509 from Stack and free them, free the stack afterwards */
static void crx_sk_X509_free(STACK_OF(X509) * sk) /* {{{ */
{
	for (;;) {
		X509* x = sk_X509_pop(sk);
		if (!x) break;
		X509_free(x);
	}
	sk_X509_free(sk);
}
/* }}} */

static STACK_OF(X509) *crx_array_to_X509_sk(zval * zcerts, uint32_t arg_num, const char *option_name) /* {{{ */
{
	zval * zcertval;
	STACK_OF(X509) * sk = NULL;
	X509 * cert;
	bool free_cert;

	sk = sk_X509_new_null();

	/* get certs */
	if (C_TYPE_P(zcerts) == IS_ARRAY) {
		CREX_HASH_FOREACH_VAL(C_ARRVAL_P(zcerts), zcertval) {
			cert = crx_openssl_x509_from_zval(zcertval, &free_cert, arg_num, true, option_name);
			if (cert == NULL) {
				// TODO Add Warning?
				goto clean_exit;
			}

			if (!free_cert) {
				cert = X509_dup(cert);

				if (cert == NULL) {
					crx_openssl_store_errors();
					goto clean_exit;
				}

			}
			sk_X509_push(sk, cert);
		} CREX_HASH_FOREACH_END();
	} else {
		/* a single certificate */
		cert = crx_openssl_x509_from_zval(zcerts, &free_cert, arg_num, false, option_name);

		if (cert == NULL) {
			// TODO Add Warning?
			goto clean_exit;
		}

		if (!free_cert) {
			cert = X509_dup(cert);
			if (cert == NULL) {
				crx_openssl_store_errors();
				goto clean_exit;
			}
		}
		sk_X509_push(sk, cert);
	}

clean_exit:
	return sk;
}
/* }}} */

/* {{{ Creates and exports a PKCS to file */
CRX_FUNCTION(openssl_pkcs12_export_to_file)
{
	X509 *cert;
	crex_object *cert_obj;
	crex_string *cert_str;
	BIO * bio_out = NULL;
	PKCS12 * p12 = NULL;
	char * filename, file_path[MAXPATHLEN];
	char * friendly_name = NULL;
	size_t filename_len;
	char * pass;
	size_t pass_len;
	zval *zpkey = NULL, *args = NULL;
	EVP_PKEY *priv_key = NULL;
	zval * item;
	STACK_OF(X509) *ca = NULL;

	CREX_PARSE_PARAMETERS_START(4, 5)
		C_PARAM_OBJ_OF_CLASS_OR_STR(cert_obj, crx_openssl_certificate_ce, cert_str)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_ZVAL(zpkey)
		C_PARAM_STRING(pass, pass_len)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY(args)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_FALSE;

	cert = crx_openssl_x509_from_param(cert_obj, cert_str, 1);
	if (cert == NULL) {
		crx_error_docref(NULL, E_WARNING, "X.509 Certificate cannot be retrieved");
		return;
	}

	priv_key = crx_openssl_pkey_from_zval(zpkey, 0, "", 0, 3);
	if (priv_key == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Cannot get private key from parameter 3");
		}
		goto cleanup;
	}
	if (!X509_check_private_key(cert, priv_key)) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Private key does not correspond to cert");
		goto cleanup;
	}
	if (!crx_openssl_check_path(filename, filename_len, file_path, 2)) {
		goto cleanup;
	}

	/* parse extra config from args array, promote this to an extra function */
	if (args &&
		(item = crex_hash_str_find(C_ARRVAL_P(args), "friendly_name", sizeof("friendly_name")-1)) != NULL &&
		C_TYPE_P(item) == IS_STRING
	) {
		friendly_name = C_STRVAL_P(item);
	}
	/* certpbe (default RC2-40)
	   keypbe (default 3DES)
	   friendly_caname
	*/

	if (args && (item = crex_hash_str_find(C_ARRVAL_P(args), "extracerts", sizeof("extracerts")-1)) != NULL) {
		ca = crx_array_to_X509_sk(item, 5, "extracerts");
	}
	/* end parse extra config */

	/*PKCS12 *PKCS12_create(char *pass, char *name, EVP_PKEY *pkey, X509 *cert, STACK_OF(X509) *ca,
				int nid_key, int nid_cert, int iter, int mac_iter, int keytype);*/

	p12 = PKCS12_create(pass, friendly_name, priv_key, cert, ca, 0, 0, 0, 0, 0);
	if (p12 != NULL) {
		bio_out = BIO_new_file(file_path, CRX_OPENSSL_BIO_MODE_W(PKCS7_BINARY));
		if (bio_out != NULL) {
			if (i2d_PKCS12_bio(bio_out, p12) == 0) {
				crx_openssl_store_errors();
				crx_error_docref(NULL, E_WARNING, "Error writing to file %s", file_path);
			} else {
				RETVAL_TRUE;
			}
			BIO_free(bio_out);
		} else {
			crx_openssl_store_errors();
			crx_error_docref(NULL, E_WARNING, "Error opening file %s", file_path);
		}

		PKCS12_free(p12);
	} else {
		crx_openssl_store_errors();
	}

	crx_sk_X509_free(ca);

cleanup:
	EVP_PKEY_free(priv_key);

	if (cert_str) {
		X509_free(cert);
	}
}
/* }}} */

/* {{{ Creates and exports a PKCS12 to a var */
CRX_FUNCTION(openssl_pkcs12_export)
{
	X509 *cert;
	crex_object *cert_obj;
	crex_string *cert_str;
	BIO * bio_out;
	PKCS12 * p12 = NULL;
	zval *zout = NULL, *zpkey, *args = NULL;
	EVP_PKEY *priv_key = NULL;
	char * pass;
	size_t pass_len;
	char * friendly_name = NULL;
	zval * item;
	STACK_OF(X509) *ca = NULL;

	CREX_PARSE_PARAMETERS_START(4, 5)
		C_PARAM_OBJ_OF_CLASS_OR_STR(cert_obj, crx_openssl_certificate_ce, cert_str)
		C_PARAM_ZVAL(zout)
		C_PARAM_ZVAL(zpkey)
		C_PARAM_STRING(pass, pass_len)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY(args)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_FALSE;

	cert = crx_openssl_x509_from_param(cert_obj, cert_str, 1);
	if (cert == NULL) {
		crx_error_docref(NULL, E_WARNING, "X.509 Certificate cannot be retrieved");
		return;
	}

	priv_key = crx_openssl_pkey_from_zval(zpkey, 0, "", 0, 3);
	if (priv_key == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Cannot get private key from parameter 3");
		}
		goto cleanup;
	}
	if (!X509_check_private_key(cert, priv_key)) {
		crx_error_docref(NULL, E_WARNING, "Private key does not correspond to cert");
		goto cleanup;
	}

	/* parse extra config from args array, promote this to an extra function */
	if (args &&
		(item = crex_hash_str_find(C_ARRVAL_P(args), "friendly_name", sizeof("friendly_name")-1)) != NULL &&
		C_TYPE_P(item) == IS_STRING
	) {
		friendly_name = C_STRVAL_P(item);
	}

	if (args && (item = crex_hash_str_find(C_ARRVAL_P(args), "extracerts", sizeof("extracerts")-1)) != NULL) {
		ca = crx_array_to_X509_sk(item, 5, "extracerts");
	}
	/* end parse extra config */

	p12 = PKCS12_create(pass, friendly_name, priv_key, cert, ca, 0, 0, 0, 0, 0);

	if (p12 != NULL) {
		bio_out = BIO_new(BIO_s_mem());
		if (i2d_PKCS12_bio(bio_out, p12)) {
			BUF_MEM *bio_buf;

			BIO_get_mem_ptr(bio_out, &bio_buf);
			CREX_TRY_ASSIGN_REF_STRINGL(zout, bio_buf->data, bio_buf->length);

			RETVAL_TRUE;
		} else {
			crx_openssl_store_errors();
		}

		BIO_free(bio_out);
		PKCS12_free(p12);
	} else {
		crx_openssl_store_errors();
	}
	crx_sk_X509_free(ca);

cleanup:
	EVP_PKEY_free(priv_key);
	if (cert_str) {
		X509_free(cert);
	}
}
/* }}} */

/* {{{ Parses a PKCS12 to an array */
CRX_FUNCTION(openssl_pkcs12_read)
{
	zval *zout = NULL, zextracerts, zcert, zpkey;
	char *pass, *zp12;
	size_t pass_len, zp12_len;
	PKCS12 * p12 = NULL;
	EVP_PKEY * pkey = NULL;
	X509 * cert = NULL;
	STACK_OF(X509) * ca = NULL;
	BIO * bio_in = NULL;
	int i;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "szs", &zp12, &zp12_len, &zout, &pass, &pass_len) == FAILURE) {
		RETURN_THROWS();
	}

	RETVAL_FALSE;

	CRX_OPENSSL_CHECK_SIZE_T_TO_INT(zp12_len, pkcs12, 1);

	bio_in = BIO_new(BIO_s_mem());

	if (0 >= BIO_write(bio_in, zp12, (int)zp12_len)) {
		crx_openssl_store_errors();
		goto cleanup;
	}

	if (d2i_PKCS12_bio(bio_in, &p12) && PKCS12_parse(p12, pass, &pkey, &cert, &ca)) {
		BIO * bio_out;
		int cert_num;

		zout = crex_try_array_init(zout);
		if (!zout) {
			goto cleanup;
		}

		if (cert) {
			bio_out = BIO_new(BIO_s_mem());
			if (PEM_write_bio_X509(bio_out, cert)) {
				BUF_MEM *bio_buf;
				BIO_get_mem_ptr(bio_out, &bio_buf);
				ZVAL_STRINGL(&zcert, bio_buf->data, bio_buf->length);
				add_assoc_zval(zout, "cert", &zcert);
			} else {
				crx_openssl_store_errors();
			}
			BIO_free(bio_out);
		}

		if (pkey) {
			bio_out = BIO_new(BIO_s_mem());
			if (PEM_write_bio_PrivateKey(bio_out, pkey, NULL, NULL, 0, 0, NULL)) {
				BUF_MEM *bio_buf;
				BIO_get_mem_ptr(bio_out, &bio_buf);
				ZVAL_STRINGL(&zpkey, bio_buf->data, bio_buf->length);
				add_assoc_zval(zout, "pkey", &zpkey);
			} else {
				crx_openssl_store_errors();
			}
			BIO_free(bio_out);
		}

		cert_num = sk_X509_num(ca);
		if (ca && cert_num) {
			array_init(&zextracerts);

			for (i = 0; i < cert_num; i++) {
				zval zextracert;
				X509* aCA = sk_X509_pop(ca);
				if (!aCA) break;

				bio_out = BIO_new(BIO_s_mem());
				if (PEM_write_bio_X509(bio_out, aCA)) {
					BUF_MEM *bio_buf;
					BIO_get_mem_ptr(bio_out, &bio_buf);
					ZVAL_STRINGL(&zextracert, bio_buf->data, bio_buf->length);
					add_index_zval(&zextracerts, i, &zextracert);
				}

				X509_free(aCA);
				BIO_free(bio_out);
			}

			sk_X509_free(ca);
			add_assoc_zval(zout, "extracerts", &zextracerts);
		}

		RETVAL_TRUE;
	} else {
		crx_openssl_store_errors();
	}

cleanup:
	BIO_free(bio_in);
	EVP_PKEY_free(pkey);
	if (cert) {
		X509_free(cert);
	}
	if (p12) {
		PKCS12_free(p12);
	}
}
/* }}} */

/* {{{ x509 CSR functions */

/* {{{ crx_openssl_make_REQ */
static int crx_openssl_make_REQ(struct crx_x509_request * req, X509_REQ * csr, zval * dn, zval * attribs)
{
	STACK_OF(CONF_VALUE) * dn_sk, *attr_sk = NULL;
	char * str, *dn_sect, *attr_sect;

	dn_sect = NCONF_get_string(req->req_config, req->section_name, "distinguished_name");
	if (dn_sect == NULL) {
		crx_openssl_store_errors();
		return FAILURE;
	}
	dn_sk = NCONF_get_section(req->req_config, dn_sect);
	if (dn_sk == NULL) {
		crx_openssl_store_errors();
		return FAILURE;
	}
	attr_sect = crx_openssl_conf_get_string(req->req_config, req->section_name, "attributes");
	if (attr_sect == NULL) {
		attr_sk = NULL;
	} else {
		attr_sk = NCONF_get_section(req->req_config, attr_sect);
		if (attr_sk == NULL) {
			crx_openssl_store_errors();
			return FAILURE;
		}
	}
	/* setup the version number: version 1 */
	if (X509_REQ_set_version(csr, 0L)) {
		int i, nid;
		char * type;
		CONF_VALUE * v;
		X509_NAME * subj;
		zval * item;
		crex_string * strindex = NULL;

		subj = X509_REQ_get_subject_name(csr);
		/* apply values from the dn hash */
		CREX_HASH_FOREACH_STR_KEY_VAL(C_ARRVAL_P(dn), strindex, item) {
			if (strindex) {
				int nid = OBJ_txt2nid(ZSTR_VAL(strindex));
				if (nid != NID_undef) {
					crex_string *str_item = zval_try_get_string(item);
					if (UNEXPECTED(!str_item)) {
						return FAILURE;
					}
					if (!X509_NAME_add_entry_by_NID(subj, nid, MBSTRING_UTF8,
								(unsigned char*)ZSTR_VAL(str_item), -1, -1, 0))
					{
						crx_openssl_store_errors();
						crx_error_docref(NULL, E_WARNING,
							"dn: add_entry_by_NID %d -> %s (failed; check error"
							" queue and value of string_mask OpenSSL option "
							"if illegal characters are reported)",
							nid, ZSTR_VAL(str_item));
						crex_string_release(str_item);
						return FAILURE;
					}
					crex_string_release(str_item);
				} else {
					crx_error_docref(NULL, E_WARNING, "dn: %s is not a recognized name", ZSTR_VAL(strindex));
				}
			}
		} CREX_HASH_FOREACH_END();

		/* Finally apply defaults from config file */
		for(i = 0; i < sk_CONF_VALUE_num(dn_sk); i++) {
			size_t len;
			char buffer[200 + 1]; /*200 + \0 !*/

			v = sk_CONF_VALUE_value(dn_sk, i);
			type = v->name;

			len = strlen(type);
			if (len < sizeof("_default")) {
				continue;
			}
			len -= sizeof("_default") - 1;
			if (strcmp("_default", type + len) != 0) {
				continue;
			}
			if (len > 200) {
				len = 200;
			}
			memcpy(buffer, type, len);
			buffer[len] = '\0';
			type = buffer;

			/* Skip past any leading X. X: X, etc to allow for multiple
			 * instances */
			for (str = type; *str; str++) {
				if (*str == ':' || *str == ',' || *str == '.') {
					str++;
					if (*str) {
						type = str;
					}
					break;
				}
			}
			/* if it is already set, skip this */
			nid = OBJ_txt2nid(type);
			if (X509_NAME_get_index_by_NID(subj, nid, -1) >= 0) {
				continue;
			}
			if (!X509_NAME_add_entry_by_txt(subj, type, MBSTRING_UTF8, (unsigned char*)v->value, -1, -1, 0)) {
				crx_openssl_store_errors();
				crx_error_docref(NULL, E_WARNING, "add_entry_by_txt %s -> %s (failed)", type, v->value);
				return FAILURE;
			}
			if (!X509_NAME_entry_count(subj)) {
				crx_error_docref(NULL, E_WARNING, "No objects specified in config file");
				return FAILURE;
			}
		}
		if (attribs) {
			CREX_HASH_FOREACH_STR_KEY_VAL(C_ARRVAL_P(attribs), strindex, item) {
				int nid;

				if (NULL == strindex) {
					crx_error_docref(NULL, E_WARNING, "dn: numeric fild names are not supported");
					continue;
				}

				nid = OBJ_txt2nid(ZSTR_VAL(strindex));
				if (nid != NID_undef) {
					crex_string *str_item = zval_try_get_string(item);
					if (UNEXPECTED(!str_item)) {
						return FAILURE;
					}
					if (!X509_NAME_add_entry_by_NID(subj, nid, MBSTRING_UTF8, (unsigned char*)ZSTR_VAL(str_item), -1, -1, 0)) {
						crx_openssl_store_errors();
						crx_error_docref(NULL, E_WARNING, "attribs: add_entry_by_NID %d -> %s (failed)", nid, ZSTR_VAL(str_item));
						crex_string_release(str_item);
						return FAILURE;
					}
					crex_string_release(str_item);
				} else {
					crx_error_docref(NULL, E_WARNING, "dn: %s is not a recognized name", ZSTR_VAL(strindex));
				}
			} CREX_HASH_FOREACH_END();
			for (i = 0; i < sk_CONF_VALUE_num(attr_sk); i++) {
				v = sk_CONF_VALUE_value(attr_sk, i);
				/* if it is already set, skip this */
				nid = OBJ_txt2nid(v->name);
				if (X509_REQ_get_attr_by_NID(csr, nid, -1) >= 0) {
					continue;
				}
				if (!X509_REQ_add1_attr_by_txt(csr, v->name, MBSTRING_UTF8, (unsigned char*)v->value, -1)) {
					crx_openssl_store_errors();
					crx_error_docref(NULL, E_WARNING,
						"add1_attr_by_txt %s -> %s (failed; check error queue "
						"and value of string_mask OpenSSL option if illegal "
						"characters are reported)",
						v->name, v->value);
					return FAILURE;
				}
			}
		}
	} else {
		crx_openssl_store_errors();
	}

	if (!X509_REQ_set_pubkey(csr, req->priv_key)) {
		crx_openssl_store_errors();
	}
	return SUCCESS;
}
/* }}} */


static X509_REQ *crx_openssl_csr_from_str(crex_string *csr_str, uint32_t arg_num)
{
	X509_REQ * csr = NULL;
	char file_path[MAXPATHLEN];
	BIO * in;

	if (ZSTR_LEN(csr_str) > 7 && memcmp(ZSTR_VAL(csr_str), "file://", sizeof("file://") - 1) == 0) {
		if (!crx_openssl_check_path_str(csr_str, file_path, arg_num)) {
			return NULL;
		}
		in = BIO_new_file(file_path, CRX_OPENSSL_BIO_MODE_R(PKCS7_BINARY));
	} else {
		in = BIO_new_mem_buf(ZSTR_VAL(csr_str), (int) ZSTR_LEN(csr_str));
	}

	if (in == NULL) {
		crx_openssl_store_errors();
		return NULL;
	}

	csr = PEM_read_bio_X509_REQ(in, NULL,NULL,NULL);
	if (csr == NULL) {
		crx_openssl_store_errors();
	}

	BIO_free(in);

	return csr;
}

static X509_REQ *crx_openssl_csr_from_param(
		crex_object *csr_obj, crex_string *csr_str, uint32_t arg_num)
{
	if (csr_obj) {
		return crx_openssl_request_from_obj(csr_obj)->csr;
	}

	CREX_ASSERT(csr_str);

	return crx_openssl_csr_from_str(csr_str, arg_num);
}

/* {{{ Exports a CSR to file */
CRX_FUNCTION(openssl_csr_export_to_file)
{
	X509_REQ *csr;
	crex_object *csr_obj;
	crex_string *csr_str;
	bool notext = 1;
	char * filename = NULL;
	size_t filename_len;
	char file_path[MAXPATHLEN];
	BIO * bio_out;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_OBJ_OF_CLASS_OR_STR(csr_obj, crx_openssl_request_ce, csr_str)
		C_PARAM_PATH(filename, filename_len)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(notext)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_FALSE;

	csr = crx_openssl_csr_from_param(csr_obj, csr_str, 1);
	if (csr == NULL) {
		crx_error_docref(NULL, E_WARNING, "X.509 Certificate Signing Request cannot be retrieved");
		return;
	}

	if (!crx_openssl_check_path(filename, filename_len, file_path, 2)) {
		return;
	}

	bio_out = BIO_new_file(file_path, CRX_OPENSSL_BIO_MODE_W(PKCS7_BINARY));
	if (bio_out != NULL) {
		if (!notext && !X509_REQ_print(bio_out, csr)) {
			crx_openssl_store_errors();
		}
		if (!PEM_write_bio_X509_REQ(bio_out, csr)) {
			crx_error_docref(NULL, E_WARNING, "Error writing PEM to file %s", file_path);
			crx_openssl_store_errors();
		} else {
			RETVAL_TRUE;
		}
		BIO_free(bio_out);
	} else {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Error opening file %s", file_path);
	}

	if (csr_str) {
		X509_REQ_free(csr);
	}
}
/* }}} */

/* {{{ Exports a CSR to file or a var */
CRX_FUNCTION(openssl_csr_export)
{
	X509_REQ *csr;
	crex_object *csr_obj;
	crex_string *csr_str;
	zval *zout;
	bool notext = 1;
	BIO * bio_out;

	CREX_PARSE_PARAMETERS_START(2, 3)
		C_PARAM_OBJ_OF_CLASS_OR_STR(csr_obj, crx_openssl_request_ce, csr_str)
		C_PARAM_ZVAL(zout)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(notext)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_FALSE;

	csr = crx_openssl_csr_from_param(csr_obj, csr_str, 1);
	if (csr == NULL) {
		crx_error_docref(NULL, E_WARNING, "X.509 Certificate Signing Request cannot be retrieved");
		return;
	}

	/* export to a var */

	bio_out = BIO_new(BIO_s_mem());
	if (!notext && !X509_REQ_print(bio_out, csr)) {
		crx_openssl_store_errors();
	}

	if (PEM_write_bio_X509_REQ(bio_out, csr)) {
		BUF_MEM *bio_buf;

		BIO_get_mem_ptr(bio_out, &bio_buf);
		CREX_TRY_ASSIGN_REF_STRINGL(zout, bio_buf->data, bio_buf->length);

		RETVAL_TRUE;
	} else {
		crx_openssl_store_errors();
	}

	if (csr_str) {
		X509_REQ_free(csr);
	}
	BIO_free(bio_out);
}
/* }}} */

/* {{{ Signs a cert with another CERT */
CRX_FUNCTION(openssl_csr_sign)
{
	X509_REQ *csr;
	crex_object *csr_obj;
	crex_string *csr_str;

	crx_openssl_certificate_object *cert_object;
	crex_object *cert_obj;
	crex_string *cert_str;
	zval *zpkey, *args = NULL;
	crex_long num_days;
	crex_long serial = C_L(0);
	X509 *cert = NULL, *new_cert = NULL;
	EVP_PKEY * key = NULL, *priv_key = NULL;
	int i;
	struct crx_x509_request req;

	CREX_PARSE_PARAMETERS_START(4, 6)
		C_PARAM_OBJ_OF_CLASS_OR_STR(csr_obj, crx_openssl_request_ce, csr_str)
		C_PARAM_OBJ_OF_CLASS_OR_STR_OR_NULL(cert_obj, crx_openssl_certificate_ce, cert_str)
		C_PARAM_ZVAL(zpkey)
		C_PARAM_LONG(num_days)
		C_PARAM_OPTIONAL
		C_PARAM_ARRAY_OR_NULL(args)
		C_PARAM_LONG(serial)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_FALSE;

	csr = crx_openssl_csr_from_param(csr_obj, csr_str, 1);
	if (csr == NULL) {
		crx_error_docref(NULL, E_WARNING, "X.509 Certificate Signing Request cannot be retrieved");
		return;
	}

	CRX_SSL_REQ_INIT(&req);

	if (cert_str || cert_obj) {
		cert = crx_openssl_x509_from_param(cert_obj, cert_str, 2);
		if (cert == NULL) {
			crx_error_docref(NULL, E_WARNING, "X.509 Certificate cannot be retrieved");
			goto cleanup;
		}
	}

	priv_key = crx_openssl_pkey_from_zval(zpkey, 0, "", 0, 3);
	if (priv_key == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Cannot get private key from parameter 3");
		}
		goto cleanup;
	}
	if (cert && !X509_check_private_key(cert, priv_key)) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Private key does not correspond to signing cert");
		goto cleanup;
	}

	if (CRX_SSL_REQ_PARSE(&req, args) == FAILURE) {
		goto cleanup;
	}
	/* Check that the request matches the signature */
	key = X509_REQ_get_pubkey(csr);
	if (key == NULL) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Error unpacking public key");
		goto cleanup;
	}
	i = X509_REQ_verify(csr, key);

	if (i < 0) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Signature verification problems");
		goto cleanup;
	}
	else if (i == 0) {
		crx_error_docref(NULL, E_WARNING, "Signature did not match the certificate request");
		goto cleanup;
	}

	/* Now we can get on with it */

	new_cert = X509_new();
	if (new_cert == NULL) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "No memory");
		goto cleanup;
	}
	/* Version 3 cert */
	if (!X509_set_version(new_cert, 2)) {
		goto cleanup;
	}

#if CRX_OPENSSL_API_VERSION >= 0x10100 && !defined (LIBRESSL_VERSION_NUMBER)
	ASN1_INTEGER_set_int64(X509_get_serialNumber(new_cert), serial);
#else
	ASN1_INTEGER_set(X509_get_serialNumber(new_cert), serial);
#endif

	X509_set_subject_name(new_cert, X509_REQ_get_subject_name(csr));

	if (cert == NULL) {
		cert = new_cert;
	}
	if (!X509_set_issuer_name(new_cert, X509_get_subject_name(cert))) {
		crx_openssl_store_errors();
		goto cleanup;
	}
	X509_gmtime_adj(X509_getm_notBefore(new_cert), 0);
	X509_gmtime_adj(X509_getm_notAfter(new_cert), 60*60*24*(long)num_days);
	i = X509_set_pubkey(new_cert, key);
	if (!i) {
		crx_openssl_store_errors();
		goto cleanup;
	}
	if (req.extensions_section) {
		X509V3_CTX ctx;

		X509V3_set_ctx(&ctx, cert, new_cert, csr, NULL, 0);
		X509V3_set_nconf(&ctx, req.req_config);
		if (!X509V3_EXT_add_nconf(req.req_config, &ctx, req.extensions_section, new_cert)) {
			crx_openssl_store_errors();
			goto cleanup;
		}
	}

	/* Now sign it */
	if (!X509_sign(new_cert, priv_key, req.digest)) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Failed to sign it");
		goto cleanup;
	}

	object_init_ex(return_value, crx_openssl_certificate_ce);
	cert_object = C_OPENSSL_CERTIFICATE_P(return_value);
	cert_object->x509 = new_cert;

cleanup:

	if (cert == new_cert) {
		cert = NULL;
	}

	CRX_SSL_REQ_DISPOSE(&req);
	EVP_PKEY_free(priv_key);
	EVP_PKEY_free(key);
	if (csr_str) {
		X509_REQ_free(csr);
	}
	if (cert_str && cert) {
		X509_free(cert);
	}
}
/* }}} */

/* {{{ Generates a privkey and CSR */
CRX_FUNCTION(openssl_csr_new)
{
	struct crx_x509_request req;
	crx_openssl_request_object *x509_request_obj;
	zval *args = NULL, *dn, *attribs = NULL;
	zval *out_pkey;
	X509_REQ *csr = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "az|a!a!", &dn, &out_pkey, &args, &attribs) == FAILURE) {
		RETURN_THROWS();
	}
	RETVAL_FALSE;

	CRX_SSL_REQ_INIT(&req);

	if (CRX_SSL_REQ_PARSE(&req, args) == SUCCESS) {
		int we_made_the_key = 0;
		zval *out_pkey_val = out_pkey;
		ZVAL_DEREF(out_pkey_val);

		/* Generate or use a private key */
		if (C_TYPE_P(out_pkey_val) != IS_NULL) {
			req.priv_key = crx_openssl_pkey_from_zval(out_pkey_val, 0, NULL, 0, 2);
		}
		if (req.priv_key == NULL) {
			crx_openssl_generate_private_key(&req);
			we_made_the_key = 1;
		}
		if (req.priv_key == NULL) {
			crx_error_docref(NULL, E_WARNING, "Unable to generate a private key");
		} else {
			csr = X509_REQ_new();
			if (csr) {
				if (crx_openssl_make_REQ(&req, csr, dn, attribs) == SUCCESS) {
					X509V3_CTX ext_ctx;

					X509V3_set_ctx(&ext_ctx, NULL, NULL, csr, NULL, 0);
					X509V3_set_nconf(&ext_ctx, req.req_config);

					/* Add extensions */
					if (req.request_extensions_section && !X509V3_EXT_REQ_add_nconf(req.req_config,
								&ext_ctx, req.request_extensions_section, csr))
					{
						crx_openssl_store_errors();
						crx_error_docref(NULL, E_WARNING, "Error loading extension section %s", req.request_extensions_section);
					} else {
						RETVAL_TRUE;

						if (X509_REQ_sign(csr, req.priv_key, req.digest)) {
							object_init_ex(return_value, crx_openssl_request_ce);
							x509_request_obj = C_OPENSSL_REQUEST_P(return_value);
							x509_request_obj->csr = csr;
							csr = NULL;
						} else {
							crx_openssl_store_errors();
							crx_error_docref(NULL, E_WARNING, "Error signing request");
						}

						if (we_made_the_key) {
							/* and an object for the private key */
							zval zkey_object;
							crx_openssl_pkey_object_init(
								&zkey_object, req.priv_key, /* is_private */ true);
							CREX_TRY_ASSIGN_REF_TMP(out_pkey, &zkey_object);
							req.priv_key = NULL; /* make sure the cleanup code doesn't zap it! */
						}
					}
				}
			} else {
				crx_openssl_store_errors();
			}

		}
	}
	if (csr) {
		X509_REQ_free(csr);
	}
	CRX_SSL_REQ_DISPOSE(&req);
}
/* }}} */

/* {{{ Returns the subject of a CERT or FALSE on error */
CRX_FUNCTION(openssl_csr_get_subject)
{
	X509_REQ *csr;
	crex_object *csr_obj;
	crex_string *csr_str;
	bool use_shortnames = 1;
	X509_NAME *subject;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_OBJ_OF_CLASS_OR_STR(csr_obj, crx_openssl_request_ce, csr_str)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(use_shortnames)
	CREX_PARSE_PARAMETERS_END();

	csr = crx_openssl_csr_from_param(csr_obj, csr_str, 1);
	if (csr == NULL) {
		RETURN_FALSE;
	}

	subject = X509_REQ_get_subject_name(csr);

	array_init(return_value);
	crx_openssl_add_assoc_name_entry(return_value, NULL, subject, use_shortnames);

	if (csr_str) {
		X509_REQ_free(csr);
	}
}
/* }}} */

static EVP_PKEY *crx_openssl_extract_public_key(EVP_PKEY *priv_key)
{
	/* Extract public key portion by round-tripping through PEM. */
	BIO *bio = BIO_new(BIO_s_mem());
	if (!bio || !PEM_write_bio_PUBKEY(bio, priv_key)) {
		BIO_free(bio);
		return NULL;
	}

	EVP_PKEY *pub_key = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
	BIO_free(bio);
	return pub_key;
}

/* {{{ Returns the subject of a CERT or FALSE on error */
CRX_FUNCTION(openssl_csr_get_public_key)
{
	crex_object *csr_obj;
	crex_string *csr_str;
	bool use_shortnames = 1;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_OBJ_OF_CLASS_OR_STR(csr_obj, crx_openssl_request_ce, csr_str)
		C_PARAM_OPTIONAL
		C_PARAM_BOOL(use_shortnames)
	CREX_PARSE_PARAMETERS_END();

	X509_REQ *csr = crx_openssl_csr_from_param(csr_obj, csr_str, 1);
	if (csr == NULL) {
		RETURN_FALSE;
	}

	/* Retrieve the public key from the CSR */
	EVP_PKEY *orig_key = X509_REQ_get_pubkey(csr);
	EVP_PKEY *tpubkey = crx_openssl_extract_public_key(orig_key);
	EVP_PKEY_free(orig_key);

	if (csr_str) {
		/* We need to free the original CSR if it was freshly created */
		X509_REQ_free(csr);
	}

	if (tpubkey == NULL) {
		crx_openssl_store_errors();
		RETURN_FALSE;
	}

	crx_openssl_pkey_object_init(return_value, tpubkey, /* is_private */ false);
}
/* }}} */

/* }}} */

/* {{{ EVP Public/Private key functions */

struct crx_openssl_pem_password {
	char *key;
	int len;
};

/* {{{ crx_openssl_pem_password_cb */
static int crx_openssl_pem_password_cb(char *buf, int size, int rwflag, void *userdata)
{
	struct crx_openssl_pem_password *password = userdata;

	if (password == NULL || password->key == NULL) {
		return -1;
	}

	size = (password->len > size) ? size : password->len;
	memcpy(buf, password->key, size);

	return size;
}
/* }}} */

static EVP_PKEY *crx_openssl_pkey_from_zval(
		zval *val, int public_key, char *passphrase, size_t passphrase_len, uint32_t arg_num)
{
	EVP_PKEY *key = NULL;
	X509 *cert = NULL;
	bool free_cert = false, is_file = false;
	char file_path[MAXPATHLEN];
	zval tmp;

	ZVAL_NULL(&tmp);

#define TMP_CLEAN \
	if (C_TYPE(tmp) == IS_STRING) {\
		zval_ptr_dtor_str(&tmp); \
	} \
	return NULL;

	if (C_TYPE_P(val) == IS_ARRAY) {
		zval * zphrase;

		/* get passphrase */

		if ((zphrase = crex_hash_index_find(C_ARRVAL_P(val), 1)) == NULL) {
			crex_value_error("Key array must be of the form array(0 => key, 1 => phrase)");
			return NULL;
		}

		if (C_TYPE_P(zphrase) == IS_STRING) {
			passphrase = C_STRVAL_P(zphrase);
			passphrase_len = C_STRLEN_P(zphrase);
		} else {
			ZVAL_COPY(&tmp, zphrase);
			if (!try_convert_to_string(&tmp)) {
				return NULL;
			}

			passphrase = C_STRVAL(tmp);
			passphrase_len = C_STRLEN(tmp);
		}

		/* now set val to be the key param and continue */
		if ((val = crex_hash_index_find(C_ARRVAL_P(val), 0)) == NULL) {
			crex_value_error("Key array must be of the form array(0 => key, 1 => phrase)");
			TMP_CLEAN;
		}
	}

	if (C_TYPE_P(val) == IS_OBJECT && C_OBJCE_P(val) == crx_openssl_pkey_ce) {
		crx_openssl_pkey_object *obj = crx_openssl_pkey_from_obj(C_OBJ_P(val));
		key = obj->pkey;
		bool is_priv = obj->is_private;

		/* check whether it is actually a private key if requested */
		if (!public_key && !is_priv) {
			crx_error_docref(NULL, E_WARNING, "Supplied key param is a public key");
			TMP_CLEAN;
		}

		if (public_key && is_priv) {
			crx_error_docref(NULL, E_WARNING, "Don't know how to get public key from this private key");
			TMP_CLEAN;
		} else {
			if (C_TYPE(tmp) == IS_STRING) {
				zval_ptr_dtor_str(&tmp);
			}

			EVP_PKEY_up_ref(key);
			return key;
		}
	} else if (C_TYPE_P(val) == IS_OBJECT && C_OBJCE_P(val) == crx_openssl_certificate_ce) {
		cert = crx_openssl_certificate_from_obj(C_OBJ_P(val))->x509;
	} else {
		/* force it to be a string and check if it refers to a file */
		/* passing non string values leaks, object uses toString, it returns NULL
		 * See bug38255.crxt
		 */
		if (!(C_TYPE_P(val) == IS_STRING || C_TYPE_P(val) == IS_OBJECT)) {
			TMP_CLEAN;
		}
		if (!try_convert_to_string(val)) {
			TMP_CLEAN;
		}

		if (C_STRLEN_P(val) > 7 && memcmp(C_STRVAL_P(val), "file://", sizeof("file://") - 1) == 0) {
			if (!crx_openssl_check_path_str(C_STR_P(val), file_path, arg_num)) {
				TMP_CLEAN;
			}
			is_file = true;
		}
		/* it's an X509 file/cert of some kind, and we need to extract the data from that */
		if (public_key) {
            crx_openssl_errors_set_mark();
			cert = crx_openssl_x509_from_str(C_STR_P(val), arg_num, false, NULL);

			if (cert) {
				free_cert = 1;
			} else {
				/* not a X509 certificate, try to retrieve public key */
				crx_openssl_errors_restore_mark();
				BIO* in;
				if (is_file) {
					in = BIO_new_file(file_path, CRX_OPENSSL_BIO_MODE_R(PKCS7_BINARY));
				} else {
					in = BIO_new_mem_buf(C_STRVAL_P(val), (int)C_STRLEN_P(val));
				}
				if (in == NULL) {
					crx_openssl_store_errors();
					TMP_CLEAN;
				}
				key = PEM_read_bio_PUBKEY(in, NULL,NULL, NULL);
				BIO_free(in);
			}
		} else {
			/* we want the private key */
			BIO *in;

			if (is_file) {
				in = BIO_new_file(file_path, CRX_OPENSSL_BIO_MODE_R(PKCS7_BINARY));
			} else {
				in = BIO_new_mem_buf(C_STRVAL_P(val), (int)C_STRLEN_P(val));
			}

			if (in == NULL) {
				TMP_CLEAN;
			}
			if (passphrase == NULL) {
				key = PEM_read_bio_PrivateKey(in, NULL, NULL, NULL);
			} else {
				struct crx_openssl_pem_password password;
				password.key = passphrase;
				password.len = passphrase_len;
				key = PEM_read_bio_PrivateKey(in, NULL, crx_openssl_pem_password_cb, &password);
			}
			BIO_free(in);
		}
	}

	if (key == NULL) {
		crx_openssl_store_errors();

		if (public_key && cert) {
			/* extract public key from X509 cert */
			key = (EVP_PKEY *) X509_get_pubkey(cert);
			if (key == NULL) {
				crx_openssl_store_errors();
			}
		}
	}

	if (free_cert) {
		X509_free(cert);
	}

	if (C_TYPE(tmp) == IS_STRING) {
		zval_ptr_dtor_str(&tmp);
	}

	return key;
}

static int crx_openssl_get_evp_pkey_type(int key_type) {
	switch (key_type) {
	case OPENSSL_KEYTYPE_RSA:
		return EVP_PKEY_RSA;
#if !defined(OPENSSL_NO_DSA)
	case OPENSSL_KEYTYPE_DSA:
		return EVP_PKEY_DSA;
#endif
#if !defined(NO_DH)
	case OPENSSL_KEYTYPE_DH:
		return EVP_PKEY_DH;
#endif
#ifdef HAVE_EVP_PKEY_EC
	case OPENSSL_KEYTYPE_EC:
		return EVP_PKEY_EC;
#endif
	default:
		return -1;
	}
}

/* {{{ crx_openssl_generate_private_key */
static EVP_PKEY * crx_openssl_generate_private_key(struct crx_x509_request * req)
{
	if (req->priv_key_bits < MIN_KEY_LENGTH) {
		crx_error_docref(NULL, E_WARNING, "Private key length must be at least %d bits, configured to %d",
			MIN_KEY_LENGTH, req->priv_key_bits);
		return NULL;
	}

	int type = crx_openssl_get_evp_pkey_type(req->priv_key_type);
	if (type < 0) {
		crx_error_docref(NULL, E_WARNING, "Unsupported private key type");
		return NULL;
	}

	int egdsocket, seeded;
	char *randfile = crx_openssl_conf_get_string(req->req_config, req->section_name, "RANDFILE");
	crx_openssl_load_rand_file(randfile, &egdsocket, &seeded);
	CRX_OPENSSL_RAND_ADD_TIME();

	EVP_PKEY *key = NULL;
	EVP_PKEY *params = NULL;
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(type, NULL);
	if (!ctx) {
		crx_openssl_store_errors();
		goto cleanup;
	}

	if (type != EVP_PKEY_RSA) {
		if (EVP_PKEY_paramgen_init(ctx) <= 0) {
			crx_openssl_store_errors();
			goto cleanup;
		}

		switch (type) {
#if !defined(OPENSSL_NO_DSA)
		case EVP_PKEY_DSA:
			if (EVP_PKEY_CTX_set_dsa_paramgen_bits(ctx, req->priv_key_bits) <= 0) {
				crx_openssl_store_errors();
				goto cleanup;
			}
			break;
#endif
#if !defined(NO_DH)
		case EVP_PKEY_DH:
			if (EVP_PKEY_CTX_set_dh_paramgen_prime_len(ctx, req->priv_key_bits) <= 0) {
				crx_openssl_store_errors();
				goto cleanup;
			}
			break;
#endif
#ifdef HAVE_EVP_PKEY_EC
		case EVP_PKEY_EC:
			if (req->curve_name == NID_undef) {
				crx_error_docref(NULL, E_WARNING, "Missing configuration value: \"curve_name\" not set");
				goto cleanup;
			}

			if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, req->curve_name) <= 0 ||
					EVP_PKEY_CTX_set_ec_param_enc(ctx, OPENSSL_EC_NAMED_CURVE) <= 0) {
				crx_openssl_store_errors();
				goto cleanup;
			}
			break;
#endif
		EMPTY_SWITCH_DEFAULT_CASE()
		}

		if (EVP_PKEY_paramgen(ctx, &params) <= 0) {
			crx_openssl_store_errors();
			goto cleanup;
		}

		EVP_PKEY_CTX_free(ctx);
		ctx = EVP_PKEY_CTX_new(params, NULL);
		if (!ctx) {
			crx_openssl_store_errors();
			goto cleanup;
		}
	}

	if (EVP_PKEY_keygen_init(ctx) <= 0) {
		crx_openssl_store_errors();
		goto cleanup;
	}

	if (type == EVP_PKEY_RSA && EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, req->priv_key_bits) <= 0) {
		crx_openssl_store_errors();
		goto cleanup;
	}

	if (EVP_PKEY_keygen(ctx, &key) <= 0) {
		crx_openssl_store_errors();
		goto cleanup;
	}

	req->priv_key = key;

cleanup:
	crx_openssl_write_rand_file(randfile, egdsocket, seeded);
	EVP_PKEY_free(params);
	EVP_PKEY_CTX_free(ctx);
	return key;
}
/* }}} */

static void crx_openssl_add_bn_to_array(zval *ary, const BIGNUM *bn, const char *name) {
	if (bn != NULL) {
		int len = BN_num_bytes(bn);
		crex_string *str = crex_string_alloc(len, 0);
		BN_bn2bin(bn, (unsigned char *)ZSTR_VAL(str));
		ZSTR_VAL(str)[len] = 0;
		add_assoc_str(ary, name, str);
	}
}

#define OPENSSL_PKEY_GET_BN(_type, _name) crx_openssl_add_bn_to_array(&_type, _name, #_name)

#define OPENSSL_PKEY_SET_BN(_data, _name) do { \
		zval *bn; \
		if ((bn = crex_hash_str_find(C_ARRVAL_P(_data), #_name, sizeof(#_name)-1)) != NULL && \
				C_TYPE_P(bn) == IS_STRING) { \
			_name = BN_bin2bn( \
				(unsigned char*)C_STRVAL_P(bn), \
				(int)C_STRLEN_P(bn), NULL); \
		} else { \
			_name = NULL; \
		} \
	} while (0);

#if CRX_OPENSSL_API_VERSION < 0x30000
static bool crx_openssl_pkey_init_legacy_rsa(RSA *rsa, zval *data)
{
	BIGNUM *n, *e, *d, *p, *q, *dmp1, *dmq1, *iqmp;

	OPENSSL_PKEY_SET_BN(data, n);
	OPENSSL_PKEY_SET_BN(data, e);
	OPENSSL_PKEY_SET_BN(data, d);
	if (!n || !d || !RSA_set0_key(rsa, n, e, d)) {
		return 0;
	}

	OPENSSL_PKEY_SET_BN(data, p);
	OPENSSL_PKEY_SET_BN(data, q);
	if ((p || q) && !RSA_set0_factors(rsa, p, q)) {
		return 0;
	}

	OPENSSL_PKEY_SET_BN(data, dmp1);
	OPENSSL_PKEY_SET_BN(data, dmq1);
	OPENSSL_PKEY_SET_BN(data, iqmp);
	if ((dmp1 || dmq1 || iqmp) && !RSA_set0_crt_params(rsa, dmp1, dmq1, iqmp)) {
		return 0;
	}

	return 1;
}
#endif

static EVP_PKEY *crx_openssl_pkey_init_rsa(zval *data)
{
#if CRX_OPENSSL_API_VERSION >= 0x30000
	BIGNUM *n = NULL, *e = NULL, *d = NULL, *p = NULL, *q = NULL;
	BIGNUM *dmp1 = NULL, *dmq1 = NULL, *iqmp = NULL;
	EVP_PKEY *pkey = NULL;
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
	OSSL_PARAM *params = NULL;
	OSSL_PARAM_BLD *bld = OSSL_PARAM_BLD_new();

	OPENSSL_PKEY_SET_BN(data, n);
	OPENSSL_PKEY_SET_BN(data, e);
	OPENSSL_PKEY_SET_BN(data, d);
	OPENSSL_PKEY_SET_BN(data, p);
	OPENSSL_PKEY_SET_BN(data, q);
	OPENSSL_PKEY_SET_BN(data, dmp1);
	OPENSSL_PKEY_SET_BN(data, dmq1);
	OPENSSL_PKEY_SET_BN(data, iqmp);

	if (!ctx || !bld || !n || !d) {
		goto cleanup;
	}

	OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_N, n);
	OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_D, d);
	if (e) {
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_E, e);
	}
	if (p) {
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_FACTOR1, p);
	}
	if (q) {
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_FACTOR2, q);
	}
	if (dmp1) {
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_EXPONENT1, dmp1);
	}
	if (dmq1) {
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_EXPONENT2, dmq1);
	}
	if (iqmp) {
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_COEFFICIENT1, iqmp);
	}

	params = OSSL_PARAM_BLD_to_param(bld);
	if (!params) {
		goto cleanup;
	}

	if (EVP_PKEY_fromdata_init(ctx) <= 0 ||
			EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_KEYPAIR, params) <= 0) {
		goto cleanup;
	}

cleanup:
	crx_openssl_store_errors();
	EVP_PKEY_CTX_free(ctx);
	OSSL_PARAM_free(params);
	OSSL_PARAM_BLD_free(bld);
	BN_free(n);
	BN_free(e);
	BN_free(d);
	BN_free(p);
	BN_free(q);
	BN_free(dmp1);
	BN_free(dmq1);
	BN_free(iqmp);
	return pkey;
#else
	EVP_PKEY *pkey = EVP_PKEY_new();
	if (!pkey) {
		crx_openssl_store_errors();
		return NULL;
	}

	RSA *rsa = RSA_new();
	if (!rsa) {
		crx_openssl_store_errors();
		EVP_PKEY_free(pkey);
		return NULL;
	}

	if (!crx_openssl_pkey_init_legacy_rsa(rsa, data)
			|| !EVP_PKEY_assign_RSA(pkey, rsa)) {
		crx_openssl_store_errors();
		EVP_PKEY_free(pkey);
		RSA_free(rsa);
		return NULL;
	}

	return pkey;
#endif
}

#if CRX_OPENSSL_API_VERSION < 0x30000
static bool crx_openssl_pkey_init_legacy_dsa(DSA *dsa, zval *data, bool *is_private)
{
	BIGNUM *p, *q, *g, *priv_key, *pub_key;
	const BIGNUM *priv_key_const, *pub_key_const;

	OPENSSL_PKEY_SET_BN(data, p);
	OPENSSL_PKEY_SET_BN(data, q);
	OPENSSL_PKEY_SET_BN(data, g);
	if (!p || !q || !g || !DSA_set0_pqg(dsa, p, q, g)) {
		return 0;
	}

	OPENSSL_PKEY_SET_BN(data, pub_key);
	OPENSSL_PKEY_SET_BN(data, priv_key);
	*is_private = priv_key != NULL;
	if (pub_key) {
		return DSA_set0_key(dsa, pub_key, priv_key);
	}

	/* generate key */
	CRX_OPENSSL_RAND_ADD_TIME();
	if (!DSA_generate_key(dsa)) {
		crx_openssl_store_errors();
		return 0;
	}

	/* if BN_mod_exp return -1, then DSA_generate_key succeed for failed key
	 * so we need to double check that public key is created */
	DSA_get0_key(dsa, &pub_key_const, &priv_key_const);
	if (!pub_key_const || BN_is_zero(pub_key_const)) {
		return 0;
	}
	/* all good */
	*is_private = true;
	return 1;
}
#endif

static EVP_PKEY *crx_openssl_pkey_init_dsa(zval *data, bool *is_private)
{
#if CRX_OPENSSL_API_VERSION >= 0x30000
	BIGNUM *p = NULL, *q = NULL, *g = NULL, *priv_key = NULL, *pub_key = NULL;
	EVP_PKEY *param_key = NULL, *pkey = NULL;
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_DSA, NULL);
	OSSL_PARAM *params = NULL;
	OSSL_PARAM_BLD *bld = OSSL_PARAM_BLD_new();

	OPENSSL_PKEY_SET_BN(data, p);
	OPENSSL_PKEY_SET_BN(data, q);
	OPENSSL_PKEY_SET_BN(data, g);
	OPENSSL_PKEY_SET_BN(data, priv_key);
	OPENSSL_PKEY_SET_BN(data, pub_key);

	*is_private = false;

	if (!ctx || !bld || !p || !q || !g) {
		goto cleanup;
	}

	OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_FFC_P, p);
	OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_FFC_Q, q);
	OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_FFC_G, g);
	// TODO: We silently ignore priv_key if pub_key is not given, unlike in the DH case.
	if (pub_key) {
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_PUB_KEY, pub_key);
		if (priv_key) {
			OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_PRIV_KEY, priv_key);
		}
	}

	params = OSSL_PARAM_BLD_to_param(bld);
	if (!params) {
		goto cleanup;
	}

	if (EVP_PKEY_fromdata_init(ctx) <= 0 ||
			EVP_PKEY_fromdata(ctx, &param_key, EVP_PKEY_KEYPAIR, params) <= 0) {
		goto cleanup;
	}

	if (pub_key) {
		*is_private = priv_key != NULL;
		EVP_PKEY_up_ref(param_key);
		pkey = param_key;
	} else {
		*is_private = true;
		CRX_OPENSSL_RAND_ADD_TIME();
		EVP_PKEY_CTX_free(ctx);
		ctx = EVP_PKEY_CTX_new(param_key, NULL);
		if (EVP_PKEY_keygen_init(ctx) <= 0 || EVP_PKEY_keygen(ctx, &pkey) <= 0) {
			goto cleanup;
		}
	}

cleanup:
	crx_openssl_store_errors();
	EVP_PKEY_free(param_key);
	EVP_PKEY_CTX_free(ctx);
	OSSL_PARAM_free(params);
	OSSL_PARAM_BLD_free(bld);
	BN_free(p);
	BN_free(q);
	BN_free(g);
	BN_free(priv_key);
	BN_free(pub_key);
	return pkey;
#else
	EVP_PKEY *pkey = EVP_PKEY_new();
	if (!pkey) {
		crx_openssl_store_errors();
		return NULL;
	}

	DSA *dsa = DSA_new();
	if (!dsa) {
		crx_openssl_store_errors();
		EVP_PKEY_free(pkey);
		return NULL;
	}

	if (!crx_openssl_pkey_init_legacy_dsa(dsa, data, is_private)
			|| !EVP_PKEY_assign_DSA(pkey, dsa)) {
		crx_openssl_store_errors();
		EVP_PKEY_free(pkey);
		DSA_free(dsa);
		return NULL;
	}

	return pkey;
#endif
}

/* {{{ crx_openssl_dh_pub_from_priv */
static BIGNUM *crx_openssl_dh_pub_from_priv(BIGNUM *priv_key, BIGNUM *g, BIGNUM *p)
{
	BIGNUM *pub_key, *priv_key_const_time;
	BN_CTX *ctx;

	pub_key = BN_new();
	if (pub_key == NULL) {
		crx_openssl_store_errors();
		return NULL;
	}

	priv_key_const_time = BN_new();
	if (priv_key_const_time == NULL) {
		BN_free(pub_key);
		crx_openssl_store_errors();
		return NULL;
	}
	ctx = BN_CTX_new();
	if (ctx == NULL) {
		BN_free(pub_key);
		BN_free(priv_key_const_time);
		crx_openssl_store_errors();
		return NULL;
	}

	BN_with_flags(priv_key_const_time, priv_key, BN_FLG_CONSTTIME);

	if (!BN_mod_exp_mont(pub_key, g, priv_key_const_time, p, ctx, NULL)) {
		BN_free(pub_key);
		crx_openssl_store_errors();
		pub_key = NULL;
	}

	BN_free(priv_key_const_time);
	BN_CTX_free(ctx);

	return pub_key;
}
/* }}} */

#if CRX_OPENSSL_API_VERSION < 0x30000
static bool crx_openssl_pkey_init_legacy_dh(DH *dh, zval *data, bool *is_private)
{
	BIGNUM *p, *q, *g, *priv_key, *pub_key;

	OPENSSL_PKEY_SET_BN(data, p);
	OPENSSL_PKEY_SET_BN(data, q);
	OPENSSL_PKEY_SET_BN(data, g);
	if (!p || !g || !DH_set0_pqg(dh, p, q, g)) {
		return 0;
	}

	OPENSSL_PKEY_SET_BN(data, priv_key);
	OPENSSL_PKEY_SET_BN(data, pub_key);
	*is_private = priv_key != NULL;
	if (pub_key) {
		return DH_set0_key(dh, pub_key, priv_key);
	}
	if (priv_key) {
		pub_key = crx_openssl_dh_pub_from_priv(priv_key, g, p);
		if (pub_key == NULL) {
			return 0;
		}
		return DH_set0_key(dh, pub_key, priv_key);
	}

	/* generate key */
	CRX_OPENSSL_RAND_ADD_TIME();
	if (!DH_generate_key(dh)) {
		crx_openssl_store_errors();
		return 0;
	}
	/* all good */
	*is_private = true;
	return 1;
}
#endif

static EVP_PKEY *crx_openssl_pkey_init_dh(zval *data, bool *is_private)
{
#if CRX_OPENSSL_API_VERSION >= 0x30000
	BIGNUM *p = NULL, *q = NULL, *g = NULL, *priv_key = NULL, *pub_key = NULL;
	EVP_PKEY *param_key = NULL, *pkey = NULL;
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_DH, NULL);
	OSSL_PARAM *params = NULL;
	OSSL_PARAM_BLD *bld = OSSL_PARAM_BLD_new();

	OPENSSL_PKEY_SET_BN(data, p);
	OPENSSL_PKEY_SET_BN(data, q);
	OPENSSL_PKEY_SET_BN(data, g);
	OPENSSL_PKEY_SET_BN(data, priv_key);
	OPENSSL_PKEY_SET_BN(data, pub_key);

	*is_private = false;

	if (!ctx || !bld || !p || !g) {
		goto cleanup;
	}

	OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_FFC_P, p);
	OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_FFC_G, g);
	if (q) {
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_FFC_Q, q);
	}
	if (priv_key) {
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_PRIV_KEY, priv_key);
		if (!pub_key) {
			pub_key = crx_openssl_dh_pub_from_priv(priv_key, g, p);
			if (!pub_key) {
				goto cleanup;
			}
		}
	}
	if (pub_key) {
		OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_PUB_KEY, pub_key);
	}

	params = OSSL_PARAM_BLD_to_param(bld);
	if (!params) {
		goto cleanup;
	}

	if (EVP_PKEY_fromdata_init(ctx) <= 0 ||
			EVP_PKEY_fromdata(ctx, &param_key, EVP_PKEY_KEYPAIR, params) <= 0) {
		goto cleanup;
	}

	if (pub_key || priv_key) {
		*is_private = priv_key != NULL;
		EVP_PKEY_up_ref(param_key);
		pkey = param_key;
	} else {
		*is_private = true;
		CRX_OPENSSL_RAND_ADD_TIME();
		EVP_PKEY_CTX_free(ctx);
		ctx = EVP_PKEY_CTX_new(param_key, NULL);
		if (EVP_PKEY_keygen_init(ctx) <= 0 || EVP_PKEY_keygen(ctx, &pkey) <= 0) {
			goto cleanup;
		}
	}

cleanup:
	crx_openssl_store_errors();
	EVP_PKEY_free(param_key);
	EVP_PKEY_CTX_free(ctx);
	OSSL_PARAM_free(params);
	OSSL_PARAM_BLD_free(bld);
	BN_free(p);
	BN_free(q);
	BN_free(g);
	BN_free(priv_key);
	BN_free(pub_key);
	return pkey;
#else
	EVP_PKEY *pkey = EVP_PKEY_new();
	if (!pkey) {
		crx_openssl_store_errors();
		return NULL;
	}

	DH *dh = DH_new();
	if (!dh) {
		EVP_PKEY_free(pkey);
		return NULL;
	}

	if (!crx_openssl_pkey_init_legacy_dh(dh, data, is_private)
			|| !EVP_PKEY_assign_DH(pkey, dh)) {
		crx_openssl_store_errors();
		EVP_PKEY_free(pkey);
		DH_free(dh);
		return NULL;
	}

	return pkey;
#endif
}

#ifdef HAVE_EVP_PKEY_EC
#if CRX_OPENSSL_API_VERSION < 0x30000
static bool crx_openssl_pkey_init_legacy_ec(EC_KEY *eckey, zval *data, bool *is_private) {
	BIGNUM *p = NULL, *a = NULL, *b = NULL, *order = NULL, *g_x = NULL, *g_y = NULL , *cofactor = NULL;
	BIGNUM *x = NULL, *y = NULL, *d = NULL;
	EC_POINT *point_g = NULL;
	EC_POINT *point_q = NULL;
	EC_GROUP *group = NULL;
	BN_CTX *bctx = BN_CTX_new();

	*is_private = false;

	zval *curve_name_zv = crex_hash_str_find(C_ARRVAL_P(data), "curve_name", sizeof("curve_name") - 1);
	if (curve_name_zv && C_TYPE_P(curve_name_zv) == IS_STRING && C_STRLEN_P(curve_name_zv) > 0) {
		int nid = OBJ_sn2nid(C_STRVAL_P(curve_name_zv));
		if (nid == NID_undef) {
			crx_error_docref(NULL, E_WARNING, "Unknown elliptic curve (short) name %s", C_STRVAL_P(curve_name_zv));
			goto clean_exit;
		}

		if (!(group = EC_GROUP_new_by_curve_name(nid))) {
			goto clean_exit;
		}
		EC_GROUP_set_asn1_flag(group, OPENSSL_EC_NAMED_CURVE);
	} else {
		OPENSSL_PKEY_SET_BN(data, p);
		OPENSSL_PKEY_SET_BN(data, a);
		OPENSSL_PKEY_SET_BN(data, b);
		OPENSSL_PKEY_SET_BN(data, order);

		if (!(p && a && b && order)) {
			if (!p && !a && !b && !order) {
				crx_error_docref(NULL, E_WARNING, "Missing params: curve_name");
			} else {
				crx_error_docref(
					NULL, E_WARNING, "Missing params: curve_name or p, a, b, order");
			}
			goto clean_exit;
		}

		if (!(group = EC_GROUP_new_curve_GFp(p, a, b, bctx))) {
			goto clean_exit;
		}

		if (!(point_g = EC_POINT_new(group))) {
			goto clean_exit;
		}

		zval *generator_zv = crex_hash_str_find(C_ARRVAL_P(data), "generator", sizeof("generator") - 1);
		if (generator_zv && C_TYPE_P(generator_zv) == IS_STRING && C_STRLEN_P(generator_zv) > 0) {
			if (!(EC_POINT_oct2point(group, point_g, (unsigned char *)C_STRVAL_P(generator_zv), C_STRLEN_P(generator_zv), bctx))) {
				goto clean_exit;
			}
		} else {
			OPENSSL_PKEY_SET_BN(data, g_x);
			OPENSSL_PKEY_SET_BN(data, g_y);

			if (!g_x || !g_y) {
				crx_error_docref(
					NULL, E_WARNING, "Missing params: generator or g_x and g_y");
				goto clean_exit;
			}

			if (!EC_POINT_set_affine_coordinates_GFp(group, point_g, g_x, g_y, bctx)) {
				goto clean_exit;
			}
		}

		zval *seed_zv = crex_hash_str_find(C_ARRVAL_P(data), "seed", sizeof("seed") - 1);
		if (seed_zv && C_TYPE_P(seed_zv) == IS_STRING && C_STRLEN_P(seed_zv) > 0) {
			if (!EC_GROUP_set_seed(group, (unsigned char *)C_STRVAL_P(seed_zv), C_STRLEN_P(seed_zv))) {
				goto clean_exit;
			}
		}

		/*
		 * OpenSSL uses 0 cofactor as a marker for "unknown cofactor".
		 * So accept cofactor == NULL or cofactor >= 0.
		 * Internally, the lib will check the cofactor value.
		 */
		OPENSSL_PKEY_SET_BN(data, cofactor);
		if (!EC_GROUP_set_generator(group, point_g, order, cofactor)) {
			goto clean_exit;
		}
		EC_GROUP_set_asn1_flag(group, OPENSSL_EC_EXPLICIT_CURVE);
	}

	EC_GROUP_set_point_conversion_form(group, POINT_CONVERSION_UNCOMPRESSED);

	if (!EC_KEY_set_group(eckey, group)) {
		goto clean_exit;
	}

	OPENSSL_PKEY_SET_BN(data, d);
	OPENSSL_PKEY_SET_BN(data, x);
	OPENSSL_PKEY_SET_BN(data, y);

	if (d) {
		*is_private = true;
		if (!EC_KEY_set_private_key(eckey, d)) {
			goto clean_exit;
		}

		point_q = EC_POINT_new(group);
		if (!point_q || !EC_POINT_mul(group, point_q, d, NULL, NULL, bctx)) {
			goto clean_exit;
		}
	} else if (x && y) {
		/* OpenSSL does not allow setting EC_PUB_X/EC_PUB_Y, so convert to encoded format. */
		point_q = EC_POINT_new(group);
		if (!point_q || !EC_POINT_set_affine_coordinates_GFp(group, point_q, x, y, bctx)) {
			goto clean_exit;
		}
	}

	if (point_q != NULL) {
		if (!EC_KEY_set_public_key(eckey, point_q)) {
			goto clean_exit;
		}
	}

	if (!EC_KEY_check_key(eckey)) {
		*is_private = true;
		CRX_OPENSSL_RAND_ADD_TIME();
		EC_KEY_generate_key(eckey);
	}

clean_exit:
	crx_openssl_store_errors();
	BN_CTX_free(bctx);
	EC_GROUP_free(group);
	EC_POINT_free(point_g);
	EC_POINT_free(point_q);
	BN_free(p);
	BN_free(a);
	BN_free(b);
	BN_free(order);
	BN_free(g_x);
	BN_free(g_y);
	BN_free(cofactor);
	BN_free(d);
	BN_free(x);
	BN_free(y);
	return EC_KEY_check_key(eckey);
}
#endif

static EVP_PKEY *crx_openssl_pkey_init_ec(zval *data, bool *is_private) {
#if CRX_OPENSSL_API_VERSION >= 0x30000
	int nid = NID_undef;
	BIGNUM *p = NULL, *a = NULL, *b = NULL, *order = NULL, *g_x = NULL, *g_y = NULL, *cofactor = NULL;
	BIGNUM *x = NULL, *y = NULL, *d = NULL;
	EC_POINT *point_g = NULL;
	EC_POINT *point_q = NULL;
	unsigned char *point_g_buf = NULL;
	unsigned char *point_q_buf = NULL;
	EC_GROUP *group = NULL;
	EVP_PKEY *param_key = NULL, *pkey = NULL;
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
	BN_CTX *bctx = BN_CTX_new();
	OSSL_PARAM *params = NULL;
	OSSL_PARAM_BLD *bld = OSSL_PARAM_BLD_new();

	*is_private = false;

	zval *curve_name_zv = crex_hash_str_find(C_ARRVAL_P(data), "curve_name", sizeof("curve_name") - 1);
	if (curve_name_zv && C_TYPE_P(curve_name_zv) == IS_STRING && C_STRLEN_P(curve_name_zv) > 0) {
		nid = OBJ_sn2nid(C_STRVAL_P(curve_name_zv));
		if (nid == NID_undef) {
			crx_error_docref(NULL, E_WARNING, "Unknown elliptic curve (short) name %s", C_STRVAL_P(curve_name_zv));
			goto cleanup;
		}

		if (!(group = EC_GROUP_new_by_curve_name(nid))) {
			goto cleanup;
		}

		if (!OSSL_PARAM_BLD_push_utf8_string(bld, OSSL_PKEY_PARAM_GROUP_NAME, C_STRVAL_P(curve_name_zv), C_STRLEN_P(curve_name_zv))) {
			goto cleanup;
		}
	} else {
		OPENSSL_PKEY_SET_BN(data, p);
		OPENSSL_PKEY_SET_BN(data, a);
		OPENSSL_PKEY_SET_BN(data, b);
		OPENSSL_PKEY_SET_BN(data, order);

		if (!(p && a && b && order)) {
			if (!p && !a && !b && !order) {
				crx_error_docref(NULL, E_WARNING, "Missing params: curve_name");
			} else {
				crx_error_docref(NULL, E_WARNING, "Missing params: curve_name or p, a, b, order");
			}
			goto cleanup;
		}

		if (!OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_EC_P, p) ||
			!OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_EC_A, a) ||
			!OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_EC_B, b) ||
			!OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_EC_ORDER, order) ||
			!OSSL_PARAM_BLD_push_utf8_string(bld, OSSL_PKEY_PARAM_EC_FIELD_TYPE, SN_X9_62_prime_field, 0)) {
				goto cleanup;
			}

		if (!(group = EC_GROUP_new_curve_GFp(p, a, b, bctx))) {
			goto cleanup;
		}

		if (!(point_g = EC_POINT_new(group))) {
			goto cleanup;
		}

		zval *generator_zv = crex_hash_str_find(C_ARRVAL_P(data), "generator", sizeof("generator") - 1);
		if (generator_zv && C_TYPE_P(generator_zv) == IS_STRING && C_STRLEN_P(generator_zv) > 0) {
			if (!EC_POINT_oct2point(group, point_g, (unsigned char *)C_STRVAL_P(generator_zv), C_STRLEN_P(generator_zv), bctx) ||
				!OSSL_PARAM_BLD_push_octet_string(bld, OSSL_PKEY_PARAM_EC_GENERATOR, C_STRVAL_P(generator_zv), C_STRLEN_P(generator_zv))) {
				goto cleanup;
			}
		} else {
			OPENSSL_PKEY_SET_BN(data, g_x);
			OPENSSL_PKEY_SET_BN(data, g_y);

			if (!g_x || !g_y) {
				crx_error_docref(
					NULL, E_WARNING, "Missing params: generator or g_x and g_y");
				goto cleanup;
			}

			if (!EC_POINT_set_affine_coordinates(group, point_g, g_x, g_y, bctx)) {
				goto cleanup;
			}

			size_t point_g_buf_len =
				EC_POINT_point2buf(group, point_g, POINT_CONVERSION_COMPRESSED, &point_g_buf, bctx);
			if (!point_g_buf_len) {
				goto cleanup;
			}

			if (!OSSL_PARAM_BLD_push_octet_string(bld, OSSL_PKEY_PARAM_EC_GENERATOR, point_g_buf, point_g_buf_len)) {
				goto cleanup;
			}
		}

		zval *seed_zv = crex_hash_str_find(C_ARRVAL_P(data), "seed", sizeof("seed") - 1);
		if (seed_zv && C_TYPE_P(seed_zv) == IS_STRING && C_STRLEN_P(seed_zv) > 0) {
			if (!EC_GROUP_set_seed(group, (unsigned char *)C_STRVAL_P(seed_zv), C_STRLEN_P(seed_zv)) ||
				!OSSL_PARAM_BLD_push_octet_string(bld, OSSL_PKEY_PARAM_EC_SEED, C_STRVAL_P(seed_zv), C_STRLEN_P(seed_zv))) {
				goto cleanup;
			}
		}

		OPENSSL_PKEY_SET_BN(data, cofactor);
		if (!OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_EC_COFACTOR, cofactor) ||
			!EC_GROUP_set_generator(group, point_g, order, cofactor)) {
			goto cleanup;
		}

		nid = EC_GROUP_check_named_curve(group, 0, bctx);
	}

	/* custom params not supported with SM2, SKIP */
	if (nid != NID_sm2) {
		OPENSSL_PKEY_SET_BN(data, d);
		OPENSSL_PKEY_SET_BN(data, x);
		OPENSSL_PKEY_SET_BN(data, y);

		if (d) {
			point_q = EC_POINT_new(group);
			if (!point_q || !EC_POINT_mul(group, point_q, d, NULL, NULL, bctx) ||
				!OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_PRIV_KEY, d)) {
				goto cleanup;
			}
		} else if (x && y) {
			/* OpenSSL does not allow setting EC_PUB_X/EC_PUB_Y, so convert to encoded format. */
			point_q = EC_POINT_new(group);
			if (!point_q || !EC_POINT_set_affine_coordinates(group, point_q, x, y, bctx)) {
				goto cleanup;
			}
		}

		if (point_q) {
			size_t point_q_buf_len =
				EC_POINT_point2buf(group, point_q, POINT_CONVERSION_COMPRESSED, &point_q_buf, bctx);
			if (!point_q_buf_len ||
				!OSSL_PARAM_BLD_push_octet_string(bld, OSSL_PKEY_PARAM_PUB_KEY, point_q_buf, point_q_buf_len)) {
				goto cleanup;
			}
		}
	}

	params = OSSL_PARAM_BLD_to_param(bld);
	if (!params) {
		goto cleanup;
	}

	if (d || (x && y)) {
		if (EVP_PKEY_fromdata_init(ctx) <= 0 ||
			EVP_PKEY_fromdata(ctx, &param_key, EVP_PKEY_KEYPAIR, params) <= 0) {
			goto cleanup;
		}
		EVP_PKEY_CTX_free(ctx);
		ctx = EVP_PKEY_CTX_new(param_key, NULL);
	}
	
	if (EVP_PKEY_check(ctx) || EVP_PKEY_public_check_quick(ctx)) {
		*is_private = d != NULL;
		EVP_PKEY_up_ref(param_key);
		pkey = param_key;
	} else {
		*is_private = true;
		CRX_OPENSSL_RAND_ADD_TIME();
		if (EVP_PKEY_keygen_init(ctx) != 1 ||
				EVP_PKEY_CTX_set_params(ctx, params) != 1 ||
				EVP_PKEY_generate(ctx, &pkey) != 1) {
			goto cleanup;
		}
	}

cleanup:
	crx_openssl_store_errors();
	EVP_PKEY_free(param_key);
	EVP_PKEY_CTX_free(ctx);
	BN_CTX_free(bctx);
	OSSL_PARAM_free(params);
	OSSL_PARAM_BLD_free(bld);
	EC_GROUP_free(group);
	EC_POINT_free(point_g);
	EC_POINT_free(point_q);
	OPENSSL_free(point_g_buf);
	OPENSSL_free(point_q_buf);
	BN_free(p);
	BN_free(a);
	BN_free(b);
	BN_free(order);
	BN_free(g_x);
	BN_free(g_y);
	BN_free(cofactor);
	BN_free(d);
	BN_free(x);
	BN_free(y);
	return pkey;
#else
	EVP_PKEY *pkey = EVP_PKEY_new();
	if (!pkey) {
		crx_openssl_store_errors();
		return NULL;
	}

	EC_KEY *ec = EC_KEY_new();
	if (!ec) {
		EVP_PKEY_free(pkey);
		return NULL;
	}

	if (!crx_openssl_pkey_init_legacy_ec(ec, data, is_private)
			|| !EVP_PKEY_assign_EC_KEY(pkey, ec)) {
		crx_openssl_store_errors();
		EVP_PKEY_free(pkey);
		EC_KEY_free(ec);
		return NULL;
	}

	return pkey;
#endif
}
#endif

/* {{{ Generates a new private key */
CRX_FUNCTION(openssl_pkey_new)
{
	struct crx_x509_request req;
	zval * args = NULL;
	zval *data;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|a!", &args) == FAILURE) {
		RETURN_THROWS();
	}
	RETVAL_FALSE;

	if (args && C_TYPE_P(args) == IS_ARRAY) {
		EVP_PKEY *pkey;

		if ((data = crex_hash_str_find(C_ARRVAL_P(args), "rsa", sizeof("rsa")-1)) != NULL &&
			C_TYPE_P(data) == IS_ARRAY) {
			pkey = crx_openssl_pkey_init_rsa(data);
			if (!pkey) {
				RETURN_FALSE;
			}
			crx_openssl_pkey_object_init(return_value, pkey, /* is_private */ true);
			return;
		} else if ((data = crex_hash_str_find(C_ARRVAL_P(args), "dsa", sizeof("dsa") - 1)) != NULL &&
			C_TYPE_P(data) == IS_ARRAY) {
			bool is_private;
			pkey = crx_openssl_pkey_init_dsa(data, &is_private);
			if (!pkey) {
				RETURN_FALSE;
			}
			crx_openssl_pkey_object_init(return_value, pkey, is_private);
			return;
		} else if ((data = crex_hash_str_find(C_ARRVAL_P(args), "dh", sizeof("dh") - 1)) != NULL &&
			C_TYPE_P(data) == IS_ARRAY) {
			bool is_private;
			pkey = crx_openssl_pkey_init_dh(data, &is_private);
			if (!pkey) {
				RETURN_FALSE;
			}
			crx_openssl_pkey_object_init(return_value, pkey, is_private);
			return;
#ifdef HAVE_EVP_PKEY_EC
		} else if ((data = crex_hash_str_find(C_ARRVAL_P(args), "ec", sizeof("ec") - 1)) != NULL &&
			C_TYPE_P(data) == IS_ARRAY) {
			bool is_private;
			pkey = crx_openssl_pkey_init_ec(data, &is_private);
			if (!pkey) {
				RETURN_FALSE;
			}
			crx_openssl_pkey_object_init(return_value, pkey, is_private);
			return;
#endif
		}
	}

	CRX_SSL_REQ_INIT(&req);

	if (CRX_SSL_REQ_PARSE(&req, args) == SUCCESS) {
		if (crx_openssl_generate_private_key(&req)) {
			/* pass back a key resource */
			crx_openssl_pkey_object_init(return_value, req.priv_key, /* is_private */ true);
			/* make sure the cleanup code doesn't zap it! */
			req.priv_key = NULL;
		}
	}
	CRX_SSL_REQ_DISPOSE(&req);
}
/* }}} */

/* {{{ Gets an exportable representation of a key into a file */
CRX_FUNCTION(openssl_pkey_export_to_file)
{
	struct crx_x509_request req;
	zval * zpkey, * args = NULL;
	char * passphrase = NULL;
	size_t passphrase_len = 0;
	char * filename = NULL, file_path[MAXPATHLEN];
	size_t filename_len = 0;
	int pem_write = 0;
	EVP_PKEY * key;
	BIO * bio_out = NULL;
	const EVP_CIPHER * cipher;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "zp|s!a!", &zpkey, &filename, &filename_len, &passphrase, &passphrase_len, &args) == FAILURE) {
		RETURN_THROWS();
	}
	RETVAL_FALSE;

	CRX_OPENSSL_CHECK_SIZE_T_TO_INT(passphrase_len, passphrase, 3);

	key = crx_openssl_pkey_from_zval(zpkey, 0, passphrase, passphrase_len, 1);
	if (key == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Cannot get key from parameter 1");
		}
		RETURN_FALSE;
	}

	if (!crx_openssl_check_path(filename, filename_len, file_path, 2)) {
		RETURN_FALSE;
	}

	CRX_SSL_REQ_INIT(&req);

	if (CRX_SSL_REQ_PARSE(&req, args) == SUCCESS) {
		bio_out = BIO_new_file(file_path, CRX_OPENSSL_BIO_MODE_W(PKCS7_BINARY));
		if (bio_out == NULL) {
			crx_openssl_store_errors();
			goto clean_exit;
		}

		if (passphrase && req.priv_key_encrypt) {
			if (req.priv_key_encrypt_cipher) {
				cipher = req.priv_key_encrypt_cipher;
			} else {
				cipher = (EVP_CIPHER *) EVP_des_ede3_cbc();
			}
		} else {
			cipher = NULL;
		}

		pem_write = PEM_write_bio_PrivateKey(
				bio_out, key, cipher,
				(unsigned char *)passphrase, (int)passphrase_len, NULL, NULL);
		if (pem_write) {
			/* Success!
			 * If returning the output as a string, do so now */
			RETVAL_TRUE;
		} else {
			crx_openssl_store_errors();
		}
	}

clean_exit:
	CRX_SSL_REQ_DISPOSE(&req);
	EVP_PKEY_free(key);
	BIO_free(bio_out);
}
/* }}} */

/* {{{ Gets an exportable representation of a key into a string or file */
CRX_FUNCTION(openssl_pkey_export)
{
	struct crx_x509_request req;
	zval * zpkey, * args = NULL, *out;
	char * passphrase = NULL; size_t passphrase_len = 0;
	int pem_write = 0;
	EVP_PKEY * key;
	BIO * bio_out = NULL;
	const EVP_CIPHER * cipher;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "zz|s!a!", &zpkey, &out, &passphrase, &passphrase_len, &args) == FAILURE) {
		RETURN_THROWS();
	}
	RETVAL_FALSE;

	CRX_OPENSSL_CHECK_SIZE_T_TO_INT(passphrase_len, passphrase, 3);

	key = crx_openssl_pkey_from_zval(zpkey, 0, passphrase, passphrase_len, 1);
	if (key == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Cannot get key from parameter 1");
		}
		RETURN_FALSE;
	}

	CRX_SSL_REQ_INIT(&req);

	if (CRX_SSL_REQ_PARSE(&req, args) == SUCCESS) {
		bio_out = BIO_new(BIO_s_mem());

		if (passphrase && req.priv_key_encrypt) {
			if (req.priv_key_encrypt_cipher) {
				cipher = req.priv_key_encrypt_cipher;
			} else {
				cipher = (EVP_CIPHER *) EVP_des_ede3_cbc();
			}
		} else {
			cipher = NULL;
		}

		pem_write = PEM_write_bio_PrivateKey(
				bio_out, key, cipher,
				(unsigned char *)passphrase, (int)passphrase_len, NULL, NULL);
		if (pem_write) {
			/* Success!
			 * If returning the output as a string, do so now */

			char * bio_mem_ptr;
			long bio_mem_len;
			RETVAL_TRUE;

			bio_mem_len = BIO_get_mem_data(bio_out, &bio_mem_ptr);
			CREX_TRY_ASSIGN_REF_STRINGL(out, bio_mem_ptr, bio_mem_len);
		} else {
			crx_openssl_store_errors();
		}
	}
	CRX_SSL_REQ_DISPOSE(&req);
	EVP_PKEY_free(key);
	BIO_free(bio_out);
}
/* }}} */

/* {{{ Gets public key from X.509 certificate */
CRX_FUNCTION(openssl_pkey_get_public)
{
	zval *cert;
	EVP_PKEY *pkey;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "z", &cert) == FAILURE) {
		RETURN_THROWS();
	}
	pkey = crx_openssl_pkey_from_zval(cert, 1, NULL, 0, 1);
	if (pkey == NULL) {
		RETURN_FALSE;
	}

	crx_openssl_pkey_object_init(return_value, pkey, /* is_private */ false);
}
/* }}} */

/* {{{ Frees a key */
CRX_FUNCTION(openssl_pkey_free)
{
	zval *key;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &key, crx_openssl_pkey_ce) == FAILURE) {
		RETURN_THROWS();
	}
}
/* }}} */

/* {{{ Gets private keys */
CRX_FUNCTION(openssl_pkey_get_private)
{
	zval *cert;
	EVP_PKEY *pkey;
	char * passphrase = "";
	size_t passphrase_len = sizeof("")-1;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "z|s!", &cert, &passphrase, &passphrase_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (passphrase) {
		CRX_OPENSSL_CHECK_SIZE_T_TO_INT(passphrase_len, passphrase, 2);
	}

	pkey = crx_openssl_pkey_from_zval(cert, 0, passphrase, passphrase_len, 1);
	if (pkey == NULL) {
		RETURN_FALSE;
	}

	crx_openssl_pkey_object_init(return_value, pkey, /* is_private */ true);
}

/* }}} */

#if CRX_OPENSSL_API_VERSION >= 0x30000
static void crx_openssl_copy_bn_param(
		zval *ary, EVP_PKEY *pkey, const char *param, const char *name) {
	BIGNUM *bn = NULL;
	if (EVP_PKEY_get_bn_param(pkey, param, &bn) > 0) {
		crx_openssl_add_bn_to_array(ary, bn, name);
		BN_free(bn);
	}
}

#ifdef HAVE_EVP_PKEY_EC
static crex_string *crx_openssl_get_utf8_param(
		EVP_PKEY *pkey, const char *param, const char *name) {
	char buf[64];
	size_t len;
	if (EVP_PKEY_get_utf8_string_param(pkey, param, buf, sizeof(buf), &len) > 0) {
		crex_string *str = crex_string_alloc(len, 0);
		memcpy(ZSTR_VAL(str), buf, len);
		ZSTR_VAL(str)[len] = '\0';
		return str;
	}
	return NULL;
}
#endif
#endif

/* {{{ returns an array with the key details (bits, pkey, type)*/
CRX_FUNCTION(openssl_pkey_get_details)
{
	zval *key;
	unsigned int pbio_len;
	char *pbio;
	crex_long ktype;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &key, crx_openssl_pkey_ce) == FAILURE) {
		RETURN_THROWS();
	}

	EVP_PKEY *pkey = C_OPENSSL_PKEY_P(key)->pkey;

	BIO *out = BIO_new(BIO_s_mem());
	if (!PEM_write_bio_PUBKEY(out, pkey)) {
		BIO_free(out);
		crx_openssl_store_errors();
		RETURN_FALSE;
	}
	pbio_len = BIO_get_mem_data(out, &pbio);

	array_init(return_value);
	add_assoc_long(return_value, "bits", EVP_PKEY_bits(pkey));
	add_assoc_stringl(return_value, "key", pbio, pbio_len);
	/*TODO: Use the real values once the openssl constants are used
	 * See the enum at the top of this file
	 */
#if CRX_OPENSSL_API_VERSION >= 0x30000
	zval ary;
	int base_id = 0;

	if (EVP_PKEY_id(pkey) != EVP_PKEY_KEYMGMT) {
		base_id = EVP_PKEY_base_id(pkey);
	} else {
		const char *type_name = EVP_PKEY_get0_type_name(pkey);
		if (type_name) {
			int nid = OBJ_txt2nid(type_name);
			if (nid != NID_undef) {
				base_id = EVP_PKEY_type(nid);
			}
		}
	}

	switch (base_id) {
		case EVP_PKEY_RSA:
			ktype = OPENSSL_KEYTYPE_RSA;
			array_init(&ary);
			add_assoc_zval(return_value, "rsa", &ary);
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_RSA_N, "n");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_RSA_E, "e");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_RSA_D, "d");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_RSA_FACTOR1, "p");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_RSA_FACTOR2, "q");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_RSA_EXPONENT1, "dmp1");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_RSA_EXPONENT2, "dmq1");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_RSA_COEFFICIENT1, "iqmp");
			break;
		case EVP_PKEY_DSA:
			ktype = OPENSSL_KEYTYPE_DSA;
			array_init(&ary);
			add_assoc_zval(return_value, "dsa", &ary);
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_FFC_P, "p");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_FFC_Q, "q");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_FFC_G, "g");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_PRIV_KEY, "priv_key");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_PUB_KEY, "pub_key");
			break;
		case EVP_PKEY_DH:
			ktype = OPENSSL_KEYTYPE_DH;
			array_init(&ary);
			add_assoc_zval(return_value, "dh", &ary);
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_FFC_P, "p");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_FFC_G, "g");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_PRIV_KEY, "priv_key");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_PUB_KEY, "pub_key");
			break;
#ifdef HAVE_EVP_PKEY_EC
		case EVP_PKEY_EC: {
			ktype = OPENSSL_KEYTYPE_EC;
			array_init(&ary);
			add_assoc_zval(return_value, "ec", &ary);

			crex_string *curve_name = crx_openssl_get_utf8_param(
				pkey, OSSL_PKEY_PARAM_GROUP_NAME, "curve_name");
			if (curve_name) {
				add_assoc_str(&ary, "curve_name", curve_name);

				int nid = OBJ_sn2nid(ZSTR_VAL(curve_name));
				if (nid != NID_undef) {
					ASN1_OBJECT *obj = OBJ_nid2obj(nid);
					if (obj) {
						// OpenSSL recommends a buffer length of 80.
						char oir_buf[80];
						int oir_len = OBJ_obj2txt(oir_buf, sizeof(oir_buf), obj, 1);
						add_assoc_stringl(&ary, "curve_oid", oir_buf, oir_len);
						ASN1_OBJECT_free(obj);
					}
				}
			}

			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_EC_PUB_X, "x");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_EC_PUB_Y, "y");
			crx_openssl_copy_bn_param(&ary, pkey, OSSL_PKEY_PARAM_PRIV_KEY, "d");
			break;
		}
#endif
		default:
			ktype = -1;
			break;
	}
#else
	switch (EVP_PKEY_base_id(pkey)) {
		case EVP_PKEY_RSA:
		case EVP_PKEY_RSA2:
			{
				RSA *rsa = EVP_PKEY_get0_RSA(pkey);
				ktype = OPENSSL_KEYTYPE_RSA;

				if (rsa != NULL) {
					zval z_rsa;
					const BIGNUM *n, *e, *d, *p, *q, *dmp1, *dmq1, *iqmp;

					RSA_get0_key(rsa, &n, &e, &d);
					RSA_get0_factors(rsa, &p, &q);
					RSA_get0_crt_params(rsa, &dmp1, &dmq1, &iqmp);

					array_init(&z_rsa);
					OPENSSL_PKEY_GET_BN(z_rsa, n);
					OPENSSL_PKEY_GET_BN(z_rsa, e);
					OPENSSL_PKEY_GET_BN(z_rsa, d);
					OPENSSL_PKEY_GET_BN(z_rsa, p);
					OPENSSL_PKEY_GET_BN(z_rsa, q);
					OPENSSL_PKEY_GET_BN(z_rsa, dmp1);
					OPENSSL_PKEY_GET_BN(z_rsa, dmq1);
					OPENSSL_PKEY_GET_BN(z_rsa, iqmp);
					add_assoc_zval(return_value, "rsa", &z_rsa);
				}
			}
			break;
		case EVP_PKEY_DSA:
		case EVP_PKEY_DSA2:
		case EVP_PKEY_DSA3:
		case EVP_PKEY_DSA4:
			{
				DSA *dsa = EVP_PKEY_get0_DSA(pkey);
				ktype = OPENSSL_KEYTYPE_DSA;

				if (dsa != NULL) {
					zval z_dsa;
					const BIGNUM *p, *q, *g, *priv_key, *pub_key;

					DSA_get0_pqg(dsa, &p, &q, &g);
					DSA_get0_key(dsa, &pub_key, &priv_key);

					array_init(&z_dsa);
					OPENSSL_PKEY_GET_BN(z_dsa, p);
					OPENSSL_PKEY_GET_BN(z_dsa, q);
					OPENSSL_PKEY_GET_BN(z_dsa, g);
					OPENSSL_PKEY_GET_BN(z_dsa, priv_key);
					OPENSSL_PKEY_GET_BN(z_dsa, pub_key);
					add_assoc_zval(return_value, "dsa", &z_dsa);
				}
			}
			break;
		case EVP_PKEY_DH:
			{
				DH *dh = EVP_PKEY_get0_DH(pkey);
				ktype = OPENSSL_KEYTYPE_DH;

				if (dh != NULL) {
					zval z_dh;
					const BIGNUM *p, *q, *g, *priv_key, *pub_key;

					DH_get0_pqg(dh, &p, &q, &g);
					DH_get0_key(dh, &pub_key, &priv_key);

					array_init(&z_dh);
					OPENSSL_PKEY_GET_BN(z_dh, p);
					OPENSSL_PKEY_GET_BN(z_dh, g);
					OPENSSL_PKEY_GET_BN(z_dh, priv_key);
					OPENSSL_PKEY_GET_BN(z_dh, pub_key);
					add_assoc_zval(return_value, "dh", &z_dh);
				}
			}
			break;
#ifdef HAVE_EVP_PKEY_EC
		case EVP_PKEY_EC:
			ktype = OPENSSL_KEYTYPE_EC;
			if (EVP_PKEY_get0_EC_KEY(pkey) != NULL) {
				zval ec;
				const EC_GROUP *ec_group;
				const EC_POINT *pub;
				int nid;
				char *crv_sn;
				ASN1_OBJECT *obj;
				// openssl recommends a buffer length of 80
				char oir_buf[80];
				const EC_KEY *ec_key = EVP_PKEY_get0_EC_KEY(pkey);
				BIGNUM *x = BN_new();
				BIGNUM *y = BN_new();
				const BIGNUM *d;

				ec_group = EC_KEY_get0_group(ec_key);

				array_init(&ec);

				/** Curve nid (numerical identifier) used for ASN1 mapping */
				nid = EC_GROUP_get_curve_name(ec_group);
				if (nid != NID_undef) {
					crv_sn = (char*) OBJ_nid2sn(nid);
					if (crv_sn != NULL) {
						add_assoc_string(&ec, "curve_name", crv_sn);
					}

					obj = OBJ_nid2obj(nid);
					if (obj != NULL) {
						int oir_len = OBJ_obj2txt(oir_buf, sizeof(oir_buf), obj, 1);
						add_assoc_stringl(&ec, "curve_oid", (char*) oir_buf, oir_len);
						ASN1_OBJECT_free(obj);
					}
				}

				pub = EC_KEY_get0_public_key(ec_key);

				if (EC_POINT_get_affine_coordinates_GFp(ec_group, pub, x, y, NULL)) {
					crx_openssl_add_bn_to_array(&ec, x, "x");
					crx_openssl_add_bn_to_array(&ec, y, "y");
				} else {
					crx_openssl_store_errors();
				}

				if ((d = EC_KEY_get0_private_key(EVP_PKEY_get0_EC_KEY(pkey))) != NULL) {
					crx_openssl_add_bn_to_array(&ec, d, "d");
				}

				add_assoc_zval(return_value, "ec", &ec);

				BN_free(x);
				BN_free(y);
			}
			break;
#endif
		default:
			ktype = -1;
			break;
	}
#endif
	add_assoc_long(return_value, "type", ktype);

	BIO_free(out);
}
/* }}} */

static crex_string *crx_openssl_pkey_derive(EVP_PKEY *key, EVP_PKEY *peer_key, size_t key_size) {
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(key, NULL);
	if (!ctx) {
		return NULL;
	}

	if (EVP_PKEY_derive_init(ctx) <= 0 ||
			EVP_PKEY_derive_set_peer(ctx, peer_key) <= 0 ||
			(key_size == 0 && EVP_PKEY_derive(ctx, NULL, &key_size) <= 0)) {
		crx_openssl_store_errors();
		EVP_PKEY_CTX_free(ctx);
		return NULL;
	}

	crex_string *result = crex_string_alloc(key_size, 0);
	if (EVP_PKEY_derive(ctx, (unsigned char *)ZSTR_VAL(result), &key_size) <= 0) {
		crx_openssl_store_errors();
		crex_string_release_ex(result, 0);
		EVP_PKEY_CTX_free(ctx);
		return NULL;
	}

	ZSTR_LEN(result) = key_size;
	ZSTR_VAL(result)[key_size] = 0;
	EVP_PKEY_CTX_free(ctx);
	return result;
}

static crex_string *crx_openssl_dh_compute_key(EVP_PKEY *pkey, char *pub_str, size_t pub_len) {
#if CRX_OPENSSL_API_VERSION >= 0x30000
	EVP_PKEY *peer_key = EVP_PKEY_new();
	if (!peer_key || EVP_PKEY_copy_parameters(peer_key, pkey) <= 0 ||
			EVP_PKEY_set1_encoded_public_key(peer_key, (unsigned char *) pub_str, pub_len) <= 0) {
		crx_openssl_store_errors();
		EVP_PKEY_free(peer_key);
		return NULL;
	}

	crex_string *result = crx_openssl_pkey_derive(pkey, peer_key, 0);
	EVP_PKEY_free(peer_key);
	return result;
#else
	DH *dh = EVP_PKEY_get0_DH(pkey);
	if (dh == NULL) {
		return NULL;
	}

	BIGNUM *pub = BN_bin2bn((unsigned char*)pub_str, (int)pub_len, NULL);
	crex_string *data = crex_string_alloc(DH_size(dh), 0);
	int len = DH_compute_key((unsigned char*)ZSTR_VAL(data), pub, dh);
	BN_free(pub);

	if (len < 0) {
		crx_openssl_store_errors();
		crex_string_release_ex(data, 0);
		return NULL;
	}

	ZSTR_LEN(data) = len;
	ZSTR_VAL(data)[len] = 0;
	return data;
#endif
}

/* {{{ Computes shared secret for public value of remote DH key and local DH key */
CRX_FUNCTION(openssl_dh_compute_key)
{
	zval *key;
	char *pub_str;
	size_t pub_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sO", &pub_str, &pub_len, &key, crx_openssl_pkey_ce) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_OPENSSL_CHECK_SIZE_T_TO_INT(pub_len, pub_key, 1);

	EVP_PKEY *pkey = C_OPENSSL_PKEY_P(key)->pkey;
	if (EVP_PKEY_base_id(pkey) != EVP_PKEY_DH) {
		RETURN_FALSE;
	}

	crex_string *result = crx_openssl_dh_compute_key(pkey, pub_str, pub_len);
	if (result) {
		RETURN_NEW_STR(result);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Computes shared secret for public value of remote and local DH or ECDH key */
CRX_FUNCTION(openssl_pkey_derive)
{
	zval *priv_key;
	zval *peer_pub_key;
	crex_long key_len = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "zz|l", &peer_pub_key, &priv_key, &key_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (key_len < 0) {
		crex_argument_value_error(3, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	EVP_PKEY *pkey = crx_openssl_pkey_from_zval(priv_key, 0, "", 0, 2);
	if (!pkey) {
		RETURN_FALSE;
	}

	EVP_PKEY *peer_key = crx_openssl_pkey_from_zval(peer_pub_key, 1, NULL, 0, 1);
	if (!peer_key) {
		EVP_PKEY_free(pkey);
		RETURN_FALSE;
	}

	crex_string *result = crx_openssl_pkey_derive(pkey, peer_key, key_len);
	EVP_PKEY_free(pkey);
	EVP_PKEY_free(peer_key);

	if (result) {
		RETURN_NEW_STR(result);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */


/* {{{ Generates a PKCS5 v2 PBKDF2 string, defaults to sha1 */
CRX_FUNCTION(openssl_pbkdf2)
{
	crex_long key_length = 0, iterations = 0;
	char *password;
	size_t password_len;
	char *salt;
	size_t salt_len;
	char *method;
	size_t method_len = 0;
	crex_string *out_buffer;

	const EVP_MD *digest;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ssll|s",
				&password, &password_len,
				&salt, &salt_len,
				&key_length, &iterations,
				&method, &method_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_OPENSSL_CHECK_SIZE_T_TO_INT(password_len, password, 1);
	CRX_OPENSSL_CHECK_SIZE_T_TO_INT(salt_len, salt, 2);
	CRX_OPENSSL_CHECK_LONG_TO_INT(key_length, key, 3);
	CRX_OPENSSL_CHECK_LONG_TO_INT(iterations, iterations, 4);

	if (key_length <= 0) {
		crex_argument_value_error(3, "must be greater than 0");
		RETURN_THROWS();
	}

	if (method_len) {
		digest = EVP_get_digestbyname(method);
	} else {
		digest = EVP_sha1();
	}

	if (!digest) {
		crx_error_docref(NULL, E_WARNING, "Unknown digest algorithm");
		RETURN_FALSE;
	}

	out_buffer = crex_string_alloc(key_length, 0);

	if (PKCS5_PBKDF2_HMAC(password, (int)password_len, (unsigned char *)salt, (int)salt_len, (int)iterations, digest, (int)key_length, (unsigned char*)ZSTR_VAL(out_buffer)) == 1) {
		ZSTR_VAL(out_buffer)[key_length] = 0;
		RETURN_NEW_STR(out_buffer);
	} else {
		crx_openssl_store_errors();
		crex_string_release_ex(out_buffer, 0);
		RETURN_FALSE;
	}
}
/* }}} */

/** openssl bio new file helper */
static BIO *crx_openssl_bio_new_file(
		const char *filename, size_t filename_len, uint32_t arg_num, const char *mode) {
	char file_path[MAXPATHLEN];
	BIO *bio;

	if (!crx_openssl_check_path(filename, filename_len, file_path, arg_num)) {
		return NULL;
	}

	bio = BIO_new_file(file_path, mode);
	if (bio == NULL) {
		crx_openssl_store_errors();
		return NULL;
	}

	return bio;
}

/* {{{ PKCS7 S/MIME functions */

/* {{{ Verifys that the data block is intact, the signer is who they say they are, and returns the CERTs of the signers */
CRX_FUNCTION(openssl_pkcs7_verify)
{
	X509_STORE * store = NULL;
	zval * cainfo = NULL;
	STACK_OF(X509) *signers= NULL;
	STACK_OF(X509) *others = NULL;
	PKCS7 * p7 = NULL;
	BIO * in = NULL, * datain = NULL, * dataout = NULL, * p7bout  = NULL;
	crex_long flags = 0;
	char * filename;
	size_t filename_len;
	char * extracerts = NULL;
	size_t extracerts_len = 0;
	char * signersfilename = NULL;
	size_t signersfilename_len = 0;
	char * datafilename = NULL;
	size_t datafilename_len = 0;
	char * p7bfilename = NULL;
	size_t p7bfilename_len = 0;

	RETVAL_LONG(-1);

	if (crex_parse_parameters(CREX_NUM_ARGS(), "pl|p!ap!p!p!", &filename, &filename_len,
				&flags, &signersfilename, &signersfilename_len, &cainfo,
				&extracerts, &extracerts_len, &datafilename, &datafilename_len, &p7bfilename, &p7bfilename_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (extracerts) {
		others = crx_openssl_load_all_certs_from_file(extracerts, extracerts_len, 5);
		if (others == NULL) {
			goto clean_exit;
		}
	}

	flags = flags & ~PKCS7_DETACHED;

	store = crx_openssl_setup_verify(cainfo, 4);

	if (!store) {
		goto clean_exit;
	}

	in = crx_openssl_bio_new_file(filename, filename_len, 1, CRX_OPENSSL_BIO_MODE_R(flags));
	if (in == NULL) {
		goto clean_exit;
	}

	p7 = SMIME_read_PKCS7(in, &datain);
	if (p7 == NULL) {
#if DEBUG_SMIME
		crex_printf("SMIME_read_PKCS7 failed\n");
#endif
		crx_openssl_store_errors();
		goto clean_exit;
	}
	if (datafilename) {
		dataout = crx_openssl_bio_new_file(
				datafilename, datafilename_len, 6, CRX_OPENSSL_BIO_MODE_W(PKCS7_BINARY));
		if (dataout == NULL) {
			goto clean_exit;
		}
	}
	if (p7bfilename) {
		p7bout = crx_openssl_bio_new_file(
				p7bfilename, p7bfilename_len, 7, CRX_OPENSSL_BIO_MODE_W(PKCS7_BINARY));
		if (p7bout == NULL) {
			goto clean_exit;
		}
	}
#if DEBUG_SMIME
	crex_printf("Calling PKCS7 verify\n");
#endif

	if (PKCS7_verify(p7, others, store, datain, dataout, (int)flags)) {

		RETVAL_TRUE;

		if (signersfilename) {
			BIO *certout;

			certout = crx_openssl_bio_new_file(
					signersfilename, signersfilename_len, 3, CRX_OPENSSL_BIO_MODE_W(PKCS7_BINARY));
			if (certout) {
				int i;
				signers = PKCS7_get0_signers(p7, others, (int)flags);
				if (signers != NULL) {

					for (i = 0; i < sk_X509_num(signers); i++) {
						if (!PEM_write_bio_X509(certout, sk_X509_value(signers, i))) {
							crx_openssl_store_errors();
							RETVAL_LONG(-1);
							crx_error_docref(NULL, E_WARNING, "Failed to write signer %d", i);
						}
					}

					sk_X509_free(signers);
				} else {
					RETVAL_LONG(-1);
					crx_openssl_store_errors();
				}

				BIO_free(certout);
			} else {
				crx_error_docref(NULL, E_WARNING, "Signature OK, but cannot open %s for writing", signersfilename);
				RETVAL_LONG(-1);
			}

			if (p7bout) {
				if (PEM_write_bio_PKCS7(p7bout, p7) == 0) {
					crx_error_docref(NULL, E_WARNING, "Failed to write PKCS7 to file");
					crx_openssl_store_errors();
					RETVAL_FALSE;
				}
			}
		}
	} else {
		crx_openssl_store_errors();
		RETVAL_FALSE;
	}
clean_exit:
	if (p7bout) {
		BIO_free(p7bout);
	}
	X509_STORE_free(store);
	BIO_free(datain);
	BIO_free(in);
	BIO_free(dataout);
	PKCS7_free(p7);
	sk_X509_pop_free(others, X509_free);
}
/* }}} */

/* {{{ Encrypts the message in the file named infile with the certificates in recipcerts and output the result to the file named outfile */
CRX_FUNCTION(openssl_pkcs7_encrypt)
{
	zval * zrecipcerts, * zheaders = NULL;
	STACK_OF(X509) * recipcerts = NULL;
	BIO * infile = NULL, * outfile = NULL;
	crex_long flags = 0;
	PKCS7 * p7 = NULL;
	zval * zcertval;
	X509 * cert;
	const EVP_CIPHER *cipher = NULL;
	crex_long cipherid = CRX_OPENSSL_CIPHER_DEFAULT;
	crex_string * strindex;
	char * infilename = NULL;
	size_t infilename_len;
	char * outfilename = NULL;
	size_t outfilename_len;

	RETVAL_FALSE;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ppza!|ll", &infilename, &infilename_len,
				&outfilename, &outfilename_len, &zrecipcerts, &zheaders, &flags, &cipherid) == FAILURE) {
		RETURN_THROWS();
	}

	infile = crx_openssl_bio_new_file(infilename, infilename_len, 1, CRX_OPENSSL_BIO_MODE_R(flags));
	if (infile == NULL) {
		goto clean_exit;
	}

	outfile = crx_openssl_bio_new_file(outfilename, outfilename_len, 2, CRX_OPENSSL_BIO_MODE_W(flags));
	if (outfile == NULL) {
		goto clean_exit;
	}

	recipcerts = sk_X509_new_null();

	/* get certs */
	if (C_TYPE_P(zrecipcerts) == IS_ARRAY) {
		CREX_HASH_FOREACH_VAL(C_ARRVAL_P(zrecipcerts), zcertval) {
			bool free_cert;

			cert = crx_openssl_x509_from_zval(zcertval, &free_cert, 3, true, NULL);
			if (cert == NULL) {
				// TODO Add warning?
				goto clean_exit;
			}

			if (!free_cert) {
				/* we shouldn't free this particular cert, as it is a resource.
					make a copy and push that on the stack instead */
				cert = X509_dup(cert);
				if (cert == NULL) {
					crx_openssl_store_errors();
					goto clean_exit;
				}
			}
			sk_X509_push(recipcerts, cert);
		} CREX_HASH_FOREACH_END();
	} else {
		/* a single certificate */
		bool free_cert;

		cert = crx_openssl_x509_from_zval(zrecipcerts, &free_cert, 3, false, NULL);
		if (cert == NULL) {
			// TODO Add warning?
			goto clean_exit;
		}

		if (!free_cert) {
			/* we shouldn't free this particular cert, as it is a resource.
				make a copy and push that on the stack instead */
			cert = X509_dup(cert);
			if (cert == NULL) {
				crx_openssl_store_errors();
				goto clean_exit;
			}
		}
		sk_X509_push(recipcerts, cert);
	}

	/* sanity check the cipher */
	cipher = crx_openssl_get_evp_cipher_from_algo(cipherid);
	if (cipher == NULL) {
		/* shouldn't happen */
		crx_error_docref(NULL, E_WARNING, "Failed to get cipher");
		goto clean_exit;
	}

	p7 = PKCS7_encrypt(recipcerts, infile, (EVP_CIPHER*)cipher, (int)flags);

	if (p7 == NULL) {
		crx_openssl_store_errors();
		goto clean_exit;
	}

	/* tack on extra headers */
	if (zheaders) {
		CREX_HASH_FOREACH_STR_KEY_VAL(C_ARRVAL_P(zheaders), strindex, zcertval) {
			crex_string *str = zval_try_get_string(zcertval);
			if (UNEXPECTED(!str)) {
				goto clean_exit;
			}
			if (strindex) {
				BIO_printf(outfile, "%s: %s\n", ZSTR_VAL(strindex), ZSTR_VAL(str));
			} else {
				BIO_printf(outfile, "%s\n", ZSTR_VAL(str));
			}
			crex_string_release(str);
		} CREX_HASH_FOREACH_END();
	}

	(void)BIO_reset(infile);

	/* write the encrypted data */
	if (!SMIME_write_PKCS7(outfile, p7, infile, (int)flags)) {
		crx_openssl_store_errors();
		goto clean_exit;
	}

	RETVAL_TRUE;

clean_exit:
	PKCS7_free(p7);
	BIO_free(infile);
	BIO_free(outfile);
	if (recipcerts) {
		sk_X509_pop_free(recipcerts, X509_free);
	}
}
/* }}} */

/* {{{ Exports the PKCS7 file to an array of PEM certificates */
CRX_FUNCTION(openssl_pkcs7_read)
{
	zval * zout = NULL, zcert;
	char *p7b;
	size_t p7b_len;
	STACK_OF(X509) *certs = NULL;
	STACK_OF(X509_CRL) *crls = NULL;
	BIO * bio_in = NULL, * bio_out = NULL;
	PKCS7 * p7 = NULL;
	int i;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sz", &p7b, &p7b_len,
				&zout) == FAILURE) {
		RETURN_THROWS();
	}

	RETVAL_FALSE;

	CRX_OPENSSL_CHECK_SIZE_T_TO_INT(p7b_len, p7b, 1);

	bio_in = BIO_new(BIO_s_mem());
	if (bio_in == NULL) {
		goto clean_exit;
	}

	if (0 >= BIO_write(bio_in, p7b, (int)p7b_len)) {
		crx_openssl_store_errors();
		goto clean_exit;
	}

	p7 = PEM_read_bio_PKCS7(bio_in, NULL, NULL, NULL);
	if (p7 == NULL) {
		crx_openssl_store_errors();
		goto clean_exit;
	}

	switch (OBJ_obj2nid(p7->type)) {
		case NID_pkcs7_signed:
			if (p7->d.sign != NULL) {
				certs = p7->d.sign->cert;
				crls = p7->d.sign->crl;
			}
			break;
		case NID_pkcs7_signedAndEnveloped:
			if (p7->d.signed_and_enveloped != NULL) {
				certs = p7->d.signed_and_enveloped->cert;
				crls = p7->d.signed_and_enveloped->crl;
			}
			break;
		default:
			break;
	}

	zout = crex_try_array_init(zout);
	if (!zout) {
		goto clean_exit;
	}

	if (certs != NULL) {
		for (i = 0; i < sk_X509_num(certs); i++) {
			X509* ca = sk_X509_value(certs, i);

			bio_out = BIO_new(BIO_s_mem());
			if (bio_out && PEM_write_bio_X509(bio_out, ca)) {
				BUF_MEM *bio_buf;
				BIO_get_mem_ptr(bio_out, &bio_buf);
				ZVAL_STRINGL(&zcert, bio_buf->data, bio_buf->length);
				add_index_zval(zout, i, &zcert);
				BIO_free(bio_out);
			}
		}
	}

	if (crls != NULL) {
		for (i = 0; i < sk_X509_CRL_num(crls); i++) {
			X509_CRL* crl = sk_X509_CRL_value(crls, i);

			bio_out = BIO_new(BIO_s_mem());
			if (bio_out && PEM_write_bio_X509_CRL(bio_out, crl)) {
				BUF_MEM *bio_buf;
				BIO_get_mem_ptr(bio_out, &bio_buf);
				ZVAL_STRINGL(&zcert, bio_buf->data, bio_buf->length);
				add_index_zval(zout, i, &zcert);
				BIO_free(bio_out);
			}
		}
	}

	RETVAL_TRUE;

clean_exit:
	BIO_free(bio_in);

	if (p7 != NULL) {
		PKCS7_free(p7);
	}
}
/* }}} */

/* {{{ Signs the MIME message in the file named infile with signcert/signkey and output the result to file name outfile. headers lists plain text headers to exclude from the signed portion of the message, and should include to, from and subject as a minimum */

CRX_FUNCTION(openssl_pkcs7_sign)
{
	X509 *cert = NULL;
	crex_object *cert_obj;
	crex_string *cert_str;
	zval *zprivkey, * zheaders;
	zval * hval;
	EVP_PKEY * privkey = NULL;
	crex_long flags = PKCS7_DETACHED;
	PKCS7 * p7 = NULL;
	BIO * infile = NULL, * outfile = NULL;
	STACK_OF(X509) *others = NULL;
	crex_string * strindex;
	char * infilename;
	size_t infilename_len;
	char * outfilename;
	size_t outfilename_len;
	char * extracertsfilename = NULL;
	size_t extracertsfilename_len;

	CREX_PARSE_PARAMETERS_START(5, 7)
		C_PARAM_PATH(infilename, infilename_len)
		C_PARAM_PATH(outfilename, outfilename_len)
		C_PARAM_OBJ_OF_CLASS_OR_STR(cert_obj, crx_openssl_certificate_ce, cert_str)
		C_PARAM_ZVAL(zprivkey)
		C_PARAM_ARRAY_OR_NULL(zheaders)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(flags)
		C_PARAM_PATH_OR_NULL(extracertsfilename, extracertsfilename_len)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_FALSE;

	if (extracertsfilename) {
		others = crx_openssl_load_all_certs_from_file(
				extracertsfilename, extracertsfilename_len, 7);
		if (others == NULL) {
			goto clean_exit;
		}
	}

	privkey = crx_openssl_pkey_from_zval(zprivkey, 0, "", 0, 4);
	if (privkey == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Error getting private key");
		}
		goto clean_exit;
	}

	cert = crx_openssl_x509_from_param(cert_obj, cert_str, 3);
	if (cert == NULL) {
		crx_error_docref(NULL, E_WARNING, "X.509 Certificate cannot be retrieved");
		goto clean_exit;
	}

	infile = crx_openssl_bio_new_file(infilename, infilename_len, 1, CRX_OPENSSL_BIO_MODE_R(flags));
	if (infile == NULL) {
		crx_error_docref(NULL, E_WARNING, "Error opening input file %s!", infilename);
		goto clean_exit;
	}

	outfile = crx_openssl_bio_new_file(outfilename, outfilename_len, 2, CRX_OPENSSL_BIO_MODE_W(PKCS7_BINARY));
	if (outfile == NULL) {
		crx_error_docref(NULL, E_WARNING, "Error opening output file %s!", outfilename);
		goto clean_exit;
	}

	p7 = PKCS7_sign(cert, privkey, others, infile, (int)flags);
	if (p7 == NULL) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Error creating PKCS7 structure!");
		goto clean_exit;
	}

	(void)BIO_reset(infile);

	/* tack on extra headers */
	if (zheaders) {
		int ret;

		CREX_HASH_FOREACH_STR_KEY_VAL(C_ARRVAL_P(zheaders), strindex, hval) {
			crex_string *str = zval_try_get_string(hval);
			if (UNEXPECTED(!str)) {
				goto clean_exit;
			}
			if (strindex) {
				ret = BIO_printf(outfile, "%s: %s\n", ZSTR_VAL(strindex), ZSTR_VAL(str));
			} else {
				ret = BIO_printf(outfile, "%s\n", ZSTR_VAL(str));
			}
			crex_string_release(str);
			if (ret < 0) {
				crx_openssl_store_errors();
			}
		} CREX_HASH_FOREACH_END();
	}
	/* write the signed data */
	if (!SMIME_write_PKCS7(outfile, p7, infile, (int)flags)) {
		crx_openssl_store_errors();
		goto clean_exit;
	}

	RETVAL_TRUE;

clean_exit:
	PKCS7_free(p7);
	BIO_free(infile);
	BIO_free(outfile);
	if (others) {
		sk_X509_pop_free(others, X509_free);
	}
	EVP_PKEY_free(privkey);
	if (cert && cert_str) {
		X509_free(cert);
	}
}
/* }}} */

/* {{{ Decrypts the S/MIME message in the file name infilename and output the results to the file name outfilename.  recipcert is a CERT for one of the recipients. recipkey specifies the private key matching recipcert, if recipcert does not include the key */

CRX_FUNCTION(openssl_pkcs7_decrypt)
{
	X509 *cert;
	zval *recipcert, *recipkey = NULL;
	bool free_recipcert;
	EVP_PKEY * key = NULL;
	BIO * in = NULL, *out = NULL, *datain = NULL;
	PKCS7 * p7 = NULL;
	char * infilename;
	size_t infilename_len;
	char * outfilename;
	size_t outfilename_len;

	CREX_PARSE_PARAMETERS_START(3, 4)
		C_PARAM_PATH(infilename, infilename_len)
		C_PARAM_PATH(outfilename, outfilename_len)
		C_PARAM_ZVAL(recipcert)
		C_PARAM_OPTIONAL
		C_PARAM_ZVAL_OR_NULL(recipkey)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_FALSE;

	cert = crx_openssl_x509_from_zval(recipcert, &free_recipcert, 3, false, NULL);
	if (cert == NULL) {
		crx_error_docref(NULL, E_WARNING, "X.509 Certificate cannot be retrieved");
		goto clean_exit;
	}

	key = crx_openssl_pkey_from_zval(recipkey ? recipkey : recipcert, 0, "", 0, 4);
	if (key == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Unable to get private key");
		}
		goto clean_exit;
	}

	in = crx_openssl_bio_new_file(
			infilename, infilename_len, 1, CRX_OPENSSL_BIO_MODE_R(PKCS7_BINARY));
	if (in == NULL) {
		goto clean_exit;
	}

	out = crx_openssl_bio_new_file(
			outfilename, outfilename_len, 2, CRX_OPENSSL_BIO_MODE_W(PKCS7_BINARY));
	if (out == NULL) {
		goto clean_exit;
	}

	p7 = SMIME_read_PKCS7(in, &datain);

	if (p7 == NULL) {
		crx_openssl_store_errors();
		goto clean_exit;
	}
	if (PKCS7_decrypt(p7, key, cert, out, PKCS7_DETACHED)) {
		RETVAL_TRUE;
	} else {
		crx_openssl_store_errors();
	}
clean_exit:
	PKCS7_free(p7);
	BIO_free(datain);
	BIO_free(in);
	BIO_free(out);
	if (cert && free_recipcert) {
		X509_free(cert);
	}
	EVP_PKEY_free(key);
}
/* }}} */

/* }}} */

/* {{{ CMS S/MIME functions taken from PKCS#7 functions */

/* {{{ Verifies that the data block is intact, the signer is who they say they are, and returns the CERTs of the signers */
CRX_FUNCTION(openssl_cms_verify)
{
	X509_STORE * store = NULL;
	zval * cainfo = NULL;
	STACK_OF(X509) *signers= NULL;
	STACK_OF(X509) *others = NULL;
	CMS_ContentInfo * cms = NULL;
	BIO * in = NULL, * datain = NULL, * dataout = NULL, * p7bout  = NULL;
	BIO *certout = NULL, *sigbio = NULL;
	crex_long flags = 0;
	char * filename;
	size_t filename_len;
	char * extracerts = NULL;
	size_t extracerts_len = 0;
	char * signersfilename = NULL;
	size_t signersfilename_len = 0;
	char * datafilename = NULL;
	size_t datafilename_len = 0;
	char * p7bfilename = NULL;
	size_t p7bfilename_len = 0;
	char * sigfile = NULL;
	size_t sigfile_len = 0;
	crex_long encoding = ENCODING_SMIME;

	RETVAL_FALSE;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "pl|p!ap!p!p!p!l", &filename, &filename_len,
				&flags, &signersfilename, &signersfilename_len, &cainfo,
				&extracerts, &extracerts_len, &datafilename, &datafilename_len,
				&p7bfilename, &p7bfilename_len,
				&sigfile, &sigfile_len, &encoding) == FAILURE) {
		RETURN_THROWS();
	}

	in = crx_openssl_bio_new_file(filename, filename_len, 1, CRX_OPENSSL_BIO_MODE_R(flags));
	if (in == NULL) {
		goto clean_exit;
	}
	if (sigfile && (flags & CMS_DETACHED)) {
		if (encoding == ENCODING_SMIME)  {
			crx_error_docref(NULL, E_WARNING,
					 "Detached signatures not possible with S/MIME encoding");
			goto clean_exit;
		}
		sigbio = crx_openssl_bio_new_file(sigfile, sigfile_len, 1, CRX_OPENSSL_BIO_MODE_R(flags));
		if (sigbio == NULL) {
			goto clean_exit;
		}
	} else  {
		sigbio = in;	/* non-detached signature */
	}

	switch (encoding) {
		case ENCODING_PEM:
			cms = PEM_read_bio_CMS(sigbio, NULL, 0, NULL);
				datain = in;
				break;
			case ENCODING_DER:
				cms = d2i_CMS_bio(sigbio, NULL);
				datain = in;
				break;
			case ENCODING_SMIME:
				cms = SMIME_read_CMS(sigbio, &datain);
				break;
			default:
				crx_error_docref(NULL, E_WARNING, "Unknown encoding");
				goto clean_exit;
	}
	if (cms == NULL) {
		crx_openssl_store_errors();
		goto clean_exit;
	}
	if (encoding != ENCODING_SMIME && !(flags & CMS_DETACHED)) {
		datain = NULL; /* when not detached, don't pass a real BIO */
	}

	if (extracerts) {
		others = crx_openssl_load_all_certs_from_file(extracerts, extracerts_len, 5);
		if (others == NULL) {
			goto clean_exit;
		}
	}

	store = crx_openssl_setup_verify(cainfo, 4);

	if (!store) {
		goto clean_exit;
	}

	if (datafilename) {
		dataout = crx_openssl_bio_new_file(
				datafilename, datafilename_len, 6, CRX_OPENSSL_BIO_MODE_W(CMS_BINARY));
		if (dataout == NULL) {
			goto clean_exit;
		}
	}

	if (p7bfilename) {
		p7bout = crx_openssl_bio_new_file(
				p7bfilename, p7bfilename_len, 7, CRX_OPENSSL_BIO_MODE_W(CMS_BINARY));
		if (p7bout == NULL) {
			goto clean_exit;
		}
	}
#if DEBUG_SMIME
	crex_printf("Calling CMS verify\n");
#endif
	if (CMS_verify(cms, others, store, datain, dataout, (unsigned int)flags)) {
		RETVAL_TRUE;

		if (signersfilename) {
			certout = crx_openssl_bio_new_file(
					signersfilename, signersfilename_len, 3, CRX_OPENSSL_BIO_MODE_W(CMS_BINARY));
			if (certout) {
				int i;
				signers = CMS_get0_signers(cms);
				if (signers != NULL) {

					for (i = 0; i < sk_X509_num(signers); i++) {
						if (!PEM_write_bio_X509(certout, sk_X509_value(signers, i))) {
							crx_openssl_store_errors();
							RETVAL_FALSE;
							crx_error_docref(NULL, E_WARNING, "Failed to write signer %d", i);
						}
					}

					sk_X509_free(signers);
				} else {
					RETVAL_FALSE;
					crx_openssl_store_errors();
				}
			} else {
				crx_error_docref(NULL, E_WARNING, "Signature OK, but cannot open %s for writing", signersfilename);
				RETVAL_FALSE;
			}

			if (p7bout) {
				if (PEM_write_bio_CMS(p7bout, cms) == 0) {
					crx_error_docref(NULL, E_WARNING, "Failed to write CMS to file");
					crx_openssl_store_errors();
					RETVAL_FALSE;
				}
			}
		}
	} else {
		crx_openssl_store_errors();
		RETVAL_FALSE;
	}
clean_exit:
	BIO_free(p7bout);
	if (store) {
		X509_STORE_free(store);
	}
	if (datain != in) {
		BIO_free(datain);
	}
	if (sigbio != in) {
		BIO_free(sigbio);
	}
	BIO_free(in);
	BIO_free(dataout);
	BIO_free(certout);
	if (cms) {
		CMS_ContentInfo_free(cms);
	}
	if (others) {
		sk_X509_pop_free(others, X509_free);
	}
}
/* }}} */

/* {{{ Encrypts the message in the file named infile with the certificates in recipcerts and output the result to the file named outfile */
CRX_FUNCTION(openssl_cms_encrypt)
{
	zval * zrecipcerts, * zheaders = NULL;
	STACK_OF(X509) * recipcerts = NULL;
	BIO * infile = NULL, * outfile = NULL;
	crex_long flags = 0;
	crex_long encoding = ENCODING_SMIME;
	CMS_ContentInfo * cms = NULL;
	zval * zcertval;
	X509 * cert;
	const EVP_CIPHER *cipher = NULL;
	crex_long cipherid = CRX_OPENSSL_CIPHER_DEFAULT;
	crex_string * strindex;
	char * infilename = NULL;
	size_t infilename_len;
	char * outfilename = NULL;
	size_t outfilename_len;
	int need_final = 0;

	RETVAL_FALSE;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ppza!|lll", &infilename, &infilename_len,
				  &outfilename, &outfilename_len, &zrecipcerts, &zheaders, &flags, &encoding, &cipherid) == FAILURE) {
		RETURN_THROWS();
	}


	infile = crx_openssl_bio_new_file(
			infilename, infilename_len, 1, CRX_OPENSSL_BIO_MODE_R(flags));
	if (infile == NULL) {
		goto clean_exit;
	}

	outfile = crx_openssl_bio_new_file(
			outfilename, outfilename_len, 2, CRX_OPENSSL_BIO_MODE_W(flags));
	if (outfile == NULL) {
		goto clean_exit;
	}

	recipcerts = sk_X509_new_null();

	/* get certs */
	if (C_TYPE_P(zrecipcerts) == IS_ARRAY) {
		CREX_HASH_FOREACH_VAL(C_ARRVAL_P(zrecipcerts), zcertval) {
			bool free_cert;

			cert = crx_openssl_x509_from_zval(zcertval, &free_cert, 3, true, NULL);
			if (cert == NULL) {
				goto clean_exit;
			}

			if (!free_cert) {
				/* we shouldn't free this particular cert, as it is a resource.
					make a copy and push that on the stack instead */
				cert = X509_dup(cert);
				if (cert == NULL) {
					crx_openssl_store_errors();
					goto clean_exit;
				}
			}
			sk_X509_push(recipcerts, cert);
		} CREX_HASH_FOREACH_END();
	} else {
		/* a single certificate */
		bool free_cert;

		cert = crx_openssl_x509_from_zval(zrecipcerts, &free_cert, 3, false, NULL);
		if (cert == NULL) {
			goto clean_exit;
		}

		if (!free_cert) {
			/* we shouldn't free this particular cert, as it is a resource.
				make a copy and push that on the stack instead */
			cert = X509_dup(cert);
			if (cert == NULL) {
				crx_openssl_store_errors();
				goto clean_exit;
			}
		}
		sk_X509_push(recipcerts, cert);
	}

	/* sanity check the cipher */
	cipher = crx_openssl_get_evp_cipher_from_algo(cipherid);
	if (cipher == NULL) {
		/* shouldn't happen */
		crx_error_docref(NULL, E_WARNING, "Failed to get cipher");
		goto clean_exit;
	}

	cms = CMS_encrypt(recipcerts, infile, (EVP_CIPHER*)cipher, (unsigned int)flags);

	if (cms == NULL) {
		crx_openssl_store_errors();
		goto clean_exit;
	}

	if (flags & CMS_PARTIAL && !(flags & CMS_STREAM)) {
		need_final=1;
	}

	/* tack on extra headers */
	if (zheaders && encoding == ENCODING_SMIME) {
		CREX_HASH_FOREACH_STR_KEY_VAL(C_ARRVAL_P(zheaders), strindex, zcertval) {
			crex_string *str = zval_try_get_string(zcertval);
			if (UNEXPECTED(!str)) {
				goto clean_exit;
			}
			if (strindex) {
				BIO_printf(outfile, "%s: %s\n", ZSTR_VAL(strindex), ZSTR_VAL(str));
			} else {
				BIO_printf(outfile, "%s\n", ZSTR_VAL(str));
			}
			crex_string_release(str);
		} CREX_HASH_FOREACH_END();
	}

	(void)BIO_reset(infile);

	switch (encoding) {
		case ENCODING_SMIME:
			if (!SMIME_write_CMS(outfile, cms, infile, (int)flags)) {
				crx_openssl_store_errors();
					goto clean_exit;
			}
			break;
		case ENCODING_DER:
			if (need_final) {
				if (CMS_final(cms, infile, NULL, (unsigned int) flags) != 1) {
					crx_openssl_store_errors();
					goto clean_exit;
				}
			}
			if (i2d_CMS_bio(outfile, cms) != 1) {
				crx_openssl_store_errors();
				goto clean_exit;
			}
			break;
		case ENCODING_PEM:
			if (need_final) {
				if (CMS_final(cms, infile, NULL, (unsigned int) flags) != 1) {
					crx_openssl_store_errors();
					goto clean_exit;
				}
			}
			if (flags & CMS_STREAM) {
				if (PEM_write_bio_CMS_stream(outfile, cms, infile, flags) == 0) {
					crx_openssl_store_errors();
					goto clean_exit;
				}
			} else {
				if (PEM_write_bio_CMS(outfile, cms) == 0) {
					crx_openssl_store_errors();
					goto clean_exit;
				}
			}
			break;
		default:
			crx_error_docref(NULL, E_WARNING, "Unknown OPENSSL encoding");
			goto clean_exit;
	}

	RETVAL_TRUE;

clean_exit:
	if (cms) {
		CMS_ContentInfo_free(cms);
	}
	BIO_free(infile);
	BIO_free(outfile);
	if (recipcerts) {
		sk_X509_pop_free(recipcerts, X509_free);
	}
}
/* }}} */

/* {{{ Exports the CMS file to an array of PEM certificates */
CRX_FUNCTION(openssl_cms_read)
{
	zval * zout = NULL, zcert;
	char *p7b;
	size_t p7b_len;
	STACK_OF(X509) *certs = NULL;
	STACK_OF(X509_CRL) *crls = NULL;
	BIO * bio_in = NULL, * bio_out = NULL;
	CMS_ContentInfo * cms = NULL;
	int i;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sz", &p7b, &p7b_len,
				&zout) == FAILURE) {
		RETURN_THROWS();
	}

	RETVAL_FALSE;

	CRX_OPENSSL_CHECK_SIZE_T_TO_INT(p7b_len, p7b, 1);

	bio_in = BIO_new(BIO_s_mem());
	if (bio_in == NULL) {
		goto clean_exit;
	}

	if (0 >= BIO_write(bio_in, p7b, (int)p7b_len)) {
		crx_openssl_store_errors();
		goto clean_exit;
	}

	cms = PEM_read_bio_CMS(bio_in, NULL, NULL, NULL);
	if (cms == NULL) {
		crx_openssl_store_errors();
		goto clean_exit;
	}

	switch (OBJ_obj2nid(CMS_get0_type(cms))) {
		case NID_pkcs7_signed:
		case NID_pkcs7_signedAndEnveloped:
			certs = CMS_get1_certs(cms);
			crls = CMS_get1_crls(cms);
			break;
		default:
			break;
	}

	zout = crex_try_array_init(zout);
	if (!zout) {
		goto clean_exit;
	}

	if (certs != NULL) {
		for (i = 0; i < sk_X509_num(certs); i++) {
			X509* ca = sk_X509_value(certs, i);

			bio_out = BIO_new(BIO_s_mem());
			if (bio_out && PEM_write_bio_X509(bio_out, ca)) {
				BUF_MEM *bio_buf;
				BIO_get_mem_ptr(bio_out, &bio_buf);
				ZVAL_STRINGL(&zcert, bio_buf->data, bio_buf->length);
				add_index_zval(zout, i, &zcert);
				BIO_free(bio_out);
			}
		}
	}

	if (crls != NULL) {
		for (i = 0; i < sk_X509_CRL_num(crls); i++) {
			X509_CRL* crl = sk_X509_CRL_value(crls, i);

			bio_out = BIO_new(BIO_s_mem());
			if (bio_out && PEM_write_bio_X509_CRL(bio_out, crl)) {
				BUF_MEM *bio_buf;
				BIO_get_mem_ptr(bio_out, &bio_buf);
				ZVAL_STRINGL(&zcert, bio_buf->data, bio_buf->length);
				add_index_zval(zout, i, &zcert);
				BIO_free(bio_out);
			}
		}
	}

	RETVAL_TRUE;

clean_exit:
	BIO_free(bio_in);
	if (cms != NULL) {
		CMS_ContentInfo_free(cms);
	}
	if (certs != NULL) {
		sk_X509_pop_free(certs, X509_free);
	}
	if (crls != NULL) {
		sk_X509_CRL_pop_free(crls, X509_CRL_free);
	}
}
/* }}} */

/* {{{ Signs the MIME message in the file named infile with signcert/signkey and output the result to file name outfile. headers lists plain text headers to exclude from the signed portion of the message, and should include to, from and subject as a minimum */

CRX_FUNCTION(openssl_cms_sign)
{
	X509 *cert = NULL;
	crex_object *cert_obj;
	crex_string *cert_str;
	zval *zprivkey, *zheaders;
	zval * hval;
	EVP_PKEY * privkey = NULL;
	crex_long flags = 0;
	crex_long encoding = ENCODING_SMIME;
	CMS_ContentInfo * cms = NULL;
	BIO * infile = NULL, * outfile = NULL;
	STACK_OF(X509) *others = NULL;
	crex_string * strindex;
	char * infilename;
	size_t infilename_len;
	char * outfilename;
	size_t outfilename_len;
	char * extracertsfilename = NULL;
	size_t extracertsfilename_len;
	int need_final = 0;

	CREX_PARSE_PARAMETERS_START(5, 8)
		C_PARAM_PATH(infilename, infilename_len)
		C_PARAM_PATH(outfilename, outfilename_len)
		C_PARAM_OBJ_OF_CLASS_OR_STR(cert_obj, crx_openssl_certificate_ce, cert_str)
		C_PARAM_ZVAL(zprivkey)
		C_PARAM_ARRAY_OR_NULL(zheaders)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(flags)
		C_PARAM_LONG(encoding)
		C_PARAM_PATH_OR_NULL(extracertsfilename, extracertsfilename_len)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_FALSE;

	if (extracertsfilename) {
		others = crx_openssl_load_all_certs_from_file(
				extracertsfilename, extracertsfilename_len, 8);
		if (others == NULL) {
			goto clean_exit;
		}
	}

	privkey = crx_openssl_pkey_from_zval(zprivkey, 0, "", 0, 4);
	if (privkey == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Error getting private key");
		}
		goto clean_exit;
	}

	cert = crx_openssl_x509_from_param(cert_obj, cert_str, 3);
	if (cert == NULL) {
		crx_error_docref(NULL, E_WARNING, "X.509 Certificate cannot be retrieved");
		goto clean_exit;
	}

	if ((encoding & ENCODING_SMIME) && (flags & CMS_DETACHED)) {
		crx_error_docref(NULL,
				 E_WARNING, "Detached signatures not possible with S/MIME encoding");
		goto clean_exit;
	}

	/* a CMS struct will not be complete if either CMS_PARTIAL or CMS_STREAM is set.
	 * However, CMS_PARTIAL requires a CMS_final call whereas CMS_STREAM requires
	 * a different write routine below.  There may be a more efficient way to do this
	 * with function pointers, but the readability goes down.
	 * References: CMS_sign(3SSL), CMS_final(3SSL)
	 */

	if (flags & CMS_PARTIAL && !(flags & CMS_STREAM)) {
		need_final=1;
	}

	infile = crx_openssl_bio_new_file(
			infilename, infilename_len, 1, CRX_OPENSSL_BIO_MODE_R(flags));
	if (infile == NULL) {
		crx_error_docref(NULL, E_WARNING, "Error opening input file %s!", infilename);
		goto clean_exit;
	}

	outfile = crx_openssl_bio_new_file(
			outfilename, outfilename_len, 2, CRX_OPENSSL_BIO_MODE_W(CMS_BINARY));
	if (outfile == NULL) {
		crx_error_docref(NULL, E_WARNING, "Error opening output file %s!", outfilename);
		goto clean_exit;
	}

	cms = CMS_sign(cert, privkey, others, infile, (unsigned int)flags);
	if (cms == NULL) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Error creating CMS structure!");
		goto clean_exit;
	}
	if (BIO_reset(infile) != 0) {
		crx_openssl_store_errors();
		goto clean_exit;
	}

	/* tack on extra headers */
	if (zheaders && encoding == ENCODING_SMIME) {
		int ret;

		CREX_HASH_FOREACH_STR_KEY_VAL(C_ARRVAL_P(zheaders), strindex, hval) {
			crex_string *str = zval_try_get_string(hval);
			if (UNEXPECTED(!str)) {
				goto clean_exit;
			}
			if (strindex) {
				ret = BIO_printf(outfile, "%s: %s\n", ZSTR_VAL(strindex), ZSTR_VAL(str));
			} else {
				ret = BIO_printf(outfile, "%s\n", ZSTR_VAL(str));
			}
			crex_string_release(str);
			if (ret < 0) {
				crx_openssl_store_errors();
			}
		} CREX_HASH_FOREACH_END();
	}
	/* writing the signed data depends on the encoding */
	switch (encoding) {
		case ENCODING_SMIME:
			if (!SMIME_write_CMS(outfile, cms, infile, (int)flags)) {
				crx_openssl_store_errors();
					goto clean_exit;
			}
			break;
		case ENCODING_DER:
			if (need_final) {
				if (CMS_final(cms, infile, NULL, (unsigned int) flags) != 1) {
					crx_openssl_store_errors();
					goto clean_exit;
				}
			}
			if (i2d_CMS_bio(outfile, cms) != 1) {
				crx_openssl_store_errors();
				goto clean_exit;
			}
			break;
		case ENCODING_PEM:
			if (need_final) {
				if (CMS_final(cms, infile, NULL, (unsigned int) flags) != 1) {
					crx_openssl_store_errors();
					goto clean_exit;
				}
			}
			if (flags & CMS_STREAM) {
				if (PEM_write_bio_CMS_stream(outfile, cms, infile, flags) == 0) {
					crx_openssl_store_errors();
					goto clean_exit;
				}
			} else {
				if (PEM_write_bio_CMS(outfile, cms) == 0) {
					crx_openssl_store_errors();
					goto clean_exit;
				}
			}
			break;
		default:
			crx_error_docref(NULL, E_WARNING, "Unknown OPENSSL encoding");
			goto clean_exit;
	}
	RETVAL_TRUE;

clean_exit:
	if (cms) {
		CMS_ContentInfo_free(cms);
	}
	BIO_free(infile);
	BIO_free(outfile);
	if (others) {
		sk_X509_pop_free(others, X509_free);
	}
	EVP_PKEY_free(privkey);
	if (cert && cert_str) {
		X509_free(cert);
	}
}
/* }}} */

/* {{{ Decrypts the S/MIME message in the file name infilename and output the results to the file name outfilename.  recipcert is a CERT for one of the recipients. recipkey specifies the private key matching recipcert, if recipcert does not include the key */

CRX_FUNCTION(openssl_cms_decrypt)
{
	X509 *cert;
	zval *recipcert, *recipkey = NULL;
	bool free_recipcert;
	EVP_PKEY * key = NULL;
	crex_long encoding = ENCODING_SMIME;
	BIO * in = NULL, * out = NULL, * datain = NULL;
	CMS_ContentInfo * cms = NULL;
	char * infilename;
	size_t infilename_len;
	char * outfilename;
	size_t outfilename_len;

	CREX_PARSE_PARAMETERS_START(3, 5)
		C_PARAM_PATH(infilename, infilename_len)
		C_PARAM_PATH(outfilename, outfilename_len)
		C_PARAM_ZVAL(recipcert)
		C_PARAM_OPTIONAL
		C_PARAM_ZVAL_OR_NULL(recipkey)
		C_PARAM_LONG(encoding)
	CREX_PARSE_PARAMETERS_END();

	RETVAL_FALSE;

	cert = crx_openssl_x509_from_zval(recipcert, &free_recipcert, 3, false, NULL);
	if (cert == NULL) {
		crx_error_docref(NULL, E_WARNING, "X.509 Certificate cannot be retrieved");
		goto clean_exit;
	}

	key = crx_openssl_pkey_from_zval(recipkey ? recipkey : recipcert, 0, "", 0, recipkey ? 4 : 3);
	if (key == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Unable to get private key");
		}
		goto clean_exit;
	}

	in = crx_openssl_bio_new_file(
			infilename, infilename_len, 1, CRX_OPENSSL_BIO_MODE_R(CMS_BINARY));
	if (in == NULL) {
		goto clean_exit;
	}

	out = crx_openssl_bio_new_file(
			outfilename, outfilename_len, 2, CRX_OPENSSL_BIO_MODE_W(CMS_BINARY));
	if (out == NULL) {
		goto clean_exit;
	}

	switch (encoding) {
		case ENCODING_DER:
			cms = d2i_CMS_bio(in, NULL);
			break;
		case ENCODING_PEM:
                        cms = PEM_read_bio_CMS(in, NULL, 0, NULL);
			break;
		case ENCODING_SMIME:
			cms = SMIME_read_CMS(in, &datain);
			break;
		default:
			crex_argument_value_error(5, "must be an OPENSSL_ENCODING_* constant");
			goto clean_exit;
	}

	if (cms == NULL) {
		crx_openssl_store_errors();
		goto clean_exit;
	}
	if (CMS_decrypt(cms, key, cert, NULL, out, 0)) {
		RETVAL_TRUE;
	} else {
		crx_openssl_store_errors();
	}
clean_exit:
	if (cms) {
		CMS_ContentInfo_free(cms);
	}
	BIO_free(datain);
	BIO_free(in);
	BIO_free(out);
	if (cert && free_recipcert) {
		X509_free(cert);
	}
	EVP_PKEY_free(key);
}
/* }}} */

/* }}} */



/* {{{ Encrypts data with private key */
CRX_FUNCTION(openssl_private_encrypt)
{
	zval *key, *crypted;
	char * data;
	size_t data_len;
	crex_long padding = RSA_PKCS1_PADDING;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "szz|l", &data, &data_len, &crypted, &key, &padding) == FAILURE) {
		RETURN_THROWS();
	}

	EVP_PKEY *pkey = crx_openssl_pkey_from_zval(key, 0, "", 0, 3);
	if (pkey == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "key param is not a valid private key");
		}
		RETURN_FALSE;
	}

	size_t out_len = 0;
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL);
	if (!ctx || EVP_PKEY_sign_init(ctx) <= 0 ||
			EVP_PKEY_CTX_set_rsa_padding(ctx, padding) <= 0 ||
			EVP_PKEY_sign(ctx, NULL, &out_len, (unsigned char *) data, data_len) <= 0) {
		crx_openssl_store_errors();
		RETVAL_FALSE;
		goto cleanup;
	}

	crex_string *out = crex_string_alloc(out_len, 0);
	if (EVP_PKEY_sign(ctx, (unsigned char *) ZSTR_VAL(out), &out_len,
			(unsigned char *) data, data_len) <= 0) {
		crex_string_release(out);
		crx_openssl_store_errors();
		RETVAL_FALSE;
		goto cleanup;
	}

	ZSTR_VAL(out)[out_len] = '\0';
	CREX_TRY_ASSIGN_REF_NEW_STR(crypted, out);
	RETVAL_TRUE;

cleanup:
	EVP_PKEY_CTX_free(ctx);
	EVP_PKEY_free(pkey);
}
/* }}} */

/* {{{ Decrypts data with private key */
CRX_FUNCTION(openssl_private_decrypt)
{
	zval *key, *crypted;
	crex_long padding = RSA_PKCS1_PADDING;
	char * data;
	size_t data_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "szz|l", &data, &data_len, &crypted, &key, &padding) == FAILURE) {
		RETURN_THROWS();
	}

	EVP_PKEY *pkey = crx_openssl_pkey_from_zval(key, 0, "", 0, 3);
	if (pkey == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "key parameter is not a valid private key");
		}
		RETURN_FALSE;
	}

	size_t out_len = 0;
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL);
	if (!ctx || EVP_PKEY_decrypt_init(ctx) <= 0 ||
			EVP_PKEY_CTX_set_rsa_padding(ctx, padding) <= 0 ||
			EVP_PKEY_decrypt(ctx, NULL, &out_len, (unsigned char *) data, data_len) <= 0) {
		crx_openssl_store_errors();
		RETVAL_FALSE;
		goto cleanup;
	}

	crex_string *out = crex_string_alloc(out_len, 0);
	if (EVP_PKEY_decrypt(ctx, (unsigned char *) ZSTR_VAL(out), &out_len,
			(unsigned char *) data, data_len) <= 0) {
		crex_string_release(out);
		crx_openssl_store_errors();
		RETVAL_FALSE;
		goto cleanup;
	}

	out = crex_string_truncate(out, out_len, 0);
	ZSTR_VAL(out)[out_len] = '\0';
	CREX_TRY_ASSIGN_REF_NEW_STR(crypted, out);
	RETVAL_TRUE;

cleanup:
	EVP_PKEY_CTX_free(ctx);
	EVP_PKEY_free(pkey);
}
/* }}} */

/* {{{ Encrypts data with public key */
CRX_FUNCTION(openssl_public_encrypt)
{
	zval *key, *crypted;
	crex_long padding = RSA_PKCS1_PADDING;
	char * data;
	size_t data_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "szz|l", &data, &data_len, &crypted, &key, &padding) == FAILURE) {
		RETURN_THROWS();
	}

	EVP_PKEY *pkey = crx_openssl_pkey_from_zval(key, 1, NULL, 0, 3);
	if (pkey == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "key parameter is not a valid public key");
		}
		RETURN_FALSE;
	}

	size_t out_len = 0;
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL);
	if (!ctx || EVP_PKEY_encrypt_init(ctx) <= 0 ||
			EVP_PKEY_CTX_set_rsa_padding(ctx, padding) <= 0 ||
			EVP_PKEY_encrypt(ctx, NULL, &out_len, (unsigned char *) data, data_len) <= 0) {
		crx_openssl_store_errors();
		RETVAL_FALSE;
		goto cleanup;
	}

	crex_string *out = crex_string_alloc(out_len, 0);
	if (EVP_PKEY_encrypt(ctx, (unsigned char *) ZSTR_VAL(out), &out_len,
			(unsigned char *) data, data_len) <= 0) {
		crex_string_release(out);
		crx_openssl_store_errors();
		RETVAL_FALSE;
		goto cleanup;
	}

	ZSTR_VAL(out)[out_len] = '\0';
	CREX_TRY_ASSIGN_REF_NEW_STR(crypted, out);
	RETVAL_TRUE;

cleanup:
	EVP_PKEY_CTX_free(ctx);
	EVP_PKEY_free(pkey);
}
/* }}} */

/* {{{ Decrypts data with public key */
CRX_FUNCTION(openssl_public_decrypt)
{
	zval *key, *crypted;
	crex_long padding = RSA_PKCS1_PADDING;
	char * data;
	size_t data_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "szz|l", &data, &data_len, &crypted, &key, &padding) == FAILURE) {
		RETURN_THROWS();
	}

	EVP_PKEY *pkey = crx_openssl_pkey_from_zval(key, 1, NULL, 0, 3);
	if (pkey == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "key parameter is not a valid public key");
		}
		RETURN_FALSE;
	}

	size_t out_len = 0;
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL);
	if (!ctx || EVP_PKEY_verify_recover_init(ctx) <= 0 ||
			EVP_PKEY_CTX_set_rsa_padding(ctx, padding) <= 0 ||
			EVP_PKEY_verify_recover(ctx, NULL, &out_len, (unsigned char *) data, data_len) <= 0) {
		crx_openssl_store_errors();
		RETVAL_FALSE;
		goto cleanup;
	}

	crex_string *out = crex_string_alloc(out_len, 0);
	if (EVP_PKEY_verify_recover(ctx, (unsigned char *) ZSTR_VAL(out), &out_len,
			(unsigned char *) data, data_len) <= 0) {
		crex_string_release(out);
		crx_openssl_store_errors();
		RETVAL_FALSE;
		goto cleanup;
	}

	out = crex_string_truncate(out, out_len, 0);
	ZSTR_VAL(out)[out_len] = '\0';
	CREX_TRY_ASSIGN_REF_NEW_STR(crypted, out);
	RETVAL_TRUE;

cleanup:
	EVP_PKEY_CTX_free(ctx);
	EVP_PKEY_free(pkey);
}
/* }}} */

/* {{{ Returns a description of the last error, and alters the index of the error messages. Returns false when the are no more messages */
CRX_FUNCTION(openssl_error_string)
{
	char buf[256];
	unsigned long val;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	crx_openssl_store_errors();

	if (OPENSSL_G(errors) == NULL || OPENSSL_G(errors)->top == OPENSSL_G(errors)->bottom) {
		RETURN_FALSE;
	}

	OPENSSL_G(errors)->bottom = (OPENSSL_G(errors)->bottom + 1) % ERR_NUM_ERRORS;
	val = OPENSSL_G(errors)->buffer[OPENSSL_G(errors)->bottom];

	if (val) {
		ERR_error_string_n(val, buf, 256);
		RETURN_STRING(buf);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Signs data */
CRX_FUNCTION(openssl_sign)
{
	zval *key, *signature;
	EVP_PKEY *pkey;
	unsigned int siglen;
	crex_string *sigbuf;
	char * data;
	size_t data_len;
	EVP_MD_CTX *md_ctx;
	crex_string *method_str = NULL;
	crex_long method_long = OPENSSL_ALGO_SHA1;
	const EVP_MD *mdtype;

	CREX_PARSE_PARAMETERS_START(3, 4)
		C_PARAM_STRING(data, data_len)
		C_PARAM_ZVAL(signature)
		C_PARAM_ZVAL(key)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_LONG(method_str, method_long)
	CREX_PARSE_PARAMETERS_END();

	pkey = crx_openssl_pkey_from_zval(key, 0, "", 0, 3);
	if (pkey == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Supplied key param cannot be coerced into a private key");
		}
		RETURN_FALSE;
	}

	if (method_str) {
		mdtype = EVP_get_digestbyname(ZSTR_VAL(method_str));
	} else {
		mdtype = crx_openssl_get_evp_md_from_algo(method_long);
	}
	if (!mdtype) {
		crx_error_docref(NULL, E_WARNING, "Unknown digest algorithm");
		RETURN_FALSE;
	}

	siglen = EVP_PKEY_size(pkey);
	sigbuf = crex_string_alloc(siglen, 0);

	md_ctx = EVP_MD_CTX_create();
	if (md_ctx != NULL &&
			EVP_SignInit(md_ctx, mdtype) &&
			EVP_SignUpdate(md_ctx, data, data_len) &&
			EVP_SignFinal(md_ctx, (unsigned char*)ZSTR_VAL(sigbuf), &siglen, pkey)) {
		ZSTR_VAL(sigbuf)[siglen] = '\0';
		ZSTR_LEN(sigbuf) = siglen;
		CREX_TRY_ASSIGN_REF_NEW_STR(signature, sigbuf);
		RETVAL_TRUE;
	} else {
		crx_openssl_store_errors();
		efree(sigbuf);
		RETVAL_FALSE;
	}
	EVP_MD_CTX_destroy(md_ctx);
	EVP_PKEY_free(pkey);
}
/* }}} */

/* {{{ Verifys data */
CRX_FUNCTION(openssl_verify)
{
	zval *key;
	EVP_PKEY *pkey;
	int err = 0;
	EVP_MD_CTX *md_ctx;
	const EVP_MD *mdtype;
	char * data;
	size_t data_len;
	char * signature;
	size_t signature_len;
	crex_string *method_str = NULL;
	crex_long method_long = OPENSSL_ALGO_SHA1;

	CREX_PARSE_PARAMETERS_START(3, 4)
		C_PARAM_STRING(data, data_len)
		C_PARAM_STRING(signature, signature_len)
		C_PARAM_ZVAL(key)
		C_PARAM_OPTIONAL
		C_PARAM_STR_OR_LONG(method_str, method_long)
	CREX_PARSE_PARAMETERS_END();

	CRX_OPENSSL_CHECK_SIZE_T_TO_UINT(signature_len, signature, 2);

	if (method_str) {
		mdtype = EVP_get_digestbyname(ZSTR_VAL(method_str));
	} else {
		mdtype = crx_openssl_get_evp_md_from_algo(method_long);
	}
	if (!mdtype) {
		crx_error_docref(NULL, E_WARNING, "Unknown digest algorithm");
		RETURN_FALSE;
	}

	pkey = crx_openssl_pkey_from_zval(key, 1, NULL, 0, 3);
	if (pkey == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Supplied key param cannot be coerced into a public key");
		}
		RETURN_FALSE;
	}

	md_ctx = EVP_MD_CTX_create();
	if (md_ctx == NULL ||
			!EVP_VerifyInit (md_ctx, mdtype) ||
			!EVP_VerifyUpdate (md_ctx, data, data_len) ||
			(err = EVP_VerifyFinal(md_ctx, (unsigned char *)signature, (unsigned int)signature_len, pkey)) < 0) {
		crx_openssl_store_errors();
	}
	EVP_MD_CTX_destroy(md_ctx);
	EVP_PKEY_free(pkey);
	RETURN_LONG(err);
}
/* }}} */

/* {{{ Seals data */
CRX_FUNCTION(openssl_seal)
{
	zval *pubkeys, *pubkey, *sealdata, *ekeys, *iv = NULL;
	HashTable *pubkeysht;
	EVP_PKEY **pkeys;
	int i, len1, len2, *eksl, nkeys, iv_len;
	unsigned char iv_buf[EVP_MAX_IV_LENGTH + 1], *buf = NULL, **eks;
	char * data;
	size_t data_len;
	char *method;
	size_t method_len;
	const EVP_CIPHER *cipher;
	EVP_CIPHER_CTX *ctx;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "szzas|z", &data, &data_len,
				&sealdata, &ekeys, &pubkeys, &method, &method_len, &iv) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_OPENSSL_CHECK_SIZE_T_TO_INT(data_len, data, 1);

	pubkeysht = C_ARRVAL_P(pubkeys);
	nkeys = pubkeysht ? crex_hash_num_elements(pubkeysht) : 0;
	if (!nkeys) {
		crex_argument_value_error(4, "cannot be empty");
		RETURN_THROWS();
	}

	cipher = EVP_get_cipherbyname(method);
	if (!cipher) {
		crx_error_docref(NULL, E_WARNING, "Unknown cipher algorithm");
		RETURN_FALSE;
	}

	iv_len = EVP_CIPHER_iv_length(cipher);
	if (!iv && iv_len > 0) {
		crex_argument_value_error(6, "cannot be null for the chosen cipher algorithm");
		RETURN_THROWS();
	}

	pkeys = safe_emalloc(nkeys, sizeof(*pkeys), 0);
	eksl = safe_emalloc(nkeys, sizeof(*eksl), 0);
	eks = safe_emalloc(nkeys, sizeof(*eks), 0);
	memset(eks, 0, sizeof(*eks) * nkeys);
	memset(pkeys, 0, sizeof(*pkeys) * nkeys);

	/* get the public keys we are using to seal this data */
	i = 0;
	CREX_HASH_FOREACH_VAL(pubkeysht, pubkey) {
		pkeys[i] = crx_openssl_pkey_from_zval(pubkey, 1, NULL, 0, 4);
		if (pkeys[i] == NULL) {
			if (!EG(exception)) {
				crx_error_docref(NULL, E_WARNING, "Not a public key (%dth member of pubkeys)", i+1);
			}
			RETVAL_FALSE;
			goto clean_exit;
		}
		eks[i] = emalloc(EVP_PKEY_size(pkeys[i]) + 1);
		i++;
	} CREX_HASH_FOREACH_END();

	ctx = EVP_CIPHER_CTX_new();
	if (ctx == NULL || !EVP_EncryptInit(ctx,cipher,NULL,NULL)) {
		EVP_CIPHER_CTX_free(ctx);
		crx_openssl_store_errors();
		RETVAL_FALSE;
		goto clean_exit;
	}

	/* allocate one byte extra to make room for \0 */
	buf = emalloc(data_len + EVP_CIPHER_CTX_block_size(ctx));
	EVP_CIPHER_CTX_reset(ctx);

	if (EVP_SealInit(ctx, cipher, eks, eksl, &iv_buf[0], pkeys, nkeys) <= 0 ||
			!EVP_SealUpdate(ctx, buf, &len1, (unsigned char *)data, (int)data_len) ||
			!EVP_SealFinal(ctx, buf + len1, &len2)) {
		efree(buf);
		EVP_CIPHER_CTX_free(ctx);
		crx_openssl_store_errors();
		RETVAL_FALSE;
		goto clean_exit;
	}

	if (len1 + len2 > 0) {
		CREX_TRY_ASSIGN_REF_NEW_STR(sealdata, crex_string_init((char*)buf, len1 + len2, 0));
		efree(buf);

		ekeys = crex_try_array_init(ekeys);
		if (!ekeys) {
			EVP_CIPHER_CTX_free(ctx);
			goto clean_exit;
		}

		for (i=0; i<nkeys; i++) {
			eks[i][eksl[i]] = '\0';
			add_next_index_stringl(ekeys, (const char*)eks[i], eksl[i]);
			efree(eks[i]);
			eks[i] = NULL;
		}

		if (iv) {
			iv_buf[iv_len] = '\0';
			CREX_TRY_ASSIGN_REF_NEW_STR(iv, crex_string_init((char*)iv_buf, iv_len, 0));
		}
	} else {
		efree(buf);
	}
	RETVAL_LONG(len1 + len2);
	EVP_CIPHER_CTX_free(ctx);

clean_exit:
	for (i=0; i<nkeys; i++) {
		if (pkeys[i] != NULL) {
			EVP_PKEY_free(pkeys[i]);
		}
		if (eks[i]) {
			efree(eks[i]);
		}
	}
	efree(eks);
	efree(eksl);
	efree(pkeys);
}
/* }}} */

/* {{{ Opens data */
CRX_FUNCTION(openssl_open)
{
	zval *privkey, *opendata;
	EVP_PKEY *pkey;
	int len1, len2, cipher_iv_len;
	unsigned char *buf, *iv_buf;
	EVP_CIPHER_CTX *ctx;
	char * data;
	size_t data_len;
	char * ekey;
	size_t ekey_len;
	char *method, *iv = NULL;
	size_t method_len, iv_len = 0;
	const EVP_CIPHER *cipher;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "szszs|s!", &data, &data_len, &opendata,
				&ekey, &ekey_len, &privkey, &method, &method_len, &iv, &iv_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_OPENSSL_CHECK_SIZE_T_TO_INT(data_len, data, 1);
	CRX_OPENSSL_CHECK_SIZE_T_TO_INT(ekey_len, ekey, 3);
	pkey = crx_openssl_pkey_from_zval(privkey, 0, "", 0, 4);

	if (pkey == NULL) {
		if (!EG(exception)) {
			crx_error_docref(NULL, E_WARNING, "Unable to coerce parameter 4 into a private key");
		}
		RETURN_FALSE;
	}

	cipher = EVP_get_cipherbyname(method);
	if (!cipher) {
		crx_error_docref(NULL, E_WARNING, "Unknown cipher algorithm");
		RETURN_FALSE;
	}

	cipher_iv_len = EVP_CIPHER_iv_length(cipher);
	if (cipher_iv_len > 0) {
		if (!iv) {
			crex_argument_value_error(6, "cannot be null for the chosen cipher algorithm");
			RETURN_THROWS();
		}
		if ((size_t)cipher_iv_len != iv_len) {
			crx_error_docref(NULL, E_WARNING, "IV length is invalid");
			RETURN_FALSE;
		}
		iv_buf = (unsigned char *)iv;
	} else {
		iv_buf = NULL;
	}

	buf = emalloc(data_len + 1);

	ctx = EVP_CIPHER_CTX_new();
	if (ctx != NULL && EVP_OpenInit(ctx, cipher, (unsigned char *)ekey, (int)ekey_len, iv_buf, pkey) &&
			EVP_OpenUpdate(ctx, buf, &len1, (unsigned char *)data, (int)data_len) &&
			EVP_OpenFinal(ctx, buf + len1, &len2) && (len1 + len2 > 0)) {
		buf[len1 + len2] = '\0';
		CREX_TRY_ASSIGN_REF_NEW_STR(opendata, crex_string_init((char*)buf, len1 + len2, 0));
		RETVAL_TRUE;
	} else {
		crx_openssl_store_errors();
		RETVAL_FALSE;
	}

	efree(buf);
	EVP_PKEY_free(pkey);
	EVP_CIPHER_CTX_free(ctx);
}
/* }}} */

static void crx_openssl_add_method_or_alias(const OBJ_NAME *name, void *arg) /* {{{ */
{
	add_next_index_string((zval*)arg, (char*)name->name);
}
/* }}} */

static void crx_openssl_add_method(const OBJ_NAME *name, void *arg) /* {{{ */
{
	if (name->alias == 0) {
		add_next_index_string((zval*)arg, (char*)name->name);
	}
}
/* }}} */

/* {{{ Return array of available digest algorithms */
CRX_FUNCTION(openssl_get_md_methods)
{
	bool aliases = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|b", &aliases) == FAILURE) {
		RETURN_THROWS();
	}
	array_init(return_value);
	OBJ_NAME_do_all_sorted(OBJ_NAME_TYPE_MD_METH,
		aliases ? crx_openssl_add_method_or_alias: crx_openssl_add_method,
		return_value);
}
/* }}} */

#if CRX_OPENSSL_API_VERSION >= 0x30000
static void crx_openssl_add_cipher_name(const char *name, void *arg)
{
	size_t len = strlen(name);
	crex_string *str = crex_string_alloc(len, 0);
	crex_str_tolower_copy(ZSTR_VAL(str), name, len);
	add_next_index_str((zval*)arg, str);
}

static void crx_openssl_add_cipher_or_alias(EVP_CIPHER *cipher, void *arg)
{
	EVP_CIPHER_names_do_all(cipher, crx_openssl_add_cipher_name, arg);
}

static void crx_openssl_add_cipher(EVP_CIPHER *cipher, void *arg)
{
	crx_openssl_add_cipher_name(EVP_CIPHER_get0_name(cipher), arg);
}

static int crx_openssl_compare_func(Bucket *a, Bucket *b)
{
	return string_compare_function(&a->val, &b->val);
}
#endif

/* {{{ Return array of available cipher algorithms */
CRX_FUNCTION(openssl_get_cipher_methods)
{
	bool aliases = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|b", &aliases) == FAILURE) {
		RETURN_THROWS();
	}
	array_init(return_value);
#if CRX_OPENSSL_API_VERSION >= 0x30000
	EVP_CIPHER_do_all_provided(NULL,
		aliases ? crx_openssl_add_cipher_or_alias : crx_openssl_add_cipher,
		return_value);
	crex_hash_sort(C_ARRVAL_P(return_value), crx_openssl_compare_func, 1);
#else
	OBJ_NAME_do_all_sorted(OBJ_NAME_TYPE_CIPHER_METH,
		aliases ? crx_openssl_add_method_or_alias : crx_openssl_add_method,
		return_value);
#endif
}
/* }}} */

/* {{{ Return array of available elliptic curves */
#ifdef HAVE_EVP_PKEY_EC
CRX_FUNCTION(openssl_get_curve_names)
{
	EC_builtin_curve *curves = NULL;
	const char *sname;
	size_t i;
	size_t len = EC_get_builtin_curves(NULL, 0);

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	curves = emalloc(sizeof(EC_builtin_curve) * len);
	if (!EC_get_builtin_curves(curves, len)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	for (i = 0; i < len; i++) {
		sname = OBJ_nid2sn(curves[i].nid);
		if (sname != NULL) {
			add_next_index_string(return_value, sname);
		}
	}
	efree(curves);
}
#endif
/* }}} */

/* {{{ Computes digest hash value for given data using given method, returns raw or binhex encoded string */
CRX_FUNCTION(openssl_digest)
{
	bool raw_output = 0;
	char *data, *method;
	size_t data_len, method_len;
	const EVP_MD *mdtype;
	EVP_MD_CTX *md_ctx;
	unsigned int siglen;
	crex_string *sigbuf;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "ss|b", &data, &data_len, &method, &method_len, &raw_output) == FAILURE) {
		RETURN_THROWS();
	}
	mdtype = EVP_get_digestbyname(method);
	if (!mdtype) {
		crx_error_docref(NULL, E_WARNING, "Unknown digest algorithm");
		RETURN_FALSE;
	}

	siglen = EVP_MD_size(mdtype);
	sigbuf = crex_string_alloc(siglen, 0);

	md_ctx = EVP_MD_CTX_create();
	if (EVP_DigestInit(md_ctx, mdtype) &&
			EVP_DigestUpdate(md_ctx, (unsigned char *)data, data_len) &&
			EVP_DigestFinal (md_ctx, (unsigned char *)ZSTR_VAL(sigbuf), &siglen)) {
		if (raw_output) {
			ZSTR_VAL(sigbuf)[siglen] = '\0';
			ZSTR_LEN(sigbuf) = siglen;
			RETVAL_STR(sigbuf);
		} else {
			int digest_str_len = siglen * 2;
			crex_string *digest_str = crex_string_alloc(digest_str_len, 0);

			make_digest_ex(ZSTR_VAL(digest_str), (unsigned char*)ZSTR_VAL(sigbuf), siglen);
			ZSTR_VAL(digest_str)[digest_str_len] = '\0';
			crex_string_release_ex(sigbuf, 0);
			RETVAL_NEW_STR(digest_str);
		}
	} else {
		crx_openssl_store_errors();
		crex_string_release_ex(sigbuf, 0);
		RETVAL_FALSE;
	}

	EVP_MD_CTX_destroy(md_ctx);
}
/* }}} */

/* Cipher mode info */
struct crx_openssl_cipher_mode {
	bool is_aead;
	bool is_single_run_aead;
	bool set_tag_length_always;
	bool set_tag_length_when_encrypting;
	int aead_get_tag_flag;
	int aead_set_tag_flag;
	int aead_ivlen_flag;
};

#if CRX_OPENSSL_API_VERSION >= 0x10100
static inline void crx_openssl_set_aead_flags(struct crx_openssl_cipher_mode *mode) {
	mode->is_aead = true;
	mode->aead_get_tag_flag = EVP_CTRL_AEAD_GET_TAG;
	mode->aead_set_tag_flag = EVP_CTRL_AEAD_SET_TAG;
	mode->aead_ivlen_flag = EVP_CTRL_AEAD_SET_IVLEN;
}
#endif

static void crx_openssl_load_cipher_mode(struct crx_openssl_cipher_mode *mode, const EVP_CIPHER *cipher_type)
{
	int cipher_mode = EVP_CIPHER_mode(cipher_type);
	memset(mode, 0, sizeof(struct crx_openssl_cipher_mode));
	switch (cipher_mode) {
#if CRX_OPENSSL_API_VERSION >= 0x10100
		/* Since OpenSSL 1.1, all AEAD ciphers use a common framework. We check for
		 * EVP_CIPH_OCB_MODE, because LibreSSL does not support it. */
		case EVP_CIPH_GCM_MODE:
		case EVP_CIPH_CCM_MODE:
# ifdef EVP_CIPH_OCB_MODE
		case EVP_CIPH_OCB_MODE:
			/* For OCB mode, explicitly set the tag length even when decrypting,
			 * see https://github.com/openssl/openssl/issues/8331. */
			mode->set_tag_length_always = cipher_mode == EVP_CIPH_OCB_MODE;
# endif
			crx_openssl_set_aead_flags(mode);
			mode->set_tag_length_when_encrypting = cipher_mode == EVP_CIPH_CCM_MODE;
			mode->is_single_run_aead = cipher_mode == EVP_CIPH_CCM_MODE;
			break;
# ifdef NID_chacha20_poly1305
		default:
			if (EVP_CIPHER_nid(cipher_type) == NID_chacha20_poly1305) {
				crx_openssl_set_aead_flags(mode);
			}
			break;

# endif
#else
# ifdef EVP_CIPH_GCM_MODE
		case EVP_CIPH_GCM_MODE:
			mode->is_aead = 1;
			mode->aead_get_tag_flag = EVP_CTRL_GCM_GET_TAG;
			mode->aead_set_tag_flag = EVP_CTRL_GCM_SET_TAG;
			mode->aead_ivlen_flag = EVP_CTRL_GCM_SET_IVLEN;
			break;
# endif
# ifdef EVP_CIPH_CCM_MODE
		case EVP_CIPH_CCM_MODE:
			mode->is_aead = 1;
			mode->is_single_run_aead = 1;
			mode->set_tag_length_when_encrypting = 1;
			mode->aead_get_tag_flag = EVP_CTRL_CCM_GET_TAG;
			mode->aead_set_tag_flag = EVP_CTRL_CCM_SET_TAG;
			mode->aead_ivlen_flag = EVP_CTRL_CCM_SET_IVLEN;
			break;
# endif
#endif
	}
}

static int crx_openssl_validate_iv(const char **piv, size_t *piv_len, size_t iv_required_len,
		bool *free_iv, EVP_CIPHER_CTX *cipher_ctx, struct crx_openssl_cipher_mode *mode) /* {{{ */
{
	char *iv_new;

	if (mode->is_aead) {
		if (EVP_CIPHER_CTX_ctrl(cipher_ctx, mode->aead_ivlen_flag, *piv_len, NULL) != 1) {
			crx_error_docref(NULL, E_WARNING, "Setting of IV length for AEAD mode failed");
			return FAILURE;
		}
		return SUCCESS;
	}

	/* Best case scenario, user behaved */
	if (*piv_len == iv_required_len) {
		return SUCCESS;
	}

	iv_new = ecalloc(1, iv_required_len + 1);

	if (*piv_len == 0) {
		/* BC behavior */
		*piv_len = iv_required_len;
		*piv = iv_new;
		*free_iv = 1;
		return SUCCESS;

	}

	if (*piv_len < iv_required_len) {
		crx_error_docref(NULL, E_WARNING,
				"IV passed is only %zd bytes long, cipher expects an IV of precisely %zd bytes, padding with \\0",
				*piv_len, iv_required_len);
		memcpy(iv_new, *piv, *piv_len);
		*piv_len = iv_required_len;
		*piv = iv_new;
		*free_iv = 1;
		return SUCCESS;
	}

	crx_error_docref(NULL, E_WARNING,
			"IV passed is %zd bytes long which is longer than the %zd expected by selected cipher, truncating",
			*piv_len, iv_required_len);
	memcpy(iv_new, *piv, iv_required_len);
	*piv_len = iv_required_len;
	*piv = iv_new;
	*free_iv = 1;
	return SUCCESS;

}
/* }}} */

static int crx_openssl_cipher_init(const EVP_CIPHER *cipher_type,
		EVP_CIPHER_CTX *cipher_ctx, struct crx_openssl_cipher_mode *mode,
		const char **ppassword, size_t *ppassword_len, bool *free_password,
		const char **piv, size_t *piv_len, bool *free_iv,
		const char *tag, int tag_len, crex_long options, int enc)  /* {{{ */
{
	unsigned char *key;
	int key_len, password_len;
	size_t max_iv_len;

	*free_password = 0;

	max_iv_len = EVP_CIPHER_iv_length(cipher_type);
	if (enc && *piv_len == 0 && max_iv_len > 0 && !mode->is_aead) {
		crx_error_docref(NULL, E_WARNING,
				"Using an empty Initialization Vector (iv) is potentially insecure and not recommended");
	}

	if (!EVP_CipherInit_ex(cipher_ctx, cipher_type, NULL, NULL, NULL, enc)) {
		crx_openssl_store_errors();
		return FAILURE;
	}
	if (crx_openssl_validate_iv(piv, piv_len, max_iv_len, free_iv, cipher_ctx, mode) == FAILURE) {
		return FAILURE;
	}
	if (mode->set_tag_length_always || (enc && mode->set_tag_length_when_encrypting)) {
		if (!EVP_CIPHER_CTX_ctrl(cipher_ctx, mode->aead_set_tag_flag, tag_len, NULL)) {
			crx_error_docref(NULL, E_WARNING, "Setting tag length for AEAD cipher failed");
			return FAILURE;
		}
	}
	if (!enc && tag && tag_len > 0) {
		if (!mode->is_aead) {
			crx_error_docref(NULL, E_WARNING, "The tag cannot be used because the cipher algorithm does not support AEAD");
		} else if (!EVP_CIPHER_CTX_ctrl(cipher_ctx, mode->aead_set_tag_flag, tag_len, (unsigned char *) tag)) {
			crx_error_docref(NULL, E_WARNING, "Setting tag for AEAD cipher decryption failed");
			return FAILURE;
		}
	}
	/* check and set key */
	password_len = (int) *ppassword_len;
	key_len = EVP_CIPHER_key_length(cipher_type);
	if (key_len > password_len) {
		if ((OPENSSL_DONT_ZERO_PAD_KEY & options) && !EVP_CIPHER_CTX_set_key_length(cipher_ctx, password_len)) {
			crx_openssl_store_errors();
			crx_error_docref(NULL, E_WARNING, "Key length cannot be set for the cipher algorithm");
			return FAILURE;
		}
		key = emalloc(key_len);
		memset(key, 0, key_len);
		memcpy(key, *ppassword, password_len);
		*ppassword = (char *) key;
		*ppassword_len = key_len;
		*free_password = 1;
	} else {
		if (password_len > key_len && !EVP_CIPHER_CTX_set_key_length(cipher_ctx, password_len)) {
			crx_openssl_store_errors();
		}
		key = (unsigned char*)*ppassword;
	}

	if (!EVP_CipherInit_ex(cipher_ctx, NULL, NULL, key, (unsigned char *)*piv, enc)) {
		crx_openssl_store_errors();
		return FAILURE;
	}
	if (options & OPENSSL_ZERO_PADDING) {
		EVP_CIPHER_CTX_set_padding(cipher_ctx, 0);
	}

	return SUCCESS;
}
/* }}} */

static int crx_openssl_cipher_update(const EVP_CIPHER *cipher_type,
		EVP_CIPHER_CTX *cipher_ctx, struct crx_openssl_cipher_mode *mode,
		crex_string **poutbuf, int *poutlen, const char *data, size_t data_len,
		const char *aad, size_t aad_len, int enc)  /* {{{ */
{
	int i = 0;

	if (mode->is_single_run_aead && !EVP_CipherUpdate(cipher_ctx, NULL, &i, NULL, (int)data_len)) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Setting of data length failed");
		return FAILURE;
	}

	if (mode->is_aead && !EVP_CipherUpdate(cipher_ctx, NULL, &i, (const unsigned char *) aad, (int) aad_len)) {
		crx_openssl_store_errors();
		crx_error_docref(NULL, E_WARNING, "Setting of additional application data failed");
		return FAILURE;
	}

	*poutbuf = crex_string_alloc((int)data_len + EVP_CIPHER_block_size(cipher_type), 0);

	if (!EVP_CipherUpdate(cipher_ctx, (unsigned char*)ZSTR_VAL(*poutbuf),
					&i, (const unsigned char *)data, (int)data_len)) {
		/* we don't show warning when we fail but if we ever do, then it should look like this:
		if (mode->is_single_run_aead && !enc) {
			crx_error_docref(NULL, E_WARNING, "Tag verifycation failed");
		} else {
			crx_error_docref(NULL, E_WARNING, enc ? "Encryption failed" : "Decryption failed");
		}
		*/
		crx_openssl_store_errors();
		crex_string_release_ex(*poutbuf, 0);
		return FAILURE;
	}

	*poutlen = i;

	return SUCCESS;
}
/* }}} */


CRX_OPENSSL_API crex_string* crx_openssl_encrypt(
	const char *data, size_t data_len,
	const char *method, size_t method_len,
	const char *password, size_t password_len,
	crex_long options,
	const char *iv, size_t iv_len,
	zval *tag, crex_long tag_len,
	const char *aad, size_t aad_len)
{
	const EVP_CIPHER *cipher_type;
	EVP_CIPHER_CTX *cipher_ctx;
	struct crx_openssl_cipher_mode mode;
	int i = 0, outlen;
	bool free_iv = 0, free_password = 0;
	crex_string *outbuf = NULL;

	CRX_OPENSSL_CHECK_SIZE_T_TO_INT_NULL_RETURN(data_len, data);
	CRX_OPENSSL_CHECK_SIZE_T_TO_INT_NULL_RETURN(password_len, password);
	CRX_OPENSSL_CHECK_SIZE_T_TO_INT_NULL_RETURN(aad_len, aad);
	CRX_OPENSSL_CHECK_LONG_TO_INT_NULL_RETURN(tag_len, tag_len);


	cipher_type = EVP_get_cipherbyname(method);
	if (!cipher_type) {
		crx_error_docref(NULL, E_WARNING, "Unknown cipher algorithm");
		return NULL;
	}

	cipher_ctx = EVP_CIPHER_CTX_new();
	if (!cipher_ctx) {
		crx_error_docref(NULL, E_WARNING, "Failed to create cipher context");
		return NULL;
	}

	crx_openssl_load_cipher_mode(&mode, cipher_type);

	if (crx_openssl_cipher_init(cipher_type, cipher_ctx, &mode,
				&password, &password_len, &free_password,
				&iv, &iv_len, &free_iv, NULL, tag_len, options, 1) == FAILURE ||
			crx_openssl_cipher_update(cipher_type, cipher_ctx, &mode, &outbuf, &outlen,
				data, data_len, aad, aad_len, 1) == FAILURE) {
		outbuf = NULL;
	} else if (EVP_EncryptFinal(cipher_ctx, (unsigned char *)ZSTR_VAL(outbuf) + outlen, &i)) {
		outlen += i;
		if (options & OPENSSL_RAW_DATA) {
			ZSTR_VAL(outbuf)[outlen] = '\0';
			ZSTR_LEN(outbuf) = outlen;
		} else {
			crex_string *base64_str;

			base64_str = crx_base64_encode((unsigned char*)ZSTR_VAL(outbuf), outlen);
			crex_string_release_ex(outbuf, 0);
			outbuf = base64_str;
		}
		if (mode.is_aead && tag) {
			crex_string *tag_str = crex_string_alloc(tag_len, 0);

			if (EVP_CIPHER_CTX_ctrl(cipher_ctx, mode.aead_get_tag_flag, tag_len, ZSTR_VAL(tag_str)) == 1) {
				ZSTR_VAL(tag_str)[tag_len] = '\0';
				ZSTR_LEN(tag_str) = tag_len;
				CREX_TRY_ASSIGN_REF_NEW_STR(tag, tag_str);
			} else {
				crx_error_docref(NULL, E_WARNING, "Retrieving verification tag failed");
				crex_string_release_ex(tag_str, 0);
				crex_string_release_ex(outbuf, 0);
				outbuf = NULL;
			}
		} else if (tag) {
			CREX_TRY_ASSIGN_REF_NULL(tag);
		} else if (mode.is_aead) {
			crx_error_docref(NULL, E_WARNING, "A tag should be provided when using AEAD mode");
			crex_string_release_ex(outbuf, 0);
			outbuf = NULL;
		}
	} else {
		crx_openssl_store_errors();
		crex_string_release_ex(outbuf, 0);
		outbuf = NULL;
	}

	if (free_password) {
		efree((void *) password);
	}
	if (free_iv) {
		efree((void *) iv);
	}
	EVP_CIPHER_CTX_reset(cipher_ctx);
	EVP_CIPHER_CTX_free(cipher_ctx);
	return outbuf;
}

/* {{{ Encrypts given data with given method and key, returns raw or base64 encoded string */
CRX_FUNCTION(openssl_encrypt)
{
	crex_long options = 0, tag_len = 16;
	char *data, *method, *password, *iv = "", *aad = "";
	size_t data_len, method_len, password_len, iv_len = 0, aad_len = 0;
	crex_string *ret;
	zval *tag = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sss|lszsl", &data, &data_len, &method, &method_len,
					&password, &password_len, &options, &iv, &iv_len, &tag, &aad, &aad_len, &tag_len) == FAILURE) {
		RETURN_THROWS();
	}

	if ((ret = crx_openssl_encrypt(data, data_len, method, method_len, password, password_len, options, iv, iv_len, tag, tag_len, aad, aad_len))) {
		RETVAL_STR(ret);
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

CRX_OPENSSL_API crex_string* crx_openssl_decrypt(
	const char *data, size_t data_len,
	const char *method, size_t method_len,
	const char *password, size_t password_len,
	crex_long options,
	const char *iv, size_t iv_len,
	const char *tag, crex_long tag_len,
	const char *aad, size_t aad_len)
{
	const EVP_CIPHER *cipher_type;
	EVP_CIPHER_CTX *cipher_ctx;
	struct crx_openssl_cipher_mode mode;
	int i = 0, outlen;
	crex_string *base64_str = NULL;
	bool free_iv = 0, free_password = 0;
	crex_string *outbuf = NULL;

	CRX_OPENSSL_CHECK_SIZE_T_TO_INT_NULL_RETURN(data_len, data);
	CRX_OPENSSL_CHECK_SIZE_T_TO_INT_NULL_RETURN(password_len, password);
	CRX_OPENSSL_CHECK_SIZE_T_TO_INT_NULL_RETURN(aad_len, aad);
	CRX_OPENSSL_CHECK_SIZE_T_TO_INT_NULL_RETURN(tag_len, tag);


	cipher_type = EVP_get_cipherbyname(method);
	if (!cipher_type) {
		crx_error_docref(NULL, E_WARNING, "Unknown cipher algorithm");
		return NULL;
	}

	cipher_ctx = EVP_CIPHER_CTX_new();
	if (!cipher_ctx) {
		crx_error_docref(NULL, E_WARNING, "Failed to create cipher context");
		return NULL;
	}

	crx_openssl_load_cipher_mode(&mode, cipher_type);

	if (!(options & OPENSSL_RAW_DATA)) {
		base64_str = crx_base64_decode((unsigned char*)data, data_len);
		if (!base64_str) {
			crx_error_docref(NULL, E_WARNING, "Failed to base64 decode the input");
			EVP_CIPHER_CTX_free(cipher_ctx);
			return NULL;
		}
		data_len = ZSTR_LEN(base64_str);
		data = ZSTR_VAL(base64_str);
	}

	if (crx_openssl_cipher_init(cipher_type, cipher_ctx, &mode,
				&password, &password_len, &free_password,
				&iv, &iv_len, &free_iv, tag, tag_len, options, 0) == FAILURE ||
			crx_openssl_cipher_update(cipher_type, cipher_ctx, &mode, &outbuf, &outlen,
				data, data_len, aad, aad_len, 0) == FAILURE) {
		outbuf = NULL;
	} else if (mode.is_single_run_aead ||
			EVP_DecryptFinal(cipher_ctx, (unsigned char *)ZSTR_VAL(outbuf) + outlen, &i)) {
		outlen += i;
		ZSTR_VAL(outbuf)[outlen] = '\0';
		ZSTR_LEN(outbuf) = outlen;
	} else {
		crx_openssl_store_errors();
		crex_string_release_ex(outbuf, 0);
		outbuf = NULL;
	}

	if (free_password) {
		efree((void *) password);
	}
	if (free_iv) {
		efree((void *) iv);
	}
	if (base64_str) {
		crex_string_release_ex(base64_str, 0);
	}
	EVP_CIPHER_CTX_reset(cipher_ctx);
	EVP_CIPHER_CTX_free(cipher_ctx);
	return outbuf;
}

/* {{{ Takes raw or base64 encoded string and decrypts it using given method and key */
CRX_FUNCTION(openssl_decrypt)
{
	crex_long options = 0;
	char *data, *method, *password, *iv = "", *tag = NULL, *aad = "";
	size_t data_len, method_len, password_len, iv_len = 0, tag_len = 0, aad_len = 0;
	crex_string *ret;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "sss|lss!s", &data, &data_len, &method, &method_len,
					&password, &password_len, &options, &iv, &iv_len, &tag, &tag_len, &aad, &aad_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (!method_len) {
		crex_argument_value_error(2, "cannot be empty");
		RETURN_THROWS();
	}

	if ((ret = crx_openssl_decrypt(data, data_len, method, method_len, password, password_len, options, iv, iv_len, tag, tag_len, aad, aad_len))) {
		RETVAL_STR(ret);
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

static inline const EVP_CIPHER *crx_openssl_get_evp_cipher_by_name(const char *method)
{
	const EVP_CIPHER *cipher_type;

	cipher_type = EVP_get_cipherbyname(method);
	if (!cipher_type) {
		crx_error_docref(NULL, E_WARNING, "Unknown cipher algorithm");
		return NULL;
	}

	return cipher_type;
}

CRX_OPENSSL_API crex_long crx_openssl_cipher_iv_length(const char *method)
{
	const EVP_CIPHER *cipher_type = crx_openssl_get_evp_cipher_by_name(method);

	return cipher_type == NULL ? -1 : EVP_CIPHER_iv_length(cipher_type);
}

CRX_FUNCTION(openssl_cipher_iv_length)
{
	crex_string *method;
	crex_long ret;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S", &method) == FAILURE) {
		RETURN_THROWS();
	}

	if (ZSTR_LEN(method) == 0) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	/* Warning is emitted in crx_openssl_cipher_iv_length */
	if ((ret = crx_openssl_cipher_iv_length(ZSTR_VAL(method))) == -1) {
		RETURN_FALSE;
	}

	RETURN_LONG(ret);
}

CRX_OPENSSL_API crex_long crx_openssl_cipher_key_length(const char *method)
{
	const EVP_CIPHER *cipher_type = crx_openssl_get_evp_cipher_by_name(method);

	return cipher_type == NULL ? -1 : EVP_CIPHER_key_length(cipher_type);
}

CRX_FUNCTION(openssl_cipher_key_length)
{
	crex_string *method;
	crex_long ret;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S", &method) == FAILURE) {
		RETURN_THROWS();
	}

	if (ZSTR_LEN(method) == 0) {
		crex_argument_value_error(1, "cannot be empty");
		RETURN_THROWS();
	}

	/* Warning is emitted in crx_openssl_cipher_key_length */
	if ((ret = crx_openssl_cipher_key_length(ZSTR_VAL(method))) == -1) {
		RETURN_FALSE;
	}

	RETURN_LONG(ret);
}

CRX_OPENSSL_API crex_string* crx_openssl_random_pseudo_bytes(crex_long buffer_length)
{
	crex_string *buffer = NULL;
	if (buffer_length <= 0) {
		crex_argument_value_error(1, "must be greater than 0");
		return NULL;
	}
	if (CREX_LONG_INT_OVFL(buffer_length)) {
		crex_argument_value_error(1, "must be less than or equal to %d", INT_MAX);
		return NULL;
	}
	buffer = crex_string_alloc(buffer_length, 0);

	CRX_OPENSSL_CHECK_LONG_TO_INT_NULL_RETURN(buffer_length, length);
	CRX_OPENSSL_RAND_ADD_TIME();
	if (RAND_bytes((unsigned char*)ZSTR_VAL(buffer), (int)buffer_length) <= 0) {
		crex_string_release_ex(buffer, 0);
		crex_throw_exception(crex_ce_exception, "Error reading from source device", 0);
		return NULL;
	} else {
		crx_openssl_store_errors();
	}

	return buffer;
}

/* {{{ Returns a string of the length specified filled with random pseudo bytes */
CRX_FUNCTION(openssl_random_pseudo_bytes)
{
	crex_string *buffer = NULL;
	crex_long buffer_length;
	zval *zstrong_result_returned = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l|z", &buffer_length, &zstrong_result_returned) == FAILURE) {
		RETURN_THROWS();
	}

	if (zstrong_result_returned) {
		CREX_TRY_ASSIGN_REF_FALSE(zstrong_result_returned);
	}

	if ((buffer = crx_openssl_random_pseudo_bytes(buffer_length))) {
		ZSTR_VAL(buffer)[buffer_length] = 0;
		RETVAL_NEW_STR(buffer);
	}

	if (zstrong_result_returned) {
		CREX_TRY_ASSIGN_REF_TRUE(zstrong_result_returned);
	}
}
/* }}} */
