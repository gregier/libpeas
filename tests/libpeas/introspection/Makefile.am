dummydir = "$(abs_top_srcdir)/.dummy-install/introspection"

noinst_LTLIBRARIES = libintrospection-1.0.la

INTROSPECTION_INCLUDES = \
	-I$(top_srcdir)	\
	-I$(srcdir)	\
	$(PEAS_CFLAGS)

INCLUDES = \
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

libintrospection_1_0_la_LDFLAGS = \
	-export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

libintrospection_1_0_la_LIBADD = \
	$(PEAS_LIBS)				\
	$(top_builddir)/libpeas/libpeas-1.0.la

libintrospection_1_0_la_SOURCES = \
	introspection-callable.c		\
	introspection-callable.h		\
	introspection-unimplementable.c	\
	introspection-unimplementable.h

CLEANFILES = $(dummydir)

INCLUDES += $(INTROSPECTION_INCLUDES)


-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = Introspection-1.0.gir
INTROSPECTION_SCANNER_ARGS = \
	--add-include-path=$(top_builddir)/libpeas	\
	--warn-all
INTROSPECTION_COMPILER_ARGS = --includedir=$(top_builddir)/libpeas

introspection_sources = $(libintrospection_1_0_la_SOURCES)

Introspection-1.0.gir: libintrospection-1.0.la
Introspection_1_0_gir_INCLUDES = GObject-2.0 GIRepository-2.0 Peas-1.0
Introspection_1_0_gir_CFLAGS = $(INTROSPECTION_INCLUDES)
Introspection_1_0_gir_LIBS = libintrospection-1.0.la
Introspection_1_0_gir_FILES = $(addprefix $(srcdir)/,$(introspection_sources))

girdir = $(dummydir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(dummydir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
