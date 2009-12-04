/*
 * peas-plugin-manager.h
 * This file is part of libpeas
 *
 * Copyright (C) 2005-2009 Paolo Maggi, Paolo Borelli
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

#ifndef __PEAS_PLUGIN_MANAGER_H__
#define __PEAS_PLUGIN_MANAGER_H__

#include <gtk/gtk.h>
#include <libpeas/peas-engine.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PEAS_TYPE_PLUGIN_MANAGER              (peas_plugin_manager_get_type())
#define PEAS_PLUGIN_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), PEAS_TYPE_PLUGIN_MANAGER, PeasPluginManager))
#define PEAS_PLUGIN_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), PEAS_TYPE_PLUGIN_MANAGER, PeasPluginManagerClass))
#define PEAS_IS_PLUGIN_MANAGER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), PEAS_TYPE_PLUGIN_MANAGER))
#define PEAS_IS_PLUGIN_MANAGER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PEAS_TYPE_PLUGIN_MANAGER))
#define PEAS_PLUGIN_MANAGER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), PEAS_TYPE_PLUGIN_MANAGER, PeasPluginManagerClass))

typedef struct _PeasPluginManager         PeasPluginManager;
typedef struct _PeasPluginManagerClass    PeasPluginManagerClass;
typedef struct _PeasPluginManagerPrivate  PeasPluginManagerPrivate;

struct _PeasPluginManager
{
  GtkVBox vbox;

  /*< private > */
  PeasPluginManagerPrivate *priv;
};

struct _PeasPluginManagerClass
{
  GtkVBoxClass parent_class;
};

GType       peas_plugin_manager_get_type  (void)  G_GNUC_CONST;
GtkWidget  *peas_plugin_manager_new       (PeasEngine *engine);

G_END_DECLS

#endif /* __PEAS_PLUGIN_MANAGER_H__  */
