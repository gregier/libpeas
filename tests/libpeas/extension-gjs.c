/*
 * extension-gjs.c
 * This file is part of libpeas
 *
 * Copyright (C) 2011 - Garrett Regier
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

#include <gjs/gi/value.h>

#include "loaders/gjs/peas-extension-gjs.h"

#include "testing/testing-extension.h"
#include "introspection/introspection-callable.h"

static void
test_extension_gjs_plugin_info (PeasEngine *engine)
{
  PeasPluginInfo *info;
  PeasExtension *extension;
  PeasExtensionGjs *gexten;
  jsval js_value;
  GValue gvalue = { 0 };

  info = peas_engine_get_plugin_info (engine, "extension-gjs");

  g_assert (peas_engine_load_plugin (engine, info));

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_CALLABLE,
                                            NULL);

  g_assert (PEAS_IS_EXTENSION (extension));

  gexten = (PeasExtensionGjs *) extension;

  g_value_init (&gvalue, PEAS_TYPE_PLUGIN_INFO);

  g_assert (JS_GetProperty (gexten->js_context, gexten->js_object,
                            "plugin_info", &js_value));
  g_assert (gjs_value_to_g_value (gexten->js_context, js_value, &gvalue));

  g_assert (g_value_get_boxed (&gvalue) == info);

  g_value_unset (&gvalue);

  g_object_unref (extension);
}

int
main (int   argc,
      char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_type_init ();

  EXTENSION_TESTS (gjs);

  EXTENSION_TEST (gjs, "plugin-info", plugin_info);

  return testing_run_tests ();
}
