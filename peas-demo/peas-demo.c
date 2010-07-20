/*
 * peas-demo.c
 * This file is part of libpeas
 *
 * Copyright (C) 2009-2010 Steve Frécinaux
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

#include <config.h>

#include <gtk/gtk.h>
#include <girepository.h>
#include <libpeas/peas.h>
#include <libpeas/peas-i18n.h>
#include <libpeasui/peas-ui.h>

#include "peas-demo-window.h"

gboolean run_from_build_dir;
static PeasEngine *engine;
static GtkWidget *main_window;
static int n_windows;

static GOptionEntry demo_args[] = {
  { "run-from-build-dir", 'b', 0, G_OPTION_ARG_NONE, &run_from_build_dir,
    N_("Run from build directory"), NULL },
};

static void
activate_plugin (GtkButton   *button,
                 const gchar *plugin_name)
{
  PeasPluginInfo *info;

  g_debug ("%s %s", G_STRFUNC, plugin_name);
  info = peas_engine_get_plugin_info (engine, plugin_name);
  g_return_if_fail (info != NULL);
  peas_engine_load_plugin (engine, info);
}

static void
create_plugin_manager (GtkButton *button,
                       gpointer   user_data)
{
  GtkWidget *window;
  GtkWidget *manager;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (window), 6);
  gtk_window_set_title (GTK_WINDOW (window), "Peas Plugin Manager");

  manager = peas_ui_plugin_manager_new (engine);
  gtk_container_add (GTK_CONTAINER (window), manager);

  gtk_widget_show_all (window);
}

static void
create_new_window (void)
{
  GtkWidget *window;

  window = demo_window_new (engine);
  gtk_widget_show_all (window);
}

static GtkWidget *
create_main_window (void)
{
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *button;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  gtk_container_set_border_width (GTK_CONTAINER (window), 6);
  gtk_window_set_title (GTK_WINDOW (window), "Peas Demo");

  box = gtk_hbox_new (TRUE, 6);
  gtk_container_add (GTK_CONTAINER (window), box);

  button = gtk_button_new_with_label ("New window");
  g_signal_connect (button, "clicked", G_CALLBACK (create_new_window), NULL);
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);

  button = gtk_button_new_with_label ("Hello World");
  g_signal_connect (button, "clicked", G_CALLBACK (activate_plugin), "helloworld");
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);

  button = gtk_button_new_with_label ("Python Hello");
  g_signal_connect (button, "clicked", G_CALLBACK (activate_plugin), "pythonhello");
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);

  button = gtk_button_new_with_label ("Seed Hello");
  g_signal_connect (button, "clicked", G_CALLBACK (activate_plugin), "seedhello");
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_PREFERENCES);
  g_signal_connect (button, "clicked", G_CALLBACK (create_plugin_manager), NULL);
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_QUIT);
  g_signal_connect (button, "clicked", G_CALLBACK (gtk_main_quit), NULL);
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);

  return window;
}

int
main (int    argc,
      char **argv)
{
  GOptionContext *option_context;
  GError *error = NULL;
  gchar **search_paths;

  option_context = g_option_context_new (_("- libpeas demo application"));
  g_option_context_add_main_entries (option_context, demo_args, GETTEXT_PACKAGE);
  g_option_context_add_group (option_context, gtk_get_option_group (TRUE));

  if (!g_option_context_parse (option_context, &argc, &argv, &error))
    {
      g_warning ("Error while parsing arguments: %s", error->message);
      g_error_free (error);
      return -1;
    }

  g_option_context_free (option_context);

  search_paths = g_new0 (gchar *, 5);

  /* User-installed plugins */
  search_paths[0] = g_build_filename (g_get_user_config_dir (), "peas-demo/plugins", NULL);
  search_paths[1] = g_build_filename (g_get_user_config_dir (), "peas-demo/plugins", NULL);

  if (run_from_build_dir)
    {
      g_debug ("Running from build dir.");
      g_irepository_prepend_search_path ("../libpeas");
      g_irepository_prepend_search_path ("../libpeasui");
      g_setenv ("PEAS_PLUGIN_LOADERS_DIR", "../loaders", TRUE);

      /* Uninstalled plugins */
      search_paths[2] = g_strdup ("./plugins");
      search_paths[3] = g_strdup ("./plugins");
    }
  else
    {
      /* System-wide plugins */
      search_paths[2] = g_strdup (PEAS_PREFIX "/lib/peas-demo/plugins/");
      search_paths[3] = g_strdup (PEAS_PREFIX "/share/peas-demo/plugins/");
    }

  g_irepository_require (g_irepository_get_default (), "PeasUI", "1.0", 0, NULL);

  engine = peas_engine_new ("PeasDemo",
                            PEAS_PREFIX "/lib/peas-demo/",
                            (const gchar **) search_paths);

  n_windows = 0;
  main_window = create_main_window ();
  gtk_widget_show_all (main_window);

  gtk_main ();

  g_object_unref (engine);
  g_strfreev (search_paths);

  return 0;
}
