/*
 * peas-seed-plugin.c
 * This file is part of libpeas
 *
 * Copyright (C) 2009 - Steve Frécinaux
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

#include "peas-seed-plugin.h"

G_DEFINE_DYNAMIC_TYPE (PeasSeedPlugin, peas_seed_plugin, PEAS_TYPE_PLUGIN);

enum {
  PROP_0,
  PROP_JS_CONTEXT,
  PROP_JS_PLUGIN,
};

static void
peas_seed_plugin_init (PeasSeedPlugin *splugin)
{
}

static void
peas_seed_plugin_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PeasSeedPlugin *splugin = PEAS_SEED_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_JS_CONTEXT:
      splugin->js_context = g_value_get_pointer (value);
      break;
    case PROP_JS_PLUGIN:
      splugin->js_plugin = g_value_get_pointer (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
peas_seed_plugin_constructed (GObject *object)
{
  PeasSeedPlugin *splugin = PEAS_SEED_PLUGIN (object);

  g_return_if_fail (splugin->js_context != NULL);
  g_return_if_fail (splugin->js_plugin != NULL);

  /* We do this here as we can't be sure the context is already set when
   * the "JS_PLUGIN" property is set. */
  seed_context_ref (splugin->js_context);
  seed_value_protect (splugin->js_context, splugin->js_plugin);
}

static void
peas_seed_plugin_finalize (GObject *object)
{
  PeasSeedPlugin *splugin = PEAS_SEED_PLUGIN (object);

  g_return_if_fail (splugin->js_context != NULL);
  g_return_if_fail (splugin->js_plugin != NULL);

  seed_value_unprotect (splugin->js_context, splugin->js_plugin);
  seed_context_unref (splugin->js_context);
}

static void
peas_seed_plugin_call_js_method (PeasSeedPlugin *splugin,
                                 const gchar    *method_name,
                                 GObject        *object)
{
  SeedValue js_method;
  SeedValue js_method_args[1];
  SeedException exc = NULL;
  gchar *exc_string;

  g_return_if_fail (splugin->js_context != NULL);
  g_return_if_fail (splugin->js_plugin != NULL);

  js_method = seed_object_get_property (splugin->js_context,
                                        splugin->js_plugin,
                                        method_name);

  if (seed_value_is_undefined (splugin->js_context, js_method))
    return;

  /* We want to display an error if the method is defined but is not a function. */
  g_return_if_fail (seed_value_is_function (splugin->js_context, js_method));

  js_method_args[0] = seed_value_from_object (splugin->js_context, object, &exc);
  if (exc)
    {
      exc_string = seed_exception_to_string (splugin->js_context, exc);
      g_warning ("Seed Exception: %s", exc_string);
      g_free (exc_string);
      return;
    }

  seed_object_call (splugin->js_context,
                    js_method,
                    splugin->js_plugin,
                    G_N_ELEMENTS (js_method_args),
                    js_method_args,
                    &exc);

  if (exc)
    {
      exc_string = seed_exception_to_string (splugin->js_context, exc);
      g_warning ("Seed Exception: %s", exc_string);
      g_free (exc_string);
    }
}

static void
peas_seed_plugin_activate (PeasPlugin *plugin,
                           GObject *object)
{
  peas_seed_plugin_call_js_method (PEAS_SEED_PLUGIN (plugin),
                                   "activate",
                                   object);
}

static void
peas_seed_plugin_deactivate (PeasPlugin *plugin,
                             GObject *object)
{
  peas_seed_plugin_call_js_method (PEAS_SEED_PLUGIN (plugin),
                                   "deactivate",
                                    object);
}

static void
peas_seed_plugin_update_ui (PeasPlugin *plugin,
                            GObject *object)
{
  peas_seed_plugin_call_js_method (PEAS_SEED_PLUGIN (plugin),
                                   "update_ui",
                                   object);
}

static void
peas_seed_plugin_class_init (PeasSeedPluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PeasPluginClass *plugin_class = PEAS_PLUGIN_CLASS (klass);

  object_class->set_property = peas_seed_plugin_set_property;
  object_class->constructed = peas_seed_plugin_constructed;
  object_class->finalize = peas_seed_plugin_finalize;

  plugin_class->activate = peas_seed_plugin_activate;
  plugin_class->deactivate = peas_seed_plugin_deactivate;
  plugin_class->update_ui = peas_seed_plugin_update_ui;

  g_object_class_install_property (object_class,
                                   PROP_JS_CONTEXT,
                                   g_param_spec_pointer ("js-context",
                                                         "Javascript Context",
                                                         "A Javascript context from Seed",
                                                         G_PARAM_WRITABLE |
                                                         G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class,
                                   PROP_JS_PLUGIN,
                                   g_param_spec_pointer ("js-plugin",
                                                         "Javascript Plugin Object",
                                                         "A Javascript plugin object from Seed",
                                                         G_PARAM_WRITABLE |
                                                         G_PARAM_CONSTRUCT_ONLY));
}

static void
peas_seed_plugin_class_finalize (PeasSeedPluginClass *klass)
{
}

void
peas_seed_plugin_register_type_ext (GTypeModule *type_module)
{
  peas_seed_plugin_register_type (type_module);
}

PeasPlugin *
peas_seed_plugin_new (PeasPluginInfo *info,
                      SeedContext     js_context,
                      SeedObject      js_plugin)
{
  g_return_val_if_fail (js_context != NULL, NULL);
  g_return_val_if_fail (js_plugin != NULL, NULL);

  return PEAS_PLUGIN (g_object_new (PEAS_TYPE_SEED_PLUGIN,
                                    "js-context", js_context,
                                    "js-plugin", js_plugin,
                                    "plugin-info", info,
                                    NULL));
}
