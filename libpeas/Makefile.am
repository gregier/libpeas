lib_LTLIBRARIES = libpeas-2.0.la

INCLUDES =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	$(PEAS_CFLAGS)							\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	-DDATADIR=\""$(datadir)"\"					\
	-DLIBDIR=\""$(libdir)"\"

libpeas_2_0_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

libpeas_2_0_la_LIBADD = $(PEAS_LIBS)

INST_H_FILES =			\
	peas-plugin.h		\
	peas-plugin-info.h	\
	peas-engine.h

NOINST_H_FILES =		\
	peas-dirs.h		\
	peas-object-module.h	\
	peas-plugin-info-priv.h	\
	peas-plugin-loader.h

C_FILES =			\
	peas-dirs.c		\
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

if ENABLE_INTROSPECTION
gir_DATA = Peas-2.0.gir
typelib_DATA = Peas-2.0.typelib

Peas-2.0.gir: $(G_IR_SCANNER) libpeas-2.0.la Makefile
	$(AM_V_GEN) $(G_IR_SCANNER)			\
		--namespace=Peas				\
		--nsversion=2.0				\
		--library=peas-2.0			\
		--include=GObject-2.0			\
		--include=Gtk-2.0			\
		--libtool="$(LIBTOOL)"			\
		$(PEAS_CFLAGS)				\
		$(addprefix $(srcdir)/,$(INST_H_FILES)) \
		$(addprefix $(srcdir)/,$(C_FILES)) 	\
		-o $@

Peas-2.0.typelib: $(gir_DATA) $(G_IR_COMPILER)
	$(AM_V_GEN) $(G_IR_COMPILER) $<	-o $@ --shared-library=libpeas.la

CLEANFILES = \
	$(gir_DATA)		\
	$(typelib_DATA)

endif
