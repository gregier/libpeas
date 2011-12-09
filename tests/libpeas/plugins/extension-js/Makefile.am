noinst_DATA = \
	extension-gjs.gschema.xml	\
	extension-gjs.js		\
	extension-gjs.plugin		\
	extension-seed.gschema.xml	\
	extension-seed.js		\
	extension-seed.plugin

extension-js.%:
	test -e $@

%.gschema.xml: extension-js.gschema.xml
	$(AM_V_GEN) cp $< $@ && \
	$(SED) -i -e 's/JS_LOADER/$(@:extension-%.gschema.xml=%)/g' $@

%.js: extension-js.js
	$(AM_V_GEN) $(LN_S) $< $@

%.plugin: extension-js.plugin
	$(AM_V_GEN) cp $< $@ && \
	$(SED) -i -e 's/JS_LOADER/$(@:extension-%.plugin=%)/g' $@

EXTRA_DIST = \
	extension-js.gschema.xml	\
	extension-js.js 		\
	extension-js.plugin

CLEANFILES = $(noinst_DATA)
DISTCLEANFILES = $(noinst_DATA)
