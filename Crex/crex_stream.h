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
   | Authors: Wez Furlong <wez@thebrainroom.com>                          |
   |          Scott MacVicar <scottmac@crx.net>                           |
   |          Nuno Lopes <nlopess@crx.net>                                |
   |          Marcus Boerger <helly@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_STREAM_H
#define CREX_STREAM_H

#include <sys/types.h>
#include <sys/stat.h>

/* Lightweight stream implementation for the ZE scanners.
 * These functions are private to the engine.
 * */
typedef size_t (*crex_stream_fsizer_t)(void* handle);
typedef ssize_t (*crex_stream_reader_t)(void* handle, char *buf, size_t len);
typedef void   (*crex_stream_closer_t)(void* handle);

#define CREX_MMAP_AHEAD 32

typedef enum {
	CREX_HANDLE_FILENAME,
	CREX_HANDLE_FP,
	CREX_HANDLE_STREAM
} crex_stream_type;

typedef struct _crex_stream {
	void        *handle;
	int         isatty;
	crex_stream_reader_t   reader;
	crex_stream_fsizer_t   fsizer;
	crex_stream_closer_t   closer;
} crex_stream;

typedef struct _crex_file_handle {
	union {
		FILE          *fp;
		crex_stream   stream;
	} handle;
	crex_string       *filename;
	crex_string       *opened_path;
	uint8_t           type; /* packed crex_stream_type */
	bool              primary_script;
	bool              in_list; /* added into CG(open_file) */
	char              *buf;
	size_t            len;
} crex_file_handle;

BEGIN_EXTERN_C()
CREX_API void crex_stream_init_fp(crex_file_handle *handle, FILE *fp, const char *filename);
CREX_API void crex_stream_init_filename(crex_file_handle *handle, const char *filename);
CREX_API void crex_stream_init_filename_ex(crex_file_handle *handle, crex_string *filename);
CREX_API crex_result crex_stream_open(crex_file_handle *handle);
CREX_API crex_result crex_stream_fixup(crex_file_handle *file_handle, char **buf, size_t *len);
CREX_API void crex_destroy_file_handle(crex_file_handle *file_handle);

void crex_stream_init(void);
void crex_stream_shutdown(void);
END_EXTERN_C()

#ifdef CREX_WIN32
# include "win32/ioutil.h"
typedef crx_win32_ioutil_stat_t crex_stat_t;
#ifdef _WIN64
#  define crex_fseek _fseeki64
#  define crex_ftell _ftelli64
#  define crex_lseek _lseeki64
# else
#  define crex_fseek fseek
#  define crex_ftell ftell
#  define crex_lseek lseek
# endif
# define crex_fstat crx_win32_ioutil_fstat
# define crex_stat crx_win32_ioutil_stat
#else
typedef struct stat crex_stat_t;
# define crex_fseek fseek
# define crex_ftell ftell
# define crex_lseek lseek
# define crex_fstat fstat
# define crex_stat stat
#endif

#endif
