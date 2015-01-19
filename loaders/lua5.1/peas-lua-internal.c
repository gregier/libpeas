/*
 * peas-lua-internal.c
 * This file is part of libpeas
 *
 * Copyright (C) 2015 - Garrett Regier
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "peas-lua-internal.h"

#include <gio/gio.h>

#include <lauxlib.h>
#include <lualib.h>

#include "peas-lua-utils.h"


static gpointer hooks_key = NULL;
static gpointer failed_err_key = NULL;


static int
failed_fn (lua_State *L)
{
  gchar *msg;

  /* The first parameter is the Hooks table instance */
  luaL_checktype (L, 1, LUA_TTABLE);

  /* The tracebacks have a trailing newline */
  msg = g_strchomp (g_strdup (luaL_checkstring (L, 2)));

  g_warning ("%s", msg);

  /* peas_lua_internal_call() knows to check for this value */
  lua_pushlightuserdata (L, &failed_err_key);

  g_free (msg);
  return lua_error (L);
}

gboolean
peas_lua_internal_setup (lua_State *L)
{
  GBytes *internal_lua;
  const gchar *code;
  gsize code_len;

  /* We don't use the byte-compiled Lua source
   * because glib-compile-resources cannot output
   * depends for generated files.
   *
   * There are also concerns that the bytecode is
   * not stable enough between different Lua versions.
   *
   * https://bugzilla.gnome.org/show_bug.cgi?id=673101
   */
  internal_lua = g_resources_lookup_data ("/org/gnome/libpeas/loaders/"
                                          "lua5.1/internal.lua",
                                          G_RESOURCE_LOOKUP_FLAGS_NONE,
                                          NULL);
  g_return_val_if_fail (internal_lua != NULL, FALSE);

  code = g_bytes_get_data (internal_lua, &code_len);

  /* Filenames are prefixed with '@' */
  if (luaL_loadbuffer (L, code, code_len, "@peas-lua-internal.lua") != 0)
    {
      g_warning ("Failed to load internal Lua code: %s",
                 lua_tostring (L, -1));

      /* Pop error */
      lua_pop (L, 1);
      g_bytes_unref (internal_lua);
      return FALSE;
    }

  g_bytes_unref (internal_lua);

  if (!peas_lua_utils_call (L, 0, 1))
    {
      g_warning ("Failed to run internal Lua code: %s",
                 lua_tostring (L, -1));

      /* Pop error */
      lua_pop (L, 1);
      return FALSE;
    }

  if (!lua_istable (L, -1))
    {
      g_warning ("Invalid result from internal Lua code: %s",
                 lua_tostring (L, -1));

      /* Pop result */
      lua_pop (L, 1);
      return FALSE;
    }

  /* Set Hooks.failed to failed_fn */
  lua_pushcfunction (L, failed_fn);
  lua_setfield (L, -2, "failed");

  /* Set registry[&hooks_key] = hooks */
  lua_pushlightuserdata (L, &hooks_key);
  lua_pushvalue (L, -2);
  lua_rawset (L, LUA_REGISTRYINDEX);

  /* Pop hooks */
  lua_pop (L, -1);
  return TRUE;
}

void
peas_lua_internal_shutdown (lua_State *L)
{
  lua_pushlightuserdata (L, &hooks_key);
  lua_pushnil (L);
  lua_rawset (L, LUA_REGISTRYINDEX);
}

gboolean
peas_lua_internal_call (lua_State   *L,
                        const gchar *name,
                        guint        n_args,
                        gint         return_type)
{
  /* Get the Hooks table */
  lua_pushlightuserdata (L, &hooks_key);
  lua_rawget (L, LUA_REGISTRYINDEX);

  /* Get the method */
  lua_getfield (L, -1, name);

  /* Swap the method and the table */
  lua_insert (L, -2);

  if (n_args > 0)
    {
      /* Before: [args..., method, table]
       * After:  [method, table, args...]
       */
      lua_insert (L, -n_args - 2);
      lua_insert (L, -n_args - 2);
    }

  if (!peas_lua_utils_call (L, 1 + n_args, 1))
    {
      /* Raised by failed_fn() to prevent printing the error */
      if (!lua_isuserdata (L, -1) ||
          lua_touserdata (L, -1) != &failed_err_key)
        {
          g_warning ("Failed to run internal Lua hook '%s':\n%s",
                     name, lua_tostring (L, -1));
        }

      /* Pop the error */
      lua_pop (L, 1);
      return FALSE;
    }

  if (lua_type (L, -1) != return_type)
    {
      /* Don't warn for a nil result */
      if (lua_type (L, -1) != LUA_TNIL)
        {
          g_warning ("Invalid return value for internal Lua hook '%s': "
                     "expected %s, got: %s (%s)", name,
                     lua_typename (L, return_type),
                     lua_typename (L, lua_type (L, -1)),
                     lua_tostring (L, -1));
        }

      /* Pop result */
      lua_pop (L, 1);
      return FALSE;
    }

  /* Pop the result if nil */
  if (return_type == LUA_TNIL)
    lua_pop (L, 1);

  return TRUE;
}
