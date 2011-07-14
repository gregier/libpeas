# Makefile to build the minizip library files
#
# All other files in this directory (and subdirectories) are copyrighted
# by their respective owner(s).

INCLUDES = -g3 -O0 $(MINIZIP_CFLAGS)

noinst_LTLIBRARIES = libminizip.la

libminizip_la_LDFLAGS = \
	-export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

libminizip_la_LIBADD = $(MINIZIP_LIBS)

libminizip_la_SOURCES = ioapi.c	unzip.c	zip.c
