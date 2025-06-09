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
   | Author: Jim Winstead <jimw@crx.net>                                  |
   +----------------------------------------------------------------------+
 */

#ifndef FOPEN_WRAPPERS_H
#define FOPEN_WRAPPERS_H

BEGIN_EXTERN_C()
#include "crx_globals.h"
#include "crx_ini.h"

CRXAPI int crx_fopen_primary_script(crex_file_handle *file_handle);
CRXAPI char *expand_filepath(const char *filepath, char *real_path);
CRXAPI char *expand_filepath_ex(const char *filepath, char *real_path, const char *relative_to, size_t relative_to_len);
CRXAPI char *expand_filepath_with_mode(const char *filepath, char *real_path, const char *relative_to, size_t relative_to_len, int use_realpath);

CRXAPI int crx_check_open_basedir(const char *path);
CRXAPI int crx_check_open_basedir_ex(const char *path, int warn);
CRXAPI int crx_check_specific_open_basedir(const char *basedir, const char *path);

/* OPENBASEDIR_CHECKPATH(filename) to ease merge between 6.x and 5.x */
#define OPENBASEDIR_CHECKPATH(filename) crx_check_open_basedir(filename)

CRXAPI int crx_check_safe_mode_include_dir(const char *path);

CRXAPI crex_string *crx_resolve_path(const char *filename, size_t filename_len, const char *path);

CRXAPI FILE *crx_fopen_with_path(const char *filename, const char *mode, const char *path, crex_string **opened_path);

CRXAPI char *crx_strip_url_passwd(char *path);

CRXAPI CREX_INI_MH(OnUpdateBaseDir);
END_EXTERN_C()

#endif
