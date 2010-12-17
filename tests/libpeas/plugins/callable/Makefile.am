INCLUDES = \
	-I$(top_srcdir)		\
	-I$(srcdir)/../../introspection	\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

noinst_LTLIBRARIES = libcallable.la

libcallable_la_SOURCES = \
	callable-plugin.c	\
	callable-plugin.h

libcallable_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)
libcallable_la_LIBADD  = $(PEAS_LIBS)

noinst_DATA = callable.plugin

EXTRA_DIST = $(noinst_DATA)
