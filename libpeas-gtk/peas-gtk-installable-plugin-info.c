/*
 * peas-gtk-installable-plugin-info.h
 * This file is part of libpeas
 *
 * Copyright (C) 2011 Garrett Regier
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "peas-gtk-installable-plugin-info.h"

G_DEFINE_BOXED_TYPE (PeasGtkInstallablePluginInfo,
                     peas_gtk_installable_plugin_info,
                     peas_gtk_installable_plugin_info_ref,
                     peas_gtk_installable_plugin_info_unref)

GQuark
peas_gtk_installable_plugin_info_error_quark (void)
{
  static volatile gsize quark = 0;

	if (g_once_init_enter (&quark))
		g_once_init_leave (&quark,
		                   g_quark_from_static_string ("peas-gtk-installable-plugin-info-error"));

	return quark;
}

PeasGtkInstallablePluginInfo *
peas_gtk_installable_plugin_info_new (guint          sizeof_info,
                                      GDestroyNotify destroy_notify)
{
  PeasGtkInstallablePluginInfo *info;

  g_return_val_if_fail (sizeof_info >= sizeof (PeasGtkInstallableInfo), NULL);

  info = g_malloc0 (sizeof_info);
  info->refcount = 1;
  info->available = TRUE;
  info->notify = destroy_notify;

  return info;
}

PeasGtkInstallablePluginInfo *
peas_gtk_installable_plugin_info_ref (PeasGtkInstallablePluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  g_atomic_int_inc (&info->refcount);

  return info;
}

void
peas_gtk_installable_plugin_info_unref (PeasGtkInstallablePluginInfo *info)
{
  g_return_if_fail (info != NULL);

  if (!g_atomic_int_dec_and_test (&info->refcount))
    return;

  if (info->notify != NULL)
    info->notify ((gpointer) info);

  g_free (info->module_name);

  g_free (info->name);
  g_free (info->desc);
  g_free (info->icon_name);

  if (info->error != NULL)
    g_error_free (info->error);

  g_free (info);
}

gboolean
peas_gtk_installable_plugin_info_is_available (const PeasGtkInstallablePluginInfo  *info,
                                               GError                             **error)
{
  g_return_val_if_fail (info != NULL, FALSE);

  if (error != NULL && info->error != NULL)
    g_propagate_error (error, g_error_copy (info->error));

  return info->available;
}

gboolean
peas_gtk_installable_plugin_info_is_installed (const PeasGtkInstallablePluginInfo *info)
{
  g_return_val_if_fail (info != NULL, FALSE);

  return info->installed;
}

gboolean
peas_gtk_installable_plugin_info_is_in_use (const PeasGtkInstallablePluginInfo *info)
{
  g_return_val_if_fail (info != NULL, FALSE);

  return info->in_use;
}

const gchar *
peas_gtk_installable_plugin_info_get_module_name (const PeasGtkInstallablePluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->module_name;
}

const gchar *
peas_gtk_installable_plugin_info_get_name (const PeasGtkInstallablePluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->name;
}

const gchar *
peas_gtk_installable_plugin_info_get_description (const PeasGtkInstallablePluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->desc;
}

const gchar *
peas_gtk_installable_plugin_info_get_icon_name (const PeasGtkInstallablePluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  if (info->icon_name == NULL)
    return "libpeas-plugin";

  return info->icon_name;
}
