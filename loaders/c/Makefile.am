# C plugin loader

loaderdir = $(libdir)/libpeas-2.0/loaders

INCLUDES = \
	-I$(top_srcdir)			\
	$(PEAS_CFLAGS)			\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED)

loader_LTLIBRARIES = libcloader.la

libcloader_la_SOURCES = \
	peas-extension-c.c	\
	peas-extension-c.h	\
	peas-plugin-loader-c.c 	\
	peas-plugin-loader-c.h

libcloader_la_LDFLAGS = $(LOADER_LIBTOOL_FLAGS)
libcloader_la_LIBADD = $(PEAS_LIBS)
