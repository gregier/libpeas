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
	-DPEAS_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

loader_LTLIBRARIES = libpython3loader.la

libpython3loader_la_SOURCES = \
	$(top_srcdir)/loaders/python/peas-plugin-loader-python.c	\
	$(top_srcdir)/loaders/python/peas-plugin-loader-python.h	\
	$(top_srcdir)/loaders/python/peas-python-internal.c 		\
	$(top_srcdir)/loaders/python/peas-python-internal.h 		\
	peas-python3-resources.c

libpython3loader_la_LDFLAGS = \
	$(LOADER_LIBTOOL_FLAGS)		\
	$(GCOV_LDFLAGS)			\
	$(PYTHON3_LDFLAGS)

libpython3loader_la_LIBADD = \
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	$(PEAS_LIBS)				\
	$(PYGOBJECT_LIBS)			\
	$(PYTHON3_LIBS)

peas-python3-internal.pyc: $(srcdir)/../python/peas-python-internal.py
	$(AM_V_GEN) $(PYTHON3_BIN) $(srcdir)/../python/peas-python-compile.py $< $@

all-local: peas-python3-internal.pyc

loader_resources_deps = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir="$(srcdir)" --generate-dependencies "$(srcdir)/peas-python3.gresource.xml")
loader_resources_c_deps = $(srcdir)/peas-python3.gresource.xml $(loader_resources_deps)

peas-python3-resources.c: $(loader_resources_c_deps)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES)		\
	--target="$@"					\
	--sourcedir="$(srcdir)"				\
	--generate-source				\
	--internal					\
	"$(srcdir)/peas-python3.gresource.xml"

EXTRA_DIST = $(loader_resources_c_deps)

CLEANFILES = \
	peas-python3-internal.pyc	\
	peas-python3-resources.c

gcov_sources = $(libpython3loader_la_SOURCES)
include $(top_srcdir)/Makefile.gcov
