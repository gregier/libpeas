/*
 * peas-extension.c
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

#include "peas-extension-wrapper.h"
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

G_DEFINE_ABSTRACT_TYPE (PeasExtensionWrapper, peas_extension_wrapper, G_TYPE_OBJECT);

/* Properties */
enum {
  PROP_0,
  PROP_EXTENSION_TYPE
};

static void
peas_extension_wrapper_init (PeasExtensionWrapper *exten)
{
}

static void
peas_extension_wrapper_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  PeasExtensionWrapper *exten = PEAS_EXTENSION_WRAPPER (object);

  switch (prop_id)
    {
    case PROP_EXTENSION_TYPE:
      exten->exten_type = g_value_get_gtype (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_extension_wrapper_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  PeasExtensionWrapper *exten = PEAS_EXTENSION_WRAPPER (object);

  switch (prop_id)
    {
    case PROP_EXTENSION_TYPE:
      g_value_set_gtype (value, exten->exten_type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_extension_wrapper_constructed (GObject *object)
{
  PeasExtensionWrapper *exten = PEAS_EXTENSION_WRAPPER (object);

  exten->constructed = TRUE;

  if (G_OBJECT_CLASS (peas_extension_wrapper_parent_class)->constructed != NULL)
    G_OBJECT_CLASS (peas_extension_wrapper_parent_class)->constructed (object);
}

static void
peas_extension_wrapper_class_init (PeasExtensionWrapperClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = peas_extension_wrapper_set_property;
  object_class->get_property = peas_extension_wrapper_get_property;
  object_class->constructed = peas_extension_wrapper_constructed;

  g_object_class_install_property (object_class, PROP_EXTENSION_TYPE,
                                   g_param_spec_gtype ("extension-type",
                                                       "Extension Type",
                                                       "The GType of the interface being proxied",
                                                       G_TYPE_NONE,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY |
                                                       G_PARAM_STATIC_STRINGS));
}

GType
peas_extension_wrapper_get_extension_type (PeasExtensionWrapper *exten)
{
  g_return_val_if_fail (PEAS_IS_EXTENSION_WRAPPER (exten), G_TYPE_INVALID);

  return exten->exten_type;
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
 * Return value: (transfer full): %TRUE on successful call.
 *
 * Deprecated: 1.2. Use the dynamically implemented interface instead.
 *
 * Rename to: peas_extension_call
 */
gboolean
peas_extension_wrapper_callv (PeasExtensionWrapper *exten,
                              const gchar   *method_name,
                              GIArgument    *args,
                              GIArgument    *return_value)
{
  PeasExtensionWrapperClass *klass;

  g_return_val_if_fail (PEAS_IS_EXTENSION_WRAPPER (exten), FALSE);
  g_return_val_if_fail (method_name != NULL, FALSE);

  klass = PEAS_EXTENSION_WRAPPER_GET_CLASS (exten);
  return klass->call (exten, method_name, args, return_value);
}
