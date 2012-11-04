include $(top_srcdir)/tests/Makefile.tests

SUBDIRS = introspection plugins testing

INCLUDES = \
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	-I$(srcdir)/../testing-util	\
	$(PEAS_CFLAGS)			\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED)

noinst_PROGRAMS = $(TEST_PROGS)

LDADD = \
	$(PEAS_LIBS)				\
	$(top_builddir)/libpeas/libpeas-1.0.la	\
	./introspection/libintrospection-1.0.la	\
	./testing/libtesting.la			\
	../testing-util/libtesting-util.la

TEST_PROGS += \
	engine		\
	extension-c	\
	extension-set	\
	plugin-info

if ENABLE_GJS
TEST_PROGS            += extension-gjs
extension_gjs_CFLAGS   = $(GJS_CFLAGS)
extension_gjs_LDADD    = $(LDADD) $(GJS_LIBS)
endif

if ENABLE_PYTHON2
TEST_PROGS               += extension-python
extension_python_CFLAGS   = $(PYGOBJECT_CFLAGS) $(PYTHON2_CFLAGS)
extension_python_LDADD    = $(LDADD) $(PYGOBJECT_LIBS) $(PYTHON2_LIBS)
endif

if ENABLE_PYTHON3
TEST_PROGS                += extension-python3
extension_python3_CFLAGS   = $(PYGOBJECT_CFLAGS) $(PYTHON3_CFLAGS)
extension_python3_LDADD    = $(LDADD) $(PYGOBJECT_LIBS) $(PYTHON3_LIBS)
endif

if ENABLE_SEED
TEST_PROGS             += extension-seed
extension_seed_CFLAGS   = $(SEED_CFLAGS)
extension_seed_LDADD    = $(LDADD) $(SEED_LIBS)
endif
