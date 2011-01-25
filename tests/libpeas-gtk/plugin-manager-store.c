/*
 * plugin-manager-store.c
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

#include <glib.h>
#include <gtk/gtk.h>
#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "libpeas-gtk/peas-gtk-plugin-manager-store.h"

#include "testing/testing.h"

typedef struct _TestFixture TestFixture;

struct _TestFixture {
  PeasEngine *engine;
  GtkTreeView *tree_view;
  PeasGtkPluginManagerView *view;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkListStore *store;
};

static void
notify_model_cb (GtkTreeView *view,
                 GParamSpec  *pspec,
                 TestFixture *fixture)
{
  fixture->model = gtk_tree_view_get_model (fixture->tree_view);

  if (GTK_IS_TREE_MODEL_FILTER (fixture->model))
    {
      GtkTreeModelFilter *filter = GTK_TREE_MODEL_FILTER (fixture->model);
      fixture->store = GTK_LIST_STORE (gtk_tree_model_filter_get_model (filter));
    }
  else
    {
      fixture->store = GTK_LIST_STORE (fixture->model);
    }
}

static void
test_setup (TestFixture   *fixture,
            gconstpointer  data)
{
  fixture->engine = testing_engine_new ();
  fixture->tree_view = GTK_TREE_VIEW (peas_gtk_plugin_manager_view_new ());
  fixture->view = PEAS_GTK_PLUGIN_MANAGER_VIEW (fixture->tree_view);
  fixture->selection = gtk_tree_view_get_selection (fixture->tree_view);

  g_signal_connect (fixture->view,
                    "notify::model",
                    G_CALLBACK (notify_model_cb),
                    fixture);

  /* Set the model, filter and store */
  g_object_notify (G_OBJECT (fixture->tree_view), "model");
}

static void
test_teardown (TestFixture   *fixture,
               gconstpointer  data)
{
  gtk_widget_destroy (GTK_WIDGET (fixture->tree_view));

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

  info2 = testing_get_plugin_info_for_iter (fixture->view, &iter);

  while (gtk_tree_model_iter_next (fixture->model, &iter))
    {
      info1 = info2;
      info2 = testing_get_plugin_info_for_iter (fixture->view, &iter);
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
  gboolean active;

  g_assert (gtk_tree_model_get_iter_first (fixture->model, &iter));
  info = testing_get_plugin_info_for_iter (fixture->view, &iter);

  gtk_tree_model_get (fixture->model, &iter,
                      PEAS_GTK_PLUGIN_MANAGER_STORE_ENABLED_COLUMN, &active,
                      -1);
  g_assert (!active);

  peas_engine_load_plugin (fixture->engine, info);

  gtk_tree_model_get (fixture->model, &iter,
                      PEAS_GTK_PLUGIN_MANAGER_STORE_ENABLED_COLUMN, &active,
                      -1);
  g_assert (active);
}

static void
test_gtk_plugin_manager_store_plugin_unloaded (TestFixture *fixture)
{
  GtkTreeIter iter;
  PeasPluginInfo *info;
  gboolean active;

  test_gtk_plugin_manager_store_plugin_loaded (fixture);

  g_assert (gtk_tree_model_get_iter_first (fixture->model, &iter));
  info = testing_get_plugin_info_for_iter (fixture->view, &iter);

  peas_engine_unload_plugin (fixture->engine, info);

  gtk_tree_model_get (fixture->model, &iter,
                      PEAS_GTK_PLUGIN_MANAGER_STORE_ENABLED_COLUMN, &active,
                      -1);
  g_assert (!active);
}

static void
verify_model (TestFixture    *fixture,
              PeasPluginInfo *info,
              gboolean        can_enable,
              const gchar    *icon_name,
              gboolean        icon_visible,
              gboolean        info_sensitive)
{
  GtkTreeIter iter;
  gboolean model_can_enable, model_icon_visible, model_info_sensitive;
  gchar *model_icon_name;

  testing_get_iter_for_plugin_info (fixture->view, info, &iter);

  gtk_tree_model_get (fixture->model, &iter,
    PEAS_GTK_PLUGIN_MANAGER_STORE_CAN_ENABLE_COLUMN,     &model_can_enable,
    PEAS_GTK_PLUGIN_MANAGER_STORE_ICON_NAME_COLUMN,      &model_icon_name,
    PEAS_GTK_PLUGIN_MANAGER_STORE_ICON_VISIBLE_COLUMN,   &model_icon_visible,
    PEAS_GTK_PLUGIN_MANAGER_STORE_INFO_SENSITIVE_COLUMN, &model_info_sensitive,
    -1);

  g_assert_cmpint (model_can_enable, ==, can_enable);
  g_assert_cmpstr (model_icon_name, ==, icon_name);
  g_assert_cmpint (model_icon_visible, ==, icon_visible);
  g_assert_cmpint (model_info_sensitive, ==, info_sensitive);

  g_free (model_icon_name);
}

static void
test_gtk_plugin_manager_store_verify_loadable (TestFixture *fixture)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (fixture->engine, "loadable");

  verify_model (fixture, info, TRUE, "libpeas-plugin", FALSE, TRUE);
}

static void
test_gtk_plugin_manager_store_verify_unavailable (TestFixture *fixture)
{
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (fixture->engine, "unavailable");

  verify_model (fixture, info, TRUE, "libpeas-plugin", FALSE, TRUE);

  peas_engine_load_plugin (fixture->engine, info);

  verify_model (fixture, info, FALSE, GTK_STOCK_DIALOG_ERROR, TRUE, FALSE);
}

static void
test_gtk_plugin_manager_store_verify_builtin (TestFixture *fixture)
{
  PeasPluginInfo *info;

  peas_gtk_plugin_manager_view_set_show_builtin (fixture->view, TRUE);

  info = peas_engine_get_plugin_info (fixture->engine, "builtin");

  verify_model (fixture, info, FALSE, "libpeas-plugin", FALSE, FALSE);

  peas_engine_load_plugin (fixture->engine, info);

  verify_model (fixture, info, FALSE, "libpeas-plugin", FALSE, TRUE);
}

static void
test_gtk_plugin_manager_store_verify_info (TestFixture *fixture)
{
  GtkTreeIter iter;
  PeasPluginInfo *info;
  gchar *model_info;

  /* Has description */
  info = peas_engine_get_plugin_info (fixture->engine, "configurable");
  testing_get_iter_for_plugin_info (fixture->view, info, &iter);

  gtk_tree_model_get (fixture->model, &iter,
    PEAS_GTK_PLUGIN_MANAGER_STORE_INFO_COLUMN,  &model_info,
    -1);
  g_assert_cmpstr (model_info, ==, "<b>Configurable</b>\nA plugin "
                                   "that can be loaded and configured.");
  g_free (model_info);

  /* Does not have description */
  info = peas_engine_get_plugin_info (fixture->engine, "min-info");
  testing_get_iter_for_plugin_info (fixture->view, info, &iter);

  gtk_tree_model_get (fixture->model, &iter,
    PEAS_GTK_PLUGIN_MANAGER_STORE_INFO_COLUMN,  &model_info,
    -1);
  g_assert_cmpstr (model_info, ==, "<b>Min Info</b>");
  g_free (model_info);
}

static void
verify_icon (TestFixture *fixture,
             const gchar *plugin_name,
             gboolean     has_pixbuf,
             const gchar *icon_name)
{
  PeasPluginInfo *info;
  GtkTreeIter iter;
  GdkPixbuf *model_icon_pixbuf;
  gchar *model_icon_name;

  info = peas_engine_get_plugin_info (fixture->engine, plugin_name);
  testing_get_iter_for_plugin_info (fixture->view, info, &iter);

  gtk_tree_model_get (fixture->model, &iter,
    PEAS_GTK_PLUGIN_MANAGER_STORE_ICON_PIXBUF_COLUMN, &model_icon_pixbuf,
    PEAS_GTK_PLUGIN_MANAGER_STORE_ICON_NAME_COLUMN, &model_icon_name,
    -1);

  if (has_pixbuf)
    g_assert (GDK_IS_PIXBUF (model_icon_pixbuf));
  else
    g_assert (!GDK_IS_PIXBUF (model_icon_pixbuf));

  g_assert_cmpstr (model_icon_name, ==, icon_name);

  if (model_icon_pixbuf != NULL)
    g_object_unref (model_icon_pixbuf);

  if (model_icon_name != NULL)
    g_free (model_icon_name);
}

static void
test_gtk_plugin_manager_store_valid_custom_icon (TestFixture *fixture)
{
  verify_icon (fixture, "valid-custom-icon", TRUE, NULL);
}

static void
test_gtk_plugin_manager_store_valid_stock_icon (TestFixture *fixture)
{
  verify_icon (fixture, "valid-stock-icon", FALSE, "gtk-about");
}

static void
test_gtk_plugin_manager_store_invalid_custom_icon (TestFixture *fixture)
{
  verify_icon (fixture, "invalid-custom-icon", FALSE, "libpeas-plugin");
}

static void
test_gtk_plugin_manager_store_invalid_stock_icon (TestFixture *fixture)
{
  verify_icon (fixture, "invalid-stock-icon", FALSE, "libpeas-plugin");
}

int
main (int    argc,
      char **argv)
{
  gtk_test_init (&argc, &argv, NULL);

  g_type_init ();

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

#undef TEST

  return g_test_run ();
}
