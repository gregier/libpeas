#ifndef __PEASDEMO_HELLO_WORLD_CONFIGURABLE_H__
#define __PEASDEMO_HELLO_WORLD_CONFIGURABLE_H__

#include <gtk/gtk.h>
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define PEASDEMO_TYPE_HELLO_WORLD_CONFIGURABLE         (peasdemo_hello_world_configurable_get_type ())
#define PEASDEMO_HELLO_WORLD_CONFIGURABLE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PEASDEMO_TYPE_HELLO_WORLD_CONFIGURABLE, PeasDemoHelloWorldConfigurable))
#define PEASDEMO_HELLO_WORLD_CONFIGURABLE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PEASDEMO_TYPE_HELLO_WORLD_CONFIGURABLE, PeasDemoHelloWorldConfigurable))
#define PEASDEMO_IS_HELLO_WORLD_CONFIGURABLE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PEASDEMO_TYPE_HELLO_WORLD_CONFIGURABLE))
#define PEASDEMO_IS_HELLO_WORLD_CONFIGURABLE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PEASDEMO_TYPE_HELLO_WORLD_CONFIGURABLE))
#define PEASDEMO_HELLO_WORLD_CONFIGURABLE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PEASDEMO_TYPE_HELLO_WORLD_CONFIGURABLE, PeasDemoHelloWorldConfigurableClass))

typedef struct _PeasDemoHelloWorldConfigurable      PeasDemoHelloWorldConfigurable;
typedef struct _PeasDemoHelloWorldConfigurableClass PeasDemoHelloWorldConfigurableClass;

struct _PeasDemoHelloWorldConfigurable {
  PeasExtensionBase parent;
};

struct _PeasDemoHelloWorldConfigurableClass {
  PeasExtensionBaseClass parent_class;
};

GType   peasdemo_hello_world_configurable_get_type  (void) G_GNUC_CONST;
void    peasdemo_hello_world_configurable_register  (GTypeModule *module);

G_END_DECLS

#endif /* __PeasDEMO_HELLO_WORLD_CONFIGURABLE_H__ */
