/*
 * peas-extension-set.c
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

#include "peas-extension-set.h"

#include "peas-i18n.h"
#include "peas-introspection.h"
#include "peas-plugin-info.h"
#include "peas-marshal.h"
#include "peas-utils.h"

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
 * wish to call specific methods on loading or unloading time.
 *
 * Here is the code for a typical setup of #PeasExtensionSet with
 * #PeasActivatable as the watched extension point, and #GtkWindow
 * instances as the target objects:
 * |[
 * static void
 * on_extension_added (PeasExtensionSet *set,
 *                     PeasPluginInfo   *info,
 *                     PeasActivatable  *activatable)
 * {
 *   peas_activatable_activate (activatable);
 * }
 *
 * static void
 * on_extension_removed (PeasExtensionSet *set,
 *                       PeasPluginInfo   *info,
 *                       PeasActivatable  *activatable)
 * {
 *   peas_activatable_deactivate (activatable);
 * }
 *
 * PeasExtensionSet *
 * setup_extension_set (PeasEngine *engine,
 *                      GtkWindow  *window)
 * {
 *   PeasExtensionSet *set;
 *
 *   set = peas_extension_set_new (engine, PEAS_TYPE_ACTIVATABLE,
 *                                 "object", window, NULL);
 *   peas_extension_set_foreach (set,
 *                               (PeasExtensionSetForeachFunc) on_extension_added,
 *                               NULL);
 *   g_signal_connect (set, "extension-added",
 *                     G_CALLBACK (on_extension_added), NULL);
 *   g_signal_connect (set, "extension-removed",
 *                     G_CALLBACK (on_extension_removed), NULL);
 *   return set;
 * }
 * ]|
 **/

struct _PeasExtensionSetPrivate {
  PeasEngine *engine;
  GType exten_type;
  guint n_parameters;
  GParameter *parameters;

  GQueue extensions;
};

typedef struct {
  PeasPluginInfo *info;
  PeasExtension *exten;
} ExtensionItem;

typedef struct {
  guint n_parameters;
  GParameter *parameters;
} PeasParameterArray;

/* Signals */
enum {
  EXTENSION_ADDED,
  EXTENSION_REMOVED,
  LAST_SIGNAL
};

/* Properties */
enum {
  PROP_0,
  PROP_ENGINE,
  PROP_EXTENSION_TYPE,
  PROP_CONSTRUCT_PROPERTIES,
  N_PROPERTIES
};

static guint signals[LAST_SIGNAL];
static GParamSpec *properties[N_PROPERTIES] = { NULL };

G_DEFINE_TYPE_WITH_PRIVATE (PeasExtensionSet,
                            peas_extension_set,
                            G_TYPE_OBJECT)

#define GET_PRIV(o) \
  (peas_extension_set_get_instance_private (o))

static void
set_construct_properties (PeasExtensionSet   *set,
                          PeasParameterArray *array)
{
  PeasExtensionSetPrivate *priv = GET_PRIV (set);
  guint i;

  priv->n_parameters = array->n_parameters;

  priv->parameters = g_new0 (GParameter, array->n_parameters);
  for (i = 0; i < array->n_parameters; i++)
    {
      priv->parameters[i].name = g_intern_string (array->parameters[i].name);
      g_value_init (&priv->parameters[i].value, G_VALUE_TYPE (&array->parameters[i].value));
      g_value_copy (&array->parameters[i].value, &priv->parameters[i].value);
    }
}

static void
peas_extension_set_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PeasExtensionSet *set = PEAS_EXTENSION_SET (object);
  PeasExtensionSetPrivate *priv = GET_PRIV (set);

  switch (prop_id)
    {
    case PROP_ENGINE:
      priv->engine = g_value_get_object (value);
      break;
    case PROP_EXTENSION_TYPE:
      priv->exten_type = g_value_get_gtype (value);
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
  PeasExtensionSetPrivate *priv = GET_PRIV (set);

  switch (prop_id)
    {
    case PROP_ENGINE:
      g_value_set_object (value, priv->engine);
      break;
    case PROP_EXTENSION_TYPE:
      g_value_set_gtype (value, priv->exten_type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
add_extension (PeasExtensionSet *set,
               PeasPluginInfo   *info)
{
  PeasExtensionSetPrivate *priv = GET_PRIV (set);
  PeasExtension *exten;
  ExtensionItem *item;

  /* Let's just ignore unloaded plugins... */
  if (!peas_plugin_info_is_loaded (info))
    return;

  if (!peas_engine_provides_extension (priv->engine, info,
                                       priv->exten_type))
    return;

  exten = peas_engine_create_extensionv (priv->engine, info,
                                         priv->exten_type,
                                         priv->n_parameters,
                                         priv->parameters);

  item = g_slice_new (ExtensionItem);
  item->info = info;
  item->exten = exten;

  g_queue_push_tail (&priv->extensions, item);
  g_signal_emit (set, signals[EXTENSION_ADDED], 0, info, exten);
}

static void
remove_extension_item (PeasExtensionSet *set,
                       ExtensionItem    *item)
{
  g_signal_emit (set, signals[EXTENSION_REMOVED], 0, item->info, item->exten);

  g_object_unref (item->exten);

  g_slice_free (ExtensionItem, item);
}

static void
remove_extension (PeasExtensionSet *set,
                  PeasPluginInfo   *info)
{
  PeasExtensionSetPrivate *priv = GET_PRIV (set);
  GList *l;

  for (l = priv->extensions.head; l != NULL; l = l->next)
    {
      ExtensionItem *item = l->data;

      if (item->info != info)
        continue;

      remove_extension_item (set, item);
      g_queue_delete_link (&priv->extensions, l);
      return;
    }
}

static void
peas_extension_set_init (PeasExtensionSet *set)
{
  PeasExtensionSetPrivate *priv = GET_PRIV (set);

  g_queue_init (&priv->extensions);
}

static void
peas_extension_set_constructed (GObject *object)
{
  PeasExtensionSet *set = PEAS_EXTENSION_SET (object);
  PeasExtensionSetPrivate *priv = GET_PRIV (set);
  GList *plugins, *l;

  if (priv->engine == NULL)
    priv->engine = peas_engine_get_default ();

  g_object_ref (priv->engine);

  plugins = (GList *) peas_engine_get_plugin_list (priv->engine);
  for (l = plugins; l; l = l->next)
    add_extension (set, (PeasPluginInfo *) l->data);

  g_signal_connect_object (priv->engine, "load-plugin",
                           G_CALLBACK (add_extension), set,
                           G_CONNECT_AFTER | G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->engine, "unload-plugin",
                           G_CALLBACK (remove_extension), set,
                           G_CONNECT_SWAPPED);

  G_OBJECT_CLASS (peas_extension_set_parent_class)->constructed (object);
}

static void
peas_extension_set_dispose (GObject *object)
{
  PeasExtensionSet *set = PEAS_EXTENSION_SET (object);
  PeasExtensionSetPrivate *priv = GET_PRIV (set);
  GList *l;

  if (priv->extensions.length > 0)
    {
      for (l = priv->extensions.tail; l != NULL; l = l->prev)
        remove_extension_item (set, (ExtensionItem *) l->data);

      g_queue_clear (&priv->extensions);
    }

  if (priv->parameters != NULL)
    {
      while (priv->n_parameters-- > 0)
        g_value_unset (&priv->parameters[priv->n_parameters].value);

      g_free (priv->parameters);
      priv->parameters = NULL;
    }

  g_clear_object (&priv->engine);

  G_OBJECT_CLASS (peas_extension_set_parent_class)->dispose (object);
}

static gboolean
peas_extension_set_call_real (PeasExtensionSet *set,
                              const gchar      *method_name,
                              GIArgument       *args)
{
  PeasExtensionSetPrivate *priv = GET_PRIV (set);
  gboolean ret = TRUE;
  GList *l;
  GIArgument dummy;

  for (l = priv->extensions.head; l != NULL; l = l->next)
    {
      ExtensionItem *item = (ExtensionItem *) l->data;
      ret = peas_extension_callv (item->exten, method_name, args, &dummy) && ret;
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
  object_class->dispose = peas_extension_set_dispose;

  klass->call = peas_extension_set_call_real;

  /**
   * PeasExtensionSet::extension-added:
   * @set: A #PeasExtensionSet.
   * @info: A #PeasPluginInfo.
   * @exten: A #PeasExtension.
   *
   * The extension-added signal is emitted when a new extension has been
   * added to the #PeasExtensionSet. It happens when a new plugin implementing
   * the extension set's extension type is loaded.
   *
   * You should connect to this signal in order to set up the extensions when
   * they are loaded. Note that this signal is not fired for extensions coming
   * from plugins that were already loaded when the #PeasExtensionSet instance
   * was created. You should set those up by yourself.
   */
  signals[EXTENSION_ADDED] =
    g_signal_new (I_("extension-added"),
                  the_type,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PeasExtensionSetClass, extension_added),
                  NULL, NULL,
                  peas_cclosure_marshal_VOID__BOXED_OBJECT,
                  G_TYPE_NONE,
                  2,
                  PEAS_TYPE_PLUGIN_INFO | G_SIGNAL_TYPE_STATIC_SCOPE,
                  PEAS_TYPE_EXTENSION);

  /**
   * PeasExtensionSet::extension-removed:
   * @set: A #PeasExtensionSet.
   * @info: A #PeasPluginInfo.
   * @exten: A #PeasExtension.
   *
   * The extension-removed signal is emitted when a new extension is about to be
   * removed from the #PeasExtensionSet. It happens when a plugin implementing
   * the extension set's extension type is unloaded, or when the
   * #PeasExtensionSet itself is destroyed.
   *
   * You should connect to this signal in order to clean up the extensions
   * when their plugin is unload. Note that this signal is not fired for the
   * #PeasExtension instances still available when the #PeasExtensionSet
   * instance is destroyed. You should clean those up by yourself.
   */
  signals[EXTENSION_REMOVED] =
    g_signal_new (I_("extension-removed"),
                  the_type,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PeasExtensionSetClass, extension_removed),
                  NULL, NULL,
                  peas_cclosure_marshal_VOID__BOXED_OBJECT,
                  G_TYPE_NONE,
                  2,
                  PEAS_TYPE_PLUGIN_INFO | G_SIGNAL_TYPE_STATIC_SCOPE,
                  PEAS_TYPE_EXTENSION);

  properties[PROP_ENGINE] =
    g_param_spec_object ("engine",
                         "Engine",
                         "The PeasEngine this set is attached to",
                         PEAS_TYPE_ENGINE,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  properties[PROP_EXTENSION_TYPE] =
    g_param_spec_gtype ("extension-type",
                        "Extension Type",
                        "The extension GType managed by this set",
                        G_TYPE_NONE,
                        G_PARAM_READWRITE |
                        G_PARAM_CONSTRUCT_ONLY |
                        G_PARAM_STATIC_STRINGS);

  properties[PROP_CONSTRUCT_PROPERTIES] =
    g_param_spec_pointer ("construct-properties",
                          "Construct Properties",
                          "The properties to pass the extensions when creating them",
                          G_PARAM_WRITABLE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

/**
 * peas_extension_set_get_extension:
 * @set: A #PeasExtensionSet
 * @info: a #PeasPluginInfo
 *
 * Returns the #PeasExtension object corresponding to @info, or %NULL
 * if the plugin doesn't provide such an extension.
 *
 * Returns: (transfer none): a reference to a #PeasExtension or %NULL
 */
PeasExtension *
peas_extension_set_get_extension (PeasExtensionSet *set,
                                  PeasPluginInfo   *info)
{
  PeasExtensionSetPrivate *priv = GET_PRIV (set);
  GList *l;

  g_return_val_if_fail (PEAS_IS_EXTENSION_SET (set), NULL);
  g_return_val_if_fail (info != NULL, NULL);

  for (l = priv->extensions.head; l != NULL; l = l->next)
    {
      ExtensionItem *item = l->data;

      if (item->info == info)
        return item->exten;
    }

  return NULL;
}

/**
 * peas_extension_set_call:
 * @set: A #PeasExtensionSet.
 * @method_name: the name of the method that should be called.
 * @...: arguments for the method.
 *
 * Call a method on all the #PeasExtension instances contained in @set.
 *
 * See peas_extension_call() for more information.
 *
 * Deprecated: 1.2: Use peas_extension_set_foreach() instead.
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

  g_return_val_if_fail (PEAS_IS_EXTENSION_SET (set), FALSE);
  g_return_val_if_fail (method_name != NULL, FALSE);

  va_start (args, method_name);
  result = peas_extension_set_call_valist (set, method_name, args);
  va_end (args);

  return result;
}

/**
 * peas_extension_set_call_valist:
 * @set: A #PeasExtensionSet.
 * @method_name: the name of the method that should be called.
 * @va_args: the arguments for the method.
 *
 * Call a method on all the #PeasExtension instances contained in @set.
 *
 * See peas_extension_call_valist() for more information.
 *
 * Deprecated: 1.2: Use peas_extension_set_foreach() instead.
 *
 * Return value: %TRUE on successful call.
 */
gboolean
peas_extension_set_call_valist (PeasExtensionSet *set,
                                const gchar      *method_name,
                                va_list           va_args)
{
  PeasExtensionSetPrivate *priv = GET_PRIV (set);
  GICallableInfo *callable_info;
  GIArgument *args;
  gint n_args;

  g_return_val_if_fail (PEAS_IS_EXTENSION_SET (set), FALSE);
  g_return_val_if_fail (method_name != NULL, FALSE);

  callable_info = peas_gi_get_method_info (priv->exten_type, method_name);

  if (callable_info == NULL)
    {
      g_warning ("Method '%s.%s' was not found",
                 g_type_name (priv->exten_type), method_name);
      return FALSE;
    }

  n_args = g_callable_info_get_n_args (callable_info);
  g_return_val_if_fail (n_args >= 0, FALSE);

  args = g_newa (GIArgument, n_args);
  peas_gi_valist_to_arguments (callable_info, va_args, args, NULL);

  g_base_info_unref ((GIBaseInfo *) callable_info);

  return peas_extension_set_callv (set, method_name, args);
}

/**
 * peas_extension_set_callv:
 * @set: A #PeasExtensionSet.
 * @method_name: the name of the method that should be called.
 * @args: the arguments for the method.
 *
 * Call a method on all the #PeasExtension instances contained in @set.
 *
 * See peas_extension_callv() for more information.
 *
 * Return value: %TRUE on successful call.
 *
 * Deprecated: 1.2: Use peas_extension_set_foreach() instead.
 */
gboolean
peas_extension_set_callv (PeasExtensionSet *set,
                          const gchar      *method_name,
                          GIArgument       *args)
{
  PeasExtensionSetClass *klass;

  g_return_val_if_fail (PEAS_IS_EXTENSION_SET (set), FALSE);
  g_return_val_if_fail (method_name != NULL, FALSE);

  klass = PEAS_EXTENSION_SET_GET_CLASS (set);
  return klass->call (set, method_name, args);
}

/**
 * peas_extension_set_foreach:
 * @set: A #PeasExtensionSet.
 * @func: (scope call): A function call for each extension.
 * @data: Optional data to be passed to the function or %NULL.
 *
 * Calls @func for each #PeasExtension.
 *
 * Since: 1.2
 */
void
peas_extension_set_foreach (PeasExtensionSet            *set,
                            PeasExtensionSetForeachFunc  func,
                            gpointer                     data)
{
  PeasExtensionSetPrivate *priv = GET_PRIV (set);
  GList *l;

  g_return_if_fail (PEAS_IS_EXTENSION_SET (set));
  g_return_if_fail (func != NULL);

  for (l = priv->extensions.head; l != NULL; l = l->next)
    {
      ExtensionItem *item = (ExtensionItem *) l->data;

      func (set, item->info, item->exten, data);
    }
}

/**
 * peas_extension_set_newv: (rename-to peas_extension_set_new)
 * @engine: (allow-none): A #PeasEngine, or %NULL.
 * @exten_type: the extension #GType.
 * @n_parameters: the length of the @parameters array.
 * @parameters: (array length=n_parameters): an array of #GParameter.
 *
 * Create a new #PeasExtensionSet for the @exten_type extension type.
 *
 * If @engine is %NULL, then the default engine will be used.
 *
 * See peas_extension_set_new() for more information.
 *
 * Returns: (transfer full): a new instance of #PeasExtensionSet.
 */
PeasExtensionSet *
peas_extension_set_newv (PeasEngine *engine,
                         GType       exten_type,
                         guint       n_parameters,
                         GParameter *parameters)
{
  PeasParameterArray construct_properties = { n_parameters, parameters };

  g_return_val_if_fail (engine == NULL || PEAS_IS_ENGINE (engine), NULL);
  g_return_val_if_fail (G_TYPE_IS_INTERFACE (exten_type), NULL);

  return PEAS_EXTENSION_SET (g_object_new (PEAS_TYPE_EXTENSION_SET,
                                           "engine", engine,
                                           "extension-type", exten_type,
                                           "construct-properties", &construct_properties,
                                           NULL));
}

/**
 * peas_extension_set_new_valist: (skip)
 * @engine: (allow-none): A #PeasEngine, or %NULL.
 * @exten_type: the extension #GType.
 * @first_property: the name of the first property.
 * @var_args: the value of the first property, followed optionally by more
 *   name/value pairs, followed by %NULL.
 *
 * Create a new #PeasExtensionSet for the @exten_type extension type.
 *
 * If @engine is %NULL, then the default engine will be used.
 *
 * See peas_extension_set_new() for more information.
 *
 * Returns: a new instance of #PeasExtensionSet.
 */
PeasExtensionSet *
peas_extension_set_new_valist (PeasEngine  *engine,
                               GType        exten_type,
                               const gchar *first_property,
                               va_list      var_args)
{
  GParameter *parameters;
  guint n_parameters;
  PeasExtensionSet *set;

  g_return_val_if_fail (engine == NULL || PEAS_IS_ENGINE (engine), NULL);
  g_return_val_if_fail (G_TYPE_IS_INTERFACE (exten_type), NULL);

  if (!peas_utils_valist_to_parameter_list (exten_type, first_property,
                                            var_args, &parameters,
                                            &n_parameters))
    {
      /* Already warned */
      return NULL;
    }

  set = peas_extension_set_newv (engine, exten_type, n_parameters, parameters);

  while (n_parameters-- > 0)
    g_value_unset (&parameters[n_parameters].value);
  g_free (parameters);

  return set;
}

/**
 * peas_extension_set_new: (skip)
 * @engine: (allow-none): A #PeasEngine, or %NULL.
 * @exten_type: the extension #GType.
 * @first_property: the name of the first property.
 * @...: the value of the first property, followed optionally by more
 *   name/value pairs, followed by %NULL.
 *
 * Create a new #PeasExtensionSet for the @exten_type extension type.
 *
 * At any moment, the #PeasExtensionSet will contain an extension instance for
 * each loaded plugin which implements the @exten_type extension type. It does
 * so by connecting to the relevant signals from #PeasEngine.
 *
 * The property values passed to peas_extension_set_new() will be used for the
 * construction of new extension instances.
 *
 * If @engine is %NULL, then the default engine will be used.
 *
 * See peas_engine_create_extension() for more information.
 *
 * Returns: a new instance of #PeasExtensionSet.
 */
PeasExtensionSet *
peas_extension_set_new (PeasEngine  *engine,
                        GType        exten_type,
                        const gchar *first_property,
                        ...)
{
  va_list var_args;
  PeasExtensionSet *set;

  g_return_val_if_fail (engine == NULL || PEAS_IS_ENGINE (engine), NULL);
  g_return_val_if_fail (G_TYPE_IS_INTERFACE (exten_type), NULL);

  va_start (var_args, first_property);
  set = peas_extension_set_new_valist (engine, exten_type, first_property, var_args);
  va_end (var_args);

  return set;
}
