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
   | Author: Stig Sæther Bakken <ssb@crx.net>                             |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_EXT_SYSLOG_H
#define CRX_EXT_SYSLOG_H

#ifdef HAVE_SYSLOG_H

#include "crx_syslog.h"

CRX_MINIT_FUNCTION(syslog);
CRX_RSHUTDOWN_FUNCTION(syslog);

#endif

#endif /* CRX_EXT_SYSLOG_H */
