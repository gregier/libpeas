dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)

m4_define(gpe_major_version,  2)
m4_define(gpe_minor_version, 28)
m4_define(gpe_micro_version,  0)
m4_define(gpe_interface_age,  0)

m4_define(gpe_binary_age, [m4_eval(100 * gpe_minor_version + gpe_micro_version)])
m4_define(gpe_version, [gpe_major_version.gpe_minor_version.gpe_micro_version])

# libtool version related macros
m4_define(gpe_lt_current, [m4_eval(100 * gpe_minor_version + gpe_micro_version - gpe_interface_age)])
m4_define(gpe_lt_revision, [gpe_interface_age])
m4_define(gpe_lt_age, [m4_eval(gpe_binary_age - gpe_interface_age)])


AC_INIT(libgpe, gpe_version, http://bugzilla.gnome.org/enter_bug.cgi?product=gedit)
AC_CONFIG_SRCDIR(libgpe/gpe-plugin.c)
AC_CONFIG_MACRO_DIR([m4])

AC_DEFINE(GPE_MAJOR_VERSION, gpe_major_version, [Gedit Plugins Engine major version])
AC_SUBST(GPE_MAJOR_VERSION, gpe_major_version)
AC_DEFINE(GPE_MINOR_VERSION, gpe_minor_version, [Gedit Plugins Engine minor version])
AC_SUBST(GPE_MINOR_VERSION, gpe_minor_version)
AC_DEFINE(GPE_MICRO_VERSION, gpe_micro_version, [Gedit Plugins Engine micro version])
AC_SUBST(GPE_MICRO_VERSION, gpe_micro_version)

GPE_API_VERSION=2.20
AC_SUBST(GPE_API_VERSION)

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

dnl make sure we keep ACLOCAL_FLAGS around for maintainer builds to work
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")

dnl ================================================================
dnl Gettext stuff.
dnl ================================================================

GETTEXT_PACKAGE=gpe
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])

AM_GLIB_GNU_GETTEXT

dnl ================================================================
dnl Start of pkg-config checks
dnl ================================================================

PKG_CHECK_MODULES(GPE, [
	glib-2.0 >= 2.18.0
	gthread-2.0 >= 2.16.0
	gio-2.0 >= 2.16.0
	gtk+-2.0 >= 2.16.0
])

dnl ================================================================
dnl Libtool
dnl ================================================================

LT_CURRENT=gpe_lt_current
LT_REVISION=gpe_lt_revision
LT_AGE=gpe_lt_age
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
LOADER_LIBTOOL_FLAGS="-module -avoid-version"
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)
AC_SUBST(LOADER_LIBTOOL_FLAGS)

AC_CONFIG_FILES([
Makefile
libgpe/Makefile
libgpeui/Makefile
loaders/Makefile
loaders/c/Makefile
gpe-demo/Makefile
gpe-demo/plugins/Makefile
gpe-demo/plugins/helloworld/Makefile
po/Makefile.in
])

AC_OUTPUT

echo "

Configuration:

	Source code location:	${srcdir}
	Compiler:		${CC}
"
