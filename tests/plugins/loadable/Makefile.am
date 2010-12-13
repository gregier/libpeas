plugindir = "$(abs_top_srcdir)/.dummy-install/plugins"

INCLUDES = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

plugin_LTLIBRARIES = libloadable.la

libloadable_la_SOURCES = \
	loadable-plugin.c	\
	loadable-plugin.h

libloadable_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libloadable_la_LIBADD  = $(PEAS_LIBS)

plugin_DATA = loadable.plugin

EXTRA_DIST = $(plugin_DATA)
