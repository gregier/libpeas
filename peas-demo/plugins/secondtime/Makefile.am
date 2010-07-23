plugindir = $(libdir)/peas-demo/plugins/secondtime

INCLUDES = \
	-I$(top_srcdir) 	\
	$(PEAS_CFLAGS)		\
	$(PEASUI_CFLAGS)

plugin_LTLIBRARIES = libsecondtime.la

libsecondtime_la_SOURCES = \
	second-time.h		\
	second-time.c

libsecondtime_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libsecondtime_la_LIBADD  = $(PEAS_LIBS) $(PEASUI_LIBS)

plugin_DATA = secondtime.peasdemo-plugin

EXTRA_DIST = $(plugin_DATA)
