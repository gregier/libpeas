#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include <libpeas/peas.h>
#include <libpeasui/peas-ui.h>

#include "peasdemo-hello-world-plugin.h"
#include "peasdemo-hello-world-configurable.h"

static void peas_activatable_iface_init     (PeasActivatableInterface    *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (PeasDemoHelloWorldPlugin,
                                peasdemo_hello_world_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_TYPE_ACTIVATABLE,
                                                               peas_activatable_iface_init))

static void
peasdemo_hello_world_plugin_init (PeasDemoHelloWorldPlugin *plugin)
{
  g_debug (G_STRFUNC);
}

static void
peasdemo_hello_world_plugin_finalize (GObject *object)
{
  PeasDemoHelloWorldPlugin *plugin = PEASDEMO_HELLO_WORLD_PLUGIN (object);

  g_debug (G_STRFUNC);

  g_object_unref (plugin->label);

  G_OBJECT_CLASS (peasdemo_hello_world_plugin_parent_class)->finalize (object);
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
  PeasDemoHelloWorldPlugin *plugin = PEASDEMO_HELLO_WORLD_PLUGIN (activatable);
  GtkWidget *window;
  GtkWidget *label;

  g_debug (G_STRFUNC);

  g_return_if_fail (GTK_IS_WINDOW (object));
  window = GTK_WIDGET (object);

  plugin->label = gtk_label_new ("Hello World!");
  gtk_box_pack_start (get_box (window), plugin->label, 1, 1, 0);
  gtk_widget_show (plugin->label);
  g_object_ref (plugin->label);
}

static void
peasdemo_hello_world_plugin_deactivate (PeasActivatable *activatable,
                                        GObject         *object)
{
  PeasDemoHelloWorldPlugin *plugin = PEASDEMO_HELLO_WORLD_PLUGIN (activatable);
  GtkWidget *window;

  g_debug (G_STRFUNC);

  g_return_if_fail (GTK_IS_WINDOW (object));
  window = GTK_WIDGET (object);

  gtk_container_remove (GTK_CONTAINER (get_box (window)), plugin->label);
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
peasdemo_hello_world_plugin_class_finalize (PeasDemoHelloWorldPluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  peasdemo_hello_world_plugin_register_type (G_TYPE_MODULE (module));
  peasdemo_hello_world_configurable_register (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_ACTIVATABLE,
                                              PEASDEMO_TYPE_HELLO_WORLD_PLUGIN);
  peas_object_module_register_extension_type (module,
                                              PEAS_UI_TYPE_CONFIGURABLE,
                                              PEASDEMO_TYPE_HELLO_WORLD_CONFIGURABLE);
}
