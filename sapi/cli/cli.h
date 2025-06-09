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
  | Author: Johannes Schlueter <johannes@crx.net>                        |
  +----------------------------------------------------------------------+
*/

#ifndef CLI_H
#define CLI_H

#ifdef CRX_WIN32
#   define CRX_CLI_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define CRX_CLI_API __attribute__ ((visibility("default")))
#else
#   define CRX_CLI_API
#endif


extern CRX_CLI_API ssize_t sapi_cli_single_write(const char *str, size_t str_length);

typedef struct  {
	size_t (*cli_shell_write)(const char *str, size_t str_length);
	size_t (*cli_shell_ub_write)(const char *str, size_t str_length);
	int (*cli_shell_run)(void);
} cli_shell_callbacks_t;

extern CRX_CLI_API cli_shell_callbacks_t *crx_cli_get_shell_callbacks(void);

#endif /* CLI_H */
