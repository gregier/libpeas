# time plugin
plugindir = $(prefix)/lib/gpe-demo/plugins

INCLUDES = \
	-I$(top_srcdir) 	\
	$(GPE_CFLAGS)

plugin_LTLIBRARIES = libhelloworld.la

libhelloworld_la_SOURCES = \
	gpedemo-hello-world-plugin.h	\
	gpedemo-hello-world-plugin.c

libhelloworld_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libhelloworld_la_LIBADD  = $(GPE_LIBS)

plugin_DATA = helloworld.gpedemo-plugin
