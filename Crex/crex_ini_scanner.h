/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   +----------------------------------------------------------------------+
*/

#ifndef _CREX_INI_SCANNER_H
#define _CREX_INI_SCANNER_H

/* Scanner modes */
#define CREX_INI_SCANNER_NORMAL 0 /* Normal mode. [DEFAULT] */
#define CREX_INI_SCANNER_RAW    1 /* Raw mode. Option values are not parsed */
#define CREX_INI_SCANNER_TYPED  2 /* Typed mode. */

BEGIN_EXTERN_C()
CREX_COLD int crex_ini_scanner_get_lineno(void);
CREX_COLD const char *crex_ini_scanner_get_filename(void);
crex_result crex_ini_open_file_for_scanning(crex_file_handle *fh, int scanner_mode);
crex_result crex_ini_prepare_string_for_scanning(const char *str, int scanner_mode);
int ini_lex(zval *ini_lval);
void shutdown_ini_scanner(void);
END_EXTERN_C()

#endif /* _CREX_INI_SCANNER_H */
