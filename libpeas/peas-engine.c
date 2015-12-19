/*
 * peas-engine.c
 * This file is part of libpeas
 *
 * Copyright (C) 2002-2005 Paolo Maggi
 * Copyright (C) 2009 Steve Frécinaux
 * Copyright (C) 2010-2014 Garrett Regier
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

#include "peas-i18n.h"
#include "peas-engine.h"
#include "peas-engine-priv.h"
#include "peas-plugin-info-priv.h"
#include "peas-plugin-loader.h"
#include "peas-plugin-loader-c.h"
#include "peas-object-module.h"
#include "peas-extension.h"
#include "peas-dirs.h"
#include "peas-debug.h"
#include "peas-utils.h"

/**
 * SECTION:peas-engine
 * @short_description: Engine at the heart of the Peas plugin system.
 * @see_also: #PeasPluginInfo
 *
 * The #PeasEngine is the object which manages the plugins.
 *
 * Its role is twofold:
 * <itemizedlist>
 *   <listitem>
 *     <para>it will fetch all the information about the available plugins
 *     from all the registered plugin directories;</para>
 *   </listitem>
 *   <listitem>
 *     <para>it will provide you an API to load, control and unload your
 *     plugins and their extensions from within your application.</para>
 *   </listitem>
 * </itemizedlist>
 **/

/* Signals */
enum {
  LOAD_PLUGIN,
  UNLOAD_PLUGIN,
  LAST_SIGNAL
};

/* Properties */
enum {
  PROP_0,
  PROP_PLUGIN_LIST,
  PROP_LOADED_PLUGINS,
  PROP_NONGLOBAL_LOADERS,
  N_PROPERTIES
};

static guint signals[LAST_SIGNAL];
static GParamSpec *properties[N_PROPERTIES] = { NULL };

typedef struct _GlobalLoaderInfo {
  PeasPluginLoader *loader;
  PeasObjectModule *module;

  guint enabled : 1;
  guint failed : 1;
} GlobalLoaderInfo;

typedef struct _LoaderInfo {
  PeasPluginLoader *loader;

  guint enabled : 1;
  guint failed : 1;
} LoaderInfo;

typedef struct _SearchPath {
  gchar *module_dir;
  gchar *data_dir;
} SearchPath;

struct _PeasEnginePrivate {
  LoaderInfo loaders[PEAS_UTILS_N_LOADERS];

  GQueue search_paths;
  GQueue plugin_list;

  guint in_dispose : 1;
  guint use_nonglobal_loaders : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (PeasEngine, peas_engine, G_TYPE_OBJECT)

#define GET_PRIV(o) \
  (peas_engine_get_instance_private (o))

static gboolean shutdown = FALSE;
static PeasEngine *default_engine = NULL;

static GMutex loaders_lock;
static GlobalLoaderInfo loaders[PEAS_UTILS_N_LOADERS];

static void peas_engine_load_plugin_real   (PeasEngine     *engine,
                                            PeasPluginInfo *info);
static void peas_engine_unload_plugin_real (PeasEngine     *engine,
                                            PeasPluginInfo *info);

static void
plugin_info_add_sorted (GQueue         *plugin_list,
                        PeasPluginInfo *info)
{
  guint i;
  GList *furthest_dep = NULL;
  const gchar **dependencies;

  dependencies = peas_plugin_info_get_dependencies (info);

  for (i = 0; dependencies[i] != NULL; ++i)
    {
      GList *pos = furthest_dep != NULL ? furthest_dep : plugin_list->head;

      for (; pos != NULL; pos = pos->next)
        {
          if (strcmp (dependencies[i],
                      peas_plugin_info_get_module_name (pos->data)) == 0)
            {
              furthest_dep = pos;
              break;
            }
        }
    }

  /* GLib changed only accepts NULL for
   * g_queue_insert_after() at version 2.44
   */
  if (furthest_dep == NULL)
    {
      g_queue_push_head (plugin_list, info);
      return;
    }

  g_debug ("Adding '%s' after '%s' due to dependencies",
           peas_plugin_info_get_module_name (info),
           peas_plugin_info_get_module_name (furthest_dep->data));

  g_queue_insert_after (plugin_list, furthest_dep, info);
}

static gboolean
load_plugin_info (PeasEngine  *engine,
                  const gchar *filename,
                  const gchar *module_dir,
                  const gchar *data_dir)
{
  PeasEnginePrivate *priv = GET_PRIV (engine);
  PeasPluginInfo *info;
  const gchar *module_name;

  info = _peas_plugin_info_new (filename,
                                module_dir,
                                data_dir);

  if (info == NULL)
    {
      g_warning ("Error loading '%s'", filename);
      return FALSE;
    }

  module_name = peas_plugin_info_get_module_name (info);
  if (peas_engine_get_plugin_info (engine, module_name) != NULL)
    {
      _peas_plugin_info_unref (info);
      return FALSE;
    }

  plugin_info_add_sorted (&priv->plugin_list, info);
  g_object_notify_by_pspec (G_OBJECT (engine),
                            properties[PROP_PLUGIN_LIST]);

  return TRUE;
}

static gboolean
load_file_dir_real (PeasEngine  *engine,
                    const gchar *module_dir,
                    const gchar *data_dir,
                    guint        recursions)
{
  GDir *d;
  const gchar *dirent;
  GError *error = NULL;
  gboolean found = FALSE;

  g_debug ("Loading %s/*.plugin...", module_dir);

  d = g_dir_open (module_dir, 0, &error);

  if (!d)
    {
      g_debug ("%s", error->message);
      g_error_free (error);
      return FALSE;
    }

  while ((dirent = g_dir_read_name (d)))
    {
      gchar *filename = g_build_filename (module_dir, dirent, NULL);

      if (g_file_test (filename, G_FILE_TEST_IS_DIR))
        {
          if (recursions > 0)
            {
              found |= load_file_dir_real (engine, filename,
                                           data_dir, recursions - 1);
            }
        }
      else if (g_str_has_suffix (dirent, ".plugin"))
        {
          found |= load_plugin_info (engine, filename,
                                     module_dir, data_dir);
        }

      g_free (filename);
    }

  g_dir_close (d);

  return found;
}

static gboolean
load_resource_dir_real (PeasEngine  *engine,
                        const gchar *module_dir,
                        const gchar *data_dir,
                        guint        recursions)
{
  guint i;
  const gchar *module_path;
  gchar **children;
  GError *error = NULL;
  gboolean found = FALSE;

  g_debug ("Loading %s/*.plugin...", module_dir);

  module_path = module_dir + strlen ("resource://");
  children = g_resources_enumerate_children (module_path,
                                             G_RESOURCE_LOOKUP_FLAGS_NONE,
                                             &error);

  if (error != NULL)
    {
      g_debug ("%s", error->message);
      g_error_free (error);
      return FALSE;
    }

  for (i = 0; children[i] != NULL; ++i)
    {
      gboolean is_dir;
      gchar *child;

      is_dir = g_str_has_suffix (children[i], "/");

      if (is_dir && recursions == 0)
        continue;

      if (!is_dir && !g_str_has_suffix (children[i], ".plugin"))
        continue;

      child = g_build_path ("/", module_dir, children[i], NULL);

      if (is_dir)
        {
          found |= load_resource_dir_real (engine, child,
                                           data_dir, recursions - 1);
        }
      else
        {
          found |= load_plugin_info (engine, child, module_dir, data_dir);
        }

      g_free (child);
    }

  g_strfreev (children);

  return found;
}

static gboolean
load_dir_real (PeasEngine *engine,
               SearchPath *sp)
{
  if (!g_str_has_prefix (sp->module_dir, "resource://"))
    return load_file_dir_real (engine, sp->module_dir, sp->data_dir, 1);

  return load_resource_dir_real (engine, sp->module_dir, sp->data_dir, 1);
}

static void
plugin_list_changed (PeasEngine *engine)
{
  PeasEnginePrivate *priv = GET_PRIV (engine);
  GString *msg;
  GList *pos;

  if (g_getenv ("PEAS_DEBUG") == NULL)
    return;

  msg = g_string_new ("Plugins: ");

  for (pos = priv->plugin_list.head; pos != NULL; pos = pos->next)
    {
      if (pos->prev != NULL)
        g_string_append (msg, ", ");

      g_string_append (msg, peas_plugin_info_get_module_name (pos->data));
    }

  g_debug ("%s", msg->str);
  g_string_free (msg, TRUE);
}

/**
 * peas_engine_rescan_plugins:
 * @engine: A #PeasEngine.
 *
 * Rescan all the registered directories to find new or updated plugins.
 *
 * Calling this function will make the newly installed plugin infos
 * be loaded by the engine, so the new plugins can be used without
 * restarting the application.
 */
void
peas_engine_rescan_plugins (PeasEngine *engine)
{
  PeasEnginePrivate *priv = GET_PRIV (engine);
  GList *item;
  gboolean found = FALSE;

  g_return_if_fail (PEAS_IS_ENGINE (engine));

  if (priv->search_paths.length == 0)
    {
      g_debug ("No search paths where provided");
      return;
    }

  g_object_freeze_notify (G_OBJECT (engine));

  /* Go and read everything from the provided search paths */
  for (item = priv->search_paths.head; item != NULL; item = item->next)
    found |= load_dir_real (engine, (SearchPath *) item->data);

  if (found)
    plugin_list_changed (engine);

  g_object_thaw_notify (G_OBJECT (engine));
}

static void
peas_engine_insert_search_path (PeasEngine  *engine,
                                gboolean     prepend,
                                const gchar *module_dir,
                                const gchar *data_dir)
{
  PeasEnginePrivate *priv = GET_PRIV (engine);
  SearchPath *sp;

  g_return_if_fail (PEAS_IS_ENGINE (engine));
  g_return_if_fail (module_dir != NULL);

  sp = g_slice_new (SearchPath);
  sp->module_dir = g_strdup (module_dir);
  sp->data_dir = g_strdup (data_dir ? data_dir : module_dir);

  if (prepend)
    g_queue_push_head (&priv->search_paths, sp);
  else
    g_queue_push_tail (&priv->search_paths, sp);

  g_object_freeze_notify (G_OBJECT (engine));

  if (load_dir_real (engine, sp))
    plugin_list_changed (engine);

  g_object_thaw_notify (G_OBJECT (engine));
}

/**
 * peas_engine_add_search_path:
 * @engine: A #PeasEngine.
 * @module_dir: the plugin module directory.
 * @data_dir: (allow-none): the plugin data directory.
 *
 * This function appends a search path to the list of paths where to
 * look for plugins.
 *
 * A so-called "search path" actually consists of both a
 * module directory (where the shared libraries or language modules
 * lie) and a data directory (where the plugin data is).
 *
 * The plugin will be able to use a correct data dir depending on
 * where it is installed, hence allowing to keep the plugin agnostic
 * when it comes to installation location: the same plugin can be
 * installed either in the system path or in the user's home directory,
 * without taking other special care than using
 * peas_plugin_info_get_data_dir() when looking for its data files.
 *
 * If @data_dir is %NULL, then it is set to the same value as
 * @module_dir.
 */
void
peas_engine_add_search_path (PeasEngine  *engine,
                             const gchar *module_dir,
                             const gchar *data_dir)
{
  peas_engine_insert_search_path (engine, FALSE, module_dir, data_dir);
}

/**
 * peas_engine_prepend_search_path:
 * @engine: A #PeasEngine.
 * @module_dir: the plugin module directory.
 * @data_dir: (allow-none): the plugin data directory.
 *
 * This function prepends a search path to the list of paths where to
 * look for plugins.
 *
 * See Also: peas_engine_add_search_path()
 *
 * Since: 1.6
 */
void
peas_engine_prepend_search_path (PeasEngine  *engine,
                                 const gchar *module_dir,
                                 const gchar *data_dir)
{
  peas_engine_insert_search_path (engine, TRUE, module_dir, data_dir);
}

static void
default_engine_weak_notify (gpointer    unused,
                            PeasEngine *engine)
{
  g_warn_if_fail (g_atomic_pointer_compare_and_exchange (&default_engine,
                                                         engine, NULL));
}

static void
peas_engine_init (PeasEngine *engine)
{
  PeasEnginePrivate *priv = GET_PRIV (engine);

  /* Don't need to use atomics as peas_engine_shutdown()
   * is private API and as such is not multithread-safe
   */
  if (shutdown)
    {
      g_error ("libpeas cannot create a plugin engine "
               "as it has been shutdown.");
    }

  /* Set the default engine here to make sure that if a
   * plugin is loaded and calls peas_engine_get_default()
   * that this engine is returned and not another.
   *
   * While peas_engine_get_default() is not thread-safe
   * we are using atomics here to try and prevent bad
   * things from happening.
   */
  if (g_atomic_pointer_compare_and_exchange (&default_engine, NULL, engine))
    {
      g_object_weak_ref (G_OBJECT (engine),
                         (GWeakNotify) default_engine_weak_notify,
                         NULL);
    }

  priv->in_dispose = FALSE;

  g_queue_init (&priv->search_paths);
  g_queue_init (&priv->plugin_list);

  /* The C plugin loader is always enabled */
  priv->loaders[PEAS_UTILS_C_LOADER_ID].enabled = TRUE;
}

/**
 * peas_engine_garbage_collect:
 * @engine: A #PeasEngine.
 *
 * This function triggers garbage collection on all the loaders currently
 * owned by the #PeasEngine.  This can be used to force the loaders to destroy
 * managed objects that still hold references to objects that are about to
 * disappear.
 */
void
peas_engine_garbage_collect (PeasEngine *engine)
{
  PeasEnginePrivate *priv = GET_PRIV (engine);
  gint i;

  g_return_if_fail (PEAS_IS_ENGINE (engine));

  for (i = 0; i < G_N_ELEMENTS (loaders); ++i)
    {
      LoaderInfo *loader_info = &priv->loaders[i];

      if (loader_info->loader != NULL)
        peas_plugin_loader_garbage_collect (loader_info->loader);
    }
}

static void
peas_engine_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  PeasEngine *engine = PEAS_ENGINE (object);
  PeasEnginePrivate *priv = GET_PRIV (engine);

  switch (prop_id)
    {
    case PROP_LOADED_PLUGINS:
      peas_engine_set_loaded_plugins (engine,
                                      (const gchar **) g_value_get_boxed (value));
      break;
    case PROP_NONGLOBAL_LOADERS:
      priv->use_nonglobal_loaders = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_engine_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  PeasEngine *engine = PEAS_ENGINE (object);
  PeasEnginePrivate *priv = GET_PRIV (engine);

  switch (prop_id)
    {
    case PROP_PLUGIN_LIST:
      g_value_set_pointer (value,
                           (gpointer) peas_engine_get_plugin_list (engine));
      break;
    case PROP_LOADED_PLUGINS:
      g_value_take_boxed (value,
                          (gconstpointer) peas_engine_get_loaded_plugins (engine));
      break;
    case PROP_NONGLOBAL_LOADERS:
      g_value_set_boolean (value, priv->use_nonglobal_loaders);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_engine_dispose (GObject *object)
{
  PeasEngine *engine = PEAS_ENGINE (object);
  PeasEnginePrivate *priv = GET_PRIV (engine);
  GList *item;
  gint i;

  /* See peas_engine_unload_plugin_real() */
  priv->in_dispose = TRUE;

  /* First unload all the plugins */
  for (item = priv->plugin_list.tail; item != NULL; item = item->prev)
    {
      PeasPluginInfo *info = PEAS_PLUGIN_INFO (item->data);

      if (peas_plugin_info_is_loaded (info))
        peas_engine_unload_plugin (engine, info);
    }

  /* Then destroy the plugin loaders */
  for (i = 0; i < G_N_ELEMENTS (priv->loaders); ++i)
    {
      LoaderInfo *loader_info = &priv->loaders[i];

      g_clear_object (&loader_info->loader);
    }

  G_OBJECT_CLASS (peas_engine_parent_class)->dispose (object);
}

static void
peas_engine_finalize (GObject *object)
{
  PeasEngine *engine = PEAS_ENGINE (object);
  PeasEnginePrivate *priv = GET_PRIV (engine);
  GList *item;

  /* free the infos */
  for (item = priv->plugin_list.head; item != NULL; item = item->next)
    {
      PeasPluginInfo *info = (PeasPluginInfo *) item->data;

      _peas_plugin_info_unref (info);
    }

  /* free the search path list */
  for (item = priv->search_paths.head; item != NULL; item = item->next)
    {
      SearchPath *sp = (SearchPath *) item->data;

      g_free (sp->module_dir);
      g_free (sp->data_dir);
      g_slice_free (SearchPath, sp);
    }

  g_queue_clear (&priv->search_paths);
  g_queue_clear (&priv->plugin_list);

  G_OBJECT_CLASS (peas_engine_parent_class)->finalize (object);
}

static void
peas_engine_class_init (PeasEngineClass *klass)
{
  GType the_type = G_TYPE_FROM_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = peas_engine_set_property;
  object_class->get_property = peas_engine_get_property;
  object_class->dispose = peas_engine_dispose;
  object_class->finalize = peas_engine_finalize;

  klass->load_plugin = peas_engine_load_plugin_real;
  klass->unload_plugin = peas_engine_unload_plugin_real;

  /**
   * PeasEngine:plugin-list:
   *
   * The list of found plugins.
   *
   * This will be modified when peas_engine_rescan_plugins() is called.
   *
   * Note: the list belongs to the engine and should not be modified or freed.
   */
  properties[PROP_PLUGIN_LIST] =
    g_param_spec_pointer ("plugin-list",
                          "Plugin list",
                          "The list of found plugins",
                          G_PARAM_READABLE |
                          G_PARAM_STATIC_STRINGS);

  /**
   * PeasEngine:loaded-plugins:
   *
   * The list of loaded plugins.
   *
   * This will be modified when peas_engine_load_plugin() or
   * peas_engine_unload_plugin() is called.
   *
   * This can be used with GSettings to save the loaded plugins by binding
   * to this property after instantiating the engine by doing:
   * |[
   *   g_settings_bind (gsettings_object,
   *                    LOADED_PLUGINS_KEY,
   *                    engine,
   *                    "loaded-plugins",
   *                    G_SETTINGS_BIND_DEFAULT);
   * ]|
   *
   * Note: notify will not be called when the engine is being destroyed.
   */
  properties[PROP_LOADED_PLUGINS] =
    g_param_spec_boxed ("loaded-plugins",
                        "Loaded plugins",
                        "The list of loaded plugins",
                        G_TYPE_STRV,
                        G_PARAM_READWRITE |
                        G_PARAM_STATIC_STRINGS);

  /**
   * PeasEngine:nonglobal-loaders:
   *
   * If non-global plugin loaders should be used.
   *
   * See peas_engine_new_with_nonglobal_loaders() for more information.
   *
   * Since: 1.14
   */
  properties[PROP_NONGLOBAL_LOADERS] =
    g_param_spec_boolean ("nonglobal-loaders",
                          "Non-global Loaders",
                          "Use non-global plugin loaders",
                          FALSE,
                          G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS);

  /**
   * PeasEngine::load-plugin:
   * @engine: A #PeasEngine.
   * @info: A #PeasPluginInfo.
   *
   * The load-plugin signal is emitted when a plugin is being loaded.
   *
   * The plugin is being loaded in the default handler. Hence, if you want to
   * perform some action before the plugin is loaded, you should use
   * g_signal_connect(), but if you want to perform some action *after* the
   * plugin is loaded (the most common case), you should use
   * g_signal_connect_after().
   */
  signals[LOAD_PLUGIN] =
    g_signal_new (I_("load-plugin"),
                  the_type,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PeasEngineClass, load_plugin),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOXED,
                  G_TYPE_NONE,
                  1,
                  PEAS_TYPE_PLUGIN_INFO |
                  G_SIGNAL_TYPE_STATIC_SCOPE);

  /**
   * PeasEngine::unload-plugin:
   * @engine: A #PeasEngine.
   * @info: A #PeasPluginInfo.
   *
   * The unload-plugin signal is emitted when a plugin is being unloaded.
   *
   * The plugin is being unloaded in the default handler. Hence, if you want
   * to perform some action before the plugin is unloaded (the most common
   * case), you should use g_signal_connect(), but if you want to perform some
   * action after the plugin is unloaded (the most common case), you should
   * use g_signal_connect_after().
   */
  signals[UNLOAD_PLUGIN] =
    g_signal_new (I_("unload-plugin"),
                  the_type,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PeasEngineClass, unload_plugin),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOXED,
                  G_TYPE_NONE,
                  1, PEAS_TYPE_PLUGIN_INFO |
                  G_SIGNAL_TYPE_STATIC_SCOPE);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);

  /* We don't support calling PeasEngine API without module support */
  if (!g_module_supported ())
    {
      g_error ("libpeas is not able to create the "
               "plugins engine as modules are not supported.");
    }

  /* We are doing some global initialization here as there is currently no
   * global init function for libpeas. */
  peas_debug_init ();

  /* This cannot be done as a compile-time
   * assert, but is critical for correct behavior
   */
  g_assert (g_strcmp0 (peas_utils_get_loader_from_id (PEAS_UTILS_C_LOADER_ID),
                       "c") == 0);

  /* The C plugin loader is always enabled */
  loaders[PEAS_UTILS_C_LOADER_ID].enabled = TRUE;
}

static PeasObjectModule *
get_plugin_loader_module (gint loader_id)
{
  GlobalLoaderInfo *global_loader_info = &loaders[loader_id];
  const gchar *loader_name, *module_name;
  gchar *module_dir;

  if (global_loader_info->module != NULL)
    return global_loader_info->module;

  loader_name = peas_utils_get_loader_from_id (loader_id);
  module_name = peas_utils_get_loader_module_from_id (loader_id);
  module_dir = peas_dirs_get_plugin_loader_dir (loader_name);

  /* Bind loaders globally, binding
   * locally can break the plugin loaders
   */
  global_loader_info->module = peas_object_module_new_full (module_name,
                                                            module_dir,
                                                            TRUE, FALSE);

  if (!g_type_module_use (G_TYPE_MODULE (global_loader_info->module)))
    {
      g_warning ("Could not load plugin loader '%s'", loader_name);
      g_clear_object (&global_loader_info->module);
    }

  g_free (module_dir);

  return global_loader_info->module;
}

static PeasPluginLoader *
create_plugin_loader (gint loader_id)
{
  PeasPluginLoader *loader;

  if (loader_id == PEAS_UTILS_C_LOADER_ID)
    {
      loader = peas_plugin_loader_c_new ();
    }
  else
    {
      PeasObjectModule *module;

      module = get_plugin_loader_module (loader_id);
      if (module == NULL)
        return NULL;

      loader = PEAS_PLUGIN_LOADER (
            peas_object_module_create_object (module,
                                              PEAS_TYPE_PLUGIN_LOADER,
                                              0, NULL));
    }

  if (loader == NULL || !peas_plugin_loader_initialize (loader))
    {
      g_warning ("Loader '%s' is not a valid PeasPluginLoader instance",
                 peas_utils_get_loader_from_id (loader_id));
      g_clear_object (&loader);
    }

  return loader;
}

static PeasPluginLoader *
get_local_plugin_loader (PeasEngine *engine,
                         gint        loader_id)
{
  PeasEnginePrivate *priv = GET_PRIV (engine);
  GlobalLoaderInfo *global_loader_info = &loaders[loader_id];
  PeasPluginLoader *loader;

  if (global_loader_info->failed)
    return NULL;

  if (global_loader_info->loader != NULL &&
      (!priv->use_nonglobal_loaders ||
       peas_plugin_loader_is_global (global_loader_info->loader)))
    {
      return g_object_ref (global_loader_info->loader);
    }

  loader = create_plugin_loader (loader_id);

  if (loader == NULL)
    {
      global_loader_info->failed = TRUE;
      return NULL;
    }

  if (!priv->use_nonglobal_loaders ||
      peas_plugin_loader_is_global (loader))
    {
      global_loader_info->loader = g_object_ref (loader);
    }

  return loader;
}

static PeasPluginLoader *
get_plugin_loader (PeasEngine *engine,
                   gint        loader_id)
{
  PeasEnginePrivate *priv = GET_PRIV (engine);
  LoaderInfo *loader_info = &priv->loaders[loader_id];
  GlobalLoaderInfo *global_loader_info = &loaders[loader_id];

  if (loader_info->loader != NULL || loader_info->failed)
    return loader_info->loader;

  g_mutex_lock (&loaders_lock);

  if (!loader_info->enabled)
    {
      if (!global_loader_info->enabled)
        {
          g_warning ("The '%s' plugin loader has not been enabled",
                     peas_utils_get_loader_from_id (loader_id));

          g_mutex_unlock (&loaders_lock);
          return NULL;
        }

      g_warning ("The '%s' plugin loader was not enabled "
                 "for this engine. This will no longer be "
                 "supported at some point in the future!",
                 peas_utils_get_loader_from_id (loader_id));

      g_mutex_unlock (&loaders_lock);

      /* Avoid bypassing logic in peas_engine_enable_loader() */
      peas_engine_enable_loader (engine,
                                 peas_utils_get_loader_from_id (loader_id));
      return get_plugin_loader (engine, loader_id);
    }

  loader_info->loader = get_local_plugin_loader (engine, loader_id);

  if (loader_info->loader == NULL)
    loader_info->failed = TRUE;

  g_mutex_unlock (&loaders_lock);
  return loader_info->loader;
}

/**
 * peas_engine_enable_loader:
 * @engine: A #PeasEngine.
 * @loader_name: The name of the loader to enable.
 *
 * Enable a loader, enables a loader for plugins.
 * The C plugin loader is always enabled. The other plugin
 * loaders are: lua5.1, python and python3.
 *
 * For instance, the following code will enable Python 2 plugins
 * to be loaded:
 * |[
 * peas_engine_enable_loader (engine, "python");
 * ]|
 *
 * Note: plugin loaders used to be shared across #PeasEngines so enabling
 *       a loader on one #PeasEngine would enable it on all #PeasEngines.
 *       This behavior has been kept to avoid breaking applications,
 *       however a warning has been added to help applications transition.
 **/
void
peas_engine_enable_loader (PeasEngine  *engine,
                           const gchar *loader_name)
{
  PeasEnginePrivate *priv = GET_PRIV (engine);
  LoaderInfo *loader_info;
  gint loader_id;

  g_return_if_fail (PEAS_IS_ENGINE (engine));
  g_return_if_fail (loader_name != NULL && *loader_name != '\0');

  loader_id = peas_utils_get_loader_id (loader_name);

  if (loader_id == -1)
    {
      g_warning ("Failed to enable unknown plugin loader '%s'", loader_name);
      return;
    }

  loader_info = &priv->loaders[loader_id];
  if (loader_info->enabled || loader_info->failed)
    return;

  g_mutex_lock (&loaders_lock);

  /* Don't check if the loader failed
   * as we want to warn multiple times
   */
  if (loaders[loader_id].enabled)
    {
      loader_info->enabled = TRUE;
      g_mutex_unlock (&loaders_lock);
      return;
    }

  /* Some tests check for mixed versions this way */
  if (g_getenv ("PEAS_ALLOW_CONFLICTING_LOADERS") == NULL)
    {
      gint i;
      const gint *loader_ids;

      loader_ids = peas_utils_get_conflicting_loaders_from_id (loader_id);

      /* Some loaders conflict with each other
       * and cannot be used in the same process
       */
      for (i = 0; loader_ids[i] != -1; ++i)
        {
          if (!loaders[loader_ids[i]].enabled)
            continue;

          g_warning ("Cannot enable plugin loader '%s' as the "
                     "'%s' plugin loader is already enabled.", loader_name,
                     peas_utils_get_loader_from_id (loader_ids[i]));

          loader_info->failed = TRUE;
          loaders[loader_id].failed = TRUE;
          g_mutex_unlock (&loaders_lock);
          return;
        }
    }

  /* We do not load the plugin loader immediately and instead
   * load it in get_plugin_loader() so that it is loaded lazily.
   */
  loader_info->enabled = TRUE;
  loaders[loader_id].enabled = TRUE;

  g_mutex_unlock (&loaders_lock);
}

/**
 * peas_engine_get_plugin_list:
 * @engine: A #PeasEngine.
 *
 * Returns the list of #PeasPluginInfo known to the engine.
 *
 * Returns: (transfer none) (element-type Peas.PluginInfo): a #GList of
 * #PeasPluginInfo. Note that the list belongs to the engine and should
 * not be freed.
 **/
const GList *
peas_engine_get_plugin_list (PeasEngine *engine)
{
  PeasEnginePrivate *priv = GET_PRIV (engine);

  g_return_val_if_fail (PEAS_IS_ENGINE (engine), NULL);

  return priv->plugin_list.head;
}

/**
 * peas_engine_get_plugin_info:
 * @engine: A #PeasEngine.
 * @plugin_name: A plugin name.
 *
 * Gets the #PeasPluginInfo corresponding with @plugin_name,
 * or %NULL if @plugin_name was not found.
 *
 * Returns: (transfer none): the #PeasPluginInfo corresponding with
 * a given plugin module name.
 */
PeasPluginInfo *
peas_engine_get_plugin_info (PeasEngine  *engine,
                             const gchar *plugin_name)
{
  PeasEnginePrivate *priv = GET_PRIV (engine);
  GList *l;

  g_return_val_if_fail (PEAS_IS_ENGINE (engine), NULL);
  g_return_val_if_fail (plugin_name != NULL, NULL);

  for (l = priv->plugin_list.head; l != NULL; l = l->next)
    {
      PeasPluginInfo *info = (PeasPluginInfo *) l->data;
      const gchar *module_name = peas_plugin_info_get_module_name (info);

      if (strcmp (module_name, plugin_name) == 0)
        return info;
    }

  return NULL;
}

static void
peas_engine_load_plugin_real (PeasEngine     *engine,
                              PeasPluginInfo *info)
{
  const gchar **dependencies;
  PeasPluginInfo *dep_info;
  guint i;
  PeasPluginLoader *loader;

  if (peas_plugin_info_is_loaded (info))
    return;

  if (!peas_plugin_info_is_available (info, NULL))
    return;

  /* We set the plugin info as loaded before trying to load the dependencies,
   * to make sure we won't have an infinite loop. */
  info->loaded = TRUE;

  dependencies = peas_plugin_info_get_dependencies (info);
  for (i = 0; dependencies[i] != NULL; i++)
    {
      dep_info = peas_engine_get_plugin_info (engine, dependencies[i]);
      if (!dep_info)
        {
          g_warning ("Could not find plugin '%s' for plugin '%s'",
                     dependencies[i], peas_plugin_info_get_module_name (info));
          g_set_error (&info->error,
                       PEAS_PLUGIN_INFO_ERROR,
                       PEAS_PLUGIN_INFO_ERROR_DEP_NOT_FOUND,
                       _("Dependency '%s' was not found"),
                       dependencies[i]);
          goto error;
        }

      if (!peas_engine_load_plugin (engine, dep_info))
        {
          g_set_error (&info->error,
                       PEAS_PLUGIN_INFO_ERROR,
                       PEAS_PLUGIN_INFO_ERROR_LOADING_FAILED,
                       _("Dependency '%s' failed to load"),
                       peas_plugin_info_get_name (dep_info));
          goto error;
        }
    }

  loader = get_plugin_loader (engine, info->loader_id);

  if (loader == NULL)
    {
      /* Already warned */
      g_set_error (&info->error,
                   PEAS_PLUGIN_INFO_ERROR,
                   PEAS_PLUGIN_INFO_ERROR_LOADER_NOT_FOUND,
                   _("Plugin loader '%s' was not found"),
                   peas_utils_get_loader_from_id (info->loader_id));
      goto error;
    }

  if (!peas_plugin_loader_load (loader, info))
    {
      g_warning ("Error loading plugin '%s'",
                 peas_plugin_info_get_module_name (info));
      g_set_error (&info->error,
                   PEAS_PLUGIN_INFO_ERROR,
                   PEAS_PLUGIN_INFO_ERROR_LOADING_FAILED,
                   _("Failed to load"));
      goto error;
    }

  g_debug ("Loaded plugin '%s'", peas_plugin_info_get_module_name (info));

  g_object_notify_by_pspec (G_OBJECT (engine),
                            properties[PROP_LOADED_PLUGINS]);

  return;

error:

  info->loaded = FALSE;
  info->available = FALSE;
}

/**
 * peas_engine_load_plugin: (virtual load_plugin)
 * @engine: A #PeasEngine.
 * @info: A #PeasPluginInfo.
 *
 * Loads the plugin corresponding to @info if it's not currently loaded.
 * Emits the "load-plugin" signal; loading the plugin
 * actually occurs in the default signal handler.
 *
 * Returns: whether the plugin has been successfully loaded.
 */
gboolean
peas_engine_load_plugin (PeasEngine     *engine,
                         PeasPluginInfo *info)
{
  g_return_val_if_fail (PEAS_IS_ENGINE (engine), FALSE);
  g_return_val_if_fail (info != NULL, FALSE);

  if (peas_plugin_info_is_loaded (info))
    return TRUE;

  if (!peas_plugin_info_is_available (info, NULL))
    return FALSE;

  g_signal_emit (engine, signals[LOAD_PLUGIN], 0, info);

  return peas_plugin_info_is_loaded (info);
}

static void
peas_engine_unload_plugin_real (PeasEngine     *engine,
                                PeasPluginInfo *info)
{
  PeasEnginePrivate *priv = GET_PRIV (engine);
  GList *item;
  const gchar *module_name;
  PeasPluginLoader *loader;

  if (!peas_plugin_info_is_loaded (info))
    return;

  /* We set the plugin info as unloaded before trying to unload the
   * dependants, to make sure we won't have an infinite loop. */
  info->loaded = FALSE;

  /* First unload all the dependant plugins */
  module_name = peas_plugin_info_get_module_name (info);
  for (item = priv->plugin_list.tail; item != NULL; item = item->prev)
    {
      PeasPluginInfo *other_info = PEAS_PLUGIN_INFO (item->data);

      if (!peas_plugin_info_is_loaded (other_info))
        continue;

      if (peas_plugin_info_has_dependency (other_info, module_name))
         peas_engine_unload_plugin (engine, other_info);
    }

  /* find the loader and tell it to gc and unload the plugin */
  loader = get_plugin_loader (engine, info->loader_id);

  peas_plugin_loader_garbage_collect (loader);
  peas_plugin_loader_unload (loader, info);

  g_debug ("Unloaded plugin '%s'", peas_plugin_info_get_module_name (info));

  /* Don't notify while in dispose so the
   * loaded plugins can easily be kept in GSettings
   */
  if (!priv->in_dispose)
    g_object_notify_by_pspec (G_OBJECT (engine),
                              properties[PROP_LOADED_PLUGINS]);
}

/**
 * peas_engine_unload_plugin: (virtual unload_plugin)
 * @engine: A #PeasEngine.
 * @info: A #PeasPluginInfo.
 *
 * Unloads the plugin corresponding to @info.
 * Emits the "unload-plugin" signal; unloading the plugin
 * actually occurs in the default signal handler.
 *
 * Returns: whether the plugin has been successfully unloaded.
 */
gboolean
peas_engine_unload_plugin (PeasEngine     *engine,
                           PeasPluginInfo *info)
{
  g_return_val_if_fail (PEAS_IS_ENGINE (engine), FALSE);
  g_return_val_if_fail (info != NULL, FALSE);

  if (!peas_plugin_info_is_loaded (info))
    return TRUE;

  g_signal_emit (engine, signals[UNLOAD_PLUGIN], 0, info);

  return !peas_plugin_info_is_loaded (info);
}

/**
 * peas_engine_provides_extension:
 * @engine: A #PeasEngine.
 * @info: A #PeasPluginInfo.
 * @extension_type: The extension #GType.
 *
 * Returns if @info provides an extension for @extension_type.
 * If the @info is not loaded than %FALSE will always be returned.
 *
 * Returns: if @info provides an extension for @extension_type.
 */
gboolean
peas_engine_provides_extension (PeasEngine     *engine,
                                PeasPluginInfo *info,
                                GType           extension_type)
{
  PeasPluginLoader *loader;

  g_return_val_if_fail (PEAS_IS_ENGINE (engine), FALSE);
  g_return_val_if_fail (info != NULL, FALSE);
  g_return_val_if_fail (G_TYPE_IS_INTERFACE (extension_type), FALSE);

  if (!peas_plugin_info_is_loaded (info))
    return FALSE;

  loader = get_plugin_loader (engine, info->loader_id);
  return peas_plugin_loader_provides_extension (loader, info, extension_type);
}

/**
 * peas_engine_create_extensionv: (rename-to peas_engine_create_extension)
 * @engine: A #PeasEngine.
 * @info: A loaded #PeasPluginInfo.
 * @extension_type: The implemented extension #GType.
 * @n_parameters: the length of the @parameters array.
 * @parameters: (allow-none) (array length=n_parameters):
 *   an array of #GParameter.
 *
 * If the plugin identified by @info implements the @extension_type interface,
 * then this function will return a new instance of this implementation,
 * wrapped in a new #PeasExtension instance. Otherwise, it will return %NULL.
 *
 * See peas_engine_create_extension() for more information.
 *
 * Returns: (transfer full): a new instance of #PeasExtension wrapping
 * the @extension_type instance, or %NULL.
 */
PeasExtension *
peas_engine_create_extensionv (PeasEngine     *engine,
                               PeasPluginInfo *info,
                               GType           extension_type,
                               guint           n_parameters,
                               GParameter     *parameters)
{
  PeasPluginLoader *loader;
  PeasExtension *extension;

  g_return_val_if_fail (PEAS_IS_ENGINE (engine), NULL);
  g_return_val_if_fail (info != NULL, NULL);
  g_return_val_if_fail (peas_plugin_info_is_loaded (info), NULL);
  g_return_val_if_fail (G_TYPE_IS_INTERFACE (extension_type), FALSE);

  loader = get_plugin_loader (engine, info->loader_id);
  extension = peas_plugin_loader_create_extension (loader, info, extension_type,
                                                   n_parameters, parameters);

  if (!G_TYPE_CHECK_INSTANCE_TYPE (extension, extension_type))
    {
      g_warning ("Plugin '%s' does not provide a '%s' extension",
                 peas_plugin_info_get_module_name (info),
                 g_type_name (extension_type));
      return NULL;
    }

  return extension;
}

/**
 * peas_engine_create_extension_valist: (skip)
 * @engine: A #PeasEngine.
 * @info: A loaded #PeasPluginInfo.
 * @extension_type: The implemented extension #GType.
 * @first_property: the name of the first property.
 * @var_args: the value of the first property, followed optionally by more
 *   name/value pairs, followed by %NULL.
 *
 * If the plugin identified by @info implements the @extension_type interface,
 * then this function will return a new instance of this implementation,
 * wrapped in a new #PeasExtension instance. Otherwise, it will return %NULL.
 *
 * See peas_engine_create_extension() for more information.
 *
 * Returns: a new instance of #PeasExtension wrapping
 * the @extension_type instance, or %NULL.
 */
PeasExtension *
peas_engine_create_extension_valist (PeasEngine     *engine,
                                     PeasPluginInfo *info,
                                     GType           extension_type,
                                     const gchar    *first_property,
                                     va_list         var_args)
{
  guint n_parameters;
  GParameter *parameters;
  PeasExtension *exten;

  g_return_val_if_fail (PEAS_IS_ENGINE (engine), NULL);
  g_return_val_if_fail (info != NULL, NULL);
  g_return_val_if_fail (peas_plugin_info_is_loaded (info), NULL);
  g_return_val_if_fail (G_TYPE_IS_INTERFACE (extension_type), FALSE);

  if (!peas_utils_valist_to_parameter_list (extension_type, first_property,
                                            var_args, &parameters,
                                            &n_parameters))
    {
      /* Already warned */
      return NULL;
    }

  exten = peas_engine_create_extensionv (engine, info, extension_type,
                                         n_parameters, parameters);

  while (n_parameters-- > 0)
    g_value_unset (&parameters[n_parameters].value);
  g_free (parameters);

  return exten;
}

/**
 * peas_engine_create_extension: (skip)
 * @engine: A #PeasEngine.
 * @info: A loaded #PeasPluginInfo.
 * @extension_type: The implemented extension #GType.
 * @first_property: the name of the first property.
 * @...: the value of the first property, followed optionally by more
 *   name/value pairs, followed by %NULL.
 *
 * If the plugin identified by @info implements the @extension_type interface,
 * then this function will return a new instance of this implementation,
 * wrapped in a new #PeasExtension instance. Otherwise, it will return %NULL.
 *
 * When creating the new instance of the @extension_type subtype, the
 * provided construct properties will be passed to the extension construction
 * handler (exactly like if you had called g_object_new() yourself).
 *
 * The new extension instance produced by this function will always be
 * returned wrapped in a #PeasExtension proxy, following the current libpeas
 * principle of never giving you the actual object (also because it might as
 * well *not* be an actual object).
 *
 * Returns: a new instance of #PeasExtension wrapping
 * the @extension_type instance, or %NULL.
 */
PeasExtension *
peas_engine_create_extension (PeasEngine     *engine,
                              PeasPluginInfo *info,
                              GType           extension_type,
                              const gchar    *first_property,
                              ...)
{
  va_list var_args;
  PeasExtension *exten;

  g_return_val_if_fail (PEAS_IS_ENGINE (engine), NULL);
  g_return_val_if_fail (info != NULL, NULL);
  g_return_val_if_fail (peas_plugin_info_is_loaded (info), NULL);
  g_return_val_if_fail (G_TYPE_IS_INTERFACE (extension_type), FALSE);

  va_start (var_args, first_property);
  exten = peas_engine_create_extension_valist (engine, info, extension_type,
                                               first_property, var_args);
  va_end (var_args);

  return exten;
}

/**
 * peas_engine_get_loaded_plugins:
 * @engine: A #PeasEngine.
 *
 * Returns the list of the names of all the loaded plugins, or an array
 * containing a single %NULL element if there is no plugin currently loaded.
 *
 * Please note that the returned array is a newly allocated one: you will need
 * to free it using g_strfreev().
 *
 * Returns: (transfer full) (array zero-terminated=1): A newly-allocated
 * %NULL-terminated array of strings.
 */
gchar **
peas_engine_get_loaded_plugins (PeasEngine *engine)
{
  PeasEnginePrivate *priv = GET_PRIV (engine);
  GArray *array;
  GList *pl;

  g_return_val_if_fail (PEAS_IS_ENGINE (engine), NULL);

  array = g_array_new (TRUE, FALSE, sizeof (gchar *));

  for (pl = priv->plugin_list.head; pl != NULL; pl = pl->next)
    {
      PeasPluginInfo *info = (PeasPluginInfo *) pl->data;
      gchar *module_name;

      if (peas_plugin_info_is_loaded (info))
        {
          module_name = g_strdup (peas_plugin_info_get_module_name (info));
          g_array_append_val (array, module_name);
        }
    }

  return (gchar **) g_array_free (array, FALSE);
}

static gboolean
string_in_strv (const gchar  *needle,
                const gchar **haystack)
{
  guint i;

  if (haystack == NULL)
    return FALSE;

  for (i = 0; haystack[i] != NULL; i++)
    {
      if (strcmp (haystack[i], needle) == 0)
        return TRUE;
    }

  return FALSE;
}

/**
 * peas_engine_set_loaded_plugins:
 * @engine: A #PeasEngine.
 * @plugin_names: (allow-none) (array zero-terminated=1): A %NULL-terminated
 *  array of plugin names, or %NULL.
 *
 * Sets the list of loaded plugins for @engine. When this function is called,
 * the #PeasEngine will load all the plugins whose names are in @plugin_names,
 * and ensures all other active plugins are unloaded.
 *
 * If @plugin_names is %NULL, all plugins will be unloaded.
 */
void
peas_engine_set_loaded_plugins (PeasEngine   *engine,
                                const gchar **plugin_names)
{
  PeasEnginePrivate *priv = GET_PRIV (engine);
  GList *pl;

  g_return_if_fail (PEAS_IS_ENGINE (engine));

  for (pl = priv->plugin_list.head; pl != NULL; pl = pl->next)
    {
      PeasPluginInfo *info = (PeasPluginInfo *) pl->data;
      const gchar *module_name;
      gboolean is_loaded;
      gboolean to_load;

      if (!peas_plugin_info_is_available (info, NULL))
        continue;

      module_name = peas_plugin_info_get_module_name (info);
      is_loaded = peas_plugin_info_is_loaded (info);

      to_load = string_in_strv (module_name, plugin_names);

      if (!is_loaded && to_load)
        g_signal_emit (engine, signals[LOAD_PLUGIN], 0, info);
      else if (is_loaded && !to_load)
        g_signal_emit (engine, signals[UNLOAD_PLUGIN], 0, info);
    }
}

/**
 * peas_engine_new:
 *
 * Return a new instance of #PeasEngine.
 * If no default #PeasEngine has been instantiated yet,
 * the first call of this function will set the default
 * engine as the new instance of #PeasEngine.
 *
 * Returns: a new instance of #PeasEngine.
 */
PeasEngine *
peas_engine_new (void)
{
  return PEAS_ENGINE (g_object_new (PEAS_TYPE_ENGINE, NULL));
}

/**
 * peas_engine_new_with_nonglobal_loaders:
 *
 * Return a new instance of #PeasEngine which will use non-global
 * plugin loaders instead of the default global ones. This allows
 * multiple threads to each have a #PeasEngine and be used without
 * internal locking.
 *
 * Note: due to CPython's GIL the python and python3
 *       plugin loaders are always global.
 *
 * Returns: a new instance of #PeasEngine that uses non-global loaders.
 *
 * Since: 1.14
 */
PeasEngine *
peas_engine_new_with_nonglobal_loaders (void)
{
  return PEAS_ENGINE (g_object_new (PEAS_TYPE_ENGINE,
                                    "nonglobal-loaders", TRUE,
                                    NULL));
}

/**
 * peas_engine_get_default:
 *
 * Return the existing instance of #PeasEngine or a subclass of it.
 * If no #PeasEngine subclass has been instantiated yet, the first call
 * of this function will return a new instance of #PeasEngine.
 *
 * Note: this function should never be used when multiple threads are
 *       using libpeas API as it is not thread-safe.
 *
 * Returns: (transfer none): the existing instance of #PeasEngine.
 */
PeasEngine *
peas_engine_get_default (void)
{
  /* peas_engine_new() will cause the default to be set for us */
  if (default_engine == NULL)
    {
      PeasEngine *engine = peas_engine_new ();

      if (engine != default_engine)
        {
          g_warning ("peas_engine_get_default() must not be called when "
                     "multiple threads are using libpeas API");
          g_object_unref (engine);
        }
    }

  return default_engine;
}

/* < private >
 * peas_engine_shutdown:
 *
 * Frees memory shared by PeasEngines.
 * No libpeas API should be called after calling this.
 *
 * Note: this is private API and as such is not thread-safe.
 */
void
peas_engine_shutdown (void)
{
  gint i;

  if (shutdown)
    return;

  shutdown = TRUE;

  g_mutex_lock (&loaders_lock);

  for (i = 0; i < G_N_ELEMENTS (loaders); ++i)
    {
      GlobalLoaderInfo *loader_info = &loaders[i];

      if (loader_info->loader != NULL)
        {
          g_object_add_weak_pointer (G_OBJECT (loader_info->loader),
                                     (gpointer *) &loader_info->loader);

          g_object_unref (loader_info->loader);
          g_assert (loader_info->loader == NULL);
        }

      /* Don't bother unloading the
       * module as it is always resident
       */
      loader_info->enabled = FALSE;
      loader_info->failed = TRUE;
    }

  g_mutex_unlock (&loaders_lock);
}
