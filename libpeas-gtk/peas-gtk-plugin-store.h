/*
 * peas-plugin-store.h
 * This file is part of libpeas
 *
 * Copyright (C) 2011 Garrett Regier
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

#ifndef _PEAS_GTK_PLUGIN_STORE_H_
#define _PEAS_GTK_PLUGIN_STORE_H_

#include <gtk/gtk.h>
#include <libpeas/peas-engine.h>

#include "peas-gtk-installable-plugin-info.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PEAS_GTK_TYPE_PLUGIN_STORE             (peas_gtk_plugin_store_get_type())
#define PEAS_GTK_PLUGIN_STORE(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), PEAS_GTK_TYPE_PLUGIN_STORE, PeasGtkPluginStore))
#define PEAS_GTK_PLUGIN_STORE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), PEAS_GTK_TYPE_PLUGIN_STORE, PeasGtkPluginStoreClass))
#define PEAS_GTK_IS_PLUGIN_STORE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), PEAS_GTK_TYPE_PLUGIN_STORE))
#define PEAS_GTK_IS_PLUGIN_STORE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), PEAS_GTK_TYPE_PLUGIN_STORE))
#define PEAS_GTK_PLUGIN_STORE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), PEAS_GTK_TYPE_PLUGIN_STORE, PeasGtkPluginStoreClass))

typedef struct _PeasGtkPluginStore        PeasGtkPluginStore;
typedef struct _PeasGtkPluginStoreClass   PeasGtkPluginStoreClass;
typedef struct _PeasGtkPluginStorePrivate PeasGtkPluginStorePrivate;

struct _PeasGtkPluginStore {
  GtkBox parent;

  /*< private > */
  PeasGtkPluginStorePrivate *priv;
};

struct _PeasGtkPluginStoreClass {
  GtkBoxClass parent_class;

  void (*back) (PeasGtkPluginStore *store);
};

GType      peas_gtk_plugin_store_get_type            (void) G_GNUC_CONST;

GtkWidget *peas_gtk_plugin_store_new                 (PeasEngine                   *engine);

void       peas_gtk_plugin_store_set_selected_plugin (PeasGtkPluginStore           *store,
                                                      PeasGtkInstallablePluginInfo *info);
PeasGtkInstallablePluginInfo *
           peas_gtk_plugin_store_get_selected_plugin (PeasGtkPluginStore           *store);

void       peas_gtk_plugin_store_install_plugin      (PeasGtkPluginStore           *store,
                                                      PeasGtkInstallablePluginInfo *info);
void       peas_gtk_plugin_store_uninstall_plugin    (PeasGtkPluginStore           *store,
                                                      PeasGtkInstallablePluginInfo *info);

G_END_DECLS

#endif /* _PEAS_GTK_PLUGIN_STORE_H_  */
