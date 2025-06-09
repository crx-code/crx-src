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
   | Author: Christian Cartus <cartus@atrior.de>                          |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"

#ifdef HAVE_SYSVSHM

#include <errno.h>

#include "crx_sysvshm.h"
#include "sysvshm_arginfo.h"
#include "ext/standard/info.h"
#include "ext/standard/crx_var.h"
#include "crex_smart_str.h"
#include "crx_ini.h"

/* SysvSharedMemory class */

crex_class_entry *sysvshm_ce;
static crex_object_handlers sysvshm_object_handlers;

static inline sysvshm_shm *sysvshm_from_obj(crex_object *obj) {
	return (sysvshm_shm *)((char *)(obj) - XtOffsetOf(sysvshm_shm, std));
}

#define C_SYSVSHM_P(zv) sysvshm_from_obj(C_OBJ_P(zv))

static crex_object *sysvshm_create_object(crex_class_entry *class_type) {
	sysvshm_shm *intern = crex_object_alloc(sizeof(sysvshm_shm), class_type);

	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	return &intern->std;
}

static crex_function *sysvshm_get_constructor(crex_object *object) {
	crex_throw_error(NULL, "Cannot directly construct SysvSharedMemory, use shm_attach() instead");
	return NULL;
}

static void sysvshm_free_obj(crex_object *object)
{
	sysvshm_shm *sysvshm = sysvshm_from_obj(object);

	if (sysvshm->ptr) {
		shmdt((void *) sysvshm->ptr);
	}

	crex_object_std_dtor(&sysvshm->std);
}

/* {{{ sysvshm_module_entry */
crex_module_entry sysvshm_module_entry = {
	STANDARD_MODULE_HEADER,
	"sysvshm",
	ext_functions,
	CRX_MINIT(sysvshm),
	NULL,
	NULL,
	NULL,
	CRX_MINFO(sysvshm),
	CRX_SYSVSHM_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SYSVSHM
CREX_GET_MODULE(sysvshm)
#endif

#undef shm_ptr					/* undefine AIX-specific macro */

/* TODO: Make this thread-safe. */
sysvshm_module crx_sysvshm;

static int crx_put_shm_data(sysvshm_chunk_head *ptr, crex_long key, const char *data, crex_long len);
static crex_long crx_check_shm_data(sysvshm_chunk_head *ptr, crex_long key);
static int crx_remove_shm_data(sysvshm_chunk_head *ptr, crex_long shm_varpos);

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(sysvshm)
{
	sysvshm_ce = register_class_SysvSharedMemory();
	sysvshm_ce->create_object = sysvshm_create_object;
	sysvshm_ce->default_object_handlers = &sysvshm_object_handlers;

	memcpy(&sysvshm_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	sysvshm_object_handlers.offset = XtOffsetOf(sysvshm_shm, std);
	sysvshm_object_handlers.free_obj = sysvshm_free_obj;
	sysvshm_object_handlers.get_constructor = sysvshm_get_constructor;
	sysvshm_object_handlers.clone_obj = NULL;
	sysvshm_object_handlers.compare = crex_objects_not_comparable;

	if (cfg_get_long("sysvshm.init_mem", &crx_sysvshm.init_mem) == FAILURE) {
		crx_sysvshm.init_mem=10000;
	}
	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(sysvshm)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "sysvshm support", "enabled");
	crx_info_print_table_end();
}
/* }}} */

/* {{{ Creates or open a shared memory segment */
CRX_FUNCTION(shm_attach)
{
	sysvshm_shm *shm_list_ptr;
	char *shm_ptr;
	sysvshm_chunk_head *chunk_ptr;
	crex_long shm_key, shm_id, shm_size, shm_flag = 0666;
	bool shm_size_is_null = 1;

	if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "l|l!l", &shm_key, &shm_size, &shm_size_is_null, &shm_flag)) {
		RETURN_THROWS();
	}

	if (shm_size_is_null) {
		shm_size = crx_sysvshm.init_mem;
	}

	if (shm_size < 1) {
		crex_argument_value_error(2, "must be greater than 0");
		RETURN_THROWS();
	}

	/* get the id from a specified key or create new shared memory */
	if ((shm_id = shmget(shm_key, 0, 0)) < 0) {
		if (shm_size < (crex_long)sizeof(sysvshm_chunk_head)) {
			crx_error_docref(NULL, E_WARNING, "Failed for key 0x" CREX_XLONG_FMT ": memorysize too small", shm_key);
			RETURN_FALSE;
		}
		if ((shm_id = shmget(shm_key, shm_size, shm_flag | IPC_CREAT | IPC_EXCL)) < 0) {
			crx_error_docref(NULL, E_WARNING, "Failed for key 0x" CREX_XLONG_FMT ": %s", shm_key, strerror(errno));
			RETURN_FALSE;
		}
	}

	if ((shm_ptr = shmat(shm_id, NULL, 0)) == (void *) -1) {
		crx_error_docref(NULL, E_WARNING, "Failed for key 0x" CREX_XLONG_FMT ": %s", shm_key, strerror(errno));
		RETURN_FALSE;
	}

	/* check if shm is already initialized */
	chunk_ptr = (sysvshm_chunk_head *) shm_ptr;
	if (strcmp((char*) &(chunk_ptr->magic), "CRX_SM") != 0) {
		strcpy((char*) &(chunk_ptr->magic), "CRX_SM");
		chunk_ptr->start = sizeof(sysvshm_chunk_head);
		chunk_ptr->end = chunk_ptr->start;
		chunk_ptr->total = shm_size;
		chunk_ptr->free = shm_size-chunk_ptr->end;
	}

	object_init_ex(return_value, sysvshm_ce);

	shm_list_ptr = C_SYSVSHM_P(return_value);

	shm_list_ptr->key = shm_key;
	shm_list_ptr->id = shm_id;
	shm_list_ptr->ptr = chunk_ptr;
}
/* }}} */

/* {{{ Disconnects from shared memory segment */
CRX_FUNCTION(shm_detach)
{
	zval *shm_id;
	sysvshm_shm *shm_list_ptr;

	if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "O", &shm_id, sysvshm_ce)) {
		RETURN_THROWS();
	}

	shm_list_ptr = C_SYSVSHM_P(shm_id);
	if (!shm_list_ptr->ptr) {
		crex_throw_error(NULL, "Shared memory block has already been destroyed");
		RETURN_THROWS();
	}

	shmdt((void *) shm_list_ptr->ptr);
	shm_list_ptr->ptr = NULL;

	RETURN_TRUE;
}
/* }}} */

/* {{{ Removes shared memory from Unix systems */
CRX_FUNCTION(shm_remove)
{
	zval *shm_id;
	sysvshm_shm *shm_list_ptr;

	if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "O", &shm_id, sysvshm_ce)) {
		RETURN_THROWS();
	}

	shm_list_ptr = C_SYSVSHM_P(shm_id);
	if (!shm_list_ptr->ptr) {
		crex_throw_error(NULL, "Shared memory block has already been destroyed");
		RETURN_THROWS();
	}

	if (shmctl(shm_list_ptr->id, IPC_RMID, NULL) < 0) {
		crx_error_docref(NULL, E_WARNING, "Failed for key 0x%x, id " CREX_LONG_FMT ": %s", shm_list_ptr->key, C_LVAL_P(shm_id), strerror(errno));
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Inserts or updates a variable in shared memory */
CRX_FUNCTION(shm_put_var)
{
	zval *shm_id, *arg_var;
	int ret;
	crex_long shm_key;
	sysvshm_shm *shm_list_ptr;
	smart_str shm_var = {0};
	crx_serialize_data_t var_hash;

	if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "Olz", &shm_id, sysvshm_ce, &shm_key, &arg_var)) {
		RETURN_THROWS();
	}

	shm_list_ptr = C_SYSVSHM_P(shm_id);
	if (!shm_list_ptr->ptr) {
		crex_throw_error(NULL, "Shared memory block has already been destroyed");
		RETURN_THROWS();
	}

	/* setup string-variable and serialize */
	CRX_VAR_SERIALIZE_INIT(var_hash);
	crx_var_serialize(&shm_var, arg_var, &var_hash);
	CRX_VAR_SERIALIZE_DESTROY(var_hash);

	/* insert serialized variable into shared memory */
	ret = crx_put_shm_data(shm_list_ptr->ptr, shm_key, shm_var.s? ZSTR_VAL(shm_var.s) : NULL, shm_var.s? ZSTR_LEN(shm_var.s) : 0);

	/* free string */
	smart_str_free(&shm_var);

	if (ret == -1) {
		crx_error_docref(NULL, E_WARNING, "Not enough shared memory left");
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Returns a variable from shared memory */
CRX_FUNCTION(shm_get_var)
{
	zval *shm_id;
	crex_long shm_key;
	sysvshm_shm *shm_list_ptr;
	char *shm_data;
	crex_long shm_varpos;
	sysvshm_chunk *shm_var;
	crx_unserialize_data_t var_hash;

	if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "Ol", &shm_id, sysvshm_ce, &shm_key)) {
		RETURN_THROWS();
	}

	shm_list_ptr = C_SYSVSHM_P(shm_id);
	if (!shm_list_ptr->ptr) {
		crex_throw_error(NULL, "Shared memory block has already been destroyed");
		RETURN_THROWS();
	}

	/* setup string-variable and serialize */
	/* get serialized variable from shared memory */
	shm_varpos = crx_check_shm_data(shm_list_ptr->ptr, shm_key);

	if (shm_varpos < 0) {
		crx_error_docref(NULL, E_WARNING, "Variable key " CREX_LONG_FMT " doesn't exist", shm_key);
		RETURN_FALSE;
	}
	shm_var = (sysvshm_chunk*) ((char *)shm_list_ptr->ptr + shm_varpos);
	shm_data = &shm_var->mem;

	CRX_VAR_UNSERIALIZE_INIT(var_hash);
	if (crx_var_unserialize(return_value, (const unsigned char **) &shm_data, (unsigned char *) shm_data + shm_var->length, &var_hash) != 1) {
		crx_error_docref(NULL, E_WARNING, "Variable data in shared memory is corrupted");
		RETVAL_FALSE;
	}
	CRX_VAR_UNSERIALIZE_DESTROY(var_hash);
}
/* }}} */

/* {{{ Checks whether a specific entry exists */
CRX_FUNCTION(shm_has_var)
{
	zval *shm_id;
	crex_long shm_key;
	sysvshm_shm *shm_list_ptr;

	if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "Ol", &shm_id, sysvshm_ce, &shm_key)) {
		RETURN_THROWS();
	}

	shm_list_ptr = C_SYSVSHM_P(shm_id);
	if (!shm_list_ptr->ptr) {
		crex_throw_error(NULL, "Shared memory block has already been destroyed");
		RETURN_THROWS();
	}

	RETURN_BOOL(crx_check_shm_data(shm_list_ptr->ptr, shm_key) >= 0);
}
/* }}} */

/* {{{ Removes variable from shared memory */
CRX_FUNCTION(shm_remove_var)
{
	zval *shm_id;
	crex_long shm_key, shm_varpos;
	sysvshm_shm *shm_list_ptr;

	if (SUCCESS != crex_parse_parameters(CREX_NUM_ARGS(), "Ol", &shm_id, sysvshm_ce, &shm_key)) {
		RETURN_THROWS();
	}

	shm_list_ptr = C_SYSVSHM_P(shm_id);
	if (!shm_list_ptr->ptr) {
		crex_throw_error(NULL, "Shared memory block has already been destroyed");
		RETURN_THROWS();
	}

	shm_varpos = crx_check_shm_data(shm_list_ptr->ptr, shm_key);

	if (shm_varpos < 0) {
		crx_error_docref(NULL, E_WARNING, "Variable key " CREX_LONG_FMT " doesn't exist", shm_key);
		RETURN_FALSE;
	}
	crx_remove_shm_data((shm_list_ptr->ptr), shm_varpos);
	RETURN_TRUE;
}
/* }}} */

/* {{{ crx_put_shm_data
 * inserts an ascii-string into shared memory */
static int crx_put_shm_data(sysvshm_chunk_head *ptr, crex_long key, const char *data, crex_long len)
{
	sysvshm_chunk *shm_var;
	crex_long total_size;
	crex_long shm_varpos;

	total_size = ((crex_long) (len + sizeof(sysvshm_chunk) - 1) / sizeof(crex_long)) * sizeof(crex_long) + sizeof(crex_long); /* crex_long alligment */

	if ((shm_varpos = crx_check_shm_data(ptr, key)) > 0) {
		crx_remove_shm_data(ptr, shm_varpos);
	}

	if (ptr->free < total_size) {
		return -1; /* not enough memory */
	}

	shm_var = (sysvshm_chunk *) ((char *) ptr + ptr->end);
	shm_var->key = key;
	shm_var->length = len;
	shm_var->next = total_size;
	memcpy(&(shm_var->mem), data, len);
	ptr->end += total_size;
	ptr->free -= total_size;
	return 0;
}
/* }}} */

/* {{{ crx_check_shm_data */
static crex_long crx_check_shm_data(sysvshm_chunk_head *ptr, crex_long key)
{
	crex_long pos;
	sysvshm_chunk *shm_var;

	CREX_ASSERT(ptr);

	pos = ptr->start;

	for (;;) {
		if (pos >= ptr->end) {
			return -1;
		}
		shm_var = (sysvshm_chunk*) ((char *) ptr + pos);
		if (shm_var->key == key) {
			return pos;
		}
		pos += shm_var->next;

		if (shm_var->next <= 0 || pos < ptr->start) {
			return -1;
		}
	}
	return -1;
}
/* }}} */

/* {{{ crx_remove_shm_data */
static int crx_remove_shm_data(sysvshm_chunk_head *ptr, crex_long shm_varpos)
{
	sysvshm_chunk *chunk_ptr, *next_chunk_ptr;
	crex_long memcpy_len;

	CREX_ASSERT(ptr);

	chunk_ptr = (sysvshm_chunk *) ((char *) ptr + shm_varpos);
	next_chunk_ptr = (sysvshm_chunk *) ((char *) ptr + shm_varpos + chunk_ptr->next);

	memcpy_len = ptr->end-shm_varpos - chunk_ptr->next;
	ptr->free += chunk_ptr->next;
	ptr->end -= chunk_ptr->next;
	if (memcpy_len > 0) {
		memmove(chunk_ptr, next_chunk_ptr, memcpy_len);
	}
	return 0;
}
/* }}} */

#endif /* HAVE_SYSVSHM */
