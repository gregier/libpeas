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

libgpe_2_0_la_SOURCES = 	\
	gpe-dirs.c		\
	gpe-object-module.c	\
	gpe-plugin.c		\
	gpe-plugin-info.c	\
	gpe-plugin-loader.c	\
	gpe-engine.c		\
	$(INST_H_FILES)		\
	$(NOINST_H_FILES)
