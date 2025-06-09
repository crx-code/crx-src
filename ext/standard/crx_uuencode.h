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
   | Author: Ilia Alshanetsky <ilia@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_UUENCODE_H
#define CRX_UUENCODE_H

CRXAPI crex_string *crx_uudecode(const char *src, size_t src_len);
CRXAPI crex_string *crx_uuencode(const char *src, size_t src_len);

#endif /* CRX_UUENCODE_H */
