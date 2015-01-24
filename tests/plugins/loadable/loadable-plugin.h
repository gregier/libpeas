/*
 * loadable-plugin.h
 * This file is part of libpeas
 *
 * Copyright (C) 2010 - Garrett Regier
 *
 * libpeas is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libpeas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 */

#ifndef __TESTING_LOADABLE_PLUGIN_H__
#define __TESTING_LOADABLE_PLUGIN_H__

#include <libpeas/peas.h>

G_BEGIN_DECLS

#define TESTING_TYPE_LOADABLE_PLUGIN         (testing_loadable_plugin_get_type ())
#define TESTING_LOADABLE_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TESTING_TYPE_LOADABLE_PLUGIN, TestingLoadablePlugin))
#define TESTING_LOADABLE_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TESTING_TYPE_LOADABLE_PLUGIN, TestingLoadablePlugin))
#define TESTING_IS_LOADABLE_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TESTING_TYPE_LOADABLE_PLUGIN))
#define TESTING_IS_LOADABLE_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TESTING_TYPE_LOADABLE_PLUGIN))
#define TESTING_LOADABLE_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TESTING_TYPE_LOADABLE_PLUGIN, TestingLoadablePluginClass))

typedef struct _TestingLoadablePlugin         TestingLoadablePlugin;
typedef struct _TestingLoadablePluginClass    TestingLoadablePluginClass;

struct _TestingLoadablePlugin {
  /* Inherit from GObject and not PeasExtensionBase
   * to check that it is possible
   */
  GObject parent_instance;
};

struct _TestingLoadablePluginClass {
  PeasExtensionBaseClass parent_class;
};

/* Used by the local linkage test */
G_MODULE_EXPORT gpointer global_symbol_clash;

GType                 testing_loadable_plugin_get_type (void) G_GNUC_CONST;
G_MODULE_EXPORT void  peas_register_types              (PeasObjectModule *module);

G_END_DECLS

#endif /* __TESTING_LOADABLE_PLUGIN_H__ */
