/*
 * peas-ui-configurable.h
 * This file is part of libpeas
 *
 * Copyright (C) 2009 Steve Frécinaux
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __PEAS_UI_CONFIGURABLE_H__
#define __PEAS_UI_CONFIGURABLE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PEAS_UI_TYPE_CONFIGURABLE              (peas_ui_configurable_get_type ())
#define PEAS_UI_CONFIGURABLE(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), PEAS_UI_TYPE_CONFIGURABLE, PeasUIConfigurable))
#define PEAS_UI_CONFIGURABLE_IFACE(obj)        (G_TYPE_CHECK_CLASS_CAST ((obj), PEAS_UI_TYPE_CONFIGURABLE, PeasUIConfigurableInterface))
#define PEAS_UI_IS_CONFIGURABLE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PEAS_UI_TYPE_CONFIGURABLE))
#define PEAS_UI_CONFIGURABLE_GET_IFACE(obj)    (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PEAS_UI_TYPE_CONFIGURABLE, PeasUIConfigurableInterface))

typedef struct _PeasUIConfigurable          PeasUIConfigurable; /* dummy typedef */
typedef struct _PeasUIConfigurableInterface     PeasUIConfigurableInterface;

struct _PeasUIConfigurableInterface
{
  GTypeInterface g_iface;

  gboolean    (*create_configure_dialog)  (PeasUIConfigurable *configurable,
                                           GtkWidget         **conf_dlg);

  /* Plugins should usually not override this, it's handled automatically
   * by the PeasPluginClass */
  gboolean    (*is_configurable)          (PeasUIConfigurable *configurable);
};

GType       peas_ui_configurable_get_type                (void);
gboolean    peas_ui_configurable_is_configurable         (PeasUIConfigurable *configurable);
gboolean    peas_ui_configurable_create_configure_dialog (PeasUIConfigurable *configurable,
                                                          GtkWidget         **conf_dlg);

G_END_DECLS

#endif /* __PEAS_UI_PLUGIN_MANAGER_H__  */
