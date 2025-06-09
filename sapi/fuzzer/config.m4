AC_MSG_CHECKING(for clang fuzzer SAPI)

CRX_ARG_ENABLE([fuzzer],,
  [AS_HELP_STRING([--enable-fuzzer],
    [Build CRX as clang fuzzing test module (for developers)])],
  [no],
  [no])

dnl For newer clang versions see https://llvm.org/docs/LibFuzzer.html#fuzzer-usage
dnl for relevant flags.

dnl Macro to define fuzzing target
dnl CRX_FUZZER_TARGET(name, target-var)
dnl
AC_DEFUN([CRX_FUZZER_TARGET], [
  CRX_FUZZER_BINARIES="$CRX_FUZZER_BINARIES $SAPI_FUZZER_PATH/crx-fuzz-$1"
  CRX_SUBST($2)
  CRX_ADD_SOURCES_X([sapi/fuzzer],[fuzzer-$1.c],[],$2)
  $2="[$]$2 $FUZZER_COMMON_OBJS"
])

if test "$CRX_FUZZER" != "no"; then
  AC_MSG_RESULT([yes])
  dnl Don't use CRX_REQUIRE_CXX() to avoid unnecessarily pulling in -lstdc++
  AC_PROG_CXX
  AC_PROG_CXXCPP
  CRX_ADD_MAKEFILE_FRAGMENT($abs_srcdir/sapi/fuzzer/Makefile.frag)
  SAPI_FUZZER_PATH=sapi/fuzzer
  CRX_SUBST(SAPI_FUZZER_PATH)
  if test -z "$LIB_FUZZING_ENGINE"; then
    FUZZING_LIB="-fsanitize=fuzzer"
    FUZZING_CC="$CC"
    AX_CHECK_COMPILE_FLAG([-fsanitize=fuzzer-no-link], [
      CFLAGS="$CFLAGS -fsanitize=fuzzer-no-link"
      CXXFLAGS="$CXXFLAGS -fsanitize=fuzzer-no-link"
    ],[
      AC_MSG_ERROR(Compiler doesn't support -fsanitize=fuzzer-no-link)
    ])
  else
    FUZZING_LIB="$LIB_FUZZING_ENGINE"
    FUZZING_CC="$CXX -stdlib=libc++"
  fi
  CRX_SUBST(FUZZING_LIB)
  CRX_SUBST(FUZZING_CC)

  dnl CRX_SELECT_SAPI(fuzzer-parser, program, $FUZZER_SOURCES, , '$(SAPI_FUZZER_PATH)')

  CRX_ADD_BUILD_DIR([sapi/fuzzer])
  CRX_FUZZER_BINARIES=""
  CRX_BINARIES="$CRX_BINARIES fuzzer"
  CRX_INSTALLED_SAPIS="$CRX_INSTALLED_SAPIS fuzzer"

  CRX_ADD_SOURCES_X([sapi/fuzzer], [fuzzer-sapi.c], [], FUZZER_COMMON_OBJS)

  CRX_FUZZER_TARGET([parser], CRX_FUZZER_PARSER_OBJS)
  CRX_FUZZER_TARGET([execute], CRX_FUZZER_EXECUTE_OBJS)
  CRX_FUZZER_TARGET([function-jit], CRX_FUZZER_FUNCTION_JIT_OBJS)
  CRX_FUZZER_TARGET([tracing-jit], CRX_FUZZER_TRACING_JIT_OBJS)
  CRX_FUZZER_TARGET([unserialize], CRX_FUZZER_UNSERIALIZE_OBJS)
  CRX_FUZZER_TARGET([unserializehash], CRX_FUZZER_UNSERIALIZEHASH_OBJS)
  CRX_FUZZER_TARGET([json], CRX_FUZZER_JSON_OBJS)

  if test -n "$enable_exif" && test "$enable_exif" != "no"; then
    CRX_FUZZER_TARGET([exif], CRX_FUZZER_EXIF_OBJS)
  fi
  if test -n "$enable_mbstring" && test "$enable_mbstring" != "no"; then
    CRX_FUZZER_TARGET([mbstring], CRX_FUZZER_MBSTRING_OBJS)
    if test -n "$enable_mbregex" && test "$enable_mbregex" != "no"; then
      CRX_FUZZER_TARGET([mbregex], CRX_FUZZER_MBREGEX_OBJS)
    fi
  fi

  CRX_SUBST(CRX_FUZZER_BINARIES)
fi

AC_MSG_RESULT($CRX_FUZZER)
