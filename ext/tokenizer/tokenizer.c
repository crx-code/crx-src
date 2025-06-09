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
   | Author: Andrei Zmievski <andrei@crx.net>                             |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "crx_ini.h"
#include "ext/standard/info.h"
#include "crx_tokenizer.h"

#include "crex.h"
#include "crex_exceptions.h"
#include "crex_language_scanner.h"
#include "crex_language_scanner_defs.h"
#include <crex_language_parser.h>
#include "crex_interfaces.h"

#include "tokenizer_data_arginfo.h"
#include "tokenizer_arginfo.h"

#define crextext   LANG_SCNG(yy_text)
#define crexleng   LANG_SCNG(yy_leng)
#define crexcursor LANG_SCNG(yy_cursor)
#define crexlimit  LANG_SCNG(yy_limit)

crex_class_entry *crx_token_ce;

/* {{{ tokenizer_module_entry */
crex_module_entry tokenizer_module_entry = {
	STANDARD_MODULE_HEADER,
	"tokenizer",
	ext_functions,
	CRX_MINIT(tokenizer),
	NULL,
	NULL,
	NULL,
	CRX_MINFO(tokenizer),
	CRX_TOKENIZER_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_TOKENIZER
CREX_GET_MODULE(tokenizer)
#endif

static zval *crx_token_get_id(zval *obj) {
	zval *id = OBJ_PROP_NUM(C_OBJ_P(obj), 0);
	if (C_ISUNDEF_P(id)) {
		crex_throw_error(NULL,
			"Typed property CrxToken::$id must not be accessed before initialization");
		return NULL;
	}

	ZVAL_DEREF(id);
	CREX_ASSERT(C_TYPE_P(id) == IS_LONG);
	return id;
}

static crex_string *crx_token_get_text(zval *obj) {
	zval *text_zval = OBJ_PROP_NUM(C_OBJ_P(obj), 1);
	if (C_ISUNDEF_P(text_zval)) {
		crex_throw_error(NULL,
			"Typed property CrxToken::$text must not be accessed before initialization");
		return NULL;
	}

	ZVAL_DEREF(text_zval);
	CREX_ASSERT(C_TYPE_P(text_zval) == IS_STRING);
	return C_STR_P(text_zval);
}

static bool tokenize_common(
		zval *return_value, crex_string *source, crex_long flags, crex_class_entry *token_class);

CRX_METHOD(CrxToken, tokenize)
{
	crex_string *source;
	crex_long flags = 0;
	crex_class_entry *token_class;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(source)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(flags)
	CREX_PARSE_PARAMETERS_END();

	token_class = crex_get_called_scope(execute_data);

	/* Check construction preconditions in advance, so these are not repeated for each token. */
	if (token_class->ce_flags & CREX_ACC_EXPLICIT_ABSTRACT_CLASS) {
		crex_throw_error(NULL, "Cannot instantiate abstract class %s", ZSTR_VAL(token_class->name));
		RETURN_THROWS();
	}
	if (crex_update_class_constants(token_class) == FAILURE) {
		RETURN_THROWS();
	}

	if (!tokenize_common(return_value, source, flags, token_class)) {
		RETURN_THROWS();
	}
}

CRX_METHOD(CrxToken, __main)
{
	crex_long id;
	crex_string *text;
	crex_long line = -1;
	crex_long pos = -1;
	crex_object *obj = C_OBJ_P(CREX_THIS);

	CREX_PARSE_PARAMETERS_START(2, 4)
		C_PARAM_LONG(id)
		C_PARAM_STR(text)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(line)
		C_PARAM_LONG(pos)
	CREX_PARSE_PARAMETERS_END();

	ZVAL_LONG(OBJ_PROP_NUM(obj, 0), id);
	zval_ptr_dtor(OBJ_PROP_NUM(obj, 1));
	ZVAL_STR_COPY(OBJ_PROP_NUM(obj, 1), text);
	ZVAL_LONG(OBJ_PROP_NUM(obj, 2), line);
	ZVAL_LONG(OBJ_PROP_NUM(obj, 3), pos);
}

CRX_METHOD(CrxToken, is)
{
	zval *kind;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(kind)
	CREX_PARSE_PARAMETERS_END();

	if (C_TYPE_P(kind) == IS_LONG) {
		zval *id_zval = crx_token_get_id(CREX_THIS);
		if (!id_zval) {
			RETURN_THROWS();
		}

		RETURN_BOOL(C_LVAL_P(id_zval) == C_LVAL_P(kind));
	} else if (C_TYPE_P(kind) == IS_STRING) {
		crex_string *text = crx_token_get_text(CREX_THIS);
		if (!text) {
			RETURN_THROWS();
		}

		RETURN_BOOL(crex_string_equals(text, C_STR_P(kind)));
	} else if (C_TYPE_P(kind) == IS_ARRAY) {
		zval *id_zval = NULL, *entry;
		crex_string *text = NULL;
		CREX_HASH_FOREACH_VAL(C_ARRVAL_P(kind), entry) {
			ZVAL_DEREF(entry);
			if (C_TYPE_P(entry) == IS_LONG) {
				if (!id_zval) {
					id_zval = crx_token_get_id(CREX_THIS);
					if (!id_zval) {
						RETURN_THROWS();
					}
				}
				if (C_LVAL_P(id_zval) == C_LVAL_P(entry)) {
					RETURN_TRUE;
				}
			} else if (C_TYPE_P(entry) == IS_STRING) {
				if (!text) {
					text = crx_token_get_text(CREX_THIS);
					if (!text) {
						RETURN_THROWS();
					}
				}
				if (crex_string_equals(text, C_STR_P(entry))) {
					RETURN_TRUE;
				}
			} else {
				crex_argument_type_error(1, "must only have elements of type string|int, %s given", crex_zval_value_name(entry));
				RETURN_THROWS();
			}
		} CREX_HASH_FOREACH_END();
		RETURN_FALSE;
	} else {
		crex_argument_type_error(1, "must be of type string|int|array, %s given", crex_zval_value_name(kind));
		RETURN_THROWS();
	}
}

CRX_METHOD(CrxToken, isIgnorable)
{
	CREX_PARSE_PARAMETERS_NONE();

	zval *id_zval = crx_token_get_id(CREX_THIS);
	if (!id_zval) {
		RETURN_THROWS();
	}

	crex_long id = C_LVAL_P(id_zval);
	RETURN_BOOL(id == T_WHITESPACE || id == T_COMMENT || id == T_DOC_COMMENT || id == T_OPEN_TAG);
}

CRX_METHOD(CrxToken, getTokenName)
{
	CREX_PARSE_PARAMETERS_NONE();

	zval *id_zval = crx_token_get_id(CREX_THIS);
	if (!id_zval) {
		RETURN_THROWS();
	}

	if (C_LVAL_P(id_zval) < 256) {
		RETURN_CHAR(C_LVAL_P(id_zval));
	} else {
		const char *token_name = get_token_type_name(C_LVAL_P(id_zval));
		if (!token_name) {
			RETURN_NULL();
		}

		RETURN_STRING(token_name);
	}
}

CRX_METHOD(CrxToken, __toString)
{
	CREX_PARSE_PARAMETERS_NONE();

	crex_string *text = crx_token_get_text(CREX_THIS);
	if (!text) {
		RETURN_THROWS();
	}

	RETURN_STR_COPY(text);
}

/* {{{ CRX_MINIT_FUNCTION */
CRX_MINIT_FUNCTION(tokenizer)
{
	register_tokenizer_data_symbols(module_number);
	register_tokenizer_symbols(module_number);
	crx_token_ce = register_class_CrxToken(crex_ce_stringable);

	return SUCCESS;
}
/* }}} */

/* {{{ CRX_MINFO_FUNCTION */
CRX_MINFO_FUNCTION(tokenizer)
{
	crx_info_print_table_start();
	crx_info_print_table_row(2, "Tokenizer Support", "enabled");
	crx_info_print_table_end();
}
/* }}} */

static crex_string *make_str(unsigned char *text, size_t leng, HashTable *interned_strings) {
	if (leng == 1) {
		return ZSTR_CHAR(text[0]);
	} else if (interned_strings) {
		crex_string *interned_str = crex_hash_str_find_ptr(interned_strings, (char *) text, leng);
		if (interned_str) {
			return crex_string_copy(interned_str);
		}
		interned_str = crex_string_init((char *) text, leng, 0);
		crex_hash_add_new_ptr(interned_strings, interned_str, interned_str);
		return interned_str;
	} else {
		return crex_string_init((char *) text, leng, 0);
	}
}

static void add_token(
		zval *return_value, int token_type, unsigned char *text, size_t leng, int lineno,
		crex_class_entry *token_class, HashTable *interned_strings) {
	zval token;
	if (token_class) {
		crex_object *obj = crex_objects_new(token_class);
		ZVAL_OBJ(&token, obj);
		ZVAL_LONG(OBJ_PROP_NUM(obj, 0), token_type);
		ZVAL_STR(OBJ_PROP_NUM(obj, 1), make_str(text, leng, interned_strings));
		ZVAL_LONG(OBJ_PROP_NUM(obj, 2), lineno);
		ZVAL_LONG(OBJ_PROP_NUM(obj, 3), text - LANG_SCNG(yy_start));

		/* If the class is extended with additional properties, initialized them as well. */
		if (UNEXPECTED(token_class->default_properties_count > 4)) {
			zval *dst = OBJ_PROP_NUM(obj, 4);
			zval *src = &token_class->default_properties_table[4];
			zval *end = token_class->default_properties_table
				+ token_class->default_properties_count;
			for (; src < end; src++, dst++) {
				ZVAL_COPY_PROP(dst, src);
			}
		}
	} else if (token_type >= 256) {
		array_init_size(&token, 3);
		crex_hash_real_init_packed(C_ARRVAL(token));
		CREX_HASH_FILL_PACKED(C_ARRVAL(token)) {
			CREX_HASH_FILL_SET_LONG(token_type);
			CREX_HASH_FILL_NEXT();
			CREX_HASH_FILL_SET_STR(make_str(text, leng, interned_strings));
			CREX_HASH_FILL_NEXT();
			CREX_HASH_FILL_SET_LONG(lineno);
			CREX_HASH_FILL_NEXT();
		} CREX_HASH_FILL_END();
	} else {
		ZVAL_STR(&token, make_str(text, leng, interned_strings));
	}
	crex_hash_next_index_insert_new(C_ARRVAL_P(return_value), &token);
}

static bool tokenize(zval *return_value, crex_string *source, crex_class_entry *token_class)
{
	zval source_zval;
	crex_lex_state original_lex_state;
	zval token;
	int token_type;
	int token_line = 1;
	int need_tokens = -1; /* for __halt_compiler lexing. -1 = disabled */
	HashTable interned_strings;

	ZVAL_STR_COPY(&source_zval, source);
	crex_save_lexical_state(&original_lex_state);

	crex_prepare_string_for_scanning(&source_zval, ZSTR_EMPTY_ALLOC());

	LANG_SCNG(yy_state) = yycINITIAL;
	crex_hash_init(&interned_strings, 0, NULL, NULL, 0);
	array_init(return_value);

	while ((token_type = lex_scan(&token, NULL))) {
		CREX_ASSERT(token_type != T_ERROR);

		add_token(
			return_value, token_type, crextext, crexleng, token_line,
			token_class, &interned_strings);

		if (C_TYPE(token) != IS_UNDEF) {
			zval_ptr_dtor_nogc(&token);
			ZVAL_UNDEF(&token);
		}

		/* after T_HALT_COMPILER collect the next three non-dropped tokens */
		if (need_tokens != -1) {
			if (token_type != T_WHITESPACE && token_type != T_OPEN_TAG
				&& token_type != T_COMMENT && token_type != T_DOC_COMMENT
				&& --need_tokens == 0
			) {
				/* fetch the rest into a T_INLINE_HTML */
				if (crexcursor < crexlimit) {
					add_token(
						return_value, T_INLINE_HTML, crexcursor, crexlimit - crexcursor,
						token_line, token_class, &interned_strings);
				}
				break;
			}
		} else if (token_type == T_HALT_COMPILER) {
			need_tokens = 3;
		}

		if (CG(increment_lineno)) {
			CG(crex_lineno)++;
			CG(increment_lineno) = 0;
		}

		token_line = CG(crex_lineno);
	}

	zval_ptr_dtor_str(&source_zval);
	crex_restore_lexical_state(&original_lex_state);
	crex_hash_destroy(&interned_strings);

	return 1;
}

struct event_context {
	zval *tokens;
	crex_class_entry *token_class;
};

static zval *extract_token_id_to_replace(zval *token_zv, const char *text, size_t length) {
	zval *id_zv, *text_zv;
	CREX_ASSERT(token_zv);
	if (C_TYPE_P(token_zv) == IS_ARRAY) {
		id_zv = crex_hash_index_find(C_ARRVAL_P(token_zv), 0);
		text_zv = crex_hash_index_find(C_ARRVAL_P(token_zv), 1);
	} else if (C_TYPE_P(token_zv) == IS_OBJECT) {
		id_zv = OBJ_PROP_NUM(C_OBJ_P(token_zv), 0);
		text_zv = OBJ_PROP_NUM(C_OBJ_P(token_zv), 1);
	} else {
		return NULL;
	}

	/* There are multiple candidate tokens to which this feedback may apply,
	 * check text to make sure this is the right one. */
	CREX_ASSERT(C_TYPE_P(text_zv) == IS_STRING);
	if (C_STRLEN_P(text_zv) == length && !memcmp(C_STRVAL_P(text_zv), text, length)) {
		return id_zv;
	}
	return NULL;
}

void on_event(
		crex_crx_scanner_event event, int token, int line,
		const char *text, size_t length, void *context)
{
	struct event_context *ctx = context;

	switch (event) {
		case ON_TOKEN:
			if (token == END) break;
			/* Special cases */
			if (token == ';' && LANG_SCNG(yy_leng) > 1) { /* ?> or ?>\n or ?>\r\n */
				token = T_CLOSE_TAG;
			} else if (token == T_ECHO && LANG_SCNG(yy_leng) == sizeof("<?=") - 1) {
				token = T_OPEN_TAG_WITH_ECHO;
			}
			add_token(
				ctx->tokens, token, (unsigned char *) text, length, line, ctx->token_class, NULL);
			break;
		case ON_FEEDBACK: {
			HashTable *tokens_ht = C_ARRVAL_P(ctx->tokens);
			zval *token_zv, *id_zv = NULL;
			CREX_HASH_REVERSE_FOREACH_VAL(tokens_ht, token_zv) {
				id_zv = extract_token_id_to_replace(token_zv, text, length);
				if (id_zv) {
					break;
				}
			} CREX_HASH_FOREACH_END();
			CREX_ASSERT(id_zv);
			ZVAL_LONG(id_zv, token);
			break;
		}
		case ON_STOP:
			if (LANG_SCNG(yy_cursor) != LANG_SCNG(yy_limit)) {
				add_token(ctx->tokens, T_INLINE_HTML, LANG_SCNG(yy_cursor),
					LANG_SCNG(yy_limit) - LANG_SCNG(yy_cursor), CG(crex_lineno),
					ctx->token_class, NULL);
			}
			break;
	}
}

static bool tokenize_parse(
		zval *return_value, crex_string *source, crex_class_entry *token_class)
{
	zval source_zval;
	struct event_context ctx;
	zval token_stream;
	crex_lex_state original_lex_state;
	bool original_in_compilation;
	bool success;

	ZVAL_STR_COPY(&source_zval, source);

	original_in_compilation = CG(in_compilation);
	CG(in_compilation) = 1;
	crex_save_lexical_state(&original_lex_state);

	crex_prepare_string_for_scanning(&source_zval, ZSTR_EMPTY_ALLOC());
	array_init(&token_stream);

	ctx.tokens = &token_stream;
	ctx.token_class = token_class;

	CG(ast) = NULL;
	CG(ast_arena) = crex_arena_create(1024 * 32);
	LANG_SCNG(yy_state) = yycINITIAL;
	LANG_SCNG(on_event) = on_event;
	LANG_SCNG(on_event_context) = &ctx;

	if((success = (crexparse() == SUCCESS))) {
		ZVAL_COPY_VALUE(return_value, &token_stream);
	} else {
		zval_ptr_dtor(&token_stream);
	}

	crex_ast_destroy(CG(ast));
	crex_arena_destroy(CG(ast_arena));

	/* restore compiler and scanner global states */
	crex_restore_lexical_state(&original_lex_state);
	CG(in_compilation) = original_in_compilation;

	zval_ptr_dtor_str(&source_zval);

	return success;
}

static bool tokenize_common(
		zval *return_value, crex_string *source, crex_long flags, crex_class_entry *token_class)
{
	if (flags & TOKEN_PARSE) {
		return tokenize_parse(return_value, source, token_class);
	} else {
		int success = tokenize(return_value, source, token_class);
		/* Normal token_get_all() should not throw. */
		crex_clear_exception();
		return success;
	}
}

/* }}} */

/* {{{ */
CRX_FUNCTION(token_get_all)
{
	crex_string *source;
	crex_long flags = 0;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_STR(source)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(flags)
	CREX_PARSE_PARAMETERS_END();

	if (!tokenize_common(return_value, source, flags, /* token_class */ NULL)) {
		RETURN_THROWS();
	}
}
/* }}} */

/* {{{ */
CRX_FUNCTION(token_name)
{
	crex_long type;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_LONG(type)
	CREX_PARSE_PARAMETERS_END();

	const char *token_name = get_token_type_name(type);
	if (!token_name) {
		token_name = "UNKNOWN";
	}
	RETURN_STRING(token_name);
}
/* }}} */
