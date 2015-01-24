/*
 * peas-lua-utils.h
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

#ifndef __PEAS_LUA_UTILS_H__
#define __PEAS_LUA_UTILS_H__

#include <glib.h>
#include <lua.h>

G_BEGIN_DECLS


gboolean peas_lua_utils_require       (lua_State   *L,
                                       const gchar *name);

gboolean peas_lua_utils_check_version (lua_State   *L,
                                       guint        req_major,
                                       guint        req_minor,
                                       guint        req_micro);

gboolean peas_lua_utils_call          (lua_State   *L,
                                       guint        n_args,
                                       guint        n_results);

gboolean peas_lua_utils_load_resource (lua_State   *L,
                                       const gchar *name,
                                       guint        n_args,
                                       guint        n_results);

G_END_DECLS

#endif /* __PEAS_LUA_UTILS_H__ */
