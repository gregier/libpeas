/*
 * peas-lua-internal.h
 * This file is part of libpeas
 *
 * Copyright (C) 2015 - Garrett Regier
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

#ifndef __PEAS_LUA_INTERNAL_H__
#define __PEAS_LUA_INTERNAL_H__

#include <glib.h>
#include <lua.h>

G_BEGIN_DECLS

gboolean  peas_lua_internal_setup    (lua_State   *L);
void      peas_lua_internal_shutdown (lua_State   *L);

gboolean  peas_lua_internal_call     (lua_State   *L,
                                      const gchar *name,
                                      guint        n_args,
                                      gint         return_type);

G_END_DECLS

#endif /* __PEAS_LUA_INTERNAL_H__ */
