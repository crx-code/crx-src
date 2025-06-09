#include <crx.h>
#include <crx_network.h>
#include "crx_sockets.h"

#ifdef CRX_WIN32
#include "windows_common.h"
#else
#include <netdb.h>
#include <arpa/inet.h>
#endif

extern crex_result crx_string_to_if_index(const char *val, unsigned *out);

#if HAVE_IPV6
/* Sets addr by hostname, or by ip in string form (AF_INET6) */
int crx_set_inet6_addr(struct sockaddr_in6 *sin6, char *string, crx_socket *crx_sock) /* {{{ */
{
	struct in6_addr tmp;
#if HAVE_GETADDRINFO
	struct addrinfo hints;
	struct addrinfo *addrinfo = NULL;
#endif
	char *scope = strchr(string, '%');

	if (inet_pton(AF_INET6, string, &tmp)) {
		memcpy(&(sin6->sin6_addr.s6_addr), &(tmp.s6_addr), sizeof(struct in6_addr));
	} else {
#if HAVE_GETADDRINFO

		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_INET6;
#if HAVE_AI_V4MAPPED
		hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;
#else
		hints.ai_flags = AI_ADDRCONFIG;
#endif
		getaddrinfo(string, NULL, &hints, &addrinfo);
		if (!addrinfo) {
#ifdef CRX_WIN32
			CRX_SOCKET_ERROR(crx_sock, "Host lookup failed", WSAGetLastError());
#else
			CRX_SOCKET_ERROR(crx_sock, "Host lookup failed", (-10000 - h_errno));
#endif
			return 0;
		}
		if (addrinfo->ai_family != PF_INET6 || addrinfo->ai_addrlen != sizeof(struct sockaddr_in6)) {
			crx_error_docref(NULL, E_WARNING, "Host lookup failed: Non AF_INET6 domain returned on AF_INET6 socket");
			freeaddrinfo(addrinfo);
			return 0;
		}

		memcpy(&(sin6->sin6_addr.s6_addr), ((struct sockaddr_in6*)(addrinfo->ai_addr))->sin6_addr.s6_addr, sizeof(struct in6_addr));
		freeaddrinfo(addrinfo);

#else
		/* No IPv6 specific hostname resolution is available on this system? */
		crx_error_docref(NULL, E_WARNING, "Host lookup failed: getaddrinfo() not available on this system");
		return 0;
#endif

	}

	if (scope) {
		crex_long lval = 0;
		double dval = 0;
		unsigned scope_id = 0;

		scope++;

		if (IS_LONG == is_numeric_string(scope, strlen(scope), &lval, &dval, 0)) {
			if (lval > 0 && (crex_ulong)lval <= UINT_MAX) {
				scope_id = lval;
			}
		} else {
			crx_string_to_if_index(scope, &scope_id);
		}

		sin6->sin6_scope_id = scope_id;
	}

	return 1;
}
/* }}} */
#endif

/* Sets addr by hostname, or by ip in string form (AF_INET)  */
int crx_set_inet_addr(struct sockaddr_in *sin, char *string, crx_socket *crx_sock) /* {{{ */
{
	struct in_addr tmp;
	struct hostent *host_entry;

#ifdef HAVE_INET_PTON
	if (inet_pton(AF_INET, string, &tmp)) {
#else
	if (inet_aton(string, &tmp)) {
#endif
		sin->sin_addr.s_addr = tmp.s_addr;
	} else {
		if (strlen(string) > MAXFQDNLEN || ! (host_entry = crx_network_gethostbyname(string))) {
			/* Note: < -10000 indicates a host lookup error */
#ifdef CRX_WIN32
			CRX_SOCKET_ERROR(crx_sock, "Host lookup failed", WSAGetLastError());
#else
			CRX_SOCKET_ERROR(crx_sock, "Host lookup failed", (-10000 - h_errno));
#endif
			return 0;
		}
		if (host_entry->h_addrtype != AF_INET) {
			crx_error_docref(NULL, E_WARNING, "Host lookup failed: Non AF_INET domain returned on AF_INET socket");
			return 0;
		}
		memcpy(&(sin->sin_addr.s_addr), host_entry->h_addr_list[0], host_entry->h_length);
	}

	return 1;
}
/* }}} */

/* Sets addr by hostname or by ip in string form (AF_INET or AF_INET6,
 * depending on the socket) */
int crx_set_inet46_addr(crx_sockaddr_storage *ss, socklen_t *ss_len, char *string, crx_socket *crx_sock) /* {{{ */
{
	if (crx_sock->type == AF_INET) {
		struct sockaddr_in t = {0};
		if (crx_set_inet_addr(&t, string, crx_sock)) {
			memcpy(ss, &t, sizeof t);
			ss->ss_family = AF_INET;
			*ss_len = sizeof(t);
			return 1;
		}
	}
#if HAVE_IPV6
	else if (crx_sock->type == AF_INET6) {
		struct sockaddr_in6 t = {0};
		if (crx_set_inet6_addr(&t, string, crx_sock)) {
			memcpy(ss, &t, sizeof t);
			ss->ss_family = AF_INET6;
			*ss_len = sizeof(t);
			return 1;
		}
	}
#endif
	else {
		crx_error_docref(NULL, E_WARNING,
			"IP address used in the context of an unexpected type of socket");
	}
	return 0;
}
