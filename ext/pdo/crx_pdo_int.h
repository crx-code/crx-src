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
  | Author: Wez Furlong <wez@crx.net>                                    |
  |         Marcus Boerger <helly@crx.net>                               |
  |         Sterling Hughes <sterling@crx.net>                           |
  +----------------------------------------------------------------------+
*/

/* Stuff private to the PDO extension and not for consumption by PDO drivers
 * */

#include "crx_pdo_error.h"

extern HashTable pdo_driver_hash;
extern crex_class_entry *pdo_exception_ce;
int crx_pdo_list_entry(void);

void pdo_dbh_init(int module_number);
void pdo_stmt_init(void);

extern crex_object *pdo_dbh_new(crex_class_entry *ce);
extern const crex_function_entry pdo_dbh_functions[];
extern crex_class_entry *pdo_dbh_ce;
extern CREX_RSRC_DTOR_FUNC(crx_pdo_pdbh_dtor);

extern crex_object *pdo_dbstmt_new(crex_class_entry *ce);
extern const crex_function_entry pdo_dbstmt_functions[];
extern crex_class_entry *pdo_dbstmt_ce;
void pdo_dbstmt_free_storage(crex_object *std);
crex_object_iterator *pdo_stmt_iter_get(crex_class_entry *ce, zval *object, int by_ref);
extern crex_object_handlers pdo_dbstmt_object_handlers;
bool pdo_stmt_describe_columns(pdo_stmt_t *stmt);
bool pdo_stmt_setup_fetch_mode(pdo_stmt_t *stmt, crex_long mode, uint32_t mode_arg_num,
	zval *args, uint32_t variadic_num_args);

extern crex_object *pdo_row_new(crex_class_entry *ce);
extern const crex_function_entry pdo_row_functions[];
extern crex_class_entry *pdo_row_ce;
void pdo_row_free_storage(crex_object *std);
extern crex_object_handlers pdo_row_object_handlers;

crex_object_iterator *crx_pdo_dbstmt_iter_get(crex_class_entry *ce, zval *object);

extern pdo_driver_t *pdo_find_driver(const char *name, int namelen);

void pdo_sqlstate_init_error_table(void);
void pdo_sqlstate_fini_error_table(void);
const char *pdo_sqlstate_state_to_description(char *state);
bool pdo_hash_methods(pdo_dbh_object_t *dbh, int kind);
