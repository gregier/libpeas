/*
 * loadable-plugin.c
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

#include "loadable-plugin.h"

typedef struct {
  GObject *object;
} TestingLoadablePluginPrivate;

/* Used by the local linkage test */
G_MODULE_EXPORT gpointer global_symbol_clash;

static void peas_activatable_iface_init (PeasActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (TestingLoadablePlugin,
                                testing_loadable_plugin,
                                G_TYPE_OBJECT,
                                0,
                                G_ADD_PRIVATE_DYNAMIC (TestingLoadablePlugin)
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_TYPE_ACTIVATABLE,
                                                               peas_activatable_iface_init))

#define GET_PRIV(o) \
  (testing_loadable_plugin_get_instance_private (o))

enum {
  PROP_0,
  PROP_GLOBAL_SYMBOL_CLASH,

  /* PeasActivatable */
  PROP_OBJECT,
  N_PROPERTIES = PROP_OBJECT
};

static GParamSpec *properties[N_PROPERTIES] = { NULL };

static void
testing_loadable_plugin_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  TestingLoadablePlugin *plugin = TESTING_LOADABLE_PLUGIN (object);
  TestingLoadablePluginPrivate *priv = GET_PRIV (plugin);

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
testing_loadable_plugin_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  TestingLoadablePlugin *plugin = TESTING_LOADABLE_PLUGIN (object);
  TestingLoadablePluginPrivate *priv = GET_PRIV (plugin);

  switch (prop_id)
    {
    case PROP_GLOBAL_SYMBOL_CLASH:
      g_value_set_pointer (value, &global_symbol_clash);
      break;

    case PROP_OBJECT:
      g_value_set_object (value, priv->object);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
testing_loadable_plugin_init (TestingLoadablePlugin *plugin)
{
}

static void
testing_loadable_plugin_activate (PeasActivatable *activatable)
{
}

static void
testing_loadable_plugin_deactivate (PeasActivatable *activatable)
{
}

static void
testing_loadable_plugin_class_init (TestingLoadablePluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = testing_loadable_plugin_set_property;
  object_class->get_property = testing_loadable_plugin_get_property;

  g_object_class_override_property (object_class, PROP_OBJECT, "object");

  properties[PROP_GLOBAL_SYMBOL_CLASH] =
    g_param_spec_pointer ("global-symbol-clash",
                          "Global symbol clash",
                          "A global symbol that clashes",
                          G_PARAM_READABLE |
                          G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
peas_activatable_iface_init (PeasActivatableInterface *iface)
{
  iface->activate = testing_loadable_plugin_activate;
  iface->deactivate = testing_loadable_plugin_deactivate;
}

static void
testing_loadable_plugin_class_finalize (TestingLoadablePluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  testing_loadable_plugin_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_ACTIVATABLE,
                                              TESTING_TYPE_LOADABLE_PLUGIN);
}
