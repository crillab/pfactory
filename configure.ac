AC_PREREQ(2.59)
AC_INIT(pFactory, 1.0.2, pfactory@cril.fr)

PACKAGE=pFactory
VERSION=1.0.2

AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE(subdir-objects)

AC_PROG_CC
: ${CXXFLAGS= -g -O0}
AC_PROG_CXX

AC_PROG_RANLIB

AX_PTHREAD
LIBS="$PTHREAD_LIBS $LIBS"
CXXFLAGS="$CXXFLAGS -I$PWD/include $PTHREAD_CFLAGS"

AC_OUTPUT(Makefile src/Makefile examples/Makefile examples/helloworld/Makefile examples/display/Makefile examples/communication1/Makefile examples/communication2/Makefile)

