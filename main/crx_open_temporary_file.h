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
   | Author: Zeev Suraski <zeev@crx.net>                                  |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_OPEN_TEMPORARY_FILE_H
#define CRX_OPEN_TEMPORARY_FILE_H

#define CRX_TMP_FILE_DEFAULT 0
#define CRX_TMP_FILE_OPEN_BASEDIR_CHECK_ON_FALLBACK (1<<0)
#define CRX_TMP_FILE_SILENT (1<<1)
#define CRX_TMP_FILE_OPEN_BASEDIR_CHECK_ON_EXPLICIT_DIR (1<<2)
#define CRX_TMP_FILE_OPEN_BASEDIR_CHECK_ALWAYS \
    (CRX_TMP_FILE_OPEN_BASEDIR_CHECK_ON_FALLBACK | CRX_TMP_FILE_OPEN_BASEDIR_CHECK_ON_EXPLICIT_DIR)

/* for compatibility purpose */
#define CRX_TMP_FILE_OPEN_BASEDIR_CHECK CRX_TMP_FILE_OPEN_BASEDIR_CHECK_ON_FALLBACK


BEGIN_EXTERN_C()
CRXAPI FILE *crx_open_temporary_file(const char *dir, const char *pfx, crex_string **opened_path_p);
CRXAPI int crx_open_temporary_fd_ex(const char *dir, const char *pfx, crex_string **opened_path_p, uint32_t flags);
CRXAPI int crx_open_temporary_fd(const char *dir, const char *pfx, crex_string **opened_path_p);
CRXAPI const char *crx_get_temporary_directory(void);
END_EXTERN_C()

#endif /* CRX_OPEN_TEMPORARY_FILE_H */
