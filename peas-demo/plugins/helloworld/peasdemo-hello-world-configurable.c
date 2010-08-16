#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "peasdemo-hello-world-configurable.h"

static void peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (PeasDemoHelloWorldConfigurable,
                                peasdemo_hello_world_configurable,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_GTK_TYPE_CONFIGURABLE,
                                                               peas_gtk_configurable_iface_init))

static void
peasdemo_hello_world_configurable_init (PeasDemoHelloWorldConfigurable *plugin)
{
  g_debug (G_STRFUNC);
}

static GtkWidget *
peasdemo_hello_world_configurable_create_configure_widget (PeasGtkConfigurable *configurable)
{
  g_debug (G_STRFUNC);

  return gtk_label_new ("This is a configuration dialog for the HelloWorld plugin.");
}

static void
peasdemo_hello_world_configurable_class_init (PeasDemoHelloWorldConfigurableClass *klass)
{
}

static void
peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface)
{
  iface->create_configure_widget = peasdemo_hello_world_configurable_create_configure_widget;
}

static void
peasdemo_hello_world_configurable_class_finalize (PeasDemoHelloWorldConfigurableClass *klass)
{
}

void
peasdemo_hello_world_configurable_register (GTypeModule *module)
{
  peasdemo_hello_world_configurable_register_type (module);
}
