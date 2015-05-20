/*
 * peas-gtk-autocleanups.h
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


#ifndef __PEAS_GTK_AUTOCLEANUPS_H__
#define __PEAS_GTK_AUTOCLEANUPS_H__

#include "peas-gtk-configurable.h"
#include "peas-gtk-plugin-manager.h"
#include "peas-gtk-plugin-manager-view.h"

G_BEGIN_DECLS

#ifndef __GI_SCANNER__
#if GLIB_CHECK_VERSION (2, 44, 0)

G_DEFINE_AUTOPTR_CLEANUP_FUNC (PeasGtkConfigurable, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (PeasGtkPluginManager, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (PeasGtkPluginManagerView, g_object_unref)

#endif /* GLIB_CHECK_VERSION (2, 44, 0) */
#endif /* __GI_SCANNER__ */

G_END_DECLS

#endif /* __PEAS_GTK_AUTOCLEANUPS_H__ */
