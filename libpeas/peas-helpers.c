/*
 * peas-helpers.c
 * This file is part of libpeas
 *
 * Copyright (C) 2010 Steve Frécinaux
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

#include <gobject/gvaluecollector.h>

#include "peas-helpers.h"

gpointer
_g_type_struct_ref (GType the_type)
{
  if (G_TYPE_IS_INTERFACE (the_type))
    return g_type_default_interface_ref (the_type);
  else if (G_TYPE_IS_OBJECT (the_type))
    return g_type_class_ref (the_type);
  else
    g_return_val_if_reached (NULL);
}

void
_g_type_struct_unref (GType    the_type,
                      gpointer type_struct)
{
  if (G_TYPE_IS_INTERFACE (the_type))
    g_type_default_interface_unref (type_struct);
  else if (G_TYPE_IS_OBJECT (the_type))
    g_type_class_unref (type_struct);
  else
    g_return_if_reached ();
}

static GParamSpec *
_g_type_struct_find_property (GType        the_type,
                              gpointer     type_struct,
                              const gchar *property_name)
{
  if (G_TYPE_IS_INTERFACE (the_type))
    return g_object_interface_find_property (type_struct, property_name);
  else if (G_TYPE_IS_OBJECT (the_type))
    return g_object_class_find_property (type_struct, property_name);
  else
    g_return_val_if_reached (NULL);
}

gboolean
_valist_to_parameter_list (GType         the_type,
                           gpointer      type_struct,
                           const gchar  *first_property_name,
                           va_list       args,
                           GParameter  **params,
                           guint        *n_params)
{
  const gchar *name;
  guint n_allocated_params;

  g_return_val_if_fail (type_struct != NULL, FALSE);

  *n_params = 0;
  n_allocated_params = 16;
  *params = g_new0 (GParameter, n_allocated_params);

  name = first_property_name;
  while (name)
    {
      gchar *error = NULL;
      GParamSpec *pspec = _g_type_struct_find_property (the_type, type_struct, name);

      if (!pspec)
        {
          g_warning ("%s: type '%s' has no property named '%s'",
                     G_STRFUNC, g_type_name (the_type), name);
          return FALSE;
        }

      if (*n_params >= n_allocated_params)
        {
          n_allocated_params += 16;
          *params = g_renew (GParameter, *params, n_allocated_params);
        }

      (*params)[*n_params].name = name;
      G_VALUE_COLLECT_INIT (&(*params)[*n_params].value, pspec->value_type,
                            args, 0, &error);
      if (error)
        {
          g_warning ("%s: %s", G_STRFUNC, error);
          g_free (error);
          g_value_unset (&(*params)[*n_params].value);
          return FALSE;
        }
      (*n_params)++;
      name = va_arg (args, gchar*);
    }

  return TRUE;
}
