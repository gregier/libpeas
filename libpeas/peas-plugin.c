/*
 * peas-plugin.h
 * This file is part of libpeas
 *
 * Copyright (C) 2002-2008 Paolo Maggi
 * Copyright (C) 2009 Steve Frécinaux
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

#include "peas-plugin.h"
#include "peas-plugin-info-priv.h"
#include "peas-dirs.h"

/**
 * SECTION:peas-plugin
 * @short_description: Base class for plugins
 * @see_also: #PeasPluginInfo
 *
 * A #PeasPlugin is an object which represents an actual loaded plugin.
 *
 * As a plugin writer, you will need to inherit from this class to perform
 * the actions you want to using the available hooks.  It will also provide
 * you a few useful pieces of information, like the location where all your
 * data lives.
 *
 * As an application developper, you might want to provide a subclass of
 * #PeasPlugin for tighter integration with your application.  But you should
 * not use this class at all in your application code apart from that, as all
 * the actions that can be performed by plugins will be proxied by the
 * #PeasEngine.
 **/

G_DEFINE_TYPE (PeasPlugin, peas_plugin, G_TYPE_OBJECT);

/* properties */
enum {
  PROP_0,
  PROP_PLUGIN_INFO,
  PROP_DATA_DIR
};

struct _PeasPluginPrivate {
  PeasPluginInfo *info;
};

static void
dummy (PeasPlugin *plugin,
       GObject    *object)
{
  /* Empty */
}

static void
peas_plugin_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  PeasPlugin *plugin = PEAS_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_PLUGIN_INFO:
      g_value_set_boxed (value, peas_plugin_get_info (plugin));
      break;
    case PROP_DATA_DIR:
      g_value_take_string (value, peas_plugin_get_data_dir (plugin));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_plugin_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  PeasPlugin *plugin = PEAS_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_PLUGIN_INFO:
      plugin->priv->info = g_value_dup_boxed (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_plugin_init (PeasPlugin *plugin)
{
  plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin, PEAS_TYPE_PLUGIN, PeasPluginPrivate);
}

static void
peas_plugin_finalize (GObject *object)
{
  PeasPlugin *plugin = PEAS_PLUGIN (object);

  if (plugin->priv->info)
    _peas_plugin_info_unref (plugin->priv->info);

  G_OBJECT_CLASS (peas_plugin_parent_class)->finalize (object);
}

static void
peas_plugin_class_init (PeasPluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  klass->activate = dummy;
  klass->deactivate = dummy;
  klass->update_ui = dummy;

  object_class->get_property = peas_plugin_get_property;
  object_class->set_property = peas_plugin_set_property;
  object_class->finalize = peas_plugin_finalize;

  g_object_class_install_property (object_class,
                                   PROP_PLUGIN_INFO,
                                   g_param_spec_boxed ("plugin-info",
                                                       "Plugin Information",
                                                       "Information relative to the current plugin",
                                                       PEAS_TYPE_PLUGIN_INFO,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
                                   PROP_DATA_DIR,
                                   g_param_spec_string ("data-dir",
                                                        "Data Directory",
                                                        "The full path of the directory where the plugin should look for its data files",
                                                        NULL,
                                                        G_PARAM_READABLE |
                                                        G_PARAM_STATIC_STRINGS));

  g_type_class_add_private (klass, sizeof (PeasPluginPrivate));
}

/**
 * peas_plugin_get_info:
 * @plugin: A #PeasPlugin.
 *
 * Get information relative to @plugin.
 *
 * Return value: the #PeasPluginInfo relative to the #PeasPlugin.
 */
PeasPluginInfo *
peas_plugin_get_info (PeasPlugin *plugin)
{
  g_return_val_if_fail (PEAS_IS_PLUGIN (plugin), NULL);

  return plugin->priv->info;
}

/**
 * peas_plugin_get_data_dir:
 * @plugin: A #PeasPlugin.
 *
 * Get the path of the directory where the plugin should look for
 * its data files.
 *
 * Return value: A newly allocated string with the path of the
 * directory where the plugin should look for its data files
 */
gchar *
peas_plugin_get_data_dir (PeasPlugin *plugin)
{
  g_return_val_if_fail (PEAS_IS_PLUGIN (plugin), NULL);

  return g_strdup (peas_plugin_info_get_data_dir (plugin->priv->info));
}

/**
 * peas_plugin_activate:
 * @plugin: A #PeasPlugin.
 * @object: The #GObject on which the plugin should be activated.
 *
 * Activates the plugin on an object.  An instance of #PeasPlugin will be
 * activated once for each object registered against the #PeasEngine which
 * controls this #PeasPlugin.  For instance, a typical GUI application like
 * gedit will activate the plugin once for each of its main windows.
 */
void
peas_plugin_activate (PeasPlugin *plugin,
                      GObject    *object)
{
  g_return_if_fail (PEAS_IS_PLUGIN (plugin));
  g_return_if_fail (G_IS_OBJECT (object));

  PEAS_PLUGIN_GET_CLASS (plugin)->activate (plugin, object);
}

/**
 * peas_plugin_deactivate:
 * @plugin: A #PeasPlugin.
 * @object: A #GObject.
 *
 * Deactivates the plugin on the given object.
 */
void
peas_plugin_deactivate (PeasPlugin *plugin,
                        GObject    *object)
{
  g_return_if_fail (PEAS_IS_PLUGIN (plugin));
  g_return_if_fail (G_IS_OBJECT (object));

  PEAS_PLUGIN_GET_CLASS (plugin)->deactivate (plugin, object);
}

/**
 * peas_plugin_update_ui:
 * @plugin: A #PeasPlugin.
 * @object: A #GObject.
 *
 * Triggers an update of the user interface to take into account state changes
 * due to a plugin or an user action.
 */
void
peas_plugin_update_ui (PeasPlugin *plugin,
                       GObject    *object)
{
  g_return_if_fail (PEAS_IS_PLUGIN (plugin));
  g_return_if_fail (G_IS_OBJECT (object));

  PEAS_PLUGIN_GET_CLASS (plugin)->update_ui (plugin, object);
}

