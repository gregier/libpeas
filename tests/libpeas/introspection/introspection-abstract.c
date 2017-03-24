/*
 * introspection-abstract.h
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

#include "introspection-abstract.h"

typedef struct {
  gint value;
} IntrospectionAbstractPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (IntrospectionAbstract,
                                     introspection_abstract,
                                     PEAS_TYPE_EXTENSION_BASE)

#define GET_PRIV(o) \
  (introspection_abstract_get_instance_private (o))

enum {
  PROP_0,
  PROP_ABSTRACT_PROPERTY,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = { NULL };

static void
introspection_abstract_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  IntrospectionAbstract *abstract = INTROSPECTION_ABSTRACT (object);
  IntrospectionAbstractPrivate *priv = GET_PRIV (abstract);

  switch (prop_id)
    {
    case PROP_ABSTRACT_PROPERTY:
      g_value_set_int (value, priv->value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
introspection_abstract_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  IntrospectionAbstract *abstract = INTROSPECTION_ABSTRACT (object);
  IntrospectionAbstractPrivate *priv = GET_PRIV (abstract);

  switch (prop_id)
    {
    case PROP_ABSTRACT_PROPERTY:
      priv->value = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
introspection_abstract_class_init (IntrospectionAbstractClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = introspection_abstract_get_property;
  object_class->set_property = introspection_abstract_set_property;

  properties[PROP_ABSTRACT_PROPERTY] =
      g_param_spec_int ("abstract-property",
                        "Abstract Property",
                        "The IntrospectionAbstract",
                        G_MININT,
                        G_MAXINT,
                        -1,
                        G_PARAM_READWRITE |
                        G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
introspection_abstract_init (IntrospectionAbstract *prereq)
{
}

gint
introspection_abstract_get_value (IntrospectionAbstract *abstract)
{
  IntrospectionAbstractPrivate *priv = GET_PRIV (abstract);

  g_return_val_if_fail (INTROSPECTION_IS_ABSTRACT (abstract), -1);

  return priv->value;
}

void
introspection_abstract_set_value (IntrospectionAbstract *abstract,
                                  gint                   value)
{
  IntrospectionAbstractPrivate *priv = GET_PRIV (abstract);

  g_return_if_fail (INTROSPECTION_IS_ABSTRACT (abstract));

  priv->value = value;
}
