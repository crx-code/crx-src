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
   | Author: Vlad Krupin <crxdevel@echospace.com>                         |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#ifdef HAVE_PSPELL

/* this will enforce compatibility in .12 version (broken after .11.2) */
#define USE_ORIGINAL_MANAGER_FUNCS

#include "crx_pspell.h"
#include <pspell.h>
#include "ext/standard/info.h"

#define PSPELL_FAST 1L
#define PSPELL_NORMAL 2L
#define PSPELL_BAD_SPELLERS 3L
#define PSPELL_SPEED_MASK_INTERNAL 3L
#define PSPELL_RUN_TOGETHER 8L

#include "pspell_arginfo.h"

/* Largest ignored word can be 999 characters (this seems sane enough),
 * and it takes 3 bytes to represent that (see pspell_config_ignore)
 */
#define PSPELL_LARGEST_WORD 3

static CRX_MINIT_FUNCTION(pspell);
static CRX_MINFO_FUNCTION(pspell);

static crex_class_entry *crx_pspell_ce = NULL;
static crex_object_handlers crx_pspell_handlers;
static crex_class_entry *crx_pspell_config_ce = NULL;
static crex_object_handlers crx_pspell_config_handlers;

crex_module_entry pspell_module_entry = {
	STANDARD_MODULE_HEADER,
	"pspell",
	ext_functions,
	CRX_MINIT(pspell),
	NULL,
	NULL,
	NULL,
	CRX_MINFO(pspell),
	CRX_PSPELL_VERSION,
	STANDARD_MODULE_PROPERTIES,
};

#ifdef COMPILE_DL_PSPELL
CREX_GET_MODULE(pspell)
#endif

/* class PSpell */

typedef struct _crx_pspell_object {
	PspellManager *mgr;
	crex_object std;
} crx_pspell_object;

static crx_pspell_object *crx_pspell_object_from_crex_object(crex_object *zobj) {
	return ((crx_pspell_object*)(zobj + 1)) - 1;
}

static crex_object *crx_pspell_object_to_crex_object(crx_pspell_object *obj) {
	return ((crex_object*)(obj + 1)) - 1;
}

static crex_function *crx_pspell_object_get_constructor(crex_object *object)
{
	crex_throw_error(NULL, "You cannot initialize a PSpell\\Dictionary object except through helper functions");
	return NULL;
}

static crex_object *crx_pspell_object_create(crex_class_entry *ce)
{
	crx_pspell_object *obj = crex_object_alloc(sizeof(crx_pspell_object), ce);
	crex_object *zobj = crx_pspell_object_to_crex_object(obj);

	obj->mgr = NULL;
	crex_object_std_init(zobj, ce);
	object_properties_init(zobj, ce);
	zobj->handlers = &crx_pspell_handlers;

	return zobj;
}

static void crx_pspell_object_free(crex_object *zobj) {
	delete_pspell_manager(crx_pspell_object_from_crex_object(zobj)->mgr);
}

/* class PSpellConfig */

typedef struct _crx_pspell_config_object {
	PspellConfig *cfg;
	crex_object std;
} crx_pspell_config_object;

static crx_pspell_config_object *crx_pspell_config_object_from_crex_object(crex_object *zobj) {
	return ((crx_pspell_config_object*)(zobj + 1)) - 1;
}

static crex_object *crx_pspell_config_object_to_crex_object(crx_pspell_config_object *obj) {
	return ((crex_object*)(obj + 1)) - 1;
}

static crex_function *crx_pspell_config_object_get_constructor(crex_object *object)
{
	crex_throw_error(NULL, "You cannot initialize a PSpell\\Config object except through helper functions");
	return NULL;
}

static crex_object *crx_pspell_config_object_create(crex_class_entry *ce)
{
	crx_pspell_config_object *obj = crex_object_alloc(sizeof(crx_pspell_config_object), ce);
	crex_object *zobj = crx_pspell_config_object_to_crex_object(obj);

	obj->cfg = NULL;
	crex_object_std_init(zobj, ce);
	object_properties_init(zobj, ce);
	zobj->handlers = &crx_pspell_config_handlers;

	return zobj;
}

static void crx_pspell_config_object_free(crex_object *zobj) {
	delete_pspell_config(crx_pspell_config_object_from_crex_object(zobj)->cfg);
}

/* {{{ CRX_MINIT_FUNCTION */
static CRX_MINIT_FUNCTION(pspell)
{
	crx_pspell_ce = register_class_PSpell_Dictionary();
	crx_pspell_ce->create_object = crx_pspell_object_create;

	memcpy(&crx_pspell_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	crx_pspell_handlers.clone_obj = NULL;
	crx_pspell_handlers.free_obj = crx_pspell_object_free;
	crx_pspell_handlers.get_constructor = crx_pspell_object_get_constructor;
	crx_pspell_handlers.offset = XtOffsetOf(crx_pspell_object, std);

	crx_pspell_config_ce = register_class_PSpell_Config();
	crx_pspell_config_ce->create_object = crx_pspell_config_object_create;

	memcpy(&crx_pspell_config_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	crx_pspell_config_handlers.clone_obj = NULL;
	crx_pspell_config_handlers.free_obj = crx_pspell_config_object_free;
	crx_pspell_config_handlers.get_constructor = crx_pspell_config_object_get_constructor;
	crx_pspell_config_handlers.offset = XtOffsetOf(crx_pspell_config_object, std);

	register_pspell_symbols(module_number);

	return SUCCESS;
}
/* }}} */

/* {{{ Load a dictionary */
CRX_FUNCTION(pspell_new)
{
	char *language, *spelling = NULL, *jargon = NULL, *encoding = NULL;
	size_t language_len, spelling_len = 0, jargon_len = 0, encoding_len = 0;
	crex_long mode = C_L(0), speed = C_L(0);
	int argc = CREX_NUM_ARGS();

#ifdef CRX_WIN32
	TCHAR aspell_dir[200];
	TCHAR data_dir[220];
	TCHAR dict_dir[220];
	HKEY hkey;
	DWORD dwType,dwLen;
#endif

	PspellCanHaveError *ret;
	PspellConfig *config;

	if (crex_parse_parameters(argc, "s|sssl", &language, &language_len, &spelling, &spelling_len,
		&jargon, &jargon_len, &encoding, &encoding_len, &mode) == FAILURE) {
		RETURN_THROWS();
	}

	config = new_pspell_config();

#ifdef CRX_WIN32
	/* If aspell was installed using installer, we should have a key
	 * pointing to the location of the dictionaries
	 */
	if (0 == RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Aspell", &hkey)) {
		LONG result;
		dwLen = sizeof(aspell_dir) - 1;
		result = RegQueryValueEx(hkey, "", NULL, &dwType, (LPBYTE)&aspell_dir, &dwLen);
		RegCloseKey(hkey);
		if (result == ERROR_SUCCESS) {
			strlcpy(data_dir, aspell_dir, sizeof(data_dir));
			strlcat(data_dir, "\\data", sizeof(data_dir));
			strlcpy(dict_dir, aspell_dir, sizeof(dict_dir));
			strlcat(dict_dir, "\\dict", sizeof(dict_dir));

			pspell_config_replace(config, "data-dir", data_dir);
			pspell_config_replace(config, "dict-dir", dict_dir);
		}
	}
#endif

	pspell_config_replace(config, "language-tag", language);

	if (spelling_len) {
		pspell_config_replace(config, "spelling", spelling);
	}

	if (jargon_len) {
		pspell_config_replace(config, "jargon", jargon);
	}

	if (encoding_len) {
		pspell_config_replace(config, "encoding", encoding);
	}

	if (mode) {
		speed = mode & PSPELL_SPEED_MASK_INTERNAL;

		/* First check what mode we want (how many suggestions) */
		if (speed == PSPELL_FAST) {
			pspell_config_replace(config, "sug-mode", "fast");
		} else if (speed == PSPELL_NORMAL) {
			pspell_config_replace(config, "sug-mode", "normal");
		} else if (speed == PSPELL_BAD_SPELLERS) {
			pspell_config_replace(config, "sug-mode", "bad-spellers");
		}

		/* Then we see if run-together words should be treated as valid components */
		if (mode & PSPELL_RUN_TOGETHER) {
			pspell_config_replace(config, "run-together", "true");
		}
	}

	ret = new_pspell_manager(config);
	delete_pspell_config(config);

	if (pspell_error_number(ret) != 0) {
		crx_error_docref(NULL, E_WARNING, "PSPELL couldn't open the dictionary. reason: %s", pspell_error_message(ret));
		delete_pspell_can_have_error(ret);
		RETURN_FALSE;
	}

	object_init_ex(return_value, crx_pspell_ce);
	crx_pspell_object_from_crex_object(C_OBJ_P(return_value))->mgr = to_pspell_manager(ret);
}
/* }}} */

/* {{{ Load a dictionary with a personal wordlist*/
CRX_FUNCTION(pspell_new_personal)
{
	char *personal, *language, *spelling = NULL, *jargon = NULL, *encoding = NULL;
	size_t personal_len, language_len, spelling_len = 0, jargon_len = 0, encoding_len = 0;
	crex_long mode = C_L(0), speed = C_L(0);
	int argc = CREX_NUM_ARGS();

#ifdef CRX_WIN32
	TCHAR aspell_dir[200];
	TCHAR data_dir[220];
	TCHAR dict_dir[220];
	HKEY hkey;
	DWORD dwType,dwLen;
#endif

	PspellCanHaveError *ret;
	PspellConfig *config;

	if (crex_parse_parameters(argc, "ps|sssl", &personal, &personal_len, &language, &language_len,
		&spelling, &spelling_len, &jargon, &jargon_len, &encoding, &encoding_len, &mode) == FAILURE) {
		RETURN_THROWS();
	}

	config = new_pspell_config();

#ifdef CRX_WIN32
	/* If aspell was installed using installer, we should have a key
	 * pointing to the location of the dictionaries
	 */
	if (0 == RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Aspell", &hkey)) {
		LONG result;
		dwLen = sizeof(aspell_dir) - 1;
		result = RegQueryValueEx(hkey, "", NULL, &dwType, (LPBYTE)&aspell_dir, &dwLen);
		RegCloseKey(hkey);
		if (result == ERROR_SUCCESS) {
			strlcpy(data_dir, aspell_dir, sizeof(data_dir));
			strlcat(data_dir, "\\data", sizeof(data_dir));
			strlcpy(dict_dir, aspell_dir, sizeof(dict_dir));
			strlcat(dict_dir, "\\dict", sizeof(dict_dir));

			pspell_config_replace(config, "data-dir", data_dir);
			pspell_config_replace(config, "dict-dir", dict_dir);
		}
	}
#endif

	if (crx_check_open_basedir(personal)) {
		delete_pspell_config(config);
		RETURN_FALSE;
	}

	pspell_config_replace(config, "personal", personal);
	pspell_config_replace(config, "save-repl", "false");

	pspell_config_replace(config, "language-tag", language);

	if (spelling_len) {
		pspell_config_replace(config, "spelling", spelling);
	}

	if (jargon_len) {
		pspell_config_replace(config, "jargon", jargon);
	}

	if (encoding_len) {
		pspell_config_replace(config, "encoding", encoding);
	}

	if (mode) {
		speed = mode & PSPELL_SPEED_MASK_INTERNAL;

		/* First check what mode we want (how many suggestions) */
		if (speed == PSPELL_FAST) {
			pspell_config_replace(config, "sug-mode", "fast");
		} else if (speed == PSPELL_NORMAL) {
			pspell_config_replace(config, "sug-mode", "normal");
		} else if (speed == PSPELL_BAD_SPELLERS) {
			pspell_config_replace(config, "sug-mode", "bad-spellers");
		}

		/* Then we see if run-together words should be treated as valid components */
		if (mode & PSPELL_RUN_TOGETHER) {
			pspell_config_replace(config, "run-together", "true");
		}
	}

	ret = new_pspell_manager(config);
	delete_pspell_config(config);

	if (pspell_error_number(ret) != 0) {
		crx_error_docref(NULL, E_WARNING, "PSPELL couldn't open the dictionary. reason: %s", pspell_error_message(ret));
		delete_pspell_can_have_error(ret);
		RETURN_FALSE;
	}

	object_init_ex(return_value, crx_pspell_ce);
	crx_pspell_object_from_crex_object(C_OBJ_P(return_value))->mgr = to_pspell_manager(ret);
}
/* }}} */

/* {{{ Load a dictionary based on the given config */
CRX_FUNCTION(pspell_new_config)
{
	zval *zcfg;
	PspellCanHaveError *ret;
	PspellConfig *config;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &zcfg, crx_pspell_config_ce) == FAILURE) {
		RETURN_THROWS();
	}
	config = crx_pspell_config_object_from_crex_object(C_OBJ_P(zcfg))->cfg;

	ret = new_pspell_manager(config);

	if (pspell_error_number(ret) != 0) {
		crx_error_docref(NULL, E_WARNING, "PSPELL couldn't open the dictionary. reason: %s", pspell_error_message(ret));
		delete_pspell_can_have_error(ret);
		RETURN_FALSE;
	}

	object_init_ex(return_value, crx_pspell_ce);
	crx_pspell_object_from_crex_object(C_OBJ_P(return_value))->mgr = to_pspell_manager(ret);
}
/* }}} */

/* {{{ Returns true if word is valid */
CRX_FUNCTION(pspell_check)
{
	zval *zmgr;
	crex_string *word;
	PspellManager *manager;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OS", &zmgr, crx_pspell_ce, &word) == FAILURE) {
		RETURN_THROWS();
	}
	manager = crx_pspell_object_from_crex_object(C_OBJ_P(zmgr))->mgr;

	if (pspell_manager_check(manager, ZSTR_VAL(word))) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Returns array of suggestions */
CRX_FUNCTION(pspell_suggest)
{
	zval *zmgr;
	crex_string *word;
	PspellManager *manager;
	const PspellWordList *wl;
	const char *sug;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OS", &zmgr, crx_pspell_ce, &word) == FAILURE) {
		RETURN_THROWS();
	}
	manager = crx_pspell_object_from_crex_object(C_OBJ_P(zmgr))->mgr;

	array_init(return_value);

	wl = pspell_manager_suggest(manager, ZSTR_VAL(word));
	if (wl) {
		PspellStringEmulation *els = pspell_word_list_elements(wl);
		while ((sug = pspell_string_emulation_next(els)) != 0) {
			add_next_index_string(return_value,(char *)sug);
		}
		delete_pspell_string_emulation(els);
	} else {
		crx_error_docref(NULL, E_WARNING, "PSPELL had a problem. details: %s", pspell_manager_error_message(manager));
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Notify the dictionary of a user-selected replacement */
CRX_FUNCTION(pspell_store_replacement)
{
	zval *zmgr;
	crex_string *miss, *corr;
	PspellManager *manager;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OSS", &zmgr, crx_pspell_ce, &miss, &corr) == FAILURE) {
		RETURN_THROWS();
	}
	manager = crx_pspell_object_from_crex_object(C_OBJ_P(zmgr))->mgr;

	pspell_manager_store_replacement(manager, ZSTR_VAL(miss), ZSTR_VAL(corr));
	if (pspell_manager_error_number(manager) == 0) {
		RETURN_TRUE;
	} else {
		crx_error_docref(NULL, E_WARNING, "pspell_store_replacement() gave error: %s", pspell_manager_error_message(manager));
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Adds a word to a personal list */
CRX_FUNCTION(pspell_add_to_personal)
{
	zval *zmgr;
	crex_string *word;
	PspellManager *manager;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OS", &zmgr, crx_pspell_ce, &word) == FAILURE) {
		RETURN_THROWS();
	}
	manager = crx_pspell_object_from_crex_object(C_OBJ_P(zmgr))->mgr;

	/*If the word is empty, we have to return; otherwise we'll segfault! ouch!*/
	if (ZSTR_LEN(word) == 0) {
		RETURN_FALSE;
	}

	pspell_manager_add_to_personal(manager, ZSTR_VAL(word));
	if (pspell_manager_error_number(manager) == 0) {
		RETURN_TRUE;
	} else {
		crx_error_docref(NULL, E_WARNING, "pspell_add_to_personal() gave error: %s", pspell_manager_error_message(manager));
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Adds a word to the current session */
CRX_FUNCTION(pspell_add_to_session)
{
	zval *zmgr;
	crex_string *word;
	PspellManager *manager;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OS", &zmgr, crx_pspell_ce, &word) == FAILURE) {
		RETURN_THROWS();
	}
	manager = crx_pspell_object_from_crex_object(C_OBJ_P(zmgr))->mgr;

	/*If the word is empty, we have to return; otherwise we'll segfault! ouch!*/
	if (ZSTR_LEN(word) == 0) {
		RETURN_FALSE;
	}

	pspell_manager_add_to_session(manager, ZSTR_VAL(word));
	if (pspell_manager_error_number(manager) == 0) {
		RETURN_TRUE;
	} else {
		crx_error_docref(NULL, E_WARNING, "pspell_add_to_session() gave error: %s", pspell_manager_error_message(manager));
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Clears the current session */
CRX_FUNCTION(pspell_clear_session)
{
	zval *zmgr;
	PspellManager *manager;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &zmgr, crx_pspell_ce) == FAILURE) {
		RETURN_THROWS();
	}
	manager = crx_pspell_object_from_crex_object(C_OBJ_P(zmgr))->mgr;

	pspell_manager_clear_session(manager);
	if (pspell_manager_error_number(manager) == 0) {
		RETURN_TRUE;
	} else {
		crx_error_docref(NULL, E_WARNING, "pspell_clear_session() gave error: %s", pspell_manager_error_message(manager));
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Saves the current (personal) wordlist */
CRX_FUNCTION(pspell_save_wordlist)
{
	zval *zmgr;
	PspellManager *manager;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "O", &zmgr, crx_pspell_ce) == FAILURE) {
		RETURN_THROWS();
	}
	manager = crx_pspell_object_from_crex_object(C_OBJ_P(zmgr))->mgr;

	pspell_manager_save_all_word_lists(manager);

	if (pspell_manager_error_number(manager) == 0) {
		RETURN_TRUE;
	} else {
		crx_error_docref(NULL, E_WARNING, "pspell_save_wordlist() gave error: %s", pspell_manager_error_message(manager));
		RETURN_FALSE;
	}

}
/* }}} */

/* {{{ Create a new config to be used later to create a manager */
CRX_FUNCTION(pspell_config_create)
{
	char *language, *spelling = NULL, *jargon = NULL, *encoding = NULL;
	size_t language_len, spelling_len = 0, jargon_len = 0, encoding_len = 0;
	PspellConfig *config;

#ifdef CRX_WIN32
	TCHAR aspell_dir[200];
	TCHAR data_dir[220];
	TCHAR dict_dir[220];
	HKEY hkey;
	DWORD dwType,dwLen;
#endif

	if (crex_parse_parameters(CREX_NUM_ARGS(), "s|sss", &language, &language_len, &spelling, &spelling_len,
		&jargon, &jargon_len, &encoding, &encoding_len) == FAILURE) {
		RETURN_THROWS();
	}

	config = new_pspell_config();

#ifdef CRX_WIN32
    /* If aspell was installed using installer, we should have a key
     * pointing to the location of the dictionaries
     */
	if (0 == RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Aspell", &hkey)) {
		LONG result;
		dwLen = sizeof(aspell_dir) - 1;
		result = RegQueryValueEx(hkey, "", NULL, &dwType, (LPBYTE)&aspell_dir, &dwLen);
		RegCloseKey(hkey);
		if (result == ERROR_SUCCESS) {
			strlcpy(data_dir, aspell_dir, sizeof(data_dir));
			strlcat(data_dir, "\\data", sizeof(data_dir));
			strlcpy(dict_dir, aspell_dir, sizeof(dict_dir));
			strlcat(dict_dir, "\\dict", sizeof(dict_dir));

			pspell_config_replace(config, "data-dir", data_dir);
			pspell_config_replace(config, "dict-dir", dict_dir);
		}
	}
#endif

	pspell_config_replace(config, "language-tag", language);

	if (spelling_len) {
		pspell_config_replace(config, "spelling", spelling);
	}

	if (jargon_len) {
		pspell_config_replace(config, "jargon", jargon);
	}

	if (encoding_len) {
		pspell_config_replace(config, "encoding", encoding);
	}

	/* By default I do not want to write anything anywhere because it'll try to write to $HOME
	which is not what we want */
	pspell_config_replace(config, "save-repl", "false");

	object_init_ex(return_value, crx_pspell_config_ce);
	crx_pspell_config_object_from_crex_object(C_OBJ_P(return_value))->cfg = config;
}
/* }}} */

/* {{{ Consider run-together words as valid components */
CRX_FUNCTION(pspell_config_runtogether)
{
	zval *zcfg;
	bool runtogether;
	PspellConfig *config;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ob", &zcfg, crx_pspell_config_ce, &runtogether) == FAILURE) {
		RETURN_THROWS();
	}
	config = crx_pspell_config_object_from_crex_object(C_OBJ_P(zcfg))->cfg;

	pspell_config_replace(config, "run-together", runtogether ? "true" : "false");

	RETURN_TRUE;
}
/* }}} */

/* {{{ Select mode for config (PSPELL_FAST, PSPELL_NORMAL or PSPELL_BAD_SPELLERS) */
CRX_FUNCTION(pspell_config_mode)
{
	zval *zcfg;
	crex_long mode;
	PspellConfig *config;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ol", &zcfg, crx_pspell_config_ce, &mode) == FAILURE) {
		RETURN_THROWS();
	}
	config = crx_pspell_config_object_from_crex_object(C_OBJ_P(zcfg))->cfg;

	/* First check what mode we want (how many suggestions) */
	if (mode == PSPELL_FAST) {
		pspell_config_replace(config, "sug-mode", "fast");
	} else if (mode == PSPELL_NORMAL) {
		pspell_config_replace(config, "sug-mode", "normal");
	} else if (mode == PSPELL_BAD_SPELLERS) {
		pspell_config_replace(config, "sug-mode", "bad-spellers");
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Ignore words <= n chars */
CRX_FUNCTION(pspell_config_ignore)
{
	char ignore_str[MAX_LENGTH_OF_LONG + 1];
	zval *zcfg;
	crex_long ignore = 0L;
	PspellConfig *config;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ol", &zcfg, crx_pspell_config_ce, &ignore) == FAILURE) {
		RETURN_THROWS();
	}
	config = crx_pspell_config_object_from_crex_object(C_OBJ_P(zcfg))->cfg;

	snprintf(ignore_str, sizeof(ignore_str), CREX_LONG_FMT, ignore);

	pspell_config_replace(config, "ignore", ignore_str);
	RETURN_TRUE;
}
/* }}} */

static void pspell_config_path(INTERNAL_FUNCTION_PARAMETERS, char *option)
{
	zval *zcfg;
	crex_string *value;
	PspellConfig *config;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OP", &zcfg, crx_pspell_config_ce, &value) == FAILURE) {
		RETURN_THROWS();
	}
	config = crx_pspell_config_object_from_crex_object(C_OBJ_P(zcfg))->cfg;

	if (crx_check_open_basedir(ZSTR_VAL(value))) {
		RETURN_FALSE;
	}

	pspell_config_replace(config, option, ZSTR_VAL(value));

	RETURN_TRUE;
}

/* {{{ Use a personal dictionary for this config */
CRX_FUNCTION(pspell_config_personal)
{
	pspell_config_path(INTERNAL_FUNCTION_PARAM_PASSTHRU, "personal");
}
/* }}} */

/* {{{ location of the main word list */
CRX_FUNCTION(pspell_config_dict_dir)
{
	pspell_config_path(INTERNAL_FUNCTION_PARAM_PASSTHRU, "dict-dir");
}
/* }}} */

/* {{{ location of language data files */
CRX_FUNCTION(pspell_config_data_dir)
{
	pspell_config_path(INTERNAL_FUNCTION_PARAM_PASSTHRU, "data-dir");
}
/* }}} */

/* {{{ Use a personal dictionary with replacement pairs for this config */
CRX_FUNCTION(pspell_config_repl)
{
	zval *zcfg;
	crex_string *repl;
	PspellConfig *config;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OP", &zcfg, crx_pspell_config_ce, &repl) == FAILURE) {
		RETURN_THROWS();
	}
	config = crx_pspell_config_object_from_crex_object(C_OBJ_P(zcfg))->cfg;

	pspell_config_replace(config, "save-repl", "true");

	if (crx_check_open_basedir(ZSTR_VAL(repl))) {
		RETURN_FALSE;
	}

	pspell_config_replace(config, "repl", ZSTR_VAL(repl));

	RETURN_TRUE;
}
/* }}} */

/* {{{ Save replacement pairs when personal list is saved for this config */
CRX_FUNCTION(pspell_config_save_repl)
{
	zval *zcfg;
	bool save;
	PspellConfig *config;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "Ob", &zcfg, crx_pspell_config_ce, &save) == FAILURE) {
		RETURN_THROWS();
	}
	config = crx_pspell_config_object_from_crex_object(C_OBJ_P(zcfg))->cfg;

	pspell_config_replace(config, "save-repl", save ? "true" : "false");

	RETURN_TRUE;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
static CRX_MINFO_FUNCTION(pspell)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "PSpell Support", "enabled");
	crx_info_print_table_end();
}
/* }}} */

#endif
