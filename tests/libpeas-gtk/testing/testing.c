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

#include "libpeas/peas-plugin-info-priv.h"

#include "testing.h"

static GLogFunc default_log_func;

/* These are warning that just have to happen for testing
 * purposes and as such we don't want to abort on them.
 *
 * Would be nice if we could assert that they were...
 */
static const gchar *allowed_patterns[] = {
  "*Plugin not found: does-not-exist*",
  "*libcloader.so*cannot open shared object file: No such file or directory*",
  "*Could not find 'IAge' in *info-missing-iage.plugin*",
  "*Could not find 'Module' in *info-missing-module.plugin*",
  "*Could not find 'Name' in *info-missing-name.plugin*",
  "*Error loading *info-missing-iage.plugin*",
  "*Error loading *info-missing-module.plugin*",
  "*Error loading *info-missing-name.plugin*"
};

static void
log_handler (const gchar    *log_domain,
             GLogLevelFlags  log_level,
             const gchar    *message,
             gpointer        user_data)
{
  gint i;

  if ((log_level & G_LOG_LEVEL_DEBUG) != 0 ||
      (log_level & G_LOG_LEVEL_INFO) != 0 ||
      (log_level & G_LOG_LEVEL_MESSAGE) != 0)
    {
      default_log_func (log_domain, log_level, message, user_data);
      return;
    }

  if ((log_level & G_LOG_LEVEL_CRITICAL) != 0 ||
      (log_level & G_LOG_LEVEL_ERROR) != 0)
    {
      goto out;
    }

  for (i = 0; i < G_N_ELEMENTS (allowed_patterns); ++i)
    {
      if (g_pattern_match_simple (allowed_patterns[i], message))
        return;
    }

out:

  default_log_func (log_domain, log_level, message, user_data);

  /* Make sure we abort for warnings */
  abort ();
}

PeasEngine *
testing_engine_new (void)
{
  PeasEngine *engine;
  GError *error = NULL;
  static gboolean initialized = FALSE;

  if (initialized)
    return peas_engine_get_default ();

  /* Don't always abort on warnings */
  g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL);

  default_log_func = g_log_set_default_handler (log_handler, NULL);

  g_irepository_prepend_search_path (BUILDDIR "/libpeas");
  g_irepository_prepend_search_path (BUILDDIR "/libpeas-gtk");

  g_setenv ("PEAS_PLUGIN_LOADERS_DIR", BUILDDIR "/loaders", TRUE);

  g_irepository_require (g_irepository_get_default (), "Peas", "1.0", 0, &error);
  g_assert_no_error (error);
  g_irepository_require (g_irepository_get_default (), "PeasGtk", "1.0", 0, &error);
  g_assert_no_error (error);

  /* Must be after requiring typelibs */
  engine = peas_engine_get_default ();

  peas_engine_add_search_path (engine, BUILDDIR "/tests/plugins", NULL);
  peas_engine_add_search_path (engine, BUILDDIR "/tests/libpeas-gtk/plugins", NULL);
  peas_engine_rescan_plugins (engine);

  initialized = TRUE;

  return engine;
}

void
testing_engine_free (PeasEngine *engine)
{
  const GList *plugins;

  /* This causes errors during the next test:
  g_object_unref (fixture->engine);*/

  /* Otherwise the tests may cause each other to fail
   * because they expected a plugin to be available
   * but it failed loading making it unavailable
   */
  plugins = peas_engine_get_plugin_list (engine);
  for (; plugins != NULL; plugins = plugins->next)
    ((PeasPluginInfo *) plugins->data)->available = TRUE;

  peas_engine_set_loaded_plugins (engine, NULL);
  g_assert (peas_engine_get_loaded_plugins (engine) == NULL);
}

PeasPluginInfo *
testing_get_plugin_info_for_iter (PeasGtkPluginManagerView *view,
                                  GtkTreeIter              *iter)
{
  GtkTreeSelection *selection;
  GtkTreeIter selected_iter;
  gboolean had_selection;
  PeasPluginInfo *info;

  /* This is annoying but the only way to get the plugin info
   * is to ask the view for the info of the selected plugin
   */

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));

  had_selection = gtk_tree_selection_get_selected (selection,
                                                   NULL, &selected_iter);

  gtk_tree_selection_select_iter (selection, iter);

  info = peas_gtk_plugin_manager_view_get_selected_plugin (view);

  if (had_selection)
    gtk_tree_selection_select_iter (selection, &selected_iter);

  return info;
}

void
testing_get_iter_for_plugin_info (PeasGtkPluginManagerView *view,
                                  PeasPluginInfo           *info,
                                  GtkTreeIter              *iter)
{
  GtkTreeModel *model;
  gboolean found = FALSE;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));

  g_assert (gtk_tree_model_get_iter_first (model, iter));

  do
    {
      if (testing_get_plugin_info_for_iter (view, iter) == info)
        found = TRUE;
    }
  while (!found && gtk_tree_model_iter_next (model, iter));

  g_assert (found);
}

void
testing_show_widget (GtkWidget *widget)
{
  GtkWidget *window;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (window), 12);

  gtk_container_add (GTK_CONTAINER (window), widget);

  gtk_widget_show_all (window);

  gtk_main ();
}
