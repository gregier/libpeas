dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])

m4_define(peas_major_version,  0)
m4_define(peas_minor_version,  5)
m4_define(peas_micro_version,  0)
m4_define(peas_interface_age,  0)

m4_define(peas_binary_age, [m4_eval(100 * peas_minor_version + peas_micro_version)])
m4_define(peas_version, [peas_major_version.peas_minor_version.peas_micro_version])

dnl libtool version related macros
m4_define(peas_lt_current, [m4_eval(100 * peas_minor_version + peas_micro_version - peas_interface_age)])
m4_define(peas_lt_revision, [peas_interface_age])
m4_define(peas_lt_age, [m4_eval(peas_binary_age - peas_interface_age)])

AC_INIT([libpeas],[peas_version],[http://bugzilla.gnome.org/enter_bug.cgi?product=gedit],[libpeas])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([libpeas/peas-engine.c])
AC_CONFIG_MACRO_DIR([m4])

AC_DEFINE(PEAS_MAJOR_VERSION, peas_major_version, [Gedit Plugins Engine major version])
AC_SUBST(PEAS_MAJOR_VERSION, peas_major_version)
AC_DEFINE(PEAS_MINOR_VERSION, peas_minor_version, [Gedit Plugins Engine minor version])
AC_SUBST(PEAS_MINOR_VERSION, peas_minor_version)
AC_DEFINE(PEAS_MICRO_VERSION, peas_micro_version, [Gedit Plugins Engine micro version])
AC_SUBST(PEAS_MICRO_VERSION, peas_micro_version)

AC_SUBST(PEAS_VERSION, peas_version)

AM_INIT_AUTOMAKE([1.11 dist-bzip2 no-dist-gzip -Wno-portability])
AM_MAINTAINER_MODE([enable])

AM_SILENT_RULES([yes])

dnl ================================================================
dnl Basic checks
dnl ================================================================

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT([disable-static])


AC_CHECK_FUNCS(fsync)
AC_CHECK_FUNC(sigaction)
AC_CHECK_SIZEOF(time_t, [], [#include <time.h>])

PKG_PROG_PKG_CONFIG

dnl ================================================================
dnl Check for native OSX
dnl ================================================================
AC_MSG_CHECKING([for native Mac OS X])

gdk_windowing=`$PKG_CONFIG --variable=target gdk-2.0`

if test "$gdk_windowing" = "quartz"; then
	os_osx=yes
else
	os_osx=no
fi
AC_MSG_RESULT([$os_osx])
AM_CONDITIONAL(OS_OSX, test "$os_osx" = "yes")

if test "$os_osx" = "yes"; then
	AC_DEFINE([OS_OSX],[1],[Defined if os is Mac OSX])

	PKG_CHECK_MODULES(IGE_MAC, ige-mac-integration)
fi

dnl ================================================================
dnl Gettext stuff.
dnl ================================================================

IT_PROG_INTLTOOL([0.40.0])

GETTEXT_PACKAGE=libpeas
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])

AM_GLIB_GNU_GETTEXT

dnl ================================================================
dnl Start of pkg-config checks
dnl ================================================================

PKG_CHECK_MODULES(PEAS, [
	glib-2.0 >= 2.18.0
	gobject-2.0 >= 2.23.6
	gmodule-2.0 >= 2.18.0
	gobject-introspection-1.0 >= 0.6.7
])

AC_MSG_CHECKING([for Gtk+ 3.0 availability.])
PKG_CHECK_EXISTS([gtk+-3.0 >= 2.90],[found_gtk3=yes],[found_gtk3=no])
AC_MSG_RESULT([$found_seed])

if test "$found_gtk3" = "yes"; then
	PKG_CHECK_MODULES(PEASUI, [gtk+-3.0 >= 2.90.0])
	GTK_TYPELIB=Gtk-3.0
else
	PKG_CHECK_MODULES(PEASUI, [gtk+-2.0 >= 2.16.0])
	GTK_TYPELIB=Gtk-2.0
fi
AC_SUBST(GTK_TYPELIB)


dnl ================================================================
dnl GObject Introspection
dnl ================================================================

GOBJECT_INTROSPECTION_REQUIRE([0.6.7])

dnl ================================================================
dnl Seed Javascript Engine
dnl ================================================================

SEED_REQUIRED=2.28.0

AC_ARG_ENABLE(seed,
	      AS_HELP_STRING([--enable-seed],[Enable Seed support]),
	      [enable_seed=$enableval],
	      [enable_seed=auto])

AC_MSG_CHECKING([for Seed JS availability.])

if test "x$enable_seed" = "xno"; then
	found_seed="no (disabled, use --enable-seed to enable)"
else
	PKG_CHECK_EXISTS([seed >= $SEED_REQUIRED],
			 [found_seed=yes],
			 [found_seed=no])
fi

if test "$enable_seed" = "yes" -a "$found_seed" = "no"; then
	AC_MSG_ERROR([You need to have seed >= $SEED_REQUIRED installed to build libpeas])
fi
AC_MSG_RESULT([$found_seed])

if test "$found_seed" = "yes"; then
	SEED_CFLAGS=`$PKG_CONFIG --cflags seed`
	SEED_LIBS=`$PKG_CONFIG --libs seed`
	AC_SUBST(SEED_CFLAGS)
	AC_SUBST(SEED_LIBS)

	AC_DEFINE(ENABLE_SEED,1,[Define to compile with Seed support])
fi

AM_CONDITIONAL([ENABLE_SEED],[test "x$found_seed" = "xyes"])

dnl ================================================================
dnl Python
dnl ================================================================

PYTHON_REQUIRED=2.5.2
PYGOBJECT_REQUIRED=2.20

AC_ARG_ENABLE(python,
	      AS_HELP_STRING([--enable-python],[Enable Python support]),
	      [enable_python=$enableval],
	      [enable_python=auto])

if test "$enable_python" = "no"; then
	found_python="no (disabled, use --enable-python to enable)"
else
	AM_PATH_PYTHON($PYTHON_REQUIRED,
		       [found_python=yes],
		       [found_python="no (python interpretor >= ${PYTHON_REQUIRED} not found"])

	if test "$found_python" = "yes"; then
		AC_PATH_TOOL(PYTHON_CONFIG, "python${PYTHON_VERSION}-config")
		if test -z "$PYTHON_CONFIG"; then
			AC_PATH_TOOL(PYTHON_CONFIG, "python-config-${PYTHON_VERSION}")
			if test -z "$PYTHON_CONFIG"; then
				found_python="no (python${PYTHON_VERSION}-config not found)"
			fi
		fi
	fi

	if test "$found_python" = "yes"; then
		save_CPPFLAGS="${CPPFLAGS}"
		CPPFLAGS="$CPPFLAGS `$PYTHON_CONFIG --includes`"
		AC_CHECK_HEADER(Python.h,,[found_python="no (Python headers not found)"])
		CPPFLAGS="${save_CPPFLAGS}"
	fi

	AC_MSG_CHECKING([for PyGObject availability.])
	if test "$found_python" = "yes"; then
		PKG_CHECK_EXISTS([pygobject-2.0 >= $PYGOBJECT_REQUIRED],,
				 [found_python="no (pygobject >= $PYGOBJECT_REQURED) not found)"])
	fi
	AC_MSG_RESULT([$found_python])

	dnl FIXME: shall we check explicitely for PyGI here? Strictly speaking,
	dnl it's not required for building libpeas, merely to use it properly.
fi

if test "$enable_python" = "yes" -a "$found_python" = "no"; then
	AC_MSG_ERROR([$found_python])
fi

if test "$found_python" = "yes"; then
	PYTHON_CFLAGS="`$PKG_CONFIG --cflags pygobject-2.0` `$PYTHON_CONFIG --includes`"
	PYTHON_LIBS="`$PKG_CONFIG --libs pygobject-2.0` `$PYTHON_CONFIG --libs`"
	AC_SUBST(PYTHON_CFLAGS)
	AC_SUBST(PYTHON_LIBS)

	AC_DEFINE(ENABLE_PYTHON,1,[Define to compile with Python support])
fi

AM_CONDITIONAL([ENABLE_PYTHON],[test "$found_python" = "yes"])

dnl ================================================================
dnl Documentation
dnl ================================================================

GTK_DOC_CHECK([1.11],[--flavour no-tmpl])

dnl ================================================================
dnl Libtool
dnl ================================================================

LT_CURRENT=peas_lt_current
LT_REVISION=peas_lt_revision
LT_AGE=peas_lt_age
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

PLUGIN_LIBTOOL_FLAGS="-module -avoid-version"
LOADER_LIBTOOL_FLAGS="-module -avoid-version -export-symbols-regex peas_register_types"
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)
AC_SUBST(LOADER_LIBTOOL_FLAGS)

dnl ================================================================
dnl Misc
dnl ================================================================
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)
AC_PATH_PROG(GLIB_MKENUMS, glib-mkenums)

GNOME_COMPILE_WARNINGS([maximum])
# Use --enable-maintainer-mode to disabled deprecated symbols
GNOME_MAINTAINER_MODE_DEFINES


AC_CONFIG_FILES([
Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/version.xml
libpeas/Makefile
libpeasui/Makefile
loaders/Makefile
loaders/c/Makefile
loaders/python/Makefile
loaders/seed/Makefile
data/Makefile
data/libpeas-1.0.pc
data/libpeasui-1.0.pc
data/icons/Makefile
peas-demo/Makefile
peas-demo/plugins/Makefile
peas-demo/plugins/helloworld/Makefile
peas-demo/plugins/pythonhello/Makefile
peas-demo/plugins/seedhello/Makefile
po/Makefile.in
])

AC_OUTPUT

echo "

Configuration:

        Source code location          : ${srcdir}
        Compiler                      : ${CC}
        Installation prefix           : ${prefix}
        Seed JS support               : ${found_seed}
        Python support                : ${found_python}
"
