/*
 * introspection-prerequisite.h
 * This file is part of libpeas
 *
 * Copyright (C) 2017 Garrett Regier
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

typedef struct {
  gint value;
} IntrospectionPrerequisitePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (IntrospectionPrerequisite,
                            introspection_prerequisite,
                            PEAS_TYPE_EXTENSION_BASE)

#define GET_PRIV(o) \
  (introspection_prerequisite_get_instance_private (o))

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
  IntrospectionPrerequisite *prereq = INTROSPECTION_PREREQUISITE (object);
  IntrospectionPrerequisitePrivate *priv = GET_PRIV (prereq);

  switch (prop_id)
    {
    case PROP_PREREQUISITE_PROPERTY:
      g_value_set_int (value, priv->value);
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
  IntrospectionPrerequisite *prereq = INTROSPECTION_PREREQUISITE (object);
  IntrospectionPrerequisitePrivate *priv = GET_PRIV (prereq);

  switch (prop_id)
    {
    case PROP_PREREQUISITE_PROPERTY:
      priv->value = g_value_get_int (value);
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
                        G_PARAM_READWRITE |
                        G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
introspection_prerequisite_init (IntrospectionPrerequisite *prereq)
{
}
