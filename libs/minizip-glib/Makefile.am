INCLUDES = $(MINIZIP_GLIB_CFLAGS)

noinst_LTLIBRARIES = libminizip-glib.la

libminizip_glib_la_LDFLAGS = \
	-export-dynamic -no-undefined -export-symbols-regex "^[^_].*"

libminizip_glib_la_LIBADD = \
        ../minizip/libminizip.la	\
        $(MINIZIP_GLIB_LIBS)
        

libminizip_glib_la_SOURCES = minizip-io.c minizip-unzip.c minizip-zip.c

bin_PROGRAMS = minizip-glib-unzipper

INCLUDES += -I$(top_srcdir)/libs/

minizip_glib_unzipper_SOURCES = minizip-glib-unzipper.c

minizip_glib_unzipper_LDADD = libminizip-glib.la
minizip_glib_unzipper_CFLAGS =
