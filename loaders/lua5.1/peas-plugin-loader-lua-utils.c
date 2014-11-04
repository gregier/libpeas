/*
 * peas-plugin-loader-lua-utils.c
 * This file is part of libpeas
 *
 * Copyright (C) 2014 - Garrett Regier
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

#include "peas-plugin-loader-lua-utils.h"

#include <string.h>

#include <lauxlib.h>
#include <lualib.h>


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
  gint *version_parts;
  gint i;
  gboolean success = FALSE;

  lua_getfield (L, -1, "_VERSION");
  version_str = lua_tostring (L, -1);

  version_str_parts = g_strsplit (version_str, ".", 0);

  n_version_parts = g_strv_length (version_str_parts);
  version_parts = g_newa (gint, n_version_parts);

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
