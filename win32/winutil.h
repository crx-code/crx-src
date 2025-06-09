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

#ifndef CRX_WIN32_WINUTIL_H
#define CRX_WIN32_WINUTIL_H

#ifdef CRX_EXPORTS
# define CRX_WINUTIL_API __declspec(dllexport)
#else
# define CRX_WINUTIL_API __declspec(dllimport)
#endif

CRX_WINUTIL_API char *crx_win32_error_to_msg(HRESULT error);
CRX_WINUTIL_API void crx_win32_error_msg_free(char *msg);

#define crx_win_err()	crx_win32_error_to_msg(GetLastError())
#define crx_win_err_free(err) crx_win32_error_msg_free(err)
int crx_win32_check_trailing_space(const char * path, const size_t path_len);
CRX_WINUTIL_API int crx_win32_get_random_bytes(unsigned char *buf, size_t size);
#ifdef CRX_EXPORTS
BOOL crx_win32_init_random_bytes(void);
BOOL crx_win32_shutdown_random_bytes(void);
#endif

#if !defined(ECURDIR)
# define ECURDIR        EACCES
#endif /* !ECURDIR */
#if !defined(ENOSYS)
# define ENOSYS         EPERM
#endif /* !ENOSYS */

CRX_WINUTIL_API int crx_win32_code_to_errno(unsigned long w32Err);

#define SET_ERRNO_FROM_WIN32_CODE(err) \
	do { \
	int ern = crx_win32_code_to_errno(err); \
	SetLastError(err); \
	_set_errno(ern); \
	} while (0)

CRX_WINUTIL_API char *crx_win32_get_username(void);

CRX_WINUTIL_API BOOL crx_win32_image_compatible(HMODULE handle, char **err);
CRX_WINUTIL_API BOOL crx_win32_crt_compatible(char **err);

#endif
