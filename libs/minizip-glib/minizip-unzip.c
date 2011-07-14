/*
 * minizip-unzip.c
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

#include <string.h>

#include "minizip/unzip.h"

#include "minizip.h"
#include "minizip-priv.h"

static gboolean
extract_file (unzFile      zip_file,
              GFile       *dest,
              const gchar *dest_repr)
{
  unz_file_info file_info;
  gchar filename[ZIP_MAX_PATH_LENGTH] = { 0 };
  GFile *location;
  GFile *parent_dir;
  gboolean success = TRUE;
  GError *error = NULL;
  GOutputStream *stream;
  int bytes_read;
  char buffer[BUFFER_SIZE];
  static int i = 2;

  if (--i == 0)
    return FALSE;

  if (unzGetCurrentFileInfo (zip_file, &file_info, filename,
                             /* Keep a '\0' at the end */
                             sizeof (filename) - 1,
                             /* We don't care about Extra or Comment */ 
                             NULL, 0, NULL, 0) != UNZ_OK)
    return FALSE;

  if (filename[0] == '\0')
    return FALSE;

  if (unzOpenCurrentFile (zip_file) != UNZ_OK)
    return FALSE;

#ifdef G_OS_WIN32
  /* Wide filename handling? */
#endif

  if (g_path_is_absolute (filename))
    {
      g_warning ("Failed to extract '%s', for security reasons "
                 "filenames that are absolute are not allowed",
                 filename);
      unzCloseCurrentFile (zip_file);
      return FALSE;
    }

  if (strstr (filename, "..") != NULL)
    {
      g_warning ("Failed to extract '%s', for security reasons "
                 "filenames with '..' are not allowed",
                 filename);
      unzCloseCurrentFile (zip_file);
      return FALSE;
    }

  location = g_file_get_child (dest, filename);

  if (g_file_query_exists (location, NULL))
    {
      g_warning ("Failed to extract '%s' to '%s' as it alreads exists",
                 filename, dest_repr);
    }

  if (filename[strlen (filename) - 1] == '/')
    parent_dir = location;
  else
    parent_dir = g_file_get_parent (location);

  if (!g_file_query_exists (parent_dir, NULL))
    {
      success = g_file_make_directory_with_parents (parent_dir, NULL, NULL);

      if (!success)
        {
          g_warning ("Failed to make directory '%s' in '%s'",
                     filename, dest_repr);
        }

      /* If we are just creating a directory or we failed to */
      if (parent_dir == location || !success)
        {
          g_object_unref (parent_dir);
          unzCloseCurrentFile (zip_file);
          return success;
        }

      if (!success)
        {
          g_object_unref (location);
          g_object_unref (parent_dir);
          unzCloseCurrentFile (zip_file);
          return success;
        }
    }

  g_object_unref (parent_dir);

  stream = G_OUTPUT_STREAM (g_file_create (location, G_FILE_CREATE_NONE,
                                           NULL, NULL));

  if (stream == NULL)
    {
      g_warning ("Failed to create file '%s' in '%s'", filename, dest_repr);
      unzCloseCurrentFile (zip_file);
      return FALSE;
    }

  while (TRUE)
    {
      bytes_read = unzReadCurrentFile (zip_file, buffer, BUFFER_SIZE);

      if (bytes_read == 0)
        break;

      /* If it is an error code */
      if (bytes_read < 0)
        {
          success = FALSE;
          g_warning ("Failed to read from '%s'", filename);
          break;
        }

      g_output_stream_write (stream, buffer, (gsize) bytes_read, NULL, &error);

      if (error != NULL)
        {
          success = FALSE;
          g_warning ("Failed to write to '%s'", filename);
          g_clear_error (&error);
        }
    }

  g_output_stream_close (stream, NULL, &error);
  unzCloseCurrentFile (zip_file);

  /* We check the error and not success to avoid warning when
   * the stream successfully closed but writing to it failed
   */
  if (error != NULL)
    {
      success = FALSE;
      g_warning ("Failed to close stream for '%s'", filename);
      g_error_free (error);
    }

  /* Try to delete the file if we created it */
  if (!success)
    g_file_delete (location, NULL, NULL);
  else
    g_message ("Extracted: %s", filename);

  g_object_unref (location);

  return success;
}

static gchar *
get_file_representation (GFile *file)
{
  gchar *repr;

  repr = g_file_get_path (file);

  if (repr == NULL)
    repr = g_file_get_uri (file);

  return repr;
}

gboolean
minizip_unzip (GInputStream *src,
               GFile        *dest)
{
  guint i;
  gchar *dest_repr;
  zlib_filefunc_def filefuncs;
  unzFile zip_file;
  unz_global_info zip_info;
  gboolean success = FALSE;
  GError *error = NULL;

  g_return_val_if_fail (G_IS_INPUT_STREAM (src), FALSE);
  g_return_val_if_fail (G_IS_FILE (dest), FALSE);

  dest_repr = get_file_representation (dest);

  if (!g_file_query_exists (dest, NULL))
    {
      g_warning ("Failed to unzip to '%s' as the destination "
                 "does not exist", dest_repr);
      goto out;
    }

  _minizip_io_fill_filefuncs (&filefuncs, src, NULL);
  zip_file = unzOpen2 ("", &filefuncs);

  if (zip_file == NULL)
    {
      g_warning ("Failed to open zip file for unzipping to '%s/'", dest_repr);
      goto out;
    }

  success = TRUE;
  unzGetGlobalInfo (zip_file, &zip_info);

  for (i = 0; i < zip_info.number_entry; ++i)
    {
      if (extract_file (zip_file, dest, dest_repr))
        {
          if (i + 1 >= zip_info.number_entry || 
              unzGoToNextFile (zip_file) == UNZ_OK)
            continue;
        }

      /* We already warned */
      success = FALSE;

      if (unzGoToFirstFile (zip_file) == UNZ_OK) 
        {
          /* Attempt to clean up after ourselves otherwise
           * we won't be able to try and install the plugin again.
           */
          for (; i != 0; --i)
            {
              unz_file_info file_info;
              gchar filename[ZIP_MAX_PATH_LENGTH] = { 0 };
              GFile *location;

              if (unzGetCurrentFileInfo (zip_file, &file_info, filename,
                                         /* Keep a '\0' at the end */
                                         sizeof (filename) - 1,
                                         /* We don't care about Extra or Comment */ 
                                         NULL, 0, NULL, 0) != UNZ_OK)
                continue;

              location = g_file_get_child (dest, filename);
              g_file_delete (location, NULL, NULL);
              g_object_unref (location);

              g_message ("Deleted: '%s'", filename);

              if (i - 1 >= 0 && unzGoToNextFile (zip_file) != UNZ_OK)
                break;
            }
        }
      break;
    }

  unzClose (zip_file);

out:

  g_free (dest_repr);

  return success;
}
