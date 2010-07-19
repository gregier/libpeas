/*
 * peas-plugin-manager-view.h
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

#ifndef __PEAS_UI_PLUGIN_MANAGER_VIEW_H__
#define __PEAS_UI_PLUGIN_MANAGER_VIEW_H__

#include <gtk/gtk.h>
#include <libpeas/peas-engine.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define PEAS_UI_TYPE_PLUGIN_MANAGER_VIEW             (peas_ui_plugin_manager_view_get_type())
#define PEAS_UI_PLUGIN_MANAGER_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), PEAS_UI_TYPE_PLUGIN_MANAGER_VIEW, PeasUIPluginManagerView))
#define PEAS_UI_PLUGIN_MANAGER_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), PEAS_UI_TYPE_PLUGIN_MANAGER_VIEW, PeasUIPluginManagerViewClass))
#define PEAS_UI_IS_PLUGIN_MANAGER_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), PEAS_UI_TYPE_PLUGIN_MANAGER_VIEW))
#define PEAS_UI_IS_PLUGIN_MANAGER_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), PEAS_UI_TYPE_PLUGIN_MANAGER_VIEW))
#define PEAS_UI_PLUGIN_MANAGER_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), PEAS_UI_TYPE_PLUGIN_MANAGER_VIEW, PeasUIPluginManagerViewClass))

typedef struct _PeasUIPluginManagerView         PeasUIPluginManagerView;
typedef struct _PeasUIPluginManagerViewClass    PeasUIPluginManagerViewClass;
typedef struct _PeasUIPluginManagerViewPrivate  PeasUIPluginManagerViewPrivate;

struct _PeasUIPluginManagerView {
  GtkTreeView parent;

  /*< private > */
  PeasUIPluginManagerViewPrivate *priv;
};

struct _PeasUIPluginManagerViewClass {
  GtkTreeViewClass parent_class;

  void  (*populate_popup)  (PeasUIPluginManagerView *view,
                            GtkMenu                 *menu);
};

GType           peas_ui_plugin_manager_view_get_type            (void) G_GNUC_CONST;
GtkWidget      *peas_ui_plugin_manager_view_new                 (PeasEngine              *engine);

void            peas_ui_plugin_manager_view_set_show_builtin    (PeasUIPluginManagerView *view,
                                                                 gboolean                 show_builtin);
gboolean        peas_ui_plugin_manager_view_get_show_builtin    (PeasUIPluginManagerView *view);

void            peas_ui_plugin_manager_view_set_selected_iter   (PeasUIPluginManagerView *view,
                                                                 GtkTreeIter             *iter);
gboolean        peas_ui_plugin_manager_view_get_selected_iter   (PeasUIPluginManagerView *view,
                                                                 GtkTreeIter             *iter);

void            peas_ui_plugin_manager_view_set_selected_plugin (PeasUIPluginManagerView *view,
                                                                 PeasPluginInfo          *info);
PeasPluginInfo *peas_ui_plugin_manager_view_get_selected_plugin (PeasUIPluginManagerView *view);

G_END_DECLS

#endif /* __PEAS_UI_PLUGIN_MANAGER_VIEW_H__  */
