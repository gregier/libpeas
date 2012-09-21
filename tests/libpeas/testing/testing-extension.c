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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

#include "introspection-base.h"
#include "introspection-callable.h"
#include "introspection-has-missing-prerequisite.h"
#include "introspection-has-prerequisite.h"
#include "introspection-properties.h"
#include "introspection-unimplemented.h"

typedef struct _TestFixture TestFixture;

struct _TestFixture {
  PeasEngine *engine;
  PeasPluginInfo *info;
};

static gchar *extension_plugin = NULL;

static void
test_setup (TestFixture    *fixture,
            gconstpointer  data)
{
  fixture->engine = testing_engine_new ();
  fixture->info = peas_engine_get_plugin_info (fixture->engine,
                                               extension_plugin);
}

static void
test_teardown (TestFixture    *fixture,
               gconstpointer  data)
{
  testing_engine_free (fixture->engine);
}

static void
test_runner  (TestFixture   *fixture,
              gconstpointer  data)
{
  g_assert (fixture->info != NULL);
  g_assert (peas_engine_load_plugin (fixture->engine, fixture->info));

  ((void (*) (PeasEngine *, PeasPluginInfo *)) data) (fixture->engine,
                                                      fixture->info);
}

static void
test_extension_garbage_collect (PeasEngine     *engine,
                                PeasPluginInfo *info)
{
  peas_engine_garbage_collect (engine);

  /* Check that we can collect the garbage when no plugins are loaded */
  g_assert (peas_engine_unload_plugin (engine, info));
  peas_engine_garbage_collect (engine);
}

static void
test_extension_provides_valid (PeasEngine     *engine,
                               PeasPluginInfo *info)
{
  g_assert (peas_engine_provides_extension (engine, info,
                                            INTROSPECTION_TYPE_CALLABLE));
}

static void
test_extension_provides_invalid (PeasEngine     *engine,
                                 PeasPluginInfo *info)
{

  testing_util_push_log_hook ("*assertion `G_TYPE_IS_INTERFACE (*)' failed");

  /* Invalid GType */
  peas_engine_provides_extension (engine, info, G_TYPE_INVALID);


  /* GObject but not a GInterface */
  peas_engine_provides_extension (engine, info, PEAS_TYPE_ENGINE);


  /* Does not implement this GType */
  g_assert (!peas_engine_provides_extension (engine, info,
                                             INTROSPECTION_TYPE_UNIMPLEMENTED));

  /* Not loaded */
  g_assert (peas_engine_unload_plugin (engine, info));
  g_assert (!peas_engine_provides_extension (engine, info,
                                             INTROSPECTION_TYPE_CALLABLE));
}

static void
test_extension_create_valid (PeasEngine     *engine,
                             PeasPluginInfo *info)
{
  PeasExtension *extension;

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_CALLABLE,
                                            NULL);

  g_assert (PEAS_IS_EXTENSION (extension));
  g_assert (INTROSPECTION_IS_CALLABLE (extension));

  g_object_unref (extension);
}

static void
test_extension_create_invalid (PeasEngine     *engine,
                               PeasPluginInfo *info)
{
  PeasExtension *extension;

  testing_util_push_log_hook ("*assertion `G_TYPE_IS_INTERFACE (*)' failed");
  testing_util_push_log_hook ("*does not provide a 'IntrospectionUnimplemented' extension");
  testing_util_push_log_hook ("*type 'IntrospectionCallable' has no property named 'invalid-property'");
  testing_util_push_log_hook ("*assertion `peas_plugin_info_is_loaded (*)' failed");

  /* Invalid GType */
  extension = peas_engine_create_extension (engine, info, G_TYPE_INVALID, NULL);
  g_assert (!PEAS_IS_EXTENSION (extension));


  /* GObject but not a GInterface */
  extension = peas_engine_create_extension (engine, info, PEAS_TYPE_ENGINE, NULL);
  g_assert (!PEAS_IS_EXTENSION (extension));


  /* Does not implement this GType */
  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_UNIMPLEMENTED,
                                            NULL);
  g_assert (!PEAS_IS_EXTENSION (extension));

  /* Interface does not have an 'invalid-property' property */
  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_CALLABLE,
                                            "invalid-property", "does-not-exist",
                                            NULL);
  g_assert (!PEAS_IS_EXTENSION (extension));

  /* This cannot be tested in PyGI and Seed's log handler messes this up */
  if (g_strcmp0 (extension_plugin, "extension-c") != 0 &&
      g_strcmp0 (extension_plugin, "extension-python") != 0 &&
      g_strcmp0 (extension_plugin, "extension-seed") != 0)
    {
      testing_util_push_log_hook ("*cannot add *IntrospectionHasMissingPrerequisite* "
                                  "which does not conform to *IntrospectionCallable*");
      testing_util_push_log_hook ("*Type *HasMissingPrerequisite* is invalid");
      testing_util_push_log_hook ("*does not provide a *HasMissingPrerequisite* extension");

      /* Missing Prerequisite */
      extension = peas_engine_create_extension (engine, info,
                                                INTROSPECTION_TYPE_HAS_MISSING_PREREQUISITE,
                                                NULL);
      g_assert (!PEAS_IS_EXTENSION (extension));
    }

  /* Not loaded */
  g_assert (peas_engine_unload_plugin (engine, info));
  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_CALLABLE,
                                            NULL);
  g_assert (!PEAS_IS_EXTENSION (extension));
}

static void
test_extension_create_with_prerequisite (PeasEngine     *engine,
                                         PeasPluginInfo *info)
{
  PeasExtension *extension;

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_HAS_PREREQUISITE,
                                            NULL);

  g_assert (INTROSPECTION_IS_HAS_PREREQUISITE (extension));
  g_assert (INTROSPECTION_IS_CALLABLE (extension));

  g_object_unref (extension);
}

static void
test_extension_reload (PeasEngine     *engine,
                       PeasPluginInfo *info)
{
  gint i;

  for (i = 0; i < 3; ++i)
    {
      g_assert (peas_engine_load_plugin (engine, info));
      g_assert (peas_engine_unload_plugin (engine, info));
    }
}

static void
test_extension_plugin_info (PeasEngine     *engine,
                            PeasPluginInfo *info)
{
  PeasExtension *extension;
  IntrospectionBase *base;

  g_assert (peas_engine_load_plugin (engine, info));

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_BASE,
                                            NULL);

  base = INTROSPECTION_BASE (extension);

  g_assert (introspection_base_get_plugin_info (base) == info);

  g_object_unref (extension);
}

static void
test_extension_get_settings (PeasEngine     *engine,
                             PeasPluginInfo *info)
{
  PeasExtension *extension;
  IntrospectionBase *base;
  GSettings *settings;

  g_assert (peas_engine_load_plugin (engine, info));

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_BASE,
                                            NULL);

  base = INTROSPECTION_BASE (extension);

  settings = introspection_base_get_settings (base);
  g_assert (G_IS_SETTINGS (settings));

  g_object_unref (settings);
  g_object_unref (extension);
}

static void
test_extension_call_no_args (PeasEngine     *engine,
                             PeasPluginInfo *info)
{
  PeasExtension *extension;
  IntrospectionCallable *callable;

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_CALLABLE,
                                            NULL);

  callable = INTROSPECTION_CALLABLE (extension);

  g_assert (peas_extension_call (extension, "call_no_args"));
  introspection_callable_call_no_args (callable);

  g_object_unref (extension);
}

static void
test_extension_call_with_return (PeasEngine     *engine,
                                 PeasPluginInfo *info)
{
  PeasExtension *extension;
  IntrospectionCallable *callable;
  const gchar *return_val = NULL;

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

static void
test_extension_call_single_arg (PeasEngine     *engine,
                                PeasPluginInfo *info)
{
  PeasExtension *extension;
  IntrospectionCallable *callable;
  gboolean called = FALSE;

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

static void
test_extension_call_multi_args (PeasEngine     *engine,
                                PeasPluginInfo *info)
{
  PeasExtension *extension;
  IntrospectionCallable *callable;
  gint in, out, inout;
  gint inout_saved;

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_CALLABLE,
                                            NULL);

  callable = INTROSPECTION_CALLABLE (extension);

  in = g_random_int ();
  inout = g_random_int ();
  inout_saved = inout;

  g_assert (peas_extension_call (extension, "call_multi_args",
                                 in, &out, &inout));

  g_assert_cmpint (inout_saved, ==, out);
  g_assert_cmpint (in, ==, inout);

  in = g_random_int ();
  inout = g_random_int ();
  inout_saved = inout;

  introspection_callable_call_multi_args (callable, in, &out, &inout);

  g_assert_cmpint (inout_saved, ==, out);
  g_assert_cmpint (in, ==, inout);

  g_object_unref (extension);
}

static void
test_extension_properties_construct_only (PeasEngine     *engine,
                                          PeasPluginInfo *info)
{
  PeasExtension *extension;
  gchar *construct_only;

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_PROPERTIES,
                                            "construct-only", "my-construct-only",
                                            NULL);

  g_object_get (extension, "construct-only", &construct_only, NULL);
  g_assert_cmpstr (construct_only, ==, "my-construct-only");
  g_free (construct_only);

  g_object_unref (extension);
}

static void
test_extension_properties_read_only (PeasEngine     *engine,
                                     PeasPluginInfo *info)
{
  PeasExtension *extension;
  gchar *read_only;

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_PROPERTIES,
                                            NULL);

  g_object_get (extension, "read-only", &read_only, NULL);
  g_assert_cmpstr (read_only, ==, "read-only");
  g_free (read_only);

  g_object_unref (extension);
}

static void
test_extension_properties_write_only (PeasEngine     *engine,
                                      PeasPluginInfo *info)
{
  PeasExtension *extension;

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_PROPERTIES,
                                            NULL);

  g_object_set (extension, "write-only", "my-write-only", NULL);

  g_object_unref (extension);
}

static void
test_extension_properties_readwrite (PeasEngine     *engine,
                                     PeasPluginInfo *info)
{
  PeasExtension *extension;
  gchar *readwrite;

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

static void
test_extension_properties_prerequisite (PeasEngine     *engine,
                                        PeasPluginInfo *info)
{
  PeasExtension *extension;
  gchar *prerequisite;

  extension = peas_engine_create_extension (engine, info,
                                            INTROSPECTION_TYPE_PROPERTIES,
                                            "prerequisite", "prerequisite",
                                            NULL);

  g_object_get (extension, "prerequisite", &prerequisite, NULL);
  g_assert_cmpstr (prerequisite, ==, "prerequisite");
  g_free (prerequisite);

  g_object_unref (extension);
}

#define _EXTENSION_TEST(loader, path, ftest) \
  G_STMT_START { \
    gchar *full_path = g_strdup_printf ("/extension/%s/" path, loader); \
\
    g_test_add (full_path, TestFixture, \
                (gpointer) test_extension_##ftest, \
                test_setup, test_runner, test_teardown); \
\
    g_free (full_path); \
  } G_STMT_END

void
testing_extension_basic (const gchar *loader)
{
  PeasEngine *engine;

  testing_init ();

  extension_plugin = g_strdup_printf ("extension-%s", loader);

  engine = testing_engine_new ();
  peas_engine_enable_loader (engine, loader);

  /* Check that the loaders are created lazily */
  g_assert (g_type_from_name ("PeasPluginLoader") == G_TYPE_INVALID);

  testing_engine_free (engine);


  _EXTENSION_TEST (loader, "garbage-collect", garbage_collect);

  _EXTENSION_TEST (loader, "provides-valid", provides_valid);
  _EXTENSION_TEST (loader, "provides-invalid", provides_invalid);

  _EXTENSION_TEST (loader, "create-valid", create_valid);
  _EXTENSION_TEST (loader, "create-invalid", create_invalid);
  _EXTENSION_TEST (loader, "create-with-prerequisite", create_with_prerequisite);

  _EXTENSION_TEST (loader, "reload", reload);

  _EXTENSION_TEST (loader, "plugin-info", plugin_info);
  _EXTENSION_TEST (loader, "get-settings", get_settings);
}

void
testing_extension_callable (const gchar *loader)
{
  _EXTENSION_TEST (loader, "call-no-args", call_no_args);
  _EXTENSION_TEST (loader, "call-with-return", call_with_return);
  _EXTENSION_TEST (loader, "call-single-arg", call_single_arg);
  _EXTENSION_TEST (loader, "call-multi-args", call_multi_args);
}

void
testing_extension_properties (const gchar *loader)
{
  _EXTENSION_TEST (loader, "properties-construct-only", properties_construct_only);
  _EXTENSION_TEST (loader, "properties-read-only", properties_read_only);
  _EXTENSION_TEST (loader, "properties-write-only", properties_write_only);
  _EXTENSION_TEST (loader, "properties-readwrite", properties_readwrite);
  _EXTENSION_TEST (loader, "properties-prerequisite", properties_prerequisite);
}

void
testing_extension_add (const gchar *path,
                       gpointer     func)
{
  g_test_add (path, TestFixture, func, test_setup, test_runner, test_teardown);
}

int
testing_extension_run_tests (void)
{
  int retval;

  retval = testing_run_tests ();

  g_free (extension_plugin);

  return retval;
}
