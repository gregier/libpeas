/*
 * peas-lua-utils.c
 * This file is part of libpeas
 *
 * Copyright (C) 2014-2015 - Garrett Regier
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

#include "peas-lua-utils.h"

#include <gio/gio.h>

#include <lauxlib.h>


gboolean
peas_lua_utils_require (lua_State   *L,
                        const gchar *name)
{
  luaL_checkstack (L, 2, "");

  lua_getglobal (L, "require");
  lua_pushstring (L, name);

  if (lua_pcall (L, 1, 1, 0) != 0)
    {
      g_warning ("Error failed to load Lua module '%s': %s",
                 name, lua_tostring (L, -1));

      /* Pop error */
      lua_pop (L, 1);
      return FALSE;
    }

  if (!lua_istable (L, -1))
    {
      g_warning ("Error invalid Lua module for '%s': "
                 "expected table, got: %s",
                 name, lua_tostring (L, -1));

      /* Pop the module's table */
      lua_pop (L, 1);
      return FALSE;
    }

  return TRUE;
}

gboolean
peas_lua_utils_check_version (lua_State *L,
                              guint      req_major,
                              guint      req_minor,
                              guint      req_micro)
{
  const gchar *version_str;
  gchar **version_str_parts;
  gint n_version_parts;
  gint64 *version_parts;
  gint i;
  gboolean success = FALSE;

  lua_getfield (L, -1, "_VERSION");
  version_str = lua_tostring (L, -1);

  version_str_parts = g_strsplit (version_str, ".", 0);

  n_version_parts = g_strv_length (version_str_parts);
  version_parts = g_newa (gint64, n_version_parts);

  for (i = 0; i < n_version_parts; ++i)
    {
      gchar *end;

      version_parts[i] = g_ascii_strtoll (version_str_parts[i], &end, 10);

      if (*end != '\0' ||
          version_parts[i] < 0 ||
          version_parts[i] == G_MAXINT64)
        {
          g_warning ("Invalid version string: %s", version_str);
          goto error;
        }
    }

  if (n_version_parts < 3 ||
      version_parts[0] != req_major ||
      version_parts[1] < req_minor ||
      (version_parts[1] == req_minor && version_parts[2] < req_micro))
    {
      g_warning ("Version mismatch %d.%d.%d is required, found %s",
                 req_major, req_minor, req_micro, version_str);
      goto error;
    }

  success = TRUE;

error:

  /* Pop _VERSION */
  lua_pop (L, 1);

  g_strfreev (version_str_parts);
  return success;
}

static gint
traceback (lua_State *L)
{
  /* Always ignore an error that isn't a string */
  if (!lua_isstring (L, 1))
    return 1;

  lua_getglobal (L, "debug");
  if (!lua_istable (L, -1))
    {
      lua_pop (L, 1);
      return 1;
    }

  lua_getfield (L, -1, "traceback");
  if (!lua_isfunction (L, -1))
    {
      lua_pop (L, 2);
      return 1;
    }

  /* Replace debug with traceback */
  lua_replace (L, -2);

  /* Push the error */
  lua_pushvalue (L, 1);

  /* Skip this function when generating the traceback */
  lua_pushinteger (L, 2);

  /* If we fail we have a new error object... */
  lua_pcall (L, 2, 1, 0);
  return 1;
}

gboolean
peas_lua_utils_call (lua_State *L,
                     guint      n_args,
                     guint      n_results)
{
  gboolean success;

  /* Push the error function */
  lua_pushcfunction (L, traceback);

  /* Move traceback to before the arguments */
  lua_insert (L, -2 - n_args);

  success = lua_pcall (L, n_args, n_results, -2 - n_args) == 0;

  /* Remove traceback */
  lua_remove (L, -1 - (success ? n_results : 1));
  return success;
}

gboolean
peas_lua_utils_load_resource (lua_State   *L,
                              const gchar *name,
                              guint        n_args,
                              guint        n_results)
{
  gchar *resource_path;
  GBytes *lua_resource;
  const gchar *code;
  gsize code_len;
  gchar *lua_filename;

  /* We don't use the byte-compiled Lua source
   * because glib-compile-resources cannot output
   * depends for generated files.
   *
   * There are also concerns that the bytecode is
   * not stable enough between different Lua versions.
   *
   * https://bugzilla.gnome.org/show_bug.cgi?id=673101
   */
  resource_path = g_strconcat ("/org/gnome/libpeas/loaders/lua5.1/",
                               name, NULL);
  lua_resource = g_resources_lookup_data (resource_path,
                                          G_RESOURCE_LOOKUP_FLAGS_NONE,
                                          NULL);
  g_free (resource_path);

  if (lua_resource == NULL)
    {
      g_warning ("Failed to find '%s' resource", name);
      return FALSE;
    }

  code = g_bytes_get_data (lua_resource, &code_len);

  /* Filenames are prefixed with '@' */
  lua_filename = g_strconcat ("@peas-lua-", name, NULL);

  if (luaL_loadbuffer (L, code, code_len, lua_filename) != 0)
    {
      g_warning ("Failed to load '%s' resource: %s",
                 name, lua_tostring (L, -1));

      /* Pop error */
      lua_pop (L, 1);
      g_free (lua_filename);
      g_bytes_unref (lua_resource);
      return FALSE;
    }

  g_free (lua_filename);
  g_bytes_unref (lua_resource);

  if (!peas_lua_utils_call (L, n_args, n_results))
    {
      g_warning ("Failed to run '%s' resource: %s",
                 name, lua_tostring (L, -1));

      /* Pop error */
      lua_pop (L, 1);
      return FALSE;
    }

  return TRUE;
}
