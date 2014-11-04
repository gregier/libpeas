# Lua 5.1 plugin loader

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
	peas-plugin-loader-lua.c	\
	peas-plugin-loader-lua.h	\
	peas-plugin-loader-lua-utils.c	\
	peas-plugin-loader-lua-utils.h

liblua51loader_la_LDFLAGS = \
	$(LOADER_LIBTOOL_FLAGS)		\
	$(GCOV_LDFLAGS)

liblua51loader_la_LIBADD = \
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	$(PEAS_LIBS)				\
	$(LUA51_LIBS)

gcov_sources = $(liblua51loader_la_SOURCES)
include $(top_srcdir)/Makefile.gcov
