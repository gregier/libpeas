/*
 * self-dep-plugin.c
 * This file is part of libpeas
 *
 * Copyright (C) 2010 - Garrett Regier
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

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <libpeas/peas.h>

#include "self-dep-plugin.h"

typedef struct {
  GObject *object;
} TestingSelfDepPluginPrivate;

static void peas_activatable_iface_init (PeasActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (TestingSelfDepPlugin,
                                testing_self_dep_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_ADD_PRIVATE_DYNAMIC (TestingSelfDepPlugin)
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_TYPE_ACTIVATABLE,
                                                               peas_activatable_iface_init))

#define GET_PRIV(o) \
  (testing_self_dep_plugin_get_instance_private (o))

enum {
  PROP_0,
  PROP_OBJECT
};

static void
testing_self_dep_plugin_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  TestingSelfDepPlugin *plugin = TESTING_SELF_DEP_PLUGIN (object);
  TestingSelfDepPluginPrivate *priv = GET_PRIV (plugin);

  switch (prop_id)
    {
    case PROP_OBJECT:
      priv->object = g_value_get_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
testing_self_dep_plugin_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  TestingSelfDepPlugin *plugin = TESTING_SELF_DEP_PLUGIN (object);
  TestingSelfDepPluginPrivate *priv = GET_PRIV (plugin);

  switch (prop_id)
    {
    case PROP_OBJECT:
      g_value_set_object (value, priv->object);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
testing_self_dep_plugin_init (TestingSelfDepPlugin *plugin)
{
}

static void
testing_self_dep_plugin_activate (PeasActivatable *activatable)
{
}

static void
testing_self_dep_plugin_deactivate (PeasActivatable *activatable)
{
}

static void
testing_self_dep_plugin_class_init (TestingSelfDepPluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = testing_self_dep_plugin_set_property;
  object_class->get_property = testing_self_dep_plugin_get_property;

  g_object_class_override_property (object_class, PROP_OBJECT, "object");
}

static void
peas_activatable_iface_init (PeasActivatableInterface *iface)
{
  iface->activate = testing_self_dep_plugin_activate;
  iface->deactivate = testing_self_dep_plugin_deactivate;
}

static void
testing_self_dep_plugin_class_finalize (TestingSelfDepPluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  testing_self_dep_plugin_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_ACTIVATABLE,
                                              TESTING_TYPE_SELF_DEP_PLUGIN);
}
