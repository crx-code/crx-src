/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_H
#define CREX_H

#define CREX_VERSION "4.3.0"

#define CREX_ENGINE_3

#include "crex_types.h"
#include "crex_map_ptr.h"
#include "crex_errors.h"
#include "crex_alloc.h"
#include "crex_llist.h"
#include "crex_string.h"
#include "crex_hash.h"
#include "crex_ast.h"
#include "crex_gc.h"
#include "crex_variables.h"
#include "crex_iterators.h"
#include "crex_stream.h"
#include "crex_smart_str_public.h"
#include "crex_smart_string_public.h"
#include "crex_signal.h"
#include "crex_max_execution_timer.h"

#define crex_sprintf sprintf

#define HANDLE_BLOCK_INTERRUPTIONS()		CREX_SIGNAL_BLOCK_INTERRUPTIONS()
#define HANDLE_UNBLOCK_INTERRUPTIONS()		CREX_SIGNAL_UNBLOCK_INTERRUPTIONS()

#define INTERNAL_FUNCTION_PARAMETERS crex_execute_data *execute_data, zval *return_value
#define INTERNAL_FUNCTION_PARAM_PASSTHRU execute_data, return_value

#define USED_RET() \
	(!EX(prev_execute_data) || \
	 !CREX_USER_CODE(EX(prev_execute_data)->func->common.type) || \
	 (EX(prev_execute_data)->opline->result_type != IS_UNUSED))

#ifdef CREX_ENABLE_STATIC_TSRMLS_CACHE
#define CREX_TSRMG TSRMG_STATIC
#define CREX_TSRMG_FAST TSRMG_FAST_STATIC
#define CREX_TSRMLS_CACHE_EXTERN() TSRMLS_CACHE_EXTERN()
#define CREX_TSRMLS_CACHE_DEFINE() TSRMLS_CACHE_DEFINE()
#define CREX_TSRMLS_CACHE_UPDATE() TSRMLS_CACHE_UPDATE()
#define CREX_TSRMLS_CACHE TSRMLS_CACHE
#else
#define CREX_TSRMG TSRMG
#define CREX_TSRMG_FAST TSRMG_FAST
#define CREX_TSRMLS_CACHE_EXTERN()
#define CREX_TSRMLS_CACHE_DEFINE()
#define CREX_TSRMLS_CACHE_UPDATE()
#define CREX_TSRMLS_CACHE
#endif

#ifndef CREX_COMPILE_DL_EXT
TSRMLS_MAIN_CACHE_EXTERN()
#else
CREX_TSRMLS_CACHE_EXTERN()
#endif

struct _crex_serialize_data;
struct _crex_unserialize_data;

typedef struct _crex_serialize_data crex_serialize_data;
typedef struct _crex_unserialize_data crex_unserialize_data;

typedef struct _crex_class_name {
	crex_string *name;
	crex_string *lc_name;
} crex_class_name;

typedef struct _crex_trait_method_reference {
	crex_string *method_name;
	crex_string *class_name;
} crex_trait_method_reference;

typedef struct _crex_trait_precedence {
	crex_trait_method_reference trait_method;
	uint32_t num_excludes;
	crex_string *exclude_class_names[1];
} crex_trait_precedence;

typedef struct _crex_trait_alias {
	crex_trait_method_reference trait_method;

	/**
	* name for method to be added
	*/
	crex_string *alias;

	/**
	* modifiers to be set on trait method
	*/
	uint32_t modifiers;
} crex_trait_alias;

typedef struct _crex_class_mutable_data {
	zval      *default_properties_table;
	HashTable *constants_table;
	uint32_t   ce_flags;
	HashTable *backed_enum_table;
} crex_class_mutable_data;

typedef struct _crex_class_dependency {
	crex_string      *name;
	crex_class_entry *ce;
} crex_class_dependency;

typedef struct _crex_inheritance_cache_entry crex_inheritance_cache_entry;

typedef struct _crex_error_info {
	int type;
	uint32_t lineno;
	crex_string *filename;
	crex_string *message;
} crex_error_info;

struct _crex_inheritance_cache_entry {
	crex_inheritance_cache_entry *next;
	crex_class_entry             *ce;
	crex_class_entry             *parent;
	crex_class_dependency        *dependencies;
	uint32_t                      dependencies_count;
	uint32_t                      num_warnings;
	crex_error_info             **warnings;
	crex_class_entry             *traits_and_interfaces[1];
};

struct _crex_class_entry {
	char type;
	crex_string *name;
	/* class_entry or string depending on CREX_ACC_LINKED */
	union {
		crex_class_entry *parent;
		crex_string *parent_name;
	};
	int refcount;
	uint32_t ce_flags;

	int default_properties_count;
	int default_static_members_count;
	zval *default_properties_table;
	zval *default_static_members_table;
	CREX_MAP_PTR_DEF(zval *, static_members_table);
	HashTable function_table;
	HashTable properties_info;
	HashTable constants_table;

	CREX_MAP_PTR_DEF(crex_class_mutable_data*, mutable_data);
	crex_inheritance_cache_entry *inheritance_cache;

	struct _crex_property_info **properties_info_table;

	crex_function *constructor;
	crex_function *destructor;
	crex_function *clone;
	crex_function *__get;
	crex_function *__set;
	crex_function *__unset;
	crex_function *__isset;
	crex_function *__call;
	crex_function *__callstatic;
	crex_function *__tostring;
	crex_function *__debugInfo;
	crex_function *__serialize;
	crex_function *__unserialize;

	const crex_object_handlers *default_object_handlers;

	/* allocated only if class implements Iterator or IteratorAggregate interface */
	crex_class_iterator_funcs *iterator_funcs_ptr;
	/* allocated only if class implements ArrayAccess interface */
	crex_class_arrayaccess_funcs *arrayaccess_funcs_ptr;

	/* handlers */
	union {
		crex_object* (*create_object)(crex_class_entry *class_type);
		int (*interface_gets_implemented)(crex_class_entry *iface, crex_class_entry *class_type); /* a class implements this interface */
	};
	crex_object_iterator *(*get_iterator)(crex_class_entry *ce, zval *object, int by_ref);
	crex_function *(*get_static_method)(crex_class_entry *ce, crex_string* method);

	/* serializer callbacks */
	int (*serialize)(zval *object, unsigned char **buffer, size_t *buf_len, crex_serialize_data *data);
	int (*unserialize)(zval *object, crex_class_entry *ce, const unsigned char *buf, size_t buf_len, crex_unserialize_data *data);

	uint32_t num_interfaces;
	uint32_t num_traits;

	/* class_entry or string(s) depending on CREX_ACC_LINKED */
	union {
		crex_class_entry **interfaces;
		crex_class_name *interface_names;
	};

	crex_class_name *trait_names;
	crex_trait_alias **trait_aliases;
	crex_trait_precedence **trait_precedences;
	HashTable *attributes;

	uint32_t enum_backing_type;
	HashTable *backed_enum_table;

	union {
		struct {
			crex_string *filename;
			uint32_t line_start;
			uint32_t line_end;
			crex_string *doc_comment;
		} user;
		struct {
			const struct _crex_function_entry *builtin_functions;
			struct _crex_module_entry *module;
		} internal;
	} info;
};

typedef struct _crex_utility_functions {
	void (*error_function)(int type, crex_string *error_filename, const uint32_t error_lineno, crex_string *message);
	size_t (*printf_function)(const char *format, ...) CREX_ATTRIBUTE_PTR_FORMAT(printf, 1, 2);
	size_t (*write_function)(const char *str, size_t str_length);
	FILE *(*fopen_function)(crex_string *filename, crex_string **opened_path);
	void (*message_handler)(crex_long message, const void *data);
	zval *(*get_configuration_directive)(crex_string *name);
	void (*ticks_function)(int ticks);
	void (*on_timeout)(int seconds);
	crex_result (*stream_open_function)(crex_file_handle *handle);
	void (*printf_to_smart_string_function)(smart_string *buf, const char *format, va_list ap);
	void (*printf_to_smart_str_function)(smart_str *buf, const char *format, va_list ap);
	char *(*getenv_function)(const char *name, size_t name_len);
	crex_string *(*resolve_path_function)(crex_string *filename);
} crex_utility_functions;

typedef struct _crex_utility_values {
	bool html_errors;
} crex_utility_values;

typedef size_t (*crex_write_func_t)(const char *str, size_t str_length);

#define crex_bailout()		_crex_bailout(__FILE__, __LINE__)

#define crex_try												\
	{															\
		JMP_BUF *__orig_bailout = EG(bailout);					\
		JMP_BUF __bailout;										\
																\
		EG(bailout) = &__bailout;								\
		if (SETJMP(__bailout)==0) {
#define crex_catch												\
		} else {												\
			EG(bailout) = __orig_bailout;
#define crex_end_try()											\
		}														\
		EG(bailout) = __orig_bailout;							\
	}
#define crex_first_try		EG(bailout)=NULL;	crex_try

BEGIN_EXTERN_C()
void crex_startup(crex_utility_functions *utility_functions);
void crex_shutdown(void);
void crex_register_standard_ini_entries(void);
crex_result crex_post_startup(void);
void crex_set_utility_values(crex_utility_values *utility_values);

CREX_API CREX_COLD CREX_NORETURN void _crex_bailout(const char *filename, uint32_t lineno);
CREX_API size_t crex_get_page_size(void);

CREX_API size_t crex_vspprintf(char **pbuf, size_t max_len, const char *format, va_list ap);
CREX_API size_t crex_spprintf(char **message, size_t max_len, const char *format, ...) CREX_ATTRIBUTE_FORMAT(printf, 3, 4);
CREX_API crex_string *crex_vstrpprintf(size_t max_len, const char *format, va_list ap);
CREX_API crex_string *crex_strpprintf(size_t max_len, const char *format, ...) CREX_ATTRIBUTE_FORMAT(printf, 2, 3);

/* Same as crex_spprintf and crex_strpprintf, without checking of format validity.
 * For use with custom printf specifiers such as %H. */
CREX_API size_t crex_spprintf_unchecked(char **message, size_t max_len, const char *format, ...);
CREX_API crex_string *crex_strpprintf_unchecked(size_t max_len, const char *format, ...);

CREX_API const char *get_crex_version(void);
CREX_API bool crex_make_printable_zval(zval *expr, zval *expr_copy);
CREX_API size_t crex_print_zval(zval *expr, int indent);
CREX_API void crex_print_zval_r(zval *expr, int indent);
CREX_API crex_string *crex_print_zval_r_to_str(zval *expr, int indent);
CREX_API void crex_print_flat_zval_r(zval *expr);
void crex_print_flat_zval_r_to_buf(smart_str *str, zval *expr);

static crex_always_inline size_t crex_print_variable(zval *var) {
	return crex_print_zval(var, 0);
}

CREX_API CREX_COLD void crex_output_debug_string(bool trigger_break, const char *format, ...) CREX_ATTRIBUTE_FORMAT(printf, 2, 3);

CREX_API void crex_activate(void);
CREX_API void crex_deactivate(void);
CREX_API void crex_call_destructors(void);
CREX_API void crex_activate_modules(void);
CREX_API void crex_deactivate_modules(void);
CREX_API void crex_post_deactivate_modules(void);

CREX_API void free_estring(char **str_p);

END_EXTERN_C()

/* output support */
#define CREX_WRITE(str, str_len)		crex_write((str), (str_len))
#define CREX_WRITE_EX(str, str_len)		write_func((str), (str_len))
#define CREX_PUTS(str)					crex_write((str), strlen((str)))
#define CREX_PUTS_EX(str)				write_func((str), strlen((str)))
#define CREX_PUTC(c)					crex_write(&(c), 1)

BEGIN_EXTERN_C()
extern CREX_API size_t (*crex_printf)(const char *format, ...) CREX_ATTRIBUTE_PTR_FORMAT(printf, 1, 2);
extern CREX_API crex_write_func_t crex_write;
extern CREX_API FILE *(*crex_fopen)(crex_string *filename, crex_string **opened_path);
extern CREX_API void (*crex_ticks_function)(int ticks);
extern CREX_API void (*crex_interrupt_function)(crex_execute_data *execute_data);
extern CREX_API void (*crex_error_cb)(int type, crex_string *error_filename, const uint32_t error_lineno, crex_string *message);
extern CREX_API void (*crex_on_timeout)(int seconds);
extern CREX_API crex_result (*crex_stream_open_function)(crex_file_handle *handle);
extern void (*crex_printf_to_smart_string)(smart_string *buf, const char *format, va_list ap);
extern void (*crex_printf_to_smart_str)(smart_str *buf, const char *format, va_list ap);
extern CREX_API char *(*crex_getenv)(const char *name, size_t name_len);
extern CREX_API crex_string *(*crex_resolve_path)(crex_string *filename);

/* These two callbacks are especially for opcache */
extern CREX_API crex_result (*crex_post_startup_cb)(void);
extern CREX_API void (*crex_post_shutdown_cb)(void);

CREX_API CREX_COLD void crex_error(int type, const char *format, ...) CREX_ATTRIBUTE_FORMAT(printf, 2, 3);
CREX_API CREX_COLD CREX_NORETURN void crex_error_noreturn(int type, const char *format, ...) CREX_ATTRIBUTE_FORMAT(printf, 2, 3);
/* For custom format specifiers like H */
CREX_API CREX_COLD void crex_error_unchecked(int type, const char *format, ...);
/* If filename is NULL the default filename is used. */
CREX_API CREX_COLD void crex_error_at(int type, crex_string *filename, uint32_t lineno, const char *format, ...) CREX_ATTRIBUTE_FORMAT(printf, 4, 5);
CREX_API CREX_COLD CREX_NORETURN void crex_error_at_noreturn(int type, crex_string *filename, uint32_t lineno, const char *format, ...) CREX_ATTRIBUTE_FORMAT(printf, 4, 5);
CREX_API CREX_COLD void crex_error_zstr(int type, crex_string *message);
CREX_API CREX_COLD void crex_error_zstr_at(int type, crex_string *filename, uint32_t lineno, crex_string *message);

CREX_API CREX_COLD void crex_throw_error(crex_class_entry *exception_ce, const char *format, ...) CREX_ATTRIBUTE_FORMAT(printf, 2, 3);
CREX_API CREX_COLD void crex_type_error(const char *format, ...) CREX_ATTRIBUTE_FORMAT(printf, 1, 2);
CREX_API CREX_COLD void crex_argument_count_error(const char *format, ...) CREX_ATTRIBUTE_FORMAT(printf, 1, 2);
CREX_API CREX_COLD void crex_value_error(const char *format, ...) CREX_ATTRIBUTE_FORMAT(printf, 1, 2);
/* type should be one of the BP_VAR_* constants, only special messages happen for isset/empty and unset */
CREX_API CREX_COLD void crex_illegal_container_offset(const crex_string *container, const zval *offset, int type);

CREX_COLD void crexerror(const char *error);

/* For internal C errors */
CREX_API CREX_COLD CREX_NORETURN void crex_strerror_noreturn(int type, int errn, const char *message);

/* The following #define is used for code duality in CRX for Engine 1 & 2 */
#define CREX_STANDARD_CLASS_DEF_PTR crex_standard_class_def
extern CREX_API crex_class_entry *crex_standard_class_def;
extern CREX_API crex_utility_values crex_uv;

/* If DTrace is available and enabled */
extern CREX_API bool crex_dtrace_enabled;
END_EXTERN_C()

#define CREX_UV(name) (crex_uv.name)

BEGIN_EXTERN_C()
CREX_API void crex_message_dispatcher(crex_long message, const void *data);

CREX_API zval *crex_get_configuration_directive(crex_string *name);
END_EXTERN_C()

/* Messages for applications of Crex */
#define ZMSG_FAILED_INCLUDE_FOPEN		1L
#define ZMSG_FAILED_REQUIRE_FOPEN		2L
#define ZMSG_FAILED_HIGHLIGHT_FOPEN		3L
#define ZMSG_MEMORY_LEAK_DETECTED		4L
#define ZMSG_MEMORY_LEAK_REPEATED		5L
#define ZMSG_LOG_SCRIPT_NAME			6L
#define ZMSG_MEMORY_LEAKS_GRAND_TOTAL	7L

typedef enum {
	EH_NORMAL = 0,
	EH_THROW
} crex_error_handling_t;

typedef struct {
	crex_error_handling_t  handling;
	crex_class_entry       *exception;
} crex_error_handling;

BEGIN_EXTERN_C()
CREX_API void crex_save_error_handling(crex_error_handling *current);
CREX_API void crex_replace_error_handling(crex_error_handling_t error_handling, crex_class_entry *exception_class, crex_error_handling *current);
CREX_API void crex_restore_error_handling(crex_error_handling *saved);
CREX_API void crex_begin_record_errors(void);
CREX_API void crex_emit_recorded_errors(void);
CREX_API void crex_free_recorded_errors(void);
END_EXTERN_C()

#define DEBUG_BACKTRACE_PROVIDE_OBJECT (1<<0)
#define DEBUG_BACKTRACE_IGNORE_ARGS    (1<<1)

#include "crex_object_handlers.h"
#include "crex_operators.h"

#endif /* CREX_H */
