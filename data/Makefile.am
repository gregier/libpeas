SUBDIRS=icons

if ENABLE_GLADE_CATALOG
SUBDIRS+=glade
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libpeas-2.0.pc

if ENABLE_GTK
pkgconfig_DATA += libpeas-gtk-2.0.pc
endif

EXTRA_DIST = libpeas-2.0.pc.in libpeas-gtk-2.0.pc.in
