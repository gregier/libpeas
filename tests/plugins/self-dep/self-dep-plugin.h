/*
 * self-dep-plugin.h
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

#ifndef __TESTING_SELF_DEP_PLUGIN_H__
#define __TESTING_SELF_DEP_PLUGIN_H__

#include <libpeas/peas.h>

G_BEGIN_DECLS

#define TESTING_TYPE_SELF_DEP_PLUGIN         (testing_self_dep_plugin_get_type ())
#define TESTING_SELF_DEP_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TESTING_TYPE_SELF_DEP_PLUGIN, TestingSelfDepPlugin))
#define TESTING_SELF_DEP_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TESTING_TYPE_SELF_DEP_PLUGIN, TestingSelfDepPlugin))
#define TESTING_IS_SELF_DEP_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TESTING_TYPE_SELF_DEP_PLUGIN))
#define TESTING_IS_SELF_DEP_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TESTING_TYPE_SELF_DEP_PLUGIN))
#define TESTING_SELF_DEP_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TESTING_TYPE_SELF_DEP_PLUGIN, TestingSelfDepPluginClass))

typedef struct _TestingSelfDepPlugin         TestingSelfDepPlugin;
typedef struct _TestingSelfDepPluginClass    TestingSelfDepPluginClass;

struct _TestingSelfDepPlugin {
  PeasExtensionBase parent_instance;
};

struct _TestingSelfDepPluginClass {
  PeasExtensionBaseClass parent_class;
};

GType                 testing_self_dep_plugin_get_type (void) G_GNUC_CONST;
G_MODULE_EXPORT void  peas_register_types              (PeasObjectModule *module);

G_END_DECLS

#endif /* __TESTING_SELF_DEP_PLUGIN_H__ */
