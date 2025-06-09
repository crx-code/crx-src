/*
   +----------------------------------------------------------------------+
   | CRX version 7                                                        |
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
   | Authors: Slava Poliakov <hackie@prohost.org>                         |
   |          Ilia Alshanetsky <ilia@prohost.org>                         |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "crx_shmop.h"
#include "shmop_arginfo.h"

# ifndef CRX_WIN32
# include <sys/ipc.h>
# include <sys/shm.h>
#else
#include "tsrm_win32.h"
#endif


#ifdef HAVE_SHMOP

#include "ext/standard/info.h"

/* {{{ shmop_module_entry */
crex_module_entry shmop_module_entry = {
	STANDARD_MODULE_HEADER,
	"shmop",
	ext_functions,
	CRX_MINIT(shmop),
	NULL,
	NULL,
	NULL,
	CRX_MINFO(shmop),
	CRX_SHMOP_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SHMOP
CREX_GET_MODULE(shmop)
#endif

typedef struct crx_shmop
{
	int shmid;
	key_t key;
	int shmflg;
	int shmatflg;
	char *addr;
	crex_long size;
	crex_object std;
} crx_shmop;

crex_class_entry *shmop_ce;
static crex_object_handlers shmop_object_handlers;

static inline crx_shmop *shmop_from_obj(crex_object *obj)
{
	return (crx_shmop *)((char *)(obj) - XtOffsetOf(crx_shmop, std));
}

#define C_SHMOP_P(zv) shmop_from_obj(C_OBJ_P(zv))

static crex_object *shmop_create_object(crex_class_entry *class_type)
{
	crx_shmop *intern = crex_object_alloc(sizeof(crx_shmop), class_type);

	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	return &intern->std;
}

static crex_function *shmop_get_constructor(crex_object *object)
{
	crex_throw_error(NULL, "Cannot directly construct Shmop, use shmop_open() instead");
	return NULL;
}

static void shmop_free_obj(crex_object *object)
{
	crx_shmop *shmop = shmop_from_obj(object);

	shmdt(shmop->addr);

	crex_object_std_dtor(&shmop->std);
}

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(shmop)
{
	shmop_ce = register_class_Shmop();
	shmop_ce->create_object = shmop_create_object;
	shmop_ce->default_object_handlers = &shmop_object_handlers;

	memcpy(&shmop_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	shmop_object_handlers.offset = XtOffsetOf(crx_shmop, std);
	shmop_object_handlers.free_obj = shmop_free_obj;
	shmop_object_handlers.get_constructor = shmop_get_constructor;
	shmop_object_handlers.clone_obj = NULL;
	shmop_object_handlers.compare = crex_objects_not_comparable;

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(shmop)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "shmop support", "enabled");
	crx_info_print_table_end();
}
/* }}} */

/* {{{ gets and attaches a shared memory segment */
CRX_FUNCTION(shmop_open)
{
	crex_long key, mode, size;
	crx_shmop *shmop;
	struct shmid_ds shm;
	char *flags;
	size_t flags_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "lsll", &key, &flags, &flags_len, &mode, &size) == FAILURE) {
		RETURN_THROWS();
	}

	if (flags_len != 1) {
		crex_argument_value_error(2, "must be a valid access mode");
		RETURN_THROWS();
	}

	object_init_ex(return_value, shmop_ce);
	shmop = C_SHMOP_P(return_value);
	shmop->key = key;
	shmop->shmflg |= mode;

	switch (flags[0])
	{
		case 'a':
			shmop->shmatflg |= SHM_RDONLY;
			break;
		case 'c':
			shmop->shmflg |= IPC_CREAT;
			shmop->size = size;
			break;
		case 'n':
			shmop->shmflg |= (IPC_CREAT | IPC_EXCL);
			shmop->size = size;
			break;
		case 'w':
			/* noop
				shm segment is being opened for read & write
				will fail if segment does not exist
			*/
			break;
		default:
			crex_argument_value_error(2, "must be a valid access mode");
			goto err;
	}

	if (shmop->shmflg & IPC_CREAT && shmop->size < 1) {
		crex_argument_value_error(4, "must be greater than 0 for the \"c\" and \"n\" access modes");
		goto err;
	}

	shmop->shmid = shmget(shmop->key, shmop->size, shmop->shmflg);
	if (shmop->shmid == -1) {
		crx_error_docref(NULL, E_WARNING, "Unable to attach or create shared memory segment \"%s\"", strerror(errno));
		goto err;
	}

	if (shmctl(shmop->shmid, IPC_STAT, &shm)) {
		/* please do not add coverage here: the segment would be leaked and impossible to delete via crx */
		crx_error_docref(NULL, E_WARNING, "Unable to get shared memory segment information \"%s\"", strerror(errno));
		goto err;
	}

	if (shm.shm_segsz > CREX_LONG_MAX) {
		crex_argument_value_error(4, "is too large");
		goto err;
	}

	shmop->addr = shmat(shmop->shmid, 0, shmop->shmatflg);
	if (shmop->addr == (char*) -1) {
		crx_error_docref(NULL, E_WARNING, "Unable to attach to shared memory segment \"%s\"", strerror(errno));
		goto err;
	}

	shmop->size = shm.shm_segsz;
	return;

err:
	crex_object_release(C_OBJ_P(return_value));
	RETURN_FALSE;
}
/* }}} */

/* {{{ reads from a shm segment */
CRX_FUNCTION(shmop_read)
{
	zval *shmid;
	crex_long start, count;
	crx_shmop *shmop;
	char *startaddr;
	int bytes;
	crex_string *return_string;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oll", &shmid, shmop_ce, &start, &count) == FAILURE) {
		RETURN_THROWS();
	}

	shmop = C_SHMOP_P(shmid);

	if (start < 0 || start > shmop->size) {
		crex_argument_value_error(2, "must be between 0 and the segment size");
		RETURN_THROWS();
	}

	if (count < 0 || start > (CREX_LONG_MAX - count) || start + count > shmop->size) {
		crex_argument_value_error(3, "is out of range");
		RETURN_THROWS();
	}

	startaddr = shmop->addr + start;
	bytes = count ? count : shmop->size - start;

	return_string = crex_string_init(startaddr, bytes, 0);

	RETURN_NEW_STR(return_string);
}
/* }}} */

/* {{{ used to close a shared memory segment; now a NOP */
CRX_FUNCTION(shmop_close)
{
	zval *shmid;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &shmid, shmop_ce) == FAILURE) {
		RETURN_THROWS();
	}
}
/* }}} */

/* {{{ returns the shm size */
CRX_FUNCTION(shmop_size)
{
	zval *shmid;
	crx_shmop *shmop;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &shmid, shmop_ce) == FAILURE) {
		RETURN_THROWS();
	}

	shmop = C_SHMOP_P(shmid);

	RETURN_LONG(shmop->size);
}
/* }}} */

/* {{{ writes to a shared memory segment */
CRX_FUNCTION(shmop_write)
{
	crx_shmop *shmop;
	crex_long writesize;
	crex_long offset;
	crex_string *data;
	zval *shmid;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OSl", &shmid, shmop_ce, &data, &offset) == FAILURE) {
		RETURN_THROWS();
	}

	shmop = C_SHMOP_P(shmid);

	if ((shmop->shmatflg & SHM_RDONLY) == SHM_RDONLY) {
		crex_throw_error(NULL, "Read-only segment cannot be written");
		RETURN_THROWS();
	}

	if (offset < 0 || offset > shmop->size) {
		crex_argument_value_error(3, "is out of range");
		RETURN_THROWS();
	}

	writesize = ((crex_long)ZSTR_LEN(data) < shmop->size - offset) ? (crex_long)ZSTR_LEN(data) : shmop->size - offset;
	memcpy(shmop->addr + offset, ZSTR_VAL(data), writesize);

	RETURN_LONG(writesize);
}
/* }}} */

/* {{{ mark segment for deletion */
CRX_FUNCTION(shmop_delete)
{
	zval *shmid;
	crx_shmop *shmop;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &shmid, shmop_ce) == FAILURE) {
		RETURN_THROWS();
	}

	shmop = C_SHMOP_P(shmid);

	if (shmctl(shmop->shmid, IPC_RMID, NULL)) {
		crx_error_docref(NULL, E_WARNING, "Can't mark segment for deletion (are you the owner?)");
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

#endif	/* HAVE_SHMOP */
