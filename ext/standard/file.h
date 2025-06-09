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
   | Author: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                        |
   +----------------------------------------------------------------------+
*/

#ifndef FILE_H
#define FILE_H

#include "crx_network.h"

CRX_MINIT_FUNCTION(file);
CRX_MSHUTDOWN_FUNCTION(file);

CRXAPI CRX_FUNCTION(fclose);
CRXAPI CRX_FUNCTION(feof);
CRXAPI CRX_FUNCTION(fread);
CRXAPI CRX_FUNCTION(fgetc);
CRXAPI CRX_FUNCTION(fgets);
CRXAPI CRX_FUNCTION(fwrite);
CRXAPI CRX_FUNCTION(fflush);
CRXAPI CRX_FUNCTION(rewind);
CRXAPI CRX_FUNCTION(ftell);
CRXAPI CRX_FUNCTION(fseek);
CRXAPI CRX_FUNCTION(fpassthru);

CRX_MINIT_FUNCTION(user_streams);

CRXAPI int crx_le_stream_context(void);
CRXAPI int crx_set_sock_blocking(crx_socket_t socketd, int block);
CRXAPI int crx_copy_file(const char *src, const char *dest);
CRXAPI int crx_copy_file_ex(const char *src, const char *dest, int src_chk);
CRXAPI int crx_copy_file_ctx(const char *src, const char *dest, int src_chk, crx_stream_context *ctx);
CRXAPI int crx_mkdir_ex(const char *dir, crex_long mode, int options);
CRXAPI int crx_mkdir(const char *dir, crex_long mode);
CRXAPI void crx_fstat(crx_stream *stream, zval *return_value);
CRXAPI void crx_flock_common(crx_stream *stream, crex_long operation, uint32_t operation_arg_num,
	zval *wouldblock, zval *return_value);

#define CRX_CSV_NO_ESCAPE EOF
CRXAPI HashTable *crx_bc_fgetcsv_empty_line(void);
CRXAPI HashTable *crx_fgetcsv(crx_stream *stream, char delimiter, char enclosure, int escape_char, size_t buf_len, char *buf);
CRXAPI ssize_t crx_fputcsv(crx_stream *stream, zval *fields, char delimiter, char enclosure, int escape_char, crex_string *eol_str);

#define META_DEF_BUFSIZE 8192

#define CRX_FILE_USE_INCLUDE_PATH (1 << 0)
#define CRX_FILE_IGNORE_NEW_LINES (1 << 1)
#define CRX_FILE_SKIP_EMPTY_LINES (1 << 2)
#define CRX_FILE_APPEND (1 << 3)
#define CRX_FILE_NO_DEFAULT_CONTEXT (1 << 4)

typedef enum _crx_meta_tags_token {
	TOK_EOF = 0,
	TOK_OPENTAG,
	TOK_CLOSETAG,
	TOK_SLASH,
	TOK_EQUAL,
	TOK_SPACE,
	TOK_ID,
	TOK_STRING,
	TOK_OTHER
} crx_meta_tags_token;

typedef struct _crx_meta_tags_data {
	crx_stream *stream;
	int ulc;
	int lc;
	char *input_buffer;
	char *token_data;
	int token_len;
	int in_meta;
} crx_meta_tags_data;

crx_meta_tags_token crx_next_meta_token(crx_meta_tags_data *);

typedef struct {
	int pclose_ret;
	size_t def_chunk_size;
	bool auto_detect_line_endings;
	crex_long default_socket_timeout;
	char *user_agent; /* for the http wrapper */
	char *from_address; /* for the ftp and http wrappers */
	const char *user_stream_current_filename; /* for simple recursion protection */
	crx_stream_context *default_context;
	HashTable *stream_wrappers;			/* per-request copy of url_stream_wrappers_hash */
	HashTable *stream_filters;			/* per-request copy of stream_filters_hash */
	HashTable *wrapper_errors;			/* key: wrapper address; value: linked list of char* */
	int pclose_wait;
#ifdef HAVE_GETHOSTBYNAME_R
	struct hostent tmp_host_info;
	char *tmp_host_buf;
	size_t tmp_host_buf_len;
#endif
} crx_file_globals;

#ifdef ZTS
#define FG(v) CREX_TSRMG(file_globals_id, crx_file_globals *, v)
extern CRXAPI int file_globals_id;
#else
#define FG(v) (file_globals.v)
extern CRXAPI crx_file_globals file_globals;
#endif


#endif /* FILE_H */
