include $(top_srcdir)/tests/Makefile.plugin

SUBDIRS = extension-c

if ENABLE_GJS
SUBDIRS += extension-js
else
if ENABLE_SEED
SUBDIRS += extension-js
endif
endif

if ENABLE_PYTHON3
SUBDIRS += extension-python
else
if ENABLE_PYTHON2
SUBDIRS += extension-python
endif
endif

noinst_PLUGIN = \
	disabled-loader.plugin			\
	extension-c-nonexistent.plugin		\
	extension-gjs-nonexistent.plugin	\
	extension-python-nonexistent.plugin	\
	extension-python3-nonexistent.plugin	\
	extension-seed-nonexistent.plugin	\
	info-missing-module.plugin		\
	info-missing-name.plugin		\
	invalid.plugin				\
	invalid-loader.plugin			\
	nonexistent-dep.plugin			\
	nonexistent-loader.plugin		\
	not-loadable.plugin			\
	os-dependant-help.plugin

EXTRA_DIST = $(noinst_PLUGIN)
