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
   | Author: Edin Kadribasic <edink@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#include "crx_embed.h"
#include "ext/standard/crx_standard.h"
#include "ext/standard/dl_arginfo.h"

#ifdef CRX_WIN32
#include <io.h>
#include <fcntl.h>
#endif

const char HARDCODED_INI[] =
	"html_errors=0\n"
	"register_argc_argv=1\n"
	"implicit_flush=1\n"
	"output_buffering=0\n"
	"max_execution_time=0\n"
	"max_input_time=-1\n\0";

#if defined(CRX_WIN32) && defined(ZTS)
CREX_TSRMLS_CACHE_DEFINE()
#endif

static char* crx_embed_read_cookies(void)
{
	return NULL;
}

static int crx_embed_deactivate(void)
{
	fflush(stdout);
	return SUCCESS;
}

/* Here we prefer to use write(), which is unbuffered, over fwrite(), which is
 * buffered. Using an unbuffered write operation to stdout will ensure CRX's
 * output buffering feature does not compete with a SAPI output buffer and
 * therefore we avoid situations wherein flushing the output buffer results in
 * nondeterministic behavior.
 */
static inline size_t crx_embed_single_write(const char *str, size_t str_length)
{
#ifdef CRX_WRITE_STDOUT
	crex_long ret;

	ret = write(STDOUT_FILENO, str, str_length);
	if (ret <= 0) return 0;
	return ret;
#else
	size_t ret;

	ret = fwrite(str, 1, MIN(str_length, 16384), stdout);
	return ret;
#endif
}

/* SAPIs only have unbuffered write operations. This is because CRX's output
 * buffering feature will handle any buffering of the output and invoke the
 * SAPI unbuffered write operation when it flushes the buffer.
 */
static size_t crx_embed_ub_write(const char *str, size_t str_length)
{
	const char *ptr = str;
	size_t remaining = str_length;
	size_t ret;

	while (remaining > 0) {
		ret = crx_embed_single_write(ptr, remaining);
		if (!ret) {
			crx_handle_aborted_connection();
		}
		ptr += ret;
		remaining -= ret;
	}

	return str_length;
}

static void crx_embed_flush(void *server_context)
{
	if (fflush(stdout)==EOF) {
		crx_handle_aborted_connection();
	}
}

static void crx_embed_send_header(sapi_header_struct *sapi_header, void *server_context)
{
}

/* The SAPI error logger that is called when the 'error_log' INI setting is not
 * set.
 *
 * https://www.crx.net/manual/en/errorfunc.configuration.crx#ini.error-log
 */
static void crx_embed_log_message(const char *message, int syslog_type_int)
{
	fprintf(stderr, "%s\n", message);
}

static void crx_embed_register_variables(zval *track_vars_array)
{
	crx_import_environment_variables(track_vars_array);
}

/* Module initialization (MINIT) */
static int crx_embed_startup(sapi_module_struct *sapi_module)
{
	return crx_module_startup(sapi_module, NULL);
}

EMBED_SAPI_API sapi_module_struct crx_embed_module = {
	"embed",                       /* name */
	"CRX Embedded Library",        /* pretty name */

	crx_embed_startup,             /* startup */
	crx_module_shutdown_wrapper,   /* shutdown */

	NULL,                          /* activate */
	crx_embed_deactivate,          /* deactivate */

	crx_embed_ub_write,            /* unbuffered write */
	crx_embed_flush,               /* flush */
	NULL,                          /* get uid */
	NULL,                          /* getenv */

	crx_error,                     /* error handler */

	NULL,                          /* header handler */
	NULL,                          /* send headers handler */
	crx_embed_send_header,         /* send header handler */

	NULL,                          /* read POST data */
	crx_embed_read_cookies,        /* read Cookies */

	crx_embed_register_variables,  /* register server variables */
	crx_embed_log_message,         /* Log message */
	NULL,                          /* Get request time */
	NULL,                          /* Child terminate */

	STANDARD_SAPI_MODULE_PROPERTIES
};
/* }}} */

static const crex_function_entry additional_functions[] = {
	CREX_FE(dl, arginfo_dl)
	CREX_FE_END
};

EMBED_SAPI_API int crx_embed_init(int argc, char **argv)
{
#if defined(SIGPIPE) && defined(SIG_IGN)
	signal(SIGPIPE, SIG_IGN); /* ignore SIGPIPE in standalone mode so
								 that sockets created via fsockopen()
								 don't kill CRX if the remote site
								 closes it.  in apache|apxs mode apache
								 does that for us!  thies@thieso.net
								 20000419 */
#endif

#ifdef ZTS
	crx_tsrm_startup();
# ifdef CRX_WIN32
	CREX_TSRMLS_CACHE_UPDATE();
# endif
#endif

	crex_signal_startup();

	/* SAPI initialization (SINIT)
	 *
	 * Initialize the SAPI globals (memset to 0). After this point we can set
	 * SAPI globals via the SG() macro.
	 *
	 * Reentrancy startup.
	 *
	 * This also sets 'crx_embed_module.ini_entries = NULL' so we cannot
	 * allocate the INI entries until after this call.
	 */
	sapi_startup(&crx_embed_module);

#ifdef CRX_WIN32
	_fmode = _O_BINARY;			/*sets default for file streams to binary */
	setmode(_fileno(stdin), O_BINARY);		/* make the stdio mode be binary */
	setmode(_fileno(stdout), O_BINARY);		/* make the stdio mode be binary */
	setmode(_fileno(stderr), O_BINARY);		/* make the stdio mode be binary */
#endif

	/* This hard-coded string of INI settings is parsed and read into CRX's
	 * configuration hash table at the very end of crx_init_config(). This
	 * means these settings will overwrite any INI settings that were set from
	 * an INI file.
	 *
	 * To provide overwritable INI defaults, hook the ini_defaults function
	 * pointer that is part of the sapi_module_struct
	 * (crx_embed_module.ini_defaults).
	 *
	 *     void (*ini_defaults)(HashTable *configuration_hash);
	 *
	 * This callback is invoked as soon as the configuration hash table is
	 * allocated so any INI settings added via this callback will have the
	 * lowest precedence and will allow INI files to overwrite them.
	 */
	crx_embed_module.ini_entries = HARDCODED_INI;

	/* SAPI-provided functions. */
	crx_embed_module.additional_functions = additional_functions;

	if (argv) {
		crx_embed_module.executable_location = argv[0];
	}

	/* Module initialization (MINIT) */
	if (crx_embed_module.startup(&crx_embed_module) == FAILURE) {
		return FAILURE;
	}

	/* Do not chdir to the script's directory. This is akin to calling the CGI
	 * SAPI with '-C'.
	 */
	SG(options) |= SAPI_OPTION_NO_CHDIR;

	SG(request_info).argc=argc;
	SG(request_info).argv=argv;

	/* Request initialization (RINIT) */
	if (crx_request_startup() == FAILURE) {
		crx_module_shutdown();
		return FAILURE;
	}

	SG(headers_sent) = 1;
	SG(request_info).no_headers = 1;
	crx_register_variable("CRX_SELF", "-", NULL);

	return SUCCESS;
}

EMBED_SAPI_API void crx_embed_shutdown(void)
{
	/* Request shutdown (RSHUTDOWN) */
	crx_request_shutdown((void *) 0);

	/* Module shutdown (MSHUTDOWN) */
	crx_module_shutdown();

	/* SAPI shutdown (SSHUTDOWN) */
	sapi_shutdown();

#ifdef ZTS
	tsrm_shutdown();
#endif
}
