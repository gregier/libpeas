SUBDIRS=plugins

bin_PROGRAMS = peas-demo

INCLUDES =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	$(PEAS_CFLAGS)							\
	$(PEASUI_CFLAGS)						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED)

peas_demo_SOURCES = \
	peas-demo-window.c		\
	peas-demo-window.h		\
	peas-demo.c

peas_demo_LDADD = \
	$(PEAS_LIBS)			\
	$(PEASUI_LIBS)			\
	../libpeas/libpeas-1.0.la	\
	../libpeasui/libpeasui-1.0.la

peas_demo_CFLAGS = \
	$(PEAS_CFLAGS)			\
	-DPEAS_PREFIX="\"$(prefix)\""

.PHONY: demo
demo: $(bin_PROGRAMS)
	./peas-demo --run-from-build-dir
