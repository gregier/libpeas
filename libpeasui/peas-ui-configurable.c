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
 * @short_description: Interface for configurable plugins.
 *
 * The #PeasUIConfigurable interface will allow a #PeasPlugin to provide a
 * graphical interface for the user to configure the plugin through the
 * #PeasUIPluginManager: the #PeasUIPluginManager will make the “Configure
 * Plugin” button active when the selected plugin implements the
 * #PeasUIConfigurable interface and peas_ui_configurable_is_configurable()
 * returns %TRUE. See peas_ui_configurable_is_configurable().
 *
 * To do so, the plugin writer will just need to implement the
 * create_configure_dialog() method. You should not implement the
 * is_configurable() method, unless create_configure_dialog() is overwritten
 * but does not always return a valid #GtkWindow.
 **/

G_DEFINE_INTERFACE(PeasUIConfigurable, peas_ui_configurable, G_TYPE_OBJECT)

static void
peas_ui_configurable_default_init (PeasUIConfigurableInterface *iface)
{
}

/**
 * peas_ui_configurable_is_configurable:
 * @configurable: A #PeasUIConfigurable
 *
 * Returns whether the plugin is configurable.
 *
 * The default implementation of the is_configurable() method will return %TRUE
 * if the create_configure_dialog() method was overriden, hence you won't
 * usually need to override this method.
 *
 * Returns: %TRUE if the plugin is configurable.
 */
gboolean
peas_ui_configurable_is_configurable (PeasUIConfigurable *configurable)
{
  PeasUIConfigurableInterface *iface;

  g_return_val_if_fail (PEAS_UI_IS_CONFIGURABLE (configurable), FALSE);

  iface = PEAS_UI_CONFIGURABLE_GET_IFACE (configurable);
 
  if (iface->is_configurable != NULL)
    return iface->is_configurable (configurable);
  
  /* Default implementation */
  return iface->create_configure_dialog != NULL;
}

/**
 * peas_ui_configurable_create_configure_dialog:
 * @configurable: A #PeasUIConfigurable
 * @conf_dlg: (out): A #GtkWindow used for configuration
 *
 * Creates the configure dialog widget for the plugin.
 *
 * The default implementation returns %NULL.
 *
 * Returns: %TRUE on success.
 */
gboolean
peas_ui_configurable_create_configure_dialog (PeasUIConfigurable  *configurable,
                                              GtkWidget          **conf_dlg)
{
  PeasUIConfigurableInterface *iface;

  g_return_val_if_fail (PEAS_UI_IS_CONFIGURABLE (configurable), FALSE);
  
  iface = PEAS_UI_CONFIGURABLE_GET_IFACE (configurable);

  if (G_LIKELY (iface->create_configure_dialog != NULL))
    return iface->create_configure_dialog (configurable, conf_dlg);

  /* Default implementation */
  return FALSE;
}
