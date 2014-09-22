# Python plugin loader

loaderdir = $(libdir)/libpeas-1.0/loaders

AM_CPPFLAGS = \
	-I$(top_srcdir)			\
	$(PEAS_CFLAGS)			\
	$(GCOV_CFLAGS)			\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED)		\
	$(PYGOBJECT_CFLAGS)		\
	$(PYTHON2_CFLAGS)		\
	-DPEAS_PYEXECDIR=\""$(PYTHON2_PYEXECDIR)"\" \
	-DPEAS_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

loader_LTLIBRARIES = libpythonloader.la

libpythonloader_la_SOURCES = \
	peas-plugin-loader-python.c 		\
	peas-plugin-loader-python.h		\
	peas-plugin-loader-python-resources.c

libpythonloader_la_LDFLAGS = \
	$(LOADER_LIBTOOL_FLAGS)		\
	$(GCOV_LDFLAGS)			\
	$(PYTHON2_LDFLAGS)

libpythonloader_la_LIBADD = \
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	$(PEAS_LIBS)				\
	$(PYGOBJECT_LIBS)			\
	$(PYTHON2_LIBS)

loader_resources_deps = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/peas-plugin-loader-python.gresource.xml)
peas-plugin-loader-python-resources.c: $(srcdir)/peas-plugin-loader-python.gresource.xml $(loader_resources_deps)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --internal --target=$@ --sourcedir=$(srcdir) --generate-source $(srcdir)/peas-plugin-loader-python.gresource.xml

EXTRA_DIST = \
	peas-plugin-loader-python.gresource.xml		\
	$(loader_resources_deps)

CLEANFILES = peas-plugin-loader-python-resources.c

gcov_sources = $(libpythonloader_la_SOURCES)
include $(top_srcdir)/Makefile.gcov
