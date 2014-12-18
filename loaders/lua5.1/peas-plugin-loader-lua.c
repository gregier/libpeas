/*
 * peas-plugin-loader-lua.c
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

#include "peas-plugin-loader-lua.h"

#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "peas-plugin-loader-lua-utils.h"


typedef void (* LgiLockFunc) (gpointer lgi_lock);


struct _PeasPluginLoaderLuaPrivate {
  lua_State *L;

  gpointer lgi_lock;
  LgiLockFunc lgi_enter_func;
  LgiLockFunc lgi_leave_func;
};

G_DEFINE_TYPE (PeasPluginLoaderLua, peas_plugin_loader_lua, PEAS_TYPE_PLUGIN_LOADER)

static
G_DEFINE_QUARK (peas-extension-type, extension_type)

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_PLUGIN_LOADER,
                                              PEAS_TYPE_PLUGIN_LOADER_LUA);
}

static gboolean
_lua_add_package_path (lua_State   *L,
                       const gchar *package_path)
{
  luaL_checkstack (L, 3, "");

  lua_getglobal (L, "package");
  lua_getfield (L, -1, "path");

  if (!lua_isstring (L, -1))
    {
      g_warning ("Invalid Lua package.path: %s", lua_tostring (L, -1));

      /* Pop path and package */
      lua_pop (L, 2);
      return FALSE;
    }

  /* ";package_path/?.lua;package_path/?/init.lua" */
  lua_pushliteral (L, ";");
  lua_pushstring (L, package_path);
  lua_pushliteral (L, G_DIR_SEPARATOR_S "?.lua;");
  lua_pushstring (L, package_path);
  lua_pushliteral (L, G_DIR_SEPARATOR_S "?" G_DIR_SEPARATOR_S "init.lua");
  lua_concat (L, 5);

  if (strstr (lua_tostring (L, -2), lua_tostring (L, -1)) != NULL)
    {
      /* Pop new path and path */
      lua_pop (L, 2);
    }
  else
    {
      /* Update package.path */
      lua_concat (L, 2);
      lua_setfield (L, -2, "path");
    }

  /* Pop package */
  lua_pop (L, 1);
  return TRUE;
}

static gboolean
_lua_pushinstance (lua_State   *L,
                   const gchar *namespace_,
                   const gchar *name,
                   GType        gtype,
                   gpointer     instance)
{
  luaL_checkstack (L, 3, "");

  if (!peas_lua_utils_require (L, "lgi"))
    return FALSE;

  lua_getfield (L, -1, namespace_);
  lua_getfield (L, -1, name);

  /* Remove the namespace and lgi's module table */
  lua_replace (L, -2);
  lua_replace (L, -2);

  lua_pushlightuserdata (L, instance);
  lua_pushboolean (L, FALSE);

  /* new(addr[, already_own[, no_sink]]) */
  if (lua_pcall (L, 2, 1, 0) != 0)
    {
      g_warning ("Failed to create Lua object of type '%s': %s",
                 g_type_name (gtype), lua_tostring (L, -1));

      /* Pop the error */
      lua_pop (L, 1);
      return FALSE;
    }

  /* Check that the Lua object was created correctly */
  lua_getfield (L, -1, "_native");
  g_assert (lua_islightuserdata (L, -1));
  g_assert (lua_touserdata (L, -1) == instance);
  lua_pop (L, 1);

  return TRUE;
}

static GType
_lua_get_gtype (lua_State *L,
                int        index)
{
  GType gtype = G_TYPE_INVALID;

  luaL_checkstack (L, 1, "");

  lua_getfield (L, index, "_gtype");

  if (lua_type (L, -1) == LUA_TLIGHTUSERDATA)
    gtype = (GType) lua_touserdata (L, -1);

  /* Pop _gtype */
  lua_pop (L, 1);
  return gtype;
}

static GType
_lua_find_extension_type (lua_State      *L,
                          PeasPluginInfo *info,
                          GType           exten_type)
{
  GType found_type = G_TYPE_INVALID;

  luaL_checkstack (L, 3, "");

  /* Get the module's table */
  lua_pushlightuserdata (L, info);
  lua_rawget (L, LUA_REGISTRYINDEX);

  /* Must always have a valid key */
  lua_pushnil (L);
  while (lua_next (L, -2) != 0 && found_type == G_TYPE_INVALID)
    {
      if (lua_istable (L, -1))
        {
          found_type = _lua_get_gtype (L, -1);

          if (found_type != G_TYPE_INVALID &&
              !g_type_is_a (found_type, exten_type))
            {
              found_type = G_TYPE_INVALID;
            }
        }

      /* Pop the value but keep the key for the next iteration */
      lua_pop (L, 1);
    }

  /* Pop the module's table */
  lua_pop (L, 1);
  return found_type;
}

static gboolean
peas_plugin_loader_lua_provides_extension (PeasPluginLoader *loader,
                                           PeasPluginInfo   *info,
                                           GType             exten_type)
{
  PeasPluginLoaderLua *lua_loader = PEAS_PLUGIN_LOADER_LUA (loader);
  lua_State *L = lua_loader->priv->L;
  GType extension_type;

  lua_loader->priv->lgi_enter_func (lua_loader->priv->lgi_lock);
  extension_type = _lua_find_extension_type (L, info, exten_type);
  lua_loader->priv->lgi_leave_func (lua_loader->priv->lgi_lock);

  return extension_type != G_TYPE_INVALID;
}

static PeasExtension *
peas_plugin_loader_lua_create_extension (PeasPluginLoader *loader,
                                         PeasPluginInfo   *info,
                                         GType             exten_type,
                                         guint             n_parameters,
                                         GParameter       *parameters)
{
  PeasPluginLoaderLua *lua_loader = PEAS_PLUGIN_LOADER_LUA (loader);
  lua_State *L = lua_loader->priv->L;
  GType the_type;
  GObject *object = NULL;

  lua_loader->priv->lgi_enter_func (lua_loader->priv->lgi_lock);

  the_type = _lua_find_extension_type (L, info, exten_type);
  if (the_type == G_TYPE_INVALID)
    goto out;

  if (!g_type_is_a (the_type, exten_type))
    {
      g_warn_if_fail (g_type_is_a (the_type, exten_type));
      goto out;
    }

  object = g_object_newv (the_type, n_parameters, parameters);
  if (object == NULL)
    goto out;

  /* We have to remember which interface we are instantiating
   * for the deprecated peas_extension_get_extension_type().
   */
  g_object_set_qdata (object, extension_type_quark (),
                      GSIZE_TO_POINTER (exten_type));

  luaL_checkstack (L, 3, "");

  if (!_lua_pushinstance (L, "GObject", "Object", the_type, object))
    {
      g_clear_object (&object);
      goto out;
    }

  lua_getfield (L, -1, "priv");

  if (!_lua_pushinstance (L, "Peas", "PluginInfo",
                          PEAS_TYPE_PLUGIN_INFO, info))
    {
      g_clear_object (&object);
    }
  else
    {
      /* Set the plugin info as self.priv.plugin_info */
      lua_setfield (L, -2, "plugin_info");
    }

  /* Pop priv and object */
  lua_pop (L, 2);

out:

  lua_loader->priv->lgi_leave_func (lua_loader->priv->lgi_lock);
  return object;
}

static gboolean
peas_plugin_loader_lua_load (PeasPluginLoader *loader,
                             PeasPluginInfo   *info)
{
  PeasPluginLoaderLua *lua_loader = PEAS_PLUGIN_LOADER_LUA (loader);
  lua_State *L = lua_loader->priv->L;
  gboolean success;

  lua_loader->priv->lgi_enter_func (lua_loader->priv->lgi_lock);

  luaL_checkstack (L, 2, "");

  /* Get the module's table */
  lua_pushlightuserdata (L, info);
  lua_rawget (L, LUA_REGISTRYINDEX);

  if (!lua_isnil (L, -1))
    {
      success = lua_istable (L, -1);
    }
  else
    {
      const gchar *module_dir, *module_name;

      module_dir = peas_plugin_info_get_module_dir (info);
      module_name = peas_plugin_info_get_module_name (info);

      /* Must push the key back onto the stack */
      lua_pushlightuserdata (L, info);

      if (!_lua_add_package_path (L, module_dir) ||
          !peas_lua_utils_require (L, module_name))
        {
          /* Push something that isn't a table */
          lua_pushboolean (L, FALSE);
        }

      success = lua_istable (L, -1);
      lua_rawset (L, LUA_REGISTRYINDEX);
    }

  /* Pop the module's table */
  lua_pop (L, 1);

  lua_loader->priv->lgi_leave_func (lua_loader->priv->lgi_lock);
  return success;
}

static void
peas_plugin_loader_lua_unload (PeasPluginLoader *loader,
                               PeasPluginInfo   *info)
{
  /* Lua always keeps a reference anyways */
}

static void
peas_plugin_loader_lua_garbage_collect (PeasPluginLoader *loader)
{
  PeasPluginLoaderLua *lua_loader = PEAS_PLUGIN_LOADER_LUA (loader);

  lua_loader->priv->lgi_enter_func (lua_loader->priv->lgi_lock);
  lua_gc (lua_loader->priv->L, LUA_GCCOLLECT, 0);
  lua_loader->priv->lgi_leave_func (lua_loader->priv->lgi_lock);
}

static int
atpanic_handler (lua_State *L)
{
  G_BREAKPOINT ();
  return 0;
}

static gboolean
peas_plugin_loader_lua_initialize (PeasPluginLoader *loader)
{
  PeasPluginLoaderLua *lua_loader = PEAS_PLUGIN_LOADER_LUA (loader);
  lua_State *L;

  L = luaL_newstate ();
  if (L == NULL)
    {
      g_critical ("Failed to allocate lua_State");
      return FALSE;
    }

  luaL_openlibs (L);

  if (!peas_lua_utils_require (L, "lgi") ||
      !peas_lua_utils_check_version (L,
                                     LGI_MAJOR_VERSION,
                                     LGI_MINOR_VERSION,
                                     LGI_MICRO_VERSION))
    {
      /* Already warned */
      lua_close (L);
      return FALSE;
    }

  lua_getfield (L, -1, "lock");
  lua_loader->priv->lgi_lock = lua_touserdata (L, -1);
  lua_pop (L, 1);

  lua_getfield (L, -1, "enter");
  lua_loader->priv->lgi_enter_func = lua_touserdata (L, -1);
  lua_pop (L, 1);

  lua_getfield (L, -1, "leave");
  lua_loader->priv->lgi_leave_func = lua_touserdata (L, -1);
  lua_pop (L, 1);

  if (lua_loader->priv->lgi_lock == NULL ||
      lua_loader->priv->lgi_enter_func == NULL ||
      lua_loader->priv->lgi_leave_func == NULL)
    {
      g_warning ("Failed to find 'lgi.lock', 'lgi.enter' and 'lgi.leave'");
      lua_close (L);
      return FALSE;
    }

  /* Pop lgi's module table */
  lua_pop (L, 1);

  if (g_getenv ("PEAS_LUA_DEBUG") != NULL)
    {
      lua_atpanic (L, atpanic_handler);
    }

  /* Initially the lock is taken by LGI,
   * release as we are not running Lua code
   */
  lua_loader->priv->lgi_leave_func (lua_loader->priv->lgi_lock);

  lua_loader->priv->L = L;
  return TRUE;
}

static gboolean
peas_plugin_loader_lua_is_global (PeasPluginLoader *loader)
{
  return FALSE;
}

static void
peas_plugin_loader_lua_init (PeasPluginLoaderLua *lua_loader)
{
  lua_loader->priv = G_TYPE_INSTANCE_GET_PRIVATE (lua_loader,
                                                  PEAS_TYPE_PLUGIN_LOADER_LUA,
                                                  PeasPluginLoaderLuaPrivate);
}

static void
peas_plugin_loader_lua_finalize (GObject *object)
{
  PeasPluginLoaderLua *lua_loader = PEAS_PLUGIN_LOADER_LUA (object);

  /* Must take the lock as Lua code will run on lua_close
   * and another thread might be running Lua code already
   */
  lua_loader->priv->lgi_enter_func (lua_loader->priv->lgi_lock);

  g_clear_pointer (&lua_loader->priv->L, (GDestroyNotify) lua_close);

  G_OBJECT_CLASS (peas_plugin_loader_lua_parent_class)->finalize (object);
}

static void
peas_plugin_loader_lua_class_init (PeasPluginLoaderLuaClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PeasPluginLoaderClass *loader_class = PEAS_PLUGIN_LOADER_CLASS (klass);

  object_class->finalize = peas_plugin_loader_lua_finalize;

  loader_class->initialize = peas_plugin_loader_lua_initialize;
  loader_class->is_global = peas_plugin_loader_lua_is_global;
  loader_class->load = peas_plugin_loader_lua_load;
  loader_class->unload = peas_plugin_loader_lua_unload;
  loader_class->create_extension = peas_plugin_loader_lua_create_extension;
  loader_class->provides_extension = peas_plugin_loader_lua_provides_extension;
  loader_class->garbage_collect = peas_plugin_loader_lua_garbage_collect;

  g_type_class_add_private (object_class, sizeof (PeasPluginLoaderLuaPrivate));
}
