loader_resources_deps = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir="$(srcdir)" --generate-dependencies "$(srcdir)/peas-lua.gresource.xml")
loader_resources_c_deps = $(srcdir)/peas-lua.gresource.xml $(loader_resources_deps)

LUAC_FILES = $(addsuffix c,$(loader_resources_deps))

peas-lua-resources.c: $(loader_resources_c_deps)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES)			\
		--target="$@"					\
		--sourcedir="$(srcdir)"		\
		--generate-source				\
		--internal 					\
		"$(srcdir)/peas-lua.gresource.xml"

%.luac: %.lua
	$(AM_V_GEN) $(LUA51_BIN) $(srcdir)/peas-lua-compile.lua $< $@

all-local: $(LUAC_FILES) peas-lua-resources.c

CLEANFILES = \
	peas-lua-resources.c	\
	$(LUAC_FILES)

EXTRA_DIST = \
	peas-lua-compile.lua		\
	$(loader_resources_c_deps)

