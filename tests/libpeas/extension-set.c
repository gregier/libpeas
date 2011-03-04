/*
 * extension-set.c
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

/* TODO:
 *        - Check that extensions sets only contain extensions of their type
 */

typedef struct _TestFixture TestFixture;

struct _TestFixture {
  PeasEngine *engine;
  PeasExtensionSet *extension_set;
  gint active;
};

/* Have dependencies before the plugin that requires them */
static const gchar *loadable_plugins[] = {
  "loadable", "has-dep", "self-dep"
};

static void
extension_added_cb (PeasExtensionSet *set,
                    PeasPluginInfo   *info,
                    PeasExtension    *exten,
                    TestFixture      *fixture)
{
  ++fixture->active;
}

static void
extension_removed_cb (PeasExtensionSet *set,
                      PeasPluginInfo   *info,
                      PeasExtension    *exten,
                      TestFixture      *fixture)
{
  --fixture->active;
}

static void
test_setup (TestFixture   *fixture,
            gconstpointer  data)
{
  fixture->engine = testing_engine_new ();

  fixture->extension_set = peas_extension_set_new (fixture->engine,
                                                   PEAS_TYPE_ACTIVATABLE,
                                                   "object", NULL,
                                                   NULL);

  g_signal_connect (fixture->extension_set,
                    "extension-added",
                    G_CALLBACK (extension_added_cb),
                    fixture);
  g_signal_connect (fixture->extension_set,
                    "extension-removed",
                    G_CALLBACK (extension_removed_cb),
                    fixture);

  fixture->active = 0;
}

static void
test_teardown (TestFixture   *fixture,
               gconstpointer  data)
{
  g_object_unref (fixture->extension_set);
  g_assert_cmpint (fixture->active, ==, 0);

  testing_engine_free (fixture->engine);
}

static void
test_runner (TestFixture   *fixture,
             gconstpointer  data)
{
  ((void (*) (TestFixture *fixture)) data) (fixture);
}

static void
test_extension_set_no_extensions (TestFixture *fixture)
{
  /* Done in teardown */
}

static void
test_extension_set_activate (TestFixture *fixture)
{
  gint i;
  PeasPluginInfo *info;

  for (i = 0; i < G_N_ELEMENTS (loadable_plugins); ++i)
    {
      g_assert_cmpint (fixture->active, ==, i);

      info = peas_engine_get_plugin_info (fixture->engine,
                                          loadable_plugins[i]);

      g_assert (peas_engine_load_plugin (fixture->engine, info));
    }

  g_assert_cmpint (fixture->active, ==, G_N_ELEMENTS (loadable_plugins));
}

static void
test_extension_set_deactivate (TestFixture *fixture)
{
  gint i;
  PeasPluginInfo *info;

  test_extension_set_activate (fixture);

  /* To keep deps in order */
  for (i = G_N_ELEMENTS (loadable_plugins); i > 0; --i)
    {
      g_assert_cmpint (fixture->active, ==, i);

      info = peas_engine_get_plugin_info (fixture->engine,
                                          loadable_plugins[i - 1]);

      g_assert (peas_engine_unload_plugin (fixture->engine, info));
    }

  g_assert_cmpint (fixture->active, ==, 0);
}

static void
test_extension_set_get_extension (TestFixture *fixture)
{
  PeasPluginInfo *info;
  PeasExtension *extension;

  info = peas_engine_get_plugin_info (fixture->engine, loadable_plugins[0]);

  g_assert (peas_extension_set_get_extension (fixture->extension_set, info) == NULL);
  g_assert (peas_engine_load_plugin (fixture->engine, info));

  extension = peas_extension_set_get_extension (fixture->extension_set, info);

  g_assert (PEAS_IS_ACTIVATABLE (extension));
}

static void
test_extension_set_call_valid (TestFixture *fixture)
{
  test_extension_set_activate (fixture);

  g_assert (peas_extension_set_call (fixture->extension_set, "activate", NULL));
}

static void
test_extension_set_call_invalid (TestFixture *fixture)
{
  test_extension_set_activate (fixture);

  if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
      peas_extension_set_call (fixture->extension_set, "invalid", NULL);
      exit (0);
    }
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*Method 'PeasActivatable.invalid' not found*");
}

int
main (int    argc,
      char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_type_init ();

#define TEST(path, ftest) \
  g_test_add ("/extension-set/" path, TestFixture, \
              (gpointer) test_extension_set_##ftest, \
              test_setup, test_runner, test_teardown)

  TEST ("no-extensions", no_extensions);
  TEST ("activate", activate);
  TEST ("deactivate", deactivate);

  TEST ("get-extension", get_extension);

  TEST ("call-valid", call_valid);
  TEST ("call-invalid", call_invalid);

#undef TEST

  return testing_run_tests ();
}
