dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)

m4_define(peas_major_version,  2)
m4_define(peas_minor_version, 28)
m4_define(peas_micro_version,  0)
m4_define(peas_interface_age,  0)

m4_define(peas_binary_age, [m4_eval(100 * peas_minor_version + peas_micro_version)])
m4_define(peas_version, [peas_major_version.peas_minor_version.peas_micro_version])

dnl libtool version related macros
m4_define(peas_lt_current, [m4_eval(100 * peas_minor_version + peas_micro_version - peas_interface_age)])
m4_define(peas_lt_revision, [peas_interface_age])
m4_define(peas_lt_age, [m4_eval(peas_binary_age - peas_interface_age)])

AC_INIT(libpeas, peas_version, http://bugzilla.gnome.org/enter_bug.cgi?product=gedit)
AC_CONFIG_SRCDIR(libpeas/peas-plugin.c)
AC_CONFIG_MACRO_DIR([m4])

AC_DEFINE(PEAS_MAJOR_VERSION, peas_major_version, [Gedit Plugins Engine major version])
AC_SUBST(PEAS_MAJOR_VERSION, peas_major_version)
AC_DEFINE(PEAS_MINOR_VERSION, peas_minor_version, [Gedit Plugins Engine minor version])
AC_SUBST(PEAS_MINOR_VERSION, peas_minor_version)
AC_DEFINE(PEAS_MICRO_VERSION, peas_micro_version, [Gedit Plugins Engine micro version])
AC_SUBST(PEAS_MICRO_VERSION, peas_micro_version)

AC_SUBST(PEAS_VERSION, peas_version)

PEAS_API_VERSION=2.20
AC_SUBST(PEAS_API_VERSION)

AM_INIT_AUTOMAKE([1.8 dist-bzip2 no-dist-gzip -Wno-portability])

AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_CONFIG_HEADERS(config.h)
AC_DISABLE_STATIC

IT_PROG_INTLTOOL([0.40.0])

PKG_PROG_PKG_CONFIG

AC_PROG_LIBTOOL

GNOME_DOC_INIT
GTK_DOC_CHECK([1.0])

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_SYS_LARGEFILE

AC_CHECK_FUNCS(fsync)
AC_CHECK_FUNC(sigaction)

dnl make sure we keep ACLOCAL_FLAGS around for maintainer builds to work
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")

dnl ================================================================
dnl Gettext stuff.
dnl ================================================================

GETTEXT_PACKAGE=libpeas
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])

AM_GLIB_GNU_GETTEXT

dnl ================================================================
dnl Start of pkg-config checks
dnl ================================================================

PKG_CHECK_MODULES(PEAS, [
	glib-2.0 >= 2.18.0
	gthread-2.0 >= 2.16.0
	gio-2.0 >= 2.16.0
	gtk+-2.0 >= 2.16.0
])

dnl ================================================================
dnl GObject Introspection
dnl ================================================================

GOBJECT_INTROSPECTION_REQUIRED=0.6.5

AC_ARG_ENABLE(introspection,
	      AS_HELP_STRING([--enable-introspection[=@<:@no/auto/yes@:>@]], [Enable GObject introspection]),
	      [enable_introspection=$enableval],
	      [enable_introspection=auto])

AC_ARG_WITH(girdir,
	    AS_HELP_STRING([--with-girdir=DIR],[Installation path for .gir files @<:@auto@:>@]),
            [ac_with_girdir=$withval],
            [ac_with_girdir=""])

AC_ARG_WITH(typelibdir,
            AS_HELP_STRING([--with-typelibdir=DIR],[Installation path for .typelib files @<:@auto@:>@]),
            [ac_with_typelibdir=$withval],
            [ac_with_typelibdir=""])

AC_MSG_CHECKING([for GObject introspection availabilty.])

if test "x$enable_introspection" = "xno"; then
	found_introspection="no (disabled, use --enable-introspection to enable)"
elif test "x$enable_introspection" = "xyes"; then
	PKG_CHECK_EXISTS([gobject-introspection-1.0],,
			 AC_MSG_ERROR([gobject-introspection-1.0 is not installed]))
	PKG_CHECK_EXISTS([gobject-introspection-1.0 >= $GOBJECT_INTROSPECTION_REQUIRED],
			 [found_introspection=yes],
			 AC_MSG_ERROR([You need to have gobject-introspection >= $1 installed to build libpeas]))
else
	PKG_CHECK_EXISTS([gobject-introspection-1.0 >= $GOBJECT_INTROSPECTION_REQUIRED],
			 [found_introspection=yes],
			 [found_introspection=no])
fi

AC_MSG_RESULT([$found_introspection])

if test "x$found_introspection" = "xyes"; then
	G_IR_SCANNER=`$PKG_CONFIG --variable=g_ir_scanner gobject-introspection-1.0`
	AC_SUBST(G_IR_SCANNER)
	G_IR_COMPILER=`$PKG_CONFIG --variable=g_ir_compiler gobject-introspection-1.0`
	AC_SUBST(G_IR_COMPILER)
	G_IR_GENERATE=`$PKG_CONFIG --variable=g_ir_generate gobject-introspection-1.0`
	AC_SUBST(G_IR_GENERATE)

	test "${ac_with_girdir}" = "" && ac_with_girdir=`$PKG_CONFIG --variable=girdir gobject-introspection-1.0`
	girdir=$ac_with_girdir
	AC_MSG_NOTICE([installing .gir files in $girdir])
	AC_SUBST(girdir)

	test "${ac_with_typelibdir}" = "" && ac_with_typelibdir="$($PKG_CONFIG --variable=typelibdir gobject-introspection-1.0)"
	typelibdir=$ac_with_typelibdir
	AC_MSG_NOTICE([installing .typelib files in $typelibdir])
	AC_SUBST(typelibdir)

	AC_DEFINE(ENABLE_INTROSPECTION,1,[Define to compile with GObject Introspection support])
fi

AM_CONDITIONAL([ENABLE_INTROSPECTION],[test "x$found_introspection" = "xyes"])

dnl ================================================================
dnl Seed Javascript Engine
dnl ================================================================

SEED_REQUIRED=2.28.0

AC_ARG_ENABLE(seed,
	      AC_HELP_STRING([--enable-seed],[Enable Seed support]),
	      [enable_seed=$enableval],
	      [enable_seed=auto])

AC_MSG_CHECKING([for Seed JS availability.])

if test "x$enable_seed" = "xno"; then
	found_seed="no (disabled, use --enable-seed to enable)"
elif test "x$found_introspection" != "xyes"; then
	found_seed="no (disabled, enable introspection first)"
elif test "x$enable_seed" = "xyes"; then
	if test "$found_introspection" != "yes"; then
		AC_MSG_ERROR([GObject introspection support must be enabled for Seed support])
	fi

	PKG_CHECK_EXISTS([seed],,
			 AC_MSG_ERROR([seed is not installed]))
	PKG_CHECK_EXISTS([seed >= $SEED_REQUIRED],
			 [found_seed=yes],
			 AC_MSG_ERROR([You need to have seed >= $1 installed to build libpeas]))
else
	PKG_CHECK_EXISTS([seed >= $SEED_REQUIRED],
			 [found_seed=yes],
			 [found_seed=no])
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

AC_MSG_CHECKING([for Python support.])
AC_ARG_ENABLE(python,
	      AS_HELP_STRING([--enable-python], [Enable python support]),
	      [enable_python=$enableval have_python=$enableval],
	      [enable_python=autodetect have_python=yes])
AC_MSG_RESULT([$enable_python])

if test "x$have_python" != "xno"; then
	AM_PATH_PYTHON([2.3],[],[no])
	if test "x$PYTHON" = "x:"; then
		have_python=no
	fi
fi

#if test "x$have_python" != "xno"; then
#	AM_CHECK_PYTHON_HEADERS([],[have_python=no])
#fi

if test "x$have_python" != "xno"; then
	PY_PREFIX=`$PYTHON -c 'import sys ; print sys.prefix'`
	PY_EXEC_PREFIX=`$PYTHON -c 'import sys ; print sys.exec_prefix'`
	PYTHON_LIBS="-lpython$PYTHON_VERSION"
	PYTHON_LIB_LOC="-L$PY_EXEC_PREFIX/lib/python$PYTHON_VERSION/config"
	PYTHON_CFLAGS="-I$PY_PREFIX/include/python$PYTHON_VERSION"
	PYTHON_MAKEFILE="$PY_EXEC_PREFIX/lib/python$PYTHON_VERSION/config/Makefile"
	PYTHON_LOCALMODLIBS=`sed -n -e 's/^LOCALMODLIBS=\(.*\)/\1/p' $PYTHON_MAKEFILE`
	PYTHON_BASEMODLIBS=`sed -n -e 's/^BASEMODLIBS=\(.*\)/\1/p' $PYTHON_MAKEFILE`
	PYTHON_OTHER_LIBS=`sed -n -e 's/^LIBS=\(.*\)/\1/p' $PYTHON_MAKEFILE`
	PYTHON_EXTRA_LIBS="$PYTHON_LOCALMODLIBS $PYTHON_BASEMODLIBS $PYTHON_OTHER_LIBS"
	AC_SUBST([PYTHON_LIBS])
	AC_SUBST([PYTHON_LIB_LOC])
	AC_SUBST([PYTHON_CFLAGS])
	AC_SUBST([PYTHON_EXTRA_LIBS])
fi

if test "x$have_python" != "xyes"; then
	if test "x$enable_python" = "xyes"; then
		AC_MSG_ERROR([Python not found])
	elif test "x$enable_python" = "xautodetect"; then
		enable_python=no
		AC_MSG_WARN([Python not found, disabling python support])
	fi
fi

if test "x$have_python" != "xno"; then
	PKG_CHECK_MODULES([PYGTK], [
		pygobject-2.0 >= 2.16
		pygtk-2.0 >= 2.14],
		[],
		[
		have_python=no
		if test "x$enable_python" = "xyes"; then
			AC_MSG_ERROR([$PYGTK_PKG_ERRORS])
		elif test "x$enable_python" = "xautodetect"; then
			enable_python=no
			AC_MSG_WARN([$PYGTK_PKG_ERRORS])
			AC_MSG_WARN([Disabling python support])
		fi
		])
fi

if test "x$have_python" != "xno"; then
	AC_MSG_CHECKING([for pygtk defs])
	PYGTK_DEFSDIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
	AC_MSG_RESULT([$PYGTK_DEFSDIR])

	AC_MSG_CHECKING([for pygtk codegen])
	PYGTK_CODEGEN="$PYTHON `$PKG_CONFIG --variable=codegendir pygtk-2.0`/codegen.py"
	AC_MSG_RESULT([$PYGTK_CODEGEN])

	AC_MSG_CHECKING([for pygtk h2def])
	PYGTK_H2DEF="$PYTHON `$PKG_CONFIG --variable=codegendir pygtk-2.0`/h2def.py"
	AC_MSG_RESULT([$PYGTK_H2DEF])

	AC_SUBST([PYGTK_DEFSDIR])
	AC_SUBST([PYGTK_CODEGEN])
	AC_SUBST([PYGTK_H2DEF])

	dnl Check for -fno-strict-aliasing
	FLAGS="-fno-strict-aliasing"
	save_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS $FLAGS"
	AC_MSG_CHECKING([whether [$]CC understands $FLAGS])
	AC_TRY_COMPILE([], [], [compiler_has_option=yes], [compiler_has_option=no])
	CFLAGS="$save_CFLAGS"
	AC_MSG_RESULT($compiler_has_option)
	if test $compiler_has_option = yes; then
		NO_STRICT_ALIASING_CFLAGS="$FLAGS"
	fi
	AC_SUBST([NO_STRICT_ALIASING_CFLAGS])
fi

if test "x$have_python" != "xno" -a "x$enable_python" != "xno"; then
	enable_python=yes
	AC_DEFINE([ENABLE_PYTHON],[1],[Define to compile with python support])
fi

AM_CONDITIONAL([ENABLE_PYTHON],[test "x$enable_python" = "xyes"])




dnl ================================================================
dnl Libtool
dnl ================================================================

LT_CURRENT=peas_lt_current
LT_REVISION=peas_lt_revision
LT_AGE=peas_lt_age
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

dnl ================================================================
dnl Misc
dnl ================================================================
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)
AC_PATH_PROG(GLIB_MKENUMS, glib-mkenums)

GNOME_COMPILE_WARNINGS(yes)

AC_ARG_ENABLE(deprecations,
              [AS_HELP_STRING([--enable-deprecations],
                              [warn about deprecated usages [default=no]])],,
              [enable_deprecations=no])

if test "x$enable_deprecations" = "xyes"; then
   DISABLE_DEPRECATED_CFLAGS="-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED \
                              -DGDK_PIXBUF_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED"
   AC_SUBST(DISABLE_DEPRECATED_CFLAGS)
fi

PLUGIN_LIBTOOL_FLAGS="-module -avoid-version"
LOADER_LIBTOOL_FLAGS="-module -avoid-version -export-symbols-regex register_peas_plugin_loader"
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)
AC_SUBST(LOADER_LIBTOOL_FLAGS)

AC_CONFIG_FILES([
Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/libpeas/Makefile
docs/reference/libpeas/version.xml
docs/reference/libpeasui/Makefile
libpeas/Makefile
libpeasui/Makefile
loaders/Makefile
loaders/c/Makefile
loaders/python/Makefile
loaders/python/bindings/Makefile
loaders/seed/Makefile
data/Makefile
data/libpeas-2.0.pc
data/libpeasui-2.0.pc
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
        GObject Introspection support : ${found_introspection}
        Seed JS support               : ${found_seed}
        Python support                : ${enable_python}
"
