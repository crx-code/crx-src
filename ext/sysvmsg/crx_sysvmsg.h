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

#ifndef CRX_SYSVMSG_H
#define CRX_SYSVMSG_H

#ifdef HAVE_SYSVMSG

extern crex_module_entry sysvmsg_module_entry;
#define crxext_sysvmsg_ptr &sysvmsg_module_entry

#include "crx_version.h"
#define CRX_SYSVMSG_VERSION CRX_VERSION

#endif /* HAVE_SYSVMSG */

/* In order to detect MSG_EXCEPT use at run time; we have no way
 * of knowing what the bit definitions are, so we can't just define
 * our own MSG_EXCEPT value. */
#define CRX_MSG_IPC_NOWAIT  1
#define CRX_MSG_NOERROR     2
#define CRX_MSG_EXCEPT      4

#endif	/* CRX_SYSVMSG_H */
