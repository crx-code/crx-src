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
   | Authors: Andrew Skalski <askalski@chek.com>                          |
   |          Stefan Esser <sesser@crx.net> (resume functions)            |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"

#ifdef HAVE_FTP_SSL
# include <openssl/ssl.h>
#endif

#ifdef HAVE_FTP

#include "ext/standard/info.h"
#include "ext/standard/file.h"
#include "Crex/crex_attributes.h"
#include "Crex/crex_exceptions.h"

#include "crx_ftp.h"
#include "ftp.h"
#include "ftp_arginfo.h"

static crex_class_entry *crx_ftp_ce = NULL;
static crex_object_handlers ftp_object_handlers;

crex_module_entry crx_ftp_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	NULL,
	"ftp",
	ext_functions,
	CRX_MINIT(ftp),
	NULL,
	NULL,
	NULL,
	CRX_MINFO(ftp),
	CRX_FTP_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_FTP
CREX_GET_MODULE(crx_ftp)
#endif

typedef struct _crx_ftp_object {
	ftpbuf_t *ftp;
	crex_object std;
} crx_ftp_object;

static inline crex_object *ftp_object_to_crex_object(crx_ftp_object *obj) {
	return ((crex_object*)(obj + 1)) - 1;
}

static inline crx_ftp_object *ftp_object_from_crex_object(crex_object *zobj) {
	return ((crx_ftp_object*)(zobj + 1)) - 1;
}

static crex_object* ftp_object_create(crex_class_entry* ce) {
	crx_ftp_object *obj = crex_object_alloc(sizeof(crx_ftp_object), ce);
	crex_object *zobj = ftp_object_to_crex_object(obj);
	obj->ftp = NULL;
	crex_object_std_init(zobj, ce);
	object_properties_init(zobj, ce);
	zobj->handlers = &ftp_object_handlers;

	return zobj;
}

static crex_function *ftp_object_get_constructor(crex_object *zobj) {
	crex_throw_error(NULL, "Cannot directly construct FTP\\Connection, use ftp_connect() or ftp_ssl_connect() instead");
	return NULL;
}

static void ftp_object_destroy(crex_object *zobj) {
	crx_ftp_object *obj = ftp_object_from_crex_object(zobj);

	if (obj->ftp) {
		ftp_close(obj->ftp);
	}

	crex_object_std_dtor(zobj);
}

CRX_MINIT_FUNCTION(ftp)
{
#ifdef HAVE_FTP_SSL
#if OPENSSL_VERSION_NUMBER < 0x10101000 && !defined(LIBRESSL_VERSION_NUMBER)
	SSL_library_init();
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();
	OpenSSL_add_all_algorithms();

	SSL_load_error_strings();
#endif
#endif

	crx_ftp_ce = register_class_FTP_Connection();
	crx_ftp_ce->create_object = ftp_object_create;

	memcpy(&ftp_object_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	ftp_object_handlers.offset = XtOffsetOf(crx_ftp_object, std);
	ftp_object_handlers.get_constructor = ftp_object_get_constructor;
	ftp_object_handlers.free_obj = ftp_object_destroy;
	ftp_object_handlers.clone_obj = NULL;

	register_ftp_symbols(module_number);

	return SUCCESS;
}

CRX_MINFO_FUNCTION(ftp)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "FTP support", "enabled");
#ifdef HAVE_FTP_SSL
	crx_info_print_table_row(2, "FTPS support", "enabled");
#else
	crx_info_print_table_row(2, "FTPS support", "disabled");
#endif
	crx_info_print_table_end();
}

#define	XTYPE(xtype, mode)	{ \
	if (mode != FTPTYPE_ASCII && mode != FTPTYPE_IMAGE) { \
		crex_argument_value_error(4, "must be either FTP_ASCII or FTP_BINARY"); \
		RETURN_THROWS(); \
	} \
	xtype = mode; \
}


/* {{{ Opens a FTP stream */
CRX_FUNCTION(ftp_connect)
{
	ftpbuf_t	*ftp;
	char		*host;
	size_t		host_len;
	crex_long 		port = 0;
	crex_long		timeout_sec = FTP_DEFAULT_TIMEOUT;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s|ll", &host, &host_len, &port, &timeout_sec) == FAILURE) {
		RETURN_THROWS();
	}

	if (timeout_sec <= 0) {
		crex_argument_value_error(3, "must be greater than 0");
		RETURN_THROWS();
	}

	/* connect */
	if (!(ftp = ftp_open(host, (short)port, timeout_sec))) {
		RETURN_FALSE;
	}

	/* autoseek for resuming */
	ftp->autoseek = FTP_DEFAULT_AUTOSEEK;
	ftp->usepasvaddress = FTP_DEFAULT_USEPASVADDRESS;
#ifdef HAVE_FTP_SSL
	/* disable ssl */
	ftp->use_ssl = 0;
#endif

	object_init_ex(return_value, crx_ftp_ce);
	ftp_object_from_crex_object(C_OBJ_P(return_value))->ftp = ftp;
}
/* }}} */

#ifdef HAVE_FTP_SSL
/* {{{ Opens a FTP-SSL stream */
CRX_FUNCTION(ftp_ssl_connect)
{
	ftpbuf_t	*ftp;
	char		*host;
	size_t		host_len;
	crex_long		port = 0;
	crex_long		timeout_sec = FTP_DEFAULT_TIMEOUT;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s|ll", &host, &host_len, &port, &timeout_sec) == FAILURE) {
		RETURN_THROWS();
	}

	if (timeout_sec <= 0) {
		crex_argument_value_error(3, "must be greater than 0");
		RETURN_THROWS();
	}

	/* connect */
	if (!(ftp = ftp_open(host, (short)port, timeout_sec))) {
		RETURN_FALSE;
	}

	/* autoseek for resuming */
	ftp->autoseek = FTP_DEFAULT_AUTOSEEK;
	ftp->usepasvaddress = FTP_DEFAULT_USEPASVADDRESS;
	/* enable ssl */
	ftp->use_ssl = 1;

	object_init_ex(return_value, crx_ftp_ce);
	ftp_object_from_crex_object(C_OBJ_P(return_value))->ftp = ftp;
}
/* }}} */
#endif

#define GET_FTPBUF(ftpbuf, zftp) \
	ftpbuf = ftp_object_from_crex_object(C_OBJ_P(zftp))->ftp; \
	if (!ftpbuf) { \
		crex_throw_exception(crex_ce_value_error, "FTP\\Connection is already closed", 0); \
		RETURN_THROWS(); \
	}

/* {{{ Logs into the FTP server */
CRX_FUNCTION(ftp_login)
{
	zval 		*z_ftp;
	ftpbuf_t	*ftp;
	char *user, *pass;
	size_t user_len, pass_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oss", &z_ftp, crx_ftp_ce, &user, &user_len, &pass, &pass_len) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	/* log in */
	if (!ftp_login(ftp, user, user_len, pass, pass_len)) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Returns the present working directory */
CRX_FUNCTION(ftp_pwd)
{
	zval 		*z_ftp;
	ftpbuf_t	*ftp;
	const char	*pwd;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &z_ftp, crx_ftp_ce) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	if (!(pwd = ftp_pwd(ftp))) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}

	RETURN_STRING((char*) pwd);
}
/* }}} */

/* {{{ Changes to the parent directory */
CRX_FUNCTION(ftp_cdup)
{
	zval 		*z_ftp;
	ftpbuf_t	*ftp;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &z_ftp, crx_ftp_ce) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	if (!ftp_cdup(ftp)) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Changes directories */
CRX_FUNCTION(ftp_chdir)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	char		*dir;
	size_t			dir_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Os", &z_ftp, crx_ftp_ce, &dir, &dir_len) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	/* change directories */
	if (!ftp_chdir(ftp, dir, dir_len)) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Requests execution of a program on the FTP server */
CRX_FUNCTION(ftp_exec)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	char		*cmd;
	size_t			cmd_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Os", &z_ftp, crx_ftp_ce, &cmd, &cmd_len) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	/* execute serverside command */
	if (!ftp_exec(ftp, cmd, cmd_len)) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Sends a literal command to the FTP server */
CRX_FUNCTION(ftp_raw)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	char		*cmd;
	size_t			cmd_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Os", &z_ftp, crx_ftp_ce, &cmd, &cmd_len) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	/* execute arbitrary ftp command */
	ftp_raw(ftp, cmd, cmd_len, return_value);
}
/* }}} */

/* {{{ Creates a directory and returns the absolute path for the new directory or false on error */
CRX_FUNCTION(ftp_mkdir)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	char		*dir;
	crex_string *tmp;
	size_t		dir_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Os", &z_ftp, crx_ftp_ce, &dir, &dir_len) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	/* create directory */
	if (NULL == (tmp = ftp_mkdir(ftp, dir, dir_len))) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}

	RETURN_STR(tmp);
}
/* }}} */

/* {{{ Removes a directory */
CRX_FUNCTION(ftp_rmdir)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	char		*dir;
	size_t		dir_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Os", &z_ftp, crx_ftp_ce, &dir, &dir_len) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	/* remove directorie */
	if (!ftp_rmdir(ftp, dir, dir_len)) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Sets permissions on a file */
CRX_FUNCTION(ftp_chmod)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	char		*filename;
	size_t		filename_len;
	crex_long		mode;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olp", &z_ftp, crx_ftp_ce, &mode, &filename, &filename_len) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	if (!ftp_chmod(ftp, mode, filename, filename_len)) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}

	RETURN_LONG(mode);
}
/* }}} */

/* {{{ Attempt to allocate space on the remote FTP server */
CRX_FUNCTION(ftp_alloc)
{
	zval		*z_ftp, *zresponse = NULL;
	ftpbuf_t	*ftp;
	crex_long		size, ret;
	crex_string	*response = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ol|z", &z_ftp, crx_ftp_ce, &size, &zresponse) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	ret = ftp_alloc(ftp, size, zresponse ? &response : NULL);

	if (response) {
		CREX_TRY_ASSIGN_REF_STR(zresponse, response);
	}

	if (!ret) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Returns an array of filenames in the given directory */
CRX_FUNCTION(ftp_nlist)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	char		**nlist, **ptr, *dir;
	size_t		dir_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Op", &z_ftp, crx_ftp_ce, &dir, &dir_len) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	/* get list of files */
	if (NULL == (nlist = ftp_nlist(ftp, dir, dir_len))) {
		RETURN_FALSE;
	}

	array_init(return_value);
	for (ptr = nlist; *ptr; ptr++) {
		add_next_index_string(return_value, *ptr);
	}
	efree(nlist);
}
/* }}} */

/* {{{ Returns a detailed listing of a directory as an array of output lines */
CRX_FUNCTION(ftp_rawlist)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	char		**llist, **ptr, *dir;
	size_t		dir_len;
	bool	recursive = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Os|b", &z_ftp, crx_ftp_ce, &dir, &dir_len, &recursive) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	/* get raw directory listing */
	if (NULL == (llist = ftp_list(ftp, dir, dir_len, recursive))) {
		RETURN_FALSE;
	}

	array_init(return_value);
	for (ptr = llist; *ptr; ptr++) {
		add_next_index_string(return_value, *ptr);
	}
	efree(llist);
}
/* }}} */

/* {{{ Returns a detailed listing of a directory as an array of parsed output lines */
CRX_FUNCTION(ftp_mlsd)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	char		**llist, **ptr, *dir;
	size_t		dir_len;
	zval		entry;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Os", &z_ftp, crx_ftp_ce, &dir, &dir_len) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	/* get raw directory listing */
	if (NULL == (llist = ftp_mlsd(ftp, dir, dir_len))) {
		RETURN_FALSE;
	}

	array_init(return_value);
	for (ptr = llist; *ptr; ptr++) {
		array_init(&entry);
		if (ftp_mlsd_parse_line(C_ARRVAL_P(&entry), *ptr) == SUCCESS) {
			crex_hash_next_index_insert(C_ARRVAL_P(return_value), &entry);
		} else {
			zval_ptr_dtor(&entry);
		}
	}

	efree(llist);
}
/* }}} */

/* {{{ Returns the system type identifier */
CRX_FUNCTION(ftp_systype)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	const char	*syst;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &z_ftp, crx_ftp_ce) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	if (NULL == (syst = ftp_syst(ftp))) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}

	RETURN_STRING((char*) syst);
}
/* }}} */

/* {{{ Retrieves a file from the FTP server and writes it to an open file */
CRX_FUNCTION(ftp_fget)
{
	zval		*z_ftp, *z_file;
	ftpbuf_t	*ftp;
	ftptype_t	xtype;
	crx_stream	*stream;
	char		*file;
	size_t		file_len;
	crex_long		mode=FTPTYPE_IMAGE, resumepos=0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ors|ll", &z_ftp, crx_ftp_ce, &z_file, &file, &file_len, &mode, &resumepos) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);
	crx_stream_from_res(stream, C_RES_P(z_file));
	XTYPE(xtype, mode);

	/* ignore autoresume if autoseek is switched off */
	if (!ftp->autoseek && resumepos == CRX_FTP_AUTORESUME) {
		resumepos = 0;
	}

	if (ftp->autoseek && resumepos) {
		/* if autoresume is wanted seek to end */
		if (resumepos == CRX_FTP_AUTORESUME) {
			crx_stream_seek(stream, 0, SEEK_END);
			resumepos = crx_stream_tell(stream);
		} else {
			crx_stream_seek(stream, resumepos, SEEK_SET);
		}
	}

	if (!ftp_get(ftp, stream, file, file_len, xtype, resumepos)) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Retrieves a file from the FTP server asynchronly and writes it to an open file */
CRX_FUNCTION(ftp_nb_fget)
{
	zval		*z_ftp, *z_file;
	ftpbuf_t	*ftp;
	ftptype_t	xtype;
	crx_stream	*stream;
	char		*file;
	size_t		file_len;
	crex_long		mode=FTPTYPE_IMAGE, resumepos=0, ret;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ors|ll", &z_ftp, crx_ftp_ce, &z_file, &file, &file_len, &mode, &resumepos) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);
	crx_stream_from_res(stream, C_RES_P(z_file));
	XTYPE(xtype, mode);

	/* ignore autoresume if autoseek is switched off */
	if (!ftp->autoseek && resumepos == CRX_FTP_AUTORESUME) {
		resumepos = 0;
	}

	if (ftp->autoseek && resumepos) {
		/* if autoresume is wanted seek to end */
		if (resumepos == CRX_FTP_AUTORESUME) {
			crx_stream_seek(stream, 0, SEEK_END);
			resumepos = crx_stream_tell(stream);
		} else {
			crx_stream_seek(stream, resumepos, SEEK_SET);
		}
	}

	/* configuration */
	ftp->direction = 0;   /* recv */
	ftp->closestream = 0; /* do not close */

	if ((ret = ftp_nb_get(ftp, stream, file, file_len, xtype, resumepos)) == CRX_FTP_FAILED) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_LONG(ret);
	}

	RETURN_LONG(ret);
}
/* }}} */

/* {{{ Turns passive mode on or off */
CRX_FUNCTION(ftp_pasv)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	bool	pasv;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ob", &z_ftp, crx_ftp_ce, &pasv) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	if (!ftp_pasv(ftp, pasv ? 1 : 0)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Retrieves a file from the FTP server and writes it to a local file */
CRX_FUNCTION(ftp_get)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	ftptype_t	xtype;
	crx_stream	*outstream;
	char		*local, *remote;
	size_t		local_len, remote_len;
	crex_long		mode=FTPTYPE_IMAGE, resumepos=0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Opp|ll", &z_ftp, crx_ftp_ce, &local, &local_len, &remote, &remote_len, &mode, &resumepos) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);
	XTYPE(xtype, mode);

	/* ignore autoresume if autoseek is switched off */
	if (!ftp->autoseek && resumepos == CRX_FTP_AUTORESUME) {
		resumepos = 0;
	}

#ifdef CRX_WIN32
	mode = FTPTYPE_IMAGE;
#endif

	if (ftp->autoseek && resumepos) {
		outstream = crx_stream_open_wrapper(local, mode == FTPTYPE_ASCII ? "rt+" : "rb+", REPORT_ERRORS, NULL);
		if (outstream == NULL) {
			outstream = crx_stream_open_wrapper(local, mode == FTPTYPE_ASCII ? "wt" : "wb", REPORT_ERRORS, NULL);
		}
		if (outstream != NULL) {
			/* if autoresume is wanted seek to end */
			if (resumepos == CRX_FTP_AUTORESUME) {
				crx_stream_seek(outstream, 0, SEEK_END);
				resumepos = crx_stream_tell(outstream);
			} else {
				crx_stream_seek(outstream, resumepos, SEEK_SET);
			}
		}
	} else {
		outstream = crx_stream_open_wrapper(local, mode == FTPTYPE_ASCII ? "wt" : "wb", REPORT_ERRORS, NULL);
	}

	if (outstream == NULL)	{
		crx_error_docref(NULL, E_WARNING, "Error opening %s", local);
		RETURN_FALSE;
	}

	if (!ftp_get(ftp, outstream, remote, remote_len, xtype, resumepos)) {
		crx_stream_close(outstream);
		VCWD_UNLINK(local);
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}

	crx_stream_close(outstream);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Retrieves a file from the FTP server nbhronly and writes it to a local file */
CRX_FUNCTION(ftp_nb_get)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	ftptype_t	xtype;
	crx_stream	*outstream;
	char		*local, *remote;
	size_t		local_len, remote_len;
	int ret;
	crex_long		mode=FTPTYPE_IMAGE, resumepos=0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oss|ll", &z_ftp, crx_ftp_ce, &local, &local_len, &remote, &remote_len, &mode, &resumepos) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);
	XTYPE(xtype, mode);

	/* ignore autoresume if autoseek is switched off */
	if (!ftp->autoseek && resumepos == CRX_FTP_AUTORESUME) {
		resumepos = 0;
	}
#ifdef CRX_WIN32
	mode = FTPTYPE_IMAGE;
#endif
	if (ftp->autoseek && resumepos) {
		outstream = crx_stream_open_wrapper(local, mode == FTPTYPE_ASCII ? "rt+" : "rb+", REPORT_ERRORS, NULL);
		if (outstream == NULL) {
			outstream = crx_stream_open_wrapper(local, mode == FTPTYPE_ASCII ? "wt" : "wb", REPORT_ERRORS, NULL);
		}
		if (outstream != NULL) {
			/* if autoresume is wanted seek to end */
			if (resumepos == CRX_FTP_AUTORESUME) {
				crx_stream_seek(outstream, 0, SEEK_END);
				resumepos = crx_stream_tell(outstream);
			} else {
				crx_stream_seek(outstream, resumepos, SEEK_SET);
			}
		}
	} else {
		outstream = crx_stream_open_wrapper(local, mode == FTPTYPE_ASCII ? "wt" : "wb", REPORT_ERRORS, NULL);
	}

	if (outstream == NULL)	{
		crx_error_docref(NULL, E_WARNING, "Error opening %s", local);
		RETURN_FALSE;
	}

	/* configuration */
	ftp->direction = 0;   /* recv */
	ftp->closestream = 1; /* do close */

	if ((ret = ftp_nb_get(ftp, outstream, remote, remote_len, xtype, resumepos)) == CRX_FTP_FAILED) {
		crx_stream_close(outstream);
		ftp->stream = NULL;
		VCWD_UNLINK(local);
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_LONG(CRX_FTP_FAILED);
	}

	if (ret == CRX_FTP_FINISHED){
		crx_stream_close(outstream);
		ftp->stream = NULL;
	}

	RETURN_LONG(ret);
}
/* }}} */

/* {{{ Continues retrieving/sending a file nbronously */
CRX_FUNCTION(ftp_nb_continue)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	crex_long		ret;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &z_ftp, crx_ftp_ce) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	if (!ftp->nb) {
		crx_error_docref(NULL, E_WARNING, "No nbronous transfer to continue");
		RETURN_LONG(CRX_FTP_FAILED);
	}

	if (ftp->direction) {
		ret=ftp_nb_continue_write(ftp);
	} else {
		ret=ftp_nb_continue_read(ftp);
	}

	if (ret != CRX_FTP_MOREDATA && ftp->closestream) {
		crx_stream_close(ftp->stream);
		ftp->stream = NULL;
	}

	if (ret == CRX_FTP_FAILED) {
		crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
	}

	RETURN_LONG(ret);
}
/* }}} */

/* {{{ Stores a file from an open file to the FTP server */
CRX_FUNCTION(ftp_fput)
{
	zval		*z_ftp, *z_file;
	ftpbuf_t	*ftp;
	ftptype_t	xtype;
	size_t		remote_len;
	crex_long		mode=FTPTYPE_IMAGE, startpos=0;
	crx_stream	*stream;
	char		*remote;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Osr|ll", &z_ftp, crx_ftp_ce, &remote, &remote_len, &z_file, &mode, &startpos) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);
	crx_stream_from_zval(stream, z_file);
	XTYPE(xtype, mode);

	/* ignore autoresume if autoseek is switched off */
	if (!ftp->autoseek && startpos == CRX_FTP_AUTORESUME) {
		startpos = 0;
	}

	if (ftp->autoseek && startpos) {
		/* if autoresume is wanted ask for remote size */
		if (startpos == CRX_FTP_AUTORESUME) {
			startpos = ftp_size(ftp, remote, remote_len);
			if (startpos < 0) {
				startpos = 0;
			}
		}
		if (startpos) {
			crx_stream_seek(stream, startpos, SEEK_SET);
		}
	}

	if (!ftp_put(ftp, remote, remote_len, stream, xtype, startpos)) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Stores a file from an open file to the FTP server nbronly */
CRX_FUNCTION(ftp_nb_fput)
{
	zval		*z_ftp, *z_file;
	ftpbuf_t	*ftp;
	ftptype_t	xtype;
	size_t		remote_len;
	int             ret;
	crex_long	mode=FTPTYPE_IMAGE, startpos=0;
	crx_stream	*stream;
	char		*remote;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Osr|ll", &z_ftp, crx_ftp_ce, &remote, &remote_len, &z_file, &mode, &startpos) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);
	crx_stream_from_res(stream, C_RES_P(z_file));
	XTYPE(xtype, mode);

	/* ignore autoresume if autoseek is switched off */
	if (!ftp->autoseek && startpos == CRX_FTP_AUTORESUME) {
		startpos = 0;
	}

	if (ftp->autoseek && startpos) {
		/* if autoresume is wanted ask for remote size */
		if (startpos == CRX_FTP_AUTORESUME) {
			startpos = ftp_size(ftp, remote, remote_len);
			if (startpos < 0) {
				startpos = 0;
			}
		}
		if (startpos) {
			crx_stream_seek(stream, startpos, SEEK_SET);
		}
	}

	/* configuration */
	ftp->direction = 1;   /* send */
	ftp->closestream = 0; /* do not close */

	if (((ret = ftp_nb_put(ftp, remote, remote_len, stream, xtype, startpos)) == CRX_FTP_FAILED)) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_LONG(ret);
	}

	RETURN_LONG(ret);
}
/* }}} */


/* {{{ Stores a file on the FTP server */
CRX_FUNCTION(ftp_put)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	ftptype_t	xtype;
	char		*remote, *local;
	size_t		remote_len, local_len;
	crex_long		mode=FTPTYPE_IMAGE, startpos=0;
	crx_stream 	*instream;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Opp|ll", &z_ftp, crx_ftp_ce, &remote, &remote_len, &local, &local_len, &mode, &startpos) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);
	XTYPE(xtype, mode);

	if (!(instream = crx_stream_open_wrapper(local, mode == FTPTYPE_ASCII ? "rt" : "rb", REPORT_ERRORS, NULL))) {
		RETURN_FALSE;
	}

	/* ignore autoresume if autoseek is switched off */
	if (!ftp->autoseek && startpos == CRX_FTP_AUTORESUME) {
		startpos = 0;
	}

	if (ftp->autoseek && startpos) {
		/* if autoresume is wanted ask for remote size */
		if (startpos == CRX_FTP_AUTORESUME) {
			startpos = ftp_size(ftp, remote, remote_len);
			if (startpos < 0) {
				startpos = 0;
			}
		}
		if (startpos) {
			crx_stream_seek(instream, startpos, SEEK_SET);
		}
	}

	if (!ftp_put(ftp, remote, remote_len, instream, xtype, startpos)) {
		crx_stream_close(instream);
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}
	crx_stream_close(instream);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Append content of a file a another file on the FTP server */
CRX_FUNCTION(ftp_append)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	ftptype_t	xtype;
	char		*remote, *local;
	size_t		remote_len, local_len;
	crex_long		mode=FTPTYPE_IMAGE;
	crx_stream 	*instream;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Opp|l", &z_ftp, crx_ftp_ce, &remote, &remote_len, &local, &local_len, &mode) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);
	XTYPE(xtype, mode);

	if (!(instream = crx_stream_open_wrapper(local, mode == FTPTYPE_ASCII ? "rt" : "rb", REPORT_ERRORS, NULL))) {
		RETURN_FALSE;
	}

	if (!ftp_append(ftp, remote, remote_len, instream, xtype)) {
		crx_stream_close(instream);
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}
	crx_stream_close(instream);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Stores a file on the FTP server */
CRX_FUNCTION(ftp_nb_put)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	ftptype_t	xtype;
	char		*remote, *local;
	size_t		remote_len, local_len;
	crex_long		mode=FTPTYPE_IMAGE, startpos=0, ret;
	crx_stream 	*instream;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Opp|ll", &z_ftp, crx_ftp_ce, &remote, &remote_len, &local, &local_len, &mode, &startpos) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);
	XTYPE(xtype, mode);

	if (!(instream = crx_stream_open_wrapper(local, mode == FTPTYPE_ASCII ? "rt" : "rb", REPORT_ERRORS, NULL))) {
		RETURN_FALSE;
	}

	/* ignore autoresume if autoseek is switched off */
	if (!ftp->autoseek && startpos == CRX_FTP_AUTORESUME) {
		startpos = 0;
	}

	if (ftp->autoseek && startpos) {
		/* if autoresume is wanted ask for remote size */
		if (startpos == CRX_FTP_AUTORESUME) {
			startpos = ftp_size(ftp, remote, remote_len);
			if (startpos < 0) {
				startpos = 0;
			}
		}
		if (startpos) {
			crx_stream_seek(instream, startpos, SEEK_SET);
		}
	}

	/* configuration */
	ftp->direction = 1;   /* send */
	ftp->closestream = 1; /* do close */

	ret = ftp_nb_put(ftp, remote, remote_len, instream, xtype, startpos);

	if (ret != CRX_FTP_MOREDATA) {
		crx_stream_close(instream);
		ftp->stream = NULL;
	}

	if (ret == CRX_FTP_FAILED) {
		crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
	}

	RETURN_LONG(ret);
}
/* }}} */

/* {{{ Returns the size of the file, or -1 on error */
CRX_FUNCTION(ftp_size)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	char		*file;
	size_t		file_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Op", &z_ftp, crx_ftp_ce, &file, &file_len) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	/* get file size */
	RETURN_LONG(ftp_size(ftp, file, file_len));
}
/* }}} */

/* {{{ Returns the last modification time of the file, or -1 on error */
CRX_FUNCTION(ftp_mdtm)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	char		*file;
	size_t		file_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Op", &z_ftp, crx_ftp_ce, &file, &file_len) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	/* get file mod time */
	RETURN_LONG(ftp_mdtm(ftp, file, file_len));
}
/* }}} */

/* {{{ Renames the given file to a new path */
CRX_FUNCTION(ftp_rename)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	char		*src, *dest;
	size_t		src_len, dest_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Oss", &z_ftp, crx_ftp_ce, &src, &src_len, &dest, &dest_len) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	/* rename the file */
	if (!ftp_rename(ftp, src, src_len, dest, dest_len)) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Deletes a file */
CRX_FUNCTION(ftp_delete)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	char		*file;
	size_t		file_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Os", &z_ftp, crx_ftp_ce, &file, &file_len) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	/* delete the file */
	if (!ftp_delete(ftp, file, file_len)) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Sends a SITE command to the server */
CRX_FUNCTION(ftp_site)
{
	zval		*z_ftp;
	ftpbuf_t	*ftp;
	char		*cmd;
	size_t		cmd_len;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Os", &z_ftp, crx_ftp_ce, &cmd, &cmd_len) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	/* send the site command */
	if (!ftp_site(ftp, cmd, cmd_len)) {
		if (*ftp->inbuf) {
			crx_error_docref(NULL, E_WARNING, "%s", ftp->inbuf);
		}
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Closes the FTP stream */
CRX_FUNCTION(ftp_close)
{
	zval		*z_ftp;
	crx_ftp_object *obj;
	bool success = true;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &z_ftp, crx_ftp_ce) == FAILURE) {
		RETURN_THROWS();
	}

	obj = ftp_object_from_crex_object(C_OBJ_P(z_ftp));
	if (obj->ftp) {
		success = ftp_quit(obj->ftp);
		ftp_close(obj->ftp);
		obj->ftp = NULL;
	}

	RETURN_BOOL(success);
}
/* }}} */

/* {{{ Sets an FTP option */
CRX_FUNCTION(ftp_set_option)
{
	zval		*z_ftp, *z_value;
	crex_long		option;
	ftpbuf_t	*ftp;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Olz", &z_ftp, crx_ftp_ce, &option, &z_value) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	switch (option) {
		case CRX_FTP_OPT_TIMEOUT_SEC:
			if (C_TYPE_P(z_value) != IS_LONG) {
				crex_argument_type_error(3, "must be of type int for the FTP_TIMEOUT_SEC option, %s given", crex_zval_value_name(z_value));
				RETURN_THROWS();
			}
			if (C_LVAL_P(z_value) <= 0) {
				crex_argument_value_error(3, "must be greater than 0 for the FTP_TIMEOUT_SEC option");
				RETURN_THROWS();
			}
			ftp->timeout_sec = C_LVAL_P(z_value);
			RETURN_TRUE;
			break;
		case CRX_FTP_OPT_AUTOSEEK:
			if (C_TYPE_P(z_value) != IS_TRUE && C_TYPE_P(z_value) != IS_FALSE) {
				crex_argument_type_error(3, "must be of type bool for the FTP_AUTOSEEK option, %s given", crex_zval_value_name(z_value));
				RETURN_THROWS();
			}
			ftp->autoseek = C_TYPE_P(z_value) == IS_TRUE ? 1 : 0;
			RETURN_TRUE;
			break;
		case CRX_FTP_OPT_USEPASVADDRESS:
			if (C_TYPE_P(z_value) != IS_TRUE && C_TYPE_P(z_value) != IS_FALSE) {
				crex_argument_type_error(3, "must be of type bool for the FTP_USEPASVADDRESS option, %s given", crex_zval_value_name(z_value));
				RETURN_THROWS();
			}
			ftp->usepasvaddress = C_TYPE_P(z_value) == IS_TRUE ? 1 : 0;
			RETURN_TRUE;
			break;
		default:
			crex_argument_value_error(2, "must be one of FTP_TIMEOUT_SEC, FTP_AUTOSEEK, or FTP_USEPASVADDRESS");
			RETURN_THROWS();
			break;
	}
}
/* }}} */

/* {{{ Gets an FTP option */
CRX_FUNCTION(ftp_get_option)
{
	zval		*z_ftp;
	crex_long		option;
	ftpbuf_t	*ftp;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ol", &z_ftp, crx_ftp_ce, &option) == FAILURE) {
		RETURN_THROWS();
	}
	GET_FTPBUF(ftp, z_ftp);

	switch (option) {
		case CRX_FTP_OPT_TIMEOUT_SEC:
			RETURN_LONG(ftp->timeout_sec);
			break;
		case CRX_FTP_OPT_AUTOSEEK:
			RETURN_BOOL(ftp->autoseek);
			break;
		case CRX_FTP_OPT_USEPASVADDRESS:
			RETURN_BOOL(ftp->usepasvaddress);
			break;
		default:
			crex_argument_value_error(2, "must be one of FTP_TIMEOUT_SEC, FTP_AUTOSEEK, or FTP_USEPASVADDRESS");
			RETURN_THROWS();
	}
}
/* }}} */

#endif /* HAVE_FTP */
