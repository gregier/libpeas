# Python 3 plugin loader

loaderdir = $(libdir)/libpeas-1.0/loaders

INCLUDES = \
	-I$(top_srcdir)			\
	$(PEAS_CFLAGS)			\
	$(GCOV_CFLAGS)			\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED)		\
	$(PYGOBJECT_CFLAGS)		\
	$(PYTHON3_CFLAGS)		\
	-DPEAS_PYEXECDIR=\""$(PYTHON3_PYEXECDIR)"\" \
	-DPEAS_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

loader_LTLIBRARIES = libpython3loader.la

libpython3loader_la_SOURCES = \
	$(top_srcdir)/loaders/python/peas-plugin-loader-python.c	\
	$(top_srcdir)/loaders/python/peas-plugin-loader-python.h

libpython3loader_la_LDFLAGS = \
	$(LOADER_LIBTOOL_FLAGS)		\
	$(GCOV_LDFLAGS)			\
	$(PYTHON3_LDFLAGS)

libpython3loader_la_LIBADD = \
	$(PEAS_LIBS)			\
	$(PYGOBJECT_LIBS)		\
	$(PYTHON3_LIBS)

gcov_sources = $(libpython3loader_la_SOURCES)
include $(top_srcdir)/Makefile.gcov
