/*
 * plugin-manager.c
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

#include "testing/testing.h"

typedef struct _TestFixture TestFixture;

struct _TestFixture {
  PeasEngine *engine;
  PeasGtkPluginManager *manager;
  PeasGtkPluginManagerView *view;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkWidget *about_button;
  GtkWidget *configure_button;
};

static void
notify_model_cb (GtkTreeView *view,
                 GParamSpec  *pspec,
                 TestFixture *fixture)
{
  fixture->model = gtk_tree_view_get_model (GTK_TREE_VIEW (fixture->view));
}

static void
plugin_manager_forall_cb (GtkWidget  *widget,
                          GtkWidget **button_box)
{
  if (GTK_IS_BUTTON_BOX (widget))
    *button_box = widget;
}

static void
test_setup (TestFixture   *fixture,
            gconstpointer  data)
{
  GList *buttons;
  GList *button;
  GtkContainer *button_box = NULL;

  fixture->engine = testing_engine_new ();
  fixture->manager = PEAS_GTK_PLUGIN_MANAGER (peas_gtk_plugin_manager_new ());
  fixture->view = PEAS_GTK_PLUGIN_MANAGER_VIEW (peas_gtk_plugin_manager_get_view (fixture->manager));
  fixture->selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (fixture->view));

  g_signal_connect (fixture->view,
                    "notify::model",
                    G_CALLBACK (notify_model_cb),
                    fixture);

  /* Set the model */
  g_object_notify (G_OBJECT (fixture->view), "model");

  /* Must use forall at the buttons are "internal" children */
  gtk_container_forall (GTK_CONTAINER (fixture->manager),
                        (GtkCallback) plugin_manager_forall_cb,
                        &button_box);

  g_assert (button_box != NULL);

  buttons = gtk_container_get_children (button_box);

  for (button = buttons; button != NULL; button = button->next)
    {
      if (GTK_IS_BUTTON (button->data))
        {
          const gchar *label = gtk_button_get_label (GTK_BUTTON (button->data));

          if (g_strcmp0 (label, GTK_STOCK_ABOUT) == 0)
            fixture->about_button = GTK_WIDGET (button->data);
          else if (g_strcmp0 (label, GTK_STOCK_PREFERENCES) == 0)
            fixture->configure_button = GTK_WIDGET (button->data);
        }
    }

  g_assert (fixture->about_button != NULL);
  g_assert (fixture->configure_button != NULL);

  g_list_free (buttons);
}

static void
test_teardown (TestFixture   *fixture,
               gconstpointer  data)
{
  gtk_widget_destroy (GTK_WIDGET (fixture->manager));

  testing_engine_free (fixture->engine);
}

static void
test_runner (TestFixture   *fixture,
             gconstpointer  data)
{
  ((void (*) (TestFixture *)) data) (fixture);
}

static void
test_gtk_plugin_manager_about_button_sensitivity (TestFixture *fixture)
{
  GtkTreeIter iter;
  PeasPluginInfo *info;

  /* Must come first otherwise the first iter may
   * be after a revealed builtin plugin
   */
  peas_gtk_plugin_manager_view_set_show_builtin (fixture->view, TRUE);

  /* Causes the plugin to become unavailable */
  info = peas_engine_get_plugin_info (fixture->engine, "unavailable");
  peas_engine_load_plugin (fixture->engine, info);

  g_assert (gtk_tree_model_get_iter_first (fixture->model, &iter));

  do
    {
      gtk_tree_selection_select_iter (fixture->selection, &iter);

      g_assert (gtk_widget_get_sensitive (fixture->about_button));
    }
  while (gtk_tree_model_iter_next (fixture->model, &iter));
}

static void
test_gtk_plugin_manager_configure_button_sensitivity (TestFixture *fixture)
{
  GtkTreeIter iter;
  PeasPluginInfo *info;

  /* Must come first otherwise the first iter may
   * be after a revealed builtin plugin
   */
  peas_gtk_plugin_manager_view_set_show_builtin (fixture->view, TRUE);

  /* So we can configure them */
  info = peas_engine_get_plugin_info (fixture->engine, "builtin-configurable");
  peas_engine_load_plugin (fixture->engine, info);
  info = peas_engine_get_plugin_info (fixture->engine, "configurable");
  peas_engine_load_plugin (fixture->engine, info);

  /* Causes the plugin to become unavailable */
  info = peas_engine_get_plugin_info (fixture->engine, "unavailable");
  peas_engine_load_plugin (fixture->engine, info);

  g_assert (gtk_tree_model_get_iter_first (fixture->model, &iter));

  do
    {
      gboolean sensitive;

      gtk_tree_selection_select_iter (fixture->selection, &iter);

      info = testing_get_plugin_info_for_iter (fixture->view, &iter);

      if (!peas_plugin_info_is_loaded (info))
        {
          sensitive = FALSE;
        }
      else
        {
          sensitive = peas_engine_provides_extension (fixture->engine, info,
                                                      PEAS_GTK_TYPE_CONFIGURABLE);
        }

      g_assert_cmpint (gtk_widget_get_sensitive (fixture->configure_button),
                       ==, sensitive);
    }
  while (gtk_tree_model_iter_next (fixture->model, &iter));
}

static void
test_gtk_plugin_manager_plugin_loaded (TestFixture *fixture)
{
  GtkTreeIter iter;
  PeasPluginInfo *info;

  info = peas_engine_get_plugin_info (fixture->engine, "configurable");
  testing_get_iter_for_plugin_info (fixture->view, info, &iter);

  gtk_tree_selection_select_iter (fixture->selection, &iter);

  g_assert (!gtk_widget_is_sensitive (fixture->configure_button));
  peas_engine_load_plugin (fixture->engine, info);
  g_assert (gtk_widget_is_sensitive (fixture->configure_button));
}

static void
test_gtk_plugin_manager_plugin_unloaded (TestFixture *fixture)
{
  GtkTreeIter iter;
  PeasPluginInfo *info;

  test_gtk_plugin_manager_plugin_loaded (fixture);

  info = peas_engine_get_plugin_info (fixture->engine, "configurable");
  testing_get_iter_for_plugin_info (fixture->view, info, &iter);

  g_assert (gtk_widget_is_sensitive (fixture->configure_button));
  peas_engine_unload_plugin (fixture->engine, info);
  g_assert (!gtk_widget_is_sensitive (fixture->configure_button));
}

int
main (int    argc,
      char **argv)
{
  gtk_test_init (&argc, &argv, NULL);

  g_type_init ();

#define TEST(path, ftest) \
  g_test_add ("/gtk/plugin-manager/" path, TestFixture, \
              (gpointer) test_gtk_plugin_manager_##ftest, \
              test_setup, test_runner, test_teardown)

  TEST ("about-button-sensitivity", about_button_sensitivity);
  TEST ("configure-button-sensitivity", configure_button_sensitivity);

  TEST ("plugin-loaded", plugin_loaded);
  TEST ("plugin-unloaded", plugin_unloaded);

#undef TEST

  g_object_unref (peas_engine_get_default ());

  return g_test_run ();
}
