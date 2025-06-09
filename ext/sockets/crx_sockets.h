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
   | Authors: Chris Vandomelen <chrisv@b0rked.dhs.org>                    |
   |          Sterling Hughes  <sterling@crx.net>                         |
   |                                                                      |
   | WinSock: Daniel Beulshausen <daniel@crx4win.de>                      |
   +----------------------------------------------------------------------+
 */

#ifndef CRX_SOCKETS_H
#define CRX_SOCKETS_H

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if HAVE_SOCKETS

#include <crx.h>
#ifdef CRX_WIN32
# include "windows_common.h"
#else
# define IS_INVALID_SOCKET(a) (a->bsd_socket < 0)
#endif

#define CRX_SOCKETS_VERSION CRX_VERSION

extern crex_module_entry sockets_module_entry;
#define crxext_sockets_ptr &sockets_module_entry

#ifdef CRX_WIN32
#include <Winsock2.h>
#else
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#endif

#ifndef CRX_WIN32
typedef int CRX_SOCKET;
#else
typedef SOCKET CRX_SOCKET;
#endif

#ifdef CRX_WIN32
#	ifdef CRX_SOCKETS_EXPORTS
#		define CRX_SOCKETS_API __declspec(dllexport)
#	else
#		define CRX_SOCKETS_API __declspec(dllimport)
#	endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define CRX_SOCKETS_API __attribute__ ((visibility("default")))
#else
#	define CRX_SOCKETS_API
#endif

/* Socket class */

typedef struct {
	CRX_SOCKET	bsd_socket;
	int			type;
	int			error;
	int			blocking;
	zval		zstream;
	crex_object std;
} crx_socket;

extern CRX_SOCKETS_API crex_class_entry *socket_ce;

static inline crx_socket *socket_from_obj(crex_object *obj) {
	return (crx_socket *)((char *)(obj) - XtOffsetOf(crx_socket, std));
}

#define C_SOCKET_P(zv) socket_from_obj(C_OBJ_P(zv))

#define ENSURE_SOCKET_VALID(crx_sock) do { \
	if (IS_INVALID_SOCKET(crx_sock)) { \
		crex_argument_error(NULL, 1, "has already been closed"); \
		RETURN_THROWS(); \
	} \
} while (0)

#ifdef CRX_WIN32
struct	sockaddr_un {
	short	sun_family;
	char	sun_path[108];
};
#endif

#define CRX_SOCKET_ERROR(socket, msg, errn) \
		do { \
			int _err = (errn); /* save value to avoid repeated calls to WSAGetLastError() on Windows */ \
			(socket)->error = _err; \
			SOCKETS_G(last_error) = _err; \
			if (_err != EAGAIN && _err != EWOULDBLOCK && _err != EINPROGRESS) { \
				crx_error_docref(NULL, E_WARNING, "%s [%d]: %s", msg, _err, sockets_strerror(_err)); \
			} \
		} while (0)

CREX_BEGIN_MODULE_GLOBALS(sockets)
	int last_error;
	char *strerror_buf;
#ifdef CRX_WIN32
	uint32_t wsa_child_count;
	HashTable wsa_info;
#endif
CREX_END_MODULE_GLOBALS(sockets)

CRX_SOCKETS_API CREX_EXTERN_MODULE_GLOBALS(sockets)
#define SOCKETS_G(v) CREX_MODULE_GLOBALS_ACCESSOR(sockets, v)

enum sockopt_return {
	SOCKOPT_ERROR,
	SOCKOPT_CONTINUE,
	SOCKOPT_SUCCESS
};

CRX_SOCKETS_API char *sockets_strerror(int error);
CRX_SOCKETS_API bool socket_import_file_descriptor(CRX_SOCKET socket, crx_socket *retsock);

#else
#define crxext_sockets_ptr NULL
#endif

#if defined(_AIX) && !defined(HAVE_SA_SS_FAMILY)
# define ss_family __ss_family
#endif

#ifndef MSG_EOF
#ifdef MSG_FIN
#define MSG_EOF MSG_FIN
#endif
#endif

#ifndef MSG_WAITALL
#ifdef LINUX
#define MSG_WAITALL 0x00000100
#else
#define MSG_WAITALL 0x00000000
#endif
#endif

#define CRX_NORMAL_READ 0x0001
#define CRX_BINARY_READ 0x0002

#ifdef WIN32
#define CRX_SOCKET_EINTR WSAEINTR
#elif defined(EINTR)
#define CRX_SOCKET_EINTR EINTR
#endif

#ifdef WIN32
#define CRX_SOCKET_EBADF WSAEBADF
#elif defined(EBADF)
#define CRX_SOCKET_EBADF EBADF
#endif

#ifdef WIN32
#define CRX_SOCKET_EACCES WSAEACCES
#elif defined(EACCES)
#define CRX_SOCKET_EACCES EACCES
#endif

#ifdef WIN32
#define CRX_SOCKET_EFAULT WSAEFAULT
#elif defined(EFAULT)
#define CRX_SOCKET_EFAULT EFAULT
#endif

#ifdef WIN32
#define CRX_SOCKET_EINVAL WSAEINVAL
#elif defined(EINVAL)
#define CRX_SOCKET_EINVAL EINVAL
#endif

#ifdef ENFILE
#define CRX_SOCKET_ENFILE ENFILE
#endif

#ifdef WIN32
#define CRX_SOCKET_EMFILE WSAEMFILE
#elif defined(EMFILE)
#define CRX_SOCKET_EMFILE EMFILE
#endif

#ifdef WIN32
#define CRX_SOCKET_EWOULDBLOCK WSAEWOULDBLOCK
#elif defined(EWOULDBLOCK)
#define CRX_SOCKET_EWOULDBLOCK EWOULDBLOCK
#endif

#ifdef WIN32
#define CRX_SOCKET_EINPROGRESS WSAEINPROGRESS
#elif defined(EINPROGRESS)
#define CRX_SOCKET_EINPROGRESS EINPROGRESS
#endif

#ifdef WIN32
#define CRX_SOCKET_EALREADY WSAEALREADY
#elif defined(EALREADY)
#define CRX_SOCKET_EALREADY EALREADY
#endif

#ifdef WIN32
#define CRX_SOCKET_ENOTSOCK WSAENOTSOCK
#elif defined(ENOTSOCK)
#define CRX_SOCKET_ENOTSOCK ENOTSOCK
#endif

#ifdef WIN32
#define CRX_SOCKET_EDESTADDRREQ WSAEDESTADDRREQ
#elif defined(EDESTADDRREQ)
#define CRX_SOCKET_EDESTADDRREQ EDESTADDRREQ
#endif

#ifdef WIN32
#define CRX_SOCKET_EMSGSIZE WSAEMSGSIZE
#elif defined(EMSGSIZE)
#define CRX_SOCKET_EMSGSIZE EMSGSIZE
#endif

#ifdef WIN32
#define CRX_SOCKET_EPROTOTYPE WSAEPROTOTYPE
#elif defined(EPROTOTYPE)
#define CRX_SOCKET_EPROTOTYPE EPROTOTYPE
#endif

#ifdef WIN32
#define CRX_SOCKET_ENOPROTOOPT WSAENOPROTOOPT
#elif defined(ENOPROTOOPT)
#define CRX_SOCKET_ENOPROTOOPT ENOPROTOOPT
#endif

#ifdef WIN32
#define CRX_SOCKET_EPROTONOSUPPORT WSAEPROTONOSUPPORT
#elif defined(EPROTONOSUPPORT)
#define CRX_SOCKET_EPROTONOSUPPORT EPROTONOSUPPORT
#endif

#ifdef WIN32
#define CRX_SOCKET_ESOCKTNOSUPPORT WSAESOCKTNOSUPPORT
#elif defined(ESOCKTNOSUPPORT)
#define CRX_SOCKET_ESOCKTNOSUPPORT ESOCKTNOSUPPORT
#endif

#ifdef WIN32
#define CRX_SOCKET_EOPNOTSUPP WSAEOPNOTSUPP
#elif defined(EOPNOTSUPP)
#define CRX_SOCKET_EOPNOTSUPP EOPNOTSUPP
#endif

#ifdef WIN32
#define CRX_SOCKET_EPFNOSUPPORT WSAEPFNOSUPPORT
#elif defined(EPFNOSUPPORT)
#define CRX_SOCKET_EPFNOSUPPORT EPFNOSUPPORT
#endif

#ifdef WIN32
#define CRX_SOCKET_EAFNOSUPPORT WSAEAFNOSUPPORT
#elif defined(EAFNOSUPPORT)
#define CRX_SOCKET_EAFNOSUPPORT EAFNOSUPPORT
#endif

#ifdef WIN32
#define CRX_SOCKET_EADDRINUSE WSAEADDRINUSE
#elif defined(EADDRINUSE)
#define CRX_SOCKET_EADDRINUSE EADDRINUSE
#endif

#ifdef WIN32
#define CRX_SOCKET_EADDRNOTAVAIL WSAEADDRNOTAVAIL
#elif defined(EADDRNOTAVAIL)
#define CRX_SOCKET_EADDRNOTAVAIL EADDRNOTAVAIL
#endif

#ifdef WIN32
#define CRX_SOCKET_ENETDOWN WSAENETDOWN
#elif defined(ENETDOWN)
#define CRX_SOCKET_ENETDOWN ENETDOWN
#endif

#ifdef WIN32
#define CRX_SOCKET_ENETUNREACH WSAENETUNREACH
#elif defined(ENETUNREACH)
#define CRX_SOCKET_ENETUNREACH ENETUNREACH
#endif

#ifdef WIN32
#define CRX_SOCKET_ENETRESET WSAENETRESET
#elif defined(ENETRESET)
#define CRX_SOCKET_ENETRESET ENETRESET
#endif

#ifdef WIN32
#define CRX_SOCKET_ECONNABORTED WSAECONNABORTED
#elif defined(ECONNABORTED)
#define CRX_SOCKET_ECONNABORTED ECONNABORTED
#endif

#ifdef WIN32
#define CRX_SOCKET_ECONNRESET WSAECONNRESET
#elif defined(ECONNRESET)
#define CRX_SOCKET_ECONNRESET ECONNRESET
#endif

#ifdef WIN32
#define CRX_SOCKET_ENOBUFS WSAENOBUFS
#elif defined(ENOBUFS)
#define CRX_SOCKET_ENOBUFS ENOBUFS
#endif

#ifdef WIN32
#define CRX_SOCKET_EISCONN WSAEISCONN
#elif defined(EISCONN)
#define CRX_SOCKET_EISCONN EISCONN
#endif

#ifdef WIN32
#define CRX_SOCKET_ENOTCONN WSAENOTCONN
#elif defined(ENOTCONN)
#define CRX_SOCKET_ENOTCONN ENOTCONN
#endif

#ifdef WIN32
#define CRX_SOCKET_ESHUTDOWN WSAESHUTDOWN
#elif defined(ESHUTDOWN)
#define CRX_SOCKET_ESHUTDOWN ESHUTDOWN
#endif

#ifdef WIN32
#define CRX_SOCKET_ETOOMANYREFS WSAETOOMANYREFS
#elif defined(ETOOMANYREFS)
#define CRX_SOCKET_ETOOMANYREFS ETOOMANYREFS
#endif

#ifdef WIN32
#define CRX_SOCKET_ETIMEDOUT WSAETIMEDOUT
#elif defined(ETIMEDOUT)
#define CRX_SOCKET_ETIMEDOUT ETIMEDOUT
#endif

#ifdef WIN32
#define CRX_SOCKET_ECONNREFUSED WSAECONNREFUSED
#elif defined(ECONNREFUSED)
#define CRX_SOCKET_ECONNREFUSED ECONNREFUSED
#endif

#ifdef WIN32
#define CRX_SOCKET_ELOOP WSAELOOP
#elif defined(ELOOP)
#define CRX_SOCKET_ELOOP ELOOP
#endif

#ifdef WIN32
#define CRX_SOCKET_ENAMETOOLONG WSAENAMETOOLONG
#elif defined(ENAMETOOLONG)
#define CRX_SOCKET_ENAMETOOLONG ENAMETOOLONG
#endif

#ifdef WIN32
#define CRX_SOCKET_EHOSTDOWN WSAEHOSTDOWN
#elif defined(EHOSTDOWN)
#define CRX_SOCKET_EHOSTDOWN EHOSTDOWN
#endif

#ifdef WIN32
#define CRX_SOCKET_EHOSTUNREACH WSAEHOSTUNREACH
#elif defined(EHOSTUNREACH)
#define CRX_SOCKET_EHOSTUNREACH EHOSTUNREACH
#endif

#ifdef WIN32
#define CRX_SOCKET_ENOTEMPTY WSAENOTEMPTY
#elif defined(ENOTEMPTY)
#define CRX_SOCKET_ENOTEMPTY ENOTEMPTY
#endif

#ifdef WIN32
#define CRX_SOCKET_EUSERS WSAEUSERS
#elif defined(EUSERS)
#define CRX_SOCKET_EUSERS EUSERS
#endif

#ifdef WIN32
#define CRX_SOCKET_EDQUOT WSAEDQUOT
#elif defined(EDQUOT)
#define CRX_SOCKET_EDQUOT EDQUOT
#endif

#ifdef WIN32
#define CRX_SOCKET_EREMOTE WSAEREMOTE
#elif defined(EREMOTE)
#define CRX_SOCKET_EREMOTE EREMOTE
#endif

#endif
