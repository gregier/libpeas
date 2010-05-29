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
  GList *extensions;

  gulong activate_handler_id;
  gulong deactivate_handler_id;
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

static void
add_extension (PeasExtensionSet *set,
               PeasPluginInfo   *info)
{
  PeasExtension *exten;
  ExtensionItem *item;

  /* Let's just ignore inactive plugins... */
  if (!peas_plugin_info_is_active (info))
    return;

  exten = peas_engine_get_extension (set->priv->engine, info,
                                     set->priv->exten_type);
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
peas_extension_set_set_internal_data (PeasExtensionSet *set,
                                      PeasEngine       *engine,
                                      GType             exten_type)
{
  GList *plugins, *l;

  set->priv->exten_type = exten_type;
  set->priv->engine = engine;
  g_object_ref (engine);

  plugins = (GList *) peas_engine_get_plugin_list (engine);
  for (l = plugins; l; l = l->next)
    add_extension (set, (PeasPluginInfo *) l->data);

  set->priv->activate_handler_id =
          g_signal_connect_data (engine, "activate-plugin",
                                 G_CALLBACK (add_extension), set,
                                 NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);
  set->priv->deactivate_handler_id =
          g_signal_connect_data (engine, "deactivate-plugin",
                                 G_CALLBACK (remove_extension), set,
                                 NULL, G_CONNECT_SWAPPED);
}

static void
peas_extension_set_finalize (GObject *object)
{
  PeasExtensionSet *set = PEAS_EXTENSION_SET (object);
  GList *l;

  g_signal_handler_disconnect (set->priv->engine, set->priv->activate_handler_id);
  g_signal_handler_disconnect (set->priv->engine, set->priv->deactivate_handler_id);

  for (l = set->priv->extensions; l;)
    {
      remove_extension_item (set, (ExtensionItem *) l->data);
      l = g_list_delete_link (l, l);
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
 * Return value: #TRUE on successful call.
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
 * Return value: #TRUE on successful call.
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
 * @exten_type: the extension #GType
 *
 * Create an #ExtensionSet for all the @exten_type extensions defined in
 * the plugins loaded in @engine.
 */
PeasExtensionSet *
peas_extension_set_new (PeasEngine *engine,
                        GType       exten_type)
{
  PeasExtensionSet *set;

  set = PEAS_EXTENSION_SET (g_object_new (PEAS_TYPE_EXTENSION_SET, NULL));
  peas_extension_set_set_internal_data (set, engine, exten_type);

  return set;
}
