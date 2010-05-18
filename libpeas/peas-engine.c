/*
 * peas-engine.c
 * This file is part of libpeas
 *
 * Copyright (C) 2002-2005 Paolo Maggi
 * Copyright (C) 2009 Steve Frécinaux
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

#include "peas-i18n.h"
#include "peas-engine.h"
#include "peas-plugin-info-priv.h"
#include "peas-plugin-loader.h"
#include "peas-object-module.h"
#include "peas-plugin.h"
#include "peas-extension.h"
#include "peas-dirs.h"

/**
 * SECTION:peas-engine
 * @short_description: Engine at the heart of the Peas plugin system.
 * @see_also: #PeasPluginInfo
 *
 * The #PeasEngine is the object which manages the plugins.  Its role is twofold:
 * First it will fetch all the information about the available plugins from all
 * the registered plugin directories, and second it will provide you an API to
 * load, control and unload the plugins from within your application.
 **/
G_DEFINE_TYPE (PeasEngine, peas_engine, G_TYPE_OBJECT);

/* Signals */
enum {
  ACTIVATE_PLUGIN,
  DEACTIVATE_PLUGIN,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

/* Properties */
enum {
  PROP_0,
  PROP_APP_NAME,
  PROP_BASE_MODULE_DIR,
  PROP_SEARCH_PATHS
};

typedef struct _LoaderInfo LoaderInfo;

struct _LoaderInfo {
  PeasPluginLoader *loader;
  PeasObjectModule *module;
};

struct _PeasEnginePrivate {
  gchar *app_name;
  gchar *base_module_dir;
  gchar **search_paths;

  GList *plugin_list;
  GHashTable *loaders;

  GList *object_list;
};

static void peas_engine_activate_plugin_real   (PeasEngine     *engine,
                                                PeasPluginInfo * info);
static void peas_engine_deactivate_plugin_real (PeasEngine     * engine,
                                                PeasPluginInfo * info);

static void
load_plugin_info (PeasEngine  *engine,
                  const gchar *filename,
                  const gchar *module_dir,
                  const gchar *data_dir)
{
  PeasPluginInfo *info;
  const gchar *module_name;

  info = _peas_plugin_info_new (filename,
                                engine->priv->app_name,
                                module_dir,
                                data_dir);

  if (info == NULL)
    return;

  /* If a plugin with this name has already been loaded
   * drop this one (user plugins override system plugins) */
  module_name = peas_plugin_info_get_module_name (info);
  if (peas_engine_get_plugin_info (engine, module_name) != NULL)
    _peas_plugin_info_unref (info);
  else
    engine->priv->plugin_list = g_list_prepend (engine->priv->plugin_list, info);
}

static void
load_dir_real (PeasEngine  *engine,
               const gchar *extension,
               const gchar *module_dir,
               const gchar *data_dir,
               unsigned int recursions)
{
  GError *error = NULL;
  GDir *d;
  const gchar *dirent;

  g_debug ("Loading %s/*%s...", module_dir, extension);

  d = g_dir_open (module_dir, 0, &error);

  if (!d)
    {
      g_warning ("%s", error->message);
      g_error_free (error);
      return;
    }

  while ((dirent = g_dir_read_name (d)))
    {
      gchar *filename = g_build_filename (module_dir, dirent, NULL);

      if (g_str_has_suffix (dirent, extension))
        load_plugin_info (engine, filename, module_dir, data_dir);

      else if (recursions > 0 && g_file_test (filename, G_FILE_TEST_IS_DIR))
        load_dir_real (engine, extension, filename, data_dir, recursions - 1);

      g_free (filename);
    }

  g_dir_close (d);
}

/**
 * peas_engine_rescan_plugins:
 * @engine: A #PeasEngine.
 *
 * Rescan all the registered directories to find new or updated plugins.
 *
 * Calling this function will make the newly installed plugin infos to be
 * loaded by the engine, so the new plugins can actually be used without
 * restarting the application.
 */
void
peas_engine_rescan_plugins (PeasEngine *engine)
{
  gchar *extension;
  guint i;
  gchar **sp;

  g_return_if_fail (PEAS_IS_ENGINE (engine));

  /* Compute the extension of the plugin files. */
  extension = g_strdup_printf (".%s-plugin", engine->priv->app_name);
  for (i = 0; extension[i] != '\0'; ++i)
    extension[i] = g_ascii_tolower (extension[i]);

  /* Go and read everything from the provided search paths */
  sp = engine->priv->search_paths;
  for (i = 0; sp[i] != NULL; i += 2)
    load_dir_real (engine, extension, sp[i], sp[i + 1], 1);

  g_free (extension);
}

static guint
hash_lowercase (gconstpointer data)
{
  gchar *lowercase;
  guint ret;

  lowercase = g_ascii_strdown ((const gchar *) data, -1);
  ret = g_str_hash (lowercase);
  g_free (lowercase);

  return ret;
}

static gboolean
equal_lowercase (gconstpointer a,
                 gconstpointer b)
{
  return g_ascii_strcasecmp ((const gchar *) a, (const gchar *) b) == 0;
}

static void
loader_destroy (LoaderInfo *info)
{
  if (!info)
    return;

  if (info->loader)
    g_object_unref (info->loader);

  g_free (info);
}

static LoaderInfo *
add_loader (PeasEngine       *engine,
            const gchar      *loader_id,
            PeasObjectModule *module,
            PeasPluginLoader *loader)
{
  LoaderInfo *info;

  info = g_new (LoaderInfo, 1);
  info->loader = loader;
  info->module = module;

  g_hash_table_insert (engine->priv->loaders, g_strdup (loader_id), info);
  return info;
}

static void
peas_engine_init (PeasEngine *engine)
{
  if (!g_module_supported ())
    {
      g_warning ("libpeas is not able to initialize the plugins engine.");
      return;
    }

  engine->priv = G_TYPE_INSTANCE_GET_PRIVATE (engine,
                                              PEAS_TYPE_ENGINE,
                                              PeasEnginePrivate);

  engine->priv->object_list = NULL;

  /* mapping from loadername -> loader object */
  engine->priv->loaders = g_hash_table_new_full (hash_lowercase,
                                                 equal_lowercase,
                                                 (GDestroyNotify) g_free,
                                                 (GDestroyNotify) loader_destroy);
}

static void
peas_engine_constructed (GObject *object)
{
  peas_engine_rescan_plugins (PEAS_ENGINE (object));
}

static void
loader_garbage_collect (const gchar *id,
                        LoaderInfo  *info)
{
  if (info->loader)
    peas_plugin_loader_garbage_collect (info->loader);
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
  g_hash_table_foreach (engine->priv->loaders,
                        (GHFunc) loader_garbage_collect,
                        NULL);
}

static void
peas_engine_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  PeasEngine *engine = PEAS_ENGINE (object);

  switch (prop_id)
    {
    case PROP_APP_NAME:
      engine->priv->app_name = g_value_dup_string (value);
      break;
    case PROP_BASE_MODULE_DIR:
      engine->priv->base_module_dir = g_value_dup_string (value);
      break;
    case PROP_SEARCH_PATHS:
      engine->priv->search_paths = g_value_dup_boxed (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_engine_get_property (GObject   *object,
                          guint      prop_id,
                          GValue    *value,
                          GParamSpec *pspec)
{
  PeasEngine *engine = PEAS_ENGINE (object);

  switch (prop_id)
    {
    case PROP_APP_NAME:
      g_value_set_string (value, engine->priv->app_name);
      break;
    case PROP_BASE_MODULE_DIR:
      g_value_set_string (value, engine->priv->base_module_dir);
      break;
    case PROP_SEARCH_PATHS:
      g_value_set_boxed (value, engine->priv->search_paths);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
peas_engine_finalize (GObject *object)
{
  PeasEngine *engine = PEAS_ENGINE (object);
  GList *item;

  /* First deactivate all plugins */
  for (item = engine->priv->plugin_list; item; item = item->next)
    {
      PeasPluginInfo *info = PEAS_PLUGIN_INFO (item->data);

      if (peas_plugin_info_is_active (info))
        peas_engine_deactivate_plugin_real (engine, info);
    }

  /* unref the loaders */
  g_hash_table_destroy (engine->priv->loaders);

  /* and finally free the infos */
  for (item = engine->priv->plugin_list; item; item = item->next)
    _peas_plugin_info_unref (PEAS_PLUGIN_INFO (item->data));

  g_list_free (engine->priv->plugin_list);
  g_list_free (engine->priv->object_list);
  g_strfreev (engine->priv->search_paths);
  g_free (engine->priv->app_name);
  g_free (engine->priv->base_module_dir);

  G_OBJECT_CLASS (peas_engine_parent_class)->finalize (object);
}

static void
peas_engine_class_init (PeasEngineClass *klass)
{
  GType the_type = G_TYPE_FROM_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = peas_engine_set_property;
  object_class->get_property = peas_engine_get_property;
  object_class->constructed = peas_engine_constructed;
  object_class->finalize = peas_engine_finalize;
  klass->activate_plugin = peas_engine_activate_plugin_real;
  klass->deactivate_plugin = peas_engine_deactivate_plugin_real;

  /**
   * PeasEngine:app-name:
   *
   * The application name. Filename extension and section header for
   * #PeasPluginInfo files are actually derived from this value.
   *
   * For instance, if app-name is "Gedit", then info files will have
   * the .gedit-plugin extension, and the engine will look for a
   * "Gedit Plugin" section in it to load the plugin data.
   */
  g_object_class_install_property (object_class, PROP_APP_NAME,
                                   g_param_spec_string ("app-name",
                                                        "Application Name",
                                                        "The application name",
                                                        "Peas",
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));

  /**
   * PeasEngine:base-module-dir:
   *
   * The base application directory for binding modules lookup.
   *
   * Each loader module will load its modules from a subdirectory of
   * the base module directory. For instance, the python loader will
   * look for python modules in "${base-module-dir}/python/".
   */
  g_object_class_install_property (object_class, PROP_BASE_MODULE_DIR,
                                   g_param_spec_string ("base-module-dir",
                                                        "Base module dir",
                                                        "The base application dir for binding modules lookup",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));

  /**
   * PeasEngine:search-paths:
   *
   * The list of path where to look for plugins.
   *
   * A so-called "search path" actually consists on a couple of both a
   * module directory (where the shared libraries or language modules
   * lie) and a data directory (where the plugin data is).
   *
   * The #PeasPlugin will be able to use a correct data dir depending on
   * where it is installed, hence allowing to keep the plugin agnostic
   * when it comes to installation location: the same plugin can be
   * installed both in the system path or in the user's home directory,
   * without taking other special care than using
   * egg_plugin_get_data_dir() when looking for its data files.
   *
   * Concretely, this property contains a NULL-terminated array of
   * strings, whose even-indexed strings are module directories, and
   * odd-indexed ones are the associated data directories.  Here is an
   * example of such an array:
   * |[
   * gchar const * const search_paths[] = {
   *         // Plugins in ~/.config
   *         g_build_filename (g_get_user_config_dir (), "example/plugins", NULL),
   *         g_build_filename (g_get_user_config_dir (), "example/plugins", NULL),
   *         // System plugins
   *         EXAMPLE_PREFIX "/lib/example/plugins/",
   *         EXAMPLE_PREFIX "/share/example/plugins/",
   *         NULL
   * };
   * ]|
   */
  g_object_class_install_property (object_class, PROP_SEARCH_PATHS,
                                   g_param_spec_boxed ("search-paths",
                                                       "Search paths",
                                                       "The list of paths where to look for plugins",
                                                       G_TYPE_STRV,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY |
                                                       G_PARAM_STATIC_STRINGS));

  /**
   * PeasEngine::activate-plugin:
   * @engine: A #PeasEngine.
   * @info: A #PeasPluginInfo.
   *
   * The activate-plugin signal is emitted when a plugin is being
   * activated.
   */
  signals[ACTIVATE_PLUGIN] =
    g_signal_new ("activate-plugin",
                  the_type,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PeasEngineClass, activate_plugin),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOXED,
                  G_TYPE_NONE,
                  1,
                  PEAS_TYPE_PLUGIN_INFO |
                  G_SIGNAL_TYPE_STATIC_SCOPE);

  /**
   * PeasEngine::deactivate-plugin:
   * @engine: A #PeasEngine.
   * @info: A #PeasPluginInfo.
   *
   * The activate-plugin signal is emitted when a plugin is being
   * deactivated.
   */
  signals[DEACTIVATE_PLUGIN] =
    g_signal_new ("deactivate-plugin",
                  the_type,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PeasEngineClass, deactivate_plugin),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOXED,
                  G_TYPE_NONE,
                  1, PEAS_TYPE_PLUGIN_INFO |
                  G_SIGNAL_TYPE_STATIC_SCOPE);

  g_type_class_add_private (klass, sizeof (PeasEnginePrivate));
}

static LoaderInfo *
load_plugin_loader (PeasEngine  *engine,
                    const gchar *loader_id)
{
  gchar *loader_dirname;
  gchar *loader_basename;
  guint i;
  PeasObjectModule *module;
  PeasPluginLoader *loader;

  /* Let's build the expected filename of the requested plugin loader */
  loader_dirname = peas_dirs_get_plugin_loaders_dir ();
  loader_basename = g_strdup_printf ("lib%sloader.%s", loader_id, G_MODULE_SUFFIX);
  for (i = 0; loader_basename[i] != '\0'; ++i)
    loader_basename[i] = g_ascii_tolower (loader_basename[i]);

  g_debug ("Loading loader '%s': '%s/%s'", loader_id, loader_dirname, loader_basename);

  /* For now all modules are resident */
  module = peas_object_module_new (loader_basename,
                                   loader_dirname,
                                   "register_peas_plugin_loader",
                                   TRUE);

  g_free (loader_dirname);
  g_free (loader_basename);

  /* make sure to load the type definition */
  if (g_type_module_use (G_TYPE_MODULE (module)))
    {
      loader = (PeasPluginLoader *) peas_object_module_new_object (module);

      if (loader == NULL || !PEAS_IS_PLUGIN_LOADER (loader))
        {
          g_warning ("Loader object is not a valid PeasPluginLoader instance");
          if (loader != NULL && G_IS_OBJECT (loader))
            g_object_unref (loader);
          module = NULL;
          loader = NULL;
        }
      else
        {
          gchar *module_dir = g_build_filename (engine->priv->base_module_dir, loader_id, NULL);
          peas_plugin_loader_add_module_directory (loader, module_dir);
          g_free (module_dir);
        }

      g_type_module_unuse (G_TYPE_MODULE (module));
    }
  else
    {
      g_warning ("Plugin loader module `%s' could not be loaded",
                 loader_basename);
      g_object_unref (module);
      module = NULL;
      loader = NULL;
    }

  return add_loader (engine, loader_id, module, loader);
}

static PeasPluginLoader *
get_plugin_loader (PeasEngine     *engine,
                   PeasPluginInfo *info)
{
  const gchar *loader_id;
  LoaderInfo *loader_info;

  loader_id = info->loader;

  loader_info = (LoaderInfo *) g_hash_table_lookup (engine->priv->loaders,
                                                    loader_id);

  if (loader_info == NULL)
    loader_info = load_plugin_loader (engine, loader_id);

  return loader_info->loader;
}

/**
 * peas_engine_get_plugin_list:
 * @engine: A #PeasEngine.
 *
 * Returns the list of #PeasPluginInfo known to the engine.
 *
 * Returns: a #GList of #PeasPluginInfo. Note that the list
 * belongs to the engine and should not be freed.
 **/
const GList *
peas_engine_get_plugin_list (PeasEngine *engine)
{
  return engine->priv->plugin_list;
}

static gint
compare_plugin_info_and_name (PeasPluginInfo *info,
                              const gchar    *module_name)
{
  return strcmp (peas_plugin_info_get_module_name (info), module_name);
}

/**
 * peas_engine_get_plugin_info:
 * @engine: A #PeasEngine.
 * @plugin_name: A plugin name.
 *
 * Returns: the #PeasPluginInfo corresponding with a given plugin name.
 */
PeasPluginInfo *
peas_engine_get_plugin_info (PeasEngine  *engine,
                             const gchar *plugin_name)
{
  GList *l = g_list_find_custom (engine->priv->plugin_list,
                                 plugin_name,
                                 (GCompareFunc) compare_plugin_info_and_name);

  return l == NULL ? NULL : (PeasPluginInfo *) l->data;
}

static gboolean
load_plugin (PeasEngine     *engine,
             PeasPluginInfo *info)
{
  PeasPluginLoader *loader;

  if (peas_plugin_info_is_active (info))
    return TRUE;

  if (!peas_plugin_info_is_available (info))
    return FALSE;

  loader = get_plugin_loader (engine, info);

  if (loader == NULL)
    {
      g_warning ("Could not find loader `%s' for plugin `%s'",
                 info->loader, info->name);
      info->available = FALSE;
      return FALSE;
    }

  info->active = peas_plugin_loader_load (loader, info);

  if (info->active == FALSE)
    {
      g_warning ("Error loading plugin '%s'", info->name);
      info->available = FALSE;
      return FALSE;
    }

  return TRUE;
}

static void
peas_engine_activate_plugin_real (PeasEngine     *engine,
                                  PeasPluginInfo *info)
{
  load_plugin (engine, info);
}

/**
 * peas_engine_activate_plugin:
 * @engine: A #PeasEngine.
 * @info: A #PeasPluginInfo.
 *
 * Activates the plugin corresponding to @info on all the objects registered
 * against @engine, loading it if it's not already available.
 *
 * Returns: whether the plugin has been successfuly activated.
 */
gboolean
peas_engine_activate_plugin (PeasEngine     *engine,
                             PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, FALSE);

  if (!peas_plugin_info_is_available (info))
    return FALSE;

  if (peas_plugin_info_is_active (info))
    return TRUE;

  g_signal_emit (engine, signals[ACTIVATE_PLUGIN], 0, info);

  return peas_plugin_info_is_active (info);
}

static void
peas_engine_deactivate_plugin_real (PeasEngine     *engine,
                                    PeasPluginInfo *info)
{
  PeasPluginLoader *loader;

  if (!peas_plugin_info_is_active (info) ||
      !peas_plugin_info_is_available (info))
    return;

  /* find the loader and tell it to gc and unload the plugin */
  loader = get_plugin_loader (engine, info);

  peas_plugin_loader_garbage_collect (loader);
  peas_plugin_loader_unload (loader, info);

  info->active = FALSE;
}

/**
 * peas_engine_deactivate_plugin:
 * @engine: A #PeasEngine.
 * @info: A #PeasPluginInfo.
 *
 * Deactivates the plugin corresponding to @info on all the objects registered
 * against @engine, eventually unloading it when it has been completely
 * deactivated.
 *
 * Returns: whether the plugin has been successfuly deactivated.
 */
gboolean
peas_engine_deactivate_plugin (PeasEngine     *engine,
                               PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, FALSE);

  if (!peas_plugin_info_is_active (info))
    return TRUE;

  g_signal_emit (engine, signals[DEACTIVATE_PLUGIN], 0, info);

  return !peas_plugin_info_is_active (info);
}

gboolean
peas_engine_provides_extension (PeasEngine *engine,
                                PeasPluginInfo *info,
                                GType extension_type)
{
  PeasPluginLoader *loader;

  g_return_val_if_fail (PEAS_IS_ENGINE (engine), FALSE);
  g_return_val_if_fail (info != NULL, FALSE);

  loader = get_plugin_loader (engine, info);
  return peas_plugin_loader_provides_extension (loader, info, extension_type);
}

PeasExtension *
peas_engine_get_extension (PeasEngine *engine,
                           PeasPluginInfo *info,
                           GType extension_type)
{
  PeasPluginLoader *loader;

  g_return_val_if_fail (PEAS_IS_ENGINE (engine), NULL);
  g_return_val_if_fail (info != NULL, NULL);

  loader = get_plugin_loader (engine, info);
  return peas_plugin_loader_get_extension (loader, info, extension_type);
}

/**
 * peas_engine_get_active_plugins:
 * @engine: A #PeasEngine.
 *
 * Returns the list of the names of all the active plugins, or %NULL if there
 * is no active plugin. Please note that the returned array is a newly
 * allocated one: you will need to free it using g_strfreev().
 *
 * Returns: A newly-allocated NULL-terminated array of strings, or %NULL.
 */
gchar **
peas_engine_get_active_plugins (PeasEngine *engine)
{
  GArray *array = g_array_new (TRUE, FALSE, sizeof (gchar *));
  GList *pl;

  for (pl = engine->priv->plugin_list; pl; pl = pl->next)
    {
      PeasPluginInfo *info = (PeasPluginInfo *) pl->data;
      gchar *module_name;

      if (peas_plugin_info_is_active (info))
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
  for (i = 0; haystack[i] != NULL; i++)
    if (strcmp (haystack[i], needle) == 0)
      return TRUE;
  return FALSE;
}

/**
 * peas_engine_set_active_plugins:
 * @engine: A #PeasEngine.
 * @plugin_names: A NULL-terminated array of plugin names.
 *
 * Sets the list of active plugins for @engine. When this function is called,
 * the #PeasEngine will activate all the plugins whose names are in
 * @plugin_names, and deactivate all other active plugins.
 */
void
peas_engine_set_active_plugins (PeasEngine   *engine,
                                const gchar **plugin_names)
{
  GList *pl;

  for (pl = engine->priv->plugin_list; pl; pl = pl->next)
    {
      PeasPluginInfo *info = (PeasPluginInfo *) pl->data;
      const gchar *module_name;
      gboolean is_active;
      gboolean to_activate;

      if (!peas_plugin_info_is_available (info))
        continue;

      module_name = peas_plugin_info_get_module_name (info);
      is_active = peas_plugin_info_is_active (info);

      to_activate = string_in_strv (module_name, plugin_names);

      if (!is_active && to_activate)
        g_signal_emit (engine, signals[ACTIVATE_PLUGIN], 0, info);
      else if (is_active && !to_activate)
        g_signal_emit (engine, signals[DEACTIVATE_PLUGIN], 0, info);
    }
}

/**
 * peas_engine_new:
 * @app_name: The name of the app
 * @base_module_dir: The base directory for language modules
 * @search_paths: The paths where to look for plugins
 *
 * Returns a new #PeasEngine object.
 * See the properties description for more information about the parameters.
 *
 * Returns: a newly created #PeasEngine object.
 */
PeasEngine *
peas_engine_new (const gchar  *app_name,
                 const gchar  *base_module_dir,
                 const gchar **search_paths)
{
  return PEAS_ENGINE (g_object_new (PEAS_TYPE_ENGINE,
                                    "app-name", app_name,
                                    "base-module-dir", base_module_dir,
                                    "search-paths", search_paths,
                                    NULL));
}
