#ifndef CRX_SOCKADR_CONV_H
#define CRX_SOCKADR_CONV_H

#include <crx_network.h>
#include "crx_sockets.h" /* crx_socket */

#ifndef CRX_WIN32
# include <netinet/in.h>
#else
# include <Winsock2.h>
#endif


/*
 * Convert an IPv6 literal or a hostname info a sockaddr_in6.
 * The IPv6 literal can be a IPv4 mapped address (like ::ffff:127.0.0.1).
 * If the hostname yields no IPv6 addresses, a mapped IPv4 address may be returned (AI_V4MAPPED)
 */
int crx_set_inet6_addr(struct sockaddr_in6 *sin6, char *string, crx_socket *crx_sock);

/*
 * Convert an IPv4 literal or a hostname into a sockaddr_in.
 */
int crx_set_inet_addr(struct sockaddr_in *sin, char *string, crx_socket *crx_sock);

/*
 * Calls either crx_set_inet6_addr() or crx_set_inet_addr(), depending on the type of the socket.
 */
int crx_set_inet46_addr(crx_sockaddr_storage *ss, socklen_t *ss_len, char *string, crx_socket *crx_sock);

#endif
