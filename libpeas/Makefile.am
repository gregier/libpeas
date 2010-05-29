lib_LTLIBRARIES = libpeas-1.0.la

INCLUDES =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	$(PEAS_CFLAGS)							\
	$(IGE_MAC_CFLAGS)						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED)						\
	-DDATADIR=\""$(datadir)"\"					\
	-DLIBDIR=\""$(libdir)"\"

libpeas_1_0_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

libpeas_1_0_la_LIBADD = $(PEAS_LIBS) $(IGE_MAC_LIBS)

INST_H_FILES =			\
	peas-plugin-info.h	\
	peas-object-module.h	\
	peas-extension-base.h	\
	peas-extension.h	\
	peas-extension-set.h	\
	peas-activatable.h	\
	peas-engine.h

NOINST_H_FILES =		\
	peas-dirs.h		\
	peas-i18n.h		\
	peas-introspection.h	\
	peas-plugin-info-priv.h	\
	peas-plugin-loader.h

C_FILES =			\
	peas-dirs.c		\
	peas-i18n.c		\
	peas-object-module.c	\
	peas-introspection.c	\
	peas-plugin-info.c	\
	peas-plugin-loader.c	\
	peas-extension-base.c	\
	peas-extension.c	\
	peas-extension-set.c	\
	peas-activatable.c	\
	peas-engine.c

BUILT_SOURCES = \
	peas-marshal.c			\
	peas-marshal.h

libpeas_1_0_la_SOURCES = 	\
	$(INST_H_FILES)		\
	$(NOINST_H_FILES)	\
	$(BUILT_SOURCES)	\
	$(C_FILES)

headerdir = $(prefix)/include/libpeas-1.0/libpeas
header_DATA = $(INST_H_FILES)

peas-marshal.c: peas-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) (echo "#include \"peas-marshal.h\"" && $(GLIB_GENMARSHAL) $< --body --prefix=peas_cclosure_marshal) > $@

peas-marshal.h: peas-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=peas_cclosure_marshal > $@

EXTRA_DIST = peas-marshal.list

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

if HAVE_INTROSPECTION
  -include $(INTROSPECTION_MAKEFILE)
  INTROSPECTION_GIRS = Peas-1.0.gir

  introspection_sources = $(INST_H_FILES) $(C_FILES)

  Peas-1.0.gir: libpeas-1.0.la
  Peas_1_0_gir_INCLUDES = GObject-2.0 GModule-2.0
  Peas_1_0_gir_CFLAGS = $(PEAS_CFLAGS)
  Peas_1_0_gir_LIBS = libpeas-1.0.la
  Peas_1_0_gir_FILES = $(addprefix $(srcdir)/,$(introspection_sources))

  girdir = $(datadir)/gir-1.0
  dist_gir_DATA = $(INTROSPECTION_GIRS)

  typelibdir = $(libdir)/girepository-1.0
  typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

  CLEANFILES += \
	$(dist_gir_DATA)		\
	$(typelib_DATA)

endif
