plugindir = "$(abs_top_srcdir)/.dummy-install/plugins"

INCLUDES = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

plugin_LTLIBRARIES = libbuiltin.la

libbuiltin_la_SOURCES = \
	builtin-plugin.c	\
	builtin-plugin.h

libbuiltin_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libbuiltin_la_LIBADD  = $(PEAS_LIBS)

plugin_DATA = builtin.plugin

EXTRA_DIST = $(plugin_DATA)
