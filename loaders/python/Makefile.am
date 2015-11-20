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
	-DPEAS_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

loader_LTLIBRARIES = libpythonloader.la

libpythonloader_la_SOURCES = \
	peas-plugin-loader-python.c 	\
	peas-plugin-loader-python.h	\
	peas-python-internal.c 		\
	peas-python-internal.h 		\
	peas-python-resources.c

libpythonloader_la_LDFLAGS = \
	$(LOADER_LIBTOOL_FLAGS)		\
	$(GCOV_LDFLAGS)			\
	$(PYTHON2_LDFLAGS)

libpythonloader_la_LIBADD = \
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	$(PEAS_LIBS)				\
	$(PYGOBJECT_LIBS)			\
	$(PYTHON2_LIBS)

peas-python-internal.pyc: peas-python-internal.py
	$(AM_V_GEN) $(PYTHON2_BIN) $(srcdir)/peas-python-compile.py $< $@

all-local: peas-python-internal.pyc

loader_resources_deps = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/peas-python.gresource.xml)
loader_resources_c_deps = $(srcdir)/peas-python.gresource.xml $(loader_resources_deps)

peas-python-resources.c: $(loader_resources_c_deps)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES)		\
	--target="$@"					\
	--sourcedir="$(srcdir)"				\
	--generate-source				\
	--internal					\
	"$(srcdir)/peas-python.gresource.xml"

EXTRA_DIST = \
	peas-python-compile.py		\
	$(loader_resources_c_deps)

CLEANFILES = \
	peas-python-internal.pyc	\
	peas-python-resources.c

gcov_sources = $(libpythonloader_la_SOURCES)
include $(top_srcdir)/Makefile.gcov
