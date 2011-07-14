/*
 * peas-plugin-store-model.h
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

#ifndef __PEAS_GTK_PLUGIN_STORE_MODEL_H__
#define __PEAS_GTK_PLUGIN_STORE_MODEL_H__

#include <gtk/gtk.h>
#include <libpeas/peas-engine.h>

#include "peas-gtk-installable-plugin-info.h"

G_BEGIN_DECLS

typedef enum {
  PEAS_GTK_PLUGIN_STORE_MODEL_INSTALLED_COLUMN = 0,
  PEAS_GTK_PLUGIN_STORE_MODEL_ACTIVE_COLUMN,
  PEAS_GTK_PLUGIN_STORE_MODEL_PULSE_COLUMN,
  PEAS_GTK_PLUGIN_STORE_MODEL_PROGRESS_COLUMN,
  PEAS_GTK_PLUGIN_STORE_MODEL_ICON_PIXBUF_COLUMN,
  PEAS_GTK_PLUGIN_STORE_MODEL_ICON_NAME_COLUMN,
  PEAS_GTK_PLUGIN_STORE_MODEL_INFO_COLUMN,
  /*PEAS_GTK_PLUGIN_STORE_MODEL_CANCELLABLE_COLUMN,*/
  PEAS_GTK_PLUGIN_STORE_MODEL_PLUGIN_COLUMN,
  PEAS_GTK_PLUGIN_STORE_MODEL_N_COLUMNS
} PeasGtkPluginStoreModelColumns;

/*
 * Type checking and casting macros
 */
#define PEAS_GTK_TYPE_PLUGIN_STORE_MODEL            (peas_gtk_plugin_store_model_get_type())
#define PEAS_GTK_PLUGIN_STORE_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PEAS_GTK_TYPE_PLUGIN_STORE_MODEL, PeasGtkPluginStoreModel))
#define PEAS_GTK_PLUGIN_STORE_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PEAS_GTK_TYPE_PLUGIN_STORE_MODEL, PeasGtkPluginStoreModelClass))
#define PEAS_GTK_IS_PLUGIN_STORE_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PEAS_GTK_TYPE_PLUGIN_STORE_MODEL))
#define PEAS_GTK_IS_PLUGIN_STORE_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), PEAS_GTK_TYPE_PLUGIN_STORE_MODEL))
#define PEAS_GTK_PLUGIN_STORE_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PEAS_GTK_TYPE_PLUGIN_STORE_MODEL, PeasGtkPluginStoreModelClass))

typedef struct _PeasGtkPluginStoreModel         PeasGtkPluginStoreModel;
typedef struct _PeasGtkPluginStoreModelClass    PeasGtkPluginStoreModelClass;
typedef struct _PeasGtkPluginStoreModelPrivate  PeasGtkPluginStoreModelPrivate;

struct _PeasGtkPluginStoreModel {
  GtkListStore parent;

  /*< private > */
  PeasGtkPluginStoreModelPrivate *priv;
};

struct _PeasGtkPluginStoreModelClass {
  GtkListStoreClass parent_class;
};

GType     peas_gtk_plugin_store_model_get_type   (void) G_GNUC_CONST;
PeasGtkPluginStoreModel *
          peas_gtk_plugin_store_model_new        (PeasEngine                         *engine);

void      peas_gtk_plugin_store_model_reload     (PeasGtkPluginStoreModel            *model,
                                                  GPtrArray                          *plugins);
void      peas_gtk_plugin_store_model_reload_plugin
                                                 (PeasGtkPluginStoreModel            *model,
                                                  const PeasGtkInstallablePluginInfo *info);

PeasGtkInstallablePluginInfo *
          peas_gtk_plugin_store_model_get_plugin (PeasGtkPluginStoreModel            *model,
                                                  GtkTreeIter                        *iter);

gboolean  peas_gtk_plugin_store_model_get_iter_from_plugin
                                                 (PeasGtkPluginStoreModel            *model,
                                                  GtkTreeIter                        *iter,
                                                  const PeasGtkInstallablePluginInfo *info);

G_END_DECLS

#endif /* __PEAS_GTK_PLUGIN_STORE_MODEL_H__  */
