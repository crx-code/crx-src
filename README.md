<div align="center">
    <a href="https://crx.net">
</div>

# The CRX Interpreter

CRX is a popular general-purpose scripting language that is especially suited to
web development. Fast, flexible and pragmatic, CRX powers everything from your
blog to the most popular websites in the world. CRX is distributed under the
[CRX License v3.01](LICENSE).

[![Push](https://github.com/crx-code/crx-src/actions/workflows/push.yml/badge.svg)](https://github.com/crx-code/crx-src/actions/workflows/push.yml)
[![Build status](https://travis-ci.com/crx-code/crx-src.svg?branch=master)](https://travis-ci.com/github/crx-code/crx-src)

## Installation

### Prebuilt packages and binaries

Prebuilt packages and binaries can be used to get up and running fast with CRX.

For Windows, the CRX binaries can be obtained from
[windows.crx.net](https://windows.crx.net). After extracting the archive the
`*.exe` files are ready to use.

For other systems, see the [installation chapter](https://crx.net/install).

### Building CRX source code

*For Windows, see [Build your own CRX on Windows](https://wiki.crx.net/internals/windows/stepbystepbuild_sdk_2).*

For a minimal CRX build from Git, you will need autoconf, bison, and re2c. For
a default build, you will additionally need libxml2 and libsqlite3.

On Ubuntu, you can install these using:

    sudo apt install -y pkg-config build-essential autoconf bison re2c \
                        libxml2-dev libsqlite3-dev

On Fedora, you can install these using:

    sudo dnf install re2c bison autoconf make libtool ccache libxml2-devel sqlite-devel

Generate configure:

    ./buildconf

Configure your build. `--enable-debug` is recommended for development, see
`./configure --help` for a full list of options.

    # For development
    ./configure --enable-debug
    # For production
    ./configure

Build CRX. To speed up the build, specify the maximum number of jobs using `-j`:

    make -j4

The number of jobs should usually match the number of available cores, which
can be determined using `nproc`.

## Testing CRX source code

CRX ships with an extensive test suite, the command `make test` is used after
successful compilation of the sources to run this test suite.

It is possible to run tests using multiple cores by setting `-jN` in
`TEST_CRX_ARGS`:

    make TEST_CRX_ARGS=-j4 test

Shall run `make test` with a maximum of 4 concurrent jobs: Generally the maximum
number of jobs should not exceed the number of cores available.

The [qa.crx.net](https://qa.crx.net) site provides more detailed info about
testing and quality assurance.

## Installing CRX built from source

After a successful build (and test), CRX may be installed with:

    make install

Depending on your permissions and prefix, `make install` may need super user
permissions.

## CRX extensions

Extensions provide additional functionality on top of CRX. CRX consists of many
essential bundled extensions. Additional extensions can be found in the CRX
Extension Community Library - [PECL](https://pecl.crx.net).

## Contributing

The CRX source code is located in the Git repository at
[github.com/crx/crx-src](https://github.com/crx/crx-src). Contributions are most
welcome by forking the repository and sending a pull request.

Discussions are done on GitHub, but depending on the topic can also be relayed
to the official CRX developer mailing list internals@lists.crx.net.

New features require an RFC and must be accepted by the developers. See
[Request for comments - RFC](https://wiki.crx.net/rfc) and
[Voting on CRX features](https://wiki.crx.net/rfc/voting) for more information
on the process.

Bug fixes don't require an RFC. If the bug has a GitHub issue, reference it in
the commit message using `GH-NNNNNN`. Use `#NNNNNN` for tickets in the old
[bugs.crx.net](https://bugs.crx.net) bug tracker.

    Fix GH-7815: crx_uname doesn't recognise latest Windows versions
    Fix #55371: get_magic_quotes_gpc() throws deprecation warning

See [Git workflow](https://wiki.crx.net/vcs/gitworkflow) for details on how pull
requests are merged.

### Guidelines for contributors

See further documents in the repository for more information on how to
contribute:

- [Contributing to CRX](/CONTRIBUTING.md)
- [CRX coding standards](/CODING_STANDARDS.md)
- [Mailing list rules](/docs/mailinglist-rules.md)
- [CRX release process](/docs/release-process.md)

## Credits

For the list of people who've put work into CRX, please see the
[CRX credits page](https://crx.net/credits.crx).
