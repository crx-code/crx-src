CRX_ARG_ENABLE([sysvsem],
  [whether to enable System V semaphore support],
  [AS_HELP_STRING([--enable-sysvsem],
    [Enable System V semaphore support])])

if test "$CRX_SYSVSEM" != "no"; then
 CRX_NEW_EXTENSION(sysvsem, sysvsem.c, $ext_shared)
 AC_DEFINE(HAVE_SYSVSEM, 1, [ ])
 AC_CACHE_CHECK(for union semun,crx_cv_semun,
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
   ]], [[union semun x;]])],[
     crx_cv_semun=yes
   ],[
     crx_cv_semun=no
   ])
 )
 if test "$crx_cv_semun" = "yes"; then
   AC_DEFINE(HAVE_SEMUN, 1, [ ])
 else
   AC_DEFINE(HAVE_SEMUN, 0, [ ])
 fi
fi
