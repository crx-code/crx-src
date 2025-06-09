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
   | Authors: Brian Schaffner <brian@tool.net>                            |
   |          Shane Caraveo <shane@caraveo.com>                           |
   |          Zeev Suraski <zeev@crx.net>                                 |
   +----------------------------------------------------------------------+
*/

#ifndef DL_H
#define DL_H

CRXAPI int crx_load_extension(const char *filename, int type, int start_now);
CRXAPI void crx_dl(const char *file, int type, zval *return_value, int start_now);
CRXAPI void *crx_load_shlib(const char *path, char **errp);

/* dynamic loading functions */
CRXAPI CRX_FUNCTION(dl);

CRX_MINFO_FUNCTION(dl);

#endif /* DL_H */
