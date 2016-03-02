/*
 * peas-utils.c
 * This file is part of libpeas
 *
 * Copyright (C) 2010 Steve Frécinaux
 * Copyright (C) 2011-2016 Garrett Regier
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
add_all_prerequisites (GType          iface_type,
                       GObjectClass **klass,
                       GPtrArray     *ifaces)
{
  GType *prereq;
  guint n_prereq;
  guint i;

  g_ptr_array_add (ifaces, g_type_default_interface_ref (iface_type));

  prereq = g_type_interface_prerequisites (iface_type, &n_prereq);

  for (i = 0; i < n_prereq; ++i)
    {
      if (G_TYPE_IS_INTERFACE (prereq[i]))
        {
          add_all_prerequisites (prereq[i], klass, ifaces);
        }
      else if (G_TYPE_IS_OBJECT (prereq[i]))
        {
          if (*klass == NULL)
            {
              *klass = g_type_class_ref (prereq[i]);
            }
          else if (g_type_is_a (prereq[i], G_OBJECT_CLASS_TYPE (*klass)))
            {
              g_type_class_unref (*klass);
              *klass = g_type_class_ref (prereq[i]);
            }
        }
    }

  g_free (prereq);
}

static GParamSpec *
find_param_spec_for_prerequisites (const gchar  *name,
                                   GObjectClass *klass,
                                   GPtrArray    *ifaces)
{
  guint i;
  GParamSpec *pspec = NULL;

  if (klass != NULL)
    pspec = g_object_class_find_property (klass, name);

  for (i = 0; i < ifaces->len && pspec == NULL; ++i)
    {
      gpointer iface = g_ptr_array_index (ifaces, i);

      pspec = g_object_interface_find_property (iface, name);
    }

  return pspec;
}

static void
parameter_unset (GParameter *param)
{
  param->name = NULL;
  g_value_unset (&param->value);
}

GArray *
peas_utils_valist_to_parameter_list (GType        iface_type,
                                     const gchar *first_property,
                                     va_list      args)
{
  GArray *params;
  GObjectClass *klass = NULL;
  GPtrArray *ifaces;
  const gchar *name;

  g_return_val_if_fail (G_TYPE_IS_INTERFACE (iface_type), FALSE);

  params = g_array_new (FALSE, TRUE, sizeof (GParameter));
  g_array_set_clear_func (params, (GDestroyNotify) parameter_unset);

  ifaces = g_ptr_array_new ();
  g_ptr_array_set_free_func (ifaces,
                             (GDestroyNotify) g_type_default_interface_unref);
  add_all_prerequisites (iface_type, &klass, ifaces);

  for (name = first_property; name != NULL; name = va_arg (args, gchar *))
    {
      GParamSpec *pspec;
      gchar *error_msg = NULL;
      GParameter *param;

      pspec = find_param_spec_for_prerequisites (name, klass, ifaces);

      if (pspec == NULL)
        {
          g_warning ("%s: type '%s' has no property named '%s'",
                     G_STRFUNC, g_type_name (iface_type), name);
          goto error;
        }

      g_array_set_size (params, params->len + 1);
      param = &g_array_index (params, GParameter, params->len - 1);

      param->name = name;
      G_VALUE_COLLECT_INIT (&param->value, pspec->value_type,
                            args, 0, &error_msg);

      if (error_msg != NULL)
        {
          g_warning ("%s: %s", G_STRFUNC, error_msg);
          g_free (error_msg);
          goto error;
        }
    }

  g_ptr_array_unref (ifaces);
  g_clear_pointer (&klass, g_type_class_unref);

  return params;

error:

  g_array_unref (params);
  g_ptr_array_unref (ifaces);
  g_clear_pointer (&klass, g_type_class_unref);

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

