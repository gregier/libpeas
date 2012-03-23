/*
 * peas-extension-subclasses.c
 * This file is part of libpeas
 *
 * Copyright (C) 2010 - Steve Frécinaux
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

#include <string.h>
#include <girepository.h>
#include <girffi.h>
#include "peas-extension-wrapper.h"
#include "peas-extension-subclasses.h"
#include "peas-introspection.h"

typedef struct _MethodImpl {
  GType interface_type;
  GIFunctionInfo *invoker_info;
  const gchar *method_name;
  ffi_cif cif;
  ffi_closure *closure;
  guint struct_offset;
} MethodImpl;

static GQuark
method_impl_quark (void)
{
  static GQuark quark = 0;

  if (quark == 0)
    quark = g_quark_from_static_string ("PeasExtensionInterfaceImplementation");

  return quark;
}

static void
handle_method_impl (ffi_cif  *cif,
                    gpointer  result,
                    gpointer *args,
                    gpointer  data)
{
  MethodImpl *impl = (MethodImpl *) data;
  GIArgInfo arg_info;
  GITypeInfo type_info;
  GITypeInfo return_type_info;
  gint n_args, i;
  PeasExtensionWrapper *instance;
  GIArgument *arguments;
  GIArgument return_value;
  gboolean success;

  instance = *((PeasExtensionWrapper **) args[0]);
  g_assert (PEAS_IS_EXTENSION_WRAPPER (instance));

  n_args = g_callable_info_get_n_args (impl->invoker_info);
  g_return_if_fail (n_args >= 0);
  arguments = g_newa (GIArgument, n_args);

  for (i = 0; i < n_args; i++)
    {
      g_callable_info_load_arg (impl->invoker_info, i, &arg_info);
      g_arg_info_load_type (&arg_info, &type_info);

      if (g_arg_info_get_direction (&arg_info) == GI_DIRECTION_IN)
        peas_gi_pointer_to_argument (&type_info, args[i + 1], &arguments[i]);
      else
        arguments[i].v_pointer = *((gpointer **) args[i + 1]);
    }

  success = peas_extension_wrapper_callv (instance, impl->interface_type,
                                          impl->invoker_info, impl->method_name,
                                          arguments, &return_value);

  if (!success)
    memset (&return_value, 0, sizeof (GIArgument));

  g_callable_info_load_return_type (impl->invoker_info, &return_type_info);
  if (g_type_info_get_tag (&return_type_info) != GI_TYPE_TAG_VOID)
    peas_gi_argument_to_pointer (&return_type_info, &return_value, result);
}

static void
create_native_closure (GType            interface_type,
                       GIInterfaceInfo *iface_info,
                       GIVFuncInfo     *vfunc_info,
                       MethodImpl      *impl)
{
  GIFunctionInfo *invoker_info;
  GIStructInfo *struct_info;
  GIFieldInfo *field_info;
  GITypeInfo *type_info;
  GICallbackInfo *callback_info;
  guint n_fields, i;
  gboolean found_field_info;

  invoker_info = g_vfunc_info_get_invoker (vfunc_info);
  if (invoker_info == NULL)
    {
      g_debug ("No invoker for VFunc '%s.%s'",
               g_base_info_get_name (iface_info),
               g_base_info_get_name (vfunc_info));
      return;
    }

  struct_info = g_interface_info_get_iface_struct (iface_info);
  n_fields = g_struct_info_get_n_fields (struct_info);

  found_field_info = FALSE;
  for (i = 0; i < n_fields; i++)
    {
      field_info = g_struct_info_get_field (struct_info, i);

      if (strcmp (g_base_info_get_name (field_info),
                  g_base_info_get_name (vfunc_info)) == 0)
        {
          found_field_info = TRUE;
          break;
        }

      g_base_info_unref (field_info);
    }

  if (!found_field_info)
    {
      g_debug ("No struct field for VFunc '%s.%s'",
               g_base_info_get_name (iface_info),
               g_base_info_get_name (vfunc_info));
      g_base_info_unref (struct_info);
      g_base_info_unref (invoker_info);
      return;
    }

  type_info = g_field_info_get_type (field_info);
  g_assert (g_type_info_get_tag (type_info) == GI_TYPE_TAG_INTERFACE);

  callback_info = g_type_info_get_interface (type_info);
  g_assert (g_base_info_get_type (callback_info) == GI_INFO_TYPE_CALLBACK);

  impl->interface_type = interface_type;
  impl->invoker_info = invoker_info;
  impl->method_name = g_base_info_get_name (invoker_info);
  impl->closure = g_callable_info_prepare_closure (callback_info, &impl->cif,
                                                   handle_method_impl, impl);
  impl->struct_offset = g_field_info_get_offset (field_info);

  g_base_info_unref (callback_info);
  g_base_info_unref (type_info);
  g_base_info_unref (field_info);
  g_base_info_unref (struct_info);
}

static void
implement_interface_methods (gpointer iface,
                             GType    proxy_type)
{
  GType exten_type = G_TYPE_FROM_INTERFACE (iface);
  guint i;
  GArray *impls;

  g_debug ("Implementing interface '%s' for proxy type '%s'",
           g_type_name (exten_type), g_type_name (proxy_type));

  impls = g_type_get_qdata (exten_type, method_impl_quark ());

  if (impls == NULL)
    {
      GIInterfaceInfo *iface_info;
      guint n_vfuncs;

      iface_info = g_irepository_find_by_gtype (NULL, exten_type);
      g_return_if_fail (iface_info != NULL);
      g_return_if_fail (g_base_info_get_type (iface_info) == GI_INFO_TYPE_INTERFACE);

      n_vfuncs = g_interface_info_get_n_vfuncs (iface_info);

      impls = g_array_new (FALSE, TRUE, sizeof (MethodImpl));
      g_array_set_size (impls, n_vfuncs);

      for (i = 0; i < n_vfuncs; i++)
        {
          GIVFuncInfo *vfunc_info;

          vfunc_info = g_interface_info_get_vfunc (iface_info, i);
          create_native_closure (exten_type, iface_info,
                                 vfunc_info,
                                 &g_array_index (impls, MethodImpl, i));

          g_base_info_unref (vfunc_info);
        }

      g_type_set_qdata (exten_type, method_impl_quark (), impls);
      g_base_info_unref (iface_info);
    }

  for (i = 0; i < impls->len; i++)
    {
      MethodImpl *impl = &g_array_index (impls, MethodImpl, i);
      gpointer *method_ptr;

      if (impl->closure == NULL)
        continue;

      method_ptr = G_STRUCT_MEMBER_P (iface, impl->struct_offset);
      *method_ptr = impl->closure;

      g_debug ("Implemented '%s.%s' at %d (%p) with %p",
               g_type_name (exten_type), impl->method_name,
               impl->struct_offset, method_ptr, impl->closure);
    }

  g_debug ("Implemented interface '%s' for '%s' proxy",
           g_type_name (exten_type), g_type_name (proxy_type));
}

static gpointer
get_parent_class (GObject *object)
{
  return g_type_class_peek (g_type_parent (G_TYPE_FROM_INSTANCE (object)));
}

static void
extension_subclass_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PeasExtensionWrapper *exten = PEAS_EXTENSION_WRAPPER (object);

  /* This will have already been set on the real instance */
  if ((pspec->flags & G_PARAM_CONSTRUCT_ONLY) != 0)
    return;

  /* Setting will fail if we are not constructed yet */
  if ((pspec->flags & G_PARAM_CONSTRUCT) != 0 && !exten->constructed)
    return;

  g_debug ("Setting '%s:%s'",
           G_OBJECT_TYPE_NAME (object),
           g_param_spec_get_name (pspec));

  G_OBJECT_CLASS (get_parent_class (object))->set_property (object, prop_id,
                                                            value, pspec);
}

static void
extension_subclass_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  g_debug ("Getting '%s:%s'",
           G_OBJECT_TYPE_NAME (object),
           g_param_spec_get_name (pspec));

  G_OBJECT_CLASS (get_parent_class (object))->get_property (object, prop_id,
                                                            value, pspec);
}

static void
extension_subclass_init (GObjectClass *klass,
                         GType        *exten_types)
{
  guint i;
  guint property_id = 1;

  g_debug ("Initializing class '%s'", G_OBJECT_CLASS_NAME (klass));

  klass->set_property = extension_subclass_set_property;
  klass->get_property = extension_subclass_get_property;

  for (i = 0; exten_types[i] != 0; ++i)
    {
      guint n_props, j;
      gpointer iface_vtable;
      GParamSpec **properties;

      iface_vtable = g_type_default_interface_peek (exten_types[i]);
      properties = g_object_interface_list_properties (iface_vtable, &n_props);

      for (j = 0; j < n_props; ++j, ++property_id)
        {
          const gchar *property_name;

          property_name = g_param_spec_get_name (properties[j]);

          g_object_class_override_property (klass, property_id, property_name);

          g_debug ("Overrided '%s:%s' for '%s' proxy",
                   g_type_name (exten_types[i]), property_name,
                   G_OBJECT_CLASS_NAME (klass));
        }

      g_free (properties);
    }

  g_debug ("Initialized class '%s'", G_OBJECT_CLASS_NAME (klass));
}

static void
extension_subclass_instance_init (GObject *instance)
{
  g_debug ("Initializing new instance of '%s'", G_OBJECT_TYPE_NAME (instance));
}

GType
peas_extension_register_subclass (GType  parent_type,
                                  GType *extension_types)
{
  guint i;
  GString *type_name;
  GType the_type;

  type_name = g_string_new (g_type_name (parent_type));

  for (i = 0; extension_types[i] != 0; ++i)
    {
      /* Use something that is not allowed in symbol names */
      g_string_append_c (type_name, '+');

      g_string_append (type_name, g_type_name (extension_types[i]));
    }

  the_type = g_type_from_name (type_name->str);

  if (the_type == G_TYPE_INVALID)
    {
      GTypeQuery query;
      GTypeInfo type_info = {
        0,
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) extension_subclass_init,
        (GClassFinalizeFunc) NULL,
        g_memdup (extension_types, sizeof (GType) * (i + 1)),
        0,
        0,
        (GInstanceInitFunc) extension_subclass_instance_init,
        NULL
      };
      GInterfaceInfo iface_info = {
        (GInterfaceInitFunc) implement_interface_methods,
        (GInterfaceFinalizeFunc) NULL,
        NULL
      };

      g_debug ("Registering new type '%s'", type_name->str);

      g_type_query (parent_type, &query);
      type_info.class_size = query.class_size;
      type_info.instance_size = query.instance_size;

      the_type = g_type_register_static (parent_type, type_name->str,
                                         &type_info, 0);

      iface_info.interface_data = GSIZE_TO_POINTER (the_type);

      for (i = 0; extension_types[i] != 0; ++i)
        g_type_add_interface_static (the_type, extension_types[i], &iface_info);
    }

  /* Must be done outside of type registration
   * in the event that the same type is requested again.
   */
  for (i = 0; extension_types[i] != 0; ++i)
    {
      if (!g_type_is_a (the_type, extension_types[i]))
        {
          g_warning ("Type '%s' is invalid", type_name->str);
          the_type = G_TYPE_INVALID;
          break;
        }
    }

  g_string_free (type_name, TRUE);

  return the_type;
}
