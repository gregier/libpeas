/*
 * plugin-dependency.c
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

#include "libpeas/peas-plugin-dependency.h"

#include "testing/testing.h"

typedef struct {
  const gchar *dep;
  const gchar *warning;
} TestWarning;

static void
test_warnings (TestWarning *tests)
{
  gint i;

  for (i = 0; tests[i].dep != NULL; ++i)
    {
      testing_util_push_log_hook (tests[i].warning);
      g_assert (peas_plugin_dependency_new (tests[i].dep) == NULL);
      testing_util_pop_log_hook ();
    }
}

static gboolean
check_dependency (const gchar *dep_str,
                  const gchar *version_str)
{
  PeasPluginDependency *dep;
  PeasPluginVersion *version;
  gboolean retval;

  dep = peas_plugin_dependency_new (dep_str);
  version = peas_plugin_version_new (version_str);

  retval = peas_plugin_dependency_check (dep, version_str);

  g_assert_cmpint (retval, ==,
                   peas_plugin_dependency_check_version (dep, version));

  peas_plugin_version_unref (version);
  peas_plugin_dependency_unref (dep);

  return retval;
}

static void
test_plugin_dependency_to_string (void)
{
  gint i;
  const gchar *deps[] = {
    "name",          "a-zA-Z0-9_-",

    "name 1.2-2.3",  "name 1.*-2.*",

    "name == 1",     "name != 1",
    "name < 1",      "name > 1",
    "name <= 1",     "name >= 1"
  };

  for (i = 0; i < G_N_ELEMENTS (deps); ++i)
    {
      PeasPluginDependency *dep;
      gchar *dep_str;

      dep = peas_plugin_dependency_new (deps[i]);
      dep_str = peas_plugin_dependency_to_string (dep);

      g_assert_cmpstr (deps[i], ==, dep_str);

      g_free (dep_str);
      peas_plugin_dependency_unref (dep);
    }
}

static void
test_plugin_dependency_any (void)
{
  PeasPluginDependency *dep;

  /* peas_plugin_dependency_check(_version) should not use
   * the version if the dependency is an any version dependency
   */

  dep = peas_plugin_dependency_new ("name");

  g_assert (check_dependency ("name", "1"));

  g_assert (peas_plugin_dependency_check (dep, NULL));
  g_assert (peas_plugin_dependency_check_version (dep, NULL));

  g_assert (peas_plugin_dependency_check (dep, ""));
  g_assert (peas_plugin_dependency_check (dep, "0"));

  peas_plugin_dependency_unref (dep);
}

static void
test_plugin_dependency_check_oddities (void)
{
  PeasPluginDependency *dep;

  /* peas_plugin_dependency_check(_version) should allow a NULL version */

  testing_util_push_log_hook ("*assert*!= '\\0'*");

  dep = peas_plugin_dependency_new ("name == 1.2");

  g_assert (!peas_plugin_dependency_check (dep, NULL));
  g_assert (!peas_plugin_dependency_check (dep, ""));

  peas_plugin_dependency_unref (dep);
}

static void
test_plugin_dependency_operator (void)
{
  gint i;
  struct {
    const gchar *dep;
    const gchar *version;
  } tests[] = {
    { "name == 1.2",   "1.2" },

    { "name != 1.2",   "1.1" },
    { "name != 1.2",   "1.3" },

    { "name < 1.2",    "1.1" },
    { "name > 1.2",    "1.3" },

    { "name <= 1.2",   "1.2" },
    { "name <= 1.2",   "1.2" },

    { "name >= 1.2",   "1.3" },
    { "name >= 1.2",   "1.2" }
  };

  for (i = 0; i < G_N_ELEMENTS (tests); ++i)
    g_assert (check_dependency (tests[i].dep, tests[i].version));
}

static void
test_plugin_dependency_range (void)
{
  PeasPluginDependency *dep;

  dep = peas_plugin_dependency_new ("name 1.1-1.3");

  g_assert (!peas_plugin_dependency_check (dep, "1.0"));
  g_assert ( peas_plugin_dependency_check (dep, "1.1"));
  g_assert ( peas_plugin_dependency_check (dep, "1.2"));
  g_assert ( peas_plugin_dependency_check (dep, "1.3"));
  g_assert (!peas_plugin_dependency_check (dep, "1.4"));

  peas_plugin_dependency_unref (dep);
}

static void
test_plugin_dependency_invalid_operator (void)
{
  TestWarning tests[] = {
    { "name #  1.2",   "Invalid operator '#'*"           },
    { "name =# 1.2",   "Missing '=' after '='*"          },
    { "name !# 1.2",   "Missing '=' after '!'*"          },
    { "name <# 1.2",   "Missing space after operator*"   },
    { "name ># 1.2",   "Missing space after operator*"   },
    { NULL }
  };

  test_warnings (tests);
}

static void
test_plugin_dependency_invalid_version (void)
{
  PeasPluginDependency *dep;
  TestWarning tests[] = {
    { "name == 0.0",    "Invalid version*"    },
    { "name 0.0-1.2",   "Invalid version*",   },
    { "name 1.2-0.0",   "Invalid version*",   },
    { NULL }
  };

  test_warnings (tests);


  dep = peas_plugin_dependency_new ("name == 1");

  testing_util_push_log_hook ("Invalid version*");
  g_assert (!peas_plugin_dependency_check (dep, "0.0.0"));
  testing_util_pop_log_hook ();

  peas_plugin_dependency_unref (dep);
}

static void
test_plugin_dependency_invalid_range (void)
{
  TestWarning tests[] = {
    { "name 1.2-1.1",   "Invalid version range*"              },
    { "name 1.1-1.1",   "Invalid version range*"              },
    { NULL }
  };

  test_warnings (tests);
}

static void
test_plugin_dependency_invalid_space (void)
{
  TestWarning tests[] = {
    { "  name",         "Invalid character ' '*"          },
    { "name  ",         "Invalid operator ' '*"           },
    { "name== 1.2",     "Invalid character '='*"          },
    { "name ==1.2",     "Missing space after operator*"   },
    { "name == 1.1 ",   "Invalid character ' '*"          },
    { "name 1  -2",     "Invalid character ' '*"          },
    { "name 1-  2",     "Invalid character ' '*"          },
    { "name 1-2 ",      "Invalid character ' '*"          },
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
  testing_util_add_func ("/plugin-dependency/" path, \
                         test_plugin_dependency_##ftest)

  TEST ("to-string", to_string);

  TEST ("check-oddities", check_oddities);

  TEST ("any", any);
  TEST ("operator", operator);
  TEST ("range", range);

  TEST ("invalid/operator", invalid_operator);
  TEST ("invalid/version", invalid_version);
  TEST ("invalid/range", invalid_range);
  TEST ("invalid/space", invalid_space);

  return testing_run_tests ();
}
