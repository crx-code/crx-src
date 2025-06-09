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
   | Author: Sascha Schumann <sascha@schumann.cx>                         |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_SESSION_H
#define CRX_SESSION_H

#include "ext/standard/crx_var.h"
#include "ext/hash/crx_hash.h"

#define CRX_SESSION_API 20161017

#include "crx_version.h"
#define CRX_SESSION_VERSION CRX_VERSION

/* save handler macros */
#define PS_OPEN_ARGS     void **mod_data, const char *save_path, const char *session_name
#define PS_CLOSE_ARGS    void **mod_data
#define PS_READ_ARGS     void **mod_data, crex_string *key, crex_string **val, crex_long maxlifetime
#define PS_WRITE_ARGS    void **mod_data, crex_string *key, crex_string *val, crex_long maxlifetime
#define PS_DESTROY_ARGS  void **mod_data, crex_string *key
#define PS_GC_ARGS       void **mod_data, crex_long maxlifetime, crex_long *nrdels
#define PS_CREATE_SID_ARGS void **mod_data
#define PS_VALIDATE_SID_ARGS void **mod_data, crex_string *key
#define PS_UPDATE_TIMESTAMP_ARGS void **mod_data, crex_string *key, crex_string *val, crex_long maxlifetime

typedef struct ps_module_struct {
	const char *s_name;
	crex_result (*s_open)(PS_OPEN_ARGS);
	crex_result (*s_close)(PS_CLOSE_ARGS);
	crex_result (*s_read)(PS_READ_ARGS);
	crex_result (*s_write)(PS_WRITE_ARGS);
	crex_result (*s_destroy)(PS_DESTROY_ARGS);
	crex_long (*s_gc)(PS_GC_ARGS);
	crex_string *(*s_create_sid)(PS_CREATE_SID_ARGS);
	crex_result (*s_validate_sid)(PS_VALIDATE_SID_ARGS);
	crex_result (*s_update_timestamp)(PS_UPDATE_TIMESTAMP_ARGS);
} ps_module;

#define PS_GET_MOD_DATA() *mod_data
#define PS_SET_MOD_DATA(a) *mod_data = (a)

#define PS_OPEN_FUNC(x) 	crex_result ps_open_##x(PS_OPEN_ARGS)
#define PS_CLOSE_FUNC(x) 	crex_result ps_close_##x(PS_CLOSE_ARGS)
#define PS_READ_FUNC(x) 	crex_result ps_read_##x(PS_READ_ARGS)
#define PS_WRITE_FUNC(x) 	crex_result ps_write_##x(PS_WRITE_ARGS)
#define PS_DESTROY_FUNC(x) 	crex_result ps_delete_##x(PS_DESTROY_ARGS)
#define PS_GC_FUNC(x) 		crex_long ps_gc_##x(PS_GC_ARGS)
#define PS_CREATE_SID_FUNC(x)	crex_string *ps_create_sid_##x(PS_CREATE_SID_ARGS)
#define PS_VALIDATE_SID_FUNC(x)	crex_result ps_validate_sid_##x(PS_VALIDATE_SID_ARGS)
#define PS_UPDATE_TIMESTAMP_FUNC(x) 	crex_result ps_update_timestamp_##x(PS_UPDATE_TIMESTAMP_ARGS)

/* Legacy save handler module definitions */
#define PS_FUNCS(x) \
	PS_OPEN_FUNC(x); \
	PS_CLOSE_FUNC(x); \
	PS_READ_FUNC(x); \
	PS_WRITE_FUNC(x); \
	PS_DESTROY_FUNC(x); \
	PS_GC_FUNC(x);	\
	PS_CREATE_SID_FUNC(x)

#define PS_MOD(x) \
	#x, ps_open_##x, ps_close_##x, ps_read_##x, ps_write_##x, \
	 ps_delete_##x, ps_gc_##x, crx_session_create_id, \
	 crx_session_validate_sid, crx_session_update_timestamp

/* Legacy SID creation enabled save handler module definitions */
#define PS_FUNCS_SID(x) \
	PS_OPEN_FUNC(x); \
	PS_CLOSE_FUNC(x); \
	PS_READ_FUNC(x); \
	PS_WRITE_FUNC(x); \
	PS_DESTROY_FUNC(x); \
	PS_GC_FUNC(x); \
	PS_CREATE_SID_FUNC(x); \
	PS_VALIDATE_SID_FUNC(x); \
	PS_UPDATE_TIMESTAMP_FUNC(x);

#define PS_MOD_SID(x) \
	#x, ps_open_##x, ps_close_##x, ps_read_##x, ps_write_##x, \
	 ps_delete_##x, ps_gc_##x, ps_create_sid_##x, \
	 crx_session_validate_sid, crx_session_update_timestamp

/* Update timestamp enabled save handler module definitions
   New save handlers should use this API */
#define PS_FUNCS_UPDATE_TIMESTAMP(x) \
	PS_OPEN_FUNC(x); \
	PS_CLOSE_FUNC(x); \
	PS_READ_FUNC(x); \
	PS_WRITE_FUNC(x); \
	PS_DESTROY_FUNC(x); \
	PS_GC_FUNC(x); \
	PS_CREATE_SID_FUNC(x); \
	PS_VALIDATE_SID_FUNC(x); \
	PS_UPDATE_TIMESTAMP_FUNC(x);

#define PS_MOD_UPDATE_TIMESTAMP(x) \
	#x, ps_open_##x, ps_close_##x, ps_read_##x, ps_write_##x, \
	 ps_delete_##x, ps_gc_##x, ps_create_sid_##x, \
	 ps_validate_sid_##x, ps_update_timestamp_##x


typedef enum {
	crx_session_disabled,
	crx_session_none,
	crx_session_active
} crx_session_status;

typedef struct _crx_session_rfc1867_progress {
	size_t    sname_len;
	zval      sid;
	smart_str key;

	crex_long      update_step;
	crex_long      next_update;
	double    next_update_time;
	bool cancel_upload;
	bool apply_trans_sid;
	size_t    content_length;

	zval      data;                 /* the array exported to session data */
	zval	 *post_bytes_processed; /* data["bytes_processed"] */
	zval      files;                /* data["files"] array */
	zval      current_file;         /* array of currently uploading file */
	zval	 *current_file_bytes_processed;
} crx_session_rfc1867_progress;

typedef struct _crx_ps_globals {
	char *save_path;
	char *session_name;
	crex_string *id;
	char *extern_referer_chk;
	char *cache_limiter;
	crex_long cookie_lifetime;
	char *cookie_path;
	char *cookie_domain;
	bool  cookie_secure;
	bool  cookie_httponly;
	char *cookie_samesite;
	const ps_module *mod;
	const ps_module *default_mod;
	void *mod_data;
	crx_session_status session_status;
	crex_string *session_started_filename;
	uint32_t session_started_lineno;
	crex_long gc_probability;
	crex_long gc_divisor;
	crex_long gc_maxlifetime;
	int module_number;
	crex_long cache_expire;
	struct {
		zval ps_open;
		zval ps_close;
		zval ps_read;
		zval ps_write;
		zval ps_destroy;
		zval ps_gc;
		zval ps_create_sid;
		zval ps_validate_sid;
		zval ps_update_timestamp;
	} mod_user_names;
	bool mod_user_implemented;
	bool mod_user_is_open;
	crex_string *mod_user_class_name;
	const struct ps_serializer_struct *serializer;
	zval http_session_vars;
	bool auto_start;
	bool use_cookies;
	bool use_only_cookies;
	bool use_trans_sid; /* contains the INI value of whether to use trans-sid */

	crex_long sid_length;
	crex_long sid_bits_per_character;
	bool send_cookie;
	bool define_sid;

	crx_session_rfc1867_progress *rfc1867_progress;
	bool rfc1867_enabled; /* session.upload_progress.enabled */
	bool rfc1867_cleanup; /* session.upload_progress.cleanup */
	char *rfc1867_prefix;  /* session.upload_progress.prefix */
	char *rfc1867_name;    /* session.upload_progress.name */
	crex_long rfc1867_freq;         /* session.upload_progress.freq */
	double rfc1867_min_freq;   /* session.upload_progress.min_freq */

	bool use_strict_mode; /* whether or not CRX accepts unknown session ids */
	bool lazy_write; /* omit session write when it is possible */
	bool in_save_handler; /* state if session is in save handler or not */
	bool set_handler;     /* state if session module i setting handler or not */
	crex_string *session_vars; /* serialized original session data */
} crx_ps_globals;

typedef crx_ps_globals crex_ps_globals;

extern crex_module_entry session_module_entry;
#define crxext_session_ptr &session_module_entry

#ifdef ZTS
#define PS(v) CREX_TSRMG(ps_globals_id, crx_ps_globals *, v)
#ifdef COMPILE_DL_SESSION
CREX_TSRMLS_CACHE_EXTERN()
#endif
#else
#define PS(v) (ps_globals.v)
#endif

#define PS_SERIALIZER_ENCODE_ARGS void
#define PS_SERIALIZER_DECODE_ARGS const char *val, size_t vallen

typedef struct ps_serializer_struct {
	const char *name;
	crex_string *(*encode)(PS_SERIALIZER_ENCODE_ARGS);
	crex_result (*decode)(PS_SERIALIZER_DECODE_ARGS);
} ps_serializer;

#define PS_SERIALIZER_ENCODE_NAME(x) ps_srlzr_encode_##x
#define PS_SERIALIZER_DECODE_NAME(x) ps_srlzr_decode_##x

#define PS_SERIALIZER_ENCODE_FUNC(x) \
	crex_string *PS_SERIALIZER_ENCODE_NAME(x)(PS_SERIALIZER_ENCODE_ARGS)
#define PS_SERIALIZER_DECODE_FUNC(x) \
	crex_result PS_SERIALIZER_DECODE_NAME(x)(PS_SERIALIZER_DECODE_ARGS)

#define PS_SERIALIZER_FUNCS(x) \
	PS_SERIALIZER_ENCODE_FUNC(x); \
	PS_SERIALIZER_DECODE_FUNC(x)

#define PS_SERIALIZER_ENTRY(x) \
	{ #x, PS_SERIALIZER_ENCODE_NAME(x), PS_SERIALIZER_DECODE_NAME(x) }

/* default create id function */
CRXAPI crex_string *crx_session_create_id(PS_CREATE_SID_ARGS);
/* Dummy PS module functions */
CRXAPI crex_result crx_session_validate_sid(PS_VALIDATE_SID_ARGS);
CRXAPI crex_result crx_session_update_timestamp(PS_UPDATE_TIMESTAMP_ARGS);

CRXAPI void session_adapt_url(const char *url, size_t url_len, char **new_url, size_t *new_len);

CRXAPI crex_result crx_session_destroy(void);
CRXAPI void crx_add_session_var(crex_string *name);
CRXAPI zval *crx_set_session_var(crex_string *name, zval *state_val, crx_unserialize_data_t *var_hash);
CRXAPI zval *crx_get_session_var(crex_string *name);

CRXAPI crex_result crx_session_register_module(const ps_module *);

CRXAPI crex_result crx_session_register_serializer(const char *name,
	        crex_string *(*encode)(PS_SERIALIZER_ENCODE_ARGS),
	        crex_result (*decode)(PS_SERIALIZER_DECODE_ARGS));

CRXAPI crex_result crx_session_start(void);
CRXAPI crex_result crx_session_flush(int write);

CRXAPI const ps_module *_crx_find_ps_module(const char *name);
CRXAPI const ps_serializer *_crx_find_ps_serializer(const char *name);

CRXAPI crex_result crx_session_valid_key(const char *key);
CRXAPI crex_result crx_session_reset_id(void);

#define PS_ADD_VARL(name) do {										\
	crx_add_session_var(name);							\
} while (0)

#define PS_ADD_VAR(name) PS_ADD_VARL(name)

#define PS_DEL_VARL(name) do {										\
	if (!C_ISNULL(PS(http_session_vars))) {							\
		crex_hash_del(C_ARRVAL(PS(http_session_vars)), name);		\
	}																\
} while (0)


#define PS_ENCODE_VARS 												\
	crex_string *key;												\
	crex_ulong num_key;													\
	zval *struc;

#define PS_ENCODE_LOOP(code) do {									\
	HashTable *_ht = C_ARRVAL_P(C_REFVAL(PS(http_session_vars)));	\
	CREX_HASH_FOREACH_KEY(_ht, num_key, key) {						\
		if (key == NULL) {											\
			crx_error_docref(NULL, E_WARNING,						\
					"Skipping numeric key " CREX_LONG_FMT, num_key);\
			continue;												\
		}															\
		if ((struc = crx_get_session_var(key))) {					\
			code;		 											\
		} 															\
	} CREX_HASH_FOREACH_END();										\
} while(0)

CRXAPI CREX_EXTERN_MODULE_GLOBALS(ps)

void crx_session_auto_start(void *data);

extern CRXAPI crex_class_entry *crx_session_class_entry;

extern CRXAPI crex_class_entry *crx_session_iface_entry;

extern CRXAPI crex_class_entry *crx_session_id_iface_entry;

extern CRXAPI crex_class_entry *crx_session_update_timestamp_iface_entry;

extern CRX_METHOD(SessionHandler, open);
extern CRX_METHOD(SessionHandler, close);
extern CRX_METHOD(SessionHandler, read);
extern CRX_METHOD(SessionHandler, write);
extern CRX_METHOD(SessionHandler, destroy);
extern CRX_METHOD(SessionHandler, gc);
extern CRX_METHOD(SessionHandler, create_sid);

#endif
