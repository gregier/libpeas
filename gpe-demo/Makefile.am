SUBDIRS=plugins

bin_PROGRAMS = gpe-demo

gpe_demo_SOURCES = \
	gpe-demo.c

gpe_demo_LDADD = \
	$(GPE_LIBS)			\
	../libgpe/libgpe-2.0.la

gpe_demo_CFLAGS = \
	$(GPE_CFLAGS)			\
	-DGPE_PREFIX="\"$(prefix)\""
