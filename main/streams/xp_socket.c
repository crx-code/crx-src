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

#include "crx.h"
#include "ext/standard/file.h"
#include "streams/crx_streams_int.h"
#include "crx_network.h"

#if defined(CRX_WIN32) || defined(__riscos__)
# undef AF_UNIX
#endif

#ifdef AF_UNIX
#include <sys/un.h>
#endif

#ifndef MSG_DONTWAIT
# define MSG_DONTWAIT 0
#endif

#ifndef MSG_PEEK
# define MSG_PEEK 0
#endif

#ifdef CRX_WIN32
/* send/recv family on windows expects int */
# define XP_SOCK_BUF_SIZE(sz) (((sz) > INT_MAX) ? INT_MAX : (int)(sz))
#else
# define XP_SOCK_BUF_SIZE(sz) (sz)
#endif

const crx_stream_ops crx_stream_generic_socket_ops;
CRXAPI const crx_stream_ops crx_stream_socket_ops;
const crx_stream_ops crx_stream_udp_socket_ops;
#ifdef AF_UNIX
const crx_stream_ops crx_stream_unix_socket_ops;
const crx_stream_ops crx_stream_unixdg_socket_ops;
#endif


static int crx_tcp_sockop_set_option(crx_stream *stream, int option, int value, void *ptrparam);

/* {{{ Generic socket stream operations */
static ssize_t crx_sockop_write(crx_stream *stream, const char *buf, size_t count)
{
	crx_netstream_data_t *sock = (crx_netstream_data_t*)stream->abstract;
	ssize_t didwrite;
	struct timeval *ptimeout;

	if (!sock || sock->socket == -1) {
		return 0;
	}

	if (sock->timeout.tv_sec == -1)
		ptimeout = NULL;
	else
		ptimeout = &sock->timeout;

retry:
	didwrite = send(sock->socket, buf, XP_SOCK_BUF_SIZE(count), (sock->is_blocked && ptimeout) ? MSG_DONTWAIT : 0);

	if (didwrite <= 0) {
		char *estr;
		int err = crx_socket_errno();

		if (CRX_IS_TRANSIENT_ERROR(err)) {
			if (sock->is_blocked) {
				int retval;

				sock->timeout_event = 0;

				do {
					retval = crx_pollfd_for(sock->socket, POLLOUT, ptimeout);

					if (retval == 0) {
						sock->timeout_event = 1;
						break;
					}

					if (retval > 0) {
						/* writable now; retry */
						goto retry;
					}

					err = crx_socket_errno();
				} while (err == EINTR);
			} else {
				/* EWOULDBLOCK/EAGAIN is not an error for a non-blocking stream.
				 * Report zero byte write instead. */
				return 0;
			}
		}

		if (!(stream->flags & CRX_STREAM_FLAG_SUPPRESS_ERRORS)) {
			estr = crx_socket_strerror(err, NULL, 0);
			crx_error_docref(NULL, E_NOTICE,
				"Send of " CREX_LONG_FMT " bytes failed with errno=%d %s",
				(crex_long)count, err, estr);
			efree(estr);
		}
	}

	if (didwrite > 0) {
		crx_stream_notify_progress_increment(CRX_STREAM_CONTEXT(stream), didwrite, 0);
	}

	return didwrite;
}

static void crx_sock_stream_wait_for_data(crx_stream *stream, crx_netstream_data_t *sock, bool has_buffered_data)
{
	int retval;
	struct timeval *ptimeout, zero_timeout;

	if (!sock || sock->socket == -1) {
		return;
	}

	sock->timeout_event = 0;

	if (has_buffered_data) {
		/* If there is already buffered data, use no timeout. */
		zero_timeout.tv_sec = 0;
		zero_timeout.tv_usec = 0;
		ptimeout = &zero_timeout;
	} else if (sock->timeout.tv_sec == -1) {
		ptimeout = NULL;
	} else {
		ptimeout = &sock->timeout;
	}

	while(1) {
		retval = crx_pollfd_for(sock->socket, CRX_POLLREADABLE, ptimeout);

		if (retval == 0)
			sock->timeout_event = 1;

		if (retval >= 0)
			break;

		if (crx_socket_errno() != EINTR)
			break;
	}
}

static ssize_t crx_sockop_read(crx_stream *stream, char *buf, size_t count)
{
	crx_netstream_data_t *sock = (crx_netstream_data_t*)stream->abstract;

	if (!sock || sock->socket == -1) {
		return -1;
	}

	int recv_flags = 0;
	/* Special handling for blocking read. */
	if (sock->is_blocked) {
		/* Find out if there is any data buffered from the previous read. */
		bool has_buffered_data = stream->has_buffered_data;
		/* No need to wait if there is any data buffered or no timeout. */
		bool dont_wait = has_buffered_data ||
				(sock->timeout.tv_sec == 0 && sock->timeout.tv_usec == 0);
		/* Set MSG_DONTWAIT if no wait is needed or there is unlimited timeout which was
		 * added by fix for #41984 commited in 9343c5404. */
		if (dont_wait || sock->timeout.tv_sec != -1) {
			recv_flags = MSG_DONTWAIT;
		}
		/* If the wait is needed or it is a platform without MSG_DONTWAIT support (e.g. Windows),
		 * then poll for data. */
		if (!dont_wait || MSG_DONTWAIT == 0) {
			crx_sock_stream_wait_for_data(stream, sock, has_buffered_data);
			if (sock->timeout_event) {
				/* It is ok to timeout if there is any data buffered so return 0, otherwise -1. */
				return has_buffered_data ? 0 : -1;
			}
		}
	}

	ssize_t nr_bytes = recv(sock->socket, buf, XP_SOCK_BUF_SIZE(count), recv_flags);
	int err = crx_socket_errno();

	if (nr_bytes < 0) {
		if (CRX_IS_TRANSIENT_ERROR(err)) {
			nr_bytes = 0;
		} else {
			stream->eof = 1;
		}
	} else if (nr_bytes == 0) {
		stream->eof = 1;
	}

	if (nr_bytes > 0) {
		crx_stream_notify_progress_increment(CRX_STREAM_CONTEXT(stream), nr_bytes, 0);
	}

	return nr_bytes;
}


static int crx_sockop_close(crx_stream *stream, int close_handle)
{
	crx_netstream_data_t *sock = (crx_netstream_data_t*)stream->abstract;
#ifdef CRX_WIN32
	int n;
#endif

	if (!sock) {
		return 0;
	}

	if (close_handle) {

#ifdef CRX_WIN32
		if (sock->socket == -1)
			sock->socket = SOCK_ERR;
#endif
		if (sock->socket != SOCK_ERR) {
#ifdef CRX_WIN32
			/* prevent more data from coming in */
			shutdown(sock->socket, SHUT_RD);

			/* try to make sure that the OS sends all data before we close the connection.
			 * Essentially, we are waiting for the socket to become writeable, which means
			 * that all pending data has been sent.
			 * We use a small timeout which should encourage the OS to send the data,
			 * but at the same time avoid hanging indefinitely.
			 * */
			do {
				n = crx_pollfd_for_ms(sock->socket, POLLOUT, 500);
			} while (n == -1 && crx_socket_errno() == EINTR);
#endif
			closesocket(sock->socket);
			sock->socket = SOCK_ERR;
		}

	}

	pefree(sock, crx_stream_is_persistent(stream));

	return 0;
}

static int crx_sockop_flush(crx_stream *stream)
{
#if 0
	crx_netstream_data_t *sock = (crx_netstream_data_t*)stream->abstract;
	return fsync(sock->socket);
#endif
	return 0;
}

static int crx_sockop_stat(crx_stream *stream, crx_stream_statbuf *ssb)
{
#ifdef CREX_WIN32
	return 0;
#else
	crx_netstream_data_t *sock = (crx_netstream_data_t*)stream->abstract;

	return crex_fstat(sock->socket, &ssb->sb);
#endif
}

static inline int sock_sendto(crx_netstream_data_t *sock, const char *buf, size_t buflen, int flags,
		struct sockaddr *addr, socklen_t addrlen
		)
{
	int ret;
	if (addr) {
		ret = sendto(sock->socket, buf, XP_SOCK_BUF_SIZE(buflen), flags, addr, XP_SOCK_BUF_SIZE(addrlen));

		return (ret == SOCK_CONN_ERR) ? -1 : ret;
	}
#ifdef CRX_WIN32
	return ((ret = send(sock->socket, buf, buflen > INT_MAX ? INT_MAX : (int)buflen, flags)) == SOCK_CONN_ERR) ? -1 : ret;
#else
	return ((ret = send(sock->socket, buf, buflen, flags)) == SOCK_CONN_ERR) ? -1 : ret;
#endif
}

static inline int sock_recvfrom(crx_netstream_data_t *sock, char *buf, size_t buflen, int flags,
		crex_string **textaddr,
		struct sockaddr **addr, socklen_t *addrlen
		)
{
	int ret;
	int want_addr = textaddr || addr;

	if (want_addr) {
		crx_sockaddr_storage sa;
		socklen_t sl = sizeof(sa);
		ret = recvfrom(sock->socket, buf, XP_SOCK_BUF_SIZE(buflen), flags, (struct sockaddr*)&sa, &sl);
		ret = (ret == SOCK_CONN_ERR) ? -1 : ret;
#ifdef CRX_WIN32
		/* POSIX discards excess bytes without signalling failure; emulate this on Windows */
		if (ret == -1 && WSAGetLastError() == WSAEMSGSIZE) {
			ret = buflen;
		}
#endif
		if (sl) {
			crx_network_populate_name_from_sockaddr((struct sockaddr*)&sa, sl,
					textaddr, addr, addrlen);
		} else {
			if (textaddr) {
				*textaddr = ZSTR_EMPTY_ALLOC();
			}
			if (addr) {
				*addr = NULL;
				*addrlen = 0;
			}
		}
	} else {
		ret = recv(sock->socket, buf, XP_SOCK_BUF_SIZE(buflen), flags);
		ret = (ret == SOCK_CONN_ERR) ? -1 : ret;
	}

	return ret;
}

static int crx_sockop_set_option(crx_stream *stream, int option, int value, void *ptrparam)
{
	int oldmode, flags;
	crx_netstream_data_t *sock = (crx_netstream_data_t*)stream->abstract;
	crx_stream_xport_param *xparam;

	if (!sock) {
		return CRX_STREAM_OPTION_RETURN_NOTIMPL;
	}

	switch(option) {
		case CRX_STREAM_OPTION_CHECK_LIVENESS:
			{
				struct timeval tv;
				char buf;
				int alive = 1;

				if (value == -1) {
					if (sock->timeout.tv_sec == -1) {
						tv.tv_sec = FG(default_socket_timeout);
						tv.tv_usec = 0;
					} else {
						tv = sock->timeout;
					}
				} else {
					tv.tv_sec = value;
					tv.tv_usec = 0;
				}

				if (sock->socket == -1) {
					alive = 0;
				} else if (
					(
						value == 0 &&
						!(stream->flags & CRX_STREAM_FLAG_NO_IO) &&
						((MSG_DONTWAIT != 0) || !sock->is_blocked)
					) ||
					crx_pollfd_for(sock->socket, CRX_POLLREADABLE|POLLPRI, &tv) > 0
				) {
					/* the poll() call was skipped if the socket is non-blocking (or MSG_DONTWAIT is available) and if the timeout is zero */
#ifdef CRX_WIN32
					int ret;
#else
					ssize_t ret;
#endif
					int err;

					ret = recv(sock->socket, &buf, sizeof(buf), MSG_PEEK|MSG_DONTWAIT);
					err = crx_socket_errno();
					if (0 == ret || /* the counterpart did properly shutdown*/
						(0 > ret && err != EWOULDBLOCK && err != EAGAIN && err != EMSGSIZE)) { /* there was an unrecoverable error */
						alive = 0;
					}
				}
				return alive ? CRX_STREAM_OPTION_RETURN_OK : CRX_STREAM_OPTION_RETURN_ERR;
			}

		case CRX_STREAM_OPTION_BLOCKING:
			oldmode = sock->is_blocked;
			if (SUCCESS == crx_set_sock_blocking(sock->socket, value)) {
				sock->is_blocked = value;
				return oldmode;
			}
			return CRX_STREAM_OPTION_RETURN_ERR;

		case CRX_STREAM_OPTION_READ_TIMEOUT:
			sock->timeout = *(struct timeval*)ptrparam;
			sock->timeout_event = 0;
			return CRX_STREAM_OPTION_RETURN_OK;

		case CRX_STREAM_OPTION_META_DATA_API:
			add_assoc_bool((zval *)ptrparam, "timed_out", sock->timeout_event);
			add_assoc_bool((zval *)ptrparam, "blocked", sock->is_blocked);
			add_assoc_bool((zval *)ptrparam, "eof", stream->eof);
			return CRX_STREAM_OPTION_RETURN_OK;

		case CRX_STREAM_OPTION_XPORT_API:
			xparam = (crx_stream_xport_param *)ptrparam;

			switch (xparam->op) {
				case STREAM_XPORT_OP_LISTEN:
					xparam->outputs.returncode = (listen(sock->socket, xparam->inputs.backlog) == 0) ?  0: -1;
					return CRX_STREAM_OPTION_RETURN_OK;

				case STREAM_XPORT_OP_GET_NAME:
					xparam->outputs.returncode = crx_network_get_sock_name(sock->socket,
							xparam->want_textaddr ? &xparam->outputs.textaddr : NULL,
							xparam->want_addr ? &xparam->outputs.addr : NULL,
							xparam->want_addr ? &xparam->outputs.addrlen : NULL
							);
					return CRX_STREAM_OPTION_RETURN_OK;

				case STREAM_XPORT_OP_GET_PEER_NAME:
					xparam->outputs.returncode = crx_network_get_peer_name(sock->socket,
							xparam->want_textaddr ? &xparam->outputs.textaddr : NULL,
							xparam->want_addr ? &xparam->outputs.addr : NULL,
							xparam->want_addr ? &xparam->outputs.addrlen : NULL
							);
					return CRX_STREAM_OPTION_RETURN_OK;

				case STREAM_XPORT_OP_SEND:
					flags = 0;
					if ((xparam->inputs.flags & STREAM_OOB) == STREAM_OOB) {
						flags |= MSG_OOB;
					}
					xparam->outputs.returncode = sock_sendto(sock,
							xparam->inputs.buf, xparam->inputs.buflen,
							flags,
							xparam->inputs.addr,
							xparam->inputs.addrlen);
					if (xparam->outputs.returncode == -1) {
						char *err = crx_socket_strerror(crx_socket_errno(), NULL, 0);
						crx_error_docref(NULL, E_WARNING,
						   	"%s\n", err);
						efree(err);
					}
					return CRX_STREAM_OPTION_RETURN_OK;

				case STREAM_XPORT_OP_RECV:
					flags = 0;
					if ((xparam->inputs.flags & STREAM_OOB) == STREAM_OOB) {
						flags |= MSG_OOB;
					}
					if ((xparam->inputs.flags & STREAM_PEEK) == STREAM_PEEK) {
						flags |= MSG_PEEK;
					}
					xparam->outputs.returncode = sock_recvfrom(sock,
							xparam->inputs.buf, xparam->inputs.buflen,
							flags,
							xparam->want_textaddr ? &xparam->outputs.textaddr : NULL,
							xparam->want_addr ? &xparam->outputs.addr : NULL,
							xparam->want_addr ? &xparam->outputs.addrlen : NULL
							);
					return CRX_STREAM_OPTION_RETURN_OK;


#ifdef HAVE_SHUTDOWN
# ifndef SHUT_RD
#  define SHUT_RD 0
# endif
# ifndef SHUT_WR
#  define SHUT_WR 1
# endif
# ifndef SHUT_RDWR
#  define SHUT_RDWR 2
# endif
				case STREAM_XPORT_OP_SHUTDOWN: {
					static const int shutdown_how[] = {SHUT_RD, SHUT_WR, SHUT_RDWR};

					xparam->outputs.returncode = shutdown(sock->socket, shutdown_how[xparam->how]);
					return CRX_STREAM_OPTION_RETURN_OK;
				}
#endif

				default:
					break;
			}
	}

	return CRX_STREAM_OPTION_RETURN_NOTIMPL;
}

static int crx_sockop_cast(crx_stream *stream, int castas, void **ret)
{
	crx_netstream_data_t *sock = (crx_netstream_data_t*)stream->abstract;

	if (!sock) {
		return FAILURE;
	}

	switch(castas)	{
		case CRX_STREAM_AS_STDIO:
			if (ret)	{
				*(FILE**)ret = fdopen(sock->socket, stream->mode);
				if (*ret)
					return SUCCESS;
				return FAILURE;
			}
			return SUCCESS;
		case CRX_STREAM_AS_FD_FOR_SELECT:
		case CRX_STREAM_AS_FD:
		case CRX_STREAM_AS_SOCKETD:
			if (ret)
				*(crx_socket_t *)ret = sock->socket;
			return SUCCESS;
		default:
			return FAILURE;
	}
}
/* }}} */

/* These may look identical, but we need them this way so that
 * we can determine which type of socket we are dealing with
 * by inspecting stream->ops.
 * A "useful" side-effect is that the user's scripts can then
 * make similar decisions using stream_get_meta_data.
 * */
const crx_stream_ops crx_stream_generic_socket_ops = {
	crx_sockop_write, crx_sockop_read,
	crx_sockop_close, crx_sockop_flush,
	"generic_socket",
	NULL, /* seek */
	crx_sockop_cast,
	crx_sockop_stat,
	crx_sockop_set_option,
};


const crx_stream_ops crx_stream_socket_ops = {
	crx_sockop_write, crx_sockop_read,
	crx_sockop_close, crx_sockop_flush,
	"tcp_socket",
	NULL, /* seek */
	crx_sockop_cast,
	crx_sockop_stat,
	crx_tcp_sockop_set_option,
};

const crx_stream_ops crx_stream_udp_socket_ops = {
	crx_sockop_write, crx_sockop_read,
	crx_sockop_close, crx_sockop_flush,
	"udp_socket",
	NULL, /* seek */
	crx_sockop_cast,
	crx_sockop_stat,
	crx_tcp_sockop_set_option,
};

#ifdef AF_UNIX
const crx_stream_ops crx_stream_unix_socket_ops = {
	crx_sockop_write, crx_sockop_read,
	crx_sockop_close, crx_sockop_flush,
	"unix_socket",
	NULL, /* seek */
	crx_sockop_cast,
	crx_sockop_stat,
	crx_tcp_sockop_set_option,
};
const crx_stream_ops crx_stream_unixdg_socket_ops = {
	crx_sockop_write, crx_sockop_read,
	crx_sockop_close, crx_sockop_flush,
	"udg_socket",
	NULL, /* seek */
	crx_sockop_cast,
	crx_sockop_stat,
	crx_tcp_sockop_set_option,
};
#endif


/* network socket operations */

#ifdef AF_UNIX
static inline int parse_unix_address(crx_stream_xport_param *xparam, struct sockaddr_un *unix_addr)
{
	memset(unix_addr, 0, sizeof(*unix_addr));
	unix_addr->sun_family = AF_UNIX;

	/* we need to be binary safe on systems that support an abstract
	 * namespace */
	if (xparam->inputs.namelen >= sizeof(unix_addr->sun_path)) {
		/* On linux, when the path begins with a NUL byte we are
		 * referring to an abstract namespace.  In theory we should
		 * allow an extra byte below, since we don't need the NULL.
		 * BUT, to get into this branch of code, the name is too long,
		 * so we don't care. */
		xparam->inputs.namelen = sizeof(unix_addr->sun_path) - 1;
		crx_error_docref(NULL, E_NOTICE,
			"socket path exceeded the maximum allowed length of %lu bytes "
			"and was truncated", (unsigned long)sizeof(unix_addr->sun_path));
	}

	memcpy(unix_addr->sun_path, xparam->inputs.name, xparam->inputs.namelen);

	return 1;
}
#endif

static inline char *parse_ip_address_ex(const char *str, size_t str_len, int *portno, int get_err, crex_string **err)
{
	char *colon;
	char *host = NULL;

#ifdef HAVE_IPV6
	char *p;

	if (*(str) == '[' && str_len > 1) {
		/* IPV6 notation to specify raw address with port (i.e. [fe80::1]:80) */
		p = memchr(str + 1, ']', str_len - 2);
		if (!p || *(p + 1) != ':') {
			if (get_err) {
				*err = strpprintf(0, "Failed to parse IPv6 address \"%s\"", str);
			}
			return NULL;
		}
		*portno = atoi(p + 2);
		return estrndup(str + 1, p - str - 1);
	}
#endif
	if (str_len) {
		colon = memchr(str, ':', str_len - 1);
	} else {
		colon = NULL;
	}
	if (colon) {
		*portno = atoi(colon + 1);
		host = estrndup(str, colon - str);
	} else {
		if (get_err) {
			*err = strpprintf(0, "Failed to parse address \"%s\"", str);
		}
		return NULL;
	}

	return host;
}

static inline char *parse_ip_address(crx_stream_xport_param *xparam, int *portno)
{
	return parse_ip_address_ex(xparam->inputs.name, xparam->inputs.namelen, portno, xparam->want_errortext, &xparam->outputs.error_text);
}

static inline int crx_tcp_sockop_bind(crx_stream *stream, crx_netstream_data_t *sock,
		crx_stream_xport_param *xparam)
{
	char *host = NULL;
	int portno, err;
	long sockopts = STREAM_SOCKOP_NONE;
	zval *tmpzval = NULL;

#ifdef AF_UNIX
	if (stream->ops == &crx_stream_unix_socket_ops || stream->ops == &crx_stream_unixdg_socket_ops) {
		struct sockaddr_un unix_addr;

		sock->socket = socket(PF_UNIX, stream->ops == &crx_stream_unix_socket_ops ? SOCK_STREAM : SOCK_DGRAM, 0);

		if (sock->socket == SOCK_ERR) {
			if (xparam->want_errortext) {
				xparam->outputs.error_text = strpprintf(0, "Failed to create unix%s socket %s",
						stream->ops == &crx_stream_unix_socket_ops ? "" : "datagram",
						strerror(errno));
			}
			return -1;
		}

		parse_unix_address(xparam, &unix_addr);

		return bind(sock->socket, (const struct sockaddr *)&unix_addr,
			(socklen_t) XtOffsetOf(struct sockaddr_un, sun_path) + xparam->inputs.namelen);
	}
#endif

	host = parse_ip_address(xparam, &portno);

	if (host == NULL) {
		return -1;
	}

#ifdef IPV6_V6ONLY
	if (CRX_STREAM_CONTEXT(stream)
		&& (tmpzval = crx_stream_context_get_option(CRX_STREAM_CONTEXT(stream), "socket", "ipv6_v6only")) != NULL
		&& C_TYPE_P(tmpzval) != IS_NULL
	) {
		sockopts |= STREAM_SOCKOP_IPV6_V6ONLY;
		sockopts |= STREAM_SOCKOP_IPV6_V6ONLY_ENABLED * crex_is_true(tmpzval);
	}
#endif

#ifdef SO_REUSEPORT
	if (CRX_STREAM_CONTEXT(stream)
		&& (tmpzval = crx_stream_context_get_option(CRX_STREAM_CONTEXT(stream), "socket", "so_reuseport")) != NULL
		&& crex_is_true(tmpzval)
	) {
		sockopts |= STREAM_SOCKOP_SO_REUSEPORT;
	}
#endif

#ifdef SO_BROADCAST
	if (stream->ops == &crx_stream_udp_socket_ops /* SO_BROADCAST is only applicable for UDP */
		&& CRX_STREAM_CONTEXT(stream)
		&& (tmpzval = crx_stream_context_get_option(CRX_STREAM_CONTEXT(stream), "socket", "so_broadcast")) != NULL
		&& crex_is_true(tmpzval)
	) {
		sockopts |= STREAM_SOCKOP_SO_BROADCAST;
	}
#endif

	sock->socket = crx_network_bind_socket_to_local_addr(host, portno,
			stream->ops == &crx_stream_udp_socket_ops ? SOCK_DGRAM : SOCK_STREAM,
			sockopts,
			xparam->want_errortext ? &xparam->outputs.error_text : NULL,
			&err
			);

	if (host) {
		efree(host);
	}

	return sock->socket == -1 ? -1 : 0;
}

static inline int crx_tcp_sockop_connect(crx_stream *stream, crx_netstream_data_t *sock,
		crx_stream_xport_param *xparam)
{
	char *host = NULL, *bindto = NULL;
	int portno, bindport = 0;
	int err = 0;
	int ret;
	zval *tmpzval = NULL;
	long sockopts = STREAM_SOCKOP_NONE;

#ifdef AF_UNIX
	if (stream->ops == &crx_stream_unix_socket_ops || stream->ops == &crx_stream_unixdg_socket_ops) {
		struct sockaddr_un unix_addr;

		sock->socket = socket(PF_UNIX, stream->ops == &crx_stream_unix_socket_ops ? SOCK_STREAM : SOCK_DGRAM, 0);

		if (sock->socket == SOCK_ERR) {
			if (xparam->want_errortext) {
				xparam->outputs.error_text = strpprintf(0, "Failed to create unix socket");
			}
			return -1;
		}

		parse_unix_address(xparam, &unix_addr);

		ret = crx_network_connect_socket(sock->socket,
				(const struct sockaddr *)&unix_addr, (socklen_t) XtOffsetOf(struct sockaddr_un, sun_path) + xparam->inputs.namelen,
				xparam->op == STREAM_XPORT_OP_CONNECT_ASYNC, xparam->inputs.timeout,
				xparam->want_errortext ? &xparam->outputs.error_text : NULL,
				&err);

		xparam->outputs.error_code = err;

		goto out;
	}
#endif

	host = parse_ip_address(xparam, &portno);

	if (host == NULL) {
		return -1;
	}

	if (CRX_STREAM_CONTEXT(stream) && (tmpzval = crx_stream_context_get_option(CRX_STREAM_CONTEXT(stream), "socket", "bindto")) != NULL) {
		if (C_TYPE_P(tmpzval) != IS_STRING) {
			if (xparam->want_errortext) {
				xparam->outputs.error_text = strpprintf(0, "local_addr context option is not a string.");
			}
			efree(host);
			return -1;
		}
		bindto = parse_ip_address_ex(C_STRVAL_P(tmpzval), C_STRLEN_P(tmpzval), &bindport, xparam->want_errortext, &xparam->outputs.error_text);
	}

#ifdef SO_BROADCAST
	if (stream->ops == &crx_stream_udp_socket_ops /* SO_BROADCAST is only applicable for UDP */
		&& CRX_STREAM_CONTEXT(stream)
		&& (tmpzval = crx_stream_context_get_option(CRX_STREAM_CONTEXT(stream), "socket", "so_broadcast")) != NULL
		&& crex_is_true(tmpzval)
	) {
		sockopts |= STREAM_SOCKOP_SO_BROADCAST;
	}
#endif

	if (stream->ops != &crx_stream_udp_socket_ops /* TCP_NODELAY is only applicable for TCP */
#ifdef AF_UNIX
		&& stream->ops != &crx_stream_unix_socket_ops
		&& stream->ops != &crx_stream_unixdg_socket_ops
#endif
		&& CRX_STREAM_CONTEXT(stream)
		&& (tmpzval = crx_stream_context_get_option(CRX_STREAM_CONTEXT(stream), "socket", "tcp_nodelay")) != NULL
		&& crex_is_true(tmpzval)
	) {
		sockopts |= STREAM_SOCKOP_TCP_NODELAY;
	}

	/* Note: the test here for crx_stream_udp_socket_ops is important, because we
	 * want the default to be TCP sockets so that the openssl extension can
	 * re-use this code. */

	sock->socket = crx_network_connect_socket_to_host(host, portno,
			stream->ops == &crx_stream_udp_socket_ops ? SOCK_DGRAM : SOCK_STREAM,
			xparam->op == STREAM_XPORT_OP_CONNECT_ASYNC,
			xparam->inputs.timeout,
			xparam->want_errortext ? &xparam->outputs.error_text : NULL,
			&err,
			bindto,
			bindport,
			sockopts
			);

	ret = sock->socket == -1 ? -1 : 0;
	xparam->outputs.error_code = err;

	if (host) {
		efree(host);
	}
	if (bindto) {
		efree(bindto);
	}

#ifdef AF_UNIX
out:
#endif

	if (ret >= 0 && xparam->op == STREAM_XPORT_OP_CONNECT_ASYNC && err == EINPROGRESS) {
		/* indicates pending connection */
		return 1;
	}

	return ret;
}

static inline int crx_tcp_sockop_accept(crx_stream *stream, crx_netstream_data_t *sock,
		crx_stream_xport_param *xparam STREAMS_DC)
{
	int clisock;
	bool nodelay = 0;
	zval *tmpzval = NULL;

	xparam->outputs.client = NULL;

	if ((NULL != CRX_STREAM_CONTEXT(stream)) &&
		(tmpzval = crx_stream_context_get_option(CRX_STREAM_CONTEXT(stream), "socket", "tcp_nodelay")) != NULL &&
		crex_is_true(tmpzval)) {
		nodelay = 1;
	}

	clisock = crx_network_accept_incoming(sock->socket,
		xparam->want_textaddr ? &xparam->outputs.textaddr : NULL,
		xparam->want_addr ? &xparam->outputs.addr : NULL,
		xparam->want_addr ? &xparam->outputs.addrlen : NULL,
		xparam->inputs.timeout,
		xparam->want_errortext ? &xparam->outputs.error_text : NULL,
		&xparam->outputs.error_code,
		nodelay);

	if (clisock >= 0) {
		crx_netstream_data_t *clisockdata = (crx_netstream_data_t*) emalloc(sizeof(*clisockdata));

		memcpy(clisockdata, sock, sizeof(*clisockdata));
		clisockdata->socket = clisock;
#ifdef __linux__
		/* O_NONBLOCK is not inherited on Linux */
		clisockdata->is_blocked = 1;
#endif

		xparam->outputs.client = crx_stream_alloc_rel(stream->ops, clisockdata, NULL, "r+");
		if (xparam->outputs.client) {
			xparam->outputs.client->ctx = stream->ctx;
			if (stream->ctx) {
				GC_ADDREF(stream->ctx);
			}
		}
	}

	return xparam->outputs.client == NULL ? -1 : 0;
}

static int crx_tcp_sockop_set_option(crx_stream *stream, int option, int value, void *ptrparam)
{
	crx_netstream_data_t *sock = (crx_netstream_data_t*)stream->abstract;
	crx_stream_xport_param *xparam;

	switch(option) {
		case CRX_STREAM_OPTION_XPORT_API:
			xparam = (crx_stream_xport_param *)ptrparam;

			switch(xparam->op) {
				case STREAM_XPORT_OP_CONNECT:
				case STREAM_XPORT_OP_CONNECT_ASYNC:
					xparam->outputs.returncode = crx_tcp_sockop_connect(stream, sock, xparam);
					return CRX_STREAM_OPTION_RETURN_OK;

				case STREAM_XPORT_OP_BIND:
					xparam->outputs.returncode = crx_tcp_sockop_bind(stream, sock, xparam);
					return CRX_STREAM_OPTION_RETURN_OK;


				case STREAM_XPORT_OP_ACCEPT:
					xparam->outputs.returncode = crx_tcp_sockop_accept(stream, sock, xparam STREAMS_CC);
					return CRX_STREAM_OPTION_RETURN_OK;
				default:
					/* fall through */
					;
			}
	}
	return crx_sockop_set_option(stream, option, value, ptrparam);
}


CRXAPI crx_stream *crx_stream_generic_socket_factory(const char *proto, size_t protolen,
		const char *resourcename, size_t resourcenamelen,
		const char *persistent_id, int options, int flags,
		struct timeval *timeout,
		crx_stream_context *context STREAMS_DC)
{
	crx_stream *stream = NULL;
	crx_netstream_data_t *sock;
	const crx_stream_ops *ops;

	/* which type of socket ? */
	if (strncmp(proto, "tcp", protolen) == 0) {
		ops = &crx_stream_socket_ops;
	} else if (strncmp(proto, "udp", protolen) == 0) {
		ops = &crx_stream_udp_socket_ops;
	}
#ifdef AF_UNIX
	else if (strncmp(proto, "unix", protolen) == 0) {
		ops = &crx_stream_unix_socket_ops;
	} else if (strncmp(proto, "udg", protolen) == 0) {
		ops = &crx_stream_unixdg_socket_ops;
	}
#endif
	else {
		/* should never happen */
		return NULL;
	}

	sock = pemalloc(sizeof(crx_netstream_data_t), persistent_id ? 1 : 0);
	memset(sock, 0, sizeof(crx_netstream_data_t));

	sock->is_blocked = 1;
	sock->timeout.tv_sec = FG(default_socket_timeout);
	sock->timeout.tv_usec = 0;

	/* we don't know the socket until we have determined if we are binding or
	 * connecting */
	sock->socket = -1;

	stream = crx_stream_alloc_rel(ops, sock, persistent_id, "r+");

	if (stream == NULL)	{
		pefree(sock, persistent_id ? 1 : 0);
		return NULL;
	}

	return stream;
}
