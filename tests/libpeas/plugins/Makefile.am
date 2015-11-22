include $(top_srcdir)/tests/Makefile.plugin

SUBDIRS = embedded extension-c

if ENABLE_LUA51
SUBDIRS += extension-lua
endif

if ENABLE_PYTHON3
SUBDIRS += extension-python
else
if ENABLE_PYTHON2
SUBDIRS += extension-python
endif
endif

noinst_PLUGIN = \
	extension-c-nonexistent.plugin		\
	extension-lua51-nonexistent.plugin	\
	extension-python-nonexistent.plugin	\
	extension-python3-nonexistent.plugin	\
	info-missing-module.plugin		\
	info-missing-name.plugin		\
	invalid.plugin				\
	nonexistent-dep.plugin			\
	not-loadable.plugin			\
	os-dependant-help.plugin		\
	two-deps.plugin				\
	unkown-loader.plugin

EXTRA_DIST = $(noinst_PLUGIN)
