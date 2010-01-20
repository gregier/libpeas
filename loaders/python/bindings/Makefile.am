PEAS_BINDINGS_HDRS = \
	$(top_srcdir)/libpeas/peas-plugin.h		\
	$(top_srcdir)/libpeas/peas-plugin-info.h	\
	$(top_srcdir)/libpeas/peas-engine.h

PEAS_UI_BINDINGS_HDRS = \
	$(top_srcdir)/libpeasui/peas-ui-configurable.h	\
	$(top_srcdir)/libpeasui/peas-ui-plugin-manager.h

pyexec_LTLIBRARIES = \
	libpeas.la					\
	libpeasui.la

libpeas_la_SOURCES = \
	libpeasmodule.c

nodist_libpeas_la_SOURCES = \
	libpeas.c

libpeasui_la_SOURCES = \
	libpeasuimodule.c

nodist_libpeasui_la_SOURCES = \
	libpeasui.c

AM_LDFLAGS = \
	-module -avoid-version 				\
	-export-symbol-regex initlibpeas

libpeas_la_LDFLAGS = $(AM_LIBADD)
libpeasui_la_LDFLAGS = $(AM_LIBADD)

AM_LIBADD = \
	$(top_builddir)/libpeas/libpeas-2.0.la 		\
	$(PEAS_LIBS)					\
	$(PYTHON_LIB_LOC)				\
	$(PYTHON_LIBS)					\
	$(PYTHON_EXTRA_LIBS)				\
	$(PYGTK_LIBS)

libpeas_la_LIBADD = $(AM_LIBADD)
libpeasui_la_LIBADD = \
	$(AM_LIBADD)					\
	$(top_builddir)/libpeasui/libpeasui-2.0.la	\
	$(PEASUI_LIBS)

AM_CFLAGS = \
	-I$(top_srcdir)					\
	-I$(top_builddir)				\
	$(PEAS_CFLAGS)					\
	$(NO_STRICT_ALIASING_CFLAGS)			\
	$(PYGTK_CFLAGS)					\
	$(PYTHON_CFLAGS)

libpeas_la_CFLAGS = $(AM_CFLAGS)
libpeasui_la_CFLAGS = $(AM_CFLAGS) $(PEASUI_CFLAGS)

libpeas.defs: $(PEAS_BINDINGS_HDRS) libpeas.defs.extra
	$(AM_V_GEN) ( $(PYGTK_H2DEF) \
		--namespace Peas \
		$(PEAS_BINDINGS_HDRS) \
		| sed -e 's/define-object \(Activatable\)/define-interface \1/'; \
		cat $(srcdir)/libpeas.defs.extra ) > $@

libpeas.c: libpeas.defs libpeas.override
	$(AM_V_GEN) ( $(PYGTK_CODEGEN) \
		--override $(srcdir)/libpeas.override \
		--prefix pypeas \
		libpeas.defs ) > $@

libpeasui.defs: $(PEAS_UI_BINDINGS_HDRS) libpeasui.defs.extra
	$(AM_V_GEN) ( $(PYGTK_H2DEF) \
		--modulename peas_ui \
		--namespace PeasUI \
		$(PEAS_UI_BINDINGS_HDRS) \
		| sed -e 's/PEASUI/PEAS_UI/' \
		      -e 's/define-object \(Configurable\)/define-interface \1/'; \
		cat $(srcdir)/libpeasui.defs.extra ) > $@

libpeasui.c: libpeasui.defs libpeasui.override
	$(AM_V_GEN) ( $(PYGTK_CODEGEN) \
		--register $(PYGTK_DEFSDIR)/gtk-types.defs \
		--register libpeas.defs \
		--override $(srcdir)/libpeasui.override \
		--prefix pypeasui \
		libpeasui.defs ) > $@

defsdir = $(datadir)/pygtk/2.0/defs
defs_DATA = \
	libpeas.defs					\
	libpeasui.defs

BUILT_SOURCES = \
	$(nodist_libpeas_la_SOURCES)			\
	$(nodist_libpeasui_la_SOURCES)			\
	$(defs_DATA)

EXTRA_DIST = \
	libpeas.override				\
	libpeas.defs.extra				\
	libpeasui.override

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
