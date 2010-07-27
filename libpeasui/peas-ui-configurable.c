/*
 * peas-ui-configurable.c
 * This file is part of libpeas
 *
 * Copyright (C) 2009 Steve Steve Frécinaux
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

#include "peas-ui-configurable.h"

/**
 * SECTION:peas-ui-configurable
 * @short_description: Interface to provide a plugin configuration UI
 *
 * The #PeasUIConfigurable interface will allow a plugin to provide a
 * graphical interface for the user to configure the plugin through the
 * #PeasUIPluginManager: the #PeasUIPluginManager will make its “Configure
 * Plugin” button active when the selected plugin implements the
 * #PeasUIConfigurable interface.
 *
 * To allow plugin configuration from the #PeasUIPluginManager, the plugin
 * writer will just need to implement the create_configure_widget() method.
 **/

G_DEFINE_INTERFACE(PeasUIConfigurable, peas_ui_configurable, G_TYPE_OBJECT)

static void
peas_ui_configurable_default_init (PeasUIConfigurableInterface *iface)
{
}

/**
 * peas_ui_configurable_create_configure_widget:
 * @configurable: A #PeasUIConfigurable
 *
 * Creates the configure widget widget for the plugin. The returned widget
 * should allow configuring all the relevant aspects of the plugin, and should
 * allow instant-apply, as promoted by the Gnome Human Interface Guidelines.
 *
 * #PeasUIPluginManager will embed the returned widget into a dialog box,
 * but you shouldn't take this behaviour for granted as other implementations
 * of a plugin manager UI might do otherwise.
 *
 * This method should always return a valid #GtkWidget instance, never %NULL.
 *
 * Returns: A #GtkWidget used for configuration.
 */
GtkWidget *
peas_ui_configurable_create_configure_widget (PeasUIConfigurable  *configurable)
{
  PeasUIConfigurableInterface *iface;

  g_return_val_if_fail (PEAS_UI_IS_CONFIGURABLE (configurable), FALSE);
  
  iface = PEAS_UI_CONFIGURABLE_GET_IFACE (configurable);

  if (G_LIKELY (iface->create_configure_widget != NULL))
    return iface->create_configure_widget (configurable);

  /* Default implementation */
  return NULL;
}
