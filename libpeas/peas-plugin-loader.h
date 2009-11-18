/*
 * peas-plugin-loader.h
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

#ifndef __PEAS_PLUGIN_LOADER_H__
#define __PEAS_PLUGIN_LOADER_H__

#include <glib-object.h>
#include "peas-plugin.h"
#include "peas-plugin-info.h"

G_BEGIN_DECLS

#define PEAS_TYPE_PLUGIN_LOADER                (peas_plugin_loader_get_type ())
#define PEAS_PLUGIN_LOADER(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), PEAS_TYPE_PLUGIN_LOADER, PeasPluginLoader))
#define PEAS_IS_PLUGIN_LOADER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PEAS_TYPE_PLUGIN_LOADER))
#define PEAS_PLUGIN_LOADER_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE ((inst), PEAS_TYPE_PLUGIN_LOADER, PeasPluginLoaderInterface))

typedef struct _PeasPluginLoader PeasPluginLoader; /* dummy object */
typedef struct _PeasPluginLoaderInterface PeasPluginLoaderInterface;

struct _PeasPluginLoaderInterface {
	GTypeInterface parent;

	const gchar	*(*get_id)		(void);

	void		 (*add_module_directory)(PeasPluginLoader	*loader,
						 const gchar		*module_dir);

	PeasPlugin	*(*load)		(PeasPluginLoader 	*loader,
						 PeasPluginInfo	        *info);

	void		 (*unload)		(PeasPluginLoader 	*loader,
						 PeasPluginInfo       	*info);

	void		 (*garbage_collect)	(PeasPluginLoader	*loader);
};

GType		 peas_plugin_loader_get_type		(void);

const gchar	*peas_plugin_loader_type_get_id		(GType 			 type);

void		 peas_plugin_loader_add_module_directory	(PeasPluginLoader	*loader,
							 const gchar		*module_dir);

PeasPlugin	*peas_plugin_loader_load			(PeasPluginLoader 	*loader,
							 PeasPluginInfo 		*info);
void		 peas_plugin_loader_unload		(PeasPluginLoader 	*loader,
							 PeasPluginInfo		*info);
void		 peas_plugin_loader_garbage_collect	(PeasPluginLoader 	*loader);

/**
 * PEAS_PLUGIN_LOADER_REGISTER_TYPE(PluginLoaderName, plugin_loader_name, PARENT_TYPE, loader_interface_init):
 *
 * Utility macro used to register plugin loaders.
 */
#define PEAS_PLUGIN_LOADER_REGISTER_TYPE(PluginLoaderName, plugin_loader_name, PARENT_TYPE, loader_iface_init) 	\
	G_DEFINE_DYNAMIC_TYPE_EXTENDED (PluginLoaderName,			\
					plugin_loader_name,			\
					PARENT_TYPE,				\
					0,					\
					G_IMPLEMENT_INTERFACE(PEAS_TYPE_PLUGIN_LOADER, loader_iface_init));	\
										\
										\
G_MODULE_EXPORT GType								\
register_peas_plugin_loader (GTypeModule *type_module)				\
{										\
	plugin_loader_name##_register_type (type_module);			\
										\
	return plugin_loader_name##_get_type();					\
}

G_END_DECLS

#endif /* __PEAS_PLUGIN_LOADER_H__ */
