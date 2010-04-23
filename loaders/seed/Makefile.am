# Seed plugin loader

loaderdir = $(libdir)/libpeas-2.0/loaders

INCLUDES = \
	-I$(top_srcdir)			\
	$(PEAS_CFLAGS)			\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED)		\
	$(SEED_CFLAGS)

loader_LTLIBRARIES = libseedloader.la

libseedloader_la_SOURCES = \
	peas-seed-plugin.c		\
	peas-seed-plugin.h		\
	peas-plugin-loader-seed.c 	\
	peas-plugin-loader-seed.h

libseedloader_la_LDFLAGS = $(LOADER_LIBTOOL_FLAGS)
libseedloader_la_LIBADD = $(PEAS_LIBS) $(SEED_LIBS)
