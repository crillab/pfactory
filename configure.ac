AC_PREREQ(2.59)
AC_INIT(pFactory, 2.0.0, pfactory@cril.fr)

PACKAGE=pFactory
VERSION=2.0.0

AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE(subdir-objects)

AC_PROG_CC
: ${CXXFLAGS= -O3}
AC_PROG_CXX

AC_PROG_RANLIB

AX_PTHREAD
LIBS="$PTHREAD_LIBS $LIBS"
CXXFLAGS="$CXXFLAGS -I$PWD/include $PTHREAD_CFLAGS"

AC_OUTPUT(Makefile)
AC_OUTPUT(src/Makefile)
AC_OUTPUT(examples/Makefile)
AC_OUTPUT(examples/helloworld/Makefile)
AC_OUTPUT(examples/display/Makefile)
AC_OUTPUT(examples/communicator/Makefile)
AC_OUTPUT(examples/restrictedcommunicator/Makefile)
AC_OUTPUT(examples/intercommunicator/Makefile)
AC_OUTPUT(examples/barrier/Makefile)
AC_OUTPUT(examples/staticDC/Makefile)
AC_OUTPUT(examples/dynamicDC/Makefile)
AC_OUTPUT(examples/concurrent/Makefile)
AC_OUTPUT(examples/multipleconcurrents/Makefile)

#AC_OUTPUT(examples/groups/Makefile)

