/*
 * callable-plugin.h
 * This file is part of libpeas
 *
 * Copyright (C) 2010 - Garrett Regier
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

#ifndef __TESTING_CALLABLE_PLUGIN_H__
#define __TESTING_CALLABLE_PLUGIN_H__

#include <libpeas/peas.h>

G_BEGIN_DECLS

#define TESTING_TYPE_CALLABLE_PLUGIN         (testing_callable_plugin_get_type ())
#define TESTING_CALLABLE_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TESTING_TYPE_CALLABLE_PLUGIN, TestingCallablePlugin))
#define TESTING_CALLABLE_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TESTING_TYPE_CALLABLE_PLUGIN, TestingCallablePlugin))
#define TESTING_IS_CALLABLE_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TESTING_TYPE_CALLABLE_PLUGIN))
#define TESTING_IS_CALLABLE_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TESTING_TYPE_CALLABLE_PLUGIN))
#define TESTING_CALLABLE_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TESTING_TYPE_CALLABLE_PLUGIN, TestingCallablePluginClass))

typedef struct _TestingCallablePlugin         TestingCallablePlugin;
typedef struct _TestingCallablePluginClass    TestingCallablePluginClass;

struct _TestingCallablePlugin {
  PeasExtensionBase parent_instance;
};

struct _TestingCallablePluginClass {
  PeasExtensionBaseClass parent_class;
};

GType                 testing_callable_plugin_get_type (void) G_GNUC_CONST;
G_MODULE_EXPORT void  peas_register_types              (PeasObjectModule *module);

G_END_DECLS

#endif /* __TESTING_CALLABLE_PLUGIN_H__ */
