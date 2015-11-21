/*
 * peas-plugin-info-priv.h
 * This file is part of libpeas
 *
 * Copyright (C) 2002-2005 - Paolo Maggi
 * Copyright (C) 2007 - Steve Frécinaux
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

#ifndef __PEAS_PLUGIN_INFO_PRIV_H__
#define __PEAS_PLUGIN_INFO_PRIV_H__

#include "peas-plugin-info.h"

struct _PeasPluginInfo {
  /*< private >*/
  gint refcount;

  /* Used and managed by PeasPluginLoader */
  gpointer loader_data;

  gchar *filename;
  gchar *module_dir;
  gchar *data_dir;

  gint loader_id;
  gchar *embedded;
  gchar *module_name;
  gchar **dependencies;

  gchar *name;
  gchar *desc;
  gchar *icon_name;
  gchar **authors;
  gchar *copyright;
  gchar *website;
  gchar *version;
  gchar *help_uri;

  GHashTable *external_data;

  GSettingsSchemaSource *schema_source;

  GError *error;

  guint loaded : 1;
  /* A plugin is unavailable if it is not possible to load it
     due to an error loading the plugin module (e.g. for Python plugins
     when the interpreter has not been correctly initializated) */
  guint available : 1;

  guint builtin : 1;
  guint hidden : 1;
};

PeasPluginInfo *_peas_plugin_info_new   (const gchar    *filename,
                                         const gchar    *module_dir,
                                         const gchar    *data_dir);
PeasPluginInfo *_peas_plugin_info_ref   (PeasPluginInfo *info);
void            _peas_plugin_info_unref (PeasPluginInfo *info);


#endif /* __PEAS_PLUGIN_INFO_PRIV_H__ */
