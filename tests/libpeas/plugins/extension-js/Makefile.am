include $(top_srcdir)/tests/Makefile.plugin

noinst_DATA = \
	extension-gjs.gschema.xml	\
	extension-gjs.js		\
	extension-gjs.plugin		\
	extension-seed.gschema.xml	\
	extension-seed.js		\
	extension-seed.plugin

extension-js.%:
	test -e $@

extension-gjs.gschema.xml extension-seed.gschema.xml: extension-js.gschema.xml
	$(AM_V_GEN) cp $< $@ && \
	$(SED) -i -e 's%JS_LOADER%$(@:extension-%.gschema.xml=%)%g' $@

extension-gjs.js extension-seed.js: extension-js.js
	$(AM_V_GEN) $(LN_S) $< $@

extension-gjs.plugin extension-seed.plugin: extension-js.plugin
	$(AM_V_GEN) cp $< $@ && \
	$(SED) -i -e 's%JS_LOADER%$(@:extension-%.plugin=%)%g' $@

EXTRA_DIST = \
	extension-js.gschema.xml	\
	extension-js.js 		\
	extension-js.plugin

CLEANFILES = $(noinst_DATA)
DISTCLEANFILES = $(noinst_DATA)
