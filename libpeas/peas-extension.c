/*
 * peas-extension.c
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

#include "peas-extension.h"
#include "peas-introspection.h"

/**
 * SECTION:peas-extension
 * @short_description: Proxy for extensions.
 * @see_also: #PeasExtensionSet
 *
 * #PeasExtension is a proxy class used to access actual extensions
 * implemented using various languages.  As such, the application writer will
 * use #PeasExtension instances to call methods on extension provided by
 * loaded plugins.
 *
 * To properly use the proxy instances, you will need GObject-introspection
 * data for the #GInterface or #GObjectClass you want to use as an extension
 * point.  For instance, if you wish to use #PeasActivatable, you will need to
 * put the following code excerpt in the engine initialization code, in order
 * to load the required "Peas" typelib:
 *
 * |[
 * g_irepository_require (g_irepository_get_default (),
 *                        "Peas", "1.0", 0, NULL);
 * ]|
 *
 * You should proceed the same way for any namespace which provides interfaces
 * you want to use as extension points. GObject-introspection data is required
 * for all the supported languages, even for C.
 *
 * #PeasExtension does not provide any way to access the underlying object.
 * The main reason is that some loaders may not rely on proper GObject
 * inheritance for the definition of extensions, and hence it would not be
 * possible for libpeas to provide a functional GObject instance at all.
 * Another reason is that it makes reference counting issues easier to deal
 * with.
 *
 * See peas_extension_call() for more information.
 **/
GType
peas_extension_get_type (void)
{
  return G_TYPE_OBJECT;
}

static
G_DEFINE_QUARK (peas-extension-type, extension_type)

static GICallableInfo *
get_method_info (PeasExtension *exten,
                 const gchar   *method_name,
                 GType         *interface)
{
  guint i;
  GType exten_type;
  GType *interfaces;
  GICallableInfo *method_info;

  /* Must prioritize the initial GType */
  exten_type = peas_extension_get_extension_type (exten);
  method_info = peas_gi_get_method_info (exten_type, method_name);

  if (method_info != NULL)
    {
      if (interface != NULL)
        *interface = exten_type;

      return method_info;
    }

  interfaces = g_type_interfaces (G_TYPE_FROM_INSTANCE (exten), NULL);

  for (i = 0; interfaces[i] != G_TYPE_INVALID; ++i)
    {
      method_info = peas_gi_get_method_info (interfaces[i], method_name);

      if (method_info != NULL)
        {
          if (interface != NULL)
            *interface = interfaces[i];

          break;
        }
    }

  if (method_info == NULL)
    g_warning ("Could not find the interface for method '%s'", method_name);

  g_free (interfaces);
  return method_info;
}

/**
 * peas_extension_get_extension_type:
 * @exten: A #PeasExtension.
 *
 * Get the type of the extension interface of the object proxied by @exten.
 *
 * Return value: The #GType proxied by @exten.
 *
 * Deprecated: 1.2.
 */
GType
peas_extension_get_extension_type (PeasExtension *exten)
{
  return GPOINTER_TO_SIZE (g_object_get_qdata (G_OBJECT (exten),
                                               extension_type_quark ()));
}

/**
 * peas_extension_call:
 * @exten: A #PeasExtension.
 * @method_name: the name of the method that should be called.
 * @...: arguments for the method.
 *
 * Call a method of the object behind @extension.
 *
 * The arguments provided to this functions should be of the same type as
 * those defined in the #GInterface or #GObjectClass used as a base for the
 * proxied extension. They should be provided in the same order, and if its
 * return type is not void, then a pointer to a variable of that type should
 * be passed as the last argument.
 *
 * For instance, if the method prototype is:
 * |[ gint (*my_method) (MyClass *instance, const gchar *str, SomeObject *obj); ]|
 * you should call peas_extension_call() this way:
 * |[ peas_extension_call (extension, "my_method", "some_str", obj, &gint_var); ]|
 *
 * This function will not do anything if the introspection data for the proxied
 * object's class has not been loaded previously through g_irepository_require().
 *
 * Return value: %TRUE on successful call.
 *
 * Deprecated: 1.2: Use the interface directly instead.
 */
gboolean
peas_extension_call (PeasExtension *exten,
                     const gchar   *method_name,
                     ...)
{
  va_list args;
  gboolean result;

  g_return_val_if_fail (PEAS_IS_EXTENSION (exten), FALSE);
  g_return_val_if_fail (method_name != NULL, FALSE);

  va_start (args, method_name);
  result = peas_extension_call_valist (exten, method_name, args);
  va_end (args);

  return result;
}

/**
 * peas_extension_call_valist:
 * @exten: A #PeasExtension.
 * @method_name: the name of the method that should be called.
 * @args: the arguments for the method.
 *
 * Call a method of the object behind @extension, using @args as arguments.
 *
 * See peas_extension_call() for more information.
 *
 * Return value: %TRUE on successful call.
 *
 * Deprecated: 1.2: Use the interface directly instead.
 */
gboolean
peas_extension_call_valist (PeasExtension *exten,
                            const gchar   *method_name,
                            va_list        args)
{
  GICallableInfo *callable_info;
  GITypeInfo retval_info;
  GIArgument *gargs;
  GIArgument retval;
  gpointer retval_ptr;
  gboolean ret;
  gint n_args;

  g_return_val_if_fail (PEAS_IS_EXTENSION (exten), FALSE);
  g_return_val_if_fail (method_name != NULL, FALSE);

  callable_info = get_method_info (exten, method_name, NULL);

  /* Already warned */
  if (callable_info == NULL)
    return FALSE;

  n_args = g_callable_info_get_n_args (callable_info);
  g_return_val_if_fail (n_args >= 0, FALSE);
  gargs = g_newa (GIArgument, n_args);
  peas_gi_valist_to_arguments (callable_info, args, gargs, &retval_ptr);

  ret = peas_extension_callv (exten, method_name, gargs, &retval);

  if (retval_ptr != NULL)
    {
      g_callable_info_load_return_type (callable_info, &retval_info);
      peas_gi_argument_to_pointer (&retval_info, &retval, retval_ptr);
    }

  g_base_info_unref ((GIBaseInfo *) callable_info);

  return ret;
}

/**
 * peas_extension_callv:
 * @exten: A #PeasExtension.
 * @method_name: the name of the method that should be called.
 * @args: the arguments for the method.
 * @return_value: the return falue for the method.
 *
 * Call a method of the object behind @extension, using @args as arguments.
 *
 * See peas_extension_call() for more information.
 *
 * Return value: %TRUE on successful call.
 *
 * Deprecated: 1.2: Use the interface directly instead.
 */
gboolean
peas_extension_callv (PeasExtension *exten,
                      const gchar   *method_name,
                      GIArgument    *args,
                      GIArgument    *return_value)
{
  GICallableInfo *method_info;
  GType interface;
  gboolean success;

  g_return_val_if_fail (PEAS_IS_EXTENSION (exten), FALSE);
  g_return_val_if_fail (method_name != NULL, FALSE);

  method_info = get_method_info (exten, method_name, &interface);

  /* Already warned */
  if (method_info == NULL)
    return FALSE;

  success = peas_gi_method_call (G_OBJECT (exten), method_info, interface,
                                 method_name, args, return_value);

  g_base_info_unref (method_info);
  return success;
}
