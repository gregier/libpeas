# C plugin loader

loaderdir = $(libdir)/libgpe-2.0/loaders

INCLUDES = \
	-I$(top_srcdir)			\
	$(GPE_CFLAGS)

loader_LTLIBRARIES = libcloader.la

libcloader_la_SOURCES = \
	gpe-plugin-loader-c.c 	\
	gpe-plugin-loader-c.h

libcloader_la_LDFLAGS = $(LOADER_LIBTOOL_FLAGS)
libcloader_la_LIBADD = $(GPE_LIBS)
