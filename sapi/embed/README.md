# The embed SAPI

A server application programming interface (SAPI) is the entry point into the Crex Engine. The embed SAPI is a lightweight SAPI for calling into the Crex Engine from C or other languages that have C bindings.

## Basic Example

Below is a basic example in C that uses the embed SAPI to boot up the Crex Engine, start a request, and print the number of functions loaded in the function table.

```c
/* embed_sapi_basic_example.c */

#include <sapi/embed/crx_embed.h>

int main(int argc, char **argv)
{
	/* Invokes the Crex Engine initialization phase: SAPI (SINIT), modules
	 * (MINIT), and request (RINIT). It also opens a 'crex_try' block to catch
	 * a crex_bailout().
	 */
	CRX_EMBED_START_BLOCK(argc, argv)

	crx_printf(
		"Number of functions loaded: %d\n",
		crex_hash_num_elements(EG(function_table))
	);

	/* Close the 'crex_try' block and invoke the shutdown phase: request
	 * (RSHUTDOWN), modules (MSHUTDOWN), and SAPI (SSHUTDOWN).
	 */
	CRX_EMBED_END_BLOCK()
}
```

To compile this, we must point the compiler to the CRX header files. The paths to the header files are listed from `crx-config --includes`.

We must also point the linker and the runtime loader to the `libcrx.so` shared lib for linking CRX (`-lcrx`) which is located at `$(crx-config --prefix)/lib`. So the complete command to compile ends up being:

```bash
$  gcc \
	$(crx-config --includes) \
	-L$(crx-config --prefix)/lib \
	embed_sapi_basic_example.c \
	-lcrx \
	-Wl,-rpath=$(crx-config --prefix)/lib
```

> :memo: The embed SAPI is disabled by default. In order for the above example to compile, CRX must be built with the embed SAPI enabled. To see what SAPIs are installed, run `crx-config --crx-sapis`. If you don't see `embed` in the list, you'll need to rebuild CRX with `./configure --enable-embed`. The CRX shared library `libcrx.so` is built when the embed SAPI is enabled.

If all goes to plan you should be able to run the program.

```bash
$ ./a.out 
Number of functions loaded: 1046
```

## Function call example

The following example calls `mt_rand()` and `var_dump()`s the return value.

```c
#include <main/crx.h>
#include <ext/standard/crx_var.h>
#include <sapi/embed/crx_embed.h>

int main(int argc, char **argv)
{
	CRX_EMBED_START_BLOCK(argc, argv)

	zval retval = {0};
	crex_fcall_info fci = {0};
	crex_fcall_info_cache fci_cache = {0};

	crex_string *func_name = crex_string_init(CREX_STRL("mt_rand"), 0);
	ZVAL_STR(&fci.function_name, func_name);

	fci.size = sizeof fci;
	fci.retval = &retval;

	if (crex_call_function(&fci, &fci_cache) == SUCCESS) {
		crx_var_dump(&retval, 1);
	}

	crex_string_release(func_name);

	CRX_EMBED_END_BLOCK()
}
```

## Execute a CRX script example

```crx
<?crx

# example.crx

echo 'Hello from userland!' . CRX_EOL;
```

```c
#include <sapi/embed/crx_embed.h>

int main(int argc, char **argv)
{
	CRX_EMBED_START_BLOCK(argc, argv)

	crex_file_handle file_handle;
	crex_stream_init_filename(&file_handle, "example.crx");

	if (crx_execute_script(&file_handle) == FAILURE) {
		crx_printf("Failed to execute CRX script.\n");
	}

	CRX_EMBED_END_BLOCK()
}
```

## INI defaults

The default value for 'error_prepend_string' is 'NULL'. The following example sets the INI default for 'error_prepend_string' to 'Embed SAPI error:'.

```c
#include <sapi/embed/crx_embed.h>

/* This callback is invoked as soon as the configuration hash table is
 * allocated so any INI settings added via this callback will have the lowest
 * precedence and will allow INI files to overwrite them.
 */
static void example_ini_defaults(HashTable *configuration_hash)
{
	zval ini_value;
	ZVAL_NEW_STR(&ini_value, crex_string_init(CREX_STRL("Embed SAPI error:"), /* persistent */ 1));
	crex_hash_str_update(configuration_hash, CREX_STRL("error_prepend_string"), &ini_value);
}

int main(int argc, char **argv)
{
	crx_embed_module.ini_defaults = example_ini_defaults;

	CRX_EMBED_START_BLOCK(argc, argv)

	zval retval;

	/* Generates an error by accessing an undefined variable '$a'. */
	if (crex_eval_stringl(CREX_STRL("var_dump($a);"), &retval, "example") == FAILURE) {
		crx_printf("Failed to eval CRX.\n");
	}

	CRX_EMBED_END_BLOCK()
}
```

After compiling and running, you should see:

```
Embed SAPI error:
Warning: Undefined variable $a in example on line 1
NULL
```

This default value is overwritable from INI files. We'll update one of the INI files (which can be found by running `$ crx --ini`), and set `error_prepend_string="Oops!"`. We don't have to recompile the program, we can just run it again and we should see:

```
Oops!
Warning: Undefined variable $a in example on line 1
NULL
```
