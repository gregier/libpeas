plugindir = "$(abs_top_srcdir)/.dummy-install/plugins"

INCLUDES = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(PEAS_GTK_CFLAGS)	\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

plugin_LTLIBRARIES = libbuiltin-configurable.la

libbuiltin_configurable_la_SOURCES = \
	builtin-plugin.c	\
	builtin-plugin.h	\
	builtin-configurable.c	\
	builtin-configurable.h

libbuiltin_configurable_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libbuiltin_configurable_la_LIBADD  = $(PEAS_LIBS) $(PEAS_GTK_LIBS)

plugin_DATA = builtin-configurable.plugin

EXTRA_DIST = $(plugin_DATA)
