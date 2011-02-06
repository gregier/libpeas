SUBDIRS = extension-c

if ENABLE_PYTHON
SUBDIRS += extension-python
endif

if ENABLE_SEED
SUBDIRS += extension-seed
endif

noinst_DATA = \
	info-missing-iage.plugin	\
	info-missing-module.plugin	\
	info-missing-name.plugin	\
	invalid-loader.plugin		\
	os-dependant-help.plugin

EXTRA_DIST = $(noinst_DATA)
