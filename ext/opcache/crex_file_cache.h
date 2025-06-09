/*
   +----------------------------------------------------------------------+
   | Crex OPcache                                                         |
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
   | Authors: Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_FILE_CACHE_H
#define CREX_FILE_CACHE_H

int crex_file_cache_script_store(crex_persistent_script *script, bool in_shm);
crex_persistent_script *crex_file_cache_script_load(crex_file_handle *file_handle);
void crex_file_cache_invalidate(crex_string *full_path);

#endif /* CREX_FILE_CACHE_H */
