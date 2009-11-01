BINDINGS_HDRS = \
	$(top_srcdir)/libgpe/gpe-plugin.h		\
	$(top_srcdir)/libgpe/gpe-plugin-info.h		\
	$(top_srcdir)/libgpe/gpe-engine.h

pyexec_LTLIBRARIES = \
	libgpe.la

libgpe_la_SOURCES = \
	libgpemodule.c

nodist_libgpe_la_SOURCES = \
	libgpe.c

libgpe_la_LDFLAGS = \
	-module -avoid-version 				\
	-export-symbol-regex initlibgpe

libgpe_la_LIBADD = \
	$(top_builddir)/libgpe/libgpe-2.0.la 		\
	$(PYTHON_LIB_LOC)				\
	$(PYTHON_LIBS)					\
	$(PYTHON_EXTRA_LIBS)				\
	$(PYGTK_LIBS)

libgpe_la_CFLAGS = 					\
	-I$(top_srcdir)					\
	-I$(top_builddir)				\
	$(GPE_CFLAGS)					\
	$(NO_STRICT_ALIASING_CFLAGS)			\
	$(PYGTK_CFLAGS)					\
	$(PYTHON_CFLAGS)				\
	$(AM_CFLAGS)

libgpe.defs: $(BINDINGS_HDRS) libgpe.defs.extra
	$(AM_V_GEN) ( $(PYGTK_H2DEF) -n GPE $(BINDINGS_HDRS); cat $@.extra ) > $@

libgpe.c: libgpe.defs libgpe.override
	$(AM_V_GEN) ( cd $(srcdir) && $(PYGTK_CODEGEN) \
		--register $(PYGTK_DEFSDIR)/gtk-types.defs \
		--override libgpe.override \
		--prefix pygpe $< ) > $@

defsdir = $(datadir)/pygtk/2.0/defs
defs_DATA = libgpe.defs

BUILT_SOURCES = \
	libgpe.c

EXTRA_DIST = \
	libgpe.override	\
	libgpe.defs.extra

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
