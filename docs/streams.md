# An overview of the CRX streams abstraction

WARNING: some prototypes in this file are out of date.

## Why streams?

You may have noticed a shed-load of issock parameters flying around the CRX
code; we don't want them - they are ugly and cumbersome and force you to special
case sockets and files every time you need to work with a "user-level" CRX file
pointer.

Streams take care of that and present the CRX extension coder with an ANSI
stdio-alike API that looks much nicer and can be extended to support non file
based data sources.

## Using streams

Streams use a `crx_stream*` parameter just as ANSI stdio (fread etc.) use a
`FILE*` parameter.

The main functions are:

```c
CRXAPI size_t crx_stream_read(crx_stream * stream, char * buf, size_t count);
CRXAPI size_t crx_stream_write(crx_stream * stream, const char * buf, size_t
        count);
CRXAPI size_t crx_stream_printf(crx_stream * stream,
        const char * fmt, ...);
CRXAPI int crx_stream_eof(crx_stream * stream);
CRXAPI int crx_stream_getc(crx_stream * stream);
CRXAPI char *crx_stream_gets(crx_stream * stream, char *buf, size_t maxlen);
CRXAPI int crx_stream_close(crx_stream * stream);
CRXAPI int crx_stream_flush(crx_stream * stream);
CRXAPI int crx_stream_seek(crx_stream * stream, off_t offset, int whence);
CRXAPI off_t crx_stream_tell(crx_stream * stream);
CRXAPI int crx_stream_lock(crx_stream * stream, int mode);
```

These (should) behave in the same way as the ANSI stdio functions with similar
names: fread, fwrite, fprintf, feof, fgetc, fgets, fclose, fflush, fseek, ftell,
flock.

## Opening streams

In most cases, you should use this API:

```c
CRXAPI crx_stream *crx_stream_open_wrapper(const char *path, const char *mode,
    int options, char **opened_path);
```

Where:

* `path` is the file or resource to open.
* `mode` is the stdio compatible mode eg: "wb", "rb" etc.
* `options` is a combination of the following values:
  * `IGNORE_PATH` (default) - don't use include path to search for the file
  * `USE_PATH` - use include path to search for the file
  * `IGNORE_URL` - do not use plugin wrappers
  * `REPORT_ERRORS` - show errors in a standard format if something goes wrong.
  * `STREAM_MUST_SEEK` - If you really need to be able to seek the stream and
    don't need to be able to write to the original file/URL, use this option to
    arrange for the stream to be copied (if needed) into a stream that can be
    seek()ed.
* `opened_path` is used to return the path of the actual file opened, but if you
  used `STREAM_MUST_SEEK`, may not be valid. You are responsible for
  `efree()ing` `opened_path`.
* `opened_path` may be (and usually is) `NULL`.

If you need to open a specific stream, or convert standard resources into
streams there are a range of functions to do this defined in `crx_streams.h`. A
brief list of the most commonly used functions:

```c
CRXAPI crx_stream *crx_stream_fopen_from_file(FILE *file, const char *mode);
    /* Convert a FILE * into a stream. */

CRXAPI crx_stream *crx_stream_fopen_tmpfile(void);
    /* Open a FILE * with tmpfile() and convert into a stream. */

CRXAPI crx_stream *crx_stream_fopen_temporary_file(const char *dir,
    const char *pfx, char **opened_path);
    /* Generate a temporary file name and open it. */
```

There are some network enabled relatives in `crx_network.h`:

```c
CRXAPI crx_stream *crx_stream_sock_open_from_socket(int socket, int persistent);
    /* Convert a socket into a stream. */

CRXAPI crx_stream *crx_stream_sock_open_host(const char *host, unsigned short port,
        int socktype, int timeout, int persistent);
    /* Open a connection to a host and return a stream. */

CRXAPI crx_stream *crx_stream_sock_open_unix(const char *path, int persistent,
    struct timeval *timeout);
    /* Open a UNIX domain socket. */
```

## Stream utilities

If you need to copy some data from one stream to another, you will be please to
know that the streams API provides a standard way to do this:

```c
CRXAPI size_t crx_stream_copy_to_stream(crx_stream *src,
    crx_stream *dest, size_t maxlen);
```

If you want to copy all remaining data from the src stream, pass
`CRX_STREAM_COPY_ALL` as the maxlen parameter, otherwise maxlen indicates the
number of bytes to copy. This function will try to use mmap where available to
make the copying more efficient.

If you want to read the contents of a stream into an allocated memory buffer,
you should use:

```c
CRXAPI size_t crx_stream_copy_to_mem(crx_stream *src, char **buf,
    size_t maxlen, int persistent);
```

This function will set buf to the address of the buffer that it allocated, which
will be maxlen bytes in length, or will be the entire length of the data
remaining on the stream if you set maxlen to `CRX_STREAM_COPY_ALL`. The buffer
is allocated using `pemalloc()`. You need to call `pefree()` to release the
memory when you are done. As with `copy_to_stream`, this function will try use
mmap where it can.

If you have an existing stream and need to be able to `seek()` it, you can use
this function to copy the contents into a new stream that can be `seek()ed`:

```c
CRXAPI int crx_stream_make_seekable(crx_stream *origstream, crx_stream **newstream);
```

It returns one of the following values:

```c
#define CRX_STREAM_UNCHANGED 0 /* orig stream was seekable anyway */
#define CRX_STREAM_RELEASED  1 /* newstream should be used; origstream is no longer valid */
#define CRX_STREAM_FAILED    2 /* an error occurred while attempting conversion */
#define CRX_STREAM_CRITICAL  3 /* an error occurred; origstream is in an unknown state; you should close origstream */
```

`make_seekable` will always set newstream to be the stream that is valid if the
function succeeds. When you have finished, remember to close the stream.

NOTE: If you only need to seek forward, there is no need to call this function,
as the `crx_stream_seek` can emulate forward seeking when the whence parameter
is `SEEK_CUR`.

NOTE: Writing to the stream may not affect the original source, so it only makes
sense to use this for read-only use.

NOTE: If the origstream is network based, this function will block until the
whole contents have been downloaded.

NOTE: Never call this function with an origstream that is referenced as a
resource! It will close the origstream on success, and this can lead to a crash
when the resource is later used/released.

NOTE: If you are opening a stream and need it to be seekable, use the
`STREAM_MUST_SEEK` option to crx_stream_open_wrapper();

```c
CRXAPI int crx_stream_supports_lock(crx_stream * stream);
```

This function will return either 1 (success) or 0 (failure) indicating whether
or not a lock can be set on this stream. Typically, you can only set locks on
stdio streams.

## Casting streams

What if your extension needs to access the `FILE*` of a user level file pointer?
You need to "cast" the stream into a `FILE*`, and this is how you do it:

```c
FILE * fp;
crx_stream * stream; /* already opened */

if (crx_stream_cast(stream, CRX_STREAM_AS_STDIO, (void*)&fp, REPORT_ERRORS) == FAILURE)    {
    RETURN_FALSE;
}
```

The prototype is:

```c
CRXAPI int crx_stream_cast(crx_stream * stream, int castas, void ** ret, int show_err);
```

The `show_err` parameter, if non-zero, will cause the function to display an
appropriate error message of type `E_WARNING` if the cast fails.

`castas` can be one of the following values:

```txt
CRX_STREAM_AS_STDIO - a stdio FILE*
CRX_STREAM_AS_FD - a generic file descriptor
CRX_STREAM_AS_SOCKETD - a socket descriptor
```

If you ask a socket stream for a `FILE*`, the abstraction will use fdopen to
create it for you. Be warned that doing so may cause buffered data to be lost
if you mix ANSI stdio calls on the FILE* with crx stream calls on the stream.

If your system has the fopencookie function, crx streams can synthesize a
`FILE*` on top of any stream, which is useful for SSL sockets, memory based
streams, database streams etc. etc.

In situations where this is not desirable, you should query the stream to see if
it naturally supports `FILE *`. You can use this code snippet for this purpose:

```c
if (crx_stream_is(stream, CRX_STREAM_IS_STDIO)) {
    /* can safely cast to FILE* with no adverse side effects */
}
```

You can use:

```c
CRXAPI int crx_stream_can_cast(crx_stream * stream, int castas)
```

to find out if a stream can be cast, without actually performing the cast, so to
check if a stream is a socket you might use:

```c
if (crx_stream_can_cast(stream, CRX_STREAM_AS_SOCKETD) == SUCCESS)  {
    /* it can be a socket */
}
```

Please note the difference between `crx_stream_is` and `crx_stream_can_cast`;
`stream_is` tells you if the stream is a particular type of stream, whereas
`can_cast` tells you if the stream can be forced into the form you request. The
former doesn't change anything, while the later *might* change some state in the
stream.

## Stream internals

There are two main structures associated with a stream - the `crx_stream`
itself, which holds some state information (and possibly a buffer) and a
`crx_stream_ops` structure, which holds the "virtual method table" for the
underlying implementation.

The `crx_streams` ops struct consists of pointers to methods that implement
read, write, close, flush, seek, gets and cast operations. Of these, an
implementation need only implement write, read, close and flush. The gets method
is intended to be used for streams if there is an underlying method that can
efficiently behave as fgets. The ops struct also contains a label for the
implementation that will be used when printing error messages - the stdio
implementation has a label of `STDIO` for example.

The idea is that a stream implementation defines a `crx_stream_ops` struct, and
associates it with a `crx_stream` using `crx_stream_alloc`.

As an example, the `crx_stream_fopen()` function looks like this:

```c
CRXAPI crx_stream * crx_stream_fopen(const char * filename, const char * mode)
{
    FILE * fp = fopen(filename, mode);
    crx_stream * ret;

    if (fp) {
        ret = crx_stream_alloc(&crx_stream_stdio_ops, fp, 0, 0, mode);
        if (ret)
            return ret;

        fclose(fp);
    }
    return NULL;
}
```

`crx_stream_stdio_ops` is a `crx_stream_ops` structure that can be used to
handle `FILE*` based streams.

A socket based stream would use code similar to that above to create a stream to
be passed back to fopen_wrapper (or it's yet to be implemented successor).

The prototype for crx_stream_alloc is this:

```c
CRXAPI crx_stream * crx_stream_alloc(crx_stream_ops * ops, void * abstract,
        size_t bufsize, int persistent, const char * mode)
```

* `ops` is a pointer to the implementation,
* `abstract` holds implementation specific data that is relevant to this
  instance of the stream,
* `bufsize` is the size of the buffer to use - if 0, then buffering at the
  stream
* `level` will be disabled (recommended for underlying sources that implement
  their own buffering - such a `FILE*`)
* `persistent` controls how the memory is to be allocated - persistently so that
  it lasts across requests, or non-persistently so that it is freed at the end
  of a request (it uses pemalloc),
* `mode` is the stdio-like mode of operation - crx streams places no real
  meaning in the mode parameter, except that it checks for a `w` in the string
  when attempting to write (this may change).

The mode parameter is passed on to `fdopen/fopencookie` when the stream is cast
into a `FILE*`, so it should be compatible with the mode parameter of `fopen()`.

## Writing your own stream implementation

* **RULE #1**: when writing your own streams: make sure you have configured CRX
  with `--enable-debug`.
  Some great great pains have been taken to hook into the Crex memory manager to
  help track down allocation problems. It will also help you spot incorrect use
  of the STREAMS_DC, STREAMS_CC and the semi-private STREAMS_REL_CC macros for
  function definitions.

* RULE #2: Please use the stdio stream as a reference; it will help you
  understand the semantics of the stream operations, and it will always be more
  up to date than these docs :-)

First, you need to figure out what data you need to associate with the
`crx_stream`. For example, you might need a pointer to some memory for memory
based streams, or if you were making a stream to read data from an RDBMS like
MySQL, you might want to store the connection and rowset handles.

The stream has a field called abstract that you can use to hold this data. If
you need to store more than a single field of data, define a structure to hold
it, allocate it (use pemalloc with the persistent flag set appropriately), and
use the abstract pointer to refer to it.

For structured state you might have this:

```c
struct my_state {
    MYSQL conn;
    MYSQL_RES * result;
};

struct my_state * state = pemalloc(sizeof(struct my_state), persistent);

/* initialize the connection, and run a query, using the fields in state to
 * hold the results */

state->result = mysql_use_result(&state->conn);

/* now allocate the stream itself */
stream = crx_stream_alloc(&my_ops, state, 0, persistent, "r");

/* now stream->abstract == state */
```

Once you have that part figured out, you can write your implementation and
define your own crx_stream_ops struct (we called it my_ops in the above
example).

For example, for reading from this weird MySQL stream:

```c
static size_t crx_mysqlop_read(crx_stream * stream, char * buf, size_t count)
{
    struct my_state * state = (struct my_state*)stream->abstract;

    if (buf == NULL && count == 0)  {
        /* in this special case, crx_streams is asking if we have reached the
         * end of file */
        if (... at end of file ...)
            return EOF;
        else
            return 0;
    }

    /* pull out some data from the stream and put it in buf */
    ... mysql_fetch_row(state->result) ...
    /* we could do something strange, like format the data as XML here,
        and place that in the buf, but that brings in some complexities,
        such as coping with a buffer size too small to hold the data,
        so I won't even go in to how to do that here */
}
```

Implement the other operations - remember that write, read, close and flush are
all mandatory. The rest are optional. Declare your stream ops struct:

```c
crx_stream_ops my_ops = {
    crx_mysqlop_write, crx_mysqlop_read, crx_mysqlop_close,
    crx_mysqlop_flush, NULL, NULL, NULL,
    "Strange MySQL example"
}
```

That's it!

Take a look at the STDIO implementation in streams.c for more information about
how these operations work.

The main thing to remember is that in your close operation you need to release
and free the resources you allocated for the abstract field. In the case of the
example above, you need to use mysql_free_result on the rowset, close the
connection and then use pefree to dispose of the struct you allocated. You may
read the stream->persistent field to determine if your struct was allocated in
persistent mode or not.
