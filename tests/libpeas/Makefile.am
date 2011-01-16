include $(top_srcdir)/tests/Makefile.tests

SUBDIRS = introspection plugins testing

INCLUDES = \
	-I$(top_srcdir)		\
	-I$(srcdir)		\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

noinst_PROGRAMS = $(TEST_PROGS)

progs_ldadd = \
	$(PEAS_LIBS)				\
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	./testing/libtesting.la			\
	./introspection/libintrospection-1.0.la

TEST_PROGS += \
	engine		\
	extension	\
	extension-set	\
	plugin-info

engine_SOURCES = engine.c
engine_LDADD   = $(progs_ldadd)

extension_SOURCES = extension.c
extension_LDADD   = $(progs_ldadd)

extension_set_SOURCES = extension-set.c
extension_set_LDADD   = $(progs_ldadd)

plugin_info_SOURCES = plugin-info.c
plugin_info_LDADD   = $(progs_ldadd)
