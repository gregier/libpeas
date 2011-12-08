/*
 * plugin-version.c
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include <glib.h>

#include "libpeas/peas-plugin-version.h"

#include "testing/testing.h"

typedef struct {
  const gchar *version;
  const gchar *warning;
} TestWarning;

static void
test_warnings (TestWarning *tests)
{
  gint i;

  for (i = 0; tests[i].version != NULL; ++i)
    {
      testing_util_push_log_hook (tests[i].warning);
      g_assert (peas_plugin_version_new (tests[i].version) == NULL);
      testing_util_pop_log_hook ();
    }
}

static void
check_versions (const gchar                **versions,
                PeasPluginVersionOperation   op)
{
  gint i;
  PeasPluginVersion *old;

  old = peas_plugin_version_new (versions[0]);

  for (i = 1; versions[i] != NULL; ++i)
    {
      PeasPluginVersion *new = peas_plugin_version_new (versions[i]);

      g_assert (peas_plugin_version_check (old, new, op));

      peas_plugin_version_unref (old);

      old = new;
    }

  peas_plugin_version_unref (old);
}

static void
test_plugin_version_to_string (void)
{
  gint i;
  const gchar *versions[] = {
    "1",   "1.2",   "1.2.3", "123.456.789",
    "1.*", "1.2.*", "0.*",   "0.0.*"
  };

  for (i = 0; i < G_N_ELEMENTS (versions); ++i)
    {
      PeasPluginVersion *version;
      gchar *version_str;

      version = peas_plugin_version_new (versions[i]);
      version_str = peas_plugin_version_to_string (version);

      g_assert_cmpstr (versions[i], ==, version_str);

      g_free (version_str);
      peas_plugin_version_unref (version);
    }
}

static void
test_plugin_version_operation_equal (void)
{
  gint i;
  const gchar *versions[3][6] = {
    { "1",     "1.0",   "1.0.0", "1.0.*", "1.*", NULL },
    { "1.2",   "1.2.0", "1.2.*", "1.*",   NULL },
    { "1.2.3", "1.2.*", "1.*",   NULL }
  };

  for (i = 0; i < G_N_ELEMENTS (versions); ++i)
    check_versions (versions[i], PEAS_PLUGIN_VERSION_OPERATION_EQ);
}

static void
test_plugin_version_operation_not_equal (void)
{
  const gchar *versions[] = { "1", "1.1", "1.1.1", NULL };

  check_versions (versions, PEAS_PLUGIN_VERSION_OPERATION_NE);
}

static void
test_plugin_version_operation_less (void)
{
  const gchar *versions[] = { "1", "1.1", "1.1.1", NULL };

  check_versions (versions, PEAS_PLUGIN_VERSION_OPERATION_LT);
}

static void
test_plugin_version_operation_greater (void)
{
  const gchar *versions[] = { "1.1.1", "1.1", "1", NULL };

  check_versions (versions, PEAS_PLUGIN_VERSION_OPERATION_GT);
}

static void
test_plugin_version_invalid_operation (void)
{
  PeasPluginVersion *a, *b;

  testing_util_push_log_hook ("Mixed operations are in use");

  a = peas_plugin_version_new ("1");
  b = peas_plugin_version_new ("2");

  peas_plugin_version_check (a, b,
                             PEAS_PLUGIN_VERSION_OPERATION_EQ |
                             PEAS_PLUGIN_VERSION_OPERATION_NE);

  peas_plugin_version_unref (a);
  peas_plugin_version_unref (b);
}

static void
test_plugin_version_invalid_version (void)
{
  TestWarning tests[] = {
    { "0",       "Invalid version*"   },
    { "0.0",     "Invalid version*"   },
    { "0.0.0",   "Invalid version*"   },
    { NULL }
  };

  test_warnings (tests);
}

static void
test_plugin_version_invalid_dot (void)
{
  TestWarning tests[] = {
    { ".",        "Invalid character '.'*"      },
    { ".1",       "Invalid character '.'*"      },
    { "1..2",     "Number missing after dot*"   },
    { "1.",       "Number missing after dot*"   },
    { "1.2.",     "Number missing after dot*"   },
    { "1.2.3.",   "Too many dots*"              },
    { NULL }
  };

  test_warnings (tests);
}

static void
test_plugin_version_invalid_space (void)
{
  TestWarning tests[] = {
    { " 1",     "Invalid character ' '*"   },
    { "1 ",     "Invalid character ' '*"   },
    { "1 2",    "Invalid character ' '*"   },
    { "1.* ",   "Star does not end*"       },
    { NULL }
  };

  test_warnings (tests);
}

static void
test_plugin_version_invalid_star (void)
{
  TestWarning tests[] = {
    { "*.1",    "Cannot use star for major*"   },
    { "1.**",   "Star does not end*"           },
    { "1.*.",   "Star does not end*"           },
    { "1.2*",   "Cannot use star in number*"   },
    { NULL }
  };

  test_warnings (tests);
}

int
main (int    argc,
      char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_type_init ();

  testing_init ();

#define TEST(path, ftest) \
  testing_util_add_func ("/plugin-version/" path, test_plugin_version_##ftest)

  TEST ("to-string", to_string);

  TEST ("operation/equal", operation_equal);
  TEST ("operation/not-equal", operation_not_equal);
  TEST ("operation/less", operation_less);
  TEST ("operation/greater", operation_greater);

  TEST ("invalid/operation", invalid_operation);
  TEST ("invalid/version", invalid_version);
  TEST ("invalid/dot", invalid_dot);
  TEST ("invalid/space", invalid_space);
  TEST ("invalid/star", invalid_star);

  return testing_run_tests ();
}
