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

#include "peas-extension-base.h"
#include "peas-plugin-info-priv.h"
#include "peas-dirs.h"

/**
 * SECTION:peas-extension-base
 * @short_description: Base class for C extensions
 * @see_also: #PeasPluginInfo
 *
 * #PeasExtensionBase can optionally be used as a base class for the extensions
 * of your plugin. By inheriting from it, you will make your extension able to
 * access the related #PeasPluginInfo, and especially the location where all
 * the data of your plugin lives.
 *
 * Non-C extensions will usually not inherit from this class: Python and Seed
 * ones automatically get a "plugin_info" attribute that serves the same
 * purpose.
 **/

G_DEFINE_ABSTRACT_TYPE (PeasExtensionBase, peas_extension_base, G_TYPE_OBJECT);

/* properties */
enum {
  PROP_0,
  PROP_PLUGIN_INFO,
  PROP_DATA_DIR
};

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

  switch (prop_id)
    {
    case PROP_PLUGIN_INFO:
      extbase->plugin_info = g_value_dup_boxed (value);
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
peas_extension_base_finalize (GObject *object)
{
  PeasExtensionBase *extbase = PEAS_EXTENSION_BASE (object);

  if (extbase->plugin_info != NULL)
    _peas_plugin_info_unref (extbase->plugin_info);

  G_OBJECT_CLASS (peas_extension_base_parent_class)->finalize (object);
}

static void
peas_extension_base_class_init (PeasExtensionBaseClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = peas_extension_base_get_property;
  object_class->set_property = peas_extension_base_set_property;
  object_class->finalize = peas_extension_base_finalize;

  g_object_class_install_property (object_class,
                                   PROP_PLUGIN_INFO,
                                   g_param_spec_boxed ("plugin-info",
                                                       "Plugin Information",
                                                       "Information related to the current plugin",
                                                       PEAS_TYPE_PLUGIN_INFO,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY |
                                                       G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
                                   PROP_DATA_DIR,
                                   g_param_spec_string ("data-dir",
                                                        "Data Directory",
                                                        "The full path of the directory where the plugin should look for its data files",
                                                        NULL,
                                                        G_PARAM_READABLE |
                                                        G_PARAM_STATIC_STRINGS));
}

/**
 * peas_extension_base_get_plugin_info:
 * @extbase: A #PeasExtensionBase.
 *
 * Get information relative to @extbase.
 *
 * Return value: the #PeasPluginInfo relative to the #PeasExtensionBase.
 */
PeasPluginInfo *
peas_extension_base_get_plugin_info (PeasExtensionBase *extbase)
{
  g_return_val_if_fail (PEAS_IS_EXTENSION_BASE (extbase), NULL);

  return extbase->plugin_info;
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
  g_return_val_if_fail (PEAS_IS_EXTENSION_BASE (extbase), NULL);

  return g_strdup (peas_plugin_info_get_data_dir (extbase->plugin_info));
}
