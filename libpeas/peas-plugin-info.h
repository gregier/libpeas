/*
 * peas-plugins-info.h
 * This file is part of libpeas
 *
 * Copyright (C) 2002-2005 - Paolo Maggi
 * Copyright (C) 2007 - Steve Frécinaux
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

#ifndef __PEAS_PLUGIN_INFO_H__
#define __PEAS_PLUGIN_INFO_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define PEAS_TYPE_PLUGIN_INFO   (peas_plugin_info_get_type ())
#define PEAS_PLUGIN_INFO(obj)   ((PeasPluginInfo *) (obj))

/**
 * PeasPluginInfo:
 *
 * Boxed type for the information relative to a plugin.
 */
typedef struct _PeasPluginInfo PeasPluginInfo;

GType         peas_plugin_info_get_type         (void) G_GNUC_CONST;

gboolean      peas_plugin_info_is_active        (PeasPluginInfo *info);
gboolean      peas_plugin_info_is_available     (PeasPluginInfo *info);
gboolean      peas_plugin_info_is_configurable  (PeasPluginInfo *info);

const gchar  *peas_plugin_info_get_module_name  (PeasPluginInfo *info);
const gchar  *peas_plugin_info_get_module_dir   (PeasPluginInfo *info);
const gchar  *peas_plugin_info_get_data_dir     (PeasPluginInfo *info);

const gchar  *peas_plugin_info_get_name         (PeasPluginInfo *info);
const gchar  *peas_plugin_info_get_description  (PeasPluginInfo *info);
const gchar  *peas_plugin_info_get_icon_name    (PeasPluginInfo *info);
const gchar **peas_plugin_info_get_authors      (PeasPluginInfo *info);
const gchar  *peas_plugin_info_get_website      (PeasPluginInfo *info);
const gchar  *peas_plugin_info_get_copyright    (PeasPluginInfo *info);
const gchar  *peas_plugin_info_get_version      (PeasPluginInfo *info);

G_END_DECLS

#endif /* __PEAS_PLUGIN_INFO_H__ */
