/*
 * gpe-plugin-loader.h
 * This file is part of libgpe
 *
 * Copyright (C) 2008 - Jesse van den Kieboom
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

#ifndef __GPE_PLUGIN_LOADER_H__
#define __GPE_PLUGIN_LOADER_H__

#include <glib-object.h>
#include "gpe-plugin.h"
#include "gpe-plugin-info.h"

G_BEGIN_DECLS

#define GPE_TYPE_PLUGIN_LOADER                (gpe_plugin_loader_get_type ())
#define GPE_PLUGIN_LOADER(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPE_TYPE_PLUGIN_LOADER, GPEPluginLoader))
#define GPE_IS_PLUGIN_LOADER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPE_TYPE_PLUGIN_LOADER))
#define GPE_PLUGIN_LOADER_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), GPE_TYPE_PLUGIN_LOADER, GPEPluginLoaderInterface))

typedef struct _GPEPluginLoader GPEPluginLoader; /* dummy object */
typedef struct _GPEPluginLoaderInterface GPEPluginLoaderInterface;

struct _GPEPluginLoaderInterface {
	GTypeInterface parent;

	const gchar	*(*get_id)		(void);

	GPEPlugin	*(*load)		(GPEPluginLoader 	*loader,
						 GPEPluginInfo	        *info,
						 const gchar       	*path,
						 const gchar		*datadir);

	void		 (*unload)		(GPEPluginLoader 	*loader,
						 GPEPluginInfo       	*info);

	void		 (*garbage_collect)	(GPEPluginLoader	*loader);
};

GType		 gpe_plugin_loader_get_type		(void);

const gchar	*gpe_plugin_loader_type_get_id		(GType 			 type);
GPEPlugin	*gpe_plugin_loader_load			(GPEPluginLoader 	*loader,
							 GPEPluginInfo 	*info,
							 const gchar		*path,
							 const gchar		*datadir);
void		 gpe_plugin_loader_unload		(GPEPluginLoader 	*loader,
							 GPEPluginInfo		*info);
void		 gpe_plugin_loader_garbage_collect	(GPEPluginLoader 	*loader);

/**
 * GPE_PLUGIN_LOADER_REGISTER_TYPE(PluginLoaderName, plugin_loader_name, PARENT_TYPE, loader_interface_init):
 *
 * Utility macro used to register plugin loaders.
 */
#define GPE_PLUGIN_LOADER_REGISTER_TYPE(PluginLoaderName, plugin_loader_name, PARENT_TYPE, loader_iface_init) 	\
	G_DEFINE_DYNAMIC_TYPE_EXTENDED (PluginLoaderName,			\
					plugin_loader_name,			\
					PARENT_TYPE,				\
					0,					\
					G_IMPLEMENT_INTERFACE(GPE_TYPE_PLUGIN_LOADER, loader_iface_init));	\
										\
										\
G_MODULE_EXPORT GType								\
register_gpe_plugin_loader (GTypeModule *type_module)				\
{										\
	plugin_loader_name##_register_type (type_module);			\
										\
	return plugin_loader_name##_get_type();					\
}

G_END_DECLS

#endif /* __GPE_PLUGIN_LOADER_H__ */
