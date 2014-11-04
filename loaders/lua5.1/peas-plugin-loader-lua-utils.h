/*
 * peas-plugin-loader-lua-utils.h
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

#ifndef __PEAS_PLUGIN_LOADER_LUA_UTILS_H__
#define __PEAS_PLUGIN_LOADER_LUA_UTILS_H__

#include <glib.h>
#include <lua.h>

G_BEGIN_DECLS


gboolean peas_lua_utils_require       (lua_State   *L,
                                       const gchar *name);

gboolean peas_lua_utils_check_version (lua_State   *L,
                                       guint        req_major,
                                       guint        req_minor,
                                       guint        req_micro);

G_END_DECLS

#endif /* __PEAS_PLUGIN_LOADER_LUA_UTILS_H__ */

