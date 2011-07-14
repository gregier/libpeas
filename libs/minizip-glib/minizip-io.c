/*
 * minizip-io.c
 * This file is part of libminizip-glib
 *
 * Copyright (C) 2011 - Garrett Regier
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "minizip-priv.h"

typedef struct _MinizipIO MinizipIO;

struct _MinizipIO {
  GInputStream *input_stream;
  GOutputStream *output_stream;

  guint error : 1;
};

/* GFileIOStream uses the output stream */
static GSeekable *
minizip_io_data_get_seekable (void *opaque)
{
  MinizipIO *io = opaque;

  if (io->output_stream != NULL)
    return G_SEEKABLE (io->output_stream);

  return G_SEEKABLE (io->input_stream);
}

void *
minizip_io_open_file_func (void        *opaque,
                           const gchar *dest_uri,
                           int          mode)
{
  MinizipIO *io = opaque;

  g_debug (G_STRFUNC);

  if ((mode & ZLIB_FILEFUNC_MODE_READ))
    g_return_val_if_fail (io->input_stream != NULL, NULL);

  if ((mode & ZLIB_FILEFUNC_MODE_WRITE))
    g_return_val_if_fail (io->output_stream != NULL, NULL);

  return io;
}

static unsigned long
minizip_io_read_file_func (void          *opaque,
                           void          *unused,
                           void          *buffer,
                           unsigned long  size)
{
  MinizipIO *io = opaque;
  gssize bytes_read;

  bytes_read = g_input_stream_read (io->input_stream, buffer,
                                    (gsize) size, NULL, NULL);

  if (bytes_read == -1)
    io->error = TRUE;

  return (unsigned long) bytes_read;
}

static unsigned long
minizip_io_write_file_func (void          *opaque,
                            void          *unused,
                            const void    *buffer,
                            unsigned long  size)
{
  MinizipIO *io = opaque;
  gssize bytes_written;

  bytes_written = g_output_stream_write (io->output_stream, buffer,
                                         (gsize) size, NULL, NULL);

  if (bytes_written == -1)
    io->error = TRUE;

  return (unsigned long) bytes_written;
}

static long
minizip_io_tell_file_func (void *opaque,
                           void *unused)
{
  return (long) g_seekable_tell (minizip_io_data_get_seekable (opaque));
}

static long
minizip_io_seek_file_func (void          *opaque,
                           void          *unused,
                           unsigned long  offset,
                           int            origin)
{
  MinizipIO *io = opaque;
  GSeekable *seekable;
  GSeekType seek_type;

  seekable = minizip_io_data_get_seekable (opaque);

  switch (origin)
    {
    case SEEK_CUR:
      seek_type = G_SEEK_CUR;
      break;
    case SEEK_SET:
      seek_type = G_SEEK_SET;
      break;
    case SEEK_END:
      seek_type = G_SEEK_END;
      break;
    default:
      io->error = TRUE;
      return -1;
    }

  if (!g_seekable_seek (seekable, offset, seek_type, NULL, NULL))
    {
      io->error = TRUE;
      return -1;
    }

  return 0;
}

static int
minizip_io_close_file_func (void *opaque,
                            void *unused)
{
  MinizipIO *io = opaque;
  gboolean success = TRUE;

  if (io->output_stream != NULL)
    {
      success = g_output_stream_close (io->output_stream, NULL, NULL);
      g_object_unref (io->output_stream);
      io->output_stream = NULL;
    }

  if (io->input_stream != NULL)
    {
      success &= g_input_stream_close (io->input_stream, NULL, NULL);
      g_object_unref (io->input_stream);
      io->input_stream = NULL;
    }

  g_free (io);

  return success;
}

static int
minizip_io_error_file_func (void *opaque,
                            void *unused)
{
  MinizipIO *io = opaque;

  return io->error;
}

void
_minizip_io_fill_filefuncs (zlib_filefunc_def *filefuncs,
                            GInputStream      *input_stream,
                            GOutputStream     *output_stream)
{
  MinizipIO *io;

  g_return_if_fail (filefuncs != NULL);
  g_return_if_fail (input_stream == NULL ||
                    (G_IS_INPUT_STREAM (input_stream) &&
                     !g_input_stream_is_closed (input_stream) &&
                     G_IS_SEEKABLE (input_stream)));
  g_return_if_fail (output_stream == NULL ||
                    (G_IS_OUTPUT_STREAM (output_stream) &&
                     !g_output_stream_is_closed (output_stream) &&
                     G_IS_SEEKABLE (output_stream)));

  filefuncs->zopen_file = minizip_io_open_file_func;
  filefuncs->zread_file = minizip_io_read_file_func;
  filefuncs->zwrite_file = minizip_io_write_file_func;
  filefuncs->ztell_file = minizip_io_tell_file_func;
  filefuncs->zseek_file = minizip_io_seek_file_func;
  filefuncs->zclose_file = minizip_io_close_file_func;
  filefuncs->zerror_file = minizip_io_error_file_func;
  filefuncs->opaque = g_new (MinizipIO, 1);

  io = filefuncs->opaque;
  io->error = FALSE;

  if (input_stream == NULL)
    io->input_stream = NULL;
  else
    io->input_stream = g_object_ref (input_stream);

  if (output_stream == NULL)
    io->output_stream = NULL;
  else
    io->output_stream = g_object_ref (output_stream);
}
