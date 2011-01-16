GTESTER = gtester

TEST_PROGS =

SUBDIRS = plugins testing

test: $(TEST_PROGS)
	@test -z "$(TEST_PROGS)" || \
	 test -z "$(DISPLAY)" || \
	 $(GTESTER) --verbose $(TEST_PROGS)

test-gdb: $(TEST_PROGS)
	@test -z "$(TEST_PROGS)" || \
	 test -z "$(DISPLAY)" || \
	 for test_prog in $(TEST_PROGS); do \
	   $(GTESTER) --verbose $$test_prog || { \
	     libtool --mode=execute gdb --ex "run --verbose" $$test_prog ; \
	     exit 1 ; \
	   } \
	 done

generate-report: $(TEST_PROGS)
	@test -z "$(TEST_PROGS)" || \
	 test -z "$(DISPLAY)" || \
	 $(GTESTER) $(GTESTER_ARGS) $(TEST_PROGS)

INCLUDES = \
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	$(PEAS_CFLAGS)			\
	$(PEAS_GTK_CFLAGS)		\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED)

noinst_PROGRAMS = $(TEST_PROGS)

progs_ldadd = \
	$(PEAS_LIBS)					\
	$(PEAS_GTK_LIBS)				\
	$(top_builddir)/libpeas/libpeas-1.0.la		\
	$(top_builddir)/libpeas-gtk/libpeas-gtk-1.0.la	\
	./testing/libtesting.la

TEST_PROGS += \
	plugin-manager		\
	plugin-manager-store	\
	plugin-manager-view

plugin_manager_SOURCES = plugin-manager.c
plugin_manager_LDADD   = $(progs_ldadd)

plugin_manager_store_SOURCES = plugin-manager-store.c
plugin_manager_store_LDADD   = $(progs_ldadd)

plugin_manager_view_SOURCES = plugin-manager-view.c
plugin_manager_view_LDADD   = $(progs_ldadd)
