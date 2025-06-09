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
   | Authors: Gustavo Lopes    <cataphract@crx.net>                       |
   +----------------------------------------------------------------------+
 */

#if defined(MCAST_JOIN_GROUP) && !defined(__APPLE__)
# define RFC3678_API 1
/* has block/unblock and source membership, in this case for both IPv4 and IPv6 */
# define HAS_MCAST_EXT 1
#elif defined(IP_ADD_SOURCE_MEMBERSHIP) && !defined(__APPLE__)
/* has block/unblock and source membership, but only for IPv4 */
# define HAS_MCAST_EXT 1
#endif

#ifndef RFC3678_API
# define CRX_MCAST_JOIN_GROUP			IP_ADD_MEMBERSHIP
# define CRX_MCAST_LEAVE_GROUP			IP_DROP_MEMBERSHIP
# ifdef HAS_MCAST_EXT
#  define CRX_MCAST_BLOCK_SOURCE		IP_BLOCK_SOURCE
#  define CRX_MCAST_UNBLOCK_SOURCE		IP_UNBLOCK_SOURCE
#  define CRX_MCAST_JOIN_SOURCE_GROUP	IP_ADD_SOURCE_MEMBERSHIP
#  define CRX_MCAST_LEAVE_SOURCE_GROUP	IP_DROP_SOURCE_MEMBERSHIP
# endif
#else
# define CRX_MCAST_JOIN_GROUP			MCAST_JOIN_GROUP
# define CRX_MCAST_LEAVE_GROUP			MCAST_LEAVE_GROUP
# define CRX_MCAST_BLOCK_SOURCE			MCAST_BLOCK_SOURCE
# define CRX_MCAST_UNBLOCK_SOURCE		MCAST_UNBLOCK_SOURCE
# define CRX_MCAST_JOIN_SOURCE_GROUP	MCAST_JOIN_SOURCE_GROUP
# define CRX_MCAST_LEAVE_SOURCE_GROUP	MCAST_LEAVE_SOURCE_GROUP
#endif

int crx_do_setsockopt_ip_mcast(crx_socket *crx_sock,
							   int level,
							   int optname,
							   zval *arg4);

int crx_do_setsockopt_ipv6_mcast(crx_socket *crx_sock,
								 int level,
								 int optname,
								 zval *arg4);

crex_result crx_if_index_to_addr4(
        unsigned if_index,
        crx_socket *crx_sock,
        struct in_addr *out_addr);

crex_result crx_add4_to_if_index(
        struct in_addr *addr,
        crx_socket *crx_sock,
        unsigned *if_index);

crex_result crx_string_to_if_index(const char *val, unsigned *out);

int crx_mcast_join(
	crx_socket *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	unsigned int if_index);

int crx_mcast_leave(
	crx_socket *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	unsigned int if_index);

#ifdef HAS_MCAST_EXT
int crx_mcast_join_source(
	crx_socket *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	struct sockaddr *source,
	socklen_t source_len,
	unsigned int if_index);

int crx_mcast_leave_source(
	crx_socket *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	struct sockaddr *source,
	socklen_t source_len,
	unsigned int if_index);

int crx_mcast_block_source(
	crx_socket *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	struct sockaddr *source,
	socklen_t source_len,
	unsigned int if_index);

int crx_mcast_unblock_source(
	crx_socket *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	struct sockaddr *source,
	socklen_t source_len,
	unsigned int if_index);
#endif
