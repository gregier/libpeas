/*
 * builtin-plugin.c
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

#include "builtin-plugin.h"

#define TESTING_TYPE_BUILTIN_PLUGIN         (testing_builtin_plugin_get_type ())
#define TESTING_BUILTIN_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TESTING_TYPE_BUILTIN_PLUGIN, TestingBuiltinPlugin))
#define TESTING_BUILTIN_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TESTING_TYPE_BUILTIN_PLUGIN, TestingBuiltinPlugin))
#define TESTING_IS_BUILTIN_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TESTING_TYPE_BUILTIN_PLUGIN))
#define TESTING_IS_BUILTIN_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TESTING_TYPE_BUILTIN_PLUGIN))
#define TESTING_BUILTIN_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TESTING_TYPE_BUILTIN_PLUGIN, TestingBuiltinPluginClass))

typedef struct _TestingBuiltinPlugin         TestingBuiltinPlugin;
typedef struct _TestingBuiltinPluginClass    TestingBuiltinPluginClass;

struct _TestingBuiltinPlugin {
  PeasExtensionBase parent_instance;

  GObject *object;
};

struct _TestingBuiltinPluginClass {
  PeasExtensionBaseClass parent_class;
};

static void peas_activatable_iface_init (PeasActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (TestingBuiltinPlugin,
                                testing_builtin_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_TYPE_ACTIVATABLE,
                                                               peas_activatable_iface_init))

enum {
  PROP_0,
  PROP_OBJECT
};

static void
testing_builtin_plugin_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  TestingBuiltinPlugin *plugin = TESTING_BUILTIN_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      plugin->object = g_value_get_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
testing_builtin_plugin_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  TestingBuiltinPlugin *plugin = TESTING_BUILTIN_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      g_value_set_object (value, plugin->object);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
testing_builtin_plugin_init (TestingBuiltinPlugin *plugin)
{
}

static void
testing_builtin_plugin_activate (PeasActivatable *activatable)
{
}

static void
testing_builtin_plugin_deactivate (PeasActivatable *activatable)
{
}

static void
testing_builtin_plugin_class_init (TestingBuiltinPluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = testing_builtin_plugin_set_property;
  object_class->get_property = testing_builtin_plugin_get_property;

  g_object_class_override_property (object_class, PROP_OBJECT, "object");
}

static void
peas_activatable_iface_init (PeasActivatableInterface *iface)
{
  iface->activate = testing_builtin_plugin_activate;
  iface->deactivate = testing_builtin_plugin_deactivate;
}

static void
testing_builtin_plugin_class_finalize (TestingBuiltinPluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  testing_builtin_plugin_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_ACTIVATABLE,
                                              TESTING_TYPE_BUILTIN_PLUGIN);
}
