lib_LTLIBRARIES = libpeasui-1.0.la

INCLUDES = \
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	$(PEAS_CFLAGS)			\
	$(PEASUI_CFLAGS)		\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED)

libpeasui_1_0_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

libpeasui_1_0_la_LIBADD = \
	$(PEAS_LIBS)							\
	$(PEASUI_LIBS)							\
	$(top_builddir)/libpeas/libpeas-1.0.la

INST_H_FILES = \
	peas-ui-configurable.h		\
	peas-ui-plugin-manager.h	\
	peas-ui-plugin-manager-view.h	\
	peas-ui.h

NOINST_H_FILES = \
	peas-ui-plugin-manager-store.h

C_FILES = \
	peas-ui-configurable.c		\
	peas-ui-plugin-manager.c	\
	peas-ui-plugin-manager-store.c	\
	peas-ui-plugin-manager-view.c

libpeasui_1_0_la_SOURCES = \
	$(INST_H_FILES)			\
	$(NOINST_H_FILES)		\
	$(C_FILES)

headerdir = $(prefix)/include/libpeas-1.0/libpeasui
header_DATA = $(INST_H_FILES)

if HAVE_INTROSPECTION
  -include $(INTROSPECTION_MAKEFILE)
  INTROSPECTION_GIRS = PeasUI-1.0.gir
  INTROSPECTION_SCANNER_ARGS = \
	--add-include-path=$(top_builddir)/libpeas	\
	--c-include=libpeasui/peas-ui.h			\
	$(PEAS_GIR_SCANNER_ARGS)
  INTROSPECTION_COMPILER_ARGS = --includedir=$(top_srcdir)/libpeas

  introspection_sources = $(INST_H_FILES) $(C_FILES)

  PeasUI-1.0.gir: libpeasui-1.0.la
  PeasUI_1_0_gir_INCLUDES = GObject-2.0 $(GTK_TYPELIB) Peas-1.0
  PeasUI_1_0_gir_CFLAGS = $(PEAS_CFLAGS) $(PEASUI_CFLAGS) -I$(top_srcdir)
  PeasUI_1_0_gir_LIBS = libpeasui-1.0.la
  PeasUI_1_0_gir_FILES = $(addprefix $(srcdir)/,$(introspection_sources))

  girdir = $(datadir)/gir-1.0
  gir_DATA = $(INTROSPECTION_GIRS)

  typelibdir = $(libdir)/girepository-1.0
  typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

  CLEANFILES = $(gir_DATA) $(typelib_DATA)
endif
