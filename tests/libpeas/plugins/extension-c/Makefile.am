include $(top_srcdir)/tests/Makefile.plugin

AM_CPPFLAGS = \
	-I$(top_srcdir)		\
	-I$(srcdir)/../../introspection	\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

noinst_LTLIBRARIES = \
	libextension-c.la			\
	libextension-c-missing-symbol.la

libextension_c_la_SOURCES = \
	extension-c-plugin.c	\
	extension-c-plugin.h

libextension_c_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)
libextension_c_la_LIBADD = \
	$(PEAS_LIBS)						\
	$(top_builddir)/libpeas/libpeas-1.0.la	                \
	$(builddir)/../../introspection/libintrospection-1.0.la

libextension_c_missing_symbol_la_SOURCES = \
	extension-c-missing-symbol-plugin.c

libextension_c_missing_symbol_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)
libextension_c_missing_symbol_la_LIBADD = $(PEAS_LIBS)

noinst_PLUGIN = \
	extension-c.gschema.xml			\
	extension-c.plugin			\
	extension-c-missing-symbol.plugin

EXTRA_DIST = $(noinst_PLUGIN)
