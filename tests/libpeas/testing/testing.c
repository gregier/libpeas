/*
 * testing.c
 * This file is part of libpeas
 *
 * Copyright (C) 2010 - Garrett Regier
 *
 * libpeas is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libpeas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include <glib.h>
#include <girepository.h>
#include <testing-util.h>

#include "testing.h"

void
testing_init (gint    *argc,
              gchar ***argv)
{
  GError *error = NULL;
  static gboolean initialized = FALSE;

  if (initialized)
    return;

  testing_util_envars ();

  g_test_init (argc, argv, NULL);

  /* Must be after g_test_init() changes the log settings*/
  testing_util_init ();

  g_irepository_require_private (g_irepository_get_default (),
                                 BUILDDIR "/tests/libpeas/introspection",
                                 "Introspection", "1.0", 0, &error);
  g_assert_no_error (error);

  initialized = TRUE;
}

PeasEngine *
testing_engine_new_full (gboolean nonglobal_loaders)
{
  PeasEngine *engine;

  testing_util_push_log_hook ("*Bad plugin file *invalid.plugin*");
  testing_util_push_log_hook ("*Error loading *invalid.plugin*");

  testing_util_push_log_hook ("*Could not find 'Module' in "
                              "*info-missing-module.plugin*");
  testing_util_push_log_hook ("*Error loading *info-missing-module.plugin*");

  testing_util_push_log_hook ("*Could not find 'Name' in "
                              "*info-missing-name.plugin*");
  testing_util_push_log_hook ("*Error loading *info-missing-name.plugin*");

  testing_util_push_log_hook ("*Unkown 'Loader' in "
                              "*unkown-loader.plugin* does-not-exist");
  testing_util_push_log_hook ("*Error loading *unkown-loader.plugin*");

  testing_util_push_log_hook ("Bad plugin file '"
                              BUILDDIR "*/embedded*.plugin': "
                              "embedded plugins must be a resource");
  testing_util_push_log_hook ("Error loading '"
                              BUILDDIR "*/embedded*.plugin'*");

  testing_util_push_log_hook ("Bad plugin file '"
                              BUILDDIR "*embedded-invalid-loader.plugin': "
                              "embedded plugins must use the C plugin loader");

  /* Must be after pushing log hooks */
  engine = testing_util_engine_new_full (nonglobal_loaders);

  peas_engine_add_search_path (engine,
                               "resource:///org/gnome/libpeas/"
                               "tests/plugins", NULL);
  peas_engine_add_search_path (engine,
                               BUILDDIR "/tests/libpeas/plugins",
                               SRCDIR   "/tests/libpeas/plugins");

  return engine;
}
