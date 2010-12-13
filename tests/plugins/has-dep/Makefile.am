plugindir = "$(abs_top_srcdir)/.dummy-install/plugins"

INCLUDES = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

plugin_LTLIBRARIES = libhas-dep.la

libhas_dep_la_SOURCES = \
	has-dep-plugin.c	\
	has-dep-plugin.h

libhas_dep_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libhas_dep_la_LIBADD  = $(PEAS_LIBS)

plugin_DATA = has-dep.plugin

EXTRA_DIST = $(plugin_DATA)
