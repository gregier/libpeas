GTESTER = gtester

TEST_PROGS =

SUBDIRS = introspection plugins testing

test: $(TEST_PROGS)
	@test -z "$(TEST_PROGS)" || \
	 $(GTESTER) --verbose $(TEST_PROGS)

test-gdb: $(TEST_PROGS)
	@test -z "$(TEST_PROGS)" || \
	 for test_prog in $(TEST_PROGS); do \
	   $(GTESTER) --verbose $$test_prog || { \
	     gdb --ex "run --verbose" ".libs/lt-$$test_prog" ; \
	     exit 1 ; \
	   } \
	 done

generate-report: $(TEST_PROGS)
	@test -z "$(TEST_PROGS)" || \
	 $(GTESTER) $(GTESTER_ARGS) $(TEST_PROGS)


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
