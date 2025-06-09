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
  | Author: Wez Furlong <wez@thebrainroom.com>                           |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_globals.h"
#include "ext/standard/info.h"
#include "crx_sysvmsg.h"
#include "sysvmsg_arginfo.h"
#include "ext/standard/crx_var.h"
#include "crex_smart_str.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

CRX_MINIT_FUNCTION(sysvmsg);
CRX_MINFO_FUNCTION(sysvmsg);

typedef struct {
	key_t key;
	crex_long id;
	crex_object std;
} sysvmsg_queue_t;

struct crx_msgbuf {
	crex_long mtype;
	char mtext[1];
};

/* {{{ sysvmsg_module_entry */
crex_module_entry sysvmsg_module_entry = {
	STANDARD_MODULE_HEADER,
	"sysvmsg",
	ext_functions,
	CRX_MINIT(sysvmsg),
	NULL,
	NULL,
	NULL,
	CRX_MINFO(sysvmsg),
	CRX_SYSVMSG_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SYSVMSG
CREX_GET_MODULE(sysvmsg)
#endif

/* SysvMessageQueue class */

crex_class_entry *sysvmsg_queue_ce;
static crex_object_handlers sysvmsg_queue_object_handlers;

static inline sysvmsg_queue_t *sysvmsg_queue_from_obj(crex_object *obj) {
	return (sysvmsg_queue_t *)((char *)(obj) - XtOffsetOf(sysvmsg_queue_t, std));
}

#define C_SYSVMSG_QUEUE_P(zv) sysvmsg_queue_from_obj(C_OBJ_P(zv))

static crex_object *sysvmsg_queue_create_object(crex_class_entry *class_type) {
	sysvmsg_queue_t *intern = crex_object_alloc(sizeof(sysvmsg_queue_t), class_type);

	crex_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	return &intern->std;
}

static crex_function *sysvmsg_queue_get_constructor(crex_object *object) {
	crex_throw_error(NULL, "Cannot directly construct SysvMessageQueue, use msg_get_queue() instead");
	return NULL;
}

static void sysvmsg_queue_free_obj(crex_object *object)
{
	sysvmsg_queue_t *sysvmsg_queue = sysvmsg_queue_from_obj(object);

	crex_object_std_dtor(&sysvmsg_queue->std);
}
/* }}} */

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(sysvmsg)
{
	sysvmsg_queue_ce = register_class_SysvMessageQueue();
	sysvmsg_queue_ce->create_object = sysvmsg_queue_create_object;
	sysvmsg_queue_ce->default_object_handlers = &sysvmsg_queue_object_handlers;

	memcpy(&sysvmsg_queue_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	sysvmsg_queue_object_handlers.offset = XtOffsetOf(sysvmsg_queue_t, std);
	sysvmsg_queue_object_handlers.free_obj = sysvmsg_queue_free_obj;
	sysvmsg_queue_object_handlers.get_constructor = sysvmsg_queue_get_constructor;
	sysvmsg_queue_object_handlers.clone_obj = NULL;
	sysvmsg_queue_object_handlers.compare = crex_objects_not_comparable;

	register_sysvmsg_symbols(module_number);

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(sysvmsg)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "sysvmsg support", "enabled");
	crx_info_print_table_end();
}
/* }}} */

/* {{{ Set information for a message queue */
CRX_FUNCTION(msg_set_queue)
{
	zval *queue, *data;
	sysvmsg_queue_t *mq = NULL;
	struct msqid_ds stat;

	RETVAL_FALSE;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oa", &queue, sysvmsg_queue_ce, &data) == FAILURE) {
		RETURN_THROWS();
	}

	mq = C_SYSVMSG_QUEUE_P(queue);

	if (msgctl(mq->id, IPC_STAT, &stat) == 0) {
		zval *item;

		/* now pull out members of data and set them in the stat buffer */
		if ((item = crex_hash_str_find(C_ARRVAL_P(data), "msg_perm.uid", sizeof("msg_perm.uid") - 1)) != NULL) {
			stat.msg_perm.uid = zval_get_long(item);
		}
		if ((item = crex_hash_str_find(C_ARRVAL_P(data), "msg_perm.gid", sizeof("msg_perm.gid") - 1)) != NULL) {
			stat.msg_perm.gid = zval_get_long(item);
		}
		if ((item = crex_hash_str_find(C_ARRVAL_P(data), "msg_perm.mode", sizeof("msg_perm.mode") - 1)) != NULL) {
			stat.msg_perm.mode = zval_get_long(item);
		}
		if ((item = crex_hash_str_find(C_ARRVAL_P(data), "msg_qbytes", sizeof("msg_qbytes") - 1)) != NULL) {
			stat.msg_qbytes = zval_get_long(item);
		}
		if (msgctl(mq->id, IPC_SET, &stat) == 0) {
			RETVAL_TRUE;
		}
	}
}
/* }}} */

/* {{{ Returns information about a message queue */
CRX_FUNCTION(msg_stat_queue)
{
	zval *queue;
	sysvmsg_queue_t *mq = NULL;
	struct msqid_ds stat;

	RETVAL_FALSE;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &queue, sysvmsg_queue_ce) == FAILURE) {
		RETURN_THROWS();
	}

	mq = C_SYSVMSG_QUEUE_P(queue);

	if (msgctl(mq->id, IPC_STAT, &stat) == 0) {
		array_init(return_value);

		add_assoc_long(return_value, "msg_perm.uid", stat.msg_perm.uid);
		add_assoc_long(return_value, "msg_perm.gid", stat.msg_perm.gid);
		add_assoc_long(return_value, "msg_perm.mode", stat.msg_perm.mode);
		add_assoc_long(return_value, "msg_stime",  stat.msg_stime);
		add_assoc_long(return_value, "msg_rtime",  stat.msg_rtime);
		add_assoc_long(return_value, "msg_ctime",  stat.msg_ctime);
		add_assoc_long(return_value, "msg_qnum",   stat.msg_qnum);
		add_assoc_long(return_value, "msg_qbytes", stat.msg_qbytes);
		add_assoc_long(return_value, "msg_lspid",  stat.msg_lspid);
		add_assoc_long(return_value, "msg_lrpid",  stat.msg_lrpid);
	}
}
/* }}} */

/* {{{ Check whether a message queue exists */
CRX_FUNCTION(msg_queue_exists)
{
	crex_long key;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l", &key) == FAILURE)	{
		RETURN_THROWS();
	}

	if (msgget(key, 0) < 0) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Attach to a message queue */
CRX_FUNCTION(msg_get_queue)
{
	crex_long key;
	crex_long perms = 0666;
	sysvmsg_queue_t *mq;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "l|l", &key, &perms) == FAILURE)	{
		RETURN_THROWS();
	}

	object_init_ex(return_value, sysvmsg_queue_ce);
	mq = C_SYSVMSG_QUEUE_P(return_value);

	mq->key = key;
	mq->id = msgget(key, 0);
	if (mq->id < 0)	{
		/* doesn't already exist; create it */
		mq->id = msgget(key, IPC_CREAT | IPC_EXCL | perms);
		if (mq->id < 0)	{
			crx_error_docref(NULL, E_WARNING, "Failed for key 0x" CREX_XLONG_FMT ": %s", key, strerror(errno));
			zval_ptr_dtor(return_value);
			RETURN_FALSE;
		}
	}
}
/* }}} */

/* {{{ Destroy the queue */
CRX_FUNCTION(msg_remove_queue)
{
	zval *queue;
	sysvmsg_queue_t *mq = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &queue, sysvmsg_queue_ce) == FAILURE) {
		RETURN_THROWS();
	}

	mq = C_SYSVMSG_QUEUE_P(queue);

	if (msgctl(mq->id, IPC_RMID, NULL) == 0) {
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ Send a message of type msgtype (must be > 0) to a message queue */
CRX_FUNCTION(msg_receive)
{
	zval *out_message, *queue, *out_msgtype, *zerrcode = NULL;
	crex_long desiredmsgtype, maxsize, flags = 0;
	crex_long realflags = 0;
	bool do_unserialize = 1;
	sysvmsg_queue_t *mq = NULL;
	struct crx_msgbuf *messagebuffer = NULL; /* buffer to transmit */
	int result;

	RETVAL_FALSE;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olzlz|blz",
				&queue, sysvmsg_queue_ce, &desiredmsgtype, &out_msgtype, &maxsize,
				&out_message, &do_unserialize, &flags, &zerrcode) == FAILURE) {
		RETURN_THROWS();
	}

	if (maxsize <= 0) {
		crex_argument_value_error(4, "must be greater than 0");
		RETURN_THROWS();
	}

	if (flags != 0) {
		if (flags & CRX_MSG_EXCEPT) {
#ifndef MSG_EXCEPT
			crx_error_docref(NULL, E_WARNING, "MSG_EXCEPT is not supported on your system");
			RETURN_FALSE;
#else
			realflags |= MSG_EXCEPT;
#endif
		}
		if (flags & CRX_MSG_NOERROR) {
			realflags |= MSG_NOERROR;
		}
		if (flags & CRX_MSG_IPC_NOWAIT) {
			realflags |= IPC_NOWAIT;
		}
	}

	mq = C_SYSVMSG_QUEUE_P(queue);

	messagebuffer = (struct crx_msgbuf *) safe_emalloc(maxsize, 1, sizeof(struct crx_msgbuf));

	result = msgrcv(mq->id, messagebuffer, maxsize, desiredmsgtype, realflags);

	if (result >= 0) {
		/* got it! */
		CREX_TRY_ASSIGN_REF_LONG(out_msgtype, messagebuffer->mtype);
		if (zerrcode) {
			CREX_TRY_ASSIGN_REF_LONG(zerrcode, 0);
		}

		RETVAL_TRUE;
		if (do_unserialize)	{
			crx_unserialize_data_t var_hash;
			zval tmp;
			const unsigned char *p = (const unsigned char *) messagebuffer->mtext;

			CRX_VAR_UNSERIALIZE_INIT(var_hash);
			if (!crx_var_unserialize(&tmp, &p, p + result, &var_hash)) {
				crx_error_docref(NULL, E_WARNING, "Message corrupted");
				CREX_TRY_ASSIGN_REF_FALSE(out_message);
				RETVAL_FALSE;
			} else {
				CREX_TRY_ASSIGN_REF_TMP(out_message, &tmp);
			}
			CRX_VAR_UNSERIALIZE_DESTROY(var_hash);
		} else {
			CREX_TRY_ASSIGN_REF_STRINGL(out_message, messagebuffer->mtext, result);
		}
	} else {
		CREX_TRY_ASSIGN_REF_LONG(out_msgtype, 0);
		CREX_TRY_ASSIGN_REF_FALSE(out_message);
		if (zerrcode) {
			CREX_TRY_ASSIGN_REF_LONG(zerrcode, errno);
		}
	}
	efree(messagebuffer);
}
/* }}} */

/* {{{ Send a message of type msgtype (must be > 0) to a message queue */
CRX_FUNCTION(msg_send)
{
	zval *message, *queue, *zerror=NULL;
	crex_long msgtype;
	bool do_serialize = 1, blocking = 1;
	sysvmsg_queue_t * mq = NULL;
	struct crx_msgbuf * messagebuffer = NULL; /* buffer to transmit */
	int result;
	size_t message_len = 0;

	RETVAL_FALSE;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olz|bbz",
				&queue, sysvmsg_queue_ce, &msgtype, &message, &do_serialize, &blocking, &zerror) == FAILURE) {
		RETURN_THROWS();
	}

	mq = C_SYSVMSG_QUEUE_P(queue);

	if (do_serialize) {
		smart_str msg_var = {0};
		crx_serialize_data_t var_hash;

		CRX_VAR_SERIALIZE_INIT(var_hash);
		crx_var_serialize(&msg_var, message, &var_hash);
		CRX_VAR_SERIALIZE_DESTROY(var_hash);

		/* NB: crx_msgbuf is 1 char bigger than a long, so there is no need to
		 * allocate the extra byte. */
		messagebuffer = safe_emalloc(ZSTR_LEN(msg_var.s), 1, sizeof(struct crx_msgbuf));
		memcpy(messagebuffer->mtext, ZSTR_VAL(msg_var.s), ZSTR_LEN(msg_var.s) + 1);
		message_len = ZSTR_LEN(msg_var.s);
		smart_str_free(&msg_var);
	} else {
		char *p;
		switch (C_TYPE_P(message)) {
			case IS_STRING:
				p = C_STRVAL_P(message);
				message_len = C_STRLEN_P(message);
				break;
			case IS_LONG:
				message_len = spprintf(&p, 0, CREX_LONG_FMT, C_LVAL_P(message));
				break;
			case IS_FALSE:
				message_len = spprintf(&p, 0, "0");
				break;
			case IS_TRUE:
				message_len = spprintf(&p, 0, "1");
				break;
			case IS_DOUBLE:
				message_len = spprintf(&p, 0, "%F", C_DVAL_P(message));
				break;

			default:
				crex_argument_type_error(3, "must be of type string|int|float|bool, %s given", crex_zval_value_name(message));
				RETURN_THROWS();
		}

		messagebuffer = safe_emalloc(message_len, 1, sizeof(struct crx_msgbuf));
		memcpy(messagebuffer->mtext, p, message_len + 1);

		if (C_TYPE_P(message) != IS_STRING) {
			efree(p);
		}
	}

	/* set the message type */
	messagebuffer->mtype = msgtype;

	result = msgsnd(mq->id, messagebuffer, message_len, blocking ? 0 : IPC_NOWAIT);

	efree(messagebuffer);

	if (result == -1) {
		crx_error_docref(NULL, E_WARNING, "msgsnd failed: %s", strerror(errno));
		if (zerror) {
			CREX_TRY_ASSIGN_REF_LONG(zerror, errno);
		}
	} else {
		RETVAL_TRUE;
	}
}
/* }}} */
