# C plugin loader

SUBDIRS=bindings

loaderdir = $(libdir)/libgpe-2.0/loaders

INCLUDES = \
	-I$(top_srcdir)			\
	$(GPE_CFLAGS)			\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED_CFLAGS)	\
	$(PYGTK_CFLAGS)			\
	$(PYTHON_CFLAGS)		\
	-DGPE_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

loader_LTLIBRARIES = libpythonloader.la

libpythonloader_la_SOURCES = \
	gpe-plugin-loader-python.c 	\
	gpe-plugin-loader-python.h

libpythonloader_la_LDFLAGS = \
	$(LOADER_LIBTOOL_FLAGS)

libpythonloader_la_LIBADD = \
	$(GPE_LIBS)			\
	$(PYTHON_LIB_LOC)		\
	$(PYTHON_LIBS)			\
	$(PYTHON_EXTRA_LIBS)		\
	$(PYGTK_LIBS)

