/*
 * peas-extension-base.c
 * This file is part of libpeas
 *
 * Copyright (C) 2002-2008 Paolo Maggi
 * Copyright (C) 2009 Steve Frécinaux
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "peas-extension-base.h"
#include "peas-plugin-info-priv.h"

/**
 * SECTION:peas-extension-base
 * @short_description: Base class for C extensions.
 * @see_also: #PeasPluginInfo
 *
 * #PeasExtensionBase can optionally be used as a base class for the extensions
 * of your plugin. By inheriting from it, you will make your extension able to
 * access the related #PeasPluginInfo, and especially the location where all
 * the data of your plugin lives.
 *
 * Non-C extensions will usually not inherit from this class: Python
 * plugins automatically get a "plugin_info" attribute that serves
 * the same purpose.
 **/

struct _PeasExtensionBasePrivate {
  PeasPluginInfo *info;
};

/* properties */
enum {
  PROP_0,
  PROP_PLUGIN_INFO,
  PROP_DATA_DIR,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = { NULL };

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (PeasExtensionBase,
                                     peas_extension_base,
                                     G_TYPE_OBJECT)

#define GET_PRIV(o) \
  (peas_extension_base_get_instance_private (o))

static void
peas_extension_base_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  PeasExtensionBase *extbase = PEAS_EXTENSION_BASE (object);

  switch (prop_id)
    {
    case PROP_PLUGIN_INFO:
      g_value_set_boxed (value, peas_extension_base_get_plugin_info (extbase));
      break;
    case PROP_DATA_DIR:
      g_value_take_string (value, peas_extension_base_get_data_dir (extbase));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_extension_base_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  PeasExtensionBase *extbase = PEAS_EXTENSION_BASE (object);
  PeasExtensionBasePrivate *priv = GET_PRIV (extbase);

  switch (prop_id)
    {
    case PROP_PLUGIN_INFO:
      priv->info = g_value_get_boxed (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_extension_base_init (PeasExtensionBase *extbase)
{
}

static void
peas_extension_base_class_init (PeasExtensionBaseClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = peas_extension_base_get_property;
  object_class->set_property = peas_extension_base_set_property;

  /**
   * PeasExtensionBase:plugin-info:
   *
   * The #PeasPluginInfo related to the current plugin.
   */
  properties[PROP_PLUGIN_INFO] =
    g_param_spec_boxed ("plugin-info",
                        "Plugin Information",
                        "Information related to the current plugin",
                        PEAS_TYPE_PLUGIN_INFO,
                        G_PARAM_READWRITE |
                        G_PARAM_CONSTRUCT_ONLY |
                        G_PARAM_STATIC_STRINGS);

  /**
   * PeasExtensionBase:data-dir:
   *
   * The The full path of the directory where the plugin
   * should look for its data files.
   *
   * Note: This is the same path as that returned by
   * peas_plugin_info_get_data_dir().
   */
  properties[PROP_DATA_DIR] =
    g_param_spec_string ("data-dir",
                         "Data Directory",
                         "The full path of the directory where the "
                         "plugin should look for its data files",
                         NULL,
                         G_PARAM_READABLE |
                         G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

/**
 * peas_extension_base_get_plugin_info:
 * @extbase: A #PeasExtensionBase.
 *
 * Get information relative to @extbase.
 *
 * Return value: (transfer none): the #PeasPluginInfo relative
 * to the #PeasExtensionBase.
 */
PeasPluginInfo *
peas_extension_base_get_plugin_info (PeasExtensionBase *extbase)
{
  PeasExtensionBasePrivate *priv = GET_PRIV (extbase);

  g_return_val_if_fail (PEAS_IS_EXTENSION_BASE (extbase), NULL);

  return priv->info;
}

/**
 * peas_extension_base_get_data_dir:
 * @extbase: A #PeasExtensionBase.
 *
 * Get the path of the directory where the plugin should look for
 * its data files.
 *
 * Return value: A newly allocated string with the path of the
 * directory where the plugin should look for its data files
 */
gchar *
peas_extension_base_get_data_dir (PeasExtensionBase *extbase)
{
  PeasExtensionBasePrivate *priv = GET_PRIV (extbase);

  g_return_val_if_fail (PEAS_IS_EXTENSION_BASE (extbase), NULL);

  return g_strdup (peas_plugin_info_get_data_dir (priv->info));
}
