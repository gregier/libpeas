/*
 * configurable-plugin.c
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
#include <libpeas-gtk/peas-gtk.h>

#include "configurable-plugin.h"

#define TESTING_TYPE_CONFIGURABLE_PLUGIN         (testing_configurable_plugin_get_type ())
#define TESTING_CONFIGURABLE_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TESTING_TYPE_CONFIGURABLE_PLUGIN, TestingConfigurablePlugin))
#define TESTING_CONFIGURABLE_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TESTING_TYPE_CONFIGURABLE_PLUGIN, TestingConfigurablePlugin))
#define TESTING_IS_CONFIGURABLE_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TESTING_TYPE_CONFIGURABLE_PLUGIN))
#define TESTING_IS_CONFIGURABLE_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TESTING_TYPE_CONFIGURABLE_PLUGIN))
#define TESTING_CONFIGURABLE_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TESTING_TYPE_CONFIGURABLE_PLUGIN, TestingConfigurablePluginClass))

typedef struct _TestingConfigurablePlugin         TestingConfigurablePlugin;
typedef struct _TestingConfigurablePluginClass    TestingConfigurablePluginClass;

struct _TestingConfigurablePlugin {
  PeasExtensionBase parent_instance;
};

struct _TestingConfigurablePluginClass {
  PeasExtensionBaseClass parent_class;
};

static void peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (TestingConfigurablePlugin,
                                testing_configurable_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_GTK_TYPE_CONFIGURABLE,
                                                               peas_gtk_configurable_iface_init))

static void
testing_configurable_plugin_init (TestingConfigurablePlugin *configurable)
{
}

static void
testing_configurable_plugin_class_init (TestingConfigurablePluginClass *klass)
{
}

static GtkWidget *
testing_configurable_plugin_create_configure_widget (PeasGtkConfigurable *configurable)
{
  return gtk_label_new ("Hello, World!");
}

static void
peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface)
{
  iface->create_configure_widget = testing_configurable_plugin_create_configure_widget;
}

static void
testing_configurable_plugin_class_finalize (TestingConfigurablePluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  testing_configurable_plugin_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              PEAS_GTK_TYPE_CONFIGURABLE,
                                              TESTING_TYPE_CONFIGURABLE_PLUGIN);
}
