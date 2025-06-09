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
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "crx_test.h"
#include "observer.h"
#include "crex_observer.h"
#include "crex_smart_str.h"
#include "ext/standard/crx_var.h"

static crex_observer_fcall_handlers observer_fcall_init(crex_execute_data *execute_data);

static int observer_show_opcode_in_user_handler(crex_execute_data *execute_data)
{
	if (ZT_G(observer_show_output)) {
		crx_printf("%*s<!-- opcode: '%s' in user handler -->\n", 2 * ZT_G(observer_nesting_depth), "", crex_get_opcode_name(EX(opline)->opcode));
	}

	return CREX_USER_OPCODE_DISPATCH;
}

static void observer_set_user_opcode_handler(const char *opcode_names, user_opcode_handler_t handler)
{
	const char *s = NULL, *e = opcode_names;

	while (1) {
		if (*e == ' ' || *e == ',' || *e == '\0') {
			if (s) {
				uint8_t opcode = crex_get_opcode_id(s, e - s);
				if (opcode <= CREX_VM_LAST_OPCODE) {
					crex_set_user_opcode_handler(opcode, handler);
				} else {
					crex_error(E_WARNING, "Invalid opcode name %.*s", (int) (e - s), e);
				}
				s = NULL;
			}
		} else {
			if (!s) {
				s = e;
			}
		}
		if (*e == '\0') {
			break;
		}
		e++;
	}
}

static void observer_show_opcode(crex_execute_data *execute_data)
{
	if (!ZT_G(observer_show_opcode) || !CREX_USER_CODE(EX(func)->type)) {
		return;
	}
	crx_printf("%*s<!-- opcode: '%s' -->\n", 2 * ZT_G(observer_nesting_depth), "", crex_get_opcode_name(EX(opline)->opcode));
}

static void observer_begin(crex_execute_data *execute_data)
{
	if (!ZT_G(observer_show_output)) {
		return;
	}

	if (execute_data->func && execute_data->func->common.function_name) {
		if (execute_data->func->common.scope) {
			crx_printf("%*s<%s::%s>\n", 2 * ZT_G(observer_nesting_depth), "", ZSTR_VAL(execute_data->func->common.scope->name), ZSTR_VAL(execute_data->func->common.function_name));
		} else {
			crx_printf("%*s<%s>\n", 2 * ZT_G(observer_nesting_depth), "", ZSTR_VAL(execute_data->func->common.function_name));
		}
	} else {
		crx_printf("%*s<file '%s'>\n", 2 * ZT_G(observer_nesting_depth), "", ZSTR_VAL(execute_data->func->op_array.filename));
	}
	ZT_G(observer_nesting_depth)++;
	observer_show_opcode(execute_data);
}

static void get_retval_info(zval *retval, smart_str *buf)
{
	if (!ZT_G(observer_show_return_type) && !ZT_G(observer_show_return_value)) {
		return;
	}

	smart_str_appendc(buf, ':');
	if (retval == NULL) {
		smart_str_appendl(buf, "NULL", 4);
	} else if (ZT_G(observer_show_return_value)) {
		if (C_TYPE_P(retval) == IS_OBJECT) {
			smart_str_appendl(buf, "object(", 7);
			smart_str_append(buf, C_OBJCE_P(retval)->name);
			smart_str_appendl(buf, ")#", 2);
			smart_str_append_long(buf, C_OBJ_HANDLE_P(retval));
		} else {
			crx_var_export_ex(retval, 2 * ZT_G(observer_nesting_depth) + 3, buf);
		}
	} else if (ZT_G(observer_show_return_type)) {
		smart_str_appends(buf, crex_zval_type_name(retval));
	}
	smart_str_0(buf);
}

static void observer_end(crex_execute_data *execute_data, zval *retval)
{
	if (!ZT_G(observer_show_output)) {
		return;
	}

	if (EG(exception)) {
		crx_printf("%*s<!-- Exception: %s -->\n", 2 * ZT_G(observer_nesting_depth), "", ZSTR_VAL(EG(exception)->ce->name));
	}
	observer_show_opcode(execute_data);
	ZT_G(observer_nesting_depth)--;
	if (execute_data->func && execute_data->func->common.function_name) {
		smart_str retval_info = {0};
		get_retval_info(retval, &retval_info);
		if (execute_data->func->common.scope) {
			crx_printf("%*s</%s::%s%s>\n", 2 * ZT_G(observer_nesting_depth), "", ZSTR_VAL(execute_data->func->common.scope->name), ZSTR_VAL(execute_data->func->common.function_name), retval_info.s ? ZSTR_VAL(retval_info.s) : "");
		} else {
			crx_printf("%*s</%s%s>\n", 2 * ZT_G(observer_nesting_depth), "", ZSTR_VAL(execute_data->func->common.function_name), retval_info.s ? ZSTR_VAL(retval_info.s) : "");
		}
		smart_str_free(&retval_info);
	} else {
		crx_printf("%*s</file '%s'>\n", 2 * ZT_G(observer_nesting_depth), "", ZSTR_VAL(execute_data->func->op_array.filename));
	}
}

static void observer_show_init(crex_function *fbc)
{
	if (fbc->common.function_name) {
		if (fbc->common.scope) {
			crx_printf("%*s<!-- init %s::%s() -->\n", 2 * ZT_G(observer_nesting_depth), "", ZSTR_VAL(fbc->common.scope->name), ZSTR_VAL(fbc->common.function_name));
		} else {
			crx_printf("%*s<!-- init %s() -->\n", 2 * ZT_G(observer_nesting_depth), "", ZSTR_VAL(fbc->common.function_name));
		}
	} else {
		crx_printf("%*s<!-- init '%s' -->\n", 2 * ZT_G(observer_nesting_depth), "", ZSTR_VAL(fbc->op_array.filename));
	}
}

static void observer_show_init_backtrace(crex_execute_data *execute_data)
{
	crex_execute_data *ex = execute_data;
	crx_printf("%*s<!--\n", 2 * ZT_G(observer_nesting_depth), "");
	do {
		crex_function *fbc = ex->func;
		int indent = 2 * ZT_G(observer_nesting_depth) + 4;
		if (fbc->common.function_name) {
			if (fbc->common.scope) {
				crx_printf("%*s%s::%s()\n", indent, "", ZSTR_VAL(fbc->common.scope->name), ZSTR_VAL(fbc->common.function_name));
			} else {
				crx_printf("%*s%s()\n", indent, "", ZSTR_VAL(fbc->common.function_name));
			}
		} else {
			crx_printf("%*s{main} %s\n", indent, "", ZSTR_VAL(fbc->op_array.filename));
		}
	} while ((ex = ex->prev_execute_data) != NULL);
	crx_printf("%*s-->\n", 2 * ZT_G(observer_nesting_depth), "");
}

static crex_observer_fcall_handlers observer_fcall_init(crex_execute_data *execute_data)
{
	crex_function *fbc = execute_data->func;
	if (ZT_G(observer_show_output)) {
		observer_show_init(fbc);
		if (ZT_G(observer_show_init_backtrace)) {
			observer_show_init_backtrace(execute_data);
		}
		observer_show_opcode(execute_data);
	}

	if (ZT_G(observer_observe_all)) {
		return (crex_observer_fcall_handlers){observer_begin, observer_end};
	} else if (fbc->common.function_name) {
		if (ZT_G(observer_observe_functions)) {
			return (crex_observer_fcall_handlers){observer_begin, observer_end};
		} else if (crex_hash_exists(ZT_G(observer_observe_function_names), fbc->common.function_name)) {
			return (crex_observer_fcall_handlers){observer_begin, observer_end};
		}
	} else {
		if (ZT_G(observer_observe_includes)) {
			return (crex_observer_fcall_handlers){observer_begin, observer_end};
		}
	}
	return (crex_observer_fcall_handlers){NULL, NULL};
}

static void fiber_init_observer(crex_fiber_context *initializing) {
	if (ZT_G(observer_fiber_init)) {
		crx_printf("<!-- alloc: %p -->\n", initializing);
	}
}

static void fiber_destroy_observer(crex_fiber_context *destroying) {
	if (ZT_G(observer_fiber_destroy)) {
		crx_printf("<!-- destroy: %p -->\n", destroying);
	}
}

static void fiber_address_observer(crex_fiber_context *from, crex_fiber_context *to)
{
	if (ZT_G(observer_fiber_switch)) {
		crx_printf("<!-- switching from fiber %p to %p -->\n", from, to);
	}
}

static void fiber_enter_observer(crex_fiber_context *from, crex_fiber_context *to)
{
	if (ZT_G(observer_fiber_switch)) {
		if (to->status == CREX_FIBER_STATUS_INIT) {
			crx_printf("<init '%p'>\n", to);
		} else if (to->kind == crex_ce_fiber) {
			crex_fiber *fiber = crex_fiber_from_context(to);
			if (fiber->caller != from) {
				return;
			}

			if (fiber->flags & CREX_FIBER_FLAG_DESTROYED) {
				crx_printf("<destroying '%p'>\n", to);
			} else if (to->status != CREX_FIBER_STATUS_DEAD) {
				crx_printf("<resume '%p'>\n", to);
			}
		}
	}
}

static void fiber_suspend_observer(crex_fiber_context *from, crex_fiber_context *to)
{
	if (ZT_G(observer_fiber_switch)) {
		if (from->status == CREX_FIBER_STATUS_DEAD) {
			crex_fiber *fiber = (from->kind == crex_ce_fiber) ? crex_fiber_from_context(from) : NULL;

			if (fiber && fiber->flags & CREX_FIBER_FLAG_THREW) {
				crx_printf("<threw '%p'>\n", from);
			} else if (fiber && fiber->flags & CREX_FIBER_FLAG_DESTROYED) {
				crx_printf("<destroyed '%p'>\n", from);
			} else {
				crx_printf("<returned '%p'>\n", from);
			}
		} else if (from->kind == crex_ce_fiber) {
			crex_fiber *fiber = crex_fiber_from_context(from);
			if (fiber->caller == NULL) {
				crx_printf("<suspend '%p'>\n", from);
			}
		}
	}
}

void declared_function_observer(crex_op_array *op_array, crex_string *name) {
	if (ZT_G(observer_observe_declaring)) {
		crx_printf("%*s<!-- declared function '%s' -->\n", 2 * ZT_G(observer_nesting_depth), "", ZSTR_VAL(name));
	}
}

void declared_class_observer(crex_class_entry *ce, crex_string *name) {
	if (ZT_G(observer_observe_declaring)) {
		crx_printf("%*s<!-- declared class '%s' -->\n", 2 * ZT_G(observer_nesting_depth), "", ZSTR_VAL(name));
	}
}

static void (*crex_test_prev_execute_internal)(crex_execute_data *execute_data, zval *return_value);
static void crex_test_execute_internal(crex_execute_data *execute_data, zval *return_value) {
	crex_function *fbc = execute_data->func;

	if (fbc->common.function_name) {
		if (fbc->common.scope) {
			crx_printf("%*s<!-- internal enter %s::%s() -->\n", 2 * ZT_G(observer_nesting_depth), "", ZSTR_VAL(fbc->common.scope->name), ZSTR_VAL(fbc->common.function_name));
		} else {
			crx_printf("%*s<!-- internal enter %s() -->\n", 2 * ZT_G(observer_nesting_depth), "", ZSTR_VAL(fbc->common.function_name));
		}
	} else {
		crx_printf("%*s<!-- internal enter '%s' -->\n", 2 * ZT_G(observer_nesting_depth), "", ZSTR_VAL(fbc->op_array.filename));
	}

	if (crex_test_prev_execute_internal) {
		crex_test_prev_execute_internal(execute_data, return_value);
	} else {
		fbc->internal_function.handler(execute_data, return_value);
	}
}

static CREX_INI_MH(crex_test_observer_OnUpdateCommaList)
{
	crex_array **p = (crex_array **) CREX_INI_GET_ADDR();
	crex_string *funcname;
	crex_function *func;
	if (stage != CRX_INI_STAGE_STARTUP && stage != CRX_INI_STAGE_ACTIVATE && stage != CRX_INI_STAGE_DEACTIVATE && stage != CRX_INI_STAGE_SHUTDOWN) {
		CREX_HASH_FOREACH_STR_KEY(*p, funcname) {
			if ((func = crex_hash_find_ptr(EG(function_table), funcname))) {
				crex_observer_remove_begin_handler(func, observer_begin);
				crex_observer_remove_end_handler(func, observer_end);
			}
		} CREX_HASH_FOREACH_END();
	}
	crex_hash_clean(*p);
	if (new_value && ZSTR_LEN(new_value)) {
		const char *start = ZSTR_VAL(new_value), *ptr;
		while ((ptr = strchr(start, ','))) {
			crex_string *str = crex_string_init(start, ptr - start, 1);
			GC_MAKE_PERSISTENT_LOCAL(str);
			crex_hash_add_empty_element(*p, str);
			crex_string_release(str);
			start = ptr + 1;
		}
		crex_string *str = crex_string_init(start, ZSTR_VAL(new_value) + ZSTR_LEN(new_value) - start, 1);
		GC_MAKE_PERSISTENT_LOCAL(str);
		crex_hash_add_empty_element(*p, str);
		crex_string_release(str);
		if (stage != CRX_INI_STAGE_STARTUP && stage != CRX_INI_STAGE_ACTIVATE && stage != CRX_INI_STAGE_DEACTIVATE && stage != CRX_INI_STAGE_SHUTDOWN) {
			CREX_HASH_FOREACH_STR_KEY(*p, funcname) {
				if ((func = crex_hash_find_ptr(EG(function_table), funcname))) {
					crex_observer_add_begin_handler(func, observer_begin);
					crex_observer_add_end_handler(func, observer_end);
				}
			} CREX_HASH_FOREACH_END();
		}
	}
	return SUCCESS;
}

CRX_INI_BEGIN()
	STD_CRX_INI_BOOLEAN("crex_test.observer.enabled", "0", CRX_INI_SYSTEM, OnUpdateBool, observer_enabled, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_BOOLEAN("crex_test.observer.show_output", "1", CRX_INI_SYSTEM, OnUpdateBool, observer_show_output, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_BOOLEAN("crex_test.observer.observe_all", "0", CRX_INI_SYSTEM, OnUpdateBool, observer_observe_all, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_BOOLEAN("crex_test.observer.observe_includes", "0", CRX_INI_SYSTEM, OnUpdateBool, observer_observe_includes, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_BOOLEAN("crex_test.observer.observe_functions", "0", CRX_INI_SYSTEM, OnUpdateBool, observer_observe_functions, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_BOOLEAN("crex_test.observer.observe_declaring", "0", CRX_INI_SYSTEM, OnUpdateBool, observer_observe_declaring, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_ENTRY("crex_test.observer.observe_function_names", "", CRX_INI_ALL, crex_test_observer_OnUpdateCommaList, observer_observe_function_names, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_BOOLEAN("crex_test.observer.show_return_type", "0", CRX_INI_SYSTEM, OnUpdateBool, observer_show_return_type, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_BOOLEAN("crex_test.observer.show_return_value", "0", CRX_INI_SYSTEM, OnUpdateBool, observer_show_return_value, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_BOOLEAN("crex_test.observer.show_init_backtrace", "0", CRX_INI_SYSTEM, OnUpdateBool, observer_show_init_backtrace, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_BOOLEAN("crex_test.observer.show_opcode", "0", CRX_INI_SYSTEM, OnUpdateBool, observer_show_opcode, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_ENTRY("crex_test.observer.show_opcode_in_user_handler", "", CRX_INI_SYSTEM, OnUpdateString, observer_show_opcode_in_user_handler, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_BOOLEAN("crex_test.observer.fiber_init", "0", CRX_INI_SYSTEM, OnUpdateBool, observer_fiber_init, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_BOOLEAN("crex_test.observer.fiber_switch", "0", CRX_INI_SYSTEM, OnUpdateBool, observer_fiber_switch, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_BOOLEAN("crex_test.observer.fiber_destroy", "0", CRX_INI_SYSTEM, OnUpdateBool, observer_fiber_destroy, crex_crex_test_globals, crex_test_globals)
	STD_CRX_INI_BOOLEAN("crex_test.observer.execute_internal", "0", CRX_INI_SYSTEM, OnUpdateBool, observer_execute_internal, crex_crex_test_globals, crex_test_globals)
CRX_INI_END()

void crex_test_observer_init(INIT_FUNC_ARGS)
{
	// Loading via dl() not supported with the observer API
	if (type != MODULE_TEMPORARY) {
		REGISTER_INI_ENTRIES();
		if (ZT_G(observer_enabled)) {
			crex_observer_fcall_register(observer_fcall_init);
		}
	} else {
		(void)ini_entries;
	}

	if (ZT_G(observer_enabled) && ZT_G(observer_show_opcode_in_user_handler)) {
		observer_set_user_opcode_handler(ZT_G(observer_show_opcode_in_user_handler), observer_show_opcode_in_user_handler);
	}

	if (ZT_G(observer_enabled)) {
		crex_observer_fiber_init_register(fiber_init_observer);
		crex_observer_fiber_switch_register(fiber_address_observer);
		crex_observer_fiber_switch_register(fiber_enter_observer);
		crex_observer_fiber_switch_register(fiber_suspend_observer);
		crex_observer_fiber_destroy_register(fiber_destroy_observer);

		crex_observer_function_declared_register(declared_function_observer);
		crex_observer_class_linked_register(declared_class_observer);
	}

	if (ZT_G(observer_execute_internal)) {
		crex_test_prev_execute_internal = crex_execute_internal;
		crex_execute_internal = crex_test_execute_internal;
	}
}

void crex_test_observer_shutdown(SHUTDOWN_FUNC_ARGS)
{
	if (type != MODULE_TEMPORARY) {
		UNREGISTER_INI_ENTRIES();
	}
}

void crex_test_observer_ginit(crex_crex_test_globals *crex_test_globals) {
	crex_test_globals->observer_observe_function_names = malloc(sizeof(HashTable));
	_crex_hash_init(crex_test_globals->observer_observe_function_names, 8, ZVAL_PTR_DTOR, 1);
	GC_MAKE_PERSISTENT_LOCAL(crex_test_globals->observer_observe_function_names);
}

void crex_test_observer_gshutdown(crex_crex_test_globals *crex_test_globals) {
	crex_hash_release(crex_test_globals->observer_observe_function_names);
}
