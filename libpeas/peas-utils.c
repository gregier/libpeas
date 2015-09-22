/*
 * peas-utils.c
 * This file is part of libpeas
 *
 * Copyright (C) 2010 Steve Frécinaux
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

#include <string.h>

#include <gobject/gvaluecollector.h>

#include "peas-utils.h"

static const gchar *all_plugin_loaders[] = {
  "c", "lua5.1", "python", "python3"
};

static const gchar *all_plugin_loader_modules[] = {
  "cloader", "lua51loader", "pythonloader", "python3loader"
};

static const gint conflicting_plugin_loaders[PEAS_UTILS_N_LOADERS][2] = {
  { -1, -1 }, /* c       => {}          */
  { -1, -1 }, /* lua5.1  => {}          */
  {  3, -1 }, /* python  => { python3 } */
  {  2, -1 }  /* python3 => { python  } */
};

G_STATIC_ASSERT (G_N_ELEMENTS (all_plugin_loaders) == PEAS_UTILS_N_LOADERS);
G_STATIC_ASSERT (G_N_ELEMENTS (all_plugin_loader_modules) == PEAS_UTILS_N_LOADERS);
G_STATIC_ASSERT (G_N_ELEMENTS (conflicting_plugin_loaders) == PEAS_UTILS_N_LOADERS);

static void
add_all_interfaces (GType      iface_type,
                    GPtrArray *type_structs)
{
  GType *prereq;
  guint n_prereq;
  guint i;

  g_ptr_array_add (type_structs,
                   g_type_default_interface_ref (iface_type));

  prereq = g_type_interface_prerequisites (iface_type, &n_prereq);

  for (i = 0; i < n_prereq; ++i)
    {
      if (G_TYPE_IS_INTERFACE (prereq[i]))
        add_all_interfaces (prereq[i], type_structs);
    }

  g_free (prereq);
}

static GParamSpec *
find_param_spec_in_interfaces (GPtrArray   *type_structs,
                               const gchar *name)
{
  guint i;
  GParamSpec *pspec = NULL;

  for (i = 0; i < type_structs->len && pspec == NULL; ++i)
    {
      gpointer iface = g_ptr_array_index (type_structs, i);

      pspec = g_object_interface_find_property (iface, name);
    }

  return pspec;
}

gboolean
peas_utils_valist_to_parameter_list (GType         iface_type,
                                     const gchar  *first_property,
                                     va_list       args,
                                     GParameter  **params,
                                     guint        *n_params)
{
  GPtrArray *type_structs;
  const gchar *name;
  guint n_allocated_params;

  g_return_val_if_fail (G_TYPE_IS_INTERFACE (iface_type), FALSE);

  type_structs = g_ptr_array_new ();
  g_ptr_array_set_free_func (type_structs,
                             (GDestroyNotify) g_type_default_interface_unref);
  add_all_interfaces (iface_type, type_structs);

  *n_params = 0;
  n_allocated_params = 16;
  *params = g_new0 (GParameter, n_allocated_params);

  name = first_property;
  while (name)
    {
      gchar *error_msg = NULL;
      GParamSpec *pspec = find_param_spec_in_interfaces (type_structs, name);

      if (!pspec)
        {
          g_warning ("%s: type '%s' has no property named '%s'",
                     G_STRFUNC, g_type_name (iface_type), name);
          goto error;
        }

      if (*n_params >= n_allocated_params)
        {
          n_allocated_params += 16;
          *params = g_renew (GParameter, *params, n_allocated_params);
          memset (*params + (n_allocated_params - 16),
                  0, sizeof (GParameter) * 16);
        }

      (*params)[*n_params].name = name;
      G_VALUE_COLLECT_INIT (&(*params)[*n_params].value, pspec->value_type,
                            args, 0, &error_msg);

      (*n_params)++;

      if (error_msg)
        {
          g_warning ("%s: %s", G_STRFUNC, error_msg);
          g_free (error_msg);
          goto error;
        }

      name = va_arg (args, gchar*);
    }

  g_ptr_array_unref (type_structs);

  return TRUE;

error:

  for (; *n_params > 0; --(*n_params))
    g_value_unset (&(*params)[*n_params].value);

  g_free (*params);
  g_ptr_array_unref (type_structs);

  return FALSE;
}

gint
peas_utils_get_loader_id (const gchar *loader)
{
  gint i;
  gsize len;
  gchar lowercase[32];

  len = strlen (loader);

  /* No loader has a name that long */
  if (len >= G_N_ELEMENTS (lowercase))
    return -1;

  for (i = 0; i < len; ++i)
    lowercase[i] = g_ascii_tolower (loader[i]);

  lowercase[len] = '\0';

  for (i = 0; i < G_N_ELEMENTS (all_plugin_loaders); ++i)
    {
      if (g_strcmp0 (lowercase, all_plugin_loaders[i]) == 0)
        return i;
    }

  return -1;
}

const gchar *
peas_utils_get_loader_from_id (gint loader_id)
{
  g_return_val_if_fail (loader_id >= 0, NULL);
  g_return_val_if_fail (loader_id < PEAS_UTILS_N_LOADERS, NULL);

  return all_plugin_loaders[loader_id];
}

const gchar *
peas_utils_get_loader_module_from_id (gint loader_id)
{
  g_return_val_if_fail (loader_id >= 0, NULL);
  g_return_val_if_fail (loader_id < PEAS_UTILS_N_LOADERS, NULL);

  return all_plugin_loader_modules[loader_id];
}

const gint *
peas_utils_get_conflicting_loaders_from_id (gint loader_id)
{
  g_return_val_if_fail (loader_id >= 0, NULL);
  g_return_val_if_fail (loader_id < PEAS_UTILS_N_LOADERS, NULL);

  return conflicting_plugin_loaders[loader_id];
}

