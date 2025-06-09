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
   | Authors: Marcus Boerger <helly@crx.net>                              |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "crx_main.h"
#include "ext/standard/info.h"
#include "crx_spl.h"
#include "crx_spl_arginfo.h"
#include "spl_functions.h"
#include "spl_engine.h"
#include "spl_array.h"
#include "spl_directory.h"
#include "spl_iterators.h"
#include "spl_exceptions.h"
#include "spl_observer.h"
#include "spl_dllist.h"
#include "spl_fixedarray.h"
#include "spl_heap.h"
#include "crex_exceptions.h"
#include "crex_interfaces.h"
#include "main/snprintf.h"

#ifdef COMPILE_DL_SPL
CREX_GET_MODULE(spl)
#endif

CREX_TLS crex_string *spl_autoload_extensions;
CREX_TLS HashTable *spl_autoload_functions;

#define SPL_DEFAULT_FILE_EXTENSIONS ".inc,.crx"

static crex_class_entry * spl_find_ce_by_name(crex_string *name, bool autoload)
{
	crex_class_entry *ce;

	if (!autoload) {
		crex_string *lc_name = crex_string_tolower(name);

		ce = crex_hash_find_ptr(EG(class_table), lc_name);
		crex_string_release(lc_name);
	} else {
		ce = crex_lookup_class(name);
	}
	if (ce == NULL) {
		crx_error_docref(NULL, E_WARNING, "Class %s does not exist%s", ZSTR_VAL(name), autoload ? " and could not be loaded" : "");
		return NULL;
	}

	return ce;
}

/* {{{ Return an array containing the names of all parent classes */
CRX_FUNCTION(class_parents)
{
	zval *obj;
	crex_class_entry *parent_class, *ce;
	bool autoload = 1;

	/* We do not use C_PARAM_OBJ_OR_STR here to be able to exclude int, float, and bool which are bogus class names */
	if (crex_parse_parameters(CREX_NUM_ARGS(), "z|b", &obj, &autoload) == FAILURE) {
		RETURN_THROWS();
	}

	if (C_TYPE_P(obj) != IS_OBJECT && C_TYPE_P(obj) != IS_STRING) {
		crex_argument_type_error(1, "must be of type object|string, %s given", crex_zval_value_name(obj));
		RETURN_THROWS();
	}

	if (C_TYPE_P(obj) == IS_STRING) {
		if (NULL == (ce = spl_find_ce_by_name(C_STR_P(obj), autoload))) {
			RETURN_FALSE;
		}
	} else {
		ce = C_OBJCE_P(obj);
	}

	array_init(return_value);
	parent_class = ce->parent;
	while (parent_class) {
		spl_add_class_name(return_value, parent_class, 0, 0);
		parent_class = parent_class->parent;
	}
}
/* }}} */

/* {{{ Return all classes and interfaces implemented by SPL */
CRX_FUNCTION(class_implements)
{
	zval *obj;
	bool autoload = 1;
	crex_class_entry *ce;

	/* We do not use C_PARAM_OBJ_OR_STR here to be able to exclude int, float, and bool which are bogus class names */
	if (crex_parse_parameters(CREX_NUM_ARGS(), "z|b", &obj, &autoload) == FAILURE) {
		RETURN_THROWS();
	}
	if (C_TYPE_P(obj) != IS_OBJECT && C_TYPE_P(obj) != IS_STRING) {
		crex_argument_type_error(1, "must be of type object|string, %s given", crex_zval_value_name(obj));
		RETURN_THROWS();
	}

	if (C_TYPE_P(obj) == IS_STRING) {
		if (NULL == (ce = spl_find_ce_by_name(C_STR_P(obj), autoload))) {
			RETURN_FALSE;
		}
	} else {
		ce = C_OBJCE_P(obj);
	}

	array_init(return_value);
	spl_add_interfaces(return_value, ce, 1, CREX_ACC_INTERFACE);
}
/* }}} */

/* {{{ Return all traits used by a class. */
CRX_FUNCTION(class_uses)
{
	zval *obj;
	bool autoload = 1;
	crex_class_entry *ce;

	/* We do not use C_PARAM_OBJ_OR_STR here to be able to exclude int, float, and bool which are bogus class names */
	if (crex_parse_parameters(CREX_NUM_ARGS(), "z|b", &obj, &autoload) == FAILURE) {
		RETURN_THROWS();
	}
	if (C_TYPE_P(obj) != IS_OBJECT && C_TYPE_P(obj) != IS_STRING) {
		crex_argument_type_error(1, "must be of type object|string, %s given", crex_zval_value_name(obj));
		RETURN_THROWS();
	}

	if (C_TYPE_P(obj) == IS_STRING) {
		if (NULL == (ce = spl_find_ce_by_name(C_STR_P(obj), autoload))) {
			RETURN_FALSE;
		}
	} else {
		ce = C_OBJCE_P(obj);
	}

	array_init(return_value);
	spl_add_traits(return_value, ce, 1, CREX_ACC_TRAIT);
}
/* }}} */

#define SPL_ADD_CLASS(class_name, z_list, sub, allow, ce_flags) \
	spl_add_classes(spl_ce_ ## class_name, z_list, sub, allow, ce_flags)

#define SPL_LIST_CLASSES(z_list, sub, allow, ce_flags) \
	SPL_ADD_CLASS(AppendIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(ArrayIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(ArrayObject, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(BadFunctionCallException, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(BadMethodCallException, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(CachingIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(CallbackFilterIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(DirectoryIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(DomainException, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(EmptyIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(FilesystemIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(FilterIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(GlobIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(InfiniteIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(InvalidArgumentException, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(IteratorIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(LengthException, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(LimitIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(LogicException, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(MultipleIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(NoRewindIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(OuterIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(OutOfBoundsException, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(OutOfRangeException, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(OverflowException, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(ParentIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(RangeException, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(RecursiveArrayIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(RecursiveCachingIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(RecursiveCallbackFilterIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(RecursiveDirectoryIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(RecursiveFilterIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(RecursiveIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(RecursiveIteratorIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(RecursiveRegexIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(RecursiveTreeIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(RegexIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(RuntimeException, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(SeekableIterator, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(SplDoublyLinkedList, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(SplFileInfo, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(SplFileObject, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(SplFixedArray, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(SplHeap, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(SplMinHeap, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(SplMaxHeap, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(SplObjectStorage, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(SplObserver, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(SplPriorityQueue, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(SplQueue, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(SplStack, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(SplSubject, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(SplTempFileObject, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(UnderflowException, z_list, sub, allow, ce_flags); \
	SPL_ADD_CLASS(UnexpectedValueException, z_list, sub, allow, ce_flags); \

/* {{{ Return an array containing the names of all clsses and interfaces defined in SPL */
CRX_FUNCTION(spl_classes)
{
	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);

	SPL_LIST_CLASSES(return_value, 0, 0, 0)
}
/* }}} */

static int spl_autoload(crex_string *class_name, crex_string *lc_name, const char *ext, int ext_len) /* {{{ */
{
	crex_string *class_file;
	zval dummy;
	crex_file_handle file_handle;
	crex_op_array *new_op_array;
	zval result;
	int ret;

	class_file = crex_strpprintf(0, "%s%.*s", ZSTR_VAL(lc_name), ext_len, ext);

#if DEFAULT_SLASH != '\\'
	{
		char *ptr = ZSTR_VAL(class_file);
		char *end = ptr + ZSTR_LEN(class_file);

		while ((ptr = memchr(ptr, '\\', (end - ptr))) != NULL) {
			*ptr = DEFAULT_SLASH;
		}
	}
#endif

	crex_stream_init_filename_ex(&file_handle, class_file);
	ret = crx_stream_open_for_crex_ex(&file_handle, USE_PATH|STREAM_OPEN_FOR_INCLUDE);

	if (ret == SUCCESS) {
		crex_string *opened_path;
		if (!file_handle.opened_path) {
			file_handle.opened_path = crex_string_copy(class_file);
		}
		opened_path = crex_string_copy(file_handle.opened_path);
		ZVAL_NULL(&dummy);
		if (crex_hash_add(&EG(included_files), opened_path, &dummy)) {
			new_op_array = crex_compile_file(&file_handle, CREX_REQUIRE);
		} else {
			new_op_array = NULL;
		}
		crex_string_release_ex(opened_path, 0);
		if (new_op_array) {
			uint32_t orig_jit_trace_num = EG(jit_trace_num);

			ZVAL_UNDEF(&result);
			crex_execute(new_op_array, &result);
			EG(jit_trace_num) = orig_jit_trace_num;

			destroy_op_array(new_op_array);
			efree(new_op_array);
			if (!EG(exception)) {
				zval_ptr_dtor(&result);
			}

			crex_destroy_file_handle(&file_handle);
			crex_string_release(class_file);
			return crex_hash_exists(EG(class_table), lc_name);
		}
	}
	crex_destroy_file_handle(&file_handle);
	crex_string_release(class_file);
	return 0;
} /* }}} */

/* {{{ Default autoloader implementation */
CRX_FUNCTION(spl_autoload)
{
	int pos_len, pos1_len;
	char *pos, *pos1;
	crex_string *class_name, *lc_name, *file_exts = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S|S!", &class_name, &file_exts) == FAILURE) {
		RETURN_THROWS();
	}

	if (!file_exts) {
		file_exts = spl_autoload_extensions;
	}

	if (file_exts == NULL) { /* autoload_extensions is not initialized, set to defaults */
		pos = SPL_DEFAULT_FILE_EXTENSIONS;
		pos_len = sizeof(SPL_DEFAULT_FILE_EXTENSIONS) - 1;
	} else {
		pos = ZSTR_VAL(file_exts);
		pos_len = (int)ZSTR_LEN(file_exts);
	}

	lc_name = crex_string_tolower(class_name);
	while (pos && *pos && !EG(exception)) {
		pos1 = strchr(pos, ',');
		if (pos1) {
			pos1_len = (int)(pos1 - pos);
		} else {
			pos1_len = pos_len;
		}
		if (spl_autoload(class_name, lc_name, pos, pos1_len)) {
			break; /* loaded */
		}
		pos = pos1 ? pos1 + 1 : NULL;
		pos_len = pos1? pos_len - pos1_len - 1 : 0;
	}
	crex_string_release(lc_name);
} /* }}} */

/* {{{ Register and return default file extensions for spl_autoload */
CRX_FUNCTION(spl_autoload_extensions)
{
	crex_string *file_exts = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "|S!", &file_exts) == FAILURE) {
		RETURN_THROWS();
	}

	if (file_exts) {
		if (spl_autoload_extensions) {
			crex_string_release_ex(spl_autoload_extensions, 0);
		}
		spl_autoload_extensions = crex_string_copy(file_exts);
	}

	if (spl_autoload_extensions == NULL) {
		RETURN_STRINGL(SPL_DEFAULT_FILE_EXTENSIONS, sizeof(SPL_DEFAULT_FILE_EXTENSIONS) - 1);
	} else {
		crex_string_addref(spl_autoload_extensions);
		RETURN_STR(spl_autoload_extensions);
	}
} /* }}} */

typedef struct {
	crex_function *func_ptr;
	crex_object *obj;
	crex_object *closure;
	crex_class_entry *ce;
} autoload_func_info;

static void autoload_func_info_destroy(autoload_func_info *alfi) {
	if (alfi->obj) {
		crex_object_release(alfi->obj);
	}
	if (alfi->func_ptr &&
		UNEXPECTED(alfi->func_ptr->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE)) {
		crex_string_release_ex(alfi->func_ptr->common.function_name, 0);
		crex_free_trampoline(alfi->func_ptr);
	}
	if (alfi->closure) {
		crex_object_release(alfi->closure);
	}
	efree(alfi);
}

static void autoload_func_info_zval_dtor(zval *element)
{
	autoload_func_info_destroy(C_PTR_P(element));
}

static autoload_func_info *autoload_func_info_from_fci(
		crex_fcall_info *fci, crex_fcall_info_cache *fcc) {
	autoload_func_info *alfi = emalloc(sizeof(autoload_func_info));
	alfi->ce = fcc->calling_scope;
	alfi->func_ptr = fcc->function_handler;
	alfi->obj = fcc->object;
	if (alfi->obj) {
		GC_ADDREF(alfi->obj);
	}
	if (C_TYPE(fci->function_name) == IS_OBJECT) {
		alfi->closure = C_OBJ(fci->function_name);
		GC_ADDREF(alfi->closure);
	} else {
		alfi->closure = NULL;
	}
	return alfi;
}

static bool autoload_func_info_equals(
		const autoload_func_info *alfi1, const autoload_func_info *alfi2) {
	if (UNEXPECTED(
		(alfi1->func_ptr->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE) &&
		(alfi2->func_ptr->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE)
	)) {
		return alfi1->obj == alfi2->obj
			&& alfi1->ce == alfi2->ce
			&& alfi1->closure == alfi2->closure
			&& crex_string_equals(alfi1->func_ptr->common.function_name, alfi2->func_ptr->common.function_name)
		;
	}
	return alfi1->func_ptr == alfi2->func_ptr
		&& alfi1->obj == alfi2->obj
		&& alfi1->ce == alfi2->ce
		&& alfi1->closure == alfi2->closure;
}

static crex_class_entry *spl_perform_autoload(crex_string *class_name, crex_string *lc_name) {
	if (!spl_autoload_functions) {
		return NULL;
	}

	/* We don't use CREX_HASH_MAP_FOREACH here,
	 * because autoloaders may be added/removed during autoloading. */
	HashPosition pos;
	crex_hash_internal_pointer_reset_ex(spl_autoload_functions, &pos);
	while (1) {
		autoload_func_info *alfi =
			crex_hash_get_current_data_ptr_ex(spl_autoload_functions, &pos);
		if (!alfi) {
			break;
		}

		crex_function *func = alfi->func_ptr;
		if (UNEXPECTED(func->common.fn_flags & CREX_ACC_CALL_VIA_TRAMPOLINE)) {
			func = emalloc(sizeof(crex_op_array));
			memcpy(func, alfi->func_ptr, sizeof(crex_op_array));
			crex_string_addref(func->op_array.function_name);
		}

		zval param;
		ZVAL_STR(&param, class_name);
		crex_call_known_function(func, alfi->obj, alfi->ce, NULL, 1, &param, NULL);
		if (EG(exception)) {
			break;
		}

		if (ZSTR_HAS_CE_CACHE(class_name) &&  ZSTR_GET_CE_CACHE(class_name)) {
			return (crex_class_entry*)ZSTR_GET_CE_CACHE(class_name);
		} else {
			crex_class_entry *ce = crex_hash_find_ptr(EG(class_table), lc_name);
			if (ce) {
				return ce;
			}
		}

		crex_hash_move_forward_ex(spl_autoload_functions, &pos);
	}
	return NULL;
}

/* {{{ Try all registered autoload function to load the requested class */
CRX_FUNCTION(spl_autoload_call)
{
	crex_string *class_name;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "S", &class_name) == FAILURE) {
		RETURN_THROWS();
	}

	crex_string *lc_name = crex_string_tolower(class_name);
	spl_perform_autoload(class_name, lc_name);
	crex_string_release(lc_name);
} /* }}} */

#define HT_MOVE_TAIL_TO_HEAD(ht)						        \
	CREX_ASSERT(!HT_IS_PACKED(ht));						        \
	do {												        \
		Bucket tmp = (ht)->arData[(ht)->nNumUsed-1];				\
		memmove((ht)->arData + 1, (ht)->arData,					\
			sizeof(Bucket) * ((ht)->nNumUsed - 1));				\
		(ht)->arData[0] = tmp;									\
		crex_hash_rehash(ht);						        	\
	} while (0)

static Bucket *spl_find_registered_function(autoload_func_info *find_alfi) {
	if (!spl_autoload_functions) {
		return NULL;
	}

	autoload_func_info *alfi;
	CREX_HASH_MAP_FOREACH_PTR(spl_autoload_functions, alfi) {
		if (autoload_func_info_equals(alfi, find_alfi)) {
			return _p;
		}
	} CREX_HASH_FOREACH_END();
	return NULL;
}

/* {{{ Register given function as autoloader */
CRX_FUNCTION(spl_autoload_register)
{
	bool do_throw = 1;
	bool prepend  = 0;
	crex_fcall_info fci = {0};
	crex_fcall_info_cache fcc;
	autoload_func_info *alfi;

	CREX_PARSE_PARAMETERS_START(0, 3)
		C_PARAM_OPTIONAL
		C_PARAM_FUNC_OR_NULL(fci, fcc)
		C_PARAM_BOOL(do_throw)
		C_PARAM_BOOL(prepend)
	CREX_PARSE_PARAMETERS_END();

	if (!do_throw) {
		crx_error_docref(NULL, E_NOTICE, "Argument #2 ($do_throw) has been ignored, "
			"spl_autoload_register() will always throw");
	}

	if (!spl_autoload_functions) {
		ALLOC_HASHTABLE(spl_autoload_functions);
		crex_hash_init(spl_autoload_functions, 1, NULL, autoload_func_info_zval_dtor, 0);
		/* Initialize as non-packed hash table for prepend functionality. */
		crex_hash_real_init_mixed(spl_autoload_functions);
	}

	/* If first arg is not null */
	if (CREX_FCI_INITIALIZED(fci)) {
		if (!fcc.function_handler) {
			/* Call trampoline has been cleared by zpp. Refetch it, because we want to deal
			 * with it outselves. It is important that it is not refetched on every call,
			 * because calls may occur from different scopes. */
			crex_is_callable_ex(&fci.function_name, NULL, IS_CALLABLE_SUPPRESS_DEPRECATIONS, NULL, &fcc, NULL);
		}

		if (fcc.function_handler->type == CREX_INTERNAL_FUNCTION &&
			fcc.function_handler->internal_function.handler == zif_spl_autoload_call) {
			crex_argument_value_error(1, "must not be the spl_autoload_call() function");
			RETURN_THROWS();
		}

		alfi = autoload_func_info_from_fci(&fci, &fcc);
		if (UNEXPECTED(alfi->func_ptr == &EG(trampoline))) {
			crex_function *copy = emalloc(sizeof(crex_op_array));

			memcpy(copy, alfi->func_ptr, sizeof(crex_op_array));
			alfi->func_ptr->common.function_name = NULL;
			alfi->func_ptr = copy;
		}
	} else {
		alfi = emalloc(sizeof(autoload_func_info));
		alfi->func_ptr = crex_hash_str_find_ptr(
			CG(function_table), "spl_autoload", sizeof("spl_autoload") - 1);
		alfi->obj = NULL;
		alfi->ce = NULL;
		alfi->closure = NULL;
	}

	if (spl_find_registered_function(alfi)) {
		autoload_func_info_destroy(alfi);
		RETURN_TRUE;
	}

	crex_hash_next_index_insert_ptr(spl_autoload_functions, alfi);
	if (prepend && spl_autoload_functions->nNumOfElements > 1) {
		/* Move the newly created element to the head of the hashtable */
		HT_MOVE_TAIL_TO_HEAD(spl_autoload_functions);
	}

	RETURN_TRUE;
} /* }}} */

/* {{{ Unregister given function as autoloader */
CRX_FUNCTION(spl_autoload_unregister)
{
	crex_fcall_info fci;
	crex_fcall_info_cache fcc;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_FUNC(fci, fcc)
	CREX_PARSE_PARAMETERS_END();

	if (fcc.function_handler && crex_string_equals_literal(
			fcc.function_handler->common.function_name, "spl_autoload_call")) {
		/* Don't destroy the hash table, as we might be iterating over it right now. */
		crex_hash_clean(spl_autoload_functions);
		RETURN_TRUE;
	}

	if (!fcc.function_handler) {
		/* Call trampoline has been cleared by zpp. Refetch it, because we want to deal
		 * with it outselves. It is important that it is not refetched on every call,
		 * because calls may occur from different scopes. */
		crex_is_callable_ex(&fci.function_name, NULL, 0, NULL, &fcc, NULL);
	}

	autoload_func_info *alfi = autoload_func_info_from_fci(&fci, &fcc);
	Bucket *p = spl_find_registered_function(alfi);
	autoload_func_info_destroy(alfi);
	if (p) {
		crex_hash_del_bucket(spl_autoload_functions, p);
		RETURN_TRUE;
	}

	RETURN_FALSE;
} /* }}} */

/* {{{ Return all registered autoloader functions */
CRX_FUNCTION(spl_autoload_functions)
{
	autoload_func_info *alfi;

	if (crex_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}

	array_init(return_value);
	if (spl_autoload_functions) {
		CREX_HASH_MAP_FOREACH_PTR(spl_autoload_functions, alfi) {
			if (alfi->closure) {
				GC_ADDREF(alfi->closure);
				add_next_index_object(return_value, alfi->closure);
			} else if (alfi->func_ptr->common.scope) {
				zval tmp;

				array_init(&tmp);
				if (alfi->obj) {
					GC_ADDREF(alfi->obj);
					add_next_index_object(&tmp, alfi->obj);
				} else {
					add_next_index_str(&tmp, crex_string_copy(alfi->ce->name));
				}
				add_next_index_str(&tmp, crex_string_copy(alfi->func_ptr->common.function_name));
				add_next_index_zval(return_value, &tmp);
			} else {
				add_next_index_str(return_value, crex_string_copy(alfi->func_ptr->common.function_name));
			}
		} CREX_HASH_FOREACH_END();
	}
} /* }}} */

/* {{{ Return hash id for given object */
CRX_FUNCTION(spl_object_hash)
{
	crex_object *obj;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_OBJ(obj)
	CREX_PARSE_PARAMETERS_END();

	RETURN_NEW_STR(crx_spl_object_hash(obj));
}
/* }}} */

/* {{{ Returns the integer object handle for the given object */
CRX_FUNCTION(spl_object_id)
{
	crex_object *obj;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_OBJ(obj)
	CREX_PARSE_PARAMETERS_END();

	RETURN_LONG((crex_long)obj->handle);
}
/* }}} */

CRXAPI crex_string *crx_spl_object_hash(crex_object *obj) /* {{{*/
{
	return strpprintf(32, "%016zx0000000000000000", (intptr_t)obj->handle);
}
/* }}} */

static void spl_build_class_list_string(zval *entry, char **list) /* {{{ */
{
	char *res;

	spprintf(&res, 0, "%s, %s", *list, C_STRVAL_P(entry));
	efree(*list);
	*list = res;
} /* }}} */

/* {{{ CRX_MINFO(spl) */
CRX_MINFO_FUNCTION(spl)
{
	zval list, *zv;
	char *strg;

	crx_info_print_table_start();
	crx_info_print_table_row(2, "SPL support", "enabled");

	array_init(&list);
	SPL_LIST_CLASSES(&list, 0, 1, CREX_ACC_INTERFACE)
	strg = estrdup("");
	CREX_HASH_MAP_FOREACH_VAL(C_ARRVAL_P(&list), zv) {
		spl_build_class_list_string(zv, &strg);
	} CREX_HASH_FOREACH_END();
	crex_array_destroy(C_ARR(list));
	crx_info_print_table_row(2, "Interfaces", strg + 2);
	efree(strg);

	array_init(&list);
	SPL_LIST_CLASSES(&list, 0, -1, CREX_ACC_INTERFACE)
	strg = estrdup("");
	CREX_HASH_MAP_FOREACH_VAL(C_ARRVAL_P(&list), zv) {
		spl_build_class_list_string(zv, &strg);
	} CREX_HASH_FOREACH_END();
	crex_array_destroy(C_ARR(list));
	crx_info_print_table_row(2, "Classes", strg + 2);
	efree(strg);

	crx_info_print_table_end();
}
/* }}} */

/* {{{ CRX_MINIT_FUNCTION(spl) */
CRX_MINIT_FUNCTION(spl)
{
	crex_autoload = spl_perform_autoload;

	CRX_MINIT(spl_exceptions)(INIT_FUNC_ARGS_PASSTHRU);
	CRX_MINIT(spl_iterators)(INIT_FUNC_ARGS_PASSTHRU);
	CRX_MINIT(spl_array)(INIT_FUNC_ARGS_PASSTHRU);
	CRX_MINIT(spl_directory)(INIT_FUNC_ARGS_PASSTHRU);
	CRX_MINIT(spl_dllist)(INIT_FUNC_ARGS_PASSTHRU);
	CRX_MINIT(spl_heap)(INIT_FUNC_ARGS_PASSTHRU);
	CRX_MINIT(spl_fixedarray)(INIT_FUNC_ARGS_PASSTHRU);
	CRX_MINIT(spl_observer)(INIT_FUNC_ARGS_PASSTHRU);

	return SUCCESS;
}
/* }}} */

CRX_RINIT_FUNCTION(spl) /* {{{ */
{
	spl_autoload_extensions = NULL;
	spl_autoload_functions = NULL;
	return SUCCESS;
} /* }}} */

CRX_RSHUTDOWN_FUNCTION(spl) /* {{{ */
{
	if (spl_autoload_extensions) {
		crex_string_release_ex(spl_autoload_extensions, 0);
		spl_autoload_extensions = NULL;
	}
	if (spl_autoload_functions) {
		crex_hash_destroy(spl_autoload_functions);
		FREE_HASHTABLE(spl_autoload_functions);
		spl_autoload_functions = NULL;
	}
	return SUCCESS;
} /* }}} */

static const crex_module_dep spl_deps[] = {
	CREX_MOD_REQUIRED("json")
	CREX_MOD_END
};

crex_module_entry spl_module_entry = {
	STANDARD_MODULE_HEADER_EX, NULL,
	spl_deps,
	"SPL",
	ext_functions,
	CRX_MINIT(spl),
	NULL,
	CRX_RINIT(spl),
	CRX_RSHUTDOWN(spl),
	CRX_MINFO(spl),
	CRX_SPL_VERSION,
	STANDARD_MODULE_PROPERTIES
};
