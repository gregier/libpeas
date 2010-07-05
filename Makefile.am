## Process this file with automake to produce Makefile.in
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc --enable-introspection

SUBDIRS = libpeas loaders

if ENABLE_LIBPEASUI
SUBDIRS += libpeasui peas-demo
endif

SUBDIRS += data po docs
