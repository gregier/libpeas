plugindir = "$(abs_top_srcdir)/.dummy-install/plugins"

INCLUDES = \
	-I$(top_srcdir)		\
	-I../../introspection	\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

plugin_LTLIBRARIES = libcallable.la

libcallable_la_SOURCES = \
	callable-plugin.c	\
	callable-plugin.h

libcallable_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libcallable_la_LIBADD  = $(PEAS_LIBS)

plugin_DATA = callable.plugin

EXTRA_DIST = $(plugin_DATA)
