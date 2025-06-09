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
#include "crx_ini.h"
#include "crx_win32_globals.h"

#define CRX_REGISTRY_KEY              "SOFTWARE\\CRX"

#define CRX_VER1(V1)                  #V1
#define CRX_VER2(V1,V2)               #V1"."#V2
#define CRX_VER3(V1,V2,V3)            #V1"."#V2"."#V3

#define CRX_REGISTRY_KEYV(VER)        CRX_REGISTRY_KEY"\\"VER
#define CRX_REGISTRY_KEY1(V1)         CRX_REGISTRY_KEY"\\"CRX_VER1(V1)
#define CRX_REGISTRY_KEY2(V1,V2)      CRX_REGISTRY_KEY"\\"CRX_VER2(V1,V2)
#define CRX_REGISTRY_KEY3(V1,V2,V3)   CRX_REGISTRY_KEY"\\"CRX_VER3(V1,V2,V3)

static const char* registry_keys[] = {
	CRX_REGISTRY_KEYV(CRX_VERSION),
	CRX_REGISTRY_KEY3(CRX_MAJOR_VERSION, CRX_MINOR_VERSION, CRX_RELEASE_VERSION),
	CRX_REGISTRY_KEY2(CRX_MAJOR_VERSION, CRX_MINOR_VERSION),
	CRX_REGISTRY_KEY1(CRX_MAJOR_VERSION),
	CRX_REGISTRY_KEY,
	NULL
};

static int OpenCrxRegistryKey(char* sub_key, HKEY *hKey)
{/*{{{*/
	const char **key_name = registry_keys;

	if (sub_key) {
		size_t main_key_len;
		size_t sub_key_len = strlen(sub_key);
		char *reg_key;

		while (*key_name) {
			LONG ret;

			main_key_len = strlen(*key_name);
			reg_key = emalloc(main_key_len + sub_key_len + 1);
			memcpy(reg_key, *key_name, main_key_len);
			memcpy(reg_key + main_key_len, sub_key, sub_key_len + 1);
			ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_key, 0, KEY_READ, hKey);
			efree(reg_key);

			if (ret == ERROR_SUCCESS) {
				return 1;
			}
			++key_name;
		}
	} else {
		while (*key_name) {
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, *key_name, 0, KEY_READ, hKey) == ERROR_SUCCESS) {
				return 1;
			}
			++key_name;
		}
	}
	return 0;
}/*}}}*/

static int LoadDirectory(HashTable *directories, HKEY key, char *path, int path_len, HashTable *parent_ht)
{/*{{{*/
	DWORD keys, values, max_key, max_name, max_value;
	int ret = 0;
	HashTable *ht = NULL;

	if (RegQueryInfoKey(key, NULL, NULL, NULL, &keys, &max_key, NULL, &values, &max_name, &max_value, NULL, NULL) == ERROR_SUCCESS) {

		if (values) {
			DWORD i;
			char *name = (char*)emalloc(max_name+1);
			char *value = (char*)emalloc(max_value+1);
			DWORD name_len, type, value_len;

			for (i = 0; i < values; i++) {
				name_len = max_name+1;
				value_len = max_value+1;

				memset(name, '\0', max_name+1);
				memset(value, '\0', max_value+1);

				if (RegEnumValue(key, i, name, &name_len, NULL, &type, value, &value_len) == ERROR_SUCCESS) {
					if ((type == REG_SZ) || (type == REG_EXPAND_SZ)) {
						zval data;

						if (!ht) {
							ht = (HashTable*)malloc(sizeof(HashTable));
							if (!ht) {
								return ret;
							}
							crex_hash_init(ht, 0, NULL, ZVAL_INTERNAL_PTR_DTOR, 1);
						}
						ZVAL_PSTRINGL(&data, value, value_len-1);
						crex_hash_str_update(ht, name, name_len, &data);
					}
				}
			}
			if (ht) {
				if (parent_ht) {
					crex_string *index;
					crex_ulong num;
					zval *tmpdata;

					CREX_HASH_MAP_FOREACH_KEY_VAL(parent_ht, num, index, tmpdata) {
						crex_hash_add(ht, index, tmpdata);
					} CREX_HASH_FOREACH_END();
				}
				crex_hash_str_update_mem(directories, path, path_len, ht, sizeof(HashTable));
				ret = 1;
			}

			efree(name);
			efree(value);
		}

		if (ht == NULL) {
			ht = parent_ht;
		}

		if (keys) {
			DWORD i;
			char *name = (char*)emalloc(max_key+1);
			char *new_path = (char*)emalloc(path_len+max_key+2);
			DWORD name_len;
			FILETIME t;
			HKEY subkey;

			for (i = 0; i < keys; i++) {
				name_len = max_key+1;
				if (RegEnumKeyEx(key, i, name, &name_len, NULL, NULL, NULL, &t) == ERROR_SUCCESS) {
					if (RegOpenKeyEx(key, name, 0, KEY_READ, &subkey) == ERROR_SUCCESS) {
						if (path_len) {
							memcpy(new_path, path, path_len);
							new_path[path_len] = '/';
							memcpy(new_path+path_len+1, name, name_len+1);
							crex_str_tolower(new_path, path_len+name_len+1);
							name_len += path_len+1;
						} else {
							memcpy(new_path, name, name_len+1);
							crex_str_tolower(new_path, name_len);
						}
						if (LoadDirectory(directories, subkey, new_path, name_len, ht)) {
							ret = 1;
						}
						RegCloseKey(subkey);
					}
				}
			}
			efree(new_path);
			efree(name);
		}
	}
	return ret;
}/*}}}*/

static void delete_internal_hashtable(zval *zv)
{/*{{{*/
	HashTable *ht = (HashTable *)C_PTR_P(zv);
	crex_hash_destroy(ht);
	free(ht);
}/*}}}*/

#define RegNotifyFlags (REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_ATTRIBUTES | REG_NOTIFY_CHANGE_LAST_SET)

void UpdateIniFromRegistry(char *path)
{/*{{{*/
	char *p, *orig_path;
	int path_len;

	if(!path) {
		return;
	}

	if (!PW32G(registry_directories)) {
		PW32G(registry_directories) = (HashTable*)malloc(sizeof(HashTable));
		if (!PW32G(registry_directories)) {
			return;
		}
		crex_hash_init(PW32G(registry_directories), 0, NULL, delete_internal_hashtable, 1);
		if (!OpenCrxRegistryKey("\\Per Directory Values", &PW32G(registry_key))) {
			PW32G(registry_key) = NULL;
			return;
		}
		PW32G(registry_event) = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (PW32G(registry_event)) {
			RegNotifyChangeKeyValue(PW32G(registry_key), TRUE, RegNotifyFlags, PW32G(registry_event), TRUE);
		}
		if (!LoadDirectory(PW32G(registry_directories), PW32G(registry_key), "", 0, NULL)) {
			return;
		}
	} else if (PW32G(registry_event) && WaitForSingleObject(PW32G(registry_event), 0) == WAIT_OBJECT_0) {
		RegNotifyChangeKeyValue(PW32G(registry_key), TRUE, RegNotifyFlags, PW32G(registry_event), TRUE);
		crex_hash_clean(PW32G(registry_directories));
		if (!LoadDirectory(PW32G(registry_directories), PW32G(registry_key), "", 0, NULL)) {
			return;
		}
	} else if (crex_hash_num_elements(PW32G(registry_directories)) == 0) {
		return;
	}

	orig_path = path = estrdup(path);

	/* Get rid of C:, if exists */
	p = strchr(path, ':');
	if (p) {
		*p = path[0];	/* replace the colon with the drive letter */
		path = p;		/* make path point to the drive letter */
	} else {
		if (path[0] != '\\' && path[0] != '/') {
			char tmp_buf[MAXPATHLEN], *cwd;
			char drive_letter;

			/* get current working directory and prepend it to the path */
			if (!VCWD_GETCWD(tmp_buf, MAXPATHLEN)) {
				efree(orig_path);
				return;
			}
			cwd = strchr(tmp_buf, ':');
			if (!cwd) {
				drive_letter = 'C';
				cwd = tmp_buf;
			} else {
				drive_letter = tmp_buf[0];
				cwd++;
			}
			while (*cwd == '\\' || *cwd == '/') {
				cwd++;
			}
			spprintf(&path, 0, "%c\\%s\\%s", drive_letter, cwd, orig_path);
			efree(orig_path);
			orig_path = path;
		}
	}

	path_len = 0;
	while (path[path_len] != 0) {
		if (path[path_len] == '\\') {
			path[path_len] = '/';
		}
		path_len++;
	}
	crex_str_tolower(path, path_len);

	while (path_len > 0) {
		HashTable *ht = (HashTable *)crex_hash_str_find_ptr(PW32G(registry_directories), path, path_len);

		if (ht != NULL) {
			crex_string *index;
			zval *data;

			CREX_HASH_MAP_FOREACH_STR_KEY_VAL(ht, index, data) {
				crex_alter_ini_entry(index, C_STR_P(data), CRX_INI_USER, CRX_INI_STAGE_ACTIVATE);
			} CREX_HASH_FOREACH_END();
		}

		do {
			path_len--;
		} while (path_len > 0 && path[path_len] != '/');
		path[path_len] = 0;
	}

	efree(orig_path);
}/*}}}*/

#define CRXRC_REGISTRY_NAME "IniFilePath"

char *GetIniPathFromRegistry()
{/*{{{*/
	char *reg_location = NULL;
	HKEY hKey;

	if (OpenCrxRegistryKey(NULL, &hKey)) {
		DWORD buflen = MAXPATHLEN;
		reg_location = emalloc(MAXPATHLEN+1);
		if(RegQueryValueEx(hKey, CRXRC_REGISTRY_NAME, 0, NULL, reg_location, &buflen) != ERROR_SUCCESS) {
			RegCloseKey(hKey);
			efree(reg_location);
			reg_location = NULL;
			return reg_location;
		}
		RegCloseKey(hKey);
	}
	return reg_location;
}/*}}}*/
