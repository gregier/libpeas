LUAC_FILES =				\
	peas-lua-internal.luac		\
	peas-lua-strict.luac


%.luac: %.lua
	$(AM_V_GEN) $(LUA51_BIN) $(srcdir)/peas-lua-compile.lua $< $@

all-local: $(LUAC_FILES)

CLEANFILES = \
	$(LUAC_FILES)

