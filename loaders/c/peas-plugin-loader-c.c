/*
 * peas-plugin-loader-c.c
 * This file is part of libpeas
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

#include "peas-plugin-loader-c.h"
#include <libpeas/peas-object-module.h>

#define PEAS_PLUGIN_LOADER_C_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE((object), PEAS_TYPE_PLUGIN_LOADER_C, PeasPluginLoaderCPrivate))

struct _PeasPluginLoaderCPrivate
{
	GHashTable *loaded_plugins;
};

static void peas_plugin_loader_iface_init (gpointer g_iface, gpointer iface_data);

PEAS_PLUGIN_LOADER_REGISTER_TYPE (PeasPluginLoaderC, peas_plugin_loader_c, G_TYPE_OBJECT, peas_plugin_loader_iface_init);

static const gchar *
peas_plugin_loader_iface_get_id (void)
{
	return "C";
}

void
peas_plugin_loader_iface_add_module_directory (PeasPluginLoader *loader,
					      const gchar     *module_dir)
{
	/* This is a no-op for C modules... */
}

static PeasPlugin *
peas_plugin_loader_iface_load (PeasPluginLoader *loader,
			      PeasPluginInfo   *info)
{
	PeasPluginLoaderC *cloader = PEAS_PLUGIN_LOADER_C (loader);
	PeasObjectModule *module;
	const gchar *module_name;
	PeasPlugin *result;

	module = (PeasObjectModule *) g_hash_table_lookup (cloader->priv->loaded_plugins, info);
	module_name = peas_plugin_info_get_module_name (info);

	if (module == NULL)
	{
		/* For now we force all modules to be resident */
		module = peas_object_module_new (module_name,
						peas_plugin_info_get_module_dir (info),
						"register_peas_plugin",
						TRUE);

		/* Infos are available for all the lifetime of the loader.
		 * If this changes, we should use weak refs or something */

		g_hash_table_insert (cloader->priv->loaded_plugins, info, module);
	}

	if (!g_type_module_use (G_TYPE_MODULE (module)))
	{
		g_warning ("Could not load plugin module: %s", peas_plugin_info_get_name (info));

		return NULL;
	}

	result = (PeasPlugin *) peas_object_module_new_object (module, "plugin-info", info, NULL);

	if (!result)
	{
		g_warning ("Could not create plugin object: %s", peas_plugin_info_get_name (info));
		g_type_module_unuse (G_TYPE_MODULE (module));

		return NULL;
	}

	g_type_module_unuse (G_TYPE_MODULE (module));

	return result;
}

static void
peas_plugin_loader_iface_unload (PeasPluginLoader *loader,
				PeasPluginInfo    *info)
{
	//PeasPluginLoaderC *cloader = PEAS_PLUGIN_LOADER_C (loader);

	/* this is a no-op, since the type module will be properly unused as
	   the last reference to the plugin dies. When the plugin is activated
	   again, the library will be reloaded */
}

static void
peas_plugin_loader_iface_init (gpointer g_iface,
			      gpointer iface_data)
{
	PeasPluginLoaderInterface *iface = (PeasPluginLoaderInterface *) g_iface;

	iface->get_id = peas_plugin_loader_iface_get_id;
	iface->add_module_directory = peas_plugin_loader_iface_add_module_directory;
	iface->load = peas_plugin_loader_iface_load;
	iface->unload = peas_plugin_loader_iface_unload;
}

static void
peas_plugin_loader_c_finalize (GObject *object)
{
	PeasPluginLoaderC *cloader = PEAS_PLUGIN_LOADER_C (object);
	GList *infos;
	GList *item;

	/* FIXME: this sanity check it's not efficient. Let's remove it
	 * once we are confident with the code */

	infos = g_hash_table_get_keys (cloader->priv->loaded_plugins);

	for (item = infos; item; item = item->next)
	{
		PeasPluginInfo *info = (PeasPluginInfo *) item->data;

		if (peas_plugin_info_is_active (info))
		{
			g_warning ("There are still C plugins loaded during destruction");
			break;
		}
	}

	g_list_free (infos);

	g_hash_table_destroy (cloader->priv->loaded_plugins);

	G_OBJECT_CLASS (peas_plugin_loader_c_parent_class)->finalize (object);
}

static void
peas_plugin_loader_c_class_init (PeasPluginLoaderCClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = peas_plugin_loader_c_finalize;

	g_type_class_add_private (object_class, sizeof (PeasPluginLoaderCPrivate));
}

static void
peas_plugin_loader_c_class_finalize (PeasPluginLoaderCClass *klass)
{
}

static void
peas_plugin_loader_c_init (PeasPluginLoaderC *self)
{
	self->priv = PEAS_PLUGIN_LOADER_C_GET_PRIVATE (self);

	/* loaded_plugins maps PeasPluginInfo to a PeasObjectModule */
	self->priv->loaded_plugins = g_hash_table_new (g_direct_hash,
						       g_direct_equal);
}

PeasPluginLoaderC *
peas_plugin_loader_c_new ()
{
	GObject *loader = g_object_new (PEAS_TYPE_PLUGIN_LOADER_C, NULL);

	return PEAS_PLUGIN_LOADER_C (loader);
}
