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

#ifdef CRX_WIN32
#include "config.w32.h"
#include <Ws2tcpip.h>
#endif

#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

typedef crx_stream *(crx_stream_transport_factory_func)(const char *proto, size_t protolen,
		const char *resourcename, size_t resourcenamelen,
		const char *persistent_id, int options, int flags,
		struct timeval *timeout,
		crx_stream_context *context STREAMS_DC);
typedef crx_stream_transport_factory_func *crx_stream_transport_factory;

BEGIN_EXTERN_C()
CRXAPI int crx_stream_xport_register(const char *protocol, crx_stream_transport_factory factory);
CRXAPI int crx_stream_xport_unregister(const char *protocol);

#define STREAM_XPORT_CLIENT			0
#define STREAM_XPORT_SERVER			1

#define STREAM_XPORT_CONNECT		2
#define STREAM_XPORT_BIND			4
#define STREAM_XPORT_LISTEN			8
#define STREAM_XPORT_CONNECT_ASYNC	16

/* Open a client or server socket connection */
CRXAPI crx_stream *_crx_stream_xport_create(const char *name, size_t namelen, int options,
		int flags, const char *persistent_id,
		struct timeval *timeout,
		crx_stream_context *context,
		crex_string **error_string,
		int *error_code
		STREAMS_DC);

#define crx_stream_xport_create(name, namelen, options, flags, persistent_id, timeout, context, estr, ecode) \
	_crx_stream_xport_create(name, namelen, options, flags, persistent_id, timeout, context, estr, ecode STREAMS_CC)

/* Bind the stream to a local address */
CRXAPI int crx_stream_xport_bind(crx_stream *stream,
		const char *name, size_t namelen,
		crex_string **error_text
		);

/* Connect to a remote address */
CRXAPI int crx_stream_xport_connect(crx_stream *stream,
		const char *name, size_t namelen,
		int asynchronous,
		struct timeval *timeout,
		crex_string **error_text,
		int *error_code
		);

/* Prepare to listen */
CRXAPI int crx_stream_xport_listen(crx_stream *stream,
		int backlog,
		crex_string **error_text
		);

/* Get the next client and their address as a string, or the underlying address
 * structure.  You must efree either of these if you request them */
CRXAPI int crx_stream_xport_accept(crx_stream *stream, crx_stream **client,
		crex_string **textaddr,
		void **addr, socklen_t *addrlen,
		struct timeval *timeout,
		crex_string **error_text
		);

/* Get the name of either the socket or it's peer */
CRXAPI int crx_stream_xport_get_name(crx_stream *stream, int want_peer,
		crex_string **textaddr,
		void **addr, socklen_t *addrlen
		);

enum crx_stream_xport_send_recv_flags {
	STREAM_OOB = 1,
	STREAM_PEEK = 2
};

/* Similar to recv() system call; read data from the stream, optionally
 * peeking, optionally retrieving OOB data */
CRXAPI int crx_stream_xport_recvfrom(crx_stream *stream, char *buf, size_t buflen,
		int flags, void **addr, socklen_t *addrlen,
		crex_string **textaddr);

/* Similar to send() system call; send data to the stream, optionally
 * sending it as OOB data */
CRXAPI int crx_stream_xport_sendto(crx_stream *stream, const char *buf, size_t buflen,
		int flags, void *addr, socklen_t addrlen);

typedef enum {
	STREAM_SHUT_RD,
	STREAM_SHUT_WR,
	STREAM_SHUT_RDWR
} stream_shutdown_t;

/* Similar to shutdown() system call; shut down part of a full-duplex
 * connection */
CRXAPI int crx_stream_xport_shutdown(crx_stream *stream, stream_shutdown_t how);
END_EXTERN_C()


/* Structure definition for the set_option interface that the above functions wrap */

typedef struct _crx_stream_xport_param {
	enum {
		STREAM_XPORT_OP_BIND, STREAM_XPORT_OP_CONNECT,
		STREAM_XPORT_OP_LISTEN, STREAM_XPORT_OP_ACCEPT,
		STREAM_XPORT_OP_CONNECT_ASYNC,
		STREAM_XPORT_OP_GET_NAME,
		STREAM_XPORT_OP_GET_PEER_NAME,
		STREAM_XPORT_OP_RECV,
		STREAM_XPORT_OP_SEND,
		STREAM_XPORT_OP_SHUTDOWN
	} op;
	unsigned int want_addr:1;
	unsigned int want_textaddr:1;
	unsigned int want_errortext:1;
	unsigned int how:2;

	struct {
		char *name;
		size_t namelen;
		struct timeval *timeout;
		struct sockaddr *addr;
		char *buf;
		size_t buflen;
		socklen_t addrlen;
		int backlog;
		int flags;
	} inputs;
	struct {
		crx_stream *client;
		struct sockaddr *addr;
		socklen_t addrlen;
		crex_string *textaddr;
		crex_string *error_text;
		int returncode;
		int error_code;
	} outputs;
} crx_stream_xport_param;

/* Because both client and server streams use the same mechanisms
   for encryption we use the LSB to denote clients.
*/
typedef enum {
	STREAM_CRYPTO_METHOD_SSLv2_CLIENT = (1 << 1 | 1),
	STREAM_CRYPTO_METHOD_SSLv3_CLIENT = (1 << 2 | 1),
	/* v23 no longer negotiates SSL2 or SSL3 */
	STREAM_CRYPTO_METHOD_SSLv23_CLIENT = ((1 << 3) | (1 << 4) | (1 << 5) | 1),
	STREAM_CRYPTO_METHOD_TLSv1_0_CLIENT = (1 << 3 | 1),
	STREAM_CRYPTO_METHOD_TLSv1_1_CLIENT = (1 << 4 | 1),
	STREAM_CRYPTO_METHOD_TLSv1_2_CLIENT = (1 << 5 | 1),
	STREAM_CRYPTO_METHOD_TLSv1_3_CLIENT = (1 << 6 | 1),
	/* TLS equates to TLS_ANY as of CRX 7.2 */
	STREAM_CRYPTO_METHOD_TLS_CLIENT = ((1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | 1),
	STREAM_CRYPTO_METHOD_TLS_ANY_CLIENT = ((1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | 1),
	STREAM_CRYPTO_METHOD_ANY_CLIENT = ((1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | 1),
	STREAM_CRYPTO_METHOD_SSLv2_SERVER = (1 << 1),
	STREAM_CRYPTO_METHOD_SSLv3_SERVER = (1 << 2),
	/* v23 no longer negotiates SSL2 or SSL3 */
	STREAM_CRYPTO_METHOD_SSLv23_SERVER = ((1 << 3) | (1 << 4) | (1 << 5) | (1 << 6)),
	STREAM_CRYPTO_METHOD_TLSv1_0_SERVER = (1 << 3),
	STREAM_CRYPTO_METHOD_TLSv1_1_SERVER = (1 << 4),
	STREAM_CRYPTO_METHOD_TLSv1_2_SERVER = (1 << 5),
	STREAM_CRYPTO_METHOD_TLSv1_3_SERVER = (1 << 6),
	/* TLS equates to TLS_ANY as of CRX 7.2 */
	STREAM_CRYPTO_METHOD_TLS_SERVER = ((1 << 3) | (1 << 4) | (1 << 5) | (1 << 6)),
	STREAM_CRYPTO_METHOD_TLS_ANY_SERVER = ((1 << 3) | (1 << 4) | (1 << 5) | (1 << 6)),
	STREAM_CRYPTO_METHOD_ANY_SERVER = ((1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6))
} crx_stream_xport_crypt_method_t;

/* These functions provide crypto support on the underlying transport */

BEGIN_EXTERN_C()
CRXAPI int crx_stream_xport_crypto_setup(crx_stream *stream, crx_stream_xport_crypt_method_t crypto_method, crx_stream *session_stream);
CRXAPI int crx_stream_xport_crypto_enable(crx_stream *stream, int activate);
END_EXTERN_C()

typedef struct _crx_stream_xport_crypto_param {
	struct {
		crx_stream *session;
		int activate;
		crx_stream_xport_crypt_method_t method;
	} inputs;
	struct {
		int returncode;
	} outputs;
	enum {
		STREAM_XPORT_CRYPTO_OP_SETUP,
		STREAM_XPORT_CRYPTO_OP_ENABLE
	} op;
} crx_stream_xport_crypto_param;

BEGIN_EXTERN_C()
CRXAPI HashTable *crx_stream_xport_get_hash(void);
CRXAPI crx_stream_transport_factory_func crx_stream_generic_socket_factory;
END_EXTERN_C()
