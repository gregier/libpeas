/*
 * version-dep-plugin.c
 * This file is part of libpeas
 *
 * Copyright (C) 2011 - Garrett Regier
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

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <libpeas/peas.h>

#include "version-dep-plugin.h"

struct _TestingVersionDepPluginPrivate {
  GObject *object;
};

static void peas_activatable_iface_init (PeasActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (TestingVersionDepPlugin,
                                testing_version_dep_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_TYPE_ACTIVATABLE,
                                                               peas_activatable_iface_init))

enum {
  PROP_0,
  PROP_OBJECT
};

static void
testing_version_dep_plugin_set_property (GObject      *object,
                                         guint         prop_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
  TestingVersionDepPlugin *plugin = TESTING_VERSION_DEP_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      plugin->priv->object = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
testing_version_dep_plugin_get_property (GObject    *object,
                                         guint       prop_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
  TestingVersionDepPlugin *plugin = TESTING_VERSION_DEP_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      g_value_set_object (value, plugin->priv->object);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
testing_version_dep_plugin_init (TestingVersionDepPlugin *plugin)
{
  plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
                                              TESTING_TYPE_VERSION_DEP_PLUGIN,
                                              TestingVersionDepPluginPrivate);
}

static void
testing_version_dep_plugin_activate (PeasActivatable *activatable)
{
}

static void
testing_version_dep_plugin_deactivate (PeasActivatable *activatable)
{
}

static void
testing_version_dep_plugin_class_init (TestingVersionDepPluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = testing_version_dep_plugin_set_property;
  object_class->get_property = testing_version_dep_plugin_get_property;

  g_object_class_override_property (object_class, PROP_OBJECT, "object");

  g_type_class_add_private (klass, sizeof (TestingVersionDepPluginPrivate));
}

static void
peas_activatable_iface_init (PeasActivatableInterface *iface)
{
  iface->activate = testing_version_dep_plugin_activate;
  iface->deactivate = testing_version_dep_plugin_deactivate;
}

static void
testing_version_dep_plugin_class_finalize (TestingVersionDepPluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  testing_version_dep_plugin_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_ACTIVATABLE,
                                              TESTING_TYPE_VERSION_DEP_PLUGIN);
}
