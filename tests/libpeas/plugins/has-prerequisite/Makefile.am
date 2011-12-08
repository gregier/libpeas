include $(top_srcdir)/tests/Makefile.plugin

INCLUDES = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

noinst_LTLIBRARIES = libhas-prerequisite.la

libhas_prerequisite_la_SOURCES = \
	has-prerequisite-plugin.c	\
	has-prerequisite-plugin.h

libhas_prerequisite_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)
libhas_prerequisite_la_LIBADD  = $(PEAS_LIBS)

noinst_PLUGIN = has-prerequisite.plugin

EXTRA_DIST = $(noinst_PLUGIN)
