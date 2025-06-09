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
   | Author: Zeev Suraski <zeev@crx.net>                                  |
   +----------------------------------------------------------------------+
 */

#include "crx.h"
#include "ext/standard/info.h"
#include "crex_ini.h"
#include "crex_ini_scanner.h"
#include "crx_ini.h"
#include "ext/standard/dl.h"
#include "crex_extensions.h"
#include "crex_highlight.h"
#include "SAPI.h"
#include "crx_main.h"
#include "crx_scandir.h"
#ifdef CRX_WIN32
#include "win32/crx_registry.h"
#include "win32/winutil.h"
#endif

#if HAVE_SCANDIR && HAVE_ALPHASORT && HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef CRX_WIN32
#define TRANSLATE_SLASHES_LOWER(path) \
	{ \
		char *tmp = path; \
		while (*tmp) { \
			if (*tmp == '\\') *tmp = '/'; \
			else *tmp = tolower(*tmp); \
				tmp++; \
		} \
	}
#else
#define TRANSLATE_SLASHES_LOWER(path)
#endif


typedef struct _crx_extension_lists {
	crex_llist engine;
	crex_llist functions;
} crx_extension_lists;

/* True globals */
static int is_special_section = 0;
static HashTable *active_ini_hash;
static HashTable configuration_hash;
static int has_per_dir_config = 0;
static int has_per_host_config = 0;
CRXAPI char *crx_ini_opened_path=NULL;
static crx_extension_lists extension_lists;
CRXAPI char *crx_ini_scanned_path=NULL;
CRXAPI char *crx_ini_scanned_files=NULL;

/* {{{ crx_ini_displayer_cb */
static CREX_COLD void crx_ini_displayer_cb(crex_ini_entry *ini_entry, int type)
{
	if (ini_entry->displayer) {
		ini_entry->displayer(ini_entry, type);
	} else {
		char *display_string;
		size_t display_string_length;
		int esc_html=0;

		if (type == CREX_INI_DISPLAY_ORIG && ini_entry->modified) {
			if (ini_entry->orig_value && ZSTR_VAL(ini_entry->orig_value)[0]) {
				display_string = ZSTR_VAL(ini_entry->orig_value);
				display_string_length = ZSTR_LEN(ini_entry->orig_value);
				esc_html = !sapi_module.crxinfo_as_text;
			} else {
				if (!sapi_module.crxinfo_as_text) {
					display_string = "<i>no value</i>";
					display_string_length = sizeof("<i>no value</i>") - 1;
				} else {
					display_string = "no value";
					display_string_length = sizeof("no value") - 1;
				}
			}
		} else if (ini_entry->value && ZSTR_VAL(ini_entry->value)[0]) {
			display_string = ZSTR_VAL(ini_entry->value);
			display_string_length = ZSTR_LEN(ini_entry->value);
			esc_html = !sapi_module.crxinfo_as_text;
		} else {
			if (!sapi_module.crxinfo_as_text) {
				display_string = "<i>no value</i>";
				display_string_length = sizeof("<i>no value</i>") - 1;
			} else {
				display_string = "no value";
				display_string_length = sizeof("no value") - 1;
			}
		}

		if (esc_html) {
			crx_html_puts(display_string, display_string_length);
		} else {
			CRXWRITE(display_string, display_string_length);
		}
	}
}
/* }}} */

/* {{{ display_ini_entries */
CRXAPI CREX_COLD void display_ini_entries(crex_module_entry *module)
{
	int module_number;
	crex_ini_entry *ini_entry;
	bool first = 1;

	if (module) {
		module_number = module->module_number;
	} else {
		module_number = 0;
	}

	CREX_HASH_MAP_FOREACH_PTR(EG(ini_directives), ini_entry) {
		if (ini_entry->module_number != module_number) {
			continue;
		}
		if (first) {
			crx_info_print_table_start();
			crx_info_print_table_header(3, "Directive", "Local Value", "Master Value");
			first = 0;
		}
		if (!sapi_module.crxinfo_as_text) {
			PUTS("<tr>");
			PUTS("<td class=\"e\">");
			CRXWRITE(ZSTR_VAL(ini_entry->name), ZSTR_LEN(ini_entry->name));
			PUTS("</td><td class=\"v\">");
			crx_ini_displayer_cb(ini_entry, CREX_INI_DISPLAY_ACTIVE);
			PUTS("</td><td class=\"v\">");
			crx_ini_displayer_cb(ini_entry, CREX_INI_DISPLAY_ORIG);
			PUTS("</td></tr>\n");
		} else {
			CRXWRITE(ZSTR_VAL(ini_entry->name), ZSTR_LEN(ini_entry->name));
			PUTS(" => ");
			crx_ini_displayer_cb(ini_entry, CREX_INI_DISPLAY_ACTIVE);
			PUTS(" => ");
			crx_ini_displayer_cb(ini_entry, CREX_INI_DISPLAY_ORIG);
			PUTS("\n");
		}
	} CREX_HASH_FOREACH_END();
	if (!first) {
		crx_info_print_table_end();
	}
}
/* }}} */

/* crx.ini support */
#define CRX_EXTENSION_TOKEN		"extension"
#define CREX_EXTENSION_TOKEN	"crex_extension"

/* {{{ config_zval_dtor */
CRXAPI void config_zval_dtor(zval *zvalue)
{
	if (C_TYPE_P(zvalue) == IS_ARRAY) {
		crex_hash_destroy(C_ARRVAL_P(zvalue));
		free(C_ARR_P(zvalue));
	} else if (C_TYPE_P(zvalue) == IS_STRING) {
		crex_string_release_ex(C_STR_P(zvalue), 1);
	}
}
/* Reset / free active_ini_section global */
#define RESET_ACTIVE_INI_HASH() do { \
	active_ini_hash = NULL;          \
	is_special_section = 0;          \
} while (0)
/* }}} */

/* {{{ crx_ini_parser_cb */
static void crx_ini_parser_cb(zval *arg1, zval *arg2, zval *arg3, int callback_type, HashTable *target_hash)
{
	zval *entry;
	HashTable *active_hash;
	char *extension_name;

	if (active_ini_hash) {
		active_hash = active_ini_hash;
	} else {
		active_hash = target_hash;
	}

	switch (callback_type) {
		case CREX_INI_PARSER_ENTRY: {
				if (!arg2) {
					/* bare string - nothing to do */
					break;
				}

				/* CRX and Crex extensions are not added into configuration hash! */
				if (!is_special_section && crex_string_equals_literal_ci(C_STR_P(arg1), CRX_EXTENSION_TOKEN)) { /* load CRX extension */
					extension_name = estrndup(C_STRVAL_P(arg2), C_STRLEN_P(arg2));
					crex_llist_add_element(&extension_lists.functions, &extension_name);
				} else if (!is_special_section && crex_string_equals_literal_ci(C_STR_P(arg1), CREX_EXTENSION_TOKEN)) { /* load Crex extension */
					extension_name = estrndup(C_STRVAL_P(arg2), C_STRLEN_P(arg2));
					crex_llist_add_element(&extension_lists.engine, &extension_name);

				/* All other entries are added into either configuration_hash or active ini section array */
				} else {
					/* Store in active hash */
					entry = crex_hash_update(active_hash, C_STR_P(arg1), arg2);
					C_STR_P(entry) = crex_string_dup(C_STR_P(entry), 1);
				}
			}
			break;

		case CREX_INI_PARSER_POP_ENTRY: {
				zval option_arr;
				zval *find_arr;

				if (!arg2) {
					/* bare string - nothing to do */
					break;
				}

/* fprintf(stdout, "CREX_INI_PARSER_POP_ENTRY: %s[%s] = %s\n",C_STRVAL_P(arg1), C_STRVAL_P(arg3), C_STRVAL_P(arg2)); */

				/* If option not found in hash or is not an array -> create array, otherwise add to existing array */
				if ((find_arr = crex_hash_find(active_hash, C_STR_P(arg1))) == NULL || C_TYPE_P(find_arr) != IS_ARRAY) {
					ZVAL_NEW_PERSISTENT_ARR(&option_arr);
					crex_hash_init(C_ARRVAL(option_arr), 8, NULL, config_zval_dtor, 1);
					find_arr = crex_hash_update(active_hash, C_STR_P(arg1), &option_arr);
				}

				/* arg3 is possible option offset name */
				if (arg3 && C_STRLEN_P(arg3) > 0) {
					entry = crex_symtable_update(C_ARRVAL_P(find_arr), C_STR_P(arg3), arg2);
				} else {
					entry = crex_hash_next_index_insert(C_ARRVAL_P(find_arr), arg2);
				}
				C_STR_P(entry) = crex_string_dup(C_STR_P(entry), 1);
			}
			break;

		case CREX_INI_PARSER_SECTION: { /* Create an array of entries of each section */

/* fprintf(stdout, "CREX_INI_PARSER_SECTION: %s\n",C_STRVAL_P(arg1)); */

				char *key = NULL;
				size_t key_len;

				/* PATH sections */
				if (!crex_binary_strncasecmp(C_STRVAL_P(arg1), C_STRLEN_P(arg1), "PATH", sizeof("PATH") - 1, sizeof("PATH") - 1)) {
					key = C_STRVAL_P(arg1);
					key = key + sizeof("PATH") - 1;
					key_len = C_STRLEN_P(arg1) - sizeof("PATH") + 1;
					is_special_section = 1;
					has_per_dir_config = 1;

					/* make the path lowercase on Windows, for case insensitivity. Does nothing for other platforms */
					TRANSLATE_SLASHES_LOWER(key);

				/* HOST sections */
				} else if (!crex_binary_strncasecmp(C_STRVAL_P(arg1), C_STRLEN_P(arg1), "HOST", sizeof("HOST") - 1, sizeof("HOST") - 1)) {
					key = C_STRVAL_P(arg1);
					key = key + sizeof("HOST") - 1;
					key_len = C_STRLEN_P(arg1) - sizeof("HOST") + 1;
					is_special_section = 1;
					has_per_host_config = 1;
					crex_str_tolower(key, key_len); /* host names are case-insensitive. */

				} else {
					is_special_section = 0;
				}

				if (key && key_len > 0) {
					/* Strip any trailing slashes */
					while (key_len > 0 && (key[key_len - 1] == '/' || key[key_len - 1] == '\\')) {
						key_len--;
						key[key_len] = 0;
					}

					/* Strip any leading whitespace and '=' */
					while (*key && (
						*key == '=' ||
						*key == ' ' ||
						*key == '\t'
					)) {
						key++;
						key_len--;
					}

					/* Search for existing entry and if it does not exist create one */
					if ((entry = crex_hash_str_find(target_hash, key, key_len)) == NULL) {
						zval section_arr;

						ZVAL_NEW_PERSISTENT_ARR(&section_arr);
						crex_hash_init(C_ARRVAL(section_arr), 8, NULL, (dtor_func_t) config_zval_dtor, 1);
						entry = crex_hash_str_update(target_hash, key, key_len, &section_arr);
					}
					if (C_TYPE_P(entry) == IS_ARRAY) {
						active_ini_hash = C_ARRVAL_P(entry);
					}
				}
			}
			break;
	}
}
/* }}} */

/* {{{ crx_load_crx_extension_cb */
static void crx_load_crx_extension_cb(void *arg)
{
#ifdef HAVE_LIBDL
	crx_load_extension(*((char **) arg), MODULE_PERSISTENT, 0);
#endif
}
/* }}} */

/* {{{ crx_load_crex_extension_cb */
#ifdef HAVE_LIBDL
static void crx_load_crex_extension_cb(void *arg)
{
	char *filename = *((char **) arg);
	const size_t length = strlen(filename);

#ifndef CRX_WIN32
	(void) length;
#endif

	if (IS_ABSOLUTE_PATH(filename, length)) {
		crex_load_extension(filename);
	} else {
		DL_HANDLE handle;
		char *libpath;
		char *extension_dir = INI_STR("extension_dir");
		int slash_suffix = 0;
		char *err1, *err2;

		if (extension_dir && extension_dir[0]) {
			slash_suffix = IS_SLASH(extension_dir[strlen(extension_dir)-1]);
		}

		/* Try as filename first */
		if (slash_suffix) {
			spprintf(&libpath, 0, "%s%s", extension_dir, filename); /* SAFE */
		} else {
			spprintf(&libpath, 0, "%s%c%s", extension_dir, DEFAULT_SLASH, filename); /* SAFE */
		}

		handle = (DL_HANDLE)crx_load_shlib(libpath, &err1);
		if (!handle) {
			/* If file does not exist, consider as extension name and build file name */
			char *orig_libpath = libpath;

			if (slash_suffix) {
				spprintf(&libpath, 0, "%s" CRX_SHLIB_EXT_PREFIX "%s." CRX_SHLIB_SUFFIX, extension_dir, filename); /* SAFE */
			} else {
				spprintf(&libpath, 0, "%s%c" CRX_SHLIB_EXT_PREFIX "%s." CRX_SHLIB_SUFFIX, extension_dir, DEFAULT_SLASH, filename); /* SAFE */
			}

			handle = (DL_HANDLE)crx_load_shlib(libpath, &err2);
			if (!handle) {
				crx_error(E_CORE_WARNING, "Failed loading Crex extension '%s' (tried: %s (%s), %s (%s))",
					filename, orig_libpath, err1, libpath, err2);
				efree(orig_libpath);
				efree(err1);
				efree(libpath);
				efree(err2);
				return;
			}

			efree(orig_libpath);
			efree(err1);
		}

#ifdef CRX_WIN32
		if (!crx_win32_image_compatible(handle, &err1)) {
				crx_error(E_CORE_WARNING, err1);
				efree(err1);
				efree(libpath);
				DL_UNLOAD(handle);
				return;
		}
#endif

		crex_load_extension_handle(handle, libpath);
		efree(libpath);
	}
}
#else
static void crx_load_crex_extension_cb(void *arg) { }
#endif
/* }}} */

static void append_ini_path(char *crx_ini_search_path, int search_path_size, const char *path)
{
	static const char paths_separator[] = { CREX_PATHS_SEPARATOR, 0 };

	if (*crx_ini_search_path) {
		strlcat(crx_ini_search_path, paths_separator, search_path_size);
	}

	strlcat(crx_ini_search_path, path, search_path_size);
}

/* {{{ crx_init_config */
int crx_init_config(void)
{
	char *crx_ini_file_name = NULL;
	char *crx_ini_search_path = NULL;
	int crx_ini_scanned_path_len;
	char *open_basedir;
	int free_ini_search_path = 0;
	crex_string *opened_path = NULL;

	crex_hash_init(&configuration_hash, 8, NULL, config_zval_dtor, 1);

	if (sapi_module.ini_defaults) {
		sapi_module.ini_defaults(&configuration_hash);
	}

	crex_llist_init(&extension_lists.engine, sizeof(char *), (llist_dtor_func_t) free_estring, 1);
	crex_llist_init(&extension_lists.functions, sizeof(char *), (llist_dtor_func_t) free_estring, 1);

	open_basedir = PG(open_basedir);

	if (sapi_module.crx_ini_path_override) {
		crx_ini_file_name = sapi_module.crx_ini_path_override;
		crx_ini_search_path = sapi_module.crx_ini_path_override;
		free_ini_search_path = 0;
	} else if (!sapi_module.crx_ini_ignore) {
		int search_path_size;
		char *default_location;
		char *env_location;
#ifdef CRX_WIN32
		char *reg_location;
		char crxrc_path[MAXPATHLEN];
#endif

		env_location = getenv("CRXRC");

#ifdef CRX_WIN32
		if (!env_location) {
			char dummybuf;
			int size;

			SetLastError(0);

			/*If the given buffer is not large enough to hold the data, the return value is
			the buffer size,  in characters, required to hold the string and its terminating
			null character. We use this return value to alloc the final buffer. */
			size = GetEnvironmentVariableA("CRXRC", &dummybuf, 0);
			if (GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
				/* The environment variable doesn't exist. */
				env_location = "";
			} else {
				if (size == 0) {
					env_location = "";
				} else {
					size = GetEnvironmentVariableA("CRXRC", crxrc_path, size);
					if (size == 0) {
						env_location = "";
					} else {
						env_location = crxrc_path;
					}
				}
			}
		}
#else
		if (!env_location) {
			env_location = "";
		}
#endif
		/*
		 * Prepare search path
		 */

		search_path_size = MAXPATHLEN * 4 + (int)strlen(env_location) + 3 + 1;
		crx_ini_search_path = (char *) emalloc(search_path_size);
		free_ini_search_path = 1;
		crx_ini_search_path[0] = 0;

		/* Add environment location */
		if (env_location[0]) {
			append_ini_path(crx_ini_search_path, search_path_size, env_location);
			crx_ini_file_name = env_location;
		}

#ifdef CRX_WIN32
		/* Add registry location */
		reg_location = GetIniPathFromRegistry();
		if (reg_location != NULL) {
			append_ini_path(crx_ini_search_path, search_path_size, reg_location);
			efree(reg_location);
		}
#endif

		/* Add cwd (not with CLI) */
		if (!sapi_module.crx_ini_ignore_cwd) {
			append_ini_path(crx_ini_search_path, search_path_size, ".");
		}

		if (PG(crx_binary)) {
			char *separator_location, *binary_location;

			binary_location = estrdup(PG(crx_binary));
			separator_location = strrchr(binary_location, DEFAULT_SLASH);

			if (separator_location && separator_location != binary_location) {
				*(separator_location) = 0;
			}
			append_ini_path(crx_ini_search_path, search_path_size, binary_location);

			efree(binary_location);
		}

		/* Add default location */
#ifdef CRX_WIN32
		default_location = (char *) emalloc(MAXPATHLEN + 1);

		if (0 < GetWindowsDirectory(default_location, MAXPATHLEN)) {
			append_ini_path(crx_ini_search_path, search_path_size, default_location);
		}

		/* For people running under terminal services, GetWindowsDirectory will
		 * return their personal Windows directory, so lets add the system
		 * windows directory too */
		if (0 < GetSystemWindowsDirectory(default_location, MAXPATHLEN)) {
			append_ini_path(crx_ini_search_path, search_path_size, default_location);
		}
		efree(default_location);

#else
		default_location = CRX_CONFIG_FILE_PATH;
		append_ini_path(crx_ini_search_path, search_path_size, default_location);
#endif
	}

	PG(open_basedir) = NULL;

	/*
	 * Find and open actual ini file
	 */

	FILE *fp = NULL;
	char *filename = NULL;
	bool free_filename = false;

	/* If SAPI does not want to ignore all ini files OR an overriding file/path is given.
	 * This allows disabling scanning for ini files in the CRX_CONFIG_FILE_SCAN_DIR but still
	 * load an optional ini file. */
	if (!sapi_module.crx_ini_ignore || sapi_module.crx_ini_path_override) {

		/* Check if crx_ini_file_name is a file and can be opened */
		if (crx_ini_file_name && crx_ini_file_name[0]) {
			crex_stat_t statbuf = {0};

			if (!VCWD_STAT(crx_ini_file_name, &statbuf)) {
				if (!((statbuf.st_mode & S_IFMT) == S_IFDIR)) {
					fp = VCWD_FOPEN(crx_ini_file_name, "r");
					if (fp) {
						filename = expand_filepath(crx_ini_file_name, NULL);
						free_filename = true;
					}
				}
			}
		}

		/* Otherwise search for crx-%sapi-module-name%.ini file in search path */
		if (!fp) {
			const char *fmt = "crx-%s.ini";
			char *ini_fname;
			spprintf(&ini_fname, 0, fmt, sapi_module.name);
			fp = crx_fopen_with_path(ini_fname, "r", crx_ini_search_path, &opened_path);
			efree(ini_fname);
			if (fp) {
				filename = ZSTR_VAL(opened_path);
			}
		}

		/* If still no ini file found, search for crx.ini file in search path */
		if (!fp) {
			fp = crx_fopen_with_path("crx.ini", "r", crx_ini_search_path, &opened_path);
			if (fp) {
				filename = ZSTR_VAL(opened_path);
			}
		}
	}

	if (free_ini_search_path) {
		efree(crx_ini_search_path);
	}

	PG(open_basedir) = open_basedir;

	if (fp) {
		crex_file_handle fh;
		crex_stream_init_fp(&fh, fp, filename);
		RESET_ACTIVE_INI_HASH();

		crex_parse_ini_file(&fh, 1, CREX_INI_SCANNER_NORMAL, (crex_ini_parser_cb_t) crx_ini_parser_cb, &configuration_hash);

		{
			zval tmp;

			ZVAL_NEW_STR(&tmp, crex_string_init(filename, strlen(filename), 1));
			crex_hash_str_update(&configuration_hash, "cfg_file_path", sizeof("cfg_file_path")-1, &tmp);
			if (opened_path) {
				crex_string_release_ex(opened_path, 0);
			}
			crx_ini_opened_path = crex_strndup(C_STRVAL(tmp), C_STRLEN(tmp));
		}
		crex_destroy_file_handle(&fh);

		if (free_filename) {
			efree(filename);
		}
	}

	/* Check for CRX_INI_SCAN_DIR environment variable to override/set config file scan directory */
	crx_ini_scanned_path = getenv("CRX_INI_SCAN_DIR");
	if (!crx_ini_scanned_path) {
		/* Or fall back using possible --with-config-file-scan-dir setting (defaults to empty string!) */
		crx_ini_scanned_path = CRX_CONFIG_FILE_SCAN_DIR;
	}
	crx_ini_scanned_path_len = (int)strlen(crx_ini_scanned_path);

	/* Scan and parse any .ini files found in scan path if path not empty. */
	if (!sapi_module.crx_ini_ignore && crx_ini_scanned_path_len) {
		struct dirent **namelist;
		int ndir, i;
		crex_stat_t sb = {0};
		char ini_file[MAXPATHLEN];
		char *p;
		crex_llist scanned_ini_list;
		crex_llist_element *element;
		int l, total_l = 0;
		char *bufpath, *debpath, *endpath;
		int lenpath;

		crex_llist_init(&scanned_ini_list, sizeof(char *), (llist_dtor_func_t) free_estring, 1);

		bufpath = estrdup(crx_ini_scanned_path);
		for (debpath = bufpath ; debpath ; debpath=endpath) {
			endpath = strchr(debpath, DEFAULT_DIR_SEPARATOR);
			if (endpath) {
				*(endpath++) = 0;
			}
			if (!debpath[0]) {
				/* empty string means default builtin value
				   to allow "/foo/crx.d:" or ":/foo/crx.d" */
				debpath = CRX_CONFIG_FILE_SCAN_DIR;
			}
			lenpath = (int)strlen(debpath);

			if (lenpath > 0 && (ndir = crx_scandir(debpath, &namelist, 0, crx_alphasort)) > 0) {

				for (i = 0; i < ndir; i++) {

					/* check for any file with .ini extension */
					if (!(p = strrchr(namelist[i]->d_name, '.')) || (p && strcmp(p, ".ini"))) {
						free(namelist[i]);
						continue;
					}
					/* Reset active ini section */
					RESET_ACTIVE_INI_HASH();

					if (IS_SLASH(debpath[lenpath - 1])) {
						snprintf(ini_file, MAXPATHLEN, "%s%s", debpath, namelist[i]->d_name);
					} else {
						snprintf(ini_file, MAXPATHLEN, "%s%c%s", debpath, DEFAULT_SLASH, namelist[i]->d_name);
					}
					if (VCWD_STAT(ini_file, &sb) == 0) {
						if (S_ISREG(sb.st_mode)) {
							crex_file_handle fh;
							FILE *file = VCWD_FOPEN(ini_file, "r");
							if (file) {
								crex_stream_init_fp(&fh, file, ini_file);
								if (crex_parse_ini_file(&fh, 1, CREX_INI_SCANNER_NORMAL, (crex_ini_parser_cb_t) crx_ini_parser_cb, &configuration_hash) == SUCCESS) {
									/* Here, add it to the list of ini files read */
									l = (int)strlen(ini_file);
									total_l += l + 2;
									p = estrndup(ini_file, l);
									crex_llist_add_element(&scanned_ini_list, &p);
								}
								crex_destroy_file_handle(&fh);
							}
						}
					}
					free(namelist[i]);
				}
				free(namelist);
			}
		}
		efree(bufpath);

		if (total_l) {
			int crx_ini_scanned_files_len = (crx_ini_scanned_files) ? (int)strlen(crx_ini_scanned_files) + 1 : 0;
			crx_ini_scanned_files = (char *) realloc(crx_ini_scanned_files, crx_ini_scanned_files_len + total_l + 1);
			if (!crx_ini_scanned_files_len) {
				*crx_ini_scanned_files = '\0';
			}
			total_l += crx_ini_scanned_files_len;
			for (element = scanned_ini_list.head; element; element = element->next) {
				if (crx_ini_scanned_files_len) {
					strlcat(crx_ini_scanned_files, ",\n", total_l);
				}
				strlcat(crx_ini_scanned_files, *(char **)element->data, total_l);
				strlcat(crx_ini_scanned_files, element->next ? ",\n" : "\n", total_l);
			}
		}
		crex_llist_destroy(&scanned_ini_list);
	} else {
		/* Make sure an empty crx_ini_scanned_path ends up as NULL */
		crx_ini_scanned_path = NULL;
	}

	if (sapi_module.ini_entries) {
		/* Reset active ini section */
		RESET_ACTIVE_INI_HASH();
		crex_parse_ini_string(sapi_module.ini_entries, 1, CREX_INI_SCANNER_NORMAL, (crex_ini_parser_cb_t) crx_ini_parser_cb, &configuration_hash);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ crx_shutdown_config */
int crx_shutdown_config(void)
{
	crex_hash_destroy(&configuration_hash);
	if (crx_ini_opened_path) {
		free(crx_ini_opened_path);
		crx_ini_opened_path = NULL;
	}
	if (crx_ini_scanned_files) {
		free(crx_ini_scanned_files);
		crx_ini_scanned_files = NULL;
	}
	return SUCCESS;
}
/* }}} */

/* {{{ crx_ini_register_extensions */
void crx_ini_register_extensions(void)
{
	crex_llist_apply(&extension_lists.engine, crx_load_crex_extension_cb);
	crex_llist_apply(&extension_lists.functions, crx_load_crx_extension_cb);

	crex_llist_destroy(&extension_lists.engine);
	crex_llist_destroy(&extension_lists.functions);
}
/* }}} */

/* {{{ crx_parse_user_ini_file */
CRXAPI int crx_parse_user_ini_file(const char *dirname, const char *ini_filename, HashTable *target_hash)
{
	crex_stat_t sb = {0};
	char ini_file[MAXPATHLEN];

	snprintf(ini_file, MAXPATHLEN, "%s%c%s", dirname, DEFAULT_SLASH, ini_filename);

	if (VCWD_STAT(ini_file, &sb) == 0) {
		if (S_ISREG(sb.st_mode)) {
			crex_file_handle fh;
			int ret = FAILURE;

			crex_stream_init_fp(&fh, VCWD_FOPEN(ini_file, "r"), ini_file);
			if (fh.handle.fp) {
				/* Reset active ini section */
				RESET_ACTIVE_INI_HASH();

#if CREX_RC_DEBUG
				/* User inis are parsed during SAPI activate (part of the request),
				 * but persistently allocated to allow caching. This is fine as long as
				 * strings are duplicated in crx_ini_activate_config(). */
				bool orig_rc_debug = crex_rc_debug;
				crex_rc_debug = false;
#endif
				ret = crex_parse_ini_file(&fh, 1, CREX_INI_SCANNER_NORMAL, (crex_ini_parser_cb_t) crx_ini_parser_cb, target_hash);
#if CREX_RC_DEBUG
				crex_rc_debug = orig_rc_debug;
#endif
				if (ret == SUCCESS) {
					/* FIXME: Add parsed file to the list of user files read? */
				}
			}
			crex_destroy_file_handle(&fh);
			return ret;
		}
	}
	return FAILURE;
}
/* }}} */

/* {{{ crx_ini_activate_config */
CRXAPI void crx_ini_activate_config(HashTable *source_hash, int modify_type, int stage)
{
	crex_string *str;
	zval *data;

	/* Walk through config hash and alter matching ini entries using the values found in the hash */
	CREX_HASH_MAP_FOREACH_STR_KEY_VAL(source_hash, str, data) {
		crex_string *data_str = crex_string_dup(C_STR_P(data), 0);
		crex_alter_ini_entry_ex(str, data_str, modify_type, stage, 0);
		crex_string_release(data_str);
	} CREX_HASH_FOREACH_END();
}
/* }}} */

/* {{{ crx_ini_has_per_dir_config */
CRXAPI int crx_ini_has_per_dir_config(void)
{
	return has_per_dir_config;
}
/* }}} */

/* {{{ crx_ini_activate_per_dir_config */
CRXAPI void crx_ini_activate_per_dir_config(char *path, size_t path_len)
{
	zval *tmp2;
	char *ptr;

#ifdef CRX_WIN32
	char path_bak[MAXPATHLEN];
#endif

#ifdef CRX_WIN32
	/* MAX_PATH is \0-terminated, path_len == MAXPATHLEN would overrun path_bak */
	if (path_len >= MAXPATHLEN) {
#else
	if (path_len > MAXPATHLEN) {
#endif
		return;
	}

#ifdef CRX_WIN32
	memcpy(path_bak, path, path_len);
	path_bak[path_len] = 0;
	TRANSLATE_SLASHES_LOWER(path_bak);
	path = path_bak;
#endif

	/* Walk through each directory in path and apply any found per-dir-system-configuration from configuration_hash */
	if (has_per_dir_config && path && path_len) {
		ptr = path + 1;
		while ((ptr = strchr(ptr, '/')) != NULL) {
			*ptr = 0;
			/* Search for source array matching the path from configuration_hash */
			if ((tmp2 = crex_hash_str_find(&configuration_hash, path, strlen(path))) != NULL) {
				crx_ini_activate_config(C_ARRVAL_P(tmp2), CRX_INI_SYSTEM, CRX_INI_STAGE_ACTIVATE);
			}
			*ptr = '/';
			ptr++;
		}
	}
}
/* }}} */

/* {{{ crx_ini_has_per_host_config */
CRXAPI int crx_ini_has_per_host_config(void)
{
	return has_per_host_config;
}
/* }}} */

/* {{{ crx_ini_activate_per_host_config */
CRXAPI void crx_ini_activate_per_host_config(const char *host, size_t host_len)
{
	zval *tmp;

	if (has_per_host_config && host && host_len) {
		/* Search for source array matching the host from configuration_hash */
		if ((tmp = crex_hash_str_find(&configuration_hash, host, host_len)) != NULL) {
			crx_ini_activate_config(C_ARRVAL_P(tmp), CRX_INI_SYSTEM, CRX_INI_STAGE_ACTIVATE);
		}
	}
}
/* }}} */

/* {{{ cfg_get_entry */
CRXAPI zval *cfg_get_entry_ex(crex_string *name)
{
	return crex_hash_find(&configuration_hash, name);
}
/* }}} */

/* {{{ cfg_get_entry */
CRXAPI zval *cfg_get_entry(const char *name, size_t name_length)
{
	return crex_hash_str_find(&configuration_hash, name, name_length);
}
/* }}} */

/* {{{ cfg_get_long */
CRXAPI int cfg_get_long(const char *varname, crex_long *result)
{
	zval *tmp;

	if ((tmp = crex_hash_str_find(&configuration_hash, varname, strlen(varname))) == NULL) {
		*result = 0;
		return FAILURE;
	}
	*result = zval_get_long(tmp);
	return SUCCESS;
}
/* }}} */

/* {{{ cfg_get_double */
CRXAPI int cfg_get_double(const char *varname, double *result)
{
	zval *tmp;

	if ((tmp = crex_hash_str_find(&configuration_hash, varname, strlen(varname))) == NULL) {
		*result = (double) 0;
		return FAILURE;
	}
	*result = zval_get_double(tmp);
	return SUCCESS;
}
/* }}} */

/* {{{ cfg_get_string */
CRXAPI int cfg_get_string(const char *varname, char **result)
{
	zval *tmp;

	if ((tmp = crex_hash_str_find(&configuration_hash, varname, strlen(varname))) == NULL) {
		*result = NULL;
		return FAILURE;
	}
	*result = C_STRVAL_P(tmp);
	return SUCCESS;
}
/* }}} */

CRXAPI HashTable* crx_ini_get_configuration_hash(void) /* {{{ */
{
	return &configuration_hash;
} /* }}} */
