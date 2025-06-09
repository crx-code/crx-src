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
   | Author: Wez Furlong <wez@thebrainroom.com>                           |
   +----------------------------------------------------------------------+
 */

#ifdef CRX_WIN32
typedef HANDLE crx_file_descriptor_t;
typedef DWORD crx_process_id_t;
# define CRX_INVALID_FD INVALID_HANDLE_VALUE
#else
typedef int crx_file_descriptor_t;
typedef pid_t crx_process_id_t;
# define CRX_INVALID_FD (-1)
#endif

/* Environment block under Win32 is a NUL terminated sequence of NUL terminated
 *   name=value strings.
 * Under Unix, it is an argv style array. */
typedef struct _crx_process_env {
	char *envp;
#ifndef CRX_WIN32
	char **envarray;
#endif
} crx_process_env;

typedef struct _crx_process_handle {
	crx_process_id_t	child;
#ifdef CRX_WIN32
	HANDLE childHandle;
#endif
	int npipes;
	crex_resource **pipes;
	crex_string *command;
	crx_process_env env;
#if HAVE_SYS_WAIT_H
	/* We can only request the status once before it becomes unavailable.
	 * Cache the result so we can request it multiple times. */
	int cached_exit_wait_status_value;
	bool has_cached_exit_wait_status;
#endif
} crx_process_handle;
