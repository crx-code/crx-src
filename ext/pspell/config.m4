CRX_ARG_WITH([pspell],
  [for PSPELL support],
  [AS_HELP_STRING([[--with-pspell[=DIR]]],
    [Include PSPELL support. GNU Aspell version 0.50.0 or higher required])])

if test "$CRX_PSPELL" != "no"; then
	dnl Add -Wno-strict-prototypes as depends on user libs
	CRX_NEW_EXTENSION(pspell, pspell.c, $ext_shared, , "-Wno-strict-prototypes")
	if test "$CRX_PSPELL" != "yes"; then
	    PSPELL_SEARCH_DIRS=$CRX_PSPELL
	else
	    PSPELL_SEARCH_DIRS="/usr/local /usr"
	fi
	for i in $PSPELL_SEARCH_DIRS; do
		if test -f $i/include/pspell/pspell.h; then
			PSPELL_DIR=$i
			PSPELL_INCDIR=$i/include/pspell
		elif test -f $i/include/pspell.h; then
			PSPELL_DIR=$i
			PSPELL_INCDIR=$i/include
		fi
	done

	if test -z "$PSPELL_DIR"; then
		AC_MSG_ERROR(Cannot find pspell)
	fi

	PSPELL_LIBDIR=$PSPELL_DIR/$CRX_LIBDIR

	CRX_ADD_LIBRARY_WITH_PATH(pspell, $PSPELL_LIBDIR, PSPELL_SHARED_LIBADD)

	dnl Add -laspell to LIBS if it exists
	CRX_CHECK_LIBRARY(aspell,new_aspell_config,
	[
		CRX_ADD_LIBRARY_WITH_PATH(aspell, $PSPELL_LIBDIR, PSPELL_SHARED_LIBADD)
	], [], [
		-L$PSPELL_LIBDIR
	])

	CRX_ADD_INCLUDE($PSPELL_INCDIR)
	CRX_SUBST(PSPELL_SHARED_LIBADD)
	AC_DEFINE(HAVE_PSPELL,1,[ ])
fi
