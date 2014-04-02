include $(top_srcdir)/tests/Makefile.plugin

AM_CPPFLAGS = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

noinst_LTLIBRARIES = libbuiltin.la

libbuiltin_la_SOURCES = \
	builtin-plugin.c	\
	builtin-plugin.h

libbuiltin_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)
libbuiltin_la_LIBADD  = \
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	$(PEAS_LIBS)

noinst_PLUGIN = builtin.plugin

EXTRA_DIST = $(noinst_PLUGIN)
