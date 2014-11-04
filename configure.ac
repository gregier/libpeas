dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.63.2)

# Copied from glib
#
# Making releases:
#   glib_micro_version += 1;
#   glib_interface_age += 1;
#   glib_binary_age += 1;
# if any functions have been added, set glib_interface_age to 0.
# if backwards compatibility has been broken,
# set glib_binary_age _and_ glib_interface_age to 0.
#
# in easier to understand terms:
#
# <mclasen> on the stable branch, interface age == micro
# <mclasen> on the unstable (ie master), interface age = 0
m4_define(peas_major_version,  1)
m4_define(peas_minor_version,  12)
m4_define(peas_micro_version,  2)
m4_define(peas_interface_age,  0)

m4_define(peas_binary_age, [m4_eval(100 * peas_minor_version + peas_micro_version)])
m4_define(peas_version, [peas_major_version.peas_minor_version.peas_micro_version])

dnl libtool version related macros
m4_define(peas_lt_current, [m4_eval(100 * peas_minor_version + peas_micro_version - peas_interface_age)])
m4_define(peas_lt_revision, [peas_interface_age])
m4_define(peas_lt_age, [m4_eval(peas_binary_age - peas_interface_age)])

AC_INIT([libpeas], [peas_version], [http://bugzilla.gnome.org/enter_bug.cgi?product=libpeas],
        [libpeas],[https://wiki.gnome.org/Projects/Libpeas])

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

AM_INIT_AUTOMAKE([1.11 tar-ustar dist-xz no-dist-gzip -Wno-portability])
AM_MAINTAINER_MODE([enable])

AM_SILENT_RULES([yes])

dnl ================================================================
dnl Basic checks
dnl ================================================================

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

# Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT([disable-static])


AC_CHECK_FUNCS(fsync)

dnl ================================================================
dnl Gettext stuff.
dnl ================================================================

IT_PROG_INTLTOOL([0.40.0])
AS_IF([test "$USE_NLS" = "yes"],
      [AC_DEFINE([ENABLE_NLS], [1], [Defined if native language support should be enabled.])])

GETTEXT_PACKAGE=libpeas
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])

dnl ================================================================
dnl Minimum libpeas requirements (GObject and Introspection)
dnl ================================================================

PKG_PROG_PKG_CONFIG

GLIB_REQUIRED=2.30.0
GIO_REQUIRED=2.32.0
INTROSPECTION_REQUIRED=1.39.0

PKG_CHECK_MODULES(PEAS, [
	glib-2.0 >= $GLIB_REQUIRED
	gobject-2.0 >= $GLIB_REQUIRED
	gmodule-2.0 >= $GLIB_REQUIRED
	gio-2.0 >= $GIO_REQUIRED
	gobject-introspection-1.0 >= $INTROSPECTION_REQUIRED
])

GOBJECT_INTROSPECTION_REQUIRE($INTROSPECTION_REQUIRED)

AC_SUBST(GLIB_REQUIRED)
AC_SUBST(GIO_REQUIRED)
AC_SUBST(INTROSPECTION_REQUIRED)

AC_DEFINE(GLIB_VERSION_MIN_REQUIRED, GLIB_VERSION_2_32, [minimum glib version])
AC_DEFINE(GLIB_VERSION_MAX_ALLOWED, GLIB_VERSION_2_38, [maximum glib version])

dnl ================================================================
dnl Build libpeas-gtk
dnl ================================================================

GTK_REQUIRED=3.0.0

AC_ARG_ENABLE(gtk,
	      AS_HELP_STRING([--enable-gtk],[Build libpeas-gtk for Gtk widgetry]),
	      [enable_gtk=$enableval],
	      [enable_gtk=yes])

if test "$enable_gtk" = "yes"; then
	PKG_CHECK_MODULES(PEAS_GTK, [
		gtk+-3.0 >= $GTK_REQUIRED
	])
fi

AC_SUBST(GTK_REQUIRED)

AC_DEFINE(GDK_VERSION_MIN_REQUIRED, GDK_VERSION_3_0, [minimum gdk version])
AC_DEFINE(GDK_VERSION_MAX_ALLOWED, GDK_VERSION_3_0, [maximum gdk version])

AM_CONDITIONAL([ENABLE_GTK],[test "x$enable_gtk" = "xyes"])

dnl ================================================================
dnl Check for native OSX
dnl ================================================================
AC_MSG_CHECKING([for native Mac OS X])


GTK_CHECK_BACKEND([quartz], [3.0.0], [os_osx=yes], [os_osx=no])
AC_MSG_RESULT([$os_osx])
AM_CONDITIONAL(OS_OSX, test "$os_osx" = "yes")

if test "$os_osx" = "yes"; then
	AC_DEFINE([OS_OSX],[1],[Defined if os is Mac OSX])
fi

dnl ================================================================
dnl Check for GDB
dnl ================================================================

if test "x$GCC" = "xno"; then
  enable_gdb=no
else
  AC_PATH_PROG([enable_gdb], [gdb], [no])
fi

AM_CONDITIONAL([GDB_ENABLED], [test "x$enable_gdb" != "xno"])

dnl ================================================================
dnl Check for Valgrind
dnl ================================================================

AC_PATH_PROG([enable_valgrind], [valgrind], [no])

AM_CONDITIONAL([VALGRIND_ENABLED], [test "x$enable_valgrind" != "xno"])

dnl ================================================================
dnl Coverage testing
dnl ================================================================

AC_ARG_ENABLE([gcov],
	      [AS_HELP_STRING([--enable-gcov],
			      [Whether to enable coverage testing (requires gcc and gcov) [default=no]])],
	      [enable_gcov=$enable_val],
	      [enable_gcov=no])

if test "x$enable_gcov" = "xno"; then
        enable_gcov="no (disabled, use --enable-gcov to enable)"
        AC_MSG_CHECKING([whether to build with coverage testing])
else
        AC_PATH_PROG([enable_gcov], [gcov], [no])
        AC_MSG_CHECKING([whether to build with coverage testing])

        if test "x$enable_gcov" = "xno"; then
                AC_MSG_ERROR([You need gcov to build with coverage testing])
        elif test "x$GCC" = "xno"; then
                AC_MSG_ERROR([You need to use gcc to build with coverage testing])
        else
                enable_gcov=yes
                GCOV_CFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"
                GCOV_LDFLAGS="-lgcov"
        fi
fi
AC_MSG_RESULT([$enable_gcov])

AM_CONDITIONAL([GCOV_ENABLED], [test "x$enable_gcov" = "xyes"])
AC_SUBST([GCOV_CFLAGS])
AC_SUBST([GCOV_LDFLAGS])

dnl ================================================================
dnl Glade
dnl ================================================================

AC_ARG_ENABLE([glade-catalog],
	[AS_HELP_STRING([--enable-glade-catalog],
		[Install a glade catalog file [default=auto]])],
	[enable_glade_catalog=$enableval],
	[enable_glade_catalog=auto])

if test "x$enable_glade_catalog" = "xno"; then
  found_glade_catalog="no (disabled, use --enable-glade-catalog to enable)"
else
  AC_MSG_CHECKING([for Glade.])

  PKG_CHECK_EXISTS([gladeui-2.0],
			 [found_glade_catalog=yes],
			 [found_glade_catalog=no])
fi

if test "x$enable_glade_catalog" = "xyes" -a "x$found_glade_catalog" = "xno"; then
  AC_MSG_ERROR([You need to have gladeui-2.0 installed to build libpeas])
fi
AC_MSG_RESULT([$found_glade_catalog])

if test "x$found_glade_catalog" = "xyes"; then
  AC_MSG_CHECKING([for Glade catalog dir])
  GLADE_DATA_DIR=`$PKG_CONFIG --variable=datadir gladeui-2.0 2>&AS_MESSAGE_LOG_FD`
  GLADE_CATALOG_DIR=`$PKG_CONFIG --variable=catalogdir gladeui-2.0 2>&AS_MESSAGE_LOG_FD | sed -e "s#^$GLADE_DATA_DIR#$datadir#"`
  AC_MSG_RESULT([$GLADE_CATALOG_DIR])
  AC_SUBST(GLADE_CATALOG_DIR)
fi

AM_CONDITIONAL([ENABLE_GLADE_CATALOG],[test "x$found_glade_catalog" = "xyes"])

dnl ================================================================
dnl Lua
dnl ================================================================

LUA51_REQUIRED=5.1.0
LUAJIT_REQUIRED=2.0

LGI_MAJOR_VERSION=0
LGI_MINOR_VERSION=8
LGI_MICRO_VERSION=0
LGI_REQUIRED=$LGI_MAJOR_VERSION.$LGI_MINOR_VERSION.$LGI_MICRO_VERSION
AC_DEFINE_UNQUOTED(LGI_MAJOR_VERSION, [$LGI_MAJOR_VERSION], [LGI major version.])
AC_DEFINE_UNQUOTED(LGI_MINOR_VERSION, [$LGI_MINOR_VERSION], [LGI minor version.])
AC_DEFINE_UNQUOTED(LGI_MICRO_VERSION, [$LGI_MICRO_VERSION], [LGI micro version.])

dnl
dnl Test program for LGI version
dnl
m4_define([peas_lgi_version_test], [
#include <lauxlib.h>
#include <lualib.h>

#include "peas-plugin-loader-lua-utils.c"

int main(int argc, char **argv)
{
  lua_State *L;
  gboolean success;

  L = luaL_newstate ();
  luaL_openlibs (L);

  success = (peas_lua_utils_require (L, "lgi") &&
             peas_lua_utils_check_version (L,
                                           LGI_MAJOR_VERSION,
                                           LGI_MINOR_VERSION,
                                           LGI_MICRO_VERSION));

  lua_close (L);
  return success ? 0 : 1;
}])

AC_ARG_ENABLE(lua5.1,
              AS_HELP_STRING([--enable-lua5.1],[Enable Lua 5.1 support]),
              [enable_lua51=$enableval],
              [enable_lua51=auto])

AC_ARG_ENABLE(luajit,
              AS_HELP_STRING([--enable-luajit],[Enable LuaJIT for Lua 5.1 support]),
              [enable_luajit=$enableval],
              [enable_luajit=auto])

AC_MSG_CHECKING([for Lua 5.1 availability.])

if test "x$enable_lua51" = "xno"; then
    found_lua51="no (disabled, use --enable-lua5.1 to enable)"
    AC_MSG_RESULT([$found_lua51])
else
    if test "x$enable_luajit" != "xno"; then
        PKG_CHECK_EXISTS([luajit >= $LUAJIT_REQUIRED], [
            found_lua51=yes
            with_lua51=luajit
        ], [
            found_lua51=no
        ])

        if test "x$enable_luajit" = "xyes" -a "x$found_lua51" = "xno"; then
            AC_MSG_ERROR([You need to have LuaJIT >= $LUAJIT_REQUIRED
                          installed to build libpeas])
        fi
    fi

    if test "x$found_lua51" != "xyes"; then
        PKG_CHECK_EXISTS([lua5.1 >= $LUA51_REQUIRED], [
            found_lua51=yes
            with_lua51=lua5.1
        ], [
            found_lua51=no
        ])

        if test "x$enable_lua51" = "xyes" -a "x$found_lua51" = "xno"; then
            AC_MSG_ERROR([You need to have Lua 5.1 >= $LUA51_REQUIRED
                          installed to build libpeas])
        fi
    fi

    if test "x$found_lua51" != "xyes"; then
        AC_MSG_RESULT([$found_lua51])
    else
        AC_MSG_RESULT([$found_lua51 ($with_lua51)])

        LUA51_CFLAGS=`$PKG_CONFIG --cflags $with_lua51`
        LUA51_LIBS=`$PKG_CONFIG --libs $with_lua51`
        AC_SUBST(LUA51_CFLAGS)
        AC_SUBST(LUA51_LIBS)

        AC_MSG_CHECKING(for LGI availability in Lua 5.1.)

        peas_save_CFLAGS="$CFLAGS"
        peas_save_LIBS="$LIBS"
        CFLAGS="$PEAS_CFLAGS $LUA51_CFLAGS -Iloaders/lua5.1"
        LIBS="$PEAS_LIBS $LUA51_LIBS"
        AC_TRY_RUN(peas_lgi_version_test(0),
                   found_lua51=yes,
                   found_lua51=no,
                   found_lua51=yes)
        CFLAGS="$peas_save_CFLAGS"
        LIBS="$peas_save_LIBS"

        if test "x$enable_lua51" = "xyes" -a "x$found_lua51" = "xno"; then
            AC_MSG_ERROR([You need to have LGI >= $LGI_REQUIRED
                          installed to build libpeas])
        fi

        if test "x$found_lua51" = "xyes"; then
            AC_DEFINE(ENABLE_LUA51, 1, [Define to compile with Lua support])
        fi

        AC_MSG_RESULT([$found_lua51])
    fi
fi

AM_CONDITIONAL([ENABLE_LUA51], [test "x$found_lua51" = "xyes"])

dnl ================================================================
dnl Python
dnl ================================================================

PYTHON2_REQUIRED=2.5.2
PYTHON3_REQUIRED=3.2.0

dnl Used by the Python loader to check the PyGObject version at runtime
PYGOBJECT_MAJOR_VERSION=3
PYGOBJECT_MINOR_VERSION=0
PYGOBJECT_MICRO_VERSION=0
PYGOBJECT_REQUIRED=$PYGOBJECT_MAJOR_VERSION.$PYGOBJECT_MINOR_VERSION.$PYGOBJECT_MICRO_VERSION
AC_DEFINE_UNQUOTED(PYGOBJECT_MAJOR_VERSION,[$PYGOBJECT_MAJOR_VERSION], [PyGObject major version.])
AC_DEFINE_UNQUOTED(PYGOBJECT_MINOR_VERSION,[$PYGOBJECT_MINOR_VERSION], [PyGObject minor version.])
AC_DEFINE_UNQUOTED(PYGOBJECT_MICRO_VERSION,[$PYGOBJECT_MICRO_VERSION], [PyGObject micro version.])

AC_MSG_CHECKING([for Python 2 availability.])

AC_ARG_ENABLE(python2,
    AS_HELP_STRING([--enable-python2],[Enable Python 2 support]),
    [enable_python2=$enableval],
    [enable_python2=auto])

if test "x$enable_python2" = "xno"; then
    found_python2="no (disabled, use --enable-python2 to enable)"
else
    dnl NOTE: we do not use AM_PATH_PYTHON, since it does not allow
    dnl to call it twice for py2 and py3, so we just check the prog
    AC_PATH_PROGS(PYTHON2, [python2 python2.7 python2.6 python2.5])
    if test -z "${PYTHON2}"; then
        found_python2="no (python2 not found)"
    else
        AM_PYTHON_CHECK_VERSION($PYTHON2, $PYTHON2_REQUIRED, [
            AC_PATH_PROGS(PYTHON2_CONFIG, [python2-config python-config python2.7-config python2.6-config python2.5-config])
            if test -n "${PYTHON2_CONFIG}"; then
                PKG_CHECK_MODULES(PYGOBJECT, pygobject-3.0 >= $PYGOBJECT_REQUIRED, [
                    found_python2=yes
                    PYTHON2_CFLAGS=`${PYTHON2_CONFIG} --includes`
                    PYTHON2_LIBS=`${PYTHON2_CONFIG} --libs`
                    PYTHON2_LDFLAGS=`${PYTHON2_CONFIG} --ldflags`
                    PYTHON2_PYEXECDIR=`$PYTHON2 -c "from distutils import sysconfig; print(sysconfig.get_python_lib(1, 0, prefix='\\$' '{exec_prefix}'))"`
                    AC_SUBST(PYTHON2_CFLAGS)
                    AC_SUBST(PYTHON2_LIBS)
                    AC_SUBST(PYTHON2_LDFLAGS)
                    AC_SUBST(PYTHON2_PYEXECDIR)
                    AC_DEFINE(ENABLE_PYTHON2, 1, [Define to compile with Python 2 support])
                ], [
                    found_python2="no (PyGObject not found)"
                ])
            else
                found_python2="no (python2-config not found)"
            fi
        ], [
            found_python2="no (python2 version too old)"
        ])
    fi
fi

if test "x$enable_python2" = "xyes" -a "x$found_python2" != "xyes"; then
    AC_MSG_ERROR([You need to have Python 2 and PyGObject installed to build libpeas])
fi
AC_MSG_RESULT([$found_python2])

AM_CONDITIONAL([ENABLE_PYTHON2],[test "x$found_python2" = "xyes"])


AC_MSG_CHECKING([for Python 3 availability.])

AC_ARG_ENABLE(python3,
    AS_HELP_STRING([--enable-python3],[Enable Python 3 support]),
    [enable_python3=$enableval],
    [enable_python3=auto])

if test "x$enable_python3" = "xno"; then
    found_python3="no (disabled, use --enable-python3 to enable)"
else
    AM_PATH_PYTHON($PYTHON3_REQUIRED, [
        AC_PATH_TOOL(PYTHON3_CONFIG, "python3-config")
        if test -n "${PYTHON3_CONFIG}"; then
            PKG_CHECK_MODULES(PYGOBJECT, pygobject-3.0 >= $PYGOBJECT_REQUIRED, [
                found_python3=yes
                PYTHON3_CFLAGS=`${PYTHON3_CONFIG} --includes`
                PYTHON3_LIBS=`${PYTHON3_CONFIG} --libs`
                PYTHON3_LDFLAGS=`${PYTHON3_CONFIG} --ldflags`
                PYTHON3_PYEXECDIR="${pyexecdir}"
                AC_SUBST(PYTHON3_CFLAGS)
                AC_SUBST(PYTHON3_LIBS)
                AC_SUBST(PYTHON3_LDFLAGS)
                AC_SUBST(PYTHON3_PYEXECDIR)
                AC_DEFINE(ENABLE_PYTHON3, 1, [Define to compile with Python 3 support])
            ], [
                found_python3="no (PyGObject not found)"
            ])
        else
            found_python3="no (python3-config not found)"
        fi
    ], [
        found_python3="no (python3 not found)"
    ])
fi

if test "x$enable_python3" = "xyes" -a "x$found_python3" != "xyes"; then
    AC_MSG_ERROR([You need to have Python 3 and PyGObject installed to build libpeas])
fi
AC_MSG_RESULT([$found_python3])

AM_CONDITIONAL([ENABLE_PYTHON3],[test "x$found_python3" = "xyes"])

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

# The reasons for -rpath /nowhere can be found at:
# http://www.mail-archive.com/automake@gnu.org/msg10725.htm

PLUGIN_LIBTOOL_FLAGS="-module -avoid-version -no-undefined"
TEST_PLUGIN_LIBTOOL_FLAGS="$PLUGIN_LIBTOOL_FLAGS -rpath /nowhere"
TEST_INTROSPECTION_LIBTOOL_FLAGS="-rpath /nowhere -export-dynamic -no-undefined"
# Cannot use "-export-symbols-regex peas_register_types"
# because it might make the python plugin loader not work!
LOADER_LIBTOOL_FLAGS="-module -avoid-version -no-undefined"
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)
AC_SUBST(TEST_PLUGIN_LIBTOOL_FLAGS)
AC_SUBST(TEST_INTROSPECTION_LIBTOOL_FLAGS)
AC_SUBST(LOADER_LIBTOOL_FLAGS)

dnl ================================================================
dnl Misc
dnl ================================================================

AC_DEFINE(G_LOG_DOMAIN, "libpeas", [Log domain])

AC_PATH_PROG(GLIB_COMPILE_RESOURCES, glib-compile-resources)
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
libpeas-gtk/Makefile
loaders/Makefile
loaders/lua5.1/Makefile
loaders/python/Makefile
loaders/python3/Makefile
data/Makefile
data/glade/Makefile
data/icons/Makefile
data/libpeas-1.0.pc
data/libpeas-gtk-1.0.pc
peas-demo/Makefile
peas-demo/plugins/Makefile
peas-demo/plugins/helloworld/Makefile
peas-demo/plugins/luahello/Makefile
peas-demo/plugins/pythonhello/Makefile
peas-demo/plugins/secondtime/Makefile
po/Makefile.in
tests/Makefile
tests/libpeas/Makefile
tests/libpeas/plugins/Makefile
tests/libpeas/plugins/extension-c/Makefile
tests/libpeas/plugins/extension-lua/Makefile
tests/libpeas/plugins/extension-python/Makefile
tests/libpeas/introspection/Makefile
tests/libpeas/testing/Makefile
tests/libpeas-gtk/Makefile
tests/libpeas-gtk/plugins/Makefile
tests/libpeas-gtk/plugins/builtin-configurable/Makefile
tests/libpeas-gtk/plugins/configurable/Makefile
tests/libpeas-gtk/plugins/valid-custom-icon/Makefile
tests/libpeas-gtk/testing/Makefile
tests/plugins/Makefile
tests/plugins/builtin/Makefile
tests/plugins/has-dep/Makefile
tests/plugins/loadable/Makefile
tests/plugins/self-dep/Makefile
tests/testing-util/Makefile
])

AC_OUTPUT

echo "

Configuration:

        Source code location          : ${srcdir}
        Compiler                      : ${CC}
        Installation prefix           : ${prefix}
        Build libpeas-gtk             : ${enable_gtk}
        Coverage testing              : ${enable_gcov}
        Glade Catalog                 : ${found_glade_catalog}
        Disable deprecated features   : ${enable_deprecation}

Languages support:

        Lua 5.1 support               : ${found_lua51}
        Python 2 support              : ${found_python2}
        Python 3 support              : ${found_python3}
"
