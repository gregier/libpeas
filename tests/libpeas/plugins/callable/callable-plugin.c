/*
 * callable-plugin.c
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <libpeas/peas.h>

#include "introspection-callable.h"

#include "callable-plugin.h"

static void introspection_callable_iface_init (IntrospectionCallableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (TestingCallablePlugin,
                                testing_callable_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (INTROSPECTION_TYPE_CALLABLE,
                                                               introspection_callable_iface_init))

static void
testing_callable_plugin_init (TestingCallablePlugin *plugin)
{
}

static const gchar *
testing_callable_plugin_call_with_return (IntrospectionCallable *callable)
{
  return "Hello, World!";
}

static void
testing_callable_plugin_call_single_arg (IntrospectionCallable *callable,
                                         gboolean              *called)
{
  *called = TRUE;
}

static void
testing_callable_plugin_call_multi_args (IntrospectionCallable *callable,
                                         gboolean              *called_1,
                                         gboolean              *called_2,
                                         gboolean              *called_3)
{
  *called_1 = TRUE;
  *called_2 = TRUE;
  *called_3 = TRUE;
}

static void
testing_callable_plugin_class_init (TestingCallablePluginClass *klass)
{
}

static void
introspection_callable_iface_init (IntrospectionCallableInterface *iface)
{
  iface->call_with_return = testing_callable_plugin_call_with_return;
  iface->call_single_arg = testing_callable_plugin_call_single_arg;
  iface->call_multi_args = testing_callable_plugin_call_multi_args;
}

static void
testing_callable_plugin_class_finalize (TestingCallablePluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  testing_callable_plugin_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              INTROSPECTION_TYPE_CALLABLE,
                                              TESTING_TYPE_CALLABLE_PLUGIN);
}
