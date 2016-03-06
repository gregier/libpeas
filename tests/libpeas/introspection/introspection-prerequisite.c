/*
 * introspection-prerequisite.h
 * This file is part of libpeas
 *
 * Copyright (C) 2016 Garrett Regier
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

#include "introspection-prerequisite.h"

G_DEFINE_TYPE (IntrospectionPrerequisite,
               introspection_prerequisite,
               PEAS_TYPE_EXTENSION_BASE)

enum {
  PROP_0,
  PROP_PREREQUISITE_PROPERTY,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = { NULL };

static void
introspection_prerequisite_get_property (GObject    *object,
                                         guint       prop_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
  gpointer tmp;

  switch (prop_id)
    {
    case PROP_PREREQUISITE_PROPERTY:
      tmp = g_object_get_data (object, "prerequisite-property");
      g_value_set_int (value, GPOINTER_TO_INT (tmp));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
introspection_prerequisite_set_property (GObject      *object,
                                         guint         prop_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
  switch (prop_id)
    {
    case PROP_PREREQUISITE_PROPERTY:
      g_object_set_data (object, "prerequisite-property",
                         GINT_TO_POINTER (g_value_get_int (value)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
introspection_prerequisite_class_init (IntrospectionPrerequisiteClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = introspection_prerequisite_get_property;
  object_class->set_property = introspection_prerequisite_set_property;

  properties[PROP_PREREQUISITE_PROPERTY] =
      g_param_spec_int ("prerequisite-property",
                        "Prerequisite Property",
                        "The IntrospectionPrerequisite",
                        G_MININT,
                        G_MAXINT,
                        -1,
                        G_PARAM_CONSTRUCT |
                        G_PARAM_READWRITE |
                        G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
introspection_prerequisite_init (IntrospectionPrerequisite *prereq)
{
}
