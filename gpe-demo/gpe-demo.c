#include <gtk/gtk.h>
#include <libgpe/gpe-engine.h>

GPEEngine *engine;
GtkWidget *main_window;
int n_windows;

static gboolean
window_delete_event_cb (GtkWidget *widget,
			GdkEvent *event,
			GPEEngine *engine)
{
	gpe_engine_remove_object (engine, G_OBJECT (widget));
	return FALSE;
}

static void
create_new_window ()
{
	GtkWidget *window;
	GtkWidget *box;
	gchar *label;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	box = gtk_vbox_new (TRUE, 6);
	gtk_container_add (GTK_CONTAINER (window), box);

	label = g_strdup_printf ("GPE Window %d", ++n_windows);
	gtk_window_set_title (GTK_WINDOW (window), label);
	g_free (label);

	gpe_engine_add_object (engine, G_OBJECT (window));

	g_signal_connect (window, "delete-event", G_CALLBACK (window_delete_event_cb), engine);

	gtk_widget_show_all (window);
}

static GtkWidget *
create_main_window ()
{
	GtkWidget *window;
	GtkWidget *box;
	GtkWidget *button;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 6);
	gtk_window_set_title (GTK_WINDOW (window), "GPE Demo");

	box = gtk_hbox_new (TRUE, 6);
	gtk_container_add (GTK_CONTAINER (window), box);

	button = gtk_button_new_with_label ("New window");
	g_signal_connect (button, "clicked", G_CALLBACK (create_new_window), NULL);
	gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
	
	button = gtk_button_new_from_stock (GTK_STOCK_QUIT);
	g_signal_connect (button, "clicked", G_CALLBACK (gtk_main_quit), NULL);
	gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);

	return window;
}

int
main (int argc, char **argv)
{
	static const GPEPathInfo paths[] = {
		{ "./plugins/",
		  "./plugins/" },
		{ GPE_PREFIX "/lib/gpe-demo/plugins/",
		  GPE_PREFIX "/share/gpe-demo/plugins/" },
		{ NULL },
	};

	gtk_init (&argc, &argv);

	engine = gpe_engine_new (paths);

	n_windows = 0;
	main_window = create_main_window ();
	gtk_widget_show_all (main_window);

	gtk_main ();

	g_object_unref (engine);

	return 0;
}
