/*
 * gpe-plugin-manager.h
 * This file is part of libgpe
 *
 * Copyright (C) 2005-2009 Paolo Maggi, Paolo Borelli
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

#ifndef __GPE_PLUGIN_MANAGER_H__
#define __GPE_PLUGIN_MANAGER_H__

#include <gtk/gtk.h>
#include <libgpe/gpe-engine.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GPE_TYPE_PLUGIN_MANAGER              (gpe_plugin_manager_get_type())
#define GPE_PLUGIN_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GPE_TYPE_PLUGIN_MANAGER, GPEPluginManager))
#define GPE_PLUGIN_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GPE_TYPE_PLUGIN_MANAGER, GPEPluginManagerClass))
#define GPE_IS_PLUGIN_MANAGER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GPE_TYPE_PLUGIN_MANAGER))
#define GPE_IS_PLUGIN_MANAGER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GPE_TYPE_PLUGIN_MANAGER))
#define GPE_PLUGIN_MANAGER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GPE_TYPE_PLUGIN_MANAGER, GPEPluginManagerClass))

/* Private structure type */
typedef struct _GPEPluginManagerPrivate GPEPluginManagerPrivate;

/*
 * Main object structure
 */
typedef struct _GPEPluginManager GPEPluginManager;

struct _GPEPluginManager
{
	GtkVBox vbox;

	/*< private > */
	GPEPluginManagerPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GPEPluginManagerClass GPEPluginManagerClass;

struct _GPEPluginManagerClass
{
	GtkVBoxClass parent_class;
};

/*
 * Public methods
 */
GType		 gpe_plugin_manager_get_type		(void) G_GNUC_CONST;

GtkWidget	*gpe_plugin_manager_new			(GPEEngine *engine);

G_END_DECLS

#endif  /* __GPE_PLUGIN_MANAGER_H__  */
