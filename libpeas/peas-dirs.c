/*
 * peas-dirs.c
 * This file is part of libpeas
 *
 * Copyright (C) 2008 Ignacio Casal Quinteiro
 *
 * libpeas is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libpeas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "peas-dirs.h"

#ifdef OS_OSX

#import <Cocoa/Cocoa.h>

static gchar *
dirs_os_x_get_bundle_resource_dir (void)
{
  NSAutoreleasePool *pool;
  gchar *str = NULL;
  NSString *path;

  pool = [[NSAutoreleasePool alloc] init];

  if ([[NSBundle mainBundle] bundleIdentifier] == nil)
    {
      [pool release];
      return NULL;
    }

  path = [[NSBundle mainBundle] resourcePath];

  if (!path)
    {
      [pool release];
      return NULL;
    }

  str = g_strdup ([path UTF8String]);
  [pool release];
  return str;
}

static gchar *
dirs_os_x_get_resource_dir (const gchar *subdir,
                            const gchar *default_dir)
{
  gchar *res_dir;
  gchar *ret;

  res_dir = dirs_os_x_get_bundle_resource_dir ();

  if (res_dir == NULL)
    {
      ret = g_build_filename (default_dir, "libpeas-1.0", NULL);
    }
  else
    {
      ret = g_build_filename (res_dir, subdir, "libpeas-1.0", NULL);
      g_free (res_dir);
    }

  return ret;
}

static gchar *
dirs_os_x_get_data_dir (void)
{
  return dirs_os_x_get_resource_dir ("share", DATADIR);
}

static gchar *
dirs_os_x_get_lib_dir (void)
{
  return dirs_os_x_get_resource_dir ("lib", LIBDIR);
}

static gchar *
dirs_os_x_get_locale_dir (void)
{
  gchar *res_dir;
  gchar *ret;

  res_dir = dirs_os_x_get_bundle_resource_dir ();

  if (res_dir == NULL)
    {
      ret = g_build_filename (DATADIR, "locale", NULL);
    }
  else
    {
      ret = g_build_filename (res_dir, "share", "locale", NULL);
      g_free (res_dir);
    }

  return ret;
}

#endif

gchar *
peas_dirs_get_data_dir (void)
{
  gchar *data_dir;

#ifdef G_OS_WIN32
  gchar *win32_dir;

  win32_dir = g_win32_get_package_installation_directory_of_module (NULL);

  data_dir = g_build_filename (win32_dir, "share", "libpeas-1.0", NULL);
  g_free (win32_dir);
#elif defined (OS_OSX)
  data_dir = dirs_os_x_get_data_dir ();
#else
  data_dir = g_build_filename (DATADIR, "libpeas-1.0", NULL);
#endif

  return data_dir;
}

gchar *
peas_dirs_get_lib_dir (void)
{
  gchar *lib_dir;

#ifdef G_OS_WIN32
  gchar *win32_dir;

  win32_dir = g_win32_get_package_installation_directory_of_module (NULL);

  lib_dir = g_build_filename (win32_dir, "lib", "libpeas-1.0", NULL);
  g_free (win32_dir);
#elif defined (OS_OSX)
  lib_dir = dirs_os_x_get_lib_dir ();
#else
  lib_dir = g_build_filename (LIBDIR, "libpeas-1.0", NULL);
#endif

  return lib_dir;
}

gchar *
peas_dirs_get_plugin_loader_dir (const gchar *loader_name)
{
  const gchar *env_var;
  gchar *lib_dir;
  gchar *loader_dir;

  env_var = g_getenv ("PEAS_PLUGIN_LOADERS_DIR");
  if (env_var != NULL)
    return g_build_filename (env_var, loader_name, NULL);

  lib_dir = peas_dirs_get_lib_dir ();
  loader_dir = g_build_filename (lib_dir, "loaders", NULL);

  g_free (lib_dir);

  return loader_dir;
}

gchar *
peas_dirs_get_locale_dir (void)
{
  gchar *locale_dir;

#ifdef G_OS_WIN32
  gchar *win32_dir;

  win32_dir = g_win32_get_package_installation_directory_of_module (NULL);

  locale_dir = g_build_filename (win32_dir, "share", "locale", NULL);

  g_free (win32_dir);
#elif defined (OS_OSX)
  locale_dir = dirs_os_x_get_locale_dir ();
#else
  locale_dir = g_build_filename (DATADIR, "locale", NULL);
#endif

  return locale_dir;
}

