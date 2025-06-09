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
   | Authors: Felipe Pena <felipe@crx.net>                                |
   | Authors: Joe Watkins <joe.watkins@live.co.uk>                        |
   | Authors: Bob Weinand <bwoebi@crx.net>                                |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_hash.h"
#include "crxdbg.h"
#include "crxdbg_bp.h"
#include "crxdbg_utils.h"
#include "crex_globals.h"
#include "ext/standard/crx_string.h"

CREX_EXTERN_MODULE_GLOBALS(crxdbg)

/* {{{ private api functions */
static inline crxdbg_breakbase_t *crxdbg_find_breakpoint_file(crex_op_array*);
static inline crxdbg_breakbase_t *crxdbg_find_breakpoint_symbol(crex_function*);
static inline crxdbg_breakbase_t *crxdbg_find_breakpoint_method(crex_op_array*);
static inline crxdbg_breakbase_t *crxdbg_find_breakpoint_opline(crxdbg_opline_ptr_t);
static inline crxdbg_breakbase_t *crxdbg_find_breakpoint_opcode(uint8_t);
static inline crxdbg_breakbase_t *crxdbg_find_conditional_breakpoint(crex_execute_data *execute_data); /* }}} */

/*
* Note:
*	A break point must always set the correct id and type
*	A set breakpoint function must always map new points
*/
static inline void _crxdbg_break_mapping(int id, HashTable *table) /* {{{ */
{
	crex_hash_index_update_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_MAP], id, table);
}
/* }}} */

#define CRXDBG_BREAK_MAPPING(id, table) _crxdbg_break_mapping(id, table)
#define CRXDBG_BREAK_UNMAPPING(id) \
	crex_hash_index_del(&CRXDBG_G(bp)[CRXDBG_BREAK_MAP], (id))

#define CRXDBG_BREAK_INIT(b, t) do {\
	memset(&b, 0, sizeof(b)); \
	b.id = CRXDBG_G(bp_count)++; \
	b.type = t; \
	b.disabled = 0;\
	b.hits = 0; \
} while(0)

static void crxdbg_file_breaks_dtor(zval *data) /* {{{ */
{
	crxdbg_breakfile_t *bp = (crxdbg_breakfile_t*) C_PTR_P(data);

	efree((char*)bp->filename);
	efree(bp);
} /* }}} */

static void crxdbg_class_breaks_dtor(zval *data) /* {{{ */
{
	crxdbg_breakmethod_t *bp = (crxdbg_breakmethod_t *) C_PTR_P(data);

	efree((char*)bp->class_name);
	efree((char*)bp->func_name);
	efree(bp);
} /* }}} */

static void crxdbg_opline_class_breaks_dtor(zval *data) /* {{{ */
{
	crex_hash_destroy(C_ARRVAL_P(data));
	efree(C_ARRVAL_P(data));
} /* }}} */

static void crxdbg_opline_breaks_dtor(zval *data) /* {{{ */
{
	crxdbg_breakopline_t *bp = (crxdbg_breakopline_t *) C_PTR_P(data);

	if (bp->class_name) {
		efree((char*)bp->class_name);
	}
	if (bp->func_name) {
		efree((char*)bp->func_name);
	}
	efree(bp);
} /* }}} */

CRXDBG_API void crxdbg_reset_breakpoints(void) /* {{{ */
{
	HashTable *table;

	CREX_HASH_FOREACH_PTR(&CRXDBG_G(bp)[CRXDBG_BREAK_MAP], table) {
		crxdbg_breakbase_t *brake;

		CREX_HASH_FOREACH_PTR(table, brake) {
			brake->hits = 0;
		} CREX_HASH_FOREACH_END();
	} CREX_HASH_FOREACH_END();
} /* }}} */

CRXDBG_API void crxdbg_export_breakpoints(FILE *handle) /* {{{ */
{
	char *string;
	crxdbg_export_breakpoints_to_string(&string);
	fputs(string, handle);
}
/* }}} */

CRXDBG_API void crxdbg_export_breakpoints_to_string(char **str) /* {{{ */
{
	HashTable *table;
	crex_ulong id = 0L;

	*str = "";

	if (crex_hash_num_elements(&CRXDBG_G(bp)[CRXDBG_BREAK_MAP])) {
		crxdbg_notice("Exporting %d breakpoints", crex_hash_num_elements(&CRXDBG_G(bp)[CRXDBG_BREAK_MAP]));

		/* this only looks like magic, it isn't */
		CREX_HASH_FOREACH_NUM_KEY_PTR(&CRXDBG_G(bp)[CRXDBG_BREAK_MAP], id, table) {
			crxdbg_breakbase_t *brake;

			CREX_HASH_FOREACH_PTR(table, brake) {
				if (brake->id == id) {
					char *new_str = NULL;

					switch (brake->type) {
						case CRXDBG_BREAK_FILE: {
							crex_string *filename = crx_addcslashes_str(((crxdbg_breakfile_t*)brake)->filename, strlen(((crxdbg_breakfile_t*)brake)->filename), "\\\"\n", 3);
							crxdbg_asprintf(&new_str,
								"%sbreak \"%s\":"CREX_ULONG_FMT"\n", *str,
								ZSTR_VAL(filename),
								((crxdbg_breakfile_t*)brake)->line);
							crex_string_release(filename);
						} break;

						case CRXDBG_BREAK_SYM: {
							crxdbg_asprintf(&new_str,
								"%sbreak %s\n", *str,
								((crxdbg_breaksymbol_t*)brake)->symbol);
						} break;

						case CRXDBG_BREAK_METHOD: {
							crxdbg_asprintf(&new_str,
								"%sbreak %s::%s\n", *str,
								((crxdbg_breakmethod_t*)brake)->class_name,
								((crxdbg_breakmethod_t*)brake)->func_name);
						} break;

						case CRXDBG_BREAK_METHOD_OPLINE: {
							crxdbg_asprintf(&new_str,
								"%sbreak %s::%s#"CREX_ULONG_FMT"\n", *str,
								((crxdbg_breakopline_t*)brake)->class_name,
								((crxdbg_breakopline_t*)brake)->func_name,
								((crxdbg_breakopline_t*)brake)->opline_num);
						} break;

						case CRXDBG_BREAK_FUNCTION_OPLINE: {
							crxdbg_asprintf(&new_str,
								"%sbreak %s#"CREX_ULONG_FMT"\n", *str,
								((crxdbg_breakopline_t*)brake)->func_name,
								((crxdbg_breakopline_t*)brake)->opline_num);
						} break;

						case CRXDBG_BREAK_FILE_OPLINE: {
							crex_string *filename = crx_addcslashes_str(((crxdbg_breakopline_t*)brake)->class_name, strlen(((crxdbg_breakopline_t*)brake)->class_name), "\\\"\n", 3);
							crxdbg_asprintf(&new_str,
								"%sbreak \"%s\":#"CREX_ULONG_FMT"\n", *str,
								ZSTR_VAL(filename),
								((crxdbg_breakopline_t*)brake)->opline_num);
							crex_string_release(filename);
						} break;

						case CRXDBG_BREAK_OPCODE: {
							crxdbg_asprintf(&new_str,
								"%sbreak %s\n", *str,
								((crxdbg_breakop_t*)brake)->name);
						} break;

						case CRXDBG_BREAK_COND: {
							crxdbg_breakcond_t *conditional = (crxdbg_breakcond_t*) brake;

							if (conditional->paramed) {
								switch (conditional->param.type) {
		                            case NUMERIC_FUNCTION_PARAM:
		                                crxdbg_asprintf(&new_str,
		                                    "%sbreak at %s#"CREX_ULONG_FMT" if %s\n",
		                                    *str, conditional->param.str, conditional->param.num, conditional->code);
		                            break;

		                            case NUMERIC_METHOD_PARAM:
		                                crxdbg_asprintf(&new_str,
		                                    "%sbreak at %s::%s#"CREX_ULONG_FMT" if %s\n",
		                                    *str, conditional->param.method.class, conditional->param.method.name, conditional->param.num, conditional->code);
		                            break;

		                            case ADDR_PARAM:
		                                crxdbg_asprintf(&new_str,
		                                    "%sbreak at 0X"CREX_ULONG_FMT" if %s\n",
		                                    *str, conditional->param.addr, conditional->code);
		                            break;

									case STR_PARAM:
										crxdbg_asprintf(&new_str,
											"%sbreak at %s if %s\n", *str, conditional->param.str, conditional->code);
									break;

									case METHOD_PARAM:
										crxdbg_asprintf(&new_str,
											"%sbreak at %s::%s if %s\n", *str,
											conditional->param.method.class, conditional->param.method.name,
											conditional->code);
									break;

									case FILE_PARAM: {
										crex_string *filename = crx_addcslashes_str(conditional->param.file.name, strlen(conditional->param.file.name), "\\\"\n", 3);
										crxdbg_asprintf(&new_str,
											"%sbreak at \"%s\":"CREX_ULONG_FMT" if %s\n", *str,
											ZSTR_VAL(filename), conditional->param.file.line,
											conditional->code);
										crex_string_release(filename);
									} break;

									default: { /* do nothing */ } break;
								}
							} else {
								crxdbg_asprintf(&new_str, "%sbreak if %s\n", str, conditional->code);
							}
						} break;

						default: continue;
					}

					if ((*str)[0]) {
						free(*str);
					}
					*str = new_str;
				}
			} CREX_HASH_FOREACH_END();
		} CREX_HASH_FOREACH_END();
	}

	if ((*str) && !(*str)[0]) {
		*str = NULL;
	}
} /* }}} */

CRXDBG_API void crxdbg_set_breakpoint_file(const char *path, size_t path_len, crex_ulong line_num) /* {{{ */
{
	crx_stream_statbuf ssb;
	char realpath[MAXPATHLEN];
	const char *original_path = path;
	bool pending = 0;
	crex_string *path_str;

	HashTable *broken, *file_breaks = &CRXDBG_G(bp)[CRXDBG_BREAK_FILE];
	crxdbg_breakfile_t new_break;

	if (!path_len) {
		if (VCWD_REALPATH(path, realpath)) {
			path = realpath;
		}
	}
	path_len = strlen(path);

	crxdbg_debug("file path: %s, resolved path: %s, was compiled: %d\n", original_path, path, crex_hash_str_exists(&CRXDBG_G(file_sources), path, path_len));

	if (!crex_hash_str_exists(&CRXDBG_G(file_sources), path, path_len)) {
		if (crx_stream_stat_path(path, &ssb) == FAILURE) {
			if (original_path[0] == '/') {
				crxdbg_error("Cannot stat %s, it does not exist", original_path);
				return;
			}

			file_breaks = &CRXDBG_G(bp)[CRXDBG_BREAK_FILE_PENDING];
			path = original_path;
			path_len = strlen(path);
			pending = 1;
		} else if (!(ssb.sb.st_mode & (S_IFREG|S_IFLNK))) {
			crxdbg_error("Cannot set breakpoint in %s, it is not a regular file", path);
			return;
		} else {
			crxdbg_debug("File exists, but not compiled\n");
		}
	}

	path_str = crex_string_init(path, path_len, 0);

	if (!(broken = crex_hash_find_ptr(file_breaks, path_str))) {
		HashTable breaks;
		crex_hash_init(&breaks, 8, NULL, crxdbg_file_breaks_dtor, 0);

		broken = crex_hash_add_mem(file_breaks, path_str, &breaks, sizeof(HashTable));
	}

	if (!crex_hash_index_exists(broken, line_num)) {
		CRXDBG_BREAK_INIT(new_break, CRXDBG_BREAK_FILE);
		new_break.filename = estrndup(path, path_len);
		new_break.line = line_num;

		crex_hash_index_update_mem(broken, line_num, &new_break, sizeof(crxdbg_breakfile_t));

		CRXDBG_BREAK_MAPPING(new_break.id, broken);

		if (pending) {
			crex_string *file;
			CREX_HASH_MAP_FOREACH_STR_KEY(&CRXDBG_G(file_sources), file) {
				HashTable *fileht;

				crxdbg_debug("Compare against loaded %s\n", ZSTR_VAL(file));

				if (!(pending = ((fileht = crxdbg_resolve_pending_file_break_ex(ZSTR_VAL(file), ZSTR_LEN(file), path_str, broken)) == NULL))) {
					new_break = *(crxdbg_breakfile_t *) crex_hash_index_find_ptr(fileht, line_num);
					break;
				}
			} CREX_HASH_FOREACH_END();
		}

		if (pending) {
			CRXDBG_G(flags) |= CRXDBG_HAS_PENDING_FILE_BP;

			crxdbg_notice("Pending breakpoint #%d added at %s:"CREX_ULONG_FMT"", new_break.id, new_break.filename, new_break.line);
		} else {
			CRXDBG_G(flags) |= CRXDBG_HAS_FILE_BP;

			crxdbg_notice("Breakpoint #%d added at %s:"CREX_ULONG_FMT"", new_break.id, new_break.filename, new_break.line);
		}
	} else {
		crxdbg_error("Breakpoint at %s:"CREX_ULONG_FMT" exists", path, line_num);
	}

	crex_string_release(path_str);
} /* }}} */

CRXDBG_API HashTable *crxdbg_resolve_pending_file_break_ex(const char *file, uint32_t filelen, crex_string *cur, HashTable *fileht) /* {{{ */
{
	crxdbg_debug("file: %s, filelen: %u, cur: %s, curlen %u, pos: %c, memcmp: %d\n", file, filelen, ZSTR_VAL(cur), ZSTR_LEN(cur), filelen > ZSTR_LEN(cur) ? file[filelen - ZSTR_LEN(cur) - 1] : '?', filelen > ZSTR_LEN(cur) ? memcmp(file + filelen - ZSTR_LEN(cur), ZSTR_VAL(cur), ZSTR_LEN(cur)) : 0);

#ifdef _WIN32
# define WIN32_PATH_CHECK file[filelen - ZSTR_LEN(cur) - 1] == '\\'
#else
# define WIN32_PATH_CHECK 0
#endif

	if (((ZSTR_LEN(cur) < filelen && (file[filelen - ZSTR_LEN(cur) - 1] == '/' || WIN32_PATH_CHECK)) || filelen == ZSTR_LEN(cur)) && !memcmp(file + filelen - ZSTR_LEN(cur), ZSTR_VAL(cur), ZSTR_LEN(cur))) {
		crxdbg_breakfile_t *brake, new_brake;
		HashTable *master;

		CRXDBG_G(flags) |= CRXDBG_HAS_FILE_BP;

		if (!(master = crex_hash_str_find_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE], file, filelen))) {
			HashTable new_ht;
			crex_hash_init(&new_ht, 8, NULL, crxdbg_file_breaks_dtor, 0);
			master = crex_hash_str_add_mem(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE], file, filelen, &new_ht, sizeof(HashTable));
		}

		CREX_HASH_FOREACH_PTR(fileht, brake) {
			new_brake = *brake;
			new_brake.filename = estrndup(file, filelen);
			CRXDBG_BREAK_UNMAPPING(brake->id);

			if (crex_hash_index_add_mem(master, brake->line, &new_brake, sizeof(crxdbg_breakfile_t))) {
				CRXDBG_BREAK_MAPPING(brake->id, master);
			}
		} CREX_HASH_FOREACH_END();

		crex_hash_del(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_PENDING], cur);

		if (!crex_hash_num_elements(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_PENDING])) {
			CRXDBG_G(flags) &= ~CRXDBG_HAS_PENDING_FILE_BP;
		}

		crxdbg_debug("compiled file: %s, cur bp file: %s\n", file, ZSTR_VAL(cur));

		return master;
	}

	return NULL;
} /* }}} */

CRXDBG_API void crxdbg_resolve_pending_file_break(const char *file) /* {{{ */
{
	HashTable *fileht;
	uint32_t filelen = strlen(file);
	crex_string *cur;

	crxdbg_debug("was compiled: %s\n", file);

	CREX_HASH_MAP_FOREACH_STR_KEY_PTR(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_PENDING], cur, fileht) {
		crxdbg_debug("check bp: %s\n", ZSTR_VAL(cur));

		crxdbg_resolve_pending_file_break_ex(file, filelen, cur, fileht);
	} CREX_HASH_FOREACH_END();
} /* }}} */

CRXDBG_API void crxdbg_set_breakpoint_symbol(const char *name, size_t name_len) /* {{{ */
{
	char *lcname;

	if (*name == '\\') {
		name++;
		name_len--;
	}

	lcname = crex_str_tolower_dup(name, name_len);

	if (!crex_hash_str_exists(&CRXDBG_G(bp)[CRXDBG_BREAK_SYM], name, name_len)) {
		crxdbg_breaksymbol_t new_break;

		CRXDBG_G(flags) |= CRXDBG_HAS_SYM_BP;

		CRXDBG_BREAK_INIT(new_break, CRXDBG_BREAK_SYM);
		new_break.symbol = estrndup(name, name_len);

		crex_hash_str_update_mem(&CRXDBG_G(bp)[CRXDBG_BREAK_SYM], lcname, name_len, &new_break, sizeof(crxdbg_breaksymbol_t));

		crxdbg_notice("Breakpoint #%d added at %s", new_break.id, new_break.symbol);

		CRXDBG_BREAK_MAPPING(new_break.id, &CRXDBG_G(bp)[CRXDBG_BREAK_SYM]);
	} else {
		crxdbg_error("Breakpoint exists at %s", name);
	}

	efree(lcname);
} /* }}} */

CRXDBG_API void crxdbg_set_breakpoint_method(const char *class_name, const char *func_name) /* {{{ */
{
	HashTable class_breaks, *class_table;
	size_t class_len = strlen(class_name);
	size_t func_len = strlen(func_name);
	char *func_lcname, *class_lcname;

	if (*class_name == '\\') {
		class_name++;
		class_len--;
	}

	func_lcname = crex_str_tolower_dup(func_name, func_len);
	class_lcname = crex_str_tolower_dup(class_name, class_len);

	if (!(class_table = crex_hash_str_find_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD], class_lcname, class_len))) {
		crex_hash_init(&class_breaks, 8, NULL, crxdbg_class_breaks_dtor, 0);
		class_table = crex_hash_str_update_mem(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD], class_lcname, class_len, &class_breaks, sizeof(HashTable));
	}

	if (!crex_hash_str_exists(class_table, func_lcname, func_len)) {
		crxdbg_breakmethod_t new_break;

		CRXDBG_G(flags) |= CRXDBG_HAS_METHOD_BP;

		CRXDBG_BREAK_INIT(new_break, CRXDBG_BREAK_METHOD);
		new_break.class_name = estrndup(class_name, class_len);
		new_break.class_len = class_len;
		new_break.func_name = estrndup(func_name, func_len);
		new_break.func_len = func_len;

		crex_hash_str_update_mem(class_table, func_lcname, func_len, &new_break, sizeof(crxdbg_breakmethod_t));

		crxdbg_notice("Breakpoint #%d added at %s::%s", new_break.id, class_name, func_name);

		CRXDBG_BREAK_MAPPING(new_break.id, class_table);
	} else {
		crxdbg_error("Breakpoint exists at %s::%s", class_name, func_name);
	}

	efree(func_lcname);
	efree(class_lcname);
} /* }}} */

CRXDBG_API void crxdbg_set_breakpoint_opline(crex_ulong opline) /* {{{ */
{
	if (!crex_hash_index_exists(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE], opline)) {
		crxdbg_breakline_t new_break;

		CRXDBG_G(flags) |= CRXDBG_HAS_OPLINE_BP;

		CRXDBG_BREAK_INIT(new_break, CRXDBG_BREAK_OPLINE);
		new_break.name = NULL;
		new_break.opline = opline;
		new_break.base = NULL;

		crex_hash_index_update_mem(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE], opline, &new_break, sizeof(crxdbg_breakline_t));

		crxdbg_notice("Breakpoint #%d added at #"CREX_ULONG_FMT, new_break.id, new_break.opline);
		CRXDBG_BREAK_MAPPING(new_break.id, &CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE]);
	} else {
		crxdbg_error("Breakpoint exists at #"CREX_ULONG_FMT, opline);
	}
} /* }}} */

CRXDBG_API int crxdbg_resolve_op_array_break(crxdbg_breakopline_t *brake, crex_op_array *op_array) /* {{{ */
{
	crxdbg_breakline_t opline_break;
	if (op_array->last <= brake->opline_num) {
		if (brake->class_name == NULL) {
			crxdbg_error("There are only %d oplines in function %s (breaking at opline "CREX_ULONG_FMT" impossible)", op_array->last, brake->func_name, brake->opline_num);
		} else if (brake->func_name == NULL) {
			crxdbg_error("There are only %d oplines in file %s (breaking at opline "CREX_ULONG_FMT" impossible)", op_array->last, brake->class_name, brake->opline_num);
		} else {
			crxdbg_error("There are only %d oplines in method %s::%s (breaking at opline "CREX_ULONG_FMT" impossible)", op_array->last, brake->class_name, brake->func_name, brake->opline_num);
		}

		return FAILURE;
	}

	opline_break.disabled = 0;
	opline_break.hits = 0;
	opline_break.id = brake->id;
	opline_break.opline = brake->opline = (crex_ulong)(op_array->opcodes + brake->opline_num);
	opline_break.name = NULL;
	opline_break.base = brake;
	if (op_array->scope) {
		opline_break.type = CRXDBG_BREAK_METHOD_OPLINE;
	} else if (op_array->function_name) {
		opline_break.type = CRXDBG_BREAK_FUNCTION_OPLINE;
	} else {
		opline_break.type = CRXDBG_BREAK_FILE_OPLINE;
	}

	CRXDBG_G(flags) |= CRXDBG_HAS_OPLINE_BP;

	crex_hash_index_update_mem(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE], opline_break.opline, &opline_break, sizeof(crxdbg_breakline_t));

	return SUCCESS;
} /* }}} */

CRXDBG_API void crxdbg_resolve_op_array_breaks(crex_op_array *op_array) /* {{{ */
{
	HashTable *func_table = &CRXDBG_G(bp)[CRXDBG_BREAK_FUNCTION_OPLINE];
	HashTable *oplines_table;
	crxdbg_breakopline_t *brake;

	if (op_array->scope != NULL && !(func_table = crex_hash_find_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD_OPLINE], op_array->scope->name))) {
		return;
	}

	if (op_array->function_name == NULL) {
		if (!(oplines_table = crex_hash_find_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_OPLINE], op_array->filename))) {
			return;
		}
	} else if (!op_array->function_name || !(oplines_table = crex_hash_find_ptr(func_table, op_array->function_name))) {
		return;
	}

	CREX_HASH_MAP_FOREACH_PTR(oplines_table, brake) {
		if (crxdbg_resolve_op_array_break(brake, op_array) == SUCCESS) {
			crxdbg_breakline_t *opline_break;

			crex_hash_internal_pointer_end(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE]);
			opline_break = crex_hash_get_current_data_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE]);

			crxdbg_notice("Breakpoint #%d resolved at %s%s%s#"CREX_ULONG_FMT" (opline #"CREX_ULONG_FMT")",
				opline_break->id,
				brake->class_name ? brake->class_name : "",
				brake->class_name && brake->func_name ? "::" : "",
				brake->func_name ? brake->func_name : "",
				brake->opline_num,
				opline_break->opline);
		}
	} CREX_HASH_FOREACH_END();
} /* }}} */

CRXDBG_API int crxdbg_resolve_opline_break(crxdbg_breakopline_t *new_break) /* {{{ */
{
	HashTable *func_table = EG(function_table);
	crex_function *func;

	if (new_break->func_name == NULL) {
		if (EG(current_execute_data) == NULL) {
			if (CRXDBG_G(ops) != NULL && !memcmp(CRXDBG_G(ops)->filename, new_break->class_name, new_break->class_len)) {
				if (crxdbg_resolve_op_array_break(new_break, CRXDBG_G(ops)) == SUCCESS) {
					return SUCCESS;
				} else {
					return 2;
				}
			}
			return FAILURE;
		} else {
			crex_execute_data *execute_data = EG(current_execute_data);
			do {
				if (CREX_USER_CODE(execute_data->func->common.type)) {
					crex_op_array *op_array = &execute_data->func->op_array;
					if (op_array->function_name == NULL && op_array->scope == NULL && new_break->class_len == ZSTR_LEN(op_array->filename) && !memcmp(ZSTR_VAL(op_array->filename), new_break->class_name, new_break->class_len)) {
						if (crxdbg_resolve_op_array_break(new_break, op_array) == SUCCESS) {
							return SUCCESS;
						} else {
							return 2;
						}
					}
				}
			} while ((execute_data = execute_data->prev_execute_data) != NULL);
			return FAILURE;
		}
	}

	if (new_break->class_name != NULL) {
		crex_class_entry *ce;
		if (!(ce = crex_hash_str_find_ptr(EG(class_table), crex_str_tolower_dup(new_break->class_name, new_break->class_len), new_break->class_len))) {
			return FAILURE;
		}
		func_table = &ce->function_table;
	}

	if (!(func = crex_hash_str_find_ptr(func_table, crex_str_tolower_dup(new_break->func_name, new_break->func_len), new_break->func_len))) {
		if (new_break->class_name != NULL && new_break->func_name != NULL) {
			crxdbg_error("Method %s doesn't exist in class %s", new_break->func_name, new_break->class_name);
			return 2;
		}
		return FAILURE;
	}

	if (func->type != CREX_USER_FUNCTION) {
		if (new_break->class_name == NULL) {
			crxdbg_error("%s is not a user defined function, no oplines exist", new_break->func_name);
		} else {
			crxdbg_error("%s::%s is not a user defined method, no oplines exist", new_break->class_name, new_break->func_name);
		}
		return 2;
	}

	if (crxdbg_resolve_op_array_break(new_break, &func->op_array) == FAILURE) {
		return 2;
	}

	return SUCCESS;
} /* }}} */

/* TODO ... method/function oplines need to be normalized (leading backslash, lowercase) and file oplines need to be resolved properly */

CRXDBG_API void crxdbg_set_breakpoint_method_opline(const char *class, const char *method, crex_ulong opline) /* {{{ */
{
	crxdbg_breakopline_t new_break;
	HashTable class_breaks, *class_table;
	HashTable method_breaks, *method_table;

	CRXDBG_BREAK_INIT(new_break, CRXDBG_BREAK_METHOD_OPLINE);
	new_break.func_len = strlen(method);
	new_break.func_name = estrndup(method, new_break.func_len);
	new_break.class_len = strlen(class);
	new_break.class_name = estrndup(class, new_break.class_len);
	new_break.opline_num = opline;
	new_break.opline = 0;

	switch (crxdbg_resolve_opline_break(&new_break)) {
		case FAILURE:
			crxdbg_notice("Pending breakpoint #%d at %s::%s#"CREX_ULONG_FMT, new_break.id, new_break.class_name, new_break.func_name, opline);
			break;

		case SUCCESS:
			crxdbg_notice("Breakpoint #%d added at %s::%s#"CREX_ULONG_FMT, new_break.id, new_break.class_name, new_break.func_name, opline);
			break;

		case 2:
			return;
	}

	if (!(class_table = crex_hash_str_find_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD_OPLINE], new_break.class_name, new_break.class_len))) {
		crex_hash_init(&class_breaks, 8, NULL, crxdbg_opline_class_breaks_dtor, 0);
		class_table = crex_hash_str_update_mem(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD_OPLINE], new_break.class_name, new_break.class_len, &class_breaks, sizeof(HashTable));
	}

	if (!(method_table = crex_hash_str_find_ptr(class_table, new_break.func_name, new_break.func_len))) {
		crex_hash_init(&method_breaks, 8, NULL, crxdbg_opline_breaks_dtor, 0);
		method_table = crex_hash_str_update_mem(class_table, new_break.func_name, new_break.func_len, &method_breaks, sizeof(HashTable));
	}

	if (crex_hash_index_exists(method_table, opline)) {
		crxdbg_error("Breakpoint already exists for %s::%s#"CREX_ULONG_FMT, new_break.class_name, new_break.func_name, opline);
		efree((char*)new_break.func_name);
		efree((char*)new_break.class_name);
		CRXDBG_G(bp_count)--;
		return;
	}

	CRXDBG_G(flags) |= CRXDBG_HAS_METHOD_OPLINE_BP;

	CRXDBG_BREAK_MAPPING(new_break.id, method_table);

	crex_hash_index_update_mem(method_table, opline, &new_break, sizeof(crxdbg_breakopline_t));
}
/* }}} */

CRXDBG_API void crxdbg_set_breakpoint_function_opline(const char *function, crex_ulong opline) /* {{{ */
{
	crxdbg_breakopline_t new_break;
	HashTable func_breaks, *func_table;

	CRXDBG_BREAK_INIT(new_break, CRXDBG_BREAK_FUNCTION_OPLINE);
	new_break.func_len = strlen(function);
	new_break.func_name = estrndup(function, new_break.func_len);
	new_break.class_len = 0;
	new_break.class_name = NULL;
	new_break.opline_num = opline;
	new_break.opline = 0;

	switch (crxdbg_resolve_opline_break(&new_break)) {
		case FAILURE:
			crxdbg_notice("Pending breakpoint #%d at %s#"CREX_ULONG_FMT, new_break.id, new_break.func_name, opline);
			break;

		case SUCCESS:
			crxdbg_notice("Breakpoint #%d added at %s#"CREX_ULONG_FMT, new_break.id, new_break.func_name, opline);
			break;

		case 2:
			return;
	}

	if (!(func_table = crex_hash_str_find_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_FUNCTION_OPLINE], new_break.func_name, new_break.func_len))) {
		crex_hash_init(&func_breaks, 8, NULL, crxdbg_opline_breaks_dtor, 0);
		func_table = crex_hash_str_update_mem(&CRXDBG_G(bp)[CRXDBG_BREAK_FUNCTION_OPLINE], new_break.func_name, new_break.func_len, &func_breaks, sizeof(HashTable));
	}

	if (crex_hash_index_exists(func_table, opline)) {
		crxdbg_error("Breakpoint already exists for %s#"CREX_ULONG_FMT, new_break.func_name, opline);
		efree((char*)new_break.func_name);
		CRXDBG_G(bp_count)--;
		return;
	}

	CRXDBG_BREAK_MAPPING(new_break.id, func_table);

	CRXDBG_G(flags) |= CRXDBG_HAS_FUNCTION_OPLINE_BP;

	crex_hash_index_update_mem(func_table, opline, &new_break, sizeof(crxdbg_breakopline_t));
}
/* }}} */

CRXDBG_API void crxdbg_set_breakpoint_file_opline(const char *file, crex_ulong opline) /* {{{ */
{
	crxdbg_breakopline_t new_break;
	HashTable file_breaks, *file_table;

	CRXDBG_BREAK_INIT(new_break, CRXDBG_BREAK_FILE_OPLINE);
	new_break.func_len = 0;
	new_break.func_name = NULL;
	new_break.class_len = strlen(file);
	new_break.class_name = estrndup(file, new_break.class_len);
	new_break.opline_num = opline;
	new_break.opline = 0;

	switch (crxdbg_resolve_opline_break(&new_break)) {
		case FAILURE:
			crxdbg_notice("Pending breakpoint #%d at %s:"CREX_ULONG_FMT, new_break.id, new_break.class_name, opline);
			break;

		case SUCCESS:
			crxdbg_notice("Breakpoint #%d added at %s:"CREX_ULONG_FMT, new_break.id, new_break.class_name, opline);
			break;

		case 2:
			return;
	}

	if (!(file_table = crex_hash_str_find_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_OPLINE], new_break.class_name, new_break.class_len))) {
		crex_hash_init(&file_breaks, 8, NULL, crxdbg_opline_breaks_dtor, 0);
		file_table = crex_hash_str_update_mem(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_OPLINE], new_break.class_name, new_break.class_len, &file_breaks, sizeof(HashTable));
	}

	if (crex_hash_index_exists(file_table, opline)) {
		crxdbg_error("Breakpoint already exists for %s:"CREX_ULONG_FMT, new_break.class_name, opline);
		efree((char*)new_break.class_name);
		CRXDBG_G(bp_count)--;
		return;
	}

	CRXDBG_BREAK_MAPPING(new_break.id, file_table);

	CRXDBG_G(flags) |= CRXDBG_HAS_FILE_OPLINE_BP;

	crex_hash_index_update_mem(file_table, opline, &new_break, sizeof(crxdbg_breakopline_t));
}
/* }}} */

CRXDBG_API void crxdbg_set_breakpoint_opcode(const char *name, size_t name_len) /* {{{ */
{
	crxdbg_breakop_t new_break;
	crex_ulong hash = crex_hash_func(name, name_len);

	if (crex_hash_index_exists(&CRXDBG_G(bp)[CRXDBG_BREAK_OPCODE], hash)) {
		crxdbg_error("Breakpoint exists for %s", name);
		return;
	}

	CRXDBG_BREAK_INIT(new_break, CRXDBG_BREAK_OPCODE);
	new_break.hash = hash;
	new_break.name = estrndup(name, name_len);

	crex_hash_index_update_mem(&CRXDBG_G(bp)[CRXDBG_BREAK_OPCODE], hash, &new_break, sizeof(crxdbg_breakop_t));

	CRXDBG_G(flags) |= CRXDBG_HAS_OPCODE_BP;

	crxdbg_notice("Breakpoint #%d added at %s", new_break.id, name);
	CRXDBG_BREAK_MAPPING(new_break.id, &CRXDBG_G(bp)[CRXDBG_BREAK_OPCODE]);
} /* }}} */

CRXDBG_API void crxdbg_set_breakpoint_opline_ex(crxdbg_opline_ptr_t opline) /* {{{ */
{
	if (!crex_hash_index_exists(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE], (crex_ulong) opline)) {
		crxdbg_breakline_t new_break;

		CRXDBG_G(flags) |= CRXDBG_HAS_OPLINE_BP;

		CRXDBG_BREAK_INIT(new_break, CRXDBG_BREAK_OPLINE);
		new_break.opline = (crex_ulong) opline;
		new_break.base = NULL;

		crex_hash_index_update_mem(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE], (crex_ulong) opline, &new_break, sizeof(crxdbg_breakline_t));

		crxdbg_notice("Breakpoint #%d added at #"CREX_ULONG_FMT, new_break.id, new_break.opline);
		CRXDBG_BREAK_MAPPING(new_break.id, &CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE]);
	} else {
		crxdbg_error("Breakpoint exists for opline #"CREX_ULONG_FMT, (crex_ulong) opline);
	}
} /* }}} */

static inline void crxdbg_create_conditional_break(crxdbg_breakcond_t *brake, const crxdbg_param_t *param, const char *expr, size_t expr_len, crex_ulong hash) /* {{{ */
{
	crxdbg_breakcond_t new_break;
	uint32_t cops = CG(compiler_options);
	crex_string *bp_code;

	if (param) {
		switch (param->type) {
			case STR_PARAM:
			case NUMERIC_FUNCTION_PARAM:
			case METHOD_PARAM:
			case NUMERIC_METHOD_PARAM:
			case FILE_PARAM:
			case ADDR_PARAM:
				/* do nothing */
			break;

			default:
				crxdbg_error("Invalid parameter type for conditional breakpoint");
				return;
		}
	}

	CRXDBG_BREAK_INIT(new_break, CRXDBG_BREAK_COND);
	new_break.hash = hash;

	if (param) {
		new_break.paramed = 1;
		crxdbg_copy_param(
			param, &new_break.param);
	    if (new_break.param.type == FILE_PARAM ||
	        new_break.param.type == NUMERIC_FILE_PARAM) {
	        char realpath[MAXPATHLEN];

	        if (VCWD_REALPATH(new_break.param.file.name, realpath)) {
	            efree(new_break.param.file.name);

	            new_break.param.file.name = estrdup(realpath);
	        } else {
	            crxdbg_error("Invalid file for conditional break %s", new_break.param.file.name);
	            crxdbg_clear_param(&new_break.param);
	            return;
	        }
	    }
	} else {
		new_break.paramed = 0;
	}

	cops = CG(compiler_options);

	CG(compiler_options) = CREX_COMPILE_DEFAULT_FOR_EVAL;

	new_break.code = estrndup(expr, expr_len);
	new_break.code_len = expr_len;

	bp_code = crex_string_concat3(
		"return ", sizeof("return ")-1, expr, expr_len, ";", sizeof(";")-1);
	new_break.ops = crex_compile_string(bp_code, "Conditional Breakpoint Code", CREX_COMPILE_POSITION_AFTER_OPEN_TAG);
	crex_string_release(bp_code);

	if (new_break.ops) {
		brake = crex_hash_index_update_mem(&CRXDBG_G(bp)[CRXDBG_BREAK_COND], hash, &new_break, sizeof(crxdbg_breakcond_t));

		crxdbg_notice("Conditional breakpoint #%d added %s/%p", brake->id, brake->code, brake->ops);

		CRXDBG_G(flags) |= CRXDBG_HAS_COND_BP;
		CRXDBG_BREAK_MAPPING(new_break.id, &CRXDBG_G(bp)[CRXDBG_BREAK_COND]);
	} else {
		 crxdbg_error("Failed to compile code for expression %s", expr);
		 efree((char*)new_break.code);
		 CRXDBG_G(bp_count)--;
	}

	CG(compiler_options) = cops;
} /* }}} */

CRXDBG_API void crxdbg_set_breakpoint_expression(const char *expr, size_t expr_len) /* {{{ */
{
	crex_ulong expr_hash = crex_hash_func(expr, expr_len);
	crxdbg_breakcond_t new_break;

	if (!crex_hash_index_exists(&CRXDBG_G(bp)[CRXDBG_BREAK_COND], expr_hash)) {
		crxdbg_create_conditional_break(
			&new_break, NULL, expr, expr_len, expr_hash);
	} else {
		crxdbg_error("Conditional break %s exists", expr);
	}
} /* }}} */

CRXDBG_API void crxdbg_set_breakpoint_at(const crxdbg_param_t *param) /* {{{ */
{
	crxdbg_breakcond_t new_break;
	crxdbg_param_t *condition;
	crex_ulong hash = 0L;

	if (param->next) {
		condition = param->next;
		hash = crex_hash_func(condition->str, condition->len);

		if (!crex_hash_index_exists(&CRXDBG_G(bp)[CRXDBG_BREAK_COND], hash)) {
			crxdbg_create_conditional_break(&new_break, param, condition->str, condition->len, hash);
		} else {
			crxdbg_notice("Conditional break %s exists at the specified location", condition->str);
		}
	}

} /* }}} */

static inline crxdbg_breakbase_t *crxdbg_find_breakpoint_file(crex_op_array *op_array) /* {{{ */
{
	HashTable *breaks;
	crxdbg_breakbase_t *brake;

#if 0
	crxdbg_debug("Op at: %.*s %d\n", ZSTR_LEN(op_array->filename), ZSTR_VAL(op_array->filename), (*EG(opline_ptr))->lineno);
#endif

	/* NOTE: realpath resolution should have happened at compile time - no reason to do it here again */
	if (!(breaks = crex_hash_find_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE], op_array->filename))) {
		return NULL;
	}

	if (EG(current_execute_data) && (brake = crex_hash_index_find_ptr(breaks, EG(current_execute_data)->opline->lineno))) {
		return brake;
	}

	return NULL;
} /* }}} */

static inline crxdbg_breakbase_t *crxdbg_find_breakpoint_symbol(crex_function *fbc) /* {{{ */
{
	crex_op_array *ops;

	if (fbc->type != CREX_USER_FUNCTION) {
		return NULL;
	}

	ops = (crex_op_array *) fbc;

	if (ops->scope) {
		/* find method breaks here */
		return crxdbg_find_breakpoint_method(ops);
	}

	if (ops->function_name) {
		crxdbg_breakbase_t *brake;
		crex_string *fname = crex_string_tolower(ops->function_name);

		brake = crex_hash_find_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_SYM], fname);

		crex_string_release(fname);
		return brake;
	} else {
		return crex_hash_str_find_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_SYM], CREX_STRL("main"));
	}
} /* }}} */

static inline crxdbg_breakbase_t *crxdbg_find_breakpoint_method(crex_op_array *ops) /* {{{ */
{
	HashTable *class_table;
	crxdbg_breakbase_t *brake = NULL;
	crex_string *class_lcname = crex_string_tolower(ops->scope->name);

	if ((class_table = crex_hash_find_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD], class_lcname))) {
		crex_string *lcname = crex_string_tolower(ops->function_name);

		brake = crex_hash_find_ptr(class_table, lcname);

		crex_string_release(lcname);
	}

	crex_string_release(class_lcname);
	return brake;
} /* }}} */

static inline crxdbg_breakbase_t *crxdbg_find_breakpoint_opline(crxdbg_opline_ptr_t opline) /* {{{ */
{
	crxdbg_breakline_t *brake;

	if ((brake = crex_hash_index_find_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE], (crex_ulong) opline)) && brake->base) {
		return (crxdbg_breakbase_t *)brake->base;
	}

	return (crxdbg_breakbase_t *) brake;
} /* }}} */

static inline crxdbg_breakbase_t *crxdbg_find_breakpoint_opcode(uint8_t opcode) /* {{{ */
{
	const char *opname = crex_get_opcode_name(opcode);

	if (!opname) {
		return NULL;
	}

	return crex_hash_index_find_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_OPCODE], crex_hash_func(opname, strlen(opname)));
} /* }}} */

static inline bool crxdbg_find_breakpoint_param(crxdbg_param_t *param, crex_execute_data *execute_data) /* {{{ */
{
	crex_function *function = execute_data->func;

	switch (param->type) {
		case NUMERIC_FUNCTION_PARAM:
		case STR_PARAM: {
			/* function breakpoint */

			if (function->type != CREX_USER_FUNCTION) {
				return 0;
			}

			{
				const char *str = NULL;
				size_t len = 0L;
				crex_op_array *ops = (crex_op_array*)function;
				str = ops->function_name ? ZSTR_VAL(ops->function_name) : "main";
				len = ops->function_name ? ZSTR_LEN(ops->function_name) : strlen(str);

				if (len == param->len && memcmp(param->str, str, len) == SUCCESS) {
					return param->type == STR_PARAM || execute_data->opline - ops->opcodes == param->num;
				}
			}
		} break;

		case FILE_PARAM: {
			if (param->file.line == crex_get_executed_lineno()) {
				const char *str = crex_get_executed_filename();
				size_t lengths[2] = {strlen(param->file.name), strlen(str)};

				if (lengths[0] == lengths[1]) {
					return (memcmp(
						param->file.name, str, lengths[0]) == SUCCESS);
				}
			}
		} break;

		case NUMERIC_METHOD_PARAM:
		case METHOD_PARAM: {
			if (function->type != CREX_USER_FUNCTION) {
				return 0;
			}

			{
				crex_op_array *ops = (crex_op_array*) function;

				if (ops->scope) {
					size_t lengths[2] = { strlen(param->method.class), ZSTR_LEN(ops->scope->name) };
					if (lengths[0] == lengths[1] && memcmp(param->method.class, ops->scope->name, lengths[0]) == SUCCESS) {
						lengths[0] = strlen(param->method.name);
						lengths[1] = ZSTR_LEN(ops->function_name);

						if (lengths[0] == lengths[1] && memcmp(param->method.name, ops->function_name, lengths[0]) == SUCCESS) {
							return param->type == METHOD_PARAM || (execute_data->opline - ops->opcodes) == param->num;
						}
					}
				}
			}
		} break;

		case ADDR_PARAM: {
			return ((crex_ulong)(crxdbg_opline_ptr_t)execute_data->opline == param->addr);
		} break;

		default: {
			/* do nothing */
		} break;
	}
	return 0;
} /* }}} */

static inline crxdbg_breakbase_t *crxdbg_find_conditional_breakpoint(crex_execute_data *execute_data) /* {{{ */
{
	crxdbg_breakcond_t *bp;
	int breakpoint = FAILURE;

	CREX_HASH_MAP_FOREACH_PTR(&CRXDBG_G(bp)[CRXDBG_BREAK_COND], bp) {
		zval retval;
		const crex_op *orig_opline = EG(current_execute_data)->opline;
		crex_function *orig_func = EG(current_execute_data)->func;
		zval *orig_retval = EG(current_execute_data)->return_value;

		if (((crxdbg_breakbase_t*)bp)->disabled) {
			continue;
		}

		if (bp->paramed) {
			if (!crxdbg_find_breakpoint_param(&bp->param, execute_data)) {
				continue;
			}
		}

		EG(no_extensions) = 1;

		crex_rebuild_symbol_table();

		crex_try {
			CRXDBG_G(flags) |= CRXDBG_IN_COND_BP;
			crex_execute(bp->ops, &retval);
			if (crex_is_true(&retval)) {
				breakpoint = SUCCESS;
			}
 		} crex_end_try();

		EG(no_extensions) = 1;
		EG(current_execute_data)->opline = orig_opline;
		EG(current_execute_data)->func = orig_func;
		EG(current_execute_data)->return_value = orig_retval;
		CRXDBG_G(flags) &= ~CRXDBG_IN_COND_BP;

		if (breakpoint == SUCCESS) {
			break;
		}
	} CREX_HASH_FOREACH_END();

	return (breakpoint == SUCCESS) ? ((crxdbg_breakbase_t *) bp) : NULL;
} /* }}} */

CRXDBG_API crxdbg_breakbase_t *crxdbg_find_breakpoint(crex_execute_data *execute_data) /* {{{ */
{
	crxdbg_breakbase_t *base = NULL;

	if (!(CRXDBG_G(flags) & CRXDBG_IS_BP_ENABLED)) {
		return NULL;
	}

	/* conditions cannot be executed by eval()'d code */
	if (!(CRXDBG_G(flags) & CRXDBG_IN_EVAL) &&
		(CRXDBG_G(flags) & CRXDBG_HAS_COND_BP) &&
		(base = crxdbg_find_conditional_breakpoint(execute_data))) {
		goto result;
	}

	if ((CRXDBG_G(flags) & CRXDBG_HAS_FILE_BP) && (base = crxdbg_find_breakpoint_file(&execute_data->func->op_array))) {
		goto result;
	}

	if (CRXDBG_G(flags) & (CRXDBG_HAS_METHOD_BP|CRXDBG_HAS_SYM_BP)) {
		crex_op_array *op_array = &execute_data->func->op_array;
		/* check we are at the beginning of the stack, but after argument RECV */
		if (execute_data->opline == op_array->opcodes + op_array->num_args + !!(op_array->fn_flags & CREX_ACC_VARIADIC)) {
			if ((base = crxdbg_find_breakpoint_symbol(execute_data->func))) {
				goto result;
			}
		}
	}

	if ((CRXDBG_G(flags) & CRXDBG_HAS_OPLINE_BP) && (base = crxdbg_find_breakpoint_opline((crxdbg_opline_ptr_t) execute_data->opline))) {
		goto result;
	}

	if ((CRXDBG_G(flags) & CRXDBG_HAS_OPCODE_BP) && (base = crxdbg_find_breakpoint_opcode(execute_data->opline->opcode))) {
		goto result;
	}

	return NULL;

result:
	/* we return nothing for disable breakpoints */
	if (base->disabled) {
		return NULL;
	}

	return base;
} /* }}} */

CRXDBG_API void crxdbg_delete_breakpoint(crex_ulong num) /* {{{ */
{
	HashTable *table;
	crxdbg_breakbase_t *brake;
	crex_string *strkey;
	crex_ulong numkey;

	if ((brake = crxdbg_find_breakbase_ex(num, &table, &numkey, &strkey))) {
		int type = brake->type;
		char *name = NULL;
		size_t name_len = 0L;

		switch (type) {
			case CRXDBG_BREAK_FILE:
			case CRXDBG_BREAK_METHOD:
				if (crex_hash_num_elements(table) == 1) {
					name = estrdup(brake->name);
					name_len = strlen(name);
					if (crex_hash_num_elements(&CRXDBG_G(bp)[type]) == 1) {
						CRXDBG_G(flags) &= ~(1<<(brake->type+1));
					}
				}
			break;

			default: {
				if (crex_hash_num_elements(table) == 1) {
					CRXDBG_G(flags) &= ~(1<<(brake->type+1));
				}
			}
		}

		switch (type) {
			case CRXDBG_BREAK_FILE_OPLINE:
			case CRXDBG_BREAK_FUNCTION_OPLINE:
			case CRXDBG_BREAK_METHOD_OPLINE:
				if (crex_hash_num_elements(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE]) == 1) {
					CRXDBG_G(flags) &= CRXDBG_HAS_OPLINE_BP;
				}
				crex_hash_index_del(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE], ((crxdbg_breakopline_t *) brake)->opline);
		}

		if (strkey) {
			crex_hash_del(table, strkey);
		} else {
			crex_hash_index_del(table, numkey);
		}

		switch (type) {
			case CRXDBG_BREAK_FILE:
			case CRXDBG_BREAK_METHOD:
				if (name) {
					crex_hash_str_del(&CRXDBG_G(bp)[type], name, name_len);
					efree(name);
				}
			break;
		}

		crxdbg_notice("Deleted breakpoint #"CREX_ULONG_FMT, num);
		CRXDBG_BREAK_UNMAPPING(num);
	} else {
		crxdbg_error("Failed to find breakpoint #"CREX_ULONG_FMT, num);
	}
} /* }}} */

CRXDBG_API void crxdbg_clear_breakpoints(void) /* {{{ */
{
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_PENDING]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_SYM]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD_OPLINE]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_FUNCTION_OPLINE]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_OPLINE]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_OPCODE]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_COND]);
	crex_hash_clean(&CRXDBG_G(bp)[CRXDBG_BREAK_MAP]);

	CRXDBG_G(flags) &= ~CRXDBG_BP_MASK;

	CRXDBG_G(bp_count) = 0;
} /* }}} */

CRXDBG_API void crxdbg_hit_breakpoint(crxdbg_breakbase_t *brake, bool output) /* {{{ */
{
	brake->hits++;

	if (output) {
		crxdbg_print_breakpoint(brake);
	}
} /* }}} */

CRXDBG_API void crxdbg_print_breakpoint(crxdbg_breakbase_t *brake) /* {{{ */
{
	if (!brake)
		goto unknown;

	switch (brake->type) {
		case CRXDBG_BREAK_FILE: {
			crxdbg_notice("Breakpoint #%d at %s:"CREX_ULONG_FMT", hits: "CREX_ULONG_FMT"",
				((crxdbg_breakfile_t*)brake)->id,
				((crxdbg_breakfile_t*)brake)->filename,
				((crxdbg_breakfile_t*)brake)->line,
				((crxdbg_breakfile_t*)brake)->hits);
		} break;

		case CRXDBG_BREAK_SYM: {
			crxdbg_notice("Breakpoint #%d in %s() at %s:%u, hits: "CREX_ULONG_FMT"",
				((crxdbg_breaksymbol_t*)brake)->id,
				((crxdbg_breaksymbol_t*)brake)->symbol,
				crex_get_executed_filename(),
				crex_get_executed_lineno(),
				((crxdbg_breakfile_t*)brake)->hits);
		} break;

		case CRXDBG_BREAK_OPLINE: {
			crxdbg_notice("Breakpoint #%d in #"CREX_ULONG_FMT" at %s:%u, hits: "CREX_ULONG_FMT"",
				((crxdbg_breakline_t*)brake)->id,
				((crxdbg_breakline_t*)brake)->opline,
				crex_get_executed_filename(),
				crex_get_executed_lineno(),
				((crxdbg_breakline_t*)brake)->hits);
		} break;

		case CRXDBG_BREAK_METHOD_OPLINE: {
			 crxdbg_notice("Breakpoint #%d in %s::%s()#"CREX_ULONG_FMT" at %s:%u, hits: "CREX_ULONG_FMT"",
				((crxdbg_breakopline_t*)brake)->id,
				((crxdbg_breakopline_t*)brake)->class_name,
				((crxdbg_breakopline_t*)brake)->func_name,
				((crxdbg_breakopline_t*)brake)->opline_num,
				crex_get_executed_filename(),
				crex_get_executed_lineno(),
				((crxdbg_breakopline_t*)brake)->hits);
		} break;

		case CRXDBG_BREAK_FUNCTION_OPLINE: {
			 crxdbg_notice("Breakpoint #%d in %s()#"CREX_ULONG_FMT" at %s:%u, hits: "CREX_ULONG_FMT"",
				((crxdbg_breakopline_t*)brake)->id,
				((crxdbg_breakopline_t*)brake)->func_name,
				((crxdbg_breakopline_t*)brake)->opline_num,
				crex_get_executed_filename(),
				crex_get_executed_lineno(),
				((crxdbg_breakopline_t*)brake)->hits);
		} break;

		case CRXDBG_BREAK_FILE_OPLINE: {
			 crxdbg_notice("Breakpoint #%d in #"CREX_ULONG_FMT" at %s:%u, hits: "CREX_ULONG_FMT"",
				((crxdbg_breakopline_t*)brake)->id,
				((crxdbg_breakopline_t*)brake)->opline_num,
				crex_get_executed_filename(),
				crex_get_executed_lineno(),
				((crxdbg_breakopline_t*)brake)->hits);
		} break;

		case CRXDBG_BREAK_OPCODE: {
			 crxdbg_notice("Breakpoint #%d in %s at %s:%u, hits: "CREX_ULONG_FMT"",
				((crxdbg_breakop_t*)brake)->id,
				((crxdbg_breakop_t*)brake)->name,
				crex_get_executed_filename(),
				crex_get_executed_lineno(),
				((crxdbg_breakop_t*)brake)->hits);
		} break;

		case CRXDBG_BREAK_METHOD: {
			 crxdbg_notice("Breakpoint #%d in %s::%s() at %s:%u, hits: "CREX_ULONG_FMT"",
				((crxdbg_breakmethod_t*)brake)->id,
				((crxdbg_breakmethod_t*)brake)->class_name,
				((crxdbg_breakmethod_t*)brake)->func_name,
				crex_get_executed_filename(),
				crex_get_executed_lineno(),
				((crxdbg_breakmethod_t*)brake)->hits);
		} break;

		case CRXDBG_BREAK_COND: {
			if (((crxdbg_breakcond_t*)brake)->paramed) {
				char *param;
				crxdbg_notice("Conditional breakpoint #%d: at %s if %s at %s:%u, hits: "CREX_ULONG_FMT"",
					((crxdbg_breakcond_t*)brake)->id,
					crxdbg_param_tostring(&((crxdbg_breakcond_t*)brake)->param, &param),
					((crxdbg_breakcond_t*)brake)->code,
					crex_get_executed_filename(),
					crex_get_executed_lineno(),
					((crxdbg_breakcond_t*)brake)->hits);
				if (param)
					free(param);
			} else {
				crxdbg_notice("Conditional breakpoint #%d: on %s == true at %s:%u, hits: "CREX_ULONG_FMT"",
					((crxdbg_breakcond_t*)brake)->id,
					((crxdbg_breakcond_t*)brake)->code,
					crex_get_executed_filename(),
					crex_get_executed_lineno(),
					((crxdbg_breakcond_t*)brake)->hits);
			}

		} break;

		default: {
unknown:
			crxdbg_notice("Unknown breakpoint at %s:%u",
				crex_get_executed_filename(),
				crex_get_executed_lineno());
		}
	}
} /* }}} */

CRXDBG_API void crxdbg_enable_breakpoint(crex_ulong id) /* {{{ */
{
	crxdbg_breakbase_t *brake = crxdbg_find_breakbase(id);

	if (brake) {
		brake->disabled = 0;
	}
} /* }}} */

CRXDBG_API void crxdbg_disable_breakpoint(crex_ulong id) /* {{{ */
{
	crxdbg_breakbase_t *brake = crxdbg_find_breakbase(id);

	if (brake) {
		brake->disabled = 1;
	}
} /* }}} */

CRXDBG_API void crxdbg_enable_breakpoints(void) /* {{{ */
{
	CRXDBG_G(flags) |= CRXDBG_IS_BP_ENABLED;
} /* }}} */

CRXDBG_API void crxdbg_disable_breakpoints(void) { /* {{{ */
	CRXDBG_G(flags) &= ~CRXDBG_IS_BP_ENABLED;
} /* }}} */

CRXDBG_API crxdbg_breakbase_t *crxdbg_find_breakbase(crex_ulong id) /* {{{ */
{
	HashTable *table;
	crex_string *strkey;
	crex_ulong numkey;

	return crxdbg_find_breakbase_ex(id, &table, &numkey, &strkey);
} /* }}} */

CRXDBG_API crxdbg_breakbase_t *crxdbg_find_breakbase_ex(crex_ulong id, HashTable **table, crex_ulong *numkey, crex_string **strkey) /* {{{ */
{
	if ((*table = crex_hash_index_find_ptr(&CRXDBG_G(bp)[CRXDBG_BREAK_MAP], id))) {
		crxdbg_breakbase_t *brake;

		CREX_HASH_FOREACH_KEY_PTR(*table, *numkey, *strkey, brake) {
			if (brake->id == id) {
				return brake;
			}
		} CREX_HASH_FOREACH_END();
	}

	return NULL;
} /* }}} */

CRXDBG_API void crxdbg_print_breakpoints(crex_ulong type) /* {{{ */
{
	switch (type) {
		case CRXDBG_BREAK_SYM: if ((CRXDBG_G(flags) & CRXDBG_HAS_SYM_BP)) {
			crxdbg_breaksymbol_t *brake;

			crxdbg_out(SEPARATE "\n");
			crxdbg_out("Function Breakpoints:\n");
			CREX_HASH_MAP_FOREACH_PTR(&CRXDBG_G(bp)[CRXDBG_BREAK_SYM], brake) {
				crxdbg_writeln("#%d\t\t%s%s",
					brake->id, brake->symbol,
					((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
			} CREX_HASH_FOREACH_END();
		} break;

		case CRXDBG_BREAK_METHOD: if ((CRXDBG_G(flags) & CRXDBG_HAS_METHOD_BP)) {
			HashTable *class_table;

			crxdbg_out(SEPARATE "\n");
			crxdbg_out("Method Breakpoints:\n");
			CREX_HASH_MAP_FOREACH_PTR(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD], class_table) {
				crxdbg_breakmethod_t *brake;

				CREX_HASH_MAP_FOREACH_PTR(class_table, brake) {
					crxdbg_writeln("#%d\t\t%s::%s%s",
						brake->id, brake->class_name, brake->func_name,
						((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
				} CREX_HASH_FOREACH_END();
			} CREX_HASH_FOREACH_END();
		} break;

		case CRXDBG_BREAK_FILE: if ((CRXDBG_G(flags) & CRXDBG_HAS_FILE_BP)) {
			HashTable *points;

			crxdbg_out(SEPARATE "\n");
			crxdbg_out("File Breakpoints:\n");
			CREX_HASH_MAP_FOREACH_PTR(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE], points) {
				crxdbg_breakfile_t *brake;

				CREX_HASH_MAP_FOREACH_PTR(points, brake) {
					crxdbg_writeln("#%d\t\t%s:"CREX_ULONG_FMT"%s",
						brake->id, brake->filename, brake->line,
						((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
				} CREX_HASH_FOREACH_END();
			} CREX_HASH_FOREACH_END();
		}  if ((CRXDBG_G(flags) & CRXDBG_HAS_PENDING_FILE_BP)) {
			HashTable *points;

			crxdbg_out(SEPARATE "\n");
			crxdbg_out("Pending File Breakpoints:\n");
			CREX_HASH_MAP_FOREACH_PTR(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_PENDING], points) {
				crxdbg_breakfile_t *brake;

				CREX_HASH_MAP_FOREACH_PTR(points, brake) {
					crxdbg_writeln("#%d\t\t%s:"CREX_ULONG_FMT"%s",
						brake->id, brake->filename, brake->line,
						((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
				} CREX_HASH_FOREACH_END();
			} CREX_HASH_FOREACH_END();
		} break;

		case CRXDBG_BREAK_OPLINE: if ((CRXDBG_G(flags) & CRXDBG_HAS_OPLINE_BP)) {
			crxdbg_breakline_t *brake;

			crxdbg_out(SEPARATE "\n");
			crxdbg_out("Opline Breakpoints:\n");
			CREX_HASH_MAP_FOREACH_PTR(&CRXDBG_G(bp)[CRXDBG_BREAK_OPLINE], brake) {
				const char *type;
				switch (brake->type) {
					case CRXDBG_BREAK_METHOD_OPLINE:
						type = "method";
						goto print_opline;
					case CRXDBG_BREAK_FUNCTION_OPLINE:
						type = "function";
						goto print_opline;
					case CRXDBG_BREAK_FILE_OPLINE:
						type = "method";

					print_opline: {
						if (brake->type == CRXDBG_BREAK_METHOD_OPLINE) {
							type = "method";
						} else if (brake->type == CRXDBG_BREAK_FUNCTION_OPLINE) {
							type = "function";
						} else if (brake->type == CRXDBG_BREAK_FILE_OPLINE) {
							type = "file";
						}

						crxdbg_writeln("#%d\t\t#"CREX_ULONG_FMT"\t\t(%s breakpoint)%s",
							brake->id, brake->opline, type,
							((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
					} break;

					default:
						crxdbg_writeln("#%d\t\t#"CREX_ULONG_FMT"%s",
							brake->id, brake->opline,
							((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
						break;
				}
			} CREX_HASH_FOREACH_END();
		} break;

		case CRXDBG_BREAK_METHOD_OPLINE: if ((CRXDBG_G(flags) & CRXDBG_HAS_METHOD_OPLINE_BP)) {
			HashTable *class_table, *method_table;

			crxdbg_out(SEPARATE "\n");
			crxdbg_out("Method opline Breakpoints:\n");
			CREX_HASH_MAP_FOREACH_PTR(&CRXDBG_G(bp)[CRXDBG_BREAK_METHOD_OPLINE], class_table) {
				CREX_HASH_MAP_FOREACH_PTR(class_table, method_table) {
					crxdbg_breakopline_t *brake;

					CREX_HASH_MAP_FOREACH_PTR(method_table, brake) {
						crxdbg_writeln("#%d\t\t%s::%s opline "CREX_ULONG_FMT"%s",
							brake->id, brake->class_name, brake->func_name, brake->opline_num,
							((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
					} CREX_HASH_FOREACH_END();
				} CREX_HASH_FOREACH_END();
			} CREX_HASH_FOREACH_END();
		} break;

		case CRXDBG_BREAK_FUNCTION_OPLINE: if ((CRXDBG_G(flags) & CRXDBG_HAS_FUNCTION_OPLINE_BP)) {
			HashTable *function_table;

			crxdbg_out(SEPARATE "\n");
			crxdbg_out("Function opline Breakpoints:\n");
			CREX_HASH_MAP_FOREACH_PTR(&CRXDBG_G(bp)[CRXDBG_BREAK_FUNCTION_OPLINE], function_table) {
				crxdbg_breakopline_t *brake;

				CREX_HASH_MAP_FOREACH_PTR(function_table, brake) {
					crxdbg_writeln("#%d\t\t%s opline "CREX_ULONG_FMT"%s",
						brake->id, brake->func_name, brake->opline_num,
						((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
				} CREX_HASH_FOREACH_END();
			} CREX_HASH_FOREACH_END();
		} break;

		case CRXDBG_BREAK_FILE_OPLINE: if ((CRXDBG_G(flags) & CRXDBG_HAS_FILE_OPLINE_BP)) {
			HashTable *file_table;

			crxdbg_out(SEPARATE "\n");
			crxdbg_out("File opline Breakpoints:\n");
			CREX_HASH_MAP_FOREACH_PTR(&CRXDBG_G(bp)[CRXDBG_BREAK_FILE_OPLINE], file_table) {
				crxdbg_breakopline_t *brake;

				CREX_HASH_MAP_FOREACH_PTR(file_table, brake) {
					crxdbg_writeln("#%d\t\t%s opline "CREX_ULONG_FMT"%s",
						brake->id, brake->class_name, brake->opline_num,
						((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
				} CREX_HASH_FOREACH_END();
			} CREX_HASH_FOREACH_END();
		} break;

		case CRXDBG_BREAK_COND: if ((CRXDBG_G(flags) & CRXDBG_HAS_COND_BP)) {
			crxdbg_breakcond_t *brake;

			crxdbg_out(SEPARATE "\n");
			crxdbg_out("Conditional Breakpoints:\n");
			CREX_HASH_MAP_FOREACH_PTR(&CRXDBG_G(bp)[CRXDBG_BREAK_COND], brake) {
				if (brake->paramed) {
					switch (brake->param.type) {
						case STR_PARAM:
							crxdbg_writeln("#%d\t\tat %s if %s%s",
				 				brake->id, brake->param.str, brake->code,
				 				((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
						break;

						case NUMERIC_FUNCTION_PARAM:
							crxdbg_writeln("#%d\t\tat %s#"CREX_ULONG_FMT" if %s%s",
				 				brake->id, brake->param.str, brake->param.num, brake->code,
				 				((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
						break;

						case METHOD_PARAM:
							crxdbg_writeln("#%d\t\tat %s::%s if %s%s",
				 				brake->id, brake->param.method.class, brake->param.method.name, brake->code,
				 				((crxdbg_breakbase_t*)brake)->disabled ? " [disabled]" : "");
						break;

						case NUMERIC_METHOD_PARAM:
							crxdbg_writeln("#%d\t\tat %s::%s#"CREX_ULONG_FMT" if %s%s",
				 				brake->id, brake->param.method.class, brake->param.method.name, brake->param.num, brake->code,
				 				((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
						break;

						case FILE_PARAM:
							crxdbg_writeln("#%d\t\tat %s:"CREX_ULONG_FMT" if %s%s",
				 				brake->id, brake->param.file.name, brake->param.file.line, brake->code,
				 				((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
						break;

						case ADDR_PARAM:
							crxdbg_writeln("#%d\t\tat #"CREX_ULONG_FMT" if %s%s",
				 				brake->id, brake->param.addr, brake->code,
				 				((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
						break;

						default:
							crxdbg_error("Invalid parameter type for conditional breakpoint");
						return;
					}
				} else {
					crxdbg_writeln("#%d\t\tif %s%s",
				 		brake->id, brake->code,
				 		((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
				}
			} CREX_HASH_FOREACH_END();
		} break;

		case CRXDBG_BREAK_OPCODE: if (CRXDBG_G(flags) & CRXDBG_HAS_OPCODE_BP) {
			crxdbg_breakop_t *brake;

			crxdbg_out(SEPARATE "\n");
			crxdbg_out("Opcode Breakpoints:\n");
			CREX_HASH_MAP_FOREACH_PTR(&CRXDBG_G(bp)[CRXDBG_BREAK_OPCODE], brake) {
				crxdbg_writeln("#%d\t\t%s%s",
					brake->id, brake->name,
					((crxdbg_breakbase_t *) brake)->disabled ? " [disabled]" : "");
			} CREX_HASH_FOREACH_END();
		} break;
	}
} /* }}} */
