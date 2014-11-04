SUBDIRS =

if ENABLE_LUA51
SUBDIRS += lua5.1
endif

if ENABLE_PYTHON2
SUBDIRS += python
endif

if ENABLE_PYTHON3
SUBDIRS += python3
endif

gcov:
	@for dir in $(SUBDIRS); do \
	   (cd $$dir && $(MAKE) $(AM_MAKEFLAGS) $(@)); \
	 done
