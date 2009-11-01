/*
 * gpe-engine.c
 * This file is part of libgpe
 *
 * Copyright (C) 2002-2005 Paolo Maggi
 * Copyright (C) 2009 Steve Frécinaux
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
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

#define LOADER_EXT	G_MODULE_SUFFIX

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
	PROP_PATH_INFOS,
	PROP_APP_NAME
};

G_DEFINE_TYPE(GPEEngine, gpe_engine, G_TYPE_OBJECT)

struct _GPEEnginePrivate
{
	const GPEPathInfo *paths;
	gchar *app_name;

	GList *plugin_list;
	GHashTable *loaders;

	GList *object_list;
};

static void	gpe_engine_activate_plugin_real		 (GPEEngine     *engine,
							  GPEPluginInfo *info);
static void	gpe_engine_deactivate_plugin_real	 (GPEEngine     *engine,
							  GPEPluginInfo *info);

typedef gboolean (*LoadDirCallback) (GPEEngine *engine, const gchar *filename, gpointer userdata);

static void
dummy (GPEEngine *engine, GPEPluginInfo *info)
{
	/* Empty */
}

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

void
gpe_engine_rescan_plugins (GPEEngine *engine)
{
	const GPEPathInfo *pathinfo;

	g_return_if_fail (GPE_IS_ENGINE (engine));

	for (pathinfo = engine->priv->paths; pathinfo->module_dir != NULL; pathinfo++)
		load_dir_real (engine, pathinfo);
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
}

static void
gpe_engine_constructed (GObject *object)
{
	gpe_engine_rescan_plugins (GPE_ENGINE (object));
}

static void
loader_garbage_collect (const char *id, LoaderInfo *info)
{
	if (info->loader)
		gpe_plugin_loader_garbage_collect (info->loader);
}

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
                case PROP_PATH_INFOS:
                        engine->priv->paths = (const GPEPathInfo *) g_value_get_pointer (value);
			break;
		case PROP_APP_NAME:
			engine->priv->app_name = g_value_dup_string (value);
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

	G_OBJECT_CLASS (gpe_engine_parent_class)->finalize (object);
}

static void
gpe_engine_class_init (GPEEngineClass *klass)
{
	GType the_type = G_TYPE_FROM_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->constructed = gpe_engine_constructed;
	object_class->set_property = gpe_engine_set_property;
	object_class->get_property = gpe_engine_get_property;
	object_class->finalize = gpe_engine_finalize;
	klass->activate_plugin = gpe_engine_activate_plugin_real;
	klass->deactivate_plugin = gpe_engine_deactivate_plugin_real;
	klass->plugin_deactivated = dummy;

        g_object_class_install_property (object_class, PROP_PATH_INFOS,
                                         g_param_spec_pointer ("path-infos",
                                                               "Path Infos",
                                                               "Information on the paths where to find plugins",
                                                               G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

        g_object_class_install_property (object_class, PROP_APP_NAME,
                                         g_param_spec_string ("app-name",
                                                              "Application Name",
                                                              "The application name",
							      "libgpe",
                                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

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
	loader_basename = g_strdup_printf ("lib%sloader.%s", loader_id, LOADER_EXT);
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

GPEPluginInfo *
gpe_engine_get_plugin_info (GPEEngine   *engine,
			    const gchar *name)
{
	GList *l = g_list_find_custom (engine->priv->plugin_list,
				       name,
				       (GCompareFunc) compare_plugin_info_and_name);

	return l == NULL ? NULL : (GPEPluginInfo *) l->data;
}

static gboolean
load_plugin (GPEEngine     *engine,
	     GPEPluginInfo *info)
{
	GPEPluginLoader *loader;
	const gchar *path;
	const gchar *data_dir;

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

	path = gpe_plugin_info_get_module_dir (info);
	data_dir = gpe_plugin_info_get_data_dir (info);
	g_return_val_if_fail (path != NULL, FALSE);
	g_return_val_if_fail (data_dir != NULL, FALSE);

	info->plugin = gpe_plugin_loader_load (loader, info, path, data_dir);

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
		gpe_plugin_activate (info->plugin, G_OBJECT (item->data));
}

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
		gpe_plugin_deactivate (info->plugin, G_OBJECT (item->data));

	/* let engine subclasses perform required cleanups. */
	GPE_ENGINE_GET_CLASS (engine)->plugin_deactivated (engine, info);

	/* first unref the plugin (the loader still has one) */
	g_object_unref (info->plugin);

	/* find the loader and tell it to gc and unload the plugin */
	loader = get_plugin_loader (engine, info);

	gpe_plugin_loader_garbage_collect (loader);
	gpe_plugin_loader_unload (loader, info);

	info->plugin = NULL;
}

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

static void
gpe_engine_activate_plugins (GPEEngine *engine,
			     GObject   *target_object)
{
	GList *pl;

	g_return_if_fail (GPE_IS_ENGINE (engine));
	g_return_if_fail (G_IS_OBJECT (target_object));

	for (pl = engine->priv->plugin_list; pl; pl = pl->next)
	{
		GPEPluginInfo *info = (GPEPluginInfo*)pl->data;

		/* check if the plugin hasn't been activated yet. */
		if (!gpe_plugin_info_is_active (info))
			continue;

		if (load_plugin (engine, info))
			gpe_plugin_activate (info->plugin, target_object);
	}

	/* also call update_ui after activation */
	gpe_engine_update_plugins_ui (engine, target_object);
}

static void
gpe_engine_deactivate_plugins (GPEEngine *engine,
			       GObject   *target_object)
{
	GList *pl;

	g_return_if_fail (GPE_IS_ENGINE (engine));
	g_return_if_fail (G_IS_OBJECT (target_object));

	for (pl = engine->priv->plugin_list; pl; pl = pl->next)
	{
		GPEPluginInfo *info = (GPEPluginInfo*)pl->data;

		/* check if the plugin is actually active */
		if (!gpe_plugin_info_is_active (info))
			continue;

		/* call deactivate for the plugin for this window */
		gpe_plugin_deactivate (info->plugin, target_object);
	}
}

void
gpe_engine_update_plugins_ui (GPEEngine *engine,
			      GObject   *target_object)
{
	GList *pl;

	g_return_if_fail (GPE_IS_ENGINE (engine));
	g_return_if_fail (G_IS_OBJECT (target_object));

	/* call update_ui for all active plugins */
	for (pl = engine->priv->plugin_list; pl; pl = pl->next)
	{
		GPEPluginInfo *info = (GPEPluginInfo*)pl->data;

		if (!gpe_plugin_info_is_active (info))
			continue;

		gpe_plugin_update_ui (info->plugin, target_object);
	}
}

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

void
gpe_engine_add_object (GPEEngine *engine,
		       GObject   *object)
{
	g_return_if_fail (GPE_IS_ENGINE (engine));
	g_return_if_fail (G_IS_OBJECT (object));

	/* Ensure we don't insert the same object twice... */
	if (g_list_find (engine->priv->object_list, object))
		return;

	/* Activate the plugin on object, and add it to the list of managed objects */
	gpe_engine_activate_plugins (engine, object);
	engine->priv->object_list = g_list_prepend (engine->priv->object_list, object);
}

void
gpe_engine_remove_object (GPEEngine *engine,
			  GObject   *object)
{
	g_return_if_fail (GPE_IS_ENGINE (engine));
	g_return_if_fail (G_IS_OBJECT (object));

	GList *item = g_list_find (engine->priv->object_list, object);
	if (item == NULL)
		return;

	/* Remove the object to the list of managed objects, and deactivate the plugin on it */
	engine->priv->object_list = g_list_delete_link (engine->priv->object_list, item);
	gpe_engine_deactivate_plugins (engine, object);
}

GPEEngine *
gpe_engine_new (const gchar *app_name, const GPEPathInfo *paths)
{
	return GPE_ENGINE (g_object_new (GPE_TYPE_ENGINE,
					 "app-name", app_name,
					 "path-infos", paths,
					 NULL));
}
