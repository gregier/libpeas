plugindir = "$(abs_top_srcdir)/.dummy-install/plugins"

INCLUDES = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(PEAS_GTK_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

plugin_LTLIBRARIES = libconfigurable.la

libconfigurable_la_SOURCES = \
	configurable-plugin.c	\
	configurable-plugin.h

libconfigurable_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libconfigurable_la_LIBADD  = $(PEAS_LIBS) $(PEAS_GTK_LIBS)

plugin_DATA = configurable.plugin

EXTRA_DIST = $(plugin_DATA)
