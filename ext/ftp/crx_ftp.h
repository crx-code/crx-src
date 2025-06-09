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
   | Authors: Andrew Skalski <askalski@chek.com>                          |
   |          Stefan Esser <sesser@crx.net> (resume functions)            |
   +----------------------------------------------------------------------+
 */

#ifndef	_INCLUDED_FTP_H
#define	_INCLUDED_FTP_H

extern crex_module_entry crx_ftp_module_entry;
#define crxext_ftp_ptr &crx_ftp_module_entry

#include "crx_version.h"
#define CRX_FTP_VERSION CRX_VERSION

#define CRX_FTP_OPT_TIMEOUT_SEC	0
#define CRX_FTP_OPT_AUTOSEEK	1
#define CRX_FTP_OPT_USEPASVADDRESS	2
#define CRX_FTP_AUTORESUME		-1

CRX_MINIT_FUNCTION(ftp);
CRX_MINFO_FUNCTION(ftp);

#endif
