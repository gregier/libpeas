include $(top_srcdir)/tests/Makefile.plugin

AM_CPPFLAGS = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

noinst_LTLIBRARIES = libself-dep.la

libself_dep_la_SOURCES = \
	self-dep-plugin.c	\
	self-dep-plugin.h

libself_dep_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)
libself_dep_la_LIBADD  = \
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	$(PEAS_LIBS)

noinst_PLUGIN = self-dep.plugin

EXTRA_DIST = $(noinst_PLUGIN)
