SUBDIRS =

if ENABLE_PYTHON2
SUBDIRS += python
endif

if ENABLE_PYTHON3
SUBDIRS += python3
endif

if ENABLE_SEED
SUBDIRS += seed
endif

gcov:
	@for dir in $(SUBDIRS); do \
	   (cd $$dir && $(MAKE) $(AM_MAKEFLAGS) $(@)); \
	 done
