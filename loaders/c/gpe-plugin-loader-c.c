/*
 * gpe-plugin-loader-c.c
 * This file is part of libgpe
 *
 * Copyright (C) 2008 - Jesse van den Kieboom
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

#include "gpe-plugin-loader-c.h"
#include <libgpe/gpe-object-module.h>

#define GPE_PLUGIN_LOADER_C_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), GPE_TYPE_PLUGIN_LOADER_C, GPEPluginLoaderCPrivate))

struct _GPEPluginLoaderCPrivate
{
	GHashTable *loaded_plugins;
};

static void gpe_plugin_loader_iface_init (gpointer g_iface, gpointer iface_data);

GPE_PLUGIN_LOADER_REGISTER_TYPE (GPEPluginLoaderC, gpe_plugin_loader_c, G_TYPE_OBJECT, gpe_plugin_loader_iface_init);

static const gchar *
gpe_plugin_loader_iface_get_id (void)
{
	return "C";
}

void
gpe_plugin_loader_iface_add_module_directory (GPEPluginLoader *loader,
					      const gchar     *module_dir)
{
	/* This is a no-op for C modules... */
}

static GPEPlugin *
gpe_plugin_loader_iface_load (GPEPluginLoader *loader,
			      GPEPluginInfo   *info)
{
	GPEPluginLoaderC *cloader = GPE_PLUGIN_LOADER_C (loader);
	GPEObjectModule *module;
	const gchar *module_name;
	GPEPlugin *result;

	module = (GPEObjectModule *) g_hash_table_lookup (cloader->priv->loaded_plugins, info);
	module_name = gpe_plugin_info_get_module_name (info);

	if (module == NULL)
	{
		/* For now we force all modules to be resident */
		module = gpe_object_module_new (module_name,
						gpe_plugin_info_get_module_dir (info),
						"register_gpe_plugin",
						TRUE);

		/* Infos are available for all the lifetime of the loader.
		 * If this changes, we should use weak refs or something */

		g_hash_table_insert (cloader->priv->loaded_plugins, info, module);
	}

	if (!g_type_module_use (G_TYPE_MODULE (module)))
	{
		g_warning ("Could not load plugin module: %s", gpe_plugin_info_get_name (info));

		return NULL;
	}

	result = (GPEPlugin *) gpe_object_module_new_object (module, "plugin-info", info, NULL);

	if (!result)
	{
		g_warning ("Could not create plugin object: %s", gpe_plugin_info_get_name (info));
		g_type_module_unuse (G_TYPE_MODULE (module));

		return NULL;
	}

	g_type_module_unuse (G_TYPE_MODULE (module));

	return result;
}

static void
gpe_plugin_loader_iface_unload (GPEPluginLoader *loader,
				GPEPluginInfo    *info)
{
	//GPEPluginLoaderC *cloader = GPE_PLUGIN_LOADER_C (loader);

	/* this is a no-op, since the type module will be properly unused as
	   the last reference to the plugin dies. When the plugin is activated
	   again, the library will be reloaded */
}

static void
gpe_plugin_loader_iface_init (gpointer g_iface,
			      gpointer iface_data)
{
	GPEPluginLoaderInterface *iface = (GPEPluginLoaderInterface *) g_iface;

	iface->get_id = gpe_plugin_loader_iface_get_id;
	iface->add_module_directory = gpe_plugin_loader_iface_add_module_directory;
	iface->load = gpe_plugin_loader_iface_load;
	iface->unload = gpe_plugin_loader_iface_unload;
}

static void
gpe_plugin_loader_c_finalize (GObject *object)
{
	GPEPluginLoaderC *cloader = GPE_PLUGIN_LOADER_C (object);
	GList *infos;
	GList *item;

	/* FIXME: this sanity check it's not efficient. Let's remove it
	 * once we are confident with the code */

	infos = g_hash_table_get_keys (cloader->priv->loaded_plugins);

	for (item = infos; item; item = item->next)
	{
		GPEPluginInfo *info = (GPEPluginInfo *) item->data;

		if (gpe_plugin_info_is_active (info))
		{
			g_warning ("There are still C plugins loaded during destruction");
			break;
		}
	}

	g_list_free (infos);

	g_hash_table_destroy (cloader->priv->loaded_plugins);

	G_OBJECT_CLASS (gpe_plugin_loader_c_parent_class)->finalize (object);
}

static void
gpe_plugin_loader_c_class_init (GPEPluginLoaderCClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gpe_plugin_loader_c_finalize;

	g_type_class_add_private (object_class, sizeof (GPEPluginLoaderCPrivate));
}

static void
gpe_plugin_loader_c_class_finalize (GPEPluginLoaderCClass *klass)
{
}

static void
gpe_plugin_loader_c_init (GPEPluginLoaderC *self)
{
	self->priv = GPE_PLUGIN_LOADER_C_GET_PRIVATE (self);

	/* loaded_plugins maps GPEPluginInfo to a GPEObjectModule */
	self->priv->loaded_plugins = g_hash_table_new (g_direct_hash,
						       g_direct_equal);
}

GPEPluginLoaderC *
gpe_plugin_loader_c_new ()
{
	GObject *loader = g_object_new (GPE_TYPE_PLUGIN_LOADER_C, NULL);

	return GPE_PLUGIN_LOADER_C (loader);
}
