lib_LTLIBRARIES = libpeasui-2.0.la

INCLUDES = \
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	$(PEAS_CFLAGS)			\
	$(PEASUI_CFLAGS)		\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED_CFLAGS)

libpeasui_2_0_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

libpeasui_2_0_la_LIBADD = \
	$(PEAS_LIBS)			\
	$(PEASUI_LIBS)

INST_H_FILES = \
	peas-ui-configurable.h		\
	peas-ui-plugin-info.h		\
	peas-ui-plugin-manager.h

libpeasui_2_0_la_SOURCES = \
	peas-ui-configurable.c		\
	peas-ui-plugin-info.c		\
	peas-ui-plugin-manager.c	\
	$(INST_H_FILES)

headerdir = $(prefix)/include/libpeas-2.0/libpeasui
header_DATA = $(INST_H_FILES)
