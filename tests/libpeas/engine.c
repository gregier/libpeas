/*
 * engine.c
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
#include <libpeas/peas.h>

#include "testing/testing.h"

typedef struct _TestFixture TestFixture;

struct _TestFixture {
  PeasEngine *engine;
};

static void
test_setup (TestFixture   *fixture,
            gconstpointer  data)
{
  fixture->engine = testing_engine_new ();
}

static void
test_teardown (TestFixture   *fixture,
               gconstpointer  data)
{
  testing_engine_free (fixture->engine);
}

static void
test_runner (TestFixture   *fixture,
             gconstpointer  data)
{
  ((void (*) (PeasEngine *engine)) data) (fixture->engine);
}

static void
test_engine_get_default (PeasEngine *engine)
{
  g_assert (engine != NULL);
  g_assert (engine == peas_engine_get_default ());
}

static void
test_engine_load_plugin (PeasEngine *engine)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (engine, "loadable");

  g_assert (peas_engine_load_plugin (engine, info));
  g_assert (peas_plugin_info_is_loaded (info));
}

static void
test_engine_load_plugin_with_dep (PeasEngine *engine)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (engine, "has-dep");

  g_assert (peas_engine_load_plugin (engine, info));
  g_assert (peas_plugin_info_is_loaded (info));

  info = peas_engine_get_plugin_info (engine, "loadable");

  g_assert (peas_plugin_info_is_loaded (info));
}

static void
test_engine_load_plugin_with_self_dep (PeasEngine *engine)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (engine, "self-dep");

  g_assert (peas_engine_load_plugin (engine, info));
  g_assert (peas_plugin_info_is_loaded (info));
}

static void
test_engine_unload_plugin (PeasEngine *engine)
{
  PeasPluginInfo *info;

  test_engine_load_plugin (engine);

  info = peas_engine_get_plugin_info (engine, "loadable");

  g_assert (peas_engine_unload_plugin (engine, info));
  g_assert (!peas_plugin_info_is_loaded (info));
}

static void
test_engine_unload_plugin_with_dep (PeasEngine *engine)
{
  PeasPluginInfo *info;

  test_engine_load_plugin_with_dep (engine);

  info = peas_engine_get_plugin_info (engine, "loadable");

  g_assert (peas_engine_unload_plugin (engine, info));
  g_assert (!peas_plugin_info_is_loaded (info));

  info = peas_engine_get_plugin_info (engine, "has-dep");

  g_assert (!peas_plugin_info_is_loaded (info));
}

static void
test_engine_unload_plugin_with_self_dep (PeasEngine *engine)
{
  PeasPluginInfo *info;

  test_engine_load_plugin_with_self_dep (engine);

  info = peas_engine_get_plugin_info (engine, "self-dep");

  g_assert (peas_engine_unload_plugin (engine, info));
  g_assert (!peas_plugin_info_is_loaded (info));
}

static void
test_engine_unavailable_plugin (PeasEngine *engine)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (engine, "unavailable");

  g_assert (!peas_engine_load_plugin (engine, info));
  g_assert (!peas_plugin_info_is_loaded (info));
  g_assert (!peas_plugin_info_is_available (info));
}

static void
load_plugin_cb (PeasEngine     *engine,
                PeasPluginInfo *info,
                gint           *loaded)
{
  ++(*loaded);
}

static void
unload_plugin_cb (PeasEngine     *engine,
                  PeasPluginInfo *info,
                  gint           *loaded)
{
  --(*loaded);
}

static void
notify_loaded_plugins_cb (PeasEngine   *engine,
                          GParamSpec   *pspec,
                          gchar      ***loaded_plugins)
{
  if (*loaded_plugins != NULL)
    g_strfreev (*loaded_plugins);

  *loaded_plugins = peas_engine_get_loaded_plugins (engine);
}

static void
test_engine_loaded_plugins (PeasEngine *engine)
{
  PeasPluginInfo *info;
  gint loaded = 0;
  gchar **loaded_plugins = NULL;

  g_signal_connect (engine,
                    "load-plugin",
                    G_CALLBACK (load_plugin_cb),
                    (gpointer) &loaded);
  g_signal_connect (engine,
                    "unload-plugin",
                    G_CALLBACK (unload_plugin_cb),
                    (gpointer) &loaded);
  g_signal_connect (engine,
                    "notify::loaded-plugins",
                    G_CALLBACK (notify_loaded_plugins_cb),
                    (gpointer) &loaded_plugins);


  info = peas_engine_get_plugin_info (engine, "loadable");

  g_assert (peas_engine_load_plugin (engine, info));

  g_assert_cmpint (loaded, ==, 1);
  g_assert (loaded_plugins != NULL);
  g_assert_cmpstr (loaded_plugins[0], ==, "loadable");
  g_assert (loaded_plugins[1] == NULL);

  g_assert (peas_engine_unload_plugin (engine, info));
  g_assert (loaded_plugins == NULL);
  g_assert_cmpint (loaded, ==, 0);


#ifdef CANNOT_TEST
  /* Cannot be done as unrefing the engine causes
   * issues when another test is run
   */

  g_assert (peas_engine_load_plugin (engine, info));

  g_assert_cmpint (loaded, ==, 1);
  g_assert (loaded_plugins != NULL);
  g_assert_cmpstr (loaded_plugins[0], ==, "loadable");
  g_assert (loaded_plugins[1] == NULL);

  g_object_unref (engine);

  g_assert_cmpint (loaded, ==, 0);
  g_assert (loaded_plugins != NULL);
  g_assert_cmpstr (loaded_plugins[0], ==, "loadable");
  g_assert (loaded_plugins[1] == NULL);

  if (loaded_plugins != NULL)
    g_strfreev (loaded_plugins);
#endif

  g_signal_handlers_disconnect_by_func (engine, load_plugin_cb, &loaded);
  g_signal_handlers_disconnect_by_func (engine, unload_plugin_cb, &loaded);
  g_signal_handlers_disconnect_by_func (engine,
                                        notify_loaded_plugins_cb,
                                        &loaded_plugins);
}

#ifdef CANNOT_TEST
static void
test_engine_disable_loader (PeasEngine *engine)
{
  PeasPluginInfo *info;

  /* Cannot be run because the engine
   * has already loaded the C plugin loader
   */

  info = peas_engine_get_plugin_info (engine, "loadable");

  peas_engine_disable_loader (engine, "C");

  g_assert (!peas_engine_load_plugin (engine, info));
  g_assert (!peas_plugin_info_is_loaded (info));
}
#endif

int
main (int    argc,
      char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_type_init ();

#define TEST(path, ftest) \
  g_test_add ("/engine/" path, TestFixture, \
              (gpointer) test_engine_##ftest, \
              test_setup, test_runner, test_teardown)

  TEST ("get-default", get_default);

  TEST ("load-plugin", load_plugin);
  TEST ("load-plugin-with-dep", load_plugin_with_dep);
  TEST ("load-plugin-with-self-dep", load_plugin_with_self_dep);

  TEST ("unload-plugin", unload_plugin);
  TEST ("unload-plugin-with-dep", unload_plugin_with_dep);
  TEST ("unload-plugin-with-self-dep", unload_plugin_with_self_dep);

  TEST ("unavailable-plugin", unavailable_plugin);

  TEST ("loaded-plugins", loaded_plugins);

#ifdef CANNOT_TEST
  TEST ("disable-loader", disable_loader);
#endif

#undef TEST

  g_object_unref (peas_engine_get_default ());

  return g_test_run ();
}
