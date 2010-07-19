/*
 * peas-extension-set.c
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

#include <string.h>

#include "peas-extension-set.h"
#include "peas-plugin-info.h"
#include "peas-marshal.h"
#include "peas-helpers.h"

/**
 * SECTION:peas-extension-set
 * @short_description: Proxy for a set of extensions of the same type.
 * @see_also: #PeasExtension
 *
 * A #PeasExtensionSet is an object which proxies method calls to a set
 * of actual extensions.  The application writer will use these objects
 * in order to call methods on several instances of an actual extension
 * exported by all the currently loaded plugins.
 *
 * #PeasExtensionSet will automatically track loading and unloading of
 * the plugins, and signal appearance and disappearance of new
 * extension instances.  You should connect to those signals if you
 * wish to call specific methods on loading or unloading time, as it
 * is typically the case for #PeasActivatable extensions.
 **/

G_DEFINE_TYPE (PeasExtensionSet, peas_extension_set, G_TYPE_OBJECT);

struct _PeasExtensionSetPrivate {
  PeasEngine *engine;
  GType exten_type;
  guint n_parameters;
  GParameter *parameters;

  GList *extensions;

  gulong load_handler_id;
  gulong unload_handler_id;
};

typedef struct {
  PeasPluginInfo *info;
  PeasExtension *exten;
} ExtensionItem;

/* Signals */
enum {
  EXTENSION_ADDED,
  EXTENSION_REMOVED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

/* Properties */
enum {
  PROP_0,
  PROP_ENGINE,
  PROP_EXTENSION_TYPE,
  PROP_CONSTRUCT_PROPERTIES
};

static void
set_construct_properties (PeasExtensionSet *set,
                          PeasParameterArray *array)
{
  unsigned i;

  set->priv->n_parameters = array->n_parameters;

  set->priv->parameters = g_new0 (GParameter, array->n_parameters);
  for (i = 0; i < array->n_parameters; i++)
    {
      set->priv->parameters[i].name = g_intern_string (array->parameters[i].name);
      g_value_init (&set->priv->parameters[i].value, G_VALUE_TYPE (&array->parameters[i].value));
      g_value_copy (&array->parameters[i].value, &set->priv->parameters[i].value);
    }
}

static void
peas_extension_set_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PeasExtensionSet *set = PEAS_EXTENSION_SET (object);

  switch (prop_id)
    {
    case PROP_ENGINE:
      set->priv->engine = g_value_get_object (value);
      g_object_ref (set->priv->engine);
      break;
    case PROP_EXTENSION_TYPE:
      set->priv->exten_type = g_value_get_gtype (value);
      break;
    case PROP_CONSTRUCT_PROPERTIES:
      set_construct_properties (set, g_value_get_pointer (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
peas_extension_set_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PeasExtensionSet *set = PEAS_EXTENSION_SET (object);

  switch (prop_id)
    {
    case PROP_ENGINE:
      g_value_set_object (value, set->priv->engine);
      break;
    case PROP_EXTENSION_TYPE:
      g_value_set_gtype (value, set->priv->exten_type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
add_extension (PeasExtensionSet *set,
               PeasPluginInfo   *info)
{
  PeasExtension *exten;
  ExtensionItem *item;

  /* Let's just ignore unloaded plugins... */
  if (!peas_plugin_info_is_loaded (info))
    return;

  exten = peas_engine_create_extensionv (set->priv->engine, info,
                                         set->priv->exten_type,
                                         set->priv->n_parameters,
                                         set->priv->parameters);
  if (!exten)
    return;

/*  peas_plugin_info_ref (info); */
  g_object_ref (exten);

  item = (ExtensionItem *) g_slice_new (ExtensionItem);
  item->info = info;
  item->exten = exten;

  set->priv->extensions = g_list_prepend (set->priv->extensions, item);
  g_signal_emit (set, signals[EXTENSION_ADDED], 0, info, exten);
}

static void
remove_extension_item (PeasExtensionSet *set,
                       ExtensionItem    *item)
{
  g_signal_emit (set, signals[EXTENSION_REMOVED], 0, item->info, item->exten);

/*  peas_plugin_info_unref (item->info); */
  g_object_unref (item->exten);

  g_slice_free (ExtensionItem, item);
}

static void
remove_extension (PeasExtensionSet *set,
                  PeasPluginInfo   *info)
{
  GList *l;
  ExtensionItem *item;

  for (l = set->priv->extensions; l; l = l->next)
    {
      item = (ExtensionItem *) l->data;
      if (item->info != info)
        continue;

      remove_extension_item (set, item);
      set->priv->extensions = g_list_delete_link (set->priv->extensions, l);
      return;
    }
}

static void
peas_extension_set_init (PeasExtensionSet *set)
{
  set->priv = G_TYPE_INSTANCE_GET_PRIVATE (set, PEAS_TYPE_EXTENSION_SET, PeasExtensionSetPrivate);
}

static void
peas_extension_set_constructed (GObject *object)
{
  PeasExtensionSet *set = PEAS_EXTENSION_SET (object);

  GList *plugins, *l;

  plugins = (GList *) peas_engine_get_plugin_list (set->priv->engine);
  for (l = plugins; l; l = l->next)
    add_extension (set, (PeasPluginInfo *) l->data);

  set->priv->load_handler_id =
          g_signal_connect_data (set->priv->engine, "load-plugin",
                                 G_CALLBACK (add_extension), set,
                                 NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);
  set->priv->unload_handler_id =
          g_signal_connect_data (set->priv->engine, "unload-plugin",
                                 G_CALLBACK (remove_extension), set,
                                 NULL, G_CONNECT_SWAPPED);
}

static void
peas_extension_set_finalize (GObject *object)
{
  PeasExtensionSet *set = PEAS_EXTENSION_SET (object);
  GList *l;

  g_signal_handler_disconnect (set->priv->engine, set->priv->load_handler_id);
  g_signal_handler_disconnect (set->priv->engine, set->priv->unload_handler_id);

  for (l = set->priv->extensions; l;)
    {
      remove_extension_item (set, (ExtensionItem *) l->data);
      l = g_list_delete_link (l, l);
    }

  if (set->priv->parameters != NULL)
    {
      while (set->priv->n_parameters-- > 0)
        g_value_unset (&set->priv->parameters[set->priv->n_parameters].value);
      g_free (set->priv->parameters);
    }

  g_object_unref (set->priv->engine);
}

static gboolean
peas_extension_set_call_real (PeasExtensionSet *set,
                              const gchar      *method,
                              va_list           args)
{
  gboolean ret = TRUE;
  GList *l;
  va_list args_copy;

  for (l = set->priv->extensions; l; l = l->next)
    {
      G_VA_COPY (args_copy, args);
      ret = peas_extension_call_valist (((ExtensionItem *) l->data)->exten, method, args_copy) && ret;
    }

  return ret;
}

static void
peas_extension_set_class_init (PeasExtensionSetClass *klass)
{
  GType the_type = G_TYPE_FROM_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = peas_extension_set_set_property;
  object_class->get_property = peas_extension_set_get_property;
  object_class->constructed = peas_extension_set_constructed;
  object_class->finalize = peas_extension_set_finalize;

  klass->call = peas_extension_set_call_real;

  signals[EXTENSION_ADDED] =
    g_signal_new ("extension-added",
                  the_type,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PeasExtensionSetClass, extension_added),
                  NULL, NULL,
                  peas_cclosure_marshal_VOID__BOXED_OBJECT,
                  G_TYPE_NONE,
                  2,
                  PEAS_TYPE_PLUGIN_INFO | G_SIGNAL_TYPE_STATIC_SCOPE,
                  PEAS_TYPE_EXTENSION);

  signals[EXTENSION_REMOVED] =
    g_signal_new ("extension-removed",
                  the_type,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PeasExtensionSetClass, extension_removed),
                  NULL, NULL,
                  peas_cclosure_marshal_VOID__BOXED_OBJECT,
                  G_TYPE_NONE,
                  2,
                  PEAS_TYPE_PLUGIN_INFO | G_SIGNAL_TYPE_STATIC_SCOPE,
                  PEAS_TYPE_EXTENSION);

  g_object_class_install_property (object_class, PROP_ENGINE,
                                   g_param_spec_object ("engine",
                                                        "Engine",
                                                        "The PeasEngine this set is attached to",
                                                        PEAS_TYPE_ENGINE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_EXTENSION_TYPE,
                                   g_param_spec_gtype ("extension-type",
                                                       "Extension Type",
                                                       "The extension GType managed by this set",
                                                       G_TYPE_NONE,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY |
                                                       G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_CONSTRUCT_PROPERTIES,
                                   g_param_spec_pointer ("construct-properties",
                                                         "Construct Properties",
                                                         "The properties to pass the extensions when creating them",
                                                         G_PARAM_WRITABLE |
                                                         G_PARAM_CONSTRUCT_ONLY |
                                                         G_PARAM_STATIC_STRINGS));

  g_type_class_add_private (klass, sizeof (PeasExtensionSetPrivate));
}

/**
 * peas_extension_set_call:
 * @set: A #PeasExtensionSet.
 * @method_name: the name of the method that should be called.
 * @Varargs: arguments for the method.
 *
 * Call a method on all the #PeasExtension instances contained in @set.
 *
 * See peas_extension_call() for more information.
 *
 * Return value: %TRUE on successful call.
 */
gboolean
peas_extension_set_call (PeasExtensionSet *set,
                         const gchar      *method_name,
                         ...)
{
  va_list args;
  gboolean result;

  va_start (args, method_name);
  result = peas_extension_set_call_valist (set, method_name, args);
  va_end (args);

  return result;
}

/**
 * peas_extension_set_call_valist:
 * @exten: A #PeasExtensionSet.
 * @method_name: the name of the method that should be called.
 * @args: the arguments for the method.
 *
 * Call a method on all the #PeasExtension instances contained in @set.
 *
 * See peas_extension_call_valist() for more information.
 *
 * Return value: %TRUE on successful call.
 */
gboolean
peas_extension_set_call_valist (PeasExtensionSet *set,
                                const gchar      *method_name,
                                va_list           args)
{
  PeasExtensionSetClass *klass;

  g_return_val_if_fail (PEAS_IS_EXTENSION_SET (set), FALSE);
  g_return_val_if_fail (method_name != NULL, FALSE);

  klass = PEAS_EXTENSION_SET_GET_CLASS (set);
  return klass->call (set, method_name, args);
}

/**
 * peas_extension_set_new:
 * @engine: A #PeasEngine.
 * @exten_type: the extension #GType.
 *
 * Create an #ExtensionSet for all the @exten_type extensions defined in
 * the plugins loaded in @engine.
 */
PeasExtensionSet *
peas_extension_set_newv (PeasEngine *engine,
                         GType       exten_type,
                         guint       n_parameters,
                         GParameter *parameters)
{
  PeasParameterArray construct_properties = { n_parameters, parameters };

  return PEAS_EXTENSION_SET (g_object_new (PEAS_TYPE_EXTENSION_SET,
                                           "engine", engine,
                                           "extension-type", exten_type,
                                           "construct-properties", &construct_properties,
                                           NULL));
}

PeasExtensionSet *
peas_extension_set_new_valist (PeasEngine  *engine,
                               GType        exten_type,
                               const gchar *first_property,
                               va_list      var_args)
{
  gpointer type_struct;
  GParameter *parameters;
  guint n_parameters;
  PeasExtensionSet *set;

  type_struct = _g_type_struct_ref (exten_type);

  if (!_valist_to_parameter_list (exten_type, type_struct, first_property,
                                  var_args, &parameters, &n_parameters))
    {
      /* WARNING */
      g_return_val_if_reached (NULL);
    }

  set = peas_extension_set_newv (engine, exten_type, n_parameters, parameters);

  while (n_parameters-- > 0)
    g_value_unset (&parameters[n_parameters].value);
  g_free (parameters);

  _g_type_struct_unref (exten_type, type_struct);

  return set;
}

PeasExtensionSet *
peas_extension_set_new (PeasEngine  *engine,
                        GType        exten_type,
                        const gchar *first_property,
                        ...)
{
  va_list var_args;
  PeasExtensionSet *set;

  va_start (var_args, first_property);
  set = peas_extension_set_new_valist (engine, exten_type, first_property, var_args);
  va_end (var_args);

  return set;
}
