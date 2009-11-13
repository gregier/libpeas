/*
 * gpe-engine.c
 * This file is part of libgpe
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

#include <glib/gi18n.h>

#include "gpe-engine.h"
#include "gpe-plugin-info-priv.h"
#include "gpe-plugin-loader.h"
#include "gpe-object-module.h"
#include "gpe-plugin.h"
#include "gpe-dirs.h"

/**
 * SECTION:gpe-engine
 * @short_description: Engine at the heart of the GPE plugin system.
 * @see_also: #GPEPluginInfo
 *
 * The #GPEEngine is the object which manages the plugins.  Its role is twofold:
 * First it will fetch all the information about the available plugins from all
 * the registered plugin directories, and second it will provide you an API to
 * load, control and unload the plugins from within your application.
 **/

typedef struct
{
	GPEPluginLoader *loader;
	GPEObjectModule *module;
} LoaderInfo;

/* Signals */
enum
{
	ACTIVATE_PLUGIN,
	DEACTIVATE_PLUGIN,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

/* Properties */
enum
{
	PROP_0,
	PROP_APP_NAME,
	PROP_BASE_MODULE_DIR
};

G_DEFINE_TYPE(GPEEngine, gpe_engine, G_TYPE_OBJECT)

struct _GPEEnginePrivate
{
	GPtrArray *pathinfos;
	gchar *app_name;
	gchar *base_module_dir;

	GList *plugin_list;
	GHashTable *loaders;

	GList *object_list;
};

static void	gpe_engine_activate_plugin_real		 (GPEEngine     *engine,
							  GPEPluginInfo *info);
static void	gpe_engine_deactivate_plugin_real	 (GPEEngine     *engine,
							  GPEPluginInfo *info);

static void
load_plugin_info (GPEEngine          *engine,
		  const gchar        *filename,
		  const GPEPathInfo  *pathinfo)
{
	GPEPluginInfo *info;

	info = _gpe_plugin_info_new (filename, pathinfo, engine->priv->app_name);

	if (info == NULL)
		return;

	/* If a plugin with this name has already been loaded
	 * drop this one (user plugins override system plugins) */
	if (gpe_engine_get_plugin_info (engine, gpe_plugin_info_get_module_name (info)) != NULL)
		_gpe_plugin_info_unref (info);
	else
		engine->priv->plugin_list = g_list_prepend (engine->priv->plugin_list, info);
}

static void
load_dir_real (GPEEngine         *engine,
	       const GPEPathInfo *pathinfo)
{
	GError *error = NULL;
	GDir *d;
	const gchar *dirent;
	gchar *plugin_extension;

	/* Compute the extension of the plugin files. */
	plugin_extension = g_strdup_printf (".%s-plugin", engine->priv->app_name);
	g_strdown (plugin_extension);

	g_debug ("Loading %s/*.%s...", pathinfo->module_dir, plugin_extension);

	d = g_dir_open (pathinfo->module_dir, 0, &error);
	if (!d)
	{
		g_warning ("%s", error->message);
		g_error_free (error);
		return;
	}

	while ((dirent = g_dir_read_name (d)))
	{
		gchar *filename;

		if (!g_str_has_suffix (dirent, plugin_extension))
			continue;

		filename = g_build_filename (pathinfo->module_dir, dirent, NULL);

		load_plugin_info (engine, filename, pathinfo);

		g_free (filename);
	}

	g_dir_close (d);
	g_free (plugin_extension);
}

/**
 * gpe_engine_add_plugin_directory:
 * @engine: A #GPEEngine.
 * @module_dir: The new module directory.
 * @data_dir: The new data directory.
 *
 * Add a directory to the list of directories where the plugins are to be
 * found.  A "directory" actually consists on a couple of both a module
 * directory (where the shared libraries or language modules are) and a data
 * directory (where the plugin data is).
 *
 * The #GPEPlugin will be able to use get a correct @data_dir depending on
 * where it is installing, hence allowing to keep the plugin agnostic when it
 * comes to installation location (the same plugin can be installed both in
 * the system path or in the user's home directory).
 */
void
gpe_engine_add_plugin_directory	(GPEEngine      *engine,
				 const gchar    *module_dir,
				 const gchar    *data_dir)
{
	GPEPathInfo *pathinfo = g_slice_new (GPEPathInfo);
	pathinfo->module_dir = g_strdup (module_dir);
	pathinfo->data_dir = g_strdup (data_dir);
	g_ptr_array_add (engine->priv->pathinfos, pathinfo);

	load_dir_real (engine, pathinfo);
}

/**
 * gpe_engine_rescan_plugins:
 * @engine: A #GPEEngine.
 *
 * Rescan all the registered directories to find new or updated plugins.
 *
 * Calling this function will make the newly installed plugin infos to be
 * loaded by the engine, so the new plugins can actually be used without
 * restarting the application.
 */
void
gpe_engine_rescan_plugins (GPEEngine *engine)
{
	guint i;

	g_return_if_fail (GPE_IS_ENGINE (engine));

	for (i = 0; i < engine->priv->pathinfos->len; i++)
		load_dir_real (engine, (GPEPathInfo *) engine->priv->pathinfos->pdata[i]);
}

static guint
hash_lowercase (gconstpointer data)
{
	gchar *lowercase;
	guint ret;

	lowercase = g_ascii_strdown ((const gchar *)data, -1);
	ret = g_str_hash (lowercase);
	g_free (lowercase);

	return ret;
}

static gboolean
equal_lowercase (gconstpointer a, gconstpointer b)
{
	return g_ascii_strcasecmp ((const gchar *)a, (const gchar *)b) == 0;
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
add_loader (GPEEngine        *engine,
	    const gchar      *loader_id,
	    GPEObjectModule  *module,
	    GPEPluginLoader  *loader)
{
	LoaderInfo *info;

	info = g_new (LoaderInfo, 1);
	info->loader = loader;
	info->module = module;

	g_hash_table_insert (engine->priv->loaders, g_strdup (loader_id), info);
	return info;
}

static void
gpe_path_info_free (GPEPathInfo *pathinfo)
{
	g_free (pathinfo->module_dir);
	if (pathinfo->data_dir)
		g_free (pathinfo->data_dir);
	g_slice_free (GPEPathInfo, pathinfo);
}

static void
gpe_engine_init (GPEEngine *engine)
{
	if (!g_module_supported ())
	{
		g_warning ("libgpe is not able to initialize the plugins engine.");
		return;
	}

	engine->priv = G_TYPE_INSTANCE_GET_PRIVATE (engine,
						    GPE_TYPE_ENGINE,
						    GPEEnginePrivate);

	engine->priv->object_list = NULL;

	/* mapping from loadername -> loader object */
	engine->priv->loaders = g_hash_table_new_full (hash_lowercase,
						       equal_lowercase,
						       (GDestroyNotify)g_free,
						       (GDestroyNotify)loader_destroy);

	/* path infos for plugin loading. */
	engine->priv->pathinfos = g_ptr_array_new ();
	g_ptr_array_set_free_func (engine->priv->pathinfos,
				   (GDestroyNotify) gpe_path_info_free);
}

static void
loader_garbage_collect (const char *id, LoaderInfo *info)
{
	if (info->loader)
		gpe_plugin_loader_garbage_collect (info->loader);
}

/**
 * gpe_engine_garbage_collect:
 * @engine: A #GPEEngine.
 *
 * This function triggers garbage collection on all the loaders currently
 * owned by the #GPEEngine.  This can be used to force the loaders to destroy
 * managed objects that still hold references to objects that are about to
 * disappear.
 */
void
gpe_engine_garbage_collect (GPEEngine *engine)
{
	g_hash_table_foreach (engine->priv->loaders,
			      (GHFunc) loader_garbage_collect,
			      NULL);
}

static void
gpe_engine_set_property (GObject      *object,
			 guint         prop_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	GPEEngine *engine = GPE_ENGINE (object);

        switch (prop_id)
        {
		case PROP_APP_NAME:
			engine->priv->app_name = g_value_dup_string (value);
			break;
		case PROP_BASE_MODULE_DIR:
			engine->priv->base_module_dir = g_value_dup_string (value);
			break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

static void
gpe_engine_get_property (GObject    *object,
			 guint       prop_id,
			 GValue     *value,
			 GParamSpec *pspec)
{
	GPEEngine *engine = GPE_ENGINE (object);

        switch (prop_id)
        {
		case PROP_APP_NAME:
			g_value_set_string (value, engine->priv->app_name);
			break;
		case PROP_BASE_MODULE_DIR:
			g_value_set_string (value, engine->priv->base_module_dir);
			break;
                default:
                        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                        break;
        }
}

static void
gpe_engine_finalize (GObject *object)
{
	GPEEngine *engine = GPE_ENGINE (object);
	GList *item;

	/* First deactivate all plugins */
	for (item = engine->priv->plugin_list; item; item = item->next)
	{
		GPEPluginInfo *info = GPE_PLUGIN_INFO (item->data);

		if (gpe_plugin_info_is_active (info))
			gpe_engine_deactivate_plugin_real (engine, info);
	}

	/* unref the loaders */
	g_hash_table_destroy (engine->priv->loaders);

	/* and finally free the infos */
	for (item = engine->priv->plugin_list; item; item = item->next)
	{
		GPEPluginInfo *info = GPE_PLUGIN_INFO (item->data);

		_gpe_plugin_info_unref (info);
	}

	g_list_free (engine->priv->plugin_list);
	g_list_free (engine->priv->object_list);
	g_ptr_array_free (engine->priv->pathinfos, TRUE);
	g_free (engine->priv->app_name);
	g_free (engine->priv->base_module_dir);

	G_OBJECT_CLASS (gpe_engine_parent_class)->finalize (object);
}

static void
gpe_engine_activate_plugin_on_object (GPEEngine     *engine,
				      GPEPluginInfo *info,
				      GObject       *object)
{
	gpe_plugin_activate (info->plugin, object);
}

static void
gpe_engine_deactivate_plugin_on_object (GPEEngine     *engine,
					GPEPluginInfo *info,
					GObject       *object)
{
	gpe_plugin_deactivate (info->plugin, object);
}

static void
gpe_engine_class_init (GPEEngineClass *klass)
{
	GType the_type = G_TYPE_FROM_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = gpe_engine_set_property;
	object_class->get_property = gpe_engine_get_property;
	object_class->finalize = gpe_engine_finalize;
	klass->activate_plugin = gpe_engine_activate_plugin_real;
	klass->deactivate_plugin = gpe_engine_deactivate_plugin_real;
	klass->activate_plugin_on_object = gpe_engine_activate_plugin_on_object;
	klass->deactivate_plugin_on_object = gpe_engine_deactivate_plugin_on_object;

	/**
	 * GPEEngine:app-name:
	 *
	 * The application name. Filename extension and section header for
	 * #GPEPluginInfo files are actually derived from this value.
	 *
	 * For instance, if app-name is "Gedit", then info files will have
	 * the .gedit-plugin extension, and the engine will look for a
	 * "Gedit Plugin" section in it to load the plugin data.
	 */
        g_object_class_install_property (object_class, PROP_APP_NAME,
                                         g_param_spec_string ("app-name",
                                                              "Application Name",
                                                              "The application name",
							      "GPE",
                                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

	/**
	 * GPEEngine:base-module-dir:
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
							      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

	/**
	 * GPEEngine::activate-plugin:
	 * @engine: A #GPEEngine.
	 * @info: A #GPEPluginInfo.
	 *
	 * The activate-plugin signal is emitted when a plugin is being
	 * activated.
	 */
	signals[ACTIVATE_PLUGIN] =
		g_signal_new ("activate-plugin",
			      the_type,
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GPEEngineClass, activate_plugin),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__BOXED,
			      G_TYPE_NONE,
			      1,
			      GPE_TYPE_PLUGIN_INFO | G_SIGNAL_TYPE_STATIC_SCOPE);

	/**
	 * GPEEngine::deactivate-plugin:
	 * @engine: A #GPEEngine.
	 * @info: A #GPEPluginInfo.
	 *
	 * The activate-plugin signal is emitted when a plugin is being
	 * deactivated.
	 */
	signals[DEACTIVATE_PLUGIN] =
		g_signal_new ("deactivate-plugin",
			      the_type,
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GPEEngineClass, deactivate_plugin),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__BOXED,
			      G_TYPE_NONE,
			      1,
			      GPE_TYPE_PLUGIN_INFO | G_SIGNAL_TYPE_STATIC_SCOPE);

	g_type_class_add_private (klass, sizeof (GPEEnginePrivate));
}

static LoaderInfo *
load_plugin_loader (GPEEngine *engine,
		    const gchar *loader_id)
{
	gchar *loader_dirname;
	gchar *loader_basename;
	GPEObjectModule *module;
	GPEPluginLoader *loader;

	/* Let's build the expected filename of the requested plugin loader */
	loader_dirname = gpe_dirs_get_plugin_loaders_dir ();
	loader_basename = g_strdup_printf ("lib%sloader.%s", loader_id, G_MODULE_SUFFIX);
	g_strdown (loader_basename);

	g_debug ("Loading loader '%s': '%s/%s'", loader_id, loader_dirname, loader_basename);

	/* For now all modules are resident */
	module = gpe_object_module_new (loader_basename, loader_dirname,
					"register_gpe_plugin_loader", TRUE);

	g_free (loader_dirname);
	g_free (loader_basename);

	/* make sure to load the type definition */
	if (g_type_module_use (G_TYPE_MODULE (module)))
	{
		loader = (GPEPluginLoader *) gpe_object_module_new_object (module, NULL);

		if (loader == NULL || !GPE_IS_PLUGIN_LOADER (loader))
		{
			g_warning ("Loader object is not a valid GPEPluginLoader instance");
			if (loader != NULL && G_IS_OBJECT (loader))
				g_object_unref (loader);
			module = NULL;
			loader = NULL;
		}
		else
		{
			gchar *module_dir = g_build_filename (engine->priv->base_module_dir, loader_id, NULL);
			gpe_plugin_loader_add_module_directory (loader, module_dir);
			g_free (module_dir);
		}

		g_type_module_unuse (G_TYPE_MODULE (module));
	}
	else
	{
		g_warning ("Plugin loader module `%s' could not be loaded", loader_basename);
		g_object_unref (module);
		module = NULL;
		loader = NULL;
	}

	return add_loader (engine, loader_id, module, loader);
}

static GPEPluginLoader *
get_plugin_loader (GPEEngine     *engine,
		   GPEPluginInfo *info)
{
	const gchar *loader_id;
	LoaderInfo *loader_info;

	loader_id = info->loader;

	loader_info = (LoaderInfo *)g_hash_table_lookup (
			engine->priv->loaders,
			loader_id);

	if (loader_info == NULL)
		loader_info = load_plugin_loader (engine, loader_id);

	return loader_info->loader;
}

const GList *
gpe_engine_get_plugin_list (GPEEngine *engine)
{
	return engine->priv->plugin_list;
}

static gint
compare_plugin_info_and_name (GPEPluginInfo *info,
			      const gchar   *module_name)
{
	return strcmp (gpe_plugin_info_get_module_name (info), module_name);
}

/**
 * gpe_engine_get_plugin_info:
 * @engine: A #GPEEngine.
 * @plugin_name: A plugin name.
 *
 * Returns the #GPEPluginInfo corresponding with a given plugin name.
 */
GPEPluginInfo *
gpe_engine_get_plugin_info (GPEEngine   *engine,
			    const gchar *plugin_name)
{
	GList *l = g_list_find_custom (engine->priv->plugin_list,
				       plugin_name,
				       (GCompareFunc) compare_plugin_info_and_name);

	return l == NULL ? NULL : (GPEPluginInfo *) l->data;
}

static gboolean
load_plugin (GPEEngine     *engine,
	     GPEPluginInfo *info)
{
	GPEPluginLoader *loader;

	if (gpe_plugin_info_is_active (info))
		return TRUE;

	if (!gpe_plugin_info_is_available (info))
		return FALSE;

	loader = get_plugin_loader (engine, info);

	if (loader == NULL)
	{
		g_warning ("Could not find loader `%s' for plugin `%s'", info->loader, info->name);
		info->available = FALSE;
		return FALSE;
	}

	info->plugin = gpe_plugin_loader_load (loader, info);

	if (info->plugin == NULL)
	{
		g_warning ("Error loading plugin '%s'", info->name);
		info->available = FALSE;
		return FALSE;
	}

	return TRUE;
}

static void
gpe_engine_activate_plugin_real (GPEEngine     *engine,
				 GPEPluginInfo *info)
{
	const GList *item;

	if (!load_plugin (engine, info))
		return;

	for (item = engine->priv->object_list; item != NULL; item = item->next)
		GPE_ENGINE_GET_CLASS (engine)->activate_plugin_on_object (engine, info, G_OBJECT (item->data));
}

/**
 * gpe_engine_activate_plugin:
 * @engine: A #GPEEngine.
 * @info: A #GPEPluginInfo.
 *
 * Activates the plugin corresponding to @info on all the objects registered
 * against @engine, loading it if it's not already available.
 */
gboolean
gpe_engine_activate_plugin (GPEEngine     *engine,
			    GPEPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, FALSE);

	if (!gpe_plugin_info_is_available (info))
		return FALSE;

	if (gpe_plugin_info_is_active (info))
		return TRUE;

	g_signal_emit (engine, signals[ACTIVATE_PLUGIN], 0, info);

	return gpe_plugin_info_is_active (info);
}

static void
gpe_engine_deactivate_plugin_real (GPEEngine     *engine,
				   GPEPluginInfo *info)
{
	const GList *item;
	GPEPluginLoader *loader;

	if (!gpe_plugin_info_is_active (info) ||
	    !gpe_plugin_info_is_available (info))
		return;

	for (item = engine->priv->object_list; item != NULL; item = item->next)
		GPE_ENGINE_GET_CLASS (engine)->deactivate_plugin_on_object (engine, info, G_OBJECT (item->data));

	/* first unref the plugin (the loader still has one) */
	g_object_unref (info->plugin);

	/* find the loader and tell it to gc and unload the plugin */
	loader = get_plugin_loader (engine, info);

	gpe_plugin_loader_garbage_collect (loader);
	gpe_plugin_loader_unload (loader, info);

	info->plugin = NULL;
}

/**
 * gpe_engine_deactivate_plugin:
 * @engine: A #GPEEngine.
 * @info: A #GPEPluginInfo.
 *
 * Deactivates the plugin corresponding to @info on all the objects registered
 * against @engine, eventually unloading it when it has been completely
 * deactivated.
 */
gboolean
gpe_engine_deactivate_plugin (GPEEngine     *engine,
			      GPEPluginInfo *info)
{
	g_return_val_if_fail (info != NULL, FALSE);

	if (!gpe_plugin_info_is_active (info))
		return TRUE;

	g_signal_emit (engine, signals[DEACTIVATE_PLUGIN], 0, info);

	return !gpe_plugin_info_is_active (info);
}

/**
 * gpe_engine_update_plugins_ui:
 * @engine: A #GPEEngine.
 * @object: A registered #GObject.
 *
 * Triggers an update of all the plugins user interfaces to take into
 * account state changes due to a plugin or an user action.
 */
void
gpe_engine_update_plugins_ui (GPEEngine *engine,
			      GObject   *object)
{
	GList *pl;

	g_return_if_fail (GPE_IS_ENGINE (engine));
	g_return_if_fail (G_IS_OBJECT (object));

	/* call update_ui for all active plugins */
	for (pl = engine->priv->plugin_list; pl; pl = pl->next)
	{
		GPEPluginInfo *info = (GPEPluginInfo*)pl->data;

		if (!gpe_plugin_info_is_active (info))
			continue;

		gpe_plugin_update_ui (info->plugin, object);
	}
}

/**
 * gpe_engine_configure_plugin:
 * @engine: A #GPEEngine.
 * @info: A #GPEPluginInfo.
 * @parent: A parent #GtkWindow for the newly created dialog.
 *
 * Created a configuration dialog for the plugin related to @info,
 * and show it transient to @parent.
 */
void
gpe_engine_configure_plugin (GPEEngine     *engine,
			     GPEPluginInfo *info,
			     GtkWindow     *parent)
{
	GtkWidget *conf_dlg;

	GtkWindowGroup *wg;

	g_return_if_fail (info != NULL);

	conf_dlg = gpe_plugin_create_configure_dialog (info->plugin);
	g_return_if_fail (conf_dlg != NULL);
	gtk_window_set_transient_for (GTK_WINDOW (conf_dlg),
				      parent);

	wg = gtk_window_get_group (parent);
	if (wg == NULL)
	{
		wg = gtk_window_group_new ();
		gtk_window_group_add_window (wg, parent);
	}

	gtk_window_group_add_window (wg,
				     GTK_WINDOW (conf_dlg));

	gtk_window_set_modal (GTK_WINDOW (conf_dlg), TRUE);
	gtk_widget_show (conf_dlg);
}

/**
 * gpe_engine_get_active_plugins:
 * @engine: A #GPEEngine.
 *
 * Returns the list of the names of all the active plugins, or %NULL if there
 * is no active plugin. Please note that the returned array is a newly
 * allocated one: you will need to free it using g_strfreev().
 *
 * Returns: A newly-allocated NULL-terminated array of strings, or %NULL.
 */
gchar **
gpe_engine_get_active_plugins (GPEEngine *engine)
{
	GArray *array = g_array_new (TRUE, FALSE, sizeof (gchar *));
	GList *pl;

	for (pl = engine->priv->plugin_list; pl; pl = pl->next)
	{
		GPEPluginInfo *info = (GPEPluginInfo*) pl->data;
		gchar *module_name;

		if (gpe_plugin_info_is_active (info))
		{
			module_name = g_strdup (gpe_plugin_info_get_module_name (info));
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
 * gpe_engine_set_active_plugins:
 * @engine: A #GPEEngine.
 * @plugin_names: A NULL-terminated array of plugin names.
 *
 * Sets the list of active plugins for @engine. When this function is called,
 * the #GPEEngine will activate all the plugins whose names are in
 * @plugin_names, and deactivate all other active plugins.
 */
void
gpe_engine_set_active_plugins (GPEEngine    *engine,
			       const gchar **plugin_names)
{
	GList *pl;

	for (pl = engine->priv->plugin_list; pl; pl = pl->next)
	{
		GPEPluginInfo *info = (GPEPluginInfo*)pl->data;
		const gchar *module_name;
		gboolean is_active;
		gboolean to_activate;

		if (!gpe_plugin_info_is_available (info))
			continue;

		module_name = gpe_plugin_info_get_module_name (info);
		is_active = gpe_plugin_info_is_active (info);

		to_activate = string_in_strv (module_name, plugin_names);

		if (!is_active && to_activate)
			g_signal_emit (engine, signals[ACTIVATE_PLUGIN], 0, info);
		else if (is_active && !to_activate)
			g_signal_emit (engine, signals[DEACTIVATE_PLUGIN], 0, info);
	}
}

/**
 * gpe_engine_add_object:
 * @engine: A #GPEEngine.
 * @object: A #GObject to register.
 *
 * Register an object against the #GPEEngine. The activate() method of all
 * the active plugins will be called on every registered object.
 */
void
gpe_engine_add_object (GPEEngine *engine,
		       GObject   *object)
{
	GList *pl;

	g_return_if_fail (GPE_IS_ENGINE (engine));
	g_return_if_fail (G_IS_OBJECT (object));

	/* Ensure we don't insert the same object twice... */
	if (g_list_find (engine->priv->object_list, object))
		return;

	/* Activate the plugin on object, and add it to the list of managed objects */
	for (pl = engine->priv->plugin_list; pl; pl = pl->next)
	{
		GPEPluginInfo *info = (GPEPluginInfo*) pl->data;

		/* check if the plugin is actually active */
		if (!gpe_plugin_info_is_active (info))
			continue;

		GPE_ENGINE_GET_CLASS (engine)->activate_plugin_on_object (engine, info, object);
	}

	engine->priv->object_list = g_list_prepend (engine->priv->object_list, object);

	/* also call update_ui after activation */
	gpe_engine_update_plugins_ui (engine, object);

}

/**
 * gpe_engine_remove_object:
 * @engine: A #GPEEngine.
 * @object: A #GObject to register.
 *
 * Unregister an object against the #GPEEngine. The deactivate() method of
 * all the active plugins will be called on the object while he is being
 * unregistered.
 */
void
gpe_engine_remove_object (GPEEngine *engine,
			  GObject   *object)
{
	GList *pl;

	g_return_if_fail (GPE_IS_ENGINE (engine));
	g_return_if_fail (G_IS_OBJECT (object));

	GList *item = g_list_find (engine->priv->object_list, object);
	if (item == NULL)
		return;

	/* Remove the object to the list of managed objects, and deactivate the plugin on it */
	engine->priv->object_list = g_list_delete_link (engine->priv->object_list, item);

	for (pl = engine->priv->plugin_list; pl; pl = pl->next)
	{
		GPEPluginInfo *info = (GPEPluginInfo*) pl->data;

		/* check if the plugin is actually active */
		if (!gpe_plugin_info_is_active (info))
			continue;

		/* call deactivate for the plugin for this window */
		GPE_ENGINE_GET_CLASS (engine)->deactivate_plugin_on_object (engine, info, object);
	}
}

/**
 * gpe_engine_new:
 * @app_name: The name of the app
 * @base_module_dir: The base directory for language modules
 *
 * Returns a new #GPEEngine object.
 * See the properties description for more information about the parameters.
 *
 * Returns: a newly created #GPEEngine object.
 */
GPEEngine *
gpe_engine_new (const gchar *app_name,
		const gchar *base_module_dir)
{
	return GPE_ENGINE (g_object_new (GPE_TYPE_ENGINE,
					 "app-name", app_name,
					 "base-module-dir", base_module_dir,
					 NULL));
}
