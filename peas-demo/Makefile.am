SUBDIRS=plugins

bin_PROGRAMS = peas-demo

peas_demo_SOURCES = \
	peas-demo.c

peas_demo_LDADD = \
	$(PEAS_LIBS)			\
	../libpeas/libpeas-2.0.la		\
	../libpeasui/libpeasui-2.0.la

peas_demo_CFLAGS = \
	$(PEAS_CFLAGS)			\
	-DPEAS_PREFIX="\"$(prefix)\""
