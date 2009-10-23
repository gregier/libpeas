#ifndef __GPEDEMO_HELLO_WORLD_PLUGIN_H__
#define __GPEDEMO_HELLO_WORLD_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <libgpe/gpe-plugin.h>

G_BEGIN_DECLS

#define GPEDEMO_TYPE_HELLO_WORLD_PLUGIN		(gpedemo_hello_world_plugin_get_type ())
#define GPEDEMO_HELLO_WORLD_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), GPEDEMO_TYPE_HELLO_WORLD_PLUGIN, GPEDemoHelloWorldPlugin))
#define GPEDEMO_HELLO_WORLD_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), GPEDEMO_TYPE_HELLO_WORLD_PLUGIN, GPEDemoHelloWorldPlugin))
#define GPEDEMO_IS_HELLO_WORLD_PLUGIN(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), GPEDEMO_TYPE_HELLO_WORLD_PLUGIN))
#define GPEDEMO_IS_HELLO_WORLD_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), GPEDEMO_TYPE_HELLO_WORLD_PLUGIN))
#define GPEDEMO_HELLO_WORLD_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GPEDEMO_TYPE_HELLO_WORLD_PLUGIN, GPEDemoHelloWorldPluginClass))

typedef struct _GPEDemoHelloWorldPlugin		GPEDemoHelloWorldPlugin;
typedef struct _GPEDemoHelloWorldPluginClass	GPEDemoHelloWorldPluginClass;

struct _GPEDemoHelloWorldPlugin
{
	GPEPlugin parent_instance;
};

struct _GPEDemoHelloWorldPluginClass
{
	GPEPluginClass parent_class;
};

GType	gpedemo_hello_world_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_gpe_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __GPEDEMO_HELLO_WORLD_PLUGIN_H__ */
