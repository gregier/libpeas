/*
 * peas-plugins-info.h
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

#ifndef __PEAS_PLUGIN_INFO_H__
#define __PEAS_PLUGIN_INFO_H__

#include <gio/gio.h>

G_BEGIN_DECLS

#define PEAS_TYPE_PLUGIN_INFO   (peas_plugin_info_get_type ())
#define PEAS_PLUGIN_INFO(obj)   ((PeasPluginInfo *) (obj))

/**
 * PEAS_PLUGIN_INFO_ERROR:
 *
 * Error domain for PeasPluginInfo. Errors in this domain will
 * be from the PeasPluginInfoError enumeration. See #GError for
 * more information on error domains.
 */
#define PEAS_PLUGIN_INFO_ERROR peas_plugin_info_error_quark ()

/**
 * PeasPluginInfoError:
 * @PEAS_PLUGIN_INFO_ERROR_LOADING_FAILED:
 *      The plugin failed to load.
 * @PEAS_PLUGIN_INFO_ERROR_LOADER_NOT_FOUND:
 *      The plugin's loader was not found.
 * @PEAS_PLUGIN_INFO_ERROR_DEP_NOT_FOUND:
 *      A dependency of the plugin was not found.
 * @PEAS_PLUGIN_INFO_ERROR_DEP_LOADING_FAILED:
 *      A dependency of the plugin failed to load.
 *
 * These identify the various errors that can occur while
 * loading a plugin.
 */
typedef enum {
  PEAS_PLUGIN_INFO_ERROR_LOADING_FAILED,
  PEAS_PLUGIN_INFO_ERROR_LOADER_NOT_FOUND,
  PEAS_PLUGIN_INFO_ERROR_DEP_NOT_FOUND,
  PEAS_PLUGIN_INFO_ERROR_DEP_LOADING_FAILED
} PeasPluginInfoError;

/**
 * PeasPluginInfo:
 *
 * The #PeasPluginInfo structure contains only private data and should only
 * be accessed using the provided API.
 */
typedef struct _PeasPluginInfo PeasPluginInfo;

GType         peas_plugin_info_get_type         (void) G_GNUC_CONST;
GQuark        peas_plugin_info_error_quark      (void);

gboolean      peas_plugin_info_is_loaded        (const PeasPluginInfo *info);
gboolean      peas_plugin_info_is_available     (const PeasPluginInfo *info,
                                                 GError               **error);
gboolean      peas_plugin_info_is_builtin       (const PeasPluginInfo *info);
gboolean      peas_plugin_info_is_hidden        (const PeasPluginInfo *info);

const gchar  *peas_plugin_info_get_module_name  (const PeasPluginInfo *info);
const gchar  *peas_plugin_info_get_module_dir   (const PeasPluginInfo *info);
const gchar  *peas_plugin_info_get_data_dir     (const PeasPluginInfo *info);
GSettings    *peas_plugin_info_get_settings     (const PeasPluginInfo *info,
                                                 const gchar          *schema_id);
const gchar **peas_plugin_info_get_dependencies (const PeasPluginInfo *info);
gboolean      peas_plugin_info_has_dependency   (const PeasPluginInfo *info,
                                                 const gchar          *module_name);

const gchar  *peas_plugin_info_get_name         (const PeasPluginInfo *info);
const gchar  *peas_plugin_info_get_description  (const PeasPluginInfo *info);
const gchar  *peas_plugin_info_get_icon_name    (const PeasPluginInfo *info);
const gchar **peas_plugin_info_get_authors      (const PeasPluginInfo *info);
const gchar  *peas_plugin_info_get_website      (const PeasPluginInfo *info);
const gchar  *peas_plugin_info_get_copyright    (const PeasPluginInfo *info);
const gchar  *peas_plugin_info_get_version      (const PeasPluginInfo *info);
const gchar  *peas_plugin_info_get_help_uri     (const PeasPluginInfo *info);

const gchar  *peas_plugin_info_get_external_data (const PeasPluginInfo *info,
                                                  const gchar          *key);

G_END_DECLS

#endif /* __PEAS_PLUGIN_INFO_H__ */
