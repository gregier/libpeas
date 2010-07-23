#ifndef __PEASDEMO_SECOND_TIME_H__
#define __PEASDEMO_SECOND_TIME_H__

#include <gtk/gtk.h>
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define PEASDEMO_TYPE_SECOND_TIME         (peasdemo_second_time_get_type ())
#define PEASDEMO_SECOND_TIME(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PEASDEMO_TYPE_SECOND_TIME, PeasDemoSecondTime))
#define PEASDEMO_SECOND_TIME_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PEASDEMO_TYPE_SECOND_TIME, PeasDemoSecondTime))
#define PEASDEMO_IS_SECOND_TIME(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PEASDEMO_TYPE_SECOND_TIME))
#define PEASDEMO_IS_SECOND_TIME_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PEASDEMO_TYPE_SECOND_TIME))
#define PEASDEMO_SECOND_TIME_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PEASDEMO_TYPE_SECOND_TIME, PeasDemoSecondTimeClass))

typedef struct _PeasDemoSecondTime       PeasDemoSecondTime;
typedef struct _PeasDemoSecondTimeClass  PeasDemoSecondTimeClass;

struct _PeasDemoSecondTime {
  PeasExtensionBase parent_instance;

  GtkWidget *window;
  GtkWidget *label;
};

struct _PeasDemoSecondTimeClass {
  PeasExtensionBaseClass parent_class;
};

GType                 peasdemo_second_time_get_type               (void) G_GNUC_CONST;
G_MODULE_EXPORT void  peas_register_types                         (PeasObjectModule *module);

G_END_DECLS

#endif /* __PeasDEMO_HELLO_WORLD_PLUGIN_H__ */
