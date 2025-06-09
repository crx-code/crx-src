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
   | Author: Stig Venaas <venaas@uninett.no>                              |
   +----------------------------------------------------------------------+
 */

#ifndef _CRX_NETWORK_H
#define _CRX_NETWORK_H

#include <crx.h>

#ifdef CRX_WIN32
# include "win32/inet.h"
#else
# undef closesocket
# define closesocket close
# include <netinet/tcp.h>
#endif

#ifndef HAVE_SHUTDOWN
#undef shutdown
#define shutdown(s,n)	/* nothing */
#endif

#ifdef CRX_WIN32
# ifdef EWOULDBLOCK
#  undef EWOULDBLOCK
# endif
# ifdef EINPROGRESS
#  undef EINPROGRESS
# endif
# define EWOULDBLOCK WSAEWOULDBLOCK
# define EINPROGRESS	WSAEWOULDBLOCK
# define fsync _commit
# define ftruncate(a, b) chsize(a, b)
#endif /* defined(CRX_WIN32) */

#ifndef EWOULDBLOCK
# define EWOULDBLOCK EAGAIN
#endif

/* This is a workaround for GCC bug 69602: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=69602 */
#if EAGAIN != EWOULDBLOCK
# define CRX_IS_TRANSIENT_ERROR(err) (err == EAGAIN || err == EWOULDBLOCK)
#else
# define CRX_IS_TRANSIENT_ERROR(err) (err == EAGAIN)
#endif

#ifdef CRX_WIN32
#define crx_socket_errno() WSAGetLastError()
#else
#define crx_socket_errno() errno
#endif

/* like strerror, but caller must efree the returned string,
 * unless buf is not NULL.
 * Also works sensibly for win32 */
BEGIN_EXTERN_C()
CRXAPI char *crx_socket_strerror(long err, char *buf, size_t bufsize);
CRXAPI crex_string *crx_socket_error_str(long err);
END_EXTERN_C()

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_GETHOSTBYNAME_R
#include <netdb.h>
#endif

/* These are here, rather than with the win32 counterparts above,
 * since <sys/socket.h> defines them. */
#ifndef SHUT_RD
# define SHUT_RD 0
# define SHUT_WR 1
# define SHUT_RDWR 2
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <stddef.h>

#ifdef CRX_WIN32
typedef SOCKET crx_socket_t;
#else
typedef int crx_socket_t;
#endif

#ifdef CRX_WIN32
# define SOCK_ERR INVALID_SOCKET
# define SOCK_CONN_ERR SOCKET_ERROR
# define SOCK_RECV_ERR SOCKET_ERROR
#else
# define SOCK_ERR -1
# define SOCK_CONN_ERR -1
# define SOCK_RECV_ERR -1
#endif

#define STREAM_SOCKOP_NONE                (1 << 0)
#define STREAM_SOCKOP_SO_REUSEPORT        (1 << 1)
#define STREAM_SOCKOP_SO_BROADCAST        (1 << 2)
#define STREAM_SOCKOP_IPV6_V6ONLY         (1 << 3)
#define STREAM_SOCKOP_IPV6_V6ONLY_ENABLED (1 << 4)
#define STREAM_SOCKOP_TCP_NODELAY         (1 << 5)


/* uncomment this to debug poll(2) emulation on systems that have poll(2) */
/* #define CRX_USE_POLL_2_EMULATION 1 */

#if defined(HAVE_POLL)
# if defined(HAVE_POLL_H)
#  include <poll.h>
# elif defined(HAVE_SYS_POLL_H)
#  include <sys/poll.h>
# endif
typedef struct pollfd crx_pollfd;
#else
typedef struct _crx_pollfd {
	crx_socket_t fd;
	short events;
	short revents;
} crx_pollfd;

CRXAPI int crx_poll2(crx_pollfd *ufds, unsigned int nfds, int timeout);

#ifndef POLLIN
# define POLLIN      0x0001    /* There is data to read */
# define POLLPRI     0x0002    /* There is urgent data to read */
# define POLLOUT     0x0004    /* Writing now will not block */
# define POLLERR     0x0008    /* Error condition */
# define POLLHUP     0x0010    /* Hung up */
# define POLLNVAL    0x0020    /* Invalid request: fd not open */
#endif

# ifndef CRX_USE_POLL_2_EMULATION
#  define CRX_USE_POLL_2_EMULATION 1
# endif
#endif

#define CRX_POLLREADABLE	(POLLIN|POLLERR|POLLHUP)

#ifndef CRX_USE_POLL_2_EMULATION
# define crx_poll2(ufds, nfds, timeout)		poll(ufds, nfds, timeout)
#endif

/* timeval-to-timeout (for poll(2)) */
static inline int crx_tvtoto(struct timeval *timeouttv)
{
	if (timeouttv) {
		return (timeouttv->tv_sec * 1000) + (timeouttv->tv_usec / 1000);
	}
	return -1;
}

/* hybrid select(2)/poll(2) for a single descriptor.
 * timeouttv follows same rules as select(2), but is reduced to millisecond accuracy.
 * Returns 0 on timeout, -1 on error, or the event mask (ala poll(2)).
 */
static inline int crx_pollfd_for(crx_socket_t fd, int events, struct timeval *timeouttv)
{
	crx_pollfd p;
	int n;

	p.fd = fd;
	p.events = events;
	p.revents = 0;

	n = crx_poll2(&p, 1, crx_tvtoto(timeouttv));

	if (n > 0) {
		return p.revents;
	}

	return n;
}

static inline int crx_pollfd_for_ms(crx_socket_t fd, int events, int timeout)
{
	crx_pollfd p;
	int n;

	p.fd = fd;
	p.events = events;
	p.revents = 0;

	n = crx_poll2(&p, 1, timeout);

	if (n > 0) {
		return p.revents;
	}

	return n;
}

/* emit warning and suggestion for unsafe select(2) usage */
CRXAPI void _crx_emit_fd_setsize_warning(int max_fd);

static inline bool _crx_check_fd_setsize(crx_socket_t *max_fd, int setsize)
{
#ifdef CRX_WIN32
	(void)(max_fd); // Unused
	if (setsize + 1 >= FD_SETSIZE) {
		_crx_emit_fd_setsize_warning(setsize);
		return false;
	}
#else
	(void)(setsize); // Unused
	if (*max_fd >= FD_SETSIZE) {
		_crx_emit_fd_setsize_warning(*max_fd);
		*max_fd = FD_SETSIZE - 1;
		return false;
	}
#endif
	return true;
}

#ifdef CRX_WIN32
/* it is safe to FD_SET too many fd's under win32; the macro will simply ignore
 * descriptors that go beyond the default FD_SETSIZE */
# define CRX_SAFE_FD_SET(fd, set)	FD_SET(fd, set)
# define CRX_SAFE_FD_CLR(fd, set)	FD_CLR(fd, set)
# define CRX_SAFE_FD_ISSET(fd, set)	FD_ISSET(fd, set)
# define CRX_SAFE_MAX_FD(m, n)		_crx_check_fd_setsize(&m, n)
#else
# define CRX_SAFE_FD_SET(fd, set)	do { if (fd < FD_SETSIZE) FD_SET(fd, set); } while(0)
# define CRX_SAFE_FD_CLR(fd, set)	do { if (fd < FD_SETSIZE) FD_CLR(fd, set); } while(0)
# define CRX_SAFE_FD_ISSET(fd, set)	((fd < FD_SETSIZE) && FD_ISSET(fd, set))
# define CRX_SAFE_MAX_FD(m, n)		_crx_check_fd_setsize(&m, n)
#endif


#define CRX_SOCK_CHUNK_SIZE	8192

#ifdef HAVE_SOCKADDR_STORAGE
typedef struct sockaddr_storage crx_sockaddr_storage;
#else
typedef struct {
#ifdef HAVE_SOCKADDR_SA_LEN
		unsigned char ss_len;
		unsigned char ss_family;
#else
        unsigned short ss_family;
#endif
        char info[126];
} crx_sockaddr_storage;
#endif

BEGIN_EXTERN_C()
CRXAPI int crx_network_getaddresses(const char *host, int socktype, struct sockaddr ***sal, crex_string **error_string);
CRXAPI void crx_network_freeaddresses(struct sockaddr **sal);

CRXAPI crx_socket_t crx_network_connect_socket_to_host(const char *host, unsigned short port,
		int socktype, int asynchronous, struct timeval *timeout, crex_string **error_string,
		int *error_code, const char *bindto, unsigned short bindport, long sockopts
		);

CRXAPI int crx_network_connect_socket(crx_socket_t sockfd,
		const struct sockaddr *addr,
		socklen_t addrlen,
		int asynchronous,
		struct timeval *timeout,
		crex_string **error_string,
		int *error_code);

#define crx_connect_nonb(sock, addr, addrlen, timeout) \
	crx_network_connect_socket((sock), (addr), (addrlen), 0, (timeout), NULL, NULL)

CRXAPI crx_socket_t crx_network_bind_socket_to_local_addr(const char *host, unsigned port,
		int socktype, long sockopts, crex_string **error_string, int *error_code
		);

CRXAPI crx_socket_t crx_network_accept_incoming(crx_socket_t srvsock,
		crex_string **textaddr,
		struct sockaddr **addr,
		socklen_t *addrlen,
		struct timeval *timeout,
		crex_string **error_string,
		int *error_code,
		int tcp_nodelay
		);

CRXAPI int crx_network_get_sock_name(crx_socket_t sock,
		crex_string **textaddr,
		struct sockaddr **addr,
		socklen_t *addrlen
		);

CRXAPI int crx_network_get_peer_name(crx_socket_t sock,
		crex_string **textaddr,
		struct sockaddr **addr,
		socklen_t *addrlen
		);

CRXAPI void crx_any_addr(int family, crx_sockaddr_storage *addr, unsigned short port);
CRXAPI int crx_sockaddr_size(crx_sockaddr_storage *addr);
END_EXTERN_C()

struct _crx_netstream_data_t	{
	crx_socket_t socket;
	char is_blocked;
	struct timeval timeout;
	char timeout_event;
	size_t ownsize;
};
typedef struct _crx_netstream_data_t crx_netstream_data_t;
CRXAPI extern const crx_stream_ops crx_stream_socket_ops;
extern const crx_stream_ops crx_stream_generic_socket_ops;
#define CRX_STREAM_IS_SOCKET	(&crx_stream_socket_ops)

BEGIN_EXTERN_C()
CRXAPI crx_stream *_crx_stream_sock_open_from_socket(crx_socket_t socket, const char *persistent_id STREAMS_DC );
/* open a connection to a host using crx_hostconnect and return a stream */
CRXAPI crx_stream *_crx_stream_sock_open_host(const char *host, unsigned short port,
		int socktype, struct timeval *timeout, const char *persistent_id STREAMS_DC);
CRXAPI void crx_network_populate_name_from_sockaddr(
		/* input address */
		struct sockaddr *sa, socklen_t sl,
		/* output readable address */
		crex_string **textaddr,
		/* output address */
		struct sockaddr **addr,
		socklen_t *addrlen
		);

CRXAPI int crx_network_parse_network_address_with_port(const char *addr,
		crex_long addrlen, struct sockaddr *sa, socklen_t *sl);

CRXAPI struct hostent*	crx_network_gethostbyname(const char *name);

CRXAPI int crx_set_sock_blocking(crx_socket_t socketd, int block);
END_EXTERN_C()

#define crx_stream_sock_open_from_socket(socket, persistent)	_crx_stream_sock_open_from_socket((socket), (persistent) STREAMS_CC)
#define crx_stream_sock_open_host(host, port, socktype, timeout, persistent)	_crx_stream_sock_open_host((host), (port), (socktype), (timeout), (persistent) STREAMS_CC)

/* {{{ memory debug */
#define crx_stream_sock_open_from_socket_rel(socket, persistent)	_crx_stream_sock_open_from_socket((socket), (persistent) STREAMS_REL_CC)
#define crx_stream_sock_open_host_rel(host, port, socktype, timeout, persistent)	_crx_stream_sock_open_host((host), (port), (socktype), (timeout), (persistent) STREAMS_REL_CC)
#define crx_stream_sock_open_unix_rel(path, pathlen, persistent, timeval)	_crx_stream_sock_open_unix((path), (pathlen), (persistent), (timeval) STREAMS_REL_CC)

/* }}} */

#ifndef MAXFQDNLEN
#define MAXFQDNLEN 255
#endif

#endif /* _CRX_NETWORK_H */
