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
   | Authors: Rasmus Lerdorf <rasmus@crx.net>                             |
   |          Jim Winstead <jimw@crx.net>                                 |
   |          Hartmut Holzgraefe <hholzgra@crx.net>                       |
   |          Sara Golemon <pollita@crx.net>                              |
   +----------------------------------------------------------------------+
 */

#include "crx.h"
#include "crx_globals.h"
#include "crx_network.h"
#include "crx_ini.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef CRX_WIN32
#include <winsock2.h>
#define O_RDONLY _O_RDONLY
#include "win32/param.h"
#else
#include <sys/param.h>
#endif

#include "crx_standard.h"

#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef CRX_WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <netdb.h>
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#endif

#if defined(CRX_WIN32) || defined(__riscos__)
#undef AF_UNIX
#endif

#if defined(AF_UNIX)
#include <sys/un.h>
#endif

#include "crx_fopen_wrappers.h"

#define FTPS_ENCRYPT_DATA 1
#define GET_FTP_RESULT(stream)	get_ftp_result((stream), tmp_line, sizeof(tmp_line))

typedef struct _crx_ftp_dirstream_data {
	crx_stream *datastream;
	crx_stream *controlstream;
	crx_stream *dirstream;
} crx_ftp_dirstream_data;

/* {{{ get_ftp_result */
static inline int get_ftp_result(crx_stream *stream, char *buffer, size_t buffer_size)
{
	buffer[0] = '\0'; /* in case read fails to read anything */
	while (crx_stream_gets(stream, buffer, buffer_size-1) &&
		   !(isdigit((int) buffer[0]) && isdigit((int) buffer[1]) &&
			 isdigit((int) buffer[2]) && buffer[3] == ' '));
	return strtol(buffer, NULL, 10);
}
/* }}} */

/* {{{ crx_stream_ftp_stream_stat */
static int crx_stream_ftp_stream_stat(crx_stream_wrapper *wrapper, crx_stream *stream, crx_stream_statbuf *ssb)
{
	/* For now, we return with a failure code to prevent the underlying
	 * file's details from being used instead. */
	return -1;
}
/* }}} */

/* {{{ crx_stream_ftp_stream_close */
static int crx_stream_ftp_stream_close(crx_stream_wrapper *wrapper, crx_stream *stream)
{
	crx_stream *controlstream = stream->wrapperthis;
	int ret = 0;

	if (controlstream) {
		if (strpbrk(stream->mode, "wa+")) {
			char tmp_line[512];
			int result;

			/* For write modes close data stream first to signal EOF to server */
			result = GET_FTP_RESULT(controlstream);
			if (result != 226 && result != 250) {
				crx_error_docref(NULL, E_WARNING, "FTP server error %d:%s", result, tmp_line);
				ret = EOF;
			}
		}

		crx_stream_write_string(controlstream, "QUIT\r\n");
		crx_stream_close(controlstream);
		stream->wrapperthis = NULL;
	}

	return ret;
}
/* }}} */

/* {{{ crx_ftp_fopen_connect */
static crx_stream *crx_ftp_fopen_connect(crx_stream_wrapper *wrapper, const char *path, const char *mode, int options,
										 crex_string **opened_path, crx_stream_context *context, crx_stream **preuseid,
										 crx_url **presource, int *puse_ssl, int *puse_ssl_on_data)
{
	crx_stream *stream = NULL, *reuseid = NULL;
	crx_url *resource = NULL;
	int result, use_ssl, use_ssl_on_data = 0;
	char tmp_line[512];
	char *transport;
	int transport_len;

	resource = crx_url_parse(path);
	if (resource == NULL || resource->path == NULL) {
		if (resource && presource) {
			*presource = resource;
		}
		return NULL;
	}

	use_ssl = resource->scheme && (ZSTR_LEN(resource->scheme) > 3) && ZSTR_VAL(resource->scheme)[3] == 's';

	/* use port 21 if one wasn't specified */
	if (resource->port == 0)
		resource->port = 21;

	transport_len = (int)spprintf(&transport, 0, "tcp://%s:%d", ZSTR_VAL(resource->host), resource->port);
	stream = crx_stream_xport_create(transport, transport_len, REPORT_ERRORS, STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT, NULL, NULL, context, NULL, NULL);
	efree(transport);
	if (stream == NULL) {
		result = 0; /* silence */
		goto connect_errexit;
	}

	crx_stream_context_set(stream, context);
	crx_stream_notify_info(context, CRX_STREAM_NOTIFY_CONNECT, NULL, 0);

	/* Start talking to ftp server */
	result = GET_FTP_RESULT(stream);
	if (result > 299 || result < 200) {
		crx_stream_notify_error(context, CRX_STREAM_NOTIFY_FAILURE, tmp_line, result);
		goto connect_errexit;
	}

	if (use_ssl)	{

		/* send the AUTH TLS request name */
		crx_stream_write_string(stream, "AUTH TLS\r\n");

		/* get the response */
		result = GET_FTP_RESULT(stream);
		if (result != 234) {
			/* AUTH TLS not supported try AUTH SSL */
			crx_stream_write_string(stream, "AUTH SSL\r\n");

			/* get the response */
			result = GET_FTP_RESULT(stream);
			if (result != 334) {
				crx_stream_wrapper_log_error(wrapper, options, "Server doesn't support FTPS.");
				goto connect_errexit;
			} else {
				/* we must reuse the old SSL session id */
				/* if we talk to an old ftpd-ssl */
				reuseid = stream;
			}
		} else {
			/* encrypt data etc */


		}

	}

	if (use_ssl) {
		if (crx_stream_xport_crypto_setup(stream,
				STREAM_CRYPTO_METHOD_SSLv23_CLIENT, NULL) < 0
				|| crx_stream_xport_crypto_enable(stream, 1) < 0) {
			crx_stream_wrapper_log_error(wrapper, options, "Unable to activate SSL mode");
			crx_stream_close(stream);
			stream = NULL;
			goto connect_errexit;
		}

		/* set PBSZ to 0 */
		crx_stream_write_string(stream, "PBSZ 0\r\n");

		/* ignore the response */
		result = GET_FTP_RESULT(stream);

		/* set data connection protection level */
#if FTPS_ENCRYPT_DATA
		crx_stream_write_string(stream, "PROT P\r\n");

		/* get the response */
		result = GET_FTP_RESULT(stream);
		use_ssl_on_data = (result >= 200 && result<=299) || reuseid;
#else
		crx_stream_write_string(stream, "PROT C\r\n");

		/* get the response */
		result = GET_FTP_RESULT(stream);
#endif
	}

#define CRX_FTP_CNTRL_CHK(val, val_len, err_msg) {	\
	unsigned char *s = (unsigned char *) val, *e = (unsigned char *) s + val_len;	\
	while (s < e) {	\
		if (iscntrl(*s)) {	\
			crx_stream_wrapper_log_error(wrapper, options, err_msg, val);	\
			goto connect_errexit;	\
		}	\
		s++;	\
	}	\
}

	/* send the user name */
	if (resource->user != NULL) {
		ZSTR_LEN(resource->user) = crx_raw_url_decode(ZSTR_VAL(resource->user), ZSTR_LEN(resource->user));

		CRX_FTP_CNTRL_CHK(ZSTR_VAL(resource->user), ZSTR_LEN(resource->user), "Invalid login %s")

		crx_stream_printf(stream, "USER %s\r\n", ZSTR_VAL(resource->user));
	} else {
		crx_stream_write_string(stream, "USER anonymous\r\n");
	}

	/* get the response */
	result = GET_FTP_RESULT(stream);

	/* if a password is required, send it */
	if (result >= 300 && result <= 399) {
		crx_stream_notify_info(context, CRX_STREAM_NOTIFY_AUTH_REQUIRED, tmp_line, 0);

		if (resource->pass != NULL) {
			ZSTR_LEN(resource->pass) = crx_raw_url_decode(ZSTR_VAL(resource->pass), ZSTR_LEN(resource->pass));

			CRX_FTP_CNTRL_CHK(ZSTR_VAL(resource->pass), ZSTR_LEN(resource->pass), "Invalid password %s")

			crx_stream_printf(stream, "PASS %s\r\n", ZSTR_VAL(resource->pass));
		} else {
			/* if the user has configured who they are,
			   send that as the password */
			if (FG(from_address)) {
				crx_stream_printf(stream, "PASS %s\r\n", FG(from_address));
			} else {
				crx_stream_write_string(stream, "PASS anonymous\r\n");
			}
		}

		/* read the response */
		result = GET_FTP_RESULT(stream);

		if (result > 299 || result < 200) {
			crx_stream_notify_error(context, CRX_STREAM_NOTIFY_AUTH_RESULT, tmp_line, result);
		} else {
			crx_stream_notify_info(context, CRX_STREAM_NOTIFY_AUTH_RESULT, tmp_line, result);
		}
	}
	if (result > 299 || result < 200) {
		goto connect_errexit;
	}

	if (puse_ssl) {
		*puse_ssl = use_ssl;
	}
	if (puse_ssl_on_data) {
		*puse_ssl_on_data = use_ssl_on_data;
	}
	if (preuseid) {
		*preuseid = reuseid;
	}
	if (presource) {
		*presource = resource;
	}

	return stream;

connect_errexit:
	crx_url_free(resource);

	if (stream) {
		crx_stream_close(stream);
	}

	return NULL;
}
/* }}} */

/* {{{ crx_fopen_do_pasv */
static unsigned short crx_fopen_do_pasv(crx_stream *stream, char *ip, size_t ip_size, char **phoststart)
{
	char tmp_line[512];
	int result, i;
	unsigned short portno;
	char *tpath, *ttpath, *hoststart=NULL;

#ifdef HAVE_IPV6
	/* We try EPSV first, needed for IPv6 and works on some IPv4 servers */
	crx_stream_write_string(stream, "EPSV\r\n");
	result = GET_FTP_RESULT(stream);

	/* check if we got a 229 response */
	if (result != 229) {
#endif
		/* EPSV failed, let's try PASV */
		crx_stream_write_string(stream, "PASV\r\n");
		result = GET_FTP_RESULT(stream);

		/* make sure we got a 227 response */
		if (result != 227) {
			return 0;
		}

		/* parse pasv command (129, 80, 95, 25, 13, 221) */
		tpath = tmp_line;
		/* skip over the "227 Some message " part */
		for (tpath += 4; *tpath && !isdigit((int) *tpath); tpath++);
		if (!*tpath) {
			return 0;
		}
		/* skip over the host ip, to get the port */
		hoststart = tpath;
		for (i = 0; i < 4; i++) {
			for (; isdigit((int) *tpath); tpath++);
			if (*tpath != ',') {
				return 0;
			}
			*tpath='.';
			tpath++;
		}
		tpath[-1] = '\0';
		memcpy(ip, hoststart, ip_size);
		ip[ip_size-1] = '\0';
		hoststart = ip;

		/* pull out the MSB of the port */
		portno = (unsigned short) strtoul(tpath, &ttpath, 10) * 256;
		if (ttpath == NULL) {
			/* didn't get correct response from PASV */
			return 0;
		}
		tpath = ttpath;
		if (*tpath != ',') {
			return 0;
		}
		tpath++;
		/* pull out the LSB of the port */
		portno += (unsigned short) strtoul(tpath, &ttpath, 10);
#ifdef HAVE_IPV6
	} else {
		/* parse epsv command (|||6446|) */
		for (i = 0, tpath = tmp_line + 4; *tpath; tpath++) {
			if (*tpath == '|') {
				i++;
				if (i == 3)
					break;
			}
		}
		if (i < 3) {
			return 0;
		}
		/* pull out the port */
		portno = (unsigned short) strtoul(tpath + 1, &ttpath, 10);
	}
#endif
	if (ttpath == NULL) {
		/* didn't get correct response from EPSV/PASV */
		return 0;
	}

	if (phoststart) {
		*phoststart = hoststart;
	}

	return portno;
}
/* }}} */

/* {{{ crx_fopen_url_wrap_ftp */
crx_stream * crx_stream_url_wrap_ftp(crx_stream_wrapper *wrapper, const char *path, const char *mode,
									 int options, crex_string **opened_path, crx_stream_context *context STREAMS_DC)
{
	crx_stream *stream = NULL, *datastream = NULL;
	crx_url *resource = NULL;
	char tmp_line[512];
	char ip[sizeof("123.123.123.123")];
	unsigned short portno;
	char *hoststart = NULL;
	int result = 0, use_ssl, use_ssl_on_data=0;
	crx_stream *reuseid=NULL;
	size_t file_size = 0;
	zval *tmpzval;
	bool allow_overwrite = 0;
	int8_t read_write = 0;
	char *transport;
	int transport_len;
	crex_string *error_message = NULL;

	tmp_line[0] = '\0';

	if (strpbrk(mode, "r+")) {
		read_write = 1; /* Open for reading */
	}
	if (strpbrk(mode, "wa+")) {
		if (read_write) {
			crx_stream_wrapper_log_error(wrapper, options, "FTP does not support simultaneous read/write connections");
			return NULL;
		}
		if (strchr(mode, 'a')) {
			read_write = 3; /* Open for Appending */
		} else {
			read_write = 2; /* Open for writing */
		}
	}
	if (!read_write) {
		/* No mode specified? */
		crx_stream_wrapper_log_error(wrapper, options, "Unknown file open mode");
		return NULL;
	}

	if (context &&
		(tmpzval = crx_stream_context_get_option(context, "ftp", "proxy")) != NULL) {
		if (read_write == 1) {
			/* Use http wrapper to proxy ftp request */
			return crx_stream_url_wrap_http(wrapper, path, mode, options, opened_path, context STREAMS_CC);
		} else {
			/* ftp proxy is read-only */
			crx_stream_wrapper_log_error(wrapper, options, "FTP proxy may only be used in read mode");
			return NULL;
		}
	}

	stream = crx_ftp_fopen_connect(wrapper, path, mode, options, opened_path, context, &reuseid, &resource, &use_ssl, &use_ssl_on_data);
	if (!stream) {
		goto errexit;
	}

	/* set the connection to be binary */
	crx_stream_write_string(stream, "TYPE I\r\n");
	result = GET_FTP_RESULT(stream);
	if (result > 299 || result < 200)
		goto errexit;

	/* find out the size of the file (verifying it exists) */
	crx_stream_printf(stream, "SIZE %s\r\n", ZSTR_VAL(resource->path));

	/* read the response */
	result = GET_FTP_RESULT(stream);
	if (read_write == 1) {
		/* Read Mode */
		char *sizestr;

		/* when reading file, it must exist */
		if (result > 299 || result < 200) {
			errno = ENOENT;
			goto errexit;
		}

		sizestr = strchr(tmp_line, ' ');
		if (sizestr) {
			sizestr++;
			file_size = atoi(sizestr);
			crx_stream_notify_file_size(context, file_size, tmp_line, result);
		}
	} else if (read_write == 2) {
		/* when writing file (but not appending), it must NOT exist, unless a context option exists which allows it */
		if (context && (tmpzval = crx_stream_context_get_option(context, "ftp", "overwrite")) != NULL) {
			allow_overwrite = crex_is_true(tmpzval);
		}
		if (result <= 299 && result >= 200) {
			if (allow_overwrite) {
				/* Context permits overwriting file,
				   so we just delete whatever's there in preparation */
				crx_stream_printf(stream, "DELE %s\r\n", ZSTR_VAL(resource->path));
				result = GET_FTP_RESULT(stream);
				if (result >= 300 || result <= 199) {
					goto errexit;
				}
			} else {
				crx_stream_wrapper_log_error(wrapper, options, "Remote file already exists and overwrite context option not specified");
				errno = EEXIST;
				goto errexit;
			}
		}
	}

	/* set up the passive connection */
	portno = crx_fopen_do_pasv(stream, ip, sizeof(ip), &hoststart);

	if (!portno) {
		goto errexit;
	}

	/* Send RETR/STOR command */
	if (read_write == 1) {
		/* set resume position if applicable */
		if (context &&
			(tmpzval = crx_stream_context_get_option(context, "ftp", "resume_pos")) != NULL &&
			C_TYPE_P(tmpzval) == IS_LONG &&
			C_LVAL_P(tmpzval) > 0) {
			crx_stream_printf(stream, "REST " CREX_LONG_FMT "\r\n", C_LVAL_P(tmpzval));
			result = GET_FTP_RESULT(stream);
			if (result < 300 || result > 399) {
				crx_stream_wrapper_log_error(wrapper, options, "Unable to resume from offset " CREX_LONG_FMT, C_LVAL_P(tmpzval));
				goto errexit;
			}
		}

		/* retrieve file */
		memcpy(tmp_line, "RETR", sizeof("RETR"));
	} else if (read_write == 2) {
		/* Write new file */
		memcpy(tmp_line, "STOR", sizeof("STOR"));
	} else {
		/* Append */
		memcpy(tmp_line, "APPE", sizeof("APPE"));
	}
	crx_stream_printf(stream, "%s %s\r\n", tmp_line, (resource->path != NULL ? ZSTR_VAL(resource->path) : "/"));

	/* open the data channel */
	if (hoststart == NULL) {
		hoststart = ZSTR_VAL(resource->host);
	}
	transport_len = (int)spprintf(&transport, 0, "tcp://%s:%d", hoststart, portno);
	datastream = crx_stream_xport_create(transport, transport_len, REPORT_ERRORS, STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT, NULL, NULL, context, &error_message, NULL);
	efree(transport);
	if (datastream == NULL) {
		tmp_line[0]='\0';
		goto errexit;
	}

	result = GET_FTP_RESULT(stream);
	if (result != 150 && result != 125) {
		/* Could not retrieve or send the file
		 * this data will only be sent to us after connection on the data port was initiated.
		 */
		crx_stream_close(datastream);
		datastream = NULL;
		goto errexit;
	}

	crx_stream_context_set(datastream, context);
	crx_stream_notify_progress_init(context, 0, file_size);

	if (use_ssl_on_data && (crx_stream_xport_crypto_setup(datastream,
			STREAM_CRYPTO_METHOD_SSLv23_CLIENT, NULL) < 0 ||
			crx_stream_xport_crypto_enable(datastream, 1) < 0)) {

		crx_stream_wrapper_log_error(wrapper, options, "Unable to activate SSL mode");
		crx_stream_close(datastream);
		datastream = NULL;
		tmp_line[0]='\0';
		goto errexit;
	}

	/* remember control stream */
	datastream->wrapperthis = stream;

	crx_url_free(resource);
	return datastream;

errexit:
	if (resource) {
		crx_url_free(resource);
	}
	if (stream) {
		crx_stream_notify_error(context, CRX_STREAM_NOTIFY_FAILURE, tmp_line, result);
		crx_stream_close(stream);
	}
	if (tmp_line[0] != '\0')
		crx_stream_wrapper_log_error(wrapper, options, "FTP server reports %s", tmp_line);

	if (error_message) {
		crx_stream_wrapper_log_error(wrapper, options, "Failed to set up data channel: %s", ZSTR_VAL(error_message));
		crex_string_release(error_message);
	}
	return NULL;
}
/* }}} */

/* {{{ crx_ftp_dirsteam_read */
static ssize_t crx_ftp_dirstream_read(crx_stream *stream, char *buf, size_t count)
{
	crx_stream_dirent *ent = (crx_stream_dirent *)buf;
	crx_stream *innerstream;
	size_t tmp_len;
	crex_string *basename;

	innerstream =  ((crx_ftp_dirstream_data *)stream->abstract)->datastream;

	if (count != sizeof(crx_stream_dirent)) {
		return -1;
	}

	if (crx_stream_eof(innerstream)) {
		return 0;
	}

	if (!crx_stream_get_line(innerstream, ent->d_name, sizeof(ent->d_name), &tmp_len)) {
		return -1;
	}

	basename = crx_basename(ent->d_name, tmp_len, NULL, 0);

	tmp_len = MIN(sizeof(ent->d_name), ZSTR_LEN(basename) - 1);
	memcpy(ent->d_name, ZSTR_VAL(basename), tmp_len);
	ent->d_name[tmp_len - 1] = '\0';
	crex_string_release_ex(basename, 0);
	ent->d_type = DT_UNKNOWN;

	/* Trim off trailing whitespace characters */
	while (tmp_len > 0 &&
			(ent->d_name[tmp_len - 1] == '\n' || ent->d_name[tmp_len - 1] == '\r' ||
			 ent->d_name[tmp_len - 1] == '\t' || ent->d_name[tmp_len - 1] == ' ')) {
		ent->d_name[--tmp_len] = '\0';
	}

	return sizeof(crx_stream_dirent);
}
/* }}} */

/* {{{ crx_ftp_dirstream_close */
static int crx_ftp_dirstream_close(crx_stream *stream, int close_handle)
{
	crx_ftp_dirstream_data *data = stream->abstract;

	/* close control connection */
	if (data->controlstream) {
		crx_stream_close(data->controlstream);
		data->controlstream = NULL;
	}
	/* close data connection */
	crx_stream_close(data->datastream);
	data->datastream = NULL;

	efree(data);
	stream->abstract = NULL;

	return 0;
}
/* }}} */

/* ftp dirstreams only need to support read and close operations,
   They can't be rewound because the underlying ftp stream can't be rewound. */
static const crx_stream_ops crx_ftp_dirstream_ops = {
	NULL, /* write */
	crx_ftp_dirstream_read, /* read */
	crx_ftp_dirstream_close, /* close */
	NULL, /* flush */
	"ftpdir",
	NULL, /* rewind */
	NULL, /* cast */
	NULL, /* stat */
	NULL  /* set option */
};

/* {{{ crx_stream_ftp_opendir */
crx_stream * crx_stream_ftp_opendir(crx_stream_wrapper *wrapper, const char *path, const char *mode, int options,
									crex_string **opened_path, crx_stream_context *context STREAMS_DC)
{
	crx_stream *stream, *reuseid, *datastream = NULL;
	crx_ftp_dirstream_data *dirsdata;
	crx_url *resource = NULL;
	int result = 0, use_ssl, use_ssl_on_data = 0;
	char *hoststart = NULL, tmp_line[512];
	char ip[sizeof("123.123.123.123")];
	unsigned short portno;

	tmp_line[0] = '\0';

	stream = crx_ftp_fopen_connect(wrapper, path, mode, options, opened_path, context, &reuseid, &resource, &use_ssl, &use_ssl_on_data);
	if (!stream) {
		goto opendir_errexit;
	}

	/* set the connection to be ascii */
	crx_stream_write_string(stream, "TYPE A\r\n");
	result = GET_FTP_RESULT(stream);
	if (result > 299 || result < 200)
		goto opendir_errexit;

	// tmp_line isn't relevant after the crx_fopen_do_pasv().
	tmp_line[0] = '\0';

	/* set up the passive connection */
	portno = crx_fopen_do_pasv(stream, ip, sizeof(ip), &hoststart);

	if (!portno) {
		goto opendir_errexit;
	}

	/* open the data channel */
	if (hoststart == NULL) {
		hoststart = ZSTR_VAL(resource->host);
	}

	datastream = crx_stream_sock_open_host(hoststart, portno, SOCK_STREAM, 0, 0);
	if (datastream == NULL) {
		goto opendir_errexit;
	}

	crx_stream_printf(stream, "NLST %s\r\n", (resource->path != NULL ? ZSTR_VAL(resource->path) : "/"));

	result = GET_FTP_RESULT(stream);
	if (result != 150 && result != 125) {
		/* Could not retrieve or send the file
		 * this data will only be sent to us after connection on the data port was initiated.
		 */
		crx_stream_close(datastream);
		datastream = NULL;
		goto opendir_errexit;
	}

	crx_stream_context_set(datastream, context);
	if (use_ssl_on_data && (crx_stream_xport_crypto_setup(datastream,
			STREAM_CRYPTO_METHOD_SSLv23_CLIENT, NULL) < 0 ||
			crx_stream_xport_crypto_enable(datastream, 1) < 0)) {

		crx_stream_wrapper_log_error(wrapper, options, "Unable to activate SSL mode");
		crx_stream_close(datastream);
		datastream = NULL;
		goto opendir_errexit;
	}

	crx_url_free(resource);

	dirsdata = emalloc(sizeof *dirsdata);
	dirsdata->datastream = datastream;
	dirsdata->controlstream = stream;
	dirsdata->dirstream = crx_stream_alloc(&crx_ftp_dirstream_ops, dirsdata, 0, mode);

	return dirsdata->dirstream;

opendir_errexit:
	if (resource) {
		crx_url_free(resource);
	}
	if (stream) {
		crx_stream_notify_error(context, CRX_STREAM_NOTIFY_FAILURE, tmp_line, result);
		crx_stream_close(stream);
	}
	if (tmp_line[0] != '\0') {
		crx_stream_wrapper_log_error(wrapper, options, "FTP server reports %s", tmp_line);
	}
	return NULL;
}
/* }}} */

/* {{{ crx_stream_ftp_url_stat */
static int crx_stream_ftp_url_stat(crx_stream_wrapper *wrapper, const char *url, int flags, crx_stream_statbuf *ssb, crx_stream_context *context)
{
	crx_stream *stream = NULL;
	crx_url *resource = NULL;
	int result;
	char tmp_line[512];

	/* If ssb is NULL then someone is misbehaving */
	if (!ssb) return -1;

	stream = crx_ftp_fopen_connect(wrapper, url, "r", 0, NULL, context, NULL, &resource, NULL, NULL);
	if (!stream) {
		goto stat_errexit;
	}

	ssb->sb.st_mode = 0644;									/* FTP won't give us a valid mode, so approximate one based on being readable */
	crx_stream_printf(stream, "CWD %s\r\n", (resource->path != NULL ? ZSTR_VAL(resource->path) : "/")); /* If we can CWD to it, it's a directory (maybe a link, but we can't tell) */
	result = GET_FTP_RESULT(stream);
	if (result < 200 || result > 299) {
		ssb->sb.st_mode |= S_IFREG;
	} else {
		ssb->sb.st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
	}

	crx_stream_write_string(stream, "TYPE I\r\n"); /* we need this since some servers refuse to accept SIZE command in ASCII mode */

	result = GET_FTP_RESULT(stream);

	if(result < 200 || result > 299) {
		goto stat_errexit;
	}

	crx_stream_printf(stream, "SIZE %s\r\n", (resource->path != NULL ? ZSTR_VAL(resource->path) : "/"));
	result = GET_FTP_RESULT(stream);
	if (result < 200 || result > 299) {
		/* Failure either means it doesn't exist
		   or it's a directory and this server
		   fails on listing directory sizes */
		if (ssb->sb.st_mode & S_IFDIR) {
			ssb->sb.st_size = 0;
		} else {
			goto stat_errexit;
		}
	} else {
		ssb->sb.st_size = atoi(tmp_line + 4);
	}

	crx_stream_printf(stream, "MDTM %s\r\n", (resource->path != NULL ? ZSTR_VAL(resource->path) : "/"));
	result = GET_FTP_RESULT(stream);
	if (result == 213) {
		char *p = tmp_line + 4;
		int n;
		struct tm tm, tmbuf, *gmt;
		time_t stamp;

		while ((size_t)(p - tmp_line) < sizeof(tmp_line) && !isdigit(*p)) {
			p++;
		}

		if ((size_t)(p - tmp_line) > sizeof(tmp_line)) {
			goto mdtm_error;
		}

		n = sscanf(p, "%4d%2d%2d%2d%2d%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
		if (n != 6) {
			goto mdtm_error;
		}

		tm.tm_year -= 1900;
		tm.tm_mon--;
		tm.tm_isdst = -1;

		/* figure out the GMT offset */
		stamp = time(NULL);
		gmt = crx_gmtime_r(&stamp, &tmbuf);
		if (!gmt) {
			goto mdtm_error;
		}
		gmt->tm_isdst = -1;

		/* apply the GMT offset */
		tm.tm_sec += (long)(stamp - mktime(gmt));
		tm.tm_isdst = gmt->tm_isdst;

		ssb->sb.st_mtime = mktime(&tm);
	} else {
		/* error or unsupported command */
mdtm_error:
		ssb->sb.st_mtime = -1;
	}

	ssb->sb.st_ino = 0;						/* Unknown values */
	ssb->sb.st_dev = 0;
	ssb->sb.st_uid = 0;
	ssb->sb.st_gid = 0;
	ssb->sb.st_atime = -1;
	ssb->sb.st_ctime = -1;

	ssb->sb.st_nlink = 1;
	ssb->sb.st_rdev = -1;
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
	ssb->sb.st_blksize = 4096;				/* Guess since FTP won't expose this information */
#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
	ssb->sb.st_blocks = (int)((4095 + ssb->sb.st_size) / ssb->sb.st_blksize); /* emulate ceil */
#endif
#endif
	crx_stream_close(stream);
	crx_url_free(resource);
	return 0;

stat_errexit:
	if (resource) {
		crx_url_free(resource);
	}
	if (stream) {
		crx_stream_close(stream);
	}
	return -1;
}
/* }}} */

/* {{{ crx_stream_ftp_unlink */
static int crx_stream_ftp_unlink(crx_stream_wrapper *wrapper, const char *url, int options, crx_stream_context *context)
{
	crx_stream *stream = NULL;
	crx_url *resource = NULL;
	int result;
	char tmp_line[512];

	stream = crx_ftp_fopen_connect(wrapper, url, "r", 0, NULL, context, NULL, &resource, NULL, NULL);
	if (!stream) {
		if (options & REPORT_ERRORS) {
			crx_error_docref(NULL, E_WARNING, "Unable to connect to %s", url);
		}
		goto unlink_errexit;
	}

	if (resource->path == NULL) {
		if (options & REPORT_ERRORS) {
			crx_error_docref(NULL, E_WARNING, "Invalid path provided in %s", url);
		}
		goto unlink_errexit;
	}

	/* Attempt to delete the file */
	crx_stream_printf(stream, "DELE %s\r\n", ZSTR_VAL(resource->path));

	result = GET_FTP_RESULT(stream);
	if (result < 200 || result > 299) {
		if (options & REPORT_ERRORS) {
			crx_error_docref(NULL, E_WARNING, "Error Deleting file: %s", tmp_line);
		}
		goto unlink_errexit;
	}

	crx_url_free(resource);
	crx_stream_close(stream);
	return 1;

unlink_errexit:
	if (resource) {
		crx_url_free(resource);
	}
	if (stream) {
		crx_stream_close(stream);
	}
	return 0;
}
/* }}} */

/* {{{ crx_stream_ftp_rename */
static int crx_stream_ftp_rename(crx_stream_wrapper *wrapper, const char *url_from, const char *url_to, int options, crx_stream_context *context)
{
	crx_stream *stream = NULL;
	crx_url *resource_from = NULL, *resource_to = NULL;
	int result;
	char tmp_line[512];

	resource_from = crx_url_parse(url_from);
	resource_to = crx_url_parse(url_to);
	/* Must be same scheme (ftp/ftp or ftps/ftps), same host, and same port
		(or a 21/0 0/21 combination which is also "same")
	   Also require paths to/from */
	if (!resource_from ||
		!resource_to ||
		!resource_from->scheme ||
		!resource_to->scheme ||
		!crex_string_equals(resource_from->scheme, resource_to->scheme) ||
		!resource_from->host ||
		!resource_to->host ||
		!crex_string_equals(resource_from->host, resource_to->host) ||
		(resource_from->port != resource_to->port &&
		 resource_from->port * resource_to->port != 0 &&
		 resource_from->port + resource_to->port != 21) ||
		!resource_from->path ||
		!resource_to->path) {
		goto rename_errexit;
	}

	stream = crx_ftp_fopen_connect(wrapper, url_from, "r", 0, NULL, context, NULL, NULL, NULL, NULL);
	if (!stream) {
		if (options & REPORT_ERRORS) {
			crx_error_docref(NULL, E_WARNING, "Unable to connect to %s", ZSTR_VAL(resource_from->host));
		}
		goto rename_errexit;
	}

	/* Rename FROM */
	crx_stream_printf(stream, "RNFR %s\r\n", ZSTR_VAL(resource_from->path));

	result = GET_FTP_RESULT(stream);
	if (result < 300 || result > 399) {
		if (options & REPORT_ERRORS) {
			crx_error_docref(NULL, E_WARNING, "Error Renaming file: %s", tmp_line);
		}
		goto rename_errexit;
	}

	/* Rename TO */
	crx_stream_printf(stream, "RNTO %s\r\n", ZSTR_VAL(resource_to->path));

	result = GET_FTP_RESULT(stream);
	if (result < 200 || result > 299) {
		if (options & REPORT_ERRORS) {
			crx_error_docref(NULL, E_WARNING, "Error Renaming file: %s", tmp_line);
		}
		goto rename_errexit;
	}

	crx_url_free(resource_from);
	crx_url_free(resource_to);
	crx_stream_close(stream);
	return 1;

rename_errexit:
	if (resource_from) {
		crx_url_free(resource_from);
	}
	if (resource_to) {
		crx_url_free(resource_to);
	}
	if (stream) {
		crx_stream_close(stream);
	}
	return 0;
}
/* }}} */

/* {{{ crx_stream_ftp_mkdir */
static int crx_stream_ftp_mkdir(crx_stream_wrapper *wrapper, const char *url, int mode, int options, crx_stream_context *context)
{
	crx_stream *stream = NULL;
	crx_url *resource = NULL;
	int result, recursive = options & CRX_STREAM_MKDIR_RECURSIVE;
	char tmp_line[512];

	stream = crx_ftp_fopen_connect(wrapper, url, "r", 0, NULL, context, NULL, &resource, NULL, NULL);
	if (!stream) {
		if (options & REPORT_ERRORS) {
			crx_error_docref(NULL, E_WARNING, "Unable to connect to %s", url);
		}
		goto mkdir_errexit;
	}

	if (resource->path == NULL) {
		if (options & REPORT_ERRORS) {
			crx_error_docref(NULL, E_WARNING, "Invalid path provided in %s", url);
		}
		goto mkdir_errexit;
	}

	if (!recursive) {
		crx_stream_printf(stream, "MKD %s\r\n", ZSTR_VAL(resource->path));
		result = GET_FTP_RESULT(stream);
	} else {
		/* we look for directory separator from the end of string, thus hopefully reducing our work load */
		char *p, *e, *buf;

		buf = estrndup(ZSTR_VAL(resource->path), ZSTR_LEN(resource->path));
		e = buf + ZSTR_LEN(resource->path);

		/* find a top level directory we need to create */
		while ((p = strrchr(buf, '/'))) {
			*p = '\0';
			crx_stream_printf(stream, "CWD %s\r\n", strlen(buf) ? buf : "/");
			result = GET_FTP_RESULT(stream);
			if (result >= 200 && result <= 299) {
				*p = '/';
				break;
			}
		}

		crx_stream_printf(stream, "MKD %s\r\n", strlen(buf) ? buf : "/");
		result = GET_FTP_RESULT(stream);

		if (result >= 200 && result <= 299) {
			if (!p) {
				p = buf;
			}
			/* create any needed directories if the creation of the 1st directory worked */
			while (p != e) {
				if (*p == '\0' && *(p + 1) != '\0') {
					*p = '/';
					crx_stream_printf(stream, "MKD %s\r\n", buf);
					result = GET_FTP_RESULT(stream);
					if (result < 200 || result > 299) {
						if (options & REPORT_ERRORS) {
							crx_error_docref(NULL, E_WARNING, "%s", tmp_line);
						}
						break;
					}
				}
				++p;
			}
		}

		efree(buf);
	}

	crx_url_free(resource);
	crx_stream_close(stream);

	if (result < 200 || result > 299) {
		/* Failure */
		return 0;
	}

	return 1;

mkdir_errexit:
	if (resource) {
		crx_url_free(resource);
	}
	if (stream) {
		crx_stream_close(stream);
	}
	return 0;
}
/* }}} */

/* {{{ crx_stream_ftp_rmdir */
static int crx_stream_ftp_rmdir(crx_stream_wrapper *wrapper, const char *url, int options, crx_stream_context *context)
{
	crx_stream *stream = NULL;
	crx_url *resource = NULL;
	int result;
	char tmp_line[512];

	stream = crx_ftp_fopen_connect(wrapper, url, "r", 0, NULL, context, NULL, &resource, NULL, NULL);
	if (!stream) {
		if (options & REPORT_ERRORS) {
			crx_error_docref(NULL, E_WARNING, "Unable to connect to %s", url);
		}
		goto rmdir_errexit;
	}

	if (resource->path == NULL) {
		if (options & REPORT_ERRORS) {
			crx_error_docref(NULL, E_WARNING, "Invalid path provided in %s", url);
		}
		goto rmdir_errexit;
	}

	crx_stream_printf(stream, "RMD %s\r\n", ZSTR_VAL(resource->path));
	result = GET_FTP_RESULT(stream);

	if (result < 200 || result > 299) {
		if (options & REPORT_ERRORS) {
			crx_error_docref(NULL, E_WARNING, "%s", tmp_line);
		}
		goto rmdir_errexit;
	}

	crx_url_free(resource);
	crx_stream_close(stream);

	return 1;

rmdir_errexit:
	if (resource) {
		crx_url_free(resource);
	}
	if (stream) {
		crx_stream_close(stream);
	}
	return 0;
}
/* }}} */

static const crx_stream_wrapper_ops ftp_stream_wops = {
	crx_stream_url_wrap_ftp,
	crx_stream_ftp_stream_close, /* stream_close */
	crx_stream_ftp_stream_stat,
	crx_stream_ftp_url_stat, /* stat_url */
	crx_stream_ftp_opendir, /* opendir */
	"ftp",
	crx_stream_ftp_unlink, /* unlink */
	crx_stream_ftp_rename, /* rename */
	crx_stream_ftp_mkdir,  /* mkdir */
	crx_stream_ftp_rmdir,  /* rmdir */
	NULL
};

CRXAPI const crx_stream_wrapper crx_stream_ftp_wrapper =	{
	&ftp_stream_wops,
	NULL,
	1 /* is_url */
};
