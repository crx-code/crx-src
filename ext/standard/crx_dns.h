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
   | Authors: The typical suspects                                        |
   |          Marcus Boerger <helly@crx.net>                              |
   |          Pollita <pollita@crx.net>                                   |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_DNS_H
#define CRX_DNS_H

#if defined(HAVE_DNS_SEARCH)
#define crx_dns_search(res, dname, class, type, answer, anslen) \
    	((int)dns_search(res, dname, class, type, (char *) answer, anslen, (struct sockaddr *)&from, &fromsize))
#define crx_dns_free_handle(res) \
		dns_free(res)
#define crx_dns_errno(handle) h_errno

#elif defined(HAVE_RES_NSEARCH)
#define crx_dns_search(res, dname, class, type, answer, anslen) \
			res_nsearch(res, dname, class, type, answer, anslen);
#ifdef HAVE_RES_NDESTROY
#define crx_dns_free_handle(res) \
			res_ndestroy(res); \
			crx_dns_free_res(res)
#else
#define crx_dns_free_handle(res) \
			res_nclose(res); \
			crx_dns_free_res(res)
#endif
#define crx_dns_errno(handle) handle->res_h_errno

#elif defined(HAVE_RES_SEARCH)
#define crx_dns_search(res, dname, class, type, answer, anslen) \
			res_search(dname, class, type, answer, anslen)
#define crx_dns_free_handle(res) /* noop */
#define crx_dns_errno(handle) h_errno

#endif

#if defined(HAVE_DNS_SEARCH) || defined(HAVE_RES_NSEARCH) || defined(HAVE_RES_SEARCH)
#define HAVE_DNS_SEARCH_FUNC 1
#endif

#if defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_DN_EXPAND) && defined(HAVE_DN_SKIPNAME)
#define HAVE_FULL_DNS_FUNCS 1
#endif

#if defined(CRX_WIN32) || (defined(HAVE_DNS_SEARCH_FUNC) && defined(HAVE_FULL_DNS_FUNCS))
#define CRX_DNS_A      0x00000001
#define CRX_DNS_NS     0x00000002
#define CRX_DNS_CNAME  0x00000010
#define CRX_DNS_SOA    0x00000020
#define CRX_DNS_PTR    0x00000800
#define CRX_DNS_HINFO  0x00001000
#if !defined(CRX_WIN32)
# define CRX_DNS_CAA    0x00002000
#endif
#define CRX_DNS_MX     0x00004000
#define CRX_DNS_TXT    0x00008000
#define CRX_DNS_A6     0x01000000
#define CRX_DNS_SRV    0x02000000
#define CRX_DNS_NAPTR  0x04000000
#define CRX_DNS_AAAA   0x08000000
#define CRX_DNS_ANY    0x10000000

#if defined(CRX_WIN32)
# define CRX_DNS_NUM_TYPES	12	/* Number of DNS Types Supported by CRX currently */
# define CRX_DNS_ALL    (CRX_DNS_A|CRX_DNS_NS|CRX_DNS_CNAME|CRX_DNS_SOA|CRX_DNS_PTR|CRX_DNS_HINFO|CRX_DNS_MX|CRX_DNS_TXT|CRX_DNS_A6|CRX_DNS_SRV|CRX_DNS_NAPTR|CRX_DNS_AAAA)
#else
# define CRX_DNS_NUM_TYPES	13	/* Number of DNS Types Supported by CRX currently */
# define CRX_DNS_ALL   (CRX_DNS_A|CRX_DNS_NS|CRX_DNS_CNAME|CRX_DNS_SOA|CRX_DNS_PTR|CRX_DNS_HINFO|CRX_DNS_CAA|CRX_DNS_MX|CRX_DNS_TXT|CRX_DNS_A6|CRX_DNS_SRV|CRX_DNS_NAPTR|CRX_DNS_AAAA)
#endif
#endif

#ifndef INT16SZ
#define INT16SZ		2
#endif

#ifndef INT32SZ
#define INT32SZ		4
#endif

#endif /* CRX_DNS_H */
