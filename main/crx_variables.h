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
   | Authors: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Zeev Suraski <zeev@crx.net>                                 |
   +----------------------------------------------------------------------+
*/

#ifndef CRX_VARIABLES_H
#define CRX_VARIABLES_H

#include "crx.h"
#include "SAPI.h"

#define PARSE_POST 0
#define PARSE_GET 1
#define PARSE_COOKIE 2
#define PARSE_STRING 3
#define PARSE_ENV 4
#define PARSE_SERVER 5
#define PARSE_SESSION 6

BEGIN_EXTERN_C()
void crx_startup_auto_globals(void);
extern CRXAPI void (*crx_import_environment_variables)(zval *array_ptr);
CRXAPI void crx_register_variable(const char *var, const char *val, zval *track_vars_array);
/* binary-safe version */
CRXAPI void crx_register_variable_safe(const char *var, const char *val, size_t val_len, zval *track_vars_array);
CRXAPI void crx_register_variable_ex(const char *var, zval *val, zval *track_vars_array);
CRXAPI void crx_register_known_variable(const char *var, size_t var_len, zval *value, zval *track_vars_array);

CRXAPI void crx_build_argv(const char *s, zval *track_vars_array);
CRXAPI int crx_hash_environment(void);
END_EXTERN_C()

#define NUM_TRACK_VARS	6

#endif /* CRX_VARIABLES_H */
