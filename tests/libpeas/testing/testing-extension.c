/*
 * testing-extensin.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <libpeas/peas.h>

#include "testing.h"
#include "testing-extension.h"

#include "introspection-callable.h"
#include "introspection-properties.h"
#include "introspection-unimplemented.h"

static gchar *extension_plugin;

void
testing_extension_set_plugin_ (const gchar *plugin)
{
  extension_plugin = (gchar *) plugin;
}

void
testing_extension_test_setup_ (TestingExtensionFixture_ *fixture,
                               gconstpointer             data)
{
  fixture->engine = testing_engine_new ();
}

void
testing_extension_test_teardown_ (TestingExtensionFixture_ *fixture,
                                  gconstpointer             data)
{
  testing_engine_free (fixture->engine);
}

void
testing_extension_test_runner_  (TestingExtensionFixture_ *fixture,
                               gconstpointer             data)
{
  ((void (*) (PeasEngine *engine)) data) (fixture->engine);
}

void
testing_extension_create_valid_ (PeasEngine *engine)
{
  PeasPluginInfo *info;
  PeasExtension *extension;

  info = peas_engine_get_plugin_info (engine, extension_plugin);

  g_assert (peas_engine_load_plugin (engine, info));

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_CALLABLE,
                                            NULL);

  g_assert (PEAS_IS_EXTENSION (extension));
  g_assert (INTROSPECTION_IS_CALLABLE (extension));

  g_object_unref (extension);
}

void
testing_extension_create_invalid_ (PeasEngine *engine)
{
  PeasPluginInfo *info;
  PeasExtension *extension;

  info = peas_engine_get_plugin_info (engine, extension_plugin);

  /* Not loaded */
  if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
      peas_engine_create_extension (engine, info,
                                    INTROSPECTION_TYPE_CALLABLE,
                                    NULL);
      exit (0);
    }
  g_test_trap_assert_failed ();

  g_assert (peas_engine_load_plugin (engine, info));

  /* Invalid GType */
  if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
      peas_engine_create_extension (engine, info, G_TYPE_INVALID, NULL);
      exit (0);
    }
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*CRITICAL*");


  /* GObject but not a GInterface */
  extension = peas_engine_create_extension (engine, info,
                                            PEAS_TYPE_ENGINE,
                                            NULL);
  g_assert (!PEAS_IS_EXTENSION (extension));


  /* Does not implement this GType */
  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_UNIMPLEMENTED,
                                            NULL);
  g_assert (!PEAS_IS_EXTENSION (extension));
}

void
testing_extension_reload_ (PeasEngine *engine)
{
  gint i;
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (engine, extension_plugin);

  for (i = 0; i < 3; ++i)
    {
      g_assert (peas_engine_load_plugin (engine, info));
      g_assert (peas_engine_unload_plugin (engine, info));
    }
}

void
testing_extension_call_invalid_ (PeasEngine *engine)
{
  PeasPluginInfo *info;
  PeasExtension *extension;

  info = peas_engine_get_plugin_info (engine, extension_plugin);

  g_assert (peas_engine_load_plugin (engine, info));

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_CALLABLE,
                                            NULL);

  if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
      peas_extension_call (extension, "invalid", NULL);
      exit (0);
    }
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*Method 'IntrospectionCallable.invalid' not found*");

  g_object_unref (extension);
}

void
testing_extension_call_no_args_ (PeasEngine *engine)
{
  PeasPluginInfo *info;
  PeasExtension *extension;
  IntrospectionCallable *callable;

  info = peas_engine_get_plugin_info (engine, extension_plugin);

  g_assert (peas_engine_load_plugin (engine, info));

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_CALLABLE,
                                            NULL);

  callable = INTROSPECTION_CALLABLE (extension);

  g_assert (peas_extension_call (extension, "call_no_args"));
  introspection_callable_call_no_args (callable);

  g_object_unref (extension);
}

void
testing_extension_call_with_return_ (PeasEngine *engine)
{
  PeasPluginInfo *info;
  PeasExtension *extension;
  IntrospectionCallable *callable;
  const gchar *return_val = NULL;

  info = peas_engine_get_plugin_info (engine, extension_plugin);

  g_assert (peas_engine_load_plugin (engine, info));

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_CALLABLE,
                                            NULL);

  callable = INTROSPECTION_CALLABLE (extension);

  g_assert (peas_extension_call (extension, "call_with_return", &return_val));
  g_assert_cmpstr (return_val, ==, "Hello, World!");

  return_val = NULL;

  return_val = introspection_callable_call_with_return (callable);
  g_assert_cmpstr (return_val, ==, "Hello, World!");

  g_object_unref (extension);
}

void
testing_extension_call_single_arg_ (PeasEngine *engine)
{
  PeasPluginInfo *info;
  PeasExtension *extension;
  IntrospectionCallable *callable;
  gboolean called = FALSE;

  info = peas_engine_get_plugin_info (engine, extension_plugin);

  g_assert (peas_engine_load_plugin (engine, info));

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_CALLABLE,
                                            NULL);

  callable = INTROSPECTION_CALLABLE (extension);

  g_assert (peas_extension_call (extension, "call_single_arg", &called));
  g_assert (called);

  called = FALSE;

  introspection_callable_call_single_arg (callable, &called);
  g_assert (called);

  g_object_unref (extension);
}

void
testing_extension_call_multi_args_ (PeasEngine *engine)
{
  PeasPluginInfo *info;
  PeasExtension *extension;
  IntrospectionCallable *callable;
  gboolean params[3] = { FALSE, FALSE, FALSE };

  info = peas_engine_get_plugin_info (engine, extension_plugin);

  g_assert (peas_engine_load_plugin (engine, info));

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_CALLABLE,
                                            NULL);

  callable = INTROSPECTION_CALLABLE (extension);

  g_assert (peas_extension_call (extension, "call_multi_args",
                                 &params[0], &params[1], &params[2]));
  g_assert (params[0] && params[1] && params[2]);

  memset (params, FALSE, G_N_ELEMENTS (params));

  introspection_callable_call_multi_args (callable, &params[0],
                                          &params[1], &params[2]);
  g_assert (params[0] && params[1] && params[2]);

  g_object_unref (extension);
}

void
testing_extension_properties_construct_only_ (PeasEngine *engine)
{
  PeasPluginInfo *info;
  PeasExtension *extension;
  gchar *construct_only;

  info = peas_engine_get_plugin_info (engine, extension_plugin);

  FILE *saved_stdout = stdout;
  FILE *saved_stderr = stderr;

  g_assert (peas_engine_load_plugin (engine, info));

  stdout = saved_stdout;
  stderr = saved_stderr;

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_PROPERTIES,
                                            "construct-only", "my-construct-only",
                                            NULL);


  g_object_get (extension, "construct-only", &construct_only, NULL);
  g_assert_cmpstr (construct_only, ==, "my-construct-only");
  g_free (construct_only);

  if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
      g_object_set (extension, "construct-only", "other-construct-only", NULL);
      exit (0);
    }
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*WARNING*");

  g_object_unref (extension);
}

void
testing_extension_properties_read_only_ (PeasEngine *engine)
{
  PeasPluginInfo *info;
  PeasExtension *extension;
  gchar *read_only;

  info = peas_engine_get_plugin_info (engine, extension_plugin);

  g_assert (peas_engine_load_plugin (engine, info));

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_PROPERTIES,
                                            NULL);

  g_object_get (extension, "read-only", &read_only, NULL);
  g_assert_cmpstr (read_only, ==, "read-only");
  g_free (read_only);

  if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
      g_object_set (extension, "read-only", "my-read-only", NULL);
      exit (0);
    }
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*WARNING*");

  g_object_unref (extension);
}

void
testing_extension_properties_write_only_ (PeasEngine *engine)
{
  PeasPluginInfo *info;
  PeasExtension *extension;

  info = peas_engine_get_plugin_info (engine, extension_plugin);

  g_assert (peas_engine_load_plugin (engine, info));

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_PROPERTIES,
                                            NULL);

  if (g_test_trap_fork (0, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR))
    {
      gchar *write_only = NULL;

      g_object_get (extension, "write-only", &write_only, NULL);
      exit (0);
    }
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*WARNING*");

  g_object_set (extension, "write-only", "my-write-only", NULL);

  g_object_unref (extension);
}

void
testing_extension_properties_readwrite_ (PeasEngine *engine)
{
  PeasPluginInfo *info;
  PeasExtension *extension;
  gchar *readwrite;

  info = peas_engine_get_plugin_info (engine, extension_plugin);

  g_assert (peas_engine_load_plugin (engine, info));

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_PROPERTIES,
                                            NULL);

  g_object_get (extension, "readwrite", &readwrite, NULL);
  g_assert_cmpstr (readwrite, ==, "readwrite");
  g_free (readwrite);

  g_object_set (extension, "readwrite", "my-readwrite", NULL);

  g_object_get (extension, "readwrite", &readwrite, NULL);
  g_assert_cmpstr (readwrite, ==, "my-readwrite");
  g_free (readwrite);

  g_object_unref (extension);
}
