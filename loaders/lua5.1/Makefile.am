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
	peas-lua-resources.c		\
	peas-plugin-loader-lua.c	\
	peas-plugin-loader-lua.h	\
	peas-lua-utils.c		\
	peas-lua-utils.h

liblua51loader_la_LDFLAGS = \
	$(LOADER_LIBTOOL_FLAGS)		\
	$(GCOV_LDFLAGS)

liblua51loader_la_LIBADD = \
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	$(PEAS_LIBS)				\
	$(LUA51_LIBS)

loader_resources_deps = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir)/resources --generate-dependencies $(srcdir)/resources/peas-lua.gresource.xml)
peas-lua-resources.c: $(srcdir)/resources/peas-lua.gresource.xml $(loader_resources_deps)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --internal --target=$@ --sourcedir=$(srcdir)/resources --generate-source $(srcdir)/resources/peas-lua.gresource.xml

EXTRA_DIST = \
	$(loader_resources_deps)

CLEANFILES = peas-lua-resources.c

gcov_sources = $(liblua51loader_la_SOURCES)
include $(top_srcdir)/Makefile.gcov
