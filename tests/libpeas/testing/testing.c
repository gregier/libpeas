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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
}

PeasEngine *
testing_engine_new (void)
{
  PeasEngine *engine;

  testing_init ();

  /* Must be after requiring typelibs */
  engine = testing_util_engine_new ();

  peas_engine_add_search_path (engine, BUILDDIR "/tests/libpeas/plugins",
                                       SRCDIR   "/tests/libpeas/plugins");

  return engine;
}
