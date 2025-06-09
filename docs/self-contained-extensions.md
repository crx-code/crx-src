# How to create a self-contained CRX extension

A self-contained extension can be distributed independently of the CRX source.
To create such an extension, two things are required:

* Configuration file (config.m4)
* Source code for your module

We will describe now how to create these and how to put things together.

## Preparing your system

While the result will run on any system, a developer's setup needs these tools:

* GNU autoconf
* GNU libtool
* GNU m4

All of these are available from

    ftp://ftp.gnu.org/pub/gnu/

## Converting an existing extension

Just to show you how easy it is to create a self-contained extension, we will
convert an embedded extension into a self-contained one. Install CRX and execute
the following commands.

```bash
mkdir /tmp/newext
cd /tmp/newext
```

You now have an empty directory. We will copy the files from the mysqli
extension:

```bash
cp -rp crx-src/ext/mysqli/* .
```

It is time to finish the module. Run:

```bash
crxize
```

You can now ship the contents of the directory - the extension can live
completely on its own.

The user instructions boil down to

```bash
./configure \
    [--with-crx-config=/path/to/crx-config] \
    [--with-mysqli=MYSQL-DIR]
make install
```

The MySQL module will either use the embedded MySQL client library or the MySQL
installation in MYSQL-DIR.

## Defining the new extension

Our demo extension is called "foobar".

It consists of two source files `foo.c` and `bar.c` (and any arbitrary amount of
header files, but that is not important here).

The demo extension does not reference any external libraries (that is important,
because the user does not need to specify anything).

`LTLIBRARY_SOURCES` specifies the names of the sources files. You can name an
arbitrary number of source files here.

## Creating the M4 configuration file

The m4 configuration can perform additional checks. For a self-contained
extension, you do not need more than a few macro calls.

```m4
CRX_ARG_ENABLE([foobar],
  [whether to enable foobar],
  [AS_HELP_STRING([--enable-foobar],
    [Enable foobar])])

if test "$CRX_FOOBAR" != "no"; then
  CRX_NEW_EXTENSION(foobar, foo.c bar.c, $ext_shared)
fi
```

`CRX_ARG_ENABLE` will automatically set the correct variables, so that the
extension will be enabled by `CRX_NEW_EXTENSION` in shared mode.

The first argument of `CRX_NEW_EXTENSION` describes the name of the extension.
The second names the source-code files. The third passes `$ext_shared` which is
set by `CRX_ARG_ENABLE/WITH` to `CRX_NEW_EXTENSION`.

Please use always `CRX_ARG_ENABLE` or `CRX_ARG_WITH`. Even if you do not plan to
distribute your module with CRX, these facilities allow you to integrate your
module easily into the main CRX module framework.

## Create source files

`ext_skel.crx` can be of great help when creating the common code for all
modules in CRX for you and also writing basic function definitions and C code
for handling arguments passed to your functions. See `./ext/ext_skel.crx --help`
for further information.

As for the rest, you are currently alone here. There are a lot of existing
modules, use a simple module as a starting point and add your own code.

## Creating the self-contained extension

Put `config.m4` and the source files into one directory. Then, run `crxize`
(this is installed during `make install` by CRX).

For example, if you configured CRX with `--prefix=/crx`, you would run

```bash
/crx/bin/crxize
```

This will automatically copy the necessary build files and create configure from
your `config.m4`.

And that's it. You now have a self-contained extension.

## Installing a self-contained extension

An extension can be installed by running:

```bash
./configure \
    [--with-crx-config=/path/to/crx-config]
make install
```

## Adding shared module support to a module

In order to be useful, a self-contained extension must be loadable as a shared
module. The following will explain now how you can add shared module support to
an existing module called `foo`.

1. In `config.m4`, use `CRX_ARG_WITH/CRX_ARG_ENABLE`. Then you will
   automatically be able to use `--with-foo=shared[,..]` or
   `--enable-foo=shared[,..]`.

2. In `config.m4`, use `CRX_NEW_EXTENSION(foo,.., $ext_shared)` to enable
   building the extension.

3. Add the following lines to your C source file:

```c
#ifdef COMPILE_DL_FOO
    CREX_GET_MODULE(foo)
#endif
```

## PECL site conformity

If you plan to release an extension to the PECL website, there are several
points to be regarded.

1. Add `LICENSE` or `COPYING` to the `package.xml`

2. The following should be defined in one of the extension header files

```c
#define CRX_FOO_VERSION "1.2.3"
```

This macro has to be used within your foo_module_entry to indicate the
extension version.
