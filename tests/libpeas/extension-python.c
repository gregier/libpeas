/*
 * extension-python.c
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "testing/testing-extension.h"

/*#define EXTENSION_TESTS("python")*/

int
main (int   argc,
      char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_type_init ();

  testing_init ();

  peas_engine_enable_loader (peas_engine_get_default (), "python");
  g_object_unref (peas_engine_get_default ());

  testing_extension_set_plugin_ ("extension-" "python");

  _EXTENSION_TEST ("python", "garbage-collect", garbage_collect);

  _EXTENSION_TEST ("python", "provides-valid", provides_valid);
  _EXTENSION_TEST ("python", "provides-invalid", provides_invalid);

  _EXTENSION_TEST ("python", "create-valid", create_valid);
  _EXTENSION_TEST ("python", "create-invalid", create_invalid);

  _EXTENSION_TEST ("python", "reload", reload);

  _EXTENSION_TEST ("python", "call-invalid", call_invalid);
  _EXTENSION_TEST ("python", "call-no-args", call_no_args);
  _EXTENSION_TEST ("python", "call-with-return", call_with_return);
  _EXTENSION_TEST ("python", "call-single-arg", call_single_arg);
  _EXTENSION_TEST ("python", "call-multi-args", call_multi_args);

#ifdef PYTHON_EXTENSION_PROPERTIES_DONT_WORK
  /* Some tests don't fail when they should */

  _EXTENSION_TEST ("python", "properties-construct-only", properties_construct_only);
  _EXTENSION_TEST ("python", "properties-read-only", properties_read_only);
  _EXTENSION_TEST ("python", "properties-write-only", properties_write_only);
  _EXTENSION_TEST ("python", "properties-readwrite", properties_readwrite);
#endif

  return testing_run_tests ();
}
