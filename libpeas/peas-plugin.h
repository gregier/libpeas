/*
 * peas-plugin.h
 * This file is part of libpeas
 *
 * Copyright (C) 2002-2005 - Paolo Maggi
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

#ifndef __PEAS_PLUGIN_H__
#define __PEAS_PLUGIN_H__

#include <glib-object.h>

#include "peas-plugin-info.h"

/* TODO: add a .h file that includes all the .h files normally needed to
 * develop a plugin */

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PEAS_TYPE_PLUGIN            (peas_plugin_get_type())
#define PEAS_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PEAS_TYPE_PLUGIN, PeasPlugin))
#define PEAS_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PEAS_TYPE_PLUGIN, PeasPluginClass))
#define PEAS_IS_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PEAS_TYPE_PLUGIN))
#define PEAS_IS_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PEAS_TYPE_PLUGIN))
#define PEAS_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PEAS_TYPE_PLUGIN, PeasPluginClass))

/**
 * PeasPlugin:
 * @parent: the parent object.
 *
 * Base class for plugins.
 */
typedef struct _PeasPlugin        PeasPlugin;
typedef struct _PeasPluginClass   PeasPluginClass;
typedef struct _PeasPluginPrivate PeasPluginPrivate;

/**
 * PeasFunc:
 * @plugin: A #PeasPlugin;
 * @object: The target #GObject on which the handler is to be executed.
 *
 * The type of the handler methods of #PeasPlugin.
 */
typedef void (*PeasFunc) (PeasPlugin *plugin,
                          GObject    *object);

struct _PeasPlugin {
  GObject parent;

  /*< private >*/
  PeasPluginPrivate *priv;
};

struct _PeasPluginClass {
  GObjectClass parent_class;

  /* Padding for future expansion */
  void        (*_peas_reserved1)          (void);
  void        (*_peas_reserved2)          (void);
  void        (*_peas_reserved3)          (void);
  void        (*_peas_reserved4)          (void);
};

/*
 * Public methods
 */
GType peas_plugin_get_type (void)  G_GNUC_CONST;

PeasPluginInfo   *peas_plugin_get_info                (PeasPlugin *plugin);
gchar            *peas_plugin_get_data_dir            (PeasPlugin *plugin);

G_END_DECLS

#endif /* __PEAS_PLUGIN_H__ */
