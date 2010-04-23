-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(top_builddir)/libpeas


lib_LTLIBRARIES = libpeasui-2.0.la

INCLUDES = \
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	$(PEAS_CFLAGS)			\
	$(PEASUI_CFLAGS)		\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED_CFLAGS)

libpeasui_2_0_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

libpeasui_2_0_la_LIBADD = \
	$(PEAS_LIBS)							\
	$(PEASUI_LIBS)							\
	$(top_builddir)/libpeas/libpeas-2.0.la

INST_H_FILES = \
	peas-ui-configurable.h		\
	peas-ui-plugin-info.h		\
	peas-ui-plugin-manager.h

libpeasui_2_0_la_SOURCES = \
	peas-ui-configurable.c		\
	peas-ui-plugin-info.c		\
	peas-ui-plugin-manager.c	\
	$(INST_H_FILES)

headerdir = $(prefix)/include/libpeas-2.0/libpeasui
header_DATA = $(INST_H_FILES)

if HAVE_INTROSPECTION
  introspection_sources = $(libpeasui_2_0_la_SOURCES)

  PeasuUI-2.0.gir: libpeasui-2.0.la
  PeasUI_2_0_gir_INCLUDES = GObject-2.0 Gtk-2.0 Peas-2.0
  PeasUI_2_0_gir_CFLAGS = $(PEAS_CFLAGS) $(PEASUI_CFLAGS)
  PeasUI_2_0_gir_LIBS = libpeasui-2.0.la
  PeasUI_2_0_gir_FILES = $(addprefix $(srcdir)/,$(introspection_sources))
  INTROSPECTION_GIRS += PeasUI-2.0.gir

  girdir = $(datadir)/gir-1.0 ../libpeas
  dist_gir_DATA = $(INTROSPECTION_GIRS)

  typelibdir = $(libdir)/girepository-1.0
  typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

  CLEANFILES = $(dist_gir_DATA) $(typelib_DATA)
endif
