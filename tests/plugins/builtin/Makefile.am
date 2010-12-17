INCLUDES = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

noinst_LTLIBRARIES = libbuiltin.la

libbuiltin_la_SOURCES = \
	builtin-plugin.c	\
	builtin-plugin.h

libbuiltin_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)
libbuiltin_la_LIBADD  = $(PEAS_LIBS)

noinst_DATA = builtin.plugin

EXTRA_DIST = $(noinst_DATA)
