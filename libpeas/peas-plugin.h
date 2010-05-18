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

/**
 * PEAS_REGISTER_TYPE_WITH_CODE(PARENT_TYPE, PluginName, plugin_name, CODE):
 * @PARENT_TYPE: The PeasPlugin subclass used as the parent type.
 * @PluginName: The name of the new plugin class, in CamelCase.
 * @plugin_name: The name of the new plugin class, in lower_case.
 * @CODE: Custom code that gets inserted in the *_get_type() function.
 *
 * A convenience macro for plugin implementations, which declares a subclass
 * of PARENT_TYPE (see G_DEFINE_DYNAMIC_TYPE_EXTENDED()) and a registration
 * function.  The resulting #GType is to be registered in a #GTypeModule.
 *
 * @CODE will be included in the resulting *_get_type() function, and will
 * usually consist of G_IMPLEMENT_INTERFACE() calls and eventually on
 * additional type registration
 */
#define PEAS_REGISTER_TYPE_WITH_CODE(PARENT_TYPE, PluginName, plugin_name, CODE) \
        G_DEFINE_DYNAMIC_TYPE_EXTENDED (PluginName,                           \
                                        plugin_name,                          \
                                        PARENT_TYPE,                          \
                                        0,                                    \
                                        CODE)                                 \
                                                                              \
/* This is not very nice, but G_DEFINE_DYNAMIC wants it and our old macro     \
 * did not support it */                                                      \
static void                                                                   \
plugin_name##_class_finalize (PluginName##Class *klass)                       \
{                                                                             \
}                                                                             \
                                                                              \
G_MODULE_EXPORT GObject *                                                     \
register_peas_plugin (GTypeModule   *type_module)                             \
{                                                                             \
        plugin_name##_register_type (type_module);                            \
                                                                              \
        return g_object_new (plugin_name##_get_type(),                        \
                             NULL);                                           \
}

/**
 * PEAS_REGISTER_TYPE(PARENT_TYPE, PluginName, plugin_name):
 * @PARENT_TYPE: The PeasPlugin subclass used as the parent type.
 * @PluginName: The name of the new plugin class, in CamelCase.
 * @plugin_name: The name of the new plugin class, in lower_case.
 *
 * A convenience macro for plugin implementations, which declares a subclass
 * of PARENT_TYPE (see G_DEFINE_DYNAMIC_TYPE_EXTENDED()) and a registration
 * function.  The resulting #GType is to be registered in a #GTypeModule.
 */
#define PEAS_REGISTER_TYPE(PARENT_TYPE, PluginName, plugin_name)              \
        PEAS_REGISTER_TYPE_WITH_CODE(PARENT_TYPE, PluginName, plugin_name, ;)

G_END_DECLS

#endif /* __PEAS_PLUGIN_H__ */
