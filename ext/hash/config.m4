CRX_ARG_WITH([mhash],
  [for mhash support],
  [AS_HELP_STRING([[--with-mhash]],
    [Include mhash support])])

if test "$CRX_MHASH" != "no"; then
  AC_DEFINE(CRX_MHASH_BC, 1, [ ])
fi

if test $ac_cv_c_bigendian_crx = yes; then
  EXT_HASH_SHA3_SOURCES="hash_sha3.c"
  AC_DEFINE(HAVE_SLOW_HASH3, 1, [Define if hash3 algo is available])
  AC_MSG_WARN("Use SHA3 slow implementation on bigendian")
else
  AC_CHECK_SIZEOF([long])
  AC_MSG_CHECKING([if we're at 64-bit platform])
  AS_IF([test "$ac_cv_sizeof_long" -eq 4],[
    AC_MSG_RESULT([no])
    SHA3_DIR="sha3/generic32lc"
    SHA3_OPT_SRC="$SHA3_DIR/KeccakP-1600-inplace32BI.c"
  ],[
    AC_MSG_RESULT([yes])
    SHA3_DIR="sha3/generic64lc"
    SHA3_OPT_SRC="$SHA3_DIR/KeccakP-1600-opt64.c"
  ])
  EXT_HASH_SHA3_SOURCES="$SHA3_OPT_SRC $SHA3_DIR/KeccakHash.c $SHA3_DIR/KeccakSponge.c hash_sha3.c"
  dnl Add -Wno-implicit-fallthrough flag as it happens on 32 bit builds
  CRX_HASH_CFLAGS="-Wno-implicit-fallthrough -I@ext_srcdir@/$SHA3_DIR -DKeccakP200_excluded -DKeccakP400_excluded -DKeccakP800_excluded -DCREX_ENABLE_STATIC_TSRMLS_CACHE=1"

  CRX_ADD_BUILD_DIR(ext/hash/$SHA3_DIR, 1)
fi

CRX_ADD_BUILD_DIR(ext/hash/murmur, 1)
CRX_HASH_CFLAGS="$CRX_HASH_CFLAGS -I@ext_srcdir@/xxhash"

EXT_HASH_SOURCES="hash.c hash_md.c hash_sha.c hash_ripemd.c hash_haval.c \
  hash_tiger.c hash_gost.c hash_snefru.c hash_whirlpool.c hash_adler32.c \
  hash_crc32.c hash_fnv.c hash_joaat.c $EXT_HASH_SHA3_SOURCES
  murmur/PMurHash.c murmur/PMurHash128.c hash_murmur.c hash_xxhash.c"
EXT_HASH_HEADERS="crx_hash.h crx_hash_md.h crx_hash_sha.h crx_hash_ripemd.h \
  crx_hash_haval.h crx_hash_tiger.h crx_hash_gost.h crx_hash_snefru.h \
  crx_hash_whirlpool.h crx_hash_adler32.h crx_hash_crc32.h \
  crx_hash_fnv.h crx_hash_joaat.h crx_hash_sha3.h crx_hash_murmur.h \
  crx_hash_xxhash.h"

CRX_NEW_EXTENSION(hash, $EXT_HASH_SOURCES, 0,,$CRX_HASH_CFLAGS)
CRX_INSTALL_HEADERS(ext/hash, $EXT_HASH_HEADERS)
