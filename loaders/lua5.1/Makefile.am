# Lua 5.1 plugin loader

SUBDIRS = resources

loaderdir = $(libdir)/libpeas-1.0/loaders

AM_CPPFLAGS = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(GCOV_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)	\
	$(LUA51_CFLAGS)

loader_LTLIBRARIES = liblua51loader.la

liblua51loader_la_SOURCES = \
	peas-lua-internal.c		\
	peas-lua-internal.h		\
	peas-plugin-loader-lua.c	\
	peas-plugin-loader-lua.h	\
	peas-lua-utils.c		\
	peas-lua-utils.h                \
	resources/peas-lua-resources.c

liblua51loader_la_LDFLAGS = \
	$(LOADER_LIBTOOL_FLAGS)		\
	$(GCOV_LDFLAGS)

liblua51loader_la_LIBADD = \
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	$(PEAS_LIBS)				\
	$(LUA51_LIBS)

gcov_sources = $(liblua51loader_la_SOURCES)
include $(top_srcdir)/Makefile.gcov
