lib_LTLIBRARIES = libgpeui-2.0.la

INCLUDES =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	$(GPE_CFLAGS)							\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)

libgpeui_2_0_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

libgpeui_2_0_la_LIBADD = $(GPE_LIBS)

INST_H_FILES =			\
	gpe-plugin-manager.h

libgpeui_2_0_la_SOURCES = 	\
	gpe-plugin-manager.c	\
	$(INST_H_FILES)
