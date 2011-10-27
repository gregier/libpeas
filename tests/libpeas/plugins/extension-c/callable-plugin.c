/*
 * callable-plugin.c
 * This file is part of libpeas
 *
 * Copyright (C) 2010 - Garrett Regier
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <libpeas/peas.h>

#include "introspection-callable.h"
#include "introspection-has-prerequisite.h"

#include "callable-plugin.h"

struct _TestingCallablePluginPrivate {
  GObject *object;
};

static void introspection_activatable_iface_init (PeasActivatableInterface *iface);
static void introspection_callable_iface_init (IntrospectionCallableInterface *iface);
static void introspection_has_prerequisite_iface_init (IntrospectionHasPrerequisiteInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (TestingCallablePlugin,
                                testing_callable_plugin,
                                G_TYPE_OBJECT,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_TYPE_ACTIVATABLE,
                                                               introspection_activatable_iface_init)
                                G_IMPLEMENT_INTERFACE_DYNAMIC (INTROSPECTION_TYPE_CALLABLE,
                                                               introspection_callable_iface_init)
                                G_IMPLEMENT_INTERFACE_DYNAMIC (INTROSPECTION_TYPE_HAS_PREREQUISITE,
                                                               introspection_has_prerequisite_iface_init))

enum {
  PROP_0,
  PROP_OBJECT
};

static void
testing_callable_plugin_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  TestingCallablePlugin *plugin = TESTING_CALLABLE_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      plugin->priv->object = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
testing_callable_plugin_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  TestingCallablePlugin *plugin = TESTING_CALLABLE_PLUGIN (object);

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
testing_callable_plugin_init (TestingCallablePlugin *plugin)
{
  plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
                                              TESTING_TYPE_CALLABLE_PLUGIN,
                                              TestingCallablePluginPrivate);
}

static void
testing_callable_plugin_activate (PeasActivatable *activatable)
{
}

static void
testing_callable_plugin_deactivate (PeasActivatable *activatable)
{
}

static const gchar *
testing_callable_plugin_call_with_return (IntrospectionCallable *callable)
{
  return "Hello, World!";
}

static void
testing_callable_plugin_call_single_arg (IntrospectionCallable *callable,
                                         gboolean              *called)
{
  *called = TRUE;
}

static void
testing_callable_plugin_call_multi_args (IntrospectionCallable *callable,
                                         gint                   in,
                                         gint                  *out,
                                         gint                  *inout)
{
  *out = *inout;
  *inout = in;
}

static void
testing_callable_plugin_class_init (TestingCallablePluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = testing_callable_plugin_set_property;
  object_class->get_property = testing_callable_plugin_get_property;

  g_object_class_override_property (object_class, PROP_OBJECT, "object");

  g_type_class_add_private (klass, sizeof (TestingCallablePluginPrivate));
}

static void
introspection_activatable_iface_init (PeasActivatableInterface *iface)
{
  iface->activate = testing_callable_plugin_activate;
  iface->deactivate = testing_callable_plugin_deactivate;
}

static void
introspection_callable_iface_init (IntrospectionCallableInterface *iface)
{
  iface->call_with_return = testing_callable_plugin_call_with_return;
  iface->call_single_arg = testing_callable_plugin_call_single_arg;
  iface->call_multi_args = testing_callable_plugin_call_multi_args;
}

static void
introspection_has_prerequisite_iface_init (IntrospectionHasPrerequisiteInterface *iface)
{
}

static void
testing_callable_plugin_class_finalize (TestingCallablePluginClass *klass)
{
}

void
testing_callable_plugin_register (GTypeModule *module)
{
  testing_callable_plugin_register_type (module);
}
