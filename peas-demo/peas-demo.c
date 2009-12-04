#include <gtk/gtk.h>
#include <libpeas/peas-engine.h>
#include <libpeasui/peas-plugin-manager.h>

PeasEngine *engine;
GtkWidget *main_window;
int n_windows;

static void
activate_plugin (GtkButton   *button,
                 const gchar *plugin_name)
{
  PeasPluginInfo *info;

  g_debug ("%s %s", G_STRFUNC, plugin_name);
  info = peas_engine_get_plugin_info (engine, plugin_name);
  g_return_if_fail (info != NULL);
  peas_engine_activate_plugin (engine, info);
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

  manager = peas_plugin_manager_new (engine);
  gtk_container_add (GTK_CONTAINER (window), manager);

  gtk_widget_show_all (window);
}

static gboolean
window_delete_event_cb (GtkWidget  *widget,
                        GdkEvent   *event,
                        PeasEngine *engine)
{
  peas_engine_remove_object (engine, G_OBJECT (widget));
  return FALSE;
}

static void
create_new_window (void)
{
  GtkWidget *window;
  GtkWidget *box;
  gchar *label;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  box = gtk_vbox_new (TRUE, 6);
  gtk_container_add (GTK_CONTAINER (window), box);

  label = g_strdup_printf ("Peas Window %d", ++n_windows);
  gtk_window_set_title (GTK_WINDOW (window), label);
  g_free (label);

  peas_engine_add_object (engine, G_OBJECT (window));

  g_signal_connect (window, "delete-event",
                    G_CALLBACK (window_delete_event_cb), engine);

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

  button = gtk_button_new_from_stock (GTK_STOCK_PREFERENCES);
  g_signal_connect (button, "clicked", G_CALLBACK (create_plugin_manager), NULL);
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_QUIT);
  g_signal_connect (button, "clicked", G_CALLBACK (gtk_main_quit), NULL);
  gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);

  return window;
}

int
main (int argc, char **argv)
{
  gchar const * const search_paths[] = {
    g_build_filename (g_get_user_config_dir (), "peas-demo/plugins", NULL),
    g_build_filename (g_get_user_config_dir (), "peas-demo/plugins", NULL),
    PEAS_PREFIX "/lib/peas-demo/plugins/",
    PEAS_PREFIX "/share/peas-demo/plugins/",
    NULL
  };

  gtk_init (&argc, &argv);

  engine = peas_engine_new ("PeasDemo",
                            PEAS_PREFIX "/lib/peas-demo/",
                            (const gchar **) search_paths);

  n_windows = 0;
  main_window = create_main_window ();
  gtk_widget_show_all (main_window);

  gtk_main ();

  g_object_unref (engine);

  return 0;
}
