/*
 * gpe-engine.h
 * This file is part of libgpe
 *
 * Copyright (C) 2002-2005 - Paolo Maggi
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

#ifndef __GPE_ENGINE_H__
#define __GPE_ENGINE_H__

#include <glib.h>
#include <gtk/gtk.h>
#include "gpe-plugin-info.h"
#include "gpe-plugin.h"
#include "gpe-path-info.h"

G_BEGIN_DECLS

#define GPE_TYPE_ENGINE              (gpe_engine_get_type ())
#define GPE_ENGINE(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GPE_TYPE_ENGINE, GPEEngine))
#define GPE_ENGINE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GPE_TYPE_ENGINE, GPEEngineClass))
#define GPE_IS_ENGINE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GPE_TYPE_ENGINE))
#define GPE_IS_ENGINE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GPE_TYPE_ENGINE))
#define GPE_ENGINE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GPE_TYPE_ENGINE, GPEEngineClass))

typedef struct _GPEEngine		GPEEngine;
typedef struct _GPEEnginePrivate	GPEEnginePrivate;

struct _GPEEngine
{
	GObject parent;
	GPEEnginePrivate *priv;
};

typedef struct _GPEEngineClass		GPEEngineClass;

struct _GPEEngineClass
{
	GObjectClass parent_class;

	void	 (* activate_plugin)			(GPEEngine      *engine,
							 GPEPluginInfo  *info);

	void	 (* deactivate_plugin)			(GPEEngine      *engine,
							 GPEPluginInfo  *info);

	/* called when done deactivating the plugin, but before the plugin
	 * gets unloaded. */
	void	 (* plugin_deactivated)			(GPEEngine      *engine,
							 GPEPluginInfo  *info);
};

GType		 gpe_engine_get_type			(void) G_GNUC_CONST;

GPEEngine	*gpe_engine_new				(const GPEPathInfo *paths);

void		 gpe_engine_garbage_collect		(GPEEngine      *engine);

const GList	*gpe_engine_get_plugin_list 		(GPEEngine      *engine);

GPEPluginInfo	*gpe_engine_get_plugin_info		(GPEEngine      *engine,
							 const gchar    *name);

/* plugin load and unloading (overall, for all windows) */
gboolean 	 gpe_engine_activate_plugin 		(GPEEngine      *engine,
							 GPEPluginInfo  *info);
gboolean 	 gpe_engine_deactivate_plugin		(GPEEngine      *engine,
							 GPEPluginInfo  *info);

void	 	 gpe_engine_configure_plugin		(GPEEngine      *engine,
							 GPEPluginInfo  *info,
							 GtkWindow      *parent);

/* plugin activation/deactivation per target_object */
void		 gpe_engine_update_plugins_ui		(GPEEngine      *engine,
							 GObject        *target_object);

/* private for gconf notification */
void		 gpe_engine_active_plugins_changed	(GPEEngine      *engine);

void		 gpe_engine_rescan_plugins		(GPEEngine      *engine);

/* object management */
void		 gpe_engine_add_object			(GPEEngine     *engine,
							 GObject       *object);
void		 gpe_engine_remove_object		(GPEEngine     *engine,
							 GObject       *object);

G_END_DECLS

#endif  /* __GPE_ENGINE_H__ */
