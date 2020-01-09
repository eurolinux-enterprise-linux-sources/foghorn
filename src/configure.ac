#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([foghorn], [0.1.2], [http://bugzilla.redhat.com/])
AM_INIT_AUTOMAKE([foghorn], [0.1.2])
AC_CONFIG_SRCDIR([src/snmp.c])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AM_PROG_CC_C_O

AC_CHECK_HEADERS([stdlib.h string.h syslog.h unistd.h])

AC_TYPE_SIZE_T

PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.22.0])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES([DBUS], [dbus-1 >= 1.2.20])
AC_SUBST(DBUS_CFLAGS)
AC_SUBST(DBUS_LIBS)

PKG_CHECK_MODULES([DBUS_GLIB], [dbus-glib-1 >= 0.86])
AC_SUBST(DBUS_GLIB_CFLAGS)
AC_SUBST(DBUS_GLIB_LIBS)

AC_CHECK_HEADERS([net-snmp/net-snmp-config.h])
AC_PATH_PROGS([SNMP_CONFIG], [net-snmp-config])

if test "X${SNMP_CONFIG}" = "X"; then
   AC_MSG_ERROR(net-snmp-config not found)
fi

SNMP_CFLAGS=`$SNMP_CONFIG --cflags`
SNMP_LIBS=`$SNMP_CONFIG --agent-libs`

AC_SUBST([SNMP_CFLAGS])
AC_SUBST([SNMP_LIBS])

AC_OUTPUT([Makefile src/Makefile])
