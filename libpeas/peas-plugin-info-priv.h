/*
 * peas-plugin-info-priv.h
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

#ifndef __PEAS_PLUGIN_INFO_PRIV_H__
#define __PEAS_PLUGIN_INFO_PRIV_H__

#include "peas-plugin-info.h"

struct _PeasPluginInfo {
  /*< private >*/
  gint refcount;

  gchar *file;
  gchar *module_dir;
  gchar *data_dir;

  gchar *module_name;
  gchar *loader;
  gchar **dependencies;

  gchar *name;
  gchar *desc;
  gchar *icon_name;
  gchar **authors;
  gchar *copyright;
  gchar *website;
  gchar *version;
  guint iage;
  gboolean visible;
  GHashTable *keys;

  gint active : 1;
  /* A plugin is unavailable if it is not possible to activate it
     due to an error loading the plugin module (e.g. for Python plugins
     when the interpreter has not been correctly initializated) */
  gint available : 1;
};

PeasPluginInfo *_peas_plugin_info_new   (const gchar    *filename,
                                         const gchar    *app_name,
                                         const gchar    *module_dir,
                                         const gchar    *data_dir);
PeasPluginInfo *_peas_plugin_info_ref   (PeasPluginInfo *info);
void            _peas_plugin_info_unref (PeasPluginInfo *info);


#endif /* __PEAS_PLUGIN_INFO_PRIV_H__ */
