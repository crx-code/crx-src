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
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

#ifndef RFC1867_H
#define RFC1867_H

#include "SAPI.h"

#define MULTIPART_CONTENT_TYPE "multipart/form-data"
#define MULTIPART_EVENT_START		0
#define MULTIPART_EVENT_FORMDATA	1
#define MULTIPART_EVENT_FILE_START	2
#define MULTIPART_EVENT_FILE_DATA	3
#define MULTIPART_EVENT_FILE_END	4
#define MULTIPART_EVENT_END		5

/* Errors */
#define CRX_UPLOAD_ERROR_OK   0  /* File upload successful */
#define CRX_UPLOAD_ERROR_A    1  /* Uploaded file exceeded upload_max_filesize */
#define CRX_UPLOAD_ERROR_B    2  /* Uploaded file exceeded MAX_FILE_SIZE */
#define CRX_UPLOAD_ERROR_C    3  /* Partially uploaded */
#define CRX_UPLOAD_ERROR_D    4  /* No file uploaded */
#define CRX_UPLOAD_ERROR_E    6  /* Missing /tmp or similar directory */
#define CRX_UPLOAD_ERROR_F    7  /* Failed to write file to disk */
#define CRX_UPLOAD_ERROR_X    8  /* File upload stopped by extension */

typedef struct _multipart_event_start {
	size_t	content_length;
} multipart_event_start;

typedef struct _multipart_event_formdata {
	size_t	post_bytes_processed;
	char	*name;
	char	**value;
	size_t	length;
	size_t	*newlength;
} multipart_event_formdata;

typedef struct _multipart_event_file_start {
	size_t	post_bytes_processed;
	char	*name;
	char	**filename;
} multipart_event_file_start;

typedef struct _multipart_event_file_data {
	size_t	post_bytes_processed;
	crex_off_t	offset;
	char	*data;
	size_t	length;
	size_t	*newlength;
} multipart_event_file_data;

typedef struct _multipart_event_file_end {
	size_t	post_bytes_processed;
	char	*temp_filename;
	int	cancel_upload;
} multipart_event_file_end;

typedef struct _multipart_event_end {
	size_t	post_bytes_processed;
} multipart_event_end;

typedef int (*crx_rfc1867_encoding_translation_t)(void);
typedef void (*crx_rfc1867_get_detect_order_t)(const crex_encoding ***list, size_t *list_size);
typedef void (*crx_rfc1867_set_input_encoding_t)(const crex_encoding *encoding);
typedef char* (*crx_rfc1867_getword_t)(const crex_encoding *encoding, char **line, char stop);
typedef char* (*crx_rfc1867_getword_conf_t)(const crex_encoding *encoding, char *str);
typedef char* (*crx_rfc1867_basename_t)(const crex_encoding *encoding, char *str);

SAPI_API SAPI_POST_HANDLER_FUNC(rfc1867_post_handler);

CRXAPI void destroy_uploaded_files_hash(void);
extern CRXAPI crex_result (*crx_rfc1867_callback)(unsigned int event, void *event_data, void **extra);

SAPI_API void crx_rfc1867_set_multibyte_callbacks(
					crx_rfc1867_encoding_translation_t encoding_translation,
					crx_rfc1867_get_detect_order_t get_detect_order,
					crx_rfc1867_set_input_encoding_t set_input_encoding,
					crx_rfc1867_getword_t getword,
					crx_rfc1867_getword_conf_t getword_conf,
					crx_rfc1867_basename_t basename);

#endif /* RFC1867_H */
