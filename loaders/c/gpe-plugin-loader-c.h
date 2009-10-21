/*
 * gpe-plugin-loader-c.h
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

#ifndef __GPE_PLUGIN_LOADER_C_H__
#define __GPE_PLUGIN_LOADER_C_H__

#include <libgpe/gpe-plugin-loader.h>

G_BEGIN_DECLS

#define GPE_TYPE_PLUGIN_LOADER_C		(gpe_plugin_loader_c_get_type ())
#define GPE_PLUGIN_LOADER_C(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GPE_TYPE_PLUGIN_LOADER_C, GPEPluginLoaderC))
#define GPE_PLUGIN_LOADER_C_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GPE_TYPE_PLUGIN_LOADER_C, GPEPluginLoaderCClass))
#define GPE_PLUGIN_IS_LOADER_C(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPE_TYPE_PLUGIN_LOADER_C))
#define GPE_PLUGIN_IS_LOADER_C_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GPE_TYPE_PLUGIN_LOADER_C))
#define GPE_PLUGIN_LOADER_C_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GPE_TYPE_PLUGIN_LOADER_C, GPEPluginLoaderCClass))

typedef struct _GPEPluginLoaderC		GPEPluginLoaderC;
typedef struct _GPEPluginLoaderCClass		GPEPluginLoaderCClass;
typedef struct _GPEPluginLoaderCPrivate		GPEPluginLoaderCPrivate;

struct _GPEPluginLoaderC {
	GObject parent;

	GPEPluginLoaderCPrivate *priv;
};

struct _GPEPluginLoaderCClass {
	GObjectClass parent_class;
};

GType gpe_plugin_loader_c_get_type (void) G_GNUC_CONST;
GPEPluginLoaderC *gpe_plugin_loader_c_new(void);

/* All the loaders must implement this function */
G_MODULE_EXPORT GType register_gpe_plugin_loader (GTypeModule * module);

G_END_DECLS

#endif /* __GPE_PLUGIN_LOADER_C_H__ */
