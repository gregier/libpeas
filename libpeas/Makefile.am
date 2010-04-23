lib_LTLIBRARIES = libpeas-2.0.la

INCLUDES =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	$(PEAS_CFLAGS)							\
	$(IGE_MAC_CFLAGS)						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED)						\
	-DDATADIR=\""$(datadir)"\"					\
	-DLIBDIR=\""$(libdir)"\"

libpeas_2_0_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

libpeas_2_0_la_LIBADD = $(PEAS_LIBS) $(IGE_MAC_LIBS)

INST_H_FILES =			\
	peas-plugin.h		\
	peas-plugin-info.h	\
	peas-engine.h

NOINST_H_FILES =		\
	peas-dirs.h		\
	peas-i18n.h		\
	peas-object-module.h	\
	peas-plugin-info-priv.h	\
	peas-plugin-loader.h

C_FILES =			\
	peas-dirs.c		\
	peas-i18n.c		\
	peas-object-module.c	\
	peas-plugin.c		\
	peas-plugin-info.c	\
	peas-plugin-loader.c	\
	peas-engine.c

libpeas_2_0_la_SOURCES = 	\
	$(INST_H_FILES)		\
	$(NOINST_H_FILES)	\
	$(C_FILES)

headerdir = $(prefix)/include/libpeas-2.0/libpeas
header_DATA = $(INST_H_FILES)

if HAVE_INTROSPECTION
  -include $(INTROSPECTION_MAKEFILE)
  INTROSPECTION_GIRS = Peas-2.0.gir

  introspection_sources = $(libpeas_2_0_la_SOURCES)

  Peas-2.0.gir: libpeas-2.0.la
  Peas_2_0_gir_INCLUDES = GObject-2.0 Gtk-2.0
  Peas_2_0_gir_CFLAGS = $(PEAS_CFLAGS)
  Peas_2_0_gir_LIBS = libpeas-2.0.la
  Peas_2_0_gir_FILES = $(addprefix $(srcdir)/,$(introspection_sources))

  girdir = $(datadir)/gir-1.0
  dist_gir_DATA = $(INTROSPECTION_GIRS)

  typelibdir = $(libdir)/girepository-1.0
  typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES = \
	$(dist_gir_DATA)		\
	$(typelib_DATA)

endif
