# C plugin loader

SUBDIRS=bindings

loaderdir = $(libdir)/libpeas-2.0/loaders

INCLUDES = \
	-I$(top_srcdir)			\
	$(PEAS_CFLAGS)			\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED)		\
	$(PYGTK_CFLAGS)			\
	$(PYTHON_CFLAGS)		\
	-DPEAS_PYEXECDIR=\""$(pyexecdir)"\" \
	-DPEAS_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

loader_LTLIBRARIES = libpythonloader.la

libpythonloader_la_SOURCES = \
	peas-plugin-loader-python.c 	\
	peas-plugin-loader-python.h

libpythonloader_la_LDFLAGS = \
	$(LOADER_LIBTOOL_FLAGS)

libpythonloader_la_LIBADD = \
	$(PEAS_LIBS)			\
	$(PYTHON_LIB_LOC)		\
	$(PYTHON_LIBS)			\
	$(PYTHON_EXTRA_LIBS)		\
	$(PYGTK_LIBS)

