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

#include <stdlib.h>

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
test_plugin_info_verify_full_info (PeasEngine *engine)
{
  PeasPluginInfo *info;
  const gchar **authors;

  info = peas_engine_get_plugin_info (engine, "full-info");

  g_assert (!peas_plugin_info_is_loaded (info));
  g_assert (peas_plugin_info_is_available (info));
  g_assert (peas_plugin_info_is_builtin (info));

  g_assert_cmpstr (peas_plugin_info_get_module_name (info), ==, "full-info");
  g_assert (g_str_has_suffix (peas_plugin_info_get_module_dir (info), "/tests/plugins"));
  g_assert (g_str_has_suffix (peas_plugin_info_get_data_dir (info), "/tests/plugins/full-info"));

  g_assert_cmpstr (peas_plugin_info_get_dependencies (info)[0], ==, "something");
  g_assert_cmpstr (peas_plugin_info_get_dependencies (info)[1], ==, "something-else");
  g_assert_cmpstr (peas_plugin_info_get_dependencies (info)[2], ==, NULL);

  g_assert_cmpstr (peas_plugin_info_get_name (info), ==, "Full Info");
  g_assert_cmpstr (peas_plugin_info_get_description (info), ==, "Has full info.");
  g_assert_cmpstr (peas_plugin_info_get_icon_name (info), ==, "full-info-icon");
  g_assert_cmpstr (peas_plugin_info_get_website (info), ==, "http://live.gnome.org/Libpeas");
  g_assert_cmpstr (peas_plugin_info_get_copyright (info), ==, "Copyright © 2010 Garrett Regier");
  g_assert_cmpstr (peas_plugin_info_get_version (info), ==, "1.0");
  g_assert_cmpstr (peas_plugin_info_get_help_uri (info), ==, "http://git.gnome.org/browse/libpeas");
  g_assert_cmpint (peas_plugin_info_get_iage (info), ==, 2);

  authors = peas_plugin_info_get_authors (info);
  g_assert (authors != NULL && authors[1] == NULL);
  g_assert_cmpstr (authors[0], ==, "Garrett Regier");
}

static void
test_plugin_info_verify_min_info (PeasEngine *engine)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (engine, "min-info");

  g_assert (!peas_plugin_info_is_loaded (info));
  g_assert (peas_plugin_info_is_available (info));
  g_assert (!peas_plugin_info_is_builtin (info));

  g_assert_cmpstr (peas_plugin_info_get_module_name (info), ==, "min-info");
  g_assert (g_str_has_suffix (peas_plugin_info_get_module_dir (info), "/tests/plugins"));
  g_assert (g_str_has_suffix (peas_plugin_info_get_data_dir (info), "/tests/plugins/min-info"));

  g_assert_cmpstr (peas_plugin_info_get_dependencies (info)[0], ==, NULL);

  g_assert_cmpstr (peas_plugin_info_get_name (info), ==, "Min Info");
  g_assert_cmpstr (peas_plugin_info_get_description (info), ==, NULL);
  g_assert_cmpstr (peas_plugin_info_get_icon_name (info), ==, "libpeas-plugin");
  g_assert_cmpstr (peas_plugin_info_get_website (info), ==, NULL);
  g_assert_cmpstr (peas_plugin_info_get_copyright (info), ==, NULL);
  g_assert_cmpstr (peas_plugin_info_get_version (info), ==, NULL);
  g_assert_cmpstr (peas_plugin_info_get_help_uri (info), ==, NULL);
  g_assert_cmpint (peas_plugin_info_get_iage (info), ==, 2);

  g_assert (peas_plugin_info_get_authors (info) == NULL);
}

static void
test_plugin_info_has_dep (PeasEngine *engine)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (engine, "full-info");

  g_assert (peas_plugin_info_has_dependency (info, "something"));
  g_assert (peas_plugin_info_has_dependency (info, "something-else"));
  g_assert (!peas_plugin_info_has_dependency (info, "does-not-exist"));

  if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
      peas_plugin_info_has_dependency (info, NULL);
      exit (0);
    }
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*CRITICAL*");


  info = peas_engine_get_plugin_info (engine, "min-info");

  g_assert (!peas_plugin_info_has_dependency (info, "does-not-exist"));
}

static void
test_plugin_info_missing_iage (PeasEngine *engine)
{
  g_assert (peas_engine_get_plugin_info (engine, "invalid-info-iage") == NULL);
}

static void
test_plugin_info_missing_module (PeasEngine *engine)
{
  g_assert (peas_engine_get_plugin_info (engine, "invalid-info-module") == NULL);
}

static void
test_plugin_info_missing_name (PeasEngine *engine)
{
  g_assert (peas_engine_get_plugin_info (engine, "invalid-info-name") == NULL);
}

int
main (int    argc,
      char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_type_init ();

#define TEST(path, ftest) \
  g_test_add ("/plugin-info/" path, TestFixture, \
              (gpointer) test_plugin_info_##ftest, \
              test_setup, test_runner, test_teardown)

  TEST ("verify-full-info", verify_full_info);
  TEST ("verify-min-info", verify_min_info);

  TEST ("has-dep", has_dep);

  TEST ("missing-iage", missing_iage);
  TEST ("missing-module", missing_module);
  TEST ("missing-name", missing_name);

#undef TEST

  return g_test_run ();
}
