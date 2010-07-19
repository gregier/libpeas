/*
 * peas-plugin-manager-store.h
 * This file is part of libpeas
 *
 * Copyright (C) 2002 Paolo Maggi and James Willcox
 * Copyright (C) 2003-2006 Paolo Maggi, Paolo Borelli
 * Copyright (C) 2007-2009 Paolo Maggi, Paolo Borelli, Steve Frécinaux
 * Copyright (C) 2010 Garrett Regier
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

#ifndef __PEAS_UI_PLUGIN_MANAGER_STORE_H__
#define __PEAS_UI_PLUGIN_MANAGER_STORE_H__

#include <gtk/gtk.h>
#include <libpeas/peas-engine.h>
#include <libpeas/peas-plugin-info.h>

G_BEGIN_DECLS

enum {
  PEAS_UI_PLUGIN_MANAGER_STORE_ENABLED_COLUMN = 0,
  PEAS_UI_PLUGIN_MANAGER_STORE_CAN_ENABLE_COLUMN,
  PEAS_UI_PLUGIN_MANAGER_STORE_ICON_COLUMN,
  PEAS_UI_PLUGIN_MANAGER_STORE_ICON_VISIBLE_COLUMN,
  PEAS_UI_PLUGIN_MANAGER_STORE_INFO_COLUMN,
  PEAS_UI_PLUGIN_MANAGER_STORE_INFO_SENSITIVE_COLUMN,
  PEAS_UI_PLUGIN_MANAGER_STORE_PLUGIN_COLUMN,
  PEAS_UI_PLUGIN_MANAGER_STORE_N_COLUMNS
} PeasUIPluginManagerStoreColumns;

/*
 * Type checking and casting macros
 */
#define PEAS_UI_TYPE_PLUGIN_MANAGER_STORE             (peas_ui_plugin_manager_store_get_type())
#define PEAS_UI_PLUGIN_MANAGER_STORE(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), PEAS_UI_TYPE_PLUGIN_MANAGER_STORE, PeasUIPluginManagerStore))
#define PEAS_UI_PLUGIN_MANAGER_STORE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), PEAS_UI_TYPE_PLUGIN_MANAGER_STORE, PeasUIPluginManagerStoreClass))
#define PEAS_UI_IS_PLUGIN_MANAGER_STORE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), PEAS_UI_TYPE_PLUGIN_MANAGER_STORE))
#define PEAS_UI_IS_PLUGIN_MANAGER_STORE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), PEAS_UI_TYPE_PLUGIN_MANAGER_STORE))
#define PEAS_UI_PLUGIN_MANAGER_STORE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), PEAS_UI_TYPE_PLUGIN_MANAGER_STORE, PeasUIPluginManagerStoreClass))

typedef struct _PeasUIPluginManagerStore         PeasUIPluginManagerStore;
typedef struct _PeasUIPluginManagerStoreClass    PeasUIPluginManagerStoreClass;
typedef struct _PeasUIPluginManagerStorePrivate  PeasUIPluginManagerStorePrivate;

struct _PeasUIPluginManagerStore {
  GtkListStore parent;

  /*< private > */
  PeasUIPluginManagerStorePrivate *priv;
};

struct _PeasUIPluginManagerStoreClass {
  GtkListStoreClass parent_class;
};

GType                     peas_ui_plugin_manager_store_get_type             (void) G_GNUC_CONST;
PeasUIPluginManagerStore *peas_ui_plugin_manager_store_new                  (PeasEngine               *engine);

void                      peas_ui_plugin_manager_store_reload               (PeasUIPluginManagerStore *store);

void                      peas_ui_plugin_manager_store_set_enabled          (PeasUIPluginManagerStore *store,
                                                                             GtkTreeIter              *iter,
                                                                             gboolean                  enabled);
gboolean                  peas_ui_plugin_manager_store_get_enabled          (PeasUIPluginManagerStore *store,
                                                                             GtkTreeIter              *iter);
void                      peas_ui_plugin_manager_store_set_all_enabled      (PeasUIPluginManagerStore *store,
                                                                             gboolean                  enabled);
void                      peas_ui_plugin_manager_store_toggle_enabled       (PeasUIPluginManagerStore *store,
                                                                             GtkTreeIter              *iter);

gboolean                  peas_ui_plugin_manager_store_can_enable           (PeasUIPluginManagerStore *store,
                                                                             GtkTreeIter              *iter);

PeasPluginInfo           *peas_ui_plugin_manager_store_get_plugin           (PeasUIPluginManagerStore *store,
                                                                             GtkTreeIter              *iter);

gboolean                  peas_ui_plugin_manager_store_get_iter_from_plugin (PeasUIPluginManagerStore *store,
                                                                             GtkTreeIter              *iter,
                                                                             const PeasPluginInfo     *info);
G_END_DECLS

#endif /* __PEAS_UI_PLUGIN_MANAGER_STORE_H__  */
