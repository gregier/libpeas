/*
 * extension-seed.c
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

#include <seed.h>

#include "loaders/seed/peas-extension-seed.h"

#include "testing/testing-extension.h"

static void
test_extension_seed_nonexistent (PeasEngine *engine)
{
  PeasPluginInfo *info;

  testing_util_push_log_hook ("*Failed to open *extension-seed-nonexistent.js*");
  testing_util_push_log_hook ("Error loading plugin 'extension-seed-nonexistent'");

  info = peas_engine_get_plugin_info (engine, "extension-seed-nonexistent");

  g_assert (!peas_engine_load_plugin (engine, info));
}

int
main (int   argc,
      char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_type_init ();

  testing_extension_all ("seed");

  EXTENSION_TEST (seed, "nonexistent", nonexistent);

  return testing_extension_run_tests ();
}
