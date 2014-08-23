/*
 * extension-c.c
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "libpeas/peas.h"

#include "testing/testing-extension.h"
#include "introspection/introspection-base.h"

static void
test_extension_c_instance_refcount (PeasEngine     *engine,
                                    PeasPluginInfo *info)
{
  PeasExtension *extension;

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_BASE,
                                            NULL);

  g_assert (PEAS_IS_EXTENSION (extension));

  /* The refcount of the returned object should be 1:
   *  - one ref for the PeasExtension
   */
  g_assert_cmpint (extension->ref_count, ==, 1);

  g_object_unref (extension);
}

static void
test_extension_c_nonexistent (PeasEngine *engine)
{
  PeasPluginInfo *info;

  testing_util_push_log_hook ("Failed to load module 'extension-c-nonexistent'*");
  testing_util_push_log_hook ("Error loading plugin 'extension-c-nonexistent'");

  info = peas_engine_get_plugin_info (engine, "extension-c-nonexistent");

  g_assert (!peas_engine_load_plugin (engine, info));
}

static void
test_extension_c_local_linkage (PeasEngine     *engine,
                                PeasPluginInfo *info)
{
  PeasPluginInfo *loadable_info;
  PeasExtension *c_extension, *loadable_extension;
  gpointer c_global_symbol, loadable_global_symbol;

  loadable_info = peas_engine_get_plugin_info (engine, "loadable");
  g_assert (peas_engine_load_plugin (engine, loadable_info));

  c_extension = peas_engine_create_extension (engine, info,
                                              INTROSPECTION_TYPE_BASE,
                                              NULL);
  loadable_extension = peas_engine_create_extension (engine, loadable_info,
                                                     PEAS_TYPE_ACTIVATABLE,
                                                     NULL);

  g_assert (PEAS_IS_EXTENSION (c_extension));
  g_assert (PEAS_IS_EXTENSION (loadable_extension));
  g_assert (c_extension != loadable_extension);

  g_object_get (c_extension,
                "global-symbol-clash", &c_global_symbol,
                NULL);
  g_object_get (loadable_extension,
                "global-symbol-clash", &loadable_global_symbol,
                NULL);

  /* Both plugins export the same global variable,
   * check that they are not the same global reference
   */
  g_assert (c_global_symbol != loadable_global_symbol);

  g_object_unref (loadable_extension);
  g_object_unref (c_extension);
}

static void
test_extension_c_missing_symbol (PeasEngine *engine)
{
  PeasPluginInfo *info;

  testing_util_push_log_hook ("Failed to get 'peas_register_types' for "
                              "module 'extension-c-missing-symbol'*");
  testing_util_push_log_hook ("Error loading plugin "
                              "'extension-c-missing-symbol'");

  info = peas_engine_get_plugin_info (engine, "extension-c-missing-symbol");

  g_assert (!peas_engine_load_plugin (engine, info));
}

int
main (int   argc,
      char *argv[])
{
  testing_init (&argc, &argv);

  /* Only test the basics */
  testing_extension_basic ("c");

  /* We still need to add the callable tests
   * because of peas_extension_call()
   */
  testing_extension_callable ("c");

  EXTENSION_TEST (c, "instance-refcount", instance_refcount);
  EXTENSION_TEST (c, "nonexistent", nonexistent);
  EXTENSION_TEST (c, "local-linkage", local_linkage);
  EXTENSION_TEST (c, "missing-symbol", missing_symbol);

  return testing_extension_run_tests ();
}
