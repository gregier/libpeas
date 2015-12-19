/*
 * testing-util.c
 * This file is part of libpeas
 *
 * Copyright (C) 2011 - Garrett Regier
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

#include "libpeas/peas-engine-priv.h"

#include "testing-util.h"

typedef struct {
  const gchar *pattern;
  gboolean hit;
} LogHook;

typedef struct {
  GPtrArray *hooks;
  GPtrArray *hits;
} LogHooks;

static void engine_private_notify (gpointer value);
static void unhandled_private_notify (gpointer value);
static void log_hooks_private_notify (gpointer value);

static gboolean initialized = FALSE;
static GLogLevelFlags fatal_flags = 0;

static gpointer dead_engine = NULL;
#define DEAD_ENGINE ((gpointer) &dead_engine)

static GPrivate engine_key = G_PRIVATE_INIT (engine_private_notify);
static GPrivate unhandled_key = G_PRIVATE_INIT (unhandled_private_notify);
static GPrivate log_hooks_key = G_PRIVATE_INIT (log_hooks_private_notify);

static void
engine_private_notify (gpointer value)
{
  if (value != NULL)
    g_error ("A PeasEngine was not freed!");
}

static void
unhandled_private_notify (gpointer value)
{
  if (value != NULL)
    g_error ("Log hooks not popped!");
}

static void
log_hooks_private_notify (gpointer value)
{
  LogHooks *log_hooks = value;

  if (log_hooks != NULL)
    {
      g_assert_cmpuint (log_hooks->hooks->len, ==, 0);
      g_ptr_array_unref (log_hooks->hooks);

      g_assert_cmpuint (log_hooks->hits->len, ==, 0);
      g_ptr_array_unref (log_hooks->hits);

      g_free (log_hooks);
    }
}

static LogHooks *
get_log_hooks (void)
{
  LogHooks *log_hooks = g_private_get (&log_hooks_key);

  if (log_hooks != NULL)
    return log_hooks;

  g_assert (initialized);

  log_hooks = g_new (LogHooks, 1);
  log_hooks->hooks = g_ptr_array_new_with_free_func (g_free);
  log_hooks->hits = g_ptr_array_new_with_free_func (g_free);
  g_private_set (&log_hooks_key, log_hooks);

  return log_hooks;
}

static void
log_handler (const gchar    *log_domain,
             GLogLevelFlags  log_level,
             const gchar    *message,
             gpointer        user_data)
{
  LogHooks *log_hooks = get_log_hooks ();
  GPtrArray *hooks = log_hooks->hooks;
  guint i;

  /* We always want to log debug, info and message logs */
  if ((log_level & G_LOG_LEVEL_DEBUG) != 0 ||
      (log_level & G_LOG_LEVEL_INFO) != 0 ||
      (log_level & G_LOG_LEVEL_MESSAGE) != 0)
    {
      g_log_default_handler (log_domain, log_level, message, user_data);
      return;
    }

  /* Don't bother trying to match errors as GLib always aborts on them */
  if ((log_level & G_LOG_LEVEL_ERROR) != 0)
    {
      g_log_default_handler (log_domain, log_level, message, user_data);

      /* Call abort() as GLib may call G_BREAKPOINT() instead */
      abort ();
    }

  for (i = 0; i < hooks->len; ++i)
    {
      LogHook *hook = g_ptr_array_index (hooks, i);
      gchar *msg;

      if (!g_pattern_match_simple (hook->pattern, message))
        continue;

      msg = g_strdup_printf ("%s-%s: %s", log_domain,
                             (log_level & G_LOG_LEVEL_WARNING) != 0 ?
                             "WARNING" : "CRITICAL",
                             message);
      g_ptr_array_add (log_hooks->hits, msg);

      hook->hit = TRUE;
      return;
    }

  /* Checked in testing_util_pop_log_hooks() */
  g_private_set (&unhandled_key, (gpointer) TRUE);

  /* Use the default log handler directly to avoid recurse complaints */
  g_log_default_handler (log_domain, log_level, message, user_data);

  /* Support for the standard G_DEBUG flags */
  if (((log_level & G_LOG_LEVEL_WARNING) != 0 &&
       (fatal_flags & G_LOG_LEVEL_WARNING) != 0) ||
      ((log_level & G_LOG_LEVEL_CRITICAL) != 0 &&
       (fatal_flags & G_LOG_LEVEL_CRITICAL) != 0))
    {
      G_BREAKPOINT ();
    }
}

void
testing_util_envars (void)
{
  /* Allow test runners to set this to 1 */
  g_setenv ("G_ENABLE_DIAGNOSTIC", "0", FALSE);

  /* Prevent GDBus from being used by GIO internally */
  g_setenv ("GIO_USE_VFS", "local", TRUE);

  /* We never want to save the settings */
  g_setenv ("GSETTINGS_BACKEND", "memory", TRUE);

  /* Prevent python from generating compiled files, they break distcheck */
  g_setenv ("PYTHONDONTWRITEBYTECODE", "yes", TRUE);

  g_setenv ("PEAS_PLUGIN_LOADERS_DIR", BUILDDIR "/loaders", TRUE);
}

void
testing_util_init (void)
{
  GError *error = NULL;
  const GDebugKey glib_debug_keys[] = {
    { "fatal-criticals", G_LOG_LEVEL_CRITICAL },
    { "fatal-warnings",  G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING }
  };

  if (initialized)
    return;

  /* Don't abort on warnings or criticals */
  g_log_set_always_fatal (G_LOG_LEVEL_ERROR);

  g_log_set_default_handler (log_handler, NULL);

  /* Force a breakpoint when the standard GLib debug flags
   * are used. This is not supplied automatically because
   * GLib's test utilities change the default.
   */
  fatal_flags = g_parse_debug_string (g_getenv ("G_DEBUG"), glib_debug_keys,
                                      G_N_ELEMENTS (glib_debug_keys));

  g_irepository_require_private (g_irepository_get_default (),
                                 BUILDDIR "/libpeas",
                                 "Peas", "1.0", 0, &error);
  g_assert_no_error (error);

  initialized = TRUE;
}

static void
engine_weak_notify (gpointer    unused,
                    PeasEngine *engine)
{
  /* Cannot use NULL because testing_util_engine_free() must be called */
  g_private_set (&engine_key, DEAD_ENGINE);
}

PeasEngine *
testing_util_engine_new_full (gboolean nonglobal_loaders)
{
  PeasEngine *engine;

  g_assert (initialized);

  /* testing_util_engine_free() checks that the
   * engine is freed so only one engine can be created
   */
  g_assert (g_private_get (&engine_key) == NULL);

  /* Must be after requiring typelibs */
  if (!nonglobal_loaders)
    engine = peas_engine_new ();
  else
    engine = peas_engine_new_with_nonglobal_loaders ();

  g_private_set (&engine_key, engine);

  g_object_weak_ref (G_OBJECT (engine),
                     (GWeakNotify) engine_weak_notify,
                     NULL);

  /* The plugins that two-deps depends on must be added
   * to the engine before it. This is used to verify that
   * the engine will order the plugin list correctly.
   */
  peas_engine_add_search_path (engine,
                               BUILDDIR "/tests/plugins/builtin",
                               SRCDIR   "/tests/plugins/builtin");
  peas_engine_add_search_path (engine,
                               BUILDDIR "/tests/plugins/loadable",
                               SRCDIR   "/tests/plugins/loadable");

  peas_engine_add_search_path (engine,
                               BUILDDIR "/tests/plugins",
                               SRCDIR   "/tests/plugins");

  return engine;
}

void
testing_util_engine_free (PeasEngine *engine)
{
  /* In case a test needs to free the engine */
  if (g_private_get (&engine_key) != DEAD_ENGINE)
    {
      g_object_unref (engine);

      /* Make sure that at the end of every test the engine is freed */
      g_assert (g_private_get (&engine_key) == DEAD_ENGINE);
    }

  g_private_set (&engine_key, NULL);

  /* Pop the log hooks so the test cases don't have to */
  testing_util_pop_log_hooks ();
}

int
testing_util_run_tests (void)
{
  int retval;

  g_assert (initialized);

  retval = g_test_run ();

  /* Cleanup various data early otherwise some
   * tools, like gcov, will not process it correctly
   */
  g_private_replace (&engine_key, NULL);
  g_private_replace (&unhandled_key, NULL);
  g_private_replace (&log_hooks_key, NULL);
  peas_engine_shutdown ();

  return retval;
}

void
testing_util_push_log_hook (const gchar *pattern)
{
  LogHooks *log_hooks = get_log_hooks ();
  LogHook *hook;

  g_return_if_fail (pattern != NULL && *pattern != '\0');

  hook = g_new (LogHook, 1);
  hook->pattern = pattern;
  hook->hit = FALSE;

  g_ptr_array_add (log_hooks->hooks, hook);
}

/* Optional - see testing_util_engine_free() */
void
testing_util_pop_log_hook (void)
{
  LogHooks *log_hooks = get_log_hooks ();
  GPtrArray *hooks = log_hooks->hooks;
  LogHook *hook;

  g_return_if_fail (hooks->len > 0);

  hook = g_ptr_array_index (hooks, hooks->len - 1);

  if (!hook->hit)
    testing_util_pop_log_hooks ();

  g_ptr_array_remove_index (hooks, hooks->len - 1);
}

void
testing_util_pop_log_hooks (void)
{
  LogHooks *log_hooks = get_log_hooks ();
  GPtrArray *hooks = log_hooks->hooks;
  GPtrArray *hits = log_hooks->hits;
  gboolean unhandled = g_private_get (&unhandled_key) != NULL;
  guint i;
  LogHook *hook;
  GPtrArray *unhit_hooks;
  GString *msg;

  if (hooks->len == 0)
    return;

  unhit_hooks = g_ptr_array_new ();

  for (i = 0; i < hooks->len; ++i)
    {
      hook = g_ptr_array_index (hooks, i);

      if (!hook->hit)
        g_ptr_array_add (unhit_hooks, hook);
    }

  if (unhit_hooks->len == 0 && !unhandled)
    {
      g_ptr_array_unref (unhit_hooks);
      g_ptr_array_set_size (hooks, 0);
      g_ptr_array_set_size (hits, 0);
      return;
    }

  msg = g_string_new ("");

  if (unhit_hooks->len != 0)
    {
      g_string_append (msg, "Log hooks were not triggered:");

      if (unhit_hooks->len == 1)
        {
          hook = g_ptr_array_index (unhit_hooks, 0);
          g_string_append_printf (msg, " '%s'", hook->pattern);
        }
      else
        {
          for (i = 0; i < unhit_hooks->len; ++i)
            {
              hook = g_ptr_array_index (unhit_hooks, i);

              g_string_append_printf (msg, "\n\t'%s'", hook->pattern);
            }
        }
    }

  if (hits->len != 0)
    {
      if (unhit_hooks->len != 0)
        g_string_append (msg, "\n\n");

      g_string_append (msg, "Log messages filtered:");

      for (i = 0; i < hits->len; ++i)
        {
          const gchar *hit = g_ptr_array_index (hits, i);

          g_string_append_printf (msg, "\n\t%s", hit);
        }
    }

  /* Use the default log handler directly to avoid recurse complaints */
  g_log_default_handler (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, msg->str, NULL);
  abort ();
}
