include $(top_srcdir)/tests/Makefile.plugin

AM_CPPFLAGS = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(PEAS_GTK_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

noinst_LTLIBRARIES = libconfigurable.la

libconfigurable_la_SOURCES = \
	configurable-plugin.c	\
	configurable-plugin.h

libconfigurable_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)
libconfigurable_la_LIBADD  = \
	$(top_builddir)/libpeas-gtk/libpeas-gtk-1.0.la	\
	$(top_builddir)/libpeas/libpeas-1.0.la		\
	$(PEAS_LIBS) 					\
	$(PEAS_GTK_LIBS)

noinst_PLUGIN = configurable.plugin

EXTRA_DIST = $(noinst_PLUGIN)
