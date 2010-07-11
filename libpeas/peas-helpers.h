#ifndef __PEAS_HELPERS_H__
#define __PEAS_HELPERS_H__

#include <glib-object.h>

gpointer  _g_type_struct_ref        (GType         the_type);
void      _g_type_struct_unref      (GType         the_type,
                                     gpointer      type_struct);

gboolean  _valist_to_parameter_list (GType         the_type,
                                     gpointer      type_struct,
                                     const gchar  *first_property_name,
                                     va_list       var_args,
                                     GParameter  **params,
                                     guint        *n_params);

#endif /* __PEAS_HELPERS_H__ */
