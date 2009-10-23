#include <glib.h>
#include <gmodule.h>

#include "gpedemo-hello-world-plugin.h"

#define WINDOW_DATA_KEY "GPEDemoHelloWorldPluginWindowData"

GPE_REGISTER_TYPE(GPE_TYPE_PLUGIN, GPEDemoHelloWorldPlugin, gpedemo_hello_world_plugin)

typedef struct
{
	GtkWidget *label;
} WindowData;

static void
gpedemo_hello_world_plugin_init (GPEDemoHelloWorldPlugin *plugin)
{
	g_debug (G_STRFUNC);
}

static void
gpedemo_hello_world_plugin_finalize (GObject *object)
{
	g_debug (G_STRFUNC);

	G_OBJECT_CLASS (gpedemo_hello_world_plugin_parent_class)->finalize (object);
}

static void
free_window_data (WindowData *data)
{
	g_debug (G_STRFUNC);
	g_return_if_fail (data != NULL);

	g_object_unref (data->label);
	g_free (data);
}

static GtkBox *
get_box (GtkWidget *window)
{
	return GTK_BOX (gtk_bin_get_child (GTK_BIN (window)));
}

static void
gpedemo_hello_world_plugin_activate (GPEPlugin *plugin,
				     GObject   *object)
{
	GtkWidget *window;
	GtkWidget *label;
	WindowData *data;

	g_debug (G_STRFUNC);

	g_return_if_fail (GTK_IS_WINDOW (object));
	window = GTK_WIDGET (object);

	label = gtk_label_new ("Hello World!");
	gtk_box_pack_start (get_box (window), label, 1, 1, 0);
	gtk_widget_show (label);

	data = g_new0 (WindowData, 1);

	data->label = label;
	g_object_ref (label);

	g_object_set_data_full (G_OBJECT (window),
				WINDOW_DATA_KEY,
				data,
				(GDestroyNotify) free_window_data);
}

static void
gpedemo_hello_world_plugin_deactivate (GPEPlugin *plugin,
				       GObject   *object)
{
	GtkWidget *window;
	WindowData *data;

	g_debug (G_STRFUNC);

	g_return_if_fail (GTK_IS_WINDOW (object));
	window = GTK_WIDGET (object);

	data = (WindowData *) g_object_get_data (G_OBJECT (window),
						 WINDOW_DATA_KEY);
	g_return_if_fail (data != NULL);

	gtk_container_remove (GTK_CONTAINER (get_box (window)), data->label);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
gpedemo_hello_world_plugin_class_init (GPEDemoHelloWorldPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GPEPluginClass *plugin_class = GPE_PLUGIN_CLASS (klass);

	object_class->finalize = gpedemo_hello_world_plugin_finalize;

	plugin_class->activate = gpedemo_hello_world_plugin_activate;
	plugin_class->deactivate = gpedemo_hello_world_plugin_deactivate;
}
