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
   | Authors: Sara Golemon <pollita@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_NET_H
#define CRX_NET_H

#include "crx.h"
#include "crx_network.h"

CRXAPI crex_string* crx_inet_ntop(const struct sockaddr *addr);

#endif /* CRX_NET_H */
