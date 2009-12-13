lib_LTLIBRARIES = libpeasui-2.0.la

INCLUDES =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	$(PEAS_CFLAGS)							\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)

libpeasui_2_0_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

libpeasui_2_0_la_LIBADD = $(PEAS_LIBS)

INST_H_FILES =				\
	peas-ui-plugin-manager.h

libpeasui_2_0_la_SOURCES = 		\
	peas-ui-plugin-manager.c	\
	$(INST_H_FILES)

headerdir = $(prefix)/include/libpeas-2.0/libpeasui
header_DATA = $(INST_H_FILES)

if ENABLE_INTROSPECTION
gir_DATA = PeasUI-2.0.gir
typelib_DATA = PeasUI-2.0.typelib

PeasUI-2.0.gir: $(G_IR_SCANNER) libpeasui-2.0.la Makefile
	$(AM_V_GEN) $(G_IR_SCANNER)			\
		--namespace=PeasUI			\
		--nsversion=2.0				\
		-I$(top_srcdir)				\
		--add-include-path=$(top_builddir)/libpeas \
		--include=GObject-2.0			\
		--include=Gtk-2.0			\
		--include=Peas-2.0			\
		--libtool="$(LIBTOOL)"			\
		--library=peasui-2.0			\
		--library=$(top_builddir)/libpeas/libpeas-2.0.la	\
		$(PEAS_CFLAGS)				\
		$(PEASUI_CFLAGS)			\
		$(addprefix $(srcdir)/,$(INST_H_FILES)) \
		$(addprefix $(srcdir)/,$(C_FILES)) 	\
		-o $@

PeasUI-2.0.typelib: $(gir_DATA) $(G_IR_COMPILER)
	$(AM_V_GEN) $(G_IR_COMPILER) $<	-o $@ --shared-library=libpeasui-2.0

CLEANFILES = \
	$(gir_DATA)		\
	$(typelib_DATA)

endif
