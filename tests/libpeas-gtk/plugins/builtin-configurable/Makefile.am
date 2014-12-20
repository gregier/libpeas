include $(top_srcdir)/tests/Makefile.plugin

AM_CPPFLAGS = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(PEAS_GTK_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

noinst_LTLIBRARIES = libbuiltin-configurable.la

libbuiltin_configurable_la_SOURCES = \
	builtin-configurable.c	\
	builtin-configurable.h

libbuiltin_configurable_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)
libbuiltin_configurable_la_LIBADD  = \
	$(top_builddir)/libpeas-gtk/libpeas-gtk-1.0.la	\
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	$(PEAS_LIBS) 				\
	$(PEAS_GTK_LIBS)

noinst_PLUGIN = builtin-configurable.plugin

EXTRA_DIST = $(noinst_PLUGIN)
