AC_PREREQ([2.69])
AC_INIT([BMFS], [1.1], [ian.seyler@returninfinity.com])
AM_INIT_AUTOMAKE(foreign])
AC_CONFIG_SRCDIR([src/bmfs.c])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC_C99

AC_CHECK_HEADERS([stdlib.h string.h strings.h])

AC_TYPE_SIZE_T

AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strcasecmp])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
