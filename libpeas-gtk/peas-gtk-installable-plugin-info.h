/*
 * peas-gtk-installable-plugin-info.h
 * This file is part of libpeas
 *
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

#ifndef __PEAS_GTK_INSTALLABLE_PLUGIN_INFO_H__
#define __PEAS_GTK_INSTALLABLE_PLUGIN_INFO_H__

#include <glib-object.h>

typedef struct _PeasGtkInstallablePluginInfo PeasGtkInstallablePluginInfo;

/* Fields should only be accessed by PeasGtkPluginStoreBackends */
struct _PeasGtkInstallablePluginInfo {
  /*< private >*/
  gint refcount;

  gchar *module_name;

  gchar *name;
  gchar *desc;
  gchar *icon_name;

  GError *error;

  GDestroyNotify notify;

  guint available : 1;
  guint installed : 1;
  guint in_use   : 1;
};

/**
 * PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR:
 *
 * Error domain for PeasGtkInstallablePluginInfo. Errors in this domain will
 * be from the PeasPluginInfoError enumeration. See GError for
 * more information on error domains.
 */
#define PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR peas_gtk_installable_plugin_info_error_quark ()

/**
 * PeasGtkInstallablePluginInfoError:
 * @PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_GET_PLUGINS: Failed to get plugins.
 * @PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_INSTALL_PLUGIN: Failed to install plugin.
 * @PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_UNINSTALL_PLUGIN: Failed to uninstall plugin.
 *
 * These identify the various errors that can occur.
 */
typedef enum {
  PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_GET_PLUGINS,
  PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_INSTALL_PLUGIN,
  PEAS_GTK_INSTALLABLE_PLUGIN_INFO_ERROR_UNINSTALL_PLUGIN
} PeasGtkInstallablePluginInfoError;

GType         peas_gtk_installable_plugin_info_get_type         (void) G_GNUC_CONST;
GQuark        peas_gtk_installable_plugin_info_error_quark      (void);

PeasGtkInstallablePluginInfo *
              peas_gtk_installable_plugin_info_new              (guint                               sizeof_info,
                                                                 GDestroyNotify                      destroy_notify);

PeasGtkInstallablePluginInfo *
              peas_gtk_installable_plugin_info_ref              (PeasGtkInstallablePluginInfo       *info);
void          peas_gtk_installable_plugin_info_unref            (PeasGtkInstallablePluginInfo       *info);

gboolean      peas_gtk_installable_plugin_info_is_available     (const PeasGtkInstallablePluginInfo *info,
                                                                 GError                             **error);
gboolean      peas_gtk_installable_plugin_info_is_installed     (const PeasGtkInstallablePluginInfo *info);
gboolean      peas_gtk_installable_plugin_info_is_in_use        (const PeasGtkInstallablePluginInfo *info);

const gchar  *peas_gtk_installable_plugin_info_get_module_name  (const PeasGtkInstallablePluginInfo *info);
const gchar  *peas_gtk_installable_plugin_info_get_name         (const PeasGtkInstallablePluginInfo *info);
const gchar  *peas_gtk_installable_plugin_info_get_description  (const PeasGtkInstallablePluginInfo *info);
const gchar  *peas_gtk_installable_plugin_info_get_icon_name    (const PeasGtkInstallablePluginInfo *info);

#endif /* __PEAS_GTK_INSTALLABLE_PLUGIN_INFO_H__ */
