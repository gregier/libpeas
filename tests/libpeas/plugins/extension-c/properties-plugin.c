/*
 * properties-plugin.c
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <libpeas/peas.h>

#include "introspection-properties.h"

#include "properties-plugin.h"

struct _TestingPropertiesPluginPrivate {
  gchar *construct_only;
  gchar *readwrite;
};

static void introspection_properties_iface_init (IntrospectionPropertiesInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (TestingPropertiesPlugin,
                                testing_properties_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (INTROSPECTION_TYPE_PROPERTIES,
                                                               introspection_properties_iface_init))

/* Properties */
enum {
  PROP_0,
  PROP_CONSTRUCT_ONLY,
  PROP_READ_ONLY,
  PROP_WRITE_ONLY,
  PROP_READWRITE
};

static void
testing_properties_plugin_init (TestingPropertiesPlugin *plugin)
{
  plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
                                              TESTING_TYPE_PROPERTIES_PLUGIN,
                                              TestingPropertiesPluginPrivate);
}

static void
testing_properties_plugin_set_property (GObject      *object,
                                        guint         prop_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  TestingPropertiesPlugin *plugin = TESTING_PROPERTIES_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_CONSTRUCT_ONLY:
      if (plugin->priv->construct_only != NULL)
        g_free (plugin->priv->construct_only);

      plugin->priv->construct_only = g_value_dup_string (value);
      break;

    case PROP_WRITE_ONLY:
      /* Don't bother actually doing anythin */
      break;

    case PROP_READWRITE:
      if (plugin->priv->readwrite != NULL)
        g_free (plugin->priv->readwrite);

      plugin->priv->readwrite = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
testing_properties_plugin_get_property (GObject    *object,
                                        guint       prop_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
  TestingPropertiesPlugin *plugin = TESTING_PROPERTIES_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_CONSTRUCT_ONLY:
      g_value_set_string (value, plugin->priv->construct_only);
      break;

    case PROP_READ_ONLY:
      g_value_set_string (value, "read-only");
      break;

    case PROP_READWRITE:
      g_value_set_string (value, plugin->priv->readwrite);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
testing_properties_plugin_dispose (GObject *object)
{
  TestingPropertiesPlugin *plugin = TESTING_PROPERTIES_PLUGIN (object);

  if (plugin->priv->construct_only != NULL)
    {
      g_free (plugin->priv->construct_only);
      plugin->priv->construct_only = NULL;
    }

  if (plugin->priv->readwrite != NULL)
    {
      g_free (plugin->priv->readwrite);
      plugin->priv->readwrite = NULL;
    }

  G_OBJECT_CLASS (testing_properties_plugin_parent_class)->dispose (object);
}

static void
testing_properties_plugin_class_init (TestingPropertiesPluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = testing_properties_plugin_set_property;
  object_class->get_property = testing_properties_plugin_get_property;
  object_class->dispose = testing_properties_plugin_dispose;

  g_object_class_override_property (object_class, PROP_CONSTRUCT_ONLY, "construct-only");
  g_object_class_override_property (object_class, PROP_READ_ONLY, "read-only");
  g_object_class_override_property (object_class, PROP_WRITE_ONLY, "write-only");
  g_object_class_override_property (object_class, PROP_READWRITE, "readwrite");

  g_type_class_add_private (klass, sizeof (TestingPropertiesPluginPrivate));
}

static void
introspection_properties_iface_init (IntrospectionPropertiesInterface *iface)
{
}

static void
testing_properties_plugin_class_finalize (TestingPropertiesPluginClass *klass)
{
}

void
testing_properties_plugin_register (GTypeModule *module)
{
  testing_properties_plugin_register_type (module);
}
