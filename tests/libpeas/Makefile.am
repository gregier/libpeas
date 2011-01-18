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


TEST_PROGS     += engine
engine_SOURCES  = engine.c
engine_LDADD    = $(progs_ldadd)

TEST_PROGS          += extension-c
extension_c_SOURCES  = extension-c.c
extension_c_LDADD    = $(progs_ldadd)

if ENABLE_PYTHON
TEST_PROGS               += extension-python
extension_python_SOURCES  = extension-python.c
extension_python_LDADD    = $(progs_ldadd)
endif

if ENABLE_SEED
TEST_PROGS             += extension-seed
extension_seed_SOURCES  = extension-seed.c
extension_seed_LDADD    = $(progs_ldadd)
endif

TEST_PROGS            += extension-set
extension_set_SOURCES  = extension-set.c
extension_set_LDADD    = $(progs_ldadd)

TEST_PROGS          += plugin-info
plugin_info_SOURCES  = plugin-info.c
plugin_info_LDADD    = $(progs_ldadd)
