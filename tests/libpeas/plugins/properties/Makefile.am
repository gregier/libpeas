INCLUDES = \
	-I$(top_srcdir)		\
	-I$(srcdir)/../../introspection	\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

noinst_LTLIBRARIES = libproperties.la

libproperties_la_SOURCES = \
	properties-plugin.c	\
	properties-plugin.h

libproperties_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)
libproperties_la_LIBADD  = $(PEAS_LIBS)

noinst_DATA = properties.plugin

EXTRA_DIST = $(noinst_DATA)
