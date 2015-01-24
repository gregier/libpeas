/*
 * plugin-manager-store.c
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

#include <glib.h>
#include <gtk/gtk.h>
#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "libpeas-gtk/peas-gtk-plugin-manager-store.h"

#include "testing/testing.h"

typedef struct _TestFixture TestFixture;

struct _TestFixture {
  PeasEngine *engine;
  GtkTreeModel *model;
  PeasGtkPluginManagerStore *store;
};

static void
test_setup (TestFixture   *fixture,
            gconstpointer  data)
{
  fixture->engine = testing_engine_new ();
  fixture->store = peas_gtk_plugin_manager_store_new (fixture->engine);
  fixture->model = GTK_TREE_MODEL (fixture->store);
}

static void
test_teardown (TestFixture   *fixture,
               gconstpointer  data)
{
  g_object_unref (fixture->store);

  testing_engine_free (fixture->engine);
}

static void
test_runner (TestFixture   *fixture,
             gconstpointer  data)
{
  ((void (*) (TestFixture *)) data) (fixture);
}

static void
test_gtk_plugin_manager_store_sorted (TestFixture *fixture)
{
  GtkTreeIter iter;
  PeasPluginInfo *info1, *info2;

  /* TODO: add a plugin that would cause this to assert if strcmp() was used */

  g_assert (gtk_tree_model_get_iter_first (fixture->model, &iter));

  info2 = peas_gtk_plugin_manager_store_get_plugin (fixture->store, &iter);

  while (gtk_tree_model_iter_next (fixture->model, &iter))
    {
      info1 = info2;
      info2 = peas_gtk_plugin_manager_store_get_plugin (fixture->store, &iter);

      g_assert_cmpint (g_utf8_collate (peas_plugin_info_get_name (info1),
                                       peas_plugin_info_get_name (info2)),
                       <, 0);
    }
}

static void
test_gtk_plugin_manager_store_plugin_loaded (TestFixture *fixture)
{
  GtkTreeIter iter;
  PeasPluginInfo *info;

  g_assert (gtk_tree_model_get_iter_first (fixture->model, &iter));
  info = peas_gtk_plugin_manager_store_get_plugin (fixture->store, &iter);

  g_assert (!peas_gtk_plugin_manager_store_get_enabled (fixture->store, &iter));

  peas_engine_load_plugin (fixture->engine, info);

  g_assert (peas_gtk_plugin_manager_store_get_enabled (fixture->store, &iter));
}

static void
test_gtk_plugin_manager_store_plugin_unloaded (TestFixture *fixture)
{
  GtkTreeIter iter;
  PeasPluginInfo *info;

  test_gtk_plugin_manager_store_plugin_loaded (fixture);

  g_assert (gtk_tree_model_get_iter_first (fixture->model, &iter));
  info = peas_gtk_plugin_manager_store_get_plugin (fixture->store, &iter);

  peas_engine_unload_plugin (fixture->engine, info);

  g_assert (!peas_gtk_plugin_manager_store_get_enabled (fixture->store, &iter));
}

static void
verify_model (TestFixture    *fixture,
              PeasPluginInfo *info,
              gboolean        can_enable,
              const gchar    *icon_name,
              GType           icon_type,
              gboolean        icon_visible,
              gboolean        info_sensitive)
{
  GtkTreeIter iter;
  gboolean model_can_enable, model_icon_visible, model_info_sensitive;
  GIcon *model_icon_gicon;
  gchar *model_icon_stock_id;

  g_assert (peas_gtk_plugin_manager_store_get_iter_from_plugin (fixture->store,
                                                                &iter, info));

  gtk_tree_model_get (fixture->model, &iter,
    PEAS_GTK_PLUGIN_MANAGER_STORE_CAN_ENABLE_COLUMN,     &model_can_enable,
    PEAS_GTK_PLUGIN_MANAGER_STORE_ICON_GICON_COLUMN,     &model_icon_gicon,
    PEAS_GTK_PLUGIN_MANAGER_STORE_ICON_STOCK_ID_COLUMN,  &model_icon_stock_id,
    PEAS_GTK_PLUGIN_MANAGER_STORE_ICON_VISIBLE_COLUMN,   &model_icon_visible,
    PEAS_GTK_PLUGIN_MANAGER_STORE_INFO_SENSITIVE_COLUMN, &model_info_sensitive,
    -1);

  g_assert_cmpint (model_can_enable, ==, can_enable);
  g_assert_cmpint (model_icon_visible, ==, icon_visible);
  g_assert_cmpint (model_info_sensitive, ==, info_sensitive);

  if (icon_type == G_TYPE_INVALID)
    {
      g_assert_cmpstr (model_icon_stock_id, ==, icon_name);
    }
  else
    {
      g_assert (g_type_is_a (G_OBJECT_TYPE (model_icon_gicon), icon_type));

      if (icon_type == G_TYPE_FILE_ICON)
        {
          GFile *file;
          gchar *basename;

          file = g_file_icon_get_file (G_FILE_ICON (model_icon_gicon));
          basename = g_file_get_basename (file);

          g_assert_cmpstr (basename, ==, icon_name);

          g_free (basename);
        }
      else if (icon_type == G_TYPE_THEMED_ICON)
        {
          GThemedIcon *themed_icon;
          const gchar * const *icon_names;

          themed_icon = G_THEMED_ICON (model_icon_gicon);
          icon_names = g_themed_icon_get_names (themed_icon);

          g_assert (icon_names);
          g_assert_cmpstr (icon_names[0], ==, icon_name);
        }
      else
        {
          g_assert_not_reached ();
        }

      g_object_unref (model_icon_gicon);
    }

  g_free (model_icon_stock_id);
}

static void
test_gtk_plugin_manager_store_verify_loadable (TestFixture *fixture)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (fixture->engine, "loadable");

  verify_model (fixture, info, TRUE, "libpeas-plugin", G_TYPE_THEMED_ICON,
                FALSE, TRUE);
}

static void
test_gtk_plugin_manager_store_verify_unavailable (TestFixture *fixture)
{
  PeasPluginInfo *info;

  testing_util_push_log_hook ("Could not find plugin 'does-not-exist'*");

  info = peas_engine_get_plugin_info (fixture->engine, "unavailable");

  verify_model (fixture, info, TRUE, "libpeas-plugin", G_TYPE_THEMED_ICON,
                FALSE, TRUE);

  peas_engine_load_plugin (fixture->engine, info);

  verify_model (fixture, info, FALSE, "dialog-error",
                G_TYPE_THEMED_ICON, TRUE, FALSE);
}

static void
test_gtk_plugin_manager_store_verify_builtin (TestFixture *fixture)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (fixture->engine, "builtin");

  verify_model (fixture, info, FALSE, "libpeas-plugin", G_TYPE_THEMED_ICON,
                FALSE, FALSE);

  peas_engine_load_plugin (fixture->engine, info);

  verify_model (fixture, info, FALSE, "libpeas-plugin", G_TYPE_THEMED_ICON,
                FALSE, TRUE);
}

static void
test_gtk_plugin_manager_store_verify_info (TestFixture *fixture)
{
  GtkTreeIter iter;
  PeasPluginInfo *info;
  gchar *model_info;

  /* Has description */
  info = peas_engine_get_plugin_info (fixture->engine, "configurable");
  g_assert (peas_gtk_plugin_manager_store_get_iter_from_plugin (fixture->store,
                                                                &iter, info));

  gtk_tree_model_get (fixture->model, &iter,
    PEAS_GTK_PLUGIN_MANAGER_STORE_INFO_COLUMN,  &model_info,
    -1);
  g_assert_cmpstr (model_info, ==, "<b>Configurable</b>\nA plugin "
                                   "that can be loaded and configured.");
  g_free (model_info);

  /* Does not have description */
  info = peas_engine_get_plugin_info (fixture->engine, "min-info");
  g_assert (peas_gtk_plugin_manager_store_get_iter_from_plugin (fixture->store,
                                                                &iter, info));

  gtk_tree_model_get (fixture->model, &iter,
    PEAS_GTK_PLUGIN_MANAGER_STORE_INFO_COLUMN,  &model_info,
    -1);
  g_assert_cmpstr (model_info, ==, "<b>Min Info</b>");
  g_free (model_info);
}

static void
verify_icon (TestFixture *fixture,
             const gchar *plugin_name,
             const gchar *icon_name,
             GType        icon_type)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (fixture->engine, plugin_name);

  verify_model (fixture, info, TRUE, icon_name, icon_type, FALSE, TRUE);
}

static void
test_gtk_plugin_manager_store_valid_custom_icon (TestFixture *fixture)
{
  verify_icon (fixture, "valid-custom-icon", "exists.png", G_TYPE_FILE_ICON);
}

static void
test_gtk_plugin_manager_store_valid_stock_icon (TestFixture *fixture)
{
  GtkIconTheme *icon_theme;
  GType icon_type = G_TYPE_INVALID;

  icon_theme = gtk_icon_theme_get_default ();

  /* Usually the theme does not have this icon */
  if (gtk_icon_theme_has_icon (icon_theme, "gtk-unindent"))
    icon_type = G_TYPE_THEMED_ICON;

  verify_icon (fixture, "valid-stock-icon", "gtk-unindent", icon_type);
}

static void
test_gtk_plugin_manager_store_invalid_custom_icon (TestFixture *fixture)
{
  verify_icon (fixture, "invalid-custom-icon", "libpeas-plugin",
               G_TYPE_THEMED_ICON);
}

static void
test_gtk_plugin_manager_store_invalid_stock_icon (TestFixture *fixture)
{
  verify_icon (fixture, "invalid-stock-icon", "libpeas-plugin",
               G_TYPE_THEMED_ICON);
}

static void
test_gtk_plugin_manager_store_hidden (TestFixture *fixture)
{
  PeasPluginInfo *info;
  GtkTreeIter iter;

  info = peas_engine_get_plugin_info (fixture->engine, "hidden");

  g_assert (peas_plugin_info_is_hidden (info));

  g_assert (!peas_gtk_plugin_manager_store_get_iter_from_plugin (fixture->store,
                                                                 &iter, info));
}

int
main (int    argc,
      char **argv)
{
  testing_init (&argc, &argv);

#define TEST(path, ftest) \
  g_test_add ("/gtk/plugin-manager-store/" path, TestFixture, \
              (gpointer) test_gtk_plugin_manager_store_##ftest, \
              test_setup, test_runner, test_teardown)

  TEST ("sorted", sorted);

  TEST ("plugin-loaded", plugin_loaded);
  TEST ("plugin-unloaded", plugin_unloaded);

  TEST ("verify-loadable", verify_loadable);
  TEST ("verify-unavailable", verify_unavailable);
  TEST ("verify-builtin", verify_builtin);
  TEST ("verify-info", verify_info);

  TEST ("valid-custom-icon", valid_custom_icon);
  TEST ("valid-stock-icon", valid_stock_icon);
  TEST ("invalid-custom-icon", invalid_custom_icon);
  TEST ("invalid-stock-icon", invalid_stock_icon);

  TEST ("hidden", hidden);

#undef TEST

  return testing_run_tests ();
}
