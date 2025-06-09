/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Zeev Suraski <zeev@crx.net>                                 |
   |          Jani Taskinen <jani@crx.net>                                |
   |          Marcus Boerger <helly@crx.net>                              |
   |          Nuno Lopes <nlopess@crx.net>                                |
   |          Scott MacVicar <scottmac@crx.net>                           |
   +----------------------------------------------------------------------+
*/

#include <errno.h>
#include "crex.h"
#include "crex_API.h"
#include "crex_globals.h"
#include <crex_ini_parser.h>
#include "crex_ini_scanner.h"

#ifdef YYDEBUG
#undef YYDEBUG
#endif

#if 0
# define YYDEBUG(s, c) printf("state: %d char: %c\n", s, c)
#else
# define YYDEBUG(s, c)
#endif

#include "crex_ini_scanner_defs.h"

#define YYCTYPE   unsigned char
/* allow the scanner to read one null byte after the end of the string (from CREX_MMAP_AHEAD)
 * so that if will be able to terminate to match the current token (e.g. non-enclosed string) */
#define YYFILL(n) { if (YYCURSOR > YYLIMIT) return 0; }
#define YYCURSOR  SCNG(yy_cursor)
#define YYLIMIT   SCNG(yy_limit)
#define YYMARKER  SCNG(yy_marker)

#define YYGETCONDITION()  SCNG(yy_state)
#define YYSETCONDITION(s) SCNG(yy_state) = s

#define STATE(name)  yyc##name

/* emulate flex constructs */
#define BEGIN(state) YYSETCONDITION(STATE(state))
#define YYSTATE      YYGETCONDITION()
#define yytext       ((const char*)SCNG(yy_text))
#define yyleng       SCNG(yy_leng)
#define yyless(x)    do {	YYCURSOR = (const unsigned char*)yytext + x; \
							yyleng   = (unsigned int)x; } while(0)

/* #define yymore()     goto yymore_restart */

/* perform sanity check. If this message is triggered you should
   increase the CREX_MMAP_AHEAD value in the crex_streams.h file */
/*!max:re2c */
#if CREX_MMAP_AHEAD < (YYMAXFILL + 1)
# error CREX_MMAP_AHEAD should be greater than YYMAXFILL
#endif


/* How it works (for the core ini directives):
 * ===========================================
 *
 * 1. Scanner scans file for tokens and passes them to parser.
 * 2. Parser parses the tokens and passes the name/value pairs to the callback
 *    function which stores them in the configuration hash table.
 * 3. Later REGISTER_INI_ENTRIES() is called which triggers the actual
 *    registering of ini entries and uses crex_get_configuration_directive()
 *    to fetch the previously stored name/value pair from configuration hash table
 *    and registers the static ini entries which match the name to the value
 *    into EG(ini_directives) hash table.
 * 4. PATH section entries are used per-request from down to top, each overriding
 *    previous if one exists. crex_alter_ini_entry() is called for each entry.
 *    Settings in PATH section are CREX_INI_SYSTEM accessible and thus mimics the
 *    crx_admin_* directives used within Apache httpd.conf when CRX is compiled as
 *    module for Apache.
 * 5. User defined ini files (like .htaccess for apache) are parsed for each request and
 *    stored in separate hash defined by SAPI.
 */

/* TODO: (ordered by importance :-)
 * ===============================================================================
 *
 *  - Separate constant lookup totally from plain strings (using CONSTANT pattern)
 *  - Add #if .. #else .. #endif and ==, !=, <, > , <=, >= operators
 *  - Add #include "some.ini"
 *  - Allow variables to refer to options also when using parse_ini_file()
 *
 */

/* Globals Macros */
#define SCNG	INI_SCNG
#ifdef ZTS
CREX_API ts_rsrc_id ini_scanner_globals_id;
CREX_API size_t ini_scanner_globals_offset;
#else
CREX_API crex_ini_scanner_globals ini_scanner_globals;
#endif

#define CREX_SYSTEM_INI CG(ini_parser_unbuffered_errors)

/* Eat leading whitespace */
#define EAT_LEADING_WHITESPACE()                     \
	while (yyleng) {                                 \
		if (yytext[0] == ' ' || yytext[0] == '\t') { \
			SCNG(yy_text)++;                         \
			yyleng--;                                \
		} else {                                     \
			break;                                   \
		}                                            \
	}

/* Eat trailing whitespace + extra char */
#define EAT_TRAILING_WHITESPACE_EX(ch)              \
	while (yyleng && (                              \
		(ch != 'X' && yytext[yyleng - 1] ==  ch) || \
		yytext[yyleng - 1] == '\n' ||               \
		yytext[yyleng - 1] == '\r' ||               \
		yytext[yyleng - 1] == '\t' ||               \
		yytext[yyleng - 1] == ' ')                  \
	) {                                             \
		yyleng--;                                   \
	}

/* Eat trailing whitespace */
#define EAT_TRAILING_WHITESPACE()	EAT_TRAILING_WHITESPACE_EX('X')

#define crex_ini_copy_value(retval, str, len)	\
	ZVAL_NEW_STR(retval, crex_string_init(str, len, CREX_SYSTEM_INI))


#define RETURN_TOKEN(type, str, len) {                             \
	if (SCNG(scanner_mode) == CREX_INI_SCANNER_TYPED &&            \
		(YYSTATE == STATE(ST_VALUE) || YYSTATE == STATE(ST_RAW))) {\
		crex_ini_copy_typed_value(ini_lval, type, str, len);       \
		C_EXTRA_P(ini_lval) = 0;                                   \
	} else {                                                       \
		crex_ini_copy_value(ini_lval, str, len);                   \
	}                                                              \
	return type;                                                   \
}

static void crex_ini_copy_typed_value(zval *retval, const int type, const char *str, int len)
{
	switch (type) {
		case BOOL_FALSE:
		case BOOL_TRUE:
			ZVAL_BOOL(retval, type == BOOL_TRUE);
			break;

		case NULL_NULL:
			ZVAL_NULL(retval);
			break;

		default:
			crex_ini_copy_value(retval, str, len);
	}
}

static void _yy_push_state(int new_state)
{
	crex_stack_push(&SCNG(state_stack), (void *) &YYGETCONDITION());
	YYSETCONDITION(new_state);
}

#define yy_push_state(state_and_tsrm) _yy_push_state(yyc##state_and_tsrm)

static void yy_pop_state(void)
{
	int *stack_state = crex_stack_top(&SCNG(state_stack));
	YYSETCONDITION(*stack_state);
	crex_stack_del_top(&SCNG(state_stack));
}

static void yy_scan_buffer(const char *str, unsigned int len)
{
	YYCURSOR = (const YYCTYPE*)str;
	SCNG(yy_start) = YYCURSOR;
	YYLIMIT  = YYCURSOR + len;
}

#define ini_filename SCNG(filename)

/* {{{ init_ini_scanner() */
static crex_result init_ini_scanner(int scanner_mode, crex_file_handle *fh)
{
	/* Sanity check */
	if (scanner_mode != CREX_INI_SCANNER_NORMAL && scanner_mode != CREX_INI_SCANNER_RAW && scanner_mode != CREX_INI_SCANNER_TYPED) {
		crex_error(E_WARNING, "Invalid scanner mode");
		return FAILURE;
	}

	SCNG(lineno) = 1;
	SCNG(scanner_mode) = scanner_mode;
	SCNG(yy_in) = fh;

	if (fh != NULL) {
		ini_filename = crex_string_copy(fh->filename);
	} else {
		ini_filename = NULL;
	}

	crex_stack_init(&SCNG(state_stack), sizeof(int));
	BEGIN(INITIAL);

	return SUCCESS;
}
/* }}} */

/* {{{ shutdown_ini_scanner() */
void shutdown_ini_scanner(void)
{
	crex_stack_destroy(&SCNG(state_stack));
	if (ini_filename) {
		crex_string_release(ini_filename);
	}
}
/* }}} */

/* {{{ crex_ini_scanner_get_lineno() */
CREX_COLD int crex_ini_scanner_get_lineno(void)
{
	return SCNG(lineno);
}
/* }}} */

/* {{{ crex_ini_scanner_get_filename() */
CREX_COLD const char *crex_ini_scanner_get_filename(void)
{
	return ini_filename ? ZSTR_VAL(ini_filename) : "Unknown";
}
/* }}} */

/* {{{ crex_ini_open_file_for_scanning() */
crex_result crex_ini_open_file_for_scanning(crex_file_handle *fh, int scanner_mode)
{
	char *buf;
	size_t size;

	if (crex_stream_fixup(fh, &buf, &size) == FAILURE) {
		return FAILURE;
	}

	if (init_ini_scanner(scanner_mode, fh) == FAILURE) {
		return FAILURE;
	}

	yy_scan_buffer(buf, (unsigned int)size);

	return SUCCESS;
}
/* }}} */

/* {{{ crex_ini_prepare_string_for_scanning() */
crex_result crex_ini_prepare_string_for_scanning(const char *str, int scanner_mode)
{
	int len = (int)strlen(str);

	if (init_ini_scanner(scanner_mode, NULL) == FAILURE) {
		return FAILURE;
	}

	yy_scan_buffer(str, len);

	return SUCCESS;
}
/* }}} */

/* {{{ crex_ini_escape_string() */
static void crex_ini_escape_string(zval *lval, const char *str, int len, char quote_type)
{
	char *s, *t;
	char *end;

	crex_ini_copy_value(lval, str, len);

	/* convert escape sequences */
	s = t = C_STRVAL_P(lval);
	end = s + C_STRLEN_P(lval);

	while (s < end) {
		if (*s == '\\') {
			s++;
			if (s >= end) {
				*t++ = '\\';
				continue;
			}
			switch (*s) {
				case '"':
					if (*s != quote_type) {
						*t++ = '\\';
						*t++ = *s;
						break;
					}
					CREX_FALLTHROUGH;
				case '\\':
				case '$':
					*t++ = *s;
					C_STRLEN_P(lval)--;
					break;
				default:
					*t++ = '\\';
					*t++ = *s;
					break;
			}
		} else {
			*t++ = *s;
		}
		if (*s == '\n' || (*s == '\r' && (*(s+1) != '\n'))) {
			SCNG(lineno)++;
		}
		s++;
	}
	*t = 0;
}
/* }}} */

int ini_lex(zval *ini_lval)
{
restart:
	SCNG(yy_text) = YYCURSOR;

/* yymore_restart: */
	/* detect EOF */
	if (YYCURSOR >= YYLIMIT) {
		if (YYSTATE == STATE(ST_VALUE) || YYSTATE == STATE(ST_RAW)) {
			BEGIN(INITIAL);
			return END_OF_LINE;
		}
		return 0;
	}

	/* Eat any UTF-8 BOM we find in the first 3 bytes */
	if (YYCURSOR == SCNG(yy_start) && YYCURSOR + 3 < YYLIMIT) {
		if (memcmp(YYCURSOR, "\xef\xbb\xbf", 3) == 0) {
			YYCURSOR += 3;
			goto restart;
		}
	}
/*!re2c
re2c:yyfill:check = 0;
LNUM [0-9]+
DNUM ([0-9]*[\.][0-9]+)|([0-9]+[\.][0-9]*)
NUMBER [-]?{LNUM}|{DNUM}
ANY_CHAR (.|[\n\t])
NEWLINE	("\r"|"\n"|"\r\n")
TABS_AND_SPACES [ \t]
WHITESPACE [ \t]+
CONSTANT [a-zA-C_][a-zA-Z0-9_]*
LABEL_CHAR [^=\n\r\t;&|^$~(){}!"\[\]\x00]
LABEL ({LABEL_CHAR}+)
TOKENS [:,.\[\]"'()&|^+-/*=%$!~<>?@{}]
OPERATORS [&|^~()!]
DOLLAR_CURLY "${"

SECTION_RAW_CHARS [^\]\n\r]
SINGLE_QUOTED_CHARS [^']
RAW_VALUE_CHARS [^\n\r;\000]

LITERAL_DOLLAR ("$"([^{\000]|("\\"{ANY_CHAR})))
VALUE_CHARS         ([^$= \t\n\r;&|^~()!"'\000]|{LITERAL_DOLLAR})
FALLBACK_CHARS		([^$\n\r;"'}\\]|("\\"{ANY_CHAR})|{LITERAL_DOLLAR})
SECTION_VALUE_CHARS ([^$\n\r;"'\]\\]|("\\"{ANY_CHAR})|{LITERAL_DOLLAR})

<!*> := yyleng = YYCURSOR - SCNG(yy_text);

<INITIAL>"[" { /* Section start */
	/* Enter section data lookup state */
	if (SCNG(scanner_mode) == CREX_INI_SCANNER_RAW) {
		BEGIN(ST_SECTION_RAW);
	} else {
		BEGIN(ST_SECTION_VALUE);
	}
	return TC_SECTION;
}

<ST_VALUE,ST_SECTION_VALUE,ST_OFFSET>"'"{SINGLE_QUOTED_CHARS}+"'" { /* Raw string */
	/* Eat leading and trailing single quotes */
	if (yytext[0] == '\'' && yytext[yyleng - 1] == '\'') {
		SCNG(yy_text)++;
		yyleng = yyleng - 2;
	}
	RETURN_TOKEN(TC_RAW, yytext, yyleng);
}

<ST_SECTION_RAW,ST_SECTION_VALUE>"]"{TABS_AND_SPACES}*{NEWLINE}? { /* End of section */
	BEGIN(INITIAL);
	SCNG(lineno)++;
	return ']';
}

<INITIAL>{LABEL}"["{TABS_AND_SPACES}* { /* Start of option with offset */
	/* Eat leading whitespace */
	EAT_LEADING_WHITESPACE();

	/* Eat trailing whitespace and [ */
	EAT_TRAILING_WHITESPACE_EX('[');

	/* Enter offset lookup state */
	BEGIN(ST_OFFSET);

	RETURN_TOKEN(TC_OFFSET, yytext, yyleng);
}

<ST_OFFSET>{TABS_AND_SPACES}*"]" { /* End of section or an option offset */
	BEGIN(INITIAL);
	return ']';
}

<ST_DOUBLE_QUOTES,ST_SECTION_VALUE,ST_VALUE,ST_OFFSET,ST_VAR_FALLBACK>{DOLLAR_CURLY} { /* Variable start */
	yy_push_state(ST_VARNAME);
	return TC_DOLLAR_CURLY;
}

<ST_VARNAME>":-" { /* End Variable name, fallback start */
fallback_lexing:
	yy_pop_state();
	yy_push_state(ST_VAR_FALLBACK);
	return TC_FALLBACK;
}

<ST_VARNAME>{LABEL_CHAR} { /* Variable name */
	if (YYCURSOR[0] == ':' && YYCURSOR[1] == '-') {
		YYCURSOR++;
		goto fallback_lexing;
	}

	while (YYCURSOR < YYLIMIT) {
		switch (*YYCURSOR++) {
			case '=':
			case '\n':
			case '\r':
			case '\t':
			case ';':
			case '&':
			case '|':
			case '^':
			case '$':
			case '~':
			case '(':
			case ')':
			case '{':
			case '}':
			case '!':
			case '"':
			case '[':
			case ']':
				break;
			/* ':' is only allowed if it isn't followed by '-'. */
			case ':':
				if (YYCURSOR[0] == '-') {
					break;
				} else {
					continue;
				}
			default:
				continue;
		}

		YYCURSOR--;
		yyleng = YYCURSOR - SCNG(yy_text);
		break;
	}

	/* Eat leading whitespace */
	EAT_LEADING_WHITESPACE();

	/* Eat trailing whitespace */
	EAT_TRAILING_WHITESPACE();

	RETURN_TOKEN(TC_VARNAME, yytext, yyleng);
}

<ST_VARNAME,ST_VAR_FALLBACK>"}" { /* Variable/fallback end */
	yy_pop_state();
	return '}';
}

<INITIAL,ST_VALUE>("true"|"on"|"yes"){TABS_AND_SPACES}* { /* TRUE value (when used outside option value/offset this causes parse error!) */
	RETURN_TOKEN(BOOL_TRUE, "1", 1);
}

<INITIAL,ST_VALUE>("false"|"off"|"no"|"none"){TABS_AND_SPACES}* { /* FALSE value (when used outside option value/offset this causes parse error!)*/
	RETURN_TOKEN(BOOL_FALSE, "", 0);
}

<INITIAL,ST_VALUE>("null"){TABS_AND_SPACES}* {
	RETURN_TOKEN(NULL_NULL, "", 0);
}

<INITIAL>{LABEL} { /* Get option name */
	/* Eat leading whitespace */
	EAT_LEADING_WHITESPACE();

	/* Eat trailing whitespace */
	EAT_TRAILING_WHITESPACE();

	RETURN_TOKEN(TC_LABEL, yytext, yyleng);
}

<INITIAL>{TABS_AND_SPACES}*[=]{TABS_AND_SPACES}* { /* Start option value */
	if (SCNG(scanner_mode) == CREX_INI_SCANNER_RAW) {
		BEGIN(ST_RAW);
	} else {
		BEGIN(ST_VALUE);
	}
	return '=';
}

<ST_RAW>{RAW_VALUE_CHARS} { /* Raw value, only used when SCNG(scanner_mode) == CREX_INI_SCANNER_RAW. */
	const unsigned char *sc = NULL;
	EAT_LEADING_WHITESPACE();
	while (YYCURSOR < YYLIMIT) {
		switch (*YYCURSOR) {
			case '\n':
			case '\r':
				goto end_raw_value_chars;
				break;
			case ';':
				if (sc == NULL) {
					sc = YYCURSOR;
				}
				YYCURSOR++;
				break;
			case '"':
				if (yytext[0] == '"') {
					sc = NULL;
				}
				YYCURSOR++;
				break;
			default:
				YYCURSOR++;
				break;
		}
	}
end_raw_value_chars:
	if (sc) {
		yyleng = sc - SCNG(yy_text);
	} else {
		yyleng = YYCURSOR - SCNG(yy_text);
	}

	EAT_TRAILING_WHITESPACE();

	/* Eat leading and trailing double quotes */
	if (yyleng > 1 && yytext[0] == '"' && yytext[yyleng - 1] == '"') {
		SCNG(yy_text)++;
		yyleng = yyleng - 2;
	}

	RETURN_TOKEN(TC_RAW, yytext, yyleng);
}

<ST_SECTION_RAW>{SECTION_RAW_CHARS}+ { /* Raw value, only used when SCNG(scanner_mode) == CREX_INI_SCANNER_RAW. */
	RETURN_TOKEN(TC_RAW, yytext, yyleng);
}

<ST_VALUE,ST_RAW>{TABS_AND_SPACES}*{NEWLINE} { /* End of option value */
	BEGIN(INITIAL);
	SCNG(lineno)++;
	return END_OF_LINE;
}

<ST_SECTION_VALUE,ST_VALUE,ST_VAR_FALLBACK,ST_OFFSET>{CONSTANT} { /* Get constant option value */
	RETURN_TOKEN(TC_CONSTANT, yytext, yyleng);
}

<ST_SECTION_VALUE,ST_VALUE,ST_VAR_FALLBACK,ST_OFFSET>{NUMBER} { /* Get number option value as string */
	RETURN_TOKEN(TC_NUMBER, yytext, yyleng);
}

<INITIAL>{TOKENS} { /* Disallow these chars outside option values */
	return yytext[0];
}

<ST_VALUE>{OPERATORS}{TABS_AND_SPACES}* { /* Boolean operators */
	return yytext[0];
}

<ST_VALUE>[=] { /* Make = used in option value to trigger error */
	yyless(0);
	BEGIN(INITIAL);
	return END_OF_LINE;
}

<ST_VALUE>{VALUE_CHARS}+ { /* Get everything else as option/offset value */
	RETURN_TOKEN(TC_STRING, yytext, yyleng);
}

<ST_VAR_FALLBACK>{FALLBACK_CHARS}+ { /* Same as below, but excluding '}' */
	RETURN_TOKEN(TC_STRING, yytext, yyleng);
}

<ST_SECTION_VALUE,ST_OFFSET>{SECTION_VALUE_CHARS}+ { /* Get rest as section/offset value */
	RETURN_TOKEN(TC_STRING, yytext, yyleng);
}

<ST_SECTION_VALUE,ST_VALUE,ST_VAR_FALLBACK,ST_OFFSET>{TABS_AND_SPACES}*["] { /* Double quoted '"' string start */
	yy_push_state(ST_DOUBLE_QUOTES);
	return '"';
}

<ST_DOUBLE_QUOTES>["]{TABS_AND_SPACES}* { /* Double quoted '"' string ends */
	yy_pop_state();
	return '"';
}

<ST_DOUBLE_QUOTES>[^] { /* Escape double quoted string contents */
	if (YYCURSOR > YYLIMIT) {
		return 0;
	}

	const unsigned char *s = SCNG(yy_text);

	while (s < YYLIMIT) {
		switch (*s++) {
			case '"':
				break;
			case '$':
				if (s < YYLIMIT && *s == '{') {
					break;
				}
				continue;
			case '\\':
				if (s < YYLIMIT) {
					unsigned char escaped = *s++;
					/* A special case for Windows paths, e.g. key="C:\path\" */
					if (escaped == '"' && (s >= YYLIMIT || *s == '\n' || *s == '\r')) {
						break;
					}
				}
				CREX_FALLTHROUGH;
			default:
				continue;
		}

		s--;
		break;
	}

	YYCURSOR = s;
	yyleng = YYCURSOR - SCNG(yy_text);

	crex_ini_escape_string(ini_lval, yytext, yyleng, '"');
	C_EXTRA_P(ini_lval) = 0;
	return TC_QUOTED_STRING;
}

<ST_SECTION_VALUE,ST_VALUE,ST_OFFSET,ST_VAR_FALLBACK>{WHITESPACE} {
	RETURN_TOKEN(TC_WHITESPACE, yytext, yyleng);
}

<INITIAL,ST_RAW>{TABS_AND_SPACES}+ {
	/* eat whitespace */
	goto restart;
}

<INITIAL>{TABS_AND_SPACES}*{NEWLINE} {
	SCNG(lineno)++;
	return END_OF_LINE;
}

<INITIAL,ST_VALUE,ST_RAW>{TABS_AND_SPACES}*[;][^\r\n]*{NEWLINE} { /* Comment */
	BEGIN(INITIAL);
	SCNG(lineno)++;
	return END_OF_LINE;
}

<ST_VALUE,ST_RAW>[^] { /* End of option value (if EOF is reached before EOL */
	BEGIN(INITIAL);
	return END_OF_LINE;
}

<*>[^] {
	return 0;
}

*/
}
