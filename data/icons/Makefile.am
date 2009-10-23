icons16x16dir = $(pkgdatadir)/icons/hicolor/16x16/actions
icons22x22dir = $(pkgdatadir)/icons/hicolor/22x22/actions
icons32x32dir = $(pkgdatadir)/icons/hicolor/32x32/actions
scalableiconsdir = $(pkgdatadir)/icons/hicolor/scalable/actions

icons16x16_DATA = 16x16/libgpe-plugin.png
icons22x22_DATA = 22x22/libgpe-plugin.png
icons32x32_DATA = 32x32/libgpe-plugin.png
scalableicons_DATA = scalable/libgpe-plugin.svg


noinst_DATA = \
	16x16/libgpe-plugin.svg
	22x22/libgpe-plugin.svg
	32x32/libgpe-plugin.svg

EXTRA_DIST = $(noinst_DATA)


gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor
install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi
