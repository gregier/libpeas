include $(top_srcdir)/tests/Makefile.plugin

AM_CPPFLAGS = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

noinst_LTLIBRARIES = libembedded.la

libembedded_la_SOURCES = \
	embedded-plugin.c	\
	embedded-plugin.h	\
	embedded-resources.c	\
	embedded-resources.h

libembedded_la_LIBADD  = \
	$(PEAS_LIBS)				\
	$(top_builddir)/libpeas/libpeas-1.0.la

embedded_resources_deps = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir "$(srcdir)" --generate-dependencies "$(srcdir)/embedded.gresource.xml")
embedded_resources_c_deps = $(srcdir)/embedded.gresource.xml $(embedded_resources_deps)

embedded-resources.h: $(embedded_resources_c_deps)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES)		\
		--target="$@"				\
		--sourcedir="$(srcdir)"			\
		--generate				\
		--c-name="embedded"			\
		--manual-register			\
		--internal				\
		"$(srcdir)/embedded.gresource.xml"

embedded-resources.c: $(embedded_resources_c_deps) embedded-resources.h
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES)		\
		--target="$@"				\
		--sourcedir="$(srcdir)"			\
		--generate				\
		--c-name="embedded"			\
		--manual-register			\
		--internal				\
		"$(srcdir)/embedded.gresource.xml"

noinst_PLUGIN = \
	embedded.gresource.xml		\
	embedded.plugin			\
	embedded-invalid-loader.plugin	\
	embedded-missing-symbol.plugin

EXTRA_DIST = $(noinst_PLUGIN)

CLEANFILES = \
	embedded-resources.c	\
	embedded-resources.h

