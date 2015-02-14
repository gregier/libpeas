LUA_FILES =				\
	peas-lua-internal.lua		\
	peas-lua-strict.lua

LUAC_FILES = $(addsuffix c,$(LUA_FILES))

%.luac: %.lua
	$(AM_V_GEN) $(LUA51_BIN) $(srcdir)/peas-lua-compile.lua $< $@

all-local: $(LUAC_FILES)

CLEANFILES = \
	$(LUAC_FILES)

EXTRA_DIST = \
	peas-lua-compile.lua	\
	peas-lua.gresource.xml	\
	$(LUA_FILES)

