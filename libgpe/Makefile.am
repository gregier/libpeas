lib_LTLIBRARIES = libgpe-2.0.la

INCLUDES =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	$(GPE_CFLAGS)							\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	-DDATADIR=\""$(datadir)"\"					\
	-DLIBDIR=\""$(libdir)"\"

libgpe_2_0_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

libgpe_2_0_la_LIBADD = $(GPE_LIBS)

INST_H_FILES =			\
	gpe-plugin.h		\
	gpe-plugin-info.h	\
	gpe-engine.h

NOINST_H_FILES =		\
	gpe-dirs.h		\
	gpe-object-module.h	\
	gpe-plugin-info-priv.h	\
	gpe-plugin-loader.h

C_FILES =			\
	gpe-dirs.c		\
	gpe-object-module.c	\
	gpe-plugin.c		\
	gpe-plugin-info.c	\
	gpe-plugin-loader.c	\
	gpe-engine.c

libgpe_2_0_la_SOURCES = 	\
	$(INST_H_FILES)		\
	$(NOINST_H_FILES)	\
	$(C_FILES)

headerdir = $(prefix)/include/libgpe-2.0/libgpe
header_DATA = $(INST_H_FILES)

if ENABLE_INTROSPECTION
gir_DATA = GPE-2.0.gir
typelib_DATA = GPE-2.0.typelib

GPE-2.0.gir: $(G_IR_SCANNER) libgpe-2.0.la Makefile
	$(AM_V_GEN) $(G_IR_SCANNER)			\
		--namespace=GPE				\
		--nsversion=2.0				\
		--library=gpe-2.0			\
		--include=GObject-2.0			\
		--include=Gtk-2.0			\
		--libtool="$(LIBTOOL)"			\
		$(GPE_CFLAGS)				\
		$(addprefix $(srcdir)/,$(INST_H_FILES)) \
		$(addprefix $(srcdir)/,$(C_FILES)) 	\
		-o $@

GPE-2.0.typelib: $(gir_DATA) $(G_IR_COMPILER)
	$(AM_V_GEN) $(G_IR_COMPILER) $<	-o $@ --shared-library=libgpe.la

CLEANFILES = \
	$(gir_DATA)		\
	$(typelib_DATA)

endif
