/*
 * testing.c
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

#include <stdlib.h>

#include <glib.h>
#include <girepository.h>
#include <testing-util.h>

#include "testing.h"

void
testing_init (void)
{
  GError *error = NULL;
  static gboolean initialized = FALSE;

  if (initialized)
    return;

  testing_util_init ();

  g_irepository_require_private (g_irepository_get_default (),
                                 BUILDDIR "/tests/libpeas/introspection",
                                 "Introspection", "1.0", 0, &error);
  g_assert_no_error (error);

  initialized = TRUE;
}

PeasEngine *
testing_engine_new (void)
{
  PeasEngine *engine;

  testing_init ();

  /* Can happen multiple times */
  testing_util_push_log_hook ("*Invalid version: '0'*");

  /* Sorted according to occurence in peas-plugin-info.c */
  testing_util_push_log_hook ("*Bad plugin file: *invalid.plugin*");
  testing_util_push_log_hook ("*Error loading *invalid.plugin*");

  testing_util_push_log_hook ("*Could not find 'Module' in *info-missing-module.plugin*");
  testing_util_push_log_hook ("*Error loading *info-missing-module.plugin*");

  testing_util_push_log_hook ("*Module Name '<> =!' is invalid*");
  testing_util_push_log_hook ("*Error loading *invalid-module-name.plugin*");

  testing_util_push_log_hook ("*Could not find 'Name' in *info-missing-name.plugin*");
  testing_util_push_log_hook ("*Error loading *info-missing-name.plugin*");

  testing_util_push_log_hook ("*Dependency 'invalid-dep == 0' is invalid*");
  testing_util_push_log_hook ("*Error loading *invalid-dep.plugin*");

  testing_util_push_log_hook ("*Version '0' is invalid*");
  testing_util_push_log_hook ("*Error loading *invalid-version.plugin*");

  /* Must be after requiring typelibs */
  engine = testing_util_engine_new ();

  peas_engine_add_search_path (engine, BUILDDIR "/tests/libpeas/plugins",
                                       SRCDIR   "/tests/libpeas/plugins");

  return engine;
}
