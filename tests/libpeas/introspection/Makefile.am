noinst_LTLIBRARIES = libintrospection-1.0.la

AM_CPPFLAGS = \
	-I$(top_srcdir)		\
	-I$(srcdir)		\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

libintrospection_1_0_la_LDFLAGS = $(TEST_INTROSPECTION_LIBTOOL_FLAGS)
libintrospection_1_0_la_LIBADD = \
	$(PEAS_LIBS)				\
	$(top_builddir)/libpeas/libpeas-1.0.la

libintrospection_1_0_la_SOURCES = \
	introspection-base.c			\
	introspection-base.h			\
	introspection-callable.c		\
	introspection-callable.h		\
	introspection-has-prerequisite.c	\
	introspection-has-prerequisite.h	\
	introspection-unimplemented.c		\
	introspection-unimplemented.h

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = Introspection-1.0.gir
INTROSPECTION_TYPELIBS = $(INTROSPECTION_GIRS:.gir=.typelib)
INTROSPECTION_SCANNER_ARGS = \
	--add-include-path=$(top_builddir)/libpeas	\
	--warn-all
INTROSPECTION_COMPILER_ARGS = \
	--includedir=$(top_srcdir)/libpeas	\
	--includedir=$(top_builddir)/libpeas

introspection_sources = $(libintrospection_1_0_la_SOURCES)

Introspection-1.0.gir: libintrospection-1.0.la
Introspection_1_0_gir_INCLUDES = GObject-2.0
Introspection_1_0_gir_CFLAGS = $(PEAS_CFLAGS) -I$(top_srcdir)
Introspection_1_0_gir_LIBS = $(top_builddir)/libpeas/libpeas-1.0.la libintrospection-1.0.la
Introspection_1_0_gir_FILES = $(addprefix $(srcdir)/,$(introspection_sources))
Introspection_1_0_gir_SCANNERFLAGS = --include-uninstalled=$(top_builddir)/libpeas/Peas-1.0.gir

CLEANFILES = $(INTROSPECTION_GIRS) $(INTROSPECTION_TYPELIBS)

all: Introspection-1.0.typelib

