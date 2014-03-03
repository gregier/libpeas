include $(top_srcdir)/tests/Makefile.tests

SUBDIRS = plugins testing

run-test-hook:
	@test -n "$(DISPLAY)"

AM_CPPFLAGS = \
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	-I$(srcdir)/../testing-util	\
	$(PEAS_CFLAGS)			\
	$(PEAS_GTK_CFLAGS)		\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED)

noinst_PROGRAMS = $(TEST_PROGS)

LDADD = \
	$(PEAS_LIBS)					\
	$(PEAS_GTK_LIBS)				\
	$(top_builddir)/libpeas/libpeas-1.0.la		\
	$(top_builddir)/libpeas-gtk/libpeas-gtk-1.0.la	\
	./testing/libtesting.la				\
	../testing-util/libtesting-util.la

TEST_PROGS += \
	plugin-manager		\
	plugin-manager-store	\
	plugin-manager-view
