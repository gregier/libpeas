# Python 3 plugin loader

loaderdir = $(libdir)/libpeas-1.0/loaders

AM_CPPFLAGS = \
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
	$(top_srcdir)/loaders/python/peas-plugin-loader-python.h	\
	peas-plugin-loader-python3-resources.c

libpython3loader_la_LDFLAGS = \
	$(LOADER_LIBTOOL_FLAGS)		\
	$(GCOV_LDFLAGS)			\
	$(PYTHON3_LDFLAGS)

libpython3loader_la_LIBADD = \
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	$(PEAS_LIBS)				\
	$(PYGOBJECT_LIBS)			\
	$(PYTHON3_LIBS)

loader_resources_deps = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/peas-plugin-loader-python3.gresource.xml)
peas-plugin-loader-python3-resources.c: $(srcdir)/peas-plugin-loader-python3.gresource.xml $(loader_resources_deps)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --internal --target=$@ --sourcedir=$(srcdir) --generate-source $(srcdir)/peas-plugin-loader-python3.gresource.xml

EXTRA_DIST = \
	peas-plugin-loader-python3.gresource.xml		\
	$(loader_resources_deps)

CLEANFILES = peas-plugin-loader-python3-resources.c

gcov_sources = $(libpython3loader_la_SOURCES)
include $(top_srcdir)/Makefile.gcov
