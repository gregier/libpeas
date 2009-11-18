BINDINGS_HDRS = \
	$(top_srcdir)/libpeas/peas-plugin.h		\
	$(top_srcdir)/libpeas/peas-plugin-info.h		\
	$(top_srcdir)/libpeas/peas-engine.h

pyexec_LTLIBRARIES = \
	libpeas.la

libpeas_la_SOURCES = \
	libpeasmodule.c

nodist_libpeas_la_SOURCES = \
	libpeas.c

libpeas_la_LDFLAGS = \
	-module -avoid-version 				\
	-export-symbol-regex initlibpeas

libpeas_la_LIBADD = \
	$(top_builddir)/libpeas/libpeas-2.0.la 		\
	$(PYTHON_LIB_LOC)				\
	$(PYTHON_LIBS)					\
	$(PYTHON_EXTRA_LIBS)				\
	$(PYGTK_LIBS)

libpeas_la_CFLAGS = 					\
	-I$(top_srcdir)					\
	-I$(top_builddir)				\
	$(PEAS_CFLAGS)					\
	$(NO_STRICT_ALIASING_CFLAGS)			\
	$(PYGTK_CFLAGS)					\
	$(PYTHON_CFLAGS)				\
	$(AM_CFLAGS)

libpeas.defs: $(BINDINGS_HDRS) libpeas.defs.extra
	$(AM_V_GEN) ( $(PYGTK_H2DEF) -n Peas $(BINDINGS_HDRS); cat $@.extra ) > $@

libpeas.c: libpeas.defs libpeas.override
	$(AM_V_GEN) ( cd $(srcdir) && $(PYGTK_CODEGEN) \
		--register $(PYGTK_DEFSDIR)/gtk-types.defs \
		--override libpeas.override \
		--prefix pypeas $< ) > $@

defsdir = $(datadir)/pygtk/2.0/defs
defs_DATA = libpeas.defs

BUILT_SOURCES = \
	libpeas.c

EXTRA_DIST = \
	libpeas.override	\
	libpeas.defs.extra

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
