include $(top_srcdir)/tests/Makefile.plugin

SUBDIRS = extension-c

if ENABLE_PYTHON
SUBDIRS += extension-python
endif

if ENABLE_SEED
SUBDIRS += extension-seed
endif

noinst_PLUGIN = \
	info-missing-iage.plugin	\
	info-missing-module.plugin	\
	info-missing-name.plugin	\
	invalid.plugin			\
	loader-disabled.plugin		\
	nonexistent-dep.plugin		\
	nonexistent-loader.plugin	\
	os-dependant-help.plugin

EXTRA_DIST = $(noinst_PLUGIN)
DISTCLEANFILES = $(noinst_PLUGIN)
