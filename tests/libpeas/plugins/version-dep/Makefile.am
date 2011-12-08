include $(top_srcdir)/tests/Makefile.plugin

INCLUDES = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

noinst_LTLIBRARIES = libversion-dep.la

libversion_dep_la_SOURCES = \
	version-dep-plugin.c	\
	version-dep-plugin.h

libversion_dep_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)
libversion_dep_la_LIBADD  = $(PEAS_LIBS)

noinst_PLUGIN = version-dep.plugin

EXTRA_DIST = $(noinst_PLUGIN)
