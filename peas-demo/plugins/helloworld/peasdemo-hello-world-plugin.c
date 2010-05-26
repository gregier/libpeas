#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include <libpeas/peas-activatable.h>
#include <libpeasui/peas-ui-configurable.h>

#include "peasdemo-hello-world-plugin.h"

#define WINDOW_DATA_KEY "PeasDemoHelloWorldPluginWindowData"

static void peas_activatable_iface_init (PeasActivatableInterface *iface);
static void peas_ui_configurable_iface_init (PeasUIConfigurableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (PeasDemoHelloWorldPlugin,
                                peasdemo_hello_world_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE (PEAS_TYPE_ACTIVATABLE,
                                                       peas_activatable_iface_init)
                                G_IMPLEMENT_INTERFACE (PEAS_UI_TYPE_CONFIGURABLE,
                                                       peas_ui_configurable_iface_init))

typedef struct {
  GtkWidget *label;
} WindowData;

static void
peasdemo_hello_world_plugin_init (PeasDemoHelloWorldPlugin *plugin)
{
  g_debug (G_STRFUNC);
}

static void
peasdemo_hello_world_plugin_finalize (GObject *object)
{
  g_debug (G_STRFUNC);

  G_OBJECT_CLASS (peasdemo_hello_world_plugin_parent_class)->finalize (object);
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
peasdemo_hello_world_plugin_activate (PeasActivatable *activatable,
                                      GObject         *object)
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
peasdemo_hello_world_plugin_deactivate (PeasActivatable *activatable,
                                        GObject         *object)
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
on_configure_dialog_response (GtkDialog *dialog,
                              gint       response_id,
                              gpointer   user_data)
{
  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static gboolean
peasdemo_hello_world_plugin_create_configure_dialog (PeasUIConfigurable *configurable,
                                                     GtkWidget         **dialog)
{
  g_debug (G_STRFUNC);

  *dialog = gtk_message_dialog_new (NULL,
                                    0,
                                    GTK_MESSAGE_INFO,
                                    GTK_BUTTONS_OK,
                                    "This is a configuration dialog for the HelloWorld plugin.");
  g_signal_connect (*dialog, "response",
                    G_CALLBACK (on_configure_dialog_response), NULL);

  return TRUE;
}

static void
peasdemo_hello_world_plugin_class_init (PeasDemoHelloWorldPluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = peasdemo_hello_world_plugin_finalize;
}

static void
peas_activatable_iface_init (PeasActivatableInterface *iface)
{
  iface->activate = peasdemo_hello_world_plugin_activate;
  iface->deactivate = peasdemo_hello_world_plugin_deactivate;
}

static void
peas_ui_configurable_iface_init (PeasUIConfigurableInterface *iface)
{
  iface->create_configure_dialog = peasdemo_hello_world_plugin_create_configure_dialog;
}

static void
peasdemo_hello_world_plugin_class_finalize (PeasDemoHelloWorldPluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  peasdemo_hello_world_plugin_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_ACTIVATABLE,
                                              PEASDEMO_TYPE_HELLO_WORLD_PLUGIN);
  peas_object_module_register_extension_type (module,
                                              PEAS_UI_TYPE_CONFIGURABLE,
                                              PEASDEMO_TYPE_HELLO_WORLD_PLUGIN);
}
