INCLUDES = \
	-I$(top_srcdir)				\
	-I$(srcdir)				\
	-I$(srcdir)/../introspection		\
	$(PEAS_CFLAGS)				\
	$(WARN_CFLAGS)				\
	$(DISABLE_DEPRECATED)			\
	-DBUILDDIR="\"$(abs_top_builddir)\""

noinst_LTLIBRARIES = libtesting.la

libtesting_la_LDFLAGS = \
	-export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

libtesting_la_LIBADD = \
	$(PEAS_LIBS)					\
	$(top_builddir)/libpeas/libpeas-1.0.la		\
	../introspection/libintrospection-1.0.la

libtesting_la_SOURCES = \
	testing.c		\
	testing.h		\
	testing-extension.c	\
	testing-extension.h
