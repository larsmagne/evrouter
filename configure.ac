dnl $Id: configure.ac 9 2009-02-18 09:09:02Z alexios $
dnl
dnl $Log$
dnl Revision 2.0  2004/09/07 22:57:36  alexios
dnl Stepped version to recover CVS repository after near-catastrophic disk
dnl crash.
dnl
dnl Revision 1.1.1.1  2004/09/07 22:52:34  alexios
dnl Initial post-crash revision, re-initialised to recover CVS repository after
dnl near-catastrophic disk crash.
dnl
dnl Revision 1.2  2004/02/12 00:13:34  alexios
dnl Changed evwheel to evrouter.
dnl
dnl Revision 1.1.1.1  2004/01/28 16:21:41  alexios
dnl Initial imported revision.
dnl

dnl Process this file with autoconf to produce a configure script.

AC_INIT(src/evrouter.c)

AC_CANONICAL_SYSTEM
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl Magic to obtain the version from debian/changelog, where it should
dnl reside.
AM_INIT_AUTOMAKE($PACKAGE,$$PACKAGE_VERSION)

dnl Essential info (some autodetected)
PACKAGE=`grep urgency= debian/changelog|head -1 | sed 's/ .*//'`
PACKAGE_VERSION=`grep urgency= debian/changelog |head -1|sed -e 's/^.*(\(.*\)).*$/\1/'`
dnl LIBVERSION=0:0:0
ISODATE=`date +%Y-%m-%d`

dnl Export essential info.
AC_SUBST(PACKAGE)
AC_SUBST(PACKAGE_VERSION)
dnl AC_SUBST(LIBVERSION)
AC_SUBST(ISODATE)

dnl Every other copy of the package version number gets its value from
dnl here (BUT NOT the files in debian/)

AM_INIT_AUTOMAKE(evrouter, $PACKAGE_VERSION)

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AC_CONFIG_HEADERS([config.h])

AC_SUBST(VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST
AC_C_CONST

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_GCC_TRADITIONAL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_MAKE_SET
AM_PROG_CC_C_O

AC_PATH_XTRA

dnl Checks for libraries

LDFLAGS="$LDFLAGS $X_LIBS"
CFLAGS="$CFLAGS $X_CFLAGS"
AC_CHECK_LIB(X11,XInitExtension,,AC_MSG_ERROR([
*** We need the libX11 development library to build this program.]))
dnl AC_CHECK_LIB(Xt,XtOpenDisplay,,AC_MSG_ERROR([
dnl *** We need the libXt development library to build this program.]))
AC_CHECK_LIB(Xtst,XTestFakeKeyEvent,,AC_MSG_ERROR([
*** We need the libXtst development library to build this program.]))

AC_CHECK_LIB(dl,dlopen,
	have_libdl=yes
	AC_DEFINE_UNQUOTED(HAVE_LIBDL, 1, [dlopen() and friends are present])
	,
	have_libdl=no)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h sys/ioctl.h unistd.h])
AC_CHECK_HEADER(argp.h,,AC_MSG_ERROR(Cannot continue: argp.h not found))

dnl Checks for functions

AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES

AC_CHECK_FUNCS([strdup strspn memset select strerror],,AC_MSG_ERROR([
*** Your C library is missing functions needed to compile this program.
*** Cowardly bailing out.]))

AC_CHECK_FUNCS([regcomp regexec],,AC_MSG_ERROR([
*** Your system lacks POSIX regular expressions. Cowardly bailing out.]))
    
dnl Checks for typedefs, structures, and compiler characteristics.

dnl Enable shell actions?

AC_ARG_ENABLE(shell,
	AS_HELP_STRING([--disable-shell],[Disable shell actions (default enabled)]),
	,[enable_shell=yes])
AC_MSG_CHECKING([whether to enable shell actions])
AC_MSG_RESULT($enable_shell)
if test x$enable_shell = xyes; then
 	AC_DEFINE_UNQUOTED(ENABLE_SHELL,1,[Enable shell actions])
fi

dnl Enable XMMS actions? (Adapted from logjam 4.2.4)

XMMS_LDFLAGS=
XMMS_CFLAGS=
AC_ARG_WITH(xmms,
AS_HELP_STRING([--with-xmms],[Enable XMMS actions (default runtime)]),,[with_xmms=yes])

if test "$have_libdl" = "no"; then
	AC_MSG_WARN([Unable to enable XMMS because this system lacks libdl])
	with_xmms=no
fi

if test "$with_xmms" = "yes"; then
	# verify they have XMMS headers
	AC_CHECK_PROG(XMMS_CONFIG, xmms-config, xmms-config, no)
	if test "$XMMS_CONFIG" = "no"; then
		AC_MSG_WARN([Cannot find xmms-config. XMMS action support disabled.])
		with_xmms=no
	else
		AC_MSG_CHECKING([XMMS library search path])
		XMMS_LIBPATH=`xmms-config --libs |
			sed -e 's/\(-[[^L]].*\)//g' -e 's/-L//g' -e 's/ *$//g' -e 's/ \+/:/g'`
		AC_MSG_RESULT($XMMS_LIBPATH)
		AC_DEFINE_UNQUOTED(XMMS_LIBPATH,"$XMMS_LIBPATH",[Possible locations of the XMMS library])

		XMMS_LDFLAGS=`$XMMS_CONFIG --libs`
		XMMS_CFLAGS=`$XMMS_CONFIG --cflags`
		AC_DEFINE_UNQUOTED(ENABLE_XMMS, 1, [XMMS action support])
	fi
fi
AC_MSG_CHECKING(whether to enable XMMS actions)
AC_MSG_RESULT($with_xmms)
AC_SUBST(XMMS_LIBPATH)
AC_SUBST(XMMS_LDFLAGS)
AC_SUBST(XMMS_CFLAGS)

dnl Generate the build infrastructure

AC_OUTPUT(Makefile src/Makefile)
